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

#include "reg_alloc.h"
#include "cleanup_empty_blocks.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/optimizations/cleanup.h"
#include "reg_alloc_graph_coloring.h"
#include "reg_alloc_linear_scan.h"
#include "reg_alloc_resolver.h"

namespace ark::compiler {

bool IsGraphColoringEnable(Graph *graph)
{
    if (graph->GetArch() == Arch::AARCH32 || !graph->IsAotMode() || !g_options.IsCompilerAotRa()) {
        return false;
    }

    size_t instCount = 0;
    for (auto bb : graph->GetBlocksRPO()) {
        instCount += bb->CountInsts();
    }
    return instCount < g_options.GetCompilerInstGraphColoringLimit();
}

bool ShouldSkipAllocation(Graph *graph)
{
#ifndef PANDA_TARGET_WINDOWS
    // Parameters spill-fills are empty
    return graph->GetCallingConvention() == nullptr && !graph->IsBytecodeOptimizer();
#else
    return !graph->IsBytecodeOptimizer();
#endif
}

void RemoveThrowEdges(Graph *graph)
{
    if (!graph->IsThrowApplied() || graph->GetThrowBlocks().empty()) {
        return;
    }
    [[maybe_unused]] constexpr size_t IMM_2 = 2;
    ASSERT(!graph->GetTryBeginBlocks().empty());
    bool updated = false;
    auto throwBlocks = graph->GetThrowBlocks();
    for (auto throwBlock : throwBlocks) {
        ASSERT(throwBlock->GetSuccsBlocks().size() <= IMM_2);
        auto succ = throwBlock->GetSuccessor(0);
        if (succ->IsEndBlock()) {
            succ = throwBlock->GetSuccessor(1);
        }
        ASSERT(succ->IsCatchBegin());
        graph->RemovePredecessorUpdateDF(succ, throwBlock);
        throwBlock->RemoveSucc(succ);
        [[maybe_unused]] auto res = graph->EraseThrowBlock(throwBlock);
        COMPILER_LOG(DEBUG, IR_BUILDER) << throwBlock->GetId() << " is erased";
        if (!res) {
            COMPILER_LOG(DEBUG, IR_BUILDER) << graph->GetRuntime()->GetMethodFullName(graph->GetMethod())
                                            << ": can't erase throwBlock " << throwBlock->GetId();
        }
        ASSERT(res);
        if (throwBlock->GetSuccsBlocks().empty()) {
            throwBlock->AddSucc(graph->GetEndBlock());
        }
        updated = true;
    }
    if (updated) {
        ASSERT(graph->GetThrowBlocks().empty());
        graph->InvalidateAnalysis<DominatorsTree>();
        graph->InvalidateAnalysis<LoopAnalyzer>();
        graph->RemoveUnreachableBlocks();
        graph->RunPass<Cleanup>();
    }
    graph->UnsetThrowApplied();
}

bool RegAlloc(Graph *graph)
{
    if (ShouldSkipAllocation(graph)) {
        return false;
    }

    // Regalloc doesn't use RPO, so additional throw arcs aren't needed
    RemoveThrowEdges(graph);

    bool raPassed = false;

    if (graph->IsBytecodeOptimizer()) {
        RegAllocResolver(graph).ResolveCatchPhis();
        raPassed = graph->RunPass<RegAllocGraphColoring>(GetFrameSize());
    } else {
        if (IsGraphColoringEnable(graph)) {
            raPassed = graph->RunPass<RegAllocGraphColoring>();
            if (!raPassed) {
                LOG(WARNING, COMPILER) << "RA graph coloring algorithm failed. Linear scan will be used.";
                graph->InvalidateAnalysis<LivenessAnalyzer>();
            }
        }
        if (!raPassed) {
            raPassed = graph->RunPass<RegAllocLinearScan>();
        }
    }
    if (raPassed) {
        CleanupEmptyBlocks(graph);
    }
    return raPassed;
}
}  // namespace ark::compiler
