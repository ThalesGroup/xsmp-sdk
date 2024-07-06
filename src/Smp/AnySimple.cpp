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
#include <Smp/InvalidAnyType.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/utility.h>
#include <Xsmp/Exception.h>
#include <cmath>
#include <limits>
#include <string>

namespace Smp {

namespace {
inline void _releaseContent(::Smp::AnySimple *any) {
  if (any->type == ::Smp::PrimitiveTypeKind::PTK_String8) {
    delete[] any->value.string8Value;
    any->value.string8Value = nullptr;
  }
}
} // namespace

AnySimple::AnySimple() : AnySimple(PrimitiveTypeKind::PTK_None) {}

AnySimple::AnySimple(::Smp::PrimitiveTypeKind kind) : type{kind}, value{} {
  value.string8Value = nullptr;
}

AnySimple::AnySimple(const ::Smp::AnySimple &other) : AnySimple() {
  *this = other;
}

AnySimple::AnySimple(::Smp::AnySimple &&other)
    : type{other.type}, value{other.value} {
  other.type = PrimitiveTypeKind::PTK_None;
  other.value.string8Value = nullptr;
}

AnySimple &AnySimple::operator=(const ::Smp::AnySimple &other) {

  if (this != &other) {
    _releaseContent(this);
    type = other.type;

    if (other.type == PrimitiveTypeKind::PTK_String8) {
      if (other.value.string8Value) {
        auto size =
            std::char_traits<char>::length(other.value.string8Value) + 1;
        auto *dest = new ::Smp::Char8[size];
        this->value.string8Value = dest;
        std::char_traits<char>::copy(dest, other.value.string8Value, size);
      } else {
        value.string8Value = nullptr;
      }
    } else {
      value = other.value;
    }
  }

  return *this;
}

AnySimple &AnySimple::operator=(::Smp::AnySimple &&other) {

  _releaseContent(this);

  type = other.type;
  value = other.value;

  other.type = PrimitiveTypeKind::PTK_None;
  other.value.string8Value = nullptr;

  return *this;
}

AnySimple::~AnySimple() noexcept { _releaseContent(this); }

template <typename T, typename U>
T convertIntegral(U newValue, ::Smp::PrimitiveTypeKind kind,
                  ::Smp::PrimitiveTypeKind expected) {
  if (!utility::in_range<T>(newValue)) {
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, expected, kind);
  }
  return static_cast<T>(newValue);
}

template <typename T, typename U>
T convertFloat(U newValue, ::Smp::PrimitiveTypeKind kind,
               ::Smp::PrimitiveTypeKind expected) {

  if (std::fabs(newValue - static_cast<U>(static_cast<T>(newValue))) >
      std::numeric_limits<U>::epsilon()) {
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, expected, kind);
  }
  return static_cast<T>(newValue);
}

void AnySimple::SetValue(::Smp::PrimitiveTypeKind kind, ::Smp::Bool newValue) {

  _releaseContent(this);

  if (kind == PrimitiveTypeKind::PTK_Bool) {
    this->value.boolValue = newValue;
    type = kind;
  } else {
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, kind,
                                           PrimitiveTypeKind::PTK_Bool);
  }
}

void AnySimple::SetValue(::Smp::PrimitiveTypeKind kind, ::Smp::Char8 newValue) {

  _releaseContent(this);

  if (kind == PrimitiveTypeKind::PTK_Char8) {
    this->value.char8Value = newValue;
    type = kind;
  } else {
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, kind,
                                           PrimitiveTypeKind::PTK_Char8);
  }
}

void AnySimple::SetValue(::Smp::PrimitiveTypeKind kind,
                         ::Smp::String8 newValue) {

  _releaseContent(this);

  if (kind == PrimitiveTypeKind::PTK_String8) {
    if (newValue) {
      auto size = std::char_traits<char>::length(newValue) + 1;
      auto *dest = new ::Smp::Char8[size];
      this->value.string8Value = dest;
      std::char_traits<char>::copy(dest, newValue, size);
    } else {
      this->value.string8Value = nullptr;
    }
    type = kind;
  } else {

    ::Xsmp::Exception::throwInvalidAnyType(nullptr, kind,
                                           PrimitiveTypeKind::PTK_String8);
  }
}

void AnySimple::SetValue(::Smp::PrimitiveTypeKind kind, ::Smp::UInt8 newValue) {

  _releaseContent(this);

  switch (kind) {
  case PrimitiveTypeKind::PTK_UInt8:
    this->value.uInt8Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_UInt16:
    this->value.uInt16Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_UInt32:
    this->value.uInt32Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_UInt64:
    this->value.uInt64Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Int8:
    this->value.int8Value = convertIntegral<::Smp::Int8>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt8);
    break;
  case PrimitiveTypeKind::PTK_Int16:
    this->value.int16Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Int32:
    this->value.int32Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Int64:
    this->value.int64Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Duration:
    this->value.durationValue = newValue;
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    this->value.dateTimeValue = newValue;
    break;
  case PrimitiveTypeKind::PTK_Float32:
    this->value.float32Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Float64:
    this->value.float64Value = newValue;
    break;
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, kind,
                                           PrimitiveTypeKind::PTK_UInt8);
  }

  type = kind;
}

