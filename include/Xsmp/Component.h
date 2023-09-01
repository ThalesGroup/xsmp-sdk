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

#ifndef XSMP_COMPONENT_H_
#define XSMP_COMPONENT_H_

#include <Smp/ComponentStateKind.h>
#include <Smp/IDynamicInvocation.h>
#include <Smp/IField.h>
#include <Smp/ILinkingComponent.h>
#include <Smp/IOperation.h>
#include <Smp/IProperty.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Object.h>

namespace Smp {
class IAggregate;
class IEventProvider;
class IPublication;
class ISimulator;
namespace Services {
class ILinkRegistry;
class ILogger;
} // namespace Services
} // namespace Smp

namespace Xsmp {

class Component: public ::Xsmp::Object,
        public virtual ::Smp::ILinkingComponent,
        public virtual ::Smp::IDynamicInvocation {
public:
    Component(::Smp::String8 name, ::Smp::String8 description = "",
            ::Smp::IObject *parent = nullptr, ::Smp::ISimulator *simulator =
                    nullptr);
    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;

    ::Smp::ComponentStateKind GetState() const final;
    ::Smp::IField* GetField(::Smp::String8) const override;
    const ::Smp::FieldCollection* GetFields() const override;
    void Publish(::Smp::IPublication *receiver) override;
    void Configure(::Smp::Services::ILogger *logger,
            ::Smp::Services::ILinkRegistry* = nullptr) override;
    void Connect(::Smp::ISimulator *simulator) override;
    void Disconnect() override;
    ::Smp::IRequest* CreateRequest(::Smp::String8 operationName) override;
    [[noreturn]] void Invoke(::Smp::IRequest *request) override;
    void DeleteRequest(::Smp::IRequest *request) override;
    const ::Smp::PropertyCollection* GetProperties() const override;
    const ::Smp::OperationCollection* GetOperations() const override;
    void RemoveLinks(const Smp::IComponent *target) override;
    [[noreturn]] const ::Smp::Uuid& GetUuid() const override;
protected:
    inline ::Smp::ISimulator* GetSimulator() const noexcept {
        return _simulator;
    }

    class Helper;
private:

    void RemoveEventProviderLinks(::Smp::IEventProvider const *eventProvider,
            const ::Smp::IComponent *target) const noexcept;

    void RemoveAggregateLinks(::Smp::IAggregate const *aggregate,
            const ::Smp::IComponent *target) const noexcept;
    void RemoveFieldLinks(::Smp::IField *field,
            const ::Smp::IComponent *target) const noexcept;
    ::Smp::ISimulator *_simulator;
    ::Smp::IPublication *_publication = nullptr;
    ::Smp::ComponentStateKind _state = ::Smp::ComponentStateKind::CSK_Created;
};

} // namespace Xsmp
#endif // XSMP_COMPONENT_H_
