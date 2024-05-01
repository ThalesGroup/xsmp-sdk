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

#include <Smp/AccessKind.h>
#include <Smp/AnySimple.h>
#include <Smp/IField.h>
#include <Smp/FieldAlreadyConnected.h>
#include <Smp/IArrayField.h>
#include <Smp/IDataflowField.h>
#include <Smp/IOperation.h>
#include <Smp/IParameter.h>
#include <Smp/IProperty.h>
#include <Smp/IRequest.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimpleField.h>
#include <Smp/InvalidArrayIndex.h>
#include <Smp/InvalidArraySize.h>
#include <Smp/InvalidFieldName.h>
#include <Smp/InvalidFieldType.h>
#include <Smp/InvalidFieldValue.h>
#include <Smp/InvalidTarget.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IEnumerationType.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Publication/ParameterDirectionKind.h>
#include <Smp/Publication/TypeNotRegistered.h>
#include <Smp/Uuid.h>
#include <Smp/ViewKind.h>
#include <Xsmp/Array.h>
#include <Xsmp/Component.h>
#include <Xsmp/Publication/Publication.h>
#include <Xsmp/Publication/TypeRegistry.h>
#include <gtest/gtest.h>
#include <type_traits>

namespace Xsmp {
namespace Publication {
TEST(PublicationTest, init) {
  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};

  EXPECT_EQ(publication.GetTypeRegistry(), &registry);
  EXPECT_TRUE(publication.GetOperations());
  EXPECT_TRUE(publication.GetProperties());
  EXPECT_TRUE(publication.GetFields());
}
TEST(PublicationTest, PublishField) {

  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};

  auto EnumUuid = Smp::Uuid{0, 1, 2, 3, 4};
  auto *EnumType =
      registry.AddEnumerationType("Enum", "", EnumUuid, sizeof(Smp::Int32));
  EnumType->AddLiteral("L1", "", 0);
  EnumType->AddLiteral("L2", "", 1);
  Smp::Int32 Enum;
  publication.PublishField("stateEnumField", "Enum description", &Enum,
                           EnumUuid, Smp::ViewKind::VK_All, true, false, false);
  auto *stateEnumField =
      dynamic_cast<Smp::ISimpleField *>(publication.GetField("stateEnumField"));
  ASSERT_TRUE(stateEnumField);

  EXPECT_STREQ(stateEnumField->GetName(), "stateEnumField");
  EXPECT_STREQ(stateEnumField->GetDescription(), "Enum description");
  EXPECT_EQ(stateEnumField->GetParent(), &component);
  EXPECT_EQ(stateEnumField->GetType(), EnumType);
  EXPECT_EQ(stateEnumField->GetPrimitiveTypeKind(),
            Smp::PrimitiveTypeKind::PTK_Int32);
  EXPECT_EQ(stateEnumField->GetView(), Smp::ViewKind::VK_All);
  EXPECT_TRUE(stateEnumField->IsState());
  EXPECT_FALSE(stateEnumField->IsInput());
  EXPECT_FALSE(stateEnumField->IsOutput());

  stateEnumField->SetValue({Smp::PrimitiveTypeKind::PTK_Int32, 1});
  EXPECT_EQ(Enum, 1);
  EXPECT_THROW(
      stateEnumField->SetValue({Smp::PrimitiveTypeKind::PTK_Int32, 42}),
      Smp::InvalidFieldValue);
}

