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
#include <Smp/IDynamicInvocation.h>
#include <Smp/IParameter.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IType.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Publication/ParameterDirectionKind.h>
#include <Smp/Uuid.h>
#include <Smp/ViewKind.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Publication/Field.h>
#include <Xsmp/Publication/Operation.h>
#include <Xsmp/Publication/Publication.h>
#include <Xsmp/Publication/Request.h>
#include <cstddef>
#include <cstring>
#include <memory>
#include <string_view>

namespace Xsmp::Publication {

Operation::Parameter::Parameter(
    ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
    ::Smp::Publication::IType *type,
    ::Smp::Publication::ParameterDirectionKind direction)
    : _name(::Xsmp::Helper::checkName(name, parent)), _description(description),
      _parent(parent), _type(type), _direction(direction) {}
::Smp::String8 Operation::Parameter::GetName() const { return _name.c_str(); }

::Smp::String8 Operation::Parameter::GetDescription() const {
  return _description.c_str();
}

::Smp::IObject *Operation::Parameter::GetParent() const { return _parent; }
::Smp::Publication::IType *Operation::Parameter::GetType() const {
  return _type;
}

::Smp::Publication::ParameterDirectionKind
Operation::Parameter::GetDirection() const {
  return _direction;
}

Operation::Operation(::Smp::String8 name, ::Smp::String8 description,
                     ::Smp::IObject *parent, ::Smp::ViewKind view,
                     ::Smp::Publication::ITypeRegistry *typeRegistry)
    : _name(::Xsmp::Helper::checkName(name, parent)), _description(description),
      _parent(parent), _parameters{"Parameters", "", parent},
      _typeRegistry{typeRegistry}, _view(view) {}
::Smp::String8 Operation::GetName() const { return _name.c_str(); }

::Smp::String8 Operation::GetDescription() const {
  return _description.c_str();
}

::Smp::IObject *Operation::GetParent() const { return _parent; }
void Operation::PublishParameter(
    ::Smp::String8 name, ::Smp::String8 description, ::Smp::Uuid typeUuid,
    ::Smp::Publication::ParameterDirectionKind direction) {

  auto *type = _typeRegistry->GetType(typeUuid);
  if (!type) {
    ::Xsmp::Exception::throwTypeNotRegistered(this, typeUuid);
  }

  if (direction == ::Smp::Publication::ParameterDirectionKind::PDK_Return) {
    if (_returnParameter) {
      ::Xsmp::Exception::throwException(
          this, "InvalidParameterDirection",
          "This Exception is thrown when trying to publish a parameter with an "
          "invalid direction.",
          "The return parameter \'", _returnParameter->GetName(),
          "` is already published.");
    }
    // check do duplicate parameter name
    if (_parameters.at(name)) {
      ::Xsmp::Exception::throwDuplicateName(this, name, &_parameters);
    }

    _returnParameter =
        std::make_unique<Parameter>(name, description, this, type, direction);
  } else {
    // check do duplicate parameter name
    if (_returnParameter &&
        std::strcmp(_returnParameter->GetName(), name) == 0) {
      ::Xsmp::Exception::throwDuplicateName(this, name, &_parameters);
    }
    _parameters.Add<Parameter>(name, description, this, type, direction);
  }
}

const ::Smp::ParameterCollection *Operation::GetParameters() const {
  return &_parameters;
}

::Smp::IParameter *Operation::GetParameter(::Smp::String8 name) const {

  if (auto *parameter = _parameters.at(name)) {
    return parameter;
  }
  if (_returnParameter && std::strcmp(_returnParameter->GetName(), name) == 0) {
    return _returnParameter.get();
  }
  return nullptr;
}

::Smp::IParameter *Operation::GetReturnParameter() const {
  return _returnParameter.get();
}

::Smp::ViewKind Operation::GetView() const { return _view; }

::Smp::IRequest *Operation::CreateRequest() {
  // create the request only if the operation is invokable
  for (auto const *param : _parameters) {
    if (param->GetType()->GetPrimitiveTypeKind() ==
        ::Smp::PrimitiveTypeKind::PTK_None) {
      return nullptr;
    }
  }
  if (_returnParameter && _returnParameter->GetType()->GetPrimitiveTypeKind() ==
                              ::Smp::PrimitiveTypeKind::PTK_None) {
    return nullptr;
  }
  return new ::Xsmp::Publication::Request(this, _typeRegistry);
}

void Operation::Invoke(::Smp::IRequest *request) {

  const auto *operationName = request ? request->GetOperationName() : nullptr;

  // check operation name
  if (!operationName || std::string_view{GetName()} != operationName) {
    ::Xsmp::Exception::throwInvalidOperationName(this, operationName);
  }
  auto *component = dynamic_cast<::Smp::IDynamicInvocation *>(_parent);
  // check dynamic invocation
  if (!component) {
    ::Xsmp::Exception::throwInvalidOperationName(this, operationName);
  }
  // check parameter count
  if (static_cast<std::size_t>(request->GetParameterCount()) !=
      _parameters.size()) {
    ::Xsmp::Exception::throwInvalidParameterCount(this,
                                                  request->GetParameterCount());
  }
  // check input parameters type
  for (auto const *param : _parameters) {

    // ignore output parameters
    if (param->GetDirection() ==
        ::Smp::Publication::ParameterDirectionKind::PDK_Out) {
      continue;
    }
    auto kind =
        request->GetParameterValue(request->GetParameterIndex(param->GetName()))
            .GetType();
    if (kind != param->GetType()->GetPrimitiveTypeKind()) {
      ::Xsmp::Exception::throwInvalidParameterType(
          this, operationName, param->GetName(), kind,
          param->GetType()->GetPrimitiveTypeKind());
    }
  }

  // invoke the request
  component->Invoke(request);
}

void Operation::DeleteRequest(::Smp::IRequest *request) { delete request; }

void Operation::Update(::Smp::String8 description,
                       ::Smp::ViewKind view) noexcept {
  _description = description;
  _view = view;
  _returnParameter = {};
  _parameters.clear();
}

} // namespace Xsmp::Publication
