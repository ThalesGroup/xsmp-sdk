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
// File Name    : Int16Matrix.h
// Generated by : XsmpSdkGenerator-1.1.0
// -----------------------------------------------------------------------------
/// @file Xsmp/Tests/Types/Int16Matrix.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef XSMP_TESTS_TYPES_INT16MATRIX_H_
#define XSMP_TESTS_TYPES_INT16MATRIX_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------
// ----------------------------------------------------------------------------

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/Array.h>
#include <Xsmp/Tests/Types/Int16Array.h>

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------
// ----------------------------------------------------------------------------

namespace Xsmp::Tests::Types {
using Int16Matrix = ::Xsmp::Array<::Xsmp::Tests::Types::Int16Array, 3>;

/// Universally unique identifier of type Int16Matrix.
static constexpr ::Smp::Uuid Uuid_Int16Matrix{0xd7d96395U, 0xe3d2U, 0x49a3U,
                                              0xb24bU, 0xf88a0bd08c4aU};

void _Register_Int16Matrix(::Smp::Publication::ITypeRegistry *registry);
} // namespace Xsmp::Tests::Types

#endif // XSMP_TESTS_TYPES_INT16MATRIX_H_
