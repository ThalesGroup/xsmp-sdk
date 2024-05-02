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
#include <Smp/IStorageReader.h>
#include <Smp/IStorageWriter.h>
#include <Smp/Uuid.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Publication/Field.h>
#include <Xsmp/Publication/Type.h>
#include <Xsmp/Publication/TypeRegistry.h>

namespace Xsmp::Publication {

namespace {
// helper function to check if a value is invalid in case of an enumeration type
bool isInvalidEnumerationValue(const ::Smp::Publication::IType *type,
                               const ::Smp::AnySimple &value) {
  const auto *enumeration =
      dynamic_cast<const ::Xsmp::Publication::EnumerationType *>(type);
  return enumeration &&
         enumeration->GetLiterals().find(static_cast<::Smp::Int32>(value)) ==
             enumeration->GetLiterals().end();
}

} // namespace

Field::Field(::Smp::String8 name, ::Smp::String8 description,
             ::Smp::IObject *parent, void *address,
             const ::Smp::Publication::IType *type, ::Smp::ViewKind view,
             ::Smp::Bool state, ::Smp::Bool input, ::Smp::Bool output)
    : _name(::Xsmp::Helper::checkName(name, parent)),
      _description(description ? description : ""), _parent(parent),
      _address(address), _type(type), _view(view), _state(state), _input(input),
      _output(output) {
  // disallow fields with String8/void type
  if (type && (type->GetUuid() == ::Smp::Uuids::Uuid_String8 ||
               type->GetUuid() == ::Smp::Uuids::Uuid_Void))
    ::Xsmp::Exception::throwInvalidFieldType(this, type);
}
::Smp::String8 Field::GetName() const { return _name.c_str(); }
::Smp::String8 Field::GetDescription() const { return _description.c_str(); }
::Smp::IObject *Field::GetParent() const { return _parent; }
::Smp::ViewKind Field::GetView() const { return _view; }
::Smp::Bool Field::IsState() const { return _state; }
::Smp::Bool Field::IsInput() const { return _input; }
::Smp::Bool Field::IsOutput() const { return _output; }
const ::Smp::Publication::IType *Field::GetType() const { return _type; }

std::unique_ptr<Field> Field::Create(::Smp::String8 name,
                                     ::Smp::String8 description,
                                     ::Smp::IObject *parent, void *address,
                                     const ::Smp::Publication::IType *type,
                                     ::Smp::ViewKind view, ::Smp::Bool state,
                                     ::Smp::Bool input, ::Smp::Bool output) {

  // create a simple field
  if (type->GetPrimitiveTypeKind() != ::Smp::PrimitiveTypeKind::PTK_None) {
    if (output)
      return std::make_unique<SimpleDataflowField>(
          name, description, parent, address, type, view, state, input, output);
    else
      return std::make_unique<SimpleField>(name, description, parent, address,
                                           type, view, state, input, output);
  }
  // create an array field
  if (const auto *array =
          dynamic_cast<const ::Xsmp::Publication::ArrayType *>(type)) {
    if (array->IsSimpleArray() &&
        array->GetItemType()->GetPrimitiveTypeKind() !=
            ::Smp::PrimitiveTypeKind::PTK_None) {
      if (output)
        return std::make_unique<SimpleArrayDataflowField>(
            name, description, parent, address, array, view, state, input,
            output);
      else
        return std::make_unique<SimpleArrayField>(name, description, parent,
                                                  address, array, view, state,
                                                  input, output);
    }
    // normal field
    else {
      if (output)
        return std::make_unique<ArrayDataflowField>(name, description, parent,
                                                    address, array, view, state,
                                                    input, output);
      else
        return std::make_unique<ArrayField>(name, description, parent, address,
                                            array, view, state, input, output);
    }
  }
  // create a structure field
  if (const auto *structure =
          dynamic_cast<const ::Xsmp::Publication::StructureType *>(type)) {
    if (output)
      return std::make_unique<StructureDataflowField>(name, description, parent,
                                                      address, structure, view,
                                                      state, input, output);
    else
      return std::make_unique<StructureField>(name, description, parent,
                                              address, structure, view, state,
                                              input, output);
  }
  ::Xsmp::Exception::throwInvalidFieldType(parent, type);
}

void DataflowField::Connect(::Smp::IField *target) {
  if (_targets.find(target) != _targets.end())
    ::Xsmp::Exception::throwFieldAlreadyConnected(this, this, target);

  if (!this->IsOutput() || !target->IsInput() || target == this ||
      !::Xsmp::Helper::AreEquivalent(this, target))
    ::Xsmp::Exception::throwInvalidTarget(this, this, target);

  _targets.emplace(target);

  // maybe we could push the value  here ?
}

void DataflowField::Push(::Smp::IField *source, ::Smp::IField *target) {

  // push a simple field
  if (auto const *simpleSource = dynamic_cast<::Smp::ISimpleField *>(source)) {
    auto *simpleTarget = dynamic_cast<::Smp::ISimpleField *>(target);

    simpleTarget->SetValue(simpleSource->GetValue());
  }
  // push a simple array field
  else if (auto const *simpleArraySource =
               dynamic_cast<::Smp::ISimpleArrayField *>(source)) {
    auto *simpleArrayTarget = dynamic_cast<::Smp::ISimpleArrayField *>(target);

    for (std::size_t i = 0; i < simpleArraySource->GetSize(); ++i)
      simpleArrayTarget->SetValue(i, simpleArraySource->GetValue(i));
  }
  // push an array field
  else if (auto const *arraySource =
               dynamic_cast<::Smp::IArrayField *>(source)) {
    auto const *arrayTarget = dynamic_cast<::Smp::IArrayField *>(target);

    for (std::size_t i = 0; i < arraySource->GetSize(); ++i)
      Push(arraySource->GetItem(i), arrayTarget->GetItem(i));
  }
  // push a structure field
  else if (auto const *structSource =
               dynamic_cast<::Smp::IStructureField *>(source)) {
    auto const *structTarget = dynamic_cast<::Smp::IStructureField *>(target);

    std::size_t index = 0;
    for (auto *f : *structSource->GetFields()) {
      Push(f, structTarget->GetFields()->at(index));
      ++index;
    }
  }
  // should not happen
  else {
    // ignore
  }
}

void DataflowField::Push() {
  for (auto *target : _targets)
    Push(this, target);
}

AnonymousArrayField::AnonymousArrayField(
    ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
    ::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::ViewKind view,
    ::Smp::Bool state)
    : Field(name, description, parent, nullptr, nullptr, view, state, false,
            false),
      ::Xsmp::Publication::Publication(parent, typeRegistry) {}

void AnonymousArrayField::Restore(::Smp::IStorageReader *reader) {
  if (IsState()) {
    for (auto *field : *GetFields())
      field->Restore(reader);
  }
}

void AnonymousArrayField::Store(::Smp::IStorageWriter *writer) {
  if (IsState()) {
    for (auto *field : *GetFields())
      field->Store(writer);
  }
}

::Smp::UInt64 AnonymousArrayField::GetSize() const {
  return GetFields()->size();
}

::Smp::IField *AnonymousArrayField::GetItem(::Smp::UInt64 index) const {
  if (index >= GetSize())
    ::Xsmp::Exception::throwInvalidArrayIndex(this, index);
  return GetFields()->at(index);
}

AnonymousSimpleArrayField::AnonymousSimpleArrayField(
    ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
    ::Smp::Int64 count, void *address, ::Smp::PrimitiveTypeKind kind,
    ::Smp::ViewKind view, ::Smp::Bool state, ::Smp::Bool input,
    ::Smp::Bool output)
    : Field(name, description, parent, address, nullptr, view, state, input,
            output),
      _count(count >= 0 ? static_cast<::Smp::UInt64>(count) : 0), _kind(kind) {}

void AnonymousSimpleArrayField::Restore(::Smp::IStorageReader *reader) {
  if (IsState()) {
    reader->Restore(GetAddress(), GetItemSize() * GetSize());
  }
}

void AnonymousSimpleArrayField::Store(::Smp::IStorageWriter *writer) {
  if (IsState()) {
    writer->Store(GetAddress(), GetItemSize() * GetSize());
  }
}

std::size_t AnonymousSimpleArrayField::GetItemSize() const {
  switch (_kind) {
  case ::Smp::PrimitiveTypeKind::PTK_Bool:
    return sizeof(::Smp::Bool);
  case ::Smp::PrimitiveTypeKind::PTK_Char8:
    return sizeof(::Smp::Char8);
  case ::Smp::PrimitiveTypeKind::PTK_Int8:
    return sizeof(::Smp::Int8);
  case ::Smp::PrimitiveTypeKind::PTK_Int16:
    return sizeof(::Smp::Int16);
  case ::Smp::PrimitiveTypeKind::PTK_Int32:
    return sizeof(::Smp::Int32);
  case ::Smp::PrimitiveTypeKind::PTK_Duration:
  case ::Smp::PrimitiveTypeKind::PTK_DateTime:
  case ::Smp::PrimitiveTypeKind::PTK_Int64:
    return sizeof(::Smp::Int64);
  case ::Smp::PrimitiveTypeKind::PTK_UInt8:
    return sizeof(::Smp::UInt8);
  case ::Smp::PrimitiveTypeKind::PTK_UInt16:
    return sizeof(::Smp::UInt16);
  case ::Smp::PrimitiveTypeKind::PTK_UInt32:
    return sizeof(::Smp::UInt32);
  case ::Smp::PrimitiveTypeKind::PTK_UInt64:
    return sizeof(::Smp::UInt64);
  case ::Smp::PrimitiveTypeKind::PTK_Float32:
    return sizeof(::Smp::Float32);
  case ::Smp::PrimitiveTypeKind::PTK_Float64:
    return sizeof(::Smp::Float64);
  default:
    ::Xsmp::Exception::throwInvalidPrimitiveType(this, "void", _kind);
  }
}

::Smp::UInt64 AnonymousSimpleArrayField::GetSize() const { return _count; }

::Smp::AnySimple
AnonymousSimpleArrayField::GetValue(::Smp::UInt64 index) const {
  if (index >= GetSize())
    ::Xsmp::Exception::throwInvalidArrayIndex(this, index);

  switch (_kind) {
  case ::Smp::PrimitiveTypeKind::PTK_Bool:
    return {_kind, static_cast<::Smp::Bool *>(GetAddress())[index]};
  case ::Smp::PrimitiveTypeKind::PTK_Char8:
    return {_kind, static_cast<::Smp::Char8 *>(GetAddress())[index]};
  case ::Smp::PrimitiveTypeKind::PTK_Int8:
    return {_kind, static_cast<::Smp::Int8 *>(GetAddress())[index]};
  case ::Smp::PrimitiveTypeKind::PTK_Int16:
    return {_kind, static_cast<::Smp::Int16 *>(GetAddress())[index]};
  case ::Smp::PrimitiveTypeKind::PTK_Int32:
    return {_kind, static_cast<::Smp::Int32 *>(GetAddress())[index]};
  case ::Smp::PrimitiveTypeKind::PTK_Duration:
  case ::Smp::PrimitiveTypeKind::PTK_DateTime:
  case ::Smp::PrimitiveTypeKind::PTK_Int64:
    return {_kind, static_cast<::Smp::Int64 *>(GetAddress())[index]};
  case ::Smp::PrimitiveTypeKind::PTK_UInt8:
    return {_kind, static_cast<::Smp::UInt8 *>(GetAddress())[index]};
  case ::Smp::PrimitiveTypeKind::PTK_UInt16:
    return {_kind, static_cast<::Smp::UInt16 *>(GetAddress())[index]};
  case ::Smp::PrimitiveTypeKind::PTK_UInt32:
    return {_kind, static_cast<::Smp::UInt32 *>(GetAddress())[index]};
  case ::Smp::PrimitiveTypeKind::PTK_UInt64:
    return {_kind, static_cast<::Smp::UInt64 *>(GetAddress())[index]};
  case ::Smp::PrimitiveTypeKind::PTK_Float32:
    return {_kind, static_cast<::Smp::Float32 *>(GetAddress())[index]};
  case ::Smp::PrimitiveTypeKind::PTK_Float64:
    return {_kind, static_cast<::Smp::Float64 *>(GetAddress())[index]};
  default:
    ::Xsmp::Exception::throwInvalidPrimitiveType(this, "void", _kind);
  }
}

void AnonymousSimpleArrayField::SetValue(::Smp::UInt64 index,
                                         ::Smp::AnySimple value) {
  if (index >= GetSize())
    ::Xsmp::Exception::throwInvalidArrayIndex(this, index);

  switch (_kind) {
  case ::Smp::PrimitiveTypeKind::PTK_Bool:
    static_cast<::Smp::Bool *>(GetAddress())[index] = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Char8:
    static_cast<::Smp::Char8 *>(GetAddress())[index] = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Int8:
    static_cast<::Smp::Int8 *>(GetAddress())[index] = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Int16:
    static_cast<::Smp::Int16 *>(GetAddress())[index] = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Int32:
    static_cast<::Smp::Int32 *>(GetAddress())[index] = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Duration:
  case ::Smp::PrimitiveTypeKind::PTK_DateTime:
  case ::Smp::PrimitiveTypeKind::PTK_Int64:
    static_cast<::Smp::Int64 *>(GetAddress())[index] = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_UInt8:
    static_cast<::Smp::UInt8 *>(GetAddress())[index] = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_UInt16:
    static_cast<::Smp::UInt16 *>(GetAddress())[index] = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_UInt32:
    static_cast<::Smp::UInt32 *>(GetAddress())[index] = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_UInt64:
    static_cast<::Smp::UInt64 *>(GetAddress())[index] = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Float32:
    static_cast<::Smp::Float32 *>(GetAddress())[index] = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Float64:
    static_cast<::Smp::Float64 *>(GetAddress())[index] = value;
    break;
  default:
    ::Xsmp::Exception::throwInvalidArrayValue(this, index, value);
  }
}

void AnonymousSimpleArrayField::GetValues(::Smp::UInt64 length,
                                          ::Smp::AnySimpleArray values) const {

  if (length != GetSize())
    ::Xsmp::Exception::throwInvalidArraySize(this, length);

  for (std::size_t i = 0; i < length; ++i)
    values[i] = GetValue(i);
}

void AnonymousSimpleArrayField::SetValues(::Smp::UInt64 length,
                                          ::Smp::AnySimpleArray values) {
  if (length != GetSize())
    ::Xsmp::Exception::throwInvalidArraySize(this, length);
  for (std::size_t i = 0; i < length; ++i)
    SetValue(i, values[i]);
}

AnonymousStructureField::AnonymousStructureField(
    ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
    ::Smp::Publication::ITypeRegistry *typeRegistry, ::Smp::ViewKind view,
    ::Smp::Bool state)
    : Field(name, description, parent, nullptr, nullptr, view, state, false,
            false),
      Publication(this, typeRegistry) {}

const ::Smp::FieldCollection *AnonymousStructureField::GetFields() const {
  return Publication::GetFields();
}

void AnonymousStructureField::Restore(::Smp::IStorageReader *reader) {
  if (IsState()) {
    for (auto *field : *GetFields())
      field->Restore(reader);
  }
}

void AnonymousStructureField::Store(::Smp::IStorageWriter *writer) {
  if (IsState()) {
    for (auto *field : *GetFields())
      field->Store(writer);
  }
}

::Smp::IField *AnonymousStructureField::GetField(::Smp::String8 name) const {
  return GetFields()->at(name);
}

ArrayField::ArrayField(::Smp::String8 name, ::Smp::String8 description,
                       ::Smp::IObject *parent, void *address,
                       const ::Xsmp::Publication::ArrayType *type,
                       ::Smp::ViewKind view, ::Smp::Bool state,
                       ::Smp::Bool input, ::Smp::Bool output)
    : Field(name, description, parent, address, type, view, state, input,
            output) {

  for (std::size_t i = 0; i < type->GetSize(); ++i)
    /// The parent of the item is the parent of the array
    /// The item name is the array name + [index]
    _fields.push_back(
        Create((std::string(name) + "[" + std::to_string(i) + "]").c_str(), "",
               parent, static_cast<char *>(address) + i * type->GetItemSize(),
               type->GetItemType(), view, state, input, output));
}

::Smp::UInt64 ArrayField::GetSize() const { return _fields.size(); }

::Smp::IField *ArrayField::GetItem(::Smp::UInt64 index) const {
  if (index >= GetSize())
    ::Xsmp::Exception::throwInvalidArrayIndex(this, index);

  return _fields[index].get();
}

void ArrayField::Restore(::Smp::IStorageReader *reader) {
  if (IsState()) {
    for (auto const &field : _fields)
      field->Restore(reader);
  }
}

void ArrayField::Store(::Smp::IStorageWriter *writer) {
  if (IsState()) {
    for (auto const &field : _fields)
      field->Store(writer);
  }
}

SimpleArrayField::SimpleArrayField(::Smp::String8 name,
                                   ::Smp::String8 description,
                                   ::Smp::IObject *parent, void *address,
                                   const ::Xsmp::Publication::ArrayType *type,
                                   ::Smp::ViewKind view, ::Smp::Bool state,
                                   ::Smp::Bool input, ::Smp::Bool output)
    : Field(name, description, parent, address, type, view, state, input,
            output),
      _size(type->GetSize()), _itemType(type->GetItemType()),
      _itemSize(type->GetItemSize()) {}

void SimpleArrayField::Restore(::Smp::IStorageReader *reader) {
  if (IsState()) {
    const auto *type =
        dynamic_cast<const ::Xsmp::Publication::ArrayType *>(GetType());
    reader->Restore(GetAddress(), type->GetItemSize() * GetSize());
  }
}

void SimpleArrayField::Store(::Smp::IStorageWriter *writer) {
  if (IsState()) {
    const auto *type =
        dynamic_cast<const ::Xsmp::Publication::ArrayType *>(GetType());
    writer->Store(GetAddress(), type->GetItemSize() * GetSize());
  }
}

::Smp::UInt64 SimpleArrayField::GetSize() const { return _size; }

::Smp::AnySimple SimpleArrayField::GetValue(::Smp::UInt64 index) const {
  if (index >= GetSize())
    ::Xsmp::Exception::throwInvalidArrayIndex(this, index);

  void *address = static_cast<char *>(GetAddress()) + index * _itemSize;
  auto kind = _itemType->GetPrimitiveTypeKind();
  switch (kind) {
  case ::Smp::PrimitiveTypeKind::PTK_Bool:
    return {kind, *static_cast<::Smp::Bool *>(address)};
  case ::Smp::PrimitiveTypeKind::PTK_Char8:
    return {kind, *static_cast<::Smp::Char8 *>(address)};
  case ::Smp::PrimitiveTypeKind::PTK_Int8:
    return {kind, *static_cast<::Smp::Int8 *>(address)};
  case ::Smp::PrimitiveTypeKind::PTK_Int16:
    return {kind, *static_cast<::Smp::Int16 *>(address)};
  case ::Smp::PrimitiveTypeKind::PTK_Int32:
    return {kind, *static_cast<::Smp::Int32 *>(address)};
  case ::Smp::PrimitiveTypeKind::PTK_Duration:
    return {kind, *static_cast<::Smp::Duration *>(address)};
  case ::Smp::PrimitiveTypeKind::PTK_DateTime:
    return {kind, *static_cast<::Smp::DateTime *>(address)};
  case ::Smp::PrimitiveTypeKind::PTK_Int64:
    return {kind, *static_cast<::Smp::Int64 *>(address)};
  case ::Smp::PrimitiveTypeKind::PTK_UInt8:
    return {kind, *static_cast<::Smp::UInt8 *>(address)};
  case ::Smp::PrimitiveTypeKind::PTK_UInt16:
    return {kind, *static_cast<::Smp::UInt16 *>(address)};
  case ::Smp::PrimitiveTypeKind::PTK_UInt32:
    return {kind, *static_cast<::Smp::UInt32 *>(address)};
  case ::Smp::PrimitiveTypeKind::PTK_UInt64:
    return {kind, *static_cast<::Smp::UInt64 *>(address)};
  case ::Smp::PrimitiveTypeKind::PTK_Float32:
    return {kind, *static_cast<::Smp::Float32 *>(address)};
  case ::Smp::PrimitiveTypeKind::PTK_Float64:
    return {kind, *static_cast<::Smp::Float64 *>(address)};
  case ::Smp::PrimitiveTypeKind::PTK_String8:
    return {kind, static_cast<::Smp::String8>(address)};
  default:
    ::Xsmp::Exception::throwInvalidPrimitiveType(this, "void", kind);
  }
}

void SimpleArrayField::SetValue(::Smp::UInt64 index, ::Smp::AnySimple value) {
  if (index >= GetSize())
    ::Xsmp::Exception::throwInvalidArrayIndex(this, index);

  void *address = static_cast<char *>(GetAddress()) + index * _itemSize;

  switch (_itemType->GetPrimitiveTypeKind()) {
  case ::Smp::PrimitiveTypeKind::PTK_Bool:
    *static_cast<::Smp::Bool *>(address) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Char8:
    *static_cast<::Smp::Char8 *>(address) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Int8:
    *static_cast<::Smp::Int8 *>(address) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Int16:
    *static_cast<::Smp::Int16 *>(address) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Int32:
    if (isInvalidEnumerationValue(GetType(), value))
      ::Xsmp::Exception::throwInvalidArrayValue(this, index, value);
    *static_cast<::Smp::Int32 *>(address) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Duration:
  case ::Smp::PrimitiveTypeKind::PTK_DateTime:
  case ::Smp::PrimitiveTypeKind::PTK_Int64:
    *static_cast<::Smp::Int64 *>(address) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_UInt8:
    *static_cast<::Smp::UInt8 *>(address) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_UInt16:
    *static_cast<::Smp::UInt16 *>(address) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_UInt32:
    *static_cast<::Smp::UInt32 *>(address) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_UInt64:
    *static_cast<::Smp::UInt64 *>(address) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Float32:
    *static_cast<::Smp::Float32 *>(address) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Float64:
    *static_cast<::Smp::Float64 *>(address) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_String8: {
    auto *dest = static_cast<char *>(address);
    const auto *type =
        dynamic_cast<const ::Xsmp::Publication::ArrayType *>(GetType());
    auto size = type->GetItemSize();
    ::Xsmp::Helper::CopyString(dest, size, value);
    break;
  }
  default:
    ::Xsmp::Exception::throwInvalidArrayValue(this, index, value);
  }
}

void SimpleArrayField::GetValues(::Smp::UInt64 length,
                                 ::Smp::AnySimpleArray values) const {

  if (length != GetSize())
    ::Xsmp::Exception::throwInvalidArraySize(this, length);

  for (std::size_t i = 0; i < length; ++i)
    values[i] = GetValue(i);
}

void SimpleArrayField::SetValues(::Smp::UInt64 length,
                                 ::Smp::AnySimpleArray values) {
  if (length != GetSize())
    ::Xsmp::Exception::throwInvalidArraySize(this, length);

  auto _itemKind = _itemType->GetPrimitiveTypeKind();
  for (std::size_t i = 0; i < length; ++i) {
    if (values[i].type != _itemKind) {
      ::Xsmp::Exception::throwInvalidArrayValue(this, i, values[i]);
    }
    SetValue(i, values[i]);
  }
}

void SimpleField::Restore(::Smp::IStorageReader *reader) {
  if (IsState())
    reader->Restore(GetAddress(), static_cast<::Smp::UInt64>(GetSize()));
}

void SimpleField::Store(::Smp::IStorageWriter *writer) {
  if (IsState())
    writer->Store(GetAddress(), static_cast<::Smp::UInt64>(GetSize()));
}

::Smp::Int64 SimpleField::GetSize() const {
  auto kind = GetPrimitiveTypeKind();
  switch (kind) {
  case ::Smp::PrimitiveTypeKind::PTK_Bool:
    return sizeof(::Smp::Bool);
  case ::Smp::PrimitiveTypeKind::PTK_Char8:
    return sizeof(::Smp::Char8);
  case ::Smp::PrimitiveTypeKind::PTK_Int8:
    return sizeof(::Smp::Int8);
  case ::Smp::PrimitiveTypeKind::PTK_Int16:
    return sizeof(::Smp::Int16);
  case ::Smp::PrimitiveTypeKind::PTK_Int32:
    return sizeof(::Smp::Int32);
  case ::Smp::PrimitiveTypeKind::PTK_Duration:
  case ::Smp::PrimitiveTypeKind::PTK_DateTime:
  case ::Smp::PrimitiveTypeKind::PTK_Int64:
    return sizeof(::Smp::Int64);
  case ::Smp::PrimitiveTypeKind::PTK_UInt8:
    return sizeof(::Smp::UInt8);
  case ::Smp::PrimitiveTypeKind::PTK_UInt16:
    return sizeof(::Smp::UInt16);
  case ::Smp::PrimitiveTypeKind::PTK_UInt32:
    return sizeof(::Smp::UInt32);
  case ::Smp::PrimitiveTypeKind::PTK_UInt64:
    return sizeof(::Smp::UInt64);
  case ::Smp::PrimitiveTypeKind::PTK_Float32:
    return sizeof(::Smp::Float32);
  case ::Smp::PrimitiveTypeKind::PTK_Float64:
    return sizeof(::Smp::Float64);
  case ::Smp::PrimitiveTypeKind::PTK_String8:
    return dynamic_cast<const ::Xsmp::Publication::StringType *>(GetType())
               ->GetLength() +
           1;
  default:
    ::Xsmp::Exception::throwInvalidPrimitiveType(this, "void", kind);
  }
}

::Smp::PrimitiveTypeKind SimpleField::GetPrimitiveTypeKind() const {
  return GetType()->GetPrimitiveTypeKind();
}

::Smp::AnySimple SimpleField::GetValue() const {
  auto kind = GetPrimitiveTypeKind();
  switch (kind) {
  case ::Smp::PrimitiveTypeKind::PTK_Bool:
    return {kind, *static_cast<::Smp::Bool *>(GetAddress())};
  case ::Smp::PrimitiveTypeKind::PTK_Char8:
    return {kind, *static_cast<::Smp::Char8 *>(GetAddress())};
  case ::Smp::PrimitiveTypeKind::PTK_Int8:
    return {kind, *static_cast<::Smp::Int8 *>(GetAddress())};
  case ::Smp::PrimitiveTypeKind::PTK_Int16:
    return {kind, *static_cast<::Smp::Int16 *>(GetAddress())};
  case ::Smp::PrimitiveTypeKind::PTK_Int32:
    return {kind, *static_cast<::Smp::Int32 *>(GetAddress())};
  case ::Smp::PrimitiveTypeKind::PTK_Duration:
    return {kind, *static_cast<::Smp::Duration *>(GetAddress())};
  case ::Smp::PrimitiveTypeKind::PTK_DateTime:
    return {kind, *static_cast<::Smp::DateTime *>(GetAddress())};
  case ::Smp::PrimitiveTypeKind::PTK_Int64:
    return {kind, *static_cast<::Smp::Int64 *>(GetAddress())};
  case ::Smp::PrimitiveTypeKind::PTK_UInt8:
    return {kind, *static_cast<::Smp::UInt8 *>(GetAddress())};
  case ::Smp::PrimitiveTypeKind::PTK_UInt16:
    return {kind, *static_cast<::Smp::UInt16 *>(GetAddress())};
  case ::Smp::PrimitiveTypeKind::PTK_UInt32:
    return {kind, *static_cast<::Smp::UInt32 *>(GetAddress())};
  case ::Smp::PrimitiveTypeKind::PTK_UInt64:
    return {kind, *static_cast<::Smp::UInt64 *>(GetAddress())};
  case ::Smp::PrimitiveTypeKind::PTK_Float32:
    return {kind, *static_cast<::Smp::Float32 *>(GetAddress())};
  case ::Smp::PrimitiveTypeKind::PTK_Float64:
    return {kind, *static_cast<::Smp::Float64 *>(GetAddress())};
  case ::Smp::PrimitiveTypeKind::PTK_String8:
    return {kind, static_cast<::Smp::String8>(GetAddress())};
  default:
    ::Xsmp::Exception::throwInvalidPrimitiveType(this, "void", kind);
  }
}

void SimpleField::SetValue(::Smp::AnySimple value) {
  auto kind = GetPrimitiveTypeKind();
  switch (kind) {

  case ::Smp::PrimitiveTypeKind::PTK_Bool:
    *static_cast<::Smp::Bool *>(GetAddress()) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Char8:
    *static_cast<::Smp::Char8 *>(GetAddress()) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Int8:
    *static_cast<::Smp::Int8 *>(GetAddress()) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Int16:
    *static_cast<::Smp::Int16 *>(GetAddress()) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Int32:
    if (isInvalidEnumerationValue(GetType(), value))
      ::Xsmp::Exception::throwInvalidFieldValue(this, value);
    *static_cast<::Smp::Int32 *>(GetAddress()) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Duration:
  case ::Smp::PrimitiveTypeKind::PTK_DateTime:
  case ::Smp::PrimitiveTypeKind::PTK_Int64:
    *static_cast<::Smp::Int64 *>(GetAddress()) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_UInt8:
    *static_cast<::Smp::UInt8 *>(GetAddress()) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_UInt16:
    *static_cast<::Smp::UInt16 *>(GetAddress()) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_UInt32:
    *static_cast<::Smp::UInt32 *>(GetAddress()) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_UInt64:
    *static_cast<::Smp::UInt64 *>(GetAddress()) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Float32:
    *static_cast<::Smp::Float32 *>(GetAddress()) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_Float64:
    *static_cast<::Smp::Float64 *>(GetAddress()) = value;
    break;
  case ::Smp::PrimitiveTypeKind::PTK_String8:
    ::Xsmp::Helper::CopyString(
        static_cast<char *>(GetAddress()),
        static_cast<::Smp::UInt64>(
            dynamic_cast<const ::Xsmp::Publication::StringType *>(GetType())
                ->GetLength() +
            1),
        value);
    break;
  default:
    ::Xsmp::Exception::throwInvalidPrimitiveType(this, "void", kind);
  }
}

StructureField::StructureField(::Smp::String8 name, ::Smp::String8 description,
                               ::Smp::IObject *parent, void *address,
                               const ::Xsmp::Publication::StructureType *type,
                               ::Smp::ViewKind view, ::Smp::Bool state,
                               ::Smp::Bool input, ::Smp::Bool output)
    : Field(name, description, parent, address, type, view, state, input,
            output),
      _fields{"Fields", "", this} {

  for (const auto &field : type->GetFields()) {
    if (auto const *fieldType = type->GetTypeRegistry()->GetType(field.uuid))
      _fields.Add(Create(field.name.c_str(), field.description.c_str(), this,
                         static_cast<char *>(address) + field.offset, fieldType,
                         field.view, field.state, field.input || input,
                         field.output || output));
    else
      ::Xsmp::Exception::throwTypeNotRegistered(this, field.uuid);
  }
}

void StructureField::Restore(::Smp::IStorageReader *reader) {
  if (IsState())
    for (auto *field : _fields)
      field->Restore(reader);
}

void StructureField::Store(::Smp::IStorageWriter *writer) {
  if (IsState())
    for (auto *field : _fields)
      field->Store(writer);
}

const ::Smp::FieldCollection *StructureField::GetFields() const {
  return &_fields;
}

::Smp::IField *StructureField::GetField(::Smp::String8 name) const {
  return _fields.at(name);
}

} // namespace Xsmp::Publication
