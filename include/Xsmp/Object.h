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

namespace Xsmp {

class Object : public virtual ::Smp::IObject {
public:
  explicit Object(::Smp::String8 name, ::Smp::String8 description = "",
                  ::Smp::IObject *parent = nullptr);
  Object(const Object &) = delete;
  Object &operator=(const Object &) = delete;
  ~Object() noexcept override = default;
  ::Smp::String8 GetName() const final;
  ::Smp::String8 GetDescription() const final;
  ::Smp::IObject *GetParent() const final;

private:
  cstring _name;
  cstring _description;
  ::Smp::IObject *_parent;
};

} // namespace Xsmp
#endif // XSMP_OBJECT_H_
