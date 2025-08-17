//===- AdornedCFG.cpp ---------------------------------------------===//
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
//  This file defines an `AdornedCFG` class that is used by dataflow analyses
//  that run over Control-Flow Graphs (CFGs).
//
//===----------------------------------------------------------------------===//

#include "language/Core/Analysis/FlowSensitive/AdornedCFG.h"
#include "language/Core/AST/ASTContext.h"
#include "language/Core/AST/Decl.h"
#include "language/Core/AST/Stmt.h"
#include "language/Core/Analysis/CFG.h"
#include "toolchain/ADT/BitVector.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/Support/Error.h"
#include <utility>

namespace language::Core {
namespace dataflow {

/// Returns a map from statements to basic blocks that contain them.
static toolchain::DenseMap<const Stmt *, const CFGBlock *>
buildStmtToBasicBlockMap(const CFG &Cfg) {
  toolchain::DenseMap<const Stmt *, const CFGBlock *> StmtToBlock;
  for (const CFGBlock *Block : Cfg) {
    if (Block == nullptr)
      continue;

    for (const CFGElement &Element : *Block) {
      auto Stmt = Element.getAs<CFGStmt>();
      if (!Stmt)
        continue;

      StmtToBlock[Stmt->getStmt()] = Block;
    }
  }
  // Some terminator conditions don't appear as a `CFGElement` anywhere else -
  // for example, this is true if the terminator condition is a `&&` or `||`
  // operator.
  // We associate these conditions with the block the terminator appears in,
  // but only if the condition has not already appeared as a regular
  // `CFGElement`. (The `insert()` below does nothing if the key already exists
  // in the map.)
  for (const CFGBlock *Block : Cfg) {
    if (Block != nullptr)
      if (const Stmt *TerminatorCond = Block->getTerminatorCondition())
        StmtToBlock.insert({TerminatorCond, Block});
  }
  // Terminator statements typically don't appear as a `CFGElement` anywhere
  // else, so we want to associate them with the block that they terminate.
  // However, there are some important special cases:
  // -  The conditional operator is a type of terminator, but it also appears
  //    as a regular `CFGElement`, and we want to associate it with the block
  //    in which it appears as a `CFGElement`.
  // -  The `&&` and `||` operators are types of terminators, but like the
  //    conditional operator, they can appear as a regular `CFGElement` or
  //    as a terminator condition (see above).
  // We process terminators last to make sure that we only associate them with
  // the block they terminate if they haven't previously occurred as a regular
  // `CFGElement` or as a terminator condition.
  for (const CFGBlock *Block : Cfg) {
    if (Block != nullptr)
      if (const Stmt *TerminatorStmt = Block->getTerminatorStmt())
        StmtToBlock.insert({TerminatorStmt, Block});
  }
  return StmtToBlock;
}

static toolchain::BitVector findReachableBlocks(const CFG &Cfg) {
  toolchain::BitVector BlockReachable(Cfg.getNumBlockIDs(), false);

  toolchain::SmallVector<const CFGBlock *> BlocksToVisit;
  BlocksToVisit.push_back(&Cfg.getEntry());
  while (!BlocksToVisit.empty()) {
    const CFGBlock *Block = BlocksToVisit.back();
    BlocksToVisit.pop_back();

    if (BlockReachable[Block->getBlockID()])
      continue;

    BlockReachable[Block->getBlockID()] = true;

    for (const CFGBlock *Succ : Block->succs())
      if (Succ)
        BlocksToVisit.push_back(Succ);
  }

  return BlockReachable;
}

static toolchain::DenseSet<const CFGBlock *>
buildContainsExprConsumedInDifferentBlock(
    const CFG &Cfg, const internal::StmtToBlockMap &StmtToBlock) {
  toolchain::DenseSet<const CFGBlock *> Result;

  auto CheckChildExprs = [&Result, &StmtToBlock](const Stmt *S,
                                                 const CFGBlock *Block) {
    for (const Stmt *Child : S->children()) {
      if (!isa_and_nonnull<Expr>(Child))
        continue;
      const CFGBlock *ChildBlock = StmtToBlock.lookup(*Child);
      if (ChildBlock != Block)
        Result.insert(ChildBlock);
    }
  };

  for (const CFGBlock *Block : Cfg) {
    if (Block == nullptr)
      continue;

    for (const CFGElement &Element : *Block)
      if (auto S = Element.getAs<CFGStmt>())
        CheckChildExprs(S->getStmt(), Block);

    if (const Stmt *TerminatorCond = Block->getTerminatorCondition())
      CheckChildExprs(TerminatorCond, Block);
  }

  return Result;
}

namespace internal {

StmtToBlockMap::StmtToBlockMap(const CFG &Cfg)
    : StmtToBlock(buildStmtToBasicBlockMap(Cfg)) {}

} // namespace internal

toolchain::Expected<AdornedCFG> AdornedCFG::build(const FunctionDecl &Func) {
  if (!Func.doesThisDeclarationHaveABody())
    return toolchain::createStringError(
        std::make_error_code(std::errc::invalid_argument),
        "Cannot analyze function without a body");

  return build(Func, *Func.getBody(), Func.getASTContext());
}

toolchain::Expected<AdornedCFG> AdornedCFG::build(const Decl &D, Stmt &S,
                                             ASTContext &C) {
  if (D.isTemplated())
    return toolchain::createStringError(
        std::make_error_code(std::errc::invalid_argument),
        "Cannot analyze templated declarations");

  // The shape of certain elements of the AST can vary depending on the
  // language. We currently only support C++.
  if (!C.getLangOpts().CPlusPlus || C.getLangOpts().ObjC)
    return toolchain::createStringError(
        std::make_error_code(std::errc::invalid_argument),
        "Can only analyze C++");

  CFG::BuildOptions Options;
  Options.PruneTriviallyFalseEdges = true;
  Options.AddImplicitDtors = true;
  Options.AddTemporaryDtors = true;
  Options.AddInitializers = true;
  Options.AddCXXDefaultInitExprInCtors = true;
  Options.AddLifetime = true;

  // Ensure that all sub-expressions in basic blocks are evaluated.
  Options.setAllAlwaysAdd();

  auto Cfg = CFG::buildCFG(&D, &S, &C, Options);
  if (Cfg == nullptr)
    return toolchain::createStringError(
        std::make_error_code(std::errc::invalid_argument),
        "CFG::buildCFG failed");

  internal::StmtToBlockMap StmtToBlock(*Cfg);

  toolchain::BitVector BlockReachable = findReachableBlocks(*Cfg);

  toolchain::DenseSet<const CFGBlock *> ContainsExprConsumedInDifferentBlock =
      buildContainsExprConsumedInDifferentBlock(*Cfg, StmtToBlock);

  return AdornedCFG(D, std::move(Cfg), std::move(StmtToBlock),
                    std::move(BlockReachable),
                    std::move(ContainsExprConsumedInDifferentBlock));
}

} // namespace dataflow
} // namespace language::Core
