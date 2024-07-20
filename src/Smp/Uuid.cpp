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

#include <Smp/Uuid.h>
#include <Xsmp/Exception.h>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>

namespace Smp {

namespace {
inline bool isHexaDigit(char chr) {
  return isxdigit(static_cast<unsigned char>(chr));
}

inline Uuid from(const char *value) {

  // check that the format is valid
  if (!value || !isHexaDigit(value[0]) || !isHexaDigit(value[1]) ||
      !isHexaDigit(value[2]) || !isHexaDigit(value[3]) ||
      !isHexaDigit(value[4]) || !isHexaDigit(value[5]) ||
      !isHexaDigit(value[6]) || !isHexaDigit(value[7]) || value[8] != '-' ||
      !isHexaDigit(value[9]) || !isHexaDigit(value[10]) ||
      !isHexaDigit(value[11]) || !isHexaDigit(value[12]) || value[13] != '-' ||
      !isHexaDigit(value[14]) || !isHexaDigit(value[15]) ||
      !isHexaDigit(value[16]) || !isHexaDigit(value[17]) || value[18] != '-' ||
      !isHexaDigit(value[19]) || !isHexaDigit(value[20]) ||
      !isHexaDigit(value[21]) || !isHexaDigit(value[22]) || value[23] != '-' ||
      !isHexaDigit(value[24]) || !isHexaDigit(value[25]) ||
      !isHexaDigit(value[26]) || !isHexaDigit(value[27]) ||
      !isHexaDigit(value[28]) || !isHexaDigit(value[29]) ||
      !isHexaDigit(value[30]) || !isHexaDigit(value[31]) ||
      !isHexaDigit(value[32]) || !isHexaDigit(value[33]) ||
      !isHexaDigit(value[34]) || !isHexaDigit(value[35]) || value[36] != '\0') {
    ::Xsmp::Exception::throwException(nullptr, "InvalidUUID", "",
                                      "Wrong format");
  }

  /// 8 hex nibbles.
  const auto data1 =
      static_cast<std::uint32_t>(std::strtoul(&value[0], nullptr, 16));

  /// 3x4 hex nibbles.
  const std::array<std::uint16_t, 3> data2{
      static_cast<std::uint16_t>(std::strtoul(&value[9], nullptr, 16)),
      static_cast<std::uint16_t>(std::strtoul(&value[14], nullptr, 16)),
      static_cast<std::uint16_t>(std::strtoul(&value[19], nullptr, 16))};

  const auto d3 =
      static_cast<std::uint64_t>(std::strtoull(&value[24], nullptr, 16));

  /// 6x2 hex nibbles.
  const std::array<std::uint8_t, 6> data3{
      static_cast<std::uint8_t>((d3 >> 40U) & 0xFF),
      static_cast<std::uint8_t>((d3 >> 32U) & 0xFF),
      static_cast<std::uint8_t>((d3 >> 24U) & 0xFF),
      static_cast<std::uint8_t>((d3 >> 16U) & 0xFF),
      static_cast<std::uint8_t>((d3 >> 8U) & 0xFF),
      static_cast<std::uint8_t>(d3 & 0xFF)};

  return {data1, data2, data3};
}
} // namespace

Uuid::Uuid(const char *value) : Uuid(from(value)) {}

bool Uuid::operator<(const Uuid &other) const {

  if (Data1 < other.Data1) {
    return true;
  }
  if (Data1 > other.Data1) {
    return false;
  }
  // Data1 are identical, check Data2
  if (Data2 < other.Data2) {
    return true;
  }
  if (Data2 > other.Data2) {
    return false;
  }
  // Data2 are identical, check Data3
  if (Data3 < other.Data3) {
    return true;
  }
  // it is greater or equal
  return false;
}

bool ::Smp::Uuid::operator==(const ::Smp::Uuid & other) const {
  return Data1 == other.Data1 && Data2 == other.Data2 && Data3 == other.Data3;
}

bool ::Smp::Uuid::operator!=(const ::Smp::Uuid & other) const {
  return Data1 != other.Data1 || Data2 != other.Data2 || Data3 != other.Data3;
}
} // namespace Smp

namespace std {
size_t std::hash<::Smp::Uuid>::operator()(const ::Smp::Uuid &uuid) const {

  const std::uint64_t left = (static_cast<std::uint64_t>(uuid.Data1) << 32) |
                             (static_cast<std::uint64_t>(uuid.Data2[0]) << 16) |
                             static_cast<std::uint64_t>(uuid.Data2[1]);
  const std::uint64_t right =
      (static_cast<std::uint64_t>(uuid.Data2[2]) << 48) |
      (static_cast<std::uint64_t>(uuid.Data3[0]) << 40) |
      (static_cast<std::uint64_t>(uuid.Data3[1]) << 32) |
      (static_cast<std::uint64_t>(uuid.Data3[2]) << 24) |
      (static_cast<std::uint64_t>(uuid.Data3[3]) << 16) |
      (static_cast<std::uint64_t>(uuid.Data3[4]) << 8) |
      static_cast<std::uint64_t>(uuid.Data3[5]);
  return left ^ right;
}
} // namespace std
