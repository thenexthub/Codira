//===--- CFGStmtMap.h - Map from Stmt* to CFGBlock* -----------*- C++ -*-===//
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
//  This file defines the CFGStmtMap class, which defines a mapping from
//  Stmt* to CFGBlock*
//
//===----------------------------------------------------------------------===//

#include "toolchain/ADT/DenseMap.h"
#include "language/Core/AST/ParentMap.h"
#include "language/Core/Analysis/CFG.h"
#include "language/Core/Analysis/CFGStmtMap.h"
#include <optional>

using namespace language::Core;

typedef toolchain::DenseMap<const Stmt*, CFGBlock*> SMap;
static SMap *AsMap(void *m) { return (SMap*) m; }

CFGStmtMap::~CFGStmtMap() { delete AsMap(M); }

CFGBlock *CFGStmtMap::getBlock(Stmt *S) {
  SMap *SM = AsMap(M);
  Stmt *X = S;

  // If 'S' isn't in the map, walk the ParentMap to see if one of its ancestors
  // is in the map.
  while (X) {
    SMap::iterator I = SM->find(X);
    if (I != SM->end()) {
      CFGBlock *B = I->second;
      // Memoize this lookup.
      if (X != S)
        (*SM)[X] = B;
      return B;
    }

    X = PM->getParentIgnoreParens(X);
  }

  return nullptr;
}

static void Accumulate(SMap &SM, CFGBlock *B) {
  // First walk the block-level expressions.
  for (CFGBlock::iterator I = B->begin(), E = B->end(); I != E; ++I) {
    const CFGElement &CE = *I;
    std::optional<CFGStmt> CS = CE.getAs<CFGStmt>();
    if (!CS)
      continue;

    CFGBlock *&Entry = SM[CS->getStmt()];
    // If 'Entry' is already initialized (e.g., a terminator was already),
    // skip.
    if (Entry)
      continue;

    Entry = B;

  }

  // Look at the label of the block.
  if (Stmt *Label = B->getLabel())
    SM[Label] = B;

  // Finally, look at the terminator.  If the terminator was already added
  // because it is a block-level expression in another block, overwrite
  // that mapping.
  if (Stmt *Term = B->getTerminatorStmt())
    SM[Term] = B;
}

CFGStmtMap *CFGStmtMap::Build(CFG *C, ParentMap *PM) {
  if (!C || !PM)
    return nullptr;

  SMap *SM = new SMap();

  // Walk all blocks, accumulating the block-level expressions, labels,
  // and terminators.
  for (CFGBlock *BB : *C)
    Accumulate(*SM, BB);

  return new CFGStmtMap(PM, SM);
}

