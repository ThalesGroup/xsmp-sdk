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

#include <cstddef>
#include <memory>
#include <string_view>

namespace Xsmp {

/// A simple holder for C String (const char*) with zero memory overhead
/// The typical implementation of std::string is 32 bytes
/// The input string is duplicated on creation and deleted when the wrapper is
/// deleted
struct cstring {
  cstring(const char *value, std::size_t size);
  cstring(const char *value);
  explicit cstring(std::string_view value);
  cstring(cstring &&) = default;
  cstring(const cstring &) = delete;
  cstring &operator=(cstring &&) = default;
  cstring &operator=(const cstring &) = delete;
  operator const char *() const noexcept;

private:
  std::unique_ptr<char[]> _value;
};

} // namespace Xsmp

#endif // XSMP_CSTRING_H_
