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

#ifndef XSMP_STRING_H_
#define XSMP_STRING_H_

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

namespace Xsmp {

/// @class String
/// A fixed-size string class template with support for various operations
/// and conversions.
template <std::size_t Nm> struct String {

  using value_type = char;
  using traits_type = std::char_traits<value_type>;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using reference = value_type &;
  using const_reference = const value_type &;
  using iterator = value_type *;
  using const_iterator = const value_type *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;

  value_type internalString[Nm + 1];

  constexpr String() noexcept : internalString{'\0'} {}

  constexpr String(const char *str) { assign(str); }

  constexpr String(const std::string &str) { assign(str); }

  constexpr String(std::string_view str) { assign(str); }

  template <size_t S> constexpr String(const String<S> &str) { assign(str); }

  constexpr String(const char *str, size_t length) { assign(str, length); }
  // iterators
  [[nodiscard]] constexpr const_iterator begin() noexcept {
    return internalString;
  }
  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    return internalString;
  }
  [[nodiscard]] constexpr const_iterator end() noexcept {
    return internalString + size();
  }
  [[nodiscard]] constexpr const_iterator end() const noexcept {
    return internalString + size();
  }
  [[nodiscard]] constexpr const_iterator cbegin() noexcept {
    return internalString;
  }
  [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
    return internalString;
  }
  [[nodiscard]] constexpr const_iterator cend() noexcept {
    return internalString + size();
  }
  [[nodiscard]] constexpr const_iterator cend() const noexcept {
    return internalString + size();
  }

  [[nodiscard]] constexpr const_reverse_iterator rbegin() noexcept {
    return const_reverse_iterator(this->end());
  }
  [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(this->end());
  }
  [[nodiscard]] constexpr const_reverse_iterator rend() noexcept {
    return const_reverse_iterator(this->begin());
  }
  [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(this->begin());
  }
  [[nodiscard]] constexpr const_reverse_iterator crbegin() noexcept {
    return const_reverse_iterator(this->end());
  }
  [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(this->end());
  }
  [[nodiscard]] constexpr const_reverse_iterator crend() noexcept {
    return const_reverse_iterator(this->begin());
  }
  [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(this->begin());
  }

