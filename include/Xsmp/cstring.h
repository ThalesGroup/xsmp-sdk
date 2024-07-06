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
#include <string_view>

namespace Xsmp {

/// A lightweight holder for C String (const char*) with zero memory overhead (8
/// bytes).
/// With a typical implementation of std::string (32 bytes), it saves 24 bytes.
/// The input string is duplicated on creation and deleted when the wrapper is
/// deleted
struct cstring {
  cstring(const char *value, std::size_t size);
  cstring(const char *value);
  explicit cstring(std::string_view value);
  cstring(cstring &&) noexcept;
  cstring &operator=(cstring &&) noexcept;
  cstring(const cstring &);
  cstring &operator=(const cstring &);
  cstring &operator=(const char *value);
  ~cstring();

  inline const char *c_str() const noexcept { return _value; }

private:
  void assign(const char *value, std::size_t size);
  char *_value;
};

} // namespace Xsmp

#endif // XSMP_CSTRING_H_
