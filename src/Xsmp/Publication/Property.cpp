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
#include <Smp/ViewKind.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Publication/Property.h>
#include <Xsmp/Publication/Request.h>
#include <string>
#include <utility>

namespace Xsmp::Publication {

class Property::Getter final : public ::Smp::IRequest {

public:
  explicit Getter(const ::Smp::IProperty *property)
      : _property{property}, _name{std::string("get_") + property->GetName()} {}
  ~Getter() noexcept override = default;
  Getter(const Getter &) = delete;
  Getter &operator=(const Getter &) = delete;

  [[nodiscard]] ::Smp::String8 GetOperationName() const override {
    return _name.c_str();
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
    if (!::Xsmp::Publication::Request::isValid(_property, _property->GetType(),
                                               value)) {
      ::Xsmp::Exception::throwInvalidReturnValue(_property, value);
    }
    _returnValue = std::move(value);
  }

  [[nodiscard]] ::Smp::AnySimple GetReturnValue() const override {
    return _returnValue;
  }

private:
  const ::Smp::IProperty *_property;
  std::string _name;
  ::Smp::AnySimple _returnValue{};
};

class Property::Setter final : public ::Smp::IRequest {

public:
  explicit Setter(const ::Smp::IProperty *property)
      : _property{property}, _name{std::string("set_") + property->GetName()} {}
  ~Setter() noexcept override = default;
  Setter(const Setter &) = delete;
  Setter &operator=(const Setter &) = delete;

  [[nodiscard]] ::Smp::String8 GetOperationName() const override {
    return _name.c_str();
  }

  [[nodiscard]] ::Smp::Int32 GetParameterCount() const override { return 1; }

  [[nodiscard]] ::Smp::Int32 GetParameterIndex(::Smp::String8) const override {
    return 0;
  }

  void SetParameterValue(::Smp::Int32 index, ::Smp::AnySimple value) override {
    if (index != 0) {
      ::Xsmp::Exception::throwInvalidParameterIndex(_property, index, 0);
    }
    if (!::Xsmp::Publication::Request::isValid(_property, _property->GetType(),
                                               value)) {
      ::Xsmp::Exception::throwInvalidParameterValue(
          _property, _property->GetName(), value);
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
  std::string _name;
  ::Smp::AnySimple _value{};
};

Property::Property(::Smp::String8 name, ::Smp::String8 description,
                   ::Smp::IObject *parent, ::Smp::Publication::IType *type,
                   ::Smp::AccessKind accessKind, ::Smp::ViewKind view)
    : _name(::Xsmp::Helper::checkName(name, parent)),
      _description(description ? description : ""), _parent(parent),
      _type(type), _accessKind(accessKind), _view(view) {}
::Smp::String8 Property::GetName() const { return _name.c_str(); }

::Smp::String8 Property::GetDescription() const { return _description.c_str(); }

::Smp::IObject *Property::GetParent() const { return _parent; }
::Smp::Publication::IType *Property::GetType() const { return _type; }

::Smp::AccessKind Property::GetAccess() const { return _accessKind; }

::Smp::ViewKind Property::GetView() const { return _view; }

::Smp::AnySimple Property::GetValue() const {

  auto *invoker = dynamic_cast<::Smp::IDynamicInvocation *>(_parent);
  if (!invoker || _accessKind == ::Smp::AccessKind::AK_WriteOnly) {
    ::Xsmp::Exception::throwException(
        this, "InvalidAccessKind", "",
        "The property getter is not invokable: ", this);
  }
  Getter request{this};
  invoker->Invoke(&request);
  return request.GetReturnValue();
}

void Property::SetValue(::Smp::AnySimple value) {
  auto *invoker = dynamic_cast<::Smp::IDynamicInvocation *>(_parent);
  if (!invoker || _accessKind == ::Smp::AccessKind::AK_ReadOnly) {
    ::Xsmp::Exception::throwException(
        this, "InvalidAccessKind", "",
        "The property setter is not invokable: ", this);
  }
  Setter request{this};
  request.SetParameterValue(0, std::move(value));
  invoker->Invoke(&request);
}

::Smp::IRequest *Property::CreateGetRequest() const noexcept {
  return _accessKind == ::Smp::AccessKind::AK_WriteOnly ? nullptr
                                                        : new Getter(this);
}

::Smp::IRequest *Property::CreateSetRequest() const noexcept {
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
