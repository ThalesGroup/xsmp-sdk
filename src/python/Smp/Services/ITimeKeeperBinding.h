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

#ifndef PYTHON_SMP_SERVICES_ITIMEKEEPER_H_
#define PYTHON_SMP_SERVICES_ITIMEKEEPER_H_

#include <python/ecss_smp.h>
#include <Smp/Services/ITimeKeeper.h>

inline void RegisterITimeKeeper(const py::module_ &m) {

    py::class_<::Smp::Services::ITimeKeeper, ::Smp::IService>(m, "ITimeKeeper",
            py::multiple_inheritance())

    .def("GetSimulationTime", &::Smp::Services::ITimeKeeper::GetSimulationTime,
            R"(Return Simulation time.
Simulation time is a relative time that starts at 0.)")

    .def("GetMissionTime", &::Smp::Services::ITimeKeeper::GetMissionTime,
            R"(Return Mission time.
Mission time is a relative time with a fixed offset to simulation time. Mission time typically progresses with simulation time, but can be changed with the two methods SetMissionTime and SetMissionStart. Further, mission time is updated when changing epoch time with SetEpochTime.)")

    .def("GetEpochTime", &::Smp::Services::ITimeKeeper::GetEpochTime,
            R"(Return Epoch time.
Epoch time is an absolute time with a fixed offset to simulation time. Epoch time typically progresses with simulation time, but can be changed with SetEpochTime.)")

    .def("GetMissionStartTime",
            &::Smp::Services::ITimeKeeper::GetMissionStartTime,
            "Get mission start time.")

    .def("GetZuluTime", &::Smp::Services::ITimeKeeper::GetZuluTime,
            R"(Return Zulu time.
Zulu time is a system dependent time and not related to simulation time. Zulu time is typically related to the system clock of the computer.)")

    .def("SetSimulationTime", &::Smp::Services::ITimeKeeper::SetSimulationTime,
            py::arg("simulation_time"),
            R"(Manually advance Simulation time.
This method can only be called during a PreSimTimeChange event.
When the Time Keeper updates simulation time in response to the Scheduler executing a new event, the update is performed in three steps.
1. First the PreSimTimeChange event is emitted.
2. Then the Simulation time is changed to the time of the event that is about to be executed.
3. Finally the PostSimTimeChange event is emitted.
This method can only set SimTime between current time and the time that is about to the set by the procedure described above.
Also, invoking this method will not trigger again the PreSimTimeChange or PostSimTimeChange events.)")

    .def("SetEpochTime", &::Smp::Services::ITimeKeeper::SetEpochTime,
            py::arg("epoch_time"),
            R"(Set Epoch time.
Changes the offset between simulation time and epoch time.
Calling this method shall raise a global EpochTimeChanged event in the Event Manager.)")

    .def("SetMissionStartTime",
            &::Smp::Services::ITimeKeeper::SetMissionStartTime,
            py::arg("mission_start"),
            R"(Set Mission time by defining the mission start time.
In future calls to GetMissionTime, the mission time is calculated using the formula MissionTime = EpochTime - missionStart.
Changes the offset between simulation time and mission time.
The mission time itself will be calculated as the offset between the current epoch time and the given mission start:
MissionTime = EpochTime - MissionStart

Calling this method shall raise a global MissionTimeChanged event in the Event Manager.)")

    .def("SetMissionTime", &::Smp::Services::ITimeKeeper::SetMissionTime,
            py::arg("mission_time"),
            R"(Set Mission time by providing the current mission time.
This effectively sets the MissionStartTime using the formula MissionStartTime = EpochTime - missionTime.
Changes the offset between simulation time and mission time.
Calling this method shall raise a global MissionTimeChanged event in the Event Manager.)")

    .doc() =
            R"(This interface gives access to the Time Keeper Service.
Components can query for the time (using the four available time kinds), and can change the epoch or mission time.)";
}

#endif // PYTHON_SMP_SERVICES_ITIMEKEEPER_H_
