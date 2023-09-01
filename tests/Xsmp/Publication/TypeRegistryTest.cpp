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
#include <Smp/Publication/InvalidPrimitiveType.h>
#include <Smp/Publication/TypeAlreadyRegistered.h>
#include <Smp/Uuid.h>
#include <Xsmp/Publication/TypeRegistry.h>
#include <type_traits>

namespace Xsmp::Publication {
TEST(TypeRegistryTest, PrimitiveTypes) {

    TypeRegistry registry { "TypeRegistry", "", nullptr };

    EXPECT_TRUE(registry.GetType(Smp::PrimitiveTypeKind::PTK_Bool));
    EXPECT_TRUE(registry.GetType(Smp::PrimitiveTypeKind::PTK_Char8));
    EXPECT_TRUE(registry.GetType(Smp::PrimitiveTypeKind::PTK_DateTime));
    EXPECT_TRUE(registry.GetType(Smp::PrimitiveTypeKind::PTK_Duration));
    EXPECT_TRUE(registry.GetType(Smp::PrimitiveTypeKind::PTK_Float32));
    EXPECT_TRUE(registry.GetType(Smp::PrimitiveTypeKind::PTK_Float64));
    EXPECT_TRUE(registry.GetType(Smp::PrimitiveTypeKind::PTK_Int16));
    EXPECT_TRUE(registry.GetType(Smp::PrimitiveTypeKind::PTK_Int32));
    EXPECT_TRUE(registry.GetType(Smp::PrimitiveTypeKind::PTK_Int64));
    EXPECT_TRUE(registry.GetType(Smp::PrimitiveTypeKind::PTK_Int8));
    EXPECT_TRUE(registry.GetType(Smp::PrimitiveTypeKind::PTK_String8));
    EXPECT_TRUE(registry.GetType(Smp::PrimitiveTypeKind::PTK_UInt16));
    EXPECT_TRUE(registry.GetType(Smp::PrimitiveTypeKind::PTK_UInt32));
    EXPECT_TRUE(registry.GetType(Smp::PrimitiveTypeKind::PTK_UInt64));
    EXPECT_TRUE(registry.GetType(Smp::PrimitiveTypeKind::PTK_UInt8));
    EXPECT_FALSE(registry.GetType(Smp::PrimitiveTypeKind::PTK_None));

    EXPECT_FALSE(registry.GetType(Smp::Uuid { }));

    EXPECT_THROW(
            registry.AddIntegerType("integer", "", Smp::Uuids::Uuid_Bool, 0, 1,
                    "", Smp::PrimitiveTypeKind::PTK_Int16),
            Smp::Publication::TypeAlreadyRegistered);

    EXPECT_THROW(
            registry.AddIntegerType("integer", "", Smp::Uuid { }, 0, 1, "",
                    Smp::PrimitiveTypeKind::PTK_Bool),
            Smp::Publication::InvalidPrimitiveType);

}

TEST(TypeRegistryTest, IntegerType) {

    TypeRegistry registry { "TypeRegistry", "", nullptr };

    EXPECT_TRUE(registry.AddIntegerType("integer", "", Smp::Uuid { 0, 0, 0, 0,
            16 }, 0, 1, "", Smp::PrimitiveTypeKind::PTK_Int16));
    EXPECT_TRUE(registry.AddIntegerType("integer", "", Smp::Uuid { 0, 0, 0, 0,
            32 }, 0, 1, "", Smp::PrimitiveTypeKind::PTK_Int32));
    EXPECT_TRUE(registry.AddIntegerType("integer", "", Smp::Uuid { 0, 0, 0, 0,
            64 }, 0, 1, "", Smp::PrimitiveTypeKind::PTK_Int64));
    EXPECT_TRUE(
            registry.AddIntegerType("integer", "", Smp::Uuid { 0, 0, 0, 0, 8 },
                    0, 1, "", Smp::PrimitiveTypeKind::PTK_Int8));

    EXPECT_TRUE(registry.AddIntegerType("integer", "", Smp::Uuid { 0, 0, 0, 1,
            16 }, 0, 1, "", Smp::PrimitiveTypeKind::PTK_UInt16));
    EXPECT_TRUE(registry.AddIntegerType("integer", "", Smp::Uuid { 0, 0, 0, 1,
            32 }, 0, 1, "", Smp::PrimitiveTypeKind::PTK_UInt32));
    EXPECT_TRUE(registry.AddIntegerType("integer", "", Smp::Uuid { 0, 0, 0, 1,
            64 }, 0, 1, "", Smp::PrimitiveTypeKind::PTK_UInt64));
    EXPECT_TRUE(
            registry.AddIntegerType("integer", "", Smp::Uuid { 0, 0, 0, 1, 8 },
                    0, 1, "", Smp::PrimitiveTypeKind::PTK_UInt8));

    EXPECT_THROW(
            registry.AddIntegerType("integer", "", Smp::Uuids::Uuid_Bool, 0, 1,
                    "", Smp::PrimitiveTypeKind::PTK_Int16),
            Smp::Publication::TypeAlreadyRegistered);

    EXPECT_THROW(
            registry.AddIntegerType("integer", "", Smp::Uuid { }, 0, 1, "",
                    Smp::PrimitiveTypeKind::PTK_Bool),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddIntegerType("integer", "", Smp::Uuid { }, 0, 1, "",
                    Smp::PrimitiveTypeKind::PTK_Char8),
            Smp::Publication::InvalidPrimitiveType);
    EXPECT_THROW(
            registry.AddIntegerType("integer", "", Smp::Uuid { }, 0, 1, "",
                    Smp::PrimitiveTypeKind::PTK_String8),
            Smp::Publication::InvalidPrimitiveType);
    EXPECT_THROW(
            registry.AddIntegerType("integer", "", Smp::Uuid { }, 0, 1, "",
                    Smp::PrimitiveTypeKind::PTK_Duration),
            Smp::Publication::InvalidPrimitiveType);
    EXPECT_THROW(
            registry.AddIntegerType("integer", "", Smp::Uuid { }, 0, 1, "",
                    Smp::PrimitiveTypeKind::PTK_DateTime),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddIntegerType("integer", "", Smp::Uuid { }, 0, 1, "",
                    Smp::PrimitiveTypeKind::PTK_Float32),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddIntegerType("integer", "", Smp::Uuid { }, 0, 1, "",
                    Smp::PrimitiveTypeKind::PTK_Float64),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddIntegerType("integer", "", Smp::Uuid { }, 0, 1, "",
                    Smp::PrimitiveTypeKind::PTK_None),
            Smp::Publication::InvalidPrimitiveType);
}

TEST(TypeRegistryTest, FloatType) {

    TypeRegistry registry { "TypeRegistry", "", nullptr };

    EXPECT_TRUE(registry.AddFloatType("floatType", "", Smp::Uuid { 0, 0, 0, 0,
            32 }, 0, 1, true, true, "", Smp::PrimitiveTypeKind::PTK_Float32));

    EXPECT_TRUE(registry.AddFloatType("floatType", "", Smp::Uuid { 0, 0, 0, 0,
            64 }, 0, 1, true, true, "", Smp::PrimitiveTypeKind::PTK_Float64));

    EXPECT_THROW(
            registry.AddFloatType("integer", "", Smp::Uuids::Uuid_Bool, 0, 1,
                    true, true, "", Smp::PrimitiveTypeKind::PTK_Float32),
            Smp::Publication::TypeAlreadyRegistered);

    EXPECT_THROW(
            registry.AddFloatType("floatType", "", Smp::Uuid { }, 0, 1, true,
                    true, "", Smp::PrimitiveTypeKind::PTK_Bool),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddFloatType("floatType", "", Smp::Uuid { }, 0, 1, true,
                    true, "", Smp::PrimitiveTypeKind::PTK_Char8),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddFloatType("floatType", "", Smp::Uuid { }, 0, 1, true,
                    true, "", Smp::PrimitiveTypeKind::PTK_String8),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddFloatType("floatType", "", Smp::Uuid { }, 0, 1, true,
                    true, "", Smp::PrimitiveTypeKind::PTK_Duration),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddFloatType("floatType", "", Smp::Uuid { }, 0, 1, true,
                    true, "", Smp::PrimitiveTypeKind::PTK_DateTime),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddFloatType("floatType", "", Smp::Uuid { }, 0, 1, true,
                    true, "", Smp::PrimitiveTypeKind::PTK_None),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddFloatType("floatType", "", Smp::Uuid { }, 0, 1, true,
                    true, "", Smp::PrimitiveTypeKind::PTK_Int8),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddFloatType("floatType", "", Smp::Uuid { }, 0, 1, true,
                    true, "", Smp::PrimitiveTypeKind::PTK_Int16),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddFloatType("floatType", "", Smp::Uuid { }, 0, 1, true,
                    true, "", Smp::PrimitiveTypeKind::PTK_Int32),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddFloatType("floatType", "", Smp::Uuid { }, 0, 1, true,
                    true, "", Smp::PrimitiveTypeKind::PTK_Int64),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddFloatType("floatType", "", Smp::Uuid { }, 0, 1, true,
                    true, "", Smp::PrimitiveTypeKind::PTK_UInt8),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddFloatType("floatType", "", Smp::Uuid { }, 0, 1, true,
                    true, "", Smp::PrimitiveTypeKind::PTK_UInt16),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddFloatType("floatType", "", Smp::Uuid { }, 0, 1, true,
                    true, "", Smp::PrimitiveTypeKind::PTK_UInt32),
            Smp::Publication::InvalidPrimitiveType);

    EXPECT_THROW(
            registry.AddFloatType("floatType", "", Smp::Uuid { }, 0, 1, true,
                    true, "", Smp::PrimitiveTypeKind::PTK_UInt64),
            Smp::Publication::InvalidPrimitiveType);

}

TEST(TypeRegistryTest, StructureType) {

    TypeRegistry registry { "TypeRegistry", "", nullptr };

    EXPECT_TRUE(registry.AddStructureType("structure", "", Smp::Uuid { 0, 0, 0,
            0, 32 }));
}
TEST(TypeRegistryTest, ClassType) {

    TypeRegistry registry { "TypeRegistry", "", nullptr };

    EXPECT_TRUE(
            registry.AddClassType("cls", "", Smp::Uuid { 0, 0, 0, 0, 32 },
                    Smp::Uuid { }));
}

} // namespace Xsmp::Publication