void AnySimple::SetValue(::Smp::PrimitiveTypeKind kind,
                         ::Smp::UInt16 newValue) {

  _releaseContent(this);

  switch (kind) {
  case PrimitiveTypeKind::PTK_UInt8:
    this->value.uInt8Value = convertIntegral<::Smp::UInt8>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt16);
    break;
  case PrimitiveTypeKind::PTK_UInt16:
    this->value.uInt16Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_UInt32:
    this->value.uInt32Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_UInt64:
    this->value.uInt64Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Int8:
    this->value.int8Value = convertIntegral<::Smp::Int8>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt16);
    break;
  case PrimitiveTypeKind::PTK_Int16:
    this->value.int16Value = convertIntegral<::Smp::Int16>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt16);
    break;
  case PrimitiveTypeKind::PTK_Int32:
    this->value.int32Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Int64:
    this->value.int64Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Duration:
    this->value.durationValue = newValue;
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    this->value.dateTimeValue = newValue;
    break;
  case PrimitiveTypeKind::PTK_Float32:
    this->value.float32Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Float64:
    this->value.float64Value = newValue;
    break;
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, kind,
                                           PrimitiveTypeKind::PTK_UInt16);
  }

  type = kind;
}

void AnySimple::SetValue(::Smp::PrimitiveTypeKind kind,
                         ::Smp::UInt32 newValue) {

  _releaseContent(this);

  switch (kind) {
  case PrimitiveTypeKind::PTK_UInt8:
    this->value.uInt8Value = convertIntegral<::Smp::UInt8>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt32);
    break;
  case PrimitiveTypeKind::PTK_UInt16:
    this->value.uInt16Value = convertIntegral<::Smp::UInt16>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt32);
    break;
  case PrimitiveTypeKind::PTK_UInt32:
    this->value.uInt32Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_UInt64:
    this->value.uInt64Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Int8:
    this->value.int8Value = convertIntegral<::Smp::Int8>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt32);
    break;
  case PrimitiveTypeKind::PTK_Int16:
    this->value.int16Value = convertIntegral<::Smp::Int16>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt32);
    break;
  case PrimitiveTypeKind::PTK_Int32:
    this->value.int32Value = convertIntegral<::Smp::Int32>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt32);
    break;
  case PrimitiveTypeKind::PTK_Int64:
    this->value.int64Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Duration:
    this->value.durationValue = newValue;
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    this->value.dateTimeValue = newValue;
    break;
  case PrimitiveTypeKind::PTK_Float32:
    this->value.float32Value = convertFloat<::Smp::Float32>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt32);
    break;
  case PrimitiveTypeKind::PTK_Float64:
    this->value.float64Value = newValue;
    break;
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, kind,
                                           PrimitiveTypeKind::PTK_UInt32);
  }

  type = kind;
}

void AnySimple::SetValue(::Smp::PrimitiveTypeKind kind,
                         ::Smp::UInt64 newValue) {

  _releaseContent(this);

  switch (kind) {
  case PrimitiveTypeKind::PTK_UInt8:
    this->value.uInt8Value = convertIntegral<::Smp::UInt8>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt64);
    break;
  case PrimitiveTypeKind::PTK_UInt16:
    this->value.uInt16Value = convertIntegral<::Smp::UInt16>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt64);
    break;
  case PrimitiveTypeKind::PTK_UInt32:
    this->value.uInt32Value = convertIntegral<::Smp::UInt32>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt64);
    break;
  case PrimitiveTypeKind::PTK_UInt64:
    this->value.uInt64Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Int8:
    this->value.int8Value = convertIntegral<::Smp::Int8>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt64);
    break;
  case PrimitiveTypeKind::PTK_Int16:
    this->value.int16Value = convertIntegral<::Smp::Int16>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt64);
    break;
  case PrimitiveTypeKind::PTK_Int32:
    this->value.int32Value = convertIntegral<::Smp::Int32>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt64);
    break;
  case PrimitiveTypeKind::PTK_Int64:
    this->value.int64Value = convertIntegral<::Smp::Int64>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt64);
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    this->value.dateTimeValue = convertIntegral<::Smp::DateTime>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt64);
    break;
  case PrimitiveTypeKind::PTK_Duration:
    this->value.durationValue = convertIntegral<::Smp::Duration>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt64);
    break;
  case PrimitiveTypeKind::PTK_Float32:
    this->value.float32Value = convertFloat<::Smp::Float32>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt64);
    break;
  case PrimitiveTypeKind::PTK_Float64:
    this->value.float64Value = convertFloat<::Smp::Float64>(
        newValue, kind, PrimitiveTypeKind::PTK_UInt64);
    break;
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, kind,
                                           PrimitiveTypeKind::PTK_UInt64);
  }

  type = kind;
}

