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

#include <Smp/FileNotFound.h>
#include <Smp/IModel.h>
#include <Smp/IPublication.h>
#include <Smp/IRequest.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimpleField.h>
#include <Smp/InvalidFile.h>
#include <Smp/InvalidObjectName.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Publication/ParameterDirectionKind.h>
#include <Smp/Uuid.h>
#include <Smp/ViewKind.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Model.h>
#include <Xsmp/Simulator.h>
#include <Xsmp/Tests/ModelWithSimpleFields.h>

#include <gtest/gtest.h>

#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

namespace Xsmp::L2 {
namespace {

class TemporarySemanticDirectory final {
public:
  TemporarySemanticDirectory() {
    const auto suffix = std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count());
    path = std::filesystem::temp_directory_path() /
           ("xsmp-simulator-l2-semantic-" + suffix);
    std::filesystem::create_directories(path);
  }

  ~TemporarySemanticDirectory() {
    std::error_code error;
    std::filesystem::remove_all(path, error);
  }

  TemporarySemanticDirectory(const TemporarySemanticDirectory &) = delete;
  TemporarySemanticDirectory &
  operator=(const TemporarySemanticDirectory &) = delete;

  std::filesystem::path Write(std::string_view name,
                              std::string_view contents) const {
    const auto result = path / name;
    std::ofstream stream{result, std::ios::binary};
    stream.write(contents.data(),
                 static_cast<std::streamsize>(contents.size()));
    stream.close();
    return result;
  }

  std::filesystem::path path;
};

std::filesystem::path
WriteSimpleAssembly(const TemporarySemanticDirectory &directory,
                    std::string_view fileName, std::string_view modelName) {
  std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="SimpleAssembly" Name="SimpleAssembly">
  <Model Name=")";
  xml += modelName;
  xml += R"(" Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
</Assembly:Assembly>)";
  return directory.Write(fileName, xml);
}

std::filesystem::path WriteFieldValueAssembly(
    const TemporarySemanticDirectory &directory, std::string_view fileName,
    std::string_view implementation, std::string_view fieldValues,
    std::string_view modelName = "Configured") {
  std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="FieldValueAssembly" Name="FieldValueAssembly">
  <Model Name=")";
  xml += modelName;
  xml += "\" Implementation=\"";
  xml += implementation;
  xml += "\">\n";
  xml += fieldValues;
  xml += R"(  </Model>
</Assembly:Assembly>)";
  return directory.Write(fileName, xml);
}

std::filesystem::path
WriteAssemblyInstance(const TemporarySemanticDirectory &directory,
                      std::string_view fileName, std::string_view assemblyFile,
                      std::string_view arguments,
                      std::string_view extraFiles = {}) {
  std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="AssemblyInstanceParent" Name="AssemblyInstanceParent">
  <Model Name="Parent" Implementation="Xsmp::Tests::ModelWithSimpleFields">
    <Assembly Name="Nested" Container="subModels">
      <Assembly>)";
  xml += assemblyFile;
  xml += "</Assembly>\n";
  xml += arguments;
  xml += extraFiles;
  xml += R"(    </Assembly>
  </Model>
</Assembly:Assembly>)";
  return directory.Write(fileName, xml);
}

std::string ConfigurationHeader(std::string_view id) {
  return std::string{R"(<?xml version="1.0" encoding="UTF-8"?>
<Configuration:Configuration
    xmlns:Configuration="http://www.ecss.nl/smp/2025/Smdl/Configuration"
    xmlns:Types="http://www.ecss.nl/smp/2025/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:xlink="http://www.w3.org/1999/xlink"
    Id=")"} +
         std::string{id} + "\" Name=\"" + std::string{id} + "\">\n";
}

class OperationProbeModel final : public ::Xsmp::Model {
public:
  OperationProbeModel(::Smp::String8 name, ::Smp::IComposite *parent,
                      ::Smp::ISimulator *simulator)
      : ::Xsmp::Model{name, "", parent, simulator} {}

  void Publish(::Smp::IPublication *receiver) override {
    ::Xsmp::Model::Publish(receiver);
    auto *operation =
        receiver->PublishOperation("combine", "", ::Smp::ViewKind::VK_All);
    operation->PublishParameter(
        "first", "", ::Smp::Uuids::Uuid_Int32,
        ::Smp::Publication::ParameterDirectionKind::PDK_In);
    operation->PublishParameter(
        "second", "", ::Smp::Uuids::Uuid_Int32,
        ::Smp::Publication::ParameterDirectionKind::PDK_In);
    operation->PublishParameter(
        "inout", "", ::Smp::Uuids::Uuid_Int32,
        ::Smp::Publication::ParameterDirectionKind::PDK_InOut);
    operation->PublishParameter(
        "output", "", ::Smp::Uuids::Uuid_Int32,
        ::Smp::Publication::ParameterDirectionKind::PDK_Out);
    operation->PublishParameter(
        "result", "", ::Smp::Uuids::Uuid_Int32,
        ::Smp::Publication::ParameterDirectionKind::PDK_Return);
  }

  void Invoke(::Smp::IRequest *request) override {
    if (!request || std::string_view{request->GetName()} != "combine") {
      ::Xsmp::Model::Invoke(request);
    }
    ++callCount;
    const auto read = [request](::Smp::String8 name) {
      const auto value =
          request->GetParameterValue(request->GetParameterIndex(name));
      return value.GetType() == ::Smp::PrimitiveTypeKind::PTK_Int32
                 ? static_cast<::Smp::Int32>(value)
                 : ::Smp::Int32{};
    };
    first = read("first");
    second = read("second");
    inout = read("inout");
    request->SetParameterValue(
        request->GetParameterIndex("output"),
        {::Smp::PrimitiveTypeKind::PTK_Int32, first + second});
    request->SetParameterValue(
        request->GetParameterIndex("inout"),
        {::Smp::PrimitiveTypeKind::PTK_Int32, inout + 1});
    request->SetReturnValue(
        {::Smp::PrimitiveTypeKind::PTK_Int32, first + second + inout});
  }

  int callCount{};
  ::Smp::Int32 first{};
  ::Smp::Int32 second{};
  ::Smp::Int32 inout{};
};

