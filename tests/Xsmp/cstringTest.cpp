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
#include <gtest/gtest.h>
#include <utility>

using namespace std::literals;
namespace Xsmp {

TEST(cstring, Constructor) {

  cstring s = "aaa";
  EXPECT_STREQ("aaa", s.c_str());

  auto s2 = std::move(s);

  EXPECT_EQ(nullptr, s.c_str());
  EXPECT_STREQ("aaa", s2.c_str());
  auto s3{s2};

  EXPECT_STREQ("aaa", s2.c_str());
  EXPECT_STREQ("aaa", s3.c_str());
  s2 = "bbb";

  EXPECT_STREQ("bbb", s2.c_str());
  EXPECT_STREQ("aaa", s3.c_str());

  s3 = s2;
  EXPECT_STREQ("bbb", s2.c_str());
  EXPECT_STREQ("bbb", s3.c_str());
}

} // namespace Xsmp
