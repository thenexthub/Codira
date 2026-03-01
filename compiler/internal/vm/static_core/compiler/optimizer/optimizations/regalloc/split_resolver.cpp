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

#include "split_resolver.h"
#include "compiler/optimizer/ir/inst.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/analysis/dominators_tree.h"
#include "compiler/optimizer/analysis/loop_analyzer.h"
#include "compiler/optimizer/optimizations/regalloc/reg_alloc_base.h"

namespace ark::compiler {

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

        auto spillFill = CreateSpillFillForSiblings(inst);
        ConnectIntervals(spillFill, prev, curr);
    }
}

void SplitResolver::ProcessBlock(BasicBlock *block)
{
    auto succBegin = liveness_->GetBlockLiveRange(block).GetBegin();
    for (auto interval : liveness_->GetLifeIntervals()) {
        // PHI and its inputs can be considered as one interval, which was split,
        // so that the logic is equivalent to the logic of connecting split intervals
        if (interval->GetBegin() == succBegin) {
            auto phi = interval->GetInst();
            if (!phi->IsPhi() || phi->GetDstReg() == GetAccReg()) {
                continue;
            }
            ASSERT(phi->IsPhi());
            for (size_t i = 0; i < phi->GetInputsCount(); i++) {
                auto inputInst = phi->GetDataFlowInput(i);
                auto inputBb = phi->CastToPhi()->GetPhiInputBb(i);
                auto inputLiveness = liveness_->GetInstLifeIntervals(inputInst);
                ConnectSplitFromPredBlock(inputBb, inputLiveness, block, interval);
            }
            continue;
        }

        // Skip not-splitted instruction
        if (interval->GetSibling() == nullptr) {
            continue;
        }
        ASSERT(!interval->IsPhysical());
        auto succSplit = interval->FindSiblingAt(succBegin);
        if (succSplit == nullptr || succSplit->GetLocation().IsConstant() || !succSplit->SplitCover(succBegin)) {
            continue;
        }
        for (auto pred : block->GetPredsBlocks()) {
            ConnectSplitFromPredBlock(pred, interval, block, succSplit);
        }
    }
}

void SplitResolver::ConnectSplitFromPredBlock(BasicBlock *srcBb, LifeIntervals *srcInterval, BasicBlock *targetBb,
                                              LifeIntervals *targetSplit)
{
    BasicBlock *resolver {nullptr};
    // It's a resolver block inserted during register allocation
    if (srcBb->GetId() >= liveness_->GetBlocksCount()) {
        ASSERT(srcBb->GetSuccsBlocks().size() == 1 && srcBb->GetPredsBlocks().size() == 1);
        resolver = srcBb;
        srcBb = srcBb->GetPredecessor(0);
    }
    auto srcLiveness = liveness_->GetBlockLiveRange(srcBb);
    // Find sibling at the 'end - LIFE_NUMBER_GAP' position to connect siblings that were split at the end of the
    // 'src_bb'
    auto srcEndPos = srcLiveness.GetEnd() - 1U;
    auto srcSplit = srcInterval->FindSiblingAt(srcEndPos);
    // Instruction was not defined at predecessor or has the same location there
    if (srcSplit == nullptr || srcSplit->GetLocation() == targetSplit->GetLocation() ||
        !srcSplit->SplitCover<true>(srcEndPos)) {
        return;
    }

    COMPILER_LOG(DEBUG, SPLIT_RESOLVER) << "Resolve split move for inst v" << srcInterval->GetInst()->GetId()
                                        << " between blocks: BB" << srcBb->GetId() << " -> BB" << targetBb->GetId();

    if (resolver == nullptr) {
        if (srcBb->GetSuccsBlocks().size() == 1U) {
            resolver = srcBb;
        } else {
            // Get rid of critical edge by inserting a new block and append SpillFill into it.
            resolver = srcBb->InsertNewBlockToSuccEdge(targetBb);
            // Fix Dominators info
            auto &domTree = graph_->GetAnalysis<DominatorsTree>();
            domTree.UpdateAfterResolverInsertion(srcBb, targetBb, resolver);
            graph_->InvalidateAnalysis<LoopAnalyzer>();
        }
    }
    auto spillFill = CreateSpillFillForSplitMove(resolver);
    ConnectIntervals(spillFill, srcSplit, targetSplit);
}

SpillFillInst *SplitResolver::CreateSpillFillForSplitMove(BasicBlock *sourceBlock)
{
    auto iter = sourceBlock->InstsReverse().begin();
    while (iter != iter.end() && (*iter)->IsControlFlow() && !(*iter)->IsPhi()) {
        ++iter;
    }

    if (iter == iter.end()) {
        auto spillFill = graph_->CreateInstSpillFill(SpillFillType::SPLIT_MOVE);
        sourceBlock->PrependInst(spillFill);
        return spillFill;
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
    auto spillFill = graph_->CreateInstSpillFill(SpillFillType::SPLIT_MOVE);
    sourceBlock->InsertAfter(spillFill, inst);
    return spillFill;
}

SpillFillInst *SplitResolver::CreateSpillFillForSiblings(Inst *connectAt)
{
    // Try to reuse existing CONNECT_SPLIT_SIBLINGS spill-fill-inst
    auto prev = connectAt->GetPrev();
    while (prev != nullptr && prev->IsSpillFill()) {
        if (Is<SpillFillType::CONNECT_SPLIT_SIBLINGS>(prev)) {
            return prev->CastToSpillFill();
        }
        ASSERT(Is<SpillFillType::INPUT_FILL>(prev));
        connectAt = prev;
        prev = prev->GetPrev();
    }
    auto spillFill = graph_->CreateInstSpillFill(SpillFillType::CONNECT_SPLIT_SIBLINGS);
    connectAt->InsertBefore(spillFill);
    return spillFill;
}

}  // namespace ark::compiler
