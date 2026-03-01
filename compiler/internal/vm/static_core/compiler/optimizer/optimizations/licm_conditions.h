/*
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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_LICM_CONDITIONS_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_LICM_CONDITIONS_H

#include "optimizer/optimizations/loop_transform.h"
#include "condition_chain_manager.h"
#include "condition_chain_cache.h"
#include "compiler_options.h"

namespace ark::compiler {

class ConditionChainContext {
public:
    ConditionChainContext(ConditionChain *chain, BasicBlock *multiplePredecessorsSuccessor,
                          BasicBlock *singlePredecessorSuccessor, bool hoistPhiAvailable)
        : chain_(chain),
          multiplePredecessorsSuccessor_(multiplePredecessorsSuccessor),
          singlePredecessorSuccessor_(singlePredecessorSuccessor),
          hoistPhiAvailable_(hoistPhiAvailable)
    {
    }

    ConditionChain *GetChain() const
    {
        return chain_;
    }

    BasicBlock *GetMultiplePredecessorsSuccessor() const
    {
        return multiplePredecessorsSuccessor_;
    }

    BasicBlock *GetSinglePredecessorSuccessor() const
    {
        return singlePredecessorSuccessor_;
    }

    void SetMultiplePredecessorsSuccessor(BasicBlock *multiplePredecessorsSuccessor)
    {
        multiplePredecessorsSuccessor_ = multiplePredecessorsSuccessor;
    }

    void SetSingleSPredecessorSuccessor(BasicBlock *singlePredecessorSuccessor)
    {
        singlePredecessorSuccessor_ = singlePredecessorSuccessor;
    }

    bool IsHoistPhiAvailable() const
    {
        return hoistPhiAvailable_;
    }

private:
    ConditionChain *chain_;
    BasicBlock *multiplePredecessorsSuccessor_;
    BasicBlock *singlePredecessorSuccessor_;
    bool hoistPhiAvailable_;
};

class LicmConditions : public LoopTransform<LoopExitPoint::LOOP_EXIT_HEADER> {
public:
    explicit LicmConditions(Graph *graph)
        : LoopTransform(graph),
          conditionChainsCtx_(graph->GetLocalAllocator()->Adapter()),
          samePhiInput_(graph->GetLocalAllocator()->Adapter()),
          multiplePredecessorsSuccessorPreds_(graph->GetLocalAllocator()->Adapter()),
          multiplePredecessorsSuccessorRemovedPredIndices_(graph->GetLocalAllocator()->Adapter()),
          conditionChainsCache_(graph->GetLocalAllocator()),
          conditionChainManager_(graph->GetLocalAllocator())
    {
    }
    bool RunImpl() override;
    const char *GetPassName() const override
    {
        return "LICM_conditions";
    }

    bool IsEnable() const override
    {
        return g_options.IsCompilerLicmConditions();
    }

    void InvalidateAnalyses() override;

private:
    static bool IsHoistPhiAvailable(const ConditionChain *chain, ArenaVector<BasicBlock *> &preds,
                                    const BasicBlock *singlePredSucc);
    bool TransformLoop(Loop *loop) override;
    void MarkHoistableInst();
    bool IsHoistable(const ConditionChain *chain);
    bool AllInputsDominate(const Inst *inst, const Loop *loop);
    void FindHoistableConditionChains(Loop *loop);
    bool AllPhiAllowConditionChainHoisting(const ConditionChain *chain, const BasicBlock *multiplePredsSucc,
                                           bool hoistPhiAvailable);
    Inst *SamePhiInputFromChain(Inst *phi, const ConditionChain *chain);
    void HoistConditionChains(Loop *loop);
    void SplitChainFirstBasicBlock(ConditionChain *chain);
    BasicBlock *ReplaceChainWithSingleBlock(BasicBlock *appendBb, const ConditionChainContext &chainCtx);
    void SaveMulitplePredecessorsSuccessorPreds(const BasicBlock *bb);
    PhiInst *AddPhiInst(BasicBlock *bb, const ConditionChain *chain);
    void AddSingleIfImmInst(BasicBlock *bb, const ConditionChain *chain, Inst *input);
    void SaveProcessedBlocks(const ConditionChain *chain);
    void AdjustPredecessorEdges(BasicBlock *chainFirstBb, BasicBlock *bb);
    void UpdateMultiplePredecessorsSuccessorsPreds(const ConditionChainContext &chainCtx, BasicBlock *phiBlock,
                                                   BasicBlock *emptyBlock);
    void UpdatePhis(const ConditionChain *chain, BasicBlock *multiplePredsSucc, BasicBlock *phiBlock,
                    bool hoistPhiAvailable);
    ArenaVector<ConditionChainContext> conditionChainsCtx_;
    ArenaUnorderedMap<std::pair<const ConditionChain *, Inst *>, Inst *, PairHash> samePhiInput_;
    ArenaVector<BasicBlock *> multiplePredecessorsSuccessorPreds_;
    ArenaVector<size_t> multiplePredecessorsSuccessorRemovedPredIndices_;
    ConditionChainCache conditionChainsCache_;
    Marker processedBlocksMarker_ {UNDEF_MARKER};
    Marker hoistableInstMarker_ {UNDEF_MARKER};
    ConditionChainManager conditionChainManager_;
    bool isApplied_ {false};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_LICM_CONDITIONS_H
