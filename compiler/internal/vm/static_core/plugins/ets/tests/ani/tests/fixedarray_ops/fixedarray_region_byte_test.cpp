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

class FixiedArraySetGetRegionByteTest : public AniGTestArrayOps {
protected:
    static constexpr ani_byte TEST_VALUE_1 = 1U;
    static constexpr ani_byte TEST_VALUE_2 = 2U;
    static constexpr ani_byte TEST_VALUE_3 = 3U;
    static constexpr ani_byte TEST_VALUE_4 = 4U;
    static constexpr ani_byte TEST_VALUE_5 = 5U;

    static constexpr ani_byte TEST_UPDATE_1 = 30U;
    static constexpr ani_byte TEST_UPDATE_2 = 40U;
    static constexpr ani_byte TEST_UPDATE_3 = 50U;
    static constexpr ani_byte TEST_UPDATE_4 = 22U;
    static constexpr ani_byte TEST_UPDATE_5 = 44U;
    static constexpr ani_byte TEST_UPDATE_6 = 33U;
};

TEST_F(FixiedArraySetGetRegionByteTest, SetByteArrayRegionErrorTests)
{
    ani_fixedarray_byte array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Byte(LENGTH_5, &array), ANI_OK);
    ani_byte nativeBuffer[LENGTH_10] = {0};
    const ani_size offset1 = -1;
    ASSERT_EQ(env_->FixedArray_SetRegion_Byte(array, offset1, LENGTH_2, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->FixedArray_SetRegion_Byte(array, OFFSET_5, LENGTH_10, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->FixedArray_SetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer), ANI_OK);
}

TEST_F(FixiedArraySetGetRegionByteTest, GetByteArrayRegionErrorTests)
{
    ani_fixedarray_byte array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Byte(LENGTH_5, &array), ANI_OK);
    ani_byte nativeBuffer[LENGTH_10] = {0};
    ASSERT_EQ(env_->FixedArray_GetRegion_Byte(array, OFFSET_0, LENGTH_1, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->FixedArray_GetRegion_Byte(array, OFFSET_5, LENGTH_10, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->FixedArray_GetRegion_Byte(array, OFFSET_0, LENGTH_1, nativeBuffer), ANI_OK);
}

TEST_F(FixiedArraySetGetRegionByteTest, GetRegionByteTest)
{
    const auto array =
        static_cast<ani_fixedarray_byte>(CallEtsFunction<ani_ref>("fixedarray_region_byte_test", "getArray"));

    ani_byte nativeBuffer[LENGTH_5] = {0};
    const ani_size offset3 = 0;
    ASSERT_EQ(env_->FixedArray_GetRegion_Byte(array, offset3, LENGTH_5, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], TEST_VALUE_1);
    ASSERT_EQ(nativeBuffer[1U], TEST_VALUE_2);
    ASSERT_EQ(nativeBuffer[2U], TEST_VALUE_3);
    ASSERT_EQ(nativeBuffer[3U], TEST_VALUE_4);
    ASSERT_EQ(nativeBuffer[4U], TEST_VALUE_5);
}

TEST_F(FixiedArraySetGetRegionByteTest, SetRegionByteTest)
{
    const auto array =
        static_cast<ani_fixedarray_byte>(CallEtsFunction<ani_ref>("fixedarray_region_byte_test", "getArray"));
    ani_byte nativeBuffer1[LENGTH_5] = {TEST_UPDATE_1, TEST_UPDATE_2, TEST_UPDATE_3};
    ASSERT_EQ(env_->FixedArray_SetRegion_Byte(array, OFFSET_2, LENGTH_3, nativeBuffer1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("fixedarray_region_byte_test", "checkArray", array), ANI_TRUE);
}

TEST_F(FixiedArraySetGetRegionByteTest, CheckChangeFromManagedRegionByteTest)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("fixedarray_region_byte_test.ArrayClass", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "array", &ref), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto array = reinterpret_cast<ani_fixedarray_byte>(ref);
    ani_byte nativeBuffer[LENGTH_5] = {0};
    const ani_size offset5 = 0;
    const ani_size len5 = LENGTH_5;

    ASSERT_EQ(env_->FixedArray_GetRegion_Byte(array, offset5, len5, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], TEST_VALUE_1);
    ASSERT_EQ(nativeBuffer[1U], TEST_VALUE_2);
    ASSERT_EQ(nativeBuffer[2U], TEST_VALUE_3);
    ASSERT_EQ(nativeBuffer[3U], TEST_VALUE_4);
    ASSERT_EQ(nativeBuffer[4U], TEST_VALUE_5);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "changeStaticArray", nullptr), ANI_OK);
    ASSERT_EQ(env_->FixedArray_GetRegion_Byte(array, offset5, len5, nativeBuffer), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], TEST_VALUE_1);
    ASSERT_EQ(nativeBuffer[1U], TEST_VALUE_2);
    ASSERT_EQ(nativeBuffer[2U], TEST_UPDATE_4);
    ASSERT_EQ(nativeBuffer[3U], TEST_VALUE_4);
    ASSERT_EQ(nativeBuffer[4U], TEST_UPDATE_5);
}

