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
#include <chrono>
#include <sstream>
#include <string>

#include "gtest/gtest.h"

namespace Xsmp {
/*
 constexpr Duration d1;
 static_assert(d1==0);

 constexpr Duration d2 = 42;
 static_assert(d2==42);

 constexpr Duration d3 { 42 };
 static_assert(d3==42);

 constexpr Duration d4 = d3;
 static_assert(d4==42);

 constexpr Duration d5 { d4 };
 static_assert(d5==42);

 constexpr std::chrono::microseconds us { 5 };
 constexpr Duration d6 { us };
 static_assert(d6==5000);
 static_assert(d6==us);
 static_assert(us==d6);

 constexpr Duration d7 = us;
 static_assert(d7==5000);
 static_assert(+d7==5000);
 static_assert(-d7==-5000);

 constexpr Duration d8 = d1 + d2;
 static_assert(d8-d2-d1==0);*/

TEST(Duration, constructor) {

  const Duration defaultConstructor;
  EXPECT_EQ(defaultConstructor, 0);
  EXPECT_EQ(defaultConstructor, std::chrono::seconds(0));

  const Duration fromSmpDuration{1000};
  EXPECT_EQ(fromSmpDuration, 1000);
  EXPECT_EQ(fromSmpDuration, std::chrono::microseconds(1));

  const Duration fromStdChrono{std::chrono::milliseconds(1)};
  EXPECT_EQ(fromStdChrono, 1000000);

  const Duration fromString{std::string("00:00:01.000000000")};
  EXPECT_EQ(fromString, 1000000000);
  EXPECT_EQ(fromString, std::chrono::seconds(1));

  const Duration fromConstCharPointer{"00:00:01.000000000"};
  EXPECT_EQ(fromConstCharPointer, 1000000000);
  EXPECT_EQ(fromConstCharPointer, std::chrono::seconds(1));

  std::stringstream ss;
  ss << "00:00:01.000000000";
  const Duration fromStream{ss};
  EXPECT_EQ(fromStream, 1000000000);
  EXPECT_EQ(fromStream, std::chrono::seconds(1));
}

TEST(Duration, operators) {

  EXPECT_GE(std::chrono::seconds(1), Duration{std::chrono::seconds(1)});
  EXPECT_GE(std::chrono::seconds(1), Duration{std::chrono::seconds(0)});
  EXPECT_GE(Duration{std::chrono::seconds(1)}, std::chrono::seconds(1));
  EXPECT_GE(Duration{std::chrono::seconds(1)}, std::chrono::seconds(0));

  EXPECT_LE(std::chrono::seconds(1), Duration{std::chrono::seconds(1)});
  EXPECT_LE(std::chrono::seconds(0), Duration{std::chrono::seconds(1)});
  EXPECT_LE(Duration{std::chrono::seconds(1)}, std::chrono::seconds(1));
  EXPECT_LE(Duration{std::chrono::seconds(0)}, std::chrono::seconds(1));
  EXPECT_LE(Duration{std::chrono::seconds(1)},
            Duration{std::chrono::seconds(1)});
  EXPECT_LE(Duration{std::chrono::seconds(0)},
            Duration{std::chrono::seconds(1)});

  EXPECT_GT(std::chrono::seconds(1), Duration{std::chrono::seconds(0)});
  EXPECT_GT(Duration{std::chrono::seconds(1)}, std::chrono::seconds(0));
  EXPECT_GT(Duration{std::chrono::seconds(1)},
            Duration{std::chrono::seconds(0)});

  EXPECT_LT(std::chrono::seconds(0), Duration{std::chrono::seconds(1)});
  EXPECT_LT(Duration{std::chrono::seconds(0)}, std::chrono::seconds(1));

  EXPECT_NE(std::chrono::seconds(1), Duration{std::chrono::seconds(0)});
  EXPECT_NE(Duration{std::chrono::seconds(1)}, std::chrono::seconds(0));

  EXPECT_EQ(std::chrono::seconds(1), Duration{std::chrono::seconds(1)});
  EXPECT_EQ(Duration{std::chrono::seconds(1)}, std::chrono::seconds(1));
}

TEST(Duration, literals) {

  EXPECT_EQ(1_w, Duration::weeks(1));
  EXPECT_EQ(1_d, Duration::days(1));
  EXPECT_EQ(1_h, Duration::hours(1));
  EXPECT_EQ(1_mn, Duration::minutes(1));
  EXPECT_EQ(1_s, Duration::seconds(1));
  EXPECT_EQ(1_ms, Duration::milliseconds(1));
  EXPECT_EQ(1_us, Duration::microseconds(1));
  EXPECT_EQ(1_ns, Duration::nanoseconds(1));
}

TEST(Duration, format) {

  const Duration fromConstCharPointer{"00:01:00.000000000"};
  EXPECT_EQ(fromConstCharPointer.format(), "00:01:00.000000000");
  EXPECT_EQ(fromConstCharPointer.format("%R"), "00:01");
  EXPECT_EQ(fromConstCharPointer.format(std::string("%R")), "00:01");
}

TEST(Duration, stream) {
#define EXPECT_STREAM_OPERATOR_EQ(literal, value)                              \
  do {                                                                         \
    std::stringstream ss;                                                      \
    ss << (literal);                                                           \
    EXPECT_EQ(ss.str(), (value));                                              \
  } while (false)

  EXPECT_STREAM_OPERATOR_EQ(1_ns, "00:00:00.000000001");
  EXPECT_STREAM_OPERATOR_EQ(1_us, "00:00:00.000001000");
  EXPECT_STREAM_OPERATOR_EQ(1_ms, "00:00:00.001000000");
  EXPECT_STREAM_OPERATOR_EQ(1_s, "00:00:01.000000000");
  EXPECT_STREAM_OPERATOR_EQ(1_mn, "00:01:00.000000000");
  EXPECT_STREAM_OPERATOR_EQ(1_h, "01:00:00.000000000");
  EXPECT_STREAM_OPERATOR_EQ(1_d, "24:00:00.000000000");
  EXPECT_STREAM_OPERATOR_EQ(1_w, "168:00:00.000000000");

#define EXPECT_STREAM_OPERATOR_FORMAT_EQ(literal, value, format)               \
  do {                                                                         \
    std::stringstream ss;                                                      \
    (literal).to_stream(ss, (format));                                         \
    EXPECT_EQ(ss.str(), (value));                                              \
  } while (false)

  EXPECT_STREAM_OPERATOR_FORMAT_EQ(1_h, "01:00:00.000000000", "%T");
  EXPECT_STREAM_OPERATOR_FORMAT_EQ(1_h, "01:00", "%R");
  EXPECT_STREAM_OPERATOR_FORMAT_EQ(1_h, "01:00:00.000000000",
                                   std::string("%T"));
  EXPECT_STREAM_OPERATOR_FORMAT_EQ(1_h, "01:00", std::string("%R"));
}

} // namespace Xsmp
