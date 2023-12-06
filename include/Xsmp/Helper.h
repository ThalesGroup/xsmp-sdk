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

#ifndef XSMP_HELPER_H_
#define XSMP_HELPER_H_

#include <Smp/AnySimple.h>
#include <Smp/PrimitiveTypes.h>
#include <array>
#include <cstddef>
#include <limits>
#include <string>
#include <type_traits>
#include <typeinfo>

namespace Smp {
class IEntryPoint;
class IField;
class IObject;
class ISimulator;
template<typename >
class ICollection;
} // namespace Smp

namespace Xsmp::Helper {

/// Execute an EntryPoint.
/// If an exception is raised by the entry point, the Simulation
/// is aborted and the exception is logged.
void SafeExecute(::Smp::ISimulator *simulator,
        const ::Smp::IEntryPoint *entryPoint);

/// Get the object's path.
/// @param obj the object
/// @return the path
[[nodiscard]] std::string GetPath(const ::Smp::IObject *obj);

[[nodiscard]] ::Smp::IObject* Resolve(
        const ::Smp::ICollection<Smp::IField> *fields, ::Smp::String8 path);
[[nodiscard]] ::Smp::IObject* Resolve(::Smp::IObject *parent,
        ::Smp::String8 path);
[[nodiscard]] ::Smp::IObject* Resolve(::Smp::IField *parent,
        ::Smp::String8 path);

[[nodiscard]] std::string demangle(::Smp::String8 typeName);

[[nodiscard]] std::string TypeName(const ::Smp::IObject *type);
template<typename T>
[[nodiscard]] std::string TypeName() {
    return demangle(typeid(T).name());
}

/// helper function to copy a string from an AnySimple
void CopyString(::Smp::Char8 *destination, size_t size,
        const ::Smp::AnySimple &value);

template<typename T>
[[nodiscard]] T* GetParentOfType(::Smp::IObject *object) {
    for (auto *e = object; e != nullptr; e = e->GetParent())
        if (auto *casted = dynamic_cast<T*>(e))
            return casted;

    return nullptr;
}

template<typename T>
[[nodiscard]] const T* GetParentOfType(const ::Smp::IObject *object) {
    for (const auto *e = object; e != nullptr; e = e->GetParent())
        if (const auto *casted = dynamic_cast<const T*>(e))
            return casted;

    return nullptr;
}

[[nodiscard]] bool AreEquivalent(const ::Smp::IField *first,
        const ::Smp::IField *second);

/// helper to check if a type is an smp string of the form
/// struct MyStr {::Smp::Char8 internalString[42];};
template<typename T, typename = void>
struct is_smp_string: std::false_type {
};
template<typename T>
struct is_smp_string<T,
        std::enable_if_t<
                std::is_same_v<
                        std::remove_reference_t<decltype(T::internalString[0])>,
                        ::Smp::Char8> && sizeof(T) == sizeof(T::internalString)>> : std::true_type {
};
template<typename T>
constexpr bool is_smp_string_v = is_smp_string<T>::value;

/// helper to check if a type is an array of ::Smp::Char8
template<typename >
struct is_char_array: public std::false_type {
};
template<std::size_t S>
struct is_char_array<::Smp::Char8[S]> : public std::true_type {
};
template<std::size_t S>
struct is_char_array<std::array<::Smp::Char8, S>> : public std::true_type {
};
template<typename T>
constexpr bool is_char_array_v = is_char_array<T>::value;

///helper to check if a type is an Smp Primitive type
template<typename T, typename = void>
struct is_primitive_type: public std::false_type {
};
template<typename T>
struct is_primitive_type<T,
        std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>>> : public std::true_type {
};
template<typename T>
constexpr bool is_primitive_type_v = is_primitive_type<T>::value;

///helper to check if a type is an Smp simple type
template<typename T, typename = void>
struct is_simple_type: public std::false_type {
};
template<typename T>
struct is_simple_type<T,
        std::enable_if_t<
                is_primitive_type_v<T> || std::is_enum_v<T>
                        || is_smp_string_v<T> || is_char_array_v<T>>> : public std::true_type {
};
template<typename T>
constexpr bool is_simple_type_v = is_simple_type<T>::value;

template<::Smp::PrimitiveTypeKind T>
struct numeric_limits {

};
template<>
struct numeric_limits<::Smp::PrimitiveTypeKind::PTK_Bool> : std::numeric_limits<
        ::Smp::Bool> {
};
template<>
struct numeric_limits<::Smp::PrimitiveTypeKind::PTK_Char8> : std::numeric_limits<
        ::Smp::Char8> {
};
template<>
struct numeric_limits<::Smp::PrimitiveTypeKind::PTK_Int8> : std::numeric_limits<
        ::Smp::Int8> {
};
template<>
struct numeric_limits<::Smp::PrimitiveTypeKind::PTK_Int16> : std::numeric_limits<
        ::Smp::Int16> {
};
template<>
struct numeric_limits<::Smp::PrimitiveTypeKind::PTK_Int32> : std::numeric_limits<
        ::Smp::Int32> {
};
template<>
struct numeric_limits<::Smp::PrimitiveTypeKind::PTK_Int64> : std::numeric_limits<
        ::Smp::Int64> {
};
template<>
struct numeric_limits<::Smp::PrimitiveTypeKind::PTK_Duration> : std::numeric_limits<
        ::Smp::Duration> {
};
template<>
struct numeric_limits<::Smp::PrimitiveTypeKind::PTK_DateTime> : std::numeric_limits<
        ::Smp::DateTime> {
};
template<>
struct numeric_limits<::Smp::PrimitiveTypeKind::PTK_UInt8> : std::numeric_limits<
        ::Smp::UInt8> {
};
template<>
struct numeric_limits<::Smp::PrimitiveTypeKind::PTK_UInt16> : std::numeric_limits<
        ::Smp::UInt16> {
};
template<>
struct numeric_limits<::Smp::PrimitiveTypeKind::PTK_UInt32> : std::numeric_limits<
        ::Smp::UInt32> {
};
template<>
struct numeric_limits<::Smp::PrimitiveTypeKind::PTK_UInt64> : std::numeric_limits<
        ::Smp::UInt64> {
};
template<>
struct numeric_limits<::Smp::PrimitiveTypeKind::PTK_Float32> : std::numeric_limits<
        ::Smp::Float32> {
};
template<>
struct numeric_limits<::Smp::PrimitiveTypeKind::PTK_Float64> : std::numeric_limits<
        ::Smp::Float64> {
};

} // namespace Xsmp::Helper

#endif // XSMP_HELPER_H_
