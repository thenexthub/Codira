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

// NOLINTBEGIN(cppcoreguidelines-pro-type-vararg, readability-magic-numbers, modernize-avoid-c-arrays)
namespace ark::ets::ani::testing {

class TupleValueGetNumberOfItemTest : public AniGTestTupleOps {};

TEST_F(TupleValueGetNumberOfItemTest, getLengthOfEmptyTuple)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getnumberofitem_test", "getEmptyTuple");

    ani_size length;
    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(tuple, &length), ANI_OK);
    ASSERT_EQ(length, 0U);
}

TEST_F(TupleValueGetNumberOfItemTest, getLengthOfValidTuple)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getnumberofitem_test", "getTestPrimitiveTuple");

    ani_size length;
    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(tuple, &length), ANI_OK);
    ASSERT_EQ(length, 8U);
}

TEST_F(TupleValueGetNumberOfItemTest, identicalTuple)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getnumberofitem_test", "getCharTuple");

    ani_size length;
    ASSERT_EQ(env_->TupleValue_GetNumberOfItems(tuple, &length), ANI_OK);

    constexpr std::array<char, 5U> EXPECTED_TUPLE = {'H', 'e', 'l', 'l', 'o'};

    ani_char currentChar;
    for (ani_size idx = 0; idx < length; ++idx) {
        env_->TupleValue_GetItem_Char(tuple, idx, &currentChar);
        ASSERT_EQ(currentChar, EXPECTED_TUPLE[idx]);
    }
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PRIMITIVE_GET_SET_TEST_CASE(calltype, anitype, idx, expectedValue, updateValue)  \
    do {                                                                                 \
        anitype _result;                                                                 \
        ASSERT_EQ(env_->TupleValue_GetItem_##calltype(tuple, idx, &_result), ANI_OK);    \
        ASSERT_EQ(_result, expectedValue);                                               \
                                                                                         \
        ASSERT_EQ(env_->TupleValue_SetItem_##calltype(tuple, idx, updateValue), ANI_OK); \
        ASSERT_EQ(env_->TupleValue_GetItem_##calltype(tuple, idx, &_result), ANI_OK);    \
        ASSERT_EQ(_result, updateValue);                                                 \
    } while (false)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TEST_TUPLE_CHANGE_VALUE(tuple, anitype, calltype, expectedArr, modifyArr)                        \
    do {                                                                                                 \
        const ani_size maxNum = std::numeric_limits<ani_size>::max();                                    \
        ASSERT_EQ(env_->TupleValue_SetItem_##calltype(tuple, maxNum, (modifyArr)[0]), ANI_OUT_OF_RANGE); \
        ani_size length = 0U;                                                                            \
        ASSERT_EQ(env_->TupleValue_GetNumberOfItems(tuple, &length), ANI_OK);                            \
        for (ani_size i = 0; i < length; ++i) {                                                          \
            PRIMITIVE_GET_SET_TEST_CASE(calltype, anitype, i, (expectedArr)[i], (modifyArr)[i]);         \
        }                                                                                                \
    } while (false)

TEST_F(TupleValueGetNumberOfItemTest, primitiveTuple)
{
    auto tuple = GetTupleWithCheck("tuplevalue_getnumberofitem_test", "getTestPrimitiveTuple");

    // clang-format off
    PRIMITIVE_GET_SET_TEST_CASE(Boolean, ani_boolean, 0U, true,    false);
    PRIMITIVE_GET_SET_TEST_CASE(Char,    ani_char,    1U, 'x',     'y');
    PRIMITIVE_GET_SET_TEST_CASE(Byte,    ani_byte,    2U, 127U,    100U);
    PRIMITIVE_GET_SET_TEST_CASE(Short,   ani_short,   3U, 300U,    100U);
    PRIMITIVE_GET_SET_TEST_CASE(Int,     ani_int,     4U, 500U,   -100L);
    PRIMITIVE_GET_SET_TEST_CASE(Long,    ani_long,    5U, 600U,   -100L);
    PRIMITIVE_GET_SET_TEST_CASE(Float,   ani_float,   6U, 100.0F, -100.0F);
    PRIMITIVE_GET_SET_TEST_CASE(Double,  ani_double,  7U, 400.0F, -100.0F);
    // clang-format on
}

TEST_F(TupleValueGetNumberOfItemTest, changeValueBooleanTuple)
{
    constexpr std::array<ani_boolean, 5U> EXPECTED_TUPLE = {ANI_TRUE, ANI_FALSE, ANI_TRUE, ANI_FALSE, ANI_TRUE};
    constexpr std::array<ani_boolean, 5U> MODIFY_TUPLE = {ANI_FALSE, ANI_TRUE, ANI_FALSE, ANI_TRUE, ANI_FALSE};
    auto tuple = GetTupleWithCheck("tuplevalue_getnumberofitem_test", "getBooleanTuple");
    TEST_TUPLE_CHANGE_VALUE(tuple, ani_boolean, Boolean, EXPECTED_TUPLE, MODIFY_TUPLE);
}

TEST_F(TupleValueGetNumberOfItemTest, changeValueCharTuple)
{
    constexpr std::array<char, 5U> EXPECTED_TUPLE = {'H', 'e', 'l', 'l', 'o'};
    constexpr std::array<char, 5U> MODIFY_TUPLE = {'p', 'o', 'w', 'e', 'r'};
    auto tuple = GetTupleWithCheck("tuplevalue_getnumberofitem_test", "getCharTuple");
    TEST_TUPLE_CHANGE_VALUE(tuple, ani_char, Char, EXPECTED_TUPLE, MODIFY_TUPLE);
}

TEST_F(TupleValueGetNumberOfItemTest, changeValueByteTuple)
{
    constexpr std::array<ani_byte, 5U> EXPECTED_TUPLE = {125, 126, 127, 1, 2};
    constexpr std::array<ani_byte, 5U> MODIFY_TUPLE = {1, 2, 3, 4, 5};
    auto tuple = GetTupleWithCheck("tuplevalue_getnumberofitem_test", "getByteTuple");
    TEST_TUPLE_CHANGE_VALUE(tuple, ani_byte, Byte, EXPECTED_TUPLE, MODIFY_TUPLE);
}

TEST_F(TupleValueGetNumberOfItemTest, changeValueShortTuple)
{
    constexpr std::array<ani_short, 5U> EXPECTED_TUPLE = {300, 350, 200, 100, 50};
    constexpr std::array<ani_short, 5U> MODIFY_TUPLE = {100, 200, 300, 400, 500};
    auto tuple = GetTupleWithCheck("tuplevalue_getnumberofitem_test", "getShortTuple");
    TEST_TUPLE_CHANGE_VALUE(tuple, ani_short, Short, EXPECTED_TUPLE, MODIFY_TUPLE);
}

TEST_F(TupleValueGetNumberOfItemTest, changeValueIntTuple)
{
    constexpr std::array<ani_int, 5U> EXPECTED_TUPLE = {300, 350, 200, 100, 50};
    constexpr std::array<ani_int, 5U> MODIFY_TUPLE = {100, 200, 300, 400, 500};
    auto tuple = GetTupleWithCheck("tuplevalue_getnumberofitem_test", "getIntTuple");
    TEST_TUPLE_CHANGE_VALUE(tuple, ani_int, Int, EXPECTED_TUPLE, MODIFY_TUPLE);
}

TEST_F(TupleValueGetNumberOfItemTest, changeValueLongTuple)
{
    constexpr std::array<ani_long, 5U> EXPECTED_TUPLE = {30000, 20000, 10000, 5000, 100000};
    constexpr std::array<ani_long, 5U> MODIFY_TUPLE = {190000, 290000, 390000, 490000, 590000};
    auto tuple = GetTupleWithCheck("tuplevalue_getnumberofitem_test", "getLongTuple");
    TEST_TUPLE_CHANGE_VALUE(tuple, ani_long, Long, EXPECTED_TUPLE, MODIFY_TUPLE);
}

TEST_F(TupleValueGetNumberOfItemTest, changeValueFloatTuple)
{
    constexpr std::array<ani_float, 5U> EXPECTED_TUPLE = {3.14, 2.71, 1.61, 0.59, 10.0};
    constexpr std::array<ani_float, 5U> MODIFY_TUPLE = {190000.12, 290000.34, 390000.56, 490000.78, 590000.09};
    auto tuple = GetTupleWithCheck("tuplevalue_getnumberofitem_test", "getFloatTuple");
    TEST_TUPLE_CHANGE_VALUE(tuple, ani_float, Float, EXPECTED_TUPLE, MODIFY_TUPLE);
}

TEST_F(TupleValueGetNumberOfItemTest, changeValueDoubleTuple)
{
    constexpr std::array<ani_double, 5U> EXPECTED_TUPLE = {3.145, 2.716, 1.611, 0.594, 10.08};
    constexpr std::array<ani_double, 5U> MODIFY_TUPLE = {190000.12, 290000.34, 390000.56, 490000.78, 590000.09};
    auto tuple = GetTupleWithCheck("tuplevalue_getnumberofitem_test", "getDoubleTuple");
    TEST_TUPLE_CHANGE_VALUE(tuple, ani_double, Double, EXPECTED_TUPLE, MODIFY_TUPLE);
}

}  // namespace ark::ets::ani::testing

// NOLINTEND(cppcoreguidelines-pro-type-vararg, readability-magic-numbers, modernize-avoid-c-arrays)