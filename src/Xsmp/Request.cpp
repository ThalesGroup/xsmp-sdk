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
#include <Smp/IArrayField.h>
#include <Smp/IComponent.h>
#include <Smp/IRequest.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimpleField.h>
#include <Smp/IStructureField.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Request.h>
#include <string>

namespace Xsmp {

void Request::setValue(::Smp::IComponent const *component,
                       ::Smp::IRequest *request, ::Smp::String8 name,
                       const ::Smp::AnySimple &value) {
  const auto index = request->GetParameterIndex(name);
  if (index == -1) {
    ::Xsmp::Exception::throwException(component, name,
                                      "No Parameter named " +
                                          std::string(name) + " in Operation " +
                                          request->GetOperationName() + ".");
  }
  request->SetParameterValue(index, value);
}

::Smp::AnySimple Request::getValue(::Smp::IComponent const *component,
                                   ::Smp::IRequest const *request,
                                   ::Smp::String8 name,
                                   ::Smp::PrimitiveTypeKind kind,
                                   bool useDefaultValue) {
  const auto index = request->GetParameterIndex(name);
  if (index == -1) {
    if (useDefaultValue) {
      return {};
    }
    ::Xsmp::Exception::throwException(component, name,
                                      "No Parameter named " +
                                          std::string(name) + " in Operation " +
                                          request->GetOperationName() + ".");
  }
  auto value = request->GetParameterValue(index);

  if (value.GetType() != kind) {
    ::Xsmp::Exception::throwInvalidParameterType(
        component, request->GetOperationName(), name, value.GetType(), kind);
  }
  return value;
}

void Request::extract(::Smp::IRequest *request, ::Smp::IField *field,
                      ::Smp::String8 name, bool ignoreMissingParameters) {
  // simple field
  if (auto *simple = dynamic_cast<::Smp::ISimpleField *>(field)) {
    const auto index = request->GetParameterIndex(name);
    if (index == -1) {
      if (ignoreMissingParameters) {
        return;
      }
      ::Xsmp::Exception::throwException(
          field, name,
          "No Parameter named " + std::string(name) + " in Operation " +
              request->GetOperationName() + ".");
    }
    simple->SetValue(request->GetParameterValue(index));
  }
  // simple array field
  else if (auto *simpleArray =
               dynamic_cast<::Smp::ISimpleArrayField *>(field)) {
    for (::Smp::UInt64 i = 0, size = simpleArray->GetSize(); i < size; ++i) {
      const std::string itemName =
          std::string(name) + "[" + std::to_string(i) + "]";
      const auto index = request->GetParameterIndex(itemName.c_str());
      if (index != -1) {
        simpleArray->SetValue(i, request->GetParameterValue(index));
      } else if (ignoreMissingParameters) {
        // ignore
      } else {
        ::Xsmp::Exception::throwException(
            field, itemName,
            "No Parameter named " + itemName + " in Operation " +
                request->GetOperationName() + ".");
      }
    }
  }
  // array field
  else if (auto const *array = dynamic_cast<::Smp::IArrayField *>(field)) {
    for (::Smp::UInt64 i = 0, size = array->GetSize(); i < size; ++i) {
      extract(request, array->GetItem(i),
              (std::string(name) + "[" + std::to_string(i) + "]").c_str(),
              ignoreMissingParameters);
    }
  }
  // structure field
  else if (auto const *structure =
               dynamic_cast<::Smp::IStructureField *>(field)) {

    for (auto *nestedField : *structure->GetFields()) {
      extract(request, nestedField,
              (std::string(name) + "." + nestedField->GetName()).c_str(),
              ignoreMissingParameters);
    }
  }
  // should not happen
  else {
    // ignore
  }
}
void Request::inject(::Smp::IRequest *request, ::Smp::IField *field,
                     ::Smp::String8 name) {

  // simple field
  if (auto const *simple = dynamic_cast<::Smp::ISimpleField *>(field)) {
    const auto index = request->GetParameterIndex(name);
    if (index == -1) {
      ::Xsmp::Exception::throwException(
          field, name,
          "No Parameter named " + std::string(name) + " in Operation " +
              request->GetOperationName() + ".");
    }
    request->SetParameterValue(index, simple->GetValue());
  }
  // simple array field
  else if (auto const *simpleArray =
               dynamic_cast<::Smp::ISimpleArrayField *>(field)) {
    for (::Smp::UInt64 i = 0, size = simpleArray->GetSize(); i < size; ++i) {
      const std::string itemName =
          std::string(name) + "[" + std::to_string(i) + "]";
      const auto index = request->GetParameterIndex(itemName.c_str());
      if (index == -1) {
        ::Xsmp::Exception::throwException(
            field, itemName,
            "No Parameter named " + itemName + " in Operation " +
                request->GetOperationName() + ".");
      }
      request->SetParameterValue(index, simpleArray->GetValue(i));
    }
  }
  // array field
  else if (auto const *array = dynamic_cast<::Smp::IArrayField *>(field)) {
    for (::Smp::UInt64 i = 0, size = array->GetSize(); i < size; ++i) {
      inject(request, array->GetItem(i),
             (std::string(name) + "[" + std::to_string(i) + "]").c_str());
    }
  }
  // structure field
  else if (auto const *structure =
               dynamic_cast<::Smp::IStructureField *>(field)) {
    for (auto *nestedField : *structure->GetFields()) {
      inject(request, nestedField,
             (std::string(name) + "." + nestedField->GetName()).c_str());
    }
  }
  // should not happen
  else {
    // ignore
  }
}

} // namespace Xsmp
