// Copyright 2026 THALES ALENIA SPACE FRANCE. All rights reserved.
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
#include <Smp/DuplicateName.h>
#include <Smp/IModel.h>
#include <Smp/IOutputField.h>
#include <Smp/ISimpleField.h>
#include <Smp/InvalidFile.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/EntryPointPublisher.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Model.h>
#include <Xsmp/Simulator.h>

#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

namespace Xsmp::L2 {
namespace {

class TemporaryContractDirectory final {
public:
  TemporaryContractDirectory() {
    const auto suffix = std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count());
    path = std::filesystem::temp_directory_path() /
           ("xsmp-simulator-l2-contract-" + suffix);
    std::filesystem::create_directories(path);
  }

  ~TemporaryContractDirectory() {
    std::error_code error;
    std::filesystem::remove_all(path, error);
  }

  std::filesystem::path Write(std::string_view name,
                              std::string_view contents) const {
    const auto result = path / name;
    std::ofstream stream{result, std::ios::binary};
    stream.write(contents.data(),
                 static_cast<std::streamsize>(contents.size()));
    return result;
  }

  std::filesystem::path path;
};

std::filesystem::path
WriteContractAssembly(const TemporaryContractDirectory &directory,
                      std::string_view fileName, std::string_view modelName) {
  std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="ContractAssembly" Name="ContractAssembly">
  <Model Name=")";
  xml += modelName;
  xml += R"(" Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
</Assembly:Assembly>)";
  return directory.Write(fileName, xml);
}

::Smp::ISimpleField *ResolveIntField(::Xsmp::Simulator &simulator,
                                     std::string_view path) {
  auto *model = dynamic_cast<::Smp::IModel *>(
      ::Xsmp::Helper::Resolve(&simulator, std::string{path}.c_str()));
  return model
             ? dynamic_cast<::Smp::ISimpleField *>(model->GetField("integer1"))
             : nullptr;
}

std::string ScheduleHeader(std::string_view name) {
  return std::string{R"(<?xml version="1.0" encoding="UTF-8"?>
<Schedule:Schedule
    xmlns:Schedule="http://www.ecss.nl/smp/2025/Smdl/Schedule"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:xlink="http://www.w3.org/1999/xlink"
    Id=")"} +
         std::string{name} + "\" Name=\"" + std::string{name} + "\">\n";
}

class TaskProbe final : public ::Xsmp::Model,
                        public virtual ::Xsmp::EntryPointPublisher {
public:
  TaskProbe(::Smp::String8 name, ::Smp::IComposite *parent,
            ::Smp::ISimulator *simulator)
      : ::Xsmp::Model{name, "", parent, simulator},
        hit{"hit", "", this, [this] { ++calls; }} {}

  int calls{};
  ::Xsmp::EntryPoint hit;
};

TEST(SimulatorL2Contract, DeferredActionDoesNotBlockIndependentOverrideChain) {
  TemporaryContractDirectory directory;
  const auto configuration = directory.Write(
      "deferred.smpcfg", R"(<?xml version="1.0" encoding="UTF-8"?>
<Configuration:Configuration
    xmlns:Configuration="http://www.ecss.nl/smp/2025/Smdl/Configuration"
    xmlns:Types="http://www.ecss.nl/smp/2025/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="Deferred" Name="Deferred">
  <Component Path="/Missing">
    <FieldValue xsi:type="Types:Int32Value" Value="1" Field="integer1"/>
  </Component>
  <Component Path="/Probe">
    <FieldValue xsi:type="Types:Int32Value" Value="10" Field="integer1"/>
    <FieldValue xsi:type="Types:Int32Value" Value="20" Field="integer1"/>
  </Component>
</Configuration:Configuration>)");
  const auto assembly =
      WriteContractAssembly(directory, "probe.smpasb", "Probe");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  simulator.LoadConfiguration(configuration.string().c_str(), "");
  simulator.LoadAssembly(assembly.string().c_str(), "", "", "");

  auto *field = ResolveIntField(simulator, "/Probe");
  ASSERT_NE(field, nullptr);
  EXPECT_EQ(static_cast<::Smp::Int32>(field->GetValue()), 20);
}