void AnySimple::SetValue(::Smp::PrimitiveTypeKind kind, ::Smp::Int8 newValue) {

  _releaseContent(this);

  switch (kind) {
  case PrimitiveTypeKind::PTK_UInt8:
    this->value.uInt8Value = convertIntegral<::Smp::UInt8>(
        newValue, kind, PrimitiveTypeKind::PTK_Int8);
    break;
  case PrimitiveTypeKind::PTK_UInt16:
    this->value.uInt16Value = convertIntegral<::Smp::UInt16>(
        newValue, kind, PrimitiveTypeKind::PTK_Int8);
    break;
  case PrimitiveTypeKind::PTK_UInt32:
    this->value.uInt32Value = convertIntegral<::Smp::UInt32>(
        newValue, kind, PrimitiveTypeKind::PTK_Int8);
    break;
  case PrimitiveTypeKind::PTK_UInt64:
    this->value.uInt64Value = convertIntegral<::Smp::UInt64>(
        newValue, kind, PrimitiveTypeKind::PTK_Int8);
    break;
  case PrimitiveTypeKind::PTK_Int8:
    this->value.int8Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Int16:
    this->value.int16Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Int32:
    this->value.int32Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Int64:
    this->value.int64Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Duration:
    this->value.durationValue = newValue;
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    this->value.dateTimeValue = newValue;
    break;
  case PrimitiveTypeKind::PTK_Float32:
    this->value.float32Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Float64:
    this->value.float64Value = newValue;
    break;
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, kind,
                                           PrimitiveTypeKind::PTK_Int8);
  }

  type = kind;
}

void AnySimple::SetValue(::Smp::PrimitiveTypeKind kind, ::Smp::Int16 newValue) {

  _releaseContent(this);

  switch (kind) {
  case PrimitiveTypeKind::PTK_UInt8:
    this->value.uInt8Value = convertIntegral<::Smp::UInt8>(
        newValue, kind, PrimitiveTypeKind::PTK_Int16);
    break;
  case PrimitiveTypeKind::PTK_UInt16:
    this->value.uInt16Value = convertIntegral<::Smp::UInt16>(
        newValue, kind, PrimitiveTypeKind::PTK_Int16);
    break;
  case PrimitiveTypeKind::PTK_UInt32:
    this->value.uInt32Value = convertIntegral<::Smp::UInt32>(
        newValue, kind, PrimitiveTypeKind::PTK_Int16);
    break;
  case PrimitiveTypeKind::PTK_UInt64:
    this->value.uInt64Value = convertIntegral<::Smp::UInt64>(
        newValue, kind, PrimitiveTypeKind::PTK_Int16);
    break;
  case PrimitiveTypeKind::PTK_Int8:
    this->value.int8Value = convertIntegral<::Smp::Int8>(
        newValue, kind, PrimitiveTypeKind::PTK_Int16);
    break;
  case PrimitiveTypeKind::PTK_Int16:
    this->value.int16Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Int32:
    this->value.int32Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Int64:
    this->value.int64Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Duration:
    this->value.durationValue = newValue;
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    this->value.dateTimeValue = newValue;
    break;
  case PrimitiveTypeKind::PTK_Float32:
    this->value.float32Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Float64:
    this->value.float64Value = newValue;
    break;
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, kind,
                                           PrimitiveTypeKind::PTK_Int16);
  }

  type = kind;
}

void AnySimple::SetValue(::Smp::PrimitiveTypeKind kind, ::Smp::Int32 newValue) {

  _releaseContent(this);

  switch (kind) {
  case PrimitiveTypeKind::PTK_UInt8:
    this->value.uInt8Value = convertIntegral<::Smp::UInt8>(
        newValue, kind, PrimitiveTypeKind::PTK_Int32);
    break;
  case PrimitiveTypeKind::PTK_UInt16:
    this->value.uInt16Value = convertIntegral<::Smp::UInt16>(
        newValue, kind, PrimitiveTypeKind::PTK_Int32);
    break;
  case PrimitiveTypeKind::PTK_UInt32:
    this->value.uInt32Value = convertIntegral<::Smp::UInt32>(
        newValue, kind, PrimitiveTypeKind::PTK_Int32);
    break;
  case PrimitiveTypeKind::PTK_UInt64:
    this->value.uInt64Value = convertIntegral<::Smp::UInt64>(
        newValue, kind, PrimitiveTypeKind::PTK_Int32);
    break;
  case PrimitiveTypeKind::PTK_Int8:
    this->value.int8Value = convertIntegral<::Smp::Int8>(
        newValue, kind, PrimitiveTypeKind::PTK_Int32);
    break;
  case PrimitiveTypeKind::PTK_Int16:
    this->value.int16Value = convertIntegral<::Smp::Int16>(
        newValue, kind, PrimitiveTypeKind::PTK_Int32);
    break;
  case PrimitiveTypeKind::PTK_Int32:
    this->value.int32Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Int64:
    this->value.int64Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Duration:
    this->value.durationValue = newValue;
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    this->value.dateTimeValue = newValue;
    break;
  case PrimitiveTypeKind::PTK_Float32:
    this->value.float32Value = convertFloat<::Smp::Float32>(
        newValue, kind, PrimitiveTypeKind::PTK_Int32);
    break;
  case PrimitiveTypeKind::PTK_Float64:
    this->value.float64Value = newValue;
    break;
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, kind,
                                           PrimitiveTypeKind::PTK_Int32);
  }

  type = kind;
}

