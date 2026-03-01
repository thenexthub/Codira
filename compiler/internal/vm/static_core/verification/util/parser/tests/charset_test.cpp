/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#include <vector>

#include <gtest/gtest.h>

#include "util/parser/charset.h"

namespace ark::parser::test {
TEST(VerifierParserTest, Charset)
{
    std::vector<unsigned char> str {'s', 't', 'r', '1', '\0', '\0'};
    Charset<unsigned char> cs1 {&(str[0])};
    str[3] = '2';
    // NOLINTNEXTLINE(readability-magic-numbers)
    str[4] = 255;
    Charset<unsigned char> cs2 {&(str[0])};
    EXPECT_TRUE(cs1('t'));
    EXPECT_FALSE(cs1('q'));
    EXPECT_TRUE(cs2('2'));
    EXPECT_FALSE(cs2('1'));
    EXPECT_TRUE(cs2(255));

    Charset cs3 = cs1 + cs2;
    EXPECT_TRUE(cs3('t'));
    EXPECT_FALSE(cs3('p'));
    EXPECT_TRUE(cs3('2'));
    EXPECT_TRUE(cs3('1'));
    EXPECT_TRUE(cs3(255));

    Charset cs4 = cs1 - cs2;
    EXPECT_FALSE(cs4('t'));
    EXPECT_FALSE(cs4('p'));
    EXPECT_FALSE(cs4('2'));
    EXPECT_TRUE(cs4('1'));
    EXPECT_FALSE(cs4(255));

    Charset cs5 = !cs1;
    EXPECT_FALSE(cs5('t'));
    EXPECT_TRUE(cs5('p'));
    EXPECT_TRUE(cs5('2'));
    EXPECT_FALSE(cs5('1'));
    EXPECT_TRUE(cs5(255));
}
}  // namespace ark::parser::test