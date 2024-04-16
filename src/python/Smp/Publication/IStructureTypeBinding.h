// Copyright 2024 THALES ALENIA SPACE FRANCE. All rights reserved.
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

#ifndef PYTHON_SMP_PUBLICATION_ISTRUCTURETYPE_H_
#define PYTHON_SMP_PUBLICATION_ISTRUCTURETYPE_H_

#include <Smp/Publication/IStructureType.h>
#include <python/ecss_smp.h>

inline void RegisterIStructureType(const py::module_ &m) {
  py::class_<::Smp::Publication::IStructureType, ::Smp::Publication::IType>(
      m, "IStructureType", py::multiple_inheritance())

      .def("AddField", &::Smp::Publication::IStructureType::AddField,
           py::arg("name"), py::arg("description"), py::arg("uuid"),
           py::arg("offset"), py::arg("view") = ::Smp::ViewKind::VK_All,
           py::arg("state") = true, py::arg("input") = false,
           py::arg("output") = false, "Add a field to the Structure.")

      .doc() = "This interface defines a user defined structure type.";
}

#endif // PYTHON_SMP_PUBLICATION_ISTRUCTURETYPE_H_
