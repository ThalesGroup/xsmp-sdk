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

#include "../Storage.h"
#include "Xsmp/Component.h"
#include <Smp/IPersist.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/EventId.h>
#include <Smp/Services/ITimeKeeper.h>
#include <Smp/Services/InvalidCycleTime.h>
#include <Smp/Services/InvalidEventId.h>
#include <Smp/Services/InvalidEventTime.h>
#include <Smp/SimulatorStateKind.h>
#include <Xsmp/Duration.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/Model.h>
#include <Xsmp/Services/XsmpScheduler.h>
#include <Xsmp/Simulator.h>
#include <atomic>
#include <chrono>
#include <limits>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

namespace Xsmp::Services {
namespace {
class TestEntryPointPublisher : public Xsmp::Component,
                                public virtual Xsmp::EntryPointPublisher {
public:
  using Xsmp::Component::Component;
};

/// A model owning its entry point, so that the scheduler can resolve it by
/// path when restoring its state.
class TestModel final : public Xsmp::Model,
                        public virtual Xsmp::EntryPointPublisher {
public:
  using Xsmp::Model::Model;
  ::Xsmp::EntryPoint ep{"ep", "", this, [] {}};
};
} // namespace
TEST(XsmpScheduler, run) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto &scheduler =
      *dynamic_cast<Services::XsmpScheduler *>(sim.GetScheduler());
  scheduler.SetTargetSpeed(100.0);

  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};
  std::vector<int> results;
  ::Xsmp::EntryPoint ep4{"ep4", "", &entryPoints,
                         [&]() { results.push_back(4); }};
  ::Smp::Services::EventId ep2Id = -1;
  ::Xsmp::EntryPoint ep1{"ep1", "", &entryPoints,
                         [&] { results.push_back(1); }};
  ::Xsmp::EntryPoint ep2{"ep2", "", &entryPoints, [&] {
                           sim.GetScheduler()->AddImmediateEvent(&ep4);
                           results.push_back(2);
                           EXPECT_EQ(scheduler.GetCurrentEventId(), ep2Id);
                         }};
  ::Xsmp::EntryPoint ep3{"ep3", "", &entryPoints,
                         [&] { results.push_back(3); }};

  sim.GetScheduler()->AddSimulationTimeEvent(&ep1, 0_ms, 0, 0);

  ep2Id = sim.GetScheduler()->AddMissionTimeEvent(&ep2, 1_ms, 1_ms, 1);
  sim.GetScheduler()->AddSimulationTimeEvent(&ep3, 2_ms, 0, 0);
  sim.GetScheduler()->AddSimulationTimeEvent(&ep1, 0_ms, 0, 0);
  sim.GetScheduler()->AddEpochTimeEvent(&ep3, 2_ms, 0, 0);
  sim.GetScheduler()->AddImmediateEvent(&ep4);

  EXPECT_EQ(sim.GetTimeKeeper()->GetSimulationTime(), 0_s);
  sim.Run(1_s);

  EXPECT_THROW(sim.GetScheduler()->AddSimulationTimeEvent(&ep1, -1_ms, 0, 0),
               ::Smp::Services::InvalidEventTime);
  EXPECT_THROW(sim.GetScheduler()->AddSimulationTimeEvent(&ep1, 0_ms, 0, 1),
               ::Smp::Services::InvalidCycleTime);

  const std::vector<int> expected = {4, 1, 1, 2, 4, 2, 4, 3, 3};
  EXPECT_EQ(results, expected);
}

class SpeedFixture : public ::testing::TestWithParam<double> {};
INSTANTIATE_TEST_SUITE_P(XsmpScheduler, SpeedFixture,
                         ::testing::Values(0.01, 0.25, 0.5, 1, 4, 10, 100));

