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
#include <cstddef>
#include <string_view>
#include <utility>

namespace Xsmp::detail {

class FieldHelper {
public:
  static bool CanConnect(const SimpleConnectableField *output,
                         const ::Smp::ISimpleField *input) {
    return !output->_connectedFields.contains(input);
  }
  static void Connect(SimpleConnectableField *output,
                      ::Smp::ISimpleField *input) {
    output->_connectedFields.add(input);
    // push value
    input->SetValue(output->GetValue());
  }

  static bool Disconnect(SimpleConnectableField *output,
                         const ::Smp::ISimpleField *input) {
    return output->_connectedFields.remove(input);
  }
  static void Push(const SimpleConnectableField *output) {
    output->internal_push();
  }

  static bool CanConnect(const SimpleArrayConnectableField *output,
                         const ::Smp::ISimpleArrayField *input) {
    return !output->_connectedFields.contains(input);
  }
  static void Connect(SimpleArrayConnectableField *output,
                      ::Smp::ISimpleArrayField *input) {
    output->_connectedFields.add(input);
    // push value
    const auto size = output->GetSize();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      input->SetValue(i, output->GetValue(i));
    }
  }
  static bool Disconnect(SimpleArrayConnectableField *output,
                         const ::Smp::ISimpleArrayField *input) {
    return output->_connectedFields.remove(input);
  }
  static void Push(const SimpleArrayConnectableField *output) {
    const auto size = output->GetSize();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      output->internal_push(i);
    }
  }
  static bool CanConnect(const ::Smp::IArrayField *output,
                         const ::Smp::IArrayField *input) {
    const auto size = output->GetSize();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      if (!CanConnect(output->GetItem(i), input->GetItem(i))) {
        return false;
      }
    }
    return true;
  }

  static void Connect(const ::Smp::IArrayField *output,
                      const ::Smp::IArrayField *input) {
    const auto size = output->GetSize();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      Connect(output->GetItem(i), input->GetItem(i));
    }
  }
  static void Disconnect(const ::Smp::IArrayField *output,
                         const ::Smp::IArrayField *input) {
    const auto size = output->GetSize();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      Disconnect(output->GetItem(i), input->GetItem(i));
    }
  }
  static void Push(const ::Smp::IArrayField *output) {
    const auto size = output->GetSize();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      Push(output->GetItem(i));
    }
  }

  static bool CanConnect(const ArrayOutputField *output,
                         const ::Smp::IArrayField *input) {
    return !output->_connectedFields.contains(input) &&
           CanConnect(static_cast<const ::Smp::IArrayField *>(output), input);
  }
  static void Connect(ArrayOutputField *output, ::Smp::IArrayField *input) {
    output->_connectedFields.add(input);
    Connect(static_cast<const ::Smp::IArrayField *>(output), input);
  }

  static bool Disconnect(ArrayOutputField *output,
                         const ::Smp::IArrayField *input) {
    if (!output->_connectedFields.remove(input)) {
      return false;
    }
    Disconnect(static_cast<const ::Smp::IArrayField *>(output), input);
    return true;
  }

  static bool CanConnect(const ::Smp::IStructureField *output,
                         const ::Smp::IStructureField *input) {
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

  static void Connect(const ::Smp::IStructureField *output,
                      const ::Smp::IStructureField *input) {
    const auto *outputFields = output->GetFields();
    const auto *inputFields = input->GetFields();
    const ::Smp::UInt64 size = outputFields->size();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      Connect(outputFields->at(i), inputFields->at(i));
    }
  }
  static void Disconnect(const ::Smp::IStructureField *output,
                         const ::Smp::IStructureField *input) {
    const auto *outputFields = output->GetFields();
    const auto *inputFields = input->GetFields();
    const ::Smp::UInt64 size = outputFields->size();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      Disconnect(outputFields->at(i), inputFields->at(i));
    }
  }
  static void Push(const ::Smp::IStructureField *output) {
    const auto *outputFields = output->GetFields();
    const ::Smp::UInt64 size = outputFields->size();
    for (::Smp::UInt64 i = 0; i < size; ++i) {
      Push(outputFields->at(i));
    }
  }
  static bool CanConnect(const StructureOutputField *output,
                         const ::Smp::IStructureField *input) {
    return !output->_connectedFields.contains(input) &&
           CanConnect(static_cast<const ::Smp::IStructureField *>(output),
                      input);
  }
  static void Connect(StructureOutputField *output,
                      ::Smp::IStructureField *input) {
    output->_connectedFields.add(input);
    Connect(static_cast<const ::Smp::IStructureField *>(output), input);
  }
  static bool Disconnect(StructureOutputField *output,
                         const ::Smp::IStructureField *input) {
    if (!output->_connectedFields.remove(input)) {
      return false;
    }
    Disconnect(static_cast<const ::Smp::IStructureField *>(output), input);
    return true;
  }
  static bool CanConnect(const ::Smp::IField *output,
                         const ::Smp::IField *input) {

    if (auto *simpleOutput =
            dynamic_cast<const SimpleConnectableField *>(output)) {
      auto *simpleInput = dynamic_cast<const ::Smp::ISimpleField *>(input);
      return simpleInput && CanConnect(simpleOutput, simpleInput);
    }
    if (auto *simpleArrayOutput =
            dynamic_cast<const SimpleArrayConnectableField *>(output)) {
      auto *simpleArrayInput =
          dynamic_cast<const ::Smp::ISimpleArrayField *>(input);
      return simpleArrayInput &&
             CanConnect(simpleArrayOutput, simpleArrayInput);
    }
    if (auto *arrayOutput = dynamic_cast<const ::Smp::IArrayField *>(output)) {
      auto *arrayInput = dynamic_cast<const ::Smp::IArrayField *>(input);
      if (!arrayInput) {
        return false;
      }
      if (const auto *arrayOutputField =
              dynamic_cast<const ArrayOutputField *>(output)) {
        return CanConnect(arrayOutputField, arrayInput);
      }
      return CanConnect(arrayOutput, arrayInput);
    }
    if (auto *structOutput =
            dynamic_cast<const ::Smp::IStructureField *>(output)) {
      auto *structInput = dynamic_cast<const ::Smp::IStructureField *>(input);
      if (!structInput) {
        return false;
      }
      if (const auto *structOutputField =
              dynamic_cast<const StructureOutputField *>(output)) {
        return CanConnect(structOutputField, structInput);
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
    } else if (const auto *arrayOutput =
                   dynamic_cast<const ::Smp::IArrayField *>(output)) {
      if (auto *arrayOutputField = dynamic_cast<ArrayOutputField *>(output)) {
        Connect(arrayOutputField, dynamic_cast<::Smp::IArrayField *>(input));
      } else {
        Connect(arrayOutput, dynamic_cast<::Smp::IArrayField *>(input));
      }
    } else if (const auto *structOutput =
                   dynamic_cast<const ::Smp::IStructureField *>(output)) {
      if (auto *structOutputField =
              dynamic_cast<StructureOutputField *>(output)) {
        Connect(structOutputField,
                dynamic_cast<::Smp::IStructureField *>(input));
      } else {
        Connect(structOutput,
                dynamic_cast<const ::Smp::IStructureField *>(input));
      }
    } else {
      // ignore
    }
  }
  static void Disconnect(::Smp::IField *output, const ::Smp::IField *input) {

    if (auto *simpleOutput = dynamic_cast<SimpleConnectableField *>(output)) {
      Disconnect(simpleOutput,
                 dynamic_cast<const ::Smp::ISimpleField *>(input));
    } else if (auto *simpleArrayOutput =
                   dynamic_cast<SimpleArrayConnectableField *>(output)) {
      Disconnect(simpleArrayOutput,
                 dynamic_cast<const ::Smp::ISimpleArrayField *>(input));
    } else if (const auto *arrayOutput =
                   dynamic_cast<const ::Smp::IArrayField *>(output)) {
      if (auto *arrayOutputField = dynamic_cast<ArrayOutputField *>(output)) {
        Disconnect(arrayOutputField,
                   dynamic_cast<const ::Smp::IArrayField *>(input));
      } else {
        Disconnect(arrayOutput,
                   dynamic_cast<const ::Smp::IArrayField *>(input));
      }
    } else if (const auto *structOutput =
                   dynamic_cast<const ::Smp::IStructureField *>(output)) {
      if (auto *structOutputField =
              dynamic_cast<StructureOutputField *>(output)) {
        Disconnect(structOutputField,
                   dynamic_cast<const ::Smp::IStructureField *>(input));
      } else {
        Disconnect(structOutput,
                   dynamic_cast<const ::Smp::IStructureField *>(input));
      }
    } else {
      // ignore
    }
  }
  static void Push(const ::Smp::IField *output) {
    if (auto *simpleOutput =
            dynamic_cast<const SimpleConnectableField *>(output)) {
      Push(simpleOutput);
    } else if (auto *simpleArrayOutput =
                   dynamic_cast<const SimpleArrayConnectableField *>(output)) {
      Push(simpleArrayOutput);
    } else if (auto *arrayOutput =
                   dynamic_cast<const ::Smp::IArrayField *>(output)) {
      Push(arrayOutput);
    } else if (auto *structOutput =
                   dynamic_cast<const ::Smp::IStructureField *>(output)) {
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

::Smp::IObject *AbstractField::GetChild(::Smp::String8 name) const {
  return ::Xsmp::Helper::GetFieldChild(this, name);
}

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
SimpleConnectableField::SimpleConnectableField() = default;

void SimpleConnectableField::internal_push() const {
  // a connected field can push back to this one: propagate the value only once
  if (_pushing) {
    return;
  }
  _pushing = true;
  try {
    for (auto *field : _connectedFields) {
      dynamic_cast<::Smp::ISimpleField *>(field)->SetValue(this->GetValue());
    }
  } catch (...) {
    _pushing = false;
    throw;
  }
  _pushing = false;
}
const ::Smp::FieldCollection *SimpleConnectableField::GetInputFields() const {
  return &_connectedFields;
}

void SimpleOutputField::Connect(::Smp::IField *target) {
  auto *simpleInput = dynamic_cast<::Smp::ISimpleField *>(target);
  if (!this->IsOutput() || !target->IsInput() || target == this ||
      !simpleInput || !::Xsmp::Helper::AreEquivalent(this, target)) {
    ::Xsmp::Exception::throwInvalidTarget(this, this, target);
  }
  if (!FieldHelper::CanConnect(this, simpleInput)) {
    ::Xsmp::Exception::throwFieldAlreadyConnected(this, this, target);
  }
  FieldHelper::Connect(this, simpleInput);
}

void SimpleOutputField::Disconnect(::Smp::IField *target) {
  const auto *simpleInput = dynamic_cast<const ::Smp::ISimpleField *>(target);
  if (!simpleInput || !FieldHelper::Disconnect(this, simpleInput)) {
    ::Xsmp::Exception::throwFieldNotConnected(this, this, target);
  }
}
void SimpleOutputField::Push() { FieldHelper::Push(this); }

const ::Smp::FieldCollection *SimpleOutputField::GetInputFields() const {
  return SimpleConnectableField::GetInputFields();
}

void SimpleArrayOutputField::Push() { FieldHelper::Push(this); }

void SimpleArrayOutputField::Connect(::Smp::IField *target) {
  auto *simpleArrayInput = dynamic_cast<::Smp::ISimpleArrayField *>(target);
  if (!this->IsOutput() || !target->IsInput() || target == this ||
      !simpleArrayInput || !::Xsmp::Helper::AreEquivalent(this, target)) {
    ::Xsmp::Exception::throwInvalidTarget(this, this, target);
  }
  if (!FieldHelper::CanConnect(this, simpleArrayInput)) {
    ::Xsmp::Exception::throwFieldAlreadyConnected(this, this, target);
  }
  FieldHelper::Connect(this, simpleArrayInput);
}

void SimpleArrayOutputField::Disconnect(::Smp::IField *target) {
  const auto *simpleArrayInput =
      dynamic_cast<const ::Smp::ISimpleArrayField *>(target);
  if (!simpleArrayInput || !FieldHelper::Disconnect(this, simpleArrayInput)) {
    ::Xsmp::Exception::throwFieldNotConnected(this, this, target);
  }
}

const ::Smp::FieldCollection *SimpleArrayOutputField::GetInputFields() const {
  return SimpleArrayConnectableField::GetInputFields();
}

::Smp::IField *InputFieldCollection::at(::Smp::String8 name) const {
  std::string_view svName = name;
  auto it = std::find_if(_fields.begin(), _fields.end(),
                         [&svName](const Smp::IField *element) {
                           return element->GetName() == svName;
                         });
  return (it != _fields.end()) ? *it : nullptr;
}

::Smp::IField *InputFieldCollection::at(size_t index) const {
  return index < _fields.size() ? _fields[index] : nullptr;
}

size_t InputFieldCollection::size() const { return _fields.size(); }

::Smp::Bool InputFieldCollection::empty() const { return _fields.empty(); }

InputFieldCollection::const_iterator InputFieldCollection::begin() const {
  return {*this, 0};
}

InputFieldCollection::const_iterator InputFieldCollection::end() const {
  return {*this, _fields.size()};
}

bool InputFieldCollection::contains(const ::Smp::IField *input) const {
  return std::find(_fields.begin(), _fields.end(), input) != _fields.end();
}

void InputFieldCollection::add(::Smp::IField *input) {
  _fields.push_back(input);
}
bool InputFieldCollection::remove(const ::Smp::IField *input) {
  if (auto it = std::find(_fields.begin(), _fields.end(), input);
      it != _fields.end()) {
    _fields.erase(it);
    return true;
  }
  return false;
}

ArrayOutputField::ArrayOutputField() = default;

void ArrayOutputField::Push() { FieldHelper::Push(this); }

void ArrayOutputField::Connect(::Smp::IField *target) {
  auto *arrayInput = dynamic_cast<::Smp::IArrayField *>(target);
  if (!this->IsOutput() || !target->IsInput() || target == this ||
      !arrayInput || !::Xsmp::Helper::AreEquivalent(this, target)) {
    ::Xsmp::Exception::throwInvalidTarget(this, this, target);
  }
  if (!FieldHelper::CanConnect(this, arrayInput)) {
    ::Xsmp::Exception::throwFieldAlreadyConnected(this, this, target);
  }
  FieldHelper::Connect(this, arrayInput);
}

void ArrayOutputField::Disconnect(::Smp::IField *target) {
  const auto *arrayInput = dynamic_cast<const ::Smp::IArrayField *>(target);
  if (!arrayInput || !FieldHelper::Disconnect(this, arrayInput)) {
    ::Xsmp::Exception::throwFieldNotConnected(this, this, target);
  }
}

const ::Smp::FieldCollection *ArrayOutputField::GetInputFields() const {
  return &_connectedFields;
}

StructureOutputField::StructureOutputField() = default;

void StructureOutputField::Push() { FieldHelper::Push(this); }
void StructureOutputField::Connect(::Smp::IField *target) {
  auto *structInput = dynamic_cast<::Smp::IStructureField *>(target);
  if (!this->IsOutput() || !target->IsInput() || target == this ||
      !structInput || !::Xsmp::Helper::AreEquivalent(this, target)) {
    ::Xsmp::Exception::throwInvalidTarget(this, this, target);
  }
  if (!FieldHelper::CanConnect(this, structInput)) {
    ::Xsmp::Exception::throwFieldAlreadyConnected(this, this, target);
  }
  FieldHelper::Connect(this, structInput);
}

void StructureOutputField::Disconnect(::Smp::IField *target) {
  const auto *structInput =
      dynamic_cast<const ::Smp::IStructureField *>(target);
  if (!structInput || !FieldHelper::Disconnect(this, structInput)) {
    ::Xsmp::Exception::throwFieldNotConnected(this, this, target);
  }
}
const ::Smp::FieldCollection *StructureOutputField::GetInputFields() const {
  return &_connectedFields;
}

SimpleArrayConnectableField::SimpleArrayConnectableField() = default;

void SimpleArrayConnectableField::internal_push(::Smp::UInt64 index) const {
  // a connected field can push back to this one: propagate the value only once
  if (_pushing) {
    return;
  }
  _pushing = true;
  try {
    for (auto *field : _connectedFields) {
      dynamic_cast<::Smp::ISimpleArrayField *>(field)->SetValue(
          index, this->GetValue(index));
    }
  } catch (...) {
    _pushing = false;
    throw;
  }
  _pushing = false;
}

const ::Smp::FieldCollection *
SimpleArrayConnectableField::GetInputFields() const {
  return &_connectedFields;
}

void AbstractForcibleField::Restore(::Smp::IStorageReader *reader) {
  reader->Restore(&_forced, sizeof(_forced));
  // restore of value performed in ForcibleField
}

void AbstractForcibleField::Store(::Smp::IStorageWriter *writer) {
  writer->Store(&_forced, sizeof(_forced));
  // store of value performed in ForcibleField
}

::Smp::Bool AbstractForcibleField::IsForced() const { return _forced; }

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
