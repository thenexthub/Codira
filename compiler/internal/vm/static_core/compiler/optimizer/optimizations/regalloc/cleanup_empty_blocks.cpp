/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cleanup_empty_blocks.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/basicblock.h"

namespace ark::compiler {
// Check around for special triangle case
static bool CheckSpecialTriangle(BasicBlock *bb)
{
    auto succ = bb->GetSuccessor(0);
    for (auto pred : bb->GetPredsBlocks()) {
        if (pred->GetSuccessor(0) == succ ||
            (pred->GetSuccsBlocks().size() == MAX_SUCCS_NUM && pred->GetSuccessor(1) == succ)) {
            return true;
        }
    }
    return false;
}

static bool TryRemoveEmptyBlock(BasicBlock *bb)
{
    if (CheckSpecialTriangle(bb)) {
        return false;
    }

    bool badLoop = bb->GetLoop()->IsIrreducible();
    bb->GetGraph()->RemoveEmptyBlockWithPhis(bb, badLoop);
    if (badLoop) {
        bb->GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
        bb->GetGraph()->RunPass<LoopAnalyzer>();
    }
    return true;
}

bool CleanupEmptyBlocks(Graph *graph)
{
    graph->RunPass<DominatorsTree>();
    graph->RunPass<LoopAnalyzer>();

    auto alloc = graph->GetAllocator();
    auto emptyBlocks = ArenaVector<BasicBlock *>(alloc->Adapter());

    for (auto bb : graph->GetVectorBlocks()) {
        if (!Cleanup::SkipBasicBlock(bb) && bb->IsEmpty()) {
            emptyBlocks.push_back(bb);
        }
    }

    auto modified = false;

    for (auto bb : emptyBlocks) {
        auto succ = bb->GetSuccessor(0);
        // Strange infinite loop with only one empty block, or loop pre-header - lets bail out
        if (succ == bb || succ->GetLoop()->GetPreHeader() == bb) {
            continue;
        }

        modified |= TryRemoveEmptyBlock(bb);
    }
    if (modified) {
        graph->InvalidateAnalysis<LinearOrder>();
        graph->InvalidateAnalysis<BoundsAnalysis>();
        graph->InvalidateAnalysis<compiler::AliasAnalysis>();
    }
    return modified;
}
}  // namespace ark::compiler
