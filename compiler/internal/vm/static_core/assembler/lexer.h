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

#ifndef PANDA_ASSEMBLER_LEXER_H
#define PANDA_ASSEMBLER_LEXER_H

#include <array>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "define.h"
#include "error.h"
#include "isa.h"
#include "libarkbase/utils/logger.h"

namespace ark::pandasm {

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct Token {
    enum class Type : std::uint32_t {
        ID_BAD = 0,
        /* delimiters */
        DEL_COMMA,            /* , */
        DEL_COLON,            /* : */
        DEL_BRACE_L,          /* { */
        DEL_BRACE_R,          /* } */
        DEL_BRACKET_L,        /* ( */
        DEL_BRACKET_R,        /* ) */
        DEL_SQUARE_BRACKET_L, /* [ */
        DEL_SQUARE_BRACKET_R, /* ] */
        DEL_GT,               /* > */
        DEL_LT,               /* < */
        DEL_EQ,               /* = */
        DEL_DOT,              /* . */
        ID,                   /* other */
        ID_STRING,            /* string literal */
        OPERATION,            /* special */

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define OPLIST(inst_code, name, optype, width, flags, dst_idx, src_idxs, prof_size) \
    ID_OP_##inst_code, /* command type list */
        PANDA_INSTRUCTION_LIST(OPLIST)
#undef OPLIST
            KEYWORD, /* special */

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define KEYWORDS(name, inst_code) ID_##inst_code, /* keyword type List */
        KEYWORDS_LIST(KEYWORDS)
#undef KEYWORDS
    };

    std::string wholeLine;
    std::uint16_t boundLeft; /* right and left bounds of tokens */
    std::uint16_t boundRight;
    Type type;

    Token() : Token(0U, 0U, Type::ID_BAD, "") {}

    Token(std::uint16_t bL, size_t bR, Type t, std::string begOfLine)
        : wholeLine(std::move(begOfLine)), boundLeft(bL), boundRight(bR), type(t)
    {
    }
};

using Tokens = std::pair<std::vector<Token>, Error>;

using TokenSet = const std::vector<std::vector<Token>>;

struct Line {
    std::vector<Token> tokens;
    std::string buffer; /* Raw line, as read from the file */
    size_t pos {0};     /* current line position */
    size_t end;

    explicit Line(std::string str) : buffer(std::move(str)), end(buffer.size()) {}
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

class Lexer {
public:
    PANDA_PUBLIC_API Lexer();
    PANDA_PUBLIC_API ~Lexer();
    NO_MOVE_SEMANTIC(Lexer);
    NO_COPY_SEMANTIC(Lexer);

    /*
     * The main function of Tokenizing, which takes a string.
     * Returns a vector of tokens.
     */
    PANDA_PUBLIC_API Tokens TokenizeString(const std::string &sourceStr);

private:
    std::vector<Line> lines_;
    Line *currLine_ {nullptr};
    Error err_;

    bool Eol() const; /* End of line */
    bool LexString();
    void LexTokens();
    void LexBadTokens();
    void LexPreprocess();
    void SkipSpace();
    void AnalyzeLine();
    void EatSpace();
    void HandleBrackets();
    bool IsAngleBracketInFunctionName(char c, Line *currLine);
    Token::Type LexGetType(size_t beg, size_t end) const;
};

/*
 * Returns a string representation of a token type.
 */
std::string_view TokenTypeWhat(Token::Type t);

}  // namespace ark::pandasm

#endif  // PANDA_ASSEMBLER_LEXER_H
