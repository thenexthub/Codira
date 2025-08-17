//===- DataflowWorklist.h ---------------------------------------*- C++ -*-===//
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
// A simple and reusable worklist for flow-sensitive analyses.
//
//===----------------------------------------------------------------------===//
#ifndef LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_DATAFLOWWORKLIST_H
#define LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_DATAFLOWWORKLIST_H

#include "language/Core/Analysis/Analyses/IntervalPartition.h"
#include "language/Core/Analysis/Analyses/PostOrderCFGView.h"
#include "language/Core/Analysis/CFG.h"
#include "toolchain/ADT/PriorityQueue.h"

namespace language::Core {
/// A worklist implementation where the enqueued blocks will be dequeued based
/// on the order defined by 'Comp'.
template <typename Comp, unsigned QueueSize> class DataflowWorklistBase {
  toolchain::BitVector EnqueuedBlocks;
  toolchain::PriorityQueue<const CFGBlock *,
                      SmallVector<const CFGBlock *, QueueSize>, Comp>
      WorkList;

public:
  DataflowWorklistBase(const CFG &Cfg, Comp C)
      : EnqueuedBlocks(Cfg.getNumBlockIDs()), WorkList(C) {}

  void enqueueBlock(const CFGBlock *Block) {
    if (Block && !EnqueuedBlocks[Block->getBlockID()]) {
      EnqueuedBlocks[Block->getBlockID()] = true;
      WorkList.push(Block);
    }
  }

  const CFGBlock *dequeue() {
    if (WorkList.empty())
      return nullptr;
    const CFGBlock *B = WorkList.top();
    WorkList.pop();
    EnqueuedBlocks[B->getBlockID()] = false;
    return B;
  }
};

struct ReversePostOrderCompare {
  PostOrderCFGView::BlockOrderCompare Cmp;
  bool operator()(const CFGBlock *lhs, const CFGBlock *rhs) const {
    return Cmp(rhs, lhs);
  }
};

/// A worklist implementation for forward dataflow analysis. The enqueued
/// blocks will be dequeued in reverse post order. The worklist cannot contain
/// the same block multiple times at once.
struct ForwardDataflowWorklist
    : DataflowWorklistBase<ReversePostOrderCompare, 20> {
  ForwardDataflowWorklist(const CFG &Cfg, PostOrderCFGView *POV)
      : DataflowWorklistBase(Cfg,
                             ReversePostOrderCompare{POV->getComparator()}) {}

  ForwardDataflowWorklist(const CFG &Cfg, AnalysisDeclContext &Ctx)
      : ForwardDataflowWorklist(Cfg, Ctx.getAnalysis<PostOrderCFGView>()) {}

  void enqueueSuccessors(const CFGBlock *Block) {
    for (auto B : Block->succs())
      enqueueBlock(B);
  }
};

/// A worklist implementation for forward dataflow analysis based on a weak
/// topological ordering of the nodes. The worklist cannot contain the same
/// block multiple times at once.
struct WTODataflowWorklist : DataflowWorklistBase<WTOCompare, 20> {
  WTODataflowWorklist(const CFG &Cfg, const WTOCompare &Cmp)
      : DataflowWorklistBase(Cfg, Cmp) {}

  void enqueueSuccessors(const CFGBlock *Block) {
    for (auto B : Block->succs())
      enqueueBlock(B);
  }
};

/// A worklist implementation for backward dataflow analysis. The enqueued
/// block will be dequeued in post order. The worklist cannot contain the same
/// block multiple times at once.
struct BackwardDataflowWorklist
    : DataflowWorklistBase<PostOrderCFGView::BlockOrderCompare, 20> {
  BackwardDataflowWorklist(const CFG &Cfg, AnalysisDeclContext &Ctx)
      : DataflowWorklistBase(
            Cfg, Ctx.getAnalysis<PostOrderCFGView>()->getComparator()) {}

  void enqueuePredecessors(const CFGBlock *Block) {
    for (auto B : Block->preds())
      enqueueBlock(B);
  }
};

} // namespace language::Core

#endif // LANGUAGE_CORE_ANALYSIS_FLOWSENSITIVE_DATAFLOWWORKLIST_H