void AnySimple::SetValue(::Smp::PrimitiveTypeKind kind, ::Smp::Int64 newValue) {

  _releaseContent(this);

  switch (kind) {
  case PrimitiveTypeKind::PTK_UInt8:
    this->value.uInt8Value = convertIntegral<::Smp::UInt8>(
        newValue, kind, PrimitiveTypeKind::PTK_Int64);
    break;
  case PrimitiveTypeKind::PTK_UInt16:
    this->value.uInt16Value = convertIntegral<::Smp::UInt16>(
        newValue, kind, PrimitiveTypeKind::PTK_Int64);
    break;
  case PrimitiveTypeKind::PTK_UInt32:
    this->value.uInt32Value = convertIntegral<::Smp::UInt32>(
        newValue, kind, PrimitiveTypeKind::PTK_Int64);
    break;
  case PrimitiveTypeKind::PTK_UInt64:
    this->value.uInt64Value = convertIntegral<::Smp::UInt64>(
        newValue, kind, PrimitiveTypeKind::PTK_Int64);
    break;
  case PrimitiveTypeKind::PTK_Int8:
    this->value.int8Value = convertIntegral<::Smp::Int8>(
        newValue, kind, PrimitiveTypeKind::PTK_Int64);
    break;
  case PrimitiveTypeKind::PTK_Int16:
    this->value.int16Value = convertIntegral<::Smp::Int16>(
        newValue, kind, PrimitiveTypeKind::PTK_Int64);
    break;
  case PrimitiveTypeKind::PTK_Int32:
    this->value.int32Value = convertIntegral<::Smp::Int32>(
        newValue, kind, PrimitiveTypeKind::PTK_Int64);
    break;
  case PrimitiveTypeKind::PTK_Int64:
    this->value.int64Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Duration:
    this->value.durationValue = newValue;
    break;
  case PrimitiveTypeKind::PTK_DateTime:
    this->value.dateTimeValue = newValue;
    break;
  case PrimitiveTypeKind::PTK_Float32:
    this->value.float32Value = convertFloat<::Smp::Float32>(
        newValue, kind, PrimitiveTypeKind::PTK_Int64);
    break;
  case PrimitiveTypeKind::PTK_Float64:
    this->value.float64Value = convertFloat<::Smp::Float64>(
        newValue, kind, PrimitiveTypeKind::PTK_Int64);
    break;
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, kind,
                                           PrimitiveTypeKind::PTK_Int64);
  }

  type = kind;
}

void AnySimple::SetValue(::Smp::PrimitiveTypeKind kind,
                         ::Smp::Float32 newValue) {

  _releaseContent(this);

  switch (kind) {
  case PrimitiveTypeKind::PTK_Float32:
    this->value.float32Value = newValue;
    break;
  case PrimitiveTypeKind::PTK_Float64:
    this->value.float64Value = static_cast<::Smp::Float64>(newValue);
    break;
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, kind,
                                           PrimitiveTypeKind::PTK_Float32);
  }

  type = kind;
}

void AnySimple::SetValue(::Smp::PrimitiveTypeKind kind,
                         ::Smp::Float64 newValue) {

  _releaseContent(this);

  switch (kind) {
  case PrimitiveTypeKind::PTK_Float32:
    this->value.float32Value = convertFloat<::Smp::Float32>(
        newValue, kind, PrimitiveTypeKind::PTK_Float64);
    break;
  case PrimitiveTypeKind::PTK_Float64:
    this->value.float64Value = newValue;
    break;
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, kind,
                                           PrimitiveTypeKind::PTK_Float64);
  }

  type = kind;
}

AnySimple::operator ::Smp::Bool() const {

  switch (type) {
  case PrimitiveTypeKind::PTK_Bool:
    return this->value.boolValue;
  case PrimitiveTypeKind::PTK_UInt8:
    return this->value.uInt8Value;
  case PrimitiveTypeKind::PTK_UInt16:
    return this->value.uInt16Value;
  case PrimitiveTypeKind::PTK_UInt32:
    return this->value.uInt32Value;
  case PrimitiveTypeKind::PTK_UInt64:
    return this->value.uInt64Value;
  case PrimitiveTypeKind::PTK_Int8:
    return this->value.int8Value;
  case PrimitiveTypeKind::PTK_Int16:
    return this->value.int16Value;
  case PrimitiveTypeKind::PTK_Int32:
    return this->value.int32Value;
  case PrimitiveTypeKind::PTK_Int64:
    return this->value.int64Value;
  case PrimitiveTypeKind::PTK_Duration:
    return this->value.durationValue;
  case PrimitiveTypeKind::PTK_DateTime:
    return this->value.dateTimeValue;
  case PrimitiveTypeKind::PTK_Char8:
    return this->value.char8Value;
  case PrimitiveTypeKind::PTK_String8:
    return this->value.string8Value;
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, type,
                                           PrimitiveTypeKind::PTK_Bool);
  }
}

