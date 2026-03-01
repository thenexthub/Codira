/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include <random>
#include <gtest/gtest.h>

const uint64_t SEED = 0x1234;
#ifndef PANDA_NIGHTLY_TEST_ON
const uint64_t ITERATION = 20;
#else
const uint64_t ITERATION = 4000;
#endif
static inline auto random_gen = std::mt19937_64(SEED);

// Encoder header
#include "optimizer/code_generator/operands.h"

namespace panda::compiler {
TEST(Operands, TypeInfo)
{
    uint8_t u8;
    int8_t i8;
    uint16_t u16;
    int16_t i16;
    uint32_t u32;
    int32_t i32;
    uint64_t u64;
    int64_t i64;

    float f32;
    double f64;

    TypeInfo arr[] = {
        TypeInfo(u8),  // 0
        TypeInfo(u16),
        TypeInfo(u32),
        TypeInfo(u64),
        TypeInfo(i8),  // 4
        TypeInfo(i16),
        TypeInfo(i32),
        TypeInfo(i64),
        TypeInfo(INT8_TYPE),  // 8
        TypeInfo(INT16_TYPE),
        TypeInfo(INT32_TYPE),
        TypeInfo(INT64_TYPE),
        TypeInfo(f32),  // 12
        TypeInfo(f64),
        TypeInfo(FLOAT32_TYPE),  // 14
        TypeInfo(FLOAT64_TYPE),
        TypeInfo(),  // 16
        INVALID_TYPE,
    };

    for (uint8_t i = 0; i < sizeof(arr) / sizeof(TypeInfo); ++i) {
        if (i >= 16) {
            ASSERT_FALSE(arr[i].IsValid());
        } else {
            ASSERT_TRUE(arr[i].IsValid());
        }
    }

    for (int i = 0; i < 4; ++i) {
        ASSERT_EQ(arr[i], arr[4 + i]);
        ASSERT_EQ(arr[i], arr[8 + i]);
        ASSERT_EQ(arr[4 + i], arr[8 + i]);

        ASSERT_EQ(arr[i].GetSize(), arr[4 + i].GetSize());
        ASSERT_EQ(arr[i].GetSize(), arr[8 + i].GetSize());
        ASSERT_EQ(arr[4 + i].GetSize(), arr[8 + i].GetSize());

        ASSERT_TRUE(arr[i].IsScalar());
        ASSERT_TRUE(arr[4 + i].IsScalar());
        ASSERT_TRUE(arr[8 + i].IsScalar());

        ASSERT_FALSE(arr[i].IsFloat());
        ASSERT_FALSE(arr[4 + i].IsFloat());
        ASSERT_FALSE(arr[8 + i].IsFloat());

        ASSERT_NE(arr[i], arr[12]);
        ASSERT_NE(arr[i], arr[13]);
        ASSERT_NE(arr[4 + i], arr[12]);
        ASSERT_NE(arr[4 + i], arr[13]);
        ASSERT_NE(arr[8 + i], arr[12]);
        ASSERT_NE(arr[8 + i], arr[13]);

        ASSERT_NE(arr[i], arr[14]);
        ASSERT_NE(arr[i], arr[15]);
        ASSERT_NE(arr[4 + i], arr[14]);
        ASSERT_NE(arr[4 + i], arr[15]);
        ASSERT_NE(arr[8 + i], arr[14]);
        ASSERT_NE(arr[8 + i], arr[15]);
        ASSERT_NE(arr[i], arr[16]);
        ASSERT_NE(arr[i], arr[17]);
        ASSERT_NE(arr[4 + i], arr[16]);
        ASSERT_NE(arr[4 + i], arr[17]);
        ASSERT_NE(arr[8 + i], arr[16]);
        ASSERT_NE(arr[8 + i], arr[17]);
    }
    // Float
    ASSERT_EQ(arr[2].GetSize(), arr[12].GetSize());
    ASSERT_EQ(arr[2].GetSize(), arr[14].GetSize());

    ASSERT_TRUE(arr[12].IsValid());
    ASSERT_TRUE(arr[14].IsValid());
    ASSERT_TRUE(arr[12].IsFloat());
    ASSERT_TRUE(arr[14].IsFloat());
    // Double
    ASSERT_EQ(arr[3].GetSize(), arr[13].GetSize());
    ASSERT_EQ(arr[3].GetSize(), arr[15].GetSize());

    // Check sizes:
    ASSERT_EQ(BYTE_SIZE, HALF_SIZE / 2);
    ASSERT_EQ(HALF_SIZE, WORD_SIZE / 2);
    ASSERT_EQ(WORD_SIZE, DOUBLE_WORD_SIZE / 2);

    ASSERT_EQ(arr[0].GetSize(), BYTE_SIZE);
    ASSERT_EQ(arr[1].GetSize(), HALF_SIZE);
    ASSERT_EQ(arr[2].GetSize(), WORD_SIZE);
    ASSERT_EQ(arr[3].GetSize(), DOUBLE_WORD_SIZE);

    ASSERT_EQ(sizeof(TypeInfo), sizeof(uint8_t));

    ASSERT_EQ(TypeInfo(u8), INT8_TYPE);
    ASSERT_EQ(TypeInfo(u16), INT16_TYPE);
    ASSERT_EQ(TypeInfo(u32), INT32_TYPE);
    ASSERT_EQ(TypeInfo(u64), INT64_TYPE);

    ASSERT_EQ(TypeInfo(f32), FLOAT32_TYPE);
    ASSERT_EQ(TypeInfo(f64), FLOAT64_TYPE);
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

TEST(Operands, Imm)
{
    // Check all possible types:
    //  Imm holds same data (static cast for un-signed)
    // GetType
    // Getsize
    // Is scalar
    //  Is Valid
    //  Bounary checks
    //  Check IsZero
    // Inc/dec checks
    // INVALID_IMM check

    for (uint64_t i = 0; i < ITERATION; ++i) {
        uint8_t u8 = random_gen(), u8_z = 0, u8_min = std::numeric_limits<uint8_t>::min(),
                u8_max = std::numeric_limits<uint8_t>::max();
        uint16_t u16 = random_gen(), u16_z = 0, u16_min = std::numeric_limits<uint16_t>::min(),
                 u16_max = std::numeric_limits<uint16_t>::max();
        uint32_t u32 = random_gen(), u32_z = 0, u32_min = std::numeric_limits<uint32_t>::min(),
                 u32_max = std::numeric_limits<uint32_t>::max();
        uint64_t u64 = random_gen(), u64_z = 0, u64_min = std::numeric_limits<uint64_t>::min(),
                 u64_max = std::numeric_limits<uint64_t>::max();

        int8_t i8 = random_gen(), i8_z = 0, i8_min = std::numeric_limits<int8_t>::min(),
               i8_max = std::numeric_limits<int8_t>::max();
        int16_t i16 = random_gen(), i16_z = 0, i16_min = std::numeric_limits<int16_t>::min(),
                i16_max = std::numeric_limits<int16_t>::max();
        int32_t i32 = random_gen(), i32_z = 0, i32_min = std::numeric_limits<int32_t>::min(),
                i32_max = std::numeric_limits<int32_t>::max();
        int64_t i64 = random_gen(), i64_z = 0, i64_min = std::numeric_limits<int64_t>::min(),
                i64_max = std::numeric_limits<int64_t>::max();

        float f32 = random_gen(), f32_z = 0, f32_min = std::numeric_limits<float>::min(),
              f32_max = std::numeric_limits<float>::max();
        double f64 = random_gen(), f64_z = 0, f64_min = std::numeric_limits<double>::min(),
               f64_max = std::numeric_limits<double>::max();

        // Unsigned part - check across static_cast

        Imm imm_u8(u8), imm_u8_z(u8_z), imm_u8_min(u8_min), imm_u8_max(u8_max);
        ASSERT_EQ(imm_u8.GetValue<int8_t>(), static_cast<int8_t>(u8));
        ASSERT_EQ(imm_u8_min.GetValue<int8_t>(), static_cast<int8_t>(u8_min));
        ASSERT_EQ(imm_u8_max.GetValue<int8_t>(), static_cast<int8_t>(u8_max));
        ASSERT_EQ(imm_u8_z.GetValue<int8_t>(), static_cast<int8_t>(u8_z));

        ASSERT_TRUE(imm_u8_min.IsZero());
        ASSERT_TRUE(imm_u8_z.IsZero());
        ASSERT_FALSE(imm_u8_max.IsZero());

        ASSERT_TRUE(imm_u8.IsValid());
        ASSERT_TRUE(imm_u8_z.IsValid());
        ASSERT_TRUE(imm_u8_min.IsValid());
        ASSERT_TRUE(imm_u8_max.IsValid());

        Imm imm_u16(u16), imm_u16_z(u16_z), imm_u16_min(u16_min), imm_u16_max(u16_max);
        ASSERT_EQ(imm_u16.GetValue<int16_t>(), static_cast<int16_t>(u16));
        ASSERT_EQ(imm_u16_min.GetValue<int16_t>(), static_cast<int16_t>(u16_min));
        ASSERT_EQ(imm_u16_max.GetValue<int16_t>(), static_cast<int16_t>(u16_max));
        ASSERT_EQ(imm_u16_z.GetValue<int16_t>(), static_cast<int16_t>(u16_z));

        ASSERT_TRUE(imm_u16_min.IsZero());
        ASSERT_TRUE(imm_u16_z.IsZero());
        ASSERT_FALSE(imm_u16_max.IsZero());

        ASSERT_TRUE(imm_u16.IsValid());
        ASSERT_TRUE(imm_u16_z.IsValid());
        ASSERT_TRUE(imm_u16_min.IsValid());
        ASSERT_TRUE(imm_u16_max.IsValid());

        Imm imm_u32(u32), imm_u32_z(u32_z), imm_u32_min(u32_min), imm_u32_max(u32_max);
        ASSERT_EQ(imm_u32.GetValue<int32_t>(), static_cast<int32_t>(u32));
        ASSERT_EQ(imm_u32_min.GetValue<int32_t>(), static_cast<int32_t>(u32_min));
        ASSERT_EQ(imm_u32_max.GetValue<int32_t>(), static_cast<int32_t>(u32_max));
        ASSERT_EQ(imm_u32_z.GetValue<int32_t>(), static_cast<int32_t>(u32_z));

        ASSERT_TRUE(imm_u32_min.IsZero());
        ASSERT_TRUE(imm_u32_z.IsZero());
        ASSERT_FALSE(imm_u32_max.IsZero());

        ASSERT_TRUE(imm_u32.IsValid());
        ASSERT_TRUE(imm_u32_z.IsValid());
        ASSERT_TRUE(imm_u32_min.IsValid());
        ASSERT_TRUE(imm_u32_max.IsValid());

        Imm imm_u64(u64), imm_u64_z(u64_z), imm_u64_min(u64_min), imm_u64_max(u64_max);
        ASSERT_EQ(imm_u64.GetValue<int64_t>(), static_cast<int64_t>(u64));
        ASSERT_EQ(imm_u64_min.GetValue<int64_t>(), static_cast<int64_t>(u64_min));
        ASSERT_EQ(imm_u64_max.GetValue<int64_t>(), static_cast<int64_t>(u64_max));
        ASSERT_EQ(imm_u64_z.GetValue<int64_t>(), static_cast<int64_t>(u64_z));

        ASSERT_TRUE(imm_u64_min.IsZero());
        ASSERT_TRUE(imm_u64_z.IsZero());
        ASSERT_FALSE(imm_u64_max.IsZero());

        ASSERT_TRUE(imm_u64.IsValid());
        ASSERT_TRUE(imm_u64_z.IsValid());
        ASSERT_TRUE(imm_u64_min.IsValid());
        ASSERT_TRUE(imm_u64_max.IsValid());

        // Signed part

        Imm imm_i8(i8), imm_i8_z(i8_z), imm_i8_min(i8_min), imm_i8_max(i8_max);
        ASSERT_EQ(imm_i8.GetValue<int8_t>(), i8);
        ASSERT_EQ(imm_i8_min.GetValue<int8_t>(), i8_min);
        ASSERT_EQ(imm_i8_max.GetValue<int8_t>(), i8_max);
        ASSERT_EQ(imm_i8_z.GetValue<int8_t>(), i8_z);

        ASSERT_FALSE(imm_i8_min.IsZero());
        ASSERT_TRUE(imm_i8_z.IsZero());
        ASSERT_FALSE(imm_i8_max.IsZero());

        ASSERT_TRUE(imm_i8.IsValid());
        ASSERT_TRUE(imm_i8_z.IsValid());
        ASSERT_TRUE(imm_i8_min.IsValid());
        ASSERT_TRUE(imm_i8_max.IsValid());

        Imm imm_i16(i16), imm_i16_z(i16_z), imm_i16_min(i16_min), imm_i16_max(i16_max);
        ASSERT_EQ(imm_i16.GetValue<int16_t>(), i16);
        ASSERT_EQ(imm_i16_min.GetValue<int16_t>(), i16_min);
        ASSERT_EQ(imm_i16_max.GetValue<int16_t>(), i16_max);
        ASSERT_EQ(imm_i16_z.GetValue<int16_t>(), i16_z);

        ASSERT_FALSE(imm_i16_min.IsZero());
        ASSERT_TRUE(imm_i16_z.IsZero());
        ASSERT_FALSE(imm_i16_max.IsZero());

        ASSERT_TRUE(imm_i16.IsValid());
        ASSERT_TRUE(imm_i16_z.IsValid());
        ASSERT_TRUE(imm_i16_min.IsValid());
        ASSERT_TRUE(imm_i16_max.IsValid());

        Imm imm_i32(i32), imm_i32_z(i32_z), imm_i32_min(i32_min), imm_i32_max(i32_max);
        ASSERT_EQ(imm_i32.GetValue<int32_t>(), i32);
        ASSERT_EQ(imm_i32_min.GetValue<int32_t>(), i32_min);
        ASSERT_EQ(imm_i32_max.GetValue<int32_t>(), i32_max);
        ASSERT_EQ(imm_i32_z.GetValue<int32_t>(), i32_z);

        ASSERT_FALSE(imm_i32_min.IsZero());
        ASSERT_TRUE(imm_i32_z.IsZero());
        ASSERT_FALSE(imm_i32_max.IsZero());

        ASSERT_TRUE(imm_i32.IsValid());
        ASSERT_TRUE(imm_i32_z.IsValid());
        ASSERT_TRUE(imm_i32_min.IsValid());
        ASSERT_TRUE(imm_i32_max.IsValid());

        Imm imm_i64(i64), imm_i64_z(i64_z), imm_i64_min(i64_min), imm_i64_max(i64_max);
        ASSERT_EQ(imm_i64.GetValue<int64_t>(), i64);
        ASSERT_EQ(imm_i64_min.GetValue<int64_t>(), i64_min);
        ASSERT_EQ(imm_i64_max.GetValue<int64_t>(), i64_max);
        ASSERT_EQ(imm_i64_z.GetValue<int64_t>(), i64_z);

        ASSERT_FALSE(imm_i64_min.IsZero());
        ASSERT_TRUE(imm_i64_z.IsZero());
        ASSERT_FALSE(imm_i64_max.IsZero());

        ASSERT_TRUE(imm_i64.IsValid());
        ASSERT_TRUE(imm_i64_z.IsValid());
        ASSERT_TRUE(imm_i64_min.IsValid());
        ASSERT_TRUE(imm_i64_max.IsValid());
    }
    // Sizeof imm:
    // Imm holds 2 uint64_t values (std::variant)
    ASSERT_LE(sizeof(Imm), sizeof(uint64_t) * 2);
}
}  // namespace panda::compiler
