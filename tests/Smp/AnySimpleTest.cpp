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

#include <Smp/AnySimple.h>
#include <Smp/InvalidAnyType.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Helper.h>
#include <Smp/utility.h>
#include <limits>
#include <vector>

#include "gtest/gtest.h"

namespace Smp {

template<typename T>
class Integral: public testing::Test {
};
TYPED_TEST_SUITE_P(Integral);

template<typename TypeParam, PrimitiveTypeKind kind>
void check() {

    constexpr auto min = ::Xsmp::Helper::numeric_limits<kind>::lowest();
    constexpr auto expectedMin = std::numeric_limits<TypeParam>::lowest();

    EXPECT_NO_THROW( (AnySimple {kind, min}))<< kind;
    EXPECT_THROW((AnySimple {PrimitiveTypeKind::PTK_None, min}), ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple {PrimitiveTypeKind::PTK_String8, min}), ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple {PrimitiveTypeKind::PTK_Char8, min}), ::Smp::InvalidAnyType);

    // expectedMin < min
    if (::Smp::utility::cmp_less(expectedMin, min)) {
        EXPECT_THROW( (AnySimple {kind, expectedMin}),::Smp::InvalidAnyType)<< kind;
        EXPECT_NO_THROW((static_cast<TypeParam>(AnySimple {kind,min})))<< kind;
    }
    // expectedMin == min
    else if (::Smp::utility::cmp_equal( expectedMin, min)) {
        EXPECT_NO_THROW((AnySimple {kind,min}))<< kind;
        EXPECT_NO_THROW((static_cast<TypeParam>(AnySimple {kind,min})))<< kind;
    }
    // expectedMin > min
    else {
        EXPECT_NO_THROW((AnySimple {kind, min}))<< kind;
        EXPECT_NO_THROW((static_cast<TypeParam>(AnySimple {kind, expectedMin})))<< kind;
        EXPECT_THROW((static_cast<TypeParam>(AnySimple {kind,min})), ::Smp::InvalidAnyType)<< kind;
    }

    constexpr auto max = ::Xsmp::Helper::numeric_limits<kind>::max();
    constexpr auto expectedMax = std::numeric_limits<TypeParam>::max();

    EXPECT_NO_THROW(
            (AnySimple {kind,
                        max}))<< kind;

    EXPECT_THROW((static_cast<::Smp::Char8>(AnySimple { kind, 0 })),
            ::Smp::InvalidAnyType);

    EXPECT_THROW((static_cast<::Smp::String8>(AnySimple { kind, 0 })),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { kind, 0 }.MoveString()), ::Smp::InvalidAnyType);

    // max < expectedMax
    if (::Smp::utility::cmp_less(max, expectedMax)) {
        EXPECT_THROW((AnySimple {kind, expectedMax}),::Smp::InvalidAnyType)<< kind;
        EXPECT_NO_THROW((static_cast<TypeParam>(AnySimple {kind,max})))<< kind;
    }
    // expectedMax == max
    else if (::Smp::utility::cmp_equal( expectedMax, max)) {
        EXPECT_NO_THROW((AnySimple {kind,max}))<< kind;
        EXPECT_NO_THROW((static_cast<TypeParam>(AnySimple {kind,max})))<< kind;
    }
    // max > expectedMax
    else {
        EXPECT_NO_THROW((AnySimple {kind, max}))<< kind;
        EXPECT_NO_THROW((static_cast<TypeParam>(AnySimple {kind,expectedMax})))<< kind;
        EXPECT_THROW((static_cast<TypeParam>(AnySimple {kind,max})), ::Smp::InvalidAnyType)<< kind;
    }
}

TYPED_TEST_P(Integral, Check) {

    check<TypeParam, PrimitiveTypeKind::PTK_UInt8>();
    check<TypeParam, PrimitiveTypeKind::PTK_UInt16>();
    check<TypeParam, PrimitiveTypeKind::PTK_UInt32>();
    check<TypeParam, PrimitiveTypeKind::PTK_UInt64>();
    check<TypeParam, PrimitiveTypeKind::PTK_Int8>();
    check<TypeParam, PrimitiveTypeKind::PTK_Int16>();
    check<TypeParam, PrimitiveTypeKind::PTK_Int32>();
    check<TypeParam, PrimitiveTypeKind::PTK_Int64>();
    check<TypeParam, PrimitiveTypeKind::PTK_DateTime>();
    check<TypeParam, PrimitiveTypeKind::PTK_Duration>();
}