TEST_P(SpeedFixture, Speed) {
  const double speed = GetParam();
  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto &scheduler =
      *dynamic_cast<Services::XsmpScheduler *>(sim.GetScheduler());
  scheduler.SetTargetSpeed(speed);
  auto runDuration = 100_ms;
  auto zuluTime = sim.GetTimeKeeper()->GetZuluTime() + runDuration;
  sim.Run(
      static_cast<::Smp::Duration>(static_cast<double>(runDuration) * speed));
  EXPECT_NEAR(static_cast<double>(zuluTime),
              static_cast<double>(sim.GetTimeKeeper()->GetZuluTime()),
              static_cast<double>(5_ms));
}

TEST(XsmpScheduler, zulu_events) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");

  sim.Connect();
  auto &scheduler =
      *dynamic_cast<Services::XsmpScheduler *>(sim.GetScheduler());

  scheduler.SetTargetSpeed(1.0);

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};

  std::vector<int> results;
  ::Xsmp::EntryPoint ep1{"ep1", "", &entryPoints, [&]() {
                           results.push_back(1);
                           std::this_thread::sleep_for(
                               std::chrono::milliseconds{10});
                         }};
  ::Xsmp::EntryPoint ep2{"ep2", "", &entryPoints, [&]() {
                           results.push_back(2);
                           std::this_thread::sleep_for(
                               std::chrono::milliseconds{10});
                         }};
  ::Xsmp::EntryPoint ep3{"ep3", "", &entryPoints, [&]() {
                           results.push_back(3);
                           std::this_thread::sleep_for(
                               std::chrono::milliseconds{10});
                         }};
  auto zuluTime = sim.GetTimeKeeper()->GetZuluTime() + 5_ms;
  sim.GetScheduler()->AddZuluTimeEvent(&ep1, zuluTime + 5_ms, 0, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds{5});
  sim.GetScheduler()->AddZuluTimeEvent(&ep2, zuluTime + 10_ms, 0, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds{5});
  sim.GetScheduler()->AddZuluTimeEvent(&ep3, zuluTime + 15_ms, 0, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds{5});
  sim.GetScheduler()->AddZuluTimeEvent(&ep1, zuluTime + 20_ms, 0, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds{5});
  sim.GetScheduler()->AddZuluTimeEvent(&ep2, zuluTime + 25_ms, 0, 0);
  std::this_thread::sleep_for(std::chrono::milliseconds{5});
  sim.GetScheduler()->AddZuluTimeEvent(&ep3, zuluTime + 30_ms, 0, 0);

  EXPECT_THROW(sim.GetScheduler()->AddZuluTimeEvent(&ep3, 0, 0, 0),
               ::Smp::Services::InvalidEventTime);
  EXPECT_THROW(
      sim.GetScheduler()->AddZuluTimeEvent(&ep3, zuluTime + 300_ms, 0, 1),
      ::Smp::Services::InvalidCycleTime);

  // sim.Run(1_ms);
  // check that in standby state zulu events are executed
  std::this_thread::sleep_for(std::chrono::milliseconds{400});
  sim.Exit();
  const std::vector<int> expected = {1, 2, 3, 1, 2, 3};
  EXPECT_EQ(results, expected);
}

TEST(XsmpScheduler, EventTime) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto &scheduler =
      *dynamic_cast<Services::XsmpScheduler *>(sim.GetScheduler());
  scheduler.SetTargetSpeed(100.0);

  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};

  ::Xsmp::EntryPoint ep1{"ep1", "", &entryPoints, [&]() {
                           // do nothing
                         }};

  EXPECT_THROW(scheduler.SetEventSimulationTime(10, 2_ms),
               ::Smp::Services::InvalidEventId);

  auto id = scheduler.AddSimulationTimeEvent(&ep1, 1_ms, 0, 0);

  EXPECT_EQ(scheduler.GetNextScheduledEventTime(), 1_ms);

  scheduler.SetEventSimulationTime(id, 2_ms);
  EXPECT_EQ(scheduler.GetNextScheduledEventTime(), 2_ms);

  EXPECT_THROW(scheduler.SetEventMissionTime(id, 2_ms),
               ::Smp::Services::InvalidEventId);
  EXPECT_THROW(scheduler.SetEventEpochTime(id, 2_ms),
               ::Smp::Services::InvalidEventId);
  EXPECT_THROW(scheduler.SetEventZuluTime(id, 2_ms),
               ::Smp::Services::InvalidEventId);
}

