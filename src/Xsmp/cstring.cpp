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

#include <Xsmp/cstring.h>
#include <cstddef>
#include <string>
#include <string_view>

namespace Xsmp {
namespace {
char *copy(const char *value, std::size_t size) {
  auto copy = new char[size + 1];
  std::char_traits<char>::copy(copy, value, size);
  copy[size] = '\0';
  return copy;
}
char *copy(const char *value) {
  return copy(value, std::char_traits<char>::length(value));
}
} // namespace

cstring::cstring(const char *value, std::size_t size)
    : _value{copy(value, size)} {}

cstring::cstring(const char *value)
    : cstring(value, std::char_traits<char>::length(value)) {}

cstring::cstring(std::string_view value)
    : _value{copy(value.data(), value.size())} {}

cstring::cstring(const cstring &other) : cstring(other.c_str()) {}

cstring &cstring::operator=(const cstring &other) {
  if (this != &other) {
    delete[] _value;
    _value = copy(other.c_str());
  }
  return *this;
}

cstring &cstring::operator=(const char *value) {
  delete[] _value;
  _value = copy(value);
  return *this;
}

cstring::~cstring() { delete[] _value; }

cstring::cstring(cstring &&other) noexcept : _value{other._value} {
  other._value = nullptr;
}
cstring &cstring::operator=(cstring &&other) noexcept {
  delete[] _value;
  _value = other._value;
  other._value = nullptr;
  return *this;
}
} // namespace Xsmp
