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

#ifndef PYTHON_SMP_SERVICES_TIMEKIND_H_
#define PYTHON_SMP_SERVICES_TIMEKIND_H_

#include <python/ecss_smp.h>

#include <Smp/Services/TimeKind.h>

inline void RegisterTimeKind(const py::module_ &m) {
    py::enum_<::Smp::Services::TimeKind>(m, "TimeKind")

    .value("TK_SimulationTime", ::Smp::Services::TimeKind::TK_SimulationTime,
            "Simulation time.")

    .value("TK_MissionTime", ::Smp::Services::TimeKind::TK_MissionTime,
            "Mission time.")

    .value("TK_EpochTime", ::Smp::Services::TimeKind::TK_EpochTime,
            "Epoch time.")

    .value("TK_ZuluTime", ::Smp::Services::TimeKind::TK_ZuluTime, "Zulu time.");
}

#endif // PYTHON_SMP_SERVICES_TIMEKIND_H_