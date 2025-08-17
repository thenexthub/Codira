//===- CFGReachabilityAnalysis.cpp - Basic reachability analysis ----------===//
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
// This file defines a flow-sensitive, (mostly) path-insensitive reachability
// analysis based on Clang's CFGs.  Clients can query if a given basic block
// is reachable within the CFG.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Analysis/Analyses/CFGReachabilityAnalysis.h"
#include "language/Core/Analysis/CFG.h"
#include "toolchain/ADT/BitVector.h"

using namespace language::Core;

CFGReverseBlockReachabilityAnalysis::CFGReverseBlockReachabilityAnalysis(
    const CFG &cfg)
    : analyzed(cfg.getNumBlockIDs(), false) {}

bool CFGReverseBlockReachabilityAnalysis::isReachable(const CFGBlock *Src,
                                          const CFGBlock *Dst) {
  const unsigned DstBlockID = Dst->getBlockID();

  // If we haven't analyzed the destination node, run the analysis now
  if (!analyzed[DstBlockID]) {
    mapReachability(Dst);
    analyzed[DstBlockID] = true;
  }

  // Return the cached result
  return reachable[DstBlockID][Src->getBlockID()];
}

// Maps reachability to a common node by walking the predecessors of the
// destination node.
void CFGReverseBlockReachabilityAnalysis::mapReachability(const CFGBlock *Dst) {
  SmallVector<const CFGBlock *, 11> worklist;
  toolchain::BitVector visited(analyzed.size());

  ReachableSet &DstReachability = reachable[Dst->getBlockID()];
  DstReachability.resize(analyzed.size(), false);

  // Start searching from the destination node, since we commonly will perform
  // multiple queries relating to a destination node.
  worklist.push_back(Dst);
  bool firstRun = true;

  while (!worklist.empty()) {
    const CFGBlock *block = worklist.pop_back_val();

    if (visited[block->getBlockID()])
      continue;
    visited[block->getBlockID()] = true;

    // Update reachability information for this node -> Dst
    if (!firstRun) {
      // Don't insert Dst -> Dst unless it was a predecessor of itself
      DstReachability[block->getBlockID()] = true;
    }
    else
      firstRun = false;

    // Add the predecessors to the worklist.
    for (CFGBlock::const_pred_iterator i = block->pred_begin(),
         e = block->pred_end(); i != e; ++i) {
      if (*i)
        worklist.push_back(*i);
    }
  }
}