class PrimitiveFloatProbeModel final : public ::Xsmp::Model {
public:
  PrimitiveFloatProbeModel(::Smp::String8 name, ::Smp::IComposite *parent,
                           ::Smp::ISimulator *simulator)
      : ::Xsmp::Model{name, "", parent, simulator} {}

  void Publish(::Smp::IPublication *receiver) override {
    ::Xsmp::Model::Publish(receiver);
    receiver->PublishField("positiveInfinity", "", &positiveInfinity,
                           ::Smp::Uuids::Uuid_Float64, ::Smp::ViewKind::VK_All,
                           true, false, false);
    receiver->PublishField("negativeInfinity", "", &negativeInfinity,
                           ::Smp::Uuids::Uuid_Float64, ::Smp::ViewKind::VK_All,
                           true, false, false);
    receiver->PublishField("notANumber", "", &notANumber,
                           ::Smp::Uuids::Uuid_Float64, ::Smp::ViewKind::VK_All,
                           true, false, false);
  }

  ::Smp::Float64 positiveInfinity{};
  ::Smp::Float64 negativeInfinity{};
  ::Smp::Float64 notANumber{};
};

OperationProbeModel *AddOperationProbe(::Xsmp::Simulator &simulator) {
  auto *probe = new OperationProbeModel{"Probe", &simulator, &simulator};
  simulator.AddModel(probe);
  return probe;
}

OperationProbeModel *AddConnectedOperationProbe(::Xsmp::Simulator &simulator) {
  simulator.LoadLibrary("xsmp_services");
  auto *probe = AddOperationProbe(simulator);
  simulator.Connect();
  return probe;
}

std::filesystem::path
WriteCallOperationSchedule(const TemporarySemanticDirectory &directory,
                           std::string_view fileName,
                           std::string_view parameters) {
  std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Schedule:Schedule
    xmlns:Schedule="http://www.ecss.nl/smp/2025/Smdl/Schedule"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:xlink="http://www.w3.org/1999/xlink"
    Id="CallOperationSchedule" Name="CallOperationSchedule">
  <Task Id="CallOperationSchedule.Main" Name="Main">
    <Activity Id="CallOperationSchedule.Main.Call" Name="Call"
              xsi:type="Schedule:CallOperation">
      <OperationPath>/Probe.combine</OperationPath>
)";
  xml += parameters;
  xml += R"(    </Activity>
  </Task>
  <Event Id="CallOperationSchedule.Start" Name="Start"
         xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#CallOperationSchedule.Main"/>
  </Event>
</Schedule:Schedule>)";
  return directory.Write(fileName, xml);
}

std::filesystem::path
WriteOperationCallAssembly(const TemporarySemanticDirectory &directory,
                           std::string_view fileName,
                           std::string_view parameters) {
  std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="OperationCallAssembly" Name="OperationCallAssembly">
  <ComponentConfiguration InstancePath="/Probe">
    <Invocation xsi:type="Assembly:OperationCall" Operation="combine">
)";
  xml += parameters;
  xml += R"(    </Invocation>
  </ComponentConfiguration>
  <Model Name="OperationCallRoot"
         Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
</Assembly:Assembly>)";
  return directory.Write(fileName, xml);
}

std::string OperationParameter(std::string_view name, std::string_view type,
                               std::string_view value) {
  return "      <Parameter Parameter=\"" + std::string{name} +
         "\">\n        <Value xsi:type=\"Types:" + std::string{type} +
         "\" Value=\"" + std::string{value} + "\"/>\n      </Parameter>\n";
}

TEST(SimulatorL2, SubstitutesInt32ArgumentBeforeSchemaValidation) {
  TemporarySemanticDirectory directory;
  const auto assembly = directory.Write(
      "templated-int32.smpasb", R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="TemplatedInt32" Name="TemplatedInt32">
  <Parameter Name="InitialValue" xsi:type="Assembly:Int32Argument" Value="42"/>
  <Model Name="Templated" Implementation="Xsmp::Tests::ModelWithSimpleFields">
    <FieldValue xsi:type="Types:Int32Value" Value="{InitialValue}" Field="integer1"/>
  </Model>
</Assembly:Assembly>)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");

  EXPECT_NO_THROW(
      simulator.LoadAssembly(assembly.string().c_str(), "", "", ""));
  auto *model = dynamic_cast<::Smp::IModel *>(
      ::Xsmp::Helper::Resolve(&simulator, "/Templated"));
  ASSERT_NE(model, nullptr);
  auto *field =
      dynamic_cast<::Smp::ISimpleField *>(model->GetField("integer1"));
  ASSERT_NE(field, nullptr);
  EXPECT_EQ(static_cast<::Smp::Int32>(field->GetValue()), 42);
}

TEST(SimulatorL2, ParsesCollapsedSimpleLexemesAndPreservesStrings) {
  TemporarySemanticDirectory directory;
  const auto assembly = WriteFieldValueAssembly(
      directory, "simple-lexemes.smpasb", "Xsmp::Tests::ModelWithSimpleFields",
      R"(    <FieldValue xsi:type="Types:BoolValue" Value="&#x9;true&#xA;" Field="boolean"/>
    <FieldValue xsi:type="Types:Char8Value" Value=" " Field="char8"/>
    <FieldValue xsi:type="Types:String8Value" Value="  text  " Field="string1"/>
    <FieldValue xsi:type="Types:Int32Value" Value="&#xD;+42&#x9;" Field="integer1"/>
)",
      "Lexical");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  ASSERT_NO_THROW(
      simulator.LoadAssembly(assembly.string().c_str(), "", "", ""));

  auto *model = dynamic_cast<::Smp::IModel *>(
      ::Xsmp::Helper::Resolve(&simulator, "/Lexical"));
  ASSERT_NE(model, nullptr);
  auto *boolean =
      dynamic_cast<::Smp::ISimpleField *>(model->GetField("boolean"));
  auto *character =
      dynamic_cast<::Smp::ISimpleField *>(model->GetField("char8"));
  auto *string =
      dynamic_cast<::Smp::ISimpleField *>(model->GetField("string1"));
  auto *integer =
      dynamic_cast<::Smp::ISimpleField *>(model->GetField("integer1"));
  ASSERT_NE(boolean, nullptr);
  ASSERT_NE(character, nullptr);
  ASSERT_NE(string, nullptr);
  ASSERT_NE(integer, nullptr);
  EXPECT_TRUE(static_cast<::Smp::Bool>(boolean->GetValue()));
  EXPECT_EQ(static_cast<::Smp::Char8>(character->GetValue()), ' ');
  EXPECT_STREQ(static_cast<::Smp::String8>(string->GetValue()), "  text  ");
  EXPECT_EQ(static_cast<::Smp::Int32>(integer->GetValue()), 42);
}

