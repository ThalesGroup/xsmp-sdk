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

#include <Smp/Publication/IType.h>
#include <Xsmp/FallibleModel.h>
#include <Xsmp/Field.h>
#include <algorithm>

namespace Xsmp::detail {

AbstractField::AbstractField(::Smp::String8 name, ::Smp::String8 description,
        ::Smp::IObject *parent, ::Smp::ViewKind view) :
        Object(name, description, parent), _view { view } {
    // auto register field in AbstractStructureField
    if (auto *structure = dynamic_cast<AbstractStructureField*>(parent)) {
        structure->AddField(*this);
    }
}

Smp::ViewKind AbstractField::GetView() const {
    return _view;
}

::Smp::Bool Failure::IsFailed() const {
    return _failed;
}
void Failure::Register() {
    if (auto *model = dynamic_cast<::Xsmp::FallibleModel*>(this->GetParent()))
        model->AddFailure(this);
}

void Failure::Unfail() {
    _failed = false;
}
void Failure::Fail() {
    _failed = true;
}
void Failure::Restore(::Smp::IStorageReader *reader) {
    reader->Restore(&_failed, sizeof(_failed));
}
void Failure::Store(::Smp::IStorageWriter *writer) {
    writer->Store(&_failed, sizeof(_failed));
}

void DataflowField::Push() {
    Push(this);
}
void DataflowField::Push(::Smp::IField *field) {

    if (auto const *simple = dynamic_cast<SimpleConnectableField*>(field)) {
        simple->internal_push();
    }
    else if (auto const *simpleArray =
            dynamic_cast<SimpleArrayConnectableField*>(field)) {
        for (::Smp::UInt64 i = 0; i < simpleArray->GetSize(); ++i)
            simpleArray->internal_push(i);
    }
    else if (const auto *arrayfield =
            dynamic_cast<const ::Smp::IArrayField*>(field)) {
        for (::Smp::UInt64 i = 0; i < arrayfield->GetSize(); ++i)
            Push(arrayfield->GetItem(i));
    }
    else if (const auto *structfield =
            dynamic_cast<const ::Smp::IStructureField*>(field)) {
        for (auto *f : *structfield->GetFields())
            Push(f);
    }
    // should not happen
    else {
        // ignore
    }

}
bool DataflowField::Connect(DataflowField *sender, ::Smp::IField *source,
        ::Smp::IField *target) {

    //connect a simple field
    if (auto *simpleSource = dynamic_cast<SimpleConnectableField*>(source)) {
        auto *simpleTarget = dynamic_cast<::Smp::ISimpleField*>(target);
        if (!simpleTarget
                || (simpleTarget->GetPrimitiveTypeKind()
                        != simpleSource->GetPrimitiveTypeKind()))
            return false;

        if (!simpleSource->_connectedFields.emplace(simpleTarget).second)
            ::Xsmp::Exception::throwFieldAlreadyConnected(sender,
                    dynamic_cast<::Smp::IDataflowField*>(source), target);
        return true;

    }
    //Connect a simple array field
    else if (auto *simpleArraySource =
            dynamic_cast<SimpleArrayConnectableField*>(source)) {
        auto *simpleArrayTarget =
                dynamic_cast<::Smp::ISimpleArrayField*>(target);
        if (!simpleArrayTarget
                || simpleArrayTarget->GetSize() != simpleArraySource->GetSize()
                || simpleArrayTarget->GetType()->GetPrimitiveTypeKind()
                        != simpleArraySource->GetType()->GetPrimitiveTypeKind())
            return false;

        if (!simpleArraySource->_connectedFields.emplace(simpleArrayTarget).second)
            ::Xsmp::Exception::throwFieldAlreadyConnected(sender,
                    dynamic_cast<::Smp::IDataflowField*>(source), target);
        return true;
    }
    //connect an array field
    else if (auto const *arraySource = dynamic_cast<::Smp::IArrayField*>(source)) {
        auto const *arrayTarget = dynamic_cast<::Smp::IArrayField*>(target);
        if (!arrayTarget || arrayTarget->GetSize() != arraySource->GetSize())
            return false;

        bool result = true;
        for (size_t i = 0; i < arraySource->GetSize(); ++i)
            result &= Connect(sender, arraySource->GetItem(i),
                    arrayTarget->GetItem(i));
        return result;
    }
    //connect a structure field
    else if (auto *structSource = dynamic_cast<AbstractStructureField*>(source)) {
        auto const *structTarget = dynamic_cast<::Smp::IStructureField*>(target);
        if (!structTarget
                || structTarget->GetFields()->size()
                        != structSource->GetFields()->size())
            return false;

        size_t index = 0;
        bool result = true;
        for (auto *f : *structSource->GetFields()) {
            result &= Connect(sender, f, structTarget->GetFields()->at(index));
            ++index;
        }
        return result;
    }
    //should not happen
    else {
        //ignore
    }

    return false;
}

void SimpleConnectableField::internal_push() const {
    for (auto *f : _connectedFields)
        f->SetValue(this->GetValue());
}

void SimpleArrayConnectableField::internal_push(size_t index) const {
    for (auto *f : _connectedFields)
        f->SetValue(index, this->GetValue(index));
}

void SimpleConnectableField::RemoveLinks(const ::Smp::IComponent *target) {
    for (auto it = _connectedFields.begin(); it != _connectedFields.end();)
        if (::Xsmp::Helper::GetParentOfType<::Smp::IComponent>(*it) == target)
            it = _connectedFields.erase(it);
        else
            ++it;
}
void SimpleArrayConnectableField::RemoveLinks(const ::Smp::IComponent *target) {
    for (auto it = _connectedFields.begin(); it != _connectedFields.end();)
        if (::Xsmp::Helper::GetParentOfType<::Smp::IComponent>(*it) == target)
            it = _connectedFields.erase(it);
        else
            ++it;
}

void DataflowField::Connect(::Smp::IField *target) {

    if (!this->IsOutput() || !target->IsInput() || target == this
            || !::Xsmp::Helper::AreEquivalent(this, target)
            || !Connect(this, this, target))
        ::Xsmp::Exception::throwInvalidTarget(this, this, target);

    Push(this);
}

void DataflowField::RemoveLinks(const ::Smp::IComponent *target) {
    RemoveLinks(this, target);
}

void DataflowField::RemoveLinks(::Smp::IField *field,
        const ::Smp::IComponent *target) {
    // Disconnect a simple field
    if (auto *simpleSource = dynamic_cast<SimpleConnectableField*>(field))
        simpleSource->RemoveLinks(target);

    // Disconnect a simple array field
    else if (auto *simpleArraySource =
            dynamic_cast<SimpleArrayConnectableField*>(field))
        simpleArraySource->RemoveLinks(target);

    // Disconnect an array field
    else if (auto const *arraySource = dynamic_cast<::Smp::IArrayField*>(field))
        for (size_t i = 0; i < arraySource->GetSize(); ++i)
            RemoveLinks(arraySource->GetItem(i), target);

    // Disconnect a structure field
    else if (auto *structSource = dynamic_cast<AbstractStructureField*>(field))
        for (auto *f : *structSource->GetFields())
            RemoveLinks(f, target);
    // should not happen
    else {
        //ignore
    }
}

void AbstractForcibleField::Restore(::Smp::IStorageReader *reader) {
    reader->Restore(&_forced, sizeof(_forced));
//restore of value performed in ForcibleField
}
void AbstractForcibleField::Store(::Smp::IStorageWriter *writer) {
    writer->Store(&_forced, sizeof(_forced));
//store of value performed in ForcibleField
}
::Smp::Bool AbstractForcibleField::IsForced() {
    return _forced;
}
void AbstractForcibleField::Force(::Smp::AnySimple value) {
    _forced = false;
    this->SetValue(std::move(value));
    _forced = true;
}
void AbstractForcibleField::Unforce() {
    _forced = false;
}
void AbstractForcibleField::Freeze() {
    _forced = true;
}

AbstractStructureField::AbstractStructureField() :
        _fields("Fields", "", this) {
}

const ::Smp::FieldCollection* AbstractStructureField::GetFields() const {
    return &_fields;
}

Collection<::Smp::IField>* AbstractStructureField::GetFields() {
    return &_fields;
}

::Smp::IField* AbstractStructureField::GetField(::Smp::String8 name) const {
    return _fields.at(name);
}
void AbstractStructureField::Restore(::Smp::IStorageReader *reader) {
    std::for_each(_fields.begin(), _fields.end(), [reader](auto *field) {
        field->Restore(reader);
    });
}
void AbstractStructureField::Store(::Smp::IStorageWriter *writer) {
    std::for_each(_fields.begin(), _fields.end(), [writer](Smp::IField *f) {
        f->Store(writer);
    });
}

} // namespace Xsmp::detail
