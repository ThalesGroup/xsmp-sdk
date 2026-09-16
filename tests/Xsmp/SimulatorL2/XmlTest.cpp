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

#include <Xsmp/SimulatorL2/Xml.h>

#include "gtest/gtest.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace Xsmp::L2::Xml {
namespace {

class TemporaryDirectory {
public:
  TemporaryDirectory() {
    const auto suffix = std::to_string(
        std::chrono::steady_clock::now().time_since_epoch().count());
    path = std::filesystem::temp_directory_path() /
           ("xsmp-simulator-l2-xml-" + suffix);
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

bool HasDiagnostic(const ReadResult &result, std::string_view text) {
  return std::any_of(result.diagnostics.begin(), result.diagnostics.end(),
                     [text](const auto &diagnostic) {
                       return diagnostic.message.find(text) !=
                              std::string::npos;
                     });
}

TEST(SimulatorL2Xml, ResolvesBundledSchemasRelativeToTheSimulatorLibrary) {
  const auto level1 = DefaultSchemaDirectory("l1");
  const auto level2 = DefaultSchemaDirectory("l2");
  EXPECT_TRUE(std::filesystem::is_regular_file(level1 / "Configuration.xsd"));
  EXPECT_TRUE(
      std::filesystem::is_regular_file(level1 / "2019/Smdl/Configuration.xsd"));
  EXPECT_TRUE(std::filesystem::is_regular_file(level2 / "Assembly.xsd"));
  EXPECT_TRUE(std::filesystem::is_regular_file(level2 / "Schedule.xsd"));
}

TEST(SimulatorL2Xml, ValidatesBothBundledConfigurationNamespaces) {
  const std::vector<std::string_view> namespaces{CONFIGURATION_2019_NAMESPACE,
                                                 CONFIGURATION_2025_NAMESPACE};
  TemporaryDirectory temporaryDirectory;
  ReadOptions options;
  options.schemas = SchemaBundle{DefaultSchemaDirectory("l1")};

  std::size_t index = 0;
  for (const auto configurationNamespace : namespaces) {
    const auto file = temporaryDirectory.Write(
        "configuration-" + std::to_string(index++) + ".smpcfg",
        "<Configuration:Configuration xmlns:Configuration=\"" +
            std::string{configurationNamespace} +
            "\" Id=\"Configuration\" Name=\"Configuration\"/>");
    const auto result = ReadFile(file, options);
    ASSERT_TRUE(result) << (result.diagnostics.empty()
                                ? std::string{}
                                : result.diagnostics.front().message);
    EXPECT_EQ(result.document->kind, DocumentKind::Configuration);
  }
}

TEST(SimulatorL2Xml, RecognisesAllDocumentRoots) {
  const std::vector<std::pair<std::string, DocumentKind>> roots{
      {"<A:Assembly xmlns:A=\"http://www.ecss.nl/smp/2025/Smdl/Assembly\"/>",
       DocumentKind::Assembly},
      {"<L:LinkBase xmlns:L=\"http://www.ecss.nl/smp/2025/Smdl/LinkBase\"/>",
       DocumentKind::LinkBase},
      {"<S:Schedule xmlns:S=\"http://www.ecss.nl/smp/2025/Smdl/Schedule\"/>",
       DocumentKind::Schedule},
      {"<C:Configuration "
       "xmlns:C=\"http://www.ecss.nl/smp/2019/Smdl/Configuration\"/>",
       DocumentKind::Configuration},
      {"<C:Configuration "
       "xmlns:C=\"http://www.ecss.nl/smp/2025/Smdl/Configuration\"/>",
       DocumentKind::Configuration}};

  TemporaryDirectory temporaryDirectory;
  std::size_t index = 0;
  for (const auto &[root, expectedKind] : roots) {
    const auto file = temporaryDirectory.Write(
        "root-" + std::to_string(index++) + ".xml", root);
    const auto result = ReadFile(file);
    ASSERT_TRUE(result) << (result.diagnostics.empty()
                                ? std::string{}
                                : result.diagnostics.front().message);
    EXPECT_EQ(result.document->kind, expectedKind);
  }
}

TEST(SimulatorL2Xml, PreservesElementOrderAndResolvesXsiType) {
  TemporaryDirectory temporaryDirectory;
  const auto file = temporaryDirectory.Write("assembly.smpasb",
                                             R"(<?xml version="1.0"?>
<Assembly:Assembly xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"
                   xmlns:LinkBase="http://www.ecss.nl/smp/2025/Smdl/LinkBase"
                   xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
                   Id="Demo" Name="Demo">
  <ComponentConfiguration InstancePath="Child"/>
  <Model Name="Root" Implementation="demo::Root">
    <Link xsi:type="LinkBase:EventLink">
      <OwnerPath> outbound </OwnerPath>
      <ClientPath>Child.inbound</ClientPath>
    </Link>
    <GlobalEventHandler EntryPointName="tick" GlobalEventName="Tick"/>
  </Model>
</Assembly:Assembly>)");

  const auto result = ReadFile(file);
  ASSERT_TRUE(result) << (result.diagnostics.empty()
                              ? std::string{}
                              : result.diagnostics.front().message);
  const auto &root = result.document->root;
  ASSERT_EQ(root.children.size(), 2U);
  EXPECT_EQ(root.children[0].name.localName, "ComponentConfiguration");
  EXPECT_EQ(root.children[1].name.localName, "Model");
  ASSERT_EQ(root.children[1].children.size(), 2U);
  EXPECT_EQ(root.children[1].children[0].name.localName, "Link");
  EXPECT_EQ(root.children[1].children[1].name.localName, "GlobalEventHandler");

  const auto &link = root.children[1].children[0];
  ASSERT_TRUE(link.xsiType.has_value());
  EXPECT_TRUE(link.xsiType->Is(LINK_BASE_NAMESPACE, "EventLink"));
  ASSERT_EQ(link.children.size(), 2U);
  EXPECT_EQ(link.children[0].TrimmedText(), "outbound");
  ASSERT_NE(root.FindAttribute("Name"), nullptr);
  EXPECT_EQ(root.FindAttribute("Name")->value, "Demo");
  EXPECT_GT(link.source.line, 0U);
}

TEST(SimulatorL2Xml, RejectsDocumentTypeDeclarations) {
  TemporaryDirectory temporaryDirectory;
  const auto file = temporaryDirectory.Write("doctype.smpasb",
                                             R"(<?xml version="1.0"?>
<!DOCTYPE Assembly:Assembly [<!ENTITY secret SYSTEM "file:///etc/passwd">]>
<Assembly:Assembly xmlns:Assembly="http://www.ecss.nl/smp/2025/Smdl/Assembly"/>)");

  const auto result = ReadFile(file);
  EXPECT_FALSE(result);
  EXPECT_FALSE(result.document.has_value());
  EXPECT_TRUE(HasDiagnostic(result, "document type declarations"));
}

TEST(SimulatorL2Xml, ValidatesWithALocalSchemaBundle) {
  TemporaryDirectory temporaryDirectory;
  temporaryDirectory.Write("Assembly.xsd",
                           R"(<?xml version="1.0"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
            xmlns:A="http://www.ecss.nl/smp/2025/Smdl/Assembly"
            targetNamespace="http://www.ecss.nl/smp/2025/Smdl/Assembly"
            elementFormDefault="unqualified">
  <xsd:element name="Assembly">
    <xsd:complexType>
      <xsd:sequence><xsd:element name="Model"/></xsd:sequence>
      <xsd:attribute name="Name" type="xsd:string" use="required"/>
    </xsd:complexType>
  </xsd:element>
</xsd:schema>)");
  const auto validFile = temporaryDirectory.Write(
      "valid.smpasb",
      R"(<A:Assembly xmlns:A="http://www.ecss.nl/smp/2025/Smdl/Assembly" Name="A"><Model/></A:Assembly>)");
  const auto invalidFile = temporaryDirectory.Write(
      "invalid.smpasb",
      R"(<A:Assembly xmlns:A="http://www.ecss.nl/smp/2025/Smdl/Assembly"><Model/></A:Assembly>)");

  ReadOptions options;
  options.schemas = SchemaBundle{temporaryDirectory.path};
  EXPECT_TRUE(ReadFile(validFile, options));
  const auto invalid = ReadFile(invalidFile, options);
  EXPECT_FALSE(invalid);
  EXPECT_TRUE(HasDiagnostic(invalid, "schema validation"));
}

TEST(SimulatorL2Xml, TransformsDomValuesBeforeSchemaValidation) {
  TemporaryDirectory temporaryDirectory;
  temporaryDirectory.Write("Assembly.xsd",
                           R"(<?xml version="1.0"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
            xmlns:A="http://www.ecss.nl/smp/2025/Smdl/Assembly"
            targetNamespace="http://www.ecss.nl/smp/2025/Smdl/Assembly"
            elementFormDefault="unqualified">
  <xsd:element name="Assembly">
    <xsd:complexType>
      <xsd:sequence><xsd:element name="Model" type="xsd:string"/></xsd:sequence>
      <xsd:attribute name="Name" type="xsd:int" use="required"/>
    </xsd:complexType>
  </xsd:element>
</xsd:schema>)");
  const auto file = temporaryDirectory.Write(
      "templated.smpasb",
      R"(<A:Assembly xmlns:A="http://www.ecss.nl/smp/2025/Smdl/Assembly" Name="{Number}"><Model>{Markup}</Model></A:Assembly>)");

  ReadOptions options;
  options.schemas = SchemaBundle{temporaryDirectory.path};
  options.valueTransformer = [](std::string_view value,
                                const SourceLocation &) {
    if (value == "{Number}") {
      return std::string{"42"};
    }
    if (value == "{Markup}") {
      return std::string{"<Injected/>"};
    }
    return std::string{value};
  };

  const auto result = ReadFile(file, options);
  ASSERT_TRUE(result) << (result.diagnostics.empty()
                              ? std::string{}
                              : result.diagnostics.front().message);
  ASSERT_NE(result.document->root.FindAttribute("Name"), nullptr);
  EXPECT_EQ(result.document->root.FindAttribute("Name")->value, "42");
  ASSERT_EQ(result.document->root.children.size(), 1U);
  EXPECT_TRUE(result.document->root.children.front().children.empty());
  EXPECT_EQ(result.document->root.children.front().text, "<Injected/>");
}

TEST(SimulatorL2Xml, RejectsNonLocalSchemaImports) {
  TemporaryDirectory temporaryDirectory;
  temporaryDirectory.Write("Assembly.xsd",
                           R"(<?xml version="1.0"?>
<xsd:schema xmlns:xsd="http://www.w3.org/2001/XMLSchema"
            xmlns:A="http://www.ecss.nl/smp/2025/Smdl/Assembly"
            targetNamespace="http://www.ecss.nl/smp/2025/Smdl/Assembly">
  <xsd:import namespace="urn:external"
              schemaLocation="https://example.invalid/external.xsd"/>
  <xsd:element name="Assembly"/>
</xsd:schema>)");
  const auto file = temporaryDirectory.Write(
      "assembly.smpasb",
      R"(<A:Assembly xmlns:A="http://www.ecss.nl/smp/2025/Smdl/Assembly"/>)");

  ReadOptions options;
  options.schemas = SchemaBundle{temporaryDirectory.path};
  const auto result = ReadFile(file, options);
  EXPECT_FALSE(result);
  EXPECT_TRUE(HasDiagnostic(result, "non-local schemaLocation"));
}

TEST(SimulatorL2Xml, ReportsMalformedXmlLocation) {
  TemporaryDirectory temporaryDirectory;
  const auto file = temporaryDirectory.Write(
      "malformed.smpasb",
      R"(<A:Assembly xmlns:A="http://www.ecss.nl/smp/2025/Smdl/Assembly">
  <Model>
</A:Assembly>)");

  const auto result = ReadFile(file);
  ASSERT_FALSE(result);
  ASSERT_FALSE(result.diagnostics.empty());
  EXPECT_GT(result.diagnostics.front().source.line, 0U);
}

} // namespace
} // namespace Xsmp::L2::Xml
