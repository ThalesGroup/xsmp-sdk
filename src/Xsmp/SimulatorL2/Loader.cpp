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

#include <Xsmp/SimulatorL2/Loader.h>

#include <Smp/AccessKind.h>
#include <Smp/AnySimple.h>
#include <Smp/DuplicateName.h>
#include <Smp/IAggregate.h>
#include <Smp/IArrayField.h>
#include <Smp/IComponent.h>
#include <Smp/IComposite.h>
#include <Smp/IContainer.h>
#include <Smp/IDynamicInvocation.h>
#include <Smp/IEntryPoint.h>
#include <Smp/IEntryPointPublisher.h>
#include <Smp/IEventSink.h>
#include <Smp/IEventSource.h>
#include <Smp/IFactory.h>
#include <Smp/IField.h>
#include <Smp/IModel.h>
#include <Smp/IOperation.h>
#include <Smp/IOutputField.h>
#include <Smp/IParameter.h>
#include <Smp/IProperty.h>
#include <Smp/IReference.h>
#include <Smp/IRequest.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimpleField.h>
#include <Smp/IStructureField.h>
#include <Smp/InvalidFile.h>
#include <Smp/InvalidObjectName.h>
#include <Smp/InvalidSimulatorState.h>
#include <Smp/Publication/IArrayType.h>
#include <Smp/Publication/IEnumerationType.h>
#include <Smp/Publication/IType.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ILinkRegistry.h>
#include <Smp/Services/ILogger.h>
#include <Smp/Services/IScheduler.h>
#include <Smp/Services/ITimeKeeper.h>
#include <Smp/SimulatorStateKind.h>
#include <Smp/Uuid.h>
#include <Xsmp/DateTime.h>
#include <Xsmp/Duration.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Publication/Request.h>
#include <Xsmp/Simulator.h>
#include <Xsmp/SimulatorL2/Xml.h>

