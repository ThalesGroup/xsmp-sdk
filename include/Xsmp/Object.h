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

#ifndef XSMP_OBJECT_H_
#define XSMP_OBJECT_H_

#include <Smp/IObject.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/cstring.h>
#include <string>

/// XSMP standard types and interfaces.
namespace Xsmp {

/// @class ServObjectice
/// XSMP implementation of ::Smp::IObject
class Object : public virtual ::Smp::IObject {
public:
  /// Constructs a new Object .
  /// @param name The name of the object.
  /// @param description The description of the object.
  /// @param parent The parent object in the hierarchy.
  explicit Object(::Smp::String8 name, ::Smp::String8 description = "",
                  ::Smp::IObject *parent = nullptr);

  /// Virtual destructor to release memory.
  ~Object() noexcept override = default;

  /// Return the name of the object.
  /// @return  Name of object.
  ::Smp::String8 GetName() const final;

  /// Return the description of the object ("property getter").
  /// @return  Description of object.
  ::Smp::String8 GetDescription() const final;

  /// Returns the parent object of the object.
  /// @return  Parent object of object or null if object has no parent.
  ::Smp::IObject *GetParent() const final;

private:
  cstring _name;
  cstring _description;
  ::Smp::IObject *_parent;
};

} // namespace Xsmp
#endif // XSMP_OBJECT_H_
