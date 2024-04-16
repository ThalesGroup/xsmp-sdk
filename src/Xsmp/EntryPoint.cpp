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

#include <Xsmp/EntryPoint.h>
#include <Xsmp/EntryPointPublisher.h>
#include <Xsmp/Helper.h>
#include <algorithm>

namespace Xsmp {

EntryPoint::EntryPoint(::Smp::String8 name, ::Smp::String8 description,
                       EntryPointPublisher *parent,
                       std::function<void()> &&callback)
    : EntryPoint(name, description, static_cast<::Smp::IObject *>(parent),
                 std::move(callback)) {
  parent->AddEntryPoint(this);
}

EntryPoint::EntryPoint(::Smp::String8 name, ::Smp::String8 description,
                       ::Smp::IObject *parent, std::function<void()> &&callback)
    : _name(::Xsmp::Helper::checkName(name, parent)),
      _description(description ? description : ""), _parent(parent),
      _callback{std::move(callback)} {}

::Smp::String8 EntryPoint::GetName() const { return _name.c_str(); }

::Smp::String8 EntryPoint::GetDescription() const {
  return _description.c_str();
}

::Smp::IObject *EntryPoint::GetParent() const { return _parent; }

void EntryPoint::Execute() const { this->operator()(); }

} // namespace Xsmp