TEST(PublicationTest, PublishString8Field) {
  TypeRegistry registry;
  Component component{"component"};
  Publication publication{&component, &registry};
  Smp::String8 string8;
  EXPECT_THROW(publication.PublishField("string8", "string8 description",
                                        &string8, ::Smp::Uuids::Uuid_String8,
                                        Smp::ViewKind::VK_All, true, false,
                                        false),
               Smp::InvalidFieldType);
}
template <typename T>
void TestPublishSimpleField(Smp::PrimitiveTypeKind kind, Smp::ViewKind view,
                            T default_value, T check_Value) {
  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};

  T state = default_value;
  publication.PublishField("state", "description", &state, view, true, false,
                           false);
  auto *stateField =
      dynamic_cast<Smp::ISimpleField *>(publication.GetField("state"));
  ASSERT_TRUE(stateField);
  EXPECT_STREQ(stateField->GetName(), "state");
  EXPECT_STREQ(stateField->GetDescription(), "description");
  EXPECT_EQ(stateField->GetParent(), &component);
  EXPECT_EQ(stateField->GetView(), view);
  EXPECT_EQ(stateField->GetType(), registry.GetType(kind));
  EXPECT_EQ(stateField->GetPrimitiveTypeKind(), kind);
  EXPECT_TRUE(stateField->IsState());
  EXPECT_FALSE(stateField->IsInput());
  EXPECT_FALSE(stateField->IsOutput());
  stateField->SetValue({kind, check_Value});
  EXPECT_EQ(state, check_Value);
  state = default_value;
  Smp::AnySimple expected{kind, default_value};
  EXPECT_EQ(stateField->GetValue(), expected);

  publication.PublishField("transient", "description", &state, view, false,
                           false, false);
  auto *transientField =
      dynamic_cast<Smp::ISimpleField *>(publication.GetField("transient"));
  ASSERT_TRUE(transientField);
  EXPECT_FALSE(transientField->IsState());

  T input = default_value;
  publication.PublishField("input", "description", &input, view, false, true,
                           false);
  auto *inputField =
      dynamic_cast<Smp::ISimpleField *>(publication.GetField("input"));
  ASSERT_TRUE(inputField);
  EXPECT_TRUE(inputField->IsInput());

  T output = check_Value;
  publication.PublishField("output", "description", &output, view, false, false,
                           true);
  auto *outputField =
      dynamic_cast<Smp::ISimpleField *>(publication.GetField("output"));
  ASSERT_TRUE(outputField);
  EXPECT_TRUE(outputField->IsOutput());

  auto *dataflowField = dynamic_cast<Smp::IDataflowField *>(outputField);
  ASSERT_TRUE(dataflowField);

  dataflowField->Connect(inputField);
  EXPECT_EQ(input, default_value);
  dataflowField->Push();
  EXPECT_EQ(input, check_Value);

  EXPECT_THROW(dataflowField->Connect(inputField), Smp::FieldAlreadyConnected);

  EXPECT_THROW(dataflowField->Connect(stateField), Smp::InvalidTarget);
}

TEST(PublicationTest, PublishSimpleField) {

  TestPublishSimpleField<Smp::Bool>(Smp::PrimitiveTypeKind::PTK_Bool,
                                    Smp::ViewKind::VK_Debug, false, true);
  TestPublishSimpleField<Smp::Char8>(Smp::PrimitiveTypeKind::PTK_Char8,
                                     Smp::ViewKind::VK_None, '\0', 'a');

  TestPublishSimpleField<Smp::Float32>(Smp::PrimitiveTypeKind::PTK_Float32,
                                       Smp::ViewKind::VK_None, 0.f, 100.f);
  TestPublishSimpleField<Smp::Float64>(Smp::PrimitiveTypeKind::PTK_Float64,
                                       Smp::ViewKind::VK_None, 0.f, 100.f);

  TestPublishSimpleField<Smp::Int8>(Smp::PrimitiveTypeKind::PTK_Int8,
                                    Smp::ViewKind::VK_None, 0, 100);

  TestPublishSimpleField<Smp::Int16>(Smp::PrimitiveTypeKind::PTK_Int16,
                                     Smp::ViewKind::VK_None, 0, 100);

  TestPublishSimpleField<Smp::Int32>(Smp::PrimitiveTypeKind::PTK_Int32,
                                     Smp::ViewKind::VK_None, 0, 100);

  TestPublishSimpleField<Smp::Int64>(Smp::PrimitiveTypeKind::PTK_Int64,
                                     Smp::ViewKind::VK_None, 0, 100);

  TestPublishSimpleField<Smp::UInt8>(Smp::PrimitiveTypeKind::PTK_UInt8,
                                     Smp::ViewKind::VK_None, 0, 100);

  TestPublishSimpleField<Smp::UInt16>(Smp::PrimitiveTypeKind::PTK_UInt16,
                                      Smp::ViewKind::VK_None, 0, 100);

  TestPublishSimpleField<Smp::UInt32>(Smp::PrimitiveTypeKind::PTK_UInt32,
                                      Smp::ViewKind::VK_None, 0, 100);

  TestPublishSimpleField<Smp::UInt64>(Smp::PrimitiveTypeKind::PTK_UInt64,
                                      Smp::ViewKind::VK_None, 0, 100);

  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};
  EXPECT_THROW(publication.PublishField("test", "", nullptr, {},
                                        Smp::ViewKind::VK_None, true, false,
                                        false),
               Smp::Publication::TypeNotRegistered);

  EXPECT_THROW(publication.GetField("invalid"), Smp::InvalidFieldName);
}

