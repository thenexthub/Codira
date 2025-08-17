//===--- FormatTokenLexer.h - Format C++ code ----------------*- C++ ----*-===//
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
/// This file contains FormatTokenLexer, which tokenizes a source file
/// into a token stream suitable for ClangFormat.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_FORMAT_FORMATTOKENLEXER_H
#define LANGUAGE_CORE_LIB_FORMAT_FORMATTOKENLEXER_H

#include "Encoding.h"
#include "FormatToken.h"
#include "toolchain/ADT/MapVector.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/StringSet.h"

#include <stack>

namespace language::Core {
namespace format {

enum LexerState {
  NORMAL,
  TEMPLATE_STRING,
  TOKEN_STASHED,
};

class FormatTokenLexer {
public:
  FormatTokenLexer(const SourceManager &SourceMgr, FileID ID, unsigned Column,
                   const FormatStyle &Style, encoding::Encoding Encoding,
                   toolchain::SpecificBumpPtrAllocator<FormatToken> &Allocator,
                   IdentifierTable &IdentTable);

  ArrayRef<FormatToken *> lex();

  const AdditionalKeywords &getKeywords() { return Keywords; }

private:
  void tryMergePreviousTokens();

  bool tryMergeLessLess();
  bool tryMergeGreaterGreater();
  bool tryMergeUserDefinedLiteral();
  bool tryMergeNSStringLiteral();
  bool tryMergeJSPrivateIdentifier();
  bool tryMergeCSharpStringLiteral();
  bool tryMergeCSharpKeywordVariables();
  bool tryMergeNullishCoalescingEqual();
  bool tryTransformCSharpForEach();
  bool tryMergeForEach();
  bool tryTransformTryUsageForC();

  // Merge the most recently lexed tokens into a single token if their kinds are
  // correct.
  bool tryMergeTokens(ArrayRef<tok::TokenKind> Kinds, TokenType NewType);
  // Merge without checking their kinds.
  bool tryMergeTokens(size_t Count, TokenType NewType);
  // Merge if their kinds match any one of Kinds.
  bool tryMergeTokensAny(ArrayRef<ArrayRef<tok::TokenKind>> Kinds,
                         TokenType NewType);

  // Returns \c true if \p Tok can only be followed by an operand in JavaScript.
  bool precedesOperand(FormatToken *Tok);

  bool canPrecedeRegexLiteral(FormatToken *Prev);

  void tryParseJavaTextBlock();

  // Tries to parse a JavaScript Regex literal starting at the current token,
  // if that begins with a slash and is in a location where JavaScript allows
  // regex literals. Changes the current token to a regex literal and updates
  // its text if successful.
  void tryParseJSRegexLiteral();

  // Handles JavaScript template strings.
  //
  // JavaScript template strings use backticks ('`') as delimiters, and allow
  // embedding expressions nested in ${expr-here}. Template strings can be
  // nested recursively, i.e. expressions can contain template strings in turn.
  //
  // The code below parses starting from a backtick, up to a closing backtick or
  // an opening ${. It also maintains a stack of lexing contexts to handle
  // nested template parts by balancing curly braces.
  void handleTemplateStrings();

  void handleCSharpVerbatimAndInterpolatedStrings();

  // Handles TableGen multiline strings. It has the form [{ ... }].
  void handleTableGenMultilineString();
  // Handles TableGen numeric like identifiers.
  // They have a forms of [0-9]*[_a-zA-Z]([_a-zA-Z0-9]*). But limited to the
  // case it is not lexed as an integer.
  void handleTableGenNumericLikeIdentifier();

  void tryParsePythonComment();

  bool tryMerge_TMacro();

  bool tryMergeConflictMarkers();

  void truncateToken(size_t NewLen);

  FormatToken *getStashedToken();

  FormatToken *getNextToken();

  FormatToken *FormatTok;
  bool IsFirstToken;
  std::stack<LexerState> StateStack;
  unsigned Column;
  unsigned TrailingWhitespace;
  std::unique_ptr<Lexer> Lex;
  LangOptions LangOpts;
  const SourceManager &SourceMgr;
  FileID ID;
  const FormatStyle &Style;
  IdentifierTable &IdentTable;
  AdditionalKeywords Keywords;
  encoding::Encoding Encoding;
  toolchain::SpecificBumpPtrAllocator<FormatToken> &Allocator;
  // Index (in 'Tokens') of the last token that starts a new line.
  unsigned FirstInLineIndex;
  SmallVector<FormatToken *, 16> Tokens;

  toolchain::SmallMapVector<IdentifierInfo *, TokenType, 8> Macros;

  toolchain::SmallPtrSet<IdentifierInfo *, 8> MacrosSkippedByRemoveParentheses,
      TemplateNames, TypeNames, VariableTemplates;

  bool FormattingDisabled;
  toolchain::Regex FormatOffRegex; // For one line.

  toolchain::Regex MacroBlockBeginRegex;
  toolchain::Regex MacroBlockEndRegex;

  // Targets that may appear inside a C# attribute.
  static const toolchain::StringSet<> CSharpAttributeTargets;

  /// Handle Verilog-specific tokens.
  bool readRawTokenVerilogSpecific(Token &Tok);

  void readRawToken(FormatToken &Tok);

  void resetLexer(unsigned Offset);
};

} // namespace format
} // namespace language::Core

#endif
