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

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/valid.h>
#include <libxml/xmlerror.h>
#include <libxml/xmlschemas.h>

#include <algorithm>
#include <cctype>
#include <memory>
#include <set>
#include <stdexcept>
#include <system_error>
#include <utility>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace Xsmp::L2::Xml {
namespace {

const char MODULE_ANCHOR{};

std::filesystem::path ModuleDirectory() {
#if defined(_WIN32)
  HMODULE module{};
  if (!GetModuleHandleExW(
          GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
          reinterpret_cast<LPCWSTR>(static_cast<const void *>(&MODULE_ANCHOR)),
          &module)) {
    return {};
  }

  std::vector<wchar_t> path(MAX_PATH);
  for (;;) {
    const auto size = GetModuleFileNameW(module, path.data(),
                                         static_cast<DWORD>(path.size()));
    if (size == 0) {
      return {};
    }
    if (size < path.size()) {
      return std::filesystem::path{path.data(), path.data() + size}
          .parent_path();
    }
    path.resize(path.size() * 2U);
  }
#else
  Dl_info information{};
  if (dladdr(static_cast<const void *>(&MODULE_ANCHOR), &information) == 0 ||
      !information.dli_fname) {
    return {};
  }
  std::error_code error;
  auto path = std::filesystem::weakly_canonical(information.dli_fname, error);
  if (error) {
    error.clear();
    path = std::filesystem::absolute(information.dli_fname, error);
  }
  return error ? std::filesystem::path{} : path.parent_path();
#endif
}

std::filesystem::path
ExistingDirectory(const std::filesystem::path &moduleDirectory,
                  std::string_view root, std::string_view level) {
  auto path = (moduleDirectory / root / level).lexically_normal();
  std::error_code error;
  if (!std::filesystem::is_directory(path, error) || error) {
    return {};
  }
  auto canonical = std::filesystem::weakly_canonical(path, error);
  return error ? path : canonical;
}

constexpr std::string_view XML_SCHEMA_NAMESPACE =
    "http://www.w3.org/2001/XMLSchema";
constexpr std::string_view XML_NAMESPACE =
    "http://www.w3.org/XML/1998/namespace";

using XmlDocument = std::unique_ptr<xmlDoc, decltype(&xmlFreeDoc)>;
using XmlParserContext =
    std::unique_ptr<xmlParserCtxt, decltype(&xmlFreeParserCtxt)>;
using XmlSchema = std::unique_ptr<xmlSchema, decltype(&xmlSchemaFree)>;
using XmlSchemaParserContext =
    std::unique_ptr<xmlSchemaParserCtxt, decltype(&xmlSchemaFreeParserCtxt)>;
using XmlSchemaValidationContext =
    std::unique_ptr<xmlSchemaValidCtxt, decltype(&xmlSchemaFreeValidCtxt)>;

struct ErrorSink {
  std::vector<Diagnostic> *diagnostics;
  std::filesystem::path fallbackFile;
  std::string phase;
};

std::string Trim(std::string value) {
  const auto isSpace = [](unsigned char character) {
    return std::isspace(character) != 0;
  };
  const auto begin = std::find_if_not(value.begin(), value.end(), isSpace);
  const auto end =
      std::find_if_not(value.rbegin(), value.rend(), isSpace).base();
  return begin < end ? std::string(begin, end) : std::string{};
}

std::string ToString(const xmlChar *value) {
  return value ? reinterpret_cast<const char *>(value) : std::string{};
}

DiagnosticSeverity SeverityOf(const xmlError &error) noexcept {
  switch (error.level) {
  case XML_ERR_WARNING:
    return DiagnosticSeverity::Warning;
  case XML_ERR_FATAL:
    return DiagnosticSeverity::Fatal;
  default:
    return DiagnosticSeverity::Error;
  }
}

void AddDiagnostic(std::vector<Diagnostic> &diagnostics,
                   DiagnosticSeverity severity,
                   const std::filesystem::path &file, std::size_t line,
                   std::size_t column, std::string message) {
  diagnostics.push_back(
      {severity, {file, line, column}, Trim(std::move(message))});
}

#if LIBXML_VERSION >= 21200
void CollectXmlError(void *context, const xmlError *error) {
#else
void CollectXmlError(void *context, xmlErrorPtr error) {
#endif
  if (!context || !error) {
    return;
  }
  auto &sink = *static_cast<ErrorSink *>(context);
  auto file =
      error->file ? std::filesystem::path{error->file} : sink.fallbackFile;
  auto message = error->message ? std::string{error->message}
                                : std::string{"unknown XML error"};
  if (!sink.phase.empty()) {
    message = sink.phase + ": " + message;
  }
  AddDiagnostic(*sink.diagnostics, SeverityOf(*error), file,
                error->line > 0 ? static_cast<std::size_t>(error->line) : 0U,
                error->int2 > 0 ? static_cast<std::size_t>(error->int2) : 0U,
                std::move(message));
}

bool IsRegularFile(const std::filesystem::path &path,
                   std::vector<Diagnostic> &diagnostics,
                   std::string_view description) {
  std::error_code error;
  const bool exists = std::filesystem::exists(path, error);
  if (error || !exists) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, path, 0, 0,
                  std::string{description} + " does not exist");
    return false;
  }
  if (!std::filesystem::is_regular_file(path, error) || error) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, path, 0, 0,
                  std::string{description} + " is not a regular file");
    return false;
  }
  return true;
}

