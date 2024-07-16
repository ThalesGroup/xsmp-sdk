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
// File Name    : Vector.cpp
// Generated by : XsmpSdkGenerator-1.1.4
// -----------------------------------------------------------------------------
/// @file Xsmp/Tests/Types/Vector.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------
// ----------------------------------------------------------------------------

#include <Smp/Publication/IStructureType.h>
#include <Xsmp/Tests/Types/Vector.h>
#include <cstddef>

namespace Xsmp::Tests::Types {
void Vector::_Register(::Smp::Publication::ITypeRegistry *registry) {
  auto *type =
      registry->AddStructureType("Vector", /// Name
                                 "",       /// description
                                 ::Xsmp::Tests::Types::Uuid_Vector /// UUID
      );

  /// Register the Fields of the Structure
  type->AddField("x", "",
                 ::Smp::Uuids::Uuid_Float64, /// UUID of the Field Type
                 offsetof(Vector, x), /// Compute the offset of the current item
                 ::Smp::ViewKind::VK_None, /// viewkind
                 true,                     /// state
                 false,                    /// is an input field
                 false                     /// is an output field
  );
  type->AddField("y", "",
                 ::Smp::Uuids::Uuid_Float64, /// UUID of the Field Type
                 offsetof(Vector, y), /// Compute the offset of the current item
                 ::Smp::ViewKind::VK_None, /// viewkind
                 true,                     /// state
                 false,                    /// is an input field
                 false                     /// is an output field
  );
  type->AddField("z", "",
                 ::Smp::Uuids::Uuid_Float64, /// UUID of the Field Type
                 offsetof(Vector, z), /// Compute the offset of the current item
                 ::Smp::ViewKind::VK_None, /// viewkind
                 true,                     /// state
                 false,                    /// is an input field
                 false                     /// is an output field
  );
}

} // namespace Xsmp::Tests::Types
