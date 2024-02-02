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
#include <Smp/ComponentStateKind.h>
#include <Smp/InvalidComponentState.h>
#include <Smp/InvalidOperationName.h>
#include <Xsmp/Component.h>
#include <Xsmp/Publication/Publication.h>
#include <Xsmp/Publication/TypeRegistry.h>
#include <type_traits>

namespace Xsmp {

TEST(ComponentTest, Component) {

    Component component { "component", "desc" };

    EXPECT_STREQ(component.GetName(), "component");
    EXPECT_STREQ(component.GetDescription(), "desc");
    EXPECT_EQ(component.GetState(), Smp::ComponentStateKind::CSK_Created);
    EXPECT_FALSE(component.GetFields());
    EXPECT_FALSE(component.GetField("name"));
    EXPECT_FALSE(component.GetOperations());
    EXPECT_FALSE(component.GetProperties());
    EXPECT_THROW(component.GetUuid(), Smp::Exception);

    EXPECT_THROW(component.Invoke(nullptr), Smp::InvalidOperationName);

    component.DeleteRequest(nullptr);

    Publication::TypeRegistry registry;

    Publication::Publication publication { &component, &registry };

    EXPECT_THROW(component.Configure(nullptr, nullptr),
            Smp::InvalidComponentState);

    EXPECT_THROW(component.Connect(nullptr), Smp::InvalidComponentState);

    EXPECT_THROW(component.Disconnect(), Smp::InvalidComponentState);

    EXPECT_THROW(component.Publish(nullptr), Smp::Exception);

    component.Publish(&publication);
    EXPECT_EQ(component.GetState(), Smp::ComponentStateKind::CSK_Publishing);
    EXPECT_TRUE(component.GetFields());
    EXPECT_TRUE(component.GetOperations());
    EXPECT_TRUE(component.GetProperties());

    auto *request = component.CreateRequest("op");
    EXPECT_FALSE(request);
    publication.PublishOperation("op", "", Smp::ViewKind::VK_Debug);
    request = component.CreateRequest("op");
    ASSERT_TRUE(request);

    EXPECT_THROW(component.Invoke(request), Smp::InvalidOperationName);

    component.DeleteRequest(request);
    component.DeleteRequest(nullptr);

    EXPECT_THROW(component.Connect(nullptr), Smp::InvalidComponentState);

    EXPECT_THROW(component.Publish(&publication), Smp::InvalidComponentState);
    EXPECT_THROW(component.Disconnect(), Smp::InvalidComponentState);

    component.Configure(nullptr);
    EXPECT_EQ(component.GetState(), Smp::ComponentStateKind::CSK_Configured);

    EXPECT_THROW(component.Publish(&publication), Smp::InvalidComponentState);
    EXPECT_THROW(component.Configure(nullptr, nullptr),
            Smp::InvalidComponentState);

    EXPECT_THROW(component.Disconnect(), Smp::InvalidComponentState);

    component.Connect(nullptr);
    EXPECT_EQ(component.GetState(), Smp::ComponentStateKind::CSK_Connected);
    EXPECT_THROW(component.Publish(&publication), Smp::InvalidComponentState);
    EXPECT_THROW(component.Configure(nullptr, nullptr),
            Smp::InvalidComponentState);
    EXPECT_THROW(component.Connect(nullptr), Smp::InvalidComponentState);

    component.Disconnect();
    EXPECT_EQ(component.GetState(), Smp::ComponentStateKind::CSK_Disconnected);
    EXPECT_THROW(component.Publish(&publication), Smp::InvalidComponentState);
    EXPECT_THROW(component.Configure(nullptr, nullptr),
            Smp::InvalidComponentState);
    EXPECT_THROW(component.Connect(nullptr), Smp::InvalidComponentState);
    EXPECT_THROW(component.Disconnect(), Smp::InvalidComponentState);

}

} // namespace Xsmp
