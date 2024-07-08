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

#include <Smp/CannotDelete.h>
#include <Smp/ContainerFull.h>
#include <Smp/DuplicateName.h>
#include <Smp/InvalidObjectType.h>
#include <Smp/NotContained.h>
#include <Xsmp/Composite.h>
#include <Xsmp/Container.h>
#include <Xsmp/Model.h>
#include <cstddef>
#include <gtest/gtest.h>

namespace Xsmp {
namespace {
class Interface {
public:
  virtual ~Interface() = default;
};

class M1 : public ::Xsmp::Model {
public:
  using ::Xsmp::Model::Model;
};
class M2 : public ::Xsmp::Model, public virtual Interface {
public:
  using ::Xsmp::Model::Model;
};

class TestComposite : public Xsmp::Component, public virtual Xsmp::Composite {
public:
  using Xsmp::Component::Component;
};

} // namespace
TEST(Container, auto_register) {

  TestComposite composite{"composite"};

  Container<Interface> ctn{"ctn", "desc", &composite, 1, 2};
  EXPECT_EQ(1, composite.GetContainers()->size());

  EXPECT_EQ(&ctn, composite.GetContainer("ctn"));
  EXPECT_EQ(&ctn, composite.GetContainers()->at("ctn"));
  EXPECT_EQ(&ctn, composite.GetContainers()->at(static_cast<std::size_t>(0)));
  EXPECT_STREQ("ctn", ctn.GetName());
  EXPECT_STREQ("desc", ctn.GetDescription());
  EXPECT_EQ(&composite, ctn.GetParent());

  ASSERT_TRUE(ctn.GetComponents());
  EXPECT_STREQ("Collection", ctn.GetComponents()->GetName());
  EXPECT_STREQ("Collection of component",
               ctn.GetComponents()->GetDescription());
  EXPECT_EQ(&ctn, ctn.GetComponents()->GetParent());

  EXPECT_EQ(1, ctn.GetLower());
  EXPECT_EQ(2, ctn.GetUpper());
  EXPECT_EQ(nullptr, ctn.GetComponent(nullptr));
  EXPECT_EQ(nullptr, ctn.GetComponent(10));

  M1 bad_cmp{"m1", "", &composite};

  EXPECT_THROW(ctn.AddComponent(&bad_cmp), ::Smp::InvalidObjectType);
  EXPECT_EQ(nullptr, ctn.GetComponent("m1"));

  auto *i1 = new M2{"i1", "", &composite};
  ctn.AddComponent(i1);

  EXPECT_EQ(1, ctn.GetCount());
  EXPECT_EQ(i1, ctn.GetComponent("i1"));
  EXPECT_EQ(i1, ctn.GetComponent(std::size_t(0)));
  EXPECT_EQ(i1, ctn.GetComponents()->at("i1"));

  EXPECT_EQ(nullptr, ctn.GetComponent(std::size_t(1)));

  EXPECT_THROW(ctn.DeleteComponent(i1), ::Smp::CannotDelete);

  auto *i1_bis = new M2{"i1", "", &composite};
  EXPECT_THROW(ctn.AddComponent(i1_bis), ::Smp::DuplicateName);
  delete i1_bis;
  auto *i2 = new M2{"i2", "", &composite};
  ctn.AddComponent(i2);

  EXPECT_EQ(2, ctn.GetCount());
  EXPECT_EQ(i2, ctn.GetComponent("i2"));
  EXPECT_EQ(i2, ctn.GetComponent(std::size_t(1)));

  auto *i3 = new M2{"i3", "", &composite};
  EXPECT_THROW(ctn.DeleteComponent(i3), ::Smp::NotContained);
  EXPECT_THROW(ctn.AddComponent(i3), ::Smp::ContainerFull);
  delete i3;
  ctn.DeleteComponent(i1);

  EXPECT_EQ(1, ctn.GetCount());
  EXPECT_EQ(nullptr, ctn.GetComponent("i1"));
}

TEST(Container, bounds) {

  TestComposite composite{"composite", "", nullptr};

  const auto *c = composite.GetContainers();

  Container<Interface> ctn{"ctn", "", &composite, 1, -1};
  EXPECT_EQ(1, c->size());
  EXPECT_EQ(1, ctn.GetLower());
  EXPECT_EQ(-1, ctn.GetUpper());

  auto *i1 = new M2{"i1", "", &composite};
  ctn.AddComponent(i1);

  EXPECT_EQ(1, ctn.GetCount());
  EXPECT_EQ(i1, ctn.GetComponent("i1"));
  EXPECT_EQ(i1, ctn.GetComponent(std::size_t(0)));
}

} // namespace Xsmp