TEST(SimulatorL2Contract, AppliesAssemblyOverridePrecedence) {
  TemporaryContractDirectory directory;
  directory.Write("inner.smpcfg", R"(<?xml version="1.0" encoding="UTF-8"?>
<Configuration:Configuration
    xmlns:Configuration="http://www.ecss.nl/smp/2025/Smdl/Configuration"
    xmlns:Types="http://www.ecss.nl/smp/2025/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="InnerConfiguration" Name="InnerConfiguration">
  <Component Path="/">
    <FieldValue xsi:type="Types:Int32Value" Value="25" Field="integer1"/>
  </Component>
</Configuration:Configuration>)");
  directory.Write("inner.smpasb", R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="Inner" Name="Inner">
  <ComponentConfiguration InstancePath=".">
    <FieldValue xsi:type="Types:Int32Value" Value="2" Field="integer1"/>
  </ComponentConfiguration>
  <Model Name="InnerTemplate"
         Implementation="Xsmp::Tests::ModelWithSimpleFields">
    <FieldValue xsi:type="Types:Int32Value" Value="1" Field="integer1"/>
  </Model>
</Assembly:Assembly>)");
  const auto outer =
      directory.Write("outer.smpasb", R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="Outer" Name="Outer">
  <ComponentConfiguration InstancePath="Inner">
    <FieldValue xsi:type="Types:Int32Value" Value="4" Field="integer1"/>
  </ComponentConfiguration>
  <Model Name="Outer" Implementation="Xsmp::Tests::ModelWithSimpleFields">
    <Assembly Name="Inner" Container="subModels">
      <Assembly>inner.smpasb</Assembly>
      <ModelConfiguration InstancePath=".">
        <FieldValue xsi:type="Types:Int32Value" Value="3" Field="integer1"/>
      </ModelConfiguration>
      <Configuration>inner.smpcfg</Configuration>
    </Assembly>
  </Model>
</Assembly:Assembly>)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  simulator.LoadAssembly(outer.string().c_str(), "", "", "");

  auto *field = ResolveIntField(simulator, "/Outer/Inner");
  ASSERT_NE(field, nullptr);
  EXPECT_EQ(static_cast<::Smp::Int32>(field->GetValue()), 3);
}

TEST(SimulatorL2Contract,
     LaterExplicitConfigurationWinsEarlierDeferredInlineConfiguration) {
  TemporaryContractDirectory directory;
  directory.Write("inner.smpasb", R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="Inner" Name="Inner">
  <Model Name="InnerTemplate"
         Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
</Assembly:Assembly>)");
  const auto outer = directory.Write("outer-deferred.smpasb",
                                     R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="OuterDeferred" Name="OuterDeferred">
  <Model Name="Outer" Implementation="Xsmp::Tests::ModelWithSimpleFields">
    <Assembly Name="Inner" Container="subModels">
      <Assembly>inner.smpasb</Assembly>
      <ModelConfiguration InstancePath="Late">
        <FieldValue xsi:type="Types:Int32Value" Value="3" Field="integer1"/>
      </ModelConfiguration>
    </Assembly>
  </Model>
</Assembly:Assembly>)");
  const auto laterConfiguration =
      directory.Write("later.smpcfg", R"(<?xml version="1.0" encoding="UTF-8"?>
<Configuration:Configuration
    xmlns:Configuration="http://www.ecss.nl/smp/2025/Smdl/Configuration"
    xmlns:Types="http://www.ecss.nl/smp/2025/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="Later" Name="Later">
  <Component Path="Late">
    <FieldValue xsi:type="Types:Int32Value" Value="4" Field="integer1"/>
  </Component>
</Configuration:Configuration>)");
  const auto lateAssembly =
      WriteContractAssembly(directory, "late.smpasb", "Template");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  simulator.LoadAssembly(outer.string().c_str(), "", "", "");
  simulator.LoadConfiguration(laterConfiguration.string().c_str(),
                              "/Outer/Inner");
  simulator.LoadAssembly(lateAssembly.string().c_str(), "/Outer/Inner",
                         "subModels", "Late");

  auto *field = ResolveIntField(simulator, "/Outer/Inner/Late");
  ASSERT_NE(field, nullptr);
  EXPECT_EQ(static_cast<::Smp::Int32>(field->GetValue()), 4);
}

TEST(SimulatorL2Contract, FieldValueCannotEscapeItsModel) {
  TemporaryContractDirectory directory;
  const auto assembly = directory.Write(
      "field-scope.smpasb", R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="FieldScope" Name="FieldScope">
  <Model Name="Root" Implementation="Xsmp::Tests::ModelWithSimpleFields">
    <Model Name="Left" Container="subModels"
           Implementation="Xsmp::Tests::ModelWithSimpleFields">
      <FieldValue xsi:type="Types:Int32Value" Value="91"
                  Field="../Right.integer1"/>
    </Model>
    <Model Name="Right" Container="subModels"
           Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
  </Model>
</Assembly:Assembly>)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  EXPECT_THROW(simulator.LoadAssembly(assembly.string().c_str(), "", "", ""),
               ::Smp::InvalidFile);

  auto *field = ResolveIntField(simulator, "/Root/Right");
  ASSERT_NE(field, nullptr);
  EXPECT_NE(static_cast<::Smp::Int32>(field->GetValue()), 91);
}

