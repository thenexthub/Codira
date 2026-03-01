/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_MEMORY_BARRIERS_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_MEMORY_BARRIERS_H

#include "compiler_options.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_visitor.h"
#include "optimizer/pass.h"

namespace ark::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class OptimizeMemoryBarriers : public Optimization, public GraphVisitor {
public:
    explicit OptimizeMemoryBarriers(Graph *graph)
        : Optimization(graph), barriersInsts_(graph->GetLocalAllocator()->Adapter())
    {
    }
    NO_MOVE_SEMANTIC(OptimizeMemoryBarriers);
    NO_COPY_SEMANTIC(OptimizeMemoryBarriers);
    ~OptimizeMemoryBarriers() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "OptimizeMemoryBarriers";
    }

    bool IsEnable() const override
    {
        return g_options.IsCompilerOptimizeMemoryBarriers();
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

public:
    void ApplyGraph();
    bool IsApplied()
    {
        return isApplied_;
    }

private:
    void VisitDefault([[maybe_unused]] Inst *inst) override
    {
        ASSERT_DO(!inst->IsCall() && !inst->IsStore(),
                  (std::cerr << "need to make a visitor for the instruction: " << *inst << std::endl));
    }

    void CheckAllInputs(Inst *inst);
    void CheckInput(Inst *input);
    void CheckTwoInputs(Inst *input1, Inst *input2);

    static void VisitCallStatic(GraphVisitor *v, Inst *inst);
    static void VisitCallIndirect(GraphVisitor *v, Inst *inst);
    static void VisitCall(GraphVisitor *v, Inst *inst);
    static void VisitCallResolvedStatic(GraphVisitor *v, Inst *inst);
    static void VisitCallVirtual(GraphVisitor *v, Inst *inst);
    static void VisitCallResolvedVirtual(GraphVisitor *v, Inst *inst);
    static void VisitCallDynamic(GraphVisitor *v, Inst *inst);
    static void VisitCallNative(GraphVisitor *v, Inst *inst);
    static void VisitSelect(GraphVisitor *v, Inst *inst);
    static void VisitSelectImm(GraphVisitor *v, Inst *inst);
    static void VisitStoreArray(GraphVisitor *v, Inst *inst);
    static void VisitStoreArrayI(GraphVisitor *v, Inst *inst);
    static void VisitStoreObject(GraphVisitor *v, Inst *inst);
    static void VisitStoreObjectPair(GraphVisitor *v, Inst *inst);
    static void VisitStoreObjectDynamic(GraphVisitor *v, Inst *inst);
    static void VisitStoreStatic(GraphVisitor *v, Inst *inst);
    static void VisitStore(GraphVisitor *v, Inst *inst);
    static void VisitStoreI(GraphVisitor *v, Inst *inst);
    static void VisitStoreResolvedObjectField(GraphVisitor *v, Inst *inst);
    static void VisitUnresolvedStoreStatic(GraphVisitor *v, Inst *inst);
    static void VisitStoreResolvedObjectFieldStatic(GraphVisitor *v, Inst *inst);
    static void VisitStoreArrayPair(GraphVisitor *v, Inst *inst);
    static void VisitStoreArrayPairI(GraphVisitor *v, Inst *inst);
    static void VisitNullCheck(GraphVisitor *v, Inst *inst);
    static void VisitBoundCheck(GraphVisitor *v, Inst *inst);
    static void VisitFillConstArray([[maybe_unused]] GraphVisitor *v, [[maybe_unused]] Inst *inst) {}
    void MergeBarriers();
    bool CheckInst(Inst *inst);

#include "optimizer/ir/visitor.inc"

private:
    InstVector barriersInsts_;
    bool isApplied_ {false};
};
}  // namespace ark::compiler

#endif  //  COMPILER_OPTIMIZER_OPTIMIZATIONS_MEMORY_BARRIERS_H
