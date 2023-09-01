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

#include <Smp/IDynamicInvocation.h>
#include <Smp/IRequest.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Publication/Property.h>
#include <string>

namespace Xsmp::Publication {

Property::Property(::Smp::String8 name, ::Smp::String8 description,
        ::Smp::IObject *parent, ::Smp::Publication::IType *type,
        ::Smp::AccessKind accessKind, ::Smp::ViewKind view) :
        Object(name, description, parent), _type(type), _accessKind(accessKind), _view(
                view) {

    if (auto *invoker = dynamic_cast<::Smp::IDynamicInvocation*>(parent)) {

        auto deleter = [invoker](::Smp::IRequest *request) {
            invoker->DeleteRequest(request);
        };
        _getter = request_ptr(
                invoker->CreateRequest((std::string("get_") + name).c_str()),
                deleter);
        _setter = request_ptr(
                invoker->CreateRequest((std::string("set_") + name).c_str()),
                deleter);

    }
}

::Smp::Publication::IType* Property::GetType() const {
    return _type;
}

::Smp::AccessKind Property::GetAccess() const {
    return _accessKind;
}

::Smp::ViewKind Property::GetView() const {
    return _view;
}

::Smp::AnySimple Property::GetValue() const {

    if (!_getter || _accessKind == ::Smp::AccessKind::AK_WriteOnly)
        ::Xsmp::Exception::throwException(this, "InvalidAccessKind", "",
                "InvalidAccess: ", _accessKind);

    // _getter is set only if the parent implements IDynamicInvocation
    dynamic_cast<::Smp::IDynamicInvocation*>(GetParent())->Invoke(
            _getter.get());

    return _getter->GetReturnValue();
}

void Property::SetValue(::Smp::AnySimple value) {
    if (!_setter || _accessKind == ::Smp::AccessKind::AK_ReadOnly)
        ::Xsmp::Exception::throwException(this, "InvalidAccessKind", "",
                "InvalidAccess: ", _accessKind);

    // assume one parameter at index 0
    _setter->SetParameterValue(0, std::move(value));

    // _setter is set only if the parent implements IDynamicInvocation
    dynamic_cast<::Smp::IDynamicInvocation*>(GetParent())->Invoke(
            _setter.get());

}

void Property::SetType(::Smp::Publication::IType *type) noexcept {
    _type = type;
}

void Property::SetAccess(::Smp::AccessKind accessKind) noexcept {
    _accessKind = accessKind;
}

void Property::SetView(::Smp::ViewKind view) noexcept {
    _view = view;
}

} // namespace Xsmp::Publication
