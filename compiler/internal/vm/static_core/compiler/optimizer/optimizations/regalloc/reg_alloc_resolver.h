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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_PHI_RESOLVER_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_PHI_RESOLVER_H

#include "compiler/optimizer/analysis/liveness_analyzer.h"
#include "reg_alloc_base.h"
#include "compiler/optimizer/ir/graph.h"

namespace ark::compiler {

class RegAllocResolver {
public:
    explicit RegAllocResolver(Graph *graph)
        : graph_(graph),
          liveness_(&graph->GetAnalysis<LivenessAnalyzer>()),
          inputLocations_(graph->GetLocalAllocator()->Adapter())
    {
    }
    NO_MOVE_SEMANTIC(RegAllocResolver);
    NO_COPY_SEMANTIC(RegAllocResolver);
    ~RegAllocResolver() = default;

    void Resolve();
    PANDA_PUBLIC_API bool ResolveCatchPhis();

private:
    /*
     * Methods to set instructions source/dest registers
     */
    void ResolveInputs(Inst *inst);
    void ResolveOutput(Inst *inst);
    /*
     * Save states should capture location of the variable at the time when its user is executed.
     * If single SaveStateInst is reused by multiple users and some life intervals corresponding to
     * SaveStateInst's inputs were split between these users then SaveStateInst should be copied in
     * order to capture correct locations for each of its users.
     */
    void ResolveSaveState(Inst *inst);

    void AddCatchPhiMoves(Inst *inst);

    Inst *SqueezeCatchPhiInputs(CatchPhiInst *catchPhi);

    bool CanStoreToAccumulator(const Inst *inst) const
    {
        return graph_->IsBytecodeOptimizer() && inst->GetDstReg() == GetAccReg();
    }
    bool CanReadFromAccumulator(const Inst *inst, size_t inputNumber) const
    {
        return graph_->IsBytecodeOptimizer() && inst->GetSrcReg(inputNumber) == GetAccReg();
    }

    void PropagateCallerMasks(SaveStateInst *saveState);

    void FillSaveStateRootsMask(SaveStateInst *saveState, Inst *user, SaveStateInst *targetSs);

    void AddMoveToFixedLocation(Inst *inst, Location inputLocation, size_t inputNum);

    static inline void AddLocationToRoots(Location location, SaveStateInst *saveState, const Graph *graph)
    {
        if (location.IsFixedRegister()) {
            saveState->GetRootsRegsMask().set(location.GetValue());
        } else if (location.IsStack()) {
            saveState->GetRootsStackMask()->SetBit(location.GetValue());
        } else {
            ASSERT(location.IsStackParameter());
            auto slotOffset = graph->GetStackSlotsCount();
            saveState->GetRootsStackMask()->SetBit(location.GetValue() + slotOffset);
        }
    }

    Graph *GetGraph() const
    {
        return graph_;
    }

private:
    Graph *graph_;
    LivenessAnalyzer *liveness_;
    ArenaVector<Location> inputLocations_;
};

}  // namespace ark::compiler
#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_PHI_RESOLVER_H
