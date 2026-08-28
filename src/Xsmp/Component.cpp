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
#include <Smp/AnySimpleArray.h>
#include <Smp/ComponentStateKind.h>
#include <Smp/IAggregate.h>
#include <Smp/IArrayField.h>
#include <Smp/ICollectionBase.h>
#include <Smp/IComposite.h>
#include <Smp/IContainer.h>
#include <Smp/IEntryPointPublisher.h>
#include <Smp/IEventConsumer.h>
#include <Smp/IEventProvider.h>
#include <Smp/IFailure.h>
#include <Smp/IFallibleModel.h>
#include <Smp/IField.h>
#include <Smp/IOperation.h>
#include <Smp/IProperty.h>
#include <Smp/IPublication.h>
#include <Smp/IRequest.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimpleField.h>
#include <Smp/IStructureField.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Uuid.h>
#include <Xsmp/Component.h>
#include <Xsmp/EventSource.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Field.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Reference.h>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace Xsmp {
Component::Component(::Smp::String8 name, ::Smp::String8 description,
                     ::Smp::IComposite *parent, ::Smp::ISimulator *simulator)
    : _name(::Xsmp::Helper::checkName(name, parent)), _description(description),
      _parent(parent), _simulator{simulator} {}
::Smp::String8 Component::GetName() const { return _name.c_str(); }

::Smp::String8 Component::GetDescription() const {
  return _description.c_str();
}

::Smp::IObject *Component::GetParent() const { return _parent; }

::Smp::ComponentStateKind Component::GetState() const { return _state; }

::Smp::IField *Component::GetField(::Smp::String8 fullName) const {
  return _publication ? _publication->GetField(fullName) : nullptr;
}

const ::Smp::FieldCollection *Component::GetFields() const {
  return _publication ? _publication->GetFields() : nullptr;
}

namespace {
/// The field accessors of a component report a bad path as an invalid field
/// name, whether the field does not exist at all or has the wrong shape.
template <typename T>
T *GetFieldAs(const ::Smp::IComponent *component, ::Smp::String8 fullName) {
  auto *field = dynamic_cast<T *>(component->GetField(fullName));
  if (!field) {
    ::Xsmp::Exception::throwInvalidFieldName(component, fullName);
  }
  return field;
}
} // namespace

::Smp::AnySimple Component::GetSimpleValue(::Smp::String8 fullName) const {
  return GetFieldAs<::Smp::ISimpleField>(this, fullName)->GetValue();
}

void Component::SetSimpleValue(::Smp::String8 fullName,
                               ::Smp::AnySimple value) {
  GetFieldAs<::Smp::ISimpleField>(this, fullName)->SetValue(value);
}

void Component::GetSimpleArrayValue(::Smp::String8 fullName,
                                    ::Smp::UInt64 length,
                                    ::Smp::AnySimple *values,
                                    ::Smp::UInt64 startIndex) const {
  GetFieldAs<::Smp::ISimpleArrayField>(this, fullName)
      ->GetValues(length, values, startIndex);
}

void Component::SetSimpleArrayValue(::Smp::String8 fullName,
                                    ::Smp::UInt64 length,
                                    ::Smp::AnySimpleArray values,
                                    ::Smp::UInt64 startIndex) {
  GetFieldAs<::Smp::ISimpleArrayField>(this, fullName)
      ->SetValues(length, values, startIndex);
}

::Smp::Bool Component::AddChild(::Smp::IObject *child,
                                const ::Smp::ICollectionBase *collection) {
  if (!child) {
    return false;
  }
  const auto it = _children.find(child->GetName());
  // the name is taken by another child, whichever collection holds it: the
  // registration of that child has to survive the rejection
  if (it != _children.end() && it->second.first != child) {
    return false;
  }
  auto &entry =
      it == _children.end() ? _children[child->GetName()] : it->second;
  entry.first = child;
  if (std::find(entry.second.begin(), entry.second.end(), collection) ==
      entry.second.end()) {
    entry.second.push_back(collection);
  }
  return true;
}

::Smp::Bool Component::RemoveChild(::Smp::IObject *child,
                                   const ::Smp::ICollectionBase *collection) {
  if (!child) {
    return false;
  }
  const auto it = _children.find(child->GetName());
  if (it == _children.end() || it->second.first != child) {
    return false;
  }
  // the child is only removed by a collection that registered it, and it
  // stays as long as another one still holds it
  auto &collections = it->second.second;
  const auto found =
      std::find(collections.begin(), collections.end(), collection);
  if (found == collections.end()) {
    return false;
  }
  collections.erase(found);
  if (collections.empty()) {
    _children.erase(it);
  }
  return true;
}

