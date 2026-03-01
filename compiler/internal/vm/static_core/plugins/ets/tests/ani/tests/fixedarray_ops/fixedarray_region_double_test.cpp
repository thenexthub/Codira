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

class FixedArraySetGetRegionDoubleTest : public AniGTestArrayOps {
protected:
    static constexpr double TEST_VALUE_1 = 1.0;
    static constexpr double TEST_VALUE_2 = 2.0;
    static constexpr double TEST_VALUE_3 = 3.0;
    static constexpr double TEST_VALUE_4 = 4.0;
    static constexpr double TEST_VALUE_5 = 5.0;

    static constexpr double TEST_UPDATE_1 = 30.0;
    static constexpr double TEST_UPDATE_2 = 40.0;
    static constexpr double TEST_UPDATE_3 = 50.0;

    static constexpr double TEST_UPDATE_4 = 22.0;
    static constexpr double TEST_UPDATE_5 = 44.0;
    static constexpr double TEST_UPDATE_6 = 33.0;
};

TEST_F(FixedArraySetGetRegionDoubleTest, SetDoubleArrayRegionErrorTests)
{
    ani_fixedarray_double array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Double(LENGTH_5, &array), ANI_OK);
    ani_double nativeBuffer[LENGTH_5] = {TEST_VALUE_1, TEST_VALUE_2, TEST_VALUE_3, TEST_VALUE_4, TEST_VALUE_5};
    const ani_size offset1 = -1;
    ASSERT_EQ(env_->FixedArray_SetRegion_Double(array, offset1, LENGTH_2, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->FixedArray_SetRegion_Double(array, OFFSET_5, LENGTH_10, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->FixedArray_SetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer), ANI_OK);
}

TEST_F(FixedArraySetGetRegionDoubleTest, GetDoubleArrayRegionErrorTests)
{
    ani_fixedarray_double array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Double(LENGTH_5, &array), ANI_OK);
    ani_double nativeBuffer[LENGTH_10] = {TEST_VALUE_1, TEST_VALUE_2, TEST_VALUE_3, TEST_VALUE_4, TEST_VALUE_5};
    ASSERT_EQ(env_->FixedArray_GetRegion_Double(array, OFFSET_0, LENGTH_1, nullptr), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->FixedArray_GetRegion_Double(array, OFFSET_5, LENGTH_10, nativeBuffer), ANI_OUT_OF_RANGE);
    ASSERT_EQ(env_->FixedArray_GetRegion_Double(array, OFFSET_0, LENGTH_1, nativeBuffer), ANI_OK);
}

TEST_F(FixedArraySetGetRegionDoubleTest, GetRegionDoubleTest)
{
    const auto array =
        static_cast<ani_fixedarray_double>(CallEtsFunction<ani_ref>("fixedarray_region_double_test", "getArray"));

    ani_double nativeBuffer[LENGTH_5] = {0.0};
    const double epsilon = 1e-6;  // Define acceptable tolerance

    ASSERT_EQ(env_->FixedArray_GetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer), ANI_OK);
    ASSERT_NEAR(nativeBuffer[0U], TEST_VALUE_1, epsilon);
    ASSERT_NEAR(nativeBuffer[1U], TEST_VALUE_2, epsilon);
    ASSERT_NEAR(nativeBuffer[2U], TEST_VALUE_3, epsilon);
    ASSERT_NEAR(nativeBuffer[3U], TEST_VALUE_4, epsilon);
    ASSERT_NEAR(nativeBuffer[4U], TEST_VALUE_5, epsilon);
}

TEST_F(FixedArraySetGetRegionDoubleTest, SetRegionDoubleTest)
{
    const auto array =
        static_cast<ani_fixedarray_double>(CallEtsFunction<ani_ref>("fixedarray_region_double_test", "getArray"));
    ani_double nativeBuffer1[LENGTH_5] = {TEST_UPDATE_1, TEST_UPDATE_2, TEST_UPDATE_3};
    ASSERT_EQ(env_->FixedArray_SetRegion_Double(array, OFFSET_2, LENGTH_3, nativeBuffer1), ANI_OK);
    ASSERT_EQ(CallEtsFunction<ani_boolean>("fixedarray_region_double_test", "checkArray", array), ANI_TRUE);
}

