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
#include <Smp/IField.h>
#include <Smp/IObject.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimpleField.h>
#include <Smp/IStorageReader.h>
#include <Smp/IStorageWriter.h>
#include <Smp/IStructureField.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/ViewKind.h>
#include <Xsmp/Collection.h>
#include <Xsmp/Exception.h>
#include <Xsmp/FallibleModel.h>
#include <Xsmp/Field.h>
#include <Xsmp/Helper.h>
#include <algorithm>
#include <utility>

namespace Xsmp::detail {

class FieldHelper {
public:
  static bool CanConnect(SimpleConnectableField *output,
                         ::Smp::ISimpleField *input) {
    return output->_connectedFields.find(input) ==
           output->_connectedFields.end();
  }
  static void Connect(SimpleConnectableField *output,
                      ::Smp::ISimpleField *input) {
    output->_connectedFields.emplace(input);
    // push value
    input->SetValue(output->GetValue());
  }

  static void Disconnect(SimpleConnectableField *output,
                         ::Smp::ISimpleField *input) {
    if (auto it = output->_connectedFields.find(input);
        it != output->_connectedFields.end()) {
      output->_connectedFields.erase(it);
    }
  }
  static void Push(SimpleConnectableField *output) { output->internal_push(); }

  static bool CanConnect(SimpleArrayConnectableField *output,
                         ::Smp::ISimpleArrayField *input) {
    return output->_connectedFields.find(input) ==
           output->_connectedFields.end();
  }
  static void Connect(SimpleArrayConnectableField *output,
                      ::Smp::ISimpleArrayField *input) {
    output->_connectedFields.emplace(input);
    // push value
    const auto size = output->GetSize();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      input->SetValue(i, output->GetValue(i));
    }
  }
  static void Disconnect(SimpleArrayConnectableField *output,
                         ::Smp::ISimpleArrayField *input) {
    if (auto it = output->_connectedFields.find(input);
        it != output->_connectedFields.end()) {
      output->_connectedFields.erase(it);
    }
  }
  static void Push(SimpleArrayConnectableField *output) {
    const auto size = output->GetSize();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      output->internal_push(i);
    }
  }
  static bool CanConnect(::Smp::IArrayField *output,
                         ::Smp::IArrayField *input) {
    const auto size = output->GetSize();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      if (!CanConnect(output->GetItem(i), input->GetItem(i))) {
        return false;
      }
    }
    return true;
  }

  static void Connect(::Smp::IArrayField *output, ::Smp::IArrayField *input) {
    const auto size = output->GetSize();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      Connect(output->GetItem(i), input->GetItem(i));
    }
  }
  static void Disconnect(::Smp::IArrayField *output,
                         ::Smp::IArrayField *input) {
    const auto size = output->GetSize();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      Disconnect(output->GetItem(i), input->GetItem(i));
    }
  }
  static void Push(::Smp::IArrayField *output) {
    const auto size = output->GetSize();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      Push(output->GetItem(i));
    }
  }

  static bool CanConnect(ArrayDataflowField *output,
                         ::Smp::IArrayField *input) {
    return output->_connectedFields.find(input) ==
               output->_connectedFields.end() &&
           CanConnect(static_cast<::Smp::IArrayField *>(output), input);
  }
  static void Connect(ArrayDataflowField *output, ::Smp::IArrayField *input) {
    output->_connectedFields.emplace(input);
    Connect(static_cast<::Smp::IArrayField *>(output), input);
  }

  static void Disconnect(ArrayDataflowField *output,
                         ::Smp::IArrayField *input) {
    if (auto it = output->_connectedFields.find(input);
        it != output->_connectedFields.end()) {
      output->_connectedFields.erase(it);
      Disconnect(static_cast<::Smp::IArrayField *>(output), input);
    }
  }

  static bool CanConnect(::Smp::IStructureField *output,
                         ::Smp::IStructureField *input) {
    const auto *outputFields = output->GetFields();
    if (!outputFields) {
      return false;
    }
    const auto *inputFields = input->GetFields();
    if (!inputFields) {
      return false;
    }
    const ::Smp::UInt64 size = outputFields->size();
    if (size != inputFields->size()) {
      return false;
    }

    for (::Smp::UInt64 i = 0; i < size; ++i) {
      if (!CanConnect(outputFields->at(i), inputFields->at(i))) {
        return false;
      }
    }
    return true;
  }

  static void Connect(::Smp::IStructureField *output,
                      ::Smp::IStructureField *input) {
    const auto *outputFields = output->GetFields();
    const auto *inputFields = input->GetFields();
    const ::Smp::UInt64 size = outputFields->size();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      Connect(outputFields->at(i), inputFields->at(i));
    }
  }
  static void Disconnect(::Smp::IStructureField *output,
                         ::Smp::IStructureField *input) {
    const auto *outputFields = output->GetFields();
    const auto *inputFields = input->GetFields();
    const ::Smp::UInt64 size = outputFields->size();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      Disconnect(outputFields->at(i), inputFields->at(i));
    }
  }
  static void Push(::Smp::IStructureField *output) {
    const auto *outputFields = output->GetFields();
    const ::Smp::UInt64 size = outputFields->size();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      Push(outputFields->at(i));
    }
  }
  static bool CanConnect(StructureDataflowField *output,
                         ::Smp::IStructureField *input) {
    return output->_connectedFields.find(input) ==
               output->_connectedFields.end() &&
           CanConnect(static_cast<::Smp::IStructureField *>(output), input);
  }
  static void Connect(StructureDataflowField *output,
                      ::Smp::IStructureField *input) {
    output->_connectedFields.emplace(input);
    Connect(static_cast<::Smp::IStructureField *>(output), input);
  }
  static void Disconnect(StructureDataflowField *output,
                         ::Smp::IStructureField *input) {
    if (auto it = output->_connectedFields.find(input);
        it != output->_connectedFields.end()) {
      output->_connectedFields.erase(it);
      Disconnect(static_cast<::Smp::IStructureField *>(output), input);
    }
  }
  static bool CanConnect(::Smp::IField *output, ::Smp::IField *input) {

    if (auto *simpleOutput = dynamic_cast<SimpleConnectableField *>(output)) {
      auto *simpleInput = dynamic_cast<::Smp::ISimpleField *>(input);
      return simpleInput && CanConnect(simpleOutput, simpleInput);
    }
    if (auto *simpleArrayOutput =
            dynamic_cast<SimpleArrayConnectableField *>(output)) {
      auto *simpleArrayInput = dynamic_cast<::Smp::ISimpleArrayField *>(input);
      return simpleArrayInput &&
             CanConnect(simpleArrayOutput, simpleArrayInput);
    }
    if (auto *arrayOutput = dynamic_cast<::Smp::IArrayField *>(output)) {
      auto *arrayInput = dynamic_cast<::Smp::IArrayField *>(input);
      if (!arrayInput) {
        return false;
      }
      if (auto *arrayDataflowOutput =
              dynamic_cast<ArrayDataflowField *>(output)) {
        return CanConnect(arrayDataflowOutput, arrayInput);
      }
      return CanConnect(arrayOutput, arrayInput);
    }
    if (auto *structOutput = dynamic_cast<::Smp::IStructureField *>(output)) {
      auto *structInput = dynamic_cast<::Smp::IStructureField *>(input);
      if (!structInput) {
        return false;
      }
      if (auto *structDataflowOutput =
              dynamic_cast<StructureDataflowField *>(output)) {
        return CanConnect(structDataflowOutput, structInput);
      }
      return CanConnect(structOutput, structInput);
    }
    return false;
  }
  static void Connect(::Smp::IField *output, ::Smp::IField *input) {
    if (auto *simpleOutput = dynamic_cast<SimpleConnectableField *>(output)) {
      Connect(simpleOutput, dynamic_cast<::Smp::ISimpleField *>(input));
    } else if (auto *simpleArrayOutput =
                   dynamic_cast<SimpleArrayConnectableField *>(output)) {
      Connect(simpleArrayOutput,
              dynamic_cast<::Smp::ISimpleArrayField *>(input));
    } else if (auto *arrayOutput = dynamic_cast<::Smp::IArrayField *>(output)) {
      if (auto *arrayDataflowOutput =
              dynamic_cast<ArrayDataflowField *>(output)) {
        Connect(arrayDataflowOutput, dynamic_cast<::Smp::IArrayField *>(input));
      } else {
        Connect(arrayOutput, dynamic_cast<::Smp::IArrayField *>(input));
      }
    } else if (auto *structOutput =
                   dynamic_cast<::Smp::IStructureField *>(output)) {
      if (auto *structDataflowOutput =
              dynamic_cast<StructureDataflowField *>(output)) {
        Connect(structDataflowOutput,
                dynamic_cast<::Smp::IStructureField *>(input));
      } else {
        Connect(structOutput, dynamic_cast<::Smp::IStructureField *>(input));
      }
    } else {
      // ignore
    }
  }
  static void Disconnect(::Smp::IField *output, ::Smp::IField *input) {

    if (auto *simpleOutput = dynamic_cast<SimpleConnectableField *>(output)) {
      Disconnect(simpleOutput, dynamic_cast<::Smp::ISimpleField *>(input));
    } else if (auto *simpleArrayOutput =
                   dynamic_cast<SimpleArrayConnectableField *>(output)) {
      Disconnect(simpleArrayOutput,
                 dynamic_cast<::Smp::ISimpleArrayField *>(input));
    } else if (auto *arrayOutput = dynamic_cast<::Smp::IArrayField *>(output)) {
      if (auto *arrayDataflowOutput =
              dynamic_cast<ArrayDataflowField *>(output)) {
        Disconnect(arrayDataflowOutput,
                   dynamic_cast<::Smp::IArrayField *>(input));
      } else {
        Disconnect(arrayOutput, dynamic_cast<::Smp::IArrayField *>(input));
      }
    } else if (auto *structOutput =
                   dynamic_cast<::Smp::IStructureField *>(output)) {
      if (auto *structDataflowOutput =
              dynamic_cast<StructureDataflowField *>(output)) {
        Disconnect(structDataflowOutput,
                   dynamic_cast<::Smp::IStructureField *>(input));
      } else {
        Disconnect(structOutput, dynamic_cast<::Smp::IStructureField *>(input));
      }
    } else {
      // ignore
    }
  }
  static void Push(::Smp::IField *output) {
    if (auto *simpleOutput = dynamic_cast<SimpleConnectableField *>(output)) {
      Push(simpleOutput);
    } else if (auto *simpleArrayOutput =
                   dynamic_cast<SimpleArrayConnectableField *>(output)) {
      Push(simpleArrayOutput);
    } else if (auto *arrayOutput = dynamic_cast<::Smp::IArrayField *>(output)) {
      Push(arrayOutput);
    } else if (auto *structOutput =
                   dynamic_cast<::Smp::IStructureField *>(output)) {
      Push(structOutput);
    } else {
      // ignore
    }
  }
};

