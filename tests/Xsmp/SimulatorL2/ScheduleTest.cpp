// Copyright 2026 THALES ALENIA SPACE FRANCE. All rights reserved.
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

#include <Smp/IProperty.h>
#include <Smp/ISimpleField.h>
#include <Smp/InvalidFile.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/IScheduler.h>
#include <Smp/Services/ITimeKeeper.h>
#include <Xsmp/DateTime.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/EntryPointPublisher.h>
#include <Xsmp/Model.h>
#include <Xsmp/Simulator.h>
#include <Xsmp/Tests/ModelWithOperations.h>
#include <Xsmp/Tests/ModelWithProperties.h>
#include <Xsmp/Tests/ModelWithSimpleFields.h>

#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <future>
#include <limits>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

namespace Xsmp::L2 {
namespace {

constexpr ::Smp::Duration ONE_SECOND = 1'000'000'000LL;

class TemporaryScheduleDirectory final {
public:
  TemporaryScheduleDirectory() {
    const auto suffix = std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count());
    path = std::filesystem::temp_directory_path() /
           ("xsmp-schedule-runtime-" + suffix);
    std::filesystem::create_directories(path);
  }

  ~TemporaryScheduleDirectory() {
    std::error_code error;
    std::filesystem::remove_all(path, error);
  }

  std::filesystem::path Write(std::string_view name,
                              std::string_view contents) const {
    const auto result = path / name;
    std::ofstream stream{result, std::ios::binary};
    stream.write(contents.data(),
                 static_cast<std::streamsize>(contents.size()));
    return result;
  }

  std::filesystem::path path;
};

class ProbeModel final : public ::Xsmp::Model,
                         public virtual ::Xsmp::EntryPointPublisher {
public:
  ProbeModel(::Smp::String8 name, ::Smp::IComposite *parent,
             ::Smp::ISimulator *simulator)
      : ::Xsmp::Model{name, "", parent, simulator},
        first{"first", "", this, [this] { calls.push_back(1); }},
        second{"second", "", this, [this] { calls.push_back(2); }},
        global{"global", "", this, [this] { calls.push_back(3); }} {}

  std::vector<int> calls;
  ::Xsmp::EntryPoint first;
  ::Xsmp::EntryPoint second;
  ::Xsmp::EntryPoint global;
};

class ConcurrentProbeModel final : public ::Xsmp::Model,
                                   public virtual ::Xsmp::EntryPointPublisher {
public:
  ConcurrentProbeModel(::Smp::String8 name, ::Smp::IComposite *parent,
                       ::Smp::ISimulator *simulator)
      : ::Xsmp::Model{name, "", parent, simulator},
        tick{"tick", "", this, [this] {
               {
                 const std::lock_guard lock{mutex};
                 ++calls;
               }
               changed.notify_all();
             }} {}

  bool WaitForCalls(int expected) {
    std::unique_lock lock{mutex};
    return changed.wait_for(lock, std::chrono::seconds{1},
                            [this, expected] { return calls >= expected; });
  }

  int GetCalls() const {
    const std::lock_guard lock{mutex};
    return calls;
  }

private:
  mutable std::mutex mutex;
  std::condition_variable changed;
  int calls{};
  ::Xsmp::EntryPoint tick;
};

class BlockingProbeModel final : public ::Xsmp::Model,
                                 public virtual ::Xsmp::EntryPointPublisher {
public:
  BlockingProbeModel(::Smp::String8 name, ::Smp::IComposite *parent,
                     ::Smp::ISimulator *simulator, std::promise<void> &entered,
                     std::shared_future<void> release,
                     std::promise<void> &finished)
      : ::Xsmp::Model{name, "", parent, simulator},
        tick{"tick", "", this,
             [&entered, release = std::move(release), &finished] {
               entered.set_value();
               release.wait();
               finished.set_value();
             }} {}

private:
  ::Xsmp::EntryPoint tick;
};

class PromiseReleaseGuard final {
public:
  explicit PromiseReleaseGuard(std::promise<void> &promise)
      : promise{promise} {}

  ~PromiseReleaseGuard() { Release(); }

