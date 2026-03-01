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

#include <iostream>
#include <string>

#include "gtest/gtest.h"

#include "Codira/Basic/Print.h"
#include "Codira/Basic/StringConvertor.h"
#include "Codira/Lex/Lexer.h"

using namespace Codira;

class LexerTerminatorTest : public ::testing::Test {
protected:
    void SetUp() override
    {
    }

    std::unique_ptr<Lexer> lexer;
    DiagnosticEngine diag{};
};

TEST_F(LexerTerminatorTest, End)
{
    SourceManager sm2;
    std::vector<Position> expectPos = {{0, 2, 6}, {0, 2, 5}, {0, 2, 4}};
    std::vector<std::string> escape_n = {"\"\"\"\nab\"\"\"", "\"\"\"a\nb\"\"\"", "\"\"\"ab\n\"\"\""};
    for (size_t i = 0; i < expectPos.size(); i++) {
        Lexer lexer = Lexer(escape_n[i], diag, sm2);
        Token tok = lexer.Next();
        Position endPos = tok.End();
        EXPECT_EQ(endPos, expectPos[i]);
    }

    std::vector<std::string> escape_r = {"\"\"\"\rab\"\"\"", "\"\"\"a\rb\"\"\"", "\"\"\"ab\r\"\"\""};
    for (size_t i = 0; i < expectPos.size(); i++) {
        Lexer lexer = Lexer(escape_r[i], diag, sm2);
        Token tok = lexer.Next();
        Position endPos = tok.End();
        EXPECT_EQ(endPos, Position(0, 1, 10));
    }

    std::vector<std::string> escape_r_n = {"\"\"\"\r\nab\"\"\"", "\"\"\"a\r\nb\"\"\"", "\"\"\"ab\r\n\"\"\""};
    for (size_t i = 0; i < expectPos.size(); i++) {
        Lexer lexer = Lexer(escape_r_n[i], diag, sm2);
        Token tok = lexer.Next();
        Position endPos = tok.End();
        EXPECT_EQ(endPos, expectPos[i]);
    }
}

TEST_F(LexerTerminatorTest, ScanComment)
{
    SourceManager sm2;
    std::vector<std::string> terminator = {"//abc\n", "//abc\r\n"};
    Position expect_pos = {0, 1, 6};
    for (size_t i = 0; i < terminator.size(); i++) {
        Lexer* lexer = new Lexer(terminator[i], diag, sm2);
        Token tok = lexer->Next();
        Position endPos = tok.End();
        EXPECT_EQ(endPos, expect_pos);
    }
    std::string nonTerminator = "//abc\r";
    Lexer lexer = Lexer(nonTerminator, diag, sm2);
    Token tok = lexer.Next();
    Position endPos = tok.End();
    EXPECT_EQ(endPos, Position(0, 1, 7));
}

TEST_F(LexerTerminatorTest, identify_terminator)
{
    SourceManager sm2;
    std::vector<std::string> terminator = {"\n", "\r\n"};
    for (size_t i = 0; i < terminator.size(); i++) {
        Lexer lexer = Lexer(terminator[i], diag, sm2);
        Token term = lexer.Next();
        EXPECT_EQ(term.kind, TokenKind::NL);
        Token end = lexer.Next();
        EXPECT_EQ(end.kind, TokenKind::END);
    }

    std::string nonTerminator = "\r";
    Lexer lexer = Lexer(nonTerminator, diag, sm2);
    Token term = lexer.Next();
    EXPECT_EQ(term.kind, TokenKind::ILLEGAL);
    Token end = lexer.Next();
    EXPECT_EQ(end.kind, TokenKind::END);
}
