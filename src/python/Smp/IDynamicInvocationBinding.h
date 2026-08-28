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

#ifndef PYTHON_SMP_IDYNAMICINVOCATION_H_
#define PYTHON_SMP_IDYNAMICINVOCATION_H_

#include <Smp/IDynamicInvocation.h>
#include <python/ecss_smp.h>

inline void RegisterIDynamicInvocation(const py::module_ &m) {
  py::class_<::Smp::IDynamicInvocation, ::Smp::IComponent>(
      m, "IDynamicInvocation", py::multiple_inheritance())

      .def("GetOperation", &::Smp::IDynamicInvocation::GetOperation,
           py::arg("name"), py::return_value_policy::reference_internal,
           "Get a published operation by name, or None when there is no such "
           "operation.")

      .def("GetOperations", &::Smp::IDynamicInvocation::GetOperations,
           py::return_value_policy::reference_internal,
           "Get the collection of all operations published by the component.")

      .def("GetProperty", &::Smp::IDynamicInvocation::GetProperty,
           py::arg("name"), py::return_value_policy::reference_internal,
           "Get a published property by name, or None when there is no such "
           "property.")

      .def("GetProperties", &::Smp::IDynamicInvocation::GetProperties,
           py::return_value_policy::reference_internal,
           "Get the collection of all properties published by the component.")

      .doc() =
      R"(Interface for a component that supports dynamic invocation of operations.
Since SMP 2025 a request object is created from the operation itself, obtained with GetOperation(), and no longer from the component.)";
}

#endif // PYTHON_SMP_IDYNAMICINVOCATION_H_