  void Release() {
    if (!released) {
      promise.set_value();
      released = true;
    }
  }

private:
  std::promise<void> &promise;
  bool released{};
};

std::string ScheduleHeader(std::string_view name) {
  return std::string{R"(<?xml version="1.0" encoding="UTF-8"?>
<Schedule:Schedule
    xmlns:Schedule="http://www.ecss.nl/smp/2025/Smdl/Schedule"
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:xlink="http://www.w3.org/1999/xlink"
    Id=")"} +
         std::string{name} + "\" Name=\"" + std::string{name} + "\">\n";
}

TEST(Schedule, ExecutesActivitiesInOrderAndResolvesTaskRoot) {
  TemporaryScheduleDirectory directory;
  auto xml = ScheduleHeader("Activities");
  xml +=
      R"(  <Parameter Name="Target" xsi:type="Assembly:StringArgument" Value="Probe"/>
  <Task Id="Activities.Sub" Name="Sub">
    <Activity Id="Activities.Sub.Second" Name="Second" xsi:type="Schedule:Trigger">
      <EntryPoint>/TemplateRoot.second</EntryPoint>
    </Activity>
  </Task>
  <Task Id="Activities.Main" Name="Main">
    <Activity Id="Activities.Main.First" Name="First" xsi:type="Schedule:Trigger">
      <EntryPoint>/{Target}.first</EntryPoint>
    </Activity>
    <Activity Id="Activities.Main.Nested" Name="Nested" xsi:type="Schedule:ExecuteTask" Root="/{Target}">
      <Task xlink:title="Sub" xlink:href="#Activities.Sub"/>
    </Activity>
    <Activity Id="Activities.Main.Transfer" Name="Transfer" xsi:type="Schedule:Transfer">
      <OutputFieldPath>/Fields.integer1Output</OutputFieldPath>
      <InputFieldPath>/Fields.integer1Input</InputFieldPath>
    </Activity>
    <Activity Id="Activities.Main.Property" Name="Property" xsi:type="Schedule:SetProperty">
      <PropertyPath>/Properties.int_property</PropertyPath>
      <Value xsi:type="Types:Int32Value" Value="42"/>
    </Activity>
    <Activity Id="Activities.Main.Operation" Name="Operation" xsi:type="Schedule:CallOperation">
      <OperationPath>/Operations.int32Operation</OperationPath>
    </Activity>
    <Activity Id="Activities.Main.Emit" Name="Emit" xsi:type="Schedule:EmitGlobalEvent">
      <EventName>ScheduleActivityEvent</EventName>
      <synchronous>true</synchronous>
    </Activity>
  </Task>
  <Event Id="Activities.Start" Name="Start"
         xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:title="Main" xlink:href="#Activities.Main"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("activities.smpsed", xml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  auto *probe = new ProbeModel{"Probe", &simulator, &simulator};
  auto *fields = new ::Xsmp::Tests::ModelWithSimpleFields{
      "Fields", "", &simulator, &simulator};
  auto *properties = new ::Xsmp::Tests::ModelWithProperties{
      "Properties", "", &simulator, &simulator};
  auto *operations = new ::Xsmp::Tests::ModelWithOperations{
      "Operations", "", &simulator, &simulator};
  simulator.AddModel(probe);
  simulator.AddModel(fields);
  simulator.AddModel(properties);
  simulator.AddModel(operations);
  simulator.Connect();
  auto *output =
      dynamic_cast<::Smp::ISimpleField *>(fields->GetField("integer1Output"));
  auto *input =
      dynamic_cast<::Smp::ISimpleField *>(fields->GetField("integer1Input"));
  ASSERT_NE(output, nullptr);
  ASSERT_NE(input, nullptr);
  output->SetValue(
      {::Smp::PrimitiveTypeKind::PTK_Int32, static_cast<::Smp::Int32>(17)});

  EXPECT_NO_THROW(simulator.LoadSchedule(schedule.string().c_str()));
  const auto activityEvent =
      simulator.GetEventManager()->QueryEventId("ScheduleActivityEvent");
  simulator.GetEventManager()->Subscribe(activityEvent, &probe->global);

  simulator.Run(1);

  EXPECT_EQ(probe->calls, (std::vector<int>{1, 2, 3}));
  EXPECT_EQ(static_cast<::Smp::Int32>(input->GetValue()), 17);
  ASSERT_NE(properties->GetProperty("int_property"), nullptr);
  EXPECT_EQ(static_cast<::Smp::Int32>(
                properties->GetProperty("int_property")->GetValue()),
            42);
}

TEST(Schedule, AppliesClockOriginsAndRegistersAllTimedEventKinds) {
  TemporaryScheduleDirectory directory;
  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");

  const auto epoch = static_cast<::Smp::DateTime>(
      ::Xsmp::DateTime{"2030-01-01T00:00:00.123456789Z", "%FT%TZ"});
  const auto zulu = ::Xsmp::DateTime{simulator.GetTimeKeeper()->GetZuluTime() +
                                     3600 * ONE_SECOND}
                        .format("%FT%TZ");
  auto xml = ScheduleHeader("Times");
  xml += "  <EpochTime>2030-01-01T02:00:00.123456789+02:00</EpochTime>\n";
  xml += "  <MissionStart>2029-12-31T18:30:00.123456789-05:30</MissionStart>\n";
  xml += R"(  <Task Id="Times.Empty" Name="Empty"/>
  <Event Id="Times.Simulation" Name="Simulation" xsi:type="Schedule:SimulationEvent" SimulationTime="PT10S">
    <Task xlink:href="#Times.Empty"/>
  </Event>
  <Event Id="Times.Mission" Name="Mission" xsi:type="Schedule:MissionEvent" MissionTime="PT10S">
    <Task xlink:href="#Times.Empty"/>
  </Event>
  <Event Id="Times.Epoch" Name="Epoch" xsi:type="Schedule:EpochEvent" EpochTime="2030-01-01T02:00:10.123456789+02:00">
    <Task xlink:href="#Times.Empty"/>
  </Event>
  <Event Id="Times.Zulu" Name="Zulu" xsi:type="Schedule:ZuluEvent" ZuluTime=")";
  xml += zulu;
  xml += R"(">
    <Task xlink:href="#Times.Empty"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("times.smpsed", xml);

  EXPECT_NO_THROW(simulator.LoadSchedule(schedule.string().c_str()));
  EXPECT_EQ(simulator.GetTimeKeeper()->GetEpochTime(), epoch);
  EXPECT_EQ(simulator.GetTimeKeeper()->GetMissionStartTime(), epoch);
  EXPECT_EQ(simulator.GetScheduler()->GetNextScheduledEventTime(),
            10 * ONE_SECOND);
  EXPECT_NO_THROW(simulator.Connect());
}

TEST(Schedule, PreservesEventOrderAtTheSameSimulationTime) {
  TemporaryScheduleDirectory directory;
  auto xml = ScheduleHeader("EventOrder");
  xml += R"(  <Task Id="EventOrder.First" Name="First">
    <Activity Id="EventOrder.First.Tick" Name="Tick" xsi:type="Schedule:Trigger">
      <EntryPoint>/Probe.first</EntryPoint>
    </Activity>
  </Task>
  <Task Id="EventOrder.Second" Name="Second">
    <Activity Id="EventOrder.Second.Tick" Name="Tick" xsi:type="Schedule:Trigger">
      <EntryPoint>/Probe.second</EntryPoint>
    </Activity>
  </Task>
  <Event Id="EventOrder.One" Name="One" xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#EventOrder.First"/>
  </Event>
  <Event Id="EventOrder.Two" Name="Two" xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#EventOrder.Second"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("event-order.smpsed", xml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  auto *probe = new ProbeModel{"Probe", &simulator, &simulator};
  simulator.AddModel(probe);
  simulator.Connect();
  simulator.LoadSchedule(schedule.string().c_str());
  simulator.Run(1);

  EXPECT_EQ(probe->calls, (std::vector<int>{1, 2}));
}

TEST(Schedule, RegistersEveryGlobalTriggeredTimeKind) {
  TemporaryScheduleDirectory directory;
  auto xml = ScheduleHeader("GlobalTimeKinds");
  xml += R"(  <Task Id="GlobalTimeKinds.Tick" Name="Tick">
    <Activity Id="GlobalTimeKinds.Tick.Trigger" Name="Trigger" xsi:type="Schedule:Trigger">
      <EntryPoint>/Probe.tick</EntryPoint>
    </Activity>
  </Task>
  <Event Id="GlobalTimeKinds.Simulation" Name="Simulation" xsi:type="Schedule:GlobalEventTriggeredEvent"
         StartEvent="StartSimulation" TimeKind="SimulationTime" Delay="PT0S">
    <Task xlink:href="#GlobalTimeKinds.Tick"/>
  </Event>
  <Event Id="GlobalTimeKinds.Mission" Name="Mission" xsi:type="Schedule:GlobalEventTriggeredEvent"
         StartEvent="StartMission" TimeKind="MissionTime" Delay="PT0S">
    <Task xlink:href="#GlobalTimeKinds.Tick"/>
  </Event>
  <Event Id="GlobalTimeKinds.Epoch" Name="Epoch" xsi:type="Schedule:GlobalEventTriggeredEvent"
         StartEvent="StartEpoch" TimeKind="EpochTime" Delay="PT0S">
    <Task xlink:href="#GlobalTimeKinds.Tick"/>
  </Event>
  <Event Id="GlobalTimeKinds.Zulu" Name="Zulu" xsi:type="Schedule:GlobalEventTriggeredEvent"
         StartEvent="StartZulu" TimeKind="ZuluTime" Delay="PT0S">
    <Task xlink:href="#GlobalTimeKinds.Tick"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("global-time-kinds.smpsed", xml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  auto *probe = new ConcurrentProbeModel{"Probe", &simulator, &simulator};
  simulator.AddModel(probe);
  simulator.Connect();
  simulator.LoadSchedule(schedule.string().c_str());
  // Start the triggered events away from time zero. Mission and Epoch events
  // take absolute times, while Simulation and Zulu events take relative
  // delays; all four must nevertheless execute immediately here.
  simulator.Run(ONE_SECOND);

  auto *events = simulator.GetEventManager();
  ASSERT_NE(events, nullptr);
  for (const auto *name :
       {"StartSimulation", "StartMission", "StartEpoch", "StartZulu"}) {
    events->Emit(events->QueryEventId(name));
  }
  simulator.Run(1);

  EXPECT_TRUE(probe->WaitForCalls(4));
  EXPECT_EQ(probe->GetCalls(), 4);
}

TEST(Schedule, AppliesGlobalTriggeredSimulationDelayFromTheStartEvent) {
  TemporaryScheduleDirectory directory;
  auto xml = ScheduleHeader("GlobalSimulationDelay");
  xml += R"(  <Task Id="GlobalSimulationDelay.Tick" Name="Tick">
    <Activity Id="GlobalSimulationDelay.Tick.Trigger" Name="Trigger" xsi:type="Schedule:Trigger">
      <EntryPoint>/Probe.first</EntryPoint>
    </Activity>
  </Task>
  <Event Id="GlobalSimulationDelay.Event" Name="Event" xsi:type="Schedule:GlobalEventTriggeredEvent"
         StartEvent="StartDelayed" TimeKind="SimulationTime" Delay="PT2S">
    <Task xlink:href="#GlobalSimulationDelay.Tick"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("global-simulation-delay.smpsed", xml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  auto *probe = new ProbeModel{"Probe", &simulator, &simulator};
  simulator.AddModel(probe);
  simulator.Connect();
  simulator.LoadSchedule(schedule.string().c_str());

  simulator.Run(ONE_SECOND);
  auto *events = simulator.GetEventManager();
  ASSERT_NE(events, nullptr);
  events->Emit(events->QueryEventId("StartDelayed"));

  simulator.Run(ONE_SECOND);
  EXPECT_TRUE(probe->calls.empty());
  simulator.Run(ONE_SECOND);
  EXPECT_EQ(probe->calls, (std::vector<int>{1}));
}

TEST(Schedule, AppliesCycleTimeAndRepeatCount) {
  TemporaryScheduleDirectory directory;
  auto xml = ScheduleHeader("Repeating");
  xml += R"(  <Task Id="Repeating.Tick" Name="Tick">
    <Activity Id="Repeating.Tick.Trigger" Name="Trigger" xsi:type="Schedule:Trigger">
      <EntryPoint>/Probe.first</EntryPoint>
    </Activity>
  </Task>
  <Event Id="Repeating.Event" Name="Event" xsi:type="Schedule:SimulationEvent"
         SimulationTime="PT0S" CycleTime="PT1S" RepeatCount="2">
    <Task xlink:href="#Repeating.Tick"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("repeating.smpsed", xml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  auto *probe = new ProbeModel{"Probe", &simulator, &simulator};
  simulator.AddModel(probe);
  simulator.Connect();
  simulator.LoadSchedule(schedule.string().c_str());
  simulator.Run(3 * ONE_SECOND);

  EXPECT_EQ(probe->calls, (std::vector<int>{1, 1, 1}));
}

TEST(Schedule, DefaultsGlobalEventEmissionToSynchronous) {
  TemporaryScheduleDirectory directory;
  auto xml = ScheduleHeader("SynchronousDefault");
  xml += R"(  <Task Id="SynchronousDefault.Task" Name="Task">
    <Activity Id="SynchronousDefault.Task.Emit" Name="Emit" xsi:type="Schedule:EmitGlobalEvent">
      <EventName>SynchronousDefaultEvent</EventName>
    </Activity>
    <Activity Id="SynchronousDefault.Task.After" Name="After" xsi:type="Schedule:Trigger">
      <EntryPoint>/Probe.first</EntryPoint>
    </Activity>
  </Task>
  <Event Id="SynchronousDefault.Start" Name="Start" xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#SynchronousDefault.Task"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("synchronous-default.smpsed", xml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  auto *probe = new ProbeModel{"Probe", &simulator, &simulator};
  simulator.AddModel(probe);
  simulator.Connect();
  simulator.LoadSchedule(schedule.string().c_str());
  const auto eventId =
      simulator.GetEventManager()->QueryEventId("SynchronousDefaultEvent");
  simulator.GetEventManager()->Subscribe(eventId, &probe->global);
  simulator.Run(1);

  EXPECT_EQ(probe->calls, (std::vector<int>{3, 1}));
}

TEST(Schedule, ExecutesExternalTaskWithTypedTemplateArguments) {
  TemporaryScheduleDirectory directory;
  auto externalXml = ScheduleHeader("External");
  externalXml +=
      R"(  <Parameter Name="Target" xsi:type="Assembly:StringArgument" Value="{Target}"/>
  <Parameter Name="Count" xsi:type="Assembly:Int32Argument" Value="{Count}"/>
  <Task Id="External.Apply" Name="Apply">
    <Activity Id="External.Apply.Property" Name="Property" xsi:type="Schedule:SetProperty">
      <PropertyPath>/{Target}.int_property</PropertyPath>
      <Value xsi:type="Types:Int32Value" Value="{Count}"/>
    </Activity>
  </Task>
</Schedule:Schedule>)";
  directory.Write("external.smpsed", externalXml);

  auto mainXml = ScheduleHeader("ExternalCaller");
  mainXml += R"(  <Task Id="ExternalCaller.Main" Name="Main">
    <Activity Id="ExternalCaller.Main.External" Name="External" xsi:type="Schedule:ExecuteTask" Root="/Properties">
      <Task xlink:title="Apply" xlink:href="external.smpsed#External.Apply"/>
      <Argument Name="Target" xsi:type="Assembly:StringArgument" Value="TemplateRoot"/>
      <Argument Name="Count" xsi:type="Assembly:Int32Argument" Value="73"/>
    </Activity>
  </Task>
  <Event Id="ExternalCaller.Start" Name="Start" xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#ExternalCaller.Main"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("external-caller.smpsed", mainXml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  auto *properties = new ::Xsmp::Tests::ModelWithProperties{
      "Properties", "", &simulator, &simulator};
  simulator.AddModel(properties);
  simulator.Connect();
  simulator.LoadSchedule(schedule.string().c_str());
  simulator.Run(1);

  auto *property = properties->GetProperty("int_property");
  ASSERT_NE(property, nullptr);
  EXPECT_EQ(static_cast<::Smp::Int32>(property->GetValue()), 73);
}

TEST(Schedule, ValidatesExternalInt32ArgumentAfterSubstitution) {
  TemporaryScheduleDirectory directory;
  auto externalXml = ScheduleHeader("External");
  externalXml +=
      R"(  <Parameter Name="Count" xsi:type="Assembly:Int32Argument" Value="{Count}"/>
  <Task Id="External.Apply" Name="Apply">
    <Activity Id="External.Apply.Property" Name="Property" xsi:type="Schedule:SetProperty">
      <PropertyPath>/TemplateRoot.int_property</PropertyPath>
      <Value xsi:type="Types:Int32Value" Value="{Count}"/>
    </Activity>
  </Task>
</Schedule:Schedule>)";
  const auto external = directory.Write("external-invalid.smpsed", externalXml);

  auto mainXml = ScheduleHeader("InvalidExternalArgument");
  mainXml += R"(  <Task Id="InvalidExternalArgument.Main" Name="Main">
    <Activity Id="InvalidExternalArgument.Main.External" Name="External" xsi:type="Schedule:ExecuteTask" Root="/Properties">
      <Task xlink:title="Apply" xlink:href="external-invalid.smpsed#External.Apply"/>
      <Argument Name="Count" xsi:type="Assembly:StringArgument" Value="not_an_int"/>
    </Activity>
  </Task>
  <Event Id="InvalidExternalArgument.Start" Name="Start" xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#InvalidExternalArgument.Main"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule =
      directory.Write("invalid-external-argument.smpsed", mainXml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  auto *properties = new ::Xsmp::Tests::ModelWithProperties{
      "Properties", "", &simulator, &simulator};
  simulator.AddModel(properties);
  simulator.Connect();
  try {
    simulator.LoadSchedule(schedule.string().c_str());
    FAIL() << "An invalid substituted Int32 value was accepted";
  } catch (const ::Smp::InvalidFile &error) {
    EXPECT_EQ(std::filesystem::path{error.GetFileName()}, external)
        << error.GetErrorMessage();
    EXPECT_NE(std::string{error.GetErrorMessage()}.find(
                  "has type 'StringArgument', expected 'Int32Argument'"),
              std::string::npos)
        << error.GetErrorMessage();
  }
}

TEST(Schedule, ResolvesTaskAfterItsTargetModelIsAdded) {
  TemporaryScheduleDirectory directory;
  auto xml = ScheduleHeader("Deferred");
  xml += R"(  <Task Id="Deferred.Task" Name="Task">
    <Activity Id="Deferred.Task.Trigger" Name="Trigger" xsi:type="Schedule:Trigger">
      <EntryPoint>/Late.first</EntryPoint>
    </Activity>
  </Task>
  <Event Id="Deferred.Event" Name="Event" xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#Deferred.Task"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("deferred.smpsed", xml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  EXPECT_NO_THROW(simulator.LoadSchedule(schedule.string().c_str()));
  EXPECT_EQ(simulator.GetScheduler()->GetNextScheduledEventTime(),
            std::numeric_limits<::Smp::Duration>::max());

  auto *probe = new ProbeModel{"Late", &simulator, &simulator};
  simulator.AddModel(probe);
  EXPECT_NO_THROW(simulator.Connect());
  simulator.Run(1);

  EXPECT_EQ(probe->calls, (std::vector<int>{1}));
}

TEST(Schedule, AcceptsDateTimeWithoutTimezone) {
  TemporaryScheduleDirectory directory;
  auto xml = ScheduleHeader("LocalDateTime");
  xml += R"(  <EpochTime>2030-01-01T00:00:00.5</EpochTime>
  <Task Id="LocalDateTime.Empty" Name="Empty"/>
  <Event Id="LocalDateTime.Event" Name="Event" xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#LocalDateTime.Empty"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("local-date-time.smpsed", xml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  EXPECT_NO_THROW(simulator.LoadSchedule(schedule.string().c_str()));
  EXPECT_EQ(simulator.GetTimeKeeper()->GetEpochTime(),
            static_cast<::Smp::DateTime>(
                ::Xsmp::DateTime{"2030-01-01T00:00:00.5", "%FT%T"}));
}

TEST(Schedule, RejectsInvalidDateTimeOffset) {
  TemporaryScheduleDirectory directory;
  auto xml = ScheduleHeader("InvalidDateTime");
  xml += R"(  <EpochTime>2030-01-01T00:00:00+15:00</EpochTime>
  <Task Id="InvalidDateTime.Empty" Name="Empty"/>
  <Event Id="InvalidDateTime.Event" Name="Event" xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#InvalidDateTime.Empty"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("invalid-date-time.smpsed", xml);

  ::Xsmp::Simulator simulator;
  EXPECT_THROW(simulator.LoadSchedule(schedule.string().c_str()),
               ::Smp::InvalidFile);
}

TEST(Schedule, RejectsZuluTimeWithoutTimezone) {
  TemporaryScheduleDirectory directory;
  auto xml = ScheduleHeader("ZuluWithoutTimezone");
  xml += R"(  <Task Id="ZuluWithoutTimezone.Empty" Name="Empty"/>
  <Event Id="ZuluWithoutTimezone.Event" Name="Event" xsi:type="Schedule:ZuluEvent" ZuluTime="2099-01-01T00:00:00">
    <Task xlink:href="#ZuluWithoutTimezone.Empty"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("zulu-without-timezone.smpsed", xml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  EXPECT_THROW(simulator.LoadSchedule(schedule.string().c_str()),
               ::Smp::InvalidFile);
}

TEST(Schedule, RollsBackPartiallyRegisteredEventsOnFailure) {
  TemporaryScheduleDirectory directory;
  auto xml = ScheduleHeader("PartialFailure");
  xml += R"(  <Task Id="PartialFailure.Empty" Name="Empty"/>
  <Event Id="PartialFailure.Triggered" Name="Triggered" xsi:type="Schedule:GlobalEventTriggeredEvent"
         StartEvent="StartPartial" TimeKind="SimulationTime" Delay="PT0S">
    <Task xlink:href="#PartialFailure.Empty"/>
  </Event>
  <Event Id="PartialFailure.Invalid" Name="Invalid" xsi:type="Schedule:SimulationEvent" SimulationTime="-PT1S">
    <Task xlink:href="#PartialFailure.Empty"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("partial-failure.smpsed", xml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  EXPECT_THROW(simulator.LoadSchedule(schedule.string().c_str()),
               ::Smp::InvalidFile);

  auto *scheduler = simulator.GetScheduler();
  auto *events = simulator.GetEventManager();
  ASSERT_NE(scheduler, nullptr);
  ASSERT_NE(events, nullptr);
  EXPECT_EQ(scheduler->GetNextScheduledEventTime(),
            std::numeric_limits<::Smp::Duration>::max());
  EXPECT_NO_THROW(events->Emit(events->QueryEventId("StartPartial")));
  EXPECT_EQ(scheduler->GetNextScheduledEventTime(),
            std::numeric_limits<::Smp::Duration>::max());
}

TEST(Schedule, GlobalEventStartsOnceAndStopAllowsRestart) {
  TemporaryScheduleDirectory directory;
  auto xml = ScheduleHeader("Triggered");
  xml += R"(  <Task Id="Triggered.Main" Name="Main">
    <Activity Id="Triggered.Main.Tick" Name="Tick" xsi:type="Schedule:Trigger">
      <EntryPoint>/Probe.first</EntryPoint>
    </Activity>
  </Task>
  <Event Id="Triggered.Event" Name="TriggeredEvent" xsi:type="Schedule:GlobalEventTriggeredEvent"
         StartEvent="StartTask" StopEvent="StopTask" TimeKind="SimulationTime"
         Delay="PT5S">
    <Task xlink:href="#Triggered.Main"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("triggered.smpsed", xml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  auto *probe = new ProbeModel{"Probe", &simulator, &simulator};
  simulator.AddModel(probe);
  simulator.Connect();
  simulator.LoadSchedule(schedule.string().c_str());

  auto *events = simulator.GetEventManager();
  const auto start = events->QueryEventId("StartTask");
  const auto stop = events->QueryEventId("StopTask");
  events->Emit(start);
  events->Emit(stop);
  simulator.Run(6 * ONE_SECOND);
  EXPECT_TRUE(probe->calls.empty());

  events->Emit(start);
  events->Emit(start);
  simulator.Run(6 * ONE_SECOND);
  EXPECT_EQ(probe->calls, (std::vector<int>{1}));
}

TEST(Schedule, ExplicitExitCleansSubscriptionsBeforeServiceDisconnect) {
  TemporaryScheduleDirectory directory;
  auto xml = ScheduleHeader("ExplicitExit");
  xml += R"(  <Task Id="ExplicitExit.Empty" Name="Empty"/>
  <Event Id="ExplicitExit.Event" Name="Event" xsi:type="Schedule:GlobalEventTriggeredEvent"
         StartEvent="StartExplicitExit" TimeKind="SimulationTime" Delay="PT0S">
    <Task xlink:href="#ExplicitExit.Empty"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("explicit-exit.smpsed", xml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.Connect();
  simulator.LoadSchedule(schedule.string().c_str());
  EXPECT_NO_THROW(simulator.Exit());
}

TEST(Schedule, ExitKeepsRunningZuluEntryPointAliveUntilSchedulerStops) {
  using namespace std::chrono_literals;

  TemporaryScheduleDirectory directory;
  auto xml = ScheduleHeader("ConcurrentExit");
  xml += R"(  <Task Id="ConcurrentExit.Tick" Name="Tick">
    <Activity Id="ConcurrentExit.Tick.Trigger" Name="Trigger" xsi:type="Schedule:Trigger">
      <EntryPoint>/Probe.tick</EntryPoint>
    </Activity>
  </Task>
  <Event Id="ConcurrentExit.Event" Name="Event" xsi:type="Schedule:GlobalEventTriggeredEvent"
         StartEvent="StartConcurrentExit" TimeKind="ZuluTime" Delay="PT0S">
    <Task xlink:href="#ConcurrentExit.Tick"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("concurrent-exit.smpsed", xml);

  std::promise<void> enteredPromise;
  auto entered = enteredPromise.get_future();
  std::promise<void> releasePromise;
  auto release = releasePromise.get_future().share();
  PromiseReleaseGuard releaseGuard{releasePromise};
  std::promise<void> finishedPromise;
  auto finished = finishedPromise.get_future();
  std::promise<void> leavingStandbyPromise;
  auto leavingStandby = leavingStandbyPromise.get_future();

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.AddModel(new BlockingProbeModel{"Probe", &simulator, &simulator,
                                            enteredPromise, release,
                                            finishedPromise});
  simulator.Connect();
  simulator.LoadSchedule(schedule.string().c_str());

  const auto leavingStandbyId =
      ::Smp::Services::IEventManager::SMP_LeaveStandbyId;
  const ::Smp::IEntryPoint *leavingStandbyEntryPointPointer{};
  ::Xsmp::EntryPoint leavingStandbyEntryPoint{
      "leavingStandby", "", static_cast<::Smp::IObject *>(&simulator), [&] {
        simulator.GetEventManager()->Unsubscribe(
            leavingStandbyId, leavingStandbyEntryPointPointer);
        leavingStandbyPromise.set_value();
      }};
  leavingStandbyEntryPointPointer = &leavingStandbyEntryPoint;
  auto *events = simulator.GetEventManager();
  events->Subscribe(leavingStandbyId, &leavingStandbyEntryPoint);
  events->Emit(events->QueryEventId("StartConcurrentExit"));

  const auto enteredStatus = entered.wait_for(5s);
  EXPECT_EQ(enteredStatus, std::future_status::ready);
  if (enteredStatus != std::future_status::ready) {
    return;
  }
  auto *scheduler = simulator.GetScheduler();
  const auto scheduledEvent = scheduler->GetCurrentEventId();
  ASSERT_NE(scheduledEvent, -1);

  auto exiting =
      std::async(std::launch::async, [&simulator] { simulator.Exit(); });
  const auto leavingStandbyStatus = leavingStandby.wait_for(5s);

  EXPECT_EQ(leavingStandbyStatus, std::future_status::ready);
  if (leavingStandbyStatus == std::future_status::ready) {
    EXPECT_EQ(exiting.wait_for(0s), std::future_status::timeout);
  }

  // Exit has now cleared the Schedule runtime while its Zulu entry point is
  // still executing. It must remain alive until the scheduler thread joins.
  releaseGuard.Release();

  EXPECT_EQ(finished.wait_for(5s), std::future_status::ready);
  EXPECT_EQ(exiting.wait_for(5s), std::future_status::ready);
  exiting.wait();
  EXPECT_NO_THROW(exiting.get());
  EXPECT_FALSE(scheduler->IsEventScheduled(scheduledEvent));
}

TEST(Schedule, RejectsExecuteTaskCycles) {
  TemporaryScheduleDirectory directory;
  auto xml = ScheduleHeader("Cycle");
  xml += R"(  <Task Id="Cycle.A" Name="A">
    <Activity Id="Cycle.A.ToB" Name="ToB" xsi:type="Schedule:ExecuteTask">
      <Task xlink:href="#Cycle.B"/>
    </Activity>
  </Task>
  <Task Id="Cycle.B" Name="B">
    <Activity Id="Cycle.B.ToA" Name="ToA" xsi:type="Schedule:ExecuteTask">
      <Task xlink:href="#Cycle.A"/>
    </Activity>
  </Task>
  <Event Id="Cycle.Start" Name="Start" xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#Cycle.A"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("cycle.smpsed", xml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  EXPECT_THROW(simulator.LoadSchedule(schedule.string().c_str()),
               ::Smp::InvalidFile);
}

} // namespace
} // namespace Xsmp::L2