#include <algorithm>
#include <cerrno>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace Xsmp::L2 {
namespace {

using Xml::Document;
using Xml::DocumentKind;
using Xml::Element;
using Xml::SourceLocation;

constexpr std::string_view TYPES_2019_NAMESPACE =
    "http://www.ecss.nl/smp/2019/Core/Types";
constexpr std::string_view TYPES_2025_NAMESPACE =
    "http://www.ecss.nl/smp/2025/Core/Types";

std::string_view FileExtension(DocumentKind kind) noexcept {
  switch (kind) {
  case DocumentKind::Assembly:
    return ".smpasb";
  case DocumentKind::LinkBase:
    return ".smplnk";
  case DocumentKind::Schedule:
    return ".smpsed";
  case DocumentKind::Configuration:
    return ".smpcfg";
  }
  return {};
}

std::string Location(const SourceLocation &source) {
  std::ostringstream stream;
  stream << source.file.string();
  if (source.line != 0U) {
    stream << ':' << source.line;
    if (source.column != 0U) {
      stream << ':' << source.column;
    }
  }
  return stream.str();
}

std::string AttributeValue(const Element &element, std::string_view name,
                           std::string_view defaultValue = {}) {
  if (const auto *attribute = element.FindAttribute(name)) {
    return attribute->value;
  }
  return std::string{defaultValue};
}

const Element *Child(const Element &element, std::string_view name) {
  return element.FindFirstChild(name);
}

std::string ChildText(const Element &element, std::string_view name,
                      std::string_view defaultValue = {}) {
  if (const auto *child = Child(element, name)) {
    return child->TrimmedText();
  }
  return std::string{defaultValue};
}

bool IsTypesNamespace(std::string_view value) {
  return value == TYPES_2019_NAMESPACE || value == TYPES_2025_NAMESPACE;
}

std::string TypeName(const Element &element) {
  if (!element.xsiType || !IsTypesNamespace(element.xsiType->namespaceUri)) {
    return {};
  }
  return element.xsiType->localName;
}

bool IsXmlWhitespace(char character) noexcept {
  return character == ' ' || character == '\t' || character == '\r' ||
         character == '\n';
}

std::string_view TrimXmlWhitespace(std::string_view text) noexcept {
  while (!text.empty() && IsXmlWhitespace(text.front())) {
    text.remove_prefix(1U);
  }
  while (!text.empty() && IsXmlWhitespace(text.back())) {
    text.remove_suffix(1U);
  }
  return text;
}

template <typename Integer>
Integer ParseInteger(std::string_view text, const Element &element) {
  const auto lexical = text;
  text = TrimXmlWhitespace(text);
  if (!text.empty() && text.front() == '+') {
    text.remove_prefix(1U);
  }
  Integer value{};
  const auto *begin = text.data();
  const auto *end = begin + text.size();
  const auto result = std::from_chars(begin, end, value, 10);
  if (result.ec != std::errc{} || result.ptr != end) {
    throw std::invalid_argument(Location(element.source) +
                                ": invalid integer value '" +
                                std::string{lexical} + "'");
  }
  return value;
}

template <typename Float>
Float ParseFloat(std::string_view text, const Element &element) {
  const auto lexical = text;
  text = TrimXmlWhitespace(text);
  if (text == "INF") {
    return std::numeric_limits<Float>::infinity();
  }
  if (text == "-INF") {
    return -std::numeric_limits<Float>::infinity();
  }
  if (text == "NaN") {
    return std::numeric_limits<Float>::quiet_NaN();
  }
  std::string owned{text};
  char *end = nullptr;
  errno = 0;
  const auto parsed = std::strtold(owned.c_str(), &end);
  if (errno == ERANGE || end != owned.c_str() + owned.size() ||
      !std::isfinite(parsed) ||
      parsed < static_cast<long double>(std::numeric_limits<Float>::lowest()) ||
      parsed > static_cast<long double>(std::numeric_limits<Float>::max())) {
    throw std::invalid_argument(Location(element.source) +
                                ": invalid floating-point value '" +
                                std::string{lexical} + "'");
  }
  return static_cast<Float>(parsed);
}

::Smp::Duration ParseIsoDuration(std::string_view text,
                                 const Element &element) {
  text = TrimXmlWhitespace(text);
  // ECSS uses xsd:dayTimeDuration. Years and months are deliberately rejected
  // because they do not have an unambiguous nanosecond representation.
  bool negative = false;
  std::size_t offset = 0U;
  if (offset < text.size() && text[offset] == '-') {
    negative = true;
    ++offset;
  }
  if (offset >= text.size() || text[offset++] != 'P') {
    throw std::invalid_argument(Location(element.source) +
                                ": invalid duration '" + std::string{text} +
                                "'");
  }

  long double totalSeconds = 0.0L;
  bool inTime = false;
  bool sawValue = false;
  while (offset < text.size()) {
    if (text[offset] == 'T') {
      if (inTime) {
        throw std::invalid_argument(Location(element.source) +
                                    ": invalid duration '" + std::string{text} +
                                    "'");
      }
      inTime = true;
      ++offset;
      continue;
    }
    const auto numberStart = offset;
    bool dot = false;
    while (offset < text.size() &&
           ((text[offset] >= '0' && text[offset] <= '9') ||
            (!dot && text[offset] == '.'))) {
      dot = dot || text[offset] == '.';
      ++offset;
    }
    if (numberStart == offset || offset >= text.size()) {
      throw std::invalid_argument(Location(element.source) +
                                  ": invalid duration '" + std::string{text} +
                                  "'");
    }
    const auto number = ParseFloat<long double>(
        text.substr(numberStart, offset - numberStart), element);
    const char unit = text[offset++];
    switch (unit) {
    case 'D':
      if (inTime || dot) {
        throw std::invalid_argument(Location(element.source) +
                                    ": invalid duration '" + std::string{text} +
                                    "'");
      }
      totalSeconds += number * 86400.0L;
      break;
    case 'H':
      if (!inTime || dot) {
        throw std::invalid_argument(Location(element.source) +
                                    ": invalid duration '" + std::string{text} +
                                    "'");
      }
      totalSeconds += number * 3600.0L;
      break;
    case 'M':
      if (!inTime || dot) {
        throw std::invalid_argument(Location(element.source) +
                                    ": invalid duration '" + std::string{text} +
                                    "'");
      }
      totalSeconds += number * 60.0L;
      break;
    case 'S':
      if (!inTime) {
        throw std::invalid_argument(Location(element.source) +
                                    ": invalid duration '" + std::string{text} +
                                    "'");
      }
      totalSeconds += number;
      break;
    default:
      throw std::invalid_argument(Location(element.source) +
                                  ": unsupported duration unit in '" +
                                  std::string{text} + "'");
    }
    sawValue = true;
  }
  if (!sawValue) {
    throw std::invalid_argument(Location(element.source) +
                                ": invalid duration '" + std::string{text} +
                                "'");
  }
  long double nanoseconds = totalSeconds * 1000000000.0L;
  if (negative) {
    nanoseconds = -nanoseconds;
  }
  if (nanoseconds < static_cast<long double>(
                        std::numeric_limits<::Smp::Duration>::lowest()) ||
      nanoseconds > static_cast<long double>(
                        std::numeric_limits<::Smp::Duration>::max())) {
    throw std::invalid_argument(Location(element.source) +
                                ": duration is outside the SMP range");
  }
  return static_cast<::Smp::Duration>(std::llround(nanoseconds));
}

::Smp::DateTime ParseDateTime(std::string_view text, const Element &element,
                              bool requireTimezone = false) {
  text = TrimXmlWhitespace(text);
  try {
    if (!text.empty() && text.back() == 'Z') {
      return static_cast<::Smp::DateTime>(::Xsmp::DateTime{text, "%FT%TZ"});
    }
    const auto timeSeparator = text.find('T');
    const auto offsetSeparator = text.find_last_of("+-");
    if (timeSeparator != std::string_view::npos &&
        offsetSeparator != std::string_view::npos &&
        offsetSeparator > timeSeparator) {
      return static_cast<::Smp::DateTime>(::Xsmp::DateTime{text, "%FT%T%Ez"});
    }
    if (requireTimezone) {
      throw std::invalid_argument("a time zone is required");
    }
    return static_cast<::Smp::DateTime>(::Xsmp::DateTime{text, "%FT%T"});
  } catch (const std::exception &) {
    throw std::invalid_argument(Location(element.source) +
                                ": invalid date-time value '" +
                                std::string{text} + "'");
  }
}

std::optional<::Smp::PrimitiveTypeKind>
PrimitiveTypeOfSimpleValue(std::string_view type) noexcept {
  if (type == "BoolValue") {
    return ::Smp::PrimitiveTypeKind::PTK_Bool;
  }
  if (type == "Char8Value") {
    return ::Smp::PrimitiveTypeKind::PTK_Char8;
  }
  if (type == "DateTimeValue") {
    return ::Smp::PrimitiveTypeKind::PTK_DateTime;
  }
  if (type == "DurationValue") {
    return ::Smp::PrimitiveTypeKind::PTK_Duration;
  }
  if (type == "EnumerationValue" || type == "Int32Value") {
    return ::Smp::PrimitiveTypeKind::PTK_Int32;
  }
  if (type == "Float32Value") {
    return ::Smp::PrimitiveTypeKind::PTK_Float32;
  }
  if (type == "Float64Value") {
    return ::Smp::PrimitiveTypeKind::PTK_Float64;
  }
  if (type == "Int8Value") {
    return ::Smp::PrimitiveTypeKind::PTK_Int8;
  }
  if (type == "Int16Value") {
    return ::Smp::PrimitiveTypeKind::PTK_Int16;
  }
  if (type == "Int64Value") {
    return ::Smp::PrimitiveTypeKind::PTK_Int64;
  }
  if (type == "String8Value") {
    return ::Smp::PrimitiveTypeKind::PTK_String8;
  }
  if (type == "UInt8Value") {
    return ::Smp::PrimitiveTypeKind::PTK_UInt8;
  }
  if (type == "UInt16Value") {
    return ::Smp::PrimitiveTypeKind::PTK_UInt16;
  }
  if (type == "UInt32Value") {
    return ::Smp::PrimitiveTypeKind::PTK_UInt32;
  }
  if (type == "UInt64Value") {
    return ::Smp::PrimitiveTypeKind::PTK_UInt64;
  }
  return std::nullopt;
}

void ValidateSimpleValueType(std::string_view valueType,
                             ::Smp::PrimitiveTypeKind expectedPrimitiveType,
                             const ::Smp::Publication::IType *expectedType,
                             const SourceLocation &source) {
  if (!expectedType) {
    throw std::invalid_argument(Location(source) +
                                ": target has no published type");
  }
  const auto primitiveType = PrimitiveTypeOfSimpleValue(valueType);
  if (!primitiveType) {
    throw std::invalid_argument(Location(source) +
                                ": unsupported simple value type '" +
                                std::string{valueType} + "'");
  }
  if (*primitiveType != expectedPrimitiveType) {
    throw std::invalid_argument(
        Location(source) +
        ": value primitive type does not match the published target type");
  }
  const bool enumerationValue = valueType == "EnumerationValue";
  const bool enumerationTarget =
      dynamic_cast<const ::Smp::Publication::IEnumerationType *>(
          expectedType) != nullptr;
  if (enumerationValue != enumerationTarget) {
    throw std::invalid_argument(
        Location(source) +
        ": EnumerationValue and Int32Value are not interchangeable");
  }
}

::Smp::AnySimple ParseSimpleValue(const Element &element,
                                  std::string_view inferredType = {}) {
  auto type = TypeName(element);
  if (type.empty()) {
    type = inferredType;
  } else if (!inferredType.empty() && type != inferredType) {
    throw std::invalid_argument(Location(element.source) +
                                ": explicit simple value type '" + type +
                                "' does not match the enclosing type '" +
                                std::string{inferredType} + "'");
  }
  if (type.empty()) {
    throw std::invalid_argument(Location(element.source) +
                                ": a simple value requires xsi:type");
  }
  const auto *valueAttribute = element.FindAttribute("Value");
  if (!valueAttribute) {
    throw std::invalid_argument(Location(element.source) +
                                ": a simple value requires Value");
  }
  const auto &rawValue = valueAttribute->value;
  const auto value = TrimXmlWhitespace(rawValue);
  if (type == "BoolValue") {
    if (value == "true" || value == "1") {
      return {::Smp::PrimitiveTypeKind::PTK_Bool, true};
    }
    if (value == "false" || value == "0") {
      return {::Smp::PrimitiveTypeKind::PTK_Bool, false};
    }
    throw std::invalid_argument(Location(element.source) +
                                ": invalid boolean value '" +
                                std::string{value} + "'");
  }
  if (type == "Char8Value") {
    if (rawValue.size() != 1U) {
      throw std::invalid_argument(Location(element.source) +
                                  ": Char8Value must contain one byte");
    }
    return {::Smp::PrimitiveTypeKind::PTK_Char8, rawValue.front()};
  }
  if (type == "String8Value") {
    return {::Smp::PrimitiveTypeKind::PTK_String8, rawValue.c_str()};
  }
  if (type == "Int8Value") {
    return {::Smp::PrimitiveTypeKind::PTK_Int8,
            ParseInteger<::Smp::Int8>(value, element)};
  }
  if (type == "Int16Value") {
    return {::Smp::PrimitiveTypeKind::PTK_Int16,
            ParseInteger<::Smp::Int16>(value, element)};
  }
  if (type == "Int32Value" || type == "EnumerationValue") {
    return {::Smp::PrimitiveTypeKind::PTK_Int32,
            ParseInteger<::Smp::Int32>(value, element)};
  }
  if (type == "Int64Value") {
    return {::Smp::PrimitiveTypeKind::PTK_Int64,
            ParseInteger<::Smp::Int64>(value, element)};
  }
  if (type == "UInt8Value") {
    return {::Smp::PrimitiveTypeKind::PTK_UInt8,
            ParseInteger<::Smp::UInt8>(value, element)};
  }
  if (type == "UInt16Value") {
    return {::Smp::PrimitiveTypeKind::PTK_UInt16,
            ParseInteger<::Smp::UInt16>(value, element)};
  }
  if (type == "UInt32Value") {
    return {::Smp::PrimitiveTypeKind::PTK_UInt32,
            ParseInteger<::Smp::UInt32>(value, element)};
  }
  if (type == "UInt64Value") {
    return {::Smp::PrimitiveTypeKind::PTK_UInt64,
            ParseInteger<::Smp::UInt64>(value, element)};
  }
  if (type == "Float32Value") {
    return {::Smp::PrimitiveTypeKind::PTK_Float32,
            ParseFloat<::Smp::Float32>(value, element)};
  }
  if (type == "Float64Value") {
    return {::Smp::PrimitiveTypeKind::PTK_Float64,
            ParseFloat<::Smp::Float64>(value, element)};
  }
  if (type == "DurationValue") {
    return {::Smp::PrimitiveTypeKind::PTK_Duration,
            ParseIsoDuration(value, element)};
  }
  if (type == "DateTimeValue") {
    return {::Smp::PrimitiveTypeKind::PTK_DateTime,
            ParseDateTime(value, element)};
  }
  throw std::invalid_argument(Location(element.source) +
                              ": unsupported simple value type '" + type + "'");
}

::Smp::AnySimple
ParseSimpleValueFor(const Element &element,
                    ::Smp::PrimitiveTypeKind expectedPrimitiveType,
                    const ::Smp::Publication::IType *expectedType,
                    std::string_view inferredType = {}) {
  auto valueType = TypeName(element);
  if (valueType.empty()) {
    valueType = inferredType;
  }
  ValidateSimpleValueType(valueType, expectedPrimitiveType, expectedType,
                          element.source);
  auto value = ParseSimpleValue(element, inferredType);
  if (!::Xsmp::Publication::Request::isValid(expectedType, value)) {
    throw std::invalid_argument(Location(element.source) +
                                ": value is outside the constraints of the "
                                "published target type");
  }
  return value;
}

bool IsUuid(std::string_view value) {
  return value.size() == 36U && value[8] == '-' && value[13] == '-' &&
         value[18] == '-' && value[23] == '-';
}

std::string DescriptionOf(const Element &element) {
  return ChildText(element, "Description");
}

std::string JoinPath(std::string_view parent, std::string_view child) {
  if (child.empty() || child == ".") {
    return std::string{parent};
  }
  if (!child.empty() && child.front() == '/') {
    return std::string{child.substr(1U)};
  }
  if (parent.empty() || parent == ".") {
    return std::string{child};
  }
  std::string result{parent};
  if (result.back() != '/' && result.back() != '.') {
    result.push_back('/');
  }
  result.append(child);
  return result;
}

std::string JoinPrefixedPath(std::string_view prefix, std::string_view path) {
  while (!path.empty() && path.front() == '/') {
    path.remove_prefix(1U);
  }
  return JoinPath(prefix, path);
}

::Smp::IObject *Resolve(::Smp::IObject *root, std::string_view path) {
  if (!root) {
    return nullptr;
  }
  if (path.empty() || path == "." || path == "/") {
    return root;
  }
  std::string owned{path};
  while (!owned.empty() && owned.front() == '/') {
    owned.erase(owned.begin());
  }
  return owned.empty() ? root : ::Xsmp::Helper::Resolve(root, owned.c_str());
}

bool IsAbsolutePath(std::string_view path) {
  return !path.empty() && path.front() == '/';
}

bool IsWithin(const ::Smp::IObject *root, const ::Smp::IObject *object) {
  for (auto *current = object; current; current = current->GetParent()) {
    if (current == root) {
      return true;
    }
  }
  return false;
}

::Smp::IObject *ResolveScoped(::Smp::IObject *root, std::string_view path) {
  auto *result = Resolve(root, path);
  return result && IsWithin(root, result) ? result : nullptr;
}

bool HasUriScheme(std::string_view value) {
  const auto separator = value.find(':');
  if (separator == std::string_view::npos || separator == 0U) {
    return false;
  }
#if defined(_WIN32)
  if (separator == 1U &&
      std::isalpha(static_cast<unsigned char>(value.front())) != 0) {
    return false;
  }
#endif
  return std::all_of(
      value.begin(), value.begin() + separator, [](unsigned char character) {
        return std::isalnum(character) != 0 || character == '+' ||
               character == '-' || character == '.';
      });
}

::Smp::IComponent *OwningComponent(::Smp::IObject *object) {
  return ::Xsmp::Helper::GetParentOfType<::Smp::IComponent>(object);
}

void RegisterLink(::Xsmp::Simulator &simulator, ::Smp::IObject *source,
                  ::Smp::IObject *target) {
  auto *registry = simulator.GetLinkRegistry();
  auto *sourceComponent = OwningComponent(source);
  auto *targetComponent = OwningComponent(target);
  if (registry && sourceComponent && targetComponent &&
      sourceComponent != targetComponent) {
    registry->AddLink(sourceComponent, targetComponent);
  }
}

void ApplyFieldValue(::Smp::IField &field, const Element &value);

void ApplyArrayValue(::Smp::IArrayField &field, const Element &value) {
  ::Smp::UInt64 index = 0U;
  for (const auto &item : value.children) {
    if (item.name.localName != "ItemValue") {
      continue;
    }
    if (index >= field.GetSize()) {
      throw std::invalid_argument(Location(item.source) +
                                  ": too many array items");
    }
    auto *target = field.GetItem(index++);
    if (!target) {
      throw std::invalid_argument(Location(item.source) +
                                  ": array item does not exist");
    }
    ApplyFieldValue(*target, item);
  }
}

void ApplySimpleArrayValue(::Smp::ISimpleArrayField &field,
                           const Element &value) {
  constexpr std::string_view ARRAY_VALUE_SUFFIX{"ArrayValue"};
  const auto arrayValueType = TypeName(value);
  if (arrayValueType.size() <= ARRAY_VALUE_SUFFIX.size() ||
      arrayValueType.compare(arrayValueType.size() - ARRAY_VALUE_SUFFIX.size(),
                             ARRAY_VALUE_SUFFIX.size(),
                             ARRAY_VALUE_SUFFIX) != 0) {
    throw std::invalid_argument(Location(value.source) +
                                ": invalid simple array value type '" +
                                arrayValueType + "'");
  }
  auto itemValueType = arrayValueType.substr(0U, arrayValueType.size() -
                                                     ARRAY_VALUE_SUFFIX.size());
  itemValueType += "Value";

  const auto *arrayType =
      dynamic_cast<const ::Smp::Publication::IArrayType *>(field.GetType());
  const auto *itemType = arrayType ? arrayType->GetItemType() : nullptr;
  if (!itemType) {
    throw std::invalid_argument(Location(value.source) +
                                ": simple array field has no published item "
                                "type");
  }
  ValidateSimpleValueType(itemValueType, itemType->GetPrimitiveTypeKind(),
                          itemType, value.source);

  ::Smp::UInt64 index = 0U;
  if (const auto *start = Child(value, "StartIndex")) {
    index = ParseInteger<::Smp::UInt64>(TrimXmlWhitespace(start->TrimmedText()),
                                        *start);
  }
  for (const auto &item : value.children) {
    if (item.name.localName != "ItemValue") {
      continue;
    }
    if (index >= field.GetSize()) {
      throw std::invalid_argument(Location(item.source) +
                                  ": simple array item is out of range");
    }
    field.SetValue(index++,
                   ParseSimpleValueFor(item, itemType->GetPrimitiveTypeKind(),
                                       itemType, itemValueType));
  }
}

void ApplyStructureValue(::Smp::IStructureField &field, const Element &value) {
  for (const auto &memberValue : value.children) {
    if (memberValue.name.localName != "FieldValue") {
      continue;
    }
    const auto memberName = AttributeValue(memberValue, "Field");
    auto *member = field.GetField(memberName.c_str());
    if (!member) {
      throw std::invalid_argument(Location(memberValue.source) +
                                  ": structure field '" + memberName +
                                  "' does not exist");
    }
    ApplyFieldValue(*member, memberValue);
  }
}

void ApplyFieldValue(::Smp::IField &field, const Element &value) {
  const auto type = TypeName(value);
  if (type == "StructureValue") {
    auto *structure = dynamic_cast<::Smp::IStructureField *>(&field);
    if (!structure) {
      throw std::invalid_argument(Location(value.source) +
                                  ": value is a structure but field is not");
    }
    ApplyStructureValue(*structure, value);
    return;
  }
  if (type == "ArrayValue") {
    auto *array = dynamic_cast<::Smp::IArrayField *>(&field);
    if (!array) {
      throw std::invalid_argument(Location(value.source) +
                                  ": value is an array but field is not");
    }
    ApplyArrayValue(*array, value);
    return;
  }
  if (type.size() > std::string_view{"ArrayValue"}.size() &&
      type.compare(type.size() - std::string_view{"ArrayValue"}.size(),
                   std::string_view{"ArrayValue"}.size(), "ArrayValue") == 0) {
    auto *array = dynamic_cast<::Smp::ISimpleArrayField *>(&field);
    if (!array) {
      throw std::invalid_argument(Location(value.source) +
                                  ": value is a simple array but field is not");
    }
    ApplySimpleArrayValue(*array, value);
    return;
  }
  auto *simple = dynamic_cast<::Smp::ISimpleField *>(&field);
  if (!simple) {
    throw std::invalid_argument(Location(value.source) +
                                ": simple value targets a complex field");
  }
  simple->SetValue(ParseSimpleValueFor(value, simple->GetPrimitiveTypeKind(),
                                       simple->GetType()));
}

std::string Substitute(std::string_view input,
                       const std::map<std::string, std::string> &arguments,
                       const SourceLocation &source) {
  std::string output;
  output.reserve(input.size());
  for (std::size_t index = 0U; index < input.size();) {
    if (input[index] == '{') {
      if (index + 1U < input.size() && input[index + 1U] == '{') {
        output.push_back('{');
        index += 2U;
        continue;
      }
      const auto close = input.find('}', index + 1U);
      if (close == std::string_view::npos) {
        throw std::invalid_argument(Location(source) +
                                    ": unterminated template placeholder");
      }
      const auto name =
          std::string{input.substr(index + 1U, close - index - 1U)};
      const auto found = arguments.find(name);
      if (found == arguments.end()) {
        throw std::invalid_argument(Location(source) +
                                    ": no value supplied for template '" +
                                    name + "'");
      }
      output.append(found->second);
      index = close + 1U;
      continue;
    }
    if (input[index] == '}') {
      if (index + 1U < input.size() && input[index + 1U] == '}') {
        output.push_back('}');
        index += 2U;
        continue;
      }
      throw std::invalid_argument(Location(source) +
                                  ": unexpected closing template brace");
    }
    output.push_back(input[index++]);
  }
  return output;
}

struct ProvidedArgument final {
  std::string value;
  std::string type;
  SourceLocation source;
};

using ProvidedArguments = std::map<std::string, ProvidedArgument>;

std::string TemplateArgumentType(const Element &argument) {
  if (!argument.xsiType ||
      argument.xsiType->namespaceUri != Xml::ASSEMBLY_NAMESPACE ||
      (argument.xsiType->localName != "Int32Argument" &&
       argument.xsiType->localName != "StringArgument")) {
    throw std::invalid_argument(
        Location(argument.source) +
        ": template argument requires Assembly:Int32Argument or "
        "Assembly:StringArgument xsi:type");
  }
  return argument.xsiType->localName;
}

std::map<std::string, std::string>
TemplateArguments(const Element &root, const ProvidedArguments &provided) {
  std::map<std::string, std::string> result;
  std::set<std::string> parameterNames;
  for (const auto &parameter : root.children) {
    if (parameter.name.localName != "Parameter") {
      continue;
    }
    const auto name = AttributeValue(parameter, "Name");
    if (!parameterNames.emplace(name).second) {
      throw std::invalid_argument(Location(parameter.source) +
                                  ": duplicate template parameter '" + name +
                                  "'");
    }
    const auto type = TemplateArgumentType(parameter);
    const auto supplied = provided.find(name);
    if (supplied != provided.end()) {
      if (supplied->second.type != type) {
        throw std::invalid_argument(Location(supplied->second.source) +
                                    ": template argument '" + name +
                                    "' has type '" + supplied->second.type +
                                    "', expected '" + type + "'");
      }
      result.emplace(name, supplied->second.value);
      continue;
    }
    if (const auto *value = parameter.FindAttribute("Value")) {
      result.emplace(name, value->value);
      continue;
    }
    throw std::invalid_argument(Location(parameter.source) +
                                ": template parameter '" + name +
                                "' has no value");
  }
  for (const auto &[name, argument] : provided) {
    if (result.find(name) == result.end()) {
      throw std::invalid_argument(Location(argument.source) +
                                  ": unknown template argument '" + name + "'");
    }
  }
  return result;
}

ProvidedArguments InstanceArguments(const Element &instance) {
  ProvidedArguments result;
  for (const auto &argument : instance.children) {
    if (argument.name.localName == "Argument") {
      const auto name = AttributeValue(argument, "Name");
      const auto *value = argument.FindAttribute("Value");
      if (!value) {
        throw std::invalid_argument(Location(argument.source) +
                                    ": template argument '" + name +
                                    "' has no Value");
      }
      ProvidedArgument provided{value->value, TemplateArgumentType(argument),
                                argument.source};
      if (!result.emplace(name, std::move(provided)).second) {
        throw std::invalid_argument(Location(argument.source) +
                                    ": duplicate template argument '" + name +
                                    "'");
      }
    }
  }
  return result;
}

} // namespace

