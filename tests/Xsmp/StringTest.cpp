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

#include <Xsmp/String.h>
#include <iterator>
#include <string>
#include <string_view>

#include "gtest/gtest.h"

using namespace std::literals;
namespace Xsmp {

using String10 = String<10>;
using String16 = String<16>;

TEST(String, Constructor) {

  EXPECT_EQ("", String10{});

  EXPECT_EQ("string8", String10{"string8"});
  EXPECT_EQ("string8", String10{"string8"s});
  EXPECT_EQ("string8", String10{"string8"sv});
  EXPECT_EQ("st", (String10{"string8", 2}));

  const char charArray[] = "array";
  EXPECT_EQ("array", String10{charArray});

  const String16 other{"otherString"};
  EXPECT_EQ("otherStrin", String10{other});
}

TEST(String, Create) {
  // using type = std::string;
  using type = String10;

  type str;
  EXPECT_TRUE(str.empty());
  EXPECT_EQ(str.size(), 0);
  EXPECT_STREQ(str.c_str(), "");
  str = "0123456789";
  EXPECT_FALSE(str.empty());

  EXPECT_TRUE(str == str);
  EXPECT_FALSE(str != str);
  EXPECT_EQ(str.size(), 10);
  EXPECT_STREQ(str.c_str(), "0123456789");
  EXPECT_EQ(str.front(), '0');
  EXPECT_EQ(str.back(), '9');
  EXPECT_EQ(*str.begin(), '0');
  EXPECT_EQ(*str.end(), '\0');
  EXPECT_EQ(*str.cbegin(), '0');
  EXPECT_EQ(*str.cend(), '\0');
  EXPECT_EQ(*str.rbegin(), '9');
  EXPECT_EQ(*(str.rend() - 1), '0');
  EXPECT_EQ(*str.crbegin(), '9');
  EXPECT_EQ(*(str.crend() - 1), '0');
  EXPECT_EQ(str[0], '0');
  EXPECT_EQ(str[9], '9');
  EXPECT_EQ(str.at(0), '0');
  EXPECT_EQ(str.at(9), '9');

  int i = 0;
  for (const auto *it = str.begin(); it != str.end(); ++it) {
    EXPECT_EQ(*it, std::to_string(i++).front());
  }
  i = 0;
  for (const auto *it = str.cbegin(); it != str.cend(); ++it) {
    EXPECT_EQ(*it, std::to_string(i++).front());
  }
  i = 9;
  for (auto it = str.rbegin(); it != str.rend(); ++it) {
    EXPECT_EQ(*it, std::to_string(i--).front());
  }
  i = 9;
  for (auto it = str.crbegin(); it != str.crend(); ++it) {
    EXPECT_EQ(*it, std::to_string(i--).front());
  }

  str = "0123456789!!!!!!!!";
  EXPECT_EQ(str.size(), 10);
  EXPECT_STREQ(str.c_str(), "0123456789");

  str.assign("0123");
  EXPECT_STREQ(str.c_str(), "0123");

  i = 0;
  for (const auto *it = str.begin(); it != str.end(); ++it) {
    EXPECT_EQ(*it, std::to_string(i++).front());
  }

  i = 0;
  for (const auto *it = str.cbegin(); it != str.cend(); ++it) {
    EXPECT_EQ(*it, std::to_string(i++).front());
  }
  i = 3;
  for (auto it = str.rbegin(); it != str.rend(); ++it) {
    EXPECT_EQ(*it, std::to_string(i--).front());
  }
  i = 3;
  for (auto it = str.crbegin(); it != str.crend(); ++it) {
    EXPECT_EQ(*it, std::to_string(i--).front());
  }
  str.append("45");
  EXPECT_STREQ(str.c_str(), "012345");
  str.append("6789!!!!!!");
  EXPECT_STREQ(str.c_str(), "0123456789");
  str.remove_prefix(2);
  EXPECT_STREQ(str.c_str(), "23456789");
  str.remove_suffix(2);
  EXPECT_STREQ(str.c_str(), "234567");

  str.clear();
  EXPECT_STREQ(str.c_str(), "");

  str.assign("012345", 2);
  EXPECT_STREQ(str.c_str(), "01");

  str += "23";
  EXPECT_STREQ(str.c_str(), "0123");
  str += std::string("45");
  EXPECT_STREQ(str.c_str(), "012345");
  str += '6';
  EXPECT_STREQ(str.c_str(), "0123456");
  str.push_back('7');
  EXPECT_STREQ(str.c_str(), "01234567");
}

TEST(String, operatorPlusEqual) {
  String10 string;

  string += "1";
  EXPECT_EQ("1", string);

  string += "2"s;
  EXPECT_EQ("12", string);

  string += "3"sv;
  EXPECT_EQ("123", string);

  string += '4';
  EXPECT_EQ("1234", string);

  string += String16{"5"};
  EXPECT_EQ("12345", string);
}

TEST(String, push_back) {
  String10 string = "012345678";

  string.push_back('9');
  EXPECT_EQ("0123456789", string);

  string.push_back('0');
  EXPECT_EQ("0123456789", string);
}

TEST(String, pop_back) {
  String10 string = "01";

  string.pop_back();
  EXPECT_EQ("0", string);

  string.pop_back();
  EXPECT_EQ("", string);

  string.pop_back();
  EXPECT_EQ("", string);
}

TEST(String, operatorPlus) {

  const String10 string = "1";

  EXPECT_EQ("12", string + "2");
  EXPECT_EQ("21", "2" + string);

  EXPECT_EQ("13", string + "3"s);
  EXPECT_EQ("31", "3"s + string);

  EXPECT_EQ("14", string + "4"sv);
  EXPECT_EQ("41", "4"sv + string);

  const String16 string2 = "5";
  EXPECT_EQ("15", string + string2);
  EXPECT_EQ("51", string2 + string);

  EXPECT_EQ("16", string + '6');
  EXPECT_EQ("61", '6' + string);
}

TEST(String, operatorEqual) {

  String10 string10;
  String16 string16;

  // from string
  string10 = "01234567890123"s;
  EXPECT_EQ("0123456789", string10);

  string16 = "01234567890123"s;
  EXPECT_EQ("01234567890123", string16);

  // from string_view
  string10 = "01234567890123"sv;
  EXPECT_EQ("0123456789", string10);

  string16 = "01234567890123"sv;
  EXPECT_EQ("01234567890123", string16);

  // from Smp::String8
  string10 = "01234567890123";
  EXPECT_EQ("0123456789", string10);

  string16 = "01234567890123";
  EXPECT_EQ("01234567890123", string16);

  // from other String
  string10 = string16;
  EXPECT_EQ("0123456789", string10);
}

TEST(String, operatorEquality) {

  const String10 string = "1";
  const String16 string2 = "2";

  EXPECT_TRUE(string == "1");
  EXPECT_TRUE(string == "1"s);
  EXPECT_TRUE(string == "1"sv);
  EXPECT_FALSE(string == string2);

  EXPECT_FALSE(string != "1");
  EXPECT_FALSE(string != "1"s);
  EXPECT_FALSE(string != "1"sv);
  EXPECT_TRUE(string != string2);

  EXPECT_TRUE(string <= "1");
  EXPECT_TRUE(string <= "1"s);
  EXPECT_TRUE(string <= "1"sv);
  EXPECT_TRUE(string <= string2);

  EXPECT_TRUE(string >= "1");
  EXPECT_TRUE(string >= "1"s);
  EXPECT_TRUE(string >= "1"sv);
  EXPECT_FALSE(string >= string2);

  EXPECT_FALSE(string < "1");
  EXPECT_FALSE(string < "1"s);
  EXPECT_FALSE(string < "1"sv);
  EXPECT_TRUE(string < "2");
  EXPECT_TRUE(string < "2"s);
  EXPECT_TRUE(string < "2"sv);
  EXPECT_TRUE(string < string2);

  EXPECT_FALSE(string > "1");
  EXPECT_FALSE(string > "1"s);
  EXPECT_FALSE(string > "1"sv);
  EXPECT_TRUE(string > "0");
  EXPECT_TRUE(string > "0"s);
  EXPECT_TRUE(string > "0"sv);
  EXPECT_FALSE(string > string2);
}

} // namespace Xsmp
