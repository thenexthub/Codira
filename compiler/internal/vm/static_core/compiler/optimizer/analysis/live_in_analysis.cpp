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

#include "live_in_analysis.h"
#include "loop_analyzer.h"

namespace ark::compiler {

bool LiveInAnalysis::IsAlive(const BasicBlock *block, const Inst *inst) const
{
    if (inst->GetType() == DataType::ANY) {
        return true;
    }
    if (!inst->HasUsers()) {
        return false;
    }
    ASSERT(DataType::IsReference(inst->GetType()));
    inst = Inst::GetDataFlowInput(const_cast<Inst *>(inst));
    ASSERT_PRINT(inst->GetId() < allInsts_.size() && allInsts_[inst->GetId()] == inst, "No inst in LiveIn: " << *inst);
    auto &liveIns = liveIn_[block->GetId()];
    return liveIns.GetBit(inst->GetId());
}

void LiveInAnalysis::ProcessBlock(BasicBlock *block)
{
    auto &liveIns = liveIn_[block->GetId()];

    for (auto succ : block->GetSuccsBlocks()) {
        liveIns |= liveIn_[succ->GetId()];
        for (auto phiInst : succ->PhiInsts()) {
            auto phiInput = phiInst->CastToPhi()->GetPhiInput(block);
            if (DataType::IsReference(phiInput->GetType())) {
                liveIns.SetBit(phiInput->GetId());
                AddInst(phiInput);
            }
        }
    }

    for (auto inst : block->InstsReverse()) {
        if (DataType::IsReference(inst->GetType())) {
            liveIns.ClearBit(inst->GetId());
        }
        for (auto &input : inst->GetInputs()) {
            auto inputInst = input.GetInst();
            if (!DataType::IsReference(inputInst->GetType())) {
                continue;
            }
            liveIns.SetBit(inputInst->GetId());
            AddInst(inputInst);
            auto dfInput = inst->GetDataFlowInput(inputInst);
            if (dfInput != inputInst) {
                liveIns.SetBit(dfInput->GetId());
                AddInst(dfInput);
            }
        }
    }
    for (auto inst : block->PhiInsts()) {
        if (DataType::IsReference(inst->GetType())) {
            liveIns.ClearBit(inst->GetId());
        }
    }

    // Loop header's live-ins are alive throughout the whole loop.
    // If current block is a header then propagate its live-ins
    // to all current loop's blocks as well as to all inner loops.
    auto loop = block->GetLoop();
    if (loop->IsRoot() || loop->GetHeader() != block) {
        return;
    }
    for (auto loopBlock : graph_->GetVectorBlocks()) {
        if (loopBlock == nullptr || loopBlock == block) {
            continue;
        }
        if (loopBlock->GetLoop() == loop || loopBlock->GetLoop()->IsInside(loop)) {
            auto &loopBlockLiveIns = liveIn_[loopBlock->GetId()];
            loopBlockLiveIns |= liveIns;
        }
    }
}

bool LiveInAnalysis::HasAllocs(Graph *graph)
{
    for (auto block : graph->GetBlocksRPO()) {
        for (auto inst : block->Insts()) {
            if (IsAllocInst(inst)) {
                return true;
            }
        }
    }
    return false;
}

bool LiveInAnalysis::Run(bool checkAllocs)
{
    if (checkAllocs && !HasAllocs(graph_)) {
        return false;
    }
    graph_->RunPass<LoopAnalyzer>();

    // Must be fresh LiveInAnalysis because we use LocalAllocator
    ASSERT(liveIn_.empty());
    ASSERT(allInsts_.empty());
    for (size_t idx = 0; idx < graph_->GetVectorBlocks().size(); ++idx) {
        liveIn_.emplace_back(graph_->GetLocalAllocator());
    }

    auto &rpo = graph_->GetBlocksRPO();
    for (auto it = rpo.rbegin(), end = rpo.rend(); it != end; ++it) {
        ProcessBlock(*it);
    }
    return true;
}

}  // namespace ark::compiler