class Loader::Impl final {
public:
  explicit Impl(::Xsmp::Simulator &simulator_) : simulator{simulator_} {}

  ~Impl() = default;

  enum class ActionKind { Link, FieldValue, Invocation, GlobalEvent };

  enum class ActionOrigin {
    ModelLocal,
    AssemblyConfiguration,
    ExternalConfiguration,
    AssemblyInstanceConfiguration
  };

  struct Action {
    ActionKind kind;
    ActionOrigin origin;
    std::size_t batch{};
    Element element;
    ::Smp::IObject *scopeRoot{};
    ::Smp::IObject *absoluteRoot{};
    std::string componentPath;
    std::size_t sequence{};
  };

  struct RootDuplicateName final {
    std::exception_ptr exception;
  };

  struct TaskPlan final {
    std::vector<std::function<void()>> activities;

    void Execute() const {
      for (const auto &activity : activities) {
        activity();
      }
    }
  };

  enum class ScheduleTimeKind { Simulation, Mission, Epoch, Zulu };

  struct EventDefinition final {
    Element element;
    std::string name;
    std::shared_ptr<TaskPlan> task;
    ScheduleTimeKind timeKind{ScheduleTimeKind::Simulation};
    ::Smp::Duration relativeTime{};
    ::Smp::DateTime absoluteTime{};
    ::Smp::Duration cycleTime{};
    ::Smp::Int64 repeatCount{};
    bool globalTriggered{};
    std::string startEvent;
    std::string stopEvent;
  };

  struct GlobalTriggeredState final {
    ::Smp::Services::IScheduler *scheduler{};
    ::Smp::Services::ITimeKeeper *timeKeeper{};
    const ::Smp::IEntryPoint *task{};
    ScheduleTimeKind timeKind{ScheduleTimeKind::Simulation};
    ::Smp::Duration delay{};
    ::Smp::Duration cycleTime{};
    ::Smp::Int64 repeatCount{};
    ::Smp::Services::EventId scheduledEvent{-1};
    bool active{};
    std::mutex mutex;

    static ::Smp::Int64 AddChecked(::Smp::Int64 left, ::Smp::Int64 right) {
      if ((right > 0 &&
           left > std::numeric_limits<::Smp::Int64>::max() - right) ||
          (right < 0 &&
           left < std::numeric_limits<::Smp::Int64>::lowest() - right)) {
        throw std::overflow_error(
            "schedule event time is outside the SMP range");
      }
      return left + right;
    }

    void Start() {
      const std::scoped_lock lock{mutex};
      if (active) {
        return;
      }
      switch (timeKind) {
      case ScheduleTimeKind::Simulation:
        scheduledEvent = scheduler->AddSimulationTimeEvent(
            task, delay, cycleTime, repeatCount);
        break;
      case ScheduleTimeKind::Mission:
        scheduledEvent = scheduler->AddMissionTimeEvent(
            task, AddChecked(timeKeeper->GetMissionTime(), delay), cycleTime,
            repeatCount);
        break;
      case ScheduleTimeKind::Epoch:
        scheduledEvent = scheduler->AddEpochTimeEvent(
            task, AddChecked(timeKeeper->GetEpochTime(), delay), cycleTime,
            repeatCount);
        break;
      case ScheduleTimeKind::Zulu:
        scheduledEvent = scheduler->AddRelativeZuluTimeEvent(
            task, delay, cycleTime, repeatCount);
        break;
      }
      active = true;
    }

    void Stop() noexcept {
      const std::scoped_lock lock{mutex};
      if (!active) {
        return;
      }
      try {
        if (scheduler->IsEventScheduled(scheduledEvent)) {
          scheduler->RemoveEvent(scheduledEvent);
        }
      } catch (...) {
        // Stopping an already completed one-shot event is idempotent.
      }
      scheduledEvent = -1;
      active = false;
    }
  };

  struct PendingSchedule final {
    std::filesystem::path path;
    Element root;
  };

  struct ScheduleSubscription final {
    ::Smp::Services::EventId event{};
    const ::Smp::IEntryPoint *entryPoint{};
  };

  ::Xsmp::Simulator &simulator;
  std::vector<Action> links;
  std::vector<Action> fieldValues;
  std::vector<Action> invocations;
  std::vector<Action> globalEvents;
  std::set<std::filesystem::path> assemblyStack;
  std::set<std::filesystem::path> configurationStack;
  bool scheduleLoaded{};
  std::optional<PendingSchedule> pendingSchedule;
  // RemoveEvent() cannot synchronously destroy an entry point that is already
  // executing (notably on the scheduler's Zulu thread). Detached entries stay
  // here until the simulator services have been destroyed.
  std::vector<std::unique_ptr<::Xsmp::EntryPoint>> scheduleEntryPoints;
  std::vector<::Smp::Services::EventId> scheduledEvents;
  std::vector<ScheduleSubscription> scheduleSubscriptions;
  std::vector<std::shared_ptr<GlobalTriggeredState>> globalTriggeredEvents;
  bool shutdown{};
  std::size_t currentBatch{};
  std::size_t nextBatch{1U};
  std::size_t nextSequence{};

  [[noreturn]] void Invalid(const std::filesystem::path &path,
                            std::string message) const {
    ::Xsmp::Exception::throwInvalidFile(&simulator, path.string().c_str(),
                                        message);
  }

  [[noreturn]] void Invalid(const Element &element, std::string message) const {
    Invalid(element.source.file,
            Location(element.source) + ": " + std::move(message));
  }

  void ValidateLevel2Path(const Element &element, std::string_view path) const {
    if (path.find("..") != std::string_view::npos) {
      Invalid(element,
              "Level 2 paths must not contain a parent ('..') segment");
    }
  }

  void CheckState() const {
    const auto state = simulator.GetState();
    if (state != ::Smp::SimulatorStateKind::SSK_Building &&
        state != ::Smp::SimulatorStateKind::SSK_Standby) {
      ::Xsmp::Exception::throwInvalidSimulatorState(&simulator, state);
    }
  }

  std::filesystem::path CheckFile(::Smp::String8 value) const {
    const std::filesystem::path path{value ? value : ""};
    std::error_code error;
    if (path.empty() || !std::filesystem::is_regular_file(path, error) ||
        error) {
      ::Xsmp::Exception::throwFileNotFound(
          &simulator, value ? value : "",
          path.empty() ? "No file name given." : "File does not exist.");
    }
    return path;
  }

  Xml::ReadOptions Options(DocumentKind kind) const {
    Xml::ReadOptions result;
    Xml::SchemaBundle schemas;
#if defined(XSMP_L1_SCHEMA_DIR)
    if (kind == DocumentKind::Configuration) {
      schemas.rootDirectory = XSMP_L1_SCHEMA_DIR;
#if defined(XSMP_L1_SCHEMA_INSTALL_DIR)
      if (!std::filesystem::is_directory(schemas.rootDirectory)) {
        schemas.rootDirectory = XSMP_L1_SCHEMA_INSTALL_DIR;
      }
#endif
      result.schemas = std::move(schemas);
      return result;
    }
#elif defined(XSMP_L1_SCHEMA_INSTALL_DIR)
    if (kind == DocumentKind::Configuration) {
      schemas.rootDirectory = XSMP_L1_SCHEMA_INSTALL_DIR;
      result.schemas = std::move(schemas);
      return result;
    }
#endif
#if defined(XSMP_L2_SCHEMA_DIR)
    if (kind != DocumentKind::Configuration) {
      schemas.rootDirectory = XSMP_L2_SCHEMA_DIR;
#if defined(XSMP_L2_SCHEMA_INSTALL_DIR)
      if (!std::filesystem::is_directory(schemas.rootDirectory)) {
        schemas.rootDirectory = XSMP_L2_SCHEMA_INSTALL_DIR;
      }
#endif
      result.schemas = std::move(schemas);
      return result;
    }
#elif defined(XSMP_L2_SCHEMA_INSTALL_DIR)
    if (kind != DocumentKind::Configuration) {
      schemas.rootDirectory = XSMP_L2_SCHEMA_INSTALL_DIR;
      result.schemas = std::move(schemas);
      return result;
    }
#endif
    return result;
  }

