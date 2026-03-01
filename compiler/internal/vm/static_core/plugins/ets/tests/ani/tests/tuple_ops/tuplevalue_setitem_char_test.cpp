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

class TupleValueSetItemCharTest : public AniGTestTupleOps {};

TEST_F(TupleValueSetItemCharTest, tupleValueSetItemChar)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_char_test", "getCharTuple");
    ani_char value = 'a';
    ASSERT_EQ(env_->TupleValue_SetItem_Char(tuple, 0U, value), ANI_OK);
    ani_char result = '\0';
    ASSERT_EQ(env_->TupleValue_GetItem_Char(tuple, 0U, &result), ANI_OK);
    ASSERT_EQ(result, value);
}

TEST_F(TupleValueSetItemCharTest, tupleValueSetItemCharNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_char_test", "getCharTuple");
    ani_char value = 'a';
    ASSERT_EQ(env_->c_api->TupleValue_SetItem_Char(nullptr, tuple, 0U, value), ANI_INVALID_ARGS);
}

TEST_F(TupleValueSetItemCharTest, tupleValueSetItemCharNullTuple)
{
    ani_char value = 'a';
    ASSERT_EQ(env_->TupleValue_SetItem_Char(nullptr, 0U, value), ANI_INVALID_ARGS);
}

TEST_F(TupleValueSetItemCharTest, tupleValueSetItemCharIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_char_test", "getCharTuple");
    ani_char value = 'a';
    ASSERT_EQ(env_->TupleValue_SetItem_Char(tuple, 6U, value), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueSetItemCharTest, tupleValueSetItemCharIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_char_test", "getCharTuple");
    ani_char value = 'a';
    ASSERT_EQ(env_->TupleValue_SetItem_Char(tuple, -1U, value), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueSetItemCharTest, tupleValueSetItemCharRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_char_test", "getCharTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_char value = 'a';
        ASSERT_EQ(env_->TupleValue_SetItem_Char(tuple, 0U, value), ANI_OK);
        ani_char result = '\0';
        ASSERT_EQ(env_->TupleValue_GetItem_Char(tuple, 0U, &result), ANI_OK);
        ASSERT_EQ(result, value);
    }
}
}  // namespace ark::ets::ani::testing