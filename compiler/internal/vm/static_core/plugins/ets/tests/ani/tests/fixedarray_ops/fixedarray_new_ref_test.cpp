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

class FixedArrayNewRefTest : public AniGTestArrayOps {
public:
    static constexpr const ani_size ZERO = 0;

    static constexpr ani_size MINI_LENGTH = 10;
    static constexpr ani_size MID_LENGTH = 50;
    static constexpr ani_size BIG_LENGTH = 200;
    static constexpr ani_size ARRAYSIZE_10K = 10240U;
    static constexpr ani_size ARRAYSIZE_100K = 102400U;
};

TEST_F(FixedArrayNewRefTest, NewRefErrorTests)
{
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_fixedarray_ref array = nullptr;
    // Test null result pointer
    ASSERT_EQ(env_->FixedArray_New_Ref(cls, LENGTH_5, nullptr, nullptr), ANI_INVALID_ARGS);

    // Test null class
    ASSERT_EQ(env_->FixedArray_New_Ref(nullptr, LENGTH_5, nullptr, &array), ANI_INVALID_ARGS);

    if (sizeof(ani_size) > sizeof(uint32_t)) {
        ani_size maxLength = std::numeric_limits<uint32_t>::max() + ani_size(1);
        ASSERT_EQ(env_->FixedArray_New_Ref(cls, maxLength, nullptr, &array), ANI_INVALID_ARGS);
    }
}

TEST_F(FixedArrayNewRefTest, NewObjectArrayTest)
{
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    // Test zero length
    ani_fixedarray_ref zeroLengthArray = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Ref(cls, ZERO, nullptr, &zeroLengthArray), ANI_OK);
    ASSERT_NE(zeroLengthArray, nullptr);
    ani_size zeroLengthSize = 0;
    ASSERT_EQ(env_->FixedArray_GetLength(zeroLengthArray, &zeroLengthSize), ANI_OK);
    ASSERT_EQ(zeroLengthSize, ZERO);

    ani_fixedarray_ref array = nullptr;
    ani_size size = 0;

    // Test creating array with undefined initial element
    ani_ref undefinedRef = nullptr;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New_Ref(cls, LENGTH_5, undefinedRef, &array), ANI_OK);
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(env_->FixedArray_GetLength(array, &size), ANI_OK);
    ASSERT_EQ(size, LENGTH_5);

    // Test creating array with null initial element
    ani_ref nullRef = nullptr;
    ASSERT_EQ(env_->GetNull(&nullRef), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New_Ref(cls, LENGTH_5, nullRef, &array), ANI_OK);
    ASSERT_NE(array, nullptr);
    ASSERT_EQ(env_->FixedArray_GetLength(array, &size), ANI_OK);
    ASSERT_EQ(size, LENGTH_5);

    // Test creating array with initial element
    ani_string str = nullptr;
    const char *utf8String = "test";
    const ani_size stringLength = strlen(utf8String);
    ASSERT_EQ(env_->String_NewUTF8(utf8String, stringLength, &str), ANI_OK);
    ASSERT_NE(str, nullptr);
    ani_fixedarray_ref array2 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Ref(cls, stringLength, str, &array2), ANI_OK);
    ASSERT_NE(array2, nullptr);

    // Verify initial element was set for all elements
    for (ani_size i = 0; i < stringLength; i++) {
        ani_ref element = nullptr;
        ASSERT_EQ(env_->FixedArray_Get_Ref(array2, i, &element), ANI_OK);
        ani_size resultSize = 0;
        char utfBuffer[LENGTH_10] = {0};
        ASSERT_EQ(env_->String_GetUTF8SubString(reinterpret_cast<ani_string>(element), ZERO, stringLength, utfBuffer,
                                                sizeof(utfBuffer), &resultSize),
                  ANI_OK);
        ASSERT_STREQ(utfBuffer, utf8String);
    }
}

