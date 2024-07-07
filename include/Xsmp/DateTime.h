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

#ifndef XSMP_DATETIME_H_
#define XSMP_DATETIME_H_

#include <Smp/PrimitiveTypes.h>
#include <chrono>
#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>

namespace Xsmp {

///
///
///
struct DateTime final {

  /// A DateTime initialized to MJD2000 +0.5
  constexpr DateTime() : _value{} {}

  DateTime(const DateTime &) = default;

  ~DateTime() = default;
  DateTime &operator=(const DateTime &) = default;

  /// A DateTime initialized with an ::Smp::DateTime
  ///
  /// @param dateTime the DateTime
  constexpr explicit DateTime(::Smp::DateTime dateTime) : _value(dateTime) {}
  /// A DateTime initialized from a c++ time_point
  ///
  /// @tparam Clock the time_point clock
  /// @tparam FromDuration the time_point from duration
  /// @param tp the time_point
  template <class Clock, class FromDuration>
  constexpr explicit DateTime(
      const std::chrono::time_point<Clock, FromDuration> &tp)
      : _value{convert(tp)} {}
  /// A DateTime initialized from a c++ duration
  ///
  /// @tparam Rep the duration rep
  /// @tparam Period the duration period
  /// @param d the duration relative to MJD2000 +0.5
  template <class Rep, class Period>
  constexpr explicit DateTime(
      const std::chrono::duration<Rep, Period> &dateTime)
      : _value{std::chrono::duration_cast<std::chrono::nanoseconds>(dateTime)
                   .count()} {}
  /// A DateTime initialized from a string date and an optional format
  ///
  /// @param date the string date
  /// @param fmt the date format
  /// @see https://en.cppreference.com/w/cpp/chrono/duration/formatter
  explicit DateTime(std::string_view date, const char *fmt = "%F %T");

  /// A DateTime initialized from a stream and an optional format
  ///
  /// @param is the input stream
  /// @param fmt the date format
  /// @see https://en.cppreference.com/w/cpp/chrono/duration/formatter
  explicit DateTime(std::istream &inputStream, const char *fmt = "%F %T");

  /// Format the current DateTime
  /// @param fmt the date format
  /// @return the formatted DateTime
  std::string format(const char *fmt = "%F %T") const;

  /// Format the current DateTime
  /// @param fmt the date format
  /// @return the formatted DateTime
  std::string format(const std::string &fmt) const;

  /// Output the current DateTime to the stream
  /// @param os the stream
  /// @param fmt the date format
  /// @return the stream
  std::ostream &to_stream(std::ostream &outputStream,
                          const char *fmt = "%F %T") const;

  /// Output the current DateTime to the stream
  /// @param os the stream
  /// @param fmt the date format
  /// @return the stream
  std::ostream &to_stream(std::ostream &outputStream,
                          const std::string &fmt) const;

  template <class Clock, class FromDuration>
  constexpr DateTime &
  operator=(const std::chrono::time_point<Clock, FromDuration> &tp) {
    _value = convert(tp);
    return *this;
  }

  /// A DateTime initialized from a c++ duration
  ///
  /// @tparam Rep the duration rep
  /// @tparam Period the duration period
  /// @param d the duration relative to MJD2000 +0.5
  template <class Rep, class Period>
  constexpr DateTime &
  operator=(const std::chrono::duration<Rep, Period> &dateTime) {
    _value = convert(dateTime);
    return *this;
  }
  constexpr DateTime &operator=(Smp::DateTime dateTime) {
    _value = dateTime;
    return *this;
  }

  /// Create a DateTime from the current system clock
  ///
  /// @return the current DateTime
  static DateTime now();

  constexpr DateTime &operator++() noexcept {
    ++_value;
    return *this;
  }
  constexpr DateTime operator++(int) noexcept {
    auto tmp(*this);
    ++(*this);
    return tmp;
  }
  constexpr DateTime &operator--() noexcept {
    --_value;
    return *this;
  }
  constexpr DateTime operator--(int) noexcept {
    auto tmp(*this);
    --(*this);
    return tmp;
  }

  constexpr DateTime &operator+=(const ::Smp::Duration &duration) noexcept {
    _value += duration;
    return *this;
  }
  constexpr DateTime &operator-=(const ::Smp::Duration &duration) noexcept {
    _value -= duration;
    return *this;
  }

  template <class Rep, class Period>
  constexpr DateTime &
  operator+=(const std::chrono::duration<Rep, Period> &duration) noexcept {

    _value +=
        std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    return *this;
  }
  template <class Rep, class Period>
  constexpr DateTime &
  operator-=(const std::chrono::duration<Rep, Period> &duration) noexcept {
    _value -=
        std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    return *this;
  }
  constexpr /*explicit*/ operator ::Smp::DateTime() const noexcept {
    return _value;
  }

  template <class Clock, class FromDuration>
  constexpr explicit
  operator std::chrono::time_point<Clock, FromDuration>() const noexcept {
    return std::chrono::time_point<Clock, FromDuration>(
        std::chrono::duration_cast<FromDuration>(
            std::chrono::nanoseconds(_value + _epochToMjd)));
  }

  constexpr friend DateTime
  operator+(const DateTime &dateTime,
            const ::Smp::Duration &duration) noexcept {
    return DateTime{static_cast<::Smp::DateTime>(dateTime) + duration};
  }
  constexpr friend DateTime operator+(const ::Smp::Duration &duration,
                                      const DateTime &dateTime) noexcept {
    return DateTime{duration + static_cast<::Smp::DateTime>(dateTime)};
  }
  constexpr friend DateTime
  operator-(const DateTime &dateTime,
            const ::Smp::Duration &duration) noexcept {
    return DateTime{static_cast<::Smp::DateTime>(dateTime) - duration};
  }

  template <class Rep, class Period>
  constexpr friend DateTime
  operator+(const DateTime &dateTime,
            const std::chrono::duration<Rep, Period> &duration) noexcept {
    return DateTime{
        static_cast<::Smp::DateTime>(dateTime) +
        std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()};
  }
  template <class Rep, class Period>
  constexpr friend DateTime
  operator+(const std::chrono::duration<Rep, Period> &duration,
            const DateTime &dateTime) noexcept {
    return DateTime{
        std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() +
        static_cast<::Smp::DateTime>(dateTime)};
  }
  template <class Rep, class Period>
  constexpr friend DateTime
  operator-(const DateTime &dateTime,
            const std::chrono::duration<Rep, Period> &duration) noexcept {
    return DateTime{
        static_cast<::Smp::DateTime>(dateTime) -
        std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()};
  }

  friend std::ostream &operator<<(std::ostream &outputStream,
                                  const DateTime &dateTime);

  // a platform independent time_point in nanoseconds
  using time_point = std::chrono::time_point<std::chrono::system_clock,
                                             std::chrono::nanoseconds>;

private:
  /// ::Smp::DateTime is expressed in nanoseconds, relative to the reference
  /// time of 01.01.2000, 12:00, Modified Julian Date (MJD) 2000+0.5; epoch time
  /// is 01.01.1970, 00:00 there are 946728000 seconds between epoch time and
  /// MJD2000 +0.5
  static constexpr ::Smp::Duration _epochToMjd = 946728000LL * 1000000000LL;

  ::Smp::DateTime _value;

  template <class Clock, class FromDuration>
  constexpr ::Smp::DateTime
  convert(const std::chrono::time_point<Clock, FromDuration> &timePoint)
      const noexcept {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
               timePoint.time_since_epoch())
               .count() -
           _epochToMjd;
  }
};

} // namespace Xsmp

#endif // XSMP_DATETIME_H_
