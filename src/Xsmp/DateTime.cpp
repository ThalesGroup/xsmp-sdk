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

#include <Xsmp/DateTime.h>
#include <chrono>
#include <date/date.h>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

namespace Xsmp {

DateTime DateTime::now() { return DateTime{std::chrono::system_clock::now()}; }

namespace {
inline DateTime::time_point parse(std::string_view date, const char *fmt) {
  std::istringstream iss{std::string(date)};
  DateTime::time_point timePoint;
  iss >> date::parse(fmt, timePoint);
  return timePoint;
}
inline DateTime::time_point parse(std::istream &inputStream, const char *fmt) {
  DateTime::time_point timePoint;
  inputStream >> date::parse(fmt, timePoint);
  return timePoint;
}
} // namespace

DateTime::DateTime(std::string_view date, const char *fmt)
    : _value{convert(parse(date, fmt))} {}

DateTime::DateTime(std::istream &inputStream, const char *fmt)
    : _value{convert(parse(inputStream, fmt))} {}

std::string DateTime::format(const char *fmt) const {
  return date::format(fmt, static_cast<time_point>(*this));
}

std::ostream &DateTime::to_stream(std::ostream &outputStream,
                                  const char *fmt) const {
  return date::to_stream(outputStream, fmt, static_cast<time_point>(*this));
}

std::string DateTime::format(const std::string &fmt) const {
  return date::format(fmt.c_str(), static_cast<time_point>(*this));
}

std::ostream &DateTime::to_stream(std::ostream &outputStream,
                                  const std::string &fmt) const {
  return date::to_stream(outputStream, fmt.c_str(),
                         static_cast<time_point>(*this));
}

std::ostream &operator<<(std::ostream &outputStream, const DateTime &dateTime) {
  return date::to_stream(outputStream, "%F %T",
                         static_cast<DateTime::time_point>(dateTime));
}

} // namespace Xsmp
