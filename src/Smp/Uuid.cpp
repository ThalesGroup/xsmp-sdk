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
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <unordered_map>

namespace Smp {

namespace {
inline bool isHexaDigit(char c) {
    return isxdigit(static_cast<unsigned char>(c));
}

inline Uuid from(const char *uuid_str) {

    //check that the format is valid
    if (!uuid_str || !isHexaDigit(uuid_str[0]) || //
            !isHexaDigit(uuid_str[1]) || //
            !isHexaDigit(uuid_str[2]) || //
            !isHexaDigit(uuid_str[3]) || //
            !isHexaDigit(uuid_str[4]) || //
            !isHexaDigit(uuid_str[5]) || //
            !isHexaDigit(uuid_str[6]) || //
            !isHexaDigit(uuid_str[7]) || //
            uuid_str[8] != '-' || //
            !isHexaDigit(uuid_str[9]) || //
            !isHexaDigit(uuid_str[10]) || //
            !isHexaDigit(uuid_str[11]) || //
            !isHexaDigit(uuid_str[12]) || //
            uuid_str[13] != '-' || //
            !isHexaDigit(uuid_str[14]) || //
            !isHexaDigit(uuid_str[15]) || //
            !isHexaDigit(uuid_str[16]) || //
            !isHexaDigit(uuid_str[17]) || //
            uuid_str[18] != '-' || //
            !isHexaDigit(uuid_str[19]) || //
            !isHexaDigit(uuid_str[20]) || //
            !isHexaDigit(uuid_str[21]) || //
            !isHexaDigit(uuid_str[22]) || //
            uuid_str[23] != '-' || //
            !isHexaDigit(uuid_str[24]) || //
            !isHexaDigit(uuid_str[25]) || //
            !isHexaDigit(uuid_str[26]) || //
            !isHexaDigit(uuid_str[27]) || //
            !isHexaDigit(uuid_str[28]) || //
            !isHexaDigit(uuid_str[29]) || //
            !isHexaDigit(uuid_str[30]) || //
            !isHexaDigit(uuid_str[31]) || //
            !isHexaDigit(uuid_str[32]) || //
            !isHexaDigit(uuid_str[33]) || //
            !isHexaDigit(uuid_str[34]) || //
            !isHexaDigit(uuid_str[35]) || //
            uuid_str[36] != '\0') {
        ::Xsmp::Exception::throwException(nullptr, "InvalidUUID", "",
                "Wrong format");
    }

    /// 8 hex nibbles.
    auto data1 = static_cast<uint32_t>(std::strtoul(&uuid_str[0], nullptr, 16));

    /// 3x4 hex nibbles.
    std::array<uint16_t, 3> data2 { static_cast<uint16_t>(std::strtoul(
            &uuid_str[9], nullptr, 16)), static_cast<uint16_t>(std::strtoul(
            &uuid_str[14], nullptr, 16)), static_cast<uint16_t>(std::strtoul(
            &uuid_str[19], nullptr, 16)) };

    auto d3 = static_cast<uint64_t>(std::strtoull(&uuid_str[24], nullptr, 16));

    /// 6x2 hex nibbles.
    std::array<uint8_t, 6> data3;
    for (std::size_t i = 0; i < data3.size(); ++i)
        data3[i] = (d3 >> ((5 - i) * 8)) & 0xFF;

    return {data1, data2, data3};
}
} // namespace

Uuid::Uuid(const char *uuid_str) :
        Uuid(from(uuid_str)) {
}

bool Uuid::operator<(const Uuid &o) const {

    if (Data1 < o.Data1)
        return true;
    if (Data1 > o.Data1)
        return false;
    //Data1 are identical, check Data2
    if (Data2 < o.Data2)
        return true;
    if (Data2 > o.Data2)
        return false;
    //Data2 are identical, check Data3
    if (Data3 < o.Data3)
        return true;
    // it is greater or equal
    return false;
}

bool ::Smp::Uuid::operator==(const ::Smp::Uuid &o) const {
    return Data1 == o.Data1 && Data2 == o.Data2 && Data3 == o.Data3;
}

bool ::Smp::Uuid::operator!=(const ::Smp::Uuid &o) const {
    return Data1 != o.Data1 || Data2 != o.Data2 || Data3 != o.Data3;
}
} // namespace Smp

namespace std {
std::size_t std::hash<::Smp::Uuid>::operator()(const ::Smp::Uuid &uuid) const {

    uint64_t h0 = (static_cast<uint64_t>(uuid.Data1) << 32)
            | (static_cast<uint64_t>(uuid.Data2[0]) << 16)
            | static_cast<uint64_t>(uuid.Data2[1]);
    uint64_t h1 = (static_cast<uint64_t>(uuid.Data2[2]) << 48)
            | (static_cast<uint64_t>(uuid.Data3[0]) << 40)
            | (static_cast<uint64_t>(uuid.Data3[1]) << 32)
            | (static_cast<uint64_t>(uuid.Data3[2]) << 24)
            | (static_cast<uint64_t>(uuid.Data3[3]) << 16)
            | (static_cast<uint64_t>(uuid.Data3[4]) << 8)
            | static_cast<uint64_t>(uuid.Data3[5]);
    return h0 ^ h1;
}
} // namespace std