XmlDocument ParseXml(const std::filesystem::path &path,
                     std::uintmax_t maximumFileSize,
                     std::vector<Diagnostic> &diagnostics,
                     std::string_view phase) {
  if (!IsRegularFile(path, diagnostics, "XML file")) {
    return {nullptr, xmlFreeDoc};
  }

  std::error_code error;
  const auto size = std::filesystem::file_size(path, error);
  if (error) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, path, 0, 0,
                  "cannot determine XML file size: " + error.message());
    return {nullptr, xmlFreeDoc};
  }
  if (maximumFileSize != 0U && size > maximumFileSize) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, path, 0, 0,
                  "XML file exceeds the configured size limit");
    return {nullptr, xmlFreeDoc};
  }

  xmlInitParser();
  XmlParserContext parser{xmlNewParserCtxt(), xmlFreeParserCtxt};
  if (!parser) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Fatal, path, 0, 0,
                  "cannot allocate an XML parser context");
    return {nullptr, xmlFreeDoc};
  }

  const auto nativePath = path.u8string();
  constexpr int parseOptions = XML_PARSE_NONET | XML_PARSE_NOERROR |
                               XML_PARSE_NOWARNING | XML_PARSE_COMPACT;
  XmlDocument document{
      xmlCtxtReadFile(parser.get(), nativePath.c_str(), nullptr, parseOptions),
      xmlFreeDoc};
  if (!document) {
    ErrorSink sink{&diagnostics, path, std::string{phase}};
    if (const auto *lastError = xmlCtxtGetLastError(parser.get())) {
      CollectXmlError(&sink, const_cast<xmlErrorPtr>(lastError));
    } else {
      AddDiagnostic(diagnostics, DiagnosticSeverity::Error, path, 0, 0,
                    std::string{phase} + ": malformed XML document");
    }
    return {nullptr, xmlFreeDoc};
  }

  if (document->intSubset != nullptr || document->extSubset != nullptr) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, path, 0, 0,
                  std::string{phase} +
                      ": document type declarations are not allowed");
    return {nullptr, xmlFreeDoc};
  }
  return document;
}

QualifiedName NameOf(const xmlNode &node) {
  return {node.ns && node.ns->href ? ToString(node.ns->href) : std::string{},
          ToString(node.name),
          node.ns && node.ns->prefix ? ToString(node.ns->prefix)
                                     : std::string{}};
}

QualifiedName NameOf(const xmlAttr &attribute) {
  return {attribute.ns && attribute.ns->href ? ToString(attribute.ns->href)
                                             : std::string{},
          ToString(attribute.name),
          attribute.ns && attribute.ns->prefix ? ToString(attribute.ns->prefix)
                                               : std::string{}};
}

SourceLocation LocationOf(const xmlNode &node,
                          const std::filesystem::path &fallbackFile) {
  const auto line = xmlGetLineNo(const_cast<xmlNode *>(&node));
  const auto file = node.doc && node.doc->URL
                        ? std::filesystem::path{ToString(node.doc->URL)}
                        : fallbackFile;
  return {file, line > 0 ? static_cast<std::size_t>(line) : 0U, 0U};
}

