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

#include <Smp/Exception.h>
#include <Smp/Uuid.h>
#include <gtest/gtest.h>

constexpr const char *uuid1_str = "123e4567-e89b-12d3-a456-426614174005";
constexpr const char *uuid2_str = "123e4567-e89b-12d3-a456-426614174006";
constexpr const char *uuid3_str = "123e4567-e89c-12d3-a456-426614174005";
namespace Smp {

TEST(Uuid, constructor) {

  Uuid uuid1{uuid1_str};

  EXPECT_EQ(uuid1.Data1, 0x123e4567);
  EXPECT_EQ(uuid1.Data2[0], 0xe89b);
  EXPECT_EQ(uuid1.Data2[1], 0x12d3);
  EXPECT_EQ(uuid1.Data2[2], 0xa456);

  EXPECT_EQ(uuid1.Data3[0], 0x42);
  EXPECT_EQ(uuid1.Data3[1], 0x66);
  EXPECT_EQ(uuid1.Data3[2], 0x14);
  EXPECT_EQ(uuid1.Data3[3], 0x17);
  EXPECT_EQ(uuid1.Data3[4], 0x40);
  EXPECT_EQ(uuid1.Data3[5], 0x05);

  EXPECT_THROW(Uuid{nullptr}, ::Smp::Exception);
  EXPECT_THROW(Uuid{""}, ::Smp::Exception);

  for (int i = 0; i < 36; ++i) {
    char uuid[37] = "123e4567-e89b-12d3-a456-426614174005";
    uuid[i] = 'x';
    EXPECT_THROW(Uuid{uuid}, ::Smp::Exception);
  }

  EXPECT_THROW(Uuid{"123e4567-e89b-12d3-a456-4266141740056"}, ::Smp::Exception);
  EXPECT_THROW(Uuid{"123e4567-e89b-12d3-a456-42661417400"}, ::Smp::Exception);
}

TEST(Uuid, operators) {

  const Uuid uuid1{uuid1_str};
  const Uuid uuid1b{uuid1_str};
  const Uuid uuid2{uuid2_str};
  const Uuid uuid3{uuid3_str};

  EXPECT_TRUE(uuid1 == uuid1);
  EXPECT_TRUE(uuid1 == uuid1b);
  EXPECT_TRUE(uuid1 != uuid2);
  EXPECT_TRUE(uuid1 != uuid3);
  EXPECT_TRUE(uuid1 < uuid2);
  EXPECT_TRUE(uuid1 < uuid3);

  EXPECT_TRUE(uuid2 != uuid3);
  EXPECT_TRUE(uuid2 < uuid3);

  Smp::Uuid u1{0, 0, 0, 0, 0};
  Smp::Uuid u2{0, 0, 0, 0, 0};
  EXPECT_FALSE(u1 < u2);
  EXPECT_TRUE(u1 == u2);
  EXPECT_FALSE(u1 != u2);
  u2.Data3 = {0, 0, 0, 0, 0, 1};
  EXPECT_TRUE(u1 < u2);
  EXPECT_FALSE(u1 == u2);
  EXPECT_TRUE(u1 != u2);
  u2.Data3 = {0, 0, 0, 0, 0, 0};
  u2.Data2 = {0, 0, 1};
  EXPECT_TRUE(u1 < u2);
  EXPECT_FALSE(u1 == u2);
  EXPECT_TRUE(u1 != u2);
  u2.Data2 = {0, 0, 0};
  u2.Data1 = 1;
  EXPECT_TRUE(u1 < u2);
  EXPECT_FALSE(u1 == u2);
  EXPECT_TRUE(u1 != u2);
  u2.Data1 = 0;

  u1.Data3 = {0, 0, 0, 0, 0, 1};
  EXPECT_FALSE(u1 < u2);
  EXPECT_FALSE(u1 == u2);
  EXPECT_TRUE(u1 != u2);
  u1.Data3 = {0, 0, 0, 0, 0, 0};
  u1.Data2 = {0, 0, 1};
  EXPECT_FALSE(u1 < u2);
  EXPECT_FALSE(u1 == u2);
  EXPECT_TRUE(u1 != u2);
  u1.Data2 = {0, 0, 0};
  u1.Data1 = 1;
  EXPECT_FALSE(u1 < u2);
  EXPECT_FALSE(u1 == u2);
  EXPECT_TRUE(u1 != u2);
}

TEST(Uuid, hash) {

  const Uuid uuid1{uuid1_str};
  const Uuid uuid1b{uuid1_str};
  const Uuid uuid2{uuid2_str};
  const Uuid uuid3{uuid3_str};
  const std::hash<Uuid> hash;
  EXPECT_EQ(hash(uuid1), hash(uuid1b));
  EXPECT_NE(hash(uuid1), hash(uuid2));
  EXPECT_NE(hash(uuid1), hash(uuid3));
  EXPECT_NE(hash(uuid2), hash(uuid3));
}

} // namespace Smp
