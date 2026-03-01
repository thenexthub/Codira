/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include "ani_gtest_array_ops.h"
#include <iostream>

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class FixedArraySetGetRegionBooleanTest : public AniGTestArrayOps {};

TEST_F(FixedArraySetGetRegionBooleanTest, SetBooleanArrayRegionErrorTests)
{
    ani_fixedarray_boolean array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Boolean(LENGTH_5, &array), ANI_OK);
    ani_boolean nativeBuffer[LENGTH_10] = {ANI_FALSE};
    const ani_size offset1 = -1;
    ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(array, offset1, LENGTH_2, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(array, OFFSET_5, LENGTH_10, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer), ANI_OK);
}

TEST_F(FixedArraySetGetRegionBooleanTest, GetBooleanArrayRegionErrorTests)
{
    ani_fixedarray_boolean array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Boolean(LENGTH_5, &array), ANI_OK);
    ani_boolean nativeBuffer[LENGTH_10] = {ANI_TRUE};
    ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(array, OFFSET_0, LENGTH_1, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(array, OFFSET_5, LENGTH_10, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(array, OFFSET_0, LENGTH_1, nativeBuffer), ANI_OK);
}

TEST_F(FixedArraySetGetRegionBooleanTest, GetRegionBooleanTest)
{
    const auto array =
        static_cast<ani_fixedarray_boolean>(CallEtsFunction<ani_ref>("fixedarray_region_boolean_test", "getArray"));
    ani_boolean nativeBuffer[LENGTH_5] = {ANI_FALSE};
    ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], ANI_TRUE);
    ASSERT_EQ(nativeBuffer[1U], ANI_TRUE);
    ASSERT_EQ(nativeBuffer[2U], ANI_TRUE);
    ASSERT_EQ(nativeBuffer[3U], ANI_TRUE);
    ASSERT_EQ(nativeBuffer[4U], ANI_TRUE);
}

TEST_F(FixedArraySetGetRegionBooleanTest, SetRegionBooleanTest)
{
    const auto array =
        static_cast<ani_fixedarray_boolean>(CallEtsFunction<ani_ref>("fixedarray_region_boolean_test", "getArray"));
    const ani_boolean nativeBuffer1[LENGTH_5] = {ANI_TRUE, ANI_FALSE, ANI_TRUE};
    ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(array, OFFSET_2, LENGTH_3, nativeBuffer1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("fixedarray_region_boolean_test", "checkArray", array), ANI_TRUE);
}

TEST_F(FixedArraySetGetRegionBooleanTest, CheckChangeFromManagedRegionBooleanTest)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("fixedarray_region_boolean_test.ArrayClass", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "array", &ref), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto array = reinterpret_cast<ani_fixedarray_boolean>(ref);
    ani_boolean nativeBuffer[LENGTH_5] = {ANI_FALSE};

    ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], ANI_TRUE);
    ASSERT_EQ(nativeBuffer[1U], ANI_TRUE);
    ASSERT_EQ(nativeBuffer[2U], ANI_TRUE);
    ASSERT_EQ(nativeBuffer[3U], ANI_TRUE);
    ASSERT_EQ(nativeBuffer[4U], ANI_TRUE);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "changeStaticArray", nullptr), ANI_OK);
    ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], ANI_TRUE);
    ASSERT_EQ(nativeBuffer[1U], ANI_TRUE);
    ASSERT_EQ(nativeBuffer[2U], ANI_FALSE);
    ASSERT_EQ(nativeBuffer[3U], ANI_TRUE);
    ASSERT_EQ(nativeBuffer[4U], ANI_FALSE);
}

TEST_F(FixedArraySetGetRegionBooleanTest, CheckChangeFromApiRegionBooleanTest)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("fixedarray_region_boolean_test.ArrayClass", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "array", &ref), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto array = reinterpret_cast<ani_fixedarray_boolean>(ref);
    ani_boolean nativeBuffer[LENGTH_3] = {ANI_FALSE, ANI_FALSE, ANI_FALSE};
    ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(array, OFFSET_2, LENGTH_3, nativeBuffer), ANI_OK);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "checkStaticArray", nullptr, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(FixedArraySetGetRegionBooleanTest, SetGetUnionToArrayTest)
{
    ani_fixedarray_boolean array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Boolean(LENGTH_5, &array), ANI_OK);

    std::array<ani_boolean, LENGTH_5> nativeBuffer = {ANI_TRUE, ANI_FALSE, ANI_TRUE, ANI_FALSE, ANI_TRUE};
    ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);

    std::array<ani_boolean, LENGTH_5> nativeBuffer2 = {ANI_FALSE};
    ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer, nativeBuffer2);

    for (ani_size i = 0; i < LENGTH_5; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer, nativeBuffer2);

    std::array<ani_boolean, LENGTH_5> nativeBuffer3 = {ANI_FALSE, ANI_TRUE, ANI_FALSE, ANI_TRUE, ANI_FALSE};
    ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer3.data()), ANI_OK);
    for (ani_size i = 0; i < LENGTH_5; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer3.data()), ANI_OK);
    ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer2, nativeBuffer3);

    ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    for (ani_size i = 0; i < LENGTH_5; i++) {
        ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
        CompareArray(nativeBuffer, nativeBuffer2);
    }
}

TEST_F(FixedArraySetGetRegionBooleanTest, SetGetStabilityToArrayTest)
{
    ani_fixedarray_boolean array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Boolean(LENGTH_5, &array), ANI_OK);

    std::array<ani_boolean, LENGTH_5> nativeBuffer = {ANI_TRUE, ANI_FALSE, ANI_TRUE, ANI_FALSE, ANI_TRUE};
    std::array<ani_boolean, LENGTH_5> nativeBuffer2 = {ANI_FALSE};
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer, nativeBuffer2);

    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
        CompareArray(nativeBuffer, nativeBuffer2);
    }

    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
        ASSERT_EQ(env_->FixedArray_GetRegion_Boolean(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
        CompareArray(nativeBuffer, nativeBuffer2);
    }
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
