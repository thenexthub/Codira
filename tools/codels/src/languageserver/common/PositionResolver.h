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

#ifndef LSPSERVER_POSITIONRESOLVER_H
#define LSPSERVER_POSITIONRESOLVER_H

#include "Codira/AST/Node.h"
#include "../../json-rpc/Common.h"
#include "../../json-rpc/Protocol.h"
#include "../ArkAST.h"
#include "Codira/Basic/Display.h"

namespace ark {
enum class UTF8Kind {
    BYTE_ONE = 0,    // 0xxxxxxx
    BYTE_TWO = 1,    // 110xxxxx 10xxxxxx
    BYTE_THREE = 2,  // 1110xxxx 10xxxxxx 10xxxxxx
    BYTE_FOUR = 3    // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
};

bool IsUTF8(const std::string &str);

std::basic_string<char32_t> UTF8ToChar32(const std::string &str);

std::string Char32ToUTF8(const std::basic_string<char32_t>& str);

int GetFirstTokenOnCurLine(const std::vector<Codira::Token> &tokens, int declLine);

int GetLastTokenOnCurLine(const std::vector<Codira::Token> &tokens, int declLine);

Codira::Position TransformFromChar2IDE(Codira::Position pos);

Codira::Position PosFromIDE2Char(Codira::Position pos);

Range TransformFromChar2IDE(Range range);

Range TransformFromIDE2Char(Range range);

bool PositionInCurToken(int line, int column, const Codira::Token &token);

int LineOfCommentEnd(const Codira::Token &token, std::string &lastString);

void InterpStringInMultiString(const std::vector<Codira::Token> &tokens, Codira::Position &pos,
                               const Codira::AST::Node &node, bool isIDEToUTF8);

bool IsMultiLine(const Codira::Token &token);

int RedundantCharacterOfMultiLineString(const std::vector<Codira::Token> &tokens, const Codira::Position &pos,
                                        int preBegin);

void PositionIDEToUTF8(const std::vector<Codira::Token> &tokens, Codira::Position &pos,
                       const Codira::AST::Node &node);

void PositionIDEToUTF8ForC(const ArkAST &input, Codira::Position &pos);

void PositionUTF8ToIDE(const std::vector<Codira::Token> &tokens, Codira::Position &pos,
                       const Codira::AST::Node &node);

int CountUnicodeCharacters(const std::string& utf8Str);

void UpdateRange(const std::vector<Codira::Token> &tokens, Range &range, const Codira::AST::Node &node,
                 bool needUpdateByName = true);
} // namespace ark

#endif // LSPSERVER_POSITIONRESOLVER_H