TEST(SimulatorL2, ParsesNonFinitePrimitiveFloatingPointLexemes) {
  TemporarySemanticDirectory directory;
  auto configuration = ConfigurationHeader("NonFiniteValues");
  configuration += R"(  <Component Path="/Floats">
    <FieldValue xsi:type="Types:Float64Value" Value="INF" Field="positiveInfinity"/>
    <FieldValue xsi:type="Types:Float64Value" Value="-INF" Field="negativeInfinity"/>
    <FieldValue xsi:type="Types:Float64Value" Value="NaN" Field="notANumber"/>
  </Component>
</Configuration:Configuration>)";
  const auto path = directory.Write("non-finite.smpcfg", configuration);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  auto *probe = new PrimitiveFloatProbeModel{"Floats", &simulator, &simulator};
  simulator.AddModel(probe);
  simulator.Connect();

  ASSERT_NO_THROW(simulator.LoadConfiguration(path.string().c_str(), ""));
  EXPECT_TRUE(std::isinf(probe->positiveInfinity));
  EXPECT_GT(probe->positiveInfinity, 0.0);
  EXPECT_TRUE(std::isinf(probe->negativeInfinity));
  EXPECT_LT(probe->negativeInfinity, 0.0);
  EXPECT_TRUE(std::isnan(probe->notANumber));
}

TEST(SimulatorL2, RejectsNonFiniteValueForConstrainedFloatType) {
  TemporarySemanticDirectory directory;
  const auto assembly = WriteFieldValueAssembly(
      directory, "constrained-nan.smpasb", "Xsmp::Tests::ModelWithSimpleFields",
      R"(    <FieldValue xsi:type="Types:Float64Value" Value="NaN" Field="float1"/>
)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  EXPECT_THROW(simulator.LoadAssembly(assembly.string().c_str(), "", "", ""),
               ::Smp::InvalidFile);
}

TEST(SimulatorL2, RejectsValueOutsidePublishedStringConstraints) {
  TemporarySemanticDirectory directory;
  const auto assembly = WriteFieldValueAssembly(
      directory, "long-string.smpasb", "Xsmp::Tests::ModelWithSimpleFields",
      R"(    <FieldValue xsi:type="Types:String8Value" Value="12345678901" Field="string1"/>
)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  EXPECT_THROW(simulator.LoadAssembly(assembly.string().c_str(), "", "", ""),
               ::Smp::InvalidFile);
}

TEST(SimulatorL2, AppliesImplicitlyTypedSimpleArrayItems) {
  TemporarySemanticDirectory directory;
  const auto assembly = WriteFieldValueAssembly(
      directory, "simple-array.smpasb",
      "Xsmp::Tests::ModelWithSimpleArrayFields",
      R"(    <FieldValue xsi:type="Types:Int32ArrayValue" Field="integer1">
      <ItemValue Value="+1"/>
      <ItemValue Value=" 2 "/>
      <ItemValue Value="3"/>
    </FieldValue>
)",
      "Arrays");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  ASSERT_NO_THROW(
      simulator.LoadAssembly(assembly.string().c_str(), "", "", ""));

  auto *model = dynamic_cast<::Smp::IModel *>(
      ::Xsmp::Helper::Resolve(&simulator, "/Arrays"));
  ASSERT_NE(model, nullptr);
  auto *field =
      dynamic_cast<::Smp::ISimpleArrayField *>(model->GetField("integer1"));
  ASSERT_NE(field, nullptr);
  ASSERT_EQ(field->GetSize(), 3U);
  EXPECT_EQ(static_cast<::Smp::Int32>(field->GetValue(0)), 1);
  EXPECT_EQ(static_cast<::Smp::Int32>(field->GetValue(1)), 2);
  EXPECT_EQ(static_cast<::Smp::Int32>(field->GetValue(2)), 3);
}

TEST(SimulatorL2, RejectsSimpleArrayEnvelopeIncompatibleWithPublishedItems) {
  TemporarySemanticDirectory directory;
  const auto assembly = WriteFieldValueAssembly(
      directory, "simple-array-type.smpasb",
      "Xsmp::Tests::ModelWithSimpleArrayFields",
      R"(    <FieldValue xsi:type="Types:BoolArrayValue" Field="integer1">
      <ItemValue Value="true"/>
    </FieldValue>
)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  EXPECT_THROW(simulator.LoadAssembly(assembly.string().c_str(), "", "", ""),
               ::Smp::InvalidFile);
}

TEST(SimulatorL2, RejectsUnescapedClosingTemplateBrace) {
  TemporarySemanticDirectory directory;
  const auto assembly = directory.Write(
      "closing-brace.smpasb", R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    Id="ClosingBrace" Name="ClosingBrace">
  <Description>unescaped }</Description>
</Assembly:Assembly>)");

  ::Xsmp::Simulator simulator;
  EXPECT_THROW(simulator.LoadAssembly(assembly.string().c_str(), "", "", ""),
               ::Smp::InvalidFile);
}

TEST(SimulatorL2, RejectsDuplicateTemplateParameters) {
  TemporarySemanticDirectory directory;
  const auto assembly =
      directory.Write("duplicate-parameters.smpasb",
                      R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="DuplicateParameters" Name="DuplicateParameters">
  <Parameter Name="Number" xsi:type="Assembly:Int32Argument" Value="1"/>
  <Parameter Name="Number" xsi:type="Assembly:Int32Argument" Value="2"/>
  <Model Name="Root" Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
</Assembly:Assembly>)");

  ::Xsmp::Simulator simulator;
  EXPECT_THROW(simulator.LoadAssembly(assembly.string().c_str(), "", "", ""),
               ::Smp::InvalidFile);
}