std::optional<QualifiedName>
ResolveQName(const xmlNode &node, const std::string &rawValue,
             const SourceLocation &source,
             std::vector<Diagnostic> &diagnostics) {
  const auto value = Trim(rawValue);
  const auto separator = value.find(':');
  if (value.empty() ||
      xmlValidateQName(reinterpret_cast<const xmlChar *>(value.c_str()), 0) !=
          0) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, source.file,
                  source.line, source.column,
                  "invalid QName value '" + rawValue + "'");
    return std::nullopt;
  }

  std::string prefix;
  std::string localName;
  if (separator == std::string::npos) {
    localName = value;
  } else {
    prefix = value.substr(0U, separator);
    localName = value.substr(separator + 1U);
  }

  const auto *namespaceDeclaration = xmlSearchNs(
      node.doc, const_cast<xmlNode *>(&node),
      prefix.empty() ? nullptr
                     : reinterpret_cast<const xmlChar *>(prefix.c_str()));
  if (!prefix.empty() && !namespaceDeclaration) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, source.file,
                  source.line, source.column,
                  "QName '" + rawValue + "' uses an undeclared prefix");
    return std::nullopt;
  }
  return QualifiedName{namespaceDeclaration && namespaceDeclaration->href
                           ? ToString(namespaceDeclaration->href)
                           : std::string{},
                       std::move(localName), std::move(prefix)};
}

Element BuildElement(const xmlNode &node,
                     const std::filesystem::path &fallbackFile,
                     std::vector<Diagnostic> &diagnostics) {
  Element result;
  result.name = NameOf(node);
  result.source = LocationOf(node, fallbackFile);

  const xmlAttr *xsiTypeAttribute = nullptr;
  for (const auto *attribute = node.properties; attribute;
       attribute = attribute->next) {
    const auto name = NameOf(*attribute);
    xmlChar *rawValue = xmlNodeListGetString(node.doc, attribute->children, 1);
    const std::string value = ToString(rawValue);
    xmlFree(rawValue);
    if (name.Is(XML_SCHEMA_INSTANCE_NAMESPACE, "type")) {
      xsiTypeAttribute = attribute;
    }
    result.attributes.push_back({name, value, LocationOf(node, fallbackFile)});
  }

  if (xsiTypeAttribute) {
    xmlChar *rawValue =
        xmlNodeListGetString(node.doc, xsiTypeAttribute->children, 1);
    const std::string value = ToString(rawValue);
    xmlFree(rawValue);
    result.xsiType = ResolveQName(node, value, result.source, diagnostics);
  }

  for (const auto *child = node.children; child; child = child->next) {
    if (child->type == XML_ELEMENT_NODE) {
      result.children.push_back(
          BuildElement(*child, fallbackFile, diagnostics));
    } else if ((child->type == XML_TEXT_NODE ||
                child->type == XML_CDATA_SECTION_NODE) &&
               child->content) {
      result.text += ToString(child->content);
    }
  }
  return result;
}