AbstractField::AbstractField(::Smp::String8 name, ::Smp::String8 description,
                             ::Smp::IObject *parent, ::Smp::ViewKind view)
    : _name(::Xsmp::Helper::checkName(name, parent)), _description(description),
      _parent(parent), _view{view} {
  // auto register field in AbstractStructureField
  if (auto *structure = dynamic_cast<AbstractStructureField *>(parent)) {
    structure->AddField(*this);
  }
}
::Smp::String8 AbstractField::GetName() const { return _name.c_str(); }

::Smp::String8 AbstractField::GetDescription() const {
  return _description.c_str();
}

::Smp::IObject *AbstractField::GetParent() const { return _parent; }

Smp::ViewKind AbstractField::GetView() const { return _view; }

::Smp::Bool Failure::IsFailed() const { return _failed; }

void Failure::Register() {
  if (auto *model = dynamic_cast<::Xsmp::FallibleModel *>(this->GetParent())) {
    model->AddFailure(this);
  }
}

void Failure::Unfail() { _failed = false; }

void Failure::Fail() { _failed = true; }

void Failure::Restore(::Smp::IStorageReader *reader) {
  reader->Restore(&_failed, sizeof(_failed));
}

void Failure::Store(::Smp::IStorageWriter *writer) {
  writer->Store(&_failed, sizeof(_failed));
}