  Document Read(const std::filesystem::path &path, DocumentKind expected,
                const Xml::ReadOptions &options) {
    const auto expectedExtension = FileExtension(expected);
    if (path.extension().string() != std::string{expectedExtension}) {
      Invalid(path, "expected '" + std::string{expectedExtension} +
                        "' file extension");
    }
    const auto result = Xml::ReadFile(path, options);
    if (!result) {
      std::ostringstream message;
      for (const auto &diagnostic : result.diagnostics) {
        if (message.tellp() > 0) {
          message << '\n';
        }
        message << Location(diagnostic.source) << ": " << diagnostic.message;
      }
      Invalid(path,
              message.str().empty() ? "Invalid XML document." : message.str());
    }
    if (result.document->kind != expected) {
      Invalid(result.document->root,
              std::string{"expected "} + Xml::ToString(expected) +
                  " document, found " + Xml::ToString(result.document->kind));
    }
    return *result.document;
  }

  Document Read(const std::filesystem::path &path, DocumentKind expected) {
    return Read(path, expected, Options(expected));
  }

  void ValidateUniqueNamedChildren(const Element &parent,
                                   std::string_view childType) {
    std::set<std::string> names;
    std::set<std::string> identifiers;
    for (const auto &child : parent.children) {
      if (child.name.localName != childType) {
        continue;
      }
      const auto name = AttributeValue(child, "Name");
      if (!names.emplace(name).second) {
        Invalid(child,
                "duplicate " + std::string{childType} + " name '" + name + "'");
      }
      if (const auto *identifier = child.FindAttribute("Id");
          identifier && !identifiers.emplace(identifier->value).second) {
        Invalid(child, "duplicate " + std::string{childType} + " Id '" +
                           identifier->value + "'");
      }
    }
  }

  void ValidateScheduleStructure(const Element &schedule) {
    ValidateUniqueNamedChildren(schedule, "Task");
    ValidateUniqueNamedChildren(schedule, "Event");
    for (const auto &task : schedule.children) {
      if (task.name.localName == "Task") {
        ValidateUniqueNamedChildren(task, "Activity");
      }
    }
  }

  Document ReadTemplated(const std::filesystem::path &path,
                         DocumentKind expected,
                         const ProvidedArguments &providedArguments) {
    auto discoveryOptions = Options(expected);
    discoveryOptions.schemas.reset();
    auto declarations = Read(path, expected, discoveryOptions);

    std::map<std::string, std::string> arguments;
    try {
      arguments = TemplateArguments(declarations.root, providedArguments);
    } catch (const ::Smp::InvalidFile &) {
      throw;
    } catch (const std::exception &error) {
      Invalid(declarations.root, error.what());
    }

    auto validatedOptions = Options(expected);
    validatedOptions.valueTransformer =
        [arguments = std::move(arguments)](std::string_view value,
                                           const SourceLocation &source) {
          return Substitute(value, arguments, source);
        };
    auto document = Read(path, expected, validatedOptions);
    if (expected == DocumentKind::Schedule) {
      ValidateScheduleStructure(document.root);
    }
    return document;
  }

  ::Smp::IComposite *AssemblyParentFor(::Smp::String8 parentPath) const {
    if (parentPath && parentPath[0] != '\0') {
      if (auto *parent = ::Xsmp::Helper::Resolve(&simulator, parentPath)) {
        if (auto *composite = dynamic_cast<::Smp::IComposite *>(parent)) {
          return composite;
        }
      }
    }
    return &simulator;
  }

  ::Smp::IObject *ComponentParentFor(::Smp::String8 parentPath) const {
    if (parentPath && parentPath[0] != '\0') {
      if (auto *parent = ::Xsmp::Helper::Resolve(&simulator, parentPath);
          dynamic_cast<::Smp::IComponent *>(parent)) {
        return parent;
      }
    }
    return &simulator;
  }

  ::Smp::IFactory *FactoryFor(const Element &model) const {
    const auto implementation = AttributeValue(model, "Implementation");
    if (implementation.empty()) {
      return nullptr;
    }
    if (IsUuid(implementation)) {
      try {
        return simulator.GetFactory(::Smp::Uuid{implementation.c_str()});
      } catch (const std::exception &) {
        return nullptr;
      }
    }
    const auto *factories = simulator.GetFactories();
    if (!factories) {
      return nullptr;
    }
    for (auto *factory : *factories) {
      if (factory && implementation == factory->GetTypeName()) {
        return factory;
      }
    }
    return nullptr;
  }

  ::Smp::IModel *CreateModel(const Element &model, ::Smp::IComposite &parent,
                             std::string_view container,
                             std::string_view nameOverride = {}) {
    auto *factory = FactoryFor(model);
    if (!factory) {
      Invalid(model, "no registered factory matches implementation '" +
                         AttributeValue(model, "Implementation") + "'");
    }
    const auto name = nameOverride.empty() ? AttributeValue(model, "Name")
                                           : std::string{nameOverride};
    const auto description = DescriptionOf(model);
    std::unique_ptr<::Smp::IComponent> component{simulator.CreateInstance(
        factory->GetUuid(), name.c_str(), description.c_str(), &parent)};
    if (!component) {
      Invalid(model, "factory failed to create model instance '" + name + "'");
    }
    auto *instance = dynamic_cast<::Smp::IModel *>(component.get());
    if (!instance) {
      Invalid(model, "factory for '" + AttributeValue(model, "Implementation") +
                         "' did not create an IModel");
    }

    if (&parent == static_cast<::Smp::IComposite *>(&simulator)) {
      simulator.AddModel(instance);
    } else {
      auto *target = parent.GetContainer(std::string{container}.c_str());
      if (!target) {
        Invalid(model, "container '" + std::string{container} +
                           "' does not exist on parent '" +
                           ::Xsmp::Helper::GetPath(&parent) + "'");
      }
      target->AddComponent(instance);
    }
    component.release();
    return instance;
  }

  static bool HasChildNamed(const ::Smp::IComposite &parent,
                            std::string_view name) {
    const std::string ownedName{name};
    if (const auto *containers = parent.GetContainers()) {
      for (const auto *candidate : *containers) {
        if (candidate && candidate->GetComponent(ownedName.c_str())) {
          return true;
        }
      }
    }
    const auto *component = dynamic_cast<const ::Smp::IComponent *>(&parent);
    return component && component->GetChild(ownedName.c_str());
  }

  void Queue(std::vector<Action> &queue, ActionKind kind, ActionOrigin origin,
             const Element &element, ::Smp::IObject &scopeRoot,
             std::string componentPath,
             ::Smp::IObject *absoluteRoot = nullptr) {
    queue.push_back({kind, origin, currentBatch, element, &scopeRoot,
                     absoluteRoot ? absoluteRoot : &scopeRoot,
                     std::move(componentPath), nextSequence++});
  }

  ::Smp::IObject *ResolveActionPath(const Action &action,
                                    std::string_view path) const {
    auto *root = IsAbsolutePath(path) ? action.absoluteRoot : action.scopeRoot;
    return ResolveScoped(root, path);
  }

  void QueueConfigurationElement(const Element &configuration,
                                 ::Smp::IObject &scopeRoot,
                                 std::string componentPath,
                                 ActionOrigin origin) {
    for (const auto &child : configuration.children) {
      if (child.name.localName == "FieldValue") {
        Queue(fieldValues, ActionKind::FieldValue, origin, child, scopeRoot,
              componentPath);
      }
    }
  }

  void ValidateAssemblyConfiguration(const Element &configuration,
                                     bool allowAbsolutePath) const {
    const auto path = AttributeValue(configuration, "InstancePath", ".");
    ValidateLevel2Path(configuration, path);
    if (IsAbsolutePath(path) && !allowAbsolutePath) {
      Invalid(configuration,
              "a nested ComponentConfiguration path must be relative");
    }
  }

  void QueueAssemblyConfiguration(const Element &configuration,
                                  ::Smp::IObject &scopeRoot,
                                  ActionOrigin origin,
                                  ::Smp::IObject *absoluteRoot = nullptr,
                                  bool allowAbsolutePath = false) {
    const auto path = AttributeValue(configuration, "InstancePath", ".");
    ValidateAssemblyConfiguration(configuration, allowAbsolutePath);
    for (const auto &child : configuration.children) {
      if (child.name.localName == "FieldValue") {
        Queue(fieldValues, ActionKind::FieldValue, origin, child, scopeRoot,
              path, absoluteRoot);
      } else if (child.name.localName == "Invocation") {
        Queue(invocations, ActionKind::Invocation, origin, child, scopeRoot,
              path, absoluteRoot);
      } else if (child.name.localName == "GlobalEventHandler") {
        Queue(globalEvents, ActionKind::GlobalEvent, origin, child, scopeRoot,
              path, absoluteRoot);
      }
    }
  }

  void QueueLinkBaseComponent(const Element &component,
                              ::Smp::IObject &scopeRoot, std::string parentPath,
                              ActionOrigin origin) {
    const auto path = AttributeValue(component, "Path", ".");
    ValidateLevel2Path(component, path);
    auto currentPath = JoinPath(parentPath, path);
    const auto hasLink = std::any_of(
        component.children.begin(), component.children.end(),
        [](const auto &child) { return child.name.localName == "Link"; });
    if (!hasLink) {
      Invalid(component,
              "a Component Link Base must contain at least one Link");
    }
    for (const auto &child : component.children) {
      if (child.name.localName == "Link") {
        ValidateLevel2Path(child, ChildText(child, "OwnerPath", "."));
        ValidateLevel2Path(child, ChildText(child, "ClientPath", "."));
        Queue(links, ActionKind::Link, origin, child, scopeRoot, currentPath);
      }
    }
    for (const auto &child : component.children) {
      if (child.name.localName == "Component") {
        QueueLinkBaseComponent(child, scopeRoot, currentPath, origin);
      }
    }
  }

  void QueueConfigurationComponent(const Element &component,
                                   ::Smp::IObject &scopeRoot,
                                   std::string basePath, std::string parentPath,
                                   const std::filesystem::path &sourcePath,
                                   ActionOrigin origin) {
    const auto path = AttributeValue(component, "Path", ".");
    auto currentPath = IsAbsolutePath(path) ? JoinPrefixedPath(basePath, path)
                                            : JoinPath(parentPath, path);

    // Includes precede child components and local field values in the schema;
    // preserving that order lets the local configuration override included
    // values deterministically.
    for (const auto &child : component.children) {
      if (child.name.localName == "Include") {
        QueueConfigurationInclude(child, sourcePath, scopeRoot, currentPath,
                                  origin);
      }
    }
    for (const auto &child : component.children) {
      if (child.name.localName == "Component") {
        QueueConfigurationComponent(child, scopeRoot, basePath, currentPath,
                                    sourcePath, origin);
      }
    }
    QueueConfigurationElement(component, scopeRoot, currentPath, origin);
  }

  void QueueConfigurationInclude(const Element &include,
                                 const std::filesystem::path &sourcePath,
                                 ::Smp::IObject &scopeRoot,
                                 std::string parentPath, ActionOrigin origin) {
    const auto *reference = Child(include, "Configuration");
    const auto *href =
        reference
            ? reference->FindAttribute("href", "http://www.w3.org/1999/xlink")
            : nullptr;
    if (!href || href->value.empty()) {
      Invalid(include, "configuration include has no xlink:href");
    }

    const auto [filePart, fragment] = SplitReference(href->value);
    if (HasUriScheme(filePart) || filePart.rfind("//", 0U) == 0U ||
        filePart.find('?') != std::string::npos) {
      Invalid(include, "remote Configuration references are not supported");
    }
    const auto includePath =
        filePart.empty() ? sourcePath : sourcePath.parent_path() / filePart;
    const auto includePrefix =
        IsAbsolutePath(AttributeValue(include, "Path", "."))
            ? JoinPrefixedPath({}, AttributeValue(include, "Path", "."))
            : JoinPath(parentPath, AttributeValue(include, "Path", "."));
    QueueConfigurationFile(includePath, scopeRoot, origin, includePrefix,
                           fragment);
  }

