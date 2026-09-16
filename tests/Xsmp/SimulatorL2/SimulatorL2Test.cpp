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
#include <Smp/ISimpleField.h>
#include <Smp/ISimulatorL2.h>
#include <Smp/InvalidFile.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Simulator.h>

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <type_traits>

namespace Xsmp::L2 {
namespace {

static_assert(std::is_base_of_v<::Smp::ISimulatorL2, ::Xsmp::Simulator>);

class TemporaryDirectory final {
public:
  TemporaryDirectory() {
    const auto suffix = std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count());
    path = std::filesystem::temp_directory_path() /
           ("xsmp-simulator-l2-runtime-" + suffix);
    std::filesystem::create_directories(path);
  }

  ~TemporaryDirectory() {
    std::error_code error;
    std::filesystem::remove_all(path, error);
  }

  TemporaryDirectory(const TemporaryDirectory &) = delete;
  TemporaryDirectory &operator=(const TemporaryDirectory &) = delete;

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

std::filesystem::path WriteAssembly(const TemporaryDirectory &directory,
                                    std::string_view modelName) {
  std::string xml = R"(<?xml version="1.0" encoding="UTF-8"?>
<Assembly:Assembly
    xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
    xmlns:Types="http://www.ecss.nl/smp/2019/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="RuntimeAssembly" Name="RuntimeAssembly">
  <Model Name=")";
  xml += modelName;
  xml += R"(" Implementation="Xsmp::Tests::ModelWithSimpleFields"/>
</Assembly:Assembly>)";
  return directory.Write("runtime.smpasb", xml);
}

TEST(SimulatorL2, ImplementsTheStandardInterface) {
  ::Xsmp::Simulator simulator;
  EXPECT_NE(dynamic_cast<::Smp::ISimulatorL2 *>(&simulator), nullptr);
}

TEST(SimulatorL2, AssemblyResolvesAnEarlierConfiguration) {
  TemporaryDirectory directory;
  const auto configuration = directory.Write(
      "deferred.smpcfg", R"(<?xml version="1.0" encoding="UTF-8"?>
<Configuration:Configuration
    xmlns:Configuration="http://www.ecss.nl/smp/2025/Smdl/Configuration"
    xmlns:Types="http://www.ecss.nl/smp/2025/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="DeferredConfiguration" Name="DeferredConfiguration">
  <Component Path="/Configured">
    <FieldValue xsi:type="Types:Int32Value" Value="42" Field="integer1"/>
  </Component>
</Configuration:Configuration>)");
  const auto assembly = WriteAssembly(directory, "Configured");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");

  simulator.LoadConfiguration(configuration.string().c_str(), "");
  simulator.LoadAssembly(assembly.string().c_str(), "", "", "");

  auto *model = dynamic_cast<::Smp::IModel *>(
      ::Xsmp::Helper::Resolve(&simulator, "/Configured"));
  ASSERT_NE(model, nullptr);
  auto *field =
      dynamic_cast<::Smp::ISimpleField *>(model->GetField("integer1"));
  ASSERT_NE(field, nullptr);
  EXPECT_EQ(static_cast<::Smp::Int32>(field->GetValue()), 42);

  EXPECT_NO_THROW(simulator.Connect());
}

TEST(SimulatorL2, InvalidParentFallsBackToTheSimulatorAndRootNameOverrides) {
  TemporaryDirectory directory;
  const auto assembly = WriteAssembly(directory, "OriginalName");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadLibrary("xsmp_tests");
  simulator.LoadAssembly(assembly.string().c_str(), "/missing-parent",
                         "ignored", "Renamed");

  EXPECT_NE(::Xsmp::Helper::Resolve(&simulator, "/Renamed"), nullptr);
  EXPECT_EQ(::Xsmp::Helper::Resolve(&simulator, "/OriginalName"), nullptr);
}

TEST(SimulatorL2, ConnectRejectsUnresolvedConfigurationElements) {
  TemporaryDirectory directory;
  const auto configuration = directory.Write(
      "unresolved.smpcfg", R"(<?xml version="1.0" encoding="UTF-8"?>
<Configuration:Configuration
    xmlns:Configuration="http://www.ecss.nl/smp/2025/Smdl/Configuration"
    xmlns:Types="http://www.ecss.nl/smp/2025/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="UnresolvedConfiguration" Name="UnresolvedConfiguration">
  <Component Path="/Missing">
    <FieldValue xsi:type="Types:Int32Value" Value="1" Field="integer1"/>
  </Component>
</Configuration:Configuration>)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.LoadConfiguration(configuration.string().c_str(), "");

  EXPECT_THROW(simulator.Connect(), ::Smp::InvalidFile);
}

TEST(SimulatorL2, ReconnectWithoutRootRejectsUnresolvedConfigurationElements) {
  TemporaryDirectory directory;
  const auto configuration = directory.Write(
      "unresolved-reconnect.smpcfg", R"(<?xml version="1.0" encoding="UTF-8"?>
<Configuration:Configuration
    xmlns:Configuration="http://www.ecss.nl/smp/2025/Smdl/Configuration"
    xmlns:Types="http://www.ecss.nl/smp/2025/Core/Types"
    xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
    Id="UnresolvedReconnectConfiguration"
    Name="UnresolvedReconnectConfiguration">
  <Component Path="/Missing">
    <FieldValue xsi:type="Types:Int32Value" Value="1" Field="integer1"/>
  </Component>
</Configuration:Configuration>)");

  ::Xsmp::Simulator simulator;
  simulator.LoadLibrary("xsmp_services");
  simulator.Connect();
  simulator.LoadConfiguration(configuration.string().c_str(), "");

  EXPECT_THROW(simulator.Reconnect(nullptr), ::Smp::InvalidFile);
  EXPECT_EQ(simulator.GetState(), ::Smp::SimulatorStateKind::SSK_Standby);
}

TEST(SimulatorL2, ReportsMissingAndMalformedFiles) {
  TemporaryDirectory directory;
  const auto malformed = directory.Write(
      "malformed.smpasb",
      R"(<Assembly xmlns="http://www.ecss.nl/smp/2025/Smdl/Assembly">)");

  ::Xsmp::Simulator simulator;
  EXPECT_THROW(simulator.LoadAssembly("does-not-exist.smpasb", "", "", ""),
               ::Smp::FileNotFound);
  EXPECT_THROW(simulator.LoadAssembly(malformed.string().c_str(), "", "", ""),
               ::Smp::InvalidFile);
}

} // namespace
} // namespace Xsmp::L2
