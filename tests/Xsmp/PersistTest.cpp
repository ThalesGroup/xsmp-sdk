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

#include <Smp/CannotRestore.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Object.h>
#include <Xsmp/Persist.h>
#include <Xsmp/Persist/StdArray.h>
#include <Xsmp/Persist/StdAtomic.h>
#include <Xsmp/Persist/StdMap.h>
#include <Xsmp/Persist/StdPair.h>
#include <Xsmp/Persist/StdSet.h>
#include <Xsmp/Persist/StdString.h>
#include <Xsmp/Persist/StdUnorderedMap.h>
#include <Xsmp/Persist/StdUnorderedSet.h>
#include <Xsmp/Persist/StdVector.h>
#include <Xsmp/Storage.h>
#include <array>
#include <atomic>
#include <gtest/gtest.h>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Xsmp::Persist {

enum class E1 { L1, L2, L3 };

TEST(Persist, SimpleType) {
  Object sender{"sender", "", nullptr};

  Storage storage;

  const ::Smp::Bool b = true;
  const ::Smp::Float32 f = 42.0;
  const ::Smp::Int64 i = 1024;
  const E1 e = E1::L2;
  Store(nullptr, &sender, &storage, b, f, i, e);

  bool b_r = false;
  ::Smp::Float32 f_r = 0.F;
  ::Smp::Int64 i_r = 0;
  E1 e_r = E1::L1;

  Restore(nullptr, &sender, &storage, b_r, f_r, i_r, e_r);
  EXPECT_EQ(b, b_r);
  EXPECT_EQ(f, f_r);
  EXPECT_EQ(e, e_r);
}

TEST(Persist, CannotRestore) {

  Object sender{"sender", "", nullptr};

  Storage storage;

  const ::Smp::Bool b = true;
  const ::Smp::Float32 f = 42.0;
  const ::Smp::Int64 i = 1024;
  const E1 e = E1::L2;
  Store(nullptr, &sender, &storage, b, f, i, e);

  bool b_r = false;
  ::Smp::Float32 f_r = 0.F;
  ::Smp::Int64 i_r = 0;
  E1 e_r = E1::L1;
  EXPECT_THROW(Restore(nullptr, &sender, &storage, b_r, i_r, f_r, e_r),
               ::Smp::CannotRestore);
}

TEST(Persist, Atomic) {

  Object sender{"sender", "", nullptr};
  Storage storage;

  const std::atomic<::Smp::Bool> b{true};
  const std::atomic<::Smp::Float32> f{42.0};
  const std::atomic<::Smp::Int64> i{1024};
  const std::atomic<E1> e{E1::L2};
  Store(nullptr, &sender, &storage, b, f, i, e);

  std::atomic<bool> b_r;
  std::atomic<::Smp::Float32> f_r;
  std::atomic<::Smp::Int64> i_r;
  std::atomic<E1> e_r;

  Restore(nullptr, &sender, &storage, b_r, f_r, i_r, e_r);
  EXPECT_EQ(b, b_r);
  EXPECT_EQ(f, f_r);
  EXPECT_EQ(e, e_r);
}
TEST(Persist, String) {

  Object sender{"sender", "", nullptr};
  Storage storage;

  const std::string s = "aaaadfefe";
  const std::string s2 = "aaaadf\0efe\n";
  const std::string s3;

  Store(nullptr, &sender, &storage, s, s2, s3);

  std::string s_r;
  std::string s2_r;
  std::string s3_r;

  Restore(nullptr, &sender, &storage, s_r, s2_r, s3_r);
  EXPECT_EQ(s, s_r);
  EXPECT_EQ(s2, s2_r);
  EXPECT_EQ(s3, s3_r);
}
TEST(Persist, Vector) {

  Object sender{"sender", "", nullptr};
  Storage storage;

  const std::vector<int> v = {1, 2, 3, 4, 5};
  const std::vector<std::string> v2 = {"1", "2", "3", "4", "5"};

  Store(nullptr, &sender, &storage, v, v2);

  std::vector<int> v_r;
  std::vector<std::string> v2_r;

  Restore(nullptr, &sender, &storage, v_r, v2_r);
  EXPECT_EQ(v, v_r);
  EXPECT_EQ(v2, v2_r);
}