TEST_F(FixedArrayNewRefTest, NewObjectArrayTest2)
{
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_fixedarray_ref array1 = nullptr;
    ani_fixedarray_ref array2 = nullptr;
    ani_fixedarray_ref array3 = nullptr;
    ani_size size = 0;

    ani_ref undefinedRef = nullptr;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New_Ref(cls, MINI_LENGTH, undefinedRef, &array1), ANI_OK);
    ASSERT_NE(array1, nullptr);
    ASSERT_EQ(env_->FixedArray_GetLength(array1, &size), ANI_OK);
    ASSERT_EQ(size, MINI_LENGTH);

    ASSERT_EQ(env_->FixedArray_New_Ref(cls, MID_LENGTH, undefinedRef, &array2), ANI_OK);
    ASSERT_NE(array2, nullptr);
    ASSERT_EQ(env_->FixedArray_GetLength(array2, &size), ANI_OK);
    ASSERT_EQ(size, MID_LENGTH);

    ASSERT_EQ(env_->FixedArray_New_Ref(cls, BIG_LENGTH, undefinedRef, &array3), ANI_OK);
    ASSERT_NE(array3, nullptr);
    ASSERT_EQ(env_->FixedArray_GetLength(array3, &size), ANI_OK);
    ASSERT_EQ(size, BIG_LENGTH);
}

TEST_F(FixedArrayNewRefTest, NewObjectArrayTest3)
{
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_ref undefinedRef = nullptr;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    for (ani_int i = 0; i < LOOP_COUNT; i++) {
        ani_fixedarray_ref array = nullptr;
        ASSERT_EQ(env_->FixedArray_New_Ref(cls, LENGTH_5, undefinedRef, &array), ANI_OK);
        ASSERT_NE(array, nullptr);
    }
}

TEST_F(FixedArrayNewRefTest, NewObjectArrayTest4)
{
    ani_class cls = nullptr;
    ASSERT_EQ(env_->FindClass("std.core.String", &cls), ANI_OK);
    ASSERT_NE(cls, nullptr);

    ani_string str = nullptr;
    auto status = env_->String_NewUTF8("", 0U, &str);
    ASSERT_EQ(status, ANI_OK);
    ASSERT_NE(str, nullptr);

    const ani_size maxNum = std::numeric_limits<uint32_t>::max();
    ani_fixedarray_ref array1 = nullptr;
    ani_ref undefinedRef = nullptr;
    ASSERT_EQ(env_->GetUndefined(&undefinedRef), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New_Ref(cls, ZERO, undefinedRef, &array1), ANI_OK);
    ASSERT_NE(array1, nullptr);

    ani_fixedarray_ref array2 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Ref(cls, maxNum, undefinedRef, &array2), ANI_OUT_OF_MEMORY);

    ani_fixedarray_ref array3 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Ref(cls, ZERO, str, &array3), ANI_PENDING_ERROR);

    ani_fixedarray_ref array4 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Ref(cls, maxNum, str, &array4), ANI_PENDING_ERROR);
}

TEST_F(FixedArrayNewRefTest, NewLargeArrayTypesTest)
{
    ani_fixedarray_boolean array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Boolean(ARRAYSIZE_10K, &array), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New_Boolean(ARRAYSIZE_100K, &array), ANI_OK);
    ani_fixedarray_char array2 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Char(ARRAYSIZE_10K, &array2), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New_Char(ARRAYSIZE_100K, &array2), ANI_OK);
    ani_fixedarray_byte array3 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Byte(ARRAYSIZE_10K, &array3), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New_Byte(ARRAYSIZE_100K, &array3), ANI_OK);
    ani_fixedarray_short array4 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Short(ARRAYSIZE_10K, &array4), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New_Short(ARRAYSIZE_100K, &array4), ANI_OK);
    ani_fixedarray_int array5 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Int(ARRAYSIZE_10K, &array5), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New_Int(ARRAYSIZE_100K, &array5), ANI_OK);
    ani_fixedarray_long array6 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Long(ARRAYSIZE_10K, &array6), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New_Long(ARRAYSIZE_100K, &array6), ANI_OK);
    ani_fixedarray_float array7 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Float(ARRAYSIZE_10K, &array7), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New_Float(ARRAYSIZE_100K, &array7), ANI_OK);
    ani_fixedarray_double array8 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Double(ARRAYSIZE_10K, &array8), ANI_OK);
    ASSERT_EQ(env_->FixedArray_New_Double(ARRAYSIZE_100K, &array8), ANI_OK);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
