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

class TupleValueSetItemFloatTest : public AniGTestTupleOps {
public:
    static constexpr ani_float SET_VALUE = 1.23456789F;
};

TEST_F(TupleValueSetItemFloatTest, tupleValueSetItemFloat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_float_test", "getFloatTuple");
    ani_float value = SET_VALUE;
    ASSERT_EQ(env_->TupleValue_SetItem_Float(tuple, 0U, value), ANI_OK);
    ani_float result = 0.0F;
    ASSERT_EQ(env_->TupleValue_GetItem_Float(tuple, 0U, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(TupleValueSetItemFloatTest, tupleValueSetItemFloatNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_float_test", "getFloatTuple");
    ani_float value = SET_VALUE;
    ASSERT_EQ(env_->c_api->TupleValue_SetItem_Float(nullptr, tuple, 0U, value), ANI_INVALID_ARGS);
}

TEST_F(TupleValueSetItemFloatTest, tupleValueSetItemFloatNullTuple)
{
    ani_float value = SET_VALUE;
    ASSERT_EQ(env_->TupleValue_SetItem_Float(nullptr, 0U, value), ANI_INVALID_ARGS);
}

TEST_F(TupleValueSetItemFloatTest, tupleValueSetItemFloatIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_float_test", "getFloatTuple");
    ani_float value = SET_VALUE;
    ASSERT_EQ(env_->TupleValue_SetItem_Float(tuple, 6U, value), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueSetItemFloatTest, tupleValueSetItemFloatIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_float_test", "getFloatTuple");
    ani_float value = SET_VALUE;
    ASSERT_EQ(env_->TupleValue_SetItem_Float(tuple, -1U, value), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueSetItemFloatTest, tupleValueSetItemFloatRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_float_test", "getFloatTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_float value = SET_VALUE;
        ASSERT_EQ(env_->TupleValue_SetItem_Float(tuple, 0U, value), ANI_OK);
        ani_float result = 0.0F;
        ASSERT_EQ(env_->TupleValue_GetItem_Float(tuple, 0U, &result), ANI_OK);
        ASSERT_EQ(result, value);
    }
}
}  // namespace ark::ets::ani::testing