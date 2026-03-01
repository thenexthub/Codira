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

#include "compiler_logger.h"
#include "optimizer/ir/basicblock.h"
#include "memory_barriers.h"

namespace ark::compiler {

void OptimizeMemoryBarriers::MergeBarriers()
{
    if (barriersInsts_.size() <= 1) {
        barriersInsts_.clear();
        return;
    }
    isApplied_ = true;
    auto lastBarrierInst = barriersInsts_.back();
    for (auto inst : barriersInsts_) {
        inst->ClearFlag(inst_flags::MEM_BARRIER);
    }
    lastBarrierInst->SetFlag(inst_flags::MEM_BARRIER);
    barriersInsts_.clear();
}

bool OptimizeMemoryBarriers::CheckInst(Inst *inst)
{
    return (std::find(barriersInsts_.begin(), barriersInsts_.end(), inst) != barriersInsts_.end());
}

void OptimizeMemoryBarriers::CheckAllInputs(Inst *inst)
{
    for (auto input : inst->GetInputs()) {
        if (CheckInst(input.GetInst())) {
            MergeBarriers();
            return;
        }
    }
}

void OptimizeMemoryBarriers::CheckInput(Inst *input)
{
    if (CheckInst(input)) {
        MergeBarriers();
    }
}

void OptimizeMemoryBarriers::CheckTwoInputs(Inst *input1, Inst *input2)
{
    if (CheckInst(input1) || CheckInst(input2)) {
        MergeBarriers();
    }
}

void OptimizeMemoryBarriers::VisitCallStatic(GraphVisitor *v, Inst *inst)
{
    if (inst->CastToCallStatic()->IsInlined()) {
        return;
    }
    static_cast<OptimizeMemoryBarriers *>(v)->CheckAllInputs(inst);
}

void OptimizeMemoryBarriers::VisitCallIndirect(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckAllInputs(inst);
}

void OptimizeMemoryBarriers::VisitCall(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckAllInputs(inst);
}

void OptimizeMemoryBarriers::VisitCallResolvedStatic(GraphVisitor *v, Inst *inst)
{
    if (inst->CastToCallResolvedStatic()->IsInlined()) {
        return;
    }
    static_cast<OptimizeMemoryBarriers *>(v)->CheckAllInputs(inst);
}

void OptimizeMemoryBarriers::VisitCallVirtual(GraphVisitor *v, Inst *inst)
{
    if (inst->CastToCallVirtual()->IsInlined()) {
        return;
    }
    static_cast<OptimizeMemoryBarriers *>(v)->CheckAllInputs(inst);
}

void OptimizeMemoryBarriers::VisitCallResolvedVirtual(GraphVisitor *v, Inst *inst)
{
    if (inst->CastToCallResolvedVirtual()->IsInlined()) {
        return;
    }
    static_cast<OptimizeMemoryBarriers *>(v)->CheckAllInputs(inst);
}

void OptimizeMemoryBarriers::VisitCallDynamic(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckAllInputs(inst);
}

void OptimizeMemoryBarriers::VisitCallNative(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckAllInputs(inst);
}

void OptimizeMemoryBarriers::VisitSelect(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckTwoInputs(inst->GetInput(0).GetInst(), inst->GetInput(1).GetInst());
}

void OptimizeMemoryBarriers::VisitSelectImm(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckTwoInputs(inst->GetInput(0).GetInst(), inst->GetInput(1).GetInst());
}

void OptimizeMemoryBarriers::VisitStoreObject(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckInput(inst->GetInput(inst->GetInputsCount() - 1).GetInst());
}

void OptimizeMemoryBarriers::VisitStoreObjectPair(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckTwoInputs(inst->GetInput(0).GetInst(), inst->GetInput(1).GetInst());
}

void OptimizeMemoryBarriers::VisitStoreObjectDynamic(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckInput(inst->GetInput(inst->GetInputsCount() - 1).GetInst());
}

void OptimizeMemoryBarriers::VisitStoreArray(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckInput(inst->GetInput(inst->GetInputsCount() - 1).GetInst());
}

void OptimizeMemoryBarriers::VisitStoreArrayI(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckInput(inst->GetInput(inst->GetInputsCount() - 1).GetInst());
}

void OptimizeMemoryBarriers::VisitStoreStatic(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckInput(inst->GetInput(inst->GetInputsCount() - 1).GetInst());
}

void OptimizeMemoryBarriers::VisitStore(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckInput(inst->GetInput(inst->GetInputsCount() - 1).GetInst());
}

void OptimizeMemoryBarriers::VisitStoreI(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckInput(inst->GetInput(inst->GetInputsCount() - 1).GetInst());
}

void OptimizeMemoryBarriers::VisitStoreResolvedObjectField(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckInput(inst->GetInput(1).GetInst());
}

void OptimizeMemoryBarriers::VisitStoreResolvedObjectFieldStatic(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckInput(inst->GetInput(1).GetInst());
}

void OptimizeMemoryBarriers::VisitUnresolvedStoreStatic(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckInput(inst->GetInput(0).GetInst());
}

void OptimizeMemoryBarriers::VisitStoreArrayPair(GraphVisitor *v, Inst *inst)
{
    auto val = inst->GetInputsCount() - 1;
    static_cast<OptimizeMemoryBarriers *>(v)->CheckTwoInputs(inst->GetInput(val).GetInst(),
                                                             inst->GetInput(val - 1).GetInst());
}

void OptimizeMemoryBarriers::VisitStoreArrayPairI(GraphVisitor *v, Inst *inst)
{
    auto val = inst->GetInputsCount() - 1;
    static_cast<OptimizeMemoryBarriers *>(v)->CheckTwoInputs(inst->GetInput(val).GetInst(),
                                                             inst->GetInput(val - 1).GetInst());
}

static Inst *GetMemInstForImplicitNullCheck(Inst *inst)
{
    if (!inst->HasUsers()) {
        return nullptr;
    }
    auto nextInst = inst->GetNext();
    while (nextInst != nullptr) {
        if (IsSuitableForImplicitNullCheck(nextInst)) {
            if (nextInst->GetInput(0) != inst) {
                return nullptr;
            }
            return nextInst;
        }
        if (!nextInst->IsSafeInst()) {
            return nullptr;
        }
        nextInst = nextInst->GetNext();
    }

    return nullptr;
}

void OptimizeMemoryBarriers::VisitNullCheck(GraphVisitor *v, Inst *inst)
{
    // NullCheck was hoisted and can't be implicit
    if (inst->CanDeoptimize()) {
        return;
    }
    static_cast<OptimizeMemoryBarriers *>(v)->CheckInput(inst->GetInput(0).GetInst());
    auto nc = static_cast<NullCheckInst *>(inst);
    auto graph = nc->GetBasicBlock()->GetGraph();
    // NOTE (pishin) support for arm32
    if (graph->GetArch() == Arch::AARCH32) {
        return;
    }
    if (!g_options.IsCompilerImplicitNullCheck() || graph->IsOsrMode() || graph->IsBytecodeOptimizer()) {
        return;
    }

    auto memInst = GetMemInstForImplicitNullCheck(inst);
    if (memInst == nullptr) {
        return;
    }
    memInst->SetFlag(compiler::inst_flags::CAN_THROW);
    nc->SetImplicit(true);
}

void OptimizeMemoryBarriers::VisitBoundCheck(GraphVisitor *v, Inst *inst)
{
    static_cast<OptimizeMemoryBarriers *>(v)->CheckTwoInputs(inst->GetInput(0).GetInst(), inst->GetInput(1).GetInst());
}

void OptimizeMemoryBarriers::ApplyGraph()
{
    barriersInsts_.clear();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->Insts()) {
            if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
                barriersInsts_.push_back(inst);
            }
            this->VisitInstruction(inst);
        }
        this->MergeBarriers();
    }
}

bool OptimizeMemoryBarriers::RunImpl()
{
    ApplyGraph();
    return IsApplied();
}

}  // namespace ark::compiler
