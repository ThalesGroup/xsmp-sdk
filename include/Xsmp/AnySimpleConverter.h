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

#ifndef XSMP_ANYSIMPLECONVERTER_H_
#define XSMP_ANYSIMPLECONVERTER_H_

#include <Smp/AnySimple.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Helper.h>
#include <type_traits>

namespace Xsmp {

/// Basic converter for standard types (integrals, floats, bool)
template<typename T, typename = void>
struct AnySimpleConverter {
    static inline T convert(const ::Smp::AnySimple &value) {
        return static_cast<T>(value);
    }
    static inline ::Smp::AnySimple convert(::Smp::PrimitiveTypeKind kind,
            T value) {
        return ::Smp::AnySimple { kind, value };
    }
};

/// converter for enum types (intermediate cast to ::Smp::Int32)
template<typename T>
struct AnySimpleConverter<T, std::enable_if_t<std::is_enum_v<T>>> {
    static inline T convert(const ::Smp::AnySimple &value) {
        return static_cast<T>(static_cast<::Smp::Int32>(value));
    }
    static inline ::Smp::AnySimple convert(::Smp::PrimitiveTypeKind kind,
            T value) {
        return {kind, static_cast<::Smp::Int32>(value)};
    }
};

/// converter for smp strings (with internalString array)
template<typename T>
struct AnySimpleConverter<T, std::enable_if_t<Helper::is_smp_string_v<T>>> {
    static inline T convert(const ::Smp::AnySimple &value) {
        T new_value;
        ::Xsmp::Helper::CopyString(&new_value.internalString[0],
                sizeof(new_value.internalString), value);
        return new_value;
    }
    static inline ::Smp::AnySimple convert(::Smp::PrimitiveTypeKind kind,
            const T &value) {
        return {kind, &value.internalString[0]};
    }
};

/// converter for array of char (std::array<::Smp::Char8, ?> or ::Smp::Char8[?])
template<typename T>
struct AnySimpleConverter<T, std::enable_if_t<Helper::is_char_array_v<T>>> {
    static inline T convert(const ::Smp::AnySimple &value) {
        T new_value;
        ::Xsmp::Helper::CopyString(&new_value[0], sizeof(new_value), value);
        return new_value;
    }
    static inline ::Smp::AnySimple convert(::Smp::PrimitiveTypeKind kind,
            const T &value) {
        return {kind, &value[0]};
    }
};

} // namespace Xsmp

#endif // XSMP_ANYSIMPLECONVERTER_H_
