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
// File Name    : Int32Array.h
// Generated by : XsmpSdkGenerator-1.1.4
// -----------------------------------------------------------------------------
/// @file Xsmp/Tests/Types/Int32Array.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef XSMP_TESTS_TYPES_INT32ARRAY_H_
#define XSMP_TESTS_TYPES_INT32ARRAY_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/Array.h>

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------
// ----------------------------------------------------------------------------

namespace Xsmp::Tests::Types {
using Int32Array = ::Xsmp::Array<::Smp::Int32, 3>;

/// Universally unique identifier of type Int32Array.
inline constexpr ::Smp::Uuid Uuid_Int32Array{0x13c47a9fU, 0xd605U, 0x4107U,
                                             0xb751U, 0x75e75463c8ffU};

void _Register_Int32Array(::Smp::Publication::ITypeRegistry *registry);
} // namespace Xsmp::Tests::Types

#endif // XSMP_TESTS_TYPES_INT32ARRAY_H_
