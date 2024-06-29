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

#include <Smp/AnySimple.h>
#include <Smp/IOperation.h>
#include <Smp/IParameter.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IArrayType.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Uuid.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Publication/Request.h>
#include <Xsmp/Publication/Type.h>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <map>
#include <string>
#include <utility>

namespace Xsmp::Publication {

bool Request::isValid(const ::Smp::IObject *sender,
                      const ::Smp::Publication::IType *type,
                      const ::Smp::AnySimple &value) {
  if (type->GetPrimitiveTypeKind() != value.type) {
    ::Xsmp::Exception::throwInvalidAnyType(sender, value.type,
                                           type->GetPrimitiveTypeKind());
  }
  // integer types
  if (const auto *integerType = dynamic_cast<const IntegerType *>(type)) {
    auto intValue = static_cast<::Smp::Int64>(value);
    if (intValue < integerType->getMinimum() ||
        intValue > integerType->getMaximum()) {
      return false;
    }
  }
  // float types
  else if (const auto *floatType = dynamic_cast<const FloatType *>(type)) {
    auto floatValue = static_cast<::Smp::Float64>(value);

    if ((floatType->IsMinInclusive() ? floatValue < floatType->getMinimum()
                                     : floatValue <= floatType->getMinimum()) ||
        (floatType->IsMaxInclusive() ? floatValue > floatType->getMaximum()
                                     : floatValue >= floatType->getMaximum())) {
      return false;
    }
  }
  // string types
  else if (const auto *stringType = dynamic_cast<const StringType *>(type)) {
    const auto *stringValue = static_cast<::Smp::String8>(value);
    if (stringValue && std::strlen(stringValue) >
                           static_cast<std::size_t>(stringType->GetLength())) {
      return false;
    }
  }
  // enumeration types
  else if (const auto *enumType = dynamic_cast<const EnumerationType *>(type)) {
    // check that a literal with the correct value exist
    if (enumType->GetLiterals().find(static_cast<::Smp::Int32>(value)) ==
        enumType->GetLiterals().end()) {
      return false;
    }
  }
  // other type
  else {
    // ignore
  }
  return true;
}

Request::Request(::Smp::IOperation *operation,
                 ::Smp::Publication::ITypeRegistry *typeRegistry)
    : _operation(operation) {

  for (auto const *parameter : *operation->GetParameters()) {
    init(parameter->GetName(), parameter->GetType(), typeRegistry);
  }
}

void Request::init(const std::string &name,
                   const ::Smp::Publication::IType *type,
                   ::Smp::Publication::ITypeRegistry *typeRegistry) {

  // init a simple type
  if (type->GetPrimitiveTypeKind() != ::Smp::PrimitiveTypeKind::PTK_None) {
    _values.emplace_back(::Smp::AnySimple{}, type);
    _indexes.try_emplace(name, static_cast<::Smp::Int32>(_indexes.size()));
  }
  // init an array type
  else if (const auto *array =
               dynamic_cast<const ::Smp::Publication::IArrayType *>(type)) {
    for (::Smp::UInt64 i = 0, size = array->GetSize(); i < size; ++i) {
      init(name + "[" + std::to_string(i) + "]", array->GetItemType(),
           typeRegistry);
    }
  }
  // init a structure type
  else if (const auto *structure = dynamic_cast<const StructureType *>(type)) {
    for (const auto &field : structure->GetFields()) {
      if (auto const *fieldType = typeRegistry->GetType(field.uuid)) {
        init(name + "." + std::string(field.name), fieldType, typeRegistry);
      } else {
        ::Xsmp::Exception::throwTypeNotRegistered(_operation, field.uuid);
      }
    }
  }
  // unsupported type
  else {
    ::Xsmp::Exception::throwInvalidAnyType(
        type, ::Smp::PrimitiveTypeKind::PTK_None, type->GetPrimitiveTypeKind());
  }
}

::Smp::String8 Request::GetOperationName() const {
  return _operation->GetName();
}

::Smp::Int32 Request::GetParameterCount() const {
  return static_cast<::Smp::Int32>(_values.size());
}

::Smp::Int32 Request::GetParameterIndex(::Smp::String8 parameterName) const {
  if (parameterName) {
    if (auto it = _indexes.find(parameterName); it != _indexes.end()) {
      return it->second;
    }
  }
  return -1;
}

void Request::SetParameterValue(::Smp::Int32 index, ::Smp::AnySimple value) {

  if (index < 0 || static_cast<std::size_t>(index) >= _values.size()) {
    ::Xsmp::Exception::throwInvalidParameterIndex(
        _operation, index, static_cast<::Smp::Int32>(_values.size()));
  }
  if (!isValid(_operation, _values[static_cast<std::size_t>(index)].second,
               value)) {
    auto it = std::find_if(
        _indexes.begin(), _indexes.end(),
        [index](std::map<std::string, ::Smp::Int32>::const_reference entry) {
          return entry.second == index;
        });

    ::Xsmp::Exception::throwInvalidParameterValue(_operation, it->first, value);
  }
  _values[static_cast<std::size_t>(index)].first = std::move(value);
}

::Smp::AnySimple Request::GetParameterValue(::Smp::Int32 index) const {
  if (index < 0 || static_cast<std::size_t>(index) >= _values.size()) {
    ::Xsmp::Exception::throwInvalidParameterIndex(
        _operation, index, static_cast<::Smp::Int32>(_values.size()));
  }
  return _values[static_cast<std::size_t>(index)].first;
}

void Request::SetReturnValue(::Smp::AnySimple value) {
  if (!_operation->GetReturnParameter()) {
    ::Xsmp::Exception::throwVoidOperation(_operation);
  }
  if (!isValid(_operation, _operation->GetReturnParameter()->GetType(),
               value)) {
    ::Xsmp::Exception::throwInvalidReturnValue(_operation, value);
  }
  _returnValue = std::move(value);
}

::Smp::AnySimple Request::GetReturnValue() const {
  if (!_operation->GetReturnParameter()) {
    ::Xsmp::Exception::throwVoidOperation(_operation);
  }
  return _returnValue;
}

} // namespace Xsmp::Publication
