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

#ifndef PYTHON_SMP_IOUTPUTFIELD_H_
#define PYTHON_SMP_IOUTPUTFIELD_H_

#include <Smp/IField.h>
#include <Smp/IOutputField.h>
#include <python/ecss_smp.h>

inline void RegisterIOutputField(const py::module_ &m) {
  py::class_<::Smp::IOutputField, ::Smp::IField>(m, "IOutputField",
                                                 py::multiple_inheritance())

      .def("Connect", &::Smp::IOutputField::Connect, py::arg("target"),
           R"(Connect this field to a target field for direct data flow.
As the Push() operation only requires to set a value, the target field can be any field (it does not need to be of type IOutputField).)")

      .def("Disconnect", &::Smp::IOutputField::Disconnect, py::arg("target"),
           "Disconnect a target field that has been connected before.")

      .def("Push", &::Smp::IOutputField::Push,
           "Push the current field value to all connected target fields.")

      .def("GetInputFields", &::Smp::IOutputField::GetInputFields,
           "Collection of fields that have been connected to the output field "
           "for data flow.",
           py::return_value_policy::reference_internal)

      .def("IsAutomatic", &::Smp::IOutputField::IsAutomatic,
           "Tell whether the output field pushes its value automatically each "
           "time it changes, or only when Push() is called.")

      .doc() = "This interface is implemented by a Field that can take part as "
               "output field in direct inter-component data flow.";
}

#endif // PYTHON_SMP_IOUTPUTFIELD_H_