TEST_F(FixiedArraySetGetRegionByteTest, CheckChangeFromApiRegionByteTest)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("fixedarray_region_byte_test.ArrayClass", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "array", &ref), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto array = reinterpret_cast<ani_fixedarray_byte>(ref);
    ani_byte nativeBuffer[LENGTH_3] = {TEST_UPDATE_4, TEST_UPDATE_6, TEST_UPDATE_5};
    ASSERT_EQ(env_->FixedArray_SetRegion_Byte(array, OFFSET_2, LENGTH_3, nativeBuffer), ANI_OK);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "checkStaticArray", nullptr, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(FixiedArraySetGetRegionByteTest, GetSpecialValueToArrayTest)
{
    const auto array =
        static_cast<ani_fixedarray_byte>(CallEtsFunction<ani_ref>("fixedarray_region_byte_test", "getSpecialArray"));
    std::array<ani_byte, LENGTH_5> nativeBuffer = {};

    ASSERT_EQ(env_->FixedArray_GetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    const ani_byte minByteValue = -128;
    const ani_byte maxByteValue = 127;
    ASSERT_EQ(nativeBuffer[0U], minByteValue);
    ASSERT_EQ(nativeBuffer[1U], maxByteValue);
    ASSERT_EQ(nativeBuffer[2U], static_cast<ani_byte>(0));
}

TEST_F(FixiedArraySetGetRegionByteTest, SetSpecialValueToArrayTest)
{
    ani_fixedarray_byte array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Byte(LENGTH_5, &array), ANI_OK);

    const ani_byte minByteValue = -128;
    const ani_byte maxByteValue = 127;
    std::array<ani_byte, LENGTH_5> nativeBuffer = {minByteValue, maxByteValue, static_cast<ani_byte>(0),
                                                   static_cast<ani_byte>(-1), static_cast<ani_byte>(1)};
    ASSERT_EQ(env_->FixedArray_SetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);

    std::array<ani_byte, LENGTH_5> nativeBuffer2 = {};
    ASSERT_EQ(env_->FixedArray_GetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    ASSERT_EQ(nativeBuffer2[0U], minByteValue);
    ASSERT_EQ(nativeBuffer2[1U], maxByteValue);
    ASSERT_EQ(nativeBuffer2[2U], static_cast<ani_byte>(0));
    ASSERT_EQ(nativeBuffer2[3U], static_cast<ani_byte>(-1));
    ASSERT_EQ(nativeBuffer2[4U], static_cast<ani_byte>(1));
}

TEST_F(FixiedArraySetGetRegionByteTest, SetGetUnionToArrayTest)
{
    ani_fixedarray_byte array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Byte(LENGTH_5, &array), ANI_OK);

    std::array<ani_byte, LENGTH_5> nativeBuffer = {TEST_VALUE_1, TEST_VALUE_2, TEST_VALUE_3, TEST_VALUE_4,
                                                   TEST_VALUE_5};
    ASSERT_EQ(env_->FixedArray_SetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);

    std::array<ani_byte, LENGTH_5> nativeBuffer2 = {};
    ASSERT_EQ(env_->FixedArray_GetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer, nativeBuffer2);

    for (ani_size i = 0; i < LENGTH_5; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    ASSERT_EQ(env_->FixedArray_GetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer, nativeBuffer2);

    std::array<ani_byte, LENGTH_5> nativeBuffer3 = {TEST_VALUE_1, TEST_VALUE_3, TEST_VALUE_5, TEST_VALUE_2,
                                                    TEST_VALUE_4};
    ASSERT_EQ(env_->FixedArray_SetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer3.data()), ANI_OK);
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    ASSERT_EQ(env_->FixedArray_SetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer3.data()), ANI_OK);
    ASSERT_EQ(env_->FixedArray_GetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer2, nativeBuffer3);

    ASSERT_EQ(env_->FixedArray_SetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    for (ani_size i = 0; i < LENGTH_5; i++) {
        ASSERT_EQ(env_->FixedArray_GetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
        CompareArray(nativeBuffer, nativeBuffer2);
    }
}

TEST_F(FixiedArraySetGetRegionByteTest, SetGetStabilityToArrayTest)
{
    ani_fixedarray_byte array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Byte(LENGTH_5, &array), ANI_OK);

    std::array<ani_byte, LENGTH_5> nativeBuffer = {TEST_VALUE_1, TEST_VALUE_2, TEST_VALUE_3, TEST_VALUE_4,
                                                   TEST_VALUE_5};
    std::array<ani_byte, LENGTH_5> nativeBuffer2 = {};
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    ASSERT_EQ(env_->FixedArray_GetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer, nativeBuffer2);

    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_GetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
        CompareArray(nativeBuffer, nativeBuffer2);
    }
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
        ASSERT_EQ(env_->FixedArray_GetRegion_Byte(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
        CompareArray(nativeBuffer, nativeBuffer2);
    }
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