template <typename T>
void TestPublishSimpleArray(Smp::PrimitiveTypeKind kind, Smp::ViewKind view,
                            T default_value, T check_Value) {
  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};

  Xsmp::Array<T, 2> state = {default_value, default_value};
  publication.PublishArray("state", "description", 2, &state, kind, view, true,
                           false, false);
  auto *stateField =
      dynamic_cast<Smp::ISimpleArrayField *>(publication.GetField("state"));
  ASSERT_TRUE(stateField);
  EXPECT_STREQ(stateField->GetName(), "state");
  EXPECT_STREQ(stateField->GetDescription(), "description");
  EXPECT_EQ(stateField->GetParent(), &component);
  EXPECT_EQ(stateField->GetView(), view);
  EXPECT_EQ(stateField->GetType(), nullptr);
  EXPECT_TRUE(stateField->IsState());
  EXPECT_FALSE(stateField->IsInput());
  EXPECT_FALSE(stateField->IsOutput());
  stateField->SetValue(0, {kind, check_Value});
  EXPECT_EQ(state[0], check_Value);
  EXPECT_EQ(state[1], default_value);
  stateField->SetValue(1, {kind, check_Value});
  EXPECT_EQ(state[0], check_Value);
  EXPECT_EQ(state[1], check_Value);

  state[0] = default_value;
  Smp::AnySimple expected{kind, default_value};
  EXPECT_EQ(stateField->GetValue(0), expected);

  state[1] = default_value;
  EXPECT_EQ(stateField->GetValue(1), expected);

  Smp::AnySimple values[2];
  stateField->GetValues(2, values);
  EXPECT_EQ(static_cast<T>(values[0]), default_value);
  EXPECT_EQ(static_cast<T>(values[1]), default_value);
  values[0] = {kind, check_Value};
  values[1] = {kind, check_Value};
  stateField->SetValues(2, values);
  EXPECT_EQ(state[0], check_Value);
  EXPECT_EQ(state[1], check_Value);

  EXPECT_THROW(stateField->GetValues(3, values), Smp::InvalidArraySize);
  EXPECT_THROW(stateField->SetValues(3, values), Smp::InvalidArraySize);
  EXPECT_THROW(stateField->GetValues(1, values), Smp::InvalidArraySize);
  EXPECT_THROW(stateField->SetValues(1, values), Smp::InvalidArraySize);

  EXPECT_THROW(stateField->GetValue(3), Smp::InvalidArrayIndex);

  EXPECT_THROW(stateField->SetValue(3, expected), Smp::InvalidArrayIndex);

  publication.PublishArray("transient", "description", 2, &state, kind, view,
                           false, false, false);
  auto *transientField =
      dynamic_cast<Smp::ISimpleArrayField *>(publication.GetField("transient"));
  ASSERT_TRUE(transientField);
  EXPECT_FALSE(transientField->IsState());

  Xsmp::Array<T, 2> input = {default_value, default_value};
  publication.PublishArray("input", "description", 2, &input, kind, view, true,
                           true, false);
  auto *inputField =
      dynamic_cast<Smp::ISimpleArrayField *>(publication.GetField("input"));
  ASSERT_TRUE(inputField);
  EXPECT_TRUE(inputField->IsInput());

  Xsmp::Array<T, 2> output = {default_value, check_Value};
  publication.PublishArray("output", "description", 2, &output, kind, view,
                           true, false, true);
  auto *outputField =
      dynamic_cast<Smp::ISimpleArrayField *>(publication.GetField("output"));
  ASSERT_TRUE(outputField);
  EXPECT_TRUE(outputField->IsOutput());

  auto *dataflowField = dynamic_cast<Smp::IDataflowField *>(outputField);

  ASSERT_TRUE(dataflowField);
  Xsmp::Array<T, 2> before = input;
  dataflowField->Connect(inputField);
  EXPECT_EQ(input, before);
  dataflowField->Push();
  EXPECT_EQ(input, output);

  EXPECT_THROW(dataflowField->Connect(inputField), Smp::FieldAlreadyConnected);

  EXPECT_THROW(dataflowField->Connect(stateField), Smp::InvalidTarget);
}