TEST(SimulatorL2, ValidatesNestedTemplateArguments) {
  TemporarySemanticDirectory directory;
  directory.Write("parameterized.smpasb",
                  R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="Parameterized" Name="Parameterized">
  <Parameter Name="Number" xsi:type="Assembly:Int32Argument" Value="1"/>
  <Model Name="Child" Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
</Assembly:Assembly>)");
  const auto missing = WriteAssemblyInstance(
      directory, "missing-argument-value.smpasb", "parameterized.smpasb",
      R"(      <Argument Name="Number" xsi:type="Assembly:Int32Argument"/>
)");
  const auto duplicate = WriteAssemblyInstance(
      directory, "duplicate-arguments.smpasb", "parameterized.smpasb",
      R"(      <Argument Name="Number" xsi:type="Assembly:Int32Argument" Value="2"/>
      <Argument Name="Number" xsi:type="Assembly:Int32Argument" Value="3"/>
)");
  const auto incompatible = WriteAssemblyInstance(
      directory, "incompatible-argument.smpasb", "parameterized.smpasb",
      R"(      <Argument Name="Number" xsi:type="Assembly:StringArgument" Value="text"/>
)");

  for (const auto &path : {missing, duplicate, incompatible}) {
    ::Xsmp::Simulator simulator;
    simulator.LoadLibrary("xsmp_services");
    simulator.LoadLibrary("xsmp_tests");
    EXPECT_THROW(simulator.LoadAssembly(path.string().c_str(), "", "", ""),
                 ::Smp::InvalidFile)
        << path;
  }
}

TEST(SimulatorL2, RequiresCanonicalLevel2FileExtensions) {
  TemporarySemanticDirectory directory;
  const auto assembly = WriteSimpleAssembly(directory, "assembly.xml", "Root");
  const auto linkBase =
      directory.Write("links.xml", R"(<?xml version="1.0" encoding="UTF-8"?>
<LinkBase:LinkBase
    xmlns:LinkBase="http://www.ecss.nl/smp/2025/Smdl/LinkBase"
    Id="Links" Name="Links"/>)");
  const auto schedule =
      directory.Write("schedule.xml", R"(<?xml version="1.0" encoding="UTF-8"?>
<Schedule:Schedule
    xmlns:Schedule="http://www.ecss.nl/smp/2025/Smdl/Schedule"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:xlink="http://www.w3.org/1999/xlink"
    Id="Schedule" Name="Schedule">
  <Task Id="Schedule.Task" Name="Task"/>
  <Event Id="Schedule.Event" Name="Event"
         xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#Schedule.Task"/>
  </Event>
</Schedule:Schedule>)");
  auto configurationXml = ConfigurationHeader("Configuration");
  configurationXml += "</Configuration:Configuration>";
  const auto configuration =
      directory.Write("configuration.xml", configurationXml);

  {
    ::Xsmp::Simulator simulator;
    EXPECT_THROW(simulator.LoadAssembly(assembly.string().c_str(), "", "", ""),
                 ::Smp::InvalidFile);
  }
  {
    ::Xsmp::Simulator simulator;
    EXPECT_THROW(simulator.LoadLinkBase(linkBase.string().c_str(), ""),
                 ::Smp::InvalidFile);
  }
  {
    ::Xsmp::Simulator simulator;
    EXPECT_THROW(simulator.LoadSchedule(schedule.string().c_str()),
                 ::Smp::InvalidFile);
  }
  {
    ::Xsmp::Simulator simulator;
    EXPECT_THROW(
        simulator.LoadConfiguration(configuration.string().c_str(), ""),
        ::Smp::InvalidFile);
  }

  const auto missing = directory.path / "missing.xml";
  ::Xsmp::Simulator simulator;
  EXPECT_THROW(simulator.LoadAssembly(missing.string().c_str(), "", "", ""),
               ::Smp::FileNotFound);
}

TEST(SimulatorL2, RequiresCanonicalExtensionsForNestedReferences) {
  TemporarySemanticDirectory directory;
  WriteSimpleAssembly(directory, "child.xml", "Child");
  const auto parent =
      WriteAssemblyInstance(directory, "parent.smpasb", "child.xml", {});

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  EXPECT_THROW(simulator.LoadAssembly(parent.string().c_str(), "", "", ""),
               ::Smp::InvalidFile);
}

TEST(SimulatorL2, RequiresCanonicalExtensionsForNestedConfigurationAndLinks) {
  TemporarySemanticDirectory directory;
  WriteSimpleAssembly(directory, "child.smpasb", "Child");
  auto configurationXml = ConfigurationHeader("NestedConfiguration");
  configurationXml += "</Configuration:Configuration>";
  directory.Write("configuration.xml", configurationXml);
  directory.Write("links.xml", R"(<?xml version="1.0" encoding="UTF-8"?>
<LinkBase:LinkBase
    xmlns:LinkBase="http://www.ecss.nl/smp/2025/Smdl/LinkBase"
    Id="NestedLinks" Name="NestedLinks"/>)");
  const auto configurationParent = WriteAssemblyInstance(
      directory, "configuration-parent.smpasb", "child.smpasb", {},
      "      <Configuration>configuration.xml</Configuration>\n");
  const auto linkParent =
      WriteAssemblyInstance(directory, "link-parent.smpasb", "child.smpasb", {},
                            "      <LinkBase>links.xml</LinkBase>\n");

  for (const auto &path : {configurationParent, linkParent}) {
    ::Xsmp::Simulator simulator;
    simulator.LoadLibrary("xsmp_services");
    simulator.LoadLibrary("xsmp_tests");
    EXPECT_THROW(simulator.LoadAssembly(path.string().c_str(), "", "", ""),
                 ::Smp::InvalidFile)
        << path;
  }
}

