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

#include "split_resolver.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/analysis/dominators_tree.h"
#include "compiler/optimizer/analysis/loop_analyzer.h"
#include "compiler/optimizer/optimizations/regalloc/reg_alloc_base.h"

namespace panda::compiler {

void SplitResolver::Run()
{
    for (auto interval : liveness_->GetLifeIntervals()) {
        if (interval->GetSibling() == nullptr) {
            continue;
        }
        ASSERT(!interval->IsPhysical());

        // Connect siblings within the same block.
        ConnectSiblings(interval);
    }
    // Resolve locations between basic blocks.
    for (auto block : liveness_->GetLinearizedBlocks()) {
        ASSERT(block != nullptr);
        if (!block->IsEndBlock()) {
            ProcessBlock(block);
        }
    }
}

void SplitResolver::ConnectSiblings(LifeIntervals *interval)
{
    for (auto prev = interval, curr = interval->GetSibling(); curr != nullptr; prev = curr, curr = curr->GetSibling()) {
        if (prev->GetEnd() != curr->GetBegin() || prev->GetLocation() == curr->GetLocation() ||
            curr->GetLocation().IsConstant()) {
            continue;
        }
        COMPILER_LOG(DEBUG, SPLIT_RESOLVER)
            << "Connect siblings for inst v" << interval->GetInst()->GetId() << " at point: " << curr->GetBegin();

        ASSERT(curr->IsSplitSibling());
        auto inst = liveness_->GetInstByLifeNumber(curr->GetBegin() + 1U);
        // inst == nullptr means that life number corresponds to some PHI instruction (== corresponds to start of
        // some block), so the SpillFill should be placed at the end of predecessor block.
        if (inst == nullptr) {
            continue;
        }

        auto spill_fill = CreateSpillFillForSiblings(inst);
        ConnectIntervals(spill_fill, prev, curr);
    }
}

void SplitResolver::ProcessBlock(BasicBlock *block)
{
    auto succ_begin = liveness_->GetBlockLiveRange(block).GetBegin();
    for (auto interval : liveness_->GetLifeIntervals()) {
        // PHI and its inputs can be considered as one interval, which was split,
        // so that the logic is equivalent to the logic of connecting split intervals
        if (interval->GetBegin() == succ_begin) {
            auto phi = interval->GetInst();
            if (!phi->IsPhi() || phi->GetDstReg() == ACC_REG_ID) {
                continue;
            }
            ASSERT(phi->IsPhi());
            for (size_t i = 0; i < phi->GetInputsCount(); i++) {
                auto input_inst = phi->GetDataFlowInput(i);
                auto input_bb = phi->CastToPhi()->GetPhiInputBb(i);
                auto input_liveness = liveness_->GetInstLifeIntervals(input_inst);
                ConnectSpiltFromPredBlock(input_bb, input_liveness, block, interval);
            }
            continue;
        }

        // Skip not-splitted instruction
        if (interval->GetSibling() == nullptr) {
            continue;
        }
        ASSERT(!interval->IsPhysical());
        auto succ_split = interval->FindSiblingAt(succ_begin);
        if (succ_split == nullptr || succ_split->GetLocation().IsConstant() || !succ_split->SplitCover(succ_begin)) {
            continue;
        }
        for (auto pred : block->GetPredsBlocks()) {
            ConnectSpiltFromPredBlock(pred, interval, block, succ_split);
        }
    }
}

void SplitResolver::ConnectSpiltFromPredBlock(BasicBlock *src_bb, LifeIntervals *src_interval, BasicBlock *target_bb,
                                              LifeIntervals *target_split)
{
    BasicBlock *resolver {nullptr};
    // It's a resolver block inserted during register allocation
    if (src_bb->GetId() >= liveness_->GetBlocksCount()) {
        ASSERT(src_bb->GetSuccsBlocks().size() == 1 && src_bb->GetPredsBlocks().size() == 1);
        resolver = src_bb;
        src_bb = src_bb->GetPredecessor(0);
    }
    auto src_liveness = liveness_->GetBlockLiveRange(src_bb);
    // Find sibling at the 'end - LIFE_NUMBER_GAP' position to connect siblings that were split at the end of the
    // 'src_bb'
    auto src_split = src_interval->FindSiblingAt(src_liveness.GetEnd() - 1U);
    // Instruction was not defined at predecessor or has the same location there
    if (src_split == nullptr || src_split->GetLocation() == target_split->GetLocation()) {
        return;
    }

    COMPILER_LOG(DEBUG, SPLIT_RESOLVER) << "Resolve split move for inst v" << src_interval->GetInst()->GetId()
                                        << " between blocks: BB" << src_bb->GetId() << " -> BB" << target_bb->GetId();

    if (resolver == nullptr) {
        if (src_bb->GetSuccsBlocks().size() == 1U) {
            resolver = src_bb;
        } else {
            // Get rid of critical edge by inserting a new block and append SpillFill into it.
            resolver = src_bb->InsertNewBlockToSuccEdge(target_bb);
            // Fix Dominators info
            auto &dom_tree = graph_->GetAnalysis<DominatorsTree>();
            dom_tree.UpdateAfterResolverInsertion(src_bb, target_bb, resolver);
            graph_->InvalidateAnalysis<LoopAnalyzer>();
        }
    }
    auto spill_fill = CreateSpillFillForSplitMove(resolver);
    ConnectIntervals(spill_fill, src_split, target_split);
}

SpillFillInst *SplitResolver::CreateSpillFillForSplitMove(BasicBlock *source_block)
{
    auto iter = source_block->InstsReverse().begin();
    while (iter != iter.end() && (*iter)->IsControlFlow() && !(*iter)->IsPhi()) {
        ++iter;
    }

    if (iter == iter.end()) {
        auto spill_fill = graph_->CreateInstSpillFill();
        spill_fill->SetSpillFillType(SpillFillType::SPLIT_MOVE);
        source_block->PrependInst(spill_fill);
        return spill_fill;
    }

    auto inst = *iter;
    // Don't reuse CONNECT_SPLIT_SIBLINGS SpillFills to avoid insertion of two opposite spill fill
    // moves in case when an interval was split after last basic block's instruction and then
    // it should moved back to original location for successor block:
    //
    //  BB0:
    //    2. Add v0, v1 -> r0
    //  BB1:
    //    4. Sub v0, v2 -> ...
    //    <Split 2's interval and spill to stack slot s0>
    //    <jump to BB1>
    //
    // Without CONNECT_SPLIT_SIBLINGS single SpillFillInst would be inserted at the end of BB1
    // with following moves: r0 -> s0 (to connect siblings), s0 -> r0 (as 2 has different location
    // at the beginning of BB1). RegAllocResolver may not handle such moves so it should be
    // avoided.
    if (inst->IsSpillFill() && !Is<SpillFillType::CONNECT_SPLIT_SIBLINGS>(inst)) {
        return inst->CastToSpillFill();
    }

    ASSERT(!inst->IsPhi());
    auto spill_fill = graph_->CreateInstSpillFill();
    spill_fill->SetSpillFillType(SpillFillType::SPLIT_MOVE);
    source_block->InsertAfter(spill_fill, inst);
    return spill_fill;
}

SpillFillInst *SplitResolver::CreateSpillFillForSiblings(Inst *connect_at)
{
    // Try to reuse existing CONNECT_SPLIT_SIBLINGS spill-fill-inst
    auto prev = connect_at->GetPrev();
    while (prev != nullptr && prev->IsSpillFill()) {
        if (Is<SpillFillType::CONNECT_SPLIT_SIBLINGS>(prev)) {
            return prev->CastToSpillFill();
        }
        ASSERT(Is<SpillFillType::INPUT_FILL>(prev));
        connect_at = prev;
        prev = prev->GetPrev();
    }
    auto spill_fill = graph_->CreateInstSpillFill();
    spill_fill->SetSpillFillType(SpillFillType::CONNECT_SPLIT_SIBLINGS);
    connect_at->InsertBefore(spill_fill);
    return spill_fill;
}

}  // namespace panda::compiler