REGISTER_TYPED_TEST_SUITE_P(Integral, Check);

using primitive_types = testing::Types<::Smp::Int8, ::Smp::Int16,::Smp::Int32,::Smp::Int64, ::Smp::UInt8, ::Smp::UInt16,::Smp::UInt32,::Smp::UInt64>;

INSTANTIATE_TYPED_TEST_SUITE_P(AnySimpleTest, Integral, primitive_types);

TEST(AnySimpleTest, Bool) {

    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Bool, false }));

    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Bool, true }));
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Bool, true }.MoveString()),
            ::Smp::InvalidAnyType);

    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int8, true }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int16, true }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int32, true }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int64, true }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt8, true }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt16, true }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt32, true }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt64, true }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Char8, true }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_None, true }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32, true }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Float64, true }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_DateTime, true }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Duration, true }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_String8, true }),
            ::Smp::InvalidAnyType);

    EXPECT_TRUE((AnySimple { PrimitiveTypeKind::PTK_Bool, true }));
    EXPECT_TRUE((AnySimple { PrimitiveTypeKind::PTK_Int8, 1 }));
    EXPECT_TRUE((AnySimple { PrimitiveTypeKind::PTK_Int16, 1 }));
    EXPECT_TRUE((AnySimple { PrimitiveTypeKind::PTK_Int32, 1 }));
    EXPECT_TRUE((AnySimple { PrimitiveTypeKind::PTK_Int64, 1 }));
    EXPECT_TRUE((AnySimple { PrimitiveTypeKind::PTK_UInt8, 1 }));
    EXPECT_TRUE((AnySimple { PrimitiveTypeKind::PTK_UInt16, 1 }));
    EXPECT_TRUE((AnySimple { PrimitiveTypeKind::PTK_UInt32, 1 }));
    EXPECT_TRUE((AnySimple { PrimitiveTypeKind::PTK_UInt64, 1 }));
    EXPECT_TRUE((AnySimple { PrimitiveTypeKind::PTK_Duration, 1 }));
    EXPECT_TRUE((AnySimple { PrimitiveTypeKind::PTK_DateTime, 1 }));
    EXPECT_TRUE((AnySimple { PrimitiveTypeKind::PTK_Char8, 'a' }));
    EXPECT_TRUE((AnySimple { PrimitiveTypeKind::PTK_String8, "test" }));
    EXPECT_THROW(static_cast<bool>(AnySimple { }), ::Smp::InvalidAnyType);
    EXPECT_THROW(
            static_cast<bool>(AnySimple { PrimitiveTypeKind::PTK_Float32, 1 }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW(
            static_cast<bool>(AnySimple { PrimitiveTypeKind::PTK_Float64, 1 }),
            ::Smp::InvalidAnyType);

}

TEST(AnySimpleTest, Char8) {

    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Char8, 'a' }));
    EXPECT_EQ(static_cast<::Smp::Char8>(AnySimple {
            PrimitiveTypeKind::PTK_Char8, 'a' }), 'a');

    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Char8, '\n' }));
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Char8, 'a' }.MoveString()),
            ::Smp::InvalidAnyType);

    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int8, 'a' }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int16, 'a' }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int32, 'a' }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int64, 'a' }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt8, 'a' }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt16, 'a' }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt32, 'a' }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt64, 'a' }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Bool, 'a' }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_None, 'a' }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32, 'a' }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Float64, 'a' }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_DateTime, 'a' }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Duration, 'a' }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_String8, 'a' }),
            ::Smp::InvalidAnyType);

    EXPECT_THROW(static_cast<::Smp::Char8>(AnySimple { }),
            ::Smp::InvalidAnyType);

    EXPECT_THROW(static_cast<::Smp::Char8>(AnySimple {
            PrimitiveTypeKind::PTK_Bool, false }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::Char8>(AnySimple {
            PrimitiveTypeKind::PTK_Int8, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::Char8>(AnySimple {
            PrimitiveTypeKind::PTK_Int16, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::Char8>(AnySimple {
            PrimitiveTypeKind::PTK_Int32, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::Char8>(AnySimple {
            PrimitiveTypeKind::PTK_Int64, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::Char8>(AnySimple {
            PrimitiveTypeKind::PTK_UInt8, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::Char8>(AnySimple {
            PrimitiveTypeKind::PTK_UInt16, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::Char8>(AnySimple {
            PrimitiveTypeKind::PTK_UInt32, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::Char8>(AnySimple {
            PrimitiveTypeKind::PTK_UInt64, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::Char8>(AnySimple {
            PrimitiveTypeKind::PTK_Float32, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::Char8>(AnySimple {
            PrimitiveTypeKind::PTK_Float64, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::Char8>(AnySimple {
            PrimitiveTypeKind::PTK_Duration, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::Char8>(AnySimple {
            PrimitiveTypeKind::PTK_DateTime, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::Char8>(AnySimple {
            PrimitiveTypeKind::PTK_String8, "test" }), ::Smp::InvalidAnyType);
}

TEST(AnySimpleTest, String8) {

    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_String8, "test" }));
    EXPECT_NO_THROW(
            (delete[] AnySimple { PrimitiveTypeKind::PTK_String8, "test" }.MoveString()));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_String8, nullptr }));

    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int8, "test" }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int16, "test" }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int32, "test" }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int64, "test" }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt8, "test" }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt16, "test" }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt32, "test" }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt64, "test" }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Bool, "test" }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_None, "test" }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32, "test" }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Float64, "test" }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_DateTime, "test" }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Duration, "test" }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Char8, "test" }),
            ::Smp::InvalidAnyType);

    EXPECT_THROW(static_cast<::Smp::String8>(AnySimple { }),
            ::Smp::InvalidAnyType);

    EXPECT_THROW(static_cast<::Smp::String8>(AnySimple {
            PrimitiveTypeKind::PTK_Bool, false }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::String8>(AnySimple {
            PrimitiveTypeKind::PTK_Int8, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::String8>(AnySimple {
            PrimitiveTypeKind::PTK_Int16, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::String8>(AnySimple {
            PrimitiveTypeKind::PTK_Int32, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::String8>(AnySimple {
            PrimitiveTypeKind::PTK_Int64, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::String8>(AnySimple {
            PrimitiveTypeKind::PTK_UInt8, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::String8>(AnySimple {
            PrimitiveTypeKind::PTK_UInt16, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::String8>(AnySimple {
            PrimitiveTypeKind::PTK_UInt32, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::String8>(AnySimple {
            PrimitiveTypeKind::PTK_UInt64, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::String8>(AnySimple {
            PrimitiveTypeKind::PTK_Float32, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::String8>(AnySimple {
            PrimitiveTypeKind::PTK_Float64, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::String8>(AnySimple {
            PrimitiveTypeKind::PTK_Duration, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::String8>(AnySimple {
            PrimitiveTypeKind::PTK_DateTime, 0 }), ::Smp::InvalidAnyType);
    EXPECT_THROW(static_cast<::Smp::String8>(AnySimple {
            PrimitiveTypeKind::PTK_Char8, 'a' }), ::Smp::InvalidAnyType);
}

TEST(AnySimpleTest, Float32) {

    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32, 3.14159f }));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float64, 3.14159f }));

    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32,
            static_cast<::Smp::Int8>(0) }));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32,
            static_cast<::Smp::Int16>(0) }));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32,
            static_cast<::Smp::Int32>(0) }));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32,
            static_cast<::Smp::Int64>(0) }));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32,
            static_cast<::Smp::UInt8>(0) }));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32,
            static_cast<::Smp::UInt16>(0) }));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32,
            static_cast<::Smp::UInt32>(0) }));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32,
            static_cast<::Smp::UInt64>(0) }));
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32,
            std::numeric_limits<::Smp::Int32>::max() }), ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32,
            std::numeric_limits<::Smp::Int64>::max() }), ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32,
            std::numeric_limits<::Smp::UInt32>::max() }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32,
            std::numeric_limits<::Smp::UInt64>::max() }),
            ::Smp::InvalidAnyType);

    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int8, 3.14159f }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int16, 3.14159f }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int32, 3.14159f }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int64, 3.14159f }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt8, 3.14159f }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt16, 3.14159f }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt32, 3.14159f }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt64, 3.14159f }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Bool, 3.14159f }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_None, 3.14159f }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_DateTime, 3.14159f }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Duration, 3.14159f }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Char8, 3.14159f }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_String8, 3.14159f }),
            ::Smp::InvalidAnyType);

    EXPECT_THROW(static_cast<::Smp::Float32>(AnySimple { }),
            ::Smp::InvalidAnyType);

    EXPECT_NO_THROW(static_cast<::Smp::Float32>(AnySimple {
            PrimitiveTypeKind::PTK_Int8, 0 }));
    EXPECT_NO_THROW(static_cast<::Smp::Float32>(AnySimple {
            PrimitiveTypeKind::PTK_Int16, 0 }));
    EXPECT_NO_THROW(static_cast<::Smp::Float32>(AnySimple {
            PrimitiveTypeKind::PTK_UInt8, 0 }));
    EXPECT_NO_THROW(static_cast<::Smp::Float32>(AnySimple {
            PrimitiveTypeKind::PTK_UInt16, 0 }));
}

