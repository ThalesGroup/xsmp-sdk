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

#include <Smp/ComponentStateKind.h>
#include <Smp/IComponent.h>
#include <Smp/IField.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/ILinkRegistry.h>
#include <Smp/Uuid.h>
#include <Xsmp/Component.h>
#include <Xsmp/Simulator.h>
#include <gtest/gtest.h>

namespace Xsmp::Services {
namespace {

/// A component that does not implement ::Smp::ILinkingComponent, so that the
/// registry cannot ask it to remove its links.
class NonLinkingComponent final : public ::Smp::IComponent {
public:
  explicit NonLinkingComponent(::Smp::String8 name) : _name{name} {}
  ::Smp::String8 GetName() const override { return _name; }
  ::Smp::String8 GetDescription() const override { return ""; }
  ::Smp::IObject *GetParent() const override { return nullptr; }
  ::Smp::ComponentStateKind GetState() const override {
    return ::Smp::ComponentStateKind::CSK_Connected;
  }
  void Publish(::Smp::IPublication *) override {}
  void Configure(::Smp::Services::ILogger *,
                 ::Smp::Services::ILinkRegistry *) override {}
  void Connect(::Smp::ISimulator *) override {}
  void Disconnect() override {}
  ::Smp::IField *GetField(::Smp::String8) const override { return nullptr; }
  const ::Smp::FieldCollection *GetFields() const override { return nullptr; }
  const ::Smp::Uuid &GetUuid() const override { return _uuid; }
  ::Smp::IObject *GetChild(::Smp::String8) const override { return nullptr; }
  ::Smp::AnySimple GetSimpleValue(::Smp::String8) const override { return {}; }
  void SetSimpleValue(::Smp::String8, ::Smp::AnySimple) override {}
  void GetSimpleArrayValue(::Smp::String8, ::Smp::UInt64, ::Smp::AnySimple *,
                           ::Smp::UInt64) const override {}
  void SetSimpleArrayValue(::Smp::String8, ::Smp::UInt64, ::Smp::AnySimpleArray,
                           ::Smp::UInt64) override {}
  ::Smp::Bool AddChild(::Smp::IObject *,
                       const ::Smp::ICollectionBase *) override {
    return false;
  }
  ::Smp::Bool RemoveChild(::Smp::IObject *,
                          const ::Smp::ICollectionBase *) override {
    return false;
  }
  ::Smp::IObject *
  IsChildInCollection(::Smp::String8,
                      const ::Smp::ICollectionBase *) const override {
    return nullptr;
  }

private:
  ::Smp::String8 _name;
  ::Smp::Uuid _uuid{0, 0, 0, 0, 0};
};

/// A component that removes its links from the registry when it is asked to,
/// as a component owning references or event links does.
class LinkingComponent final : public Component {
public:
  LinkingComponent(::Smp::String8 name, ::Smp::ISimulator *simulator,
                   ::Smp::Services::ILinkRegistry *registry)
      : Component(name, "", simulator), _registry{registry} {}

  void RemoveLinks(const ::Smp::IComponent *target) override {
    while (_registry->RemoveLink(this, target)) {
      // remove every link to that target
    }
  }

private:
  ::Smp::Services::ILinkRegistry *_registry;
};

bool contains(const ::Smp::ComponentCollection *collection,
              const ::Smp::IComponent *component) {
  if (!collection) {
    return false;
  }
  for (const auto *element : *collection) {
    if (element == component) {
      return true;
    }
  }
  return false;
}

} // namespace

TEST(XsmpLinkRegistry, AddAndCountLinks) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto *registry = sim.GetLinkRegistry();
  ASSERT_TRUE(registry);

  Component source{"source", "", &sim};
  Component other{"other", "", &sim};
  Component target{"target", "", &sim};

  EXPECT_EQ(registry->GetLinkCount(&source, &target), 0U);

  registry->AddLink(&source, &target);
  EXPECT_EQ(registry->GetLinkCount(&source, &target), 1U);

