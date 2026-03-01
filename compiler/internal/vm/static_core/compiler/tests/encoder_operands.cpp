/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <random>
#include <gtest/gtest.h>

const uint64_t SEED = 0x1234;
#ifndef PANDA_NIGHTLY_TEST_ON
const uint64_t ITERATION = 20;
#else
const uint64_t ITERATION = 4000;
#endif
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects,cert-msc51-cpp)
static auto g_randomGen = std::mt19937_64(SEED);

// Encoder header
#include "optimizer/code_generator/operands.h"

// NOLINTBEGIN(readability-isolate-declaration)
namespace ark::compiler {

class TypeInfoTest : public ::testing::Test {
public:
    void CheckValid()
    {
        for (uint64_t i = 0; i < sizeof(arr_) / sizeof(TypeInfo); ++i) {
            if (i >= 16U) {  // NOLINT(readability-magic-numbers)
                ASSERT_FALSE(arr_[i].IsValid());
            } else {
                ASSERT_TRUE(arr_[i].IsValid());
            }
        }
    }

    void CheckSizes()
    {
        ASSERT_EQ(BYTE_SIZE, HALF_SIZE / 2U);
        ASSERT_EQ(HALF_SIZE, WORD_SIZE / 2U);
        ASSERT_EQ(WORD_SIZE, DOUBLE_WORD_SIZE / 2U);

        ASSERT_EQ(arr_[0U].GetSize(), BYTE_SIZE);
        ASSERT_EQ(arr_[1U].GetSize(), HALF_SIZE);
        ASSERT_EQ(arr_[2U].GetSize(), WORD_SIZE);
        ASSERT_EQ(arr_[3U].GetSize(), DOUBLE_WORD_SIZE);

        ASSERT_EQ(sizeof(TypeInfo), sizeof(uint8_t));

        ASSERT_EQ(TypeInfo(u8_), INT8_TYPE);
        ASSERT_EQ(TypeInfo(u16_), INT16_TYPE);
        ASSERT_EQ(TypeInfo(u32_), INT32_TYPE);
        ASSERT_EQ(TypeInfo(u64_), INT64_TYPE);

        ASSERT_EQ(TypeInfo(f32_), FLOAT32_TYPE);
        ASSERT_EQ(TypeInfo(f64_), FLOAT64_TYPE);
        // Float
        ASSERT_EQ(arr_[2U].GetSize(), arr_[12U].GetSize());
        ASSERT_EQ(arr_[2U].GetSize(), arr_[14U].GetSize());
        // Double
        ASSERT_EQ(arr_[3U].GetSize(), arr_[13U].GetSize());
        ASSERT_EQ(arr_[3U].GetSize(), arr_[15U].GetSize());
    }

    void CompareSizes()
    {
        for (size_t i = 0; i < 4U; ++i) {
            ASSERT_EQ(arr_[i], arr_[4U + i]);
            ASSERT_EQ(arr_[i], arr_[8U + i]);
            ASSERT_EQ(arr_[4U + i], arr_[8U + i]);

            ASSERT_EQ(arr_[i].GetSize(), arr_[4U + i].GetSize());
            ASSERT_EQ(arr_[i].GetSize(), arr_[8U + i].GetSize());
            ASSERT_EQ(arr_[4U + i].GetSize(), arr_[8U + i].GetSize());

            ASSERT_TRUE(arr_[i].IsScalar());
            ASSERT_TRUE(arr_[4U + i].IsScalar());
            ASSERT_TRUE(arr_[8U + i].IsScalar());

            ASSERT_FALSE(arr_[i].IsFloat());
            ASSERT_FALSE(arr_[4U + i].IsFloat());
            ASSERT_FALSE(arr_[8U + i].IsFloat());

            ASSERT_NE(arr_[i], arr_[12U]);
            ASSERT_NE(arr_[i], arr_[13U]);
            ASSERT_NE(arr_[4U + i], arr_[12U]);
            ASSERT_NE(arr_[4U + i], arr_[13U]);
            ASSERT_NE(arr_[8U + i], arr_[12U]);
            ASSERT_NE(arr_[8U + i], arr_[13U]);

            ASSERT_NE(arr_[i], arr_[14U]);
            ASSERT_NE(arr_[i], arr_[15U]);
            ASSERT_NE(arr_[4U + i], arr_[14U]);
            ASSERT_NE(arr_[4U + i], arr_[15U]);
            ASSERT_NE(arr_[8U + i], arr_[14U]);
            ASSERT_NE(arr_[8U + i], arr_[15U]);
            ASSERT_NE(arr_[i], arr_[16U]);
            ASSERT_NE(arr_[i], arr_[17U]);
            ASSERT_NE(arr_[4U + i], arr_[16U]);
            ASSERT_NE(arr_[4U + i], arr_[17U]);
            ASSERT_NE(arr_[8U + i], arr_[16U]);
            ASSERT_NE(arr_[8U + i], arr_[17U]);
        }
    }

