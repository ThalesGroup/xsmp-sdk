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

#ifndef XSMP_FACTORY_H_
#define XSMP_FACTORY_H_

#include <Smp/IComponent.h>
#include <Smp/IFactory.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Uuid.h>
#include <functional>
#include <type_traits>
#include <typeinfo>

namespace Smp {
class ISimulator;
} // namespace Smp

namespace Xsmp::Factory {

using _factory_instantiator_t = std::function<std::unique_ptr<::Smp::IComponent>(
        ::Smp::String8 name,
        ::Smp::String8 description,
        ::Smp::IComposite* parent,
        ::Smp::ISimulator *simulator)>;

::Smp::IFactory* Create(::Smp::String8 name, ::Smp::String8 description,
        ::Smp::ISimulator *simulator, ::Smp::Uuid uuid,
        const std::type_info &type, _factory_instantiator_t &&callback);

template<typename T>
::Smp::IFactory* Create(::Smp::String8 name, ::Smp::String8 description,
        ::Smp::ISimulator *simulator, ::Smp::Uuid uuid) {

    static_assert(std::is_base_of_v<::Smp::IComponent, T>, "T must inherit from ::Smp::IComponent");

    return Create(name, description, simulator, uuid, typeid(T),
            [](::Smp::String8 _name, ::Smp::String8 _description,
                    ::Smp::IObject *_parent, ::Smp::ISimulator *_simulator) {
                return std::make_unique < T
                        > (_name, _description, _parent, _simulator);
            });
}

} // namespace Xsmp::Factory
#endif // XSMP_FACTORY_H_
