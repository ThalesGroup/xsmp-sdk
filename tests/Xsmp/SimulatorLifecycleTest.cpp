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

#include <Smp/IPersist.h>
#include <Smp/IPublication.h>
#include <Smp/IStorageReader.h>
#include <Smp/IStorageWriter.h>
#include <Smp/InvalidSimulatorState.h>
#include <Smp/PrimitiveTypes.h>
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
  sim.Store(directory.c_str());
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
  sim.Hold(true);
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
  sim.Reconnect(assembly);
}

} // namespace Xsmp