TEST(SimulatorL2, RequiresCanonicalExtensionForExternalTaskReference) {
  TemporarySemanticDirectory directory;
  directory.Write("external.xml", R"(<?xml version="1.0" encoding="UTF-8"?>
<Schedule:Schedule
    xmlns:Schedule="http://www.ecss.nl/smp/2025/Smdl/Schedule"
    Id="External" Name="External">
  <Task Id="External.Task" Name="Task"/>
</Schedule:Schedule>)");
  const auto schedule =
      directory.Write("external-reference.smpsed",
                      R"(<?xml version="1.0" encoding="UTF-8"?>
<Schedule:Schedule
    xmlns:Schedule="http://www.ecss.nl/smp/2025/Smdl/Schedule"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:xlink="http://www.w3.org/1999/xlink"
    Id="ExternalReference" Name="ExternalReference">
  <Task Id="ExternalReference.Task" Name="Task">
    <Activity Id="ExternalReference.Execute" Name="Execute"
              xsi:type="Schedule:ExecuteTask">
      <Task xlink:href="external.xml#External.Task"/>
    </Activity>
  </Task>
  <Event Id="ExternalReference.Event" Name="Event"
         xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#ExternalReference.Task"/>
  </Event>
  </Schedule:Schedule>)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  EXPECT_THROW(simulator.LoadSchedule(schedule.string().c_str()),
               ::Smp::InvalidFile);
}

TEST(SimulatorL2, RejectsDuplicateTaskActivityAndEventNames) {
  TemporarySemanticDirectory directory;
  const auto duplicateTasks = directory.Write(
      "duplicate-tasks.smpsed", R"(<?xml version="1.0" encoding="UTF-8"?>
<Schedule:Schedule
    xmlns:Schedule="http://www.ecss.nl/smp/2025/Smdl/Schedule"
    Id="DuplicateTasks" Name="DuplicateTasks">
  <Task Id="DuplicateTasks.First" Name="Same"/>
  <Task Id="DuplicateTasks.Second" Name="Same"/>
</Schedule:Schedule>)");
  const auto duplicateActivities = directory.Write(
      "duplicate-activities.smpsed", R"(<?xml version="1.0" encoding="UTF-8"?>
<Schedule:Schedule
    xmlns:Schedule="http://www.ecss.nl/smp/2025/Smdl/Schedule"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="DuplicateActivities" Name="DuplicateActivities">
  <Task Id="DuplicateActivities.Task" Name="Task">
    <Activity Id="DuplicateActivities.First" Name="Same"
              xsi:type="Schedule:EmitGlobalEvent"><EventName>First</EventName></Activity>
    <Activity Id="DuplicateActivities.Second" Name="Same"
              xsi:type="Schedule:EmitGlobalEvent"><EventName>Second</EventName></Activity>
  </Task>
</Schedule:Schedule>)");
  const auto duplicateEvents = directory.Write(
      "duplicate-events.smpsed", R"(<?xml version="1.0" encoding="UTF-8"?>
<Schedule:Schedule
    xmlns:Schedule="http://www.ecss.nl/smp/2025/Smdl/Schedule"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:xlink="http://www.w3.org/1999/xlink"
    Id="DuplicateEvents" Name="DuplicateEvents">
  <Task Id="DuplicateEvents.Task" Name="Task"/>
  <Event Id="DuplicateEvents.First" Name="Same"
         xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#DuplicateEvents.Task"/>
  </Event>
  <Event Id="DuplicateEvents.Second" Name="Same"
         xsi:type="Schedule:SimulationEvent" SimulationTime="PT1S">
    <Task xlink:href="#DuplicateEvents.Task"/>
  </Event>
</Schedule:Schedule>)");

  for (const auto &path :
       {duplicateTasks, duplicateActivities, duplicateEvents}) {
    ::Xsmp::Simulator simulator;
    EXPECT_THROW(simulator.LoadSchedule(path.string().c_str()),
                 ::Smp::InvalidFile)
        << path;
  }
}

TEST(SimulatorL2, SetPropertyRejectsReadOnlyTargetsDuringCompilation) {
  TemporarySemanticDirectory directory;
  const auto assembly = directory.Write(
      "properties.smpasb", R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    Id="Properties" Name="Properties">
  <Model Name="Properties" Implementation="Xsmp::Tests::ModelWithProperties"/>
</Assembly:Assembly>)");
  const auto schedule = directory.Write(
      "readonly-property.smpsed", R"(<?xml version="1.0" encoding="UTF-8"?>
<Schedule:Schedule
    xmlns:Schedule="http://www.ecss.nl/smp/2025/Smdl/Schedule"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:xlink="http://www.w3.org/1999/xlink"
    Id="ReadOnlyProperty" Name="ReadOnlyProperty">
  <Task Id="ReadOnlyProperty.Task" Name="Task">
    <Activity Id="ReadOnlyProperty.Set" Name="Set"
              xsi:type="Schedule:SetProperty">
      <PropertyPath>/Properties.readonly_int_property</PropertyPath>
      <Value xsi:type="Types:Int32Value" Value="42"/>
    </Activity>
  </Task>
  <Event Id="ReadOnlyProperty.Event" Name="Event"
         xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#ReadOnlyProperty.Task"/>
  </Event>
</Schedule:Schedule>)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  ASSERT_NO_THROW(
      simulator.LoadAssembly(assembly.string().c_str(), "", "", ""));
  EXPECT_THROW(simulator.LoadSchedule(schedule.string().c_str()),
               ::Smp::InvalidFile);
}

TEST(SimulatorL2, InvalidContainerNameRaisesInvalidObjectName) {
  TemporarySemanticDirectory directory;
  const auto assembly =
      WriteSimpleAssembly(directory, "container-name.smpasb", "Original");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  simulator.LoadAssembly(assembly.string().c_str(), "", "", "Parent");

  EXPECT_THROW(simulator.LoadAssembly(assembly.string().c_str(), "/Parent",
                                      "0invalid", "Child"),
               ::Smp::InvalidObjectName);
  EXPECT_EQ(::Xsmp::Helper::Resolve(&simulator, "/Parent/Child"), nullptr);
}