TEST(SimulatorL2Contract, ModelLocalLinkCannotReachSiblingModel) {
  TemporaryContractDirectory directory;
  const auto assembly = directory.Write(
      "link-scope.smpasb", R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:LinkBase="http://www.ecss.nl/smp/2025/Smdl/LinkBase"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="LinkScope" Name="LinkScope">
  <Model Name="Root" Implementation="Xsmp::Tests::ModelWithSimpleFields">
    <Model Name="Left" Container="subModels"
           Implementation="Xsmp::Tests::ModelWithSimpleFields">
      <Link xsi:type="LinkBase:FieldLink">
        <OwnerPath>/Root/Right.integer1Output</OwnerPath>
        <ClientPath>/Root/Left.integer1Input</ClientPath>
      </Link>
    </Model>
    <Model Name="Right" Container="subModels"
           Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
  </Model>
</Assembly:Assembly>)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  EXPECT_THROW(simulator.LoadAssembly(assembly.string().c_str(), "", "", ""),
               ::Smp::InvalidFile);
}

TEST(SimulatorL2Contract, ModelLocalLinkResolvesAbsolutePathsFromSimulator) {
  TemporaryContractDirectory directory;
  const auto assembly = directory.Write(
      "absolute-link.smpasb", R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:LinkBase="http://www.ecss.nl/smp/2025/Smdl/LinkBase"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="AbsoluteLink" Name="AbsoluteLink">
  <Model Name="Root" Implementation="Xsmp::Tests::ModelWithSimpleFields">
    <Model Name="Left" Container="subModels"
           Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
    <Model Name="Right" Container="subModels"
           Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
    <Link xsi:type="LinkBase:FieldLink">
      <OwnerPath>/Root/Left.integer1Output</OwnerPath>
      <ClientPath>/Root/Right.integer1Input</ClientPath>
    </Link>
  </Model>
</Assembly:Assembly>)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  ASSERT_NO_THROW(
      simulator.LoadAssembly(assembly.string().c_str(), "", "", ""));

  auto *source = dynamic_cast<::Smp::IOutputField *>(
      ::Xsmp::Helper::Resolve(&simulator, "/Root/Left.integer1Output"));
  auto *target = dynamic_cast<::Smp::IField *>(
      ::Xsmp::Helper::Resolve(&simulator, "/Root/Right.integer1Input"));
  ASSERT_NE(source, nullptr);
  ASSERT_NE(target, nullptr);
  ASSERT_NE(source->GetInputFields(), nullptr);
  ASSERT_EQ(source->GetInputFields()->size(), 1U);
  EXPECT_EQ(source->GetInputFields()->at(std::size_t{0}), target);
}

TEST(SimulatorL2Contract, ModelLocalLinkResolvesRelativePathsFromCurrentModel) {
  TemporaryContractDirectory directory;
  const auto assembly = directory.Write(
      "relative-link.smpasb", R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:LinkBase="http://www.ecss.nl/smp/2025/Smdl/LinkBase"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="RelativeParentLink" Name="RelativeParentLink">
  <Model Name="Root" Implementation="Xsmp::Tests::ModelWithSimpleFields">
    <Model Name="Left" Container="subModels"
           Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
    <Model Name="Right" Container="subModels"
           Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
    <Link xsi:type="LinkBase:FieldLink">
      <OwnerPath>Left.integer1Output</OwnerPath>
      <ClientPath>Right.integer1Input</ClientPath>
    </Link>
  </Model>
</Assembly:Assembly>)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  ASSERT_NO_THROW(
      simulator.LoadAssembly(assembly.string().c_str(), "", "", ""));

  auto *source = dynamic_cast<::Smp::IOutputField *>(
      ::Xsmp::Helper::Resolve(&simulator, "/Root/Left.integer1Output"));
  auto *target = dynamic_cast<::Smp::IField *>(
      ::Xsmp::Helper::Resolve(&simulator, "/Root/Right.integer1Input"));
  ASSERT_NE(source, nullptr);
  ASSERT_NE(target, nullptr);
  ASSERT_NE(source->GetInputFields(), nullptr);
  ASSERT_EQ(source->GetInputFields()->size(), 1U);
  EXPECT_EQ(source->GetInputFields()->at(std::size_t{0}), target);
}

