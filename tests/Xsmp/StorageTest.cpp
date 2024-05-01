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
#include <Xsmp/StorageReader.h>
#include <Xsmp/StorageWriter.h>
#include <cstdio>
#include <gtest/gtest.h>
#include <string>

#if !defined(__clang__) && defined(__GNUC__) && __GNUC__ <= 7
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif
namespace Xsmp {

TEST(StorageTest, Writer) {

  auto dir = testing::TempDir();
  auto filename = "Writer.bin";

  StorageWriter writer(dir.c_str(), filename);
  EXPECT_STREQ(writer.GetStateVectorFilePath(), dir.c_str());
  EXPECT_STREQ(writer.GetStateVectorFileName(), filename);

  StorageReader reader(dir.c_str(), filename);
  EXPECT_STREQ(reader.GetStateVectorFilePath(), dir.c_str());
  EXPECT_STREQ(reader.GetStateVectorFileName(), filename);

  std::remove((fs::path(dir) / filename).string().c_str());
}
TEST(StorageTest, Reader) {

  auto dir = testing::TempDir();
  auto filename = "Reader.bin";

  EXPECT_THROW(StorageReader reader(dir.c_str(), filename),
               ::Smp::CannotRestore);
}

TEST(StorageTest, StoreRestore) {

  auto dir = fs::path(testing::TempDir()).append("StorageTest").string();
  auto filename = "StoreRestore.bin";

  double value1 = 42.;
  double value2 = 43.;
  {
    StorageWriter writer(dir.c_str(), filename);
    writer.Store(&value1, sizeof(value1));
    writer.Store(&value2, sizeof(value2));
  }
  value1 = 0.0;
  value2 = 0.0;
  {
    StorageReader reader(dir.c_str(), filename);
    reader.Restore(&value1, sizeof(value1));
    reader.Restore(&value2, sizeof(value2));
  }
  EXPECT_EQ(value1, 42.);
  EXPECT_EQ(value2, 43.);

  std::remove(dir.c_str());
}

} // namespace Xsmp