TEST(PublicationTest, PublishSimpleArray) {

  TestPublishSimpleArray<Smp::Bool>(Smp::PrimitiveTypeKind::PTK_Bool,
                                    Smp::ViewKind::VK_Debug, false, true);
  TestPublishSimpleArray<Smp::Char8>(Smp::PrimitiveTypeKind::PTK_Char8,
                                     Smp::ViewKind::VK_None, '\0', 'a');

  TestPublishSimpleArray<Smp::Float32>(Smp::PrimitiveTypeKind::PTK_Float32,
                                       Smp::ViewKind::VK_None, 0.f, 100.f);
  TestPublishSimpleArray<Smp::Float64>(Smp::PrimitiveTypeKind::PTK_Float64,
                                       Smp::ViewKind::VK_None, 0.f, 100.f);

  TestPublishSimpleArray<Smp::Int8>(Smp::PrimitiveTypeKind::PTK_Int8,
                                    Smp::ViewKind::VK_None, 0, 100);

  TestPublishSimpleArray<Smp::Int16>(Smp::PrimitiveTypeKind::PTK_Int16,
                                     Smp::ViewKind::VK_None, 0, 100);

  TestPublishSimpleArray<Smp::Int32>(Smp::PrimitiveTypeKind::PTK_Int32,
                                     Smp::ViewKind::VK_None, 0, 100);

  TestPublishSimpleArray<Smp::Int64>(Smp::PrimitiveTypeKind::PTK_Int64,
                                     Smp::ViewKind::VK_None, 0, 100);

  TestPublishSimpleArray<Smp::Duration>(Smp::PrimitiveTypeKind::PTK_Duration,
                                        Smp::ViewKind::VK_None, 0, 100);

  TestPublishSimpleArray<Smp::DateTime>(Smp::PrimitiveTypeKind::PTK_DateTime,
                                        Smp::ViewKind::VK_None, 0, 100);

  TestPublishSimpleArray<Smp::UInt8>(Smp::PrimitiveTypeKind::PTK_UInt8,
                                     Smp::ViewKind::VK_None, 0, 100);

  TestPublishSimpleArray<Smp::UInt16>(Smp::PrimitiveTypeKind::PTK_UInt16,
                                      Smp::ViewKind::VK_None, 0, 100);

  TestPublishSimpleArray<Smp::UInt32>(Smp::PrimitiveTypeKind::PTK_UInt32,
                                      Smp::ViewKind::VK_None, 0, 100);

  TestPublishSimpleArray<Smp::UInt64>(Smp::PrimitiveTypeKind::PTK_UInt64,
                                      Smp::ViewKind::VK_None, 0, 100);

  EXPECT_THROW(
      TestPublishSimpleArray<Smp::UInt64>(Smp::PrimitiveTypeKind::PTK_None,
                                          Smp::ViewKind::VK_None, 0, 100),
      Smp::InvalidFieldType);

  EXPECT_THROW(
      TestPublishSimpleArray<Smp::String8>(Smp::PrimitiveTypeKind::PTK_String8,
                                           Smp::ViewKind::VK_None, "", "test"),
      Smp::InvalidFieldType);
}

