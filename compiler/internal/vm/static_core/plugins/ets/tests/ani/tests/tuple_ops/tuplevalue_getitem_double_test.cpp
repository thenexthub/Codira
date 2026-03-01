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

class TupleValueGetItemDoubleTest : public AniGTestTupleOps {
public:
    static constexpr ani_double EXPECTED_RESULT = 3.14;
    static constexpr ani_double EXPECTED_RESULT_2 = 2.71;
    static constexpr ani_double EXPECTED_RESULT_3 = 1.61;
    static constexpr ani_double EXPECTED_RESULT_4 = 0.59;
    static constexpr ani_double EXPECTED_RESULT_5 = 10.0;
};

TEST_F(TupleValueGetItemDoubleTest, tupleValueGetItemDouble)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_double_test", "getDoubleTuple");
    ani_double result = 0.0;
    ASSERT_EQ(env_->TupleValue_GetItem_Double(tuple, 0U, &result), ANI_OK);
    ASSERT_EQ(result, EXPECTED_RESULT);
}

TEST_F(TupleValueGetItemDoubleTest, tupleValueGetItemDoubleNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_double_test", "getDoubleTuple");
    ani_double result = 0.0;
    ASSERT_EQ(env_->c_api->TupleValue_GetItem_Double(nullptr, tuple, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemDoubleTest, tupleValueGetItemDoubleNullTuple)
{
    ani_double result = 0.0;
    ASSERT_EQ(env_->TupleValue_GetItem_Double(nullptr, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemDoubleTest, tupleValueGetItemDoubleIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_double_test", "getDoubleTuple");
    ani_double result = 0.0;
    ASSERT_EQ(env_->TupleValue_GetItem_Double(tuple, 6U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemDoubleTest, tupleValueGetItemDoubleIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_double_test", "getDoubleTuple");
    ani_double result = 0.0;
    ASSERT_EQ(env_->TupleValue_GetItem_Double(tuple, -1U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemDoubleTest, tupleValueGetItemDoubleIndexOutOfRange3)
{
    const ani_size maxNum = std::numeric_limits<ani_size>::max();
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_double_test", "getDoubleTuple");
    ani_double result = 0.0;
    ASSERT_EQ(env_->TupleValue_GetItem_Double(tuple, maxNum, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemDoubleTest, tupleValueGetItemDoubleCompositeScene)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_double_test", "getDoubleTuple");

    const std::array<ani_double, 5U> expectedValues = {EXPECTED_RESULT, EXPECTED_RESULT_2, EXPECTED_RESULT_3,
                                                       EXPECTED_RESULT_4, EXPECTED_RESULT_5};

    ani_double result = 0.0;
    for (size_t i = 0; i < expectedValues.size(); ++i) {
        ASSERT_EQ(env_->TupleValue_GetItem_Double(tuple, i, &result), ANI_OK);
        ASSERT_EQ(result, expectedValues[i]);
    }
}

TEST_F(TupleValueGetItemDoubleTest, tupleValueGetItemDoubleNullResult)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_double_test", "getDoubleTuple");
    ASSERT_EQ(env_->TupleValue_GetItem_Double(tuple, 0U, nullptr), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemDoubleTest, tupleValueGetItemDoubleRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_double_test", "getDoubleTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_double result = 0.0;
        ASSERT_EQ(env_->TupleValue_GetItem_Double(tuple, 0U, &result), ANI_OK);
        ASSERT_EQ(result, EXPECTED_RESULT);
    }
}
}  // namespace ark::ets::ani::testing