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
// File Name    : SimpleBoolArray.h
// Generated by : XsmpSdkGenerator-1.1.4
// -----------------------------------------------------------------------------
/// @file Xsmp/Tests/Types/SimpleBoolArray.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef XSMP_TESTS_TYPES_SIMPLEBOOLARRAY_H_
#define XSMP_TESTS_TYPES_SIMPLEBOOLARRAY_H_

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
using SimpleBoolArray = ::Xsmp::Array<::Smp::Bool, 3>::simple;

/// Universally unique identifier of type SimpleBoolArray.
inline constexpr ::Smp::Uuid Uuid_SimpleBoolArray{0x658aea44U, 0xedeeU, 0x43d4U,
                                                  0xa974U, 0xe438cd6dd475U};

void _Register_SimpleBoolArray(::Smp::Publication::ITypeRegistry *registry);
} // namespace Xsmp::Tests::Types

#endif // XSMP_TESTS_TYPES_SIMPLEBOOLARRAY_H_