void SimpleConnectableField::internal_push() const {
  for (auto *field : _connectedFields) {
    field->SetValue(this->GetValue());
  }
}

void SimpleDataflowField::Connect(::Smp::IField *target) {
  if (!this->IsOutput() || !target->IsInput() || target == this ||
      !::Xsmp::Helper::AreEquivalent(this, target)) {
    ::Xsmp::Exception::throwInvalidTarget(this, this, target);
  }
  auto *simpleInput = dynamic_cast<::Smp::ISimpleField *>(target);
  if (!FieldHelper::CanConnect(this, simpleInput)) {
    ::Xsmp::Exception::throwFieldAlreadyConnected(this, this, target);
  }
  FieldHelper::Connect(this, simpleInput);
}

void SimpleDataflowField::Disconnect(::Smp::IField *target) {
  if (auto *simpleInput = dynamic_cast<::Smp::ISimpleField *>(target)) {
    FieldHelper::Disconnect(this, simpleInput);
  }
}
void SimpleDataflowField::Push() { FieldHelper::Push(this); }

void SimpleArrayDataflowField::Push() { FieldHelper::Push(this); }

void SimpleArrayDataflowField::Connect(::Smp::IField *target) {
  if (!this->IsOutput() || !target->IsInput() || target == this ||
      !::Xsmp::Helper::AreEquivalent(this, target)) {
    ::Xsmp::Exception::throwInvalidTarget(this, this, target);
  }
  auto *simpleArrayInput = dynamic_cast<::Smp::ISimpleArrayField *>(target);
  if (!FieldHelper::CanConnect(this, simpleArrayInput)) {
    ::Xsmp::Exception::throwFieldAlreadyConnected(this, this, target);
  }
  FieldHelper::Connect(this, simpleArrayInput);
}