TEST(SimulatorL2Contract, ExecuteTaskWithoutRootKeepsSimulatorRoot) {
  TemporaryContractDirectory directory;
  auto xml = ScheduleHeader("NoRoot");
  xml += R"(  <Task Id="NoRoot.Sub" Name="Sub">
    <Activity Id="NoRoot.Sub.Hit" Name="Hit" xsi:type="Schedule:Trigger">
      <EntryPoint>/Probe.hit</EntryPoint>
    </Activity>
  </Task>
  <Task Id="NoRoot.Main" Name="Main">
    <Activity Id="NoRoot.Main.Sub" Name="Sub" xsi:type="Schedule:ExecuteTask">
      <Task xlink:href="#NoRoot.Sub"/>
    </Activity>
  </Task>
  <Event Id="NoRoot.Start" Name="Start"
         xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#NoRoot.Main"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("no-root.smpsed", xml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  auto *probe = new TaskProbe{"Probe", &simulator, &simulator};
  simulator.AddModel(probe);
  simulator.Connect();
  simulator.LoadSchedule(schedule.string().c_str());
  simulator.Run(1);

  EXPECT_EQ(probe->calls, 1);
}

TEST(SimulatorL2Contract, RejectsCycleInUnusedLocalTask) {
  TemporaryContractDirectory directory;
  auto xml = ScheduleHeader("UnusedCycle");
  xml += R"(  <Task Id="UnusedCycle.Main" Name="Main"/>
  <Task Id="UnusedCycle.Dead" Name="Dead">
    <Activity Id="UnusedCycle.Dead.Self" Name="Self"
              xsi:type="Schedule:ExecuteTask">
      <Task xlink:href="#UnusedCycle.Dead"/>
    </Activity>
  </Task>
  <Event Id="UnusedCycle.Start" Name="Start"
         xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#UnusedCycle.Main"/>
  </Event>
</Schedule:Schedule>)";
  const auto schedule = directory.Write("unused-cycle.smpsed", xml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.Connect();
  EXPECT_THROW(simulator.LoadSchedule(schedule.string().c_str()),
               ::Smp::InvalidFile);
}

TEST(SimulatorL2Contract, DistinguishesRootAndInternalDuplicateNames) {
  TemporaryContractDirectory directory;
  const auto simple =
      WriteContractAssembly(directory, "duplicate-root.smpasb", "Root");
  const auto internal = directory.Write(
      "duplicate-child.smpasb", R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="DuplicateChild" Name="DuplicateChild">
  <Model Name="Parent" Implementation="Xsmp::Tests::ModelWithSimpleFields">
    <Model Name="Child" Container="subModels"
           Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
    <Model Name="Child" Container="subModels"
           Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
  </Model>
</Assembly:Assembly>)");

  ::Xsmp::Simulator rootSimulator;
  rootSimulator.LoadLibrary("xsmp_services");
  rootSimulator.LoadLibrary("xsmp_tests");
  rootSimulator.LoadAssembly(simple.string().c_str(), "", "", "");
  EXPECT_THROW(rootSimulator.LoadAssembly(simple.string().c_str(), "", "", ""),
               ::Smp::DuplicateName);

  ::Xsmp::Simulator internalSimulator;
  internalSimulator.LoadLibrary("xsmp_services");
  internalSimulator.LoadLibrary("xsmp_tests");
  EXPECT_THROW(
      internalSimulator.LoadAssembly(internal.string().c_str(), "", "", ""),
      ::Smp::InvalidFile);
}

TEST(SimulatorL2Contract, ReconnectWithoutRootConnectsNewTopLevelModel) {
  TemporaryContractDirectory directory;
  const auto assembly =
      WriteContractAssembly(directory, "late-root.smpasb", "Late");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  simulator.Connect();
  simulator.LoadAssembly(assembly.string().c_str(), "", "", "");
  auto *late = dynamic_cast<::Smp::IModel *>(
      ::Xsmp::Helper::Resolve(&simulator, "/Late"));
  ASSERT_NE(late, nullptr);
  ASSERT_EQ(late->GetState(), ::Smp::ComponentStateKind::CSK_Created);

  simulator.Reconnect(nullptr);

  EXPECT_EQ(late->GetState(), ::Smp::ComponentStateKind::CSK_Connected);
}

TEST(SimulatorL2Contract, ReconnectIncludesANonCompositeRoot) {
  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.Connect();
  auto *late = new ::Xsmp::Model{"Late", "", &simulator, &simulator};
  simulator.AddModel(late);
  ASSERT_EQ(late->GetState(), ::Smp::ComponentStateKind::CSK_Created);

  simulator.Reconnect(late);

  EXPECT_EQ(late->GetState(), ::Smp::ComponentStateKind::CSK_Connected);
}

} // namespace
} // namespace Xsmp::L2
