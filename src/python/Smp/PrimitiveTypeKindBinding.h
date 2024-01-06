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

#ifndef PYTHON_SMP_PRIMITIVETYPEKIND_H_
#define PYTHON_SMP_PRIMITIVETYPEKIND_H_

#include <python/ecss_smp.h>
#include <Smp/PrimitiveTypes.h>

inline void RegisterPrimitiveTypeKind(const py::module_ &m) {
    py::enum_<::Smp::PrimitiveTypeKind>(m, "PrimitiveTypeKind")

    .value("PTK_None", ::Smp::PrimitiveTypeKind::PTK_None,
            R"(No type, e.g. for void.)")

    .value("PTK_Char8", ::Smp::PrimitiveTypeKind::PTK_Char8,
            R"(8 bit character type.)")

    .value("PTK_Bool", ::Smp::PrimitiveTypeKind::PTK_Bool,
            R"(Boolean with true and false.)")

    .value("PTK_Int8", ::Smp::PrimitiveTypeKind::PTK_Int8,
            R"(8 bit signed integer type.)")

    .value("PTK_UInt8", ::Smp::PrimitiveTypeKind::PTK_UInt8,
            R"(8 bit unsigned integer type.)")

    .value("PTK_Int16", ::Smp::PrimitiveTypeKind::PTK_Int16,
            R"(16 bit signed integer type.)")

    .value("PTK_UInt16", ::Smp::PrimitiveTypeKind::PTK_UInt16,
            R"(16 bit unsigned integer type.)")

    .value("PTK_Int32", ::Smp::PrimitiveTypeKind::PTK_Int32,
            R"(32 bit signed integer type.)")

    .value("PTK_UInt32", ::Smp::PrimitiveTypeKind::PTK_UInt32,
            R"(32 bit unsigned integer type.)")

    .value("PTK_Int64", ::Smp::PrimitiveTypeKind::PTK_Int64,
            R"(64 bit signed integer type.)")

    .value("PTK_UInt64", ::Smp::PrimitiveTypeKind::PTK_UInt64,
            R"(64 bit unsigned integer type.)")

    .value("PTK_Float32", ::Smp::PrimitiveTypeKind::PTK_Float32,
            R"(32 bit single-precision floating-point type.)")

    .value("PTK_Float64", ::Smp::PrimitiveTypeKind::PTK_Float64,
            R"(64 bit double-precision floating-point type.)")

    .value("PTK_Duration", ::Smp::PrimitiveTypeKind::PTK_Duration,
            R"(Duration in nanoseconds.)")

    .value("PTK_DateTime", ::Smp::PrimitiveTypeKind::PTK_DateTime,
            R"(Absolute time in nanoseconds.)")

    .value("PTK_String8", ::Smp::PrimitiveTypeKind::PTK_String8,
            R"(8 bit character string.)");
}

#endif // PYTHON_SMP_PRIMITIVETYPEKIND_H_
