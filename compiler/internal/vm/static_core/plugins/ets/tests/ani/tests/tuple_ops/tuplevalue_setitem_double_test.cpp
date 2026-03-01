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

class TupleValueSetItemDoubleTest : public AniGTestTupleOps {
public:
    static constexpr ani_double SET_VALUE = 123.456;
};

TEST_F(TupleValueSetItemDoubleTest, tupleValueSetItemDouble)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_double_test", "getDoubleTuple");
    ani_double value = SET_VALUE;
    ASSERT_EQ(env_->TupleValue_SetItem_Double(tuple, 0U, value), ANI_OK);
    ani_double result = 0.0;
    ASSERT_EQ(env_->TupleValue_GetItem_Double(tuple, 0U, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(TupleValueSetItemDoubleTest, tupleValueSetItemDoubleNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_double_test", "getDoubleTuple");
    ani_double value = SET_VALUE;
    ASSERT_EQ(env_->c_api->TupleValue_SetItem_Double(nullptr, tuple, 0U, value), ANI_INVALID_ARGS);
}

TEST_F(TupleValueSetItemDoubleTest, tupleValueSetItemDoubleNullTuple)
{
    ani_double value = SET_VALUE;
    ASSERT_EQ(env_->TupleValue_SetItem_Double(nullptr, 0U, value), ANI_INVALID_ARGS);
}

TEST_F(TupleValueSetItemDoubleTest, tupleValueSetItemDoubleIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_double_test", "getDoubleTuple");
    ani_double value = SET_VALUE;
    ASSERT_EQ(env_->TupleValue_SetItem_Double(tuple, 6U, value), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueSetItemDoubleTest, tupleValueSetItemDoubleIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_double_test", "getDoubleTuple");
    ani_double value = SET_VALUE;
    ASSERT_EQ(env_->TupleValue_SetItem_Double(tuple, -1U, value), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueSetItemDoubleTest, tupleValueSetItemDoubleRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_double_test", "getDoubleTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_double value = SET_VALUE;
        ASSERT_EQ(env_->TupleValue_SetItem_Double(tuple, 0U, value), ANI_OK);
        ani_double result = 0.0;
        ASSERT_EQ(env_->TupleValue_GetItem_Double(tuple, 0U, &result), ANI_OK);
        ASSERT_EQ(result, value);
    }
}
}  // namespace ark::ets::ani::testing