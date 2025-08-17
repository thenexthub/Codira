//===-- AdornedCFG.h ------------------------------------*- C++ -*-===//
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
//  This file defines an AdornedCFG class that is used by dataflow analyses that
//  run over Control-Flow Graphs (CFGs).
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_ADORNEDCFG_H
#define LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_ADORNEDCFG_H

#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/Stmt.h"
#include "language/Core/Analysis/CFG.h"
#include "language/Core/Analysis/FlowSensitive/ASTOps.h"
#include "toolchain/ADT/BitVector.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/Support/Error.h"
#include <memory>
#include <utility>

namespace language::Core {
namespace dataflow {

namespace internal {
class StmtToBlockMap {
public:
  StmtToBlockMap(const CFG &Cfg);

  const CFGBlock *lookup(const Stmt &S) const {
    return StmtToBlock.lookup(&ignoreCFGOmittedNodes(S));
  }

private:
  toolchain::DenseMap<const Stmt *, const CFGBlock *> StmtToBlock;
};
} // namespace internal

/// Holds CFG with additional information derived from it that is needed to
/// perform dataflow analysis.
class AdornedCFG {
public:
  /// Builds an `AdornedCFG` from a `FunctionDecl`.
  /// `Func.doesThisDeclarationHaveABody()` must be true, and
  /// `Func.isTemplated()` must be false.
  static toolchain::Expected<AdornedCFG> build(const FunctionDecl &Func);

  /// Builds an `AdornedCFG` from an AST node. `D` is the function in which
  /// `S` resides. `D.isTemplated()` must be false.
  static toolchain::Expected<AdornedCFG> build(const Decl &D, Stmt &S,
                                          ASTContext &C);

  /// Returns the `Decl` containing the statement used to construct the CFG, if
  /// available.
  const Decl &getDecl() const { return ContainingDecl; }

  /// Returns the CFG that is stored in this context.
  const CFG &getCFG() const { return *Cfg; }

  /// Returns the basic block that contains `S`, or null if no basic block
  /// containing `S` is found.
  const CFGBlock *blockForStmt(const Stmt &S) const {
    return StmtToBlock.lookup(S);
  }

  /// Returns whether `B` is reachable from the entry block.
  bool isBlockReachable(const CFGBlock &B) const {
    return BlockReachable[B.getBlockID()];
  }

  /// Returns whether `B` contains an expression that is consumed in a
  /// different block than `B` (i.e. the parent of the expression is in a
  /// different block).
  /// This happens if there is control flow within a full-expression (triggered
  /// by `&&`, `||`, or the conditional operator). Note that the operands of
  /// these operators are not the only expressions that can be consumed in a
  /// different block. For example, in the function call
  /// `f(&i, cond() ? 1 : 0)`, `&i` is in a different block than the `CallExpr`.
  bool containsExprConsumedInDifferentBlock(const CFGBlock &B) const {
    return ContainsExprConsumedInDifferentBlock.contains(&B);
  }

private:
  AdornedCFG(
      const Decl &D, std::unique_ptr<CFG> Cfg,
      internal::StmtToBlockMap StmtToBlock, toolchain::BitVector BlockReachable,
      toolchain::DenseSet<const CFGBlock *> ContainsExprConsumedInDifferentBlock)
      : ContainingDecl(D), Cfg(std::move(Cfg)),
        StmtToBlock(std::move(StmtToBlock)),
        BlockReachable(std::move(BlockReachable)),
        ContainsExprConsumedInDifferentBlock(
            std::move(ContainsExprConsumedInDifferentBlock)) {}

  /// The `Decl` containing the statement used to construct the CFG.
  const Decl &ContainingDecl;
  std::unique_ptr<CFG> Cfg;
  internal::StmtToBlockMap StmtToBlock;
  toolchain::BitVector BlockReachable;
  toolchain::DenseSet<const CFGBlock *> ContainsExprConsumedInDifferentBlock;
};

} // namespace dataflow
} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_ADORNEDCFG_H
