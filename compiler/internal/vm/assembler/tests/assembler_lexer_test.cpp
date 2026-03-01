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

#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include "define.h"
#include "lexer.h"

using namespace panda::pandasm;
using namespace testing::ext;

namespace panda::pandasm {
class LexerTest : public testing::Test {
};

/**
 * @tc.name: lexer_test_001
 * @tc.desc: Verify the TokenTypeWhat function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LexerTest, lexer_test_001, TestSize.Level1)
{
    Lexer l;
    std::string s = "mov v1, v2";
    Tokens tok = l.TokenizeString(s);
    EXPECT_EQ(TokenTypeWhat(tok.first[0].type), "OPERATION") << "OPERATION expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[1].type), "ID") << "ID expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[2].type), "DEL_COMMA") << "DEL_COMMA expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[3].type), "ID") << "ID expected";
    EXPECT_EQ(tok.second.err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: lexer_test_002
 * @tc.desc: Verify the TokenTypeWhat function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LexerTest, lexer_test_002, TestSize.Level1)
{
    Lexer l;
    std::string s = "ldai 1";
    Tokens tok = l.TokenizeString(s);
    EXPECT_EQ(TokenTypeWhat(tok.first[0].type), "OPERATION") << "OPERATION expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[1].type), "ID") << "ID expected";
    EXPECT_EQ(tok.second.err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: lexer_test_003
 * @tc.desc: Verify the TokenTypeWhat function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LexerTest, lexer_test_003, TestSize.Level1)
{
    Lexer l;
    std::string s = "movi\nlda v2 v10 mov v2";
    Tokens tok = l.TokenizeString(s);
    EXPECT_EQ(TokenTypeWhat(tok.first[0].type), "ID") << "ID expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[1].type), "OPERATION") << "OPERATION expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[2].type), "ID") << "ID expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[3].type), "ID") << "ID expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[4].type), "OPERATION") << "OPERATION expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[5].type), "ID") << "ID expected";
    EXPECT_EQ(tok.second.err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: lexer_test_004
 * @tc.desc: Verify the TokenTypeWhat function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LexerTest, lexer_test_004, TestSize.Level1)
{
    Lexer l;
    std::string s = "jmp Iasdfsadkfjhasifhsaiuhdacoisjdaociewhasdasdfkjasdfhjksadhfkhsakdfjhksajhdkfjhskhdfkjahhjdskaj";
    Tokens tok = l.TokenizeString(s);
    EXPECT_EQ(TokenTypeWhat(tok.first[0].type), "OPERATION") << "OPERATION expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[1].type), "ID") << "ID expected";
    EXPECT_EQ(tok.second.err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: lexer_test_005
 * @tc.desc: Verify the TokenTypeWhat function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LexerTest, lexer_test_005, TestSize.Level1)
{
    Lexer l;
    std::string s = "call.short 1111, 1";
    Tokens tok = l.TokenizeString(s);
    EXPECT_EQ(TokenTypeWhat(tok.first[0].type), "ID") << "ID expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[1].type), "ID") << "ID expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[2].type), "DEL_COMMA") << "DEL_COMMA expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[3].type), "ID") << "ID expected";
    EXPECT_EQ(tok.second.err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: lexer_test_006
 * @tc.desc: Verify the TokenTypeWhat function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LexerTest, lexer_test_006, TestSize.Level1)
{
    Lexer l;
    std::string s = "jle v1 met";
    Tokens tok = l.TokenizeString(s);
    EXPECT_EQ(TokenTypeWhat(tok.first[0].type), "ID") << "ID expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[1].type), "ID") << "ID expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[2].type), "ID") << "ID expected";
    EXPECT_EQ(tok.second.err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: lexer_test_007
 * @tc.desc: Verify the TokenTypeWhat function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LexerTest, lexer_test_007, TestSize.Level1)
{
    Lexer l;
    std::string s = "label:";
    Tokens tok = l.TokenizeString(s);
    EXPECT_EQ(TokenTypeWhat(tok.first[0].type), "ID") << "ID expected";
    EXPECT_EQ(tok.second.err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: lexer_test_008
 * @tc.desc: Verify the TokenTypeWhat function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LexerTest, lexer_test_008, TestSize.Level1)
{
    Lexer l;
    std::string s = ",";
    Tokens tok = l.TokenizeString(s);
    EXPECT_EQ(TokenTypeWhat(tok.first[0].type), "DEL_COMMA") << "DEL_COMMA expected";
    EXPECT_EQ(tok.second.err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: lexer_test_009
 * @tc.desc: Verify the TokenTypeWhat function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LexerTest, lexer_test_009, TestSize.Level1)
{
    Lexer l;
    std::string s = ",:{}()<>=";
    Tokens tok = l.TokenizeString(s);
    EXPECT_EQ(TokenTypeWhat(tok.first[0].type), "DEL_COMMA") << "DEL_COMMA expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[1].type), "DEL_COLON") << "DEL_COMMA expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[2].type), "DEL_BRACE_L") << "DEL_COMMA expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[3].type), "DEL_BRACE_R") << "DEL_COMMA expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[4].type), "DEL_BRACKET_L") << "DEL_BRACKET_L expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[5].type), "DEL_BRACKET_R") << "DEL_BRACKET_R expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[6].type), "DEL_LT") << "DEL_LT expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[7].type), "DEL_GT") << "DEL_GT expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[8].type), "DEL_EQ") << "DEL_EQ expected";
    EXPECT_EQ(tok.second.err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: lexer_test_010
 * @tc.desc: Verify the TokenTypeWhat function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LexerTest, lexer_test_010, TestSize.Level1)
{
    Lexer l;
    std::string s =
        "i64.to.f32 alsdhashdjskhfka "
        "shdkfhkasdhfkhsakdhfkshkfhskahlfkjsdfkjadskhfkshadkhfsdakhfksahdkfaksdfkhaskldhkfashdlfkjhasdkjfhklasjhdfklhsa"
        "fhska";
    Tokens tok = l.TokenizeString(s);
    EXPECT_EQ(tok.second.err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: lexer_test_011
 * @tc.desc: Verify the TokenTypeWhat function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LexerTest, lexer_test_011, TestSize.Level1)
{
    Lexer l;
    std::string s = ".function asd(u32){}";
    Tokens tok = l.TokenizeString(s);

    EXPECT_EQ(TokenTypeWhat(tok.first[0].type), "KEYWORD") << "KEYWORD expected";
    EXPECT_EQ(TokenTypeWhat(tok.first[1].type), "ID") << "ID expected";
    EXPECT_EQ(tok.second.err, Error::ErrorType::ERR_NONE) << "ERR_NONE expected";
}

/**
 * @tc.name: lexer_test_012
 * @tc.desc: Verify the TokenizeString function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LexerTest, lexer_test_012, TestSize.Level1)
{
    {
        Lexer l;
        std::string s = "\"123";
        Tokens tok = l.TokenizeString(s);

        Error e = tok.second;

        EXPECT_EQ(e.err, Error::ErrorType::ERR_STRING_MISSING_TERMINATING_CHARACTER);
    }

    {
        Lexer l;
        std::string s = "\"123\\\"";
        Tokens tok = l.TokenizeString(s);

        Error e = tok.second;

        EXPECT_EQ(e.err, Error::ErrorType::ERR_STRING_MISSING_TERMINATING_CHARACTER);
    }

    {
        Lexer l;
        std::string s = "\" a b \\ c d \"";
        Tokens tok = l.TokenizeString(s);

        Error e = tok.second;

        EXPECT_EQ(e.err, Error::ErrorType::ERR_NONE);
        EXPECT_EQ(tok.first.size(), 1U);
        EXPECT_EQ(tok.first[0].type, Token::Type::ID_STRING);
        EXPECT_EQ(tok.first[0].bound_left, 0U);
        EXPECT_EQ(tok.first[0].bound_right, s.length());
    }

    {
        Lexer l;
        std::string s = "\"abcd\"1234";
        Tokens tok = l.TokenizeString(s);

        Error e = tok.second;

        EXPECT_EQ(e.err, Error::ErrorType::ERR_NONE);
        EXPECT_EQ(tok.first.size(), 2U);
        EXPECT_EQ(tok.first[0].type, Token::Type::ID_STRING);
        EXPECT_EQ(tok.first[0].bound_left, 0U);
        EXPECT_EQ(tok.first[0].bound_right, s.find('1'));
    }
}

/**
 * @tc.name: lexer_test_013
 * @tc.desc: Verify the TokenizeString function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LexerTest, lexer_test_013, TestSize.Level1)
{
    Lexer l;
    std::string s = "i32[]";
    Tokens tok = l.TokenizeString(s);

    Error e = tok.second;
    EXPECT_EQ(e.err, Error::ErrorType::ERR_NONE);
    EXPECT_EQ(tok.first.size(), 3U);
    EXPECT_EQ(tok.first[0].type, Token::Type::ID);
    EXPECT_EQ(tok.first[1].type, Token::Type::DEL_SQUARE_BRACKET_L);
    EXPECT_EQ(tok.first[2].type, Token::Type::DEL_SQUARE_BRACKET_R);
}
}
