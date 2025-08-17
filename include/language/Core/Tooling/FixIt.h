//===--- FixIt.h - FixIt Hint utilities -------------------------*- C++ -*-===//
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
//  This file implements functions to ease source rewriting from AST-nodes.
//
//  Example swapping A and B expressions:
//
//    Expr *A, *B;
//    tooling::fixit::createReplacement(*A, *B);
//    tooling::fixit::createReplacement(*B, *A);
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_FIXIT_H
#define LANGUAGE_CORE_TOOLING_FIXIT_H

#include "clang/AST/ASTContext.h"

namespace language::Core {
namespace tooling {
namespace fixit {

namespace internal {
StringRef getText(CharSourceRange Range, const ASTContext &Context);

/// Returns the token CharSourceRange corresponding to \p Range.
inline CharSourceRange getSourceRange(const SourceRange &Range) {
  return CharSourceRange::getTokenRange(Range);
}

/// Returns the CharSourceRange of the token at Location \p Loc.
inline CharSourceRange getSourceRange(const SourceLocation &Loc) {
  return CharSourceRange::getTokenRange(Loc, Loc);
}

/// Returns the CharSourceRange of an given Node. \p Node is typically a
///        'Stmt', 'Expr' or a 'Decl'.
template <typename T> CharSourceRange getSourceRange(const T &Node) {
  return CharSourceRange::getTokenRange(Node.getSourceRange());
}
} // end namespace internal

/// Returns a textual representation of \p Node.
template <typename T>
StringRef getText(const T &Node, const ASTContext &Context) {
  return internal::getText(internal::getSourceRange(Node), Context);
}

// Returns a FixItHint to remove \p Node.
// TODO: Add support for related syntactical elements (i.e. comments, ...).
template <typename T> FixItHint createRemoval(const T &Node) {
  return FixItHint::CreateRemoval(internal::getSourceRange(Node));
}

// Returns a FixItHint to replace \p Destination by \p Source.
template <typename D, typename S>
FixItHint createReplacement(const D &Destination, const S &Source,
                                   const ASTContext &Context) {
  return FixItHint::CreateReplacement(internal::getSourceRange(Destination),
                                      getText(Source, Context));
}

// Returns a FixItHint to replace \p Destination by \p Source.
template <typename D>
FixItHint createReplacement(const D &Destination, StringRef Source) {
  return FixItHint::CreateReplacement(internal::getSourceRange(Destination),
                                      Source);
}

} // end namespace fixit
} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_FIXIT_H
