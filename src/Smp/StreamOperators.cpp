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

#include <Smp/AccessKind.h>
#include <Smp/AnySimple.h>
#include <Smp/ComponentStateKind.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ParameterDirectionKind.h>
#include <Smp/Services/TimeKind.h>
#include <Smp/SimulatorStateKind.h>
#include <Smp/Uuid.h>
#include <Smp/ViewKind.h>
#include <Xsmp/DateTime.h>
#include <Xsmp/Duration.h>
#include <cstdint>
#include <iomanip>
#include <iostream>

namespace Smp {
namespace Publication {

std::ostream &operator<<(std::ostream &os, const ParameterDirectionKind &obj) {
  switch (obj) {
  case ParameterDirectionKind::PDK_In:
    return os << "In";
  case ParameterDirectionKind::PDK_Out:
    return os << "Out";
  case ParameterDirectionKind::PDK_InOut:
    return os << "InOut";
  case ParameterDirectionKind::PDK_Return:
    return os << "Return";
  }
  return os << "Unknown ::Smp::Publication::ParameterDirectionKind("
            << static_cast<::Smp::Int32>(obj) << ")";
}

} // namespace Publication
namespace Services {

std::ostream &operator<<(std::ostream &os, const TimeKind &obj) {
  switch (obj) {
  case TimeKind::TK_SimulationTime:
    return os << "SimulationTime";
  case TimeKind::TK_MissionTime:
    return os << "MissionTime";
  case TimeKind::TK_EpochTime:
    return os << "EpochTime";
  case TimeKind::TK_ZuluTime:
    return os << "ZuluTime";
  }
  return os << "Unknown ::Smp::Services::TimeKind("
            << static_cast<::Smp::Int32>(obj) << ")";
}

} // namespace Services

std::ostream &operator<<(std::ostream &os, const ComponentStateKind &obj) {
  switch (obj) {
  case ComponentStateKind::CSK_Created:
    return os << "Created";
  case ComponentStateKind::CSK_Publishing:
    return os << "Publishing";
  case ComponentStateKind::CSK_Configured:
    return os << "Configured";
  case ComponentStateKind::CSK_Connected:
    return os << "Connected";
  case ComponentStateKind::CSK_Disconnected:
    return os << "Disconnected";
  }
  return os << "Unknown ::Smp::ComponentStateKind("
            << static_cast<::Smp::Int32>(obj) << ")";
}
std::ostream &operator<<(std::ostream &os, const SimulatorStateKind &obj) {
  switch (obj) {
  case SimulatorStateKind::SSK_Building:
    return os << "Building";
  case SimulatorStateKind::SSK_Connecting:
    return os << "Connecting";
  case SimulatorStateKind::SSK_Initialising:
    return os << "Initialising";
  case SimulatorStateKind::SSK_Standby:
    return os << "Standby";
  case SimulatorStateKind::SSK_Executing:
    return os << "Executing";
  case SimulatorStateKind::SSK_Storing:
    return os << "Storing";
  case SimulatorStateKind::SSK_Restoring:
    return os << "Restoring";
  case SimulatorStateKind::SSK_Reconnecting:
    return os << "Reconnecting";
  case SimulatorStateKind::SSK_Exiting:
    return os << "Exiting";
  case SimulatorStateKind::SSK_Aborting:
    return os << "Aborting";
  }
  return os << "Unknown ::Smp::SimulatorStateKind("
            << static_cast<::Smp::Int32>(obj) << ")";
}
std::ostream &operator<<(std::ostream &os, const ViewKind &obj) {

  switch (obj) {
  case ViewKind::VK_None:
    return os << "None";
  case ViewKind::VK_Debug:
    return os << "Debug";
  case ViewKind::VK_Expert:
    return os << "Expert";
  case ViewKind::VK_All:
    return os << "All";
  }
  return os << "Unknown ::Smp::ViewKind(" << static_cast<::Smp::Int32>(obj)
            << ")";
}

std::ostream &operator<<(std::ostream &os, const AccessKind &obj) {

  switch (obj) {
  case AccessKind::AK_ReadOnly:
    return os << "ReadOnly";
  case AccessKind::AK_ReadWrite:
    return os << "ReadWrite";
  case AccessKind::AK_WriteOnly:
    return os << "WriteOnly";
  }
  return os << "Unknown ::Smp::AccessKind(" << static_cast<::Smp::Int32>(obj)
            << ")";
}

std::ostream &operator<<(std::ostream &os, const PrimitiveTypeKind &obj) {
  switch (obj) {
  case PrimitiveTypeKind::PTK_None:
    return os << "None";
  case PrimitiveTypeKind::PTK_Char8:
    return os << "Char8";
  case PrimitiveTypeKind::PTK_Bool:
    return os << "Bool";
  case PrimitiveTypeKind::PTK_Int8:
    return os << "Int8";
  case PrimitiveTypeKind::PTK_UInt8:
    return os << "UInt8";
  case PrimitiveTypeKind::PTK_Int16:
    return os << "Int16";
  case PrimitiveTypeKind::PTK_UInt16:
    return os << "UInt16";
  case PrimitiveTypeKind::PTK_Int32:
    return os << "Int32";
  case PrimitiveTypeKind::PTK_UInt32:
    return os << "UInt32";
  case PrimitiveTypeKind::PTK_Int64:
    return os << "Int64";
  case PrimitiveTypeKind::PTK_UInt64:
    return os << "UInt64";
  case PrimitiveTypeKind::PTK_Float32:
    return os << "Float32";
  case PrimitiveTypeKind::PTK_Float64:
    return os << "Float64";
  case PrimitiveTypeKind::PTK_Duration:
    return os << "Duration";
  case PrimitiveTypeKind::PTK_DateTime:
    return os << "DateTime";
  case PrimitiveTypeKind::PTK_String8:
    return os << "String8";
  }
  return os << "Unknown ::Smp::PrimitiveTypeKind("
            << static_cast<::Smp::Int32>(obj) << ")";
}

std::ostream &operator<<(std::ostream &os, const AnySimple &obj) {

  switch (obj.GetType()) {
  case PrimitiveTypeKind::PTK_None:
    return os << "None";
  case PrimitiveTypeKind::PTK_Char8: {

    switch (obj.value.char8Value) {
    case '\'':
      return os << R"('\'')";
    case '\?':
      return os << R"('\?')";
    case '\\':
      return os << R"('\\')";
    case '\a':
      return os << R"('\a')";
    case '\b':
      return os << R"('\b')";
    case '\f':
      return os << R"('\f')";
    case '\n':
      return os << R"('\n')";
    case '\r':
      return os << R"('\r')";
    case '\t':
      return os << R"('\t')";
    case '\v':
      return os << R"('\v')";
    default:
      if (obj.value.char8Value >= ' ' && obj.value.char8Value <= '~') {
        return os << "'" << obj.value.char8Value << "'";
      } else {
        return os << "'\\" << std::oct
                  << static_cast<unsigned int>(obj.value.char8Value) << std::dec
                  << "'";
      }
    }
  }
  case PrimitiveTypeKind::PTK_Bool:
    return os << std::boolalpha << obj.value.boolValue << std::noboolalpha;
  case PrimitiveTypeKind::PTK_Int8:
    return os << static_cast<int>(obj.value.int8Value) << "i8";
  case PrimitiveTypeKind::PTK_UInt8:
    return os << static_cast<unsigned int>(obj.value.uInt8Value) << "u8";
  case PrimitiveTypeKind::PTK_Int16:
    return os << obj.value.int16Value << "i16";
  case PrimitiveTypeKind::PTK_UInt16:
    return os << obj.value.uInt16Value << "u16";
  case PrimitiveTypeKind::PTK_Int32:
    return os << obj.value.int32Value << "i32";
  case PrimitiveTypeKind::PTK_UInt32:
    return os << obj.value.uInt32Value << "u32";
  case PrimitiveTypeKind::PTK_Int64:
    return os << obj.value.int64Value << "i64";
  case PrimitiveTypeKind::PTK_UInt64:
    return os << obj.value.uInt64Value << "u64";
  case PrimitiveTypeKind::PTK_Float32:
    return os << obj.value.float32Value << "f32";
  case PrimitiveTypeKind::PTK_Float64:
    return os << obj.value.float64Value << "f64";
  case PrimitiveTypeKind::PTK_Duration:
    return os << "\"" << ::Xsmp::Duration(obj.value.durationValue) << "\"";
  case PrimitiveTypeKind::PTK_DateTime:
    return os << "\"" << ::Xsmp::DateTime(obj.value.dateTimeValue) << "\"";
  case PrimitiveTypeKind::PTK_String8:
    return os << "\"" << obj.value.string8Value << "\"";
  }
  return os << "<invalid AnySimple type=\""
            << static_cast<::Smp::Int32>(obj.type) << "\">";
}

std::ostream &operator<<(std::ostream &ostream, const ::Smp::Uuid &uuid) {

  ostream << std::hex << std::setw(8) << std::setfill('0') << uuid.Data1;

  ostream << "-" << std::setw(4) << uuid.Data2[0];
  ostream << "-" << std::setw(4) << uuid.Data2[1];
  ostream << "-" << std::setw(4) << uuid.Data2[2];

  ostream << "-";

  for (const std::uint16_t data : uuid.Data3) {
    ostream << std::setw(2) << data;
  }
  ostream << std::setfill(' ') << std::dec;

  return ostream;
}

} // namespace Smp
