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
#include <Xsmp/Factory.h>
#include <Xsmp/Helper.h>
#include <algorithm>
#include <typeinfo>

namespace Xsmp {

Factory::Factory(::Smp::String8 name, ::Smp::String8 description,
                 ::Smp::ISimulator *simulator, ::Smp::Uuid uuid,
                 const std::type_info &type, _factory_instantiator_t &&callback)
    : _name(::Xsmp::Helper::checkName(name, simulator)),
      _description(description ? description : ""), _parent(simulator),
      _uuid(uuid), _callback{std::move(callback)},
      _typeName(Xsmp::Helper::demangle(type.name())) {}

::Smp::String8 Factory::GetName() const { return _name.c_str(); }
::Smp::String8 Factory::GetDescription() const { return _description.c_str(); }
::Smp::IObject *Factory::GetParent() const { return _parent; }
::Smp::Uuid Factory::GetUuid() const { return _uuid; }

::Smp::IComponent *Factory::CreateInstance(::Smp::String8 name,
                                           ::Smp::String8 description,
                                           ::Smp::IComposite *parent) {
  auto instance = _callback(name, description, parent, _parent);

  if (instance->GetUuid() != _uuid) {
    Xsmp::Exception::throwException(
        this, "InvalidFactory", "", "The instance UUID ", instance->GetUuid(),
        " does not match with the Factory UUID ", _uuid);
  }
  return instance.release();
}
void Factory::DeleteInstance(::Smp::IComponent *instance) { delete instance; }
::Smp::String8 Factory::GetTypeName() const { return _typeName.c_str(); }

::Smp::IFactory *Factory::Create(::Smp::String8 name,
                                 ::Smp::String8 description,
                                 ::Smp::ISimulator *simulator, ::Smp::Uuid uuid,
                                 const std::type_info &type,
                                 _factory_instantiator_t &&callback) {
  return new ::Xsmp::Factory(name, description, simulator, uuid, type,
                             std::move(callback));
}

} // namespace Xsmp
