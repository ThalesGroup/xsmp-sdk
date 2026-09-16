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

#ifndef XSMP_SIMULATORL2_XML_H_
#define XSMP_SIMULATORL2_XML_H_

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Xsmp::L2::Xml {

inline constexpr std::string_view ASSEMBLY_NAMESPACE =
    "http://www.ecss.nl/smp/2025/Smdl/Assembly";
inline constexpr std::string_view LINK_BASE_NAMESPACE =
    "http://www.ecss.nl/smp/2025/Smdl/LinkBase";
inline constexpr std::string_view SCHEDULE_NAMESPACE =
    "http://www.ecss.nl/smp/2025/Smdl/Schedule";
inline constexpr std::string_view CONFIGURATION_2019_NAMESPACE =
    "http://www.ecss.nl/smp/2019/Smdl/Configuration";
inline constexpr std::string_view CONFIGURATION_2025_NAMESPACE =
    "http://www.ecss.nl/smp/2025/Smdl/Configuration";
inline constexpr std::string_view XML_SCHEMA_INSTANCE_NAMESPACE =
    "http://www.w3.org/2001/XMLSchema-instance";

enum class DocumentKind : std::uint8_t {
  Assembly,
  LinkBase,
  Schedule,
  Configuration
};

const char *ToString(DocumentKind kind) noexcept;

enum class DiagnosticSeverity : std::uint8_t { Warning, Error, Fatal };

struct SourceLocation {
  std::filesystem::path file;
  std::size_t line{};
  std::size_t column{};
};

struct Diagnostic {
  DiagnosticSeverity severity{DiagnosticSeverity::Error};
  SourceLocation source;
  std::string message;
};

struct QualifiedName {
  std::string namespaceUri;
  std::string localName;
  std::string prefix;

  bool Is(std::string_view expectedNamespace,
          std::string_view expectedLocalName) const noexcept;
};

struct Attribute {
  QualifiedName name;
  std::string value;
  SourceLocation source;
};

// This deliberately small, owning representation prevents libxml2 objects from
// escaping into the simulator. Child and attribute vectors retain document
// order, which is significant for Level 2 application and scheduling rules.
struct Element {
  QualifiedName name;
  std::optional<QualifiedName> xsiType;
  std::vector<Attribute> attributes;
  std::vector<Element> children;
  std::string text;
  SourceLocation source;

  const Attribute *
  FindAttribute(std::string_view localName,
                std::string_view namespaceUri = {}) const noexcept;
  const Element *
  FindFirstChild(std::string_view localName,
                 std::string_view namespaceUri = {}) const noexcept;
  std::string TrimmedText() const;
};

struct Document {
  DocumentKind kind;
  std::filesystem::path sourcePath;
  Element root;
};

// Schema paths are relative to rootDirectory unless absolute. Keeping every
// entry explicit supports both flat bundles and the Core/Smdl layout used by
// the ECSS archives without relying on process-wide XML catalogues.
struct SchemaBundle {
  std::filesystem::path rootDirectory;
  std::filesystem::path assemblySchema{"Assembly.xsd"};
  std::filesystem::path linkBaseSchema{"LinkBase.xsd"};
  std::filesystem::path scheduleSchema{"Schedule.xsd"};
  std::filesystem::path configurationSchema{"Configuration.xsd"};
  std::filesystem::path configuration2019Schema{"2019/Smdl/Configuration.xsd"};

  std::filesystem::path
  SchemaFor(DocumentKind kind, std::string_view documentNamespace = {}) const;
};

struct ReadOptions {
  std::optional<SchemaBundle> schemas;
  std::uintmax_t maximumFileSize{64U * 1024U * 1024U};

  // Applied to attribute values (except xsi:type) and character data in the
  // parsed DOM before schema validation and conversion to Element. This makes
  // template expansion safe: replacement text remains XML data and the
  // expanded document, rather than the template source, is validated.
  std::function<std::string(std::string_view, const SourceLocation &)>
      valueTransformer;
};

struct ReadResult {
  std::optional<Document> document;
  std::vector<Diagnostic> diagnostics;

  bool HasErrors() const noexcept;
  explicit operator bool() const noexcept;
};

// Resolve a bundled schema directory relative to the Simulator shared
// library. The build and install layouts are supplied privately by CMake, so
// no build-machine or configured install-prefix path is embedded in the
// library.
std::filesystem::path DefaultSchemaDirectory(std::string_view level);

// Reads one XML artefact with network access and entity substitution disabled.
// When a schema bundle is supplied, the source is validated before it is
// converted to the owning representation. Schema imports/includes are accepted
// only when they resolve to regular files below the bundle root.
ReadResult ReadFile(const std::filesystem::path &path,
                    const ReadOptions &options = {});

} // namespace Xsmp::L2::Xml

#endif // XSMP_SIMULATORL2_XML_H_