  void QueueConfigurationFile(const std::filesystem::path &path,
                              ::Smp::IObject &scopeRoot, ActionOrigin origin,
                              std::string prefix = {},
                              std::string_view fragment = {}) {
    std::error_code error;
    const auto canonical = std::filesystem::weakly_canonical(path, error);
    const auto identity = error ? path.lexically_normal() : canonical;
    if (!configurationStack.emplace(identity).second) {
      Invalid(path, "cyclic Configuration include detected");
    }
    struct StackGuard final {
      std::set<std::filesystem::path> &stack;
      std::filesystem::path path;
      ~StackGuard() { stack.erase(path); }
    } guard{configurationStack, identity};

    auto document = Read(path, DocumentKind::Configuration);
    if (!fragment.empty() && fragment != AttributeValue(document.root, "Id") &&
        fragment != AttributeValue(document.root, "Name")) {
      Invalid(document.root, "Configuration reference fragment '" +
                                 std::string{fragment} +
                                 "' does not identify the document root");
    }

    for (const auto &child : document.root.children) {
      if (child.name.localName == "Include") {
        QueueConfigurationInclude(child, path, scopeRoot, prefix, origin);
      }
    }
    for (const auto &child : document.root.children) {
      if (child.name.localName == "Component") {
        QueueConfigurationComponent(child, scopeRoot, prefix, prefix, path,
                                    origin);
      }
    }
  }

  ::Smp::IModel *CreateAssembly(const std::filesystem::path &path,
                                ::Smp::IComposite &parent,
                                std::string_view container,
                                std::string_view rootName,
                                const ProvidedArguments &providedArguments,
                                bool exposeRootDuplicate = false) {
    std::error_code error;
    const auto canonical = std::filesystem::weakly_canonical(path, error);
    const auto identity = error ? path.lexically_normal() : canonical;
    if (!assemblyStack.emplace(identity).second) {
      Invalid(path, "cyclic Assembly reference detected");
    }
    struct StackGuard {
      std::set<std::filesystem::path> &stack;
      std::filesystem::path path;
      ~StackGuard() { stack.erase(path); }
    } guard{assemblyStack, identity};

    auto document =
        ReadTemplated(path, DocumentKind::Assembly, providedArguments);
    const auto *model = Child(document.root, "Model");
    if (!model) {
      Invalid(document.root, "assembly does not contain a root Model");
    }
    const bool topLevel =
        &parent == static_cast<::Smp::IComposite *>(&simulator);
    for (const auto &child : document.root.children) {
      if (child.name.localName == "ComponentConfiguration") {
        ValidateAssemblyConfiguration(child, topLevel);
      }
    }
    const auto requestedRootName = rootName.empty()
                                       ? AttributeValue(*model, "Name")
                                       : std::string{rootName};
    const bool rootNameConflict =
        exposeRootDuplicate && HasChildNamed(parent, requestedRootName);
    ::Smp::IModel *root{};
    try {
      root = CreateModel(*model, parent, container, rootName);
    } catch (const ::Smp::DuplicateName &) {
      if (rootNameConflict) {
        throw RootDuplicateName{std::current_exception()};
      }
      throw;
    }
    CreateModelChildren(*model, *root, *root, path.parent_path());

    // Model-local values have already been queued. Assembly-level component
    // configurations deliberately follow them to implement override order.
    for (const auto &child : document.root.children) {
      if (child.name.localName == "ComponentConfiguration") {
        QueueAssemblyConfiguration(
            child, *root, ActionOrigin::AssemblyConfiguration,
            topLevel ? static_cast<::Smp::IObject *>(&simulator)
                     : static_cast<::Smp::IObject *>(root),
            topLevel);
      }
    }
    return root;
  }

  void CreateModelChildren(const Element &model, ::Smp::IModel &instance,
                           ::Smp::IObject &scopeRoot,
                           const std::filesystem::path &documentDirectory) {
    auto *composite = dynamic_cast<::Smp::IComposite *>(&instance);
    for (const auto &child : model.children) {
      if (child.name.localName == "Model") {
        if (!composite) {
          Invalid(child, "parent model is not an IComposite");
        }
        const auto container = AttributeValue(child, "Container");
        auto *created = CreateModel(child, *composite, container);
        CreateModelChildren(child, *created, scopeRoot, documentDirectory);
      } else if (child.name.localName == "Assembly") {
        if (!composite) {
          Invalid(child, "parent model is not an IComposite");
        }
        const auto assemblyName = ChildText(child, "Assembly");
        if (assemblyName.empty()) {
          Invalid(child, "Assembly Instance has an empty Assembly path");
        }
        const auto instanceName = AttributeValue(child, "Name");
        const auto container = AttributeValue(child, "Container");
        auto *assemblyRoot =
            CreateAssembly(documentDirectory / assemblyName, *composite,
                           container, instanceName, InstanceArguments(child));

        if (const auto configuration = ChildText(child, "Configuration");
            !configuration.empty()) {
          QueueConfigurationFile(documentDirectory / configuration,
                                 *assemblyRoot,
                                 ActionOrigin::ExternalConfiguration);
        }
        if (const auto linkBase = ChildText(child, "LinkBase");
            !linkBase.empty()) {
          auto linkDocument =
              Read(documentDirectory / linkBase, DocumentKind::LinkBase);
          for (const auto &component : linkDocument.root.children) {
            if (component.name.localName == "Component") {
              QueueLinkBaseComponent(component, *assemblyRoot, {},
                                     ActionOrigin::ExternalConfiguration);
            }
          }
        }
        for (const auto &configuration : child.children) {
          if (configuration.name.localName == "ModelConfiguration") {
            QueueAssemblyConfiguration(
                configuration, *assemblyRoot,
                ActionOrigin::AssemblyInstanceConfiguration, assemblyRoot,
                false);
          }
        }
      } else if (child.name.localName == "Link") {
        ValidateLevel2Path(child, ChildText(child, "OwnerPath", "."));
        ValidateLevel2Path(child, ChildText(child, "ClientPath", "."));
        Queue(links, ActionKind::Link, ActionOrigin::ModelLocal, child,
              instance, ".", &simulator);
      } else if (child.name.localName == "FieldValue") {
        Queue(fieldValues, ActionKind::FieldValue, ActionOrigin::ModelLocal,
              child, instance, ".");
      } else if (child.name.localName == "Invocation") {
        Queue(invocations, ActionKind::Invocation, ActionOrigin::ModelLocal,
              child, instance, ".");
      } else if (child.name.localName == "GlobalEventHandler") {
        Queue(globalEvents, ActionKind::GlobalEvent, ActionOrigin::ModelLocal,
              child, instance, ".");
      }
    }
  }

  bool ApplyLink(const Action &action) {
    auto *context = ResolveActionPath(action, action.componentPath);
    if (!context) {
      return false;
    }
    if (!dynamic_cast<::Smp::IComponent *>(context)) {
      Invalid(action.element,
              "the containing Component Link Base path does not resolve to "
              "an IComponent");
    }
    const auto ownerPath = ChildText(action.element, "OwnerPath", ".");
    const auto clientPath = ChildText(action.element, "ClientPath", ".");
    const auto resolveEndpoint = [&](std::string_view path) {
      auto *root = IsAbsolutePath(path) ? action.absoluteRoot : context;
      auto *endpoint = Resolve(root, path);
      if (endpoint && !IsWithin(context, endpoint)) {
        Invalid(action.element, "Link endpoint path '" + std::string{path} +
                                    "' escapes its current model instance");
      }
      return endpoint;
    };
    auto *owner = resolveEndpoint(ownerPath);
    auto *client = resolveEndpoint(clientPath);
    if (!owner || !client) {
      return false;
    }
    if (!action.element.xsiType ||
        action.element.xsiType->namespaceUri != Xml::LINK_BASE_NAMESPACE) {
      Invalid(action.element, "link requires a LinkBase xsi:type");
    }
    const auto &type = action.element.xsiType->localName;
    if (type == "FieldLink") {
      auto *source = dynamic_cast<::Smp::IOutputField *>(owner);
      auto *target = dynamic_cast<::Smp::IField *>(client);
      if (!source || !target || !source->IsOutput() || !target->IsInput()) {
        Invalid(action.element,
                "FieldLink endpoints are not compatible output/input fields");
      }
      source->Connect(target);
      RegisterLink(simulator, owner, client);
      return true;
    }
    if (type == "EventLink") {
      auto *source = dynamic_cast<::Smp::IEventSource *>(owner);
      auto *target = dynamic_cast<::Smp::IEventSink *>(client);
      if (!source || !target) {
        Invalid(action.element,
                "EventLink endpoints are not an event source and sink");
      }
      source->Subscribe(target);
      RegisterLink(simulator, owner, client);
      return true;
    }
    if (type == "InterfaceLink") {
      auto *ownerComponent = dynamic_cast<::Smp::IComponent *>(owner);
      auto *clientComponent = dynamic_cast<::Smp::IComponent *>(client);
      auto *aggregate = dynamic_cast<::Smp::IAggregate *>(ownerComponent);
      const auto referenceName = ChildText(action.element, "Reference");
      auto *reference =
          aggregate ? aggregate->GetReference(referenceName.c_str()) : nullptr;
      if (!ownerComponent || !clientComponent || !reference) {
        Invalid(action.element,
                "InterfaceLink owner/reference/client is not compatible");
      }
      reference->AddComponent(clientComponent);
      RegisterLink(simulator, ownerComponent, clientComponent);
      if (const auto backName = ChildText(action.element, "BackReference");
          !backName.empty()) {
        auto *clientAggregate =
            dynamic_cast<::Smp::IAggregate *>(clientComponent);
        auto *backReference =
            clientAggregate ? clientAggregate->GetReference(backName.c_str())
                            : nullptr;
        if (!backReference) {
          Invalid(action.element, "InterfaceLink BackReference does not exist");
        }
        backReference->AddComponent(ownerComponent);
        RegisterLink(simulator, clientComponent, ownerComponent);
      }
      return true;
    }
    Invalid(action.element, "unsupported link type '" + type + "'");
  }

  bool ApplyField(const Action &action) {
    auto *component = dynamic_cast<::Smp::IComponent *>(
        ResolveActionPath(action, action.componentPath));
    if (!component) {
      return false;
    }
    const auto fieldName = AttributeValue(action.element, "Field");
    ValidateLevel2Path(action.element, fieldName);
    auto *field =
        dynamic_cast<::Smp::IField *>(ResolveScoped(component, fieldName));
    if (!field) {
      return false;
    }
    ApplyFieldValue(*field, action.element);
    return true;
  }

  void ValidateOperationInputSignature(
      const Element &element, const ::Smp::IOperation &operation,
      const std::set<std::string> &providedNames) const {
    const auto *publishedParameters = operation.GetParameters();
    if (!publishedParameters) {
      Invalid(element, "operation has no published parameter collection");
    }
    for (const auto *parameter : *publishedParameters) {
      if (!parameter || !parameter->GetName()) {
        Invalid(element, "operation has an invalid published parameter");
      }
      const auto direction = parameter->GetDirection();
      if ((direction == ::Smp::Publication::ParameterDirectionKind::PDK_In ||
           direction ==
               ::Smp::Publication::ParameterDirectionKind::PDK_InOut) &&
          providedNames.find(parameter->GetName()) == providedNames.end()) {
        Invalid(element, "missing input value for operation parameter '" +
                             std::string{parameter->GetName()} + "'");
      }
    }
  }

