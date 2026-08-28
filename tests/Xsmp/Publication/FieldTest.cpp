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

#include "../Storage.h"
#include <Smp/AnySimple.h>
#include <Smp/IArrayField.h>
#include <Smp/IField.h>
#include <Smp/IPersist.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimpleField.h>
#include <Smp/IStructureField.h>
#include <Smp/InvalidAnyType.h>
#include <Smp/InvalidArrayIndex.h>
#include <Smp/InvalidArraySize.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Uuid.h>
#include <Smp/ViewKind.h>
#include <Xsmp/Array.h>
#include <Xsmp/Component.h>
#include <Xsmp/Publication/Publication.h>
#include <Xsmp/Publication/TypeRegistry.h>
#include <gtest/gtest.h>

namespace Xsmp::Publication {
namespace {

constexpr ::Smp::Uuid uuidSimpleArray{0x01, 0, 0, 0, 0};
constexpr ::Smp::Uuid uuidStructure{0x02, 0, 0, 0, 0};

struct Point {
  ::Smp::Int32 x;
  ::Smp::Int32 y;
};

} // namespace

/// Exercise the per primitive type branches of the simple array accessors.
template <typename T>
void testSimpleArray(::Smp::PrimitiveTypeKind kind, T first, T second) {

  TypeRegistry registry;
  Component component{"component"};
  Publication publication{&component, &registry};

  registry.AddArrayType("Array", "", uuidSimpleArray,
                        registry.GetType(kind)->GetUuid(), sizeof(T), 3, true);

  Xsmp::Array<T, 3> values = {first, first, first};
  publication.PublishField("array", "", &values, uuidSimpleArray);

  auto *field =
      dynamic_cast<::Smp::ISimpleArrayField *>(publication.GetField("array"));
  ASSERT_TRUE(field);
  EXPECT_EQ(field->GetSize(), 3U);

  // element access
  EXPECT_EQ(field->GetValue(0), (::Smp::AnySimple{kind, first}));
  field->SetValue(2, {kind, second});
  EXPECT_EQ(values[2], second);

  EXPECT_THROW(static_cast<void>(field->GetValue(3)), ::Smp::InvalidArrayIndex);
  EXPECT_THROW(field->SetValue(3, {kind, first}), ::Smp::InvalidArrayIndex);

  // bulk access
  ::Smp::AnySimple read[3];
  field->GetValues(3, read);
  EXPECT_EQ(static_cast<T>(read[0]), first);
  EXPECT_EQ(static_cast<T>(read[2]), second);

  ::Smp::AnySimple written[3] = {
      {kind, second}, {kind, second}, {kind, second}};
  field->SetValues(3, written);
  EXPECT_EQ(values[0], second);

  EXPECT_THROW(field->GetValues(2, read), ::Smp::InvalidArraySize);
  EXPECT_THROW(field->SetValues(2, written), ::Smp::InvalidArraySize);

  // store and restore the state of the field
  Storage storage;
  auto *persist = dynamic_cast<::Smp::IPersist *>(field);
  ASSERT_TRUE(persist);
  persist->Store(&storage);
  values[0] = first;
  persist->Restore(&storage);
  EXPECT_EQ(values[0], second);

  // an array published without a registered type behaves the same way
  Xsmp::Array<T, 2> anonymous = {first, first};
  publication.PublishArray("anonymous", "", 2, &anonymous, kind);
  auto *anonymousField = dynamic_cast<::Smp::ISimpleArrayField *>(
      publication.GetField("anonymous"));
  ASSERT_TRUE(anonymousField);
  anonymousField->SetValue(1, {kind, second});
  EXPECT_EQ(anonymous[1], second);

  auto *anonymousPersist = dynamic_cast<::Smp::IPersist *>(anonymousField);
  ASSERT_TRUE(anonymousPersist);
  anonymousPersist->Store(&storage);
  anonymous[1] = first;
  anonymousPersist->Restore(&storage);
  EXPECT_EQ(anonymous[1], second);
}

/// Exercise the per primitive type branches of the simple field accessors.
template <typename T>
void testSimpleField(::Smp::PrimitiveTypeKind kind, T first, T second) {

  TypeRegistry registry;
  Component component{"component"};
  Publication publication{&component, &registry};

  T value = first;
  publication.PublishField("field", "", &value,
                           registry.GetType(kind)->GetUuid());

  auto *field =
      dynamic_cast<::Smp::ISimpleField *>(publication.GetField("field"));
  ASSERT_TRUE(field);
  EXPECT_EQ(field->GetPrimitiveTypeKind(), kind);
  EXPECT_EQ(field->GetValue(), (::Smp::AnySimple{kind, first}));

  field->SetValue({kind, second});
  EXPECT_EQ(value, second);

  // store and restore the state of the field
  Storage storage;
  auto *persist = dynamic_cast<::Smp::IPersist *>(field);
  ASSERT_TRUE(persist);
  persist->Store(&storage);
  value = first;
  persist->Restore(&storage);
  EXPECT_EQ(value, second);
}

TEST(PublishedField, SimpleField) {

  testSimpleField<::Smp::Bool>(::Smp::PrimitiveTypeKind::PTK_Bool, false, true);
  testSimpleField<::Smp::Char8>(::Smp::PrimitiveTypeKind::PTK_Char8, 'a', 'b');
  testSimpleField<::Smp::Int8>(::Smp::PrimitiveTypeKind::PTK_Int8, 1, 2);
  testSimpleField<::Smp::Int16>(::Smp::PrimitiveTypeKind::PTK_Int16, 1, 2);
  testSimpleField<::Smp::Int32>(::Smp::PrimitiveTypeKind::PTK_Int32, 1, 2);
  testSimpleField<::Smp::Int64>(::Smp::PrimitiveTypeKind::PTK_Int64, 1, 2);
  testSimpleField<::Smp::UInt8>(::Smp::PrimitiveTypeKind::PTK_UInt8, 1, 2);
  testSimpleField<::Smp::UInt16>(::Smp::PrimitiveTypeKind::PTK_UInt16, 1, 2);
  testSimpleField<::Smp::UInt32>(::Smp::PrimitiveTypeKind::PTK_UInt32, 1, 2);
  testSimpleField<::Smp::UInt64>(::Smp::PrimitiveTypeKind::PTK_UInt64, 1, 2);
  testSimpleField<::Smp::Float32>(::Smp::PrimitiveTypeKind::PTK_Float32, 1.F,
                                  2.F);
  testSimpleField<::Smp::Float64>(::Smp::PrimitiveTypeKind::PTK_Float64, 1.,
                                  2.);
  testSimpleField<::Smp::Duration>(::Smp::PrimitiveTypeKind::PTK_Duration, 1,
                                   2);
  testSimpleField<::Smp::DateTime>(::Smp::PrimitiveTypeKind::PTK_DateTime, 1,
                                   2);
}

TEST(PublishedField, SimpleArrayField) {

  testSimpleArray<::Smp::Bool>(::Smp::PrimitiveTypeKind::PTK_Bool, false, true);
  testSimpleArray<::Smp::Char8>(::Smp::PrimitiveTypeKind::PTK_Char8, 'a', 'b');
  testSimpleArray<::Smp::Int8>(::Smp::PrimitiveTypeKind::PTK_Int8, 1, 2);
  testSimpleArray<::Smp::Int16>(::Smp::PrimitiveTypeKind::PTK_Int16, 1, 2);
  testSimpleArray<::Smp::Int32>(::Smp::PrimitiveTypeKind::PTK_Int32, 1, 2);
  testSimpleArray<::Smp::Int64>(::Smp::PrimitiveTypeKind::PTK_Int64, 1, 2);
  testSimpleArray<::Smp::UInt8>(::Smp::PrimitiveTypeKind::PTK_UInt8, 1, 2);
  testSimpleArray<::Smp::UInt16>(::Smp::PrimitiveTypeKind::PTK_UInt16, 1, 2);
  testSimpleArray<::Smp::UInt32>(::Smp::PrimitiveTypeKind::PTK_UInt32, 1, 2);
  testSimpleArray<::Smp::UInt64>(::Smp::PrimitiveTypeKind::PTK_UInt64, 1, 2);
  testSimpleArray<::Smp::Float32>(::Smp::PrimitiveTypeKind::PTK_Float32, 1.F,
                                  2.F);
  testSimpleArray<::Smp::Float64>(::Smp::PrimitiveTypeKind::PTK_Float64, 1.,
                                  2.);
  testSimpleArray<::Smp::Duration>(::Smp::PrimitiveTypeKind::PTK_Duration, 1,
                                   2);
  testSimpleArray<::Smp::DateTime>(::Smp::PrimitiveTypeKind::PTK_DateTime, 1,
                                   2);
}

TEST(PublishedField, SimpleArrayFieldInvalidValue) {

  TypeRegistry registry;
  Component component{"component"};
  Publication publication{&component, &registry};

  registry.AddArrayType("Int32Array", "", uuidSimpleArray,
                        ::Smp::Uuids::Uuid_Int32, sizeof(::Smp::Int32), 3,
                        true);
  Xsmp::Array<::Smp::Int32, 3> values = {1, 2, 3};
  publication.PublishField("array", "", &values, uuidSimpleArray);
  auto *field =
      dynamic_cast<::Smp::ISimpleArrayField *>(publication.GetField("array"));
  ASSERT_TRUE(field);

  // the value is not convertible to the item type
  EXPECT_THROW(field->SetValue(0, {::Smp::PrimitiveTypeKind::PTK_Bool, true}),
               ::Smp::InvalidAnyType);
}

TEST(PublishedField, StoreRestore) {

  TypeRegistry registry;
  Component component{"component"};
  Publication publication{&component, &registry};

  registry.AddArrayType("Int32Array", "", uuidSimpleArray,
                        ::Smp::Uuids::Uuid_Int32, sizeof(::Smp::Int32), 3,
                        true);
  auto *structure = registry.AddStructureType("Point", "", uuidStructure);
  structure->AddField("x", "", ::Smp::Uuids::Uuid_Int32, offsetof(Point, x));
  structure->AddField("y", "", ::Smp::Uuids::Uuid_Int32, offsetof(Point, y));

  ::Smp::Int32 simple = 1;
  Xsmp::Array<::Smp::Int32, 3> array = {1, 2, 3};
  Xsmp::Array<::Smp::Int32, 2> anonymous = {1, 2};
  Point point{1, 2};

  publication.PublishField("simple", "", &simple);
  publication.PublishField("array", "", &array, uuidSimpleArray);
  publication.PublishArray("anonymous", "", 2, &anonymous,
                           ::Smp::PrimitiveTypeKind::PTK_Int32);
  publication.PublishField("point", "", &point, uuidStructure);
  // a non state field is not part of the state vector
  ::Smp::Int32 transientValue = 1;
  publication.PublishField("transient", "", &transientValue,
                           ::Smp::ViewKind::VK_All, false);

  Storage storage;
  for (auto *field : *publication.GetFields()) {
    if (auto *persist = dynamic_cast<::Smp::IPersist *>(field)) {
      persist->Store(&storage);
    }
  }

  simple = 100;
  array[1] = 100;
  anonymous[0] = 100;
  point.y = 100;
  transientValue = 100;

  for (auto *field : *publication.GetFields()) {
    if (auto *persist = dynamic_cast<::Smp::IPersist *>(field)) {
      persist->Restore(&storage);
    }
  }

  EXPECT_EQ(simple, 1);
  EXPECT_EQ(array[1], 2);
  EXPECT_EQ(anonymous[0], 1);
  EXPECT_EQ(point.y, 2);
  // the transient field keeps its current value
  EXPECT_EQ(transientValue, 100);
}

} // namespace Xsmp::Publication