::Smp::IObject *
Component::IsChildInCollection(::Smp::String8 child,
                               const ::Smp::ICollectionBase *collection) const {
  if (!child) {
    return nullptr;
  }
  const auto it = _children.find(child);
  if (it == _children.end()) {
    return nullptr;
  }
  const auto &collections = it->second.second;
  return std::find(collections.begin(), collections.end(), collection) !=
                 collections.end()
             ? it->second.first
             : nullptr;
}

::Smp::IObject *Component::GetChild(::Smp::String8 name) const {
  if (!name || name[0] == '\0') {
    return nullptr;
  }
  // the children registered by the collections that take part in name
  // resolution
  if (const auto it = _children.find(name); it != _children.end()) {
    return it->second.first;
  }
  // the features the component publishes
  if (_publication) {
    if (const auto *fields = _publication->GetFields()) {
      if (auto *field = fields->at(name)) {
        return field;
      }
    }
    if (auto *operation = _publication->GetOperation(name)) {
      return operation;
    }
    if (auto *property = _publication->GetProperty(name)) {
      return property;
    }
  }
  // the features the optional component mechanisms own
  if (const auto *publisher =
          dynamic_cast<const ::Smp::IEntryPointPublisher *>(this)) {
    if (auto *entryPoint = publisher->GetEntryPoint(name)) {
      return entryPoint;
    }
  }
  if (const auto *consumer =
          dynamic_cast<const ::Smp::IEventConsumer *>(this)) {
    if (auto *eventSink = consumer->GetEventSink(name)) {
      return eventSink;
    }
  }
  if (const auto *provider =
          dynamic_cast<const ::Smp::IEventProvider *>(this)) {
    if (auto *eventSource = provider->GetEventSource(name)) {
      return eventSource;
    }
  }
  if (const auto *fallible =
          dynamic_cast<const ::Smp::IFallibleModel *>(this)) {
    if (auto *failure = fallible->GetFailure(name)) {
      return failure;
    }
  }
  // a container and the components it holds are children of the composite,
  // a reference is a child of the aggregate but the components it points to
  // are not
  if (const auto *composite = dynamic_cast<const ::Smp::IComposite *>(this)) {
    if (auto *container = composite->GetContainer(name)) {
      return container;
    }
    if (const auto *containers = composite->GetContainers()) {
      for (auto *container : *containers) {
        if (auto *component = container->GetComponent(name)) {
          return component;
        }
      }
    }
  }
  if (const auto *aggregate = dynamic_cast<const ::Smp::IAggregate *>(this)) {
    if (auto *reference = aggregate->GetReference(name)) {
      return reference;
    }
  }
  return nullptr;
}

void Component::Publish(::Smp::IPublication *receiver) {
  if (_state != ::Smp::ComponentStateKind::CSK_Created) {
    ::Xsmp::Exception::throwInvalidComponentState(
        this, _state, ::Smp::ComponentStateKind::CSK_Created);
  }
  if (!receiver) {
    ::Xsmp::Exception::throwException(this, "NullPointerException", "",
                                      "the publication receiver is null");
  }

  _publication = receiver;
  _state = ::Smp::ComponentStateKind::CSK_Publishing;
}

void Component::Configure(::Smp::Services::ILogger *,
                          ::Smp::Services::ILinkRegistry *) {
  if (_state != ::Smp::ComponentStateKind::CSK_Publishing) {
    ::Xsmp::Exception::throwInvalidComponentState(
        this, _state, ::Smp::ComponentStateKind::CSK_Publishing);
  }
  _state = ::Smp::ComponentStateKind::CSK_Configured;
}

void Component::Connect(::Smp::ISimulator *simulator) {
  if (_state != ::Smp::ComponentStateKind::CSK_Configured) {
    ::Xsmp::Exception::throwInvalidComponentState(
        this, _state, ::Smp::ComponentStateKind::CSK_Configured);
  }
  _state = ::Smp::ComponentStateKind::CSK_Connected;

  _simulator = simulator;
}

void Component::Disconnect() {
  if (_state != ::Smp::ComponentStateKind::CSK_Connected) {
    ::Xsmp::Exception::throwInvalidComponentState(
        this, _state, ::Smp::ComponentStateKind::CSK_Connected);
  }
  _state = ::Smp::ComponentStateKind::CSK_Disconnected;

  _simulator = nullptr;
}

