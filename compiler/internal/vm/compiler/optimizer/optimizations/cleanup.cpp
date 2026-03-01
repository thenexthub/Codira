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

#include "cleanup.h"

#include "compiler_logger.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/basicblock.h"

namespace panda::compiler {

static bool SkipBasicBlock(BasicBlock *bb)
{
    // TODO (a.popov) Make empty catch-begin and try-end blocks removeable
    return bb == nullptr || bb->IsStartBlock() || bb->IsEndBlock() || bb->IsCatchBegin() || bb->IsTryEnd();
}

/* Cleanup pass works like dead code elimination (DCE) and removes code which does not affect the program results.
 * It also removes empty basic blocks when it is possible and merges a linear basic block sequence to one bigger
 * basic block, thus simplifying control flow graph.
 */
bool Cleanup::RunImpl()
{
    GetGraph()->RunPass<DominatorsTree>();
    GetGraph()->RunPass<LoopAnalyzer>();

    // Two vectors to store basic blocks lists
    auto empty_blocks = &empty1_;
    auto new_empty_blocks = &empty2_;

    bool modified = PhiChecker();

    for (auto bb : GetGraph()->GetVectorBlocks()) {
        if (!SkipBasicBlock(bb) && bb->IsEmpty()) {
            empty_blocks->insert(bb);
        }
    }

    bool first_run = true;
    do {
        modified |= RunOnce(empty_blocks, new_empty_blocks, !first_run);
        first_run = false;
        // Swap vectors pointers
        auto temp = empty_blocks;
        empty_blocks = new_empty_blocks;
        new_empty_blocks = temp;
        // Clean the "new" list
        new_empty_blocks->clear();
    } while (!empty_blocks->empty());

    empty1_.clear();
    empty2_.clear();

    /* Merge linear sectors.
     * For merging possibility a block must have one successor, and its successor must have one predecessor.
     * EXAMPLE:
     *              [1]
     *               |
     *              [2]
     *               |
     *              [3]
     *               |
     *              [4]
     *
     * turns into this:
     *              [1']
     */
    for (auto bb : GetGraph()->GetVectorBlocks()) {
        if (bb == nullptr || bb->IsPseudoControlFlowBlock()) {
            continue;
        }

        while (bb->GetSuccsBlocks().size() == 1 && bb->GetSuccessor(0)->GetPredsBlocks().size() == 1 &&
               !bb->GetSuccessor(0)->IsPseudoControlFlowBlock() && bb->IsTry() == bb->GetSuccessor(0)->IsTry() &&
               bb->GetTryId() == bb->GetSuccessor(0)->GetTryId()) {
            ASSERT(!bb->GetSuccessor(0)->HasPhi());
            COMPILER_LOG(DEBUG, CLEANUP) << "Merged block " << bb->GetSuccessor(0)->GetId() << " into " << bb->GetId();
            bb->JoinSuccessorBlock();
            modified = true;
        }
    }
    return modified;
}

bool Cleanup::RunOnce(ArenaSet<BasicBlock *> *empty_blocks, ArenaSet<BasicBlock *> *new_empty_blocks, bool simple_dce)
{
    bool modified = false;
    auto marker_holder = MarkerHolder(GetGraph());
    auto dead_mrk = marker_holder.GetMarker();

    // Check all basic blocks in "old" list
    for (auto bb : *empty_blocks) {
        // In some tricky cases block may be already deleted in previous iteration
        if (bb->GetGraph() == nullptr) {
            continue;
        }

        auto succ = bb->GetSuccessor(0);
        // Strange infinite loop with only one empty block, or loop pre-header - lets bail out
        if (succ == bb || succ->GetLoop()->GetPreHeader() == bb) {
            continue;
        }

#ifndef NDEBUG
        // Now we know that 'bb' is not a loop pre-header, so if both 'bb' and 'succ' have many predecessors
        // all 'bb' Phi(s) must have user only in successor Phis
        if (succ->GetPredsBlocks().size() > 1) {
            for (auto phi : bb->PhiInsts()) {
                for (auto &user_item : phi->GetUsers()) {
                    auto user = user_item.GetInst();
                    ASSERT((user->GetBasicBlock() == succ && user->IsPhi()) || user->IsCatchPhi());
                }
            }
        }
#endif
        modified |= ProcessBB(bb, dead_mrk, new_empty_blocks);
    }

    if (simple_dce) {
        modified |= SimpleDce(dead_mrk, new_empty_blocks);
    } else {
        modified |= Dce(dead_mrk, new_empty_blocks);
    }
    dead_.clear();

    return modified;
}

// Check around for special triangle case
bool Cleanup::CheckSpecialTriangle(BasicBlock *bb)
{
    auto succ = bb->GetSuccessor(0);
    size_t i = 0;
    for (auto pred : bb->GetPredsBlocks()) {
        if (pred->GetSuccessor(0) == succ ||
            (pred->GetSuccsBlocks().size() == MAX_SUCCS_NUM && pred->GetSuccessor(1) == succ)) {
            // Checking all Phis
            for (auto phi : succ->PhiInsts()) {
                size_t index_bb = phi->CastToPhi()->GetPredBlockIndex(bb);
                size_t index_pred = phi->CastToPhi()->GetPredBlockIndex(pred);
                ASSERT(index_bb != index_pred);

                auto inst_pred = phi->GetInput(index_pred).GetInst();
                auto inst_bb = phi->GetInput(index_bb).GetInst();
                // If phi input is in 'bb', check input of that phi instead
                if (inst_bb->GetBasicBlock() == bb) {
                    ASSERT(inst_bb->IsPhi());
                    inst_bb = inst_bb->CastToPhi()->GetInput(i).GetInst();
                }
                if (inst_bb != inst_pred) {
                    return true;
                }
            }
            // Would fully remove 'straight' pred->succ edge, and second one would stay after 'bb' removal
            saved_preds_.push_back(pred);
        }
        i++;
    }
    return false;
}

void Cleanup::RemoveDeadPhi(BasicBlock *bb, ArenaSet<BasicBlock *> *new_empty_blocks)
{
    for (auto phi : bb->PhiInstsSafe()) {
        if (!phi->GetUsers().Empty()) {
            continue;
        }
        bb->RemoveInst(phi);
        COMPILER_LOG(DEBUG, CLEANUP) << "Dead Phi removed " << phi->GetId();
        GetGraph()->GetEventWriter().EventCleanup(phi->GetId(), phi->GetPc());

        for (auto pred : bb->GetPredsBlocks()) {
            if (pred->IsEmpty() && !SkipBasicBlock(pred)) {
                COMPILER_LOG(DEBUG, CLEANUP) << "Would re-check empty block " << pred->GetId();
                new_empty_blocks->insert(pred);
            }
        }
    }
}

bool Cleanup::ProcessBB(BasicBlock *bb, Marker dead_mrk, ArenaSet<BasicBlock *> *new_empty_blocks)
{
    auto succ = bb->GetSuccessor(0);
    if (CheckSpecialTriangle(bb)) {
        return false;
    }
    // Remove dead Phi(s)
    RemoveDeadPhi(bb, new_empty_blocks);
    // Process saved predecessors
    for (auto pred : saved_preds_) {
        ASSERT(pred->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
        constexpr auto PREDS_BLOCK_NUM = 2;
        ASSERT(succ->GetPredsBlocks().size() >= PREDS_BLOCK_NUM);

        auto last = pred->GetLastInst();
        if (last->GetOpcode() == Opcode::If || last->GetOpcode() == Opcode::IfImm) {
            last->SetMarker(dead_mrk);
            dead_.push_back(last);
        } else {
            ASSERT(last->GetOpcode() == Opcode::Try);
        }
        pred->RemoveSucc(succ);
        if (succ->GetPredsBlocks().size() == PREDS_BLOCK_NUM) {
            for (auto phi : succ->PhiInstsSafe()) {
                auto rm_index = phi->CastToPhi()->GetPredBlockIndex(pred);
                auto remaining_inst = phi->GetInputs()[1 - rm_index].GetInst();
                phi->ReplaceUsers(remaining_inst);
                succ->RemoveInst(phi);
            }
        } else {  // more than 2 predecessors
            for (auto phi : succ->PhiInstsSafe()) {
                auto rm_index = phi->CastToPhi()->GetPredBlockIndex(pred);
                phi->CastToPhi()->RemoveInput(rm_index);
            }
        }
        succ->RemovePred(pred);
        // Fixing LoopAnalysis or DomTree is no necessary here, because there would be another edge
    }
    saved_preds_.clear();
    bool bad_loop = bb->GetLoop()->IsIrreducible();
    GetGraph()->RemoveEmptyBlockWithPhis(bb, bad_loop);
    if (bad_loop) {
        GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
        GetGraph()->RunPass<LoopAnalyzer>();
    }
    COMPILER_LOG(DEBUG, CLEANUP) << "Removed empty block: " << bb->GetId();
    return true;
}

// Mark instructions that have the NOT_REMOVABLE property
// and recursively mark all their inputs
void Cleanup::MarkLiveRec(Marker live_mrk, Inst *inst)
{
    // No recursion for one-input case, otherwise got stackoverflow on TSAN job
    bool marked = false;
    while (inst->GetInputsCount() == 1) {
        marked = inst->SetMarker(live_mrk);
        if (marked) {
            break;
        }
        inst = inst->GetInput(0).GetInst();
    }
    if (!marked && !inst->SetMarker(live_mrk)) {
        for (auto input : inst->GetInputs()) {
            MarkLiveRec(live_mrk, input.GetInst());
        }
    }
}

bool Cleanup::Dce(Marker dead_mrk, ArenaSet<BasicBlock *> *new_empty_blocks)
{
    bool modified = false;
    auto marker_holder = MarkerHolder(GetGraph());
    auto live_mrk = marker_holder.GetMarker();

    // Mark live instructions
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            if (inst->IsNotRemovable() && !inst->IsMarked(dead_mrk)) {
                MarkLiveRec(live_mrk, inst);
            }
        }
    }
    // Remove non-live instructions
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInstsSafe()) {
            if (inst->IsMarked(live_mrk)) {
                continue;
            }
            bool is_phi = inst->IsPhi();
            bb->RemoveInst(inst);
            COMPILER_LOG(DEBUG, CLEANUP) << "Dead instruction " << inst->GetId();
            GetGraph()->GetEventWriter().EventCleanup(inst->GetId(), inst->GetPc());
            modified = true;

            if (is_phi) {
                for (auto pred : bb->GetPredsBlocks()) {
                    if (pred->IsEmpty() && !SkipBasicBlock(pred)) {
                        COMPILER_LOG(DEBUG, CLEANUP) << "Would re-check empty block " << pred->GetId();
                        new_empty_blocks->insert(pred);
                    }
                }
            } else if (bb->IsEmpty() && !SkipBasicBlock(bb)) {
                COMPILER_LOG(DEBUG, CLEANUP) << "No more non-Phi instructions in block " << bb->GetId();
                new_empty_blocks->insert(bb);
            }
        }
    }
    return modified;
}

