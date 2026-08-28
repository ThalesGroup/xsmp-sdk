// Copyright 2023 THALES ALENIA SPACE FRANCE. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <Smp/CannotRestore.h>
#include <Smp/IPersist.h>
#include <Smp/IPublication.h>
#include <Smp/IStorageReader.h>
#include <Smp/IStorageWriter.h>
#include <Smp/InvalidSimulatorState.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/IScheduler.h>
#include <Smp/Services/ITimeKeeper.h>
#include <Smp/SimulatorStateKind.h>
#include <Xsmp/Composite.h>
#include <Xsmp/Container.h>
#include <Xsmp/Duration.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/EntryPointPublisher.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Model.h>
#include <Xsmp/Service.h>
#include <Xsmp/Simulator.h>
#include <cstddef>
#include <filesystem>
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

namespace Xsmp {
namespace {

/// A model holding one state field, so that the simulation has a state vector
/// to store and restore.
class Counter final : public Model, public virtual ::Smp::IPersist {
public:
  using Model::Model;
  void Publish(::Smp::IPublication *receiver) override {
    Model::Publish(receiver);
    receiver->PublishField("count", "", &count);
  }
  void Store(::Smp::IStorageWriter *writer) override {
    writer->Store(&count, sizeof(count));
  }
  void Restore(::Smp::IStorageReader *reader) override {
    reader->Restore(&count, sizeof(count));
  }
  ::Smp::Int32 count{};
};

/// A model whose whole state is the field it publishes to the simulation
/// environment.
class Gauge final : public Model {
public:
  using Model::Model;
  void Publish(::Smp::IPublication *receiver) override {
    Model::Publish(receiver);
    receiver->PublishField("value", "", &value);
  }
  ::Smp::Int32 value{};
};

/// A model owning other models, the root Reconnect walks through.
class Assembly final : public Model, public virtual ::Xsmp::Composite {
public:
  using Model::Model;
  Container<Counter> counters{"counters", "", this, 0, -1};
};

/// A model publishing the same field name as Counter, with a wider type.
class WideCounter final : public Model {
public:
  using Model::Model;
  void Publish(::Smp::IPublication *receiver) override {
    Model::Publish(receiver);
    receiver->PublishField("count", "", &count);
  }
  ::Smp::Int64 count{};
};

/// A model reading its published fields while being destroyed, as one
/// releasing resources tied to them would.
class SelfReadingModel final : public Model {
public:
  using Model::Model;
  void Publish(::Smp::IPublication *receiver) override {
    Model::Publish(receiver);
    receiver->PublishField("value", "", &value);
  }
  ~SelfReadingModel() override {
    if (const auto *fields = GetFields()) {
      seen = fields->size();
    }
  }
  ::Smp::Int32 value{};
  std::size_t seen{};
};

} // namespace

TEST(SimulatorLifecycle, StoreAndRestore) {

  const auto directory =
      (std::filesystem::temp_directory_path() / "xsmp-store-test").string();
  std::filesystem::remove_all(directory);

  Simulator sim;
  sim.LoadLibrary("xsmp_services");

  auto *counter = new Counter("counter", "", &sim, &sim);
  sim.AddModel(counter);
  auto *gauge = new Gauge("gauge", "", &sim, &sim);
  sim.AddModel(gauge);
  sim.Connect();
  ASSERT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);

  counter->count = 42;
  gauge->value = 7;
  sim.Store(directory.c_str());
  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);
  EXPECT_TRUE(std::filesystem::exists(directory + "/simulator.bin"));
  EXPECT_TRUE(std::filesystem::exists(directory + "/components.bin"));

  counter->count = 0;
  gauge->value = 0;
  sim.Restore(directory.c_str());
  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);
  // the state a model persists itself and the state it publishes are both
  // part of the simulation state
  EXPECT_EQ(counter->count, 42);
  EXPECT_EQ(gauge->value, 7);

  sim.Exit();
  // storing is refused outside of the Standby state
  EXPECT_THROW(sim.Store(directory.c_str()), ::Smp::InvalidSimulatorState);
  std::filesystem::remove_all(directory);
}

