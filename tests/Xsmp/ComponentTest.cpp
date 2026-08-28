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

#include "Smp/Exception.h"
#include <Smp/ComponentStateKind.h>
#include <Smp/DuplicateName.h>
#include <Smp/IPublication.h>
#include <Smp/InvalidComponentState.h>
#include <Smp/InvalidOperationName.h>
#include <Smp/ViewKind.h>
#include <Xsmp/Component.h>
#include <Xsmp/Composite.h>
#include <Xsmp/Container.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/EntryPointPublisher.h>
#include <Xsmp/Model.h>
#include <Xsmp/Publication/Publication.h>
#include <Xsmp/Publication/TypeRegistry.h>
#include <gtest/gtest.h>

namespace Xsmp {

TEST(Component, CheckGetters) {

  Component component{"component", "desc"};

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

  Publication::Publication publication{&component, &registry};

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

/// A model with one entry point, one container and one published field, to
/// check that IObject::GetChild reaches every kind of named child.
class ModelWithChildren final : public ::Xsmp::Model,
                                public ::Xsmp::EntryPointPublisher,
                                public ::Xsmp::Composite {
public:
  ModelWithChildren(::Smp::String8 name, ::Smp::IComposite *parent)
      : ::Xsmp::Model(name, "", parent, nullptr) {}

  void Publish(::Smp::IPublication *receiver) override {
    ::Xsmp::Model::Publish(receiver);
    receiver->PublishField("counter", "", &_counter);
    auto *structure = receiver->PublishStructure("structure", "");
    structure->PublishField("x", "", &_x);
    auto *array = receiver->PublishArray("array", "");
    array->PublishField("[0]", "", &_items[0]);
    array->PublishField("[1]", "", &_items[1]);
  }

  ::Xsmp::EntryPoint step{"step", "", this, [] {}};
  ::Xsmp::Container<::Smp::IModel> models{"models", "", this, 0, -1};

private:
  ::Smp::Int32 _counter{};
  ::Smp::Int32 _x{};
  ::Smp::Int32 _items[2]{};
};

TEST(Component, GetChild) {
  Publication::TypeRegistry registry;
  ModelWithChildren model{"model", nullptr};
  Publication::Publication publication{&model, &registry};
  model.Publish(&publication);

  // SMP 2025 asks every object for its children by name
  EXPECT_EQ(model.GetChild("step"), &model.step);
  EXPECT_EQ(model.GetChild("models"), &model.models);
  EXPECT_EQ(model.GetChild("counter"), publication.GetField("counter"));
  EXPECT_FALSE(model.GetChild("absent"));
  EXPECT_FALSE(model.GetChild(nullptr));
  EXPECT_FALSE(model.GetChild(""));

  // a component held by a container is a child of its owner too
  auto *child = new ModelWithChildren{"child", &model};
  model.models.AddComponent(child);
  EXPECT_EQ(model.GetChild("child"), child);

  // a leaf has no child
  EXPECT_FALSE(model.step.GetChild("anything"));

  // a structure field answers with its sub-fields, an array field with its
  // items, which are named "[i]"
  auto *structure = publication.GetField("structure");
  ASSERT_TRUE(structure);
  EXPECT_EQ(structure->GetChild("x"), publication.GetField("structure.x"));
  EXPECT_FALSE(structure->GetChild("absent"));

  auto *array = publication.GetField("array");
  ASSERT_TRUE(array);
  EXPECT_EQ(array->GetChild("[1]"), publication.GetField("array[1]"));
  EXPECT_FALSE(array->GetChild("[9]"));
  EXPECT_FALSE(array->GetChild("x"));
}

TEST(Component, IsChildInCollection) {
  Publication::TypeRegistry registry;
  ModelWithChildren model{"model", nullptr};
  Publication::Publication publication{&model, &registry};
  model.Publish(&publication);

  // the collections of the component register their elements, so a caller
  // can ask which collection a name belongs to
  EXPECT_EQ(model.IsChildInCollection("step", model.GetEntryPoints()),
            &model.step);
  EXPECT_EQ(model.IsChildInCollection("models", model.GetContainers()),
            &model.models);
  EXPECT_EQ(model.IsChildInCollection("counter", publication.GetFields()),
            publication.GetField("counter"));

  // the right name in the wrong collection is not a member
  EXPECT_FALSE(model.IsChildInCollection("step", model.GetContainers()));
  EXPECT_FALSE(model.IsChildInCollection("absent", model.GetEntryPoints()));

  // a component held by a container is registered by the container
  auto *child = new ModelWithChildren{"child", &model};
  model.models.AddComponent(child);
  EXPECT_EQ(model.IsChildInCollection("child", model.models.GetComponents()),
            child);
  model.models.DeleteComponent(child);
  EXPECT_FALSE(
      model.IsChildInCollection("child", model.models.GetComponents()));
  EXPECT_FALSE(model.GetChild("child"));

  // a name already used by another collection of the component is rejected
  EXPECT_THROW(::Xsmp::EntryPoint("models", "", &model, [] {}),
               ::Smp::DuplicateName);

  // a rejected registration must not evict the child that owns the name
  ::Xsmp::Object impostor{"step", "", &model};
  EXPECT_FALSE(model.AddChild(&impostor, model.GetContainers()));
  EXPECT_EQ(model.IsChildInCollection("step", model.GetEntryPoints()),
            &model.step);
  EXPECT_EQ(model.GetChild("step"), &model.step);

  // unpublishing releases the published names
  publication.Unpublish();
  EXPECT_FALSE(model.IsChildInCollection("counter", publication.GetFields()));
}

} // namespace Xsmp
