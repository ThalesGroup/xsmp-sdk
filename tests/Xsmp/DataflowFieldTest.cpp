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

// FieldAlreadyConnected.h is not self contained, it uses ::Smp::IField
#include <Smp/IField.h>
// clang-format off
#include <Smp/FieldAlreadyConnected.h>
// clang-format on
#include <Smp/IPublication.h>
#include <Smp/InvalidTarget.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Uuid.h>
#include <Xsmp/Array.h>
#include <Xsmp/Component.h>
#include <Xsmp/Field.h>
#include <Xsmp/Publication/Publication.h>
#include <Xsmp/Publication/TypeRegistry.h>
#include <Xsmp/Simulator.h>
#include <cstddef>
#include <gtest/gtest.h>

namespace Xsmp {

TEST(DataflowField, SimpleConnectAndDisconnect) {

  Xsmp::Publication::TypeRegistry registry;

  Field<::Smp::Int32>::output output{&registry, ::Smp::Uuids::Uuid_Int32,
                                     "output"};
  Field<::Smp::Int32>::input input{&registry, ::Smp::Uuids::Uuid_Int32,
                                   "input"};

  ASSERT_TRUE(output.GetInputFields());
  EXPECT_EQ(output.GetInputFields()->size(), 0U);

  output.Connect(&input);
  EXPECT_EQ(output.GetInputFields()->size(), 1U);
  // connecting the value pushes it immediately
  EXPECT_EQ(input, 0);

  // the same target cannot be connected twice
  EXPECT_THROW(output.Connect(&input), ::Smp::FieldAlreadyConnected);

  output = 42;
  EXPECT_EQ(input, 42);

  output.Disconnect(&input);
  EXPECT_EQ(output.GetInputFields()->size(), 0U);

  // the value is no longer propagated
  output = 7;
  EXPECT_EQ(input, 42);

  // disconnecting an unknown target is a no-op
  EXPECT_NO_THROW(output.Disconnect(&input));
}

TEST(DataflowField, SimpleArrayConnectAndDisconnect) {

  Xsmp::Publication::TypeRegistry registry;
  using Type = Array<::Smp::Int32, 2>::simple;
  registry.AddArrayType("Int32SimpleArray", "", ::Smp::Uuid{},
                        ::Smp::Uuids::Uuid_Int32, sizeof(::Smp::Int32), 2,
                        true);

  Field<Type>::output output{&registry, ::Smp::Uuid{}, "output"};
  Field<Type>::input input{&registry, ::Smp::Uuid{}, "input"};

  EXPECT_EQ(output.GetInputFields()->size(), 0U);
  output.Connect(&input);
  EXPECT_EQ(output.GetInputFields()->size(), 1U);
  EXPECT_THROW(output.Connect(&input), ::Smp::FieldAlreadyConnected);

  output[0] = 42;
  EXPECT_EQ(input[0], 42);

  output.Disconnect(&input);
  EXPECT_EQ(output.GetInputFields()->size(), 0U);
  output[0] = 7;
  EXPECT_EQ(input[0], 42);
}

TEST(DataflowField, ArrayConnectAndDisconnect) {

  Xsmp::Publication::TypeRegistry registry;
  using Type = Array<::Smp::Int32, 2>;
  registry.AddArrayType("Int32Array", "", ::Smp::Uuid{},
                        ::Smp::Uuids::Uuid_Int32, sizeof(::Smp::Int32), 2);

  Field<Type>::output output{&registry, ::Smp::Uuid{}, "output"};
  Field<Type>::input input{&registry, ::Smp::Uuid{}, "input"};

  output.Connect(&input);
  EXPECT_THROW(output.Connect(&input), ::Smp::FieldAlreadyConnected);

  output[0] = 42;
  EXPECT_EQ(input[0], 42);
  input[0] = 0;
  output.Push();
  EXPECT_EQ(input[0], 42);

  output.Disconnect(&input);
  output[0] = 7;
  EXPECT_EQ(input[0], 42);
}

namespace {
/// A component owning and publishing one input field, so that the field
/// belongs to the component whose links are removed.
class Sink final : public Component {
public:
  Sink(::Smp::String8 name, ::Smp::IComposite *parent,
       ::Smp::Publication::ITypeRegistry *registry)
      : Component(name, "", parent),
        first{registry, ::Smp::Uuids::Uuid_Int32, "first", "", this},
        second{registry, ::Smp::Uuids::Uuid_Int32, "second", "", this} {}
  Field<::Smp::Int32>::input first;
  Field<::Smp::Int32>::input second;
};

/// A component owning and publishing one output field.
class Source final : public Component {
public:
  Source(::Smp::String8 name, ::Smp::IComposite *parent,
         ::Smp::Publication::ITypeRegistry *registry)
      : Component(name, "", parent),
        output{registry, ::Smp::Uuids::Uuid_Int32, "output", "", this} {}
  void Publish(::Smp::IPublication *receiver) override {
    Component::Publish(receiver);
    receiver->PublishField(&output);
  }
  Field<::Smp::Int32>::output output;
};
} // namespace

TEST(DataflowField, RemoveLinksOfSeveralFields) {

  Simulator sim;
  Xsmp::Publication::TypeRegistry registry;

  Source source{"source", &sim, &registry};
  Publication::Publication publication{&source, &registry};
  source.Publish(&publication);

  // both inputs of one component, plus one of another, on the same output
  Sink sink{"sink", &sim, &registry};
  Sink other{"other", &sim, &registry};
  source.output.Connect(&sink.first);
  source.output.Connect(&sink.second);
  source.output.Connect(&other.first);
  EXPECT_EQ(source.output.GetInputFields()->size(), 3U);

  // the two fields of the same component are disconnected by a single call
  source.RemoveLinks(&sink);
  EXPECT_EQ(source.output.GetInputFields()->size(), 1U);

  source.RemoveLinks(&other);
  EXPECT_EQ(source.output.GetInputFields()->size(), 0U);
}

namespace {
/// A structure usable as a field type, following the shape the code generator
/// produces.
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
    typename _BASE::template Field<::Smp::Int32> x;
    typename _BASE::template Field<::Smp::Int32> y;
  };
};

