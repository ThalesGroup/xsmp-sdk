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

#include <Smp/IArrayField.h>
#include <Smp/IComponent.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimpleField.h>
#include <Smp/IStructureField.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Request.h>
#include <cstddef>

namespace Xsmp {

void Request::setValue(::Smp::IComponent const *component,
                       ::Smp::IRequest *request, const std::string &name,
                       const ::Smp::AnySimple &value) {
  ::Smp::Int32 index = request->GetParameterIndex(name.c_str());
  if (index == -1)
    ::Xsmp::Exception::throwException(component, name,
                                      "No Parameter named " + name +
                                          " in Operation " +
                                          request->GetOperationName() + ".");

  request->SetParameterValue(index, value);
}

::Smp::AnySimple Request::getValue(::Smp::IComponent const *component,
                                   ::Smp::IRequest const *request,
                                   const std::string &name,
                                   ::Smp::PrimitiveTypeKind kind,
                                   bool useDefaultValue) {
  ::Smp::Int32 index = request->GetParameterIndex(name.c_str());
  if (index == -1) {
    if (useDefaultValue)
      return {};

    ::Xsmp::Exception::throwException(component, name,
                                      "No Parameter named " + name +
                                          " in Operation " +
                                          request->GetOperationName() + ".");
  }
  auto value = request->GetParameterValue(index);

  if (value.GetType() != kind)
    ::Xsmp::Exception::throwInvalidParameterType(
        component, request->GetOperationName(), name, value.GetType(), kind);

  return value;
}

void Request::extract(::Smp::IRequest *request, ::Smp::IField *field,
                      const std::string &name, bool ignoreMissingParameters) {

  // simple field
  if (auto *simple = dynamic_cast<::Smp::ISimpleField *>(field)) {
    auto index = request->GetParameterIndex(name.c_str());
    if (index == -1) {
      if (ignoreMissingParameters)
        return;
      ::Xsmp::Exception::throwException(field, name,
                                        "No Parameter named " + name +
                                            " in Operation " +
                                            request->GetOperationName() + ".");
    }
    simple->SetValue(request->GetParameterValue(index));
  }
  // simple array field
  else if (auto *simpleArray =
               dynamic_cast<::Smp::ISimpleArrayField *>(field)) {

    for (std::size_t i = 0; i < simpleArray->GetSize(); ++i) {
      std::string itemName = name + "[" + std::to_string(i) + "]";
      auto index = request->GetParameterIndex(itemName.c_str());
      if (index != -1)
        simpleArray->SetValue(i, request->GetParameterValue(index));
      else if (ignoreMissingParameters) {
        // ignore
      } else
        ::Xsmp::Exception::throwException(
            field, itemName,
            "No Parameter named " + itemName + " in Operation " +
                request->GetOperationName() + ".");
    }
  }
  // array field
  else if (auto const *array = dynamic_cast<::Smp::IArrayField *>(field)) {
    for (std::size_t i = 0; i < array->GetSize(); ++i)
      extract(request, array->GetItem(i), name + "[" + std::to_string(i) + "]",
              ignoreMissingParameters);
  }
  // structure field
  else if (auto const *structure =
               dynamic_cast<::Smp::IStructureField *>(field)) {

    for (auto *nestedField : *structure->GetFields())
      extract(request, nestedField, name + "." + nestedField->GetName(),
              ignoreMissingParameters);
  }
  // should not happen
  else {
    // ignore
  }
}
void Request::inject(::Smp::IRequest *request, ::Smp::IField *field,
                     const std::string &name) {

  // simple field
  if (auto const *simple = dynamic_cast<::Smp::ISimpleField *>(field)) {
    auto index = request->GetParameterIndex(name.c_str());
    if (index == -1)
      ::Xsmp::Exception::throwException(field, name,
                                        "No Parameter named " + name +
                                            " in Operation " +
                                            request->GetOperationName() + ".");
    request->SetParameterValue(index, simple->GetValue());
  }
  // simple array field
  else if (auto const *simpleArray =
               dynamic_cast<::Smp::ISimpleArrayField *>(field)) {
    for (std::size_t i = 0; i < simpleArray->GetSize(); ++i) {
      std::string itemName = name + "[" + std::to_string(i) + "]";
      auto index = request->GetParameterIndex(itemName.c_str());
      if (index == -1)
        ::Xsmp::Exception::throwException(
            field, itemName,
            "No Parameter named " + itemName + " in Operation " +
                request->GetOperationName() + ".");
      request->SetParameterValue(index, simpleArray->GetValue(i));
    }
  }
  // array field
  else if (auto const *array = dynamic_cast<::Smp::IArrayField *>(field)) {
    for (std::size_t i = 0; i < array->GetSize(); ++i)
      inject(request, array->GetItem(i), name + "[" + std::to_string(i) + "]");
  }
  // structure field
  else if (auto const *structure =
               dynamic_cast<::Smp::IStructureField *>(field)) {
    for (auto *nestedField : *structure->GetFields())
      inject(request, nestedField, name + "." + nestedField->GetName());
  }
  // should not happen
  else {
    // ignore
  }
}

} // namespace Xsmp