TEST(Persist, Map) {

  Object sender{"sender", "", nullptr};
  Storage storage;

  const std::map<int, int> v = {{1, 5}, {2, 2}, {3, 9}, {4, 0}, {5, -5}};
  const std::map<std::string, int> v2 = {
      {"1", 1}, {"2", 2}, {"3", 3}, {"4", 4}, {"5", 5}};

  Store(nullptr, &sender, &storage, v, v2);

  std::map<int, int> v_r;
  std::map<std::string, int> v2_r;

  Restore(nullptr, &sender, &storage, v_r, v2_r);
  EXPECT_EQ(v, v_r);
  EXPECT_EQ(v2, v2_r);
}

TEST(Persist, UnorderedMap) {

  Object sender{"sender", "", nullptr};
  Storage storage;

  const std::unordered_map<int, int> v = {
      {1, 5}, {2, 2}, {3, 9}, {4, 0}, {5, -5}};
  const std::unordered_map<std::string, int> v2 = {
      {"1", 1}, {"2", 2}, {"3", 3}, {"4", 4}, {"5", 5}};

  Store(nullptr, &sender, &storage, v, v2);

  std::unordered_map<int, int> v_r;
  std::unordered_map<std::string, int> v2_r;

  Restore(nullptr, &sender, &storage, v_r, v2_r);
  EXPECT_EQ(v, v_r);
  EXPECT_EQ(v2, v2_r);
}

TEST(Persist, Set) {

  Object sender{"sender", "", nullptr};
  Storage storage;

  const std::set<int> v = {1, 2, 3, 4, 5};
  const std::set<std::string> v2 = {"1", "2", "3", "4", "5"};

  Store(nullptr, &sender, &storage, v, v2);

  std::set<int> v_r;
  std::set<std::string> v2_r;

  Restore(nullptr, &sender, &storage, v_r, v2_r);
  EXPECT_EQ(v, v_r);
  EXPECT_EQ(v2, v2_r);
}

TEST(Persist, UnorderedSet) {

  Object sender{"sender", "", nullptr};
  Storage storage;

  const std::unordered_set<int> v = {1, 2, 3, 4, 5};
  const std::unordered_set<std::string> v2 = {"1", "2", "3", "4", "5"};

  Store(nullptr, &sender, &storage, v, v2);

  std::unordered_set<int> v_r;
  std::unordered_set<std::string> v2_r;

  Restore(nullptr, &sender, &storage, v_r, v2_r);
  EXPECT_EQ(v, v_r);
  EXPECT_EQ(v2, v2_r);
}

TEST(Persist, Pair) {

  Object sender{"sender", "", nullptr};
  Storage storage;

  const std::pair<int, double> v = {1, 2.47};
  const std::pair<std::string, bool> v2 = {"1", true};

  Store(nullptr, &sender, &storage, v, v2);

  std::pair<int, double> v_r;
  std::pair<std::string, bool> v2_r;

  Restore(nullptr, &sender, &storage, v_r, v2_r);
  EXPECT_EQ(v, v_r);
  EXPECT_EQ(v2, v2_r);
}

TEST(Persist, Array) {

  Object sender{"sender", "", nullptr};
  Storage storage;

  const std::array<int, 5> v = {1, 2, 3, 4, 5};
  const std::array<std::string, 5> v2 = {"1", "2", "3", "4", "5"};

  Store(nullptr, &sender, &storage, v, v2);

  std::array<int, 5> v_r;
  std::array<std::string, 5> v2_r;

  Restore(nullptr, &sender, &storage, v_r, v2_r);
  EXPECT_EQ(v, v_r);
  EXPECT_EQ(v2, v2_r);
}

TEST(Persist, RawArray) {

  Object sender{"sender", "", nullptr};
  Storage storage;

  int v[] = {1, 2, 3, 4, 5};
  std::string v2[] = {"1", "2", "3", "4", "5"};

  Store(nullptr, &sender, &storage, v, v2);

  decltype(v) v_r;
  decltype(v2) v2_r;

  Restore(nullptr, &sender, &storage, v_r, v2_r);

  for (int i = 0; i < 5; ++i) {
    EXPECT_EQ(v[i], v_r[i]);
    EXPECT_EQ(v2[i], v2_r[i]);
  }
}

} // namespace Xsmp::Persist
