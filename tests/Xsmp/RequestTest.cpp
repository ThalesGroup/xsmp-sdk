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

#include <Smp/Exception.h>
#include <Smp/IRequest.h>
#include <Smp/InvalidParameterValue.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Publication/ParameterDirectionKind.h>
#include <Smp/Uuid.h>
#include <Smp/ViewKind.h>
#include <Xsmp/Array.h>
#include <Xsmp/Component.h>
#include <Xsmp/Publication/Publication.h>
#include <Xsmp/Request.h>
#include <Xsmp/Simulator.h>
#include <cstddef>
#include <gtest/gtest.h>

namespace Xsmp {
namespace {

/// A component publishing a single operation taking an Int32 parameter and
/// returning an Int32.
class TestComponent final : public Component {
public:
  using Component::Component;
  void Publish(::Smp::IPublication *receiver) override {
    Component::Publish(receiver);
    auto *operation =
        receiver->PublishOperation("operation", "", ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "input", "", ::Smp::Uuids::Uuid_Int32,
        ::Smp::Publication::ParameterDirectionKind::PDK_In);
    operation->PublishParameter(
        "output", "", ::Smp::Uuids::Uuid_Int32,
        ::Smp::Publication::ParameterDirectionKind::PDK_Out);
    operation->PublishParameter(
        "returnValue", "", ::Smp::Uuids::Uuid_Int32,
        ::Smp::Publication::ParameterDirectionKind::PDK_Return);
  }

  // the uuid based accessors build a field from the type registry of the
  // simulator, as generated code does from inside the component
  ::Smp::Int32 readInput(::Smp::IRequest *request) {
    return Request::get<::Smp::Int32>(this, request, "input",
                                      ::Smp::Uuids::Uuid_Int32);
  }
  ::Smp::Int32 readMissing(::Smp::IRequest *request) {
    return Request::get<::Smp::Int32>(this, request, "unknown",
                                      ::Smp::Uuids::Uuid_Int32, 7);
  }
  void writeInput(::Smp::IRequest *request, ::Smp::Int32 value) {
    Request::set<::Smp::Int32>(this, request, "input", ::Smp::Uuids::Uuid_Int32,
                               value);
  }
};

/// A structure usable as a parameter type, following the shape the code
/// generator produces.
struct Pair {
  ::Smp::Int32 x;
  ::Smp::Int32 y;

  template <typename _BASE> struct _Field : public _BASE {
    _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
           ::Smp::Uuid typeUuid, ::Smp::String8 name,
           ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
           ::Smp::ViewKind view = ::Smp::ViewKind::VK_None,
           const Pair &value = {})
        : _BASE(typeRegistry, typeUuid, name, description, parent, view),
          x{typeRegistry, ::Smp::Uuids::Uuid_Int32, "x", "", this, view,
            value.x},
          y{typeRegistry, ::Smp::Uuids::Uuid_Int32, "y", "", this, view,
            value.y} {}
    _Field &operator=(const Pair &other) {
      this->x = other.x;
      this->y = other.y;
      return *this;
    }
    operator Pair() const noexcept { return {x, y}; }
    typename _BASE::template Field<::Smp::Int32> x;
    typename _BASE::template Field<::Smp::Int32> y;
  };
};

constexpr ::Smp::Uuid uuidPair{0x30, 0, 0, 0, 0};
constexpr ::Smp::Uuid uuidArray{0x31, 0, 0, 0, 0};
using Values = ::Xsmp::Array<::Smp::Int32, 2>;

void registerTypes(::Smp::Publication::ITypeRegistry *registry) {
  auto *pair = registry->AddStructureType("Pair", "", uuidPair);
  pair->AddField("x", "", ::Smp::Uuids::Uuid_Int32, offsetof(Pair, x));
  pair->AddField("y", "", ::Smp::Uuids::Uuid_Int32, offsetof(Pair, y));
  registry->AddArrayType("Values", "", uuidArray, ::Smp::Uuids::Uuid_Int32,
                         sizeof(::Smp::Int32), 2);
}

/// A component publishing an operation with a structure and an array
/// parameter.
class StructuredComponent final : public Component {
public:
  using Component::Component;
  void Publish(::Smp::IPublication *receiver) override {
    Component::Publish(receiver);
    auto *operation =
        receiver->PublishOperation("operation", "", ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "point", "", uuidPair,
        ::Smp::Publication::ParameterDirectionKind::PDK_InOut);
    operation->PublishParameter(
        "values", "", uuidArray,
        ::Smp::Publication::ParameterDirectionKind::PDK_InOut);
  }

  Pair readPoint(::Smp::IRequest *request) {
    return Request::get<Pair>(this, request, "point", uuidPair);
  }
  void writePoint(::Smp::IRequest *request, const Pair &value) {
    Request::set<Pair>(this, request, "point", uuidPair, value);
  }
  Values readValues(::Smp::IRequest *request) {
    return Request::get<Values>(this, request, "values", uuidArray);
  }
  void writeValues(::Smp::IRequest *request, const Values &value) {
    Request::set<Values>(this, request, "values", uuidArray, value);
  }
  Pair readMissing(::Smp::IRequest *request) {
    return Request::get<Pair>(this, request, "unknown", uuidPair, Pair{8, 9});
  }
};

} // namespace

