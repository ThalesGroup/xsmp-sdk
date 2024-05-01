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
#include <Smp/ISimpleArrayField.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/Uuid.h>
#include <Smp/ViewKind.h>
#include <Xsmp/Field.h>
#include <Xsmp/Publication/TypeRegistry.h>
#include <Xsmp/Storage.h>
#include <cstddef>
#include <gtest/gtest.h>
#include <type_traits>

namespace Xsmp {

struct StructTest {
  ::Smp::Bool booleanInput;
  ::Smp::Char8 char8Input;

  template <typename _BASE> struct _Field : public _BASE {
    // constructor
    // template<typename ..._Args>
    _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
           ::Smp::Uuid typeUuid, ::Smp::String8 name,
           ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
           ::Smp::ViewKind view = ::Smp::ViewKind::VK_None,
           const StructTest &value = {})
        : _BASE(typeRegistry, typeUuid, name, description, parent, view),
          /// booleanInput initialization
          booleanInput{
              typeRegistry, ::Smp::Uuids::Uuid_Bool, "booleanInput", "", this,
              view,         value.booleanInput},
          /// char8Input initialization
          char8Input{
              typeRegistry, ::Smp::Uuids::Uuid_Char8, "char8Input", "", this,
              view,         value.char8Input} {}
    _Field(const _Field &) = delete;
    _Field &operator=(const _Field &) = delete;

    // copy operator from ::Xsmp::Tests::Types::StructureWithCustomFields
    _Field &operator=(const StructTest &other) {
      this->booleanInput = other.booleanInput;
      this->char8Input = other.char8Input;
      return *this;
    }

    // convert to ::Xsmp::Tests::Types::StructureWithCustomFields
    operator StructTest() const noexcept { return {booleanInput, char8Input}; }

    // Fields declaration
    typename _BASE::template Field<::Smp::Bool>::input booleanInput;
    typename _BASE::template Field<::Smp::Char8>::input char8Input;
  };
};

// static checks
using field = Field<StructTest>;

static_assert(!std::is_base_of_v<Smp::ISimpleField, field>);
static_assert(!std::is_base_of_v<Smp::IDataflowField, field>);
static_assert(!std::is_base_of_v<Smp::IForcibleField, field>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field>);
static_assert(!std::is_base_of_v<Smp::ISimpleArrayField, field>);
static_assert(std::is_base_of_v<Smp::IStructureField, field>);
static_assert(!std::is_base_of_v<Smp::IFailure, field>);

static_assert(!std::is_base_of_v<Smp::ISimpleField, field::transient>);
static_assert(!std::is_base_of_v<Smp::IDataflowField, field::transient>);
static_assert(!std::is_base_of_v<Smp::IForcibleField, field::transient>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field::transient>);
static_assert(!std::is_base_of_v<Smp::ISimpleArrayField, field::transient>);
static_assert(std::is_base_of_v<Smp::IStructureField, field::transient>);
static_assert(!std::is_base_of_v<Smp::IFailure, field::transient>);

static_assert(!std::is_base_of_v<Smp::ISimpleField, field::output>);
static_assert(std::is_base_of_v<Smp::IDataflowField, field::output>);
static_assert(!std::is_base_of_v<Smp::IForcibleField, field::output>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field::output>);
static_assert(!std::is_base_of_v<Smp::ISimpleArrayField, field::output>);
static_assert(std::is_base_of_v<Smp::IStructureField, field::output>);
static_assert(!std::is_base_of_v<Smp::IFailure, field::output>);

static_assert(!std::is_base_of_v<Smp::ISimpleField, field::forcible>);
static_assert(!std::is_base_of_v<Smp::IDataflowField, field::forcible>);
static_assert(!std::is_base_of_v<Smp::IForcibleField, field::forcible>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field::forcible>);
static_assert(!std::is_base_of_v<Smp::ISimpleArrayField, field::forcible>);
static_assert(std::is_base_of_v<Smp::IStructureField, field::forcible>);
static_assert(!std::is_base_of_v<Smp::IFailure, field::forcible>);

static_assert(!std::is_base_of_v<Smp::ISimpleField, field::failure>);
static_assert(!std::is_base_of_v<Smp::IDataflowField, field::failure>);
static_assert(!std::is_base_of_v<Smp::IForcibleField, field::failure>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field::failure>);
static_assert(!std::is_base_of_v<Smp::ISimpleArrayField, field::failure>);
static_assert(std::is_base_of_v<Smp::IStructureField, field::failure>);
static_assert(std::is_base_of_v<Smp::IFailure, field::failure>);

TEST(StructureFieldTest, Instanciate) {
  Xsmp::Publication::TypeRegistry typeRegistry;

  auto *pStructure = typeRegistry.AddStructureType("StructTest" /// Name
                                                   ,
                                                   "" /// description
                                                   ,
                                                   {} /// UUID
  );

  /// Register the Fields of the Structure
  pStructure->AddField(
      "booleanInput", "", ::Smp::Uuids::Uuid_Bool /// UUID of the Field Type
      ,
      offsetof(StructTest,
               booleanInput) /// Compute the offset of the current item
      ,
      ::Smp::ViewKind::VK_None /// viewkind
      ,
      true /// state
      ,
      true /// is an input field
      ,
      false /// is an output field
  );
  pStructure->AddField(
      "char8Input", "", ::Smp::Uuids::Uuid_Char8 /// UUID of the Field Type
      ,
      offsetof(StructTest,
               char8Input) /// Compute the offset of the current item
      ,
      ::Smp::ViewKind::VK_None /// viewkind
      ,
      true /// state
      ,
      true /// is an input field
      ,
      false /// is an output field
  );

  Field<StructTest> field{&typeRegistry, ::Smp::Uuid{}, "name"};

  EXPECT_TRUE(field.IsState());
  EXPECT_FALSE(field.IsOutput());
  EXPECT_FALSE(field.IsInput());
  EXPECT_EQ(field.GetType(), pStructure);
  EXPECT_EQ(field.GetField("booleanInput"), &field.booleanInput);
  EXPECT_EQ(field.GetField("char8Input"), &field.char8Input);

  Field<StructTest>::forcible::failure toStore{&typeRegistry, {}, "name"};
  toStore.booleanInput.Force(true);
  toStore.char8Input = 'a';
  toStore.Fail();

  Storage storage;
  toStore.Store(&storage);

  Field<StructTest>::forcible::failure toRestore{&typeRegistry, {}, "name"};
  toRestore.Restore(&storage);

  EXPECT_TRUE(toRestore.IsFailed());
  EXPECT_TRUE(toRestore.booleanInput);
  EXPECT_TRUE(toRestore.booleanInput.IsForced());
  EXPECT_EQ(toRestore.char8Input, 'a');

  Field<StructTest>::output output{&typeRegistry, ::Smp::Uuid{}, "output"};

  output.booleanInput = true;
  output.char8Input = 'i';

  Field<StructTest>::input input{&typeRegistry, ::Smp::Uuid{}, "input"};
  output.Connect(&input);
  EXPECT_EQ(output.booleanInput, input.booleanInput);
  EXPECT_EQ(output.char8Input, input.char8Input);

  output.Push();
  EXPECT_EQ(output.booleanInput, input.booleanInput);
  EXPECT_EQ(output.char8Input, input.char8Input);
  output.booleanInput = false;
  output.char8Input = 'a';

  EXPECT_EQ(output.booleanInput, input.booleanInput);
  EXPECT_EQ(output.char8Input, input.char8Input);
}

} // namespace Xsmp
