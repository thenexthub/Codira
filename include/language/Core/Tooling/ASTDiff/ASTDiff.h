//===- ASTDiff.h - AST differencing API -----------------------*- C++ -*- -===//
//
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
// This file specifies an interface that can be used to compare C++ syntax
// trees.
//
// We use the gumtree algorithm which combines a heuristic top-down search that
// is able to match large subtrees that are equivalent, with an optimal
// algorithm to match small subtrees.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_ASTDIFF_ASTDIFF_H
#define LANGUAGE_CORE_TOOLING_ASTDIFF_ASTDIFF_H

#include "language/Core/Tooling/ASTDiff/ASTDiffInternal.h"
#include <optional>

namespace language::Core {
namespace diff {

enum ChangeKind {
  None,
  Delete,    // (Src): delete node Src.
  Update,    // (Src, Dst): update the value of node Src to match Dst.
  Insert,    // (Src, Dst, Pos): insert Src as child of Dst at offset Pos.
  Move,      // (Src, Dst, Pos): move Src to be a child of Dst at offset Pos.
  UpdateMove // Same as Move plus Update.
};

/// Represents a Clang AST node, alongside some additional information.
struct Node {
  NodeId Parent, LeftMostDescendant, RightMostDescendant;
  int Depth, Height, Shift = 0;
  DynTypedNode ASTNode;
  SmallVector<NodeId, 4> Children;
  ChangeKind Change = None;

  ASTNodeKind getType() const;
  StringRef getTypeLabel() const;
  bool isLeaf() const { return Children.empty(); }
  std::optional<StringRef> getIdentifier() const;
  std::optional<std::string> getQualifiedIdentifier() const;
};

/// SyntaxTree objects represent subtrees of the AST.
/// They can be constructed from any Decl or Stmt.
class SyntaxTree {
public:
  /// Constructs a tree from a translation unit.
  SyntaxTree(ASTContext &AST);
  /// Constructs a tree from any AST node.
  template <class T>
  SyntaxTree(T *Node, ASTContext &AST)
      : TreeImpl(std::make_unique<Impl>(this, Node, AST)) {}
  SyntaxTree(SyntaxTree &&Other) = default;
  ~SyntaxTree();

  const ASTContext &getASTContext() const;
  StringRef getFilename() const;

  int getSize() const;
  NodeId getRootId() const;
  using PreorderIterator = NodeId;
  PreorderIterator begin() const;
  PreorderIterator end() const;

  const Node &getNode(NodeId Id) const;
  int findPositionInParent(NodeId Id) const;

  // Returns the starting and ending offset of the node in its source file.
  std::pair<unsigned, unsigned> getSourceRangeOffsets(const Node &N) const;

  /// Serialize the node attributes to a string representation. This should
  /// uniquely distinguish nodes of the same kind. Note that this function just
  /// returns a representation of the node value, not considering descendants.
  std::string getNodeValue(NodeId Id) const;
  std::string getNodeValue(const Node &Node) const;

  class Impl;
  std::unique_ptr<Impl> TreeImpl;
};

struct ComparisonOptions {
  /// During top-down matching, only consider nodes of at least this height.
  int MinHeight = 2;

  /// During bottom-up matching, match only nodes with at least this value as
  /// the ratio of their common descendants.
  double MinSimilarity = 0.5;

  /// Whenever two subtrees are matched in the bottom-up phase, the optimal
  /// mapping is computed, unless the size of either subtrees exceeds this.
  int MaxSize = 100;

  bool StopAfterTopDown = false;

  /// Returns false if the nodes should never be matched.
  bool isMatchingAllowed(const Node &N1, const Node &N2) const {
    return N1.getType().isSame(N2.getType());
  }
};

class ASTDiff {
public:
  ASTDiff(SyntaxTree &Src, SyntaxTree &Dst, const ComparisonOptions &Options);
  ~ASTDiff();

  // Returns the ID of the node that is mapped to the given node in SourceTree.
  NodeId getMapped(const SyntaxTree &SourceTree, NodeId Id) const;

  class Impl;

private:
  std::unique_ptr<Impl> DiffImpl;
};

} // end namespace diff
} // end namespace language::Core

#endif
