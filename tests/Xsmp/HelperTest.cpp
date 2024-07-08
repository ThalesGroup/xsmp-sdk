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

#include <Smp/InvalidObjectName.h>
#include <Xsmp/Helper.h>
#include <gtest/gtest.h>

namespace Xsmp {

TEST(Helper, checkName) {

  EXPECT_THROW(::Xsmp::Helper::checkName(nullptr, nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("0aaa", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("_aaa", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("abczABCZ0129_[", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("[abc]", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("[0]a", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("AZé", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("[]", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("[4][3]", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("[432", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("[", nullptr),
               ::Smp::InvalidObjectName);
  EXPECT_THROW(::Xsmp::Helper::checkName("[43][", nullptr),
               ::Smp::InvalidObjectName);

  EXPECT_NO_THROW(::Xsmp::Helper::checkName("aZZZ___4541", nullptr));
  EXPECT_NO_THROW(::Xsmp::Helper::checkName("A", nullptr));
  EXPECT_NO_THROW(::Xsmp::Helper::checkName("[4]", nullptr));
  EXPECT_NO_THROW(::Xsmp::Helper::checkName("[43]", nullptr));
  EXPECT_NO_THROW(::Xsmp::Helper::checkName("[432]", nullptr));
}

} // namespace Xsmp