AnySimple::operator ::Smp::Char8() const {
  if (type != PrimitiveTypeKind::PTK_Char8) {
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, type,
                                           PrimitiveTypeKind::PTK_Char8);
  }
  return this->value.char8Value;
}

AnySimple::operator ::Smp::String8() const {
  if (type != PrimitiveTypeKind::PTK_String8) {
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, type,
                                           PrimitiveTypeKind::PTK_String8);
  }
  return this->value.string8Value;
}

AnySimple::operator ::Smp::UInt8() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_Int8:
    return convertIntegral<::Smp::UInt8>(value.int8Value, type,
                                         PrimitiveTypeKind::PTK_UInt8);
  case PrimitiveTypeKind::PTK_Int16:
    return convertIntegral<::Smp::UInt8>(value.int16Value, type,
                                         PrimitiveTypeKind::PTK_UInt8);
  case PrimitiveTypeKind::PTK_Int32:
    return convertIntegral<::Smp::UInt8>(value.int32Value, type,
                                         PrimitiveTypeKind::PTK_UInt8);
  case PrimitiveTypeKind::PTK_Int64:
    return convertIntegral<::Smp::UInt8>(value.int64Value, type,
                                         PrimitiveTypeKind::PTK_UInt8);
  case PrimitiveTypeKind::PTK_DateTime:
    return convertIntegral<::Smp::UInt8>(value.dateTimeValue, type,
                                         PrimitiveTypeKind::PTK_UInt8);
  case PrimitiveTypeKind::PTK_Duration:
    return convertIntegral<::Smp::UInt8>(value.durationValue, type,
                                         PrimitiveTypeKind::PTK_UInt8);
  case PrimitiveTypeKind::PTK_UInt8:
    return this->value.uInt8Value;
  case PrimitiveTypeKind::PTK_UInt16:
    return convertIntegral<::Smp::UInt8>(value.uInt16Value, type,
                                         PrimitiveTypeKind::PTK_UInt8);
  case PrimitiveTypeKind::PTK_UInt32:
    return convertIntegral<::Smp::UInt8>(value.uInt32Value, type,
                                         PrimitiveTypeKind::PTK_UInt8);
  case PrimitiveTypeKind::PTK_UInt64:
    return convertIntegral<::Smp::UInt8>(value.uInt64Value, type,
                                         PrimitiveTypeKind::PTK_UInt8);
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, type,
                                           PrimitiveTypeKind::PTK_UInt8);
  }
}

AnySimple::operator ::Smp::UInt16() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_Int8:
    return convertIntegral<::Smp::UInt16>(value.int8Value, type,
                                          PrimitiveTypeKind::PTK_UInt16);
  case PrimitiveTypeKind::PTK_Int16:
    return convertIntegral<::Smp::UInt16>(value.int16Value, type,
                                          PrimitiveTypeKind::PTK_UInt16);
  case PrimitiveTypeKind::PTK_Int32:
    return convertIntegral<::Smp::UInt16>(value.int32Value, type,
                                          PrimitiveTypeKind::PTK_UInt16);
  case PrimitiveTypeKind::PTK_Int64:
    return convertIntegral<::Smp::UInt16>(value.int64Value, type,
                                          PrimitiveTypeKind::PTK_UInt16);
  case PrimitiveTypeKind::PTK_DateTime:
    return convertIntegral<::Smp::UInt16>(value.dateTimeValue, type,
                                          PrimitiveTypeKind::PTK_UInt16);
  case PrimitiveTypeKind::PTK_Duration:
    return convertIntegral<::Smp::UInt16>(value.durationValue, type,
                                          PrimitiveTypeKind::PTK_UInt16);
  case PrimitiveTypeKind::PTK_UInt8:
    return this->value.uInt8Value;
  case PrimitiveTypeKind::PTK_UInt16:
    return this->value.uInt16Value;
  case PrimitiveTypeKind::PTK_UInt32:
    return convertIntegral<::Smp::UInt16>(value.uInt32Value, type,
                                          PrimitiveTypeKind::PTK_UInt16);
  case PrimitiveTypeKind::PTK_UInt64:
    return convertIntegral<::Smp::UInt16>(value.uInt64Value, type,
                                          PrimitiveTypeKind::PTK_UInt16);
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, type,
                                           PrimitiveTypeKind::PTK_UInt16);
  }
}

