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
#include <Smp/ISimpleArrayField.h>
#include <Smp/IStructureField.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Uuid.h>
#include <Xsmp/Field.h>
#include <Xsmp/Publication/TypeRegistry.h>
#include <Xsmp/Storage.h>
#include <Xsmp/String.h>
#include <gtest/gtest.h>
#include <type_traits>

namespace Xsmp {

// static checks
using field = Field<::Smp::Bool>;

static_assert(std::is_base_of_v<Smp::ISimpleField, field>);
static_assert(!std::is_base_of_v<Smp::IDataflowField, field>);
static_assert(!std::is_base_of_v<Smp::IForcibleField, field>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field>);
static_assert(!std::is_base_of_v<Smp::ISimpleArrayField, field>);
static_assert(!std::is_base_of_v<Smp::IStructureField, field>);
static_assert(!std::is_base_of_v<Smp::IFailure, field>);

static_assert(std::is_base_of_v<Smp::ISimpleField, field::transient>);
static_assert(!std::is_base_of_v<Smp::IDataflowField, field::transient>);
static_assert(!std::is_base_of_v<Smp::IForcibleField, field::transient>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field::transient>);
static_assert(!std::is_base_of_v<Smp::ISimpleArrayField, field::transient>);
static_assert(!std::is_base_of_v<Smp::IStructureField, field::transient>);
static_assert(!std::is_base_of_v<Smp::IFailure, field::transient>);

static_assert(std::is_base_of_v<Smp::ISimpleField, field::input>);
static_assert(!std::is_base_of_v<Smp::IDataflowField, field::input>);
static_assert(!std::is_base_of_v<Smp::IForcibleField, field::input>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field::input>);
static_assert(!std::is_base_of_v<Smp::ISimpleArrayField, field::input>);
static_assert(!std::is_base_of_v<Smp::IStructureField, field::input>);
static_assert(!std::is_base_of_v<Smp::IFailure, field::input>);

static_assert(std::is_base_of_v<Smp::ISimpleField, field::output>);
static_assert(std::is_base_of_v<Smp::IDataflowField, field::output>);
static_assert(!std::is_base_of_v<Smp::IForcibleField, field::output>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field::output>);
static_assert(!std::is_base_of_v<Smp::ISimpleArrayField, field::output>);
static_assert(!std::is_base_of_v<Smp::IStructureField, field::output>);
static_assert(!std::is_base_of_v<Smp::IFailure, field::output>);

static_assert(std::is_base_of_v<Smp::ISimpleField, field::forcible>);
static_assert(!std::is_base_of_v<Smp::IDataflowField, field::forcible>);
static_assert(std::is_base_of_v<Smp::IForcibleField, field::forcible>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field::forcible>);
static_assert(!std::is_base_of_v<Smp::ISimpleArrayField, field::forcible>);
static_assert(!std::is_base_of_v<Smp::IStructureField, field::forcible>);
static_assert(!std::is_base_of_v<Smp::IFailure, field::forcible>);

static_assert(std::is_base_of_v<Smp::ISimpleField, field::failure>);
static_assert(!std::is_base_of_v<Smp::IDataflowField, field::failure>);
static_assert(!std::is_base_of_v<Smp::IForcibleField, field::failure>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field::failure>);
static_assert(!std::is_base_of_v<Smp::ISimpleArrayField, field::failure>);
static_assert(!std::is_base_of_v<Smp::IStructureField, field::failure>);
static_assert(std::is_base_of_v<Smp::IFailure, field::failure>);

static_assert(std::is_base_of_v<Smp::ISimpleField, field::failure::output>);
static_assert(std::is_base_of_v<Smp::IDataflowField, field::failure::output>);
static_assert(!std::is_base_of_v<Smp::IForcibleField, field::failure::output>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field::failure::output>);
static_assert(
    !std::is_base_of_v<Smp::ISimpleArrayField, field::failure::output>);
static_assert(!std::is_base_of_v<Smp::IStructureField, field::failure::output>);
static_assert(std::is_base_of_v<Smp::IFailure, field::failure::output>);

static_assert(std::is_base_of_v<Smp::ISimpleField, field::output::failure>);
static_assert(std::is_base_of_v<Smp::IDataflowField, field::output::failure>);
static_assert(!std::is_base_of_v<Smp::IForcibleField, field::output::failure>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field::output::failure>);
static_assert(
    !std::is_base_of_v<Smp::ISimpleArrayField, field::output::failure>);
static_assert(!std::is_base_of_v<Smp::IStructureField, field::output::failure>);
static_assert(std::is_base_of_v<Smp::IFailure, field::output::failure>);

static_assert(std::is_base_of_v<Smp::ISimpleField, field::forcible::output>);
static_assert(std::is_base_of_v<Smp::IDataflowField, field::forcible::output>);
static_assert(std::is_base_of_v<Smp::IForcibleField, field::forcible::output>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field::forcible::output>);
static_assert(
    !std::is_base_of_v<Smp::ISimpleArrayField, field::forcible::output>);
static_assert(
    !std::is_base_of_v<Smp::IStructureField, field::forcible::output>);
static_assert(!std::is_base_of_v<Smp::IFailure, field::forcible::output>);

static_assert(std::is_base_of_v<Smp::ISimpleField, field::output::forcible>);
static_assert(std::is_base_of_v<Smp::IDataflowField, field::output::forcible>);
static_assert(std::is_base_of_v<Smp::IForcibleField, field::output::forcible>);
static_assert(!std::is_base_of_v<Smp::IArrayField, field::output::forcible>);
static_assert(
    !std::is_base_of_v<Smp::ISimpleArrayField, field::output::forcible>);
static_assert(
    !std::is_base_of_v<Smp::IStructureField, field::output::forcible>);
static_assert(!std::is_base_of_v<Smp::IFailure, field::output::forcible>);

TEST(SimpleFieldTest, BoolType) {
  Xsmp::Publication::TypeRegistry typeRegistry;
  Field<bool> field{&typeRegistry, Smp::Uuids::Uuid_Bool, "name"};

  EXPECT_TRUE(field.IsState());
  EXPECT_FALSE(field.IsOutput());
  EXPECT_FALSE(field.IsInput());

  field = false;
  field = field || false;

  EXPECT_EQ(field, false);
  EXPECT_EQ(false, field);
  EXPECT_NE(field, true);
  EXPECT_NE(true, field);
  field = true;
  EXPECT_EQ(field, true);
  field.SetValue({::Smp::PrimitiveTypeKind::PTK_Bool, false});
  EXPECT_EQ(field, false);
  EXPECT_EQ(static_cast<::Smp::Bool>(field.GetValue()), false);

  Field<::Smp::Bool>::input input{&typeRegistry, Smp::Uuids::Uuid_Bool,
                                  "input"};
  EXPECT_TRUE(input.IsState());
  EXPECT_FALSE(input.IsOutput());
  EXPECT_TRUE(input.IsInput());

  Field<::Smp::Bool>::output output{&typeRegistry, Smp::Uuids::Uuid_Bool,
                                    "output"};
  EXPECT_TRUE(output.IsState());
  EXPECT_TRUE(output.IsOutput());
  EXPECT_FALSE(output.IsInput());

  EXPECT_NO_THROW(output.Connect(&input));

  EXPECT_FALSE(input);
  output = true;

  EXPECT_TRUE(input);

  Field<bool>::forcible::failure toStore{&typeRegistry, Smp::Uuids::Uuid_Bool,
                                         "name"};
  toStore.Force(true);
  toStore.Fail();

  Storage storage;
  toStore.Store(&storage);

  Field<bool>::forcible::failure toRestore{&typeRegistry, Smp::Uuids::Uuid_Bool,
                                           "name"};
  toRestore.Restore(&storage);

  EXPECT_TRUE(toRestore);
  EXPECT_TRUE(toRestore.IsFailed());
  EXPECT_TRUE(toRestore.IsForced());
}

TEST(SimpleFieldTest, Int8Type) {

  Xsmp::Publication::TypeRegistry typeRegistry;
  Field<::Smp::Int8> field{&typeRegistry, Smp::Uuids::Uuid_Int8, "name"};

  EXPECT_TRUE(field.IsState());
  EXPECT_FALSE(field.IsOutput());
  EXPECT_FALSE(field.IsInput());

  field = 0;
  field = field + 0;

  EXPECT_EQ(field, 0);
  EXPECT_EQ(0, field);
  EXPECT_NE(field, 1);
  EXPECT_NE(1, field);
  EXPECT_GT(field, -1);
  EXPECT_GT(1, field);
  EXPECT_LT(field, 1);
  EXPECT_LT(-1, field);
  EXPECT_GE(field, -1);
  EXPECT_GE(1, field);
  EXPECT_LE(field, 1);
  EXPECT_LE(-1, field);

  field += static_cast<::Smp::Int8>(-1);
  EXPECT_EQ(field, -1);
  const ::Smp::Int8 raw = field;
  EXPECT_EQ(raw, -1);
  field.SetValue({::Smp::PrimitiveTypeKind::PTK_Int8, 1});
  EXPECT_EQ(field, 1);
  EXPECT_EQ(static_cast<::Smp::Int8>(field.GetValue()), 1);

  Field<::Smp::Int8>::input input{&typeRegistry, Smp::Uuids::Uuid_Int8,
                                  "input"};
  EXPECT_TRUE(input.IsState());
  EXPECT_FALSE(input.IsOutput());
  EXPECT_TRUE(input.IsInput());

  Field<::Smp::Int8>::output output{&typeRegistry, Smp::Uuids::Uuid_Int8,
                                    "output"};
  EXPECT_TRUE(output.IsState());
  EXPECT_TRUE(output.IsOutput());
  EXPECT_FALSE(output.IsInput());

  output.Connect(&input);

  EXPECT_EQ(input, 0);
  output = 42;

  EXPECT_EQ(input, 42);
  EXPECT_EQ(input, output);
  EXPECT_EQ(output, output);
  EXPECT_EQ(output, input);

  EXPECT_EQ(input | 0, 42);
  EXPECT_EQ(input & 42, 42);
  EXPECT_EQ(input ^ 0, 42);
  EXPECT_EQ(input / 2, 21);
  EXPECT_EQ(input * 2, 84);
  EXPECT_EQ(input + 2, 44);
  EXPECT_EQ(input - 2, 40);

  ++output;
  EXPECT_EQ(input, 43);

  --output;
  EXPECT_EQ(input, 42);

  output += 2;
  EXPECT_EQ(input, 44);

  output -= 2;
  EXPECT_EQ(input, 42);

  output *= 2;
  EXPECT_EQ(input, 84);

  output /= 2;
  EXPECT_EQ(input, 42);

  output <<= 1;
  EXPECT_EQ(input, 84);

  output >>= 1;
  EXPECT_EQ(input, 42);

  output %= 42;
  EXPECT_EQ(input, 0);

  output |= 42;
  EXPECT_EQ(input, 42);

  output &= 0;
  EXPECT_EQ(input, 0);

  output ^= 42;
  EXPECT_EQ(input, 42);

  EXPECT_TRUE(input == 42);
  EXPECT_TRUE(input == input);
  EXPECT_TRUE(42 == input);

  EXPECT_FALSE(input != input);
  EXPECT_TRUE(input != 43);
  EXPECT_TRUE(43 != input);

  EXPECT_FALSE(input < input);
  EXPECT_TRUE(input < 43);
  EXPECT_TRUE(41 < input);

  EXPECT_TRUE(input <= input);
  EXPECT_TRUE(input <= 42);
  EXPECT_TRUE(42 <= input);

  EXPECT_FALSE(input > input);
  EXPECT_TRUE(input > 41);
  EXPECT_TRUE(43 > input);

  EXPECT_TRUE(input >= input);
  EXPECT_TRUE(input >= 42);
  EXPECT_TRUE(42 >= input);

  Field<::Smp::Int8>::forcible::failure toStore{&typeRegistry,
                                                Smp::Uuids::Uuid_Int8, "name"};
  toStore = 42;

  Storage storage;
  toStore.Store(&storage);

  Field<::Smp::Int8>::forcible::failure toRestore{
      &typeRegistry, Smp::Uuids::Uuid_Int8, "name"};
  toRestore.Restore(&storage);

  EXPECT_EQ(toRestore, 42);
  EXPECT_FALSE(toRestore.IsFailed());
  EXPECT_FALSE(toRestore.IsForced());
}

enum class Enum { L1, L2, L3 };
TEST(SimpleFieldTest, EnumType) {
  Xsmp::Publication::TypeRegistry typeRegistry;
  Field<Enum> field{&typeRegistry, Smp::Uuids::Uuid_Int32, "name"};

  EXPECT_TRUE(field.IsState());
  EXPECT_FALSE(field.IsOutput());
  EXPECT_FALSE(field.IsInput());

  field = Enum::L1;

  EXPECT_EQ(field, Enum::L1);
  field = Enum::L2;
  EXPECT_EQ(field, Enum::L2);
  field.SetValue({::Smp::PrimitiveTypeKind::PTK_Int32,
                  static_cast<::Smp::Int32>(Enum::L3)});
  EXPECT_EQ(field, Enum::L3);
  EXPECT_EQ(static_cast<Enum>(static_cast<::Smp::Int32>(field.GetValue())),
            Enum::L3);

  Field<Enum>::input input{&typeRegistry, Smp::Uuids::Uuid_Int32, "input"};
  EXPECT_TRUE(input.IsState());
  EXPECT_FALSE(input.IsOutput());
  EXPECT_TRUE(input.IsInput());

  Field<Enum>::output output{&typeRegistry, Smp::Uuids::Uuid_Int32, "output"};
  EXPECT_TRUE(output.IsState());
  EXPECT_TRUE(output.IsOutput());
  EXPECT_FALSE(output.IsInput());

  output.Connect(&input);

  EXPECT_EQ(input, Enum::L1);
  output = Enum::L2;

  EXPECT_EQ(input, Enum::L2);
}
using String20 = ::Xsmp::String<20>;

TEST(SimpleFieldTest, StringType) {

  Xsmp::Publication::TypeRegistry typeRegistry;
  Field<String20> field{&typeRegistry, Smp::Uuids::Uuid_String8, "name"};

  EXPECT_TRUE(field.IsState());
  EXPECT_FALSE(field.IsOutput());
  EXPECT_FALSE(field.IsInput());

  field = "";

  EXPECT_STREQ(field->c_str(), "");
  field += "text";
  EXPECT_STREQ(field->c_str(), "text");

  const ::Smp::AnySimple v = {::Smp::PrimitiveTypeKind::PTK_String8, "text2"};
  field.SetValue(v);
  EXPECT_STREQ(field->c_str(), "text2");
  EXPECT_STREQ(static_cast<::Smp::String8>(field.GetValue()), "text2");

  Field<String20>::input input{&typeRegistry, Smp::Uuids::Uuid_String8,
                               "input"};
  EXPECT_TRUE(input.IsState());
  EXPECT_FALSE(input.IsOutput());
  EXPECT_TRUE(input.IsInput());

  Field<String20>::output output{&typeRegistry, Smp::Uuids::Uuid_String8,
                                 "output"};
  EXPECT_TRUE(output.IsState());
  EXPECT_TRUE(output.IsOutput());
  EXPECT_FALSE(output.IsInput());

  output.Connect(&input);

  EXPECT_EQ(input, "");
  EXPECT_EQ("", input);
  output = "test";

  EXPECT_EQ(input, "test");
  EXPECT_EQ(*input, "test");
  EXPECT_STREQ(input->c_str(), "test");
  EXPECT_NE(input, "test2");
}

TEST(SimpleFieldTest, Failure) {
  Xsmp::Publication::TypeRegistry typeRegistry;
  Field<bool>::failure field{&typeRegistry, Smp::Uuids::Uuid_Bool, "name"};

  EXPECT_TRUE(field.IsState());
  EXPECT_FALSE(field.IsOutput());
  EXPECT_FALSE(field.IsInput());

  EXPECT_EQ(field.IsFailed(), false);

  field.Fail();
  EXPECT_EQ(field.IsFailed(), true);
  field.Unfail();

  EXPECT_EQ(field.IsFailed(), false);
}

TEST(SimpleFieldTest, Forcible) {
  Xsmp::Publication::TypeRegistry typeRegistry;
  Field<bool>::forcible field{&typeRegistry, Smp::Uuids::Uuid_Bool, "name"};

  EXPECT_TRUE(field.IsState());
  EXPECT_FALSE(field.IsOutput());
  EXPECT_FALSE(field.IsInput());

  EXPECT_EQ(field.IsForced(), false);
  EXPECT_EQ(field, false);
  field.Freeze();
  EXPECT_EQ(field.IsForced(), true);
  EXPECT_EQ(field, false);
  EXPECT_EQ(field.IsForced(), true);
  field.Unforce();

  EXPECT_EQ(field.IsForced(), false);
  EXPECT_EQ(field, false);

  field.Force(true);

  EXPECT_EQ(field.IsForced(), true);
  EXPECT_EQ(field, true);
  field.Force(Smp::AnySimple{::Smp::PrimitiveTypeKind::PTK_Bool, false});
  EXPECT_EQ(field, false);

  field = true;
  EXPECT_EQ(field, false);
}

} // namespace Xsmp
