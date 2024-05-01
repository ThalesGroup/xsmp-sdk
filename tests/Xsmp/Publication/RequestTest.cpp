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

#include <Smp/ViewKind.h>
#include <Xsmp/Component.h>
#include <Xsmp/Publication/Publication.h>
#include <Xsmp/Publication/TypeRegistry.h>
#include <gtest/gtest.h>

namespace Xsmp {
namespace Publication {

TEST(RequestTest, VoidOperation) {

  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};

  publication.PublishOperation("operation", "", Smp::ViewKind::VK_None);

  auto *request = publication.CreateRequest("operation");

  ASSERT_TRUE(request);
  EXPECT_STREQ(request->GetOperationName(), "operation");
  EXPECT_EQ(request->GetParameterCount(), 0);
  EXPECT_EQ(request->GetParameterIndex(nullptr), -1);
  EXPECT_EQ(request->GetParameterIndex("invalid"), -1);

  EXPECT_THROW(request->GetReturnValue(), Smp::VoidOperation);
  EXPECT_THROW(request->SetReturnValue({}), Smp::VoidOperation);
  EXPECT_THROW(request->GetParameterValue(-1), Smp::InvalidParameterIndex);
  EXPECT_THROW(request->GetParameterValue(0), Smp::InvalidParameterIndex);
  EXPECT_THROW(request->SetParameterValue(-1, {}), Smp::InvalidParameterIndex);
  EXPECT_THROW(request->SetParameterValue(0, {}), Smp::InvalidParameterIndex);

  publication.DeleteRequest(request);
}

TEST(RequestTest, EnumOperation) {

  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};

  auto *op =
      publication.PublishOperation("operation", "", Smp::ViewKind::VK_None);
  op->PublishParameter("returnValue", "", Smp::Uuids::Uuid_ComponentStateKind,
                       Smp::Publication::ParameterDirectionKind::PDK_Return);

  auto *request = publication.CreateRequest("operation");

  ASSERT_TRUE(request);

  EXPECT_STREQ(request->GetOperationName(), "operation");
  EXPECT_EQ(request->GetParameterCount(), 0);
  EXPECT_EQ(request->GetReturnValue(), Smp::AnySimple{});
  Smp::AnySimple value = {
      Smp::PrimitiveTypeKind::PTK_Int32,
      static_cast<Smp::Int32>(Smp::ComponentStateKind::CSK_Created)};
  request->SetReturnValue(value);
  EXPECT_EQ(request->GetReturnValue(), value);

  EXPECT_THROW(request->SetReturnValue({}), Smp::InvalidAnyType);

  Smp::AnySimple invalidValue = {Smp::PrimitiveTypeKind::PTK_Int32, 42};
  EXPECT_THROW(request->SetReturnValue(invalidValue), Smp::InvalidReturnValue);
  publication.DeleteRequest(request);
}

TEST(RequestTest, StringOperation) {

  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};

  auto *op =
      publication.PublishOperation("operation", "", Smp::ViewKind::VK_None);
  Smp::Uuid typeUuid{0, 1, 2, 3, 4};
  registry.AddStringType("String", "", typeUuid, 4);
  op->PublishParameter("returnValue", "", typeUuid,
                       Smp::Publication::ParameterDirectionKind::PDK_Return);

  auto *request = publication.CreateRequest("operation");
  ASSERT_TRUE(request);

  EXPECT_STREQ(request->GetOperationName(), "operation");
  EXPECT_EQ(request->GetParameterCount(), 0);
  EXPECT_EQ(request->GetReturnValue(), Smp::AnySimple{});
  Smp::AnySimple value = {Smp::PrimitiveTypeKind::PTK_String8, "1234"};
  request->SetReturnValue(value);
  EXPECT_EQ(request->GetReturnValue(), value);

  EXPECT_THROW(request->SetReturnValue({}), Smp::InvalidAnyType);

  Smp::AnySimple invalidValue = {Smp::PrimitiveTypeKind::PTK_String8, "12345"};
  EXPECT_THROW(request->SetReturnValue(invalidValue), Smp::InvalidReturnValue);
  publication.DeleteRequest(request);
}

TEST(RequestTest, IntOperation) {

  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};

  auto *op =
      publication.PublishOperation("operation", "", Smp::ViewKind::VK_None);
  Smp::Uuid typeUuid{0, 1, 2, 3, 4};
  registry.AddIntegerType("Integer", "", typeUuid, 0, 10, "",
                          Smp::PrimitiveTypeKind::PTK_Int32);
  op->PublishParameter("returnValue", "", typeUuid,
                       Smp::Publication::ParameterDirectionKind::PDK_Return);

  auto *request = publication.CreateRequest("operation");
  ASSERT_TRUE(request);

  EXPECT_STREQ(request->GetOperationName(), "operation");
  EXPECT_EQ(request->GetParameterCount(), 0);
  EXPECT_EQ(request->GetReturnValue(), Smp::AnySimple{});
  Smp::AnySimple value = {Smp::PrimitiveTypeKind::PTK_Int32, 0};
  request->SetReturnValue(value);
  EXPECT_EQ(request->GetReturnValue(), value);

  EXPECT_THROW(request->SetReturnValue({}), Smp::InvalidAnyType);

  EXPECT_THROW(request->SetReturnValue({Smp::PrimitiveTypeKind::PTK_Int32, 11}),
               Smp::InvalidReturnValue);

  EXPECT_THROW(request->SetReturnValue({Smp::PrimitiveTypeKind::PTK_Int32, -1}),
               Smp::InvalidReturnValue);
  publication.DeleteRequest(request);
}