TEST(PublicationTest, PublishArray) {

  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};

  auto *publishedField =
      publication.PublishArray("name", "desc", Smp::ViewKind::VK_None, true);

  ASSERT_TRUE(publishedField);

  auto *arrayField =
      dynamic_cast<Smp::IArrayField *>(publication.GetField("name"));
  ASSERT_TRUE(arrayField);

  EXPECT_STREQ(arrayField->GetName(), "name");
  EXPECT_STREQ(arrayField->GetDescription(), "desc");

  EXPECT_EQ(arrayField->GetParent(), &component);
  EXPECT_EQ(arrayField->GetSize(), 0);
  EXPECT_THROW(arrayField->GetItem(0), Smp::InvalidArrayIndex);

  bool item;
  publishedField->PublishField("item", "item desc", &item,
                               Smp::ViewKind::VK_Debug, true, false, false);

  EXPECT_EQ(arrayField->GetSize(), 1);
  auto *itemField = arrayField->GetItem(0);

  ASSERT_TRUE(itemField);
  EXPECT_STREQ(itemField->GetName(), "item");
  EXPECT_STREQ(itemField->GetDescription(), "item desc");
  EXPECT_EQ(itemField->GetParent(), &component);
}

TEST(PublicationTest, PublishStructure) {

  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};

  auto *structureField =
      publication.PublishStructure("name", "", Smp::ViewKind::VK_None, true);

  ASSERT_TRUE(structureField);
  ASSERT_TRUE(structureField->GetFields());
  EXPECT_EQ(structureField->GetFields()->size(), 0);

  bool item;
  structureField->PublishField("item", "item desc", &item,
                               Smp::ViewKind::VK_Debug, true, false, false);

  EXPECT_EQ(structureField->GetFields()->size(), 1);
  auto *itemField = structureField->GetField("item");

  ASSERT_TRUE(itemField);
  EXPECT_STREQ(itemField->GetName(), "item");
  EXPECT_STREQ(itemField->GetDescription(), "item desc");
  EXPECT_EQ(itemField->GetParent(),
            dynamic_cast<Smp::IObject *>(structureField));
}

TEST(PublicationTest, PublishOperation) {

  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};

  auto *operation =
      publication.PublishOperation("operation", "", Smp::ViewKind::VK_None);

  auto *op = publication.GetOperations()->at("operation");

  ASSERT_TRUE(op);
  EXPECT_STREQ(op->GetName(), "operation");
  EXPECT_STREQ(op->GetDescription(), "");
  EXPECT_EQ(op->GetView(), Smp::ViewKind::VK_None);
  EXPECT_EQ(op->GetReturnParameter(), nullptr);
  EXPECT_EQ(op->GetParameters()->size(), 0);

  EXPECT_EQ(operation, publication.PublishOperation("operation", "desc",
                                                    Smp::ViewKind::VK_Debug));

  EXPECT_STREQ(op->GetDescription(), "desc");
  EXPECT_EQ(op->GetView(), Smp::ViewKind::VK_Debug);

  operation->PublishParameter("param1", "desc", Smp::Uuids::Uuid_Bool,
                              Smp::Publication::ParameterDirectionKind::PDK_In);
  EXPECT_EQ(op->GetParameters()->size(), 1);
  auto *param1 = op->GetParameter("param1");
  ASSERT_TRUE(param1);
  EXPECT_STREQ(param1->GetName(), "param1");
  EXPECT_STREQ(param1->GetDescription(), "desc");
  EXPECT_EQ(param1->GetDirection(),
            Smp::Publication::ParameterDirectionKind::PDK_In);
  EXPECT_EQ(param1->GetType(), registry.GetType(Smp::Uuids::Uuid_Bool));

  EXPECT_FALSE(op->GetParameter("invalid"));

  operation->PublishParameter(
      "returnValue", "bool", Smp::Uuids::Uuid_Bool,
      Smp::Publication::ParameterDirectionKind::PDK_Return);
  auto *returnParameter = op->GetReturnParameter();
  ASSERT_TRUE(returnParameter);
  EXPECT_TRUE(op->GetParameter("returnValue"));
  EXPECT_STREQ(returnParameter->GetName(), "returnValue");
  EXPECT_STREQ(returnParameter->GetDescription(), "bool");
  EXPECT_EQ(returnParameter->GetDirection(),
            Smp::Publication::ParameterDirectionKind::PDK_Return);

  EXPECT_THROW(operation->PublishParameter(
                   "returnValue2", "", Smp::Uuids::Uuid_Bool,
                   Smp::Publication::ParameterDirectionKind::PDK_Return),
               Smp::Exception);

  operation->PublishParameter(
      "param2", "desc", Smp::Uuids::Uuid_Float32,
      Smp::Publication::ParameterDirectionKind::PDK_InOut);
  EXPECT_EQ(op->GetParameters()->size(), 2);
  auto *param2 = op->GetParameter("param2");
  ASSERT_TRUE(param2);
  EXPECT_STREQ(param2->GetName(), "param2");
  EXPECT_STREQ(param2->GetDescription(), "desc");
  EXPECT_EQ(param2->GetDirection(),
            Smp::Publication::ParameterDirectionKind::PDK_InOut);
  EXPECT_EQ(param2->GetType(), registry.GetType(Smp::Uuids::Uuid_Float32));

  operation->PublishParameter(
      "param3", "desc", Smp::Uuids::Uuid_Int64,
      Smp::Publication::ParameterDirectionKind::PDK_Out);
  EXPECT_EQ(op->GetParameters()->size(), 3);
  auto *param3 = op->GetParameter("param3");
  ASSERT_TRUE(param3);
  EXPECT_STREQ(param3->GetName(), "param3");
  EXPECT_STREQ(param3->GetDescription(), "desc");
  EXPECT_EQ(param3->GetDirection(),
            Smp::Publication::ParameterDirectionKind::PDK_Out);

  EXPECT_EQ(param3->GetType(), registry.GetType(Smp::Uuids::Uuid_Int64));

  EXPECT_THROW(
      operation->PublishParameter(
          "param4", "", {}, Smp::Publication::ParameterDirectionKind::PDK_Out),
      Smp::Publication::TypeNotRegistered);
}

