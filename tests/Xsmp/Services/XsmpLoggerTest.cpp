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

#include <gtest/gtest.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/ILogger.h>
#include <Xsmp/Simulator.h>
#include <string>

namespace Xsmp::Services {
TEST(XsmpLoggerTest, QueryLogMessageKind) {

    Simulator sim;
    sim.LoadLibrary("xsmp_services");
    EXPECT_EQ(
            sim.GetLogger()->QueryLogMessageKind(
                    ::Smp::Services::ILogger::LMK_InformationName),
            ::Smp::Services::ILogger::LMK_Information);

    EXPECT_EQ(
            sim.GetLogger()->QueryLogMessageKind(
                    ::Smp::Services::ILogger::LMK_DebugName),
            ::Smp::Services::ILogger::LMK_Debug);

    EXPECT_EQ(
            sim.GetLogger()->QueryLogMessageKind(
                    ::Smp::Services::ILogger::LMK_ErrorName),
            ::Smp::Services::ILogger::LMK_Error);

    EXPECT_EQ(
            sim.GetLogger()->QueryLogMessageKind(
                    ::Smp::Services::ILogger::LMK_EventName),
            ::Smp::Services::ILogger::LMK_Event);

    EXPECT_EQ(
            sim.GetLogger()->QueryLogMessageKind(
                    ::Smp::Services::ILogger::LMK_WarningName),
            ::Smp::Services::ILogger::LMK_Warning);

    constexpr ::Smp::UInt32 max = 50;
    // create max number of message kind
    for (::Smp::UInt32 i = ::Smp::Services::ILogger::LMK_Debug + 1; i < max;
            ++i)
        sim.GetLogger()->QueryLogMessageKind(std::to_string(i).c_str());

    // retrieve all msg kinds
    for (::Smp::UInt32 i = ::Smp::Services::ILogger::LMK_Debug + 1; i < max;
            ++i)
        EXPECT_EQ(
                sim.GetLogger()->QueryLogMessageKind(std::to_string(i).c_str()),
                i);
}

} // namespace Xsmp::Services
