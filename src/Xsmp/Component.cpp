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

#include <Smp/IAggregate.h>
#include <Smp/IArrayField.h>
#include <Smp/IEventProvider.h>
#include <Smp/IPublication.h>
#include <Smp/IRequest.h>
#include <Smp/IStructureField.h>
#include <Xsmp/Component.h>
#include <Xsmp/EventSource.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Field.h>
#include <Xsmp/Reference.h>

namespace Xsmp {
Component::Component(::Smp::String8 name, ::Smp::String8 description,
        ::Smp::IObject *parent, ::Smp::ISimulator *simulator) :
        Object(name, description, parent), _simulator { simulator } {
}

::Smp::ComponentStateKind Component::GetState() const {
    return _state;
}

::Smp::IField* Component::GetField(::Smp::String8 fullName) const {
    return _publication ? _publication->GetField(fullName) : nullptr;
}

const ::Smp::FieldCollection* Component::GetFields() const {
    return _publication ? _publication->GetFields() : nullptr;
}

void Component::Publish(::Smp::IPublication *receiver) {
    if (_state != ::Smp::ComponentStateKind::CSK_Created) {
        ::Xsmp::Exception::throwInvalidComponentState(this, _state,
                ::Smp::ComponentStateKind::CSK_Created);
    }
    if (receiver == nullptr) {
        ::Xsmp::Exception::throwException(this, "NullPointerException", "",
                "the publication receiver is null");
    }

    _publication = receiver;
    _state = ::Smp::ComponentStateKind::CSK_Publishing;
}

void Component::Configure(::Smp::Services::ILogger*,
        ::Smp::Services::ILinkRegistry*) {
    if (_state != ::Smp::ComponentStateKind::CSK_Publishing) {
        ::Xsmp::Exception::throwInvalidComponentState(this, _state,
                ::Smp::ComponentStateKind::CSK_Publishing);
    }

    _state = ::Smp::ComponentStateKind::CSK_Configured;
}

void Component::Connect(::Smp::ISimulator *simulator) {
    if (_state != ::Smp::ComponentStateKind::CSK_Configured) {
        ::Xsmp::Exception::throwInvalidComponentState(this, _state,
                ::Smp::ComponentStateKind::CSK_Configured);
    }
    _state = ::Smp::ComponentStateKind::CSK_Connected;

    _simulator = simulator;
}

void Component::Disconnect() {
    if (_state != ::Smp::ComponentStateKind::CSK_Connected) {
        ::Xsmp::Exception::throwInvalidComponentState(this, _state,
                ::Smp::ComponentStateKind::CSK_Connected);
    }

    _state = ::Smp::ComponentStateKind::CSK_Disconnected;

    _publication->Unpublish();
    _publication = nullptr;
    _simulator = nullptr;
}

void Component::Invoke(::Smp::IRequest *request) {
    ::Xsmp::Exception::throwInvalidOperationName(this,
            request ? request->GetOperationName() : "");
}

::Smp::IRequest* Component::CreateRequest(::Smp::String8 operationName) {
    return _publication ? _publication->CreateRequest(operationName) : nullptr;
}

void Component::DeleteRequest(::Smp::IRequest *request) {
    if (_publication)
        _publication->DeleteRequest(request);
}

const ::Smp::PropertyCollection* Component::GetProperties() const {
    return _publication ? _publication->GetProperties() : nullptr;
}

const ::Smp::OperationCollection* Component::GetOperations() const {
    return _publication ? _publication->GetOperations() : nullptr;
}

const ::Smp::Uuid& Component::GetUuid() const {
    Xsmp::Exception::throwException(this, "NotImplemented", "",
            "GetUuid is not implemented");
}

void Component::RemoveEventProviderLinks(
        ::Smp::IEventProvider const *eventProvider,
        const ::Smp::IComponent *target) const noexcept {
    if (const auto *eventSources = eventProvider->GetEventSources()) {
        for (auto *eventSource : *eventSources) {
            // we can disconnect only AbstractEventSource
            if (auto *eso =
                    dynamic_cast<detail::AbstractEventSource*>(eventSource))
                eso->RemoveLinks(target);
        }
    }
}

void Component::RemoveAggregateLinks(::Smp::IAggregate const *aggregate,
        const ::Smp::IComponent *target) const noexcept {
    if (const auto *references = aggregate->GetReferences()) {
        for (auto *reference : *references) {
            // we can disconnect only AbstractReference
            if (auto *ref = dynamic_cast<detail::AbstractReference*>(reference))
                ref->RemoveLinks(target);
        }
    }
}
void Component::RemoveFieldLinks(::Smp::IField *field,
        const ::Smp::IComponent *target) const noexcept {

    if (auto *dff = dynamic_cast<detail::DataflowField*>(field))
        dff->RemoveLinks(target);

    if (auto const *af = dynamic_cast<::Smp::IArrayField*>(field)) {
        for (::Smp::UInt64 i = 0; i < af->GetSize(); ++i)
            RemoveFieldLinks(af->GetItem(i), target);
    }
    if (auto const *sf = dynamic_cast<::Smp::IStructureField*>(field)) {
        for (auto *nf : *sf->GetFields())
            RemoveFieldLinks(nf, target);
    }

}

void Component::RemoveLinks(const ::Smp::IComponent *target) {

    // disconnect event sources
    if (auto const *eventProvider = dynamic_cast<::Smp::IEventProvider*>(this))
        RemoveEventProviderLinks(eventProvider, target);

    // disconnect references
    if (auto const *aggregate = dynamic_cast<::Smp::IAggregate*>(this))
        RemoveAggregateLinks(aggregate, target);

    // disconnect fields
    if (auto const *fields = GetFields())
        for (auto *field : *fields)
            RemoveFieldLinks(field, target);
}

} // namespace Xsmp