TEST(AnySimpleTest, Float64) {
    EXPECT_NO_THROW(
            (AnySimple { PrimitiveTypeKind::PTK_Float64, 3.14159265359 }));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32, 2.0 }));
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Float32, 3.14159265359 }),
            ::Smp::InvalidAnyType);

    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float64,
            static_cast<::Smp::Int8>(0) }));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float64,
            static_cast<::Smp::Int16>(0) }));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float64,
            static_cast<::Smp::Int32>(0) }));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float64,
            static_cast<::Smp::Int64>(0) }));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float64,
            static_cast<::Smp::UInt8>(0) }));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float64,
            static_cast<::Smp::UInt16>(0) }));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float64,
            static_cast<::Smp::UInt32>(0) }));
    EXPECT_NO_THROW((AnySimple { PrimitiveTypeKind::PTK_Float64,
            static_cast<::Smp::UInt64>(0) }));

    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Float64,
            std::numeric_limits<::Smp::Int64>::max() }), ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Float64,
            std::numeric_limits<::Smp::UInt64>::max() }),
            ::Smp::InvalidAnyType);

    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int8, 3.14159265359 }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int16, 3.14159265359 }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int32, 3.14159265359 }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Int64, 3.14159265359 }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt8, 3.14159265359 }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt16, 3.14159265359 }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt32, 3.14159265359 }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_UInt64, 3.14159265359 }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Bool, 3.14159265359 }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_None, 3.14159265359 }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_DateTime, 3.14159265359 }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Duration, 3.14159265359 }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_Char8, 3.14159265359 }),
            ::Smp::InvalidAnyType);
    EXPECT_THROW((AnySimple { PrimitiveTypeKind::PTK_String8, 3.14159265359 }),
            ::Smp::InvalidAnyType);

    EXPECT_NO_THROW(static_cast<::Smp::Float64>(AnySimple {
            PrimitiveTypeKind::PTK_Int8, 0 }));
    EXPECT_NO_THROW(static_cast<::Smp::Float64>(AnySimple {
            PrimitiveTypeKind::PTK_Int16, 0 }));
    EXPECT_NO_THROW(static_cast<::Smp::Float64>(AnySimple {
            PrimitiveTypeKind::PTK_Int32, 0 }));
    EXPECT_NO_THROW(static_cast<::Smp::Float64>(AnySimple {
            PrimitiveTypeKind::PTK_UInt8, 0 }));
    EXPECT_NO_THROW(static_cast<::Smp::Float64>(AnySimple {
            PrimitiveTypeKind::PTK_UInt16, 0 }));
    EXPECT_NO_THROW(static_cast<::Smp::Float64>(AnySimple {
            PrimitiveTypeKind::PTK_UInt32, 0 }));
    EXPECT_NO_THROW(static_cast<::Smp::Float64>(AnySimple {
            PrimitiveTypeKind::PTK_Float32, 0.f }));
}

TEST(AnySimpleTest, AssignmentOperator) {
    // with String8
    AnySimple first { PrimitiveTypeKind::PTK_String8, "test" };
    auto second = first;
    EXPECT_EQ(first.GetType(), second.GetType());
    EXPECT_STREQ(static_cast<Smp::String8>(first),
            static_cast<Smp::String8>(second));

    //with nullptr
    first = { PrimitiveTypeKind::PTK_String8, nullptr };
    second = first;
    EXPECT_EQ(first.GetType(), second.GetType());
    EXPECT_STREQ(static_cast<Smp::String8>(first),
            static_cast<Smp::String8>(second));

    // with Float64
    first = { PrimitiveTypeKind::PTK_Float64, 999999999999. };
    second = first;
    EXPECT_EQ(first.GetType(), second.GetType());
    EXPECT_EQ(static_cast<Smp::Float64>(first),
            static_cast<Smp::Float64>(second));

}
} // namespace Smp
