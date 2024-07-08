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
#include <Smp/IForcibleField.h>
#include <Smp/IStructureField.h>
#include <Smp/InvalidArrayIndex.h>
#include <Smp/InvalidFieldValue.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IEnumerationType.h>
#include <Smp/Uuid.h>
#include <Xsmp/Array.h>
#include <Xsmp/Field.h>
#include <Xsmp/Publication/TypeRegistry.h>
#include <Xsmp/String.h>
#include <gtest/gtest.h>
#include <type_traits>

namespace Xsmp {

// static checks

using field = Field<Xsmp::Array<::Smp::Bool, 1>::simple>;

static_assert(!std::is_base_of_v<Smp::ISimpleField, field>);
static_assert(!std::is_base_of_v<Smp::IDataflowField, field>);
static_assert(!std::is_base_of_v<Smp::IForcibleField, field>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field>);
static_assert(std::is_base_of_v<Smp::ISimpleArrayField, field>);
static_assert(!std::is_base_of_v<Smp::IStructureField, field>);
static_assert(!std::is_base_of_v<Smp::IFailure, field>);

static_assert(!std::is_base_of_v<Smp::ISimpleField, field::failure>);
static_assert(!std::is_base_of_v<Smp::IDataflowField, field::failure>);
static_assert(!std::is_base_of_v<Smp::IForcibleField, field::failure>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field::failure>);
static_assert(std::is_base_of_v<Smp::ISimpleArrayField, field::failure>);
static_assert(!std::is_base_of_v<Smp::IStructureField, field::failure>);
static_assert(std::is_base_of_v<Smp::IFailure, field::failure>);

static_assert(!std::is_base_of_v<Smp::ISimpleField, field::output>);
static_assert(std::is_base_of_v<Smp::IDataflowField, field::output>);
static_assert(!std::is_base_of_v<Smp::IForcibleField, field::output>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field::output>);
static_assert(std::is_base_of_v<Smp::ISimpleArrayField, field::output>);
static_assert(!std::is_base_of_v<Smp::IStructureField, field::output>);
static_assert(!std::is_base_of_v<Smp::IFailure, field::output>);

enum class Enum { L1, L2, L3 };
using String20 = ::Xsmp::String<20>;
TEST(SimpleArrayField, BoolType) {

  Xsmp::Publication::TypeRegistry typeRegistry;
  using Type = Array<Smp::Bool, 4>::simple;

  typeRegistry.AddArrayType("BoolSimpleArray", "", Smp::Uuid{},
                            Smp::Uuids::Uuid_Bool, sizeof(Smp::Bool), 4, true);

  Field<Type> field{&typeRegistry, Smp::Uuid{}, "name"};

  EXPECT_TRUE(field.IsState());
  EXPECT_FALSE(field.IsOutput());
  EXPECT_FALSE(field.IsInput());

  EXPECT_FALSE(field[0]);
  field[0] = true;
  EXPECT_TRUE(field[0]);

  field.SetValue(0, {::Smp::PrimitiveTypeKind::PTK_Bool, false});
  EXPECT_EQ(field[0], false);
  EXPECT_EQ(static_cast<::Smp::Bool>(field.GetValue(0)), false);

  Field<Type>::input input{&typeRegistry, Smp::Uuid{}, "input"};
  EXPECT_TRUE(input.IsState());
  EXPECT_FALSE(input.IsOutput());
  EXPECT_TRUE(input.IsInput());

  Field<Type>::output output{&typeRegistry, Smp::Uuid{}, "output"};
  EXPECT_TRUE(output.IsState());
  EXPECT_TRUE(output.IsOutput());
  EXPECT_FALSE(output.IsInput());

  output.Connect(&input);

  EXPECT_EQ(input[0], false);
  output[0] = true;

  EXPECT_EQ(input[0], true);

  output.at(0) = false;
  EXPECT_EQ(input.at(0), false);
  output.front() = true;
  EXPECT_EQ(input.front(), true);

  *output.begin() = false;
  EXPECT_EQ(*input.begin(), false);

  output.back() = true;
  EXPECT_EQ(input.back(), true);

  EXPECT_THROW(output.GetValue(42), Smp::InvalidArrayIndex);
  EXPECT_THROW(output.SetValue(42, {::Smp::PrimitiveTypeKind::PTK_Bool, false}),
               Smp::InvalidArrayIndex);
  EXPECT_THROW(output.SetValue(0, {::Smp::PrimitiveTypeKind::PTK_Int8, 42}),
               Smp::InvalidFieldValue);
}

TEST(SimpleArrayField, Int8Type) {
  Xsmp::Publication::TypeRegistry typeRegistry;
  using Type = Array<Smp::Int8, 4>::simple;
  typeRegistry.AddArrayType("Int8SimpleArray", "", Smp::Uuid{},
                            Smp::Uuids::Uuid_Int8, sizeof(Smp::Int8), 4, true);

  Field<Type> field{&typeRegistry, Smp::Uuid{}, "name"};

  EXPECT_TRUE(field.IsState());
  EXPECT_FALSE(field.IsOutput());
  EXPECT_FALSE(field.IsInput());

  EXPECT_FALSE(field[0]);
  field[0] = 42;
  EXPECT_EQ(field[0], 42);
  Type v{42, 0, 0, 0};
  EXPECT_EQ(field, v);

  field.SetValue(0, {::Smp::PrimitiveTypeKind::PTK_Int8, 10});
  EXPECT_EQ(field[0], 10);
  EXPECT_EQ(static_cast<::Smp::Int8>(field.GetValue(0)), 10);
  EXPECT_NE(field, v);

  Field<Type>::input input{&typeRegistry, Smp::Uuid{}, "input"};
  EXPECT_TRUE(input.IsState());
  EXPECT_FALSE(input.IsOutput());
  EXPECT_TRUE(input.IsInput());

  Field<Type>::output output{&typeRegistry, Smp::Uuid{}, "output"};
  EXPECT_TRUE(output.IsState());
  EXPECT_TRUE(output.IsOutput());
  EXPECT_FALSE(output.IsInput());

  output.Connect(&input);

  EXPECT_EQ(input[0], 0);
  output[0] = 42;

  EXPECT_EQ(input, output);

  EXPECT_EQ(input[0], 42);

  Smp::Int8 i = 0;
  for (auto elem : output) {
    elem = i++;
  }
  i = 0;
  for (const auto &elem : input) {
    EXPECT_EQ(elem, i++);
  }
  i = 10;
  for (auto it = output.begin(); it != output.end(); ++it) {
    *it = i++;
  }
  i = 10;
  for (auto *it = input.begin(); it != input.end(); ++it) {
    EXPECT_EQ(*it, i++);
  }
  /*
   i = 20;
   for (auto it = output.rbegin(); it != output.rend(); ++it)
   *it = i++;

   i = 20;
   for (auto it = input.rbegin(); it != input.rend(); ++it)
   EXPECT_EQ(*it, i++);
   */
}

TEST(SimpleArrayField, EnumType) {
  Xsmp::Publication::TypeRegistry typeRegistry;
  using Type = Array<Enum, 4>::simple;
  const Smp::Uuid enumUuid{0, 1, 2, 3, 4};
  auto *enumType =
      typeRegistry.AddEnumerationType("Enum", "", enumUuid, sizeof(Enum));
  enumType->AddLiteral("L1", "", static_cast<Smp::Int32>(Enum::L1));
  typeRegistry.AddArrayType("EnumSimpleArray", "", Smp::Uuid{}, enumUuid,
                            sizeof(Enum), 4, true);

  Field<Type> field{&typeRegistry, Smp::Uuid{}, "name"};

  EXPECT_TRUE(field.IsState());
  EXPECT_FALSE(field.IsOutput());
  EXPECT_FALSE(field.IsInput());

  EXPECT_EQ(field[0], Enum::L1);
  field[0] = Enum::L2;
  EXPECT_EQ(field[0], Enum::L2);

  field.SetValue(0, {::Smp::PrimitiveTypeKind::PTK_Int32,
                     static_cast<Smp::Int32>(Enum::L3)});
  EXPECT_EQ(field[0], Enum::L3);
  EXPECT_EQ(static_cast<::Smp::Int32>(field.GetValue(0)),
            static_cast<Smp::Int32>(Enum::L3));

  Field<Type>::input input{&typeRegistry, Smp::Uuid{}, "input"};
  EXPECT_TRUE(input.IsState());
  EXPECT_FALSE(input.IsOutput());
  EXPECT_TRUE(input.IsInput());

  Field<Type>::output output{&typeRegistry, Smp::Uuid{}, "output"};
  EXPECT_TRUE(output.IsState());
  EXPECT_TRUE(output.IsOutput());
  EXPECT_FALSE(output.IsInput());

  output.Connect(&input);

  EXPECT_EQ(input[0], Enum::L1);
  output[0] = Enum::L2;

  EXPECT_EQ(input[0], Enum::L2);
}

TEST(SimpleArrayField, StringType) {
  Xsmp::Publication::TypeRegistry typeRegistry;
  using Type = Array<String20, 4>::simple;
  const Smp::Uuid stringUuid{0, 1, 2, 3, 4};
  typeRegistry.AddStringType("String20", "", stringUuid, 20);

  typeRegistry.AddArrayType("String20SimpleArray", "", Smp::Uuid{}, stringUuid,
                            sizeof(String20), 4, true);

  Field<Type> field{&typeRegistry, Smp::Uuid{}, "name"};

  EXPECT_TRUE(field.IsState());
  EXPECT_FALSE(field.IsOutput());
  EXPECT_FALSE(field.IsInput());

  EXPECT_EQ(field[0], "");
  field[0] = "test";
  EXPECT_EQ(field[0], "test");

  field.SetValue(0, {::Smp::PrimitiveTypeKind::PTK_String8, "value"});
  EXPECT_EQ(field[0], "value");
  EXPECT_STREQ(static_cast<::Smp::String8>(field.GetValue(0)), "value");

  Field<Type>::input input{&typeRegistry, Smp::Uuid{}, "input"};
  EXPECT_TRUE(input.IsState());
  EXPECT_FALSE(input.IsOutput());
  EXPECT_TRUE(input.IsInput());

  Field<Type>::output output{&typeRegistry, Smp::Uuid{}, "output"};
  EXPECT_TRUE(output.IsState());
  EXPECT_TRUE(output.IsOutput());
  EXPECT_FALSE(output.IsInput());

  output.Connect(&input);

  EXPECT_EQ(input[0], "");
  output[0] = "test";

  EXPECT_EQ(input[0], "test");
}

} // namespace Xsmp
