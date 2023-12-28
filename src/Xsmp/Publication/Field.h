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

#ifndef XSMP_PUBLICATION_FIELD_H_
#define XSMP_PUBLICATION_FIELD_H_

#include <cstddef>
#include <Smp/AnySimpleArray.h>
#include <Smp/IArrayField.h>
#include <Smp/IDataflowField.h>
#include <Smp/IField.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimpleField.h>
#include <Smp/IStructureField.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/ViewKind.h>
#include <Xsmp/Collection.h>
#include <Xsmp/Publication/Publication.h>
#include <memory>
#include <set>
#include <vector>

namespace Smp {
namespace Publication {
class ITypeRegistry;
} // namespace Publication
class IComponent;
class IStorageReader;
class IStorageWriter;
} // namespace Smp

namespace Xsmp::Publication {

class EnumerationType;
class ArrayType;

class StructureType;
class Field: public ::Xsmp::Object, public virtual ::Smp::IField {
public:
    Field(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, void *address,
            const ::Smp::Publication::IType *type, ::Smp::ViewKind view,
            ::Smp::Bool state, ::Smp::Bool input, ::Smp::Bool output);

    /// Field cannot be copied
    Field(const Field&) = delete;
    /// Field cannot be copied
    Field& operator=(const Field&) = delete;

    /// Field cannot be moved
    Field(Field&&) = delete;
    /// Field cannot be moved
    Field& operator=(const Field&&) = delete;

    static std::unique_ptr<Field> Create(::Smp::String8 name,
            ::Smp::String8 description, ::Smp::IObject *parent, void *address,
            const ::Smp::Publication::IType *type, ::Smp::ViewKind view,
            ::Smp::Bool state, ::Smp::Bool input, ::Smp::Bool output);

    ::Smp::ViewKind GetView() const final;

    ::Smp::Bool IsState() const final;

    ::Smp::Bool IsInput() const final;

    ::Smp::Bool IsOutput() const final;

    const ::Smp::Publication::IType* GetType() const override;
protected:
    void checkValid(const ::Xsmp::Publication::EnumerationType *enumeration,
            ::Smp::AnySimple value) const;
    inline void* GetAddress() const noexcept {
        return _address;
    }
private:
    void *_address;
    const ::Smp::Publication::IType *_type;
    ::Smp::ViewKind _view;
    ::Smp::Bool _state;
    ::Smp::Bool _input;
    ::Smp::Bool _output;
};

class DataflowField: public virtual ::Smp::IDataflowField {
public:
    void Connect(::Smp::IField *target) final;

    /// Push the current field value to all connected target fields.
    void Push() final;
private:
    void Push(::Smp::IField *source, ::Smp::IField *target);
    std::set<::Smp::IField*> _targets { };
};

class AnonymousArrayField: public Field,
        public virtual ::Smp::IArrayField,
        public ::Xsmp::Publication::Publication {
public:
    AnonymousArrayField(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent,
            ::Smp::Publication::ITypeRegistry *typeRegistry,
            ::Smp::ViewKind view, ::Smp::Bool state);

    void Restore(::Smp::IStorageReader *reader) override;

    void Store(::Smp::IStorageWriter *writer) override;

    ::Smp::UInt64 GetSize() const override;

    ::Smp::IField* GetItem(::Smp::UInt64 index) const override;
};

class AnonymousSimpleArrayField: public Field,
        public virtual ::Smp::ISimpleArrayField {
public:

    AnonymousSimpleArrayField(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, ::Smp::Int64 count, void *address,
            ::Smp::PrimitiveTypeKind kind, ::Smp::ViewKind view,
            ::Smp::Bool state, ::Smp::Bool input, ::Smp::Bool output);

    void Restore(::Smp::IStorageReader *reader) override;

    void Store(::Smp::IStorageWriter *writer) override;

    ::Smp::UInt64 GetSize() const override;

    ::Smp::AnySimple GetValue(::Smp::UInt64 index) const override;

    void SetValue(::Smp::UInt64 index, ::Smp::AnySimple value) override;

    void GetValues(::Smp::UInt64 length, ::Smp::AnySimpleArray values) const
            override;

    void SetValues(::Smp::UInt64 length, ::Smp::AnySimpleArray values) override;
private:
    std::size_t GetItemSize() const;
    ::Smp::UInt64 _count;
    ::Smp::PrimitiveTypeKind _kind;
};

class AnonymousSimpleArrayDataflowField: public AnonymousSimpleArrayField,
        public DataflowField {
public:
    using AnonymousSimpleArrayField::AnonymousSimpleArrayField;
};

class AnonymousStructureField: public Field,
        public Publication,
        public ::Smp::IStructureField {

public:
    AnonymousStructureField(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent,
            ::Smp::Publication::ITypeRegistry *typeRegistry,
            ::Smp::ViewKind view, ::Smp::Bool state);

    void Restore(::Smp::IStorageReader *reader) override;

    void Store(::Smp::IStorageWriter *writer) override;

    const ::Smp::FieldCollection* GetFields() const override;

    ::Smp::IField* GetField(::Smp::String8 name) const override;
};

class ArrayField: public Field, public virtual ::Smp::IArrayField {
public:
    ArrayField(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, void *address,
            const ::Xsmp::Publication::ArrayType *type, ::Smp::ViewKind view,
            ::Smp::Bool state, ::Smp::Bool input, ::Smp::Bool output);

