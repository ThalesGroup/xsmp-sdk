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

#ifndef XSMP_DURATION_H_
#define XSMP_DURATION_H_

#include <Smp/PrimitiveTypes.h>
#include <chrono>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>

namespace Xsmp {

struct Duration final {
  constexpr Duration() : _value{} {}
  constexpr explicit Duration(::Smp::Duration duration) : _value(duration) {}

  template <class Rep, class Period>
  constexpr explicit Duration(const std::chrono::duration<Rep, Period> &d)
      : _value{
            std::chrono::duration_cast<std::chrono::nanoseconds>(d).count()} {}
  explicit Duration(std::string_view date, const char *fmt = "%T");

  explicit Duration(std::istream &inputStream, const char *fmt = "%T");

  std::string format(const char *fmt = "%T") const;
  std::string format(const std::string &fmt) const;

  std::ostream &to_stream(std::ostream &outputStream,
                          const char *fmt = "%T") const;
  std::ostream &to_stream(std::ostream &outputStream,
                          const std::string &fmt) const;

  template <class Rep, class Period>
  inline Duration &operator=(const std::chrono::duration<Rep, Period> &d) {
    _value = std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();
    return *this;
  }
  inline Duration &operator=(const ::Smp::Duration &d) {
    _value = d;
    return *this;
  }

  constexpr Duration operator+() const { return *this; }
  constexpr Duration operator-() const { return Duration(-_value); }

  Duration &operator+=(const ::Smp::Duration &d) noexcept {
    _value += d;
    return *this;
  }
  template <class Rep, class Period>
  Duration &operator+=(const std::chrono::duration<Rep, Period> &d) noexcept {
    _value += std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();
    return *this;
  }
  Duration &operator-=(const ::Smp::Duration &d) noexcept {
    _value -= d;
    return *this;
  }
  template <class Rep, class Period>
  Duration &operator-=(const std::chrono::duration<Rep, Period> &d) noexcept {
    _value -= std::chrono::duration_cast<std::chrono::nanoseconds>(d).count();
    return *this;
  }

  constexpr operator ::Smp::Duration() const noexcept { return _value; }

  template <class Rep, class Period>
  constexpr explicit operator std::chrono::duration<Rep, Period>() {
    return std::chrono::duration_cast<std::chrono::duration<Rep, Period>>(
        std::chrono::nanoseconds(_value));
  }
  // TODO handle operator +,-,*,/,+=,-=,*=,/=, ...

  template <typename T> Duration &operator=(T _i) noexcept {
    _value = _i;
    return *this;
  }
  Duration operator++(int) noexcept {
    auto tmp(*this);
    ++(*this);
    return tmp;
  }
  Duration operator--(int) noexcept {
    auto tmp(*this);
    --(*this);
    return tmp;
  }
  Duration &operator++() noexcept {
    ++_value;
    return *this;
  }
  Duration &operator--() noexcept {
    --_value;
    return *this;
  }
  template <typename T> Duration &operator<<=(const T &_i) noexcept {
    _value <<= _i;
    return *this;
  }
  template <typename T> Duration &operator>>=(const T &_i) noexcept {
    _value >>= _i;
    return *this;
  }
  template <typename T> Duration &operator+=(const T &_i) noexcept {
    _value += _i;
    return *this;
  }
  template <typename T> Duration &operator-=(const T &_i) noexcept {
    _value -= _i;
    return *this;
  }
  template <typename T> Duration &operator*=(const T &_i) noexcept {
    _value *= _i;
    return *this;
  }
  template <typename T> Duration &operator/=(const T &_i) noexcept {
    _value /= _i;
    return *this;
  }
  template <typename T> Duration &operator%=(const T &_i) noexcept {
    _value %= _i;
    return *this;
  }
  template <typename T> Duration &operator&=(const T &_i) noexcept {
    _value &= _i;
    return *this;
  }
  template <typename T> Duration &operator|=(const T &_i) noexcept {
    _value |= _i;
    return *this;
  }
  template <typename T> Duration &operator^=(const T &_i) noexcept {
    _value ^= _i;
    return *this;
  }
  template <typename T>
  friend constexpr bool operator==(const Duration &lhs, const T &rhs) noexcept {
    return lhs._value == rhs;
  }
  template <typename Rep, typename Period>
  friend constexpr bool
  operator==(const Duration &lhs,
             const std::chrono::duration<Rep, Period> &rhs) noexcept {
    return lhs._value ==
           std::chrono::duration_cast<std::chrono::nanoseconds>(rhs).count();
  }
  template <typename T>
  friend constexpr bool operator!=(const Duration &lhs, const T &rhs) noexcept {
    return !(lhs == rhs);
  }
  template <typename T>
  friend constexpr bool operator<(const Duration &lhs, const T &rhs) noexcept {
    return lhs._value < rhs;
  }
  template <typename Rep, typename Period>
  friend constexpr bool
  operator<(const Duration &lhs,
            const std::chrono::duration<Rep, Period> &rhs) noexcept {
    return lhs._value <
           std::chrono::duration_cast<std::chrono::nanoseconds>(rhs).count();
  }
  template <typename Rep, typename Period>
  friend constexpr bool
  operator>(const Duration &lhs,
            const std::chrono::duration<Rep, Period> &rhs) noexcept {
    return lhs._value >
           std::chrono::duration_cast<std::chrono::nanoseconds>(rhs).count();
  }
  template <typename Rep, typename Period>
  friend constexpr bool
  operator<=(const Duration &lhs,
             const std::chrono::duration<Rep, Period> &rhs) noexcept {
    return lhs._value <=
           std::chrono::duration_cast<std::chrono::nanoseconds>(rhs).count();
  }
  template <typename T>
  friend constexpr bool operator>(const Duration &lhs, const T &rhs) noexcept {
    return rhs < lhs;
  }
  template <typename T>
  friend constexpr bool operator<=(const Duration &lhs, const T &rhs) noexcept {
    return !(lhs > rhs);
  }
  template <typename T>
  friend constexpr bool operator>=(const Duration &lhs, const T &rhs) noexcept {
    return !(lhs < rhs);
  }