AnySimple::operator ::Smp::UInt32() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_Int8:
    return convertIntegral<::Smp::UInt32>(value.int8Value, type,
                                          PrimitiveTypeKind::PTK_UInt32);
  case PrimitiveTypeKind::PTK_Int16:
    return convertIntegral<::Smp::UInt32>(value.int16Value, type,
                                          PrimitiveTypeKind::PTK_UInt32);
  case PrimitiveTypeKind::PTK_Int32:
    return convertIntegral<::Smp::UInt32>(value.int32Value, type,
                                          PrimitiveTypeKind::PTK_UInt32);
  case PrimitiveTypeKind::PTK_Int64:
    return convertIntegral<::Smp::UInt32>(value.int64Value, type,
                                          PrimitiveTypeKind::PTK_UInt32);
  case PrimitiveTypeKind::PTK_DateTime:
    return convertIntegral<::Smp::UInt32>(value.dateTimeValue, type,
                                          PrimitiveTypeKind::PTK_UInt32);
  case PrimitiveTypeKind::PTK_Duration:
    return convertIntegral<::Smp::UInt32>(value.durationValue, type,
                                          PrimitiveTypeKind::PTK_UInt32);
  case PrimitiveTypeKind::PTK_UInt8:
    return this->value.uInt8Value;
  case PrimitiveTypeKind::PTK_UInt16:
    return this->value.uInt16Value;
  case PrimitiveTypeKind::PTK_UInt32:
    return this->value.uInt32Value;
  case PrimitiveTypeKind::PTK_UInt64:
    return convertIntegral<::Smp::UInt32>(value.uInt64Value, type,
                                          PrimitiveTypeKind::PTK_UInt32);
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, type,
                                           PrimitiveTypeKind::PTK_UInt32);
  }
}

AnySimple::operator ::Smp::UInt64() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_Int8:
    return convertIntegral<::Smp::UInt64>(value.int8Value, type,
                                          PrimitiveTypeKind::PTK_UInt64);
  case PrimitiveTypeKind::PTK_Int16:
    return convertIntegral<::Smp::UInt64>(value.int16Value, type,
                                          PrimitiveTypeKind::PTK_UInt64);
  case PrimitiveTypeKind::PTK_Int32:
    return convertIntegral<::Smp::UInt64>(value.int32Value, type,
                                          PrimitiveTypeKind::PTK_UInt64);
  case PrimitiveTypeKind::PTK_Int64:
    return convertIntegral<::Smp::UInt64>(value.int64Value, type,
                                          PrimitiveTypeKind::PTK_UInt64);
  case PrimitiveTypeKind::PTK_DateTime:
    return convertIntegral<::Smp::UInt64>(value.dateTimeValue, type,
                                          PrimitiveTypeKind::PTK_UInt64);
  case PrimitiveTypeKind::PTK_Duration:
    return convertIntegral<::Smp::UInt64>(value.durationValue, type,
                                          PrimitiveTypeKind::PTK_UInt64);
  case PrimitiveTypeKind::PTK_UInt8:
    return this->value.uInt8Value;
  case PrimitiveTypeKind::PTK_UInt16:
    return this->value.uInt16Value;
  case PrimitiveTypeKind::PTK_UInt32:
    return this->value.uInt32Value;
  case PrimitiveTypeKind::PTK_UInt64:
    return this->value.uInt64Value;
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, type,
                                           PrimitiveTypeKind::PTK_UInt64);
  }
}

AnySimple::operator ::Smp::Int8() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_Int8:
    return this->value.int8Value;
  case PrimitiveTypeKind::PTK_Int16:
    return convertIntegral<::Smp::Int8>(value.int16Value, type,
                                        PrimitiveTypeKind::PTK_Int8);
  case PrimitiveTypeKind::PTK_Int32:
    return convertIntegral<::Smp::Int8>(value.int32Value, type,
                                        PrimitiveTypeKind::PTK_Int8);
  case PrimitiveTypeKind::PTK_Int64:
    return convertIntegral<::Smp::Int8>(value.int64Value, type,
                                        PrimitiveTypeKind::PTK_Int8);
  case PrimitiveTypeKind::PTK_DateTime:
    return convertIntegral<::Smp::Int8>(value.dateTimeValue, type,
                                        PrimitiveTypeKind::PTK_Int8);
  case PrimitiveTypeKind::PTK_Duration:
    return convertIntegral<::Smp::Int8>(value.durationValue, type,
                                        PrimitiveTypeKind::PTK_Int8);
  case PrimitiveTypeKind::PTK_UInt8:
    return convertIntegral<::Smp::Int8>(value.uInt8Value, type,
                                        PrimitiveTypeKind::PTK_Int8);
  case PrimitiveTypeKind::PTK_UInt16:
    return convertIntegral<::Smp::Int8>(value.uInt16Value, type,
                                        PrimitiveTypeKind::PTK_Int8);
  case PrimitiveTypeKind::PTK_UInt32:
    return convertIntegral<::Smp::Int8>(value.uInt32Value, type,
                                        PrimitiveTypeKind::PTK_Int8);
  case PrimitiveTypeKind::PTK_UInt64:
    return convertIntegral<::Smp::Int8>(value.uInt64Value, type,
                                        PrimitiveTypeKind::PTK_Int8);
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, type,
                                           PrimitiveTypeKind::PTK_Int8);
  }
}

