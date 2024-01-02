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

#include <Xsmp/Exception.h>
#include <Xsmp/Object.h>
#include <cctype>
#include <cstddef>

namespace Xsmp {

namespace {

///Check that an Object name is valid
::Smp::String8 checkName(::Smp::String8 name, ::Smp::IObject const *parent) {

    // the name cannot be null
    if (!name)
        ::Xsmp::Exception::throwInvalidObjectName(parent, "<nullptr>");

    // the name must start with a letter
    if (!std::isalpha(static_cast<unsigned char>(name[0])))
        ::Xsmp::Exception::throwInvalidObjectName(parent, name);

    std::size_t i = 1;
    // skip following letters, digits and "_"
    while (std::isalnum(static_cast<unsigned char>(name[i])) || (name[i] == '_')) {
        ++i;
    }

    // parse an optional array
    while (name[i] == '[') {
        ++i;
        // index must start with a digit
        if (!std::isdigit(static_cast<unsigned char>(name[i])))
            ::Xsmp::Exception::throwInvalidObjectName(parent, name);
        ++i;
        // skip following digits
        while (std::isdigit(static_cast<unsigned char>(name[i]))) {
            ++i;
        }

        // check closing bracket
        if (name[i++] != ']')
            ::Xsmp::Exception::throwInvalidObjectName(parent, name);
    }

    // check end of name
    if (name[i] != '\0')
        ::Xsmp::Exception::throwInvalidObjectName(parent, name);

    return name;

}
} // namespace
Object::Object(::Smp::String8 name, ::Smp::String8 description,
        ::Smp::IObject *parent) :
        _name(checkName(name, parent)), _description(
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

void Object::SetDescription(::Smp::String8 description) noexcept {
    _description = description ? description : "";
}

} // namespace Xsmp
