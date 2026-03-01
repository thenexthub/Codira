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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_DEOPTIMIZEELIMINATION_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_DEOPTIMIZEELIMINATION_H

#include "compiler_logger.h"
#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/graph_visitor.h"

namespace ark::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class DeoptimizeElimination : public Optimization, public GraphVisitor {
    using Optimization::Optimization;

public:
    explicit DeoptimizeElimination(Graph *graph)
        : Optimization(graph),
          blocksType_(graph->GetLocalAllocator()->Adapter()),
          deoptimizeMustThrow_(graph->GetLocalAllocator()->Adapter())
    {
    }

    NO_MOVE_SEMANTIC(DeoptimizeElimination);
    NO_COPY_SEMANTIC(DeoptimizeElimination);
    ~DeoptimizeElimination() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "DeoptimizeElimination";
    }

    void InvalidateAnalyses() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerDeoptimizeElimination();
    }

    void ReplaceDeoptimizeIfByUnconditionalDeoptimize();

    void RemoveSafePoints();

    /*
     * By default all blocks have INVALID_TYPE.
     * GUARD - If block have IsMustDeootimize before runtime call inst(in reverse order)
     * RUNTIME_CALL - If block have runtime call inst before IsMustDeoptimize(in reverse order)
     * NOTHING - If block is preccessed, but it doesn't contain GUARD and RUNTIME_CALL
     */
    enum BlockType { INVALID, GUARD, RUNTIME_CALL, NOTHING };

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    void SetApplied()
    {
        isApplied_ = true;
    }

    bool IsApplied() const
    {
        return isApplied_;
    }

    void SetLoopDeleted()
    {
        isLoopDeleted_ = true;
    }

    bool IsLoopDeleted() const
    {
        return isLoopDeleted_;
    }

    static void VisitDeoptimizeIf(GraphVisitor *v, Inst *inst);

#include "optimizer/ir/visitor.inc"

private:
    void PushNewDeoptimizeIf(Inst *inst)
    {
        deoptimizeMustThrow_.push_back(inst);
    }

    void PushNewBlockType(BasicBlock *block, BlockType type)
    {
        ASSERT(blocksType_.find(block) == blocksType_.end());
        blocksType_.emplace(block, type);
    }

    BlockType GetBlockType(BasicBlock *block)
    {
        if (blocksType_.find(block) != blocksType_.end()) {
            return blocksType_.at(block);
        }
        return BlockType::INVALID;
    }

    bool CanRemoveGuard(Inst *guard);
    bool CanRemoveGuardRec(BasicBlock *block, Inst *guard, const Marker &mrk, const Marker &removeMrk);
    void RemoveGuard(Inst *guard);
    void RemoveDeoptimizeIf(Inst *inst);

private:
    bool isApplied_ {false};
    bool isLoopDeleted_ {false};
    ArenaUnorderedMap<BasicBlock *, BlockType> blocksType_;
    InstVector deoptimizeMustThrow_;
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_DEOPTIMIZEELIMINATION_H
