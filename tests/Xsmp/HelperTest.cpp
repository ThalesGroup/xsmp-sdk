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

#include <Smp/IPublication.h>
#include <Smp/InvalidObjectName.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Services/IResolver.h>
#include <Smp/Uuid.h>
#include <Xsmp/Array.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Model.h>
#include <Xsmp/Simulator.h>
#include <cstddef>
#include <gtest/gtest.h>

namespace Xsmp {

TEST(Helper, checkName) {

  EXPECT_THROW(::Xsmp::Helper::checkName(nullptr, nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("0aaa", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("_aaa", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("abczABCZ0129_[", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("[abc]", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("[0]a", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("AZé", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("[]", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("[4][3]", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("[432", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("[", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("[43][", nullptr),
               ::Smp::InvalidObjectName);

  EXPECT_NO_THROW(::Xsmp::Helper::checkName("aZZZ___4541", nullptr));
  EXPECT_NO_THROW(::Xsmp::Helper::checkName("A", nullptr));
  EXPECT_NO_THROW(::Xsmp::Helper::checkName("[4]", nullptr));
  EXPECT_NO_THROW(::Xsmp::Helper::checkName("[43]", nullptr));
  EXPECT_NO_THROW(::Xsmp::Helper::checkName("[432]", nullptr));
}

} // namespace Xsmp

TEST(Helper, SafeExecuteWithoutEntryPoint) {

  // a null entry point is reported, with or without a simulator, instead of
  // being called
  EXPECT_NO_THROW(::Xsmp::Helper::SafeExecute(nullptr, nullptr));

  ::Xsmp::Simulator sim;
  sim.LoadLibrary("xsmp_services");
  EXPECT_NO_THROW(::Xsmp::Helper::SafeExecute(&sim, nullptr));
  sim.Exit();
}

namespace {
/// A component publishing one simple field, one array field and one structure
/// field, so that the paths of every kind of object can be resolved.
struct Point {
  ::Smp::Int32 x;
  ::Smp::Int32 y;
};

constexpr ::Smp::Uuid uuidArray{0x10, 0, 0, 0, 0};
constexpr ::Smp::Uuid uuidStructure{0x11, 0, 0, 0, 0};

class Publisher final : public ::Xsmp::Model {
public:
  using ::Xsmp::Model::Model;
  void Publish(::Smp::IPublication *receiver) override {
    ::Xsmp::Model::Publish(receiver);
    receiver->PublishField("simple", "", &simple);
    receiver->PublishField("array", "", &array, uuidArray);
    receiver->PublishField("point", "", &point, uuidStructure);
  }
  ::Smp::Int32 simple{1};
  ::Xsmp::Array<::Smp::Int32, 3> array{1, 2, 3};
  Point point{4, 5};
};

void registerTypes(::Smp::Publication::ITypeRegistry *registry) {
  registry->AddArrayType("Int32Array", "", uuidArray, ::Smp::Uuids::Uuid_Int32,
                         sizeof(::Smp::Int32), 3);
  auto *structure = registry->AddStructureType("Point", "", uuidStructure);
  structure->AddField("x", "", ::Smp::Uuids::Uuid_Int32, offsetof(Point, x));
  structure->AddField("y", "", ::Smp::Uuids::Uuid_Int32, offsetof(Point, y));
}
} // namespace

TEST(Helper, ResolveFields) {

  ::Xsmp::Simulator sim;
  sim.LoadLibrary("xsmp_services");
  registerTypes(sim.GetTypeRegistry());
  auto *publisher = new Publisher("publisher", "", &sim, &sim);
  sim.AddModel(publisher);
  sim.Connect();

  auto *resolver = sim.GetResolver();

  // a field of a model, an item of an array field and a member of a structure
  EXPECT_EQ(resolver->ResolveAbsolute("/publisher.simple"),
            publisher->GetField("simple"));
  EXPECT_TRUE(resolver->ResolveAbsolute("/publisher.array[0]"));
  EXPECT_TRUE(resolver->ResolveAbsolute("/publisher.point.x"));

  // a negative index counts from the end
  EXPECT_EQ(resolver->ResolveAbsolute("/publisher.array[-1]"),
            resolver->ResolveAbsolute("/publisher.array[2]"));

  // an index out of range, a malformed index and an unknown name
  EXPECT_FALSE(resolver->ResolveAbsolute("/publisher.array[3]"));
  EXPECT_FALSE(resolver->ResolveAbsolute("/publisher.array[-4]"));
  EXPECT_FALSE(resolver->ResolveAbsolute("/publisher.array[abc]"));
  EXPECT_FALSE(resolver->ResolveAbsolute("/publisher.array[0"));
  EXPECT_FALSE(resolver->ResolveAbsolute("/publisher.point.z"));
  EXPECT_FALSE(resolver->ResolveAbsolute("/unknown"));

  // resolving relative to a field itself
  auto *point = publisher->GetField("point");
  ASSERT_TRUE(point);
  EXPECT_EQ(::Xsmp::Helper::Resolve(static_cast<::Smp::IObject *>(point), "x"),
            resolver->ResolveAbsolute("/publisher.point.x"));
  auto *array = publisher->GetField("array");
  ASSERT_TRUE(array);
  EXPECT_EQ(
      ::Xsmp::Helper::Resolve(static_cast<::Smp::IObject *>(array), "[1]"),
      resolver->ResolveAbsolute("/publisher.array[1]"));

  // '.' and '..' segments
  EXPECT_EQ(resolver->ResolveRelative("../publisher", publisher), publisher);
  EXPECT_EQ(resolver->ResolveRelative("./simple", publisher),
            publisher->GetField("simple"));

  // the path of every kind of object round trips through the resolver
  for (const auto *path : {"/publisher", "/publisher.simple",
                           "/publisher.array[1]", "/publisher.point.y"}) {
    const auto *object = resolver->ResolveAbsolute(path);
    ASSERT_TRUE(object) << path;
    EXPECT_EQ(::Xsmp::Helper::GetPath(object), path);
  }

  sim.Exit();
}

TEST(Helper, AreEquivalent) {

  ::Xsmp::Simulator sim;
  sim.LoadLibrary("xsmp_services");
  registerTypes(sim.GetTypeRegistry());
  auto *first = new Publisher("first", "", &sim, &sim);
  auto *second = new Publisher("second", "", &sim, &sim);
  sim.AddModel(first);
  sim.AddModel(second);
  sim.Connect();

  // the same kind of field with the same type is equivalent
  EXPECT_TRUE(::Xsmp::Helper::AreEquivalent(first->GetField("simple"),
                                            second->GetField("simple")));
  EXPECT_TRUE(::Xsmp::Helper::AreEquivalent(first->GetField("array"),
                                            second->GetField("array")));
  EXPECT_TRUE(::Xsmp::Helper::AreEquivalent(first->GetField("point"),
                                            second->GetField("point")));

  // fields of different kinds are not
  EXPECT_FALSE(::Xsmp::Helper::AreEquivalent(first->GetField("simple"),
                                             second->GetField("array")));
  EXPECT_FALSE(::Xsmp::Helper::AreEquivalent(first->GetField("array"),
                                             second->GetField("point")));

  sim.Exit();
}
