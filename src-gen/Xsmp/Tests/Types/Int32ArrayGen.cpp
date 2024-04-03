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
// File Name    : Int32Array.cpp
// Generated by : XsmpSdkGenerator-1.1.0
// -----------------------------------------------------------------------------
/// @file Xsmp/Tests/Types/Int32Array.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------
// ----------------------------------------------------------------------------

#include <Xsmp/Tests/Types/Int32Array.h>

namespace Xsmp::Tests::Types {
void _Register_Int32Array(::Smp::Publication::ITypeRegistry *registry) {
  registry->AddArrayType("Int32Array",                          // Name
                         "",                                    // Description
                         ::Xsmp::Tests::Types::Uuid_Int32Array, // UUID
                         ::Smp::Uuids::Uuid_Int32, // Item Type UUID
                         sizeof(::Smp::Int32),     // Item Type size
                         3,                        // size of the array
                         false                     // is simple array
  );
}
} // namespace Xsmp::Tests::Types