bool TransformValues(
    xmlNode &node, const std::filesystem::path &fallbackFile,
    const std::function<std::string(std::string_view, const SourceLocation &)>
        &transformer,
    std::vector<Diagnostic> &diagnostics) {
  bool valid = true;
  for (auto *attribute = node.properties; attribute;) {
    auto *next = attribute->next;
    const auto name = NameOf(*attribute);
    if (!name.Is(XML_SCHEMA_INSTANCE_NAMESPACE, "type")) {
      xmlChar *rawValue =
          xmlNodeListGetString(node.doc, attribute->children, 1);
      const std::string value = ToString(rawValue);
      xmlFree(rawValue);
      const auto source = LocationOf(node, fallbackFile);
      try {
        auto transformed = transformer(value, source);
        if (transformed.find('\0') != std::string::npos) {
          throw std::invalid_argument(
              "transformed XML values must not contain NUL characters");
        }
        if (transformed != value &&
            !xmlSetNsProp(
                &node, attribute->ns, attribute->name,
                reinterpret_cast<const xmlChar *>(transformed.c_str()))) {
          AddDiagnostic(diagnostics, DiagnosticSeverity::Fatal, source.file,
                        source.line, source.column,
                        "cannot update a transformed XML attribute");
          valid = false;
        }
      } catch (const std::exception &error) {
        AddDiagnostic(diagnostics, DiagnosticSeverity::Error, source.file,
                      source.line, source.column,
                      "value transformation: " + std::string{error.what()});
        valid = false;
      } catch (...) {
        AddDiagnostic(diagnostics, DiagnosticSeverity::Error, source.file,
                      source.line, source.column,
                      "value transformation failed with an unknown error");
        valid = false;
      }
    }
    attribute = next;
  }

  for (auto *child = node.children; child; child = child->next) {
    if (child->type == XML_ELEMENT_NODE) {
      valid = TransformValues(*child, fallbackFile, transformer, diagnostics) &&
              valid;
      continue;
    }
    if ((child->type != XML_TEXT_NODE &&
         child->type != XML_CDATA_SECTION_NODE) ||
        !child->content) {
      continue;
    }
    const std::string value = ToString(child->content);
    const auto source = LocationOf(*child, fallbackFile);
    try {
      auto transformed = transformer(value, source);
      if (transformed.find('\0') != std::string::npos) {
        throw std::invalid_argument(
            "transformed XML values must not contain NUL characters");
      }
      if (transformed != value) {
        xmlNodeSetContent(
            child, reinterpret_cast<const xmlChar *>(transformed.c_str()));
      }
    } catch (const std::exception &error) {
      AddDiagnostic(diagnostics, DiagnosticSeverity::Error, source.file,
                    source.line, source.column,
                    "value transformation: " + std::string{error.what()});
      valid = false;
    } catch (...) {
      AddDiagnostic(diagnostics, DiagnosticSeverity::Error, source.file,
                    source.line, source.column,
                    "value transformation failed with an unknown error");
      valid = false;
    }
  }
  return valid;
}

std::optional<DocumentKind> KindOf(const xmlNode &root) {
  const auto name = NameOf(root);
  if (name.Is(ASSEMBLY_NAMESPACE, "Assembly")) {
    return DocumentKind::Assembly;
  }
  if (name.Is(LINK_BASE_NAMESPACE, "LinkBase")) {
    return DocumentKind::LinkBase;
  }
  if (name.Is(SCHEDULE_NAMESPACE, "Schedule")) {
    return DocumentKind::Schedule;
  }
  if ((name.namespaceUri == CONFIGURATION_2019_NAMESPACE ||
       name.namespaceUri == CONFIGURATION_2025_NAMESPACE) &&
      name.localName == "Configuration") {
    return DocumentKind::Configuration;
  }
  return std::nullopt;
}

std::filesystem::path CanonicalPath(const std::filesystem::path &path,
                                    std::error_code &error) {
  auto result = std::filesystem::weakly_canonical(path, error);
  return error ? std::filesystem::path{} : std::move(result);
}

bool IsBelow(const std::filesystem::path &root,
             const std::filesystem::path &path) {
  const auto relative = path.lexically_relative(root);
  if (relative.empty()) {
    return path == root;
  }
  if (relative.is_absolute()) {
    return false;
  }
  return std::none_of(relative.begin(), relative.end(),
                      [](const auto &part) { return part == ".."; });
}

bool HasUriScheme(const std::string &location) {
  const auto separator = location.find(':');
  if (separator == std::string::npos || separator == 0U) {
    return false;
  }
#if defined(_WIN32)
  if (separator == 1U &&
      std::isalpha(static_cast<unsigned char>(location.front())) != 0) {
    return false;
  }
#endif
  return std::all_of(location.begin(),
                     location.begin() + static_cast<std::ptrdiff_t>(separator),
                     [](unsigned char character) {
                       return std::isalnum(character) != 0 ||
                              character == '+' || character == '-' ||
                              character == '.';
                     });
}

bool HasXmlBase(const xmlNode &node) {
  for (const auto *attribute = node.properties; attribute;
       attribute = attribute->next) {
    if (NameOf(*attribute).Is(XML_NAMESPACE, "base")) {
      return true;
    }
  }
  for (const auto *child = node.children; child; child = child->next) {
    if (child->type == XML_ELEMENT_NODE && HasXmlBase(*child)) {
      return true;
    }
  }
  return false;
}

