//===--- QualifierAlignmentFixer.h -------------------------------*- C++-*-===//
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
/// This file declares QualifierAlignmentFixer, a TokenAnalyzer that
/// enforces either east or west const depending on the style.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_FORMAT_QUALIFIERALIGNMENTFIXER_H
#define LANGUAGE_CORE_LIB_FORMAT_QUALIFIERALIGNMENTFIXER_H

#include "TokenAnalyzer.h"

namespace language::Core {
namespace format {

typedef std::function<std::pair<tooling::Replacements, unsigned>(
    const Environment &)>
    AnalyzerPass;

void addQualifierAlignmentFixerPasses(const FormatStyle &Style,
                                      SmallVectorImpl<AnalyzerPass> &Passes);

void prepareLeftRightOrderingForQualifierAlignmentFixer(
    const std::vector<std::string> &Order, std::vector<std::string> &LeftOrder,
    std::vector<std::string> &RightOrder,
    std::vector<tok::TokenKind> &Qualifiers);

// Is the Token a simple or qualifier type
bool isQualifierOrType(const FormatToken *Tok, const LangOptions &LangOpts);
bool isConfiguredQualifierOrType(const FormatToken *Tok,
                                 const std::vector<tok::TokenKind> &Qualifiers,
                                 const LangOptions &LangOpts);

// Is the Token likely a Macro
bool isPossibleMacro(const FormatToken *Tok);

class LeftRightQualifierAlignmentFixer : public TokenAnalyzer {
  std::string Qualifier;
  bool RightAlign;
  SmallVector<tok::TokenKind, 8> QualifierTokens;
  std::vector<tok::TokenKind> ConfiguredQualifierTokens;

public:
  LeftRightQualifierAlignmentFixer(
      const Environment &Env, const FormatStyle &Style,
      const std::string &Qualifier,
      const std::vector<tok::TokenKind> &ConfiguredQualifierTokens,
      bool RightAlign);

  std::pair<tooling::Replacements, unsigned>
  analyze(TokenAnnotator &Annotator,
          SmallVectorImpl<AnnotatedLine *> &AnnotatedLines,
          FormatTokenLexer &Tokens) override;

  static tok::TokenKind getTokenFromQualifier(const std::string &Qualifier);

  void fixQualifierAlignment(SmallVectorImpl<AnnotatedLine *> &AnnotatedLines,
                             FormatTokenLexer &Tokens,
                             tooling::Replacements &Fixes);

  const FormatToken *analyzeRight(const SourceManager &SourceMgr,
                                  const AdditionalKeywords &Keywords,
                                  tooling::Replacements &Fixes,
                                  const FormatToken *Tok,
                                  const std::string &Qualifier,
                                  tok::TokenKind QualifierType);

  const FormatToken *analyzeLeft(const SourceManager &SourceMgr,
                                 const AdditionalKeywords &Keywords,
                                 tooling::Replacements &Fixes,
                                 const FormatToken *Tok,
                                 const std::string &Qualifier,
                                 tok::TokenKind QualifierType);
};

} // end namespace format
} // end namespace language::Core

#endif