  bool ApplyInvocation(const Action &action) {
    auto *component = dynamic_cast<::Smp::IDynamicInvocation *>(
        ResolveActionPath(action, action.componentPath));
    if (!component) {
      return false;
    }
    if (!action.element.xsiType ||
        action.element.xsiType->namespaceUri != Xml::ASSEMBLY_NAMESPACE) {
      Invalid(action.element, "Invocation requires an Assembly xsi:type");
    }
    const auto &type = action.element.xsiType->localName;
    if (type == "PropertyValue") {
      const auto name = AttributeValue(action.element, "Property");
      auto *property = component->GetProperty(name.c_str());
      const auto *value = Child(action.element, "Value");
      if (!property) {
        return false;
      }
      if (!value) {
        Invalid(action.element, "PropertyValue has no Value");
      }
      property->SetValue(ParseSimpleValueFor(
          *value, property->GetPrimitiveTypeKind(), property->GetType()));
      return true;
    }
    if (type == "OperationCall") {
      const auto name = AttributeValue(action.element, "Operation");
      auto *operation = component->GetOperation(name.c_str());
      if (!operation) {
        return false;
      }
      auto *request = operation->CreateRequest();
      if (!request) {
        Invalid(action.element,
                "operation '" + name + "' does not support invocation");
      }
      struct RequestGuard {
        ::Smp::IOperation *operation;
        ::Smp::IRequest *request;
        ~RequestGuard() { operation->DeleteRequest(request); }
      } guard{operation, request};
      struct ExpectedReturn {
        ::Smp::AnySimple value;
      };
      std::optional<ExpectedReturn> expectedReturn;
      std::set<std::string> parameterNames;
      ::Smp::Int32 previousParameterIndex = -1;
      bool returnSeen = false;
      for (const auto &parameter : action.element.children) {
        if (parameter.name.localName != "Parameter") {
          continue;
        }
        const auto parameterName = AttributeValue(parameter, "Parameter");
        if (!parameterNames.emplace(parameterName).second) {
          Invalid(parameter,
                  "duplicate operation parameter '" + parameterName + "'");
        }
        const auto *value = Child(parameter, "Value");
        if (!value) {
          Invalid(parameter, "operation parameter has no Value");
        }
        auto *publishedParameter =
            operation->GetParameter(parameterName.c_str());
        if (!publishedParameter || !publishedParameter->GetType()) {
          Invalid(parameter,
                  "operation has no parameter named '" + parameterName + "'");
        }
        const auto parsed = ParseSimpleValueFor(
            *value, publishedParameter->GetType()->GetPrimitiveTypeKind(),
            publishedParameter->GetType());
        const auto index = request->GetParameterIndex(parameterName.c_str());
        if (index >= 0) {
          if (returnSeen || index <= previousParameterIndex) {
            Invalid(parameter,
                    "operation parameters are not in signature order");
          }
          if (publishedParameter->GetDirection() ==
              ::Smp::Publication::ParameterDirectionKind::PDK_Out) {
            Invalid(parameter,
                    "an output-only operation parameter cannot have an input "
                    "value");
          }
          request->SetParameterValue(index, parsed);
          previousParameterIndex = index;
        } else if (operation->GetReturnParameter() &&
                   parameterName ==
                       operation->GetReturnParameter()->GetName()) {
          if (returnSeen) {
            Invalid(parameter, "duplicate operation return value");
          }
          expectedReturn.emplace(ExpectedReturn{parsed});
          returnSeen = true;
        } else {
          Invalid(parameter,
                  "operation has no parameter named '" + parameterName + "'");
        }
      }
      ValidateOperationInputSignature(action.element, *operation,
                                      parameterNames);
      operation->Invoke(request);
      if (expectedReturn &&
          request->GetReturnValue() != expectedReturn->value) {
        Invalid(action.element,
                "operation '" + name + "' returned an unexpected value");
      }
      return true;
    }
    Invalid(action.element, "unsupported invocation type '" + type + "'");
  }

  bool ApplyGlobalEvent(const Action &action) {
    auto *publisher = dynamic_cast<::Smp::IEntryPointPublisher *>(
        ResolveActionPath(action, action.componentPath));
    if (!publisher || !simulator.GetEventManager()) {
      return false;
    }
    const auto entryPointName =
        AttributeValue(action.element, "EntryPointName");
    auto *entryPoint = publisher->GetEntryPoint(entryPointName.c_str());
    if (!entryPoint) {
      return false;
    }
    const auto eventName = AttributeValue(action.element, "GlobalEventName");
    auto *manager = simulator.GetEventManager();
    manager->Subscribe(manager->QueryEventId(eventName.c_str()), entryPoint);
    return true;
  }

  struct TaskContext final {
    ::Smp::IObject *root{};
    bool replaceTopLevel{};
  };

  static std::pair<std::string, std::string>
  SplitReference(std::string_view href) {
    const auto separator = href.find('#');
    if (separator == std::string_view::npos) {
      return {std::string{href}, {}};
    }
    return {std::string{href.substr(0U, separator)},
            std::string{href.substr(separator + 1U)}};
  }

  static std::string TaskIdentity(const Element &task) {
    auto identity = AttributeValue(task, "Id");
    if (identity.empty()) {
      identity = AttributeValue(task, "Name");
    }
    return identity;
  }

  const Element *FindTask(const Element &schedule, std::string fragment,
                          std::string_view title) const {
    while (!fragment.empty() && fragment.front() == '/') {
      fragment.erase(fragment.begin());
    }
    std::string dottedFragment = fragment;
    std::replace(dottedFragment.begin(), dottedFragment.end(), '/', '.');

    const Element *byTitle = nullptr;
    for (const auto &task : schedule.children) {
      if (task.name.localName != "Task") {
        continue;
      }
      const auto id = AttributeValue(task, "Id");
      const auto name = AttributeValue(task, "Name");
      if ((!fragment.empty() &&
           (fragment == id || fragment == name || dottedFragment == id)) ||
          (!dottedFragment.empty() && dottedFragment.size() > name.size() &&
           dottedFragment.compare(dottedFragment.size() - name.size(),
                                  name.size(), name) == 0 &&
           dottedFragment[dottedFragment.size() - name.size() - 1U] == '.')) {
        return &task;
      }
      if (!title.empty() && title == name) {
        if (byTitle) {
          Invalid(task, "ambiguous Task title '" + std::string{title} + "'");
        }
        byTitle = &task;
      }
    }
    return byTitle;
  }

  ::Smp::IObject *ResolveSchedulePath(const TaskContext &context,
                                      std::string_view path) const {
    if (!context.root) {
      return nullptr;
    }
    if (!IsAbsolutePath(path)) {
      return Resolve(context.root, path);
    }
    if (!context.replaceTopLevel) {
      return Resolve(&simulator, path);
    }

    // ExecuteTask.Root replaces the first (assembly-root) segment of every
    // absolute path in the referenced task. Relative paths are simply
    // resolved from that root.
    auto remainder = path.substr(1U);
    const auto separator = remainder.find_first_of("/.");
    if (separator == std::string_view::npos) {
      return context.root;
    }
    remainder.remove_prefix(separator + 1U);
    return remainder.empty() ? context.root : Resolve(context.root, remainder);
  }

  std::string CanonicalTaskKey(const std::filesystem::path &path,
                               const Element &task) const {
    std::error_code error;
    const auto canonical = std::filesystem::weakly_canonical(path, error);
    return (error ? path.lexically_normal() : canonical).string() + '#' +
           TaskIdentity(task);
  }

  std::shared_ptr<TaskPlan> CompileTaskReference(
      const Element &owner, const Element &schedule,
      const std::filesystem::path &schedulePath, const TaskContext &context,
      const ProvidedArguments &arguments, std::set<std::string> &stack,
      bool localOnly, std::set<std::string> *visitedTasks) {
    const auto *reference = Child(owner, "Task");
    const auto *hrefAttribute =
        reference
            ? reference->FindAttribute("href", "http://www.w3.org/1999/xlink")
            : nullptr;
    if (!reference || !hrefAttribute) {
      Invalid(owner, "Task reference has no xlink:href");
    }
    const auto [filePart, fragment] = SplitReference(hrefAttribute->value);
    const auto *titleAttribute =
        reference->FindAttribute("title", "http://www.w3.org/1999/xlink");
    const std::string_view title = titleAttribute
                                       ? std::string_view{titleAttribute->value}
                                       : std::string_view{};
    if (localOnly && !filePart.empty()) {
      Invalid(owner, "an Event must reference a Task in the same Schedule");
    }
    if (filePart.find("://") != std::string::npos ||
        filePart.rfind("//", 0U) == 0U) {
      Invalid(owner, "remote Schedule references are not supported");
    }

    if (filePart.empty() && arguments.empty()) {
      const auto *target = FindTask(schedule, fragment, title);
      if (!target) {
        Invalid(owner, "referenced Task '" +
                           (fragment.empty() ? std::string{title} : fragment) +
                           "' does not exist");
      }
      return CompileTask(schedule, schedulePath, *target, context, stack,
                         visitedTasks);
    }

    const auto targetPath =
        filePart.empty() ? schedulePath : schedulePath.parent_path() / filePart;
    auto document =
        ReadTemplated(targetPath, DocumentKind::Schedule, arguments);
    const auto *target = FindTask(document.root, fragment, title);
    if (!target) {
      Invalid(owner, "referenced Task '" +
                         (fragment.empty() ? std::string{title} : fragment) +
                         "' does not exist in '" + targetPath.string() + "'");
    }
    return CompileTask(document.root, targetPath, *target, context, stack,
                       visitedTasks);
  }