bool InspectSchema(const std::filesystem::path &schemaPath,
                   const std::filesystem::path &bundleRoot,
                   std::uintmax_t maximumFileSize,
                   std::set<std::filesystem::path> &visited,
                   std::vector<Diagnostic> &diagnostics) {
  std::error_code error;
  const auto canonicalSchema = CanonicalPath(schemaPath, error);
  if (error || canonicalSchema.empty()) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, schemaPath, 0, 0,
                  "cannot resolve schema path: " + error.message());
    return false;
  }
  if (!IsBelow(bundleRoot, canonicalSchema)) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, canonicalSchema, 0, 0,
                  "schema is outside the configured bundle root");
    return false;
  }
  if (!visited.insert(canonicalSchema).second) {
    return true;
  }

  auto schema =
      ParseXml(canonicalSchema, maximumFileSize, diagnostics, "schema parsing");
  if (!schema) {
    return false;
  }
  const auto *root = xmlDocGetRootElement(schema.get());
  if (!root || !NameOf(*root).Is(XML_SCHEMA_NAMESPACE, "schema")) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, canonicalSchema, 0, 0,
                  "schema file does not have an xsd:schema root element");
    return false;
  }
  if (HasXmlBase(*root)) {
    const auto source = LocationOf(*root, canonicalSchema);
    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, source.file,
                  source.line, source.column,
                  "xml:base is not allowed in a local schema bundle");
    return false;
  }

  bool valid = true;
  for (const auto *child = root->children; child; child = child->next) {
    if (child->type != XML_ELEMENT_NODE) {
      continue;
    }
    const auto childName = NameOf(*child);
    if (childName.namespaceUri != XML_SCHEMA_NAMESPACE ||
        (childName.localName != "import" && childName.localName != "include" &&
         childName.localName != "redefine")) {
      continue;
    }

    xmlChar *rawLocation =
        xmlGetProp(child, reinterpret_cast<const xmlChar *>("schemaLocation"));
    const auto location = ToString(rawLocation);
    xmlFree(rawLocation);
    const auto source = LocationOf(*child, canonicalSchema);
    if (location.empty()) {
      AddDiagnostic(diagnostics, DiagnosticSeverity::Error, source.file,
                    source.line, source.column,
                    "schema import/include has no local schemaLocation");
      valid = false;
      continue;
    }
    if (HasUriScheme(location) || location.rfind("//", 0U) == 0U ||
        location.find_first_of("?#") != std::string::npos) {
      AddDiagnostic(diagnostics, DiagnosticSeverity::Error, source.file,
                    source.line, source.column,
                    "non-local schemaLocation is not allowed: '" + location +
                        "'");
      valid = false;
      continue;
    }

    auto importedPath = std::filesystem::path{location};
    if (!importedPath.is_absolute()) {
      importedPath = canonicalSchema.parent_path() / importedPath;
    }
    const auto canonicalImport = CanonicalPath(importedPath, error);
    if (error || canonicalImport.empty()) {
      AddDiagnostic(diagnostics, DiagnosticSeverity::Error, source.file,
                    source.line, source.column,
                    "cannot resolve imported schema '" + location +
                        "': " + error.message());
      error.clear();
      valid = false;
      continue;
    }
    if (!IsRegularFile(canonicalImport, diagnostics, "imported schema") ||
        !IsBelow(bundleRoot, canonicalImport)) {
      if (!IsBelow(bundleRoot, canonicalImport)) {
        AddDiagnostic(diagnostics, DiagnosticSeverity::Error, source.file,
                      source.line, source.column,
                      "imported schema escapes the bundle root: '" + location +
                          "'");
      }
      valid = false;
      continue;
    }
    valid = InspectSchema(canonicalImport, bundleRoot, maximumFileSize, visited,
                          diagnostics) &&
            valid;
  }
  return valid;
}

