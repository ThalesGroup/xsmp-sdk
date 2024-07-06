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

#include <Smp/IModel.h>
#include <Smp/InvalidLibrary.h>
#include <Smp/LibraryNotFound.h>
#include <Smp/SimulatorStateKind.h>
#include <Smp/Uuid.h>
#include <Xsmp/Simulator.h>
#include <Xsmp/Tests/ModelWithArrayFieldsGen.h>
#include <Xsmp/Tests/ModelWithSimpleArrayFieldsGen.h>
#include <Xsmp/Tests/ModelWithSimpleFieldsGen.h>
#include <gtest/gtest.h>
#include <type_traits>

namespace Xsmp {

TEST(SimulatorTest, init) {

  Simulator sim;

  EXPECT_TRUE(sim.GetTypeRegistry());

  EXPECT_TRUE(!sim.GetEventManager());
  EXPECT_TRUE(!sim.GetLinkRegistry());
  EXPECT_TRUE(!sim.GetLogger());
  EXPECT_TRUE(!sim.GetResolver());
  EXPECT_TRUE(!sim.GetScheduler());
  EXPECT_TRUE(!sim.GetTimeKeeper());

  sim.LoadLibrary("xsmp_logger");
  EXPECT_EQ(sim.GetService("XsmpLogger"), sim.GetLogger());
  EXPECT_TRUE(!sim.GetEventManager());
  EXPECT_TRUE(!sim.GetLinkRegistry());
  EXPECT_TRUE(sim.GetLogger());
  EXPECT_TRUE(!sim.GetResolver());
  EXPECT_TRUE(!sim.GetScheduler());
  EXPECT_TRUE(!sim.GetTimeKeeper());

  sim.LoadLibrary("xsmp_time_keeper");
  EXPECT_EQ(sim.GetService("XsmpTimeKeeper"), sim.GetTimeKeeper());
  EXPECT_TRUE(!sim.GetEventManager());
  EXPECT_TRUE(!sim.GetLinkRegistry());
  EXPECT_TRUE(sim.GetLogger());
  EXPECT_TRUE(!sim.GetResolver());
  EXPECT_TRUE(!sim.GetScheduler());
  EXPECT_TRUE(sim.GetTimeKeeper());

  sim.LoadLibrary("xsmp_resolver");
  EXPECT_EQ(sim.GetService("XsmpResolver"), sim.GetResolver());
  EXPECT_TRUE(!sim.GetEventManager());
  EXPECT_TRUE(!sim.GetLinkRegistry());
  EXPECT_TRUE(sim.GetLogger());
  EXPECT_TRUE(sim.GetResolver());
  EXPECT_TRUE(!sim.GetScheduler());
  EXPECT_TRUE(sim.GetTimeKeeper());

  sim.LoadLibrary("xsmp_event_manager");
  EXPECT_EQ(sim.GetService("XsmpEventManager"), sim.GetEventManager());
  EXPECT_TRUE(sim.GetEventManager());
  EXPECT_TRUE(!sim.GetLinkRegistry());
  EXPECT_TRUE(sim.GetLogger());
  EXPECT_TRUE(sim.GetResolver());
  EXPECT_TRUE(!sim.GetScheduler());
  EXPECT_TRUE(sim.GetTimeKeeper());

  sim.LoadLibrary("xsmp_link_registry");
  EXPECT_EQ(sim.GetService("XsmpLinkRegistry"), sim.GetLinkRegistry());
  EXPECT_TRUE(sim.GetEventManager());
  EXPECT_TRUE(sim.GetLinkRegistry());
  EXPECT_TRUE(sim.GetLogger());
  EXPECT_TRUE(sim.GetResolver());
  EXPECT_TRUE(!sim.GetScheduler());
  EXPECT_TRUE(sim.GetTimeKeeper());

  sim.LoadLibrary("xsmp_scheduler");
  EXPECT_EQ(sim.GetService("XsmpScheduler"), sim.GetScheduler());
  EXPECT_TRUE(sim.GetEventManager());
  EXPECT_TRUE(sim.GetLinkRegistry());
  EXPECT_TRUE(sim.GetLogger());
  EXPECT_TRUE(sim.GetResolver());
  EXPECT_TRUE(sim.GetScheduler());
  EXPECT_TRUE(sim.GetTimeKeeper());

  sim.LoadLibrary("xsmp_tests");

  sim.AddModel(dynamic_cast<::Smp::IModel *>(sim.CreateInstance(
      Xsmp::Tests::Uuid_ModelWithSimpleFields, "SimpleFields", "", &sim)));
  sim.AddModel(dynamic_cast<::Smp::IModel *>(
      sim.CreateInstance(Xsmp::Tests::Uuid_ModelWithSimpleArrayFields,
                         "SimpleArrayFields", "", &sim)));

  sim.AddModel(dynamic_cast<::Smp::IModel *>(sim.CreateInstance(
      Xsmp::Tests::Uuid_ModelWithArrayFields, "ArrayFields", "", &sim)));
  sim.Publish();

  sim.Configure();
  sim.Connect();
}
TEST(SimulatorTest, State) {
  Simulator sim;

  sim.LoadLibrary("xsmp_services");

  EXPECT_EQ(sim.GetState(), Smp::SimulatorStateKind::SSK_Building);

  sim.Publish();

  sim.Configure();
  sim.Connect();

  EXPECT_EQ(sim.GetState(), Smp::SimulatorStateKind::SSK_Standby);

  sim.Initialise();
}

TEST(SimulatorTest, LoadLibrary) {
  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  EXPECT_THROW(sim.LoadLibrary("invalid_library"), Smp::LibraryNotFound);
  EXPECT_THROW(sim.LoadLibrary("xsmp_simulator"), Smp::InvalidLibrary);
}

} // namespace Xsmp