TEST(RequestTest, FloatOperation) {

  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};

  auto *op =
      publication.PublishOperation("operation", "", Smp::ViewKind::VK_None);
  Smp::Uuid typeUuid{0, 1, 2, 3, 4};
  registry.AddFloatType("Float", "", typeUuid, 0, 10, true, true, "",
                        Smp::PrimitiveTypeKind::PTK_Float64);
  op->PublishParameter("returnValue", "", typeUuid,
                       Smp::Publication::ParameterDirectionKind::PDK_Return);

  auto *request = publication.CreateRequest("operation");
  ASSERT_TRUE(request);

  EXPECT_STREQ(request->GetOperationName(), "operation");
  EXPECT_EQ(request->GetParameterCount(), 0);
  EXPECT_EQ(request->GetReturnValue(), Smp::AnySimple{});
  Smp::AnySimple value = {Smp::PrimitiveTypeKind::PTK_Float64, 0.};
  request->SetReturnValue(value);
  EXPECT_EQ(request->GetReturnValue(), value);

  EXPECT_THROW(request->SetReturnValue({}), Smp::InvalidAnyType);

  EXPECT_THROW(
      request->SetReturnValue({Smp::PrimitiveTypeKind::PTK_Float64, 11.}),
      Smp::InvalidReturnValue);

  EXPECT_THROW(
      request->SetReturnValue({Smp::PrimitiveTypeKind::PTK_Float64, -1.}),
      Smp::InvalidReturnValue);

  EXPECT_NO_THROW(
      request->SetReturnValue({Smp::PrimitiveTypeKind::PTK_Float64, 10.}));

  EXPECT_NO_THROW(
      request->SetReturnValue({Smp::PrimitiveTypeKind::PTK_Float64, 0.}));

  publication.DeleteRequest(request);
}

TEST(RequestTest, FloatExclusiveOperation) {

  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};

  auto *op =
      publication.PublishOperation("operation", "", Smp::ViewKind::VK_None);
  Smp::Uuid typeUuid{0, 1, 2, 3, 4};
  registry.AddFloatType("Float", "", typeUuid, 0, 10, false, false, "",
                        Smp::PrimitiveTypeKind::PTK_Float64);
  op->PublishParameter("returnValue", "", typeUuid,
                       Smp::Publication::ParameterDirectionKind::PDK_Return);

  auto *request = publication.CreateRequest("operation");
  ASSERT_TRUE(request);

  EXPECT_STREQ(request->GetOperationName(), "operation");
  EXPECT_EQ(request->GetParameterCount(), 0);
  EXPECT_EQ(request->GetReturnValue(), Smp::AnySimple{});
  Smp::AnySimple value = {Smp::PrimitiveTypeKind::PTK_Float64, 1.};
  request->SetReturnValue(value);
  EXPECT_EQ(request->GetReturnValue(), value);

  EXPECT_THROW(request->SetReturnValue({}), Smp::InvalidAnyType);

  EXPECT_THROW(
      request->SetReturnValue({Smp::PrimitiveTypeKind::PTK_Float64, 10.}),
      Smp::InvalidReturnValue);

  EXPECT_THROW(
      request->SetReturnValue({Smp::PrimitiveTypeKind::PTK_Float64, 0.}),
      Smp::InvalidReturnValue);

  EXPECT_NO_THROW(
      request->SetReturnValue({Smp::PrimitiveTypeKind::PTK_Float64, 9.9999}));

  EXPECT_NO_THROW(
      request->SetReturnValue({Smp::PrimitiveTypeKind::PTK_Float64, 0.00001}));

  publication.DeleteRequest(request);
}

TEST(RequestTest, OperationWithParameters) {

  TypeRegistry registry;

  Component component{"component"};

  Publication publication{&component, &registry};

  auto *op =
      publication.PublishOperation("operation", "", Smp::ViewKind::VK_None);
  Smp::Uuid typeUuid{0, 1, 2, 3, 4};
  registry.AddFloatType("Float", "", typeUuid, 0, 10, false, false, "",
                        Smp::PrimitiveTypeKind::PTK_Float64);
  op->PublishParameter("p1", "", typeUuid,
                       Smp::Publication::ParameterDirectionKind::PDK_In);

  auto *request = publication.CreateRequest("operation");
  ASSERT_TRUE(request);

  EXPECT_STREQ(request->GetOperationName(), "operation");
  EXPECT_EQ(request->GetParameterCount(), 1);
  EXPECT_EQ(request->GetParameterIndex("p1"), 0);

  auto p1 = Smp::AnySimple{Smp::PrimitiveTypeKind::PTK_Float64, 2.0};
  request->SetParameterValue(0, p1);
  EXPECT_EQ(request->GetParameterValue(0), p1);

  publication.DeleteRequest(request);
}

} // namespace Publication
} // namespace Xsmp