    void CheckFloats()
    {
        // Float
        ASSERT_TRUE(arr_[12U].IsValid());
        ASSERT_TRUE(arr_[14U].IsValid());
        ASSERT_TRUE(arr_[12U].IsFloat());
        ASSERT_TRUE(arr_[14U].IsFloat());
    }

private:
    uint8_t u8_ = 0;
    int8_t i8_ = 0;
    uint16_t u16_ = 0;
    int16_t i16_ = 0;
    uint32_t u32_ = 0;
    int32_t i32_ = 0;
    uint64_t u64_ = 0;
    int64_t i64_ = 0;

    float f32_ = 0.0;
    double f64_ = 0.0;

    // NOLINTNEXTLINE(readability-magic-numbers)
    std::array<TypeInfo, 18U> arr_ {
        TypeInfo(u8_),  // 0
        TypeInfo(u16_),
        TypeInfo(u32_),
        TypeInfo(u64_),
        TypeInfo(i8_),  // 4
        TypeInfo(i16_),
        TypeInfo(i32_),
        TypeInfo(i64_),
        TypeInfo(INT8_TYPE),  // 8
        TypeInfo(INT16_TYPE),
        TypeInfo(INT32_TYPE),
        TypeInfo(INT64_TYPE),
        TypeInfo(f32_),  // 12
        TypeInfo(f64_),
        TypeInfo(FLOAT32_TYPE),  // 14
        TypeInfo(FLOAT64_TYPE),
        TypeInfo(),  // 16
        INVALID_TYPE,
    };
};

TEST_F(TypeInfoTest, Valid)
{
    CheckValid();
    CheckFloats();
}

TEST_F(TypeInfoTest, Sizes)
{
    CheckSizes();
    CompareSizes();
}

TEST(Operands, Reg)
{
    //  Size of structure
    ASSERT_LE(sizeof(Reg), sizeof(size_t));

    ASSERT_EQ(INVALID_REGISTER.GetId(), INVALID_REG_ID);

    // Check, what it is possible to create all 32 registers
    // for each type

    // Check what special registers are possible to compare with others

    // Check equality between registers

    // Check invalid registers
}

TEST(Operands, ImmUnsignedSmall)
{
    // Check all possible types:
    //  Imm holds same data (static cast for un-signed)

    for (uint64_t i = 0; i < ITERATION; ++i) {
        uint8_t u8 = g_randomGen(), u8Z = 0U, u8Min = std::numeric_limits<uint8_t>::min(),
                u8Max = std::numeric_limits<uint8_t>::max();
        uint16_t u16 = g_randomGen(), u16Z = 0U, u16Min = std::numeric_limits<uint16_t>::min(),
                 u16Max = std::numeric_limits<uint16_t>::max();
        // Unsigned part - check across static_cast

        Imm immU8(u8), immU8Z(u8Z), immU8Min(u8Min), immU8Max(u8Max);
        ASSERT_EQ(immU8.GetAsInt(), u8);
        ASSERT_EQ(immU8Min.GetAsInt(), u8Min);
        ASSERT_EQ(immU8Max.GetAsInt(), u8Max);
        ASSERT_EQ(immU8Z.GetAsInt(), u8Z);

        TypedImm typedImmU8(u8), typedImmU8Z(u8Z);
        ASSERT_EQ(typedImmU8Z.GetType(), INT8_TYPE);
        ASSERT_EQ(typedImmU8.GetType(), INT8_TYPE);
        ASSERT_EQ(typedImmU8Z.GetImm().GetAsInt(), u8Z);
        ASSERT_EQ(typedImmU8.GetImm().GetAsInt(), u8);

        Imm immU16(u16), immU16Z(u16Z), immU16Min(u16Min), immU16Max(u16Max);
        ASSERT_EQ(immU16.GetAsInt(), u16);
        ASSERT_EQ(immU16Min.GetAsInt(), u16Min);
        ASSERT_EQ(immU16Max.GetAsInt(), u16Max);
        ASSERT_EQ(immU16Z.GetAsInt(), u16Z);

        TypedImm typedImmU16(u16), typedImmU16Z(u16Z);
        ASSERT_EQ(typedImmU16Z.GetType(), INT16_TYPE);
        ASSERT_EQ(typedImmU16.GetType(), INT16_TYPE);
        ASSERT_EQ(typedImmU16Z.GetImm().GetAsInt(), u16Z);
        ASSERT_EQ(typedImmU16.GetImm().GetAsInt(), u16);
    }

#ifndef NDEBUG
    // Imm holds std::variant:
    ASSERT_EQ(sizeof(Imm), sizeof(uint64_t) * 2U);
#else
    // Imm holds 64-bit storage only:
    ASSERT_EQ(sizeof(Imm), sizeof(uint64_t));
#endif  // NDEBUG
}

TEST(Operands, ImmUnsignedLarge)
{
    for (uint64_t i = 0; i < ITERATION; ++i) {
        uint32_t u32 = g_randomGen(), u32Z = 0U, u32Min = std::numeric_limits<uint32_t>::min(),
                 u32Max = std::numeric_limits<uint32_t>::max();
        uint64_t u64 = g_randomGen(), u64Z = 0U, u64Min = std::numeric_limits<uint64_t>::min(),
                 u64Max = std::numeric_limits<uint64_t>::max();

        Imm immU32(u32), immU32Z(u32Z), immU32Min(u32Min), immU32Max(u32Max);
        ASSERT_EQ(immU32.GetAsInt(), u32);
        ASSERT_EQ(immU32Min.GetAsInt(), u32Min);
        ASSERT_EQ(immU32Max.GetAsInt(), u32Max);
        ASSERT_EQ(immU32Z.GetAsInt(), u32Z);

        TypedImm typedImmU32(u32), typedImmU32Z(u32Z);
        ASSERT_EQ(typedImmU32Z.GetType(), INT32_TYPE);
        ASSERT_EQ(typedImmU32.GetType(), INT32_TYPE);
        ASSERT_EQ(typedImmU32Z.GetImm().GetAsInt(), u32Z);
        ASSERT_EQ(typedImmU32.GetImm().GetAsInt(), u32);

        Imm immU64(u64), immU64Z(u64Z), immU64Min(u64Min), immU64Max(u64Max);
        ASSERT_EQ(immU64.GetAsInt(), u64);
        ASSERT_EQ(immU64Min.GetAsInt(), u64Min);
        ASSERT_EQ(immU64Max.GetAsInt(), u64Max);
        ASSERT_EQ(immU64Z.GetAsInt(), u64Z);

        TypedImm typedImmU64(u64), typedImmU64Z(u64Z);
        ASSERT_EQ(typedImmU64Z.GetType(), INT64_TYPE);
        ASSERT_EQ(typedImmU64.GetType(), INT64_TYPE);
        ASSERT_EQ(typedImmU64Z.GetImm().GetAsInt(), u64Z);
        ASSERT_EQ(typedImmU64.GetImm().GetAsInt(), u64);
    }
}

TEST(Operands, ImmSignedSmall)
{
    for (uint64_t i = 0; i < ITERATION; ++i) {
        // Signed part

        int8_t i8 = g_randomGen(), i8Z = 0U, i8Min = std::numeric_limits<int8_t>::min(),
               i8Max = std::numeric_limits<int8_t>::max();
        int16_t i16 = g_randomGen(), i16Z = 0U, i16Min = std::numeric_limits<int16_t>::min(),
                i16Max = std::numeric_limits<int16_t>::max();

        Imm immI8(i8), immI8Z(i8Z), immI8Min(i8Min), immI8Max(i8Max);
        ASSERT_EQ(immI8.GetAsInt(), i8);
        ASSERT_EQ(immI8Min.GetAsInt(), i8Min);
        ASSERT_EQ(immI8Max.GetAsInt(), i8Max);
        ASSERT_EQ(immI8Z.GetAsInt(), i8Z);

        TypedImm typedImmI8(i8), typedImmI8Z(i8Z);
        ASSERT_EQ(typedImmI8Z.GetType(), INT8_TYPE);
        ASSERT_EQ(typedImmI8.GetType(), INT8_TYPE);
        ASSERT_EQ(typedImmI8Z.GetImm().GetAsInt(), i8Z);
        ASSERT_EQ(typedImmI8.GetImm().GetAsInt(), i8);

        Imm immI16(i16), immI16Z(i16Z), immI16Min(i16Min), immI16Max(i16Max);
        ASSERT_EQ(immI16.GetAsInt(), i16);
        ASSERT_EQ(immI16Min.GetAsInt(), i16Min);
        ASSERT_EQ(immI16Max.GetAsInt(), i16Max);
        ASSERT_EQ(immI16Z.GetAsInt(), i16Z);

        TypedImm typedImmI16(i16), typedImmI16Z(i16Z);
        ASSERT_EQ(typedImmI16Z.GetType(), INT16_TYPE);
        ASSERT_EQ(typedImmI16.GetType(), INT16_TYPE);
        ASSERT_EQ(typedImmI16Z.GetImm().GetAsInt(), i16Z);
        ASSERT_EQ(typedImmI16.GetImm().GetAsInt(), i16);
    }
}

TEST(Operands, ImmSignedLarge)
{
    for (uint64_t i = 0; i < ITERATION; ++i) {
        int32_t i32 = g_randomGen(), i32Z = 0U, i32Min = std::numeric_limits<int32_t>::min(),
                i32Max = std::numeric_limits<int32_t>::max();
        int64_t i64 = g_randomGen(), i64Z = 0U, i64Min = std::numeric_limits<int64_t>::min(),
                i64Max = std::numeric_limits<int64_t>::max();

        Imm immI32(i32), immI32Z(i32Z), immI32Min(i32Min), immI32Max(i32Max);
        ASSERT_EQ(immI32.GetAsInt(), i32);
        ASSERT_EQ(immI32Min.GetAsInt(), i32Min);
        ASSERT_EQ(immI32Max.GetAsInt(), i32Max);
        ASSERT_EQ(immI32Z.GetAsInt(), i32Z);

        TypedImm typedImmI32(i32), typedImmI32Z(i32Z);
        ASSERT_EQ(typedImmI32Z.GetType(), INT32_TYPE);
        ASSERT_EQ(typedImmI32.GetType(), INT32_TYPE);
        ASSERT_EQ(typedImmI32Z.GetImm().GetAsInt(), i32Z);
        ASSERT_EQ(typedImmI32.GetImm().GetAsInt(), i32);

        Imm immI64(i64), immI64Z(i64Z), immI64Min(i64Min), immI64Max(i64Max);
        ASSERT_EQ(immI64.GetAsInt(), i64);
        ASSERT_EQ(immI64Min.GetAsInt(), i64Min);
        ASSERT_EQ(immI64Max.GetAsInt(), i64Max);
        ASSERT_EQ(immI64Z.GetAsInt(), i64Z);

        TypedImm typedImmI64(i64), typedImmI64Z(i64Z);
        ASSERT_EQ(typedImmI64Z.GetType(), INT64_TYPE);
        ASSERT_EQ(typedImmI64.GetType(), INT64_TYPE);
        ASSERT_EQ(typedImmI64Z.GetImm().GetAsInt(), i64Z);
        ASSERT_EQ(typedImmI64.GetImm().GetAsInt(), i64);
    }
}

TEST(Operands, ImmFloat)
{
    for (uint64_t i = 0; i < ITERATION; ++i) {
        float f32 = g_randomGen(), f32Z = 0.0, f32Min = std::numeric_limits<float>::min(),
              f32Max = std::numeric_limits<float>::max();
        double f64 = g_randomGen(), f64Z = 0.0, f64Min = std::numeric_limits<double>::min(),
               f64Max = std::numeric_limits<double>::max();

        // Float test:
        Imm immF32(f32), immF32Z(f32Z), immF32Min(f32Min), immF32Max(f32Max);
        ASSERT_EQ(immF32.GetAsFloat(), f32);
        ASSERT_EQ(immF32Min.GetAsFloat(), f32Min);
        ASSERT_EQ(immF32Max.GetAsFloat(), f32Max);
        ASSERT_EQ(immF32Z.GetAsFloat(), f32Z);
        ASSERT_EQ(bit_cast<float>(static_cast<int32_t>(immF32.GetRawValue())), f32);
        ASSERT_EQ(bit_cast<float>(static_cast<int32_t>(immF32Min.GetRawValue())), f32Min);
        ASSERT_EQ(bit_cast<float>(static_cast<int32_t>(immF32Max.GetRawValue())), f32Max);
        ASSERT_EQ(bit_cast<float>(static_cast<int32_t>(immF32Z.GetRawValue())), f32Z);

        TypedImm typedImmF32(f32), typedImmF32Z(f32Z);
        ASSERT_EQ(typedImmF32Z.GetType(), FLOAT32_TYPE);
        ASSERT_EQ(typedImmF32.GetType(), FLOAT32_TYPE);
        ASSERT_EQ(typedImmF32Z.GetImm().GetAsFloat(), f32Z);
        ASSERT_EQ(typedImmF32.GetImm().GetAsFloat(), f32);

        Imm immF64(f64), immF64Z(f64Z), immF64Min(f64Min), immF64Max(f64Max);
        ASSERT_EQ(immF64.GetAsDouble(), f64);
        ASSERT_EQ(immF64Min.GetAsDouble(), f64Min);
        ASSERT_EQ(immF64Max.GetAsDouble(), f64Max);
        ASSERT_EQ(immF64Z.GetAsDouble(), f64Z);
        ASSERT_EQ(bit_cast<double>(immF64.GetRawValue()), f64);
        ASSERT_EQ(bit_cast<double>(immF64Min.GetRawValue()), f64Min);
        ASSERT_EQ(bit_cast<double>(immF64Max.GetRawValue()), f64Max);
        ASSERT_EQ(bit_cast<double>(immF64Z.GetRawValue()), f64Z);

        TypedImm typedImmF64(f64), typedImmF64Z(f64Z);
        ASSERT_EQ(typedImmF64Z.GetType(), FLOAT64_TYPE);
        ASSERT_EQ(typedImmF64.GetType(), FLOAT64_TYPE);
        ASSERT_EQ(typedImmF64Z.GetImm().GetAsDouble(), f64Z);
        ASSERT_EQ(typedImmF64.GetImm().GetAsDouble(), f64);
    }
}

TEST(Operands, MemRef)
{
    Reg r1(1U, INT64_TYPE), r2(2U, INT64_TYPE), rI(INVALID_REG_ID, INVALID_TYPE);
    ssize_t i1(0x0U), i2(0x2U);

    // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
    std::array<MemRef, 3U> arr {MemRef(r1), MemRef(r1, i1), MemRef(r1)};
    // 1. Check constructors
    //  for getters
    //  for validness
    //  for operator ==
    // 2. Create mem with invalid_reg / invalid imm
}
// NOLINTEND(readability-isolate-declaration)

}  // namespace ark::compiler
