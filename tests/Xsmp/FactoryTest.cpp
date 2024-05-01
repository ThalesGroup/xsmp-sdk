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

#include <Smp/CollectionIterator.h>
#include <Smp/DuplicateUuid.h>
#include <Smp/IFactory.h>
#include <Smp/Uuid.h>
#include <Xsmp/Composite.h>
#include <Xsmp/Factory.h>
#include <Xsmp/FactoryCollection.h>
#include <Xsmp/Model.h>
#include <cstddef>
#include <gtest/gtest.h>
#include <type_traits>

namespace Xsmp::Factory {

namespace {
class TestComposite : public Xsmp::Component, public virtual Xsmp::Composite {
public:
  using Xsmp::Component::Component;
};
} // namespace
class M1 : public ::Xsmp::Model {
public:
  using ::Xsmp::Model::Model;
  const ::Smp::Uuid &GetUuid() const override { return uuid; }
  static constexpr ::Smp::Uuid uuid{1, 2, 3, 4, 5};
};
constexpr ::Smp::Uuid M1::uuid;
class M2 : public ::Xsmp::Model {
public:
  using ::Xsmp::Model::Model;
  const ::Smp::Uuid &GetUuid() const override { return uuid; }
  static constexpr ::Smp::Uuid uuid{1, 2, 3, 4, 6};
};
constexpr ::Smp::Uuid M2::uuid;

TEST(FactoryTest, Create) {

  auto *factory{::Xsmp::Factory::Create<M1>("M1",       // name
                                            "M1 model", // description
                                            nullptr,    // simulator
                                            M1::uuid    // UUID
                                            )};

  EXPECT_STREQ(factory->GetName(), "M1");
  EXPECT_STREQ(factory->GetDescription(), "M1 model");
  EXPECT_STREQ(factory->GetTypeName(), "Xsmp::Factory::M1");
  EXPECT_EQ(factory->GetUuid(), M1::uuid);
  EXPECT_EQ(factory->GetParent(), nullptr);

  TestComposite composite{"composite"};

  auto *instance =
      factory->CreateInstance("instance", "description", &composite);
  ASSERT_TRUE(dynamic_cast<M1 *>(instance));

  EXPECT_STREQ(instance->GetName(), "instance");
  EXPECT_STREQ(instance->GetDescription(), "description");
  EXPECT_EQ(instance->GetUuid(), M1::uuid);
  EXPECT_EQ(instance->GetParent(), &composite);

  factory->DeleteInstance(instance);

  delete factory;
}

TEST(FactoryTest, WrongUuid) {

  auto *factory{::Xsmp::Factory::Create<M1>("M1",       // name
                                            "M1 model", // description
                                            nullptr,    // simulator
                                            M2::uuid    // UUID
                                            )};

  EXPECT_STREQ(factory->GetName(), "M1");
  EXPECT_STREQ(factory->GetDescription(), "M1 model");
  EXPECT_STREQ(factory->GetTypeName(), "Xsmp::Factory::M1");
  EXPECT_EQ(factory->GetUuid(), M2::uuid);
  EXPECT_EQ(factory->GetParent(), nullptr);

  EXPECT_THROW(factory->CreateInstance("instance", "description", nullptr),
               Smp::Exception);

  delete factory;
}

TEST(FactoryCollectionTest, FactoryCollection) {

  Xsmp::FactoryCollection collection{nullptr};
  auto *m1 = ::Xsmp::Factory::Create<M1>("M1", "M1 model", nullptr, M1::uuid);
  collection.Add(m1);
  auto *m2 = ::Xsmp::Factory::Create<M2>("M2", "M2 model", nullptr, M2::uuid);
  collection.Add(m2);

  EXPECT_STREQ(collection.GetName(), "Factories");
  EXPECT_STREQ(collection.GetDescription(), "");

  EXPECT_EQ(collection.at(nullptr), nullptr);
  EXPECT_EQ(collection.at("M1"), m1);
  EXPECT_EQ(collection.at("M2"), m2);
  EXPECT_EQ(collection.at("M3"), nullptr);

  EXPECT_EQ(collection.at(static_cast<std::size_t>(0)), m1);
  EXPECT_EQ(collection.at(1), m2);
  EXPECT_EQ(collection.at(2), nullptr);

  EXPECT_EQ(collection.at(M1::uuid), m1);
  EXPECT_EQ(collection.at(M2::uuid), m2);
  EXPECT_EQ(collection.at(::Smp::Uuid{}), nullptr);

  auto it = collection.begin();
  EXPECT_EQ(*it, m1);
  EXPECT_TRUE(it != collection.end());
  ++it;
  EXPECT_EQ(*it, m2);
  EXPECT_TRUE(it != collection.end());
  ++it;
  EXPECT_FALSE(it != collection.end());
  EXPECT_EQ(collection.size(), 2);

  EXPECT_THROW(collection.Add(::Xsmp::Factory::Create<M1>("M1_bis", "M1 model",
                                                          nullptr, M1::uuid)),
               Smp::DuplicateUuid);
  EXPECT_EQ(collection.size(), 2);
}
} // namespace Xsmp::Factory
