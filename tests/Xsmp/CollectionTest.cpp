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

#include <gtest/gtest.h>
#include <cstddef>
#include <Smp/CollectionIterator.h>
#include <Smp/DuplicateName.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Collection.h>
#include <type_traits>

namespace Xsmp {

class Interface {
public:
    virtual ~Interface() = default;
};

class InterfaceObj: public Object, public virtual Interface {
public:
    InterfaceObj(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent) :
            Object(name, description, parent) {

    }

    ~InterfaceObj() noexcept override = default;
};

TEST(CollectionTest, simple) {

    Collection<Interface> c { "collection", "", nullptr };

    EXPECT_EQ(0, c.size());
    EXPECT_EQ(nullptr, c.at(10));
    EXPECT_EQ(nullptr, c.at("toto"));
    EXPECT_FALSE(c.end() != c.begin());

    InterfaceObj o1 { "o1", "", nullptr };
    c.Add(&o1);

    EXPECT_EQ(1, c.size());
    EXPECT_EQ(&o1, c.at(std::size_t(0)));
    EXPECT_EQ(&o1, c.at("o1"));

    EXPECT_EQ(nullptr, c.at(10));
    EXPECT_EQ(nullptr, c.at("toto"));

    InterfaceObj o1_bis { "o1", "", nullptr };
    EXPECT_THROW(c.Add(&o1_bis), ::Smp::DuplicateName);

    Interface i;
    EXPECT_THROW(c.Add(&i), ::Smp::Exception);
    EXPECT_THROW(c.Remove(&i), ::Smp::Exception);

    InterfaceObj o2 { "o2", "", nullptr };
    c.Add(&o2);
    EXPECT_EQ(2, c.size());
    EXPECT_EQ(&o2, c.at(std::size_t(1)));
    EXPECT_EQ(&o2, c.at("o2"));

    EXPECT_EQ(nullptr, c.at(10));

    auto it = c.begin();
    ASSERT_TRUE(c.end() != it);
    EXPECT_EQ(&o1, *it);
    ++it;
    ASSERT_TRUE(c.end() != it);
    EXPECT_EQ(&o2, *it);
    ++it;
    ASSERT_FALSE(c.end() != it);

    c.Remove(&o1);
    EXPECT_EQ(1, c.size());
    EXPECT_EQ(&o2, c.at(std::size_t(0)));
    c.clear();
    EXPECT_EQ(0, c.size());
}

TEST(CollectionTest, delegate) {
    Object parent { "p", "", nullptr };
    Collection<Interface> c { "collection", "desc", &parent };

    InterfaceObj o1 { "o1", "", nullptr };
    c.Add(&o1);

    InterfaceObj o2 { "o2", "", nullptr };
    c.Add(&o2);

    DelegateCollection<Interface> delegate { &c };

    EXPECT_EQ(c.GetName(), delegate.GetName());
    EXPECT_EQ(c.GetDescription(), delegate.GetDescription());
    EXPECT_EQ(c.GetParent(), delegate.GetParent());
    EXPECT_EQ(c.size(), delegate.size());
    EXPECT_EQ(&o1, c.at(std::size_t(0)));
    EXPECT_EQ(&o2, c.at(std::size_t(1)));
    EXPECT_EQ(&o1, c.at("o1"));
    EXPECT_EQ(&o2, c.at("o2"));

    EXPECT_EQ(&o1, *c.begin());

    InterfaceObj o3 { "o3", "", nullptr };
    delegate.Add(&o3);
    EXPECT_EQ(c.size() + 1, delegate.size());
    EXPECT_EQ(&o1, delegate.at(std::size_t(0)));
    EXPECT_EQ(&o2, delegate.at(std::size_t(1)));
    EXPECT_EQ(&o1, delegate.at("o1"));
    EXPECT_EQ(&o2, delegate.at("o2"));
    EXPECT_EQ(&o3, delegate.at("o3"));
    EXPECT_EQ(&o3, delegate.at(std::size_t(2)));

    auto it = delegate.begin();
    ASSERT_TRUE(delegate.end() != it);
    EXPECT_EQ(&o1, *it);
    ++it;
    ASSERT_TRUE(delegate.end() != it);
    EXPECT_EQ(&o2, *it);
    ++it;
    ASSERT_TRUE(delegate.end() != it);
    EXPECT_EQ(&o3, *it);
    ++it;
    ASSERT_FALSE(delegate.end() != it);

    InterfaceObj o1_bis { "o1", "", nullptr };
    EXPECT_THROW(delegate.Add(&o1_bis), ::Smp::DuplicateName);

    delegate.Remove(&o3);
    EXPECT_EQ(c.size(), delegate.size());

}

} // namespace Xsmp
