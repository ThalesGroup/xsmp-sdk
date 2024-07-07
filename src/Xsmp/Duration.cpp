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

#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Duration.h>
#include <chrono>
#include <date/date.h>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

namespace Xsmp {
static_assert(sizeof(::Smp::Duration) == sizeof(::Xsmp::Duration),
              "Size of ::Xsmp::Duration shall be identical to ::Smp::Duration");
static_assert(std::is_standard_layout_v<::Xsmp::Duration>,
              "::Xsmp::Duration shall be a standard layout class");

namespace {
::Smp::Duration from(std::istream &inputStream, const char *fmt) {
  std::chrono::nanoseconds timePoint;
  inputStream >> date::parse(fmt, timePoint);
  return timePoint.count();
}
::Smp::Duration from(std::string_view date, const char *fmt) {
  std::istringstream inputStream{std::string(date)};
  return from(inputStream, fmt);
}
} // namespace

Duration::Duration(std::string_view date, const char *fmt)
    : _value{from(date, fmt)} {}

Duration::Duration(std::istream &inputStream, const char *fmt)
    : _value{from(inputStream, fmt)} {}

std::string Duration::format(const char *fmt) const {
  return date::format(fmt, static_cast<std::chrono::nanoseconds>(*this));
}

std::string Duration::format(const std::string &fmt) const {
  return date::format(fmt, static_cast<std::chrono::nanoseconds>(*this));
}

std::ostream &Duration::to_stream(std::ostream &outputStream,
                                  const char *fmt) const {
  return date::to_stream(outputStream, fmt,
                         static_cast<std::chrono::nanoseconds>(*this));
}

std::ostream &Duration::to_stream(std::ostream &outputStream,
                                  const std::string &fmt) const {
  return to_stream(outputStream, fmt.c_str());
}

std::ostream &operator<<(std::ostream &outputStream, const Duration &duration) {
  return date::to_stream(outputStream, "%T",
                         static_cast<std::chrono::nanoseconds>(duration));
}
} // namespace Xsmp
