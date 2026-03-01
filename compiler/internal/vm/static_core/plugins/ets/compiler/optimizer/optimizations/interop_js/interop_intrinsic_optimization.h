/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_ETS_COMPILER_OPTIMIZER_INTEROP_JS_INTEROP_INTRINSIC_OPTIMIZATION_H_
#define PLUGINS_ETS_COMPILER_OPTIMIZER_INTEROP_JS_INTEROP_INTRINSIC_OPTIMIZATION_H_

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"

#include "compiler_logger.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/graph_visitor.h"

namespace ark::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class InteropIntrinsicOptimization : public Optimization, public GraphVisitor {
    using Optimization::Optimization;

public:
    explicit InteropIntrinsicOptimization(Graph *graph, bool trySingleScope = false)
        : Optimization(graph),
          trySingleScope_(trySingleScope),
          scopeObjectLimit_(g_options.GetCompilerInteropScopeObjectLimit()),
          forbiddenLoops_(GetGraph()->GetLocalAllocator()->Adapter()),
          blockInfo_(GetGraph()->GetVectorBlocks().size(), GetGraph()->GetLocalAllocator()->Adapter()),
          components_(GetGraph()->GetLocalAllocator()->Adapter()),
          currentComponentBlocks_(GetGraph()->GetLocalAllocator()->Adapter()),
          scopeForInst_(GetGraph()->GetLocalAllocator()->Adapter()),
          blocksToProcess_(GetGraph()->GetLocalAllocator()->Adapter())
    {
    }

    NO_MOVE_SEMANTIC(InteropIntrinsicOptimization);
    NO_COPY_SEMANTIC(InteropIntrinsicOptimization);
    ~InteropIntrinsicOptimization() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerEnableFastInterop() && g_options.IsCompilerInteropIntrinsicOptimization();
    }

    const char *GetPassName() const override
    {
        return "InteropIntrinsicOptimization";
    }

    bool IsApplied() const
    {
        return isApplied_;
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

#include "optimizer/ir/visitor.inc"

private:
    struct BlockInfo {
        Inst *lastScopeStart {};
        int32_t dfsNumIn {DFS_NOT_VISITED};
        int32_t dfsNumOut {DFS_NOT_VISITED};
        int32_t domTreeIn {DFS_NOT_VISITED};
        int32_t domTreeOut {DFS_NOT_VISITED};
        int32_t subgraphMinNum {DFS_NOT_VISITED};
        int32_t subgraphMaxNum {DFS_NOT_VISITED};
        std::array<int32_t, 2U> blockComponent {DFS_NOT_VISITED, DFS_NOT_VISITED};
        int32_t maxChain {};
        int32_t maxDepth {};
    };

    struct Component {
        int32_t parent {-1};
        uint32_t objectCount {};
        uint32_t lastUsed {};
        bool isForbidden {};
    };

    // Mechanism is similar to markers
    uint32_t GetObjectCountIfUnused(Component &comp, uint32_t usedNumber)
    {
        if (comp.lastUsed == usedNumber + 1) {
            return 0;
        }
        comp.lastUsed = usedNumber + 1;
        return comp.objectCount;
    }

    BlockInfo &GetInfo(BasicBlock *block);
    void MergeScopesInsideBlock(BasicBlock *block);
    bool TryCreateSingleScope(BasicBlock *bb);
    bool TryCreateSingleScope();
    std::optional<uint64_t> FindForbiddenLoops(Loop *loop);
    bool IsForbiddenLoopEntry(BasicBlock *block);
    int32_t GetParentComponent(int32_t component);
    void MergeComponents(int32_t first, int32_t second);
    void UpdateStatsForMerging(Inst *inst, int32_t otherEndComponent, uint32_t *scopeSwitches,
                               uint32_t *objectsInBlockAfterMerge);
    template <bool IS_END>
    void IterateBlockFromBoundary(BasicBlock *block);
    template <bool IS_END>
    void BlockBoundaryDfs(BasicBlock *block);
    void MergeCurrentComponentWithNeighbours();
    template <bool IS_END>
    void FindComponentAndTryMerge(BasicBlock *block);
    void MergeInterScopeRegions();

    void DfsNumbering(BasicBlock *block);
    void CalculateReachabilityRec(BasicBlock *block);
    template <void (InteropIntrinsicOptimization::*DFS)(BasicBlock *)>
    void DoDfs();
    bool CreateScopeStartInBlock(BasicBlock *block);
    void RemoveReachableScopeStarts(BasicBlock *block, BasicBlock *newStartBlock);
    void HoistScopeStarts();

    void InvalidateScopesInSubgraph(BasicBlock *block);
    void CheckGraphRec(BasicBlock *block, Inst *scopeStart);
    void CheckGraphAndFindScopes();

    void MarkRequireRegMap(Inst *inst);
    void TryRemoveUnwrapAndWrap(Inst *inst, Inst *input);
    void TryRemoveUnwrapToJSValue(Inst *inst);
    void TryRemoveIntrinsic(Inst *inst);
    void EliminateCastPairs();

    void DomTreeDfs(BasicBlock *block);
    void MarkPartiallyAnticipated(BasicBlock *block, BasicBlock *stopBlock);
    void CalculateDownSafe(BasicBlock *block);
    void ReplaceInst(Inst *inst, Inst **newInst, Inst *insertAfter);
    bool CanHoistTo(BasicBlock *block);
    void HoistAndEliminateRec(BasicBlock *block, const BlockInfo &startInfo, Inst **newInst, Inst *insertAfter);
    Inst *FindEliminationCandidate(BasicBlock *block);
    void HoistAndEliminate(BasicBlock *startBlock, Inst *boundaryInst);
    void DoRedundancyElimination(Inst *scopeStart, InstVector &insts);
    void SaveSiblingForElimination(Inst *sibling, ArenaMap<Inst *, InstVector> &currentInsts,
                                   RuntimeInterface::IntrinsicId id, Marker processed);
    void RedundancyElimination();

private:
    static constexpr uint64_t MAX_LOOP_ITERATIONS = 10;
    static constexpr int32_t DFS_NOT_VISITED = -1;
    static constexpr int32_t DFS_INVALIDATED = -2;

    bool trySingleScope_;
    uint32_t scopeObjectLimit_;
    Marker startDfs_ {};
    Marker canHoistTo_ {};
    Marker visited_ {};
    Marker instAnticipated_ {};
    Marker scopeStartInvalidated_ {};
    Marker eliminationCandidate_ {};
    Marker requireRegMap_ {};
    bool hasScopes_ {false};
    bool isApplied_ {false};
    int32_t dfsNum_ {};
    int32_t domTreeNum_ {};
    int32_t currentComponent_ {};
    uint32_t objectsInScopeAfterMerge_ {};
    bool canMerge_ {};
    bool objectLimitHit_ {false};
    ArenaUnorderedSet<Loop *> forbiddenLoops_;
    ArenaVector<BlockInfo> blockInfo_;
    ArenaVector<Component> components_;
    ArenaVector<BasicBlock *> currentComponentBlocks_;
    ArenaUnorderedMap<Inst *, Inst *> scopeForInst_;
    ArenaVector<BasicBlock *> blocksToProcess_;
    SaveStateBridgesBuilder ssb_;
};
}  // namespace ark::compiler

#endif  // PLUGINS_ETS_COMPILER_OPTIMIZER_INTEROP_JS_INTEROP_INTRINSIC_OPTIMIZATION_H_
