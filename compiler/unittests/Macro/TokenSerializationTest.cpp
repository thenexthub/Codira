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

#include <string>
#include "gtest/gtest.h"
#include "Codira/Macro/TokenSerialization.h"
#include "Codira/Lex/Lexer.h"
using namespace Codira;

class TokenSerializationTest : public testing::Test {
protected:
    void SetUp() override
    {
        lexer = new Lexer(code, diag, sm);
    }
    std::string code = R"(
?:
    true false
    main(argc:Int64=1, argv:String) {
    let a:Int64=-40
    let pi:float64=3.14
    let alpha=0x1.1p1
    let c:char = '\''
    // grh
    /*/**/*/
    let d:String = "asdqwe"
    let b = 2 ** -a
    print((a+3*b, (a+3) *b))
    @abc
    };
)";
    Lexer* lexer;
    DiagnosticEngine diag{};
    SourceManager sm;
};

TEST_F(TokenSerializationTest, BufferCase)
{
    std::vector<Token> tokens{};
    for (;;) {
        Token tok = lexer->Next();
        tokens.emplace_back(tok);
        if (tok.kind == TokenKind::END) {
            break;
        }
    }
    std::vector<uint8_t> buf = TokenSerialization::GetTokensBytes(tokens);
    std::vector<Token> backTokens = TokenSerialization::GetTokensFromBytes(buf.data());
    EXPECT_EQ(tokens.size(), 93);
    ASSERT_TRUE(tokens.size() == backTokens.size());
    for (int i = 0; i < 93; ++i) {
        EXPECT_EQ(tokens[i].kind, backTokens[i].kind);
        EXPECT_EQ(tokens[i].Value(), backTokens[i].Value());
        EXPECT_EQ(tokens[i].Begin(), backTokens[i].Begin());
    }
}