AnySimple::operator ::Smp::Int16() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_Int8:
    return this->value.int8Value;
  case PrimitiveTypeKind::PTK_Int16:
    return this->value.int16Value;
  case PrimitiveTypeKind::PTK_Int32:
    return convertIntegral<::Smp::Int16>(value.int32Value, type,
                                         PrimitiveTypeKind::PTK_Int16);
  case PrimitiveTypeKind::PTK_Int64:
    return convertIntegral<::Smp::Int16>(value.int64Value, type,
                                         PrimitiveTypeKind::PTK_Int16);
  case PrimitiveTypeKind::PTK_DateTime:
    return convertIntegral<::Smp::Int16>(value.dateTimeValue, type,
                                         PrimitiveTypeKind::PTK_Int16);
  case PrimitiveTypeKind::PTK_Duration:
    return convertIntegral<::Smp::Int16>(value.durationValue, type,
                                         PrimitiveTypeKind::PTK_Int16);
  case PrimitiveTypeKind::PTK_UInt8:
    return this->value.uInt8Value;
  case PrimitiveTypeKind::PTK_UInt16:
    return convertIntegral<::Smp::Int16>(value.uInt16Value, type,
                                         PrimitiveTypeKind::PTK_Int16);
  case PrimitiveTypeKind::PTK_UInt32:
    return convertIntegral<::Smp::Int16>(value.uInt32Value, type,
                                         PrimitiveTypeKind::PTK_Int16);
  case PrimitiveTypeKind::PTK_UInt64:
    return convertIntegral<::Smp::Int16>(value.uInt64Value, type,
                                         PrimitiveTypeKind::PTK_Int16);
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, type,
                                           PrimitiveTypeKind::PTK_Int16);
  }
}

AnySimple::operator ::Smp::Int32() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_Int8:
    return this->value.int8Value;
  case PrimitiveTypeKind::PTK_Int16:
    return this->value.int16Value;
  case PrimitiveTypeKind::PTK_Int32:
    return this->value.int32Value;
  case PrimitiveTypeKind::PTK_Int64:
    return convertIntegral<::Smp::Int32>(value.int64Value, type,
                                         PrimitiveTypeKind::PTK_Int32);
  case PrimitiveTypeKind::PTK_DateTime:
    return convertIntegral<::Smp::Int32>(value.dateTimeValue, type,
                                         PrimitiveTypeKind::PTK_Int32);
  case PrimitiveTypeKind::PTK_Duration:
    return convertIntegral<::Smp::Int32>(value.durationValue, type,
                                         PrimitiveTypeKind::PTK_Int32);
  case PrimitiveTypeKind::PTK_UInt8:
    return this->value.uInt8Value;
  case PrimitiveTypeKind::PTK_UInt16:
    return this->value.uInt16Value;
  case PrimitiveTypeKind::PTK_UInt32:
    return convertIntegral<::Smp::Int32>(value.uInt32Value, type,
                                         PrimitiveTypeKind::PTK_Int32);
  case PrimitiveTypeKind::PTK_UInt64:
    return convertIntegral<::Smp::Int32>(value.uInt64Value, type,
                                         PrimitiveTypeKind::PTK_Int32);
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, type,
                                           PrimitiveTypeKind::PTK_Int32);
  }
}

AnySimple::operator ::Smp::Int64() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_Int8:
    return this->value.int8Value;
  case PrimitiveTypeKind::PTK_Int16:
    return this->value.int16Value;
  case PrimitiveTypeKind::PTK_Int32:
    return this->value.int32Value;
  case PrimitiveTypeKind::PTK_Int64:
    return this->value.int64Value;
  case PrimitiveTypeKind::PTK_Duration:
    return this->value.durationValue;
  case PrimitiveTypeKind::PTK_DateTime:
    return this->value.dateTimeValue;
  case PrimitiveTypeKind::PTK_UInt8:
    return this->value.uInt8Value;
  case PrimitiveTypeKind::PTK_UInt16:
    return this->value.uInt16Value;
  case PrimitiveTypeKind::PTK_UInt32:
    return this->value.uInt32Value;
  case PrimitiveTypeKind::PTK_UInt64:
    return convertIntegral<::Smp::Int64>(value.uInt64Value, type,
                                         PrimitiveTypeKind::PTK_Int64);
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, type,
                                           PrimitiveTypeKind::PTK_Int64);
  }
}

AnySimple::operator ::Smp::Float32() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_UInt8:
    return this->value.uInt8Value;
  case PrimitiveTypeKind::PTK_UInt16:
    return this->value.uInt16Value;
  case PrimitiveTypeKind::PTK_Int8:
    return this->value.int8Value;
  case PrimitiveTypeKind::PTK_Int16:
    return this->value.int16Value;

  case PrimitiveTypeKind::PTK_Int32:
    return convertFloat<::Smp::Float32>(value.int32Value, type,
                                        PrimitiveTypeKind::PTK_Float32);
  case PrimitiveTypeKind::PTK_Int64:
    return convertFloat<::Smp::Float32>(value.int64Value, type,
                                        PrimitiveTypeKind::PTK_Float32);
  case PrimitiveTypeKind::PTK_UInt32:
    return convertFloat<::Smp::Float32>(value.uInt32Value, type,
                                        PrimitiveTypeKind::PTK_Float32);
  case PrimitiveTypeKind::PTK_UInt64:
    return convertFloat<::Smp::Float32>(value.uInt64Value, type,
                                        PrimitiveTypeKind::PTK_Float32);
  case PrimitiveTypeKind::PTK_Float32:
    return this->value.float32Value;
  case PrimitiveTypeKind::PTK_Float64:
    return convertFloat<::Smp::Float32>(value.float64Value, type,
                                        PrimitiveTypeKind::PTK_Float32);
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, type,
                                           PrimitiveTypeKind::PTK_Float32);
  }
}