void Cleanup::SetLiveRec(Inst *inst, Marker mrk, Marker live_mrk)
{
    for (auto input_item : inst->GetInputs()) {
        auto input = input_item.GetInst();
        if (!input->IsMarked(live_mrk) && input->IsMarked(mrk)) {
            input->ResetMarker(mrk);
            input->SetMarker(live_mrk);
            SetLiveRec(input, mrk, live_mrk);
        }
    }
}

void Cleanup::LiveUserSearchRec(Inst *inst, Marker mrk, Marker live_mrk, Marker dead_mrk)
{
    ASSERT(!inst->IsMarked(mrk));
    ASSERT(!inst->IsMarked(dead_mrk));
    if (inst->IsMarked(live_mrk)) {
        SetLiveRec(inst, mrk, live_mrk);
        return;
    }
    if (inst->IsNotRemovable()) {
        inst->SetMarker(live_mrk);
        SetLiveRec(inst, mrk, live_mrk);
        return;
    }
    inst->SetMarker(mrk);
    temp_.push_back(inst);
    bool unknown = false;
    for (auto &user_item : inst->GetUsers()) {
        auto user = user_item.GetInst();
        if (user->IsMarked(mrk)) {
            unknown = true;
            continue;
        }
        if (user->IsMarked(dead_mrk)) {
            continue;
        }
        LiveUserSearchRec(user, mrk, live_mrk, dead_mrk);
        if (user->IsMarked(live_mrk)) {
            ASSERT(!inst->IsMarked(mrk) && inst->IsMarked(live_mrk));
            return;
        }
        ASSERT(inst->IsMarked(mrk));
        if (user->IsMarked(mrk)) {
            ASSERT(!user->IsMarked(live_mrk) && !user->IsMarked(dead_mrk));
            unknown = true;
        } else {
            ASSERT(user->IsMarked(dead_mrk));
        }
    }
    if (!unknown) {
        inst->ResetMarker(mrk);
        inst->SetMarker(dead_mrk);
        dead_.push_back(inst);
    }
}