/// A structure whose members are not of the same types as Pair.
struct Mixed {
  ::Smp::Int32 x;
  ::Smp::Bool flag;

  template <typename _BASE> struct _Field : public _BASE {
    _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
           ::Smp::Uuid typeUuid, ::Smp::String8 name,
           ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
           ::Smp::ViewKind view = ::Smp::ViewKind::VK_None,
           const Mixed &value = {})
        : _BASE(typeRegistry, typeUuid, name, description, parent, view),
          x{typeRegistry, ::Smp::Uuids::Uuid_Int32, "x", "", this, view,
            value.x},
          flag{typeRegistry, ::Smp::Uuids::Uuid_Bool, "flag", "", this, view,
               value.flag} {}
    typename _BASE::template Field<::Smp::Int32> x;
    typename _BASE::template Field<::Smp::Bool> flag;
  };
};

/// A structure holding a structure and an array, so that connecting it walks
/// into its members.
struct Nested {
  Pair pair;
  ::Xsmp::Array<::Smp::Int32, 2> values;

  template <typename _BASE> struct _Field : public _BASE {
    _Field(::Smp::Publication::ITypeRegistry *typeRegistry,
           ::Smp::Uuid typeUuid, ::Smp::String8 name,
           ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
           ::Smp::ViewKind view = ::Smp::ViewKind::VK_None,
           const Nested &value = {})
        : _BASE(typeRegistry, typeUuid, name, description, parent, view),
          pair{typeRegistry, ::Smp::Uuid{0x20, 0, 0, 0, 0},
               "pair",       "",
               this,         view,
               value.pair},
          values{typeRegistry, ::Smp::Uuid{0x22, 0, 0, 0, 0},
                 "values",     "",
                 this,         view,
                 value.values} {}
    typename _BASE::template Field<Pair> pair;
    typename _BASE::template Field<::Xsmp::Array<::Smp::Int32, 2>> values;
  };
};

constexpr ::Smp::Uuid uuidPair{0x20, 0, 0, 0, 0};
constexpr ::Smp::Uuid uuidValues{0x22, 0, 0, 0, 0};
constexpr ::Smp::Uuid uuidNested{0x23, 0, 0, 0, 0};

void registerPair(Xsmp::Publication::TypeRegistry &registry) {
  auto *type = registry.AddStructureType("Pair", "", uuidPair);
  type->AddField("x", "", ::Smp::Uuids::Uuid_Int32, offsetof(Pair, x));
  type->AddField("y", "", ::Smp::Uuids::Uuid_Int32, offsetof(Pair, y));
}
} // namespace