TEST(SimulatorL2, InvalidRootInstanceNameRaisesInvalidObjectName) {
  TemporarySemanticDirectory directory;
  const auto assembly =
      WriteSimpleAssembly(directory, "root-name.smpasb", "Original");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");

  EXPECT_THROW(
      simulator.LoadAssembly(assembly.string().c_str(), "", "", "0invalid"),
      ::Smp::InvalidObjectName);
  EXPECT_EQ(::Xsmp::Helper::Resolve(&simulator, "/Original"), nullptr);
}

TEST(SimulatorL2, LoadsLegacy2019Configuration) {
  TemporarySemanticDirectory directory;
  const auto assembly =
      WriteSimpleAssembly(directory, "legacy-configuration.smpasb", "Legacy");
  const auto configuration = directory.Write(
      "legacy-configuration.smpcfg", R"(<?xml version="1.0" encoding="UTF-8"?>
<Configuration:Configuration
    xmlns:Configuration="http://www.ecss.nl/smp/2019/Smdl/Configuration"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="LegacyConfiguration" Name="LegacyConfiguration">
  <Component Path="/Legacy">
    <FieldValue xsi:type="Types:Int32Value" Value="42" Field="integer1"/>
  </Component>
</Configuration:Configuration>)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  simulator.LoadAssembly(assembly.string().c_str(), "", "", "");

  EXPECT_NO_THROW(
      simulator.LoadConfiguration(configuration.string().c_str(), ""));
  auto *model = dynamic_cast<::Smp::IModel *>(
      ::Xsmp::Helper::Resolve(&simulator, "/Legacy"));
  ASSERT_NE(model, nullptr);
  auto *field =
      dynamic_cast<::Smp::ISimpleField *>(model->GetField("integer1"));
  ASSERT_NE(field, nullptr);
  EXPECT_EQ(static_cast<::Smp::Int32>(field->GetValue()), 42);
}

TEST(SimulatorL2,
     NestedConfigurationIncludesPreserveOverridesAndAbsolutePrefix) {
  TemporarySemanticDirectory directory;
  const auto assembly =
      WriteSimpleAssembly(directory, "target.smpasb", "Target");

  auto leafXml = ConfigurationHeader("Leaf");
  leafXml += R"(  <Component Path="/">
    <FieldValue xsi:type="Types:BoolValue" Value="true" Field="boolean"/>
    <FieldValue xsi:type="Types:Int32Value" Value="11" Field="integer1"/>
  </Component>
</Configuration:Configuration>)";
  directory.Write("leaf.smpcfg", leafXml);

  auto middleXml = ConfigurationHeader("Middle");
  middleXml += R"(  <Include>
    <Configuration xlink:href="leaf.smpcfg#Leaf"/>
  </Include>
  <Component Path="/">
    <FieldValue xsi:type="Types:Int32Value" Value="22" Field="integer1"/>
  </Component>
</Configuration:Configuration>)";
  directory.Write("middle.smpcfg", middleXml);

  auto rootXml = ConfigurationHeader("Root");
  rootXml += R"(  <Include Path="/Target">
    <Configuration xlink:href="middle.smpcfg#Middle"/>
  </Include>
  <Component Path="/Target">
    <FieldValue xsi:type="Types:Int32Value" Value="33" Field="integer1"/>
  </Component>
</Configuration:Configuration>)";
  const auto configuration = directory.Write("root.smpcfg", rootXml);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  simulator.LoadAssembly(assembly.string().c_str(), "", "", "");

  EXPECT_NO_THROW(
      simulator.LoadConfiguration(configuration.string().c_str(), ""));
  auto *model = dynamic_cast<::Smp::IModel *>(
      ::Xsmp::Helper::Resolve(&simulator, "/Target"));
  ASSERT_NE(model, nullptr);
  auto *boolean =
      dynamic_cast<::Smp::ISimpleField *>(model->GetField("boolean"));
  auto *integer =
      dynamic_cast<::Smp::ISimpleField *>(model->GetField("integer1"));
  ASSERT_NE(boolean, nullptr);
  ASSERT_NE(integer, nullptr);
  EXPECT_TRUE(static_cast<::Smp::Bool>(boolean->GetValue()));
  EXPECT_EQ(static_cast<::Smp::Int32>(integer->GetValue()), 33);
}

TEST(SimulatorL2, CyclicConfigurationIncludeIsInvalidFile) {
  TemporarySemanticDirectory directory;
  auto firstXml = ConfigurationHeader("First");
  firstXml += R"(  <Include>
    <Configuration xlink:href="second.smpcfg#Second"/>
  </Include>
</Configuration:Configuration>)";
  const auto first = directory.Write("first.smpcfg", firstXml);

  auto secondXml = ConfigurationHeader("Second");
  secondXml += R"(  <Include>
    <Configuration xlink:href="first.smpcfg#First"/>
  </Include>
</Configuration:Configuration>)";
  directory.Write("second.smpcfg", secondXml);

  ::Xsmp::Simulator simulator;
  EXPECT_THROW(simulator.LoadConfiguration(first.string().c_str(), ""),
               ::Smp::InvalidFile);
}

TEST(SimulatorL2, ConfigurationIncludeFragmentMustIdentifyDocumentRoot) {
  TemporarySemanticDirectory directory;
  auto leafXml = ConfigurationHeader("Leaf");
  leafXml += "</Configuration:Configuration>";
  directory.Write("fragment-leaf.smpcfg", leafXml);

  auto rootXml = ConfigurationHeader("FragmentRoot");
  rootXml += R"(  <Include>
    <Configuration xlink:href="fragment-leaf.smpcfg#Missing"/>
  </Include>
</Configuration:Configuration>)";
  const auto root = directory.Write("fragment-root.smpcfg", rootXml);

  ::Xsmp::Simulator simulator;
  EXPECT_THROW(simulator.LoadConfiguration(root.string().c_str(), ""),
               ::Smp::InvalidFile);
}

