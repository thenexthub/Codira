//===--- ASTSelection.h - Clang refactoring library -----------------------===//
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

#ifndef LANGUAGE_CORE_TOOLING_REFACTORING_ASTSELECTION_H
#define LANGUAGE_CORE_TOOLING_REFACTORING_ASTSELECTION_H

#include "language/Core/AST/ASTTypeTraits.h"
#include "language/Core/AST/Stmt.h"
#include "language/Core/Basic/LLVM.h"
#include "language/Core/Basic/SourceLocation.h"
#include "toolchain/Support/raw_ostream.h"
#include <optional>
#include <vector>

namespace language::Core {

class ASTContext;

namespace tooling {

enum class SourceSelectionKind {
  /// A node that's not selected.
  None,

  /// A node that's considered to be selected because the whole selection range
  /// is inside of its source range.
  ContainsSelection,
  /// A node that's considered to be selected because the start of the selection
  /// range is inside its source range.
  ContainsSelectionStart,
  /// A node that's considered to be selected because the end of the selection
  /// range is inside its source range.
  ContainsSelectionEnd,

  /// A node that's considered to be selected because the node is entirely in
  /// the selection range.
  InsideSelection,
};

/// Represents a selected AST node.
///
/// AST selection is represented using a tree of \c SelectedASTNode. The tree
/// follows the top-down shape of the actual AST. Each selected node has
/// a selection kind. The kind might be none as the node itself might not
/// actually be selected, e.g. a statement in macro whose child is in a macro
/// argument.
struct SelectedASTNode {
  DynTypedNode Node;
  SourceSelectionKind SelectionKind;
  std::vector<SelectedASTNode> Children;

  SelectedASTNode(const DynTypedNode &Node, SourceSelectionKind SelectionKind)
      : Node(Node), SelectionKind(SelectionKind) {}
  SelectedASTNode(SelectedASTNode &&) = default;
  SelectedASTNode &operator=(SelectedASTNode &&) = default;

  void dump(toolchain::raw_ostream &OS = toolchain::errs()) const;

  using ReferenceType = std::reference_wrapper<const SelectedASTNode>;
};

/// Traverses the given ASTContext and creates a tree of selected AST nodes.
///
/// \returns std::nullopt if no nodes are selected in the AST, or a selected AST
/// node that corresponds to the TranslationUnitDecl otherwise.
std::optional<SelectedASTNode> findSelectedASTNodes(const ASTContext &Context,
                                                    SourceRange SelectionRange);

/// An AST selection value that corresponds to a selection of a set of
/// statements that belong to one body of code (like one function).
///
/// For example, the following selection in the source.
///
/// \code
/// void function() {
///  // selection begin:
///  int x = 0;
///  {
///     // selection end
///     x = 1;
///  }
///  x = 2;
/// }
/// \endcode
///
/// Would correspond to a code range selection of statements "int x = 0"
/// and the entire compound statement that follows it.
///
/// A \c CodeRangeASTSelection value stores references to the full
/// \c SelectedASTNode tree and should not outlive it.
class CodeRangeASTSelection {
public:
  CodeRangeASTSelection(CodeRangeASTSelection &&) = default;
  CodeRangeASTSelection &operator=(CodeRangeASTSelection &&) = default;

  /// Returns the parent hierarchy (top to bottom) for the selected nodes.
  ArrayRef<SelectedASTNode::ReferenceType> getParents() { return Parents; }

  /// Returns the number of selected statements.
  size_t size() const {
    if (!AreChildrenSelected)
      return 1;
    return SelectedNode.get().Children.size();
  }

  const Stmt *operator[](size_t I) const {
    if (!AreChildrenSelected) {
      assert(I == 0 && "Invalid index");
      return SelectedNode.get().Node.get<Stmt>();
    }
    return SelectedNode.get().Children[I].Node.get<Stmt>();
  }

  /// Returns true when a selected code range is in a function-like body
  /// of code, like a function, method or a block.
  ///
  /// This function can be used to test against selected expressions that are
  /// located outside of a function, e.g. global variable initializers, default
  /// argument values, or even template arguments.
  ///
  /// Use the \c getFunctionLikeNearestParent to get the function-like parent
  /// declaration.
  bool isInFunctionLikeBodyOfCode() const;

  /// Returns the nearest function-like parent declaration or null if such
  /// declaration doesn't exist.
  const Decl *getFunctionLikeNearestParent() const;

  static std::optional<CodeRangeASTSelection>
  create(SourceRange SelectionRange, const SelectedASTNode &ASTSelection);

private:
  CodeRangeASTSelection(SelectedASTNode::ReferenceType SelectedNode,
                        ArrayRef<SelectedASTNode::ReferenceType> Parents,
                        bool AreChildrenSelected)
      : SelectedNode(SelectedNode), Parents(Parents),
        AreChildrenSelected(AreChildrenSelected) {}

  /// The reference to the selected node (or reference to the selected
  /// child nodes).
  SelectedASTNode::ReferenceType SelectedNode;
  /// The parent hierarchy (top to bottom) for the selected noe.
  toolchain::SmallVector<SelectedASTNode::ReferenceType, 8> Parents;
  /// True only when the children of the selected node are actually selected.
  bool AreChildrenSelected;
};

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_REFACTORING_ASTSELECTION_H
