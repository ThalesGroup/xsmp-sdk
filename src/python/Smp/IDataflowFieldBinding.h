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

#ifndef PYTHON_SMP_IDATAFLOWFIELD_H_
#define PYTHON_SMP_IDATAFLOWFIELD_H_


#include <python/ecss_smp.h>
#include <Smp/IField.h>
#include <Smp/IDataflowField.h>

inline void RegisterIDataflowField(const py::module_ &m) {
    py::class_<::Smp::IDataflowField, ::Smp::IField>(m, "IDataflowField",
            py::multiple_inheritance())

    .def("Connect", &::Smp::IDataflowField::Connect, py::arg("target"),
            R"(Connect this field to a target field for direct data flow.
As the Push() operation only requires to set a value, the target field can be any field (it does not need to be of type IDataflowField).)")

    .def("Push", &::Smp::IDataflowField::Push,
            "Push the current field value to all connected target fields.")

    .doc() =
            "This interface is implemented by a Field that can take part in direct inter-component data flow.";
}

#endif // PYTHON_SMP_IDATAFLOWFIELD_H_
