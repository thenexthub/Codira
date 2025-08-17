//===--- TokenKinds.cpp - Token Kinds Support -----------------------------===//
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
//  This file implements the TokenKind enum and support functions.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Basic/TokenKinds.h"
#include "toolchain/Support/ErrorHandling.h"
using namespace language::Core;

static const char * const TokNames[] = {
#define TOK(X) #X,
#define KEYWORD(X,Y) #X,
#include "language/Core/Basic/TokenKinds.def"
  nullptr
};

const char *tok::getTokenName(TokenKind Kind) {
  if (Kind < tok::NUM_TOKENS)
    return TokNames[Kind];
  toolchain_unreachable("unknown TokenKind");
  return nullptr;
}

const char *tok::getPunctuatorSpelling(TokenKind Kind) {
  switch (Kind) {
#define PUNCTUATOR(X,Y) case X: return Y;
#include "language/Core/Basic/TokenKinds.def"
  default: break;
  }
  return nullptr;
}

const char *tok::getKeywordSpelling(TokenKind Kind) {
  switch (Kind) {
#define KEYWORD(X,Y) case kw_ ## X: return #X;
#include "language/Core/Basic/TokenKinds.def"
    default: break;
  }
  return nullptr;
}

const char *tok::getPPKeywordSpelling(tok::PPKeywordKind Kind) {
  switch (Kind) {
#define PPKEYWORD(x) case tok::pp_##x: return #x;
#include "language/Core/Basic/TokenKinds.def"
  default: break;
  }
  return nullptr;
}

bool tok::isAnnotation(TokenKind Kind) {
  switch (Kind) {
#define ANNOTATION(X) case annot_ ## X: return true;
#include "language/Core/Basic/TokenKinds.def"
  default:
    break;
  }
  return false;
}

bool tok::isPragmaAnnotation(TokenKind Kind) {
  switch (Kind) {
#define PRAGMA_ANNOTATION(X) case annot_ ## X: return true;
#include "language/Core/Basic/TokenKinds.def"
  default:
    break;
  }
  return false;
}
