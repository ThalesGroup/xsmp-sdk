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
#include <Smp/Uuid.h>
#include <Smp/ViewKind.h>
#include <Xsmp/FallibleModel.h>
#include <Xsmp/Field.h>
#include <Xsmp/Model.h>
#include <Xsmp/Simulator.h>
#include <gtest/gtest.h>

namespace Xsmp {

class FallibleModeltest : public Model, public FallibleModel {
public:
  FallibleModeltest(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::IComposite *parent, ::Smp::ISimulator *simulator)
      : Model(name, description, parent, simulator),

        failure{simulator->GetTypeRegistry(),
                ::Smp::Uuids::Uuid_Bool,
                "failure",
                "",
                this,
                ::Smp::ViewKind::VK_None},

        failure2{simulator->GetTypeRegistry(),
                 ::Smp::Uuids::Uuid_Bool,
                 "failure2",
                 "",
                 this,
                 ::Smp::ViewKind::VK_None} {}

  Xsmp::Field<bool>::failure failure;
  Xsmp::Field<bool>::failure failure2;
};

TEST(FallibleModelTest, IsFailed) {

  Simulator sim;
  FallibleModeltest m{"m", "", &sim, &sim};

  EXPECT_EQ(&m.failure, m.GetFailure("failure"));
  EXPECT_EQ(&m.failure, m.GetFailures()->at("failure"));

  EXPECT_EQ(&m.failure2, m.GetFailure("failure2"));
  EXPECT_EQ(&m.failure2, m.GetFailures()->at("failure2"));

  EXPECT_FALSE(m.IsFailed());
  m.failure.Fail();
  EXPECT_TRUE(m.IsFailed());
  m.failure2.Fail();
  EXPECT_TRUE(m.IsFailed());

  m.failure.Unfail();
  EXPECT_TRUE(m.IsFailed());
  m.failure2.Unfail();

  EXPECT_FALSE(m.IsFailed());
}

} // namespace Xsmp
