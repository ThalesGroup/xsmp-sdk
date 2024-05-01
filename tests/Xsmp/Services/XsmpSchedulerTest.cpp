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

#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/ITimeKeeper.h>
#include <Smp/SimulatorStateKind.h>
#include <Xsmp/Duration.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/Services/XsmpScheduler.h>
#include <Xsmp/Simulator.h>
#include <chrono>
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
} // namespace
TEST(XsmpSchedulerTest, run) {

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

  std::vector<int> expected = {4, 1, 1, 2, 4, 2, 4, 3, 3};
  EXPECT_EQ(results, expected);
}

class SpeedTestFixture : public ::testing::TestWithParam<double> {};
INSTANTIATE_TEST_SUITE_P(XsmpSchedulerTest, SpeedTestFixture,
                         ::testing::Values(0.01, 0.25, 0.5, 1, 4, 10, 100));

TEST_P(SpeedTestFixture, SpeedTest) {
  double speed = GetParam();
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

TEST(XsmpSchedulerTest, zulu_events) {

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
  std::vector<int> expected = {1, 2, 3, 1, 2, 3};
  EXPECT_EQ(results, expected);
}

TEST(XsmpSchedulerTest, EventTime) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto &scheduler =
      *dynamic_cast<Services::XsmpScheduler *>(sim.GetScheduler());
  scheduler.SetTargetSpeed(100.0);

  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};

  ::Xsmp::EntryPoint ep1{"ep1", "", &entryPoints, [&](void) {
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

TEST(XsmpSchedulerTest, EventCycleAndRepeatTime) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto &scheduler =
      *dynamic_cast<Services::XsmpScheduler *>(sim.GetScheduler());
  scheduler.SetTargetSpeed(100.0);

  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};

  ::Xsmp::EntryPoint ep1{"ep1", "", &entryPoints, [&](void) {
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

TEST(XsmpSchedulerTest, RemoveEvent) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto &scheduler =
      *dynamic_cast<Services::XsmpScheduler *>(sim.GetScheduler());
  scheduler.SetTargetSpeed(100.0);

  EXPECT_EQ(sim.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);

  TestEntryPointPublisher entryPoints{"entryPoints", "", &sim};

  ::Xsmp::EntryPoint ep1{"ep1", "", &entryPoints, [&](void) {
                           // do nothing
                         }};

  EXPECT_THROW(scheduler.RemoveEvent(10), ::Smp::Services::InvalidEventId);

  auto id = scheduler.AddSimulationTimeEvent(&ep1, 1_ms, 0, 0);

  EXPECT_NO_THROW(scheduler.RemoveEvent(id));
}

} // namespace Xsmp::Services
