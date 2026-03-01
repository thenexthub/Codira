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

#include "ani_gtest_tuple_ops.h"

namespace ark::ets::ani::testing {

class TupleValueGetItemByteTest : public AniGTestTupleOps {
public:
    static constexpr ani_byte EXPECTED_RESULT = 125;
    static constexpr ani_byte EXPECTED_RESULT_2 = 126;
    static constexpr ani_byte EXPECTED_RESULT_3 = 127;
    static constexpr ani_byte EXPECTED_RESULT_4 = 1;
    static constexpr ani_byte EXPECTED_RESULT_5 = 2;
};

TEST_F(TupleValueGetItemByteTest, tupleValueGetItemByte)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_byte_test", "getByteTuple");
    ani_byte result = 0;
    ASSERT_EQ(env_->TupleValue_GetItem_Byte(tuple, 0U, &result), ANI_OK);
    ASSERT_EQ(result, EXPECTED_RESULT);
}

TEST_F(TupleValueGetItemByteTest, tupleValueGetItemByteNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_byte_test", "getByteTuple");
    ani_byte result = 0;
    ASSERT_EQ(env_->c_api->TupleValue_GetItem_Byte(nullptr, tuple, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemByteTest, tupleValueGetItemByteNullTuple)
{
    ani_byte result = 0;
    ASSERT_EQ(env_->TupleValue_GetItem_Byte(nullptr, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemByteTest, tupleValueGetItemByteIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_byte_test", "getByteTuple");
    ani_byte result = 0;
    ASSERT_EQ(env_->TupleValue_GetItem_Byte(tuple, 6U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemByteTest, tupleValueGetItemByteIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_byte_test", "getByteTuple");
    ani_byte result = 0;
    ASSERT_EQ(env_->TupleValue_GetItem_Byte(tuple, -1U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemByteTest, tupleValueGetItemByteIndexOutOfRange3)
{
    const ani_size maxNum = std::numeric_limits<ani_size>::max();
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_byte_test", "getByteTuple");
    ani_byte result = 0;
    ASSERT_EQ(env_->TupleValue_GetItem_Byte(tuple, maxNum, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemByteTest, tupleValueGetItemByteCompositeScene)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_byte_test", "getByteTuple");

    const std::array<ani_byte, 5U> expectedValues = {EXPECTED_RESULT, EXPECTED_RESULT_2, EXPECTED_RESULT_3,
                                                     EXPECTED_RESULT_4, EXPECTED_RESULT_5};

    ani_byte result = 0;
    for (size_t i = 0; i < expectedValues.size(); ++i) {
        ASSERT_EQ(env_->TupleValue_GetItem_Byte(tuple, i, &result), ANI_OK);
        ASSERT_EQ(result, expectedValues[i]);
    }
}

TEST_F(TupleValueGetItemByteTest, tupleValueGetItemByteNullResult)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_byte_test", "getByteTuple");
    ASSERT_EQ(env_->TupleValue_GetItem_Byte(tuple, 0U, nullptr), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemByteTest, tupleValueGetItemByteRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_byte_test", "getByteTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_byte result = 0;
        ASSERT_EQ(env_->TupleValue_GetItem_Byte(tuple, 0U, &result), ANI_OK);
        ASSERT_EQ(result, EXPECTED_RESULT);
    }
}
}  // namespace ark::ets::ani::testing