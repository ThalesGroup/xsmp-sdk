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

#include <Xsmp/Helper.h>
#include <Xsmp/Object.h>
#include <cctype>
#include <cstddef>

namespace Xsmp {

Object::Object(::Smp::String8 name, ::Smp::String8 description,
        ::Smp::IObject *parent) :
        _name(::Xsmp::Helper::checkName(name, parent)), _description(
                description ? description : ""), _parent(parent) {

}

::Smp::String8 Object::GetName() const {
    return _name.c_str();
}

::Smp::String8 Object::GetDescription() const {
    return _description.c_str();
}

::Smp::IObject* Object::GetParent() const {
    return _parent;
}

} // namespace Xsmp
