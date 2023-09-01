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

#include <gtest/gtest.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Array.h>

namespace Xsmp {

using EmptyArrayBool = Array<::Smp::Bool,0>;

using Array3Bool = Array<::Smp::Bool,3>;
using Matrix3_3Bool = Array<Array3Bool,3>;

TEST(ArrayTest, Constructor) {

    EmptyArrayBool emptyAr { };
    EXPECT_EQ(emptyAr.size(), 0);

    Array3Bool ar { };

    EXPECT_EQ(ar[0], false);
    EXPECT_EQ(ar[1], false);
    EXPECT_EQ(ar[2], false);

    Array3Bool ar2 { { true, false, true } };

    EXPECT_EQ(ar2[0], true);
    EXPECT_EQ(ar2[1], false);
    EXPECT_EQ(ar2[2], true);

    Array3Bool ar3(ar);

    EXPECT_EQ(ar3[0], false);
    EXPECT_EQ(ar3[1], false);
    EXPECT_EQ(ar3[2], false);

    Matrix3_3Bool mx { };

    EXPECT_EQ(mx[0][0], false);
    EXPECT_EQ(mx[0][1], false);
    EXPECT_EQ(mx[0][2], false);
    EXPECT_EQ(mx[1][0], false);
    EXPECT_EQ(mx[1][1], false);
    EXPECT_EQ(mx[1][2], false);
    EXPECT_EQ(mx[2][0], false);
    EXPECT_EQ(mx[2][1], false);
    EXPECT_EQ(mx[2][2], false);

    Matrix3_3Bool mx2 { { { { true, false, true } }, { { true, false, true } },
            { { true, false, true } } } };

    EXPECT_EQ(mx2[0][0], true);
    EXPECT_EQ(mx2[0][1], false);
    EXPECT_EQ(mx2[0][2], true);
    EXPECT_EQ(mx2[1][0], true);
    EXPECT_EQ(mx2[1][1], false);
    EXPECT_EQ(mx2[1][2], true);
    EXPECT_EQ(mx2[2][0], true);
    EXPECT_EQ(mx2[2][1], false);
    EXPECT_EQ(mx2[2][2], true);

    Matrix3_3Bool mx3 { mx };

    EXPECT_EQ(mx3[0][0], false);
    EXPECT_EQ(mx3[0][1], false);
    EXPECT_EQ(mx3[0][2], false);
    EXPECT_EQ(mx3[1][0], false);
    EXPECT_EQ(mx3[1][1], false);
    EXPECT_EQ(mx3[1][2], false);
    EXPECT_EQ(mx3[2][0], false);
    EXPECT_EQ(mx3[2][1], false);
    EXPECT_EQ(mx3[2][2], false);
}

TEST(ArrayTest, OperatorEqual) {
    Array3Bool ar = { };

    EXPECT_EQ(ar[0], false);
    EXPECT_EQ(ar[1], false);
    EXPECT_EQ(ar[2], false);

    Array3Bool ar2 = { { true, false, true } };

    EXPECT_EQ(ar2[0], true);
    EXPECT_EQ(ar2[1], false);
    EXPECT_EQ(ar2[2], true);

    Array3Bool ar3 = ar;

    EXPECT_EQ(ar3[0], false);
    EXPECT_EQ(ar3[1], false);
    EXPECT_EQ(ar3[2], false);

    Matrix3_3Bool mx = { };

    EXPECT_EQ(mx[0][0], false);
    EXPECT_EQ(mx[0][1], false);
    EXPECT_EQ(mx[0][2], false);
    EXPECT_EQ(mx[1][0], false);
    EXPECT_EQ(mx[1][1], false);
    EXPECT_EQ(mx[1][2], false);
    EXPECT_EQ(mx[2][0], false);
    EXPECT_EQ(mx[2][1], false);
    EXPECT_EQ(mx[2][2], false);

    Matrix3_3Bool mx3 = mx;

    EXPECT_EQ(mx3[0][0], false);
    EXPECT_EQ(mx3[0][1], false);
    EXPECT_EQ(mx3[0][2], false);
    EXPECT_EQ(mx3[1][0], false);
    EXPECT_EQ(mx3[1][1], false);
    EXPECT_EQ(mx3[1][2], false);
    EXPECT_EQ(mx3[2][0], false);
    EXPECT_EQ(mx3[2][1], false);
    EXPECT_EQ(mx3[2][2], false);
}

} // namespace Xsmp
