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

#ifndef XSMP_REQUEST_H_
#define XSMP_REQUEST_H_

#include <Smp/AnySimple.h>
#include <Smp/IRequest.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Uuid.h>
#include <Xsmp/AnySimpleConverter.h>
#include <Xsmp/Field.h>
#include <string>

namespace Smp {
class IComponent;
} // namespace Smp

/// XSMP standard types and interfaces.
namespace Xsmp {

/// @class Request
/// XSMP request helper.
struct Request {
private:
  static void setValue(::Smp::IComponent const *component,
                       ::Smp::IRequest *request, const std::string &name,
                       const ::Smp::AnySimple &value);

  [[nodiscard]] static ::Smp::AnySimple
  getValue(::Smp::IComponent const *component, ::Smp::IRequest const *request,
           const std::string &name, ::Smp::PrimitiveTypeKind kind,
           bool useDefaultValue = false);

  /// Extract a field value from a request
  static void extract(::Smp::IRequest *request, ::Smp::IField *field,
                      const std::string &name,
                      bool ignoreMissingParameters = false);
  /// inject a field value in a request
  static void inject(::Smp::IRequest *request, ::Smp::IField *field,
                     const std::string &name);

public:
  /// Get an input parameter value from a request
  /// @tparam T The value type
  /// @param component the base component
  /// @param request the request
  /// @param name the parameter name
  /// @param kind the primitive type kind
  /// @return the parameter value
  template <typename T>
  [[nodiscard]] static T
  get(::Smp::IComponent const *component, ::Smp::IRequest const *request,
      const std::string &name, ::Smp::PrimitiveTypeKind kind) {
    return AnySimpleConverter<T>::convert(
        getValue(component, request, name, kind));
  }
  /// Get an input parameter value from a request with default value
  /// @tparam T The value type
  /// @param component the base component
  /// @param request the request
  /// @param name the parameter name
  /// @param kind the primitive type kind
  /// @param defaultValue the default value to return if the parameter is not
  /// found
  /// @return the parameter value or defaultValue
  template <typename T>
  [[nodiscard]] static T
  get(::Smp::IComponent const *component, ::Smp::IRequest const *request,
      const std::string &name, ::Smp::PrimitiveTypeKind kind, T defaultValue) {
    auto value = getValue(component, request, name, kind, true);

    if (value.GetType() == ::Smp::PrimitiveTypeKind::PTK_None) {
      return defaultValue;
    }
    return AnySimpleConverter<T>::convert(value);
  }

  /// Get an input parameter value from a request
  /// @param component the base component
  /// @param request the request
  /// @param name the parameter name
  /// @param uuid the type uuid
  /// @return the parameter value
  template <typename T, class C>
  [[nodiscard]] static T get(C *component, ::Smp::IRequest *request,
                             const std::string &name, const ::Smp::Uuid &uuid) {
    Field<T> parameter{component, uuid, name};
    extract(request, &parameter, parameter.GetName());
    return parameter;
  }
  /// Get an input parameter value from a request with default value
  /// @param component the base component
  /// @param request the request
  /// @param name the parameter name
  /// @param uuid the type uuid
  /// @return the parameter value
  template <typename T, class C>
  [[nodiscard]] static T get(C *component, ::Smp::IRequest *request,
                             const std::string &name, const ::Smp::Uuid &uuid,
                             const T &default_value) {
    Field<T> parameter{component, uuid, name};
    parameter = default_value;
    extract(request, &parameter, parameter.GetName(), true);
    return parameter;
  }

  /// Set an output parameter value in a request
  /// @tparam T The parameter type
  /// @param component the base component
  /// @param request the request
  /// @param name The parameter name
  /// @param kind The value primitive type kind
  /// @param value The value
  template <typename T>
  static void set(::Smp::IComponent *component, ::Smp::IRequest *request,
                  const std::string &name, ::Smp::PrimitiveTypeKind kind,
                  const T &value) {
    setValue(component, request, name,
             AnySimpleConverter<T>::convert(kind, value));
  }
  /// Set an output parameter value in a request
  /// @tparam T The parameter type
  /// @param component the base component
  /// @param request the request
  /// @param name The parameter name
  /// @param uuid The type UUID
  /// @param value The value
  template <typename T, class C>
  static void set(C *component, ::Smp::IRequest *request,
                  const std::string &name, const ::Smp::Uuid &uuid,
                  const T &value) {
    Field<T> parameter{component, getTypeRegistry(component), uuid, name};
    parameter = value;
    inject(request, &parameter, parameter.GetName());
  }

  /// Set a parameter value in a request
  /// @tparam T The parameter type
  /// @param request the request
  /// @param kind The value primitive type kind
  /// @param value The value
  template <typename T>
  static void setReturnValue(::Smp::IRequest *request,
                             ::Smp::PrimitiveTypeKind kind, const T &value) {
    request->SetReturnValue(AnySimpleConverter<T>::convert(kind, value));
  }
};
} // namespace Xsmp

#endif // XSMP_REQUEST_H_
