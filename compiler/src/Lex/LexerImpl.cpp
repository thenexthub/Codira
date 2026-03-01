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

/**
 * @file
 *
 * This file implements class LexerImpl.
 */

#include "LexerImpl.h"

namespace Codira {
Token Lexer::Next()
{
    return impl->Next();
}

const std::list<Token>& Lexer::LookAhead(size_t num)
{
    return impl->LookAhead(num);
}
bool Lexer::Seeing(const std::vector<TokenKind>::const_iterator& begin,
    const std::vector<TokenKind>::const_iterator& end, bool skipNewline, bool skipComments)
{
    return impl->Seeing(begin, end, skipNewline, skipComments);
}
bool Lexer::Seeing(const std::vector<TokenKind>& kinds, bool skipNewline, bool skipComments)
{
    return impl->Seeing(kinds, skipNewline, skipComments);
}

std::list<Token> Lexer::LookAheadSkipNL(size_t num)
{
    return impl->LookAheadSkipNL(num);
}
std::vector<Token> Lexer::GetComments()
{
    return std::move(impl->comments);
}
const std::vector<Token>& Lexer::GetTokenStream() const
{
    return impl->tokenStream;
}

void Lexer::SetResetPoint()
{
    impl->SetResetPoint();
}
void Lexer::Reset()
{
    impl->Reset();
}
const Position& Lexer::GetPosBase() const
{
    return impl->posBase;
}

unsigned Lexer::GetFileID() const
{
    return impl->fileID;
}

void Lexer::ClearStringParts(const Token& t)
{
    impl->stringPartsMap.erase(t);
}

void Lexer::SetEHEnabled(bool enabled) const
{
    static const auto EH_KEYWORDS = {
        TokenKind::PERFORM,
        TokenKind::RESUME,
        TokenKind::THROWING,
        TokenKind::HANDLE
    };
    for (auto keyword : EH_KEYWORDS) {
        const auto tokenStr = TOKENS[static_cast<size_t>(keyword)];
        if (enabled) {
            impl->tokenMap[tokenStr] = keyword;
        } else {
            impl->tokenMap.erase(tokenStr);
        }
    }
}

const std::vector<StringPart>& Lexer::GetStrParts(const Token& t)
{
    return impl->GetStrParts(t);
}
std::vector<Token> Lexer::GetTokens()
{
    return impl->GetTokens();
}
std::vector<Token> Lexer::GetCollectTokens() const
{
    return impl->GetCollectTokens();
}
bool Lexer::StartCollectTokens()
{
    return impl->StartCollectTokens();
}
void Lexer::StopCollectTokens(bool bStart)
{
    impl->StopCollectTokens(bStart);
}
std::size_t Lexer::GetCurrentToken() const
{
    return impl->GetCurrentToken();
}
} // namespace Codira