TEST(SimulatorL2,
     TopLevelAbsoluteComponentConfigurationTargetsAnotherComponent) {
  TemporarySemanticDirectory directory;
  const auto assembly =
      directory.Write("absolute-component-configuration.smpasb",
                      R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="AbsoluteComponentConfiguration"
    Name="AbsoluteComponentConfiguration">
  <ComponentConfiguration InstancePath="/Other">
    <FieldValue xsi:type="Types:Int32Value" Value="73" Field="integer1"/>
  </ComponentConfiguration>
  <Model Name="AssemblyRoot" Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
</Assembly:Assembly>)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  auto *other = new ::Xsmp::Tests::ModelWithSimpleFields{
      "Other", "", &simulator, &simulator};
  simulator.AddModel(other);

  EXPECT_NO_THROW(
      simulator.LoadAssembly(assembly.string().c_str(), "", "", ""));
  auto *otherField =
      dynamic_cast<::Smp::ISimpleField *>(other->GetField("integer1"));
  ASSERT_NE(otherField, nullptr);
  EXPECT_EQ(static_cast<::Smp::Int32>(otherField->GetValue()), 73);

  auto *root = dynamic_cast<::Smp::IModel *>(
      ::Xsmp::Helper::Resolve(&simulator, "/AssemblyRoot"));
  ASSERT_NE(root, nullptr);
  auto *rootField =
      dynamic_cast<::Smp::ISimpleField *>(root->GetField("integer1"));
  ASSERT_NE(rootField, nullptr);
  EXPECT_NE(static_cast<::Smp::Int32>(rootField->GetValue()), 73);
}

TEST(SimulatorL2, ParentSegmentsAreRejectedInLevel2Paths) {
  TemporarySemanticDirectory directory;
  const auto assembly = directory.Write(
      "parent-segment.smpasb", R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="ParentSegment" Name="ParentSegment">
  <ComponentConfiguration InstancePath="../Outside"/>
  <Model Name="RejectedRoot" Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
</Assembly:Assembly>)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  EXPECT_THROW(simulator.LoadAssembly(assembly.string().c_str(), "", "", ""),
               ::Smp::InvalidFile);
  EXPECT_EQ(::Xsmp::Helper::Resolve(&simulator, "/RejectedRoot"), nullptr);
}

TEST(SimulatorL2, CallOperationAcceptsOrderedInputAndInOutParameters) {
  TemporarySemanticDirectory directory;
  const auto parameters = OperationParameter("first", "Int32Value", "3") +
                          OperationParameter("second", "Int32Value", "5") +
                          OperationParameter("inout", "Int32Value", "7");
  const auto schedule = WriteCallOperationSchedule(
      directory, "call-operation-valid.smpsed", parameters);

  ::Xsmp::Simulator simulator;
  auto *probe = AddConnectedOperationProbe(simulator);

  ASSERT_NO_THROW(simulator.LoadSchedule(schedule.string().c_str()));
  EXPECT_NO_THROW(simulator.Run(1));
  EXPECT_EQ(probe->callCount, 1);
  EXPECT_EQ(probe->first, 3);
  EXPECT_EQ(probe->second, 5);
  EXPECT_EQ(probe->inout, 7);
}

TEST(SimulatorL2, CallOperationRejectsMissingInputParameters) {
  TemporarySemanticDirectory directory;
  const auto parameters = OperationParameter("first", "Int32Value", "3") +
                          OperationParameter("inout", "Int32Value", "7");
  const auto schedule = WriteCallOperationSchedule(
      directory, "call-operation-missing-input.smpsed", parameters);

  ::Xsmp::Simulator simulator;
  auto *probe = AddConnectedOperationProbe(simulator);

  EXPECT_THROW(simulator.LoadSchedule(schedule.string().c_str()),
               ::Smp::InvalidFile);
  EXPECT_EQ(probe->callCount, 0);
}

TEST(SimulatorL2, CallOperationRejectsOutOfOrderParameters) {
  TemporarySemanticDirectory directory;
  const auto parameters = OperationParameter("second", "Int32Value", "5") +
                          OperationParameter("first", "Int32Value", "3") +
                          OperationParameter("inout", "Int32Value", "7");
  const auto schedule = WriteCallOperationSchedule(
      directory, "call-operation-order.smpsed", parameters);

  ::Xsmp::Simulator simulator;
  auto *probe = AddConnectedOperationProbe(simulator);

  EXPECT_THROW(simulator.LoadSchedule(schedule.string().c_str()),
               ::Smp::InvalidFile);
  EXPECT_EQ(probe->callCount, 0);
}

TEST(SimulatorL2, CallOperationRejectsDuplicateParameters) {
  TemporarySemanticDirectory directory;
  const auto parameters = OperationParameter("first", "Int32Value", "3") +
                          OperationParameter("first", "Int32Value", "4");
  const auto schedule = WriteCallOperationSchedule(
      directory, "call-operation-duplicate.smpsed", parameters);

  ::Xsmp::Simulator simulator;
  auto *probe = AddConnectedOperationProbe(simulator);

  EXPECT_THROW(simulator.LoadSchedule(schedule.string().c_str()),
               ::Smp::InvalidFile);
  EXPECT_EQ(probe->callCount, 0);
}

TEST(SimulatorL2, CallOperationRejectsOutputParameterValues) {
  TemporarySemanticDirectory directory;
  const auto parameters = OperationParameter("output", "Int32Value", "8");
  const auto schedule = WriteCallOperationSchedule(
      directory, "call-operation-output.smpsed", parameters);

  ::Xsmp::Simulator simulator;
  auto *probe = AddConnectedOperationProbe(simulator);

  EXPECT_THROW(simulator.LoadSchedule(schedule.string().c_str()),
               ::Smp::InvalidFile);
  EXPECT_EQ(probe->callCount, 0);
}

TEST(SimulatorL2, CallOperationRejectsReturnParameterValues) {
  TemporarySemanticDirectory directory;
  const auto parameters = OperationParameter("result", "Int32Value", "15");
  const auto schedule = WriteCallOperationSchedule(
      directory, "call-operation-return.smpsed", parameters);

  ::Xsmp::Simulator simulator;
  auto *probe = AddConnectedOperationProbe(simulator);

  EXPECT_THROW(simulator.LoadSchedule(schedule.string().c_str()),
               ::Smp::InvalidFile);
  EXPECT_EQ(probe->callCount, 0);
}

