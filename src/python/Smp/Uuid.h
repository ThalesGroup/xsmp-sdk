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

#include <python/ecss_smp.h>
#include <pybind11/pybind11.h>
#include <Smp/Uuid.h>

inline void RegisterUuid(const py::module_ &m) {
    py::class_<::Smp::Uuid>(m, "Uuid")

    .def(py::init<uint32_t, uint16_t, uint16_t, uint16_t, uint64_t>(),
            "Constructor based on the format of the UUID string")

    .def(py::init<const char*>(), "Constructor based on string.",
            py::arg("uuid"))

    .def(py::init<const ::Smp::Uuid&>(), "Default copy constructor")

    .def_readwrite("Data1", &::Smp::Uuid::Data1, "8 hex nibbles.")

    .def_readwrite("Data2", &::Smp::Uuid::Data2, "3x4 hex nibbles.")

    .def_readwrite("Data3", &::Smp::Uuid::Data3, "6x2 hex nibbles.")

    .doc() =
            R"(Universally Unique Identifier.
For a unique identification of types (and hence models), SMP uses Universally Unique Identifiers with the format specified by the Open Group (http://www.opengroup.org).)";
}