TEST(XsmpScheduler, EventCycleAndRepeatTime) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto &scheduler =
      *dynamic_cast<Services::XsmpScheduler *>(sim.GetScheduler());
  scheduler.SetTargetSpeed(100.0);

  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};

  ::Xsmp::EntryPoint ep1{"ep1", "", &entryPoints, [&]() {
                           // do nothing
                         }};

  EXPECT_THROW(scheduler.SetEventSimulationTime(10, 2_ms),
               ::Smp::Services::InvalidEventId);

  auto id = scheduler.AddSimulationTimeEvent(&ep1, 1_ms, 0, 0);

  EXPECT_EQ(scheduler.GetNextScheduledEventTime(), 1_ms);

  scheduler.SetEventSimulationTime(id, 2_ms);
  EXPECT_EQ(scheduler.GetNextScheduledEventTime(), 2_ms);

  EXPECT_THROW(scheduler.SetEventRepeat(id, 1),
               ::Smp::Services::InvalidCycleTime);

  EXPECT_NO_THROW(scheduler.SetEventCycleTime(id, -1_ms));

  EXPECT_THROW(scheduler.SetEventRepeat(id, 1),
               ::Smp::Services::InvalidCycleTime);

  EXPECT_NO_THROW(scheduler.SetEventCycleTime(id, 1_ms));

  EXPECT_NO_THROW(scheduler.SetEventRepeat(id, 1));

  EXPECT_THROW(scheduler.SetEventCycleTime(id, 0),
               ::Smp::Services::InvalidCycleTime);

  EXPECT_THROW(scheduler.SetEventCycleTime(10, 2_ms),
               ::Smp::Services::InvalidEventId);
  EXPECT_THROW(scheduler.SetEventRepeat(10, 2),
               ::Smp::Services::InvalidEventId);
}

TEST(XsmpScheduler, RemoveEvent) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto &scheduler =
      *dynamic_cast<Services::XsmpScheduler *>(sim.GetScheduler());
  scheduler.SetTargetSpeed(100.0);

  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};

  ::Xsmp::EntryPoint ep1{"ep1", "", &entryPoints, [&]() {
                           // do nothing
                         }};

  EXPECT_THROW(scheduler.RemoveEvent(10), ::Smp::Services::InvalidEventId);

  auto id = scheduler.AddSimulationTimeEvent(&ep1, 1_ms, 0, 0);

  EXPECT_NO_THROW(scheduler.RemoveEvent(id));
}

TEST(XsmpScheduler, RemoveZuluTimeEvent) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};
  ::Xsmp::EntryPoint ep{"ep", "", &entryPoints, [] {}};

  // far enough in the future for the zulu thread not to execute it
  auto zuluTime = sim.GetTimeKeeper()->GetZuluTime() + 1_h;
  auto eventId = sim.GetScheduler()->AddZuluTimeEvent(&ep, zuluTime);

  sim.GetScheduler()->RemoveEvent(eventId);

  // the event is gone from both the event map and the zulu table
  EXPECT_THROW(sim.GetScheduler()->RemoveEvent(eventId),
               ::Smp::Services::InvalidEventId);
  EXPECT_THROW(sim.GetScheduler()->SetEventZuluTime(eventId, zuluTime),
               ::Smp::Services::InvalidEventId);

  // a simulation time event is not affected by the removal
  auto simEventId = sim.GetScheduler()->AddSimulationTimeEvent(&ep, 1_ms);
  EXPECT_EQ(sim.GetScheduler()->GetNextScheduledEventTime(), 1_ms);
  sim.GetScheduler()->RemoveEvent(simEventId);
  sim.Exit();
}