TEST(SimulatorLifecycle, InitEntryPointsAndHold) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");

  ::Smp::Int32 initialised = 0;
  ::Xsmp::EntryPoint init{"init", "", &sim, [&] { ++initialised; }};
  sim.AddInitEntryPoint(&init);

  sim.Connect();
  // the initialisation entry points are executed once, during Connect
  EXPECT_EQ(initialised, 1);

  // holding outside of the Executing state is refused and does not change it
  EXPECT_THROW(sim.Hold(true), ::Smp::InvalidSimulatorState);
  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);

  // a non immediate hold only takes effect at the next simulation time change
  ::Xsmp::EntryPoint hold{"hold", "", &sim, [&] { sim.Hold(false); }};
  sim.GetScheduler()->AddSimulationTimeEvent(&hold, 1_ms);
  sim.Run(10_ms);
  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);
  EXPECT_GE(sim.GetTimeKeeper()->GetSimulationTime(), 1_ms);

  sim.Exit();
}

/// An entry point that throws aborts the simulation instead of letting the
/// exception escape.
template <typename Thrower> void testFailingEntryPoint(Thrower &&thrower) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();

  ::Xsmp::EntryPoint failing{"failing", "", &sim, thrower};
  sim.GetScheduler()->AddSimulationTimeEvent(&failing, 1_ms);

  EXPECT_NO_THROW(sim.Run(10_ms));
  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Aborting);
}

TEST(SimulatorLifecycle, FailingEntryPoint) {

  // an SMP exception
  testFailingEntryPoint([] {
    ::Xsmp::Exception::throwInvalidObjectName(nullptr, "!invalid",
                                              "invalid name");
  });
  // a standard C++ exception
  testFailingEntryPoint([] { throw std::runtime_error("failure"); });
  // anything else
  testFailingEntryPoint([] { throw 42; });
}

TEST(SimulatorLifecycle, AddServiceOutsideBuilding) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");

  // the simulator owns the services it is given
  auto *building = new Service("building", "", &sim);
  EXPECT_NO_THROW(sim.AddService(building));

  sim.Connect();

  // a service can only be added while the simulation is being built
  Service standby{"standby", "", &sim};
  EXPECT_THROW(sim.AddService(&standby), ::Smp::InvalidSimulatorState);
  sim.Exit();
}

TEST(SimulatorLifecycle, Reconnect) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  auto *assembly = new Assembly("assembly", "", &sim, &sim);
  sim.AddModel(assembly);
  sim.Connect();

  // a model added after the connection is published, configured and connected
  // by Reconnect
  auto *counter = new Counter("counter", "", assembly);
  assembly->counters.AddComponent(counter);
  EXPECT_EQ(counter->GetState(), ::Smp::ComponentStateKind::CSK_Created);

  sim.Reconnect(assembly);
  EXPECT_EQ(counter->GetState(), ::Smp::ComponentStateKind::CSK_Connected);
  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);

  sim.Exit();
  // reconnecting is refused outside of the Standby state
  EXPECT_THROW(sim.Reconnect(assembly), ::Smp::InvalidSimulatorState);
}

TEST(SimulatorLifecycle, OperationsRefusedOutsideTheirState) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  ASSERT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);

  // SMP 2025 raises InvalidSimulatorState where SMP 2020 returned without
  // acting: the operations of the building phase are refused once connected
  EXPECT_THROW(sim.Publish(), ::Smp::InvalidSimulatorState);
  EXPECT_THROW(sim.Configure(), ::Smp::InvalidSimulatorState);
  EXPECT_THROW(sim.Connect(), ::Smp::InvalidSimulatorState);
  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);

  // holding is refused outside the executing state
  EXPECT_THROW(sim.Hold(true), ::Smp::InvalidSimulatorState);
  EXPECT_THROW(sim.Hold(false), ::Smp::InvalidSimulatorState);
  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);

  sim.Exit();
  ASSERT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Exiting);

  // and everything is refused once the simulation has exited
  const auto directory =
      (std::filesystem::temp_directory_path() / "xsmp-refused").string();
  EXPECT_THROW(sim.Publish(), ::Smp::InvalidSimulatorState);
  EXPECT_THROW(sim.Configure(), ::Smp::InvalidSimulatorState);
  EXPECT_THROW(sim.Connect(), ::Smp::InvalidSimulatorState);
  EXPECT_THROW(sim.Initialise(), ::Smp::InvalidSimulatorState);
  EXPECT_THROW(sim.Run(), ::Smp::InvalidSimulatorState);
  EXPECT_THROW(sim.Hold(true), ::Smp::InvalidSimulatorState);
  EXPECT_THROW(sim.Store(directory.c_str()), ::Smp::InvalidSimulatorState);
  EXPECT_THROW(sim.Reconnect(nullptr), ::Smp::InvalidSimulatorState);
  EXPECT_THROW(sim.Exit(), ::Smp::InvalidSimulatorState);

  // Restore is the exception: it still returns without acting
  EXPECT_NO_THROW(sim.Restore(directory.c_str()));

  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Exiting);
  EXPECT_FALSE(std::filesystem::exists(directory));
}

