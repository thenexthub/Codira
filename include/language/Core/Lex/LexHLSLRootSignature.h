//===--- LexHLSLRootSignature.h ---------------------------------*- C++ -*-===//
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
//  This file defines the LexHLSLRootSignature interface.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LEX_LEXHLSLROOTSIGNATURE_H
#define LANGUAGE_CORE_LEX_LEXHLSLROOTSIGNATURE_H

#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/SourceLocation.h"

#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/StringSwitch.h"

namespace language::Core {
namespace hlsl {

struct RootSignatureToken {
  enum Kind {
#define TOK(X, SPELLING) X,
#include "language/Core/Lex/HLSLRootSignatureTokenKinds.def"
  };

  Kind TokKind = Kind::invalid;

  // Retain the location offset of the token in the Signature
  // string
  uint32_t LocOffset;

  // Retain spelling of an numeric constant to be parsed later
  StringRef NumSpelling;

  // Constructors
  RootSignatureToken(uint32_t LocOffset) : LocOffset(LocOffset) {}
  RootSignatureToken(Kind TokKind, uint32_t LocOffset)
      : TokKind(TokKind), LocOffset(LocOffset) {}
};

inline const DiagnosticBuilder &
operator<<(const DiagnosticBuilder &DB, const RootSignatureToken::Kind Kind) {
  switch (Kind) {
#define TOK(X, SPELLING)                                                       \
  case RootSignatureToken::Kind::X:                                            \
    DB << SPELLING;                                                            \
    break;
#define PUNCTUATOR(X, SPELLING)                                                \
  case RootSignatureToken::Kind::pu_##X:                                       \
    DB << #SPELLING;                                                           \
    break;
#include "language/Core/Lex/HLSLRootSignatureTokenKinds.def"
  }
  return DB;
}

class RootSignatureLexer {
public:
  RootSignatureLexer(StringRef Signature) : Buffer(Signature) {}

  /// Consumes and returns the next token.
  RootSignatureToken consumeToken();

  /// Returns the token that proceeds CurToken
  RootSignatureToken peekNextToken();

  bool isEndOfBuffer() {
    advanceBuffer(Buffer.take_while(isspace).size());
    return Buffer.empty();
  }

private:
  // Internal buffer state
  StringRef Buffer;
  uint32_t LocOffset = 0;

  // Current peek state
  std::optional<RootSignatureToken> NextToken = std::nullopt;

  /// Consumes the buffer and returns the lexed token.
  RootSignatureToken lexToken();

  /// Advance the buffer by the specified number of characters.
  /// Updates the SourceLocation appropriately.
  void advanceBuffer(unsigned NumCharacters = 1) {
    Buffer = Buffer.drop_front(NumCharacters);
    LocOffset += NumCharacters;
  }
};

} // namespace hlsl
} // namespace language::Core

#endif // LANGUAGE_CORE_LEX_PARSEHLSLROOTSIGNATURE_H