  bool CompileActivity(const Element &activity, const Element &schedule,
                       const std::filesystem::path &schedulePath,
                       const TaskContext &context, std::set<std::string> &stack,
                       TaskPlan &plan, std::set<std::string> *visitedTasks) {
    if (!activity.xsiType ||
        activity.xsiType->namespaceUri != Xml::SCHEDULE_NAMESPACE) {
      Invalid(activity, "Activity requires a Schedule xsi:type");
    }
    const auto &type = activity.xsiType->localName;

    if (type == "Trigger") {
      const auto path = ChildText(activity, "EntryPoint");
      ValidateLevel2Path(activity, path);
      auto *object = ResolveSchedulePath(context, path);
      if (!object) {
        return false;
      }
      auto *entryPoint = dynamic_cast<::Smp::IEntryPoint *>(object);
      if (!entryPoint) {
        Invalid(activity, "Trigger path '" + path +
                              "' does not resolve to an IEntryPoint");
      }
      plan.activities.emplace_back([entryPoint] { entryPoint->Execute(); });
      return true;
    }

    if (type == "Transfer") {
      const auto outputPath = ChildText(activity, "OutputFieldPath", ".");
      const auto inputPath = ChildText(activity, "InputFieldPath", ".");
      ValidateLevel2Path(activity, outputPath);
      ValidateLevel2Path(activity, inputPath);
      auto *outputObject = ResolveSchedulePath(context, outputPath);
      auto *inputObject = ResolveSchedulePath(context, inputPath);
      if (!outputObject || !inputObject) {
        return false;
      }
      auto *source = dynamic_cast<::Smp::ISimpleField *>(outputObject);
      auto *target = dynamic_cast<::Smp::ISimpleField *>(inputObject);
      if (!source || !target || !source->IsOutput() || !target->IsInput() ||
          !::Xsmp::Helper::AreEquivalent(source, target)) {
        Invalid(activity,
                "Transfer endpoints are not compatible simple output/input "
                "fields");
      }
      plan.activities.emplace_back(
          [source, target] { target->SetValue(source->GetValue()); });
      return true;
    }

    if (type == "SetProperty") {
      const auto path = ChildText(activity, "PropertyPath", ".");
      ValidateLevel2Path(activity, path);
      auto *object = ResolveSchedulePath(context, path);
      if (!object) {
        return false;
      }
      auto *property = dynamic_cast<::Smp::IProperty *>(object);
      const auto *value = Child(activity, "Value");
      if (!property) {
        Invalid(activity, "SetProperty path '" + path +
                              "' does not resolve to an IProperty");
      }
      if (!property->GetType()) {
        Invalid(activity, "SetProperty target has no published type");
      }
      if (property->GetAccess() == ::Smp::AccessKind::AK_ReadOnly) {
        Invalid(activity, "SetProperty target is read-only");
      }
      if (!value) {
        Invalid(activity, "SetProperty has no Value");
      }
      try {
        ParseSimpleValueFor(*value, property->GetPrimitiveTypeKind(),
                            property->GetType());
      } catch (const ::Smp::InvalidFile &) {
        throw;
      } catch (const std::exception &error) {
        Invalid(activity, error.what());
      }
      const Element ownedValue = *value;
      plan.activities.emplace_back([property, ownedValue] {
        property->SetValue(ParseSimpleValueFor(
            ownedValue, property->GetPrimitiveTypeKind(), property->GetType()));
      });
      return true;
    }

    if (type == "CallOperation") {
      const auto path = ChildText(activity, "OperationPath", ".");
      ValidateLevel2Path(activity, path);
      auto *object = ResolveSchedulePath(context, path);
      if (!object) {
        return false;
      }
      auto *operation = dynamic_cast<::Smp::IOperation *>(object);
      if (!operation) {
        Invalid(activity, "CallOperation path '" + path +
                              "' does not resolve to an IOperation");
      }
      std::vector<std::pair<std::string, ::Smp::AnySimple>> parameters;
      std::set<std::string> names;
      ::Smp::Int32 previousParameterIndex = -1;
      try {
        for (const auto &parameter : activity.children) {
          if (parameter.name.localName != "Parameter") {
            continue;
          }
          const auto name = AttributeValue(parameter, "Parameter");
          const auto *value = Child(parameter, "Value");
          if (!value) {
            Invalid(parameter, "operation parameter has no Value");
          }
          if (!names.emplace(name).second) {
            Invalid(parameter, "duplicate operation parameter '" + name + "'");
          }
          auto *publishedParameter = operation->GetParameter(name.c_str());
          if (!publishedParameter || !publishedParameter->GetType() ||
              publishedParameter == operation->GetReturnParameter()) {
            Invalid(parameter,
                    "operation has no input parameter named '" + name + "'");
          }
          if (publishedParameter->GetDirection() ==
              ::Smp::Publication::ParameterDirectionKind::PDK_Out) {
            Invalid(parameter,
                    "an output-only operation parameter cannot have an input "
                    "value");
          }
          parameters.emplace_back(
              name,
              ParseSimpleValueFor(
                  *value, publishedParameter->GetType()->GetPrimitiveTypeKind(),
                  publishedParameter->GetType()));
        }
      } catch (const ::Smp::InvalidFile &) {
        throw;
      } catch (const std::exception &error) {
        Invalid(activity, error.what());
      }

      ValidateOperationInputSignature(activity, *operation, names);

      auto *request = operation->CreateRequest();
      if (!request) {
        Invalid(activity,
                "operation '" + path + "' does not support dynamic invocation");
      }
      struct RequestGuard final {
        ::Smp::IOperation *operation;
        ::Smp::IRequest *request;
        ~RequestGuard() { operation->DeleteRequest(request); }
      } guard{operation, request};
      try {
        for (const auto &[name, value] : parameters) {
          const auto index = request->GetParameterIndex(name.c_str());
          if (index < 0 || index <= previousParameterIndex) {
            Invalid(activity, index < 0
                                  ? "operation has no input parameter named '" +
                                        name + "'"
                                  : "operation parameters are not in signature "
                                    "order");
          }
          request->SetParameterValue(index, value);
          previousParameterIndex = index;
        }
      } catch (const ::Smp::InvalidFile &) {
        throw;
      } catch (const std::exception &error) {
        Invalid(activity, error.what());
      }

      plan.activities.emplace_back([operation, parameters] {
        auto *call = operation->CreateRequest();
        if (!call) {
          throw std::runtime_error(
              "scheduled operation no longer supports dynamic invocation");
        }
        struct Guard final {
          ::Smp::IOperation *operation;
          ::Smp::IRequest *request;
          ~Guard() { operation->DeleteRequest(request); }
        } callGuard{operation, call};
        for (const auto &[name, value] : parameters) {
          call->SetParameterValue(call->GetParameterIndex(name.c_str()), value);
        }
        operation->Invoke(call);
      });
      return true;
    }

    if (type == "EmitGlobalEvent") {
      auto *manager = simulator.GetEventManager();
      if (!manager) {
        return false;
      }
      const auto name = ChildText(activity, "EventName");
      if (name.empty()) {
        Invalid(activity, "EmitGlobalEvent has an empty EventName");
      }
      const auto synchronousText = ChildText(activity, "synchronous", "true");
      bool synchronous{};
      if (synchronousText == "true" || synchronousText == "1") {
        synchronous = true;
      } else if (synchronousText == "false" || synchronousText == "0") {
        synchronous = false;
      } else {
        Invalid(activity, "invalid synchronous flag '" + synchronousText + "'");
      }
      const auto eventId = manager->QueryEventId(name.c_str());
      plan.activities.emplace_back([manager, eventId, synchronous] {
        manager->Emit(eventId, synchronous);
      });
      return true;
    }

    if (type == "ExecuteTask") {
      auto *root = static_cast<::Smp::IObject *>(&simulator);
      bool replaceTopLevel = false;
      if (const auto *rootAttribute = activity.FindAttribute("Root")) {
        ValidateLevel2Path(activity, rootAttribute->value);
        root = ResolveSchedulePath(context, rootAttribute->value);
        if (!root) {
          return false;
        }
        if (root != static_cast<::Smp::IObject *>(&simulator) &&
            !dynamic_cast<::Smp::IComponent *>(root)) {
          Invalid(activity, "ExecuteTask Root does not resolve to a Component");
        }
        replaceTopLevel = true;
      }
      auto nested = CompileTaskReference(
          activity, schedule, schedulePath, {root, replaceTopLevel},
          InstanceArguments(activity), stack, false, visitedTasks);
      if (!nested) {
        return false;
      }
      plan.activities.emplace_back(
          [nested = std::move(nested)] { nested->Execute(); });
      return true;
    }

    Invalid(activity, "unsupported Schedule Activity type '" + type + "'");
  }

  std::shared_ptr<TaskPlan> CompileTask(const Element &schedule,
                                        const std::filesystem::path &path,
                                        const Element &task,
                                        const TaskContext &context,
                                        std::set<std::string> &stack,
                                        std::set<std::string> *visitedTasks) {
    const auto key = CanonicalTaskKey(path, task);
    if (!stack.emplace(key).second) {
      Invalid(task, "cyclic ExecuteTask reference involving Task '" +
                        TaskIdentity(task) + "'");
    }
    if (visitedTasks) {
      visitedTasks->emplace(key);
    }
    struct StackGuard final {
      std::set<std::string> &stack;
      std::string key;
      ~StackGuard() { stack.erase(key); }
    } guard{stack, key};

    auto plan = std::make_shared<TaskPlan>();
    for (const auto &activity : task.children) {
      if (activity.name.localName == "Activity" &&
          !CompileActivity(activity, schedule, path, context, stack, *plan,
                           visitedTasks)) {
        return nullptr;
      }
    }
    return plan;
  }

  bool CompileSchedule(const PendingSchedule &pending,
                       std::vector<EventDefinition> &events,
                       std::optional<::Smp::DateTime> &epochTime,
                       std::optional<::Smp::DateTime> &missionStart) {
    if (!simulator.GetScheduler()) {
      return false;
    }

    if (const auto *epoch = Child(pending.root, "EpochTime")) {
      epochTime = ParseDateTime(epoch->TrimmedText(), *epoch);
    } else if (const auto *epoch = pending.root.FindAttribute("EpochTime")) {
      epochTime = ParseDateTime(epoch->value, pending.root);
    }
    if (const auto *mission = Child(pending.root, "MissionStart")) {
      missionStart = ParseDateTime(mission->TrimmedText(), *mission);
    } else if (const auto *mission =
                   pending.root.FindAttribute("MissionStart")) {
      missionStart = ParseDateTime(mission->value, pending.root);
    }
    if ((epochTime || missionStart) && !simulator.GetTimeKeeper()) {
      return false;
    }

    std::set<std::string> visitedTasks;
    for (const auto &event : pending.root.children) {
      if (event.name.localName != "Event") {
        continue;
      }
      if (!event.xsiType ||
          event.xsiType->namespaceUri != Xml::SCHEDULE_NAMESPACE) {
        Invalid(event, "Event requires a Schedule xsi:type");
      }

      EventDefinition definition;
      definition.element = event;
      definition.name = AttributeValue(event, "Name");
      definition.cycleTime =
          ParseIsoDuration(AttributeValue(event, "CycleTime", "PT0S"), event);
      definition.repeatCount = ParseInteger<::Smp::Int64>(
          AttributeValue(event, "RepeatCount", "0"), event);
      if (definition.repeatCount < -1) {
        Invalid(event, "RepeatCount must be -1 or greater");
      }
      if (definition.repeatCount != 0 && definition.cycleTime <= 0) {
        Invalid(event, "a repeated Event requires a positive CycleTime");
      }

      std::set<std::string> stack;
      definition.task = CompileTaskReference(
          event, pending.root, pending.path,
          {static_cast<::Smp::IObject *>(&simulator), false}, {}, stack, true,
          &visitedTasks);
      if (!definition.task) {
        return false;
      }

      const auto &type = event.xsiType->localName;
      if (type == "SimulationEvent") {
        definition.timeKind = ScheduleTimeKind::Simulation;
        definition.relativeTime =
            ParseIsoDuration(AttributeValue(event, "SimulationTime"), event);
      } else if (type == "MissionEvent") {
        definition.timeKind = ScheduleTimeKind::Mission;
        definition.relativeTime =
            ParseIsoDuration(AttributeValue(event, "MissionTime"), event);
        if (!simulator.GetTimeKeeper()) {
          return false;
        }
      } else if (type == "EpochEvent") {
        definition.timeKind = ScheduleTimeKind::Epoch;
        definition.absoluteTime =
            ParseDateTime(AttributeValue(event, "EpochTime"), event);
        if (!simulator.GetTimeKeeper()) {
          return false;
        }
      } else if (type == "ZuluEvent") {
        definition.timeKind = ScheduleTimeKind::Zulu;
        // xsd:dateTime permits an absent zone, but Zulu time denotes an
        // absolute wall-clock instant and therefore requires one.
        definition.absoluteTime =
            ParseDateTime(AttributeValue(event, "ZuluTime"), event, true);
        if (!simulator.GetTimeKeeper()) {
          return false;
        }
      } else if (type == "GlobalEventTriggeredEvent") {
        if (!simulator.GetEventManager() || !simulator.GetTimeKeeper()) {
          return false;
        }
        definition.globalTriggered = true;
        definition.startEvent = AttributeValue(event, "StartEvent");
        definition.stopEvent = AttributeValue(event, "StopEvent");
        definition.relativeTime =
            ParseIsoDuration(AttributeValue(event, "Delay", "PT0S"), event);
        if (definition.relativeTime < 0) {
          Invalid(event,
                  "GlobalEventTriggeredEvent Delay must not be negative");
        }
        const auto timeKind =
            AttributeValue(event, "TimeKind", "SimulationTime");
        if (timeKind == "SimulationTime") {
          definition.timeKind = ScheduleTimeKind::Simulation;
        } else if (timeKind == "MissionTime") {
          definition.timeKind = ScheduleTimeKind::Mission;
        } else if (timeKind == "EpochTime") {
          definition.timeKind = ScheduleTimeKind::Epoch;
        } else if (timeKind == "ZuluTime") {
          definition.timeKind = ScheduleTimeKind::Zulu;
        } else {
          Invalid(event, "unsupported TimeKind '" + timeKind + "'");
        }
        if (definition.startEvent.empty()) {
          Invalid(event, "GlobalEventTriggeredEvent has an empty StartEvent");
        }
      } else {
        Invalid(event, "unsupported Schedule Event type '" + type + "'");
      }
      events.push_back(std::move(definition));
    }

    // Event task graphs above are compiled in their actual ExecuteTask Root
    // contexts. Validate every remaining local Task from the simulator root,
    // without scheduling plans that no Event uses.
    bool allTasksResolved = true;
    for (const auto &task : pending.root.children) {
      if (task.name.localName != "Task" ||
          visitedTasks.find(CanonicalTaskKey(pending.path, task)) !=
              visitedTasks.end()) {
        continue;
      }
      std::set<std::string> stack;
      if (!CompileTask(pending.root, pending.path, task,
                       {static_cast<::Smp::IObject *>(&simulator), false},
                       stack, &visitedTasks)) {
        allTasksResolved = false;
      }
    }
    return allTasksResolved;
  }

  void ClearScheduleRuntime() noexcept {
    if (auto *manager = simulator.GetEventManager()) {
      for (auto iterator = scheduleSubscriptions.rbegin();
           iterator != scheduleSubscriptions.rend(); ++iterator) {
        try {
          manager->Unsubscribe(iterator->event, iterator->entryPoint);
        } catch (...) {
          // Destruction and rollback must not throw.
        }
      }
    }
    scheduleSubscriptions.clear();

    for (const auto &triggered : globalTriggeredEvents) {
      triggered->Stop();
    }
    globalTriggeredEvents.clear();

    if (auto *scheduler = simulator.GetScheduler()) {
      for (const auto event : scheduledEvents) {
        try {
          if (scheduler->IsEventScheduled(event)) {
            scheduler->RemoveEvent(event);
          }
        } catch (...) {
          // Destruction and rollback must not throw.
        }
      }
    }
    scheduledEvents.clear();
  }

  void Shutdown() noexcept {
    if (shutdown) {
      return;
    }
    shutdown = true;
    ClearScheduleRuntime();
  }

