//===- TokenBufferTokenManager.h  -----------------------------------------===//
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

#ifndef LANGUAGE_CORE_TOOLING_SYNTAX_TOKEN_BUFFER_TOKEN_MANAGER_H
#define LANGUAGE_CORE_TOOLING_SYNTAX_TOKEN_BUFFER_TOKEN_MANAGER_H

#include "language/Core/Tooling/Syntax/TokenManager.h"
#include "language/Core/Tooling/Syntax/Tokens.h"

namespace language::Core {
namespace syntax {

/// A TokenBuffer-powered token manager.
/// It tracks the underlying token buffers, source manager, etc.
class TokenBufferTokenManager : public TokenManager {
public:
  TokenBufferTokenManager(const TokenBuffer &Tokens,
                          const LangOptions &LangOpts, SourceManager &SourceMgr)
      : Tokens(Tokens), LangOpts(LangOpts), SM(SourceMgr) {}

  static bool classof(const TokenManager *N) { return N->kind() == Kind; }
  toolchain::StringLiteral kind() const override { return Kind; }

  toolchain::StringRef getText(Key I) const override {
    const auto *Token = getToken(I);
    assert(Token);
    // Handle 'eof' separately, calling text() on it produces an empty string.
    // FIXME: this special logic is for syntax::Leaf dump, move it when we
    // have a direct way to retrive token kind in the syntax::Leaf.
    if (Token->kind() == tok::eof)
      return "<eof>";
    return Token->text(SM);
  }

  const syntax::Token *getToken(Key I) const {
    return reinterpret_cast<const syntax::Token *>(I);
  }
  SourceManager &sourceManager() { return SM; }
  const SourceManager &sourceManager() const { return SM; }
  const TokenBuffer &tokenBuffer() const { return Tokens; }

private:
  // This manager is powered by the TokenBuffer.
  static constexpr toolchain::StringLiteral Kind = "TokenBuffer";

  /// Add \p Buffer to the underlying source manager, tokenize it and store the
  /// resulting tokens. Used exclusively in `FactoryImpl` to materialize tokens
  /// that were not written in user code.
  std::pair<FileID, ArrayRef<Token>>
  lexBuffer(std::unique_ptr<toolchain::MemoryBuffer> Buffer);
  friend class FactoryImpl;

  const TokenBuffer &Tokens;
  const LangOptions &LangOpts;

  /// The underlying source manager for the ExtraTokens.
  SourceManager &SM;
  /// IDs and storage for additional tokenized files.
  toolchain::DenseMap<FileID, std::vector<Token>> ExtraTokens;
};

} // namespace syntax
} // namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_SYNTAX_TOKEN_BUFFER_TOKEN_MANAGER_H
