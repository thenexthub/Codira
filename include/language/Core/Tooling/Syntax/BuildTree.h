//===- BuildTree.h - build syntax trees -----------------------*- C++ -*-=====//
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
// Functions to construct a syntax tree from an AST.
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_TOOLING_SYNTAX_BUILDTREE_H
#define LANGUAGE_CORE_TOOLING_SYNTAX_BUILDTREE_H

#include "language/Core/AST/Decl.h"
#include "language/Core/Basic/TokenKinds.h"
#include "language/Core/Tooling/Syntax/Nodes.h"
#include "language/Core/Tooling/Syntax/TokenBufferTokenManager.h"
#include "language/Core/Tooling/Syntax/Tree.h"

namespace language::Core {
namespace syntax {

/// Build a syntax tree for the main file.
/// This usually covers the whole TranslationUnitDecl, but can be restricted by
/// the ASTContext's traversal scope.
syntax::TranslationUnit *
buildSyntaxTree(Arena &A, TokenBufferTokenManager &TBTM, ASTContext &Context);

// Create syntax trees from subtrees not backed by the source code.

// Synthesis of Leafs
/// Create `Leaf` from token with `Spelling` and assert it has the desired
/// `TokenKind`.
syntax::Leaf *createLeaf(syntax::Arena &A, TokenBufferTokenManager &TBTM,
                         tok::TokenKind K, StringRef Spelling);

/// Infer the token spelling from its `TokenKind`, then create `Leaf` from
/// this token
syntax::Leaf *createLeaf(syntax::Arena &A, TokenBufferTokenManager &TBTM,
                         tok::TokenKind K);

// Synthesis of Trees
/// Creates the concrete syntax node according to the specified `NodeKind` `K`.
/// Returns it as a pointer to the base class `Tree`.
syntax::Tree *
createTree(syntax::Arena &A,
           ArrayRef<std::pair<syntax::Node *, syntax::NodeRole>> Children,
           syntax::NodeKind K);

// Synthesis of Syntax Nodes
syntax::EmptyStatement *createEmptyStatement(syntax::Arena &A,
                                             TokenBufferTokenManager &TBTM);

/// Creates a completely independent copy of `N` with its macros expanded.
///
/// The copy is:
/// * Detached, i.e. `Parent == NextSibling == nullptr` and
/// `Role == Detached`.
/// * Synthesized, i.e. `Original == false`.
syntax::Node *deepCopyExpandingMacros(syntax::Arena &A,
                                      TokenBufferTokenManager &TBTM,
                                      const syntax::Node *N);
} // namespace syntax
} // namespace language::Core
#endif