  bool TryApplySchedule() {
    if (!pendingSchedule) {
      return true;
    }

    std::vector<EventDefinition> events;
    std::optional<::Smp::DateTime> epochTime;
    std::optional<::Smp::DateTime> missionStart;
    if (!CompileSchedule(*pendingSchedule, events, epochTime, missionStart)) {
      return false;
    }

    auto *scheduler = simulator.GetScheduler();
    auto *timeKeeper = simulator.GetTimeKeeper();
    const auto previousEpoch = timeKeeper ? timeKeeper->GetEpochTime() : 0;
    const auto previousMissionStart =
        timeKeeper ? timeKeeper->GetMissionStartTime() : 0;
    bool timeChanged = false;
    try {
      if (epochTime) {
        timeKeeper->SetEpochTime(*epochTime);
        timeChanged = true;
      }
      if (missionStart) {
        timeKeeper->SetMissionStartTime(*missionStart);
        timeChanged = true;
      }

      std::size_t index = 0U;
      for (const auto &event : events) {
        auto name = event.name.empty()
                        ? std::string{"ScheduleEvent"} + std::to_string(index)
                        : event.name;
        auto taskEntryPoint = std::make_unique<::Xsmp::EntryPoint>(
            name.c_str(), DescriptionOf(event.element).c_str(),
            static_cast<::Smp::IObject *>(&simulator),
            [task = event.task] { task->Execute(); });
        const auto *taskEntryPointPointer = taskEntryPoint.get();
        scheduleEntryPoints.push_back(std::move(taskEntryPoint));

        if (!event.globalTriggered) {
          ::Smp::Services::EventId id{-1};
          switch (event.timeKind) {
          case ScheduleTimeKind::Simulation:
            id = scheduler->AddSimulationTimeEvent(
                taskEntryPointPointer, event.relativeTime, event.cycleTime,
                event.repeatCount);
            break;
          case ScheduleTimeKind::Mission:
            id = scheduler->AddMissionTimeEvent(
                taskEntryPointPointer, event.relativeTime, event.cycleTime,
                event.repeatCount);
            break;
          case ScheduleTimeKind::Epoch:
            id = scheduler->AddEpochTimeEvent(
                taskEntryPointPointer, event.absoluteTime, event.cycleTime,
                event.repeatCount);
            break;
          case ScheduleTimeKind::Zulu:
            id = scheduler->AddZuluTimeEvent(
                taskEntryPointPointer, event.absoluteTime, event.cycleTime,
                event.repeatCount);
            break;
          }
          scheduledEvents.push_back(id);
          ++index;
          continue;
        }

        auto triggered = std::make_shared<GlobalTriggeredState>();
        triggered->scheduler = scheduler;
        triggered->timeKeeper = timeKeeper;
        triggered->task = taskEntryPointPointer;
        triggered->timeKind = event.timeKind;
        triggered->delay = event.relativeTime;
        triggered->cycleTime = event.cycleTime;
        triggered->repeatCount = event.repeatCount;
        globalTriggeredEvents.push_back(triggered);

        auto *manager = simulator.GetEventManager();
        const auto startId = manager->QueryEventId(event.startEvent.c_str());
        auto startEntryPoint = std::make_unique<::Xsmp::EntryPoint>(
            (name + "_Start").c_str(), "Start a schedule event",
            static_cast<::Smp::IObject *>(&simulator),
            [triggered] { triggered->Start(); });
        const auto *startEntryPointPointer = startEntryPoint.get();
        scheduleEntryPoints.push_back(std::move(startEntryPoint));
        manager->Subscribe(startId, startEntryPointPointer);
        scheduleSubscriptions.push_back({startId, startEntryPointPointer});

        if (!event.stopEvent.empty()) {
          const auto stopId = manager->QueryEventId(event.stopEvent.c_str());
          auto stopEntryPoint = std::make_unique<::Xsmp::EntryPoint>(
              (name + "_Stop").c_str(), "Stop a schedule event",
              static_cast<::Smp::IObject *>(&simulator),
              [triggered] { triggered->Stop(); });
          const auto *stopEntryPointPointer = stopEntryPoint.get();
          scheduleEntryPoints.push_back(std::move(stopEntryPoint));
          manager->Subscribe(stopId, stopEntryPointPointer);
          scheduleSubscriptions.push_back({stopId, stopEntryPointPointer});
        }
        ++index;
      }
    } catch (...) {
      ClearScheduleRuntime();
      if (timeChanged && timeKeeper) {
        try {
          timeKeeper->SetEpochTime(previousEpoch);
          timeKeeper->SetMissionStartTime(previousMissionStart);
        } catch (...) {
          // Preserve the original loading error.
        }
      }
      throw;
    }

    pendingSchedule.reset();
    return true;
  }

  void RetrySchedule() {
    if (!pendingSchedule) {
      return;
    }
    try {
      TryApplySchedule();
    } catch (const ::Smp::InvalidFile &) {
      throw;
    } catch (const std::exception &error) {
      Invalid(pendingSchedule->root, error.what());
    }
  }

  void LoadSchedule(::Smp::String8 schedulePath) {
    CheckState();
    if (scheduleLoaded) {
      if (auto *logger = simulator.GetLogger()) {
        logger->Log(&simulator,
                    "LoadSchedule ignored: a Schedule is already loaded.",
                    ::Smp::Services::ILogger::LMK_Warning);
      }
      return;
    }
    const auto path = CheckFile(schedulePath);

    auto document = ReadTemplated(path, DocumentKind::Schedule, {});
    const auto hasEvent = std::any_of(
        document.root.children.begin(), document.root.children.end(),
        [](const auto &child) { return child.name.localName == "Event"; });
    if (!hasEvent) {
      Invalid(document.root, "a Schedule must contain at least one Event");
    }
    pendingSchedule.emplace(PendingSchedule{path, std::move(document.root)});
    scheduleLoaded = true;
    try {
      RetrySchedule();
    } catch (...) {
      pendingSchedule.reset();
      scheduleLoaded = false;
      ClearScheduleRuntime();
      throw;
    }
  }

  static bool ActionPrecedes(const Action &left, const Action &right) {
    if (left.batch != right.batch) {
      return left.batch < right.batch;
    }
    if (left.origin != right.origin) {
      return left.origin < right.origin;
    }
    return left.sequence < right.sequence;
  }

  std::optional<std::string> RetryChainKey(const Action &action) const {
    if (action.kind != ActionKind::FieldValue &&
        action.kind != ActionKind::Invocation) {
      return std::nullopt;
    }

    std::string key =
        action.kind == ActionKind::FieldValue ? "field:" : "invocation:";
    if (auto *target = ResolveActionPath(action, action.componentPath)) {
      key += ::Xsmp::Helper::GetPath(target);
    } else {
      auto *root = IsAbsolutePath(action.componentPath) ? action.absoluteRoot
                                                        : action.scopeRoot;
      key += ::Xsmp::Helper::GetPath(root);
      key += '|';
      key += action.componentPath;
    }
    if (action.kind == ActionKind::FieldValue) {
      key += '|';
      key += AttributeValue(action.element, "Field");
    }
    return key;
  }

  template <typename Function>
  void Retry(std::vector<Action> &actions, Function &&function) {
    std::stable_sort(actions.begin(), actions.end(), ActionPrecedes);
    std::vector<Action> unresolved;
    unresolved.reserve(actions.size());
    std::set<std::string> blockedChains;
    for (auto &action : actions) {
      try {
        const auto chain = RetryChainKey(action);
        if (chain && blockedChains.find(*chain) != blockedChains.end()) {
          unresolved.push_back(std::move(action));
          continue;
        }
        if (!function(action)) {
          if (chain) {
            blockedChains.emplace(*chain);
          }
          unresolved.push_back(std::move(action));
        }
      } catch (const ::Smp::InvalidFile &) {
        throw;
      } catch (const std::exception &error) {
        Invalid(action.element, error.what());
      }
    }
    actions = std::move(unresolved);
  }

  void RetryDeferred() {
    Retry(links, [this](const auto &action) { return ApplyLink(action); });
    Retry(fieldValues,
          [this](const auto &action) { return ApplyField(action); });
    Retry(invocations,
          [this](const auto &action) { return ApplyInvocation(action); });
    Retry(globalEvents,
          [this](const auto &action) { return ApplyGlobalEvent(action); });
    RetrySchedule();
  }

  void ResolveOrThrow() {
    RetryDeferred();
    const Action *first = nullptr;
    for (const auto *queue :
         {&links, &fieldValues, &invocations, &globalEvents}) {
      if (!queue->empty() &&
          (!first || ActionPrecedes(queue->front(), *first))) {
        first = &queue->front();
      }
    }
    if (first) {
      Invalid(first->element, "unresolved Level 2 element '" +
                                  first->element.name.localName + "'");
    }
    if (pendingSchedule) {
      Invalid(pendingSchedule->root, "unresolved Schedule Task");
    }
  }

  void LoadAssembly(::Smp::String8 assemblyPath, ::Smp::String8 parentPath,
                    ::Smp::String8 containerName,
                    ::Smp::String8 rootInstanceName) {
    CheckState();
    const auto path = CheckFile(assemblyPath);
    auto *parent = AssemblyParentFor(parentPath);
    std::string container;
    if (parent != static_cast<::Smp::IComposite *>(&simulator)) {
      container = ::Xsmp::Helper::checkName(containerName, parent).c_str();
    }
    std::string rootName;
    if (rootInstanceName && rootInstanceName[0] != '\0') {
      rootName = ::Xsmp::Helper::checkName(rootInstanceName, parent).c_str();
    }
    currentBatch = nextBatch++;
    try {
      CreateAssembly(path, *parent, container, rootName, {}, true);
      if (simulator.GetState() == ::Smp::SimulatorStateKind::SSK_Building) {
        simulator.Publish();
        RetryDeferred();
        simulator.Configure();
      } else {
        RetryDeferred();
      }
    } catch (const RootDuplicateName &duplicate) {
      std::rethrow_exception(duplicate.exception);
    } catch (const ::Smp::InvalidFile &) {
      throw;
    } catch (const ::Smp::DuplicateName &duplicate) {
      Invalid(path, "duplicate component name '" +
                        std::string{duplicate.GetDuplicateName()} +
                        "' in Assembly content");
    } catch (const std::exception &error) {
      Invalid(path, error.what());
    }
  }

  void LoadLinkBase(::Smp::String8 linkBasePath, ::Smp::String8 parentPath) {
    CheckState();
    const auto path = CheckFile(linkBasePath);
    currentBatch = nextBatch++;
    try {
      auto document = Read(path, DocumentKind::LinkBase);
      auto *parent = ComponentParentFor(parentPath);
      for (const auto &component : document.root.children) {
        if (component.name.localName == "Component") {
          QueueLinkBaseComponent(component, *parent, {},
                                 ActionOrigin::ExternalConfiguration);
        }
      }
      RetryDeferred();
    } catch (const ::Smp::InvalidFile &) {
      throw;
    } catch (const std::exception &error) {
      Invalid(path, error.what());
    }
  }

  void LoadConfiguration(::Smp::String8 configurationPath,
                         ::Smp::String8 parentPath) {
    CheckState();
    const auto path = CheckFile(configurationPath);
    currentBatch = nextBatch++;
    try {
      auto *parent = ComponentParentFor(parentPath);
      QueueConfigurationFile(path, *parent,
                             ActionOrigin::ExternalConfiguration);
      RetryDeferred();
    } catch (const ::Smp::InvalidFile &) {
      throw;
    } catch (const std::exception &error) {
      Invalid(path, error.what());
    }
  }
};

Loader::Loader(::Xsmp::Simulator &simulator)
    : _impl{std::make_unique<Impl>(simulator)} {}

Loader::~Loader() = default;

void Loader::LoadAssembly(::Smp::String8 assemblyPath,
                          ::Smp::String8 parentPath,
                          ::Smp::String8 containerName,
                          ::Smp::String8 rootInstanceName) {
  _impl->LoadAssembly(assemblyPath, parentPath, containerName,
                      rootInstanceName);
}

void Loader::LoadLinkBase(::Smp::String8 linkBasePath,
                          ::Smp::String8 parentPath) {
  _impl->LoadLinkBase(linkBasePath, parentPath);
}

void Loader::LoadSchedule(::Smp::String8 schedulePath) {
  _impl->LoadSchedule(schedulePath);
}

void Loader::LoadConfiguration(::Smp::String8 configurationPath,
                               ::Smp::String8 parentPath) {
  _impl->LoadConfiguration(configurationPath, parentPath);
}

void Loader::RetryDeferred() { _impl->RetryDeferred(); }

void Loader::ResolveOrThrow() { _impl->ResolveOrThrow(); }

void Loader::Shutdown() noexcept { _impl->Shutdown(); }

} // namespace Xsmp::L2