TEST(Request, StructureAndArrayParameters) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  registerTypes(sim.GetTypeRegistry());

  StructuredComponent component{"component", "", &sim};
  Publication::Publication publication{&component, sim.GetTypeRegistry()};
  component.Publish(&publication);
  component.Configure(nullptr, nullptr);
  component.Connect(&sim);

  auto *request = component.CreateRequest("operation");
  ASSERT_TRUE(request);

  // a structure parameter is written and read member by member
  component.writePoint(request, Pair{1, 2});
  const auto point = component.readPoint(request);
  EXPECT_EQ(point.x, 1);
  EXPECT_EQ(point.y, 2);

  // an array parameter, item by item
  component.writeValues(request, Values{3, 4});
  const auto values = component.readValues(request);
  EXPECT_EQ(values[0], 3);
  EXPECT_EQ(values[1], 4);

  // a missing parameter falls back on the default value
  const auto missing = component.readMissing(request);
  EXPECT_EQ(missing.x, 8);
  EXPECT_EQ(missing.y, 9);

  component.DeleteRequest(request);
  sim.Exit();
}

TEST(Request, GetAndSetValue) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();

  TestComponent component{"component", "", &sim};
  Publication::Publication publication{&component, sim.GetTypeRegistry()};
  component.Publish(&publication);
  component.Configure(nullptr, nullptr);
  component.Connect(&sim);

  auto *request = component.CreateRequest("operation");
  ASSERT_TRUE(request);

  Request::set(&component, request, "input",
               ::Smp::PrimitiveTypeKind::PTK_Int32, 42);
  EXPECT_EQ((Request::get<::Smp::Int32>(&component, request, "input",
                                        ::Smp::PrimitiveTypeKind::PTK_Int32)),
            42);

  // an unknown parameter is an error on both sides
  EXPECT_THROW(Request::set(&component, request, "unknown",
                            ::Smp::PrimitiveTypeKind::PTK_Int32, 1),
               ::Smp::Exception);
  EXPECT_THROW(
      static_cast<void>(Request::get<::Smp::Int32>(
          &component, request, "unknown", ::Smp::PrimitiveTypeKind::PTK_Int32)),
      ::Smp::Exception);

  // the parameter exists but is not of the expected type
  EXPECT_THROW(
      static_cast<void>(Request::get<::Smp::Bool>(
          &component, request, "input", ::Smp::PrimitiveTypeKind::PTK_Bool)),
      ::Smp::InvalidParameterValue);

  component.DeleteRequest(request);
  sim.Exit();
}

TEST(Request, DefaultValue) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();

  TestComponent component{"component", "", &sim};
  Publication::Publication publication{&component, sim.GetTypeRegistry()};
  component.Publish(&publication);
  component.Configure(nullptr, nullptr);
  component.Connect(&sim);

  auto *request = component.CreateRequest("operation");
  ASSERT_TRUE(request);

  // a missing parameter falls back on the default value
  EXPECT_EQ(
      (Request::get<::Smp::Int32>(&component, request, "unknown",
                                  ::Smp::PrimitiveTypeKind::PTK_Int32, 7)),
      7);

  // an existing parameter takes precedence over the default value
  Request::set(&component, request, "input",
               ::Smp::PrimitiveTypeKind::PTK_Int32, 42);
  EXPECT_EQ(
      (Request::get<::Smp::Int32>(&component, request, "input",
                                  ::Smp::PrimitiveTypeKind::PTK_Int32, 7)),
      42);

  component.DeleteRequest(request);
  sim.Exit();
}

TEST(Request, FieldParameter) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();

  TestComponent component{"component", "", &sim};
  Publication::Publication publication{&component, sim.GetTypeRegistry()};
  component.Publish(&publication);
  component.Configure(nullptr, nullptr);
  component.Connect(&sim);

  auto *request = component.CreateRequest("operation");
  ASSERT_TRUE(request);

  component.writeInput(request, 42);
  EXPECT_EQ(component.readInput(request), 42);

  // a missing parameter falls back on the default value
  EXPECT_EQ(component.readMissing(request), 7);

  component.DeleteRequest(request);
  sim.Exit();
}

TEST(Request, ReturnValue) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();

  TestComponent component{"component", "", &sim};
  Publication::Publication publication{&component, sim.GetTypeRegistry()};
  component.Publish(&publication);
  component.Configure(nullptr, nullptr);
  component.Connect(&sim);

  auto *request = component.CreateRequest("operation");
  ASSERT_TRUE(request);

  Request::setReturnValue(request, ::Smp::PrimitiveTypeKind::PTK_Int32, 42);
  EXPECT_EQ(request->GetReturnValue(),
            (::Smp::AnySimple{::Smp::PrimitiveTypeKind::PTK_Int32, 42}));

  component.DeleteRequest(request);
  sim.Exit();
}

} // namespace Xsmp
