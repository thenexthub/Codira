//===- TokenRewriter.cpp - Token-based code rewriting interface -----------===//
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
//
//  This file implements the TokenRewriter class, which is used for code
//  transformations.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Rewrite/Core/TokenRewriter.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Lex/Lexer.h"
#include "language/Core/Lex/ScratchBuffer.h"
#include "language/Core/Lex/Token.h"
#include <cassert>
#include <cstring>
#include <map>
#include <utility>

using namespace language::Core;

TokenRewriter::TokenRewriter(FileID FID, SourceManager &SM,
                             const LangOptions &LangOpts) {
  ScratchBuf.reset(new ScratchBuffer(SM));

  // Create a lexer to lex all the tokens of the main file in raw mode.
  toolchain::MemoryBufferRef FromFile = SM.getBufferOrFake(FID);
  Lexer RawLex(FID, FromFile, SM, LangOpts);

  // Return all comments and whitespace as tokens.
  RawLex.SetKeepWhitespaceMode(true);

  // Lex the file, populating our datastructures.
  Token RawTok;
  RawLex.LexFromRawLexer(RawTok);
  while (RawTok.isNot(tok::eof)) {
#if 0
    if (Tok.is(tok::raw_identifier)) {
      // Look up the identifier info for the token.  This should use
      // IdentifierTable directly instead of PP.
      PP.LookUpIdentifierInfo(Tok);
    }
#endif

    AddToken(RawTok, TokenList.end());
    RawLex.LexFromRawLexer(RawTok);
  }
}

TokenRewriter::~TokenRewriter() = default;

/// RemapIterator - Convert from token_iterator (a const iterator) to
/// TokenRefTy (a non-const iterator).
TokenRewriter::TokenRefTy TokenRewriter::RemapIterator(token_iterator I) {
  if (I == token_end()) return TokenList.end();

  // FIXME: This is horrible, we should use our own list or something to avoid
  // this.
  std::map<SourceLocation, TokenRefTy>::iterator MapIt =
    TokenAtLoc.find(I->getLocation());
  assert(MapIt != TokenAtLoc.end() && "iterator not in rewriter?");
  return MapIt->second;
}

/// AddToken - Add the specified token into the Rewriter before the other
/// position.
TokenRewriter::TokenRefTy
TokenRewriter::AddToken(const Token &T, TokenRefTy Where) {
  Where = TokenList.insert(Where, T);

  bool InsertSuccess = TokenAtLoc.insert(std::make_pair(T.getLocation(),
                                                        Where)).second;
  assert(InsertSuccess && "Token location already in rewriter!");
  (void)InsertSuccess;
  return Where;
}

TokenRewriter::token_iterator
TokenRewriter::AddTokenBefore(token_iterator I, const char *Val) {
  unsigned Len = strlen(Val);

  // Plop the string into the scratch buffer, then create a token for this
  // string.
  Token Tok;
  Tok.startToken();
  const char *Spelling;
  Tok.setLocation(ScratchBuf->getToken(Val, Len, Spelling));
  Tok.setLength(Len);

  // TODO: Form a whole lexer around this and relex the token!  For now, just
  // set kind to tok::unknown.
  Tok.setKind(tok::unknown);

  return AddToken(Tok, RemapIterator(I));
}
