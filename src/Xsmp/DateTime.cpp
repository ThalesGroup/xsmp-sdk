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
#include <date/date.h>

namespace Xsmp {

constexpr ::Smp::Duration DateTime::_epochToMjd;
constexpr const char *DateTime::_defaultFmt;

DateTime DateTime::now() { return DateTime{std::chrono::system_clock::now()}; }

namespace {
inline DateTime::time_point parse(std::string_view date, const char *fmt) {
  std::istringstream is{std::string(date)};
  DateTime::time_point tp;
  is >> date::parse(fmt, tp);
  return tp;
}
inline DateTime::time_point parse(std::istream &is, const char *fmt) {
  DateTime::time_point tp;
  is >> date::parse(fmt, tp);
  return tp;
}
} // namespace

DateTime::DateTime(std::string_view date, const char *fmt)
    : _value{convert(parse(date, fmt))} {}

DateTime::DateTime(std::istream &is, const char *fmt)
    : _value{convert(parse(is, fmt))} {}

std::string DateTime::format(const char *fmt) const {
  return date::format(fmt, static_cast<time_point>(*this));
}

std::ostream &DateTime::to_stream(std::ostream &os, const char *fmt) const {
  return date::to_stream(os, fmt, static_cast<time_point>(*this));
}

std::string DateTime::format(const std::string &fmt) const {
  return date::format(fmt.c_str(), static_cast<time_point>(*this));
}

std::ostream &DateTime::to_stream(std::ostream &os,
                                  const std::string &fmt) const {
  return date::to_stream(os, fmt.c_str(), static_cast<time_point>(*this));
}

std::ostream &operator<<(std::ostream &os, const DateTime &d) {
  return date::to_stream(os, "%F %T", static_cast<DateTime::time_point>(d));
}

} // namespace Xsmp
