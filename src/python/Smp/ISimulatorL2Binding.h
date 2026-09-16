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

#ifndef PYTHON_SMP_ISIMULATORL2_H_
#define PYTHON_SMP_ISIMULATORL2_H_

#include <Smp/ISimulatorL2.h>
#include <python/ecss_smp.h>

inline void RegisterISimulatorL2(const py::module_ &m) {
  py::class_<::Smp::ISimulatorL2, ::Smp::ISimulator>(m, "ISimulatorL2",
                                                     py::multiple_inheritance())
      .def("LoadAssembly", &::Smp::ISimulatorL2::LoadAssembly,
           py::arg("assembly_path"), py::arg("parent_path") = "",
           py::arg("container_name") = "", py::arg("root_instance_name") = "",
           "Load an SMP Level 2 Assembly file.")
      .def("LoadLinkBase", &::Smp::ISimulatorL2::LoadLinkBase,
           py::arg("link_base_path"), py::arg("parent_path") = "",
           "Load an SMP Level 2 LinkBase file.")
      .def("LoadSchedule", &::Smp::ISimulatorL2::LoadSchedule,
           py::arg("schedule_path"), "Load an SMP Level 2 Schedule file.")
      .def("LoadConfiguration", &::Smp::ISimulatorL2::LoadConfiguration,
           py::arg("configuration_path"), py::arg("parent_path") = "",
           "Load an SMP Level 1 Configuration file.")
      .doc() =
      "SMP Level 2 extension of ISimulator for loading SMDL documents.";
}

#endif // PYTHON_SMP_ISIMULATORL2_H_
