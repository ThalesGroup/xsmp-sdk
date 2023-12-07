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
#include <Smp/Publication/IType.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Uuid.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Publication/Operation.h>
#include <Xsmp/Publication/Request.h>
#include <cstddef>
#include <cstring>
#include <string_view>

namespace Xsmp::Publication {

Operation::Parameter::Parameter(::Smp::String8 name, ::Smp::String8 description,
        ::Smp::IObject *parent, ::Smp::Publication::IType *type,
        ::Smp::Publication::ParameterDirectionKind direction) :
        Object(name, description, parent), _type(type), _direction(direction) {
}

::Smp::Publication::IType* Operation::Parameter::GetType() const {
    return _type;
}

::Smp::Publication::ParameterDirectionKind Operation::Parameter::GetDirection() const {
    return _direction;
}

Operation::Operation(::Smp::String8 name, ::Smp::String8 description,
        ::Smp::IObject *parent, ::Smp::ViewKind view,
        ::Smp::Publication::ITypeRegistry *typeRegistry) :
        Object(name, description, parent), _parameters { "Parameters", "",
                parent }, _typeRegistry { typeRegistry }, _view(view) {

}

void Operation::PublishParameter(::Smp::String8 name,
        ::Smp::String8 description, ::Smp::Uuid typeUuid,
        ::Smp::Publication::ParameterDirectionKind direction) {

    auto *type = _typeRegistry->GetType(typeUuid);
    if (!type)
        ::Xsmp::Exception::throwTypeNotRegistered(this, typeUuid);

    if (direction == ::Smp::Publication::ParameterDirectionKind::PDK_Return) {
        if (_returnParameter)
            ::Xsmp::Exception::throwException(this,
                    "ReturnParameterAlreadyPublished",
                    "This Exception is thrown when trying to publish a return parameter.",
                    "The return parameter \'", _returnParameter->GetName(),
                    "` is already published.");
        _returnParameter = std::make_unique<Parameter>(name, description, this,
                type, direction);
    }
    else
        _parameters.Add<Parameter>(name, description, this, type, direction);

}

const ::Smp::ParameterCollection* Operation::GetParameters() const {
    return &_parameters;
}

::Smp::IParameter* Operation::GetParameter(::Smp::String8 name) const {

    if (auto *parameter = _parameters.at(name))
        return parameter;
    if (_returnParameter && std::strcmp(_returnParameter->GetName(), name) == 0)
        return _returnParameter.get();
    return nullptr;
}

::Smp::IParameter* Operation::GetReturnParameter() const {
    return _returnParameter.get();
}

::Smp::ViewKind Operation::GetView() const {
    return _view;
}

::Smp::IRequest* Operation::CreateRequest() {
    // create the request only if the operation is invokable
    for (auto const *param : _parameters) {
        if (param->GetType()->GetPrimitiveTypeKind()
                == ::Smp::PrimitiveTypeKind::PTK_None)
            return nullptr;
    }
    if (_returnParameter
            && _returnParameter->GetType()->GetPrimitiveTypeKind()
                    == ::Smp::PrimitiveTypeKind::PTK_None) {
        return nullptr;
    }
    return _requests.emplace(std::make_unique<Request>(this, _typeRegistry)).first->get();
}

void Operation::Invoke(::Smp::IRequest *request) {

    auto *component = dynamic_cast<::Smp::IDynamicInvocation*>(GetParent());

    // check operation is invokable and name is ok
    if (!component
            || std::string_view { GetName() } != request->GetOperationName())
        ::Xsmp::Exception::throwInvalidOperationName(this,
                request->GetOperationName());

    // check parameter count
    if (static_cast<std::size_t>(request->GetParameterCount())
            != _parameters.size())
        ::Xsmp::Exception::throwInvalidParameterCount(this,
                request->GetParameterCount());

    // check input parameters type
    for (auto const *param : _parameters) {

        // ignore output parameters
        if (param->GetDirection()
                == ::Smp::Publication::ParameterDirectionKind::PDK_Out)
            continue;

        auto kind = request->GetParameterValue(
                request->GetParameterIndex(param->GetName())).GetType();
        if (kind != param->GetType()->GetPrimitiveTypeKind())
            ::Xsmp::Exception::throwInvalidParameterType(this,
                    request->GetOperationName(), param->GetName(), kind,
                    param->GetType()->GetPrimitiveTypeKind());

    }

    // invoke the request
    component->Invoke(request);
}

void Operation::DeleteRequest(::Smp::IRequest *request) {
    std::unique_ptr<::Smp::IRequest> stale_ptr { request };
    _requests.erase(stale_ptr);
    stale_ptr.release();
}

void Operation::Update(::Smp::String8 description,::Smp::ViewKind view) noexcept {
    SetDescription(description);
    _view = view;
    _returnParameter = {};
    _parameters.clear();
}

} // namespace Xsmp::Publication