bool Validate(const xmlDoc &document, DocumentKind kind,
              std::string_view documentNamespace, const SchemaBundle &bundle,
              std::uintmax_t maximumFileSize,
              std::vector<Diagnostic> &diagnostics,
              const std::filesystem::path &sourcePath) {
  std::error_code error;
  auto rootDirectory = bundle.rootDirectory.empty()
                           ? std::filesystem::current_path(error)
                           : bundle.rootDirectory;
  if (error) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, bundle.rootDirectory,
                  0, 0,
                  "cannot resolve schema bundle root: " + error.message());
    return false;
  }
  rootDirectory = CanonicalPath(rootDirectory, error);
  if (error || rootDirectory.empty() ||
      !std::filesystem::is_directory(rootDirectory, error) || error) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, bundle.rootDirectory,
                  0, 0, "schema bundle root is not a readable directory");
    return false;
  }

  auto schemaPath = bundle.SchemaFor(kind, documentNamespace);
  if (schemaPath.empty()) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, rootDirectory, 0, 0,
                  std::string{"schema bundle has no schema for "} +
                      ToString(kind));
    return false;
  }
  if (!schemaPath.is_absolute()) {
    schemaPath = rootDirectory / schemaPath;
  }
  schemaPath = CanonicalPath(schemaPath, error);
  if (error || schemaPath.empty()) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Error, schemaPath, 0, 0,
                  "cannot resolve root schema path: " + error.message());
    return false;
  }

  std::set<std::filesystem::path> visited;
  if (!InspectSchema(schemaPath, rootDirectory, maximumFileSize, visited,
                     diagnostics)) {
    return false;
  }

  auto schemaDocument =
      ParseXml(schemaPath, maximumFileSize, diagnostics, "schema parsing");
  if (!schemaDocument) {
    return false;
  }
  XmlSchemaParserContext parserContext{
      xmlSchemaNewDocParserCtxt(schemaDocument.get()), xmlSchemaFreeParserCtxt};
  if (!parserContext) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Fatal, schemaPath, 0, 0,
                  "cannot allocate an XML Schema parser context");
    return false;
  }
  ErrorSink schemaSink{&diagnostics, schemaPath, "schema compilation"};
  xmlSchemaSetParserStructuredErrors(parserContext.get(), CollectXmlError,
                                     &schemaSink);
  XmlSchema schema{xmlSchemaParse(parserContext.get()), xmlSchemaFree};
  if (!schema) {
    if (!std::any_of(
            diagnostics.begin(), diagnostics.end(), [](const auto &diagnostic) {
              return diagnostic.severity != DiagnosticSeverity::Warning;
            })) {
      AddDiagnostic(diagnostics, DiagnosticSeverity::Error, schemaPath, 0, 0,
                    "XML Schema compilation failed");
    }
    return false;
  }

  XmlSchemaValidationContext validationContext{
      xmlSchemaNewValidCtxt(schema.get()), xmlSchemaFreeValidCtxt};
  if (!validationContext) {
    AddDiagnostic(diagnostics, DiagnosticSeverity::Fatal, sourcePath, 0, 0,
                  "cannot allocate an XML Schema validation context");
    return false;
  }
  ErrorSink validationSink{&diagnostics, sourcePath, "schema validation"};
  xmlSchemaSetValidStructuredErrors(validationContext.get(), CollectXmlError,
                                    &validationSink);
  const auto status = xmlSchemaValidateDoc(validationContext.get(),
                                           const_cast<xmlDoc *>(&document));
  if (status != 0) {
    if (status < 0) {
      AddDiagnostic(diagnostics, DiagnosticSeverity::Fatal, sourcePath, 0, 0,
                    "internal XML Schema validation error");
    } else if (!std::any_of(diagnostics.begin(), diagnostics.end(),
                            [&sourcePath](const auto &diagnostic) {
                              return diagnostic.source.file == sourcePath &&
                                     diagnostic.severity !=
                                         DiagnosticSeverity::Warning;
                            })) {
      AddDiagnostic(diagnostics, DiagnosticSeverity::Error, sourcePath, 0, 0,
                    "document does not conform to its XML Schema");
    }
    return false;
  }
  return true;
}

} // namespace

std::filesystem::path DefaultSchemaDirectory(std::string_view level) {
  const auto moduleDirectory = ModuleDirectory();
  if (moduleDirectory.empty()) {
    return (std::filesystem::path{XSMP_SCHEMA_INSTALL_ROOT} / level)
        .lexically_normal();
  }

  if (auto installed =
          ExistingDirectory(moduleDirectory, XSMP_SCHEMA_INSTALL_ROOT, level);
      !installed.empty()) {
    return installed;
  }
  if (auto built =
          ExistingDirectory(moduleDirectory, XSMP_SCHEMA_BUILD_ROOT, level);
      !built.empty()) {
    return built;
  }
  // Multi-configuration generators append the configuration to the output
  // directory. Try the same build-relative path from its parent as well.
  if (auto built = ExistingDirectory(moduleDirectory.parent_path(),
                                     XSMP_SCHEMA_BUILD_ROOT, level);
      !built.empty()) {
    return built;
  }
  return (moduleDirectory / XSMP_SCHEMA_INSTALL_ROOT / level)
      .lexically_normal();
}