TEST_F(FixedArraySetGetRegionDoubleTest, CheckChangeFromManagedRegionDoubleTest)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("fixedarray_region_double_test.ArrayClass", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "array", &ref), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto array = reinterpret_cast<ani_fixedarray_double>(ref);
    ani_double nativeBuffer[LENGTH_5] = {0.0};
    const ani_size offset5 = 0;
    const ani_size len5 = 5;
    const double epsilon = 1e-6;  // Define acceptable tolerance

    ASSERT_EQ(env_->FixedArray_GetRegion_Double(array, offset5, len5, nativeBuffer), ANI_OK);
    ASSERT_NEAR(nativeBuffer[0U], TEST_VALUE_1, epsilon);
    ASSERT_NEAR(nativeBuffer[1U], TEST_VALUE_2, epsilon);
    ASSERT_NEAR(nativeBuffer[2U], TEST_VALUE_3, epsilon);
    ASSERT_NEAR(nativeBuffer[3U], TEST_VALUE_4, epsilon);
    ASSERT_NEAR(nativeBuffer[4U], TEST_VALUE_5, epsilon);

    ASSERT_EQ(env_->Class_CallStaticMethodByName_Void(cls, "changeStaticArray", nullptr), ANI_OK);
    ASSERT_EQ(env_->FixedArray_GetRegion_Double(array, offset5, len5, nativeBuffer), ANI_OK);
    ASSERT_NEAR(nativeBuffer[0U], TEST_VALUE_1, epsilon);
    ASSERT_NEAR(nativeBuffer[1U], TEST_VALUE_2, epsilon);
    ASSERT_NEAR(nativeBuffer[2U], TEST_UPDATE_4, epsilon);
    ASSERT_NEAR(nativeBuffer[3U], TEST_VALUE_4, epsilon);
    ASSERT_NEAR(nativeBuffer[4U], TEST_UPDATE_5, epsilon);
}