void Component::Invoke(::Smp::IRequest *request) {
  ::Xsmp::Exception::throwInvalidOperationName(
      this, request ? request->GetName() : nullptr);
}

::Smp::IRequest *Component::CreateRequest(::Smp::String8 operationName) {
  auto *operation =
      _publication ? _publication->GetOperation(operationName) : nullptr;
  return operation ? operation->CreateRequest() : nullptr;
}

void Component::DeleteRequest(::Smp::IRequest *request) {
  if (auto *operation = request && _publication
                            ? _publication->GetOperation(request->GetName())
                            : nullptr) {
    operation->DeleteRequest(request);
  }
}

::Smp::IProperty *Component::GetProperty(::Smp::String8 name) const {
  return _publication ? _publication->GetProperty(name) : nullptr;
}

::Smp::IOperation *Component::GetOperation(::Smp::String8 name) const {
  return _publication ? _publication->GetOperation(name) : nullptr;
}

const ::Smp::PropertyCollection *Component::GetProperties() const {
  return _publication ? _publication->GetProperties() : nullptr;
}

const ::Smp::OperationCollection *Component::GetOperations() const {
  return _publication ? _publication->GetOperations() : nullptr;
}

const ::Smp::Uuid &Component::GetUuid() const {
  Xsmp::Exception::throwException(this, "NotImplemented", "",
                                  "GetUuid is not implemented");
}

void Component::RemoveEventProviderLinks(
    const ::Smp::IEventProvider *eventProvider,
    const ::Smp::IComponent *target) noexcept {
  if (const auto *eventSources = eventProvider->GetEventSources()) {
    for (auto *eventSource : *eventSources) {
      // we can disconnect only AbstractEventSource
      if (auto *eso =
              dynamic_cast<detail::AbstractEventSource *>(eventSource)) {
        eso->RemoveLinks(target);
      }
    }
  }
}
void Component::RemoveAggregateLinks(const ::Smp::IAggregate *aggregate,
                                     const ::Smp::IComponent *target) noexcept {
  if (const auto *references = aggregate->GetReferences()) {
    for (auto *reference : *references) {
      // we can disconnect only AbstractReference
      if (auto *ref = dynamic_cast<detail::AbstractReference *>(reference)) {
        ref->RemoveLinks(target);
      }
    }
  }
}
void Component::RemoveFieldLinks(::Smp::IField *field,
                                 const ::Smp::IComponent *target) noexcept {
  // disconnect a dataflow field
  if (auto *dataflowField =
          dynamic_cast<::Xsmp::detail::IOutputFieldExtension *>(field)) {
    if (const auto *inputFields = dataflowField->GetInputFields()) {
      // the fields are collected first: disconnecting one removes it from the
      // collection being iterated
      std::vector<::Smp::IField *> connectedToTarget;
      for (auto *inputField : *inputFields) {
        if (::Xsmp::Helper::IsAncestor(target, inputField)) {
          connectedToTarget.emplace_back(inputField);
        }
      }
      for (auto *inputField : connectedToTarget) {
        dataflowField->Disconnect(inputField);
      }
    }
  }
  // recursively disconnect subfields of a structure field
  else if (const auto *structureField =
               dynamic_cast<::Smp::IStructureField *>(field)) {
    if (const auto *fields = structureField->GetFields()) {
      for (auto *subField : *fields) {
        RemoveFieldLinks(subField, target);
      }
    }
  }
  // recursively disconnect items of an array field
  else if (const auto *arrayField = dynamic_cast<::Smp::IArrayField *>(field)) {
    const auto size = arrayField->GetSize();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      RemoveFieldLinks(arrayField->GetItem(i), target);
    }
  } else {
    // ignore: the field is a simple field or simple array field without links
  }
}

void Component::RemoveLinks(const ::Smp::IComponent *target) {

  // disconnect event sources
  if (auto const *eventProvider = dynamic_cast<::Smp::IEventProvider *>(this)) {
    RemoveEventProviderLinks(eventProvider, target);
  }
  // disconnect references
  if (auto const *aggregate = dynamic_cast<::Smp::IAggregate *>(this)) {
    RemoveAggregateLinks(aggregate, target);
  }
  // disconnect fields
  if (auto const *fields = GetFields()) {
    for (auto *field : *fields) {
      RemoveFieldLinks(field, target);
    }
  }
}

} // namespace Xsmp