TEST(XsmpScheduler, AddRelativeZuluTimeEvent) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};
  ::Xsmp::EntryPoint ep{"ep", "", &entryPoints, [] {}};

  auto *scheduler = sim.GetScheduler();

  // far enough in the future for the zulu thread not to execute it
  auto eventId = scheduler->AddRelativeZuluTimeEvent(&ep, 1_h);
  EXPECT_TRUE(scheduler->IsEventScheduled(eventId));

  // the delay is counted from the current zulu time, so the event landed in
  // the zulu table and not in the simulation time one
  EXPECT_NO_THROW(scheduler->SetEventZuluTime(
      eventId, sim.GetTimeKeeper()->GetZuluTime() + 2_h));
  EXPECT_THROW(scheduler->SetEventSimulationTime(eventId, 1_ms),
               ::Smp::Services::InvalidEventId);

  // a delay in the past is rejected
  EXPECT_THROW(scheduler->AddRelativeZuluTimeEvent(&ep, -1_h),
               ::Smp::Services::InvalidEventTime);

  // a null delay is legal: only a negative one is rejected, and the date is
  // derived from the zulu clock so it must not be checked against a later
  // reading of that same clock
  EXPECT_NO_THROW(
      static_cast<void>(scheduler->AddRelativeZuluTimeEvent(&ep, 0)));

  scheduler->RemoveEvent(eventId);
  EXPECT_FALSE(scheduler->IsEventScheduled(eventId));
  sim.Exit();
}

TEST(XsmpScheduler, IsEventScheduled) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};
  ::Xsmp::EntryPoint ep{"ep", "", &entryPoints, [] {}};

  auto *scheduler = sim.GetScheduler();

  // an identifier that was never handed out
  EXPECT_FALSE(scheduler->IsEventScheduled(-1));

  auto eventId = scheduler->AddSimulationTimeEvent(&ep, 1_ms);
  EXPECT_TRUE(scheduler->IsEventScheduled(eventId));

  // the identifier is only valid until the event is removed
  scheduler->RemoveEvent(eventId);
  EXPECT_FALSE(scheduler->IsEventScheduled(eventId));
  EXPECT_THROW(scheduler->RemoveEvent(eventId),
               ::Smp::Services::InvalidEventId);
  sim.Exit();
}

TEST(XsmpScheduler, EpochTimeChanged) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto &scheduler =
      *dynamic_cast<Services::XsmpScheduler *>(sim.GetScheduler());
  scheduler.SetTargetSpeed(100.0);

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};
  std::vector<::Smp::Duration> times;
  ::Xsmp::EntryPoint ep{
      "ep", "", &entryPoints,
      [&] { times.push_back(sim.GetTimeKeeper()->GetSimulationTime()); }};

  const auto epochTime = sim.GetTimeKeeper()->GetEpochTime();
  scheduler.AddEpochTimeEvent(&ep, epochTime + 10_ms);
  EXPECT_EQ(scheduler.GetNextScheduledEventTime(), 10_ms);

  // the event keeps the epoch time it was scheduled with: moving the epoch
  // time forward makes it due at an earlier simulation time
  sim.GetTimeKeeper()->SetEpochTime(epochTime + 6_ms);
  EXPECT_EQ(scheduler.GetNextScheduledEventTime(), 4_ms);

  // an event whose epoch time is now in the past is removed
  scheduler.AddEpochTimeEvent(&ep, epochTime + 8_ms);
  sim.GetTimeKeeper()->SetEpochTime(epochTime + 9_ms);
  EXPECT_EQ(scheduler.GetNextScheduledEventTime(), 1_ms);

  sim.Run(20_ms);
  ASSERT_EQ(times.size(), 1U);
  EXPECT_EQ(times[0], 1_ms);
  sim.Exit();
}

