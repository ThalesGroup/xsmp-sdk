// Copyright 2024 THALES ALENIA SPACE FRANCE. All rights reserved.
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

#ifndef XSMP_CSTRING_H_
#define XSMP_CSTRING_H_

#include <cstdlib>
#include <memory>
#include <string.h>
#include <string_view>

namespace Xsmp {

/// A simple wrapper to hold a C String
/// zero overhead compared to a const char*
/// The implementation of std::string is typically 32 bytes
/// The input string is duplicated on creation and freed when the wrapper is
/// deleted
struct cstring {
  cstring(const char *value) : _value{strdup(value)} {}
  cstring(const char *value, std::size_t size) : _value{strndup(value, size)} {}
  explicit cstring(std::string_view value)
      : _value{strndup(value.data(), value.size())} {}
  cstring(cstring &&) = default;
  cstring(const cstring &) = delete;
  cstring &operator=(cstring &&) = default;
  cstring &operator=(const cstring &) = delete;
  operator const char *() const noexcept { return _value.get(); }

private:
  struct cstring_deleter {
    void operator()(char *p) const { std::free(p); }
  };
  std::unique_ptr<char, cstring_deleter> _value;
};

} // namespace Xsmp

#endif // XSMP_CSTRING_H_