TEST(SimulatorLifecycle, ModelReadsItsFieldsWhileBeingDestroyed) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.AddModel(new SelfReadingModel("model", "", &sim, &sim));
  sim.Connect();

  // the publications outlive the components they describe, so a model can
  // still use them while the simulator destroys it
  sim.Exit();
}

TEST(SimulatorLifecycle, AnInterruptedRestoreLeavesTheSimulationUsable) {

  const auto directory =
      (std::filesystem::temp_directory_path() / "xsmp-interrupted").string();
  std::filesystem::remove_all(directory);

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  auto *counter = new Counter("counter", "", &sim, &sim);
  sim.AddModel(counter);
  sim.Connect();

  // restoring from a state that does not exist fails without leaving the
  // simulation in the Restoring state
  EXPECT_THROW(sim.Restore(directory.c_str()), ::Smp::CannotRestore);
  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);

  // so that a correct store and restore still works afterwards
  counter->count = 13;
  sim.Store(directory.c_str());
  counter->count = 0;
  sim.Restore(directory.c_str());
  EXPECT_EQ(counter->count, 13);

  sim.Exit();
  std::filesystem::remove_all(directory);
}

TEST(SimulatorLifecycle, RestoringAStateOfAnotherModelTreeIsRefused) {

  const auto directory =
      (std::filesystem::temp_directory_path() / "xsmp-other-tree").string();
  std::filesystem::remove_all(directory);

  {
    Simulator sim;
    sim.LoadLibrary("xsmp_services");
    auto *counter = new Counter("counter", "", &sim, &sim);
    sim.AddModel(counter);
    sim.Connect();
    counter->count = 42;
    sim.Store(directory.c_str());
    sim.Exit();
  }
  {
    // the same path, holding a field of a wider type: restoring would read the
    // stored bytes into a field they do not belong to
    Simulator sim;
    sim.LoadLibrary("xsmp_services");
    auto *counter = new WideCounter("counter", "", &sim, &sim);
    sim.AddModel(counter);
    sim.Connect();
    EXPECT_THROW(sim.Restore(directory.c_str()), ::Smp::CannotRestore);
    EXPECT_EQ(counter->count, 0);
    sim.Exit();
  }
  std::filesystem::remove_all(directory);
}

TEST(SimulatorLifecycle, AbortAfterExit) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  sim.Exit();

  // aborting is allowed from any state, including once the services have been
  // disconnected
  sim.Abort();
  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Aborting);
}

TEST(SimulatorLifecycle, TransitionRefusedDuringItsOwnGlobalEvent) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  ASSERT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);

  // Exit() announces itself with SMP_LeaveStandby; a transition requested
  // from a subscriber of that event would run nested inside it, which SMP
  // 2025 rejects
  ::Smp::Int32 caught = 0;
  ::Xsmp::EntryPoint reentrant{
      "reentrant", "", &sim, [&sim, &caught] {
        EXPECT_THROW(sim.Run(), ::Smp::InvalidSimulatorState);
        EXPECT_THROW(sim.Exit(), ::Smp::InvalidSimulatorState);
        EXPECT_THROW(sim.Initialise(), ::Smp::InvalidSimulatorState);
        ++caught;
      }};
  sim.GetEventManager()->Subscribe(
      ::Smp::Services::IEventManager::SMP_LeaveStandbyId, &reentrant);

  sim.Exit();
  EXPECT_EQ(caught, 1);
  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Exiting);
}

} // namespace Xsmp
