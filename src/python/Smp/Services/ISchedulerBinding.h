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

#ifndef PYTHON_SMP_SERVICES_ISCHEDULER_H_
#define PYTHON_SMP_SERVICES_ISCHEDULER_H_

#include <python/ecss_smp.h>
#include <Smp/Services/IScheduler.h>

inline void RegisterIScheduler(const py::module_ &m) {

    py::class_<::Smp::Services::IScheduler, ::Smp::IService>(m, "IScheduler",
            py::multiple_inheritance())

    .def("AddImmediateEvent", &::Smp::Services::IScheduler::AddImmediateEvent,
            py::arg("entry_point"),
            R"(Add an immediate event to the scheduler.
An immediate event is an event that will be added as a simulation time event (with simulation delta time of 0) at the front of the event queue. As such, it will be executed when the scheduler processes its simulation time events again, but not immediately in the call to AddImmediateEvent().
When the simulator is in Standby state, simulation time does not progress, and simulation time events (including immediate events) are not processed.
@remarks To execute an entry point immediately without going through the scheduler, its Execute() method can be called.
@remarks For events scheduled at the same time (including immediate events), the Scheduler policy is to always execute those first that have been posted first ("First scheduled, first executed").)")

    .def("AddSimulationTimeEvent",
            &::Smp::Services::IScheduler::AddSimulationTimeEvent,
            py::arg("entry_point"), py::arg("simulation_time"),
            py::arg("cycle_time") = 0, py::arg("repeat") = 0,
            R"(Add event to scheduler that is called based on simulation time.
An event with repeat=0 is not cyclic. It will be removed automatically after is has been triggered.
An event with repeat>0 is cyclic, and will be repeated 'repeat' times. Therefore, it will be called repeat+1 times, and then it will be removed automatically.
An event with repeat=-1 is cyclic as well, but it will be triggered forever, unless it is removed from the scheduler using the RemoveEvent() method.
For a cyclic event, the cycleTime needs to be positive. Otherwise, an InvalidCycleTime exception is thrown. For non-cyclic events, it is stored but not used.
The simulationTime must not be negative. Otherwise, an InvalidEventTime exception is thrown.)")

    .def("AddMissionTimeEvent",
            &::Smp::Services::IScheduler::AddMissionTimeEvent,
            py::arg("entry_point"), py::arg("mission_time"),
            py::arg("cycle_time") = 0, py::arg("repeat") = 0,
            R"(Add event to scheduler that is called based on mission time.
An event with repeat=0 is not cyclic. It will be removed automatically after is has been triggered.
An event with repeat>0 is cyclic, and will be repeated repeat times. Therefore, it will be called repeat+1 times, and then it will be removed automatically.
An event with repeat=-1 is cyclic as well, but it will be triggered forever, unless it is removed from the scheduler using the RemoveEvent() method.
For a cyclic event, the cycleTime needs to be positive. Otherwise, an InvalidCycleTime exception is thrown. For non-cyclic events, it is stored but not used.
The missionTime must not be before the current mission time of the ITimeKeeper service. Otherwise, an InvalidEventTime exception is thrown.)")

    .def("AddEpochTimeEvent", &::Smp::Services::IScheduler::AddEpochTimeEvent,
            py::arg("entry_point"), py::arg("epoch_time"),
            py::arg("cycle_time") = 0, py::arg("repeat") = 0,
            R"(Add event to scheduler that is called based on epoch time.
An event with repeat=0 is not cyclic. It will be removed automatically after is has been triggered.
An event with repeat>0 is cyclic, and will be repeated repeat times. Therefore, it will be called repeat+1 times, and then it will be removed automatically.
An event with repeat=-1 is cyclic as well, but it will be triggered forever, unless it is removed from the scheduler using the RemoveEvent() method.
For a cyclic event, the cycleTime needs to be positive. Otherwise, an InvalidCycleTime exception is thrown. For non-cyclic events, it is stored but not used.
The epochTime must not be before the current epoch time of the ITimeKeeper service. Otherwise, an InvalidEventTime exception is thrown.)")

    .def("AddZuluTimeEvent", &::Smp::Services::IScheduler::AddZuluTimeEvent,
            py::arg("entry_point"), py::arg("zulu_time"),
            py::arg("cycle_time") = 0, py::arg("repeat") = 0,
            R"(Add event to scheduler that is called based on Zulu time.
An event with repeat=0 is not cyclic. It will be removed automatically after is has been triggered.
An event with repeat>0 is cyclic, and will be repeated repeat times. Therefore, it will be called repeat+1 times, and then it will be removed automatically.
An event with repeat=-1 is cyclic as well, but it will be triggered forever, unless it is removed from the scheduler using the RemoveEvent() method.
For a cyclic event, the cycleTime needs to be positive. Otherwise, an InvalidCycleTime exception is thrown. For non-cyclic events, it is stored but not used.
The zuluTime must not be before the current Zulu time of the ITimeKeeper service. Otherwise, an InvalidEventTime exception is thrown.)")

    .def("SetEventSimulationTime",
            &::Smp::Services::IScheduler::SetEventSimulationTime,
            py::arg("event"), py::arg("simulation_time"),
            R"(Update when an existing simulation time event on the scheduler shall be triggered.
When the given event Id is not a valid identifier of a scheduler event, the method throws an exception of type InvalidEventId.
In case an event is registered under the given event Id but it is not a simulation time event, the method throws an exception of type InvalidEventId as well.
The simulationTime must not be negative. Otherwise, the event  will never be executed, but immediately removed.)")

    .def("SetEventMissionTime",
            &::Smp::Services::IScheduler::SetEventMissionTime, py::arg("event"),
            py::arg("mission_time"),
            R"(Update when an existing mission time event on the scheduler shall be triggered.
When the given event Id is not a valid identifier of a scheduler event, the method throws an exception of type InvalidEventId.
In case an event is registered under the given event Id but it is not a mission time event, the method throws an exception of type InvalidEventId as well.
The missionTime must not be before the current mission time of the ITimeKeeper service. Otherwise, the event will never be executed, but immediately removed.)")

    .def("SetEventEpochTime", &::Smp::Services::IScheduler::SetEventEpochTime,
            py::arg("event"), py::arg("epoch_time"),
            R"(Update when an existing epoch time event on the scheduler (an event that has been registered using AddEpochTimeEvent()) shall be triggered.
When the given event Id  is not a valid identifier of a scheduler event, the method throws an exception of type InvalidEventId.
In case an event is registered under the given event Id but it is not an epoch time event, the method throws an exception of type InvalidEventId as well.
The epochTime must not be before the current epoch time of the ITimeKeeper service. Otherwise, the event will never be executed, but immediately removed.)")

    .def("SetEventZuluTime", &::Smp::Services::IScheduler::SetEventZuluTime,
            py::arg("event"), py::arg("zulu_time"),
            R"(Update when an existing zulu time event on the scheduler shall be triggered.
When the given event Id is not a valid identifier of a scheduler event, the method throws an exception of type InvalidEventId.
In case an event is registered under the given event Id but it is not a zulu time event, the method throws an exception of type InvalidEventId as well.
The zuluTime must not be before the current Zulu time of the ITimeKeeper service. Otherwise, the event will never be executed, but immediately removed.)")

    .def("SetEventCycleTime", &::Smp::Services::IScheduler::SetEventCycleTime,
            py::arg("event"), py::arg("cycle_time"),
            R"(Update cycle time of an existing event on the scheduler.
When the given event is not a valid identifier of a scheduler event, the method throws an exception of type InvalidEventId.
For a cyclic event, the cycleTime needs to be positive. For non-cyclic events, it is stored but not used.)")

    .def("SetEventRepeat", &::Smp::Services::IScheduler::SetEventRepeat,
            py::arg("event"), py::arg("repeat"),
            R"(Update the repeat counter of an existing event on the scheduler.
When the given event is not a valid identifier of a scheduler event, the method throws an exception of type InvalidEventId.
An event with repeat=0 is not cyclic. It will be removed automatically after is has been triggered.
An event with repeat>0 is cyclic, and will be repeated 'repeat' times. Therefore, it will be called repeat+1 times, and then it will be removed automatically.
An event with repeat=-1 is cyclic as well, but it will be triggered forever, unless it is removed from the scheduler using the RemoveEvent() method.
For a cyclic event, the cycleTime needs to be positive. For non-cyclic events, it is ignored.)")

    .def("RemoveEvent", &::Smp::Services::IScheduler::RemoveEvent,
            py::arg("event"),
            R"(Remove an event from the scheduler.
When the given event is not a valid identifier of a scheduler event, the method throws an exception of type InvalidEventId.
An event with repeat=0 is removed automatically after it has been triggered.)")

    .def("GetCurrentEventId", &::Smp::Services::IScheduler::GetCurrentEventId,
            R"(Return the ID of the event currently executed by the scheduler.
If not event is executed, this function returns -1.
The operation returns -1 when no event is being executed.)")

    .def("GetNextScheduledEventTime",
            &::Smp::Services::IScheduler::GetNextScheduledEventTime,
            R"(Get the time of next scheduled Event.
This function can only provide the time of the next event at the time of invocation.
There is no guarantee that no events will be inserted between the method invocation and the (currently) next event.
Events scheduled in Zulu Time are not considered, as these Events do not have a fixed defined Simulation Time.)")

    .doc() =
            R"(This interface gives access to the Scheduler Service.
Components can register (Add) and unregister (Remove) entry points for scheduling. Further, they can set (Set) individual attributes of events on the scheduler.
The complete state of the Scheduler, with the exception of Events scheduled using ZuluTime, shall be part of persisted data and saved/restored to/from breakpoints.)";
}

#endif // PYTHON_SMP_SERVICES_ISCHEDULER_H_