const char *ToString(DocumentKind kind) noexcept {
  switch (kind) {
  case DocumentKind::Assembly:
    return "Assembly";
  case DocumentKind::LinkBase:
    return "LinkBase";
  case DocumentKind::Schedule:
    return "Schedule";
  case DocumentKind::Configuration:
    return "Configuration";
  }
  return "unknown";
}

bool QualifiedName::Is(std::string_view expectedNamespace,
                       std::string_view expectedLocalName) const noexcept {
  return namespaceUri == expectedNamespace && localName == expectedLocalName;
}

const Attribute *
Element::FindAttribute(std::string_view expectedLocalName,
                       std::string_view expectedNamespace) const noexcept {
  const auto iterator = std::find_if(
      attributes.begin(), attributes.end(),
      [expectedLocalName, expectedNamespace](const auto &attribute) {
        return attribute.name.Is(expectedNamespace, expectedLocalName);
      });
  return iterator == attributes.end() ? nullptr : &*iterator;
}

const Element *
Element::FindFirstChild(std::string_view expectedLocalName,
                        std::string_view expectedNamespace) const noexcept {
  const auto iterator =
      std::find_if(children.begin(), children.end(),
                   [expectedLocalName, expectedNamespace](const auto &child) {
                     return child.name.Is(expectedNamespace, expectedLocalName);
                   });
  return iterator == children.end() ? nullptr : &*iterator;
}

std::string Element::TrimmedText() const { return Trim(text); }

std::filesystem::path
SchemaBundle::SchemaFor(DocumentKind kind,
                        std::string_view documentNamespace) const {
  switch (kind) {
  case DocumentKind::Assembly:
    return assemblySchema;
  case DocumentKind::LinkBase:
    return linkBaseSchema;
  case DocumentKind::Schedule:
    return scheduleSchema;
  case DocumentKind::Configuration:
    return documentNamespace == CONFIGURATION_2019_NAMESPACE
               ? configuration2019Schema
               : configurationSchema;
  }
  return {};
}

bool ReadResult::HasErrors() const noexcept {
  return std::any_of(
      diagnostics.begin(), diagnostics.end(), [](const auto &diagnostic) {
        return diagnostic.severity != DiagnosticSeverity::Warning;
      });
}

ReadResult::operator bool() const noexcept {
  return document.has_value() && !HasErrors();
}

ReadResult ReadFile(const std::filesystem::path &path,
                    const ReadOptions &options) {
  ReadResult result;
  auto xmlDocument = ParseXml(path, options.maximumFileSize, result.diagnostics,
                              "document parsing");
  if (!xmlDocument) {
    return result;
  }

  auto *root = xmlDocGetRootElement(xmlDocument.get());
  if (!root) {
    AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, path, 0, 0,
                  "XML document has no root element");
    return result;
  }
  const auto kind = KindOf(*root);
  if (!kind) {
    const auto name = NameOf(*root);
    const auto location = LocationOf(*root, path);
    AddDiagnostic(result.diagnostics, DiagnosticSeverity::Error, location.file,
                  location.line, location.column,
                  "unsupported SMP document root '{" + name.namespaceUri + "}" +
                      name.localName + "'");
    return result;
  }

  if (options.valueTransformer &&
      !TransformValues(*root, path, options.valueTransformer,
                       result.diagnostics)) {
    return result;
  }

  if (options.schemas &&
      !Validate(*xmlDocument, *kind, NameOf(*root).namespaceUri,
                *options.schemas, options.maximumFileSize, result.diagnostics,
                path)) {
    return result;
  }

  auto rootElement = BuildElement(*root, path, result.diagnostics);
  if (result.HasErrors()) {
    return result;
  }
  result.document = Document{*kind, path, std::move(rootElement)};
  return result;
}

} // namespace Xsmp::L2::Xml
