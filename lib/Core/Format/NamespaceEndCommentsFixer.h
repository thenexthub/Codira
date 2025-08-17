//===--- NamespaceEndCommentsFixer.h ----------------------------*- C++ -*-===//
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
/// This file declares NamespaceEndCommentsFixer, a TokenAnalyzer that
/// fixes namespace end comments.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_FORMAT_NAMESPACEENDCOMMENTSFIXER_H
#define LANGUAGE_CORE_LIB_FORMAT_NAMESPACEENDCOMMENTSFIXER_H

#include "TokenAnalyzer.h"

namespace language::Core {
namespace format {

// Finds the namespace token corresponding to a closing namespace `}`, if that
// is to be formatted.
// If \p Line contains the closing `}` of a namespace, is affected and is not in
// a preprocessor directive, the result will be the matching namespace token.
// Otherwise returns null.
// \p AnnotatedLines is the sequence of lines from which \p Line is a member of.
const FormatToken *
getNamespaceToken(const AnnotatedLine *Line,
                  const SmallVectorImpl<AnnotatedLine *> &AnnotatedLines);

class NamespaceEndCommentsFixer : public TokenAnalyzer {
public:
  NamespaceEndCommentsFixer(const Environment &Env, const FormatStyle &Style);

  std::pair<tooling::Replacements, unsigned>
  analyze(TokenAnnotator &Annotator,
          SmallVectorImpl<AnnotatedLine *> &AnnotatedLines,
          FormatTokenLexer &Tokens) override;
};

} // end namespace format
} // end namespace language::Core

#endif
