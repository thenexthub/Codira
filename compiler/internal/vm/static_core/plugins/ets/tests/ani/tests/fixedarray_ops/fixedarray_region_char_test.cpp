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

#include "ani_gtest_array_ops.h"
#include <iostream>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class FixedArraySetGetRegionCharTest : public AniGTestArrayOps {};

TEST_F(FixedArraySetGetRegionCharTest, SetCharArrayRegionErrorTests)
{
    ani_fixedarray_char array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Char(LENGTH_5, &array), ANI_OK);
    ani_char nativeBuffer[LENGTH_10] = {0};
    const ani_size offset1 = -1;
    ASSERT_EQ(env_->FixedArray_SetRegion_Char(array, offset1, LENGTH_2, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->FixedArray_SetRegion_Char(array, OFFSET_5, LENGTH_10, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->FixedArray_SetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer), ANI_OK);
}

TEST_F(FixedArraySetGetRegionCharTest, GetCharArrayRegionErrorTests)
{
    ani_fixedarray_char array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Char(LENGTH_5, &array), ANI_OK);
    ani_char nativeBuffer[LENGTH_5] = {0};
    ASSERT_EQ(env_->FixedArray_GetRegion_Char(array, OFFSET_0, LENGTH_1, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->FixedArray_GetRegion_Char(array, OFFSET_5, LENGTH_10, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->FixedArray_GetRegion_Char(array, OFFSET_0, LENGTH_1, nativeBuffer), ANI_OK);
}

TEST_F(FixedArraySetGetRegionCharTest, GetRegionCharTest)
{
    const auto array =
        static_cast<ani_fixedarray_char>(CallEtsFunction<ani_ref>("fixedarray_region_char_test", "getArray"));

    ani_char nativeBuffer[LENGTH_5] = {0};
    ASSERT_EQ(env_->FixedArray_GetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], 'a');
    ASSERT_EQ(nativeBuffer[1U], 'b');
    ASSERT_EQ(nativeBuffer[2U], 'c');
    ASSERT_EQ(nativeBuffer[3U], 'd');
    ASSERT_EQ(nativeBuffer[4U], 'e');
}

TEST_F(FixedArraySetGetRegionCharTest, SetRegionCharTest)
{
    const auto array =
        static_cast<ani_fixedarray_char>(CallEtsFunction<ani_ref>("fixedarray_region_char_test", "getArray"));
    ani_char nativeBuffer1[LENGTH_5] = {'x', 'y', 'z'};
    ASSERT_EQ(env_->FixedArray_SetRegion_Char(array, OFFSET_2, LENGTH_3, nativeBuffer1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("fixedarray_region_char_test", "checkArray", array), ANI_TRUE);
}

TEST_F(FixedArraySetGetRegionCharTest, SetRegionCharChangeTest)
{
    ani_fixedarray_char array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Char(LENGTH_5, &array), ANI_OK);

    const auto changedArray =
        static_cast<ani_fixedarray_char>(CallEtsFunction<ani_ref>("fixedarray_region_char_test", "changeArray", array));
    ani_char nativeBuffer[LENGTH_5] = {0};
    const ani_size offset = 0;
    const ani_size len = LENGTH_5;
    ASSERT_EQ(env_->FixedArray_GetRegion_Char(changedArray, offset, len, nativeBuffer), ANI_OK);

    ASSERT_EQ(nativeBuffer[0U], 'a');
    ASSERT_EQ(nativeBuffer[1U], 'b');
    ASSERT_EQ(nativeBuffer[2U], 'x');
    ASSERT_EQ(nativeBuffer[3U], 'y');
    ASSERT_EQ(nativeBuffer[4U], 'z');
}

