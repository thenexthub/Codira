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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_IF_MERGING_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_IF_MERGING_H

#include <optional>

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"

namespace ark::compiler {
class IfMerging : public Optimization {
public:
    PANDA_PUBLIC_API explicit IfMerging(Graph *graph);

    NO_MOVE_SEMANTIC(IfMerging);
    NO_COPY_SEMANTIC(IfMerging);
    ~IfMerging() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerIfMerging();
    }

    const char *GetPassName() const override
    {
        return "IfMerging";
    }

    void InvalidateAnalyses() override;

private:
    bool TrySimplifyConstantPhi(BasicBlock *block);
    bool TryRemoveConstantPhiIf(BasicBlock *ifBlock);
    IfImmInst *GetIfImm(BasicBlock *block);
    bool TryMergeEquivalentIfs(BasicBlock *bb);
    bool TryRemoveConstantPhiIf(IfImmInst *ifImm, PhiInst *phi, uint64_t constant, ConditionCode cc);
    bool MarkInstBranches(BasicBlock *bb, BasicBlock *trueBb, BasicBlock *falseBb);
    std::optional<bool> GetUserBranch(Inst *userInst, BasicBlock *bb, BasicBlock *trueBb, BasicBlock *falseBb);
    bool IsDominateEdge(BasicBlock *edgeBb, BasicBlock *targetBb);
    void SplitBlockWithEquivalentIf(BasicBlock *bb, BasicBlock *trueBb, bool invertedIf);
    void SplitBlockWithConstantPhi(BasicBlock *bb, BasicBlock *trueBb, PhiInst *phi, uint64_t constant,
                                   ConditionCode cc);
    BasicBlock *SplitBlock(BasicBlock *bb);
    void FixDominatorsTree(BasicBlock *trueBranchBb, BasicBlock *falseBranchBb);
    void TryJoinSuccessorBlock(BasicBlock *bb);
    void TryUpdateDominator(BasicBlock *bb);

#ifndef NDEBUG
    void CheckDomTreeValid();
#endif

private:
    bool isApplied_ {false};
    Marker trueBranchMarker_ {};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_IF_MERGING_H