TEST(XsmpScheduler, MissionTimeChanged) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto &scheduler =
      *dynamic_cast<Services::XsmpScheduler *>(sim.GetScheduler());
  scheduler.SetTargetSpeed(100.0);

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};
  std::vector<::Smp::Duration> times;
  ::Xsmp::EntryPoint ep{
      "ep", "", &entryPoints,
      [&] { times.push_back(sim.GetTimeKeeper()->GetSimulationTime()); }};

  const auto missionTime = sim.GetTimeKeeper()->GetMissionTime();
  // 4 executions at mission time 10ms, 15ms, 20ms and 25ms
  scheduler.AddMissionTimeEvent(&ep, missionTime + 10_ms, 5_ms, 3);

  // the first two executions fall before the new mission time: they are not
  // executed and the repeat count is reduced accordingly
  sim.GetTimeKeeper()->SetMissionTime(missionTime + 18_ms);
  EXPECT_EQ(scheduler.GetNextScheduledEventTime(), 2_ms);

  sim.Run(20_ms);
  ASSERT_EQ(times.size(), 2U);
  EXPECT_EQ(times[0], 2_ms);
  EXPECT_EQ(times[1], 7_ms);
  sim.Exit();
}

TEST(XsmpScheduler, StoreRestoreKeepsZuluEvents) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto *scheduler = sim.GetScheduler();
  auto *persist = dynamic_cast<::Smp::IPersist *>(scheduler);
  ASSERT_TRUE(persist);

  auto *model = new TestModel("model", "", &sim, &sim);
  sim.AddModel(model);
  auto *ep = &model->ep;

  // far enough in the future for the zulu thread not to execute them
  const auto zuluTime = sim.GetTimeKeeper()->GetZuluTime();
  auto storedZulu = scheduler->AddZuluTimeEvent(ep, zuluTime + 1_h);

  Storage storage;
  persist->Store(&storage);

  auto laterZulu = scheduler->AddZuluTimeEvent(ep, zuluTime + 2_h);
  auto laterEvent = scheduler->AddSimulationTimeEvent(ep, 1_ms);

  persist->Restore(&storage);

  // the state of the scheduler is restored, except for the zulu time events
  EXPECT_THROW(scheduler->RemoveEvent(laterEvent),
               ::Smp::Services::InvalidEventId);
  EXPECT_NO_THROW(scheduler->SetEventZuluTime(storedZulu, zuluTime + 3_h));
  EXPECT_NO_THROW(scheduler->SetEventZuluTime(laterZulu, zuluTime + 3_h));

  // the identifier of a zulu event posted after the store is not handed out
  // again
  EXPECT_NE(scheduler->AddSimulationTimeEvent(ep, 1_ms), laterZulu);
  sim.Exit();
}

TEST(XsmpScheduler, ZuluEventPostedBeforeAPendingOne) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};
  std::atomic_int count{0};
  ::Xsmp::EntryPoint ep{"ep", "", &entryPoints, [&] { ++count; }};

  const auto zuluTime = sim.GetTimeKeeper()->GetZuluTime();
  // the event due first is posted last: it must not wait for the pending one
  sim.GetScheduler()->AddZuluTimeEvent(&ep, zuluTime + 1_h);
  sim.GetScheduler()->AddZuluTimeEvent(&ep, zuluTime + 20_ms);

  for (int i = 0; i < 1000 && count == 0; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds{1});
  }
  EXPECT_EQ(count, 1);
  sim.Exit();
}