TEST(SimulatorL2, CallOperationRejectsIncompatibleParameterValues) {
  TemporarySemanticDirectory directory;
  const auto parameters = OperationParameter("first", "BoolValue", "true");
  const auto schedule = WriteCallOperationSchedule(
      directory, "call-operation-incompatible.smpsed", parameters);

  ::Xsmp::Simulator simulator;
  auto *probe = AddConnectedOperationProbe(simulator);

  EXPECT_THROW(simulator.LoadSchedule(schedule.string().c_str()),
               ::Smp::InvalidFile);
  EXPECT_EQ(probe->callCount, 0);
}

TEST(SimulatorL2, OperationCallAcceptsOrderedInputsAndExpectedReturn) {
  TemporarySemanticDirectory directory;
  const auto parameters = OperationParameter("first", "Int32Value", "3") +
                          OperationParameter("second", "Int32Value", "5") +
                          OperationParameter("inout", "Int32Value", "7") +
                          OperationParameter("result", "Int32Value", "15");
  const auto assembly = WriteOperationCallAssembly(
      directory, "operation-call-valid.smpasb", parameters);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  auto *probe = AddOperationProbe(simulator);

  EXPECT_NO_THROW(
      simulator.LoadAssembly(assembly.string().c_str(), "", "", ""));
  EXPECT_EQ(probe->callCount, 1);
  EXPECT_EQ(probe->first, 3);
  EXPECT_EQ(probe->second, 5);
  EXPECT_EQ(probe->inout, 7);
}

TEST(SimulatorL2, OperationCallRejectsMissingInputParameters) {
  TemporarySemanticDirectory directory;
  const auto parameters = OperationParameter("first", "Int32Value", "3") +
                          OperationParameter("inout", "Int32Value", "7");
  const auto assembly = WriteOperationCallAssembly(
      directory, "operation-call-missing-input.smpasb", parameters);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  auto *probe = AddOperationProbe(simulator);

  EXPECT_THROW(simulator.LoadAssembly(assembly.string().c_str(), "", "", ""),
               ::Smp::InvalidFile);
  EXPECT_EQ(probe->callCount, 0);
}

TEST(SimulatorL2, OperationCallRejectsOutOfOrderParameters) {
  TemporarySemanticDirectory directory;
  const auto parameters = OperationParameter("second", "Int32Value", "5") +
                          OperationParameter("first", "Int32Value", "3");
  const auto assembly = WriteOperationCallAssembly(
      directory, "operation-call-order.smpasb", parameters);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  auto *probe = AddOperationProbe(simulator);

  EXPECT_THROW(simulator.LoadAssembly(assembly.string().c_str(), "", "", ""),
               ::Smp::InvalidFile);
  EXPECT_EQ(probe->callCount, 0);
}

TEST(SimulatorL2, OperationCallRejectsDuplicateParameters) {
  TemporarySemanticDirectory directory;
  const auto parameters = OperationParameter("first", "Int32Value", "3") +
                          OperationParameter("first", "Int32Value", "4");
  const auto assembly = WriteOperationCallAssembly(
      directory, "operation-call-duplicate.smpasb", parameters);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  auto *probe = AddOperationProbe(simulator);

  EXPECT_THROW(simulator.LoadAssembly(assembly.string().c_str(), "", "", ""),
               ::Smp::InvalidFile);
  EXPECT_EQ(probe->callCount, 0);
}

TEST(SimulatorL2, OperationCallRejectsOutputParameterValues) {
  TemporarySemanticDirectory directory;
  const auto parameters = OperationParameter("output", "Int32Value", "8");
  const auto assembly = WriteOperationCallAssembly(
      directory, "operation-call-output.smpasb", parameters);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  auto *probe = AddOperationProbe(simulator);

  EXPECT_THROW(simulator.LoadAssembly(assembly.string().c_str(), "", "", ""),
               ::Smp::InvalidFile);
  EXPECT_EQ(probe->callCount, 0);
}

TEST(SimulatorL2, OperationCallRejectsIncompatibleParameterValues) {
  TemporarySemanticDirectory directory;
  const auto parameters = OperationParameter("first", "BoolValue", "true");
  const auto assembly = WriteOperationCallAssembly(
      directory, "operation-call-incompatible.smpasb", parameters);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  auto *probe = AddOperationProbe(simulator);

  EXPECT_THROW(simulator.LoadAssembly(assembly.string().c_str(), "", "", ""),
               ::Smp::InvalidFile);
  EXPECT_EQ(probe->callCount, 0);
}

TEST(SimulatorL2, OperationCallRejectsUnexpectedReturnValue) {
  TemporarySemanticDirectory directory;
  const auto parameters = OperationParameter("first", "Int32Value", "3") +
                          OperationParameter("second", "Int32Value", "5") +
                          OperationParameter("inout", "Int32Value", "7") +
                          OperationParameter("result", "Int32Value", "14");
  const auto assembly = WriteOperationCallAssembly(
      directory, "operation-call-return.smpasb", parameters);

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  auto *probe = AddOperationProbe(simulator);

  EXPECT_THROW(simulator.LoadAssembly(assembly.string().c_str(), "", "", ""),
               ::Smp::InvalidFile);
  EXPECT_EQ(probe->callCount, 1);
}

TEST(SimulatorL2, MissingSecondScheduleIsIgnored) {
  TemporarySemanticDirectory directory;
  const auto schedule =
      directory.Write("first.smpsed", R"(<?xml version="1.0" encoding="UTF-8"?>
<Schedule:Schedule
    xmlns:Schedule="http://www.ecss.nl/smp/2025/Smdl/Schedule"
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    xmlns:xlink="http://www.w3.org/1999/xlink"
    Id="FirstSchedule" Name="FirstSchedule">
  <Task Id="FirstSchedule.Empty" Name="Empty"/>
  <Event Id="FirstSchedule.Start" Name="Start"
         xsi:type="Schedule:SimulationEvent" SimulationTime="PT0S">
    <Task xlink:href="#FirstSchedule.Empty"/>
  </Event>
</Schedule:Schedule>)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  ASSERT_NO_THROW(simulator.LoadSchedule(schedule.string().c_str()));

  const auto missing = directory.path / "does-not-exist.smpsed";
  EXPECT_NO_THROW(simulator.LoadSchedule(missing.string().c_str()));
}

} // namespace
} // namespace Xsmp::L2
