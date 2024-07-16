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
// -----------------------------------------------------------------------------
// File Name    : Enum1.h
// Generated by : XsmpSdkGenerator-1.1.4
// -----------------------------------------------------------------------------
/// @file Xsmp/Tests/Types/Enum1.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef XSMP_TESTS_TYPES_ENUM1_H_
#define XSMP_TESTS_TYPES_ENUM1_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <map>

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------
// ----------------------------------------------------------------------------

namespace Xsmp::Tests::Types {
enum class Enum1 : ::Smp::Int32 { L1 = 0, L2 = 1, L3 = 2 };

/// Universally unique identifier of type Enum1.
inline constexpr ::Smp::Uuid Uuid_Enum1{0xebca672cU, 0xed02U, 0x4decU, 0x8199U,
                                        0x7049f2f3c03bU};

void _Register_Enum1(::Smp::Publication::ITypeRegistry *registry);

const std::map<Enum1, std::string> Enum1_name_map = {
    {Enum1::L1, "L1"}, {Enum1::L2, "L2"}, {Enum1::L3, "L3"}};

const std::map<Enum1, std::string> Enum1_descr_map = {
    {Enum1::L1, ""}, {Enum1::L2, ""}, {Enum1::L3, ""}};
} // namespace Xsmp::Tests::Types

#endif // XSMP_TESTS_TYPES_ENUM1_H_
