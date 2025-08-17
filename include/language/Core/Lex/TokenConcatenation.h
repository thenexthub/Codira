//===--- TokenConcatenation.h - Token Concatenation Avoidance ---*- C++ -*-===//
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
// This file defines the TokenConcatenation class.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LEX_TOKENCONCATENATION_H
#define LANGUAGE_CORE_LEX_TOKENCONCATENATION_H

#include "language/Core/Basic/TokenKinds.h"

namespace language::Core {
  class Preprocessor;
  class Token;

  /// TokenConcatenation class, which answers the question of
  ///   "Is it safe to emit two tokens without a whitespace between them, or
  ///    would that cause implicit concatenation of the tokens?"
  ///
  /// For example, it emitting two identifiers "foo" and "bar" next to each
  /// other would cause the lexer to produce one "foobar" token.  Emitting "1"
  /// and ")" next to each other is safe.
  ///
  class TokenConcatenation {
    const Preprocessor &PP;

    enum AvoidConcatInfo {
      /// By default, a token never needs to avoid concatenation.  Most tokens
      /// (e.g. ',', ')', etc) don't cause a problem when concatenated.
      aci_never_avoid_concat = 0,

      /// aci_custom_firstchar - AvoidConcat contains custom code to handle this
      /// token's requirements, and it needs to know the first character of the
      /// token.
      aci_custom_firstchar = 1,

      /// aci_custom - AvoidConcat contains custom code to handle this token's
      /// requirements, but it doesn't need to know the first character of the
      /// token.
      aci_custom = 2,

      /// aci_avoid_equal - Many tokens cannot be safely followed by an '='
      /// character.  For example, "<<" turns into "<<=" when followed by an =.
      aci_avoid_equal = 4
    };

    /// TokenInfo - This array contains information for each token on what
    /// action to take when avoiding concatenation of tokens in the AvoidConcat
    /// method.
    char TokenInfo[tok::NUM_TOKENS];
  public:
    TokenConcatenation(const Preprocessor &PP);

    bool AvoidConcat(const Token &PrevPrevTok,
                     const Token &PrevTok,
                     const Token &Tok) const;

  private:
    /// IsIdentifierStringPrefix - Return true if the spelling of the token
    /// is literally 'L', 'u', 'U', or 'u8'.
    bool IsIdentifierStringPrefix(const Token &Tok) const;
  };
  } // end clang namespace

#endif
