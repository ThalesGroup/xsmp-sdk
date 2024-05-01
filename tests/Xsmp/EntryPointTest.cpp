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

#include <Xsmp/Component.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/EntryPointPublisher.h>
#include <gtest/gtest.h>

namespace Xsmp {

namespace {
class TestEntryPointPublisher : public Xsmp::Component,
                                public Xsmp::EntryPointPublisher {
public:
  using Xsmp::Component::Component;
};
} // namespace

TEST(EntryPointTest, auto_register) {

  TestEntryPointPublisher c{"collection", "", nullptr};

  int i = 0;
  auto cb = [&i]() { i++; };
  EntryPoint ep{"ep", "desc", &c, cb};

  EXPECT_EQ(1, c.GetEntryPoints()->size());
  EXPECT_EQ(&ep, c.GetEntryPoint("ep"));
  EXPECT_EQ(&ep, c.GetEntryPoints()->at("ep"));

  ep.Execute();
  EXPECT_EQ(1, i);
  ep();
  EXPECT_EQ(2, i);
}

} // namespace Xsmp
