// Copyright 2023-2024 THALES ALENIA SPACE FRANCE. All rights reserved.
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

#ifndef PYTHON_SMP_UUID_H_
#define PYTHON_SMP_UUID_H_

#include <Smp/PrimitiveTypes.h>
#include <Smp/Uuid.h>
#include <python/ecss_smp.h>

inline void RegisterUuid(const py::module_ &m) {
  py::class_<::Smp::Uuid>(m, "Uuid")

      .def(py::init<uint32_t, uint16_t, uint16_t, uint16_t, uint64_t>(),
           "Constructor based on the format of the UUID string")

      .def(py::init<const char *>(), "Constructor based on string.",
           py::arg("uuid"))

      .def(py::init<const ::Smp::Uuid &>(), "Default copy constructor")

      .def_readwrite("Data1", &::Smp::Uuid::Data1, "8 hex nibbles.")

      .def_readwrite("Data2", &::Smp::Uuid::Data2, "3x4 hex nibbles.")

      .def_readwrite("Data3", &::Smp::Uuid::Data3, "6x2 hex nibbles.")

      .doc() =
      R"(Universally Unique Identifier.
For a unique identification of types (and hence models), SMP uses Universally Unique Identifiers with the format specified by the Open Group (http://www.opengroup.org).)";

  py::class_<::Smp::Uuids>(m, "Uuids")

      .def_readonly_static("Uuid_Uuid", &::Smp::Uuids::Uuid_Uuid,
                           "Unique Identifier of type Uuid.")

      .def_readonly_static("Uuid_Char8", &::Smp::Uuids::Uuid_Char8,
                           "Unique Identifier of type Char8.")

      .def_readonly_static("Uuid_Bool", &::Smp::Uuids::Uuid_Bool,
                           "Unique Identifier of type Bool.")

      .def_readonly_static("Uuid_Int8", &::Smp::Uuids::Uuid_Int8,
                           "Unique Identifier of type Int8.")

      .def_readonly_static("Uuid_UInt8", &::Smp::Uuids::Uuid_UInt8,
                           "Unique Identifier of type UInt8.")

      .def_readonly_static("Uuid_Int16", &::Smp::Uuids::Uuid_Int16,
                           "Unique Identifier of type Int16.")

      .def_readonly_static("Uuid_UInt16", &::Smp::Uuids::Uuid_UInt16,
                           "Unique Identifier of type UInt16.")

      .def_readonly_static("Uuid_Int32", &::Smp::Uuids::Uuid_Int32,
                           "Unique Identifier of type Int32.")

      .def_readonly_static("Uuid_UInt32", &::Smp::Uuids::Uuid_UInt32,
                           "Unique Identifier of type UInt32.")

      .def_readonly_static("Uuid_Int64", &::Smp::Uuids::Uuid_Int64,
                           "Unique Identifier of type Int64.")

      .def_readonly_static("Uuid_UInt64", &::Smp::Uuids::Uuid_UInt64,
                           "Unique Identifier of type UInt64.")

      .def_readonly_static("Uuid_Float32", &::Smp::Uuids::Uuid_Float32,
                           "Unique Identifier of type Float32.")

      .def_readonly_static("Uuid_Float64", &::Smp::Uuids::Uuid_Float64,
                           "Unique Identifier of type Float64.")

      .def_readonly_static("Uuid_Duration", &::Smp::Uuids::Uuid_Duration,
                           "Unique Identifier of type Duration.")

      .def_readonly_static("Uuid_DateTime", &::Smp::Uuids::Uuid_DateTime,
                           "Unique Identifier of type DateTime.")

      .def_readonly_static("Uuid_String8", &::Smp::Uuids::Uuid_String8,
                           "Unique Identifier of type String8.")

      .def_readonly_static("Uuid_PrimitiveTypeKind",
                           &::Smp::Uuids::Uuid_PrimitiveTypeKind,
                           "Unique Identifier of type PrimitiveTypeKind.")

      .def_readonly_static("Uuid_EventId", &::Smp::Uuids::Uuid_EventId,
                           "Unique Identifier of type EventId.")

      .def_readonly_static("Uuid_LogMessageKind",
                           &::Smp::Uuids::Uuid_LogMessageKind,
                           "Unique Identifier of type LogMessageKind.")

      .def_readonly_static("Uuid_TimeKind", &::Smp::Uuids::Uuid_TimeKind,
                           "Unique Identifier of type TimeKind.")

      .def_readonly_static("Uuid_ViewKind", &::Smp::Uuids::Uuid_ViewKind,
                           "Unique Identifier of type ViewKind.")

      .def_readonly_static("Uuid_ParameterDirectionKind",
                           &::Smp::Uuids::Uuid_ParameterDirectionKind,
                           "Unique Identifier of type ParameterDirectionKind.")

      .def_readonly_static("Uuid_ComponentStateKind",
                           &::Smp::Uuids::Uuid_ComponentStateKind,
                           "Unique Identifier of type ComponentStateKind.")

      .def_readonly_static("Uuid_AccessKind", &::Smp::Uuids::Uuid_AccessKind,
                           "Unique Identifier of type AccessKind.")

      .def_readonly_static("Uuid_SimulatorStateKind",
                           &::Smp::Uuids::Uuid_SimulatorStateKind,
                           "Unique Identifier of type SimulatorStateKind.")

      .doc() = "Placeholder class with universally unique identifiers of "
               "pre-defined SMP types.";
}

#endif // PYTHON_SMP_UUID_H_