    void Restore(::Smp::IStorageReader *reader) override;

    void Store(::Smp::IStorageWriter *writer) override;

    ::Smp::UInt64 GetSize() const override;

    ::Smp::IField* GetItem(::Smp::UInt64 index) const override;
private:
    std::vector<std::unique_ptr<::Smp::IField>> _fields { };
};

class ArrayDataflowField: public ArrayField, public DataflowField {
public:
    using ArrayField::ArrayField;
};

class SimpleArrayField: public Field, public virtual ::Smp::ISimpleArrayField {
public:
    SimpleArrayField(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, void *address,
            const ::Xsmp::Publication::ArrayType *type, ::Smp::ViewKind view,
            ::Smp::Bool state, ::Smp::Bool input, ::Smp::Bool output);

    /// SimpleArrayField cannot be copied
    SimpleArrayField(const SimpleArrayField&) = delete;
    /// SimpleArrayField cannot be copied
    SimpleArrayField& operator=(const SimpleArrayField&) = delete;

    /// SimpleArrayField cannot be moved
    SimpleArrayField(SimpleArrayField&&) = delete;
    /// SimpleArrayField cannot be moved
    SimpleArrayField& operator=(const SimpleArrayField&&) = delete;

    void Restore(::Smp::IStorageReader *reader) override;

    void Store(::Smp::IStorageWriter *writer) override;

    ::Smp::UInt64 GetSize() const override;

    ::Smp::AnySimple GetValue(::Smp::UInt64 index) const override;

    void SetValue(::Smp::UInt64 index, ::Smp::AnySimple value) override;

    void GetValues(::Smp::UInt64 length, ::Smp::AnySimpleArray values) const
            override;

    void SetValues(::Smp::UInt64 length, ::Smp::AnySimpleArray values) override;

private:
    ::Smp::UInt64 _size;
    const ::Smp::Publication::IType *_itemType;
    ::Smp::UInt64 _itemSize;
};

class SimpleArrayDataflowField: public SimpleArrayField,
        public virtual DataflowField {
public:
    using SimpleArrayField::SimpleArrayField;
};

class SimpleField: public Field, public virtual ::Smp::ISimpleField {
public:
    using Field::Field;

    void Restore(::Smp::IStorageReader *reader) override;

    void Store(::Smp::IStorageWriter *writer) override;

    ::Smp::PrimitiveTypeKind GetPrimitiveTypeKind() const override;

    ::Smp::AnySimple GetValue() const override;

    void SetValue(::Smp::AnySimple value) override;
private:
    ::Smp::Int64 GetSize() const;
};

class SimpleDataflowField: public SimpleField, public DataflowField {
public:
    using SimpleField::SimpleField;
};

class StructureField: public Field, public virtual ::Smp::IStructureField {
public:

    StructureField(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, void *address,
            const ::Xsmp::Publication::StructureType *type,
            ::Smp::ViewKind view, ::Smp::Bool state, ::Smp::Bool input,
            ::Smp::Bool output);

    void Restore(::Smp::IStorageReader *reader) override;

    void Store(::Smp::IStorageWriter *writer) override;

    const ::Smp::FieldCollection* GetFields() const override;

    ::Smp::IField* GetField(::Smp::String8 name) const override;
private:
    ::Xsmp::ContainingCollection<::Smp::IField> _fields;
};

class StructureDataflowField: public StructureField, public DataflowField {
public:
    using StructureField::StructureField;
};
} // namespace Xsmp::Publication

#endif // XSMP_PUBLICATION_FIELD_H_