  // capacity
  [[nodiscard]] constexpr size_type size() const noexcept {
    return traits_type::length(internalString);
  }
  [[nodiscard]] constexpr size_type length() const noexcept { return size(); }
  [[nodiscard]] constexpr size_type max_size() const noexcept { return Nm; }
  [[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }

  // element access
  [[nodiscard]] constexpr reference operator[](size_type _pos) noexcept {
    return internalString[_pos];
  }
  [[nodiscard]] constexpr const_reference
  operator[](size_type _pos) const noexcept {
    return internalString[_pos];
  }
  constexpr reference at(size_type _pos) {
    if (_pos >= this->size()) {
      throw std::out_of_range(
          "String::at: _pos (which is " + std::to_string(_pos) +
          ") >= this-size() (which is " + std::to_string(size()) + ")");
    }
    return internalString[_pos];
  }
  constexpr const_reference at(size_type _pos) const {
    if (_pos >= this->size()) {
      throw std::out_of_range(
          "String::at: _pos (which is " + std::to_string(_pos) +
          ") >= this-size() (which is " + std::to_string(size()) + ")");
    }
    return internalString[_pos];
  }

  [[nodiscard]] constexpr reference front() noexcept {
    return internalString[0];
  }

  [[nodiscard]] constexpr const_reference front() const noexcept {
    return internalString[0];
  }

  [[nodiscard]] constexpr reference back() noexcept {
    return internalString[size() - 1];
  }

  [[nodiscard]] constexpr const_reference back() const noexcept {
    return internalString[size() - 1];
  }

  [[nodiscard]] constexpr pointer data() noexcept { return internalString; }
  [[nodiscard]] constexpr const_pointer data() const noexcept {
    return internalString;
  }

  [[nodiscard]] constexpr const_pointer c_str() const noexcept {
    return data();
  }

  constexpr void remove_prefix(size_t length) {
    auto newLength = this->length();
    std::copy(internalString + length, internalString + newLength,
              internalString);
    newLength -= length;
    internalString[newLength] = '\0';
  }

  constexpr void remove_suffix(size_t length) noexcept {
    auto currentLength = this->length();
    internalString[length > currentLength ? 0 : currentLength - length] = '\0';
  }

  [[nodiscard]] constexpr std::string_view
  substr(size_type pos = 0, size_type n = std::string_view::npos) const
      noexcept(false) {
    return std::string_view{internalString}.substr(pos, n);
  }

  [[nodiscard]] constexpr size_type find(std::string_view str,
                                         size_type pos = 0) const noexcept {
    return std::string_view{internalString}.find(str, pos);
  }

  [[nodiscard]] constexpr size_type find(char chr,
                                         size_type pos = 0) const noexcept {
    return std::string_view{internalString}.find(chr, pos);
  }

  [[nodiscard]] constexpr size_type find(const char *str, size_type pos,
                                         size_type n) const noexcept {
    return std::string_view{internalString}.find(str, pos, n);
  }

  [[nodiscard]] constexpr size_type find(const char *str,
                                         size_type pos = 0) const noexcept {
    return std::string_view{internalString}.find(str, pos);
  }

  [[nodiscard]] constexpr size_type
  rfind(std::string_view str,
        size_type pos = std::string_view::npos) const noexcept {
    return std::string_view{internalString}.rfind(str, pos);
  }

  [[nodiscard]] constexpr size_type
  rfind(char chr, size_type pos = std::string_view::npos) const noexcept {
    return std::string_view{internalString}.rfind(chr, pos);
  }

  [[nodiscard]] constexpr size_type rfind(const char *str, size_type pos,
                                          size_type n) const noexcept {
    return std::string_view{internalString}.rfind(str, pos, n);
  }

  [[nodiscard]] constexpr size_type
  rfind(const char *str,
        size_type pos = std::string_view::npos) const noexcept {
    return std::string_view{internalString}.rfind(str, pos);
  }

  [[nodiscard]] constexpr size_type
  find_first_of(std::string_view str, size_type pos = 0) const noexcept {
    return std::string_view{internalString}.find_first_of(str, pos);
  }
  [[nodiscard]] constexpr size_type
  find_first_of(char chr, size_type pos = 0) const noexcept {
    return std::string_view{internalString}.find_first_of(chr, pos);
  }

  [[nodiscard]] constexpr size_type
  find_first_of(const char *str, size_type pos, size_type n) const noexcept {
    return std::string_view{internalString}.find_first_of(str, pos, n);
  }

  [[nodiscard]] constexpr size_type
  find_first_of(const char *str, size_type pos = 0) const noexcept {
    return std::string_view{internalString}.find_first_of(str, pos);
  }
  [[nodiscard]] constexpr size_type
  find_last_of(std::string_view str,
               size_type pos = std::string_view::npos) const noexcept {
    return std::string_view{internalString}.find_last_of(str, pos);
  }

  [[nodiscard]] constexpr size_type
  find_last_of(char chr,
               size_type pos = std::string_view::npos) const noexcept {
    return std::string_view{internalString}.find_last_of(chr, pos);
  }

  [[nodiscard]] constexpr size_type find_last_of(const char *str, size_type pos,
                                                 size_type n) const noexcept {
    return std::string_view{internalString}.find_last_of(str, pos, n);
  }

  [[nodiscard]] constexpr size_type
  find_last_of(const char *str,
               size_type pos = std::string_view::npos) const noexcept {
    return std::string_view{internalString}.find_last_of(str, pos);
  }

  [[nodiscard]] constexpr size_type
  find_first_not_of(std::string_view str, size_type pos = 0) const noexcept {
    return std::string_view{internalString}.find_first_not_of(str, pos);
  }

  [[nodiscard]] constexpr size_type
  find_first_not_of(char chr, size_type pos = 0) const noexcept {
    return std::string_view{internalString}.find_first_not_of(chr, pos);
  }

  [[nodiscard]] constexpr size_type
  find_first_not_of(const char *str, size_type pos,
                    size_type n) const noexcept {
    return std::string_view{internalString}.find_first_not_of(str, pos, n);
  }

  [[nodiscard]] constexpr size_type
  find_first_not_of(const char *str, size_type pos = 0) const noexcept {
    return std::string_view{internalString}.find_first_not_of(str, pos);
  }

  [[nodiscard]] constexpr size_type
  find_last_not_of(std::string_view str,
                   size_type pos = std::string_view::npos) const noexcept {
    return std::string_view{internalString}.find_last_not_of(str, pos);
  }

  [[nodiscard]] constexpr size_type
  find_last_not_of(char chr,
                   size_type pos = std::string_view::npos) const noexcept {
    return std::string_view{internalString}.find_last_not_of(chr, pos);
  }

  [[nodiscard]] constexpr size_type
  find_last_not_of(const char *str, size_type pos, size_type n) const noexcept {
    return std::string_view{internalString}.find_last_not_of(str, pos, n);
  }

  [[nodiscard]] constexpr size_type
  find_last_not_of(const char *str,
                   size_type pos = std::string_view::npos) const noexcept {
    return std::string_view{internalString}.find_last_not_of(str, pos);
  }

  constexpr void clear() noexcept { internalString[0] = '\0'; }

  template <size_t S> constexpr String &assign(const String<S> &str) {
    return assign(str.c_str(), S);
  }
  constexpr String &assign(std::string_view str) {
    const size_t length = std::min(str.length(), Nm);
    traits_type::copy(internalString, str.data(), length);
    internalString[length] = '\0';
    return *this;
  }

  constexpr String &assign(const char *str, size_t length) {
    return assign(std::string_view{str, length});
  }

  template <size_t S> constexpr String &append(const String<S> &str) {
    return append(str.c_str());
  }
  constexpr String &append(std::string_view str) {
    return append(str.data(), str.length());
  }

  constexpr String &append(const char *str, size_t length) {
    auto currentLength = this->length();
    auto to_copy = std::min(length, Nm - currentLength);

    internalString[currentLength + to_copy] = '\0';
    std::copy(str, str + to_copy, internalString + currentLength);
    return *this;
  }
  constexpr String &operator+=(char chr) {
    push_back(chr);
    return *this;
  }
  constexpr String &operator+=(std::string_view str) { return append(str); }
  template <size_t S> constexpr String &operator+=(const String<S> &str) {
    return append(str);
  }

  constexpr void push_back(char chr) { append(&chr, 1); }
  constexpr void pop_back() noexcept { remove_suffix(1); }

  template <size_t T> constexpr String &operator=(const String<T> &str) {
    return assign(str);
  }

  template <size_t T>
  [[nodiscard]] constexpr int compare(const String<T> &str) const {
    return compare(data(), size(), str.data(), str.size());
  }
  [[nodiscard]] constexpr int compare(std::string_view str) const {
    return compare(data(), size(), str.data(), str.size());
  }

  constexpr void swap(String &rhs) noexcept {
    std::swap(internalString, rhs.internalString);
  }

  template <size_t Nm2>
  [[nodiscard]] friend constexpr bool
  operator==(const String &_lhs, const String<Nm2> &_rhs) noexcept {
    return _lhs.compare(_rhs) == 0;
  }
  [[nodiscard]] friend constexpr bool operator==(std::string_view _lhs,
                                                 const String &_rhs) noexcept {
    return _lhs.compare(_rhs.c_str()) == 0;
  }
  [[nodiscard]] friend constexpr bool
  operator==(const String &_lhs, std::string_view _rhs) noexcept {
    return _lhs.compare(_rhs) == 0;
  }

  template <size_t Nm2>
  [[nodiscard]] friend constexpr bool
  operator!=(const String &_lhs, const String<Nm2> &_rhs) noexcept {
    return !(_lhs == _rhs);
  }
  [[nodiscard]] friend constexpr bool operator!=(std::string_view _lhs,
                                                 const String &_rhs) noexcept {
    return !(_lhs == _rhs);
  }
  [[nodiscard]] friend constexpr bool
  operator!=(const String &_lhs, std::string_view _rhs) noexcept {
    return !(_lhs == _rhs);
  }

  template <size_t Nm2>
  [[nodiscard]] friend constexpr bool
  operator<(const String &_lhs, const String<Nm2> &_rhs) noexcept {
    return _lhs.compare(_rhs) < 0;
  }

  [[nodiscard]] friend constexpr bool operator<(std::string_view _lhs,
                                                const String &_rhs) noexcept {
    return _lhs.compare(_rhs.c_str()) < 0;
  }

  [[nodiscard]] friend constexpr bool
  operator<(const String &_lhs, std::string_view _rhs) noexcept {
    return _lhs.compare(_rhs) < 0;
  }

  template <size_t Nm2>
  [[nodiscard]] friend constexpr bool
  operator>(const String &_lhs, const String<Nm2> &_rhs) noexcept {
    return _lhs.compare(_rhs) > 0;
  }

  [[nodiscard]] friend constexpr bool operator>(std::string_view _lhs,
                                                const String &_rhs) noexcept {
    return _lhs.compare(_rhs.c_str()) > 0;
  }

  [[nodiscard]] friend constexpr bool
  operator>(const String &_lhs, std::string_view _rhs) noexcept {
    return _lhs.compare(_rhs) > 0;
  }

  template <size_t Nm2>
  [[nodiscard]] friend constexpr bool
  operator<=(const String &_lhs, const String<Nm2> &_rhs) noexcept {
    return _lhs.compare(_rhs) <= 0;
  }
  [[nodiscard]] friend constexpr bool operator<=(std::string_view _lhs,
                                                 const String &_rhs) noexcept {
    return _lhs.compare(_rhs.c_str()) <= 0;
  }
  [[nodiscard]] friend constexpr bool
  operator<=(const String &_lhs, std::string_view _rhs) noexcept {
    return _lhs.compare(_rhs) <= 0;
  }

  [[nodiscard]] friend constexpr bool operator<=(const char *_lhs,
                                                 const String &_rhs) noexcept {
    return _rhs.compare(_lhs) >= 0;
  }

  [[nodiscard]] friend constexpr bool operator<=(const String &_lhs,
                                                 const char *_rhs) noexcept {
    return _lhs.compare(_rhs) <= 0;
  }

  template <size_t Nm2>
  [[nodiscard]] friend constexpr bool
  operator>=(const String &_lhs, const String<Nm2> &_rhs) noexcept {
    return _lhs.compare(_rhs) >= 0;
  }

  [[nodiscard]] friend constexpr bool operator>=(std::string_view _lhs,
                                                 const String &_rhs) noexcept {
    return _lhs.compare(_rhs.c_str()) >= 0;
  }
  [[nodiscard]] friend constexpr bool
  operator>=(const String &_lhs, std::string_view _rhs) noexcept {
    return _lhs.compare(_rhs) >= 0;
  }

private:
  constexpr int compare(const char *str, size_t size, const char *ostr,
                        size_t osize) const {
    const auto len = std::min(size, osize);

    const int result = traits_type::compare(str, ostr, len);
    if (!result) {
      const auto diff = static_cast<difference_type>(size - osize);
      if (diff > std::numeric_limits<int>::max()) {
        return std::numeric_limits<int>::max();
      }
      if (diff < std::numeric_limits<int>::min()) {
        return std::numeric_limits<int>::min();
      }
      return static_cast<int>(diff);
    }
    return result;
  }
};

template <std::size_t Nm, std::size_t Nm2>
constexpr void swap(const String<Nm> &lhs, const String<Nm2> &rhs) noexcept {
  rhs.swap(lhs);
}

template <std::size_t Nm>
inline std::ostream &operator<<(std::ostream &os, const String<Nm> &str) {
  return os << str.c_str();
}
template <std::size_t Nm>
std::istream &operator>>(std::istream &is, String<Nm> &str) {
  std::string tmp;
  is >> tmp;
  str = tmp;
  return is;
}

template <std::size_t Nm, std::size_t Nm2>
[[nodiscard]] std::string operator+(const String<Nm> &_lhs,
                                    const String<Nm2> &_rhs) {
  return std::string(_lhs.c_str()) + _rhs;
}
template <std::size_t Nm>
[[nodiscard]] std::string operator+(std::string_view _lhs,
                                    const String<Nm> &_rhs) {
  return std::string(_lhs) + _rhs.c_str();
}
template <std::size_t Nm>
[[nodiscard]] std::string operator+(const String<Nm> &_lhs,
                                    std::string_view _rhs) {
  return std::string(_lhs.c_str()) + std::string(_rhs);
}

template <std::size_t Nm>
[[nodiscard]] std::string operator+(char _lhs, const String<Nm> &_rhs) {
  return std::string(1, _lhs) + _rhs.c_str();
}
template <std::size_t Nm>
[[nodiscard]] std::string operator+(const String<Nm> &_lhs, char _rhs) {
  return std::string(_lhs.c_str()) + _rhs;
}

} // namespace Xsmp

// hash
namespace std {
template <size_t N> struct hash<::Xsmp::String<N>> {
  size_t operator()(const ::Xsmp::String<N> &str) const {
    return std::hash<std::string_view>()(
        static_cast<std::string_view>(str.c_str()));
  }
};
} // namespace std
#endif // XSMP_STRING_H_