  // comparison operator for std::chrono::duration
  template <typename Rep, typename Period>
  friend constexpr bool operator==(const std::chrono::duration<Rep, Period> &_i,
                                   const Duration &d) noexcept {
    return d == _i;
  }
  template <typename Rep, typename Period>
  friend constexpr bool operator!=(const std::chrono::duration<Rep, Period> &_i,
                                   const Duration &d) noexcept {
    return d != _i;
  }

  template <typename Rep, typename Period>
  friend constexpr bool operator<(const std::chrono::duration<Rep, Period> &_i,
                                  const Duration &d) noexcept {
    return d > _i;
  }
  template <typename Rep, typename Period>
  friend constexpr bool operator>(const std::chrono::duration<Rep, Period> &rhs,
                                  const Duration &d) noexcept {
    return d < rhs;
  }
  template <typename Rep, typename Period>
  friend constexpr bool
  operator<=(const std::chrono::duration<Rep, Period> &rhs,
             const Duration &d) noexcept {
    return d >= rhs;
  }
  template <typename Rep, typename Period>
  friend constexpr bool
  operator>=(const std::chrono::duration<Rep, Period> &rhs,
             const Duration &d) noexcept {
    return d <= rhs;
  }

  /// operator for nano seconds (identity)
  static constexpr ::Xsmp::Duration nanoseconds(::Smp::Int64 value) {
    return Duration{value};
  }
  /// operator for micro seconds (1000 nanoseconds)
  static constexpr ::Xsmp::Duration microseconds(::Smp::Int64 value) {
    return Duration{value * nanoseconds(1000)};
  }

  /// operator for mili seconds (1000 microseconds)
  static constexpr ::Xsmp::Duration milliseconds(::Smp::Int64 value) {
    return Duration{value * microseconds(1000)};
  }

  /// operator for seconds (1000 milliseconds)
  static constexpr ::Xsmp::Duration seconds(::Smp::Int64 value) {
    return Duration{value * milliseconds(1000)};
  }

  /// operator for micro minutes (60 seconds)
  static constexpr ::Xsmp::Duration minutes(::Smp::Int64 value) {
    return Duration{value * seconds(60)};
  }

  /// operator for hours (60 minutes)
  static constexpr ::Xsmp::Duration hours(::Smp::Int64 value) {
    return Duration{value * minutes(60)};
  }
  /// operator for days (24 hours)
  static constexpr ::Xsmp::Duration days(::Smp::Int64 value) {
    return Duration{value * hours(24)};
  }

  /// operator for weeks (7 days)
  static constexpr ::Xsmp::Duration weeks(::Smp::Int64 value) {
    return Duration{value * days(7)};
  }

  friend std::ostream &operator<<(std::ostream &outputStream,
                                  const Duration &duration);

private:
  ::Smp::Duration _value;
};

/// Provides ::Xsmp::Duration literals for _ns/_us/_ms/_s/_mn/_h/_d
/// e.g: 5_mn + 10_s + 3_ms
inline namespace literals {

/// operator for nano seconds (identity)
constexpr Duration operator"" _ns(unsigned long long int value) {
  return Duration{static_cast<::Smp::Duration>(value)};
}
/// operator for micro seconds (1000 nano seconds)
constexpr Duration operator"" _us(unsigned long long int value) {
  return Duration{static_cast<::Smp::Duration>(value) * 1000_ns};
}

/// operator for mili seconds (1000 micro seconds)
constexpr Duration operator"" _ms(unsigned long long int value) {
  return Duration{static_cast<::Smp::Duration>(value) * 1000_us};
}

/// operator for seconds (1000 mili seconds)
constexpr Duration operator"" _s(unsigned long long int value) {
  return Duration{static_cast<::Smp::Duration>(value) * 1000_ms};
}

/// operator for micro minutes (60 seconds)
constexpr Duration operator"" _mn(unsigned long long int value) {
  return Duration{static_cast<::Smp::Duration>(value) * 60_s};
}

/// operator for hours (60 minutes)
constexpr Duration operator"" _h(unsigned long long int value) {
  return Duration{static_cast<::Smp::Duration>(value) * 60_mn};
}
/// operator for days (24 hours)
constexpr Duration operator"" _d(unsigned long long int value) {
  return Duration{static_cast<::Smp::Duration>(value) * 24_h};
}

/// operator for weeks (7 days)
constexpr Duration operator"" _w(unsigned long long int value) {
  return Duration{static_cast<::Smp::Duration>(value) * 7_d};
}

} // namespace literals
} // namespace Xsmp

#endif // XSMP_DURATION_H_
