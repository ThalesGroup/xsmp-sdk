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

#include <gtest/gtest.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/InvalidSimulationTime.h>
#include <Smp/Services/IScheduler.h>
#include <Smp/Services/ITimeKeeper.h>
#include <Xsmp/Component.h>
#include <Xsmp/Duration.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/EntryPointPublisher.h>
#include <Xsmp/Simulator.h>
#include <type_traits>

namespace Xsmp::Services {
namespace {
class TestEntryPointPublisher: public Xsmp::Component,
        public virtual Xsmp::EntryPointPublisher {
public:
    using Xsmp::Component::Component;
};
}

TEST(XsmpTimeKeeperTest, simulationTime) {

    Simulator sim;
    sim.LoadLibrary("xsmp_services");

    sim.Connect();

    EXPECT_EQ(sim.GetTimeKeeper()->GetSimulationTime(), 0_s);
    //set simulation time outside a PreSimTimeChang event does nothing
    sim.GetTimeKeeper()->SetSimulationTime(1_s);
    EXPECT_EQ(sim.GetTimeKeeper()->GetSimulationTime(), 0_s);

    sim.Run(1_s);
    EXPECT_EQ(sim.GetTimeKeeper()->GetSimulationTime(), 1_s);

    TestEntryPointPublisher entryPoints { "entryPoints", "", &sim };

    ::Xsmp::EntryPoint ep1 { "ep1", "", &entryPoints, [&]() {

    } };

    sim.GetEventManager()->Emit(
            ::Smp::Services::IEventManager::SMP_PreSimTimeChangeId);
    //check that InvalidSimulationTime is thrown if new simulation time is less than current simulation time
    EXPECT_THROW(sim.GetTimeKeeper()->SetSimulationTime(1_s - 1),
            ::Smp::Services::InvalidSimulationTime);
    sim.GetTimeKeeper()->SetSimulationTime(1_s);
    sim.GetScheduler()->AddSimulationTimeEvent(&ep1, 1_s, 0, 0);

    //check that InvalidSimulationTime is thrown in new simulation time is > to next event time
    EXPECT_THROW(sim.GetTimeKeeper()->SetSimulationTime(2_s + 1),
            ::Smp::Services::InvalidSimulationTime);
    sim.GetTimeKeeper()->SetSimulationTime(2_s);

    sim.GetEventManager()->Emit(
            ::Smp::Services::IEventManager::SMP_PostSimTimeChangeId);

    EXPECT_EQ(sim.GetTimeKeeper()->GetSimulationTime(), 2_s);
    sim.Run(1_s);
    EXPECT_EQ(sim.GetTimeKeeper()->GetSimulationTime(), 3_s);
}

TEST(XsmpTimeKeeperTest, epochTime) {

    Simulator sim;
    sim.LoadLibrary("xsmp_services");

    sim.Connect();

    EXPECT_EQ(sim.GetTimeKeeper()->GetEpochTime(), 0_s);
    sim.GetTimeKeeper()->SetEpochTime(1_s);
    EXPECT_EQ(sim.GetTimeKeeper()->GetEpochTime(), 1_s);

    sim.Run(1_s);
    EXPECT_EQ(sim.GetTimeKeeper()->GetEpochTime(), 2_s);

}

TEST(XsmpTimeKeeperTest, missionTime) {

    Simulator sim;
    sim.LoadLibrary("xsmp_services");

    sim.Connect();

    EXPECT_EQ(sim.GetTimeKeeper()->GetMissionTime(), 0_s);
    sim.GetTimeKeeper()->SetMissionStartTime(1_s);
    EXPECT_EQ(sim.GetTimeKeeper()->GetMissionTime(), -1_s);
    EXPECT_EQ(sim.GetTimeKeeper()->GetMissionStartTime(), 1_s);

    sim.Run(1_s);
    EXPECT_EQ(sim.GetTimeKeeper()->GetMissionTime(), 0_s);

}

TEST(XsmpTimeKeeperTest, zuluTime) {

    Simulator sim;
    sim.LoadLibrary("xsmp_services");

    sim.Connect();

    EXPECT_EQ(sim.GetTimeKeeper()->GetMissionTime(), 0_s);
    sim.GetTimeKeeper()->SetMissionTime(1_s);
    EXPECT_EQ(sim.GetTimeKeeper()->GetMissionTime(), 1_s);

    sim.Run(1_s);
    EXPECT_EQ(sim.GetTimeKeeper()->GetMissionTime(), 2_s);

}
} // namespace Xsmp::Services