  // the same link twice is counted twice
  registry->AddLink(&source, &target);
  EXPECT_EQ(registry->GetLinkCount(&source, &target), 2U);

  // links are directed and specific to a pair
  EXPECT_EQ(registry->GetLinkCount(&target, &source), 0U);
  EXPECT_EQ(registry->GetLinkCount(&other, &target), 0U);

  registry->AddLink(&other, &target);
  EXPECT_TRUE(contains(registry->GetLinkSources(&target), &source));
  EXPECT_TRUE(contains(registry->GetLinkSources(&target), &other));
  EXPECT_FALSE(contains(registry->GetLinkSources(&source), &target));
}

TEST(XsmpLinkRegistry, RemoveLink) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto *registry = sim.GetLinkRegistry();
  ASSERT_TRUE(registry);

  Component source{"source", "", &sim};
  Component target{"target", "", &sim};

  // removing an unknown link does nothing
  EXPECT_FALSE(registry->RemoveLink(&source, &target));

  registry->AddLink(&source, &target);
  registry->AddLink(&source, &target);

  // the link is only dropped when its counter reaches 0
  EXPECT_TRUE(registry->RemoveLink(&source, &target));
  EXPECT_EQ(registry->GetLinkCount(&source, &target), 1U);
  EXPECT_TRUE(contains(registry->GetLinkSources(&target), &source));

  EXPECT_TRUE(registry->RemoveLink(&source, &target));
  EXPECT_EQ(registry->GetLinkCount(&source, &target), 0U);
  EXPECT_FALSE(contains(registry->GetLinkSources(&target), &source));

  EXPECT_FALSE(registry->RemoveLink(&source, &target));
}

TEST(XsmpLinkRegistry, CanRemove) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto *registry = sim.GetLinkRegistry();
  ASSERT_TRUE(registry);

  Component linking{"linking", "", &sim};
  NonLinkingComponent nonLinking{"nonLinking"};
  Component target{"target", "", &sim};

  // a target without any source can always be removed
  EXPECT_TRUE(registry->CanRemove(&target));

  // an ::Smp::ILinkingComponent can be asked to remove its links
  registry->AddLink(&linking, &target);
  EXPECT_TRUE(registry->CanRemove(&target));

  // a source that is not an ::Smp::ILinkingComponent cannot
  registry->AddLink(&nonLinking, &target);
  EXPECT_FALSE(registry->CanRemove(&target));
}

TEST(XsmpLinkRegistry, RemoveLinks) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto *registry = sim.GetLinkRegistry();
  ASSERT_TRUE(registry);

  Component source{"source", "", &sim};
  NonLinkingComponent nonLinking{"nonLinking"};
  Component target{"target", "", &sim};
  Component otherTarget{"otherTarget", "", &sim};

  registry->AddLink(&source, &target);
  registry->AddLink(&nonLinking, &target);
  registry->AddLink(&source, &otherTarget);

  // every source that can is asked to remove its links to the target; a
  // component without any reference or event source has none to remove
  registry->RemoveLinks(&target);

  // the links to the other targets are left untouched
  EXPECT_EQ(registry->GetLinkCount(&source, &otherTarget), 1U);
}

TEST(XsmpLinkRegistry, RemoveLinksOfLinkingComponents) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  sim.Connect();
  auto *registry = sim.GetLinkRegistry();
  ASSERT_TRUE(registry);

  Component target{"target", "", &sim};
  LinkingComponent first{"first", &sim, registry};
  LinkingComponent second{"second", &sim, registry};

  registry->AddLink(&first, &target);
  registry->AddLink(&first, &target);
  registry->AddLink(&second, &target);

  // each source drops its own links while the registry walks the sources
  registry->RemoveLinks(&target);

  EXPECT_EQ(registry->GetLinkCount(&first, &target), 0U);
  EXPECT_EQ(registry->GetLinkCount(&second, &target), 0U);
  EXPECT_FALSE(contains(registry->GetLinkSources(&target), &first));
  EXPECT_FALSE(contains(registry->GetLinkSources(&target), &second));
}

} // namespace Xsmp::Services