TEST(XsmpScheduler, SetEventTime) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto *scheduler = sim.GetScheduler();
  auto *timeKeeper = sim.GetTimeKeeper();

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};
  ::Xsmp::EntryPoint ep{"ep", "", &entryPoints, [] {}};

  // an event moved to a later time is rescheduled
  auto simulationEvent = scheduler->AddSimulationTimeEvent(&ep, 10_ms);
  scheduler->SetEventSimulationTime(simulationEvent, 5_ms);
  EXPECT_EQ(scheduler->GetNextScheduledEventTime(), 5_ms);

  // the time of an event can only be changed with its own time kind
  EXPECT_THROW(scheduler->SetEventMissionTime(simulationEvent, 1_ms),
               ::Smp::Services::InvalidEventId);
  EXPECT_THROW(scheduler->SetEventEpochTime(simulationEvent, 1_ms),
               ::Smp::Services::InvalidEventId);
  EXPECT_THROW(scheduler->SetEventZuluTime(simulationEvent, 1_ms),
               ::Smp::Services::InvalidEventId);

  // an event moved to a time in the past is removed
  scheduler->SetEventSimulationTime(simulationEvent, -1_ms);
  EXPECT_THROW(scheduler->RemoveEvent(simulationEvent),
               ::Smp::Services::InvalidEventId);

  // the same for the mission and epoch time kinds
  auto missionEvent =
      scheduler->AddMissionTimeEvent(&ep, timeKeeper->GetMissionTime() + 10_ms);
  scheduler->SetEventMissionTime(missionEvent,
                                 timeKeeper->GetMissionTime() + 5_ms);
  EXPECT_EQ(scheduler->GetNextScheduledEventTime(), 5_ms);
  scheduler->SetEventMissionTime(missionEvent,
                                 timeKeeper->GetMissionTime() - 1_ms);
  EXPECT_THROW(scheduler->RemoveEvent(missionEvent),
               ::Smp::Services::InvalidEventId);

  auto epochEvent =
      scheduler->AddEpochTimeEvent(&ep, timeKeeper->GetEpochTime() + 10_ms);
  scheduler->SetEventEpochTime(epochEvent, timeKeeper->GetEpochTime() + 5_ms);
  EXPECT_EQ(scheduler->GetNextScheduledEventTime(), 5_ms);
  scheduler->SetEventEpochTime(epochEvent, timeKeeper->GetEpochTime() - 1_ms);
  EXPECT_THROW(scheduler->RemoveEvent(epochEvent),
               ::Smp::Services::InvalidEventId);

  // a zulu event moved to the past is removed as well
  auto zuluEvent =
      scheduler->AddZuluTimeEvent(&ep, timeKeeper->GetZuluTime() + 1_h);
  scheduler->SetEventZuluTime(zuluEvent, timeKeeper->GetZuluTime() + 2_h);
  scheduler->SetEventZuluTime(zuluEvent, timeKeeper->GetZuluTime() - 1_h);
  EXPECT_THROW(scheduler->RemoveEvent(zuluEvent),
               ::Smp::Services::InvalidEventId);

  sim.Exit();
}

TEST(XsmpScheduler, EventTimeOverflow) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};
  ::Xsmp::EntryPoint ep{"ep", "", &entryPoints, [] {}};

  constexpr auto max = std::numeric_limits<::Smp::Duration>::max();
  constexpr auto min = std::numeric_limits<::Smp::Duration>::lowest();

  // the time arithmetic saturates instead of overflowing, which would wrap
  // around to a time in the past
  EXPECT_NO_THROW(sim.GetScheduler()->AddSimulationTimeEvent(&ep, max));
  EXPECT_NO_THROW(sim.GetScheduler()->AddMissionTimeEvent(&ep, max));
  EXPECT_NO_THROW(sim.GetScheduler()->AddEpochTimeEvent(&ep, max));

  // a time in the past is still rejected
  EXPECT_THROW(sim.GetScheduler()->AddMissionTimeEvent(&ep, min),
               ::Smp::Services::InvalidEventTime);

  // a cyclic event whose next occurrence saturates stays scheduled
  auto eventId = sim.GetScheduler()->AddSimulationTimeEvent(&ep, 0, max, -1);
  EXPECT_NO_THROW(sim.GetScheduler()->SetEventSimulationTime(eventId, max));
  sim.Exit();
}

