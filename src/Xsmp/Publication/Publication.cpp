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
#include <Smp/IComponent.h>
#include <Smp/IDynamicInvocation.h>
#include <Smp/IField.h>
#include <Smp/IOperation.h>
#include <Smp/IProperty.h>
#include <Smp/IPublication.h>
#include <Smp/IRequest.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Uuid.h>
#include <Smp/ViewKind.h>
#include <Xsmp/Collection.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Publication/Field.h>
#include <Xsmp/Publication/Operation.h>
#include <Xsmp/Publication/Property.h>
#include <Xsmp/Publication/Publication.h>
#include <cstring>

namespace Xsmp::Publication {

Publication::Publication(::Smp::IObject *parent,
                         ::Smp::Publication::ITypeRegistry *typeRegistry)
    : _parent(parent), _typeRegistry{typeRegistry},
      _fields{"Fields", "", parent}, _allFields{&_fields},
      _properties{"Properties", "", parent}, _allProperties{&_properties},
      _operations{"Operations", "", parent}, _allOperations{&_operations} {}

::Smp::Publication::ITypeRegistry *Publication::GetTypeRegistry() const {
  return _typeRegistry;
}

::Smp::IField *Publication::PublishField(::Smp::String8 name,
                                         ::Smp::String8 description,
                                         ::Smp::Char8 *address,
                                         ::Smp::ViewKind view,
                                         ::Smp::Bool state, ::Smp::Bool input,
                                         ::Smp::Bool output) {
  return PublishField(name, description, address, ::Smp::Uuids::Uuid_Char8,
                      view, state, input, output);
}

::Smp::IField *Publication::PublishField(::Smp::String8 name,
                                         ::Smp::String8 description,
                                         ::Smp::Bool *address,
                                         ::Smp::ViewKind view,
                                         ::Smp::Bool state, ::Smp::Bool input,
                                         ::Smp::Bool output) {
  return PublishField(name, description, address, ::Smp::Uuids::Uuid_Bool, view,
                      state, input, output);
}

::Smp::IField *Publication::PublishField(::Smp::String8 name,
                                         ::Smp::String8 description,
                                         ::Smp::Int8 *address,
                                         ::Smp::ViewKind view,
                                         ::Smp::Bool state, ::Smp::Bool input,
                                         ::Smp::Bool output) {
  return PublishField(name, description, address, ::Smp::Uuids::Uuid_Int8, view,
                      state, input, output);
}

::Smp::IField *Publication::PublishField(::Smp::String8 name,
                                         ::Smp::String8 description,
                                         ::Smp::Int16 *address,
                                         ::Smp::ViewKind view,
                                         ::Smp::Bool state, ::Smp::Bool input,
                                         ::Smp::Bool output) {
  return PublishField(name, description, address, ::Smp::Uuids::Uuid_Int16,
                      view, state, input, output);
}

::Smp::IField *Publication::PublishField(::Smp::String8 name,
                                         ::Smp::String8 description,
                                         ::Smp::Int32 *address,
                                         ::Smp::ViewKind view,
                                         ::Smp::Bool state, ::Smp::Bool input,
                                         ::Smp::Bool output) {
  return PublishField(name, description, address, ::Smp::Uuids::Uuid_Int32,
                      view, state, input, output);
}

::Smp::IField *Publication::PublishField(::Smp::String8 name,
                                         ::Smp::String8 description,
                                         ::Smp::Int64 *address,
                                         ::Smp::ViewKind view,
                                         ::Smp::Bool state, ::Smp::Bool input,
                                         ::Smp::Bool output) {
  return PublishField(name, description, address, ::Smp::Uuids::Uuid_Int64,
                      view, state, input, output);
}

::Smp::IField *Publication::PublishField(::Smp::String8 name,
                                         ::Smp::String8 description,
                                         ::Smp::UInt8 *address,
                                         ::Smp::ViewKind view,
                                         ::Smp::Bool state, ::Smp::Bool input,
                                         ::Smp::Bool output) {
  return PublishField(name, description, address, ::Smp::Uuids::Uuid_UInt8,
                      view, state, input, output);
}

::Smp::IField *Publication::PublishField(::Smp::String8 name,
                                         ::Smp::String8 description,
                                         ::Smp::UInt16 *address,
                                         ::Smp::ViewKind view,
                                         ::Smp::Bool state, ::Smp::Bool input,
                                         ::Smp::Bool output) {
  return PublishField(name, description, address, ::Smp::Uuids::Uuid_UInt16,
                      view, state, input, output);
}

::Smp::IField *Publication::PublishField(::Smp::String8 name,
                                         ::Smp::String8 description,
                                         ::Smp::UInt32 *address,
                                         ::Smp::ViewKind view,
                                         ::Smp::Bool state, ::Smp::Bool input,
                                         ::Smp::Bool output) {
  return PublishField(name, description, address, ::Smp::Uuids::Uuid_UInt32,
                      view, state, input, output);
}

::Smp::IField *Publication::PublishField(::Smp::String8 name,
                                         ::Smp::String8 description,
                                         ::Smp::UInt64 *address,
                                         ::Smp::ViewKind view,
                                         ::Smp::Bool state, ::Smp::Bool input,
                                         ::Smp::Bool output) {
  return PublishField(name, description, address, ::Smp::Uuids::Uuid_UInt64,
                      view, state, input, output);
}

::Smp::IField *Publication::PublishField(::Smp::String8 name,
                                         ::Smp::String8 description,
                                         ::Smp::Float32 *address,
                                         ::Smp::ViewKind view,
                                         ::Smp::Bool state, ::Smp::Bool input,
                                         ::Smp::Bool output) {
  return PublishField(name, description, address, ::Smp::Uuids::Uuid_Float32,
                      view, state, input, output);
}

::Smp::IField *Publication::PublishField(::Smp::String8 name,
                                         ::Smp::String8 description,
                                         ::Smp::Float64 *address,
                                         ::Smp::ViewKind view,
                                         ::Smp::Bool state, ::Smp::Bool input,
                                         ::Smp::Bool output) {
  return PublishField(name, description, address, ::Smp::Uuids::Uuid_Float64,
                      view, state, input, output);
}

::Smp::IField *Publication::PublishField(::Smp::String8 name,
                                         ::Smp::String8 description,
                                         void *address, ::Smp::Uuid typeUuid,
                                         ::Smp::ViewKind view,
                                         ::Smp::Bool state, ::Smp::Bool input,
                                         ::Smp::Bool output) {
  auto const *type = _typeRegistry->GetType(typeUuid);
  if (!type) {
    ::Xsmp::Exception::throwTypeNotRegistered(_parent, typeUuid);
  }
  CheckName(name);
  return Register(_fields.Add(Field::Create(name, description, _parent, address,
                                            type, view, state, input, output)),
                  &_allFields);
}

void Publication::PublishField(::Smp::IField *field) {
  Register(field, &_allFields);
  _allFields.Add(field);
}

::Smp::ISimpleArrayField *Publication::PublishArray(
    ::Smp::String8 name, ::Smp::String8 description, ::Smp::Int64 count,
    void *address, ::Smp::PrimitiveTypeKind type, ::Smp::ViewKind view,
    ::Smp::Bool state, ::Smp::Bool input, ::Smp::Bool output) {
  CheckName(name);
  if (output) {
    return Register(_fields.Add<AnonymousSimpleArrayOutputField>(
                        name, description, _parent, count, address,
                        _typeRegistry->GetType(type), view, state, input,
                        output),
                    &_allFields);
  }
  return Register(_fields.Add<AnonymousSimpleArrayField>(
                      name, description, _parent, count, address,
                      _typeRegistry->GetType(type), view, state, input, output),
                  &_allFields);
}

::Smp::Publication::IPublishField *
Publication::PublishArray(::Smp::String8 name, ::Smp::String8 description,
                          ::Smp::ViewKind view, ::Smp::Bool state) {
  CheckName(name);
  return Register(_fields.Add<AnonymousArrayField>(name, description, _parent,
                                                   _typeRegistry, view, state),
                  &_allFields);
}

::Smp::Publication::IPublishField *
Publication::PublishStructure(::Smp::String8 name, ::Smp::String8 description,
                              ::Smp::ViewKind view, ::Smp::Bool state) {
  CheckName(name);
  return Register(_fields.Add<AnonymousStructureField>(
                      name, description, _parent, _typeRegistry, view, state),
                  &_allFields);
}

namespace {
/// SMP 2025 only allows publishing an operation or a property for a component
/// that can be invoked dynamically.
void CheckDynamicInvocation(const ::Smp::IObject *sender,
                            ::Smp::IObject *owner) {
  if (!dynamic_cast<::Smp::IDynamicInvocation *>(owner)) {
    ::Xsmp::Exception::throwNoDynamicInvocation(
        sender, dynamic_cast<::Smp::IComponent *>(owner));
  }
}
} // namespace

::Smp::Publication::IPublishOperation *
Publication::PublishOperation(::Smp::String8 name, ::Smp::String8 description,
                              ::Smp::ViewKind view) {
  CheckDynamicInvocation(_parent, _parent);
  auto *operation = dynamic_cast<Operation *>(_operations.at(name));
  if (operation) {
    operation->Update(description, view);
  } else {
    CheckName(name);
    operation = Register(_operations.Add<Operation>(name, description, _parent,
                                                    view, _typeRegistry),
                         &_allOperations);
  }
  return operation;
}

void Publication::PublishOperation(::Smp::IOperation *operation) {
  CheckDynamicInvocation(_parent, operation ? operation->GetParent() : nullptr);
  Register(operation, &_allOperations);
  _allOperations.Add(operation);
}

::Smp::IProperty *Publication::PublishProperty(::Smp::String8 name,
                                               ::Smp::String8 description,
                                               ::Smp::Uuid typeUuid,
                                               ::Smp::AccessKind accessKind,
                                               ::Smp::ViewKind view) {
  CheckDynamicInvocation(_parent, _parent);
  auto *type = GetTypeRegistry()->GetType(typeUuid);
  if (!type) {
    ::Xsmp::Exception::throwTypeNotRegistered(_parent, typeUuid);
  }
  // a property is accessed through an AnySimple, so its type must be simple
  if (type->GetPrimitiveTypeKind() == ::Smp::PrimitiveTypeKind::PTK_None) {
    ::Xsmp::Exception::throwInvalidType(_parent, type);
  }
  if (auto *property = dynamic_cast<Property *>(_properties.at(name))) {
    property->Update(description, type, accessKind, view);
    return property;
  }
  CheckName(name);
  return Register(_properties.Add<Property>(name, description, _parent, type,
                                            accessKind, view),
                  &_allProperties);
}

void Publication::PublishProperty(::Smp::IProperty *property) {
  CheckDynamicInvocation(_parent, property ? property->GetParent() : nullptr);
  Register(property, &_allProperties);
  _allProperties.Add(property);
}

::Smp::IField *Publication::GetField(::Smp::String8 fullName) const {
  // SMP 2025 reports an unknown field name with nullptr; 2020 raised
  // InvalidFieldName
  if (!fullName) {
    return nullptr;
  }
  return dynamic_cast<::Smp::IField *>(
      ::Xsmp::Helper::Resolve(&_allFields, _parent, fullName));
}

const ::Smp::FieldCollection *Publication::GetFields() const {
  return &_allFields;
}

::Smp::IProperty *Publication::GetProperty(::Smp::String8 name) const {
  return _allProperties.at(name);
}

const ::Smp::PropertyCollection *Publication::GetProperties() const {
  return &_allProperties;
}

::Smp::IOperation *Publication::GetOperation(::Smp::String8 name) const {
  return _allOperations.at(name);
}

const ::Smp::OperationCollection *Publication::GetOperations() const {
  return &_allOperations;
}

::Smp::IRequest *Publication::CreateRequest(::Smp::String8 operationName) {
  if (operationName) {
    if (auto *operation = _operations.at(operationName)) {
      return operation->CreateRequest();
    }
    // fallback on property getter if any
    if (std::strncmp(operationName, "get_", 4) == 0) {
      return CreateGetRequest(operationName + 4);
    }
    // fallback on property setter if any
    if (std::strncmp(operationName, "set_", 4) == 0) {
      return CreateSetRequest(operationName + 4);
    }
  }
  return nullptr;
}

::Smp::IRequest *Publication::CreateGetRequest(::Smp::String8 propertyName) {
  if (const auto *property =
          dynamic_cast<const Property *>(_properties.at(propertyName))) {
    return property->CreateGetRequest();
  }
  return nullptr;
}

::Smp::IRequest *Publication::CreateSetRequest(::Smp::String8 propertyName) {
  if (const auto *property =
          dynamic_cast<const Property *>(_properties.at(propertyName))) {
    return property->CreateSetRequest();
  }
  return nullptr;
}

void Publication::DeleteRequest(::Smp::IRequest *request) {
  if (request) {
    if (auto *operation = _operations.at(request->GetName())) {
      operation->DeleteRequest(request);
    } else {
      // delete
      delete request;
    }
  }
}

void Publication::CheckName(::Smp::String8 name) const {
  if (const auto *component = dynamic_cast<const ::Smp::IComponent *>(_parent);
      component && component->GetChild(name)) {
    ::Xsmp::Exception::throwDuplicateName(_parent, name, _parent);
  }
}

template <typename T>
T *Publication::Register(T *element, const ::Smp::ICollectionBase *collection) {
  if (auto *component = dynamic_cast<::Smp::IComponent *>(_parent);
      component && !component->AddChild(element, collection)) {
    ::Xsmp::Exception::throwDuplicateName(_parent, element->GetName(), _parent);
  }
  return element;
}

void Publication::UnregisterChildren() {
  auto *component = dynamic_cast<::Smp::IComponent *>(_parent);
  if (!component) {
    return;
  }
  for (auto *field : _allFields) {
    component->RemoveChild(field, &_allFields);
  }
  for (auto *operation : _allOperations) {
    component->RemoveChild(operation, &_allOperations);
  }
  for (auto *property : _allProperties) {
    component->RemoveChild(property, &_allProperties);
  }
}

void Publication::Unpublish() {
  UnregisterChildren();
  _fields.clear();
  _allFields.clear();
  _operations.clear();
  _allOperations.clear();
  _properties.clear();
  _allProperties.clear();
}

} // namespace Xsmp::Publication
