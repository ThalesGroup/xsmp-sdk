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
#include <Xsmp/cstring.h>
#include <functional>
#include <memory>
#include <type_traits>

namespace Smp {
class ISimulator;
} // namespace Smp

/// XSMP standard types and interfaces.
namespace Xsmp {

/// @class Factory
/// XSMP implementation of ::Smp::IFactory.
class Factory final : public ::Smp::IFactory {
public:
  using _factory_instantiator_t =
      std::function<std::unique_ptr<::Smp::IComponent>(
          ::Smp::String8 name, ::Smp::String8 description,
          ::Smp::IComposite *parent, ::Smp::ISimulator *simulator)>;

  /// Create a factory for a component.
  /// @param name The name of the factory.
  /// @param description The description of the factory.
  /// @param simulator The simulator in which components will be created.
  /// @param uuid The universally unique identifier of the type instantiated by
  /// the factory.
  /// @param type The fully qualified C++ name of the type.
  /// @param callback A function object that creates an instance of a specific
  /// component type.
  /// @return The newly created Factory object.
  [[nodiscard]] static ::Smp::IFactory *
  Create(::Smp::String8 name, ::Smp::String8 description,
         ::Smp::ISimulator *simulator, ::Smp::Uuid uuid,
         const std::type_info &type, _factory_instantiator_t &&callback);

  /// Create a factory for a component.
  /// @param name The name of the factory.
  /// @param description The description of the factory.
  /// @param simulator The simulator in which components will be created.
  /// @param uuid The universally unique identifier of the type instantiated by
  /// the factory.
  /// @return The newly created Factory object.
  template <typename T>
  [[nodiscard]] static ::Smp::IFactory *
  Create(::Smp::String8 name, ::Smp::String8 description,
         ::Smp::ISimulator *simulator, ::Smp::Uuid uuid) {

    static_assert(std::is_base_of_v<::Smp::IComponent, T>,
                  "T must inherit from ::Smp::IComponent");

    return Create(
        name, description, simulator, uuid, typeid(T),
        [](::Smp::String8 name_, ::Smp::String8 description_,
           ::Smp::IComposite *parent_, ::Smp::ISimulator *simulator_) {
          return std::make_unique<T>(name_, description_, parent_, simulator_);
        });
  }

  /// Virtual destructor to release memory.
  ~Factory() noexcept override = default;

  /// Get the name of the factory
  /// @return  the name of the factory.
  ::Smp::String8 GetName() const override;

  /// Get the description of the factory
  /// @return  the description of the factory.
  ::Smp::String8 GetDescription() const override;

  /// Get the parent of the factory
  /// @return  the parent of the factory.
  ::Smp::IObject *GetParent() const override;

  /// Get Universally unique identifier of the type instantiated by the
  /// factory.
  /// @return  Universally unique identifier of component.
  ::Smp::Uuid GetUuid() const override;

  /// Create a new instance with given name, description and parent.
  /// @param   name Name of the new instance.
  /// f the name provided is not a valid object name, an exception of
  /// type InvalidObjectName is raised.
  /// @param   description Description of the new instance.
  /// @param   parent Parent object of the new instance.
  /// @return  New component instance.
  /// @throws  Smp::InvalidObjectName
  ::Smp::IComponent *CreateInstance(::Smp::String8 name,
                                    ::Smp::String8 description,
                                    ::Smp::IComposite *parent) override;

  /// Delete an existing instance.
  /// @param   instance Instance to delete.
  void DeleteInstance(::Smp::IComponent *instance) override;

  /// Returns the fully qualified C++ name of the type.
  /// @return  Fully qualified C++ name of type that is created by this
  ///          factory.
  ::Smp::String8 GetTypeName() const override;

private:
  Factory(::Smp::String8 name, ::Smp::String8 description,
          ::Smp::ISimulator *simulator, ::Smp::Uuid uuid,
          const std::type_info &type, _factory_instantiator_t &&callback);
  ::Xsmp::cstring _name;
  ::Xsmp::cstring _description;
  ::Smp::ISimulator *_simulator;
  ::Smp::Uuid _uuid;
  _factory_instantiator_t _callback;
  ::Xsmp::cstring _typeName;
};

} // namespace Xsmp
#endif // XSMP_FACTORY_H_
