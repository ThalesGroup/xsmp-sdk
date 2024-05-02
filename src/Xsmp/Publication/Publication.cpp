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
      _properties{"Properties", "", parent},
      _operations{"Operations", "", parent} {}

::Smp::Publication::ITypeRegistry *Publication::GetTypeRegistry() const {
  return _typeRegistry;
}

void Publication::PublishField(::Smp::String8 name, ::Smp::String8 description,
                               ::Smp::Char8 *address, ::Smp::ViewKind view,
                               ::Smp::Bool state, ::Smp::Bool input,
                               ::Smp::Bool output) {

  PublishField(name, description, address, ::Smp::Uuids::Uuid_Char8, view,
               state, input, output);
}

void Publication::PublishField(::Smp::String8 name, ::Smp::String8 description,
                               ::Smp::Bool *address, ::Smp::ViewKind view,
                               ::Smp::Bool state, ::Smp::Bool input,
                               ::Smp::Bool output) {
  PublishField(name, description, address, ::Smp::Uuids::Uuid_Bool, view, state,
               input, output);
}

void Publication::PublishField(::Smp::String8 name, ::Smp::String8 description,
                               ::Smp::Int8 *address, ::Smp::ViewKind view,
                               ::Smp::Bool state, ::Smp::Bool input,
                               ::Smp::Bool output) {
  PublishField(name, description, address, ::Smp::Uuids::Uuid_Int8, view, state,
               input, output);
}

void Publication::PublishField(::Smp::String8 name, ::Smp::String8 description,
                               ::Smp::Int16 *address, ::Smp::ViewKind view,
                               ::Smp::Bool state, ::Smp::Bool input,
                               ::Smp::Bool output) {
  PublishField(name, description, address, ::Smp::Uuids::Uuid_Int16, view,
               state, input, output);
}

void Publication::PublishField(::Smp::String8 name, ::Smp::String8 description,
                               ::Smp::Int32 *address, ::Smp::ViewKind view,
                               ::Smp::Bool state, ::Smp::Bool input,
                               ::Smp::Bool output) {
  PublishField(name, description, address, ::Smp::Uuids::Uuid_Int32, view,
               state, input, output);
}

void Publication::PublishField(::Smp::String8 name, ::Smp::String8 description,
                               ::Smp::Int64 *address, ::Smp::ViewKind view,
                               ::Smp::Bool state, ::Smp::Bool input,
                               ::Smp::Bool output) {
  PublishField(name, description, address, ::Smp::Uuids::Uuid_Int64, view,
               state, input, output);
}

void Publication::PublishField(::Smp::String8 name, ::Smp::String8 description,
                               ::Smp::UInt8 *address, ::Smp::ViewKind view,
                               ::Smp::Bool state, ::Smp::Bool input,
                               ::Smp::Bool output) {
  PublishField(name, description, address, ::Smp::Uuids::Uuid_UInt8, view,
               state, input, output);
}

void Publication::PublishField(::Smp::String8 name, ::Smp::String8 description,
                               ::Smp::UInt16 *address, ::Smp::ViewKind view,
                               ::Smp::Bool state, ::Smp::Bool input,
                               ::Smp::Bool output) {
  PublishField(name, description, address, ::Smp::Uuids::Uuid_UInt16, view,
               state, input, output);
}

void Publication::PublishField(::Smp::String8 name, ::Smp::String8 description,
                               ::Smp::UInt32 *address, ::Smp::ViewKind view,
                               ::Smp::Bool state, ::Smp::Bool input,
                               ::Smp::Bool output) {
  PublishField(name, description, address, ::Smp::Uuids::Uuid_UInt32, view,
               state, input, output);
}

void Publication::PublishField(::Smp::String8 name, ::Smp::String8 description,
                               ::Smp::UInt64 *address, ::Smp::ViewKind view,
                               ::Smp::Bool state, ::Smp::Bool input,
                               ::Smp::Bool output) {
  PublishField(name, description, address, ::Smp::Uuids::Uuid_UInt64, view,
               state, input, output);
}

void Publication::PublishField(::Smp::String8 name, ::Smp::String8 description,
                               ::Smp::Float32 *address, ::Smp::ViewKind view,
                               ::Smp::Bool state, ::Smp::Bool input,
                               ::Smp::Bool output) {
  PublishField(name, description, address, ::Smp::Uuids::Uuid_Float32, view,
               state, input, output);
}

void Publication::PublishField(::Smp::String8 name, ::Smp::String8 description,
                               ::Smp::Float64 *address, ::Smp::ViewKind view,
                               ::Smp::Bool state, ::Smp::Bool input,
                               ::Smp::Bool output) {
  PublishField(name, description, address, ::Smp::Uuids::Uuid_Float64, view,
               state, input, output);
}

