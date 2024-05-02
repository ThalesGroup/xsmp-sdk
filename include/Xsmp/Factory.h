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
#include <memory>
#include <string>
#include <type_traits>

namespace Smp {
class ISimulator;
} // namespace Smp

namespace Xsmp {

class Factory final : public ::Smp::IFactory {
public:
  using _factory_instantiator_t =
      std::function<std::unique_ptr<::Smp::IComponent>(
          ::Smp::String8 name, ::Smp::String8 description,
          ::Smp::IComposite *parent, ::Smp::ISimulator *simulator)>;

  [[nodiscard]] static ::Smp::IFactory *
  Create(::Smp::String8 name, ::Smp::String8 description,
         ::Smp::ISimulator *simulator, ::Smp::Uuid uuid,
         const std::type_info &type, _factory_instantiator_t &&callback);

  template <typename T>
  [[nodiscard]] static ::Smp::IFactory *
  Create(::Smp::String8 name, ::Smp::String8 description,
         ::Smp::ISimulator *simulator, ::Smp::Uuid uuid) {

    static_assert(std::is_base_of_v<::Smp::IComponent, T>,
                  "T must inherit from ::Smp::IComponent");

    return Create(
        name, description, simulator, uuid, typeid(T),
        [](::Smp::String8 _name, ::Smp::String8 _description,
           ::Smp::IComposite *_parent, ::Smp::ISimulator *_simulator) {
          return std::make_unique<T>(_name, _description, _parent, _simulator);
        });
  }

  ~Factory() noexcept override = default;
  Factory(const Factory &) = delete;
  Factory &operator=(const Factory &) = delete;
  ::Smp::String8 GetName() const override;
  ::Smp::String8 GetDescription() const override;
  ::Smp::IObject *GetParent() const override;
  ::Smp::Uuid GetUuid() const override;

  ::Smp::IComponent *CreateInstance(::Smp::String8 name,
                                    ::Smp::String8 description,
                                    ::Smp::IComposite *parent) override;
  void DeleteInstance(::Smp::IComponent *instance) override;
  ::Smp::String8 GetTypeName() const override;

private:
  Factory(::Smp::String8 name, ::Smp::String8 description,
          ::Smp::ISimulator *simulator, ::Smp::Uuid uuid,
          const std::type_info &type, _factory_instantiator_t &&callback);
  std::string _name;
  std::string _description;
  ::Smp::ISimulator *_parent;
  ::Smp::Uuid _uuid;
  _factory_instantiator_t _callback;
  std::string _typeName;
};

} // namespace Xsmp
#endif // XSMP_FACTORY_H_
