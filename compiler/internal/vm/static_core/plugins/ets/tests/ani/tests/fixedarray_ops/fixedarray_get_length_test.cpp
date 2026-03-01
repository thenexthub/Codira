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

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class FixedArrayGetLengthTest : public AniGTestArrayOps {};

TEST_F(FixedArrayGetLengthTest, GetLengthErrorTests)
{
    ani_fixedarray_byte array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Byte(LENGTH_5, &array), ANI_OK);
    ani_size length = 0;
    ASSERT_EQ(env_->FixedArray_GetLength(nullptr, &length), ANI_INVALID_ARGS);
    ASSERT_EQ(env_->FixedArray_GetLength(array, nullptr), ANI_INVALID_ARGS);
}

TEST_F(FixedArrayGetLengthTest, GetLengthOkTests)
{
    ani_fixedarray_byte array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Byte(LENGTH_5, &array), ANI_OK);
    ani_size length = 0;
    ASSERT_EQ(env_->FixedArray_GetLength(array, &length), ANI_OK);
    ASSERT_EQ(length, LENGTH_5);
}

TEST_F(FixedArrayGetLengthTest, GetLengthTypesTests)
{
    ani_size arraySize = LENGTH_5;
    ani_size length = 0;
    ani_fixedarray_boolean array = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Boolean(arraySize, &array), ANI_OK);
    ASSERT_EQ(env_->FixedArray_GetLength(array, &length), ANI_OK);
    ASSERT_EQ(length, arraySize);
    arraySize = arraySize + 1U;
    ani_fixedarray_char array2 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Char(arraySize, &array2), ANI_OK);
    ASSERT_EQ(env_->FixedArray_GetLength(array2, &length), ANI_OK);
    ASSERT_EQ(length, arraySize);
    arraySize = arraySize + 1U;
    ani_fixedarray_byte array3 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Byte(arraySize, &array3), ANI_OK);
    ASSERT_EQ(env_->FixedArray_GetLength(array3, &length), ANI_OK);
    ASSERT_EQ(length, arraySize);
    arraySize = arraySize + 1U;
    ani_fixedarray_short array4 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Short(arraySize, &array4), ANI_OK);
    ASSERT_EQ(env_->FixedArray_GetLength(array4, &length), ANI_OK);
    ASSERT_EQ(length, arraySize);
    arraySize = arraySize + 1U;
    ani_fixedarray_int array5 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Int(arraySize, &array5), ANI_OK);
    ASSERT_EQ(env_->FixedArray_GetLength(array5, &length), ANI_OK);
    ASSERT_EQ(length, arraySize);
    arraySize = arraySize + 1U;
    ani_fixedarray_long array6 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Long(arraySize, &array6), ANI_OK);
    ASSERT_EQ(env_->FixedArray_GetLength(array6, &length), ANI_OK);
    ASSERT_EQ(length, arraySize);
    arraySize = arraySize + 1U;
    ani_fixedarray_float array7 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Float(arraySize, &array7), ANI_OK);
    ASSERT_EQ(env_->FixedArray_GetLength(array7, &length), ANI_OK);
    ASSERT_EQ(length, arraySize);
    arraySize = arraySize + 1U;
    ani_fixedarray_double array8 = nullptr;
    ASSERT_EQ(env_->FixedArray_New_Double(arraySize, &array8), ANI_OK);
    ASSERT_EQ(env_->FixedArray_GetLength(array8, &length), ANI_OK);
    ASSERT_EQ(length, arraySize);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays)
