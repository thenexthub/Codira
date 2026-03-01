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

#include "basicblock.h"
#include "graph.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/dominators_tree.h"

namespace panda::compiler {
class Inst;
BasicBlock::BasicBlock(Graph *graph, uint32_t guest_pc)
    : graph_(graph),
      preds_(graph_->GetAllocator()->Adapter()),
      succs_(graph_->GetAllocator()->Adapter()),
      dom_blocks_(graph_->GetAllocator()->Adapter()),
      guest_pc_(guest_pc)
{
}

bool BasicBlock::IsStartBlock() const
{
    return (graph_->GetStartBlock() == this);
}
bool BasicBlock::IsEndBlock() const
{
    return (graph_->GetEndBlock() == this);
}
bool BasicBlock::IsPseudoControlFlowBlock() const
{
    return IsStartBlock() || IsEndBlock() || IsTryBegin() || IsTryEnd() || IsCatchBegin();
}

bool BasicBlock::IsLoopHeader() const
{
    ASSERT(GetLoop() != nullptr);
    return (GetLoop()->GetHeader() == this);
}

BasicBlock *BasicBlock::SplitBlockAfterInstruction(Inst *inst, bool make_edge)
{
    ASSERT(inst != nullptr);
    ASSERT(inst->GetBasicBlock() == this);
    ASSERT(!IsStartBlock() && !IsEndBlock());

    auto next_inst = inst->GetNext();
    auto new_bb = GetGraph()->CreateEmptyBlock((next_inst != nullptr) ? next_inst->GetPc() : INVALID_PC);
    new_bb->SetAllFields(this->GetAllFields());
    GetLoop()->AppendBlock(new_bb);

    for (; next_inst != nullptr; next_inst = next_inst->GetNext()) {
        new_bb->AppendInst(next_inst);
    }
    inst->SetNext(nullptr);
    last_inst_ = inst;
    if (inst->IsPhi()) {
        first_inst_ = nullptr;
    }
    for (auto succ : GetSuccsBlocks()) {
        succ->ReplacePred(this, new_bb);
    }
    GetSuccsBlocks().clear();

    ASSERT(GetSuccsBlocks().empty());
    if (make_edge) {
        AddSucc(new_bb);
    }
    return new_bb;
}

void BasicBlock::AddSucc(BasicBlock *succ, bool can_add_empty_block)
{
    auto it = std::find(succs_.begin(), succs_.end(), succ);
    ASSERT_PRINT(it == succs_.end() || can_add_empty_block, "Uncovered case where empty block needed to fix CFG");
    if (it != succs_.end() && can_add_empty_block) {
        // If edge already exists we create empty block on it
        auto empty_bb = GetGraph()->CreateEmptyBlock(GetGuestPc());
        ReplaceSucc(succ, empty_bb);
        succ->ReplacePred(this, empty_bb);
    }
    succs_.push_back(succ);
    succ->GetPredsBlocks().push_back(this);
}

void BasicBlock::ReplaceSucc(const BasicBlock *prev_succ, BasicBlock *new_succ, bool can_add_empty_block)
{
    auto it = std::find(succs_.begin(), succs_.end(), new_succ);
    ASSERT_PRINT(it == succs_.end() || can_add_empty_block, "Uncovered case where empty block needed to fix CFG");
    if (it != succs_.end() && can_add_empty_block) {
        // If edge already exists we create empty block on it
        auto empty_bb = GetGraph()->CreateEmptyBlock(GetGuestPc());
        ReplaceSucc(new_succ, empty_bb);
        new_succ->ReplacePred(this, empty_bb);
    }
    succs_[GetSuccBlockIndex(prev_succ)] = new_succ;
    new_succ->preds_.push_back(this);
}

BasicBlock *BasicBlock::InsertNewBlockToSuccEdge(BasicBlock *succ)
{
    auto block = GetGraph()->CreateEmptyBlock(succ->GetGuestPc());
    this->ReplaceSucc(succ, block);
    succ->ReplacePred(this, block);
    return block;
}

BasicBlock *BasicBlock::InsertEmptyBlockBefore()
{
    auto block = GetGraph()->CreateEmptyBlock(this->GetGuestPc());
    for (auto pred : preds_) {
        pred->ReplaceSucc(this, block);
        this->RemovePred(pred);
    }
    block->AddSucc(this);
    return block;
}

void BasicBlock::InsertBlockBeforeSucc(BasicBlock *block, BasicBlock *succ)
{
    this->ReplaceSucc(succ, block);
    succ->ReplacePred(this, block);
}

static void RemovePhiProcessing(BasicBlock *bb, BasicBlock *succ)
{
    size_t num_preds = bb->GetPredsBlocks().size();

    for (auto phi : succ->PhiInsts()) {
        auto index = phi->CastToPhi()->GetPredBlockIndex(bb);
        auto inst = phi->GetInput(index).GetInst();
        if (inst->GetBasicBlock() == bb) {  // When INST is from empty basic block ...
            ASSERT(inst->IsPhi());
            // ... we have to copy it's inputs into corresponding inputs of PHI
            auto pred_bb = bb->GetPredBlockByIndex(0);
            phi->SetInput(index, inst->CastToPhi()->GetPhiInput(pred_bb));
            for (size_t i = 1; i < num_preds; i++) {
                pred_bb = bb->GetPredBlockByIndex(i);
                phi->AppendInput(inst->CastToPhi()->GetPhiInput(pred_bb));
            }
        } else {  // otherwise, just copy inputs for new arrived predecessors
            for (size_t i = 1; i < num_preds; i++) {
                phi->AppendInput(inst);
            }
        }
    }
    // And now we should remove Phis from the empty block
    for (auto phi : bb->PhiInstsSafe()) {
        bb->RemoveInst(phi);
    }
}

// Remove empty block with one successor, may have more than one predecessors and Phi(s)
void BasicBlock::RemoveEmptyBlock(bool irr_loop)
{
    ASSERT(GetFirstInst() == nullptr);
    ASSERT(!GetPredsBlocks().empty());
    ASSERT(GetSuccsBlocks().size() == 1);
    auto succ = succs_[0];

    // Save old amount of predecessors in successor block
    size_t succ_preds_num = succ->GetPredsBlocks().size();

    size_t num_preds = preds_.size();
    // If empty block had more than one predecessors
    if (num_preds > 1) {
        if (succ_preds_num > 1) {
            // We have to process Phi instructions in successor block in a special way
            RemovePhiProcessing(this, succ);
        } else {  // successor didn't have other predecessors, we are moving Phi(s) into successor
            ASSERT(!succ->HasPhi());
            for (auto phi : PhiInstsSafe()) {
                succ->AppendPhi(phi);
            }
            first_phi_ = nullptr;
            last_inst_ = nullptr;
        }
    }

    // Set successor for all predecessor blocks, at the same time in successor we replace one predecessor
    // and add others to the end (if there were many of them in empty block)
    auto pred = preds_[0];
    pred->succs_[pred->GetSuccBlockIndex(this)] = succ;
    succ->preds_[succ->GetPredBlockIndex(this)] = pred;
    for (size_t i = 1; i < num_preds; ++i) {
        pred = preds_[i];
        pred->succs_[pred->GetSuccBlockIndex(this)] = succ;
        succ->preds_.push_back(pred);
    }

    ASSERT(GetLastInst() == nullptr);
    ASSERT(GetLoop()->IsIrreducible() == irr_loop);
    // N.B. info about Irreducible loop can not be fixed on the fly
    if (!irr_loop) {
        RemoveFixLoopInfo();
    }
    // Finally clean lists
    preds_.clear();
    succs_.clear();
}

static void FixLoopInfoHelper(BasicBlock *bb)
{
    ASSERT(!bb->GetPredsBlocks().empty());
    auto loop = bb->GetLoop();
    auto first_pred = bb->GetPredBlockByIndex(0);
    // Do not dup back-edge
    if (loop->HasBackEdge(first_pred)) {
        loop->RemoveBackEdge(bb);
    } else {
        loop->ReplaceBackEdge(bb, first_pred);
    }
    // If empty block has more than 1 predecessor, append others to the loop back-edges' list
    for (size_t i = 1; i < bb->GetPredsBlocks().size(); ++i) {
        auto pred = bb->GetPredBlockByIndex(i);
        if (!loop->HasBackEdge(pred)) {
            loop->AppendBackEdge(pred);
        }
    }
}

void BasicBlock::RemoveFixLoopInfo()
{
    auto loop = GetLoop();
    ASSERT(loop != nullptr);
    ASSERT(!loop->IsIrreducible());
    while (!loop->IsRoot()) {
        if (loop->HasBackEdge(this)) {
            FixLoopInfoHelper(this);
        }
        loop = loop->GetOuterLoop();
    }
    if (this == GetLoop()->GetHeader()) {
        GetLoop()->MoveHeaderToSucc();
    }
    GetLoop()->RemoveBlock(this);
}

/**
 * Join single successor into single predecessor.
 * Block must have one successor, and its successor must have one predecessor (this block).
 * EXAMPLE:
 *              [1]
 *               |
 *              [2]
 *
 * turns into this:
 *              [1']
 */
void BasicBlock::JoinSuccessorBlock()
{
    ASSERT(!IsStartBlock());

    ASSERT(GetSuccsBlocks().size() == 1);
    auto succ = GetSuccessor(0);
    ASSERT(!succ->IsEndBlock());

    ASSERT(succ->GetPredsBlocks().size() == 1);
    ASSERT(succ->GetPredBlockByIndex(0) == this);

    // moving instructions from successor
    ASSERT(!succ->HasPhi());
    for (auto succ_inst : succ->Insts()) {
        AppendInst(succ_inst);
    }

    // moving successor blocks from the successor
    GetSuccsBlocks().clear();
    for (auto succ_succ : succ->GetSuccsBlocks()) {
        succ_succ->ReplacePred(succ, this);
    }

    // fixing loop information
    // invariant: succ has one predecessor, so it cannot be a loop header
    auto loop = succ->GetLoop();
    ASSERT(loop != nullptr);
    // Irreducible loop can not be fixed on the fly
    if (loop->IsIrreducible()) {
        GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    } else {
        // edge can have 2 successors, so it can be back-edge in 2 loops: own loop and outer loop
        if (loop->HasBackEdge(succ)) {
            loop->ReplaceBackEdge(succ, this);
        }
        if (auto outer_loop = loop->GetOuterLoop()) {
            if (outer_loop->HasBackEdge(succ)) {
                outer_loop->ReplaceBackEdge(succ, this);
            }
        }

        for (auto inner_loop : loop->GetInnerLoops()) {
            if (inner_loop->GetPreHeader() == succ) {
                inner_loop->SetPreHeader(this);
            }
        }
        loop->RemoveBlock(succ);
    }

    succ->first_inst_ = nullptr;
    succ->last_inst_ = nullptr;
    succ->GetPredsBlocks().clear();
    succ->GetSuccsBlocks().clear();

    this->bit_fields_ |= succ->bit_fields_;
    // TODO (a.popov) replace by assert
    if (succ->try_id_ != INVALID_ID) {
        this->try_id_ = succ->try_id_;
    }
    GetGraph()->RemoveEmptyBlock(succ);
}

void BasicBlock::SelectsFixLoopInfo(BasicBlock *select_bb, BasicBlock *other)
{
    // invariant: 'this' block has one predecessor, so it cannot be a loop header
    auto loop = GetLoop();
    ASSERT(loop != nullptr);
    // Irreducible loop can not be fixed on the fly
    if (loop->IsIrreducible()) {
        GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    } else {
        if (loop->HasBackEdge(this)) {
            loop->RemoveBackEdge(this);
        }
        for (auto inner_loop : loop->GetInnerLoops()) {
            if (inner_loop->GetPreHeader() == this) {
                inner_loop->SetPreHeader(select_bb);
                break;
            }
        }
        loop->RemoveBlock(this);
    }

    // Remove outgoing 'this->other' edge
    RemoveSucc(other);
    other->RemovePred(this);
    // Disconnect
    GetGraph()->RemoveEmptyBlock(this);
}

void BasicBlock::AppendPhi(Inst *inst)
{
    ASSERT_PRINT(inst->IsPhi(), "Instruction must be phi");
    inst->SetBasicBlock(this);
    if (first_phi_ == nullptr) {
        inst->SetNext(first_inst_);
        if (first_inst_ != nullptr) {
            first_inst_->SetPrev(inst);
        }
        first_phi_ = inst;
        if (last_inst_ == nullptr) {
            last_inst_ = inst;
        }
    } else {
        if (first_inst_ != nullptr) {
            Inst *prev = first_inst_->GetPrev();
            ASSERT_PRINT(prev && prev->IsPhi(), "There is no phi in the block");
            inst->SetPrev(prev);
            prev->SetNext(inst);
            inst->SetNext(first_inst_);
            first_inst_->SetPrev(inst);
        } else {
            ASSERT_PRINT(last_inst_ && last_inst_->IsPhi(),
                         "If first_phi is defined and first_inst is undefined, last_inst must be phi");
            last_inst_->SetNext(inst);
            inst->SetPrev(last_inst_);
            last_inst_ = inst;
        }
    }
}

template <bool to_end>
void BasicBlock::AddInst(Inst *inst)
{
    ASSERT_PRINT(!inst->IsPhi(), "Instruction mustn't be phi");
    inst->SetBasicBlock(this);
    if (first_inst_ == nullptr) {
        inst->SetPrev(last_inst_);
        if (last_inst_ != nullptr) {
            ASSERT(last_inst_->IsPhi());
            last_inst_->SetNext(inst);
        }
        first_inst_ = inst;
        last_inst_ = inst;
    } else {
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (to_end) {
            ASSERT_PRINT(last_inst_, "Last instruction is undefined");
            inst->SetPrev(last_inst_);
            last_inst_->SetNext(inst);
            last_inst_ = inst;
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            auto first_prev = first_inst_->GetPrev();
            if (first_prev != nullptr) {
                first_prev->SetNext(inst);
            }
            inst->SetPrev(first_prev);
            inst->SetNext(first_inst_);
            first_inst_->SetPrev(inst);
            first_inst_ = inst;
        }
    }
}

void BasicBlock::AppendRangeInst(Inst *range_first, Inst *range_last)
{
#ifndef NDEBUG
    ASSERT(range_first && range_last && range_first->IsDominate(range_last));
    ASSERT(range_first->GetPrev() == nullptr);
    ASSERT(range_last->GetNext() == nullptr);
    auto inst_db = range_first;
    while (inst_db != range_last) {
        ASSERT_PRINT(!inst_db->IsPhi(), "Instruction mustn't be phi");
        ASSERT_PRINT(inst_db->GetBasicBlock() == this, "Inst::SetBasicBlock() should be called beforehand");
        inst_db = inst_db->GetNext();
    }
    ASSERT_PRINT(!inst_db->IsPhi(), "Instruction mustn't be phi");
    ASSERT_PRINT(inst_db->GetBasicBlock() == this, "Inst::SetBasicBlock() should be called beforehand");
#endif

    if (first_inst_ == nullptr) {
        range_first->SetPrev(last_inst_);
        if (last_inst_ != nullptr) {
            ASSERT(last_inst_->IsPhi());
            last_inst_->SetNext(range_first);
        }
        first_inst_ = range_first;
        last_inst_ = range_last;
    } else {
        ASSERT_PRINT(last_inst_, "Last instruction is undefined");
        range_first->SetPrev(last_inst_);
        last_inst_->SetNext(range_first);
        last_inst_ = range_last;
    }
}

void BasicBlock::InsertAfter(Inst *inst, Inst *after)
{
    ASSERT(inst && after);
    ASSERT(inst->IsPhi() == after->IsPhi());
    ASSERT(after->GetBasicBlock() == this);
    ASSERT(inst->GetBasicBlock() == nullptr);
    inst->SetBasicBlock(this);
    Inst *next = after->GetNext();
    inst->SetPrev(after);
    inst->SetNext(next);
    after->SetNext(inst);
    if (next != nullptr) {
        next->SetPrev(inst);
    } else {
        ASSERT(after == last_inst_);
        last_inst_ = inst;
    }
}

void BasicBlock::InsertBefore(Inst *inst, Inst *before)
{
    ASSERT(inst && before);
    ASSERT(inst->IsPhi() == before->IsPhi());
    ASSERT(before->GetBasicBlock() == this);
    ASSERT(inst->GetBasicBlock() == nullptr);
    inst->SetBasicBlock(this);
    Inst *prev = before->GetPrev();
    inst->SetPrev(prev);
    inst->SetNext(before);
    before->SetPrev(inst);
    if (prev != nullptr) {
        prev->SetNext(inst);
    }
    if (before == first_phi_) {
        first_phi_ = inst;
    }
    if (before == first_inst_) {
        first_inst_ = inst;
    }
}

void BasicBlock::InsertRangeBefore(Inst *range_first, Inst *range_last, Inst *before)
{
#ifndef NDEBUG
    ASSERT(range_first && range_last && range_first->IsDominate(range_last));
    ASSERT(before && !before->IsPhi());
    ASSERT(range_first->GetPrev() == nullptr);
    ASSERT(range_last->GetNext() == nullptr);
    ASSERT(before->GetBasicBlock() == this);
    auto inst_db = range_first;
    while (inst_db != range_last) {
        ASSERT_PRINT(!inst_db->IsPhi(), "Instruction mustn't be phi");
        ASSERT_PRINT(inst_db->GetBasicBlock() == this, "Inst::SetBasicBlock() should be called beforehand");
        inst_db = inst_db->GetNext();
    }
    ASSERT_PRINT(!inst_db->IsPhi(), "Instruction mustn't be phi");
    ASSERT_PRINT(inst_db->GetBasicBlock() == this, "Inst::SetBasicBlock() should be called beforehand");
#endif

    Inst *prev = before->GetPrev();
    range_first->SetPrev(prev);
    range_last->SetNext(before);
    before->SetPrev(range_last);
    if (prev != nullptr) {
        prev->SetNext(range_first);
    }
    if (before == first_inst_) {
        first_inst_ = range_first;
    }
}

void BasicBlock::ReplaceInst(Inst *old_inst, Inst *new_inst)
{
    ASSERT(old_inst && new_inst);
    ASSERT(old_inst->IsPhi() == new_inst->IsPhi());
    ASSERT(old_inst->GetBasicBlock() == this);
    ASSERT(new_inst->GetBasicBlock() == nullptr);
    new_inst->SetBasicBlock(this);
    Inst *prev = old_inst->GetPrev();
    Inst *next = old_inst->GetNext();

    old_inst->SetBasicBlock(nullptr);
    if (prev != nullptr) {
        prev->SetNext(new_inst);
    }
    if (next != nullptr) {
        next->SetPrev(new_inst);
    }
    new_inst->SetPrev(prev);
    new_inst->SetNext(next);
    if (first_phi_ == old_inst) {
        first_phi_ = new_inst;
    }
    if (first_inst_ == old_inst) {
        first_inst_ = new_inst;
    }
    if (last_inst_ == old_inst) {
        last_inst_ = new_inst;
    }

    if (graph_->IsInstThrowable(old_inst)) {
        graph_->ReplaceThrowableInst(old_inst, new_inst);
    }
}

void BasicBlock::EraseInst(Inst *inst, [[maybe_unused]] bool will_be_moved)
{
    ASSERT(will_be_moved || !GetGraph()->IsInstThrowable(inst));
    Inst *prev = inst->GetPrev();
    Inst *next = inst->GetNext();

    ASSERT(inst->GetBasicBlock() == this);
    inst->SetBasicBlock(nullptr);
    if (prev != nullptr) {
        prev->SetNext(next);
    }
    if (next != nullptr) {
        next->SetPrev(prev);
    }
    inst->SetPrev(nullptr);
    inst->SetNext(nullptr);
    if (inst == first_phi_) {
        first_phi_ = (next != nullptr && next->IsPhi()) ? next : nullptr;
    }
    if (inst == first_inst_) {
        first_inst_ = next;
    }
    if (inst == last_inst_) {
        last_inst_ = prev;
    }
}

void BasicBlock::RemoveInst(Inst *inst)
{
    inst->RemoveUsers();
    ASSERT(!inst->HasUsers());
    inst->RemoveInputs();
    if (inst->GetOpcode() == Opcode::Constant) {
        graph_->RemoveConstFromList(static_cast<ConstantInst *>(inst));
    }

    if (graph_->IsInstThrowable(inst)) {
        graph_->RemoveThrowableInst(inst);
    }
    EraseInst(inst);
}

void BasicBlock::Clear()
{
    for (auto inst : AllInstsSafeReverse()) {
        RemoveInst(inst);
    }
}

/*
 * Check if this block is dominate other
 */
bool BasicBlock::IsDominate(const BasicBlock *other) const
{
    if (other == this) {
        return true;
    }
    ASSERT(GetGraph()->IsAnalysisValid<DominatorsTree>());
    BasicBlock *dom_block = other->GetDominator();
    while (dom_block != nullptr) {
        if (dom_block == this) {
            return true;
        }
        // Otherwise we are in infinite loop!?
        ASSERT(dom_block != dom_block->GetDominator());
        dom_block = dom_block->GetDominator();
    }
    return false;
}

BasicBlock *BasicBlock::CreateImmediateDominator()
{
    ASSERT(GetGraph()->IsAnalysisValid<DominatorsTree>());
    auto dominator = GetGraph()->CreateEmptyBlock();
    GetGraph()->GetAnalysis<DominatorsTree>().SetValid(true);
    if (GetDominator() != nullptr) {
        GetDominator()->RemoveDominatedBlock(this);
        GetDominator()->AddDominatedBlock(dominator);
        dominator->SetDominator(GetDominator());
    }
    dominator->AddDominatedBlock(this);
    SetDominator(dominator);
    return dominator;
}

BasicBlock *BasicBlock::GetDominator() const
{
    ASSERT(GetGraph()->IsAnalysisValid<DominatorsTree>());
    return dominator_;
}

const ArenaVector<BasicBlock *> &BasicBlock::GetDominatedBlocks() const
{
    ASSERT(GetGraph()->IsAnalysisValid<DominatorsTree>());
    return dom_blocks_;
}

PhiInstIter BasicBlock::PhiInsts() const
{
    return PhiInstIter(*this);
}
InstIter BasicBlock::Insts() const
{
    return InstIter(*this);
}
AllInstIter BasicBlock::AllInsts() const
{
    return AllInstIter(*this);
}

InstReverseIter BasicBlock::InstsReverse() const
{
    return InstReverseIter(*this);
}

PhiInstSafeIter BasicBlock::PhiInstsSafe() const
{
    return PhiInstSafeIter(*this);
}
InstSafeIter BasicBlock::InstsSafe() const
{
    return InstSafeIter(*this);
}
AllInstSafeIter BasicBlock::AllInstsSafe() const
{
    return AllInstSafeIter(*this);
}

PhiInstSafeReverseIter BasicBlock::PhiInstsSafeReverse() const
{
    return PhiInstSafeReverseIter(*this);
}
InstSafeReverseIter BasicBlock::InstsSafeReverse() const
{
    return InstSafeReverseIter(*this);
}
AllInstSafeReverseIter BasicBlock::AllInstsSafeReverse() const
{
    return AllInstSafeReverseIter(*this);
}

template void BasicBlock::AddInst<false>(Inst *inst);
template void BasicBlock::AddInst<true>(Inst *inst);

void BasicBlock::InsertBlockBefore(BasicBlock *block)
{
    for (auto pred : GetPredsBlocks()) {
        pred->ReplaceSucc(this, block);
    }
    GetPredsBlocks().clear();
    block->AddSucc(this);
}

BasicBlock *BasicBlock::Clone(Graph *target_graph) const
{
    BasicBlock *clone = nullptr;
#ifndef NDEBUG
    if (GetGraph() == target_graph) {
        clone = target_graph->CreateEmptyBlock();
    } else {
        clone = target_graph->CreateEmptyBlock(GetId(), GetGuestPc());
    }
#else
    clone = target_graph->CreateEmptyBlock();
#endif
    clone->SetAllFields(this->GetAllFields());
    clone->try_id_ = GetTryId();
    if (this->IsStartBlock()) {
        target_graph->SetStartBlock(clone);
    } else if (this->IsEndBlock()) {
        target_graph->SetEndBlock(clone);
    }
    return clone;
}

Inst *BasicBlock::GetFistThrowableInst() const
{
    if (!IsTry()) {
        return nullptr;
    }
    for (auto inst : AllInsts()) {
        if (GetGraph()->IsInstThrowable(inst)) {
            return inst;
        }
    }
    return nullptr;
}

void BasicBlock::InvalidateLoopIfIrreducible()
{
    auto loop = GetLoop();
    ASSERT(loop != nullptr);
    if (IsLoopHeader() && loop->IsIrreducible()) {
        GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    }
}

bool BlocksPathDfsSearch(Marker marker, BasicBlock *block, const BasicBlock *target_block,
                         const BasicBlock *exclude_block)
{
    ASSERT(marker != UNDEF_MARKER);
    if (block == target_block) {
        return true;
    }
    block->SetMarker(marker);

    for (auto succ_block : block->GetSuccsBlocks()) {
        if (!succ_block->IsMarked(marker) && succ_block != exclude_block) {
            if (BlocksPathDfsSearch(marker, succ_block, target_block, exclude_block)) {
                return true;
            }
        }
    }
    return false;
}
}  // namespace panda::compiler