TEST(DataflowField, StructureConnectAndDisconnect) {

  Xsmp::Publication::TypeRegistry registry;
  registerPair(registry);

  Field<Pair>::output output{&registry, uuidPair, "output"};
  Field<Pair>::input input{&registry, uuidPair, "input"};

  ASSERT_TRUE(output.GetInputFields());
  output.Connect(&input);
  EXPECT_EQ(output.GetInputFields()->size(), 1U);
  EXPECT_THROW(output.Connect(&input), ::Smp::FieldAlreadyConnected);

  // a member is pushed when it is assigned
  output.x = 42;
  EXPECT_EQ(input.x, 42);

  // and the whole structure on demand
  input.y = 0;
  output.y = 7;
  EXPECT_EQ(input.y, 7);
  input.x = 0;
  output.Push();
  EXPECT_EQ(input.x, 42);

  output.Disconnect(&input);
  EXPECT_EQ(output.GetInputFields()->size(), 0U);
  output.x = 1;
  EXPECT_EQ(input.x, 42);
  EXPECT_NO_THROW(output.Disconnect(&input));
}

TEST(DataflowField, NestedConnectAndDisconnect) {

  Xsmp::Publication::TypeRegistry registry;
  registerPair(registry);
  registry.AddArrayType("Values", "", uuidValues, ::Smp::Uuids::Uuid_Int32,
                        sizeof(::Smp::Int32), 2);
  auto *nested = registry.AddStructureType("Nested", "", uuidNested);
  nested->AddField("pair", "", uuidPair, offsetof(Nested, pair));
  nested->AddField("values", "", uuidValues, offsetof(Nested, values));

  Field<Nested>::output output{&registry, uuidNested, "output"};
  Field<Nested>::input input{&registry, uuidNested, "input"};

  // connecting walks into the structure and the array members
  output.Connect(&input);
  EXPECT_THROW(output.Connect(&input), ::Smp::FieldAlreadyConnected);

  output.pair.x = 42;
  EXPECT_EQ(input.pair.x, 42);
  output.values[1] = 7;
  EXPECT_EQ(input.values[1], 7);

  input.pair.y = 0;
  input.values[0] = 0;
  output.pair.y = 5;
  output.values[0] = 6;
  output.Push();
  EXPECT_EQ(input.pair.y, 5);
  EXPECT_EQ(input.values[0], 6);

  output.Disconnect(&input);
  output.pair.x = 1;
  output.values[1] = 1;
  EXPECT_EQ(input.pair.x, 42);
  EXPECT_EQ(input.values[1], 7);
}

TEST(DataflowField, StructureIncompatibleTarget) {

  Xsmp::Publication::TypeRegistry registry;
  registerPair(registry);

  Field<Pair>::output output{&registry, uuidPair, "output"};
  Field<::Smp::Int32>::input simpleInput{&registry, ::Smp::Uuids::Uuid_Int32,
                                         "simpleInput"};
  EXPECT_THROW(output.Connect(&simpleInput), ::Smp::InvalidTarget);

  // a structure whose members are of other types is not equivalent
  auto *mixedType =
      registry.AddStructureType("Mixed", "", ::Smp::Uuid{0x21, 0, 0, 0, 0});
  mixedType->AddField("x", "", ::Smp::Uuids::Uuid_Int32, offsetof(Mixed, x));
  mixedType->AddField("flag", "", ::Smp::Uuids::Uuid_Bool,
                      offsetof(Mixed, flag));
  Field<Mixed>::input mixed{&registry, ::Smp::Uuid{0x21, 0, 0, 0, 0}, "mixed"};
  EXPECT_THROW(output.Connect(&mixed), ::Smp::InvalidTarget);
}

TEST(DataflowField, IncompatibleTarget) {

  Xsmp::Publication::TypeRegistry registry;

  Field<::Smp::Int32>::output output{&registry, ::Smp::Uuids::Uuid_Int32,
                                     "output"};
  Field<::Smp::Bool>::input boolInput{&registry, ::Smp::Uuids::Uuid_Bool,
                                      "boolInput"};

  // a simple field can only be connected to a simple field of the same type
  EXPECT_THROW(output.Connect(&boolInput), ::Smp::InvalidTarget);

  using Type = Array<::Smp::Int32, 2>;
  registry.AddArrayType("Int32Array", "", ::Smp::Uuid{},
                        ::Smp::Uuids::Uuid_Int32, sizeof(::Smp::Int32), 2);
  Field<Type>::input arrayInput{&registry, ::Smp::Uuid{}, "arrayInput"};
  EXPECT_THROW(output.Connect(&arrayInput), ::Smp::InvalidTarget);

  Field<Type>::output arrayOutput{&registry, ::Smp::Uuid{}, "arrayOutput"};
  Field<::Smp::Int32>::input simpleInput{&registry, ::Smp::Uuids::Uuid_Int32,
                                         "simpleInput"};
  EXPECT_THROW(arrayOutput.Connect(&simpleInput), ::Smp::InvalidTarget);
}

} // namespace Xsmp
