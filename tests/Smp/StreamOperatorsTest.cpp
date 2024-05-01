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
#include <sstream>

#include <gtest/gtest.h>

namespace Smp {

#define EXPECT_STREAM_OPERATOR_EQ(literal, value)                              \
  do {                                                                         \
    std::stringstream ss;                                                      \
    ss << (literal);                                                           \
    EXPECT_EQ(ss.str(), (value));                                              \
  } while (false)

namespace Publication {
TEST(StreamOperatorsTest, ParameterDirectionKind) {
  EXPECT_STREAM_OPERATOR_EQ(ParameterDirectionKind::PDK_In, "In");
  EXPECT_STREAM_OPERATOR_EQ(ParameterDirectionKind::PDK_Out, "Out");
  EXPECT_STREAM_OPERATOR_EQ(ParameterDirectionKind::PDK_InOut, "InOut");
  EXPECT_STREAM_OPERATOR_EQ(ParameterDirectionKind::PDK_Return, "Return");
  EXPECT_STREAM_OPERATOR_EQ(
      static_cast<ParameterDirectionKind>(42),
      "Unknown ::Smp::Publication::ParameterDirectionKind(42)");
}
} // namespace Publication
namespace Services {

TEST(StreamOperatorsTest, TimeKind) {
  EXPECT_STREAM_OPERATOR_EQ(TimeKind::TK_SimulationTime, "SimulationTime");
  EXPECT_STREAM_OPERATOR_EQ(TimeKind::TK_MissionTime, "MissionTime");
  EXPECT_STREAM_OPERATOR_EQ(TimeKind::TK_EpochTime, "EpochTime");
  EXPECT_STREAM_OPERATOR_EQ(TimeKind::TK_ZuluTime, "ZuluTime");
  EXPECT_STREAM_OPERATOR_EQ(static_cast<TimeKind>(42),
                            "Unknown ::Smp::Services::TimeKind(42)");
}

} // namespace Services

TEST(StreamOperatorsTest, ComponentStateKind) {
  EXPECT_STREAM_OPERATOR_EQ(ComponentStateKind::CSK_Created, "Created");
  EXPECT_STREAM_OPERATOR_EQ(ComponentStateKind::CSK_Publishing, "Publishing");
  EXPECT_STREAM_OPERATOR_EQ(ComponentStateKind::CSK_Configured, "Configured");
  EXPECT_STREAM_OPERATOR_EQ(ComponentStateKind::CSK_Connected, "Connected");
  EXPECT_STREAM_OPERATOR_EQ(ComponentStateKind::CSK_Disconnected,
                            "Disconnected");
  EXPECT_STREAM_OPERATOR_EQ(static_cast<ComponentStateKind>(42),
                            "Unknown ::Smp::ComponentStateKind(42)");
}

TEST(StreamOperatorsTest, SimulatorStateKind) {
  EXPECT_STREAM_OPERATOR_EQ(SimulatorStateKind::SSK_Building, "Building");
  EXPECT_STREAM_OPERATOR_EQ(SimulatorStateKind::SSK_Connecting, "Connecting");
  EXPECT_STREAM_OPERATOR_EQ(SimulatorStateKind::SSK_Initialising,
                            "Initialising");
  EXPECT_STREAM_OPERATOR_EQ(SimulatorStateKind::SSK_Standby, "Standby");
  EXPECT_STREAM_OPERATOR_EQ(SimulatorStateKind::SSK_Executing, "Executing");
  EXPECT_STREAM_OPERATOR_EQ(SimulatorStateKind::SSK_Storing, "Storing");
  EXPECT_STREAM_OPERATOR_EQ(SimulatorStateKind::SSK_Restoring, "Restoring");
  EXPECT_STREAM_OPERATOR_EQ(SimulatorStateKind::SSK_Reconnecting,
                            "Reconnecting");
  EXPECT_STREAM_OPERATOR_EQ(SimulatorStateKind::SSK_Exiting, "Exiting");
  EXPECT_STREAM_OPERATOR_EQ(SimulatorStateKind::SSK_Aborting, "Aborting");
  EXPECT_STREAM_OPERATOR_EQ(static_cast<SimulatorStateKind>(42),
                            "Unknown ::Smp::SimulatorStateKind(42)");
}

TEST(StreamOperatorsTest, ViewKind) {
  EXPECT_STREAM_OPERATOR_EQ(ViewKind::VK_None, "None");
  EXPECT_STREAM_OPERATOR_EQ(ViewKind::VK_Debug, "Debug");
  EXPECT_STREAM_OPERATOR_EQ(ViewKind::VK_Expert, "Expert");
  EXPECT_STREAM_OPERATOR_EQ(ViewKind::VK_All, "All");
  EXPECT_STREAM_OPERATOR_EQ(static_cast<ViewKind>(42),
                            "Unknown ::Smp::ViewKind(42)");
}

TEST(StreamOperatorsTest, AccessKind) {
  EXPECT_STREAM_OPERATOR_EQ(AccessKind::AK_ReadOnly, "ReadOnly");
  EXPECT_STREAM_OPERATOR_EQ(AccessKind::AK_ReadWrite, "ReadWrite");
  EXPECT_STREAM_OPERATOR_EQ(AccessKind::AK_WriteOnly, "WriteOnly");
  EXPECT_STREAM_OPERATOR_EQ(static_cast<AccessKind>(42),
                            "Unknown ::Smp::AccessKind(42)");
}

TEST(StreamOperatorsTest, PrimitiveTypeKind) {
  EXPECT_STREAM_OPERATOR_EQ(PrimitiveTypeKind::PTK_None, "None");
  EXPECT_STREAM_OPERATOR_EQ(PrimitiveTypeKind::PTK_Char8, "Char8");
  EXPECT_STREAM_OPERATOR_EQ(PrimitiveTypeKind::PTK_Bool, "Bool");
  EXPECT_STREAM_OPERATOR_EQ(PrimitiveTypeKind::PTK_Int8, "Int8");
  EXPECT_STREAM_OPERATOR_EQ(PrimitiveTypeKind::PTK_UInt8, "UInt8");
  EXPECT_STREAM_OPERATOR_EQ(PrimitiveTypeKind::PTK_Int16, "Int16");
  EXPECT_STREAM_OPERATOR_EQ(PrimitiveTypeKind::PTK_UInt16, "UInt16");
  EXPECT_STREAM_OPERATOR_EQ(PrimitiveTypeKind::PTK_Int32, "Int32");
  EXPECT_STREAM_OPERATOR_EQ(PrimitiveTypeKind::PTK_UInt32, "UInt32");
  EXPECT_STREAM_OPERATOR_EQ(PrimitiveTypeKind::PTK_Int64, "Int64");
  EXPECT_STREAM_OPERATOR_EQ(PrimitiveTypeKind::PTK_UInt64, "UInt64");
  EXPECT_STREAM_OPERATOR_EQ(PrimitiveTypeKind::PTK_Float32, "Float32");
  EXPECT_STREAM_OPERATOR_EQ(PrimitiveTypeKind::PTK_Float64, "Float64");
  EXPECT_STREAM_OPERATOR_EQ(PrimitiveTypeKind::PTK_Duration, "Duration");
  EXPECT_STREAM_OPERATOR_EQ(PrimitiveTypeKind::PTK_DateTime, "DateTime");
  EXPECT_STREAM_OPERATOR_EQ(PrimitiveTypeKind::PTK_String8, "String8");
  EXPECT_STREAM_OPERATOR_EQ(static_cast<PrimitiveTypeKind>(42),
                            "Unknown ::Smp::PrimitiveTypeKind(42)");
}

TEST(StreamOperatorsTest, Uuid) {
  EXPECT_STREAM_OPERATOR_EQ(Uuid("123E4567-e89B-12d3-a456-426614174005"),
                            "123e4567-e89b-12d3-a456-426614174005");
}

TEST(StreamOperatorsTest, AnySimple) {
  AnySimple any{};
  EXPECT_STREAM_OPERATOR_EQ(any, "None");

  any.SetValue(PrimitiveTypeKind::PTK_Char8, '\0');
  EXPECT_STREAM_OPERATOR_EQ(any, "'\\0'");

  any.SetValue(PrimitiveTypeKind::PTK_Char8, '\177');
  EXPECT_STREAM_OPERATOR_EQ(any, "'\\177'");

  any.SetValue(PrimitiveTypeKind::PTK_Char8, '\'');
  EXPECT_STREAM_OPERATOR_EQ(any, "'\\''");
  any.SetValue(PrimitiveTypeKind::PTK_Char8, '\?');
  EXPECT_STREAM_OPERATOR_EQ(any, "'\\?'");
  any.SetValue(PrimitiveTypeKind::PTK_Char8, '\\');
  EXPECT_STREAM_OPERATOR_EQ(any, "'\\\\'");
  any.SetValue(PrimitiveTypeKind::PTK_Char8, '\a');
  EXPECT_STREAM_OPERATOR_EQ(any, "'\\a'");
  any.SetValue(PrimitiveTypeKind::PTK_Char8, '\b');
  EXPECT_STREAM_OPERATOR_EQ(any, "'\\b'");
  any.SetValue(PrimitiveTypeKind::PTK_Char8, '\f');
  EXPECT_STREAM_OPERATOR_EQ(any, "'\\f'");
  any.SetValue(PrimitiveTypeKind::PTK_Char8, '\n');
  EXPECT_STREAM_OPERATOR_EQ(any, "'\\n'");
  any.SetValue(PrimitiveTypeKind::PTK_Char8, '\r');
  EXPECT_STREAM_OPERATOR_EQ(any, "'\\r'");
  any.SetValue(PrimitiveTypeKind::PTK_Char8, '\t');
  EXPECT_STREAM_OPERATOR_EQ(any, "'\\t'");
  any.SetValue(PrimitiveTypeKind::PTK_Char8, '\v');
  EXPECT_STREAM_OPERATOR_EQ(any, "'\\v'");
  any.SetValue(PrimitiveTypeKind::PTK_Char8, ' ');
  EXPECT_STREAM_OPERATOR_EQ(any, "' '");
  any.SetValue(PrimitiveTypeKind::PTK_Char8, 'a');
  EXPECT_STREAM_OPERATOR_EQ(any, "'a'");
  any.SetValue(PrimitiveTypeKind::PTK_Char8, '~');
  EXPECT_STREAM_OPERATOR_EQ(any, "'~'");

  any.SetValue(PrimitiveTypeKind::PTK_String8, "test");
  EXPECT_STREAM_OPERATOR_EQ(any, "\"test\"");

  any.SetValue(PrimitiveTypeKind::PTK_Bool, true);
  EXPECT_STREAM_OPERATOR_EQ(any, "true");
  any.SetValue(PrimitiveTypeKind::PTK_Bool, false);
  EXPECT_STREAM_OPERATOR_EQ(any, "false");

  any.SetValue(PrimitiveTypeKind::PTK_Int8, 0);
  EXPECT_STREAM_OPERATOR_EQ(any, "0i8");

  any.SetValue(PrimitiveTypeKind::PTK_UInt8, 0);
  EXPECT_STREAM_OPERATOR_EQ(any, "0u8");

  any.SetValue(PrimitiveTypeKind::PTK_Int16, 0);
  EXPECT_STREAM_OPERATOR_EQ(any, "0i16");

  any.SetValue(PrimitiveTypeKind::PTK_UInt16, 0);
  EXPECT_STREAM_OPERATOR_EQ(any, "0u16");

  any.SetValue(PrimitiveTypeKind::PTK_Int32, 0);
  EXPECT_STREAM_OPERATOR_EQ(any, "0i32");

  any.SetValue(PrimitiveTypeKind::PTK_UInt32, 0);
  EXPECT_STREAM_OPERATOR_EQ(any, "0u32");

  any.SetValue(PrimitiveTypeKind::PTK_Int64, 0);
  EXPECT_STREAM_OPERATOR_EQ(any, "0i64");

  any.SetValue(PrimitiveTypeKind::PTK_UInt64, 0);
  EXPECT_STREAM_OPERATOR_EQ(any, "0u64");

  any.SetValue(PrimitiveTypeKind::PTK_Float32, 0.f);
  EXPECT_STREAM_OPERATOR_EQ(any, "0f32");

  any.SetValue(PrimitiveTypeKind::PTK_Float64, 0.);
  EXPECT_STREAM_OPERATOR_EQ(any, "0f64");

  any.SetValue(PrimitiveTypeKind::PTK_Duration, 0);
  EXPECT_STREAM_OPERATOR_EQ(any, "\"00:00:00.000000000\"");

  any.SetValue(PrimitiveTypeKind::PTK_DateTime, 0);
  EXPECT_STREAM_OPERATOR_EQ(any, "\"2000-01-01 12:00:00.000000000\"");

  // check with wrong PrimitiveTypeKind
  any.type = static_cast<PrimitiveTypeKind>(42);
  EXPECT_STREAM_OPERATOR_EQ(any, "<invalid AnySimple type=\"42\">");
}

} // namespace Smp