void Cleanup::Marking(Marker dead_mrk, Marker mrk, Marker live_mrk)
{
    size_t i = 0;
    while (i < dead_.size()) {
        auto inst = dead_.at(i);
        for (auto input_item : inst->GetInputs()) {
            auto input = input_item.GetInst();
            if (input->IsMarked(dead_mrk) || input->IsMarked(mrk)) {
                continue;
            }
            LiveUserSearchRec(input, mrk, live_mrk, dead_mrk);
            for (auto temp : temp_) {
                if (temp->IsMarked(mrk)) {
                    ASSERT(!temp->IsMarked(live_mrk) && !temp->IsMarked(dead_mrk));
                    inst->ResetMarker(mrk);
                    inst->SetMarker(dead_mrk);
                    dead_.push_back(inst);
                }
            }
            temp_.clear();
        }
        i++;
    }
}

bool Cleanup::Removal(ArenaSet<BasicBlock *> *new_empty_blocks)
{
    bool modified = false;

    for (auto inst : dead_) {
        inst->ClearMarkers();
        auto bb = inst->GetBasicBlock();
        if (bb == nullptr) {
            continue;
        }
        bb->RemoveInst(inst);
        COMPILER_LOG(DEBUG, CLEANUP) << "Dead instruction " << inst->GetId();
        GetGraph()->GetEventWriter().EventCleanup(inst->GetId(), inst->GetPc());
        modified = true;

        if (inst->IsPhi()) {
            for (auto pred : bb->GetPredsBlocks()) {
                if (pred->IsEmpty() && !SkipBasicBlock(pred)) {
                    COMPILER_LOG(DEBUG, CLEANUP) << "Would re-check empty block " << pred->GetId();
                    new_empty_blocks->insert(pred);
                }
            }
        } else {
            if (bb->IsEmpty() && !SkipBasicBlock(bb)) {
                COMPILER_LOG(DEBUG, CLEANUP) << "No more non-Phi instructions in block " << bb->GetId();
                new_empty_blocks->insert(bb);
            }
        }
    }
    return modified;
}

