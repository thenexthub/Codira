//===--- TokenKinds.h - Enum values for C Token Kinds -----------*- C++ -*-===//
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
///
/// \file
/// Defines the language::Core::TokenKind enum and support functions.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_TOKENKINDS_H
#define LANGUAGE_CORE_BASIC_TOKENKINDS_H

#include "toolchain/ADT/DenseMapInfo.h"
#include "toolchain/Support/Compiler.h"

namespace language::Core {

namespace tok {

/// Provides a simple uniform namespace for tokens from all C languages.
enum TokenKind : unsigned short {
#define TOK(X) X,
#include "language/Core/Basic/TokenKinds.def"
  NUM_TOKENS
};

/// Provides a namespace for preprocessor keywords which start with a
/// '#' at the beginning of the line.
enum PPKeywordKind {
#define PPKEYWORD(X) pp_##X,
#include "language/Core/Basic/TokenKinds.def"
  NUM_PP_KEYWORDS
};

/// Provides a namespace for Objective-C keywords which start with
/// an '@'.
enum ObjCKeywordKind {
#define OBJC_AT_KEYWORD(X) objc_##X,
#include "language/Core/Basic/TokenKinds.def"
  NUM_OBJC_KEYWORDS
};

/// Provides a namespace for notable identifers such as float_t and
/// double_t.
enum NotableIdentifierKind {
#define NOTABLE_IDENTIFIER(X) X,
#include "language/Core/Basic/TokenKinds.def"
  NUM_NOTABLE_IDENTIFIERS
};

/// Defines the possible values of an on-off-switch (C99 6.10.6p2).
enum OnOffSwitch {
  OOS_ON, OOS_OFF, OOS_DEFAULT
};

/// Determines the name of a token as used within the front end.
///
/// The name of a token will be an internal name (such as "l_square")
/// and should not be used as part of diagnostic messages.
const char *getTokenName(TokenKind Kind) LLVM_READNONE;

/// Determines the spelling of simple punctuation tokens like
/// '!' or '%', and returns NULL for literal and annotation tokens.
///
/// This routine only retrieves the "simple" spelling of the token,
/// and will not produce any alternative spellings (e.g., a
/// digraph). For the actual spelling of a given Token, use
/// Preprocessor::getSpelling().
const char *getPunctuatorSpelling(TokenKind Kind) LLVM_READNONE;

/// Determines the spelling of simple keyword and contextual keyword
/// tokens like 'int' and 'dynamic_cast'. Returns NULL for other token kinds.
const char *getKeywordSpelling(TokenKind Kind) LLVM_READNONE;

/// Returns the spelling of preprocessor keywords, such as "else".
const char *getPPKeywordSpelling(PPKeywordKind Kind) LLVM_READNONE;

/// Return true if this is a raw identifier or an identifier kind.
inline bool isAnyIdentifier(TokenKind K) {
  return (K == tok::identifier) || (K == tok::raw_identifier);
}

/// Return true if this is a C or C++ string-literal (or
/// C++11 user-defined-string-literal) token.
inline bool isStringLiteral(TokenKind K) {
  return K == tok::string_literal || K == tok::wide_string_literal ||
         K == tok::utf8_string_literal || K == tok::utf16_string_literal ||
         K == tok::utf32_string_literal;
}

/// Return true if this is a "literal" kind, like a numeric
/// constant, string, etc.
inline bool isLiteral(TokenKind K) {
  const bool isInLiteralRange =
      K >= tok::numeric_constant && K <= tok::utf32_string_literal;

#if !NDEBUG
  const bool isLiteralExplicit =
      K == tok::numeric_constant || K == tok::char_constant ||
      K == tok::wide_char_constant || K == tok::utf8_char_constant ||
      K == tok::utf16_char_constant || K == tok::utf32_char_constant ||
      isStringLiteral(K) || K == tok::header_name || K == tok::binary_data;
  assert(isInLiteralRange == isLiteralExplicit &&
         "TokenKind literals should be contiguous");
#endif

  return isInLiteralRange;
}

/// Return true if this is any of tok::annot_* kinds.
bool isAnnotation(TokenKind K);

/// Return true if this is an annotation token representing a pragma.
bool isPragmaAnnotation(TokenKind K);

inline constexpr bool isRegularKeywordAttribute(TokenKind K) {
  return (false
#define KEYWORD_ATTRIBUTE(X, ...) || (K == tok::kw_##X)
#include "language/Core/Basic/RegularKeywordAttrInfo.inc"
  );
}

} // end namespace tok
} // end namespace language::Core

namespace toolchain {
template <> struct DenseMapInfo<language::Core::tok::PPKeywordKind> {
  static inline language::Core::tok::PPKeywordKind getEmptyKey() {
    return language::Core::tok::PPKeywordKind::pp_not_keyword;
  }
  static inline language::Core::tok::PPKeywordKind getTombstoneKey() {
    return language::Core::tok::PPKeywordKind::NUM_PP_KEYWORDS;
  }
  static unsigned getHashValue(const language::Core::tok::PPKeywordKind &Val) {
    return static_cast<unsigned>(Val);
  }
  static bool isEqual(const language::Core::tok::PPKeywordKind &LHS,
                      const language::Core::tok::PPKeywordKind &RHS) {
    return LHS == RHS;
  }
};
} // namespace toolchain

#endif
