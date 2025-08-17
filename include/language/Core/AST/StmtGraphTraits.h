//===- StmtGraphTraits.h - Graph Traits for the class Stmt ------*- C++ -*-===//
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
//  This file defines a template specialization of toolchain::GraphTraits to
//  treat ASTs (Stmt*) as graphs
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_AST_STMTGRAPHTRAITS_H
#define LANGUAGE_CORE_AST_STMTGRAPHTRAITS_H

#include "language/Core/AST/Stmt.h"
#include "toolchain/ADT/DepthFirstIterator.h"
#include "toolchain/ADT/GraphTraits.h"

namespace toolchain {

template <> struct GraphTraits<language::Core::Stmt *> {
  using NodeRef = language::Core::Stmt *;
  using ChildIteratorType = language::Core::Stmt::child_iterator;
  using nodes_iterator = toolchain::df_iterator<language::Core::Stmt *>;

  static NodeRef getEntryNode(language::Core::Stmt *S) { return S; }

  static ChildIteratorType child_begin(NodeRef N) {
    if (N) return N->child_begin();
    else return ChildIteratorType();
  }

  static ChildIteratorType child_end(NodeRef N) {
    if (N) return N->child_end();
    else return ChildIteratorType();
  }

  static nodes_iterator nodes_begin(language::Core::Stmt* S) {
    return df_begin(S);
  }

  static nodes_iterator nodes_end(language::Core::Stmt* S) {
    return df_end(S);
  }
};

template <> struct GraphTraits<const language::Core::Stmt *> {
  using NodeRef = const language::Core::Stmt *;
  using ChildIteratorType = language::Core::Stmt::const_child_iterator;
  using nodes_iterator = toolchain::df_iterator<const language::Core::Stmt *>;

  static NodeRef getEntryNode(const language::Core::Stmt *S) { return S; }

  static ChildIteratorType child_begin(NodeRef N) {
    if (N) return N->child_begin();
    else return ChildIteratorType();
  }

  static ChildIteratorType child_end(NodeRef N) {
    if (N) return N->child_end();
    else return ChildIteratorType();
  }

  static nodes_iterator nodes_begin(const language::Core::Stmt* S) {
    return df_begin(S);
  }

  static nodes_iterator nodes_end(const language::Core::Stmt* S) {
    return df_end(S);
  }
};

} // namespace toolchain

#endif // LANGUAGE_CORE_AST_STMTGRAPHTRAITS_H
