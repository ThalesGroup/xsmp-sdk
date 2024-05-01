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

#include <Smp/CannotRemove.h>
#include <Smp/CollectionIterator.h>
#include <Smp/InvalidObjectType.h>
#include <Smp/NotReferenced.h>
#include <Smp/ReferenceFull.h>
#include <Xsmp/Aggregate.h>
#include <Xsmp/Component.h>
#include <Xsmp/Reference.h>
#include <cstddef>
#include <gtest/gtest.h>
#include <type_traits>

namespace Xsmp {

namespace {
class Interface {
public:
  virtual ~Interface() = default;
};

class M1 : public ::Xsmp::Component {
public:
  using ::Xsmp::Component::Component;
};
class M2 : public ::Xsmp::Component, public virtual Interface {
public:
  using ::Xsmp::Component::Component;
};

class TestAggregate : public Xsmp::Component, public virtual Xsmp::Aggregate {
public:
  using Xsmp::Component::Component;
};

} // namespace

TEST(ReferenceTest, auto_register) {

  TestAggregate c{"collection"};

  EXPECT_EQ(0, c.GetReferences()->size());
  Reference<Interface> ref{"ref1", "", &c, 1, 2};
  EXPECT_EQ(1, c.GetReferences()->size());
  EXPECT_EQ(&ref, c.GetReference("ref1"));
  EXPECT_EQ(&ref, c.GetReferences()->at("ref1"));

  ASSERT_TRUE(ref.GetComponents());
  EXPECT_STREQ("Collection", ref.GetComponents()->GetName());
  EXPECT_STREQ("Collection of component",
               ref.GetComponents()->GetDescription());
  EXPECT_EQ(&ref, ref.GetComponents()->GetParent());

  EXPECT_FALSE(ref.GetComponents()->begin() != ref.GetComponents()->end());
  EXPECT_EQ(ref.GetComponents()->size(), 0);

  EXPECT_EQ(1, ref.GetLower());
  EXPECT_EQ(2, ref.GetUpper());

  EXPECT_EQ(nullptr, ref.GetComponent(nullptr));

  M1 bad_cmp{"m1", "", nullptr};

  EXPECT_THROW(ref.AddComponent(&bad_cmp), ::Smp::InvalidObjectType);

  EXPECT_EQ(nullptr, ref.GetComponent("m1"));

  M2 i1{"i1", "", nullptr};
  ref.AddComponent(&i1);

  EXPECT_EQ(1, ref.GetCount());
  EXPECT_EQ(&i1, ref.GetComponent("i1"));
  EXPECT_EQ(&i1, ref.GetComponents()->at("i1"));
  EXPECT_EQ(&i1, ref.GetComponents()->at(std::size_t(0)));
  EXPECT_EQ(&i1, ref.GetComponent(std::size_t(0)));
  EXPECT_EQ(nullptr, ref.GetComponent(std::size_t(1)));

  EXPECT_THROW(ref.RemoveComponent(&i1), ::Smp::CannotRemove);

  M2 i1_bis{"i1", "", nullptr};
  ref.AddComponent(&i1_bis);

  EXPECT_EQ(2, ref.GetCount());
  EXPECT_EQ(&i1, ref.GetComponent("i1"));
  EXPECT_EQ(&i1_bis, ref.GetComponent(std::size_t(1)));

  M2 i3{"i3", "", nullptr};
  EXPECT_THROW(ref.RemoveComponent(&i3), ::Smp::NotReferenced);
  EXPECT_THROW(ref.AddComponent(&i3), ::Smp::ReferenceFull);

  ref.RemoveComponent(&i1);

  EXPECT_EQ(1, ref.GetCount());
  EXPECT_EQ(&i1_bis, ref.GetComponent("i1"));
}

} // namespace Xsmp
