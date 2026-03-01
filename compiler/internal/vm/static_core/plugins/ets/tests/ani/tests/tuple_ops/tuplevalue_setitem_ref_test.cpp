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

class TupleValueSetItemRefTest : public AniGTestTupleOps {
public:
    static constexpr std::string_view EXPECTED_ELEM = "abcdef";
};

TEST_F(TupleValueSetItemRefTest, tupleValueSetItemRef)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_ref_test", "getReferenceTuple");
    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8(EXPECTED_ELEM.data(), EXPECTED_ELEM.size(), &string), ANI_OK);
    ASSERT_EQ(env_->TupleValue_SetItem_Ref(tuple, 0U, string), ANI_OK);
    ani_ref result {};
    ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 0U, &result), ANI_OK);
    auto internalStr = reinterpret_cast<ani_string>(result);
    std::array<char, EXPECTED_ELEM.size() + 1> elem {};
    ani_size copiedSize = 0;
    ASSERT_EQ(env_->String_GetUTF8SubString(internalStr, 0U, elem.size() - 1, elem.data(), elem.size(), &copiedSize),
              ANI_OK);
    ASSERT_EQ(copiedSize, EXPECTED_ELEM.size());
    ASSERT_STREQ(elem.data(), EXPECTED_ELEM.data());
}

TEST_F(TupleValueSetItemRefTest, tupleValueSetItemRefNullEnv)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_ref_test", "getReferenceTuple");
    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8(EXPECTED_ELEM.data(), EXPECTED_ELEM.size(), &string), ANI_OK);
    ASSERT_EQ(env_->c_api->TupleValue_SetItem_Ref(nullptr, tuple, 0U, string), ANI_INVALID_ARGS);
}

TEST_F(TupleValueSetItemRefTest, DISABLED_tupleValueSetItemRefCharTuple)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_ref_test", "getCharTuple");
    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8(EXPECTED_ELEM.data(), EXPECTED_ELEM.size(), &string), ANI_OK);
    ASSERT_EQ(env_->TupleValue_SetItem_Ref(tuple, 0U, string), ANI_INVALID_TYPE);
}

TEST_F(TupleValueSetItemRefTest, tupleValueSetItemRefNullTuple)
{
    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8(EXPECTED_ELEM.data(), EXPECTED_ELEM.size(), &string), ANI_OK);
    ASSERT_EQ(env_->TupleValue_SetItem_Ref(nullptr, 0U, string), ANI_INVALID_ARGS);
}

TEST_F(TupleValueSetItemRefTest, tupleValueSetItemRefIndexOutOfRange1)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_ref_test", "getReferenceTuple");
    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8(EXPECTED_ELEM.data(), EXPECTED_ELEM.size(), &string), ANI_OK);
    ASSERT_EQ(env_->TupleValue_SetItem_Ref(tuple, 4U, string), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueSetItemRefTest, tupleValueSetItemRefIndexOutOfRange2)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_ref_test", "getReferenceTuple");
    ani_string string {};
    ASSERT_EQ(env_->String_NewUTF8(EXPECTED_ELEM.data(), EXPECTED_ELEM.size(), &string), ANI_OK);
    ASSERT_EQ(env_->TupleValue_SetItem_Ref(tuple, -1U, string), ANI_OUT_OF_RANGE);
}

TEST_F(TupleValueSetItemRefTest, tupleValueSetItemRefNullValue)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_ref_test", "getReferenceTuple");
    ASSERT_EQ(env_->TupleValue_SetItem_Ref(tuple, 0U, nullptr), ANI_INVALID_ARGS);
}

TEST_F(TupleValueSetItemRefTest, tupleValueSetItemRefRepeat)
{
    auto tuple = GetTupleWithCheck("tuplevalue_setitem_ref_test", "getReferenceTuple");
    for (int32_t i = 0; i < LOOP_COUNT; i++) {
        ani_string string {};
        ASSERT_EQ(env_->String_NewUTF8(EXPECTED_ELEM.data(), EXPECTED_ELEM.size(), &string), ANI_OK);
        ASSERT_EQ(env_->TupleValue_SetItem_Ref(tuple, 0U, string), ANI_OK);
        ani_ref result {};
        ASSERT_EQ(env_->TupleValue_GetItem_Ref(tuple, 0U, &result), ANI_OK);
        auto internalStr = reinterpret_cast<ani_string>(result);
        std::array<char, EXPECTED_ELEM.size() + 1> elem {};
        ani_size copiedSize = 0;
        ASSERT_EQ(
            env_->String_GetUTF8SubString(internalStr, 0U, elem.size() - 1, elem.data(), elem.size(), &copiedSize),
            ANI_OK);
        ASSERT_EQ(copiedSize, EXPECTED_ELEM.size());
        ASSERT_STREQ(elem.data(), EXPECTED_ELEM.data());
    }
}
}  // namespace ark::ets::ani::testing