void SimpleArrayDataflowField::Disconnect(::Smp::IField *target) {

  if (auto *simpleArrayInput =
          dynamic_cast<::Smp::ISimpleArrayField *>(target)) {
    FieldHelper::Disconnect(this, simpleArrayInput);
  }
}

void ArrayDataflowField::Push() { FieldHelper::Push(this); }

void ArrayDataflowField::Connect(::Smp::IField *target) {
  if (!this->IsOutput() || !target->IsInput() || target == this ||
      !::Xsmp::Helper::AreEquivalent(this, target)) {
    ::Xsmp::Exception::throwInvalidTarget(this, this, target);
  }
  auto *arrayInput = dynamic_cast<::Smp::IArrayField *>(target);
  if (!FieldHelper::CanConnect(this, arrayInput)) {
    ::Xsmp::Exception::throwFieldAlreadyConnected(this, this, target);
  }
  FieldHelper::Connect(this, arrayInput);
}

void ArrayDataflowField::Disconnect(::Smp::IField *target) {
  if (auto *arrayInput = dynamic_cast<::Smp::IArrayField *>(target)) {
    FieldHelper::Disconnect(this, arrayInput);
  }
}

void StructureDataflowField::Push() { FieldHelper::Push(this); }
void StructureDataflowField::Connect(::Smp::IField *target) {
  if (!this->IsOutput() || !target->IsInput() || target == this ||
      !::Xsmp::Helper::AreEquivalent(this, target)) {
    ::Xsmp::Exception::throwInvalidTarget(this, this, target);
  }
  auto *structInput = dynamic_cast<::Smp::IStructureField *>(target);
  if (!FieldHelper::CanConnect(this, structInput)) {
    ::Xsmp::Exception::throwFieldAlreadyConnected(this, this, target);
  }
  FieldHelper::Connect(this, structInput);
}

void StructureDataflowField::Disconnect(::Smp::IField *target) {
  if (auto *structInput = dynamic_cast<::Smp::IStructureField *>(target)) {
    FieldHelper::Disconnect(this, structInput);
  }
}

void SimpleArrayConnectableField::internal_push(::Smp::UInt64 index) const {
  for (auto *field : _connectedFields) {
    field->SetValue(index, this->GetValue(index));
  }
}

void AbstractForcibleField::Restore(::Smp::IStorageReader *reader) {
  reader->Restore(&_forced, sizeof(_forced));
  // restore of value performed in ForcibleField
}

void AbstractForcibleField::Store(::Smp::IStorageWriter *writer) {
  writer->Store(&_forced, sizeof(_forced));
  // store of value performed in ForcibleField
}

::Smp::Bool AbstractForcibleField::IsForced() { return _forced; }

void AbstractForcibleField::Force(::Smp::AnySimple value) {
  _forced = false;
  this->SetValue(std::move(value));
  _forced = true;
}

void AbstractForcibleField::Unforce() { _forced = false; }

void AbstractForcibleField::Freeze() { _forced = true; }

AbstractStructureField::AbstractStructureField()
    : _fields("Fields", "", this) {}

const ::Smp::FieldCollection *AbstractStructureField::GetFields() const {
  return &_fields;
}

Collection<::Smp::IField> *AbstractStructureField::GetFields() {
  return &_fields;
}

::Smp::IField *AbstractStructureField::GetField(::Smp::String8 name) const {
  return _fields.at(name);
}

void AbstractStructureField::Restore(::Smp::IStorageReader *reader) {
  std::for_each(_fields.begin(), _fields.end(),
                [reader](auto *field) { field->Restore(reader); });
}

void AbstractStructureField::Store(::Smp::IStorageWriter *writer) {
  std::for_each(_fields.begin(), _fields.end(),
                [writer](Smp::IField *field) { field->Store(writer); });
}

} // namespace Xsmp::detail
