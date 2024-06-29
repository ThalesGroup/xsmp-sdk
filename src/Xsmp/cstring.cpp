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
#include <cstring>

namespace Xsmp {

cstring::cstring(const char *value, std::size_t size)
    : _value{std::make_unique<char[]>(size + 1)} {
  std::memcpy(_value.get(), value, size);
  _value.get()[size] = '\0';
}
cstring::cstring(const char *value) : cstring(value, std::strlen(value)) {}
cstring::cstring(std::string_view value)
    : cstring(value.data(), value.size()) {}

cstring::operator const char *() const noexcept { return _value.get(); }
} // namespace Xsmp