TEST(XsmpScheduler, InfiniteEventWithoutCycleTime) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto &scheduler =
      *dynamic_cast<Services::XsmpScheduler *>(sim.GetScheduler());
  scheduler.SetTargetSpeed(100.0);

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};
  ::Smp::Int32 count = 0;
  ::Xsmp::EntryPoint ep{"ep", "", &entryPoints, [&] { ++count; }};

  auto eventId =
      sim.GetScheduler()->AddSimulationTimeEvent(&ep, 1_ms, 1_ms, -1);

  // an event repeated for ever is cyclic: it needs a positive cycle time,
  // otherwise it would be rescheduled at the very same time, for ever
  EXPECT_THROW(sim.GetScheduler()->SetEventCycleTime(eventId, 0),
               ::Smp::Services::InvalidCycleTime);
  EXPECT_THROW(sim.GetScheduler()->SetEventCycleTime(eventId, -1_ms),
               ::Smp::Services::InvalidCycleTime);

  sim.Run(10_ms);
  EXPECT_GT(count, 0);
  sim.Exit();
}

TEST(XsmpScheduler, HoldDuringEvents) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto &scheduler =
      *dynamic_cast<Services::XsmpScheduler *>(sim.GetScheduler());
  scheduler.SetTargetSpeed(100.0);

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};
  std::vector<int> results;
  ::Xsmp::EntryPoint ep1{"ep1", "", &entryPoints,
                         [&] { results.push_back(1); }};
  ::Xsmp::EntryPoint ep2{"ep2", "", &entryPoints, [&] {
                           results.push_back(2);
                           sim.Hold(true);
                         }};
  ::Xsmp::EntryPoint ep3{"ep3", "", &entryPoints,
                         [&] { results.push_back(3); }};

  // three events scheduled at the same time, the second one holds
  sim.GetScheduler()->AddSimulationTimeEvent(&ep1, 1_ms);
  sim.GetScheduler()->AddSimulationTimeEvent(&ep2, 1_ms);
  sim.GetScheduler()->AddSimulationTimeEvent(&ep3, 1_ms);

  sim.Run(10_ms);
  EXPECT_EQ(results, (std::vector<int>{1, 2}));

  // the events left un-executed are kept and run on the next Run
  sim.Run(10_ms);
  EXPECT_EQ(results, (std::vector<int>{1, 2, 3}));
  sim.Exit();
}

TEST(XsmpScheduler, RunDurationHoldEvent) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto &scheduler =
      *dynamic_cast<Services::XsmpScheduler *>(sim.GetScheduler());
  scheduler.SetTargetSpeed(100.0);

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};
  ::Xsmp::EntryPoint ep{"ep", "", &entryPoints, [&] { sim.Hold(true); }};

  sim.GetScheduler()->AddSimulationTimeEvent(&ep, 1_ms);

  // the run is interrupted by the model, before its own hold event at 10 ms
  sim.Run(10_ms);
  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);

  // Run() posts an entry point living on its stack: it must not stay in the
  // scheduler once it returns
  EXPECT_EQ(sim.GetScheduler()->GetNextScheduledEventTime(),
            std::numeric_limits<::Smp::Duration>::max());
  sim.Exit();
}

TEST(XsmpScheduler, StoreRestore) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto *scheduler = dynamic_cast<::Smp::IPersist *>(sim.GetScheduler());
  ASSERT_TRUE(scheduler);

  // the entry point is owned by a model of the simulation: the scheduler
  // persists it by path
  auto *model = new TestModel("model", "", &sim, &sim);
  sim.AddModel(model);
  auto *ep = &model->ep;

  sim.GetScheduler()->AddSimulationTimeEvent(ep, 2_ms);
  auto removedId = sim.GetScheduler()->AddSimulationTimeEvent(ep, 1_ms);
  EXPECT_EQ(sim.GetScheduler()->GetNextScheduledEventTime(), 1_ms);

  Storage storage;
  scheduler->Store(&storage);

  // the state of the scheduler changes, then is restored
  sim.GetScheduler()->RemoveEvent(removedId);
  EXPECT_EQ(sim.GetScheduler()->GetNextScheduledEventTime(), 2_ms);

  scheduler->Restore(&storage);
  EXPECT_EQ(sim.GetScheduler()->GetNextScheduledEventTime(), 1_ms);
  sim.Exit();
}

} // namespace Xsmp::Services
