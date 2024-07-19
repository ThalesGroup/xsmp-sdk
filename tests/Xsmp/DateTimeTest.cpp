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

#include <Smp/PrimitiveTypes.h>
#include <Xsmp/DateTime.h>
#include <Xsmp/Duration.h>
#include <chrono>
#include <sstream>
#include <string>

#include "gtest/gtest.h"

namespace Xsmp {
/*
 TEST(DateTime, constexprFunctions) {

 constexpr DateTime d1;
 static_assert(d1==0,"");
 static_assert(0==d1,"");
 static_assert(d1!=1,"");
 static_assert(1!=d1,"");
 static_assert(d1<=0,"");
 static_assert(0<=d1,"");
 static_assert(d1>=0,"");
 static_assert(0>=d1,"");

 constexpr DateTime d2 = 42;
 static_assert(d2==42,"");
 //static_assert(d2==fortytwo_ns,"");

 constexpr DateTime d3 { 42 };
 static_assert(d3==42,"");

 constexpr DateTime d4 = d3;
 static_assert(d4==42,"");

 constexpr DateTime d5 { d4 };
 static_assert(d5==42,"");

 constexpr DateTime d6 = 1_ns + d5 + 1_ns;
 static_assert(d6==44,"");

 constexpr DateTime d7 = d5 * 2;
 static_assert(d7==84,"");
 }*/

TEST(DateTime, constructors) {

  const DateTime defaultDateTime;
  EXPECT_EQ(defaultDateTime, 0);

  const DateTime dateTimeFromSmp(Smp::DateTime{10});
  EXPECT_EQ(dateTimeFromSmp, 10);

  const DateTime dateTimeFromDuration(std::chrono::nanoseconds{100});
  EXPECT_EQ(dateTimeFromDuration, 100);

  EXPECT_EQ(DateTime{"2000-01-01 12:00:00"}, 0);

  const DateTime dateTimeFromCharPtrWithFormat("2000-01-01 12", "%Y-%m-%d %H");
  EXPECT_EQ(dateTimeFromCharPtrWithFormat, 0);

  const DateTime dateTimeFromStringWithFormat(std::string("2000-01-01 12"),
                                              "%Y-%m-%d %H");
  EXPECT_EQ(dateTimeFromStringWithFormat, 0);

  std::stringstream ss;
  ss << "2000-01-01 12:00:00";
  const DateTime fromStream{ss};
  EXPECT_EQ(fromStream, 0);
}

TEST(DateTime, format) {

  const DateTime date{"2023-08-09 16:00:35"};
  EXPECT_EQ(date.format(), "2023-08-09 16:00:35.000000000");
  EXPECT_EQ(date.format("%x"), "08/09/23");
  EXPECT_EQ(date.format(std::string("%x")), "08/09/23");
}

TEST(DateTime, operators) {

  DateTime date{"2023-08-09 16:00:35"};

  date++;
  EXPECT_EQ(date.format(), "2023-08-09 16:00:35.000000001");

  date--;
  EXPECT_EQ(date.format(), "2023-08-09 16:00:35.000000000");

  date += Duration{std::chrono::seconds(5)};
  EXPECT_EQ(date.format(), "2023-08-09 16:00:40.000000000");

  date -= Duration{std::chrono::seconds(10)};
  EXPECT_EQ(date.format(), "2023-08-09 16:00:30.000000000");

  date += std::chrono::seconds(15);
  EXPECT_EQ(date.format(), "2023-08-09 16:00:45.000000000");

  date -= std::chrono::seconds(15);
  EXPECT_EQ(date.format(), "2023-08-09 16:00:30.000000000");

  date = date + Duration{std::chrono::minutes(5)};
  EXPECT_EQ(date.format(), "2023-08-09 16:05:30.000000000");

  date = date - Duration{std::chrono::hours(1)};
  EXPECT_EQ(date.format(), "2023-08-09 15:05:30.000000000");

  date = Duration{std::chrono::seconds(10)} + date;
  EXPECT_EQ(date.format(), "2023-08-09 15:05:40.000000000");

  date = date + std::chrono::minutes(5);
  EXPECT_EQ(date.format(), "2023-08-09 15:10:40.000000000");

  date = date - std::chrono::minutes(5);
  EXPECT_EQ(date.format(), "2023-08-09 15:05:40.000000000");

  date = std::chrono::minutes(5) + date;
  EXPECT_EQ(date.format(), "2023-08-09 15:10:40.000000000");

  date += std::chrono::seconds{5};
  EXPECT_EQ(date.format(), "2023-08-09 15:10:45.000000000");

  date -= std::chrono::seconds{15};
  EXPECT_EQ(date.format(), "2023-08-09 15:10:30.000000000");
}

TEST(DateTime, stream) {
#define EXPECT_STREAM_OPERATOR_EQ(date, value)                                 \
  do {                                                                         \
    std::stringstream ss;                                                      \
    (date).to_stream(ss);                                                      \
    EXPECT_EQ(date, (value));                                                  \
  } while (false)

  EXPECT_STREAM_OPERATOR_EQ(DateTime{"2000-01-01 12:00:00"}, 0);
}

} // namespace Xsmp