TEST(PublicationTest, PublishProperty) {

  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};

  EXPECT_THROW(publication.PublishProperty("property", "", {},
                                           ::Smp::AccessKind::AK_ReadOnly,
                                           Smp::ViewKind::VK_None),
               Smp::Publication::TypeNotRegistered);

  publication.PublishProperty("property", "", Smp::Uuids::Uuid_Bool,
                              ::Smp::AccessKind::AK_ReadOnly,
                              Smp::ViewKind::VK_None);

  auto *property = publication.GetProperties()->at("property");

  ASSERT_TRUE(property);
  EXPECT_STREQ(property->GetName(), "property");
  EXPECT_STREQ(property->GetDescription(), "");
  EXPECT_EQ(property->GetView(), Smp::ViewKind::VK_None);
  EXPECT_EQ(property->GetAccess(), Smp::AccessKind::AK_ReadOnly);
  EXPECT_EQ(property->GetType(), registry.GetType(Smp::Uuids::Uuid_Bool));

  publication.PublishProperty("property", "desc", Smp::Uuids::Uuid_Int8,
                              ::Smp::AccessKind::AK_ReadWrite,
                              Smp::ViewKind::VK_Debug);

  EXPECT_STREQ(property->GetDescription(), "desc");
  EXPECT_EQ(property->GetView(), Smp::ViewKind::VK_Debug);
  EXPECT_EQ(property->GetAccess(), Smp::AccessKind::AK_ReadWrite);
  EXPECT_EQ(property->GetType(), registry.GetType(Smp::Uuids::Uuid_Int8));

  EXPECT_THROW(property->GetValue(), Smp::Exception);
  EXPECT_THROW(property->SetValue({}), Smp::Exception);
}

TEST(PublicationTest, Request) {

  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};

  publication.PublishOperation("operation", "", Smp::ViewKind::VK_None);

  ASSERT_FALSE(publication.CreateRequest("invalid"));
  auto *request = publication.CreateRequest("operation");
  ASSERT_TRUE(request);
  EXPECT_STREQ(request->GetOperationName(), "operation");
  EXPECT_EQ(request->GetParameterCount(), 0);

  publication.DeleteRequest(request);
}

} // namespace Publication
} // namespace Xsmp