bool Cleanup::SimpleDce(Marker dead_mrk, ArenaSet<BasicBlock *> *new_empty_blocks)
{
    auto marker_holder = MarkerHolder(GetGraph());
    auto mrk = marker_holder.GetMarker();
    auto live_marker_holder = MarkerHolder(GetGraph());
    auto live_mrk = live_marker_holder.GetMarker();

    // Step 1. Marking
    Marking(dead_mrk, mrk, live_mrk);

    // Step 2. Removal
    return Removal(new_empty_blocks);
}

void Cleanup::BuildDominators()
{
    size_t amount = 0;
    fake_root_ = reinterpret_cast<Inst *>(sizeof(Inst *));
    map_.insert({fake_root_, amount});
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->PhiInsts()) {
            amount++;
            map_.insert({inst, amount});
            for (auto input : inst->GetInputs()) {
                auto pred = input.GetInst();
                if (!pred->IsPhi() && map_.count(pred) == 0) {
                    amount++;
                    map_.insert({pred, amount});
                }
            }
        }
    }
    Init(amount + 1);
    SetVertex(0, fake_root_);
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->Insts()) {
            if (map_.count(inst) > 0 && GetSemi(inst) == DEFAULT_DFS_VAL) {
                SetParent(inst, fake_root_);
                DfsNumbering(inst);
            }
        }
    }
    ASSERT(static_cast<size_t>(dfs_num_) == amount);

    for (size_t i = amount; i > 0; i--) {
        ComputeImmediateDominators(GetVertex(i));
    }

    for (size_t i = 1; i <= amount; i++) {
        AdjustImmediateDominators(GetVertex(i));
    }
}

/*
 * Adjust immediate dominators,
 * Update dominator information for 'inst'
 */
void Cleanup::AdjustImmediateDominators(Inst *inst)
{
    ASSERT(inst != nullptr);

    if (GetIdom(inst) != GetVertex(GetSemi(inst))) {
        SetIdom(inst, GetIdom(GetIdom(inst)));
    }
}

/*
 * Compute initial values for semidominators,
 * store instructions with the same semidominator in the same bucket,
 * compute immediate dominators for instructions in the bucket of 'inst' parent
 */
