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

#include <Xsmp/Exception.h>
#include <Xsmp/Factory.h>
#include <Xsmp/Object.h>
#include <Xsmp/Helper.h>
#include <algorithm>
#include <memory>
#include <string>

namespace Xsmp::Factory {

namespace {
class Factory final : public ::Xsmp::Object, public ::Smp::IFactory {
public:
    Factory(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::ISimulator *simulator, ::Smp::Uuid uuid,
            const std::type_info &type, _factory_instantiator_t &&callback) :
            Object(name, description, simulator), _uuid(uuid), _callback {
                    std::move(callback) }, _typeName(
                    Xsmp::Helper::demangle(type.name())) {
    }

    ~Factory() noexcept override = default;
    ::Smp::Uuid GetUuid() const override {
        return _uuid;
    }

    ::Smp::IComponent* CreateInstance(::Smp::String8 name,
            ::Smp::String8 description, ::Smp::IComposite *parent) override {
        auto instance = _callback(name, description, parent,
                dynamic_cast<::Smp::ISimulator*>(GetParent()));

        if (instance->GetUuid() != _uuid) {
            Xsmp::Exception::throwException(this, "InvalidFactory", "",
                    "The instance UUID ", instance->GetUuid(),
                    " does not match with the Factory UUID ", _uuid);
        }
        return instance.release();
    }
    void DeleteInstance(::Smp::IComponent *instance) override {
        delete instance;
    }
    ::Smp::String8 GetTypeName() const override {
        return _typeName.c_str();
    }
private:
    ::Smp::Uuid _uuid;
    _factory_instantiator_t _callback;
    std::string _typeName;
};
} //namespace

::Smp::IFactory* Create(::Smp::String8 name, ::Smp::String8 description,
        ::Smp::ISimulator *simulator, ::Smp::Uuid uuid,
        const std::type_info &type, _factory_instantiator_t &&callback) {
    return new Factory(name, description, simulator, uuid, type,
            std::move(callback));
}

} // namespace Xsmp::Factory
