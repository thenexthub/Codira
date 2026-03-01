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

#include "lexer.h"

namespace ark::pandasm {

/*-------------------------------*/

/* Is this a delimiter ? */
Token::Type FindDelim(char c)
{
    /* The map of delimiters */
    static const std::unordered_map<char, Token::Type> DELIM = {{',', Token::Type::DEL_COMMA},
                                                                {':', Token::Type::DEL_COLON},
                                                                {'{', Token::Type::DEL_BRACE_L},
                                                                {'}', Token::Type::DEL_BRACE_R},
                                                                {'(', Token::Type::DEL_BRACKET_L},
                                                                {')', Token::Type::DEL_BRACKET_R},
                                                                {'<', Token::Type::DEL_LT},
                                                                {'>', Token::Type::DEL_GT},
                                                                {'=', Token::Type::DEL_EQ},
                                                                {'[', Token::Type::DEL_SQUARE_BRACKET_L},
                                                                {']', Token::Type::DEL_SQUARE_BRACKET_R}};

    auto iter = DELIM.find(c);
    if (iter == DELIM.end()) {
        return Token::Type::ID_BAD;
    }

    return DELIM.at(c);
}

Token::Type FindOperation(std::string_view s)
{
    /* Generate the map of OPERATIONS from ISA: */
    static const std::unordered_map<std::string_view, Token::Type> OPERATIONS = {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define OPLIST(inst_code, name, optype, width, flags, dst_idx, use_idxs, prof_size) \
    {std::string_view(name), Token::Type::ID_OP_##inst_code},
        PANDA_INSTRUCTION_LIST(OPLIST)
#undef OPLIST
    };

    auto iter = OPERATIONS.find(s);
    if (iter == OPERATIONS.end()) {
        return Token::Type::ID_BAD;
    }

    return OPERATIONS.at(s);
}

Token::Type Findkeyword(std::string_view s)
{
    /* Generate the map of KEYWORDS: */
    static const std::unordered_map<std::string_view, Token::Type> KEYWORDS = {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define KEYWORDS(name, inst_code) {std::string_view(name), Token::Type::ID_##inst_code},
        KEYWORDS_LIST(KEYWORDS)
#undef KEYWORDS
    };

    auto iter = KEYWORDS.find(s);
    if (iter == KEYWORDS.end()) {
        return Token::Type::ID_BAD;
    }

    return KEYWORDS.at(s);
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) big switch case
std::string_view TokenTypeWhat(Token::Type t)
{
    if (t >= Token::Type::OPERATION && t < Token::Type::KEYWORD) {
        return "OPERATION";
    }

    if (t >= Token::Type::KEYWORD) {
        return "KEYWORD";
    }

    switch (t) {
        case Token::Type::ID_BAD: {
            return "ID_BAD";
        }
        case Token::Type::DEL_COMMA: {
            return "DEL_COMMA";
        }
        case Token::Type::DEL_COLON: {
            return "DEL_COLON";
        }
        case Token::Type::DEL_BRACE_L: {
            return "DEL_BRACE_L";
        }
        case Token::Type::DEL_BRACE_R: {
            return "DEL_BRACE_R";
        }
        case Token::Type::DEL_BRACKET_L: {
            return "DEL_BRACKET_L";
        }
        case Token::Type::DEL_BRACKET_R: {
            return "DEL_BRACKET_R";
        }
        case Token::Type::DEL_SQUARE_BRACKET_L: {
            return "DEL_SQUARE_BRACKET_L";
        }
        case Token::Type::DEL_SQUARE_BRACKET_R: {
            return "DEL_SQUARE_BRACKET_R";
        }
        case Token::Type::DEL_GT: {
            return "DEL_GT";
        }
        case Token::Type::DEL_LT: {
            return "DEL_LT";
        }
        case Token::Type::DEL_EQ: {
            return "DEL_EQ";
        }
        case Token::Type::DEL_DOT: {
            return "DEL_DOT";
        }
        case Token::Type::ID: {
            return "ID";
        }
        case Token::Type::ID_STRING: {
            return "ID_STRING";
        }
        default:
            return "NONE";
    }
}

static bool IsQuote(char c)
{
    return c == '"';
}

Lexer::Lexer()
{
    LOG(DEBUG, ASSEMBLER) << "element of class Lexer initialized";
}

Lexer::~Lexer()
{
    LOG(DEBUG, ASSEMBLER) << "element of class Lexer destructed";
}

Tokens Lexer::TokenizeString(const std::string &sourceStr)
{
    LOG(DEBUG, ASSEMBLER) << "started tokenizing of line " << (lines_.size() + 1) << ": ";

    lines_.emplace_back(sourceStr);

    currLine_ = &lines_.back();

    LOG(DEBUG, ASSEMBLER) << std::string_view(&*(currLine_->buffer.begin() + currLine_->pos),
                                              currLine_->end - currLine_->pos);

    AnalyzeLine();

    LOG(DEBUG, ASSEMBLER) << "tokenization of line " << lines_.size() << " is successful";
    LOG(DEBUG, ASSEMBLER) << "         tokens identified: ";

    for (const auto &fI : lines_.back().tokens) {
        LOG(DEBUG, ASSEMBLER) << "\n                           "
                              << std::string_view(&*(fI.wholeLine.begin() + fI.boundLeft), fI.boundRight - fI.boundLeft)
                              << " (type: " << TokenTypeWhat(fI.type) << ")";

        LOG(DEBUG, ASSEMBLER);
        LOG(DEBUG, ASSEMBLER);
    }
    return std::pair<std::vector<Token>, Error>(lines_.back().tokens, err_);
}

/* End of line? */
bool Lexer::Eol() const
{
    return currLine_->pos == currLine_->end;
}

/* Return the type of token */
Token::Type Lexer::LexGetType(size_t beg, size_t end) const
{
    if (FindDelim(currLine_->buffer[beg]) != Token::Type::ID_BAD) { /* delimiter */
        return FindDelim(currLine_->buffer[beg]);
    }

    std::string_view p(&*(currLine_->buffer.begin() + beg), end - beg);

    Token::Type type = Findkeyword(p);
    if (type != Token::Type::ID_BAD) {
        return type;
    }

    type = FindOperation(p);
    if (type != Token::Type::ID_BAD) {
        return type;
    }

    if (IsQuote(currLine_->buffer[beg])) {
        return Token::Type::ID_STRING;
    }

    return Token::Type::ID; /* other */
}

/* Handle string literal */
bool Lexer::LexString()
{
    bool isEscapeSeq = false;
    char quote = currLine_->buffer[currLine_->pos];
    size_t begin = currLine_->pos;
    while (!Eol()) {
        ++(currLine_->pos);

        char c = currLine_->buffer[currLine_->pos];

        if (isEscapeSeq) {
            isEscapeSeq = false;
            continue;
        }

        if (c == '\\') {
            isEscapeSeq = true;
        }

        if (c == quote) {
            break;
        }
    }

    if (currLine_->buffer[currLine_->pos] != quote) {
        err_ = Error(std::string("Missing terminating ") + quote + " character", 0,
                     Error::ErrorType::ERR_STRING_MISSING_TERMINATING_CHARACTER, "", begin, currLine_->pos,
                     currLine_->buffer);
        return false;
    }

    ++(currLine_->pos);

    return true;
}

bool Lexer::IsAngleBracketInFunctionName(char c, Line *currLine)
{
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr size_t FUNCTION_KEY_WORD_OFFSET = 2;
    size_t currTokenSize = currLine->tokens.size();
    if (currTokenSize < FUNCTION_KEY_WORD_OFFSET) {
        return false;
    }
    bool isManglingName = (FindDelim(c) == Token::Type::DEL_LT || FindDelim(c) == Token::Type::DEL_GT);
    return currLine->tokens[currTokenSize - FUNCTION_KEY_WORD_OFFSET].type == Token::Type::ID_FUN && isManglingName;
}

void Lexer::EatSpace()
{
    while (currLine_->end > currLine_->pos && isspace(currLine_->buffer[currLine_->end - 1]) != 0) {
        --(currLine_->end);
    }

    while (isspace(currLine_->buffer[currLine_->pos]) != 0 && !Eol()) {
        ++(currLine_->pos);
    }
}

void Lexer::HandleBrackets()
{
    size_t position = currLine_->pos;
    while (FindDelim(currLine_->buffer[position]) == Token::Type::DEL_SQUARE_BRACKET_L ||
           FindDelim(currLine_->buffer[position]) == Token::Type::DEL_SQUARE_BRACKET_R) {
        position++;
    }
    if (IsAngleBracketInFunctionName(currLine_->buffer[position], currLine_)) {
        position++;
    }
    if (isspace(currLine_->buffer[position]) == 0 && (position != currLine_->end)) {
        currLine_->pos = position;
    }
}

/*
 * Tokens handling: set a corresponding
 * elements bound_left and bound_right of the array tokens
 * to the first and last characters of a corresponding token.
 *
 *                                                  bound_r1   bound_r2    bound_r3
 *                                                  |          |           |
 *                                                  v          v           v
 *       token1 token2 token3 ...             token1     token2      token3 ...
 *                                       =>   ^          ^           ^
 *                                            |          |           |
 *    bound1    bound2    bound3 ...          bound_l1   bound_l2    bound_l3 ...
 *
 */
void Lexer::LexTokens()
{
    if (Eol()) {
        return;
    }

    LOG(DEBUG, ASSEMBLER) << "token search started (line " << lines_.size() << "): "
                          << std::string_view(&*(currLine_->buffer.begin() + currLine_->pos),
                                              currLine_->end - currLine_->pos);
    EatSpace();
    size_t boundRight;
    size_t boundLeft;

    while (!Eol()) {
        boundLeft = currLine_->pos;

        if (FindDelim(currLine_->buffer[currLine_->pos]) != Token::Type::ID_BAD) {
            ++(currLine_->pos);
        } else if (IsQuote(currLine_->buffer[currLine_->pos])) {
            if (!LexString()) {
                return;
            }
        } else {
            LexBadTokens();
        }

        boundRight = currLine_->pos;

        LOG(DEBUG, ASSEMBLER) << "token identified (line " << lines_.size() << ", "
                              << "token " << currLine_->tokens.size() + 1 << "): "
                              << std::string_view(&*(currLine_->buffer.begin() + boundLeft), boundRight - boundLeft)
                              << " ("
                              << "type: " << TokenTypeWhat(LexGetType(boundLeft, boundRight)) << ")";

        currLine_->tokens.emplace_back(boundLeft, boundRight, LexGetType(boundLeft, boundRight), currLine_->buffer);

        EatSpace();
    }

    LOG(DEBUG, ASSEMBLER) << "all tokens identified (line " << lines_.size() << ")";
}

void Lexer::LexBadTokens()
{
    while (!Eol() && FindDelim(currLine_->buffer[currLine_->pos]) == Token::Type::ID_BAD &&
           isspace(currLine_->buffer[currLine_->pos]) == 0) {
        ++(currLine_->pos);
        HandleBrackets();
    }
}

/*
 * Ignore comments:
 * find PARSE_COMMENT_MARKER and move line->end
 * to another position (next after the last character of the last
 * significant (this is no a comment) element in a current
 * line: line->buffer).
 *
 * Ex:
 *   [Label:] operation operand[,operand] [# comment]
 *
 *   L1: mov v0, v1 # moving!        L1: mov v0, v1 # moving!
 *                          ^   =>                 ^
 *                          |                      |
 *                         end                    end
 */
void Lexer::LexPreprocess()
{
    LOG(DEBUG, ASSEMBLER) << "started removing comments (line " << lines_.size() << "): "
                          << std::string_view(&*(currLine_->buffer.begin() + currLine_->pos),
                                              currLine_->end - currLine_->pos);

    // Searching for comment marker located outside of string literals.
    bool insideStrLit = !currLine_->buffer.empty() && currLine_->buffer[0] == '\"';
    size_t cmtPos = currLine_->buffer.find_first_of("\"#", 0);
    if (cmtPos != std::string::npos) {
        do {
            if (cmtPos != 0 && currLine_->buffer[cmtPos - 1] != '\\' && currLine_->buffer[cmtPos] == '\"') {
                insideStrLit = !insideStrLit;
            } else if (currLine_->buffer[cmtPos] == PARSE_COMMENT_MARKER && !insideStrLit) {
                break;
            }
        } while ((cmtPos = currLine_->buffer.find_first_of("\"#", cmtPos + 1)) != std::string::npos);
    }

    if (cmtPos != std::string::npos) {
        currLine_->end = cmtPos;
    }

    while (currLine_->end > currLine_->pos && isspace(currLine_->buffer[currLine_->end - 1]) != 0) {
        --(currLine_->end);
    }

    LOG(DEBUG, ASSEMBLER) << "comments removed (line " << lines_.size() << "): "
                          << std::string_view(&*(currLine_->buffer.begin() + currLine_->pos),
                                              currLine_->end - currLine_->pos);
}

void Lexer::SkipSpace()
{
    while (!Eol() && isspace(currLine_->buffer[currLine_->pos]) != 0) {
        ++(currLine_->pos);
    }
}

void Lexer::AnalyzeLine()
{
    LexPreprocess();

    SkipSpace();

    LexTokens();
}

/*-------------------------------*/

}  // namespace ark::pandasm
