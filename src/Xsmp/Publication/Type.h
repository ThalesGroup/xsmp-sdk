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

#ifndef XSMP_PUBLICATION_TYPE_H_
#define XSMP_PUBLICATION_TYPE_H_

#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IArrayType.h>
#include <Smp/Publication/IClassType.h>
#include <Smp/Publication/IEnumerationType.h>
#include <Smp/Uuid.h>
#include <Smp/ViewKind.h>
#include <map>
#include <string>
#include <vector>

namespace Smp {
class IPublication;
} // namespace Smp

namespace Xsmp::Publication {
class TypeRegistry;

class Type: public virtual ::Smp::Publication::IType {
public:
    Type(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::Publication::TypeRegistry *typeRegistry, ::Smp::Uuid uuid);
    /// Type cannot be copied
    Type(const Type&) = delete;
    /// Type cannot be copied
    Type& operator=(const Type&) = delete;
    ~Type() noexcept override = default;
    ::Smp::String8 GetName() const final;
    ::Smp::String8 GetDescription() const final;
    ::Smp::IObject* GetParent() const final;
    ::Smp::PrimitiveTypeKind GetPrimitiveTypeKind() const override;

    ::Smp::Uuid GetUuid() const final;

    void Publish(::Smp::IPublication *receiver, ::Smp::String8 name,
            ::Smp::String8 description, void *address, ::Smp::ViewKind view =
                    ::Smp::ViewKind::VK_All, ::Smp::Bool state = true,
            ::Smp::Bool input = false, ::Smp::Bool output = false) final;
    ::Xsmp::Publication::TypeRegistry* GetTypeRegistry() const noexcept;
private:
    std::string _name;
    std::string _description;
    ::Xsmp::Publication::TypeRegistry *_parent;
    ::Smp::Uuid _uuid;
};

class ArrayType: public Type, public virtual ::Smp::Publication::IArrayType {
public:
    ArrayType(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::Publication::TypeRegistry *typeRegistry,
            ::Smp::Uuid typeUuid, ::Smp::Uuid itemTypeUuid,
            ::Smp::Int64 itemSize, ::Smp::Int64 arrayCount,
            ::Smp::Bool simpleArray);
    ~ArrayType() noexcept override = default;
    ArrayType(const ArrayType&) = delete;
    ArrayType& operator=(const ArrayType&) = delete;

    ::Smp::UInt64 GetSize() const override;

    const ::Smp::Publication::IType* GetItemType() const override;

    ::Smp::UInt64 GetItemSize() const;
    ::Smp::Bool IsSimpleArray() const;
private:
    ::Smp::Uuid _itemTypeUuid;
    ::Smp::UInt64 _itemSize;
    ::Smp::UInt64 _arrayCount;
    ::Smp::Bool _simpleArray;

};

class SimpleType: public Type {
public:
    SimpleType(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::Publication::TypeRegistry *parent,
            ::Smp::PrimitiveTypeKind kind, ::Smp::Uuid uuid);

    ~SimpleType() noexcept override = default;

    ::Smp::PrimitiveTypeKind GetPrimitiveTypeKind() const final;

private:
    ::Smp::PrimitiveTypeKind _kind;
};

class EnumerationType final: public SimpleType,
        public ::Smp::Publication::IEnumerationType {
public:
    EnumerationType(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::Publication::TypeRegistry *parent, ::Smp::Uuid typeUuid,
            ::Smp::Int16 memorySize);
    ~EnumerationType() noexcept override = default;

    void AddLiteral(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::Int32 value) override;

    struct Literal {
        std::string name;
        std::string description;
    };
    const std::map<::Smp::Int32, Literal>& GetLiterals() const noexcept;
private:
    std::map<::Smp::Int32, Literal> _literals { };
};

class FloatType final: public SimpleType {
public:
    FloatType(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::Publication::TypeRegistry *parent, ::Smp::Uuid typeUuid,
            ::Smp::Float64 minimum, ::Smp::Float64 maximum,
            ::Smp::Bool minInclusive, ::Smp::Bool maxInclusive,
            ::Smp::String8 unit, ::Smp::PrimitiveTypeKind type);
    ~FloatType() noexcept override = default;
    ::Smp::Float64 getMinimum() const;
    ::Smp::Float64 getMaximum() const;
    ::Smp::String8 getUnit() const;
    ::Smp::Bool IsMinInclusive() const;
    ::Smp::Bool IsMaxInclusive() const;
private:
    std::string _unit;
    ::Smp::Float64 _minimum;
    ::Smp::Float64 _maximum;
    ::Smp::Bool _minInclusive;
    ::Smp::Bool _maxInclusive;
};

class IntegerType final: public SimpleType {
public:
    IntegerType(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::Publication::TypeRegistry *parent, ::Smp::Uuid typeUuid,
            ::Smp::Int64 minimum, ::Smp::Int64 maximum, ::Smp::String8 unit,
            ::Smp::PrimitiveTypeKind type);
    ~IntegerType() noexcept override = default;

    ::Smp::Int64 getMinimum() const;
    ::Smp::Int64 getMaximum() const;
    ::Smp::String8 getUnit() const;
private:
    ::Smp::Int64 _minimum;
    ::Smp::Int64 _maximum;
    std::string _unit;
};

class PrimitiveType final: public SimpleType {
public:
    PrimitiveType(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::Publication::TypeRegistry *parent,
            ::Smp::PrimitiveTypeKind kind, ::Smp::Uuid uuid);

    ~PrimitiveType() noexcept override = default;
};

class StringType final: public SimpleType {
public:
    StringType(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::Publication::TypeRegistry *parent, ::Smp::Uuid typeUuid,
            ::Smp::Int64 length);
    ~StringType() noexcept override = default;

    ::Smp::Int64 GetLength() const;
private:
    ::Smp::Int64 _length;
};

class StructureType: public Type,
        public virtual ::Smp::Publication::IStructureType {

public:
    StructureType(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::Publication::TypeRegistry *typeRegistry,
            ::Smp::Uuid typeUuid);
    ~StructureType() noexcept override = default;
    StructureType(const StructureType&) = delete;
    StructureType& operator=(const StructureType&) = delete;

    void AddField(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::Uuid uuid, ::Smp::Int64 offset, ::Smp::ViewKind view =
                    ::Smp::ViewKind::VK_All, ::Smp::Bool state = true,
            ::Smp::Bool input = false, ::Smp::Bool output = false) override;

    struct Field {
        std::string name;
        std::string description;
        ::Smp::Uuid uuid;
        ::Smp::Int64 offset;
        ::Smp::ViewKind view;
        ::Smp::Bool state;
        ::Smp::Bool input;
        ::Smp::Bool output;
    };

    const std::vector<Field>& GetFields() const;
private:
    std::vector<Field> _fields { };
};

class ClassType final: public StructureType,
        public virtual ::Smp::Publication::IClassType {
public:
    ClassType(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::Publication::TypeRegistry *typeRegistry,
            ::Smp::Uuid typeUuid, ::Smp::Uuid baseClassUuid);
    ~ClassType() noexcept override = default;
    ClassType(const ClassType&) = delete;
    ClassType& operator=(const ClassType&) = delete;
private:
    ::Smp::Uuid _baseClassUuid;
};

} // namespace Xsmp::Publication

#endif // XSMP_PUBLICATION_TYPE_H_
