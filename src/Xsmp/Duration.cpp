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

#include <Xsmp/Duration.h>
#include <date/date.h>

namespace Xsmp {

const constexpr char *Duration::_defaultFmt;

namespace {
::Smp::Duration from(std::istream &is, const char *fmt) {
  std::chrono::nanoseconds tp;
  is >> date::parse(fmt, tp);
  return tp.count();
}
::Smp::Duration from(std::string_view date, const char *fmt) {
  std::istringstream is{std::string(date)};
  return from(is, fmt);
}
} // namespace

Duration::Duration(std::string_view date, const char *fmt)
    : _value{from(date, fmt)} {}

Duration::Duration(std::istream &is, const char *fmt) : _value{from(is, fmt)} {}

std::string Duration::format(const char *fmt) const {
  return date::format(fmt, static_cast<std::chrono::nanoseconds>(*this));
}

std::string Duration::format(const std::string &fmt) const {
  return date::format(fmt, static_cast<std::chrono::nanoseconds>(*this));
}

std::ostream &Duration::to_stream(std::ostream &os, const char *fmt) const {
  return date::to_stream(os, fmt, static_cast<std::chrono::nanoseconds>(*this));
}

std::ostream &Duration::to_stream(std::ostream &os,
                                  const std::string &fmt) const {
  return to_stream(os, fmt.c_str());
}

std::ostream &operator<<(std::ostream &os, const Duration &d) {
  return date::to_stream(os, "%T", static_cast<std::chrono::nanoseconds>(d));
}
} // namespace Xsmp
