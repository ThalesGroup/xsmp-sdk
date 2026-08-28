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
#include <Xsmp/cstring.h>
#include <algorithm>
#include <cstddef>
#include <map>
#include <string>
#include <utility>

namespace Xsmp::Publication {

bool Request::isValid(const ::Smp::Publication::IType *type,
                      const ::Smp::AnySimple &value) {
  // a mismatching kind is reported by the caller, which knows whether the
  // value belongs to a parameter, a return value or a property: SMP 2025 asks
  // for InvalidParameterValue and InvalidPropertyValue there, not for the
  // generic InvalidAnyType
  if (type->GetPrimitiveTypeKind() != value.type) {
    return false;
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
    if (stringValue &&
        std::char_traits<char>::length(stringValue) >
            static_cast<std::size_t>(stringType->GetMaxLength())) {
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
        init(name + "." + std::string(field.name.c_str()), fieldType,
             typeRegistry);
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

::Smp::String8 Request::GetName() const { return _operation->GetName(); }

::Smp::RequestType Request::GetType() const {
  return ::Smp::RequestType::RT_Invoke;
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
  if (!isValid(_values[static_cast<std::size_t>(index)].second, value)) {
    auto it = std::find_if(
        _indexes.begin(), _indexes.end(),
        [index](std::map<std::string, ::Smp::Int32>::const_reference entry) {
          return entry.second == index;
        });

    const auto *expected = _values[static_cast<std::size_t>(index)].second;
    ::Xsmp::Exception::throwInvalidParameterValue(
        _operation, it->first, value,
        expected ? expected->GetPrimitiveTypeKind()
                 : ::Smp::PrimitiveTypeKind::PTK_None);
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
  const auto *returnParameter = _operation->GetReturnParameter();
  if (!returnParameter) {
    ::Xsmp::Exception::throwVoidOperation(_operation);
  }
  const auto *type = returnParameter->GetType();
  if (!isValid(type, value)) {
    // SMP 2025 made the return value a parameter of direction PDK_Return
    ::Xsmp::Exception::throwInvalidParameterValue(
        _operation, returnParameter->GetName(), value,
        type ? type->GetPrimitiveTypeKind()
             : ::Smp::PrimitiveTypeKind::PTK_None);
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