AnySimple::operator ::Smp::Float64() const {
  switch (type) {
  case PrimitiveTypeKind::PTK_UInt8:
    return this->value.uInt8Value;
  case PrimitiveTypeKind::PTK_UInt16:
    return this->value.uInt16Value;
  case PrimitiveTypeKind::PTK_UInt32:
    return this->value.uInt32Value;
  case PrimitiveTypeKind::PTK_Int8:
    return this->value.int8Value;
  case PrimitiveTypeKind::PTK_Int16:
    return this->value.int16Value;
  case PrimitiveTypeKind::PTK_Int32:
    return this->value.int32Value;
  case PrimitiveTypeKind::PTK_Int64:
    return convertFloat<::Smp::Float64>(value.int64Value, type,
                                        PrimitiveTypeKind::PTK_Float64);
  case PrimitiveTypeKind::PTK_UInt64:
    return convertFloat<::Smp::Float64>(value.uInt64Value, type,
                                        PrimitiveTypeKind::PTK_Float64);
  case PrimitiveTypeKind::PTK_Float32:
    return static_cast<::Smp::Float64>(this->value.float32Value);
  case PrimitiveTypeKind::PTK_Float64:
    return this->value.float64Value;
  default:
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, type,
                                           PrimitiveTypeKind::PTK_Float64);
  }
}

::Smp::String8 AnySimple::MoveString() {
  if (type == PrimitiveTypeKind::PTK_String8) {
    const auto *result = this->value.string8Value;
    this->value.string8Value = nullptr;
    return result;
  }
  ::Xsmp::Exception::throwInvalidAnyType(nullptr, type,
                                         PrimitiveTypeKind::PTK_String8);
}

::Smp::PrimitiveTypeKind AnySimple::GetType() const noexcept { return type; }

bool ::Smp::AnySimple::operator==(const ::Smp::AnySimple & other) const {
  try {
    switch (type) {
    case PrimitiveTypeKind::PTK_None:
      return true;
    case PrimitiveTypeKind::PTK_Bool:
      return value.boolValue == static_cast<::Smp::Bool>(other);
    case PrimitiveTypeKind::PTK_UInt8:
      return value.uInt8Value == static_cast<::Smp::UInt8>(other);
    case PrimitiveTypeKind::PTK_UInt16:
      return value.uInt16Value == static_cast<::Smp::UInt16>(other);
    case PrimitiveTypeKind::PTK_UInt32:
      return value.uInt32Value == static_cast<::Smp::UInt32>(other);
    case PrimitiveTypeKind::PTK_UInt64:
      return value.uInt64Value == static_cast<::Smp::UInt64>(other);
    case PrimitiveTypeKind::PTK_Int8:
      return value.int8Value == static_cast<::Smp::Int8>(other);
    case PrimitiveTypeKind::PTK_Int16:
      return value.int16Value == static_cast<::Smp::Int16>(other);
    case PrimitiveTypeKind::PTK_Int32:
      return value.int32Value == static_cast<::Smp::Int32>(other);
    case PrimitiveTypeKind::PTK_Int64:
      return value.int64Value == static_cast<::Smp::Int64>(other);
    case PrimitiveTypeKind::PTK_Duration:
      return value.durationValue == static_cast<::Smp::Duration>(other);
    case PrimitiveTypeKind::PTK_DateTime:
      return value.dateTimeValue == static_cast<::Smp::DateTime>(other);
    case PrimitiveTypeKind::PTK_Float32:
      return std::fabs(value.float32Value -
                       static_cast<::Smp::Float32>(other)) <=
             std::numeric_limits<Float32>::epsilon();
    case PrimitiveTypeKind::PTK_Float64:
      return std::fabs(value.float64Value -
                       static_cast<::Smp::Float64>(other)) <=
             std::numeric_limits<Float64>::epsilon();
    case PrimitiveTypeKind::PTK_Char8:
      return value.char8Value == static_cast<::Smp::Char8>(other);
    case PrimitiveTypeKind::PTK_String8:
      const auto *string8 = static_cast<::Smp::String8>(other);
      if (value.string8Value == string8) {
        return true;
      }
      if (value.string8Value && string8) {
        auto l1 = std::char_traits<char>::length(value.string8Value);
        auto l2 = std::char_traits<char>::length(string8);
        if (l1 == l2) {
          return std::char_traits<char>::compare(value.string8Value, string8,
                                                 l1) == 0;
        }
      }
      return false;
    }
  } catch (::Smp::InvalidAnyType &) {
    return false;
  }
  return false;
}

bool ::Smp::AnySimple::operator!=(const ::Smp::AnySimple & other) const {
  return !(*this == other);
}

} // namespace Smp