TEST_F(FixedArraySetGetRegionCharTest, GetSpecialValueToArrayTest)
{
    const auto array =
        static_cast<ani_fixedarray_char>(CallEtsFunction<ani_ref>("fixedarray_region_char_test", "getSpecialArray"));
    std::array<ani_char, LENGTH_10> nativeBuffer = {};
    ASSERT_EQ(env_->FixedArray_GetRegion_Char(array, OFFSET_0, LENGTH_10, nativeBuffer.data()), ANI_OK);
    ASSERT_EQ(nativeBuffer[1U], '\0');
    ASSERT_EQ(nativeBuffer[2U], '\n');
    ASSERT_EQ(nativeBuffer[3U], '\r');
    ASSERT_EQ(nativeBuffer[4U], '\t');
    ASSERT_EQ(nativeBuffer[5U], '\b');
    ASSERT_EQ(nativeBuffer[7U], '\v');
    ASSERT_EQ(nativeBuffer[8U], '\f');
    ASSERT_EQ(nativeBuffer[9U], '\0');
}

TEST_F(FixedArraySetGetRegionCharTest, SetSpecialValueToArrayTest)
{
    ani_fixedarray_char array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Char(LENGTH_10, &array), ANI_OK);

    const std::array<ani_char, LENGTH_10> nativeBuffer = {'a', '0', '\n', '\r', '\t', '\b', '\a', '\v', '\f', '\0'};
    ASSERT_EQ(env_->FixedArray_SetRegion_Char(array, OFFSET_0, LENGTH_10, nativeBuffer.data()), ANI_OK);

    std::array<ani_char, LENGTH_10> nativeBuffer2 = {};
    ASSERT_EQ(env_->FixedArray_GetRegion_Char(array, OFFSET_0, LENGTH_10, nativeBuffer2.data()), ANI_OK);
    ASSERT_EQ(nativeBuffer2[0U], 'a');
    ASSERT_EQ(nativeBuffer2[1U], '0');
    ASSERT_EQ(nativeBuffer2[2U], '\n');
    ASSERT_EQ(nativeBuffer2[3U], '\r');
    ASSERT_EQ(nativeBuffer2[4U], '\t');
    ASSERT_EQ(nativeBuffer2[5U], '\b');
    ASSERT_EQ(nativeBuffer2[6U], '\a');
    ASSERT_EQ(nativeBuffer2[7U], '\v');
    ASSERT_EQ(nativeBuffer2[8U], '\f');
    ASSERT_EQ(nativeBuffer2[9U], '\0');
}

TEST_F(FixedArraySetGetRegionCharTest, SetGetUnionToArrayTest)
{
    ani_fixedarray_char array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Char(LENGTH_5, &array), ANI_OK);

    std::array<ani_char, LENGTH_5> nativeBuffer = {'a', 'b', 'c', 'd', 'e'};
    ASSERT_EQ(env_->FixedArray_SetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    std::array<ani_char, LENGTH_5> nativeBuffer2 = {};
    ASSERT_EQ(env_->FixedArray_GetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer, nativeBuffer2);

    for (ani_size i = 0; i < LENGTH_5; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    ASSERT_EQ(env_->FixedArray_GetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer, nativeBuffer2);

    std::array<ani_char, LENGTH_5> nativeBuffer3 = {'a', 'c', 'e', 'x', 'y'};
    ASSERT_EQ(env_->FixedArray_SetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer3.data()), ANI_OK);
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    ASSERT_EQ(env_->FixedArray_SetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer3.data()), ANI_OK);
    ASSERT_EQ(env_->FixedArray_GetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer2, nativeBuffer3);

    ASSERT_EQ(env_->FixedArray_SetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    for (ani_size i = 0; i < LENGTH_5; i++) {
        ASSERT_EQ(env_->FixedArray_GetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
        CompareArray(nativeBuffer, nativeBuffer2);
    }
}

TEST_F(FixedArraySetGetRegionCharTest, SetGetStabilityToArrayTest)
{
    ani_fixedarray_char array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Char(LENGTH_5, &array), ANI_OK);

    std::array<ani_char, LENGTH_5> nativeBuffer = {'a', 'b', 'c', 'd', 'e'};
    std::array<ani_char, LENGTH_5> nativeBuffer2 = {};
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    ASSERT_EQ(env_->FixedArray_GetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer, nativeBuffer2);

    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_GetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
        CompareArray(nativeBuffer, nativeBuffer2);
    }

    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
        ASSERT_EQ(env_->FixedArray_GetRegion_Char(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
        CompareArray(nativeBuffer, nativeBuffer2);
    }
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
