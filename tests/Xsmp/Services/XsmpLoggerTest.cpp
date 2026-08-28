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

#include <Smp/IObject.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/ILogger.h>
#include <Xsmp/Simulator.h>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

namespace Xsmp::Services {
TEST(XsmpLogger, QueryLogMessageKind) {

  Simulator sim;
  sim.LoadLibrary("xsmp_services");
  EXPECT_EQ(sim.GetLogger()->QueryLogMessageKind(
                ::Smp::Services::ILogger::LMK_InformationName),
            ::Smp::Services::ILogger::LMK_Information);

  EXPECT_EQ(sim.GetLogger()->QueryLogMessageKind(
                ::Smp::Services::ILogger::LMK_DebugName),
            ::Smp::Services::ILogger::LMK_Debug);

  EXPECT_EQ(sim.GetLogger()->QueryLogMessageKind(
                ::Smp::Services::ILogger::LMK_ErrorName),
            ::Smp::Services::ILogger::LMK_Error);

  EXPECT_EQ(sim.GetLogger()->QueryLogMessageKind(
                ::Smp::Services::ILogger::LMK_EventName),
            ::Smp::Services::ILogger::LMK_Event);

  EXPECT_EQ(sim.GetLogger()->QueryLogMessageKind(
                ::Smp::Services::ILogger::LMK_WarningName),
            ::Smp::Services::ILogger::LMK_Warning);

  constexpr ::Smp::UInt32 max = 50;
  // create max number of message kind
  for (::Smp::UInt32 i = ::Smp::Services::ILogger::LMK_Debug + 1; i < max;
       ++i) {
    sim.GetLogger()->QueryLogMessageKind(std::to_string(i).c_str());
  }
  // retrieve all msg kinds
  for (::Smp::UInt32 i = ::Smp::Services::ILogger::LMK_Debug + 1; i < max;
       ++i) {
    EXPECT_EQ(sim.GetLogger()->QueryLogMessageKind(std::to_string(i).c_str()),
              i);
  }
}

namespace {
/// Write the configuration read by the logger when it is created, in the
/// current directory.
void writeProperties(const std::filesystem::path &directory) {
  std::ofstream properties{directory / "XsmpLogger.properties"};
  properties << R"(# the appenders of the root logger
log.rootLogger = file, console, unknown, badLayout, badRegex, missing

log.appender.file = FileAppender
log.appender.file.File = xsmp-logger-test.log
log.appender.file.layout = PatternLayout
log.appender.file.layout.conversionPattern = %d{%F %T}\t%S{%T}\t%E{%F}\t%M{%T}\t%S\t%p\t%k\t%m\t%%\t%z%n

log.appender.console = ConsoleAppender
log.appender.console.layout = SimpleLayout
log.appender.console.levels = Error, Warning
log.appender.console.path = /Xsmp.*

log.appender.unknown = UnknownAppender

log.appender.badLayout = ConsoleAppender
log.appender.badLayout.layout = UnknownLayout

log.appender.badRegex = ConsoleAppender
log.appender.badRegex.path = [

a line without a separator
)";
}
} // namespace

TEST(XsmpLogger, Configuration) {

  const auto directory =
      std::filesystem::temp_directory_path() / "xsmp-logger-test";
  std::filesystem::remove_all(directory);
  std::filesystem::create_directories(directory);
  writeProperties(directory);

  const auto previous = std::filesystem::current_path();
  std::filesystem::current_path(directory);

  {
    // the logger reads the configuration of the current directory when it is
    // created
    Simulator sim;
    sim.LoadLibrary("xsmp_services");
    auto *logger = sim.GetLogger();

    for (auto kind : {::Smp::Services::ILogger::LMK_Information,
                      ::Smp::Services::ILogger::LMK_Event,
                      ::Smp::Services::ILogger::LMK_Warning,
                      ::Smp::Services::ILogger::LMK_Error,
                      ::Smp::Services::ILogger::LMK_Debug}) {
      logger->Log(logger, "message", kind);
      logger->Log(nullptr, "message without a sender", kind);
    }

    // a message kind registered by a model, and one that was never registered
    logger->Log(logger, "custom kind", logger->QueryLogMessageKind("Custom"));
    logger->Log(logger, "unregistered kind",
                static_cast<::Smp::Services::LogMessageKind>(42));

    sim.Connect();
    sim.Exit();
  }

  std::filesystem::current_path(previous);

  // the file appender wrote the messages it was given
  const auto logFile = directory / "xsmp-logger-test.log";
  ASSERT_TRUE(std::filesystem::exists(logFile));
  const std::string content{
      std::istreambuf_iterator<char>{
          *std::make_unique<std::ifstream>(logFile).get()},
      std::istreambuf_iterator<char>{}};
  EXPECT_NE(content.find("message"), std::string::npos);
  EXPECT_NE(content.find("<unknown: 42>"), std::string::npos);

  std::filesystem::remove_all(directory);
}

} // namespace Xsmp::Services