void Cleanup::ComputeImmediateDominators(Inst *inst)
{
    ASSERT(inst != nullptr);

    if (inst->IsPhi()) {
        for (auto input : inst->GetInputs()) {
            auto pred = input.GetInst();
            auto eval = Eval(pred);
            if (GetSemi(eval) < GetSemi(inst)) {
                SetSemi(inst, GetSemi(eval));
            }
        }
    } else {
        auto eval = fake_root_;
        if (GetSemi(eval) < GetSemi(inst)) {
            SetSemi(inst, GetSemi(eval));
        }
    }

    auto vertex = GetVertex(GetSemi(inst));
    GetBucket(vertex).push_back(inst);
    auto parent = GetParent(inst);
    SetAncestor(inst, parent);

    auto &bucket = GetBucket(parent);
    while (!bucket.empty()) {
        auto v = *bucket.rbegin();
        auto eval = Eval(v);
        if (GetSemi(eval) < GetSemi(v)) {
            SetIdom(v, eval);
        } else {
            SetIdom(v, parent);
        }
        bucket.pop_back();
    }
}

/*
 * Compress ancestor path to 'inst' to the instruction whose label has the maximal semidominator number
 */
void Cleanup::Compress(Inst *inst)
{
    auto anc = GetAncestor(inst);
    ASSERT(anc != nullptr);

    if (GetAncestor(anc) != nullptr) {
        Compress(anc);
        if (GetSemi(GetLabel(anc)) < GetSemi(GetLabel(inst))) {
            SetLabel(inst, GetLabel(anc));
        }
        SetAncestor(inst, GetAncestor(anc));
    }
}

/*
 *  Depth-first search with numbering instruction in order they are reaching
 */
void Cleanup::DfsNumbering(Inst *inst)
{
    ASSERT(inst != nullptr || inst != fake_root_);
    dfs_num_++;
    ASSERT_PRINT(static_cast<size_t>(dfs_num_) < vertices_.size(), "DFS-number overflow");

    SetVertex(dfs_num_, inst);
    SetLabel(inst, inst);
    SetSemi(inst, dfs_num_);
    SetAncestor(inst, nullptr);

    for (auto &user : inst->GetUsers()) {
        auto succ = user.GetInst();
        if (succ->IsPhi() && GetSemi(succ) == DEFAULT_DFS_VAL) {
            SetParent(succ, inst);
            DfsNumbering(succ);
        }
    }
}

/*
 * Return 'inst' if it is the root of a tree
 * Otherwise, after tree compressing
 * return the instruction in the ancestors chain with the minimal semidominator DFS-number
 */
Inst *Cleanup::Eval(Inst *inst)
{
    ASSERT(inst != nullptr);
    if (GetAncestor(inst) == nullptr) {
        return inst;
    }
    Compress(inst);
    return GetLabel(inst);
}

/*
 * Initialize data structures to start DFS
 */
void Cleanup::Init(size_t count)
{
    ancestors_.clear();
    idoms_.clear();
    labels_.clear();
    parents_.clear();
    vertices_.clear();
    semi_.clear();

    ancestors_.resize(count);
    idoms_.resize(count);
    labels_.resize(count);
    parents_.resize(count);
    vertices_.resize(count);
    semi_.resize(count);

    std::fill(vertices_.begin(), vertices_.end(), nullptr);
    std::fill(semi_.begin(), semi_.end(), DEFAULT_DFS_VAL);

    if (buckets_.size() < count) {
        buckets_.resize(count, InstVector(GetGraph()->GetLocalAllocator()->Adapter()));
    }
    for (auto &bucket : buckets_) {
        bucket.clear();
    }
    dfs_num_ = DEFAULT_DFS_VAL;
}

/*
 * Selecting phi instructions that can be deleted
 * and replaced with a single instruction for all uses
 *
 * Example
 *    ...
 *    6p.u64  Phi                        v2(bb4), v2(bb3)
 *    7.u64  Return                      v6p
 *
 * Removing instruction 6 and replacing use with v2
 *
 *    ...
 *    7.u64  Return                      v2
 */
bool Cleanup::PhiChecker()
{
    BuildDominators();
    bool modified = false;
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto phi : bb->PhiInstsSafe()) {
            auto change = GetIdom(phi);
            if (change == fake_root_) {
                continue;
            }
            while (GetIdom(change) != fake_root_) {
                change = GetIdom(change);
            }
            auto basic_block = phi->GetBasicBlock();
            phi->ReplaceUsers(change);
            basic_block->RemoveInst(phi);
            modified = true;
        }
    }
    map_.clear();
    return modified;
}

void Cleanup::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<LinearOrder>();
}
}  // namespace panda::compiler
