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

#ifndef XSMP_PUBLICATION_TYPEREGISTRY_H_
#define XSMP_PUBLICATION_TYPEREGISTRY_H_

#include <Smp/IObject.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Uuid.h>
#include <memory>
#include <unordered_map>

namespace Smp::Publication {
class IArrayType;
class IClassType;
class IStructureType;
} // namespace Smp::Publication

namespace Xsmp::Publication {

class TypeRegistry final : public ::Smp::IObject,
                           public ::Smp::Publication::ITypeRegistry {
public:
  explicit TypeRegistry(::Smp::IObject *parent = nullptr);
  /// TypeRegistry cannot be copied
  TypeRegistry(const TypeRegistry &) = delete;
  /// TypeRegistry cannot be copied
  TypeRegistry &operator=(const TypeRegistry &) = delete;
  ~TypeRegistry() noexcept override = default;

  ::Smp::String8 GetName() const override;
  ::Smp::String8 GetDescription() const override;
  ::Smp::IObject *GetParent() const override;
  /// Returns a type by its primitive type kind.
  /// @remarks This method can be used to map primitive types to the
  ///          IType interface, to treat all types identically.
  /// @param   type Primitive type the type is requested for.
  /// @return  Interface to the requested type.
  ::Smp::Publication::IType *
  GetType(::Smp::PrimitiveTypeKind type) const override;

  /// Returns a type by universally unique identifier.
  /// @remarks This method can be used to find out whether a specific
  ///          type has been registered before.
  /// @param   typeUuid Universally unique identifier for the
  ///          requested type.
  /// @return  Interface of the requested type, or null if no type
  ///          with the registered Uuid could be found.
  ::Smp::Publication::IType *GetType(::Smp::Uuid typeUuid) const override;

  /// Add a float type to the registry.
  /// IComponent and IDynamicInvocation support fields, parameters
  /// and operations of Float types via the PTK_Float32 and
  /// PTK_Float64 primitive type, as a Float is mapped either to
  /// Float32 or Float64.
  /// @param   name Name of the type.
  /// @param   description Description of the type.
  /// @param   typeUuid Universally unique identifier of the type.
  /// @param   minimum Minimum value for float.
  /// @param   maximum Maximum value for float.
  /// @param   minInclusive Flag whether the minimum value for float
  ///          is valid or not.
  /// @param   maxInclusive Flag whether the maximum value for float
  ///          is valid or not.
  /// @param   unit Unit of the type.
  /// @param   type Primitive type to use for Float type. This has to
  ///          be either PTK_Float32 or PTK_Float64. For all other
  ///          types, an exception of type InvalidPrimitiveType is
  ///          raised.
  /// @return  Interface to new type.
  /// @throws  ::Smp::Publication::InvalidPrimitiveType
  /// @throws  ::Smp::Publication::TypeAlreadyRegistered
  ::Smp::Publication::IType *
  AddFloatType(::Smp::String8 name, ::Smp::String8 description,
               ::Smp::Uuid typeUuid, ::Smp::Float64 minimum,
               ::Smp::Float64 maximum, ::Smp::Bool minInclusive,
               ::Smp::Bool maxInclusive, ::Smp::String8 unit,
               ::Smp::PrimitiveTypeKind type =
                   ::Smp::PrimitiveTypeKind::PTK_Float64) override;

  /// Add an integer type to the registry.
  /// IComponent and IDynamicInvocation support fields, parameters
  /// and operations of Integer types via the PTK_Int primitive
  /// types, as an Integer is mapped to one of Int8 / Int16 / Int32 /
  /// Int64 / UInt8 / UInt16 / UInt32 / UInt64.
  /// @param   name Name of the type.
  /// @param   description Description of the type.
  /// @param   typeUuid Universally unique identifier of the type.
  /// @param   minimum Minimum value for integer.
  /// @param   maximum Maximum value for integer.
  /// @param   unit Unit of the type.
  /// @param   type Primitive type to use for Integer type. This has
  ///          to be one of the available signed or unsigned integer
  ///          types. For all other types, an exception of type
  ///          InvalidPrimitiveType is raised.
  /// @return  Interface to new type.
  /// @throws  ::Smp::Publication::InvalidPrimitiveType
  /// @throws  ::Smp::Publication::TypeAlreadyRegistered
  ::Smp::Publication::IType *
  AddIntegerType(::Smp::String8 name, ::Smp::String8 description,
                 ::Smp::Uuid typeUuid, ::Smp::Int64 minimum,
                 ::Smp::Int64 maximum, ::Smp::String8 unit,
                 ::Smp::PrimitiveTypeKind type =
                     ::Smp::PrimitiveTypeKind::PTK_Int32) override;

  /// Add an enumeration type to the registry.
  /// @param   name Name of the type.
  /// @param   description Description of the type.
  /// @param   typeUuid Universally unique identifier of the type.
  /// @param   memorySize Size of an instance of this enumeration in
  ///          bytes. Valid values are 1, 2, 4 and 8.
  /// @return  Interface to new type.
  /// @throws  ::Smp::Publication::TypeAlreadyRegistered
  ::Smp::Publication::IEnumerationType *
  AddEnumerationType(::Smp::String8 name, ::Smp::String8 description,
                     ::Smp::Uuid typeUuid, ::Smp::Int16 memorySize) override;

  /// Add an array type to the registry.
  /// @param   name Name of the type.
  /// @param   description Description of the type.
  /// @param   typeUuid Universally unique identifier of the type.
  /// @param   itemTypeUuid Universally unique identifier of the Type
  ///          of the array items.
  /// @param   itemSize Size of an array item in bytes. This needs to
  ///          take possible padding into account, as it may be used
  ///          by the simulation environment to calculate the memory
  ///          offset between array items.
  /// @param   arrayCount Number of elements in the array.
  /// @param   simpleArray True if array shall be implemented using
  ///          ISimpleArrayField, i.e. without representing array
  ///          items as fields.
  /// @return  Interface to new type.
  /// @throws  ::Smp::Publication::TypeAlreadyRegistered
  ::Smp::Publication::IArrayType *
  AddArrayType(::Smp::String8 name, ::Smp::String8 description,
               ::Smp::Uuid typeUuid, ::Smp::Uuid itemTypeUuid,
               ::Smp::Int64 itemSize, ::Smp::Int64 arrayCount,
               ::Smp::Bool simpleArray = false) override;

  /// Add a string type to the registry.
  /// @param   name Name of the type.
  /// @param   description Description of the type.
  /// @param   typeUuid Universally unique identifier of the type.
  /// @param   length Maximum length of the string.
  /// @return  Interface to new type.
  /// @throws  ::Smp::Publication::TypeAlreadyRegistered
  ::Smp::Publication::IType *AddStringType(::Smp::String8 name,
                                           ::Smp::String8 description,
                                           ::Smp::Uuid typeUuid,
                                           ::Smp::Int64 length) override;

  /// Add a structure type to the registry.
  /// @param   name Name of the type.
  /// @param   description Description of the type.
  /// @param   typeUuid Universally unique identifier of the type.
  /// @return  Interface to new type that allows adding fields.
  /// @throws  ::Smp::Publication::TypeAlreadyRegistered
  ::Smp::Publication::IStructureType *
  AddStructureType(::Smp::String8 name, ::Smp::String8 description,
                   ::Smp::Uuid typeUuid) override;

  /// Add a class type to the registry.
  /// @param   name Name of the type.
  /// @param   description Description of the type.
  /// @param   typeUuid Universally unique identifier of the type.
  /// @param   baseClassUuid Universally unique identifier of the
  ///          base class.
  /// Use Uuid_Void when the type has no base type.
  /// @return  Interface to new type that allows adding fields.
  /// @throws  ::Smp::Publication::TypeAlreadyRegistered
  ::Smp::Publication::IClassType *
  AddClassType(::Smp::String8 name, ::Smp::String8 description,
               ::Smp::Uuid typeUuid, ::Smp::Uuid baseClassUuid) override;

private:
  ::Smp::IObject *_parent;
  std::unordered_map<::Smp::Uuid, std::unique_ptr<::Smp::Publication::IType>>
      _types{};

  /// Add a type to the registry.
  /// @param   args type parameters.
  /// @return  the added type.
  /// @throws  ::Smp::Publication::TypeAlreadyRegistered
  template <typename T, class... Args> T *AddType(Args &&...args);
};

} // namespace Xsmp::Publication

#endif // XSMP_PUBLICATION_TYPEREGISTRY_H_