TEST_F(FixedArraySetGetRegionDoubleTest, CheckChangeFromApiRegionDoubleTest)
{
    ani_class cls {};
    ASSERT_EQ(env_->FindClass("fixedarray_region_double_test.ArrayClass", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_ref ref = nullptr;
    ASSERT_EQ(env_->Class_GetStaticFieldByName_Ref(cls, "array", &ref), ANI_OK);
    ASSERT_NE(ref, nullptr);

    auto array = reinterpret_cast<ani_fixedarray_double>(ref);
    ani_double nativeBuffer[LENGTH_3] = {TEST_UPDATE_4, TEST_UPDATE_6, TEST_UPDATE_5};
    ASSERT_EQ(env_->FixedArray_SetRegion_Double(array, OFFSET_2, LENGTH_3, nativeBuffer), ANI_OK);

    ani_boolean result = ANI_FALSE;
    ASSERT_EQ(env_->Class_CallStaticMethodByName_Boolean(cls, "checkStaticArray", nullptr, &result), ANI_OK);
    ASSERT_EQ(result, ANI_TRUE);
}

TEST_F(FixedArraySetGetRegionDoubleTest, GetSpecialValueToArrayTest)
{
    const auto array = static_cast<ani_fixedarray_double>(
        CallEtsFunction<ani_ref>("fixedarray_region_double_test", "getSpecialArray"));
    const ani_double max = 4.9406564584124654e-324;
    const ani_double min = 1.79769313486231570e+308;
    const ani_double minPositive = -1.79769313486231570e+308;
    std::array<ani_double, LENGTH_6> nativeBuffer = {};
    ASSERT_EQ(env_->FixedArray_GetRegion_Double(array, OFFSET_0, LENGTH_6, nativeBuffer.data()), ANI_OK);
    ASSERT_EQ(nativeBuffer[0U], max);
    ASSERT_EQ(nativeBuffer[1U], min);
    ASSERT_EQ(nativeBuffer[2U], minPositive);
    ASSERT_EQ(nativeBuffer[3U], 0.0);
}

TEST_F(FixedArraySetGetRegionDoubleTest, SetSpecialValueToArrayTest)
{
    ani_fixedarray_double array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Double(LENGTH_6, &array), ANI_OK);

    ani_double max = std::numeric_limits<double>::max();
    ani_double minPositive = std::numeric_limits<double>::min();
    ani_double min = -std::numeric_limits<double>::max();

    const std::array<ani_double, LENGTH_6> nativeBuffer = {minPositive, max, min, 0.0, -1.0, 1.0};
    ASSERT_EQ(env_->FixedArray_SetRegion_Double(array, OFFSET_0, LENGTH_6, nativeBuffer.data()), ANI_OK);

    std::array<ani_double, LENGTH_6> nativeBuffer2 = {};
    ASSERT_EQ(env_->FixedArray_GetRegion_Double(array, OFFSET_0, LENGTH_6, nativeBuffer2.data()), ANI_OK);
    ASSERT_EQ(nativeBuffer2[0U], minPositive);
    ASSERT_EQ(nativeBuffer2[1U], max);
    ASSERT_EQ(nativeBuffer2[2U], min);
    ASSERT_EQ(nativeBuffer2[3U], 0.0);
    ASSERT_EQ(nativeBuffer2[4U], -1.0);
}

TEST_F(FixedArraySetGetRegionDoubleTest, SetGetUnionToArrayTest)
{
    ani_fixedarray_double array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Double(LENGTH_5, &array), ANI_OK);

    std::array<ani_double, LENGTH_5> nativeBuffer = {TEST_VALUE_1, TEST_VALUE_2, TEST_VALUE_3, TEST_VALUE_4,
                                                     TEST_VALUE_5};
    ASSERT_EQ(env_->FixedArray_SetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    std::array<ani_double, LENGTH_5> nativeBuffer2 = {};
    ASSERT_EQ(env_->FixedArray_GetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer, nativeBuffer2);

    for (ani_size i = 0; i < LENGTH_5; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    ASSERT_EQ(env_->FixedArray_GetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer, nativeBuffer2);

    std::array<ani_double, LENGTH_5> nativeBuffer3 = {TEST_VALUE_1, TEST_VALUE_3, TEST_VALUE_5, TEST_VALUE_2,
                                                      TEST_VALUE_4};
    ASSERT_EQ(env_->FixedArray_SetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer3.data()), ANI_OK);
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    ASSERT_EQ(env_->FixedArray_SetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer3.data()), ANI_OK);
    ASSERT_EQ(env_->FixedArray_GetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer2, nativeBuffer3);

    ASSERT_EQ(env_->FixedArray_SetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    for (ani_size i = 0; i < LENGTH_5; i++) {
        ASSERT_EQ(env_->FixedArray_GetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
        CompareArray(nativeBuffer, nativeBuffer2);
    }
}

TEST_F(FixedArraySetGetRegionDoubleTest, SetGetStabilityToArrayTest)
{
    ani_fixedarray_double array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Double(LENGTH_5, &array), ANI_OK);

    std::array<ani_double, LENGTH_5> nativeBuffer = {TEST_VALUE_1, TEST_VALUE_2, TEST_VALUE_3, TEST_VALUE_4,
                                                     TEST_VALUE_5};
    std::array<ani_double, LENGTH_5> nativeBuffer2 = {};
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    ASSERT_EQ(env_->FixedArray_GetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
    CompareArray(nativeBuffer, nativeBuffer2);

    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
    }
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_GetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
        CompareArray(nativeBuffer, nativeBuffer2);
    }

    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ASSERT_EQ(env_->FixedArray_SetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer.data()), ANI_OK);
        ASSERT_EQ(env_->FixedArray_GetRegion_Double(array, OFFSET_0, LENGTH_5, nativeBuffer2.data()), ANI_OK);
        CompareArray(nativeBuffer, nativeBuffer2);
    }
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