void Publication::PublishField(::Smp::String8 name, ::Smp::String8 description,
                               void *address, ::Smp::Uuid typeUuid,
                               ::Smp::ViewKind view, ::Smp::Bool state,
                               ::Smp::Bool input, ::Smp::Bool output) {

  if (auto const *type = _typeRegistry->GetType(typeUuid)) {
    _fields.Add(Field::Create(name, description, _parent, address, type, view,
                              state, input, output));
  } else {
    ::Xsmp::Exception::throwTypeNotRegistered(_parent, typeUuid);
  }
}

void Publication::PublishField(::Smp::IField *field) { _allFields.Add(field); }

void Publication::PublishArray(::Smp::String8 name, ::Smp::String8 description,
                               ::Smp::Int64 count, void *address,
                               ::Smp::PrimitiveTypeKind type,
                               ::Smp::ViewKind view, ::Smp::Bool state,
                               ::Smp::Bool input, ::Smp::Bool output) {

  if (type == ::Smp::PrimitiveTypeKind::PTK_None ||
      type == ::Smp::PrimitiveTypeKind::PTK_String8) {
    ::Xsmp::Exception::throwInvalidFieldType(_parent, type);
  }
  if (output) {
    _fields.Add<AnonymousSimpleArrayDataflowField>(name, description, _parent,
                                                   count, address, type, view,
                                                   state, input, output);
  } else {
    _fields.Add<AnonymousSimpleArrayField>(name, description, _parent, count,
                                           address, type, view, state, input,
                                           output);
  }
}

::Smp::IPublication *Publication::PublishArray(::Smp::String8 name,
                                               ::Smp::String8 description,
                                               ::Smp::ViewKind view,
                                               ::Smp::Bool state) {

  return _fields.Add<AnonymousArrayField>(name, description, _parent,
                                          _typeRegistry, view, state);
}

::Smp::IPublication *Publication::PublishStructure(::Smp::String8 name,
                                                   ::Smp::String8 description,
                                                   ::Smp::ViewKind view,
                                                   ::Smp::Bool state) {

  return _fields.Add<AnonymousStructureField>(name, description, _parent,
                                              _typeRegistry, view, state);
}

::Smp::Publication::IPublishOperation *
Publication::PublishOperation(::Smp::String8 name, ::Smp::String8 description,
                              ::Smp::ViewKind view) {

  auto *operation = dynamic_cast<Operation *>(_operations.at(name));
  if (operation) {
    operation->Update(description, view);
  } else {
    operation = _operations.Add<Operation>(name, description, _parent, view,
                                           _typeRegistry);
  }

  return operation;
}

void Publication::PublishProperty(::Smp::String8 name,
                                  ::Smp::String8 description,
                                  ::Smp::Uuid typeUuid,
                                  ::Smp::AccessKind accessKind,
                                  ::Smp::ViewKind view) {

  auto *type = GetTypeRegistry()->GetType(typeUuid);
  if (!type) {
    ::Xsmp::Exception::throwTypeNotRegistered(_parent, typeUuid);
  }
  if (auto *property = dynamic_cast<Property *>(_properties.at(name))) {
    property->Update(description, type, accessKind, view);
  } else {
    _properties.Add<Property>(name, description, _parent, type, accessKind,
                              view);
  }
}

::Smp::IField *Publication::GetField(::Smp::String8 fullName) const {
  if (fullName) {
    if (auto *field = dynamic_cast<::Smp::IField *>(
            ::Xsmp::Helper::Resolve(GetFields(), fullName))) {
      return field;
    }
  }
  ::Xsmp::Exception::throwInvalidFieldName(_parent, fullName);
}

const ::Smp::FieldCollection *Publication::GetFields() const {
  return &_allFields;
}

const ::Smp::PropertyCollection *Publication::GetProperties() const {
  return &_properties;
}

const ::Smp::OperationCollection *Publication::GetOperations() const {
  return &_operations;
}

::Smp::IRequest *Publication::CreateRequest(::Smp::String8 operationName) {
  if (operationName) {
    if (auto *operation = _operations.at(operationName)) {
      return operation->CreateRequest();
    }
    // check if property getter exist
    if (std::strncmp(operationName, "get_", 4) == 0) {
      if (const auto *property = dynamic_cast<const Property *>(
              _properties.at(operationName + 4))) {
        return property->CreateGetRequest();
      }
    }
    // check if property setter exist
    else if (std::strncmp(operationName, "set_", 4) == 0) {
      if (const auto *property = dynamic_cast<const Property *>(
              _properties.at(operationName + 4))) {
        return property->CreateSetRequest();
      }
    }
  }
  return nullptr;
}

void Publication::DeleteRequest(::Smp::IRequest *request) {
  if (request) {
    if (auto *operation = _operations.at(request->GetOperationName())) {
      operation->DeleteRequest(request);
    } else {
      // delete
      delete request;
    }
  }
}

void Publication::Unpublish() {
  _fields.clear();
  _allFields.clear();
  _operations.clear();
  _properties.clear();
}

} // namespace Xsmp::Publication
