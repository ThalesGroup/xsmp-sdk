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

#include <Smp/IPublication.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Uuid.h>
#include <Smp/ViewKind.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Publication/Type.h>
#include <Xsmp/Publication/TypeRegistry.h>
#include <map>
#include <vector>

namespace Xsmp::Publication {

Type::Type(::Smp::String8 name, ::Smp::String8 description,
           ::Xsmp::Publication::TypeRegistry *typeRegistry, ::Smp::Uuid uuid)
    : _name(::Xsmp::Helper::checkName(name, typeRegistry)),
      _description(description), _parent(typeRegistry), _uuid(uuid) {}
::Smp::String8 Type::GetName() const { return _name.c_str(); }

::Smp::String8 Type::GetDescription() const { return _description.c_str(); }

::Smp::IObject *Type::GetParent() const { return _parent; }
::Smp::PrimitiveTypeKind Type::GetPrimitiveTypeKind() const {
  return ::Smp::PrimitiveTypeKind::PTK_None;
}

::Smp::Uuid Type::GetUuid() const { return _uuid; }
::Xsmp::Publication::TypeRegistry *Type::GetTypeRegistry() const noexcept {
  return _parent;
}

void Type::Publish(::Smp::IPublication *receiver, ::Smp::String8 name,
                   ::Smp::String8 description, void *address,
                   ::Smp::ViewKind view, ::Smp::Bool state, ::Smp::Bool input,
                   ::Smp::Bool output) {
  receiver->PublishField(name, description, address, _uuid, view, state, input,
                         output);
}

ArrayType::ArrayType(::Smp::String8 name, ::Smp::String8 description,
                     ::Xsmp::Publication::TypeRegistry *typeRegistry,
                     ::Smp::Uuid typeUuid, ::Smp::Uuid itemTypeUuid,
                     ::Smp::Int64 itemSize, ::Smp::Int64 arrayCount,
                     ::Smp::Bool simpleArray)
    : Type(name, description, typeRegistry, typeUuid),
      _itemTypeUuid(itemTypeUuid),
      _itemSize(itemSize >= 0 ? static_cast<::Smp::UInt64>(itemSize) : 0),
      _arrayCount(arrayCount >= 0 ? static_cast<::Smp::UInt64>(arrayCount) : 0),
      _simpleArray(simpleArray) {}

::Smp::UInt64 ArrayType::GetSize() const { return _arrayCount; }

const ::Smp::Publication::IType *ArrayType::GetItemType() const {
  return GetTypeRegistry()->GetType(_itemTypeUuid);
}

::Smp::UInt64 ArrayType::GetItemSize() const { return _itemSize; }

::Smp::Bool ArrayType::IsSimpleArray() const { return _simpleArray; }

SimpleType::SimpleType(::Smp::String8 name, ::Smp::String8 description,
                       ::Xsmp::Publication::TypeRegistry *parent,
                       ::Smp::Uuid uuid, ::Smp::PrimitiveTypeKind kind)
    : Type(name, description, parent, uuid), _kind(kind) {}

::Smp::PrimitiveTypeKind SimpleType::GetPrimitiveTypeKind() const {
  return _kind;
}

namespace {
::Smp::PrimitiveTypeKind kind(::Smp::String8 name, ::Smp::Int16 memorySize) {
  switch (memorySize) {
  case sizeof(::Smp::Int8):
    return ::Smp::PrimitiveTypeKind::PTK_Int8;
  case sizeof(::Smp::Int16):
    return ::Smp::PrimitiveTypeKind::PTK_Int16;
  case sizeof(::Smp::Int32):
    return ::Smp::PrimitiveTypeKind::PTK_Int32;
  case sizeof(::Smp::Int64):
    return ::Smp::PrimitiveTypeKind::PTK_Int64;
  default:
    ::Xsmp::Exception::throwInvalidPrimitiveType(
        nullptr, name, ::Smp::PrimitiveTypeKind::PTK_None);
  }
}
} // namespace
EnumerationType::EnumerationType(::Smp::String8 name,
                                 ::Smp::String8 description,
                                 ::Xsmp::Publication::TypeRegistry *parent,
                                 ::Smp::Uuid typeUuid, ::Smp::Int16 memorySize)
    : SimpleType(name, description, parent, typeUuid, kind(name, memorySize)) {}

void EnumerationType::AddLiteral(::Smp::String8 name,
                                 ::Smp::String8 description,
                                 ::Smp::Int32 value) {

  if (auto it = _literals.find(value); it != _literals.end()) {
    ::Xsmp::Exception::throwDuplicateLiteral(this, it->second.name.c_str(), value);
  }
  _literals.try_emplace(value, Literal{name, description});
}

const std::map<::Smp::Int32, EnumerationType::Literal> &
EnumerationType::GetLiterals() const noexcept {
  return _literals;
}

FloatType::FloatType(::Smp::String8 name, ::Smp::String8 description,
                     ::Xsmp::Publication::TypeRegistry *parent,
                     ::Smp::Uuid typeUuid, ::Smp::Float64 minimum,
                     ::Smp::Float64 maximum, ::Smp::Bool minInclusive,
                     ::Smp::Bool maxInclusive, ::Smp::String8 unit,
                     ::Smp::PrimitiveTypeKind type)
    : SimpleType(name, description, parent, typeUuid, type), _unit(unit),
      _minimum(minimum), _maximum(maximum), _minInclusive(minInclusive),
      _maxInclusive(maxInclusive) {}

::Smp::Float64 FloatType::getMinimum() const { return _minimum; }

::Smp::Float64 FloatType::getMaximum() const { return _maximum; }

::Smp::String8 FloatType::getUnit() const { return _unit.c_str(); }

::Smp::Bool FloatType::IsMinInclusive() const { return _minInclusive; }

::Smp::Bool FloatType::IsMaxInclusive() const { return _maxInclusive; }

IntegerType::IntegerType(::Smp::String8 name, ::Smp::String8 description,
                         ::Xsmp::Publication::TypeRegistry *parent,
                         ::Smp::Uuid typeUuid, ::Smp::Int64 minimum,
                         ::Smp::Int64 maximum, ::Smp::String8 unit,
                         ::Smp::PrimitiveTypeKind type)
    : SimpleType(name, description, parent, typeUuid, type), _minimum(minimum),
      _maximum(maximum), _unit(unit) {}

::Smp::Int64 IntegerType::getMinimum() const { return _minimum; }

::Smp::Int64 IntegerType::getMaximum() const { return _maximum; }

::Smp::String8 IntegerType::getUnit() const { return _unit.c_str(); }

PrimitiveType::PrimitiveType(::Smp::String8 name, ::Smp::String8 description,
                             ::Xsmp::Publication::TypeRegistry *parent,
                             ::Smp::Uuid uuid, ::Smp::PrimitiveTypeKind kind)
    : SimpleType(name, description, parent, uuid, kind) {}

StringType::StringType(::Smp::String8 name, ::Smp::String8 description,
                       ::Xsmp::Publication::TypeRegistry *parent,
                       ::Smp::Uuid typeUuid, ::Smp::Int64 length)
    : SimpleType(name, description, parent, typeUuid,
                 ::Smp::PrimitiveTypeKind::PTK_String8),
      _length(length) {}

::Smp::Int64 StringType::GetLength() const { return _length; }

StructureType::StructureType(::Smp::String8 name, ::Smp::String8 description,
                             ::Xsmp::Publication::TypeRegistry *typeRegistry,
                             ::Smp::Uuid typeUuid)
    : Type(name, description, typeRegistry, typeUuid) {}

void StructureType::AddField(::Smp::String8 name, ::Smp::String8 description,
                             ::Smp::Uuid uuid, ::Smp::Int64 offset,
                             ::Smp::ViewKind view, ::Smp::Bool state,
                             ::Smp::Bool input, ::Smp::Bool output) {

  _fields.push_back(
      {name, description, uuid, offset, view, state, input, output});
}

const std::vector<StructureType::Field> &StructureType::GetFields() const {
  return _fields;
}

ClassType::ClassType(::Smp::String8 name, ::Smp::String8 description,
                     ::Xsmp::Publication::TypeRegistry *typeRegistry,
                     ::Smp::Uuid typeUuid, ::Smp::Uuid baseClassUuid)
    : StructureType(name, description, typeRegistry, typeUuid),
      _baseClassUuid{baseClassUuid} {
  if (baseClassUuid == ::Smp::Uuids::Uuid_Void) {
    return;
  }
  if (baseClassUuid == typeUuid) {
    ::Xsmp::Exception::throwException(
        this, "Invalid base Class",
        "The base class must be different from the current class.");
  }
  auto *baseClass = typeRegistry->GetType(baseClassUuid);
  if (!baseClass) {
    ::Xsmp::Exception::throwTypeNotRegistered(this, baseClassUuid);
  }
  if (!dynamic_cast<::Smp::Publication::IClassType *>(baseClass)) {
    ::Xsmp::Exception::throwException(
        this, "Invalid base Class",
        "A base class must be void or a class type.");
  }
}

} // namespace Xsmp::Publication
