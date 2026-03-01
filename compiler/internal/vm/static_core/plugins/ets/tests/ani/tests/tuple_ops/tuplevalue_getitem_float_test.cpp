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

class TupleValueGetItemFloatTest : public AniGTestTupleOps {
public:
    static constexpr ani_float EXPECTED_RESULT = 3.14F;
    static constexpr ani_float EXPECTED_RESULT_2 = 2.71F;
    static constexpr ani_float EXPECTED_RESULT_3 = 1.61F;
    static constexpr ani_float EXPECTED_RESULT_4 = 0.59F;
    static constexpr ani_float EXPECTED_RESULT_5 = 10.0F;
};

TEST_F(TupleValueGetItemFloatTest, tupleValueGetItemFloat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_float_test", "getFloatTuple");
    ani_float result = 0.0F;
    ASSERT_EQ(env_->TupleValue_GetItem_Float(tuple, 0U, &result), ANI_OK);
    ASSERT_EQ(result, EXPECTED_RESULT);
}

TEST_F(TupleValueGetItemFloatTest, tupleValueGetItemFloatNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_float_test", "getFloatTuple");
    ani_float result = 0.0F;
    ASSERT_EQ(env_->c_api->TupleValue_GetItem_Float(nullptr, tuple, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemFloatTest, tupleValueGetItemFloatNullTuple)
{
    ani_float result = 0.0F;
    ASSERT_EQ(env_->TupleValue_GetItem_Float(nullptr, 0U, &result), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemFloatTest, tupleValueGetItemFloatIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_float_test", "getFloatTuple");
    ani_float result = 0.0F;
    ASSERT_EQ(env_->TupleValue_GetItem_Float(tuple, 6U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemFloatTest, tupleValueGetItemFloatIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_float_test", "getFloatTuple");
    ani_float result = 0.0F;
    ASSERT_EQ(env_->TupleValue_GetItem_Float(tuple, -1U, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemFloatTest, tupleValueGetItemFloatIndexOutOfRange3)
{
    const ani_size maxNum = std::numeric_limits<ani_size>::max();
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_float_test", "getFloatTuple");
    ani_float result = 0.0F;
    ASSERT_EQ(env_->TupleValue_GetItem_Float(tuple, maxNum, &result), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueGetItemFloatTest, tupleValueGetItemFloatCompositeScene)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_float_test", "getFloatTuple");

    const std::array<ani_float, 5U> expectedValues = {EXPECTED_RESULT, EXPECTED_RESULT_2, EXPECTED_RESULT_3,
                                                      EXPECTED_RESULT_4, EXPECTED_RESULT_5};

    ani_float result = 0.0F;
    for (size_t i = 0; i < expectedValues.size(); ++i) {
        ASSERT_EQ(env_->TupleValue_GetItem_Float(tuple, i, &result), ANI_OK);
        ASSERT_EQ(result, expectedValues[i]);
    }
}

TEST_F(TupleValueGetItemFloatTest, tupleValueGetItemFloatNullResult)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_float_test", "getFloatTuple");
    ASSERT_EQ(env_->TupleValue_GetItem_Float(tuple, 0U, nullptr), ANI_INVALID_ARGS);
}

TEST_F(TupleValueGetItemFloatTest, tupleValueGetItemFloatRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getitem_float_test", "getFloatTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_float result = 0.0F;
        ASSERT_EQ(env_->TupleValue_GetItem_Float(tuple, 0U, &result), ANI_OK);
        ASSERT_EQ(result, EXPECTED_RESULT);
    }
}
}  // namespace ark::ets::ani::testing