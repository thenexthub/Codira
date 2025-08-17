//=== LexHLSLRootSignature.cpp - Lex Root Signature -----------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Lex/LexHLSLRootSignature.h"

namespace language::Core {
namespace hlsl {

using TokenKind = RootSignatureToken::Kind;

// Lexer Definitions

static bool isNumberChar(char C) {
  return isdigit(C)                                      // integer support
         || C == '.'                                     // float support
         || C == 'e' || C == 'E' || C == '-' || C == '+' // exponent support
         || C == 'f' || C == 'F'; // explicit float support
}

RootSignatureToken RootSignatureLexer::lexToken() {
  // Discard any leading whitespace
  advanceBuffer(Buffer.take_while(isspace).size());

  if (isEndOfBuffer())
    return RootSignatureToken(TokenKind::end_of_stream, LocOffset);

  // Record where this token is in the text for usage in parser diagnostics
  RootSignatureToken Result(LocOffset);

  char C = Buffer.front();

  // Punctuators
  switch (C) {
#define PUNCTUATOR(X, Y)                                                       \
  case Y: {                                                                    \
    Result.TokKind = TokenKind::pu_##X;                                        \
    advanceBuffer();                                                           \
    return Result;                                                             \
  }
#include "language/Core/Lex/HLSLRootSignatureTokenKinds.def"
  default:
    break;
  }

  // Number literal
  if (isdigit(C) || C == '.') {
    Result.NumSpelling = Buffer.take_while(isNumberChar);

    // If all values are digits then we have an int literal
    bool IsInteger = Result.NumSpelling.find_if_not(isdigit) == StringRef::npos;

    Result.TokKind =
        IsInteger ? TokenKind::int_literal : TokenKind::float_literal;
    advanceBuffer(Result.NumSpelling.size());
    return Result;
  }

  // All following tokens require at least one additional character
  if (Buffer.size() <= 1) {
    Result = RootSignatureToken(TokenKind::invalid, LocOffset);
    return Result;
  }

  // Peek at the next character to deteremine token type
  char NextC = Buffer[1];

  // Registers: [tsub][0-9+]
  if ((C == 't' || C == 's' || C == 'u' || C == 'b') && isdigit(NextC)) {
    // Convert character to the register type.
    switch (C) {
    case 'b':
      Result.TokKind = TokenKind::bReg;
      break;
    case 't':
      Result.TokKind = TokenKind::tReg;
      break;
    case 'u':
      Result.TokKind = TokenKind::uReg;
      break;
    case 's':
      Result.TokKind = TokenKind::sReg;
      break;
    default:
      toolchain_unreachable("Switch for an expected token was not provided");
    }

    advanceBuffer();

    // Lex the integer literal
    Result.NumSpelling = Buffer.take_while(isNumberChar);
    advanceBuffer(Result.NumSpelling.size());

    return Result;
  }

  // Keywords and Enums:
  StringRef TokSpelling =
      Buffer.take_while([](char C) { return isalnum(C) || C == '_'; });

  // Define a large string switch statement for all the keywords and enums
  auto Switch = toolchain::StringSwitch<TokenKind>(TokSpelling);
#define KEYWORD(NAME) Switch.CaseLower(#NAME, TokenKind::kw_##NAME);
#define ENUM(NAME, LIT) Switch.CaseLower(LIT, TokenKind::en_##NAME);
#include "language/Core/Lex/HLSLRootSignatureTokenKinds.def"

  // Then attempt to retreive a string from it
  Result.TokKind = Switch.Default(TokenKind::invalid);
  advanceBuffer(TokSpelling.size());
  return Result;
}

RootSignatureToken RootSignatureLexer::consumeToken() {
  // If we previously peeked then just return the previous value over
  if (NextToken && NextToken->TokKind != TokenKind::end_of_stream) {
    RootSignatureToken Result = *NextToken;
    NextToken = std::nullopt;
    return Result;
  }
  return lexToken();
}

RootSignatureToken RootSignatureLexer::peekNextToken() {
  // Already peeked from the current token
  if (NextToken)
    return *NextToken;

  NextToken = lexToken();
  return *NextToken;
}

} // namespace hlsl
} // namespace language::Core
