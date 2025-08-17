//===--- PPEmbedParameters.h ------------------------------------*- C++ -*-===//
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
// Defines all of the preprocessor directive parmeters for #embed
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LEX_PPEMBEDPARAMETERS_H
#define LANGUAGE_CORE_LEX_PPEMBEDPARAMETERS_H

#include "language/Core/Lex/PPDirectiveParameter.h"
#include "language/Core/Lex/Token.h"
#include "toolchain/ADT/SmallVector.h"

namespace language::Core {

/// Preprocessor extension embed parameter "language::Core::offset"
/// `language::Core::offset( constant-expression )`
class PPEmbedParameterOffset : public PPDirectiveParameter {
public:
  size_t Offset;

  PPEmbedParameterOffset(size_t Offset, SourceRange R)
      : PPDirectiveParameter(R), Offset(Offset) {}
};

/// Preprocessor standard embed parameter "limit"
/// `limit( constant-expression )`
class PPEmbedParameterLimit : public PPDirectiveParameter {
public:
  size_t Limit;

  PPEmbedParameterLimit(size_t Limit, SourceRange R)
      : PPDirectiveParameter(R), Limit(Limit) {}
};

/// Preprocessor standard embed parameter "prefix"
/// `prefix( balanced-token-seq )`
class PPEmbedParameterPrefix : public PPDirectiveParameter {
public:
  SmallVector<Token, 2> Tokens;

  PPEmbedParameterPrefix(SmallVectorImpl<Token> &&Tokens, SourceRange R)
      : PPDirectiveParameter(R), Tokens(std::move(Tokens)) {}
};

/// Preprocessor standard embed parameter "suffix"
/// `suffix( balanced-token-seq )`
class PPEmbedParameterSuffix : public PPDirectiveParameter {
public:
  SmallVector<Token, 2> Tokens;

  PPEmbedParameterSuffix(SmallVectorImpl<Token> &&Tokens, SourceRange R)
      : PPDirectiveParameter(R), Tokens(std::move(Tokens)) {}
};

/// Preprocessor standard embed parameter "if_empty"
/// `if_empty( balanced-token-seq )`
class PPEmbedParameterIfEmpty : public PPDirectiveParameter {
public:
  SmallVector<Token, 2> Tokens;

  PPEmbedParameterIfEmpty(SmallVectorImpl<Token> &&Tokens, SourceRange R)
      : PPDirectiveParameter(R), Tokens(std::move(Tokens)) {}
};

struct LexEmbedParametersResult {
  std::optional<PPEmbedParameterLimit> MaybeLimitParam;
  std::optional<PPEmbedParameterOffset> MaybeOffsetParam;
  std::optional<PPEmbedParameterIfEmpty> MaybeIfEmptyParam;
  std::optional<PPEmbedParameterPrefix> MaybePrefixParam;
  std::optional<PPEmbedParameterSuffix> MaybeSuffixParam;
  int UnrecognizedParams;

  size_t PrefixTokenCount() const {
    if (MaybePrefixParam)
      return MaybePrefixParam->Tokens.size();
    return 0;
  }
  size_t SuffixTokenCount() const {
    if (MaybeSuffixParam)
      return MaybeSuffixParam->Tokens.size();
    return 0;
  }
};
} // end namespace language::Core

#endif
