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

class TupleValueGetItemIntTest : public AniGTestTupleOps {
public:
    static constexpr ani_int EXPECTED_RESULT = 300U;
    static constexpr ani_int EXPECTED_RESULT_2 = 350U;
    static constexpr ani_int EXPECTED_RESULT_3 = 200U;
    static constexpr ani_int EXPECTED_RESULT_4 = 100U;
    static constexpr ani_int EXPECTED_RESULT_5 = 50U;
};

TEST_F(TupleValueGetItemIntTest, tupleValueGetItemInt)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_int_test", "getIntTuple");
    ani_int result = 0U;
    ASSERT_EQ(env_->TupleValue_GetItem_Int(tuple, 0U, &result), ANI_OK);
    ASSERT_EQ(result, EXPECTED_RESULT);
}

TEST_F(TupleValueGetItemIntTest, tupleValueGetItemIntNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_int_test", "getIntTuple");
    ani_int result = 0U;
    ASSERT_EQ(env_->c_api->TupleValue_GetItem_Int(nullptr, tuple, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemIntTest, tupleValueGetItemIntNullTuple)
{
    ani_int result = 0U;
    ASSERT_EQ(env_->TupleValue_GetItem_Int(nullptr, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemIntTest, tupleValueGetItemIntIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_int_test", "getIntTuple");
    ani_int result = 0U;
    ASSERT_EQ(env_->TupleValue_GetItem_Int(tuple, 6U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemIntTest, tupleValueGetItemIntIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_int_test", "getIntTuple");
    ani_int result = 0U;
    ASSERT_EQ(env_->TupleValue_GetItem_Int(tuple, -1U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemIntTest, tupleValueGetItemIntIndexOutOfRange3)
{
    const ani_size maxNum = std::numeric_limits<ani_size>::max();
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_int_test", "getIntTuple");
    ani_int result = 0U;
    ASSERT_EQ(env_->TupleValue_GetItem_Int(tuple, maxNum, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemIntTest, tupleValueGetItemIntCompositeScene)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_int_test", "getIntTuple");

    const std::array<ani_int, 5U> expectedValues = {EXPECTED_RESULT, EXPECTED_RESULT_2, EXPECTED_RESULT_3,
                                                    EXPECTED_RESULT_4, EXPECTED_RESULT_5};

    ani_int result = 0U;
    for (size_t i = 0; i < expectedValues.size(); ++i) {
        ASSERT_EQ(env_->TupleValue_GetItem_Int(tuple, i, &result), ANI_OK);
        ASSERT_EQ(result, expectedValues[i]);
    }
}

TEST_F(TupleValueGetItemIntTest, tupleValueGetItemIntNullResult)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_int_test", "getIntTuple");
    ASSERT_EQ(env_->TupleValue_GetItem_Int(tuple, 0U, nullptr), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemIntTest, tupleValueGetItemIntRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_int_test", "getIntTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_int result = 0U;
        ASSERT_EQ(env_->TupleValue_GetItem_Int(tuple, 0U, &result), ANI_OK);
        ASSERT_EQ(result, EXPECTED_RESULT);
    }
}
}  // namespace ark::ets::ani::testing