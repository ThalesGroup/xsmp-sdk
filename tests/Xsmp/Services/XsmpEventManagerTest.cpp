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
#include <Smp/Services/EntryPointAlreadySubscribed.h>
#include <Smp/Services/EntryPointNotSubscribed.h>
#include <Smp/Services/EventId.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/InvalidEventId.h>
#include <Smp/Services/InvalidEventName.h>
#include <Xsmp/Component.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/EntryPointPublisher.h>
#include <Xsmp/Simulator.h>
#include <initializer_list>
#include <map>
#include <string>
#include <vector>

namespace Xsmp::Services {
namespace {
class TestEntryPointPublisher: public Xsmp::Component,
        public virtual Xsmp::EntryPointPublisher {
public:
    using Xsmp::Component::Component;
};
}
TEST(XsmpEventManagerTest, QueryEventId) {

    Simulator sim;
    sim.LoadLibrary("xsmp_services");

    sim.Connect();
    ASSERT_TRUE(sim.GetEventManager());

    EXPECT_THROW(sim.GetEventManager()->QueryEventId(nullptr),
            ::Smp::Services::InvalidEventName);
    EXPECT_THROW(sim.GetEventManager()->QueryEventId(""),
            ::Smp::Services::InvalidEventName);

    std::map<std::string, ::Smp::Services::EventId> events;
    for (auto i = 0; i < 1000; ++i) {
        auto key = std::to_string(i);
        events.emplace(key, sim.GetEventManager()->QueryEventId(key.c_str()));
    }

    for (const auto &entry : events) {
        EXPECT_EQ(sim.GetEventManager()->QueryEventId(entry.first.c_str()),
                entry.second);
    }
    using namespace ::Smp::Services;

    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_LeaveConnecting),
            IEventManager::SMP_LeaveConnectingId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_EnterInitialising),
            IEventManager::SMP_EnterInitialisingId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_LeaveInitialising),
            IEventManager::SMP_LeaveInitialisingId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_EnterStandby),
            IEventManager::SMP_EnterStandbyId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_LeaveStandby),
            IEventManager::SMP_LeaveStandbyId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_EnterExecuting),
            IEventManager::SMP_EnterExecutingId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_LeaveExecuting),
            IEventManager::SMP_LeaveExecutingId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_EnterStoring),
            IEventManager::SMP_EnterStoringId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_LeaveStoring),
            IEventManager::SMP_LeaveStoringId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_EnterRestoring),
            IEventManager::SMP_EnterRestoringId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_LeaveRestoring),
            IEventManager::SMP_LeaveRestoringId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_EnterExiting),
            IEventManager::SMP_EnterExitingId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_EnterAborting),
            IEventManager::SMP_EnterAbortingId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_EpochTimeChanged),
            IEventManager::SMP_EpochTimeChangedId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_MissionTimeChanged),
            IEventManager::SMP_MissionTimeChangedId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_EnterReconnecting),
            IEventManager::SMP_EnterReconnectingId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_LeaveReconnecting),
            IEventManager::SMP_LeaveReconnectingId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_PreSimTimeChange),
            IEventManager::SMP_PreSimTimeChangeId);
    EXPECT_EQ(
            sim.GetEventManager()->QueryEventId(
                    IEventManager::SMP_PostSimTimeChange),
            IEventManager::SMP_PostSimTimeChangeId);

}

TEST(XsmpEventManagerTest, ep) {

    Simulator sim;
    sim.LoadLibrary("xsmp_services");
    ASSERT_TRUE(sim.GetEventManager());
    TestEntryPointPublisher entryPoints { "entryPoints", "", &sim };

    using namespace ::Smp::Services;
    std::vector<int> results;
    ::Xsmp::EntryPoint ep1 { "ep1", "", &entryPoints, [&]() {
        results.push_back(1);
        sim.GetEventManager()->Unsubscribe(IEventManager::SMP_LeaveConnectingId,
                &ep1);
    } };
    ::Xsmp::EntryPoint ep2 { "ep2", "", &entryPoints, [&]() {

        results.push_back(2);
    } };
    ::Xsmp::EntryPoint ep3 { "ep3", "", &entryPoints, [&]() {
        sim.GetEventManager()->Subscribe(IEventManager::SMP_LeaveConnectingId,
                &ep2);
        sim.GetEventManager()->Subscribe(IEventManager::SMP_LeaveConnectingId,
                &ep1);
        sim.GetEventManager()->Unsubscribe(IEventManager::SMP_LeaveConnectingId,
                &ep3);

        results.push_back(3);
    } };

    sim.GetEventManager()->Subscribe(IEventManager::SMP_LeaveConnectingId,
            &ep3);

    sim.GetEventManager()->Emit(IEventManager::SMP_LeaveConnectingId);
    std::vector<int> expected = { 3 };
    EXPECT_EQ(results, expected);

    results.clear();

    sim.GetEventManager()->Emit(IEventManager::SMP_LeaveConnectingId);
    expected = { 2, 1 };
    EXPECT_EQ(results, expected);
    results.clear();
    sim.GetEventManager()->Emit(IEventManager::SMP_LeaveConnectingId);
    expected = { 2 };
    EXPECT_EQ(results, expected);

    EXPECT_THROW(
            sim.GetEventManager()->Subscribe(
                    IEventManager::SMP_LeaveConnectingId, &ep2),
            ::Smp::Services::EntryPointAlreadySubscribed);
    EXPECT_THROW(
            sim.GetEventManager()->Unsubscribe(
                    IEventManager::SMP_LeaveConnectingId, &ep1),
            ::Smp::Services::EntryPointNotSubscribed);

    EXPECT_THROW(sim.GetEventManager()->Unsubscribe(0xffff, &ep1),
            ::Smp::Services::InvalidEventId);
    EXPECT_THROW(sim.GetEventManager()->Subscribe(0xffff, &ep1),
            ::Smp::Services::InvalidEventId);
    EXPECT_THROW(sim.GetEventManager()->Emit(0xffff),
            ::Smp::Services::InvalidEventId);
}
} // namespace Xsmp::Services
