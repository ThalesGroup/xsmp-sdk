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

#include <Smp/AccessKind.h>
#include <Smp/AnySimple.h>
#include <Smp/IDynamicInvocation.h>
#include <Smp/IProperty.h>
#include <Smp/IRequest.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IType.h>
#include <Smp/RequestType.h>
#include <Smp/ViewKind.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Publication/Property.h>
#include <Xsmp/Publication/Request.h>
#include <Xsmp/cstring.h>
#include <string>
#include <utility>

namespace Xsmp::Publication {

class Property::Getter final : public ::Smp::IRequest {

public:
  explicit Getter(const ::Smp::IProperty *property)
      // SMP 2025 has the request carry the bare property name; a getter is
      // told from a setter by GetType()
      : _property{property}, _name{property->GetName()} {}
  ~Getter() noexcept override = default;
  Getter(const Getter &) = delete;
  Getter(Getter &&) = delete;
  Getter &operator=(const Getter &) = delete;
  Getter &operator=(Getter &&) = delete;

  [[nodiscard]] ::Smp::String8 GetName() const override {
    return _name.c_str();
  }

  [[nodiscard]] ::Smp::RequestType GetType() const override {
    return ::Smp::RequestType::RT_Get;
  }

  [[nodiscard]] ::Smp::Int32 GetParameterCount() const override { return 0; }

  [[nodiscard]] ::Smp::Int32 GetParameterIndex(::Smp::String8) const override {
    return -1;
  }

  [[noreturn]] void SetParameterValue(::Smp::Int32 index,
                                      ::Smp::AnySimple) override {
    ::Xsmp::Exception::throwInvalidParameterIndex(_property, index, 0);
  }

  [[noreturn]] ::Smp::AnySimple
  GetParameterValue(::Smp::Int32 index) const override {
    ::Xsmp::Exception::throwInvalidParameterIndex(_property, index, 0);
  }

  void SetReturnValue(::Smp::AnySimple value) override {
    if (!::Xsmp::Publication::Request::isValid(_property->GetType(), value)) {
      ::Xsmp::Exception::throwInvalidPropertyValue(_property, value);
    }
    _returnValue = std::move(value);
  }

  [[nodiscard]] ::Smp::AnySimple GetReturnValue() const override {
    return _returnValue;
  }

private:
  const ::Smp::IProperty *_property;
  ::Xsmp::cstring _name;
  ::Smp::AnySimple _returnValue;
};

class Property::Setter final : public ::Smp::IRequest {

public:
  explicit Setter(const ::Smp::IProperty *property)
      // SMP 2025 has the request carry the bare property name; a getter is
      // told from a setter by GetType()
      : _property{property}, _name{property->GetName()} {}
  ~Setter() noexcept override = default;
  Setter(const Setter &) = delete;
  Setter(Setter &&) = delete;
  Setter &operator=(const Setter &) = delete;
  Setter &operator=(Setter &&) = delete;

  [[nodiscard]] ::Smp::String8 GetName() const override {
    return _name.c_str();
  }

  [[nodiscard]] ::Smp::RequestType GetType() const override {
    return ::Smp::RequestType::RT_Set;
  }

  [[nodiscard]] ::Smp::Int32 GetParameterCount() const override { return 1; }

  [[nodiscard]] ::Smp::Int32 GetParameterIndex(::Smp::String8) const override {
    return 0;
  }

  void SetParameterValue(::Smp::Int32 index, ::Smp::AnySimple value) override {
    if (index != 0) {
      ::Xsmp::Exception::throwInvalidParameterIndex(_property, index, 0);
    }
    if (!::Xsmp::Publication::Request::isValid(_property->GetType(), value)) {
      ::Xsmp::Exception::throwInvalidParameterValue(
          _property, _property->GetName(), value,
          _property->GetPrimitiveTypeKind());
    }
    _value = std::move(value);
  }

  [[nodiscard]] ::Smp::AnySimple
  GetParameterValue(::Smp::Int32 index) const override {
    if (index != 0) {
      ::Xsmp::Exception::throwInvalidParameterIndex(_property, index, 0);
    }
    return _value;
  }

  [[noreturn]] void SetReturnValue(::Smp::AnySimple) override {
    ::Xsmp::Exception::throwVoidOperation(_property);
  }

  [[noreturn]] ::Smp::AnySimple GetReturnValue() const override {
    ::Xsmp::Exception::throwVoidOperation(_property);
  }

private:
  const ::Smp::IProperty *_property;
  ::Xsmp::cstring _name;
  ::Smp::AnySimple _value;
};

Property::Property(::Smp::String8 name, ::Smp::String8 description,
                   ::Smp::IObject *parent, ::Smp::Publication::IType *type,
                   ::Smp::AccessKind accessKind, ::Smp::ViewKind view)
    : _name(::Xsmp::Helper::checkName(name, parent)), _description(description),
      _parent(parent), _type(type), _accessKind(accessKind), _view(view) {}
::Smp::String8 Property::GetName() const { return _name.c_str(); }

::Smp::String8 Property::GetDescription() const { return _description.c_str(); }

::Smp::IObject *Property::GetParent() const { return _parent; }

::Smp::IObject *Property::GetChild(::Smp::String8) const { return nullptr; }
const ::Smp::Publication::IType *Property::GetType() const { return _type; }

::Smp::PrimitiveTypeKind Property::GetPrimitiveTypeKind() const {
  return _type ? _type->GetPrimitiveTypeKind()
               : ::Smp::PrimitiveTypeKind::PTK_None;
}

::Smp::AccessKind Property::GetAccess() const { return _accessKind; }

::Smp::ViewKind Property::GetView() const { return _view; }

::Smp::AnySimple Property::GetValue() const {

  auto *invoker = dynamic_cast<::Smp::IDynamicInvocation *>(_parent);
  if (!invoker || _accessKind == ::Smp::AccessKind::AK_WriteOnly) {
    ::Xsmp::Exception::throwInvalidAccess(this, _accessKind, false);
  }
  Getter request{this};
  invoker->Invoke(&request);
  return request.GetReturnValue();
}

void Property::SetValue(::Smp::AnySimple value) {
  auto *invoker = dynamic_cast<::Smp::IDynamicInvocation *>(_parent);
  if (!invoker || _accessKind == ::Smp::AccessKind::AK_ReadOnly) {
    ::Xsmp::Exception::throwInvalidAccess(this, _accessKind, true);
  }
  // SMP 2025 reports a type mismatch on a property as InvalidPropertyValue
  if (value.GetType() != GetPrimitiveTypeKind()) {
    ::Xsmp::Exception::throwInvalidPropertyValue(this, value);
  }
  Setter request{this};
  request.SetParameterValue(0, std::move(value));
  invoker->Invoke(&request);
}

::Smp::IRequest *Property::CreateGetRequest() const {
  return _accessKind == ::Smp::AccessKind::AK_WriteOnly ? nullptr
                                                        : new Getter(this);
}

::Smp::IRequest *Property::CreateSetRequest() const {
  return _accessKind == ::Smp::AccessKind::AK_ReadOnly ? nullptr
                                                       : new Setter(this);
}

void Property::Update(::Smp::String8 description,
                      ::Smp::Publication::IType *type,
                      ::Smp::AccessKind accessKind,
                      ::Smp::ViewKind view) noexcept {
  _description = description;
  _type = type;
  _accessKind = accessKind;
  _view = view;
}

} // namespace Xsmp::Publication
