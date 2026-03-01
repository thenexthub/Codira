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
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/basicblock.h"

namespace ark::compiler {

bool Cleanup::CanBeMerged(BasicBlock *bb)
{
    // TryCatchResolving with profile data needs separate CatchBegin with CatchPhis blocks
    if (!GetGraph()->IsBytecodeOptimizer() &&
        (!GetGraph()->IsAotMode() || GetGraph()->GetAotData()->HasProfileData()) && bb->IsCatchBegin()) {
        return false;
    }
    return bb->GetSuccsBlocks().size() == 1 && bb->GetSuccessor(0)->GetPredsBlocks().size() == 1 &&
           !bb->GetSuccessor(0)->IsPseudoControlFlowBlock() && bb->IsTry() == bb->GetSuccessor(0)->IsTry();
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
    auto emptyBlocks = &empty1_;
    auto newEmptyBlocks = &empty2_;

    bool modified = (lightMode_ ? PhiCheckerLight() : PhiChecker());

    for (auto bb : GetGraph()->GetVectorBlocks()) {
        if (!SkipBasicBlock(bb) && bb->IsEmpty()) {
            emptyBlocks->insert(bb);
        }
    }

    bool firstRun = true;
    do {
        modified |= RunOnce(emptyBlocks, newEmptyBlocks, !firstRun);
        firstRun = false;
        // Swap vectors pointers
        auto temp = emptyBlocks;
        emptyBlocks = newEmptyBlocks;
        newEmptyBlocks = temp;
        // Clean the "new" list
        newEmptyBlocks->clear();
    } while (!emptyBlocks->empty());

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

        while (CanBeMerged(bb)) {
            ASSERT(!bb->GetSuccessor(0)->HasPhi());
            COMPILER_LOG(DEBUG, CLEANUP) << "Merged block " << bb->GetSuccessor(0)->GetId() << " into " << bb->GetId();
            if (GetGraph()->EraseThrowBlock(bb->GetSuccessor(0))) {
                [[maybe_unused]] auto res = GetGraph()->AppendThrowBlock(bb);
                ASSERT(res);
            }
            bb->JoinSuccessorBlock();
            modified = true;
        }
    }
    return modified;
}

#ifndef NDEBUG
void Cleanup::CheckBBPhisUsers(BasicBlock *succ, BasicBlock *bb)
{
    if (succ->GetPredsBlocks().size() > 1) {
        for (auto phi : bb->PhiInsts()) {
            for (auto &userItem : phi->GetUsers()) {
                auto user = userItem.GetInst();
                ASSERT((user->GetBasicBlock() == succ && user->IsPhi()) || user->IsCatchPhi());
            }
        }
    }
}
#endif

bool Cleanup::RunOnce(ArenaSet<BasicBlock *> *emptyBlocks, ArenaSet<BasicBlock *> *newEmptyBlocks, bool simpleDce)
{
    bool modified = false;
    auto markerHolder = MarkerHolder(GetGraph());
    auto deadMrk = markerHolder.GetMarker();
    // Check all basic blocks in "old" list
    for (auto bb : *emptyBlocks) {
        // In some tricky cases block may be already deleted in previous iteration
        if (bb->GetGraph() == nullptr) {
            continue;
        }

        // Strange infinite loop with only one empty block, or loop pre-header - lets bail out
        auto succ = bb->GetSuccessor(0);
        if (succ == bb || succ->GetLoop()->GetPreHeader() == bb) {
            continue;
        }

#ifndef NDEBUG
        // Now we know that 'bb' is not a loop pre-header, so if both 'bb' and 'succ' have many predecessors
        // all 'bb' Phi(s) must have user only in successor Phis
        CheckBBPhisUsers(succ, bb);
#endif
        modified |= ProcessBB(bb, deadMrk, newEmptyBlocks);
    }

    if (simpleDce) {
        modified |= SimpleDce(deadMrk, newEmptyBlocks);
    } else if (lightMode_) {
        modified |= Dce<true>(deadMrk, newEmptyBlocks);
    } else {
        modified |= Dce<false>(deadMrk, newEmptyBlocks);
    }
    dead_.clear();

    return modified;
}

bool Cleanup::SkipBasicBlock(BasicBlock *bb)
{
    // NOTE (a.popov) Make empty catch-begin and try-end blocks removeable
    return bb == nullptr || bb->IsStartBlock() || bb->IsEndBlock() || bb->IsCatchBegin() || bb->IsTryEnd();
}

// Check around for special triangle case
bool Cleanup::CheckSpecialTriangle(BasicBlock *bb)
{
    auto succ = bb->GetSuccessor(0);
    size_t i = 0;
    for (auto pred : bb->GetPredsBlocks()) {
        if (pred->GetSuccessor(0) != succ &&
            (pred->GetSuccsBlocks().size() != MAX_SUCCS_NUM || pred->GetSuccessor(1) != succ)) {
            ++i;
            continue;
        }
        // Checking all Phis
        for (auto phi : succ->PhiInsts()) {
            size_t indexBb = phi->CastToPhi()->GetPredBlockIndex(bb);
            size_t indexPred = phi->CastToPhi()->GetPredBlockIndex(pred);
            ASSERT(indexBb != indexPred);

            auto instPred = phi->GetInput(indexPred).GetInst();
            auto instBb = phi->GetInput(indexBb).GetInst();
            // If phi input is in 'bb', check input of that phi instead
            if (instBb->GetBasicBlock() == bb) {
                ASSERT(instBb->IsPhi());
                instBb = instBb->CastToPhi()->GetInput(i).GetInst();
            }
            if (instBb != instPred) {
                return true;
            }
        }
        // Would fully remove 'straight' pred->succ edge, and second one would stay after 'bb' removal
        savedPreds_.push_back(pred);
        i++;
    }
    return false;
}

void Cleanup::RemoveDeadPhi(BasicBlock *bb, ArenaSet<BasicBlock *> *newEmptyBlocks)
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
                newEmptyBlocks->insert(pred);
            }
        }
    }
}

bool Cleanup::ProcessBB(BasicBlock *bb, Marker deadMrk, ArenaSet<BasicBlock *> *newEmptyBlocks)
{
    auto succ = bb->GetSuccessor(0);
    if (CheckSpecialTriangle(bb)) {
        return false;
    }
    // Remove dead Phi(s)
    RemoveDeadPhi(bb, newEmptyBlocks);
    // Process saved predecessors
    for (auto pred : savedPreds_) {
        ASSERT(pred->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
        constexpr auto PREDS_BLOCK_NUM = 2;
        ASSERT(succ->GetPredsBlocks().size() >= PREDS_BLOCK_NUM);

        auto last = pred->GetLastInst();
        if (last->GetOpcode() == Opcode::If || last->GetOpcode() == Opcode::IfImm ||
            last->GetOpcode() == Opcode::AddOverflow || last->GetOpcode() == Opcode::SubOverflow) {
            last->SetMarker(deadMrk);
            dead_.push_back(last);
        } else {
            ASSERT(last->GetOpcode() == Opcode::Throw || last->GetOpcode() == Opcode::Try);
        }
        pred->RemoveSucc(succ);
        if (succ->GetPredsBlocks().size() == PREDS_BLOCK_NUM) {
            for (auto phi : succ->PhiInstsSafe()) {
                auto rmIndex = phi->CastToPhi()->GetPredBlockIndex(pred);
                auto remainingInst = phi->GetInputs()[1 - rmIndex].GetInst();
                phi->ReplaceUsers(remainingInst);
                succ->RemoveInst(phi);
            }
        } else {  // more than 2 predecessors
            for (auto phi : succ->PhiInstsSafe()) {
                auto rmIndex = phi->CastToPhi()->GetPredBlockIndex(pred);
                phi->CastToPhi()->RemoveInput(rmIndex);
            }
        }
        succ->RemovePred(pred);
        // Fixing LoopAnalysis or DomTree is no necessary here, because there would be another edge
    }
    savedPreds_.clear();
    bool badLoop = bb->GetLoop()->IsIrreducible() || GetGraph()->IsThrowApplied();
    if (!GetGraph()->IsThrowApplied()) {
        GetGraph()->RemoveEmptyBlockWithPhis(bb, badLoop);
    }
    if (badLoop) {
        GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
        GetGraph()->RunPass<LoopAnalyzer>();
    }
    COMPILER_LOG(DEBUG, CLEANUP) << "Removed empty block: " << bb->GetId();
    return true;
}

void Cleanup::MarkInlinedCaller(Marker liveMrk, Inst *saveState)
{
    if (!saveState->IsSaveState()) {
        return;
    }
    auto caller = static_cast<SaveStateInst *>(saveState)->GetCallerInst();
    // if caller->IsInlined() is false, graph is being inlined and caller is not in the graph
    if (caller != nullptr && !caller->IsMarked(liveMrk) && caller->IsInlined()) {
        MarkLiveRec<false>(liveMrk, caller);
    }
}

bool Cleanup::IsRemovableCall(Inst *inst)
{
    if (inst->IsCall() && static_cast<CallInst *>(inst)->IsInlined() && !static_cast<CallInst *>(inst)->GetIsNative()) {
        auto saveState = inst->GetSaveState();
        ASSERT(saveState != nullptr);
        for (auto &ssUser : saveState->GetUsers()) {
            if (ssUser.GetInst()->GetOpcode() == Opcode::ReturnInlined &&
                ssUser.GetInst()->GetFlag(inst_flags::MEM_BARRIER)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

// Mark instructions that have the NOT_REMOVABLE property
// and recursively mark all their inputs
template <bool LIGHT_MODE>
void Cleanup::MarkLiveRec(Marker liveMrk, Inst *inst)
{
    // No recursion for one-input case, otherwise got stackoverflow on TSAN job
    bool marked = false;
    while (inst->GetInputsCount() == 1) {
        marked = inst->SetMarker(liveMrk);
        if (marked) {
            break;
        }
        if constexpr (!LIGHT_MODE) {
            MarkInlinedCaller(liveMrk, inst);
        }
        inst = inst->GetInput(0).GetInst();
    }
    if (!marked && !inst->SetMarker(liveMrk)) {
        if constexpr (!LIGHT_MODE) {
            MarkInlinedCaller(liveMrk, inst);
        }
        for (auto input : inst->GetInputs()) {
            MarkLiveRec<LIGHT_MODE>(liveMrk, input.GetInst());
        }
    }
}

template <bool LIGHT_MODE>
void Cleanup::MarkOneLiveInst(Marker deadMrk, Marker liveMrk, Inst *inst)
{
    if constexpr (LIGHT_MODE) {
        if (inst->IsNotRemovable() && !inst->IsMarked(deadMrk)) {
            MarkLiveRec<true>(liveMrk, inst);
        }
    } else {
        if (inst->GetOpcode() == Opcode::ReturnInlined) {
            // SaveState input of ReturnInlined will be marked as live through CallInlined if needed
            inst->SetMarker(liveMrk);
        } else if (inst->IsNotRemovable() && !inst->IsMarked(deadMrk) && !IsRemovableCall(inst)) {
            MarkLiveRec<false>(liveMrk, inst);
        }
    }
}

template <bool LIGHT_MODE>
void Cleanup::MarkLiveInstructions(Marker deadMrk, Marker liveMrk)
{
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            MarkOneLiveInst<LIGHT_MODE>(deadMrk, liveMrk, inst);
        }
    }
}

template <bool LIGHT_MODE>
bool Cleanup::TryToRemoveNonLiveInst(Inst *inst, BasicBlock *bb, ArenaSet<BasicBlock *> *newEmptyBlocks, Marker liveMrk)
{
    bool modified = false;
    if (inst->IsMarked(liveMrk)) {
        auto saveState = inst->GetSaveState();
        auto opcode = inst->GetOpcode();
        ASSERT((LIGHT_MODE || opcode != Opcode::ReturnInlined) || saveState != nullptr);
        if (LIGHT_MODE || !(opcode == Opcode::ReturnInlined && saveState->GetBasicBlock() == nullptr)) {
            return modified;
        }
    }
    if (!LIGHT_MODE && inst->IsCall() && static_cast<CallInst *>(inst)->IsInlined()) {
        auto callerSs = inst->GetSaveState();
        for (auto &user : callerSs->GetUsers()) {
            auto userInst = user.GetInst();
            // we mark all ReturnInlined as live initially to reduce number of IsDominate checks
            // IsDominate check is needed because several pairs of Call/Return inlined can share SaveState
            if (userInst->GetOpcode() == Opcode::ReturnInlined && inst->IsDominate(userInst)) {
                userInst->ResetMarker(liveMrk);
            }
        }
    }
    bool isPhi = inst->IsPhi();
    bb->RemoveInst(inst);
    COMPILER_LOG(DEBUG, CLEANUP) << "Dead instruction " << inst->GetId();
    GetGraph()->GetEventWriter().EventCleanup(inst->GetId(), inst->GetPc());
    modified = true;

    if (isPhi) {
        for (auto pred : bb->GetPredsBlocks()) {
            if (pred->IsEmpty() && !SkipBasicBlock(pred)) {
                COMPILER_LOG(DEBUG, CLEANUP) << "Would re-check empty block " << pred->GetId();
                newEmptyBlocks->insert(pred);
            }
        }
    } else if (bb->IsEmpty() && !SkipBasicBlock(bb)) {
        COMPILER_LOG(DEBUG, CLEANUP) << "No more non-Phi instructions in block " << bb->GetId();
        newEmptyBlocks->insert(bb);
    }
    return modified;
}

template <bool LIGHT_MODE>
bool Cleanup::Dce(Marker deadMrk, ArenaSet<BasicBlock *> *newEmptyBlocks)
{
    bool modified = false;
    auto markerHolder = MarkerHolder(GetGraph());
    auto liveMrk = markerHolder.GetMarker();
    MarkLiveInstructions<LIGHT_MODE>(deadMrk, liveMrk);

    // Remove non-live instructions
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->AllInstsSafe()) {
            modified |= TryToRemoveNonLiveInst<LIGHT_MODE>(inst, bb, newEmptyBlocks, liveMrk);
        }
    }
    return modified;
}

void Cleanup::SetLiveRec(Inst *inst, Marker mrk, Marker liveMrk)
{
    for (auto inputItem : inst->GetInputs()) {
        auto input = inputItem.GetInst();
        if (!input->IsMarked(liveMrk) && input->IsMarked(mrk)) {
            input->ResetMarker(mrk);
            input->SetMarker(liveMrk);
            SetLiveRec(input, mrk, liveMrk);
        }
    }
}

void Cleanup::LiveUserSearchRec(Inst *inst, Marker mrk, Marker liveMrk, Marker deadMrk)
{
    ASSERT(!inst->IsMarked(mrk));
    ASSERT(!inst->IsMarked(deadMrk));
    if (inst->IsMarked(liveMrk)) {
        SetLiveRec(inst, mrk, liveMrk);
        return;
    }
    if (inst->IsNotRemovable()) {
        inst->SetMarker(liveMrk);
        SetLiveRec(inst, mrk, liveMrk);
        return;
    }
    inst->SetMarker(mrk);
    temp_.push_back(inst);
    bool unknown = false;
    for (auto &userItem : inst->GetUsers()) {
        auto user = userItem.GetInst();
        if (user->IsMarked(mrk)) {
            unknown = true;
            continue;
        }
        if (user->IsMarked(deadMrk)) {
            continue;
        }
        LiveUserSearchRec(user, mrk, liveMrk, deadMrk);
        if (user->IsMarked(liveMrk)) {
            ASSERT(!inst->IsMarked(mrk) && inst->IsMarked(liveMrk));
            return;
        }
        ASSERT(inst->IsMarked(mrk));
        if (user->IsMarked(mrk)) {
            ASSERT(!user->IsMarked(liveMrk) && !user->IsMarked(deadMrk));
            unknown = true;
        } else {
            ASSERT(user->IsMarked(deadMrk));
        }
    }
    if (!unknown) {
        inst->ResetMarker(mrk);
        inst->SetMarker(deadMrk);
        dead_.push_back(inst);
    }
}

void Cleanup::TryMarkInstIsDead(Inst *inst, Marker deadMrk, Marker mrk, [[maybe_unused]] Marker liveMrk)
{
    if (!inst->IsMarked(mrk)) {
        return;
    }
    ASSERT(!inst->IsMarked(liveMrk) && !inst->IsMarked(deadMrk));
    inst->ResetMarker(mrk);
    inst->SetMarker(deadMrk);
    dead_.push_back(inst);
}

void Cleanup::Marking(Marker deadMrk, Marker mrk, Marker liveMrk)
{
    size_t i = 0;
    while (i < dead_.size()) {
        auto inst = dead_.at(i);
        for (auto inputItem : inst->GetInputs()) {
            auto input = inputItem.GetInst();
            if (input->IsMarked(deadMrk) || input->IsMarked(mrk)) {
                continue;
            }
            LiveUserSearchRec(input, mrk, liveMrk, deadMrk);
            for (auto temp : temp_) {
                TryMarkInstIsDead(temp, deadMrk, mrk, liveMrk);
            }
            temp_.clear();
        }
        i++;
    }
}

void Cleanup::RemovalPhi(BasicBlock *bb, ArenaSet<BasicBlock *> *newEmptyBlocks)
{
    for (auto pred : bb->GetPredsBlocks()) {
        if (pred->IsEmpty() && !SkipBasicBlock(pred)) {
            COMPILER_LOG(DEBUG, CLEANUP) << "Would re-check empty block " << pred->GetId();
            newEmptyBlocks->insert(pred);
        }
    }
}

bool Cleanup::Removal(ArenaSet<BasicBlock *> *newEmptyBlocks)
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
            RemovalPhi(bb, newEmptyBlocks);
        } else {
            if (bb->IsEmpty() && !SkipBasicBlock(bb)) {
                COMPILER_LOG(DEBUG, CLEANUP) << "No more non-Phi instructions in block " << bb->GetId();
                newEmptyBlocks->insert(bb);
            }
        }
    }
    return modified;
}

bool Cleanup::SimpleDce(Marker deadMrk, ArenaSet<BasicBlock *> *newEmptyBlocks)
{
    auto markerHolder = MarkerHolder(GetGraph());
    auto mrk = markerHolder.GetMarker();
    auto liveMarkerHolder = MarkerHolder(GetGraph());
    auto liveMrk = liveMarkerHolder.GetMarker();

    // Step 1. Marking
    Marking(deadMrk, mrk, liveMrk);

    // Step 2. Removal
    return Removal(newEmptyBlocks);
}

void Cleanup::BuildDominatorsVisitPhi(Inst *inst, size_t &amount)
{
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

void Cleanup::BuildDominators()
{
    size_t amount = 0;
    fakeRoot_ = reinterpret_cast<Inst *>(sizeof(Inst *));
    map_.insert({fakeRoot_, amount});
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->PhiInsts()) {
            BuildDominatorsVisitPhi(inst, amount);
        }
    }
    Init(amount + 1);
    SetVertex(0, fakeRoot_);
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->Insts()) {
            if (map_.count(inst) > 0 && GetSemi(inst) == DEFAULT_DFS_VAL) {
                SetParent(inst, fakeRoot_);
                DfsNumbering(inst);
            }
        }
    }
    ASSERT(static_cast<size_t>(dfsNum_) == amount);

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
        auto eval = fakeRoot_;
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
    ASSERT(inst != nullptr || inst != fakeRoot_);
    dfsNum_++;
    ASSERT_PRINT(static_cast<size_t>(dfsNum_) < vertices_.size(), "DFS-number overflow");

    SetVertex(dfsNum_, inst);
    SetLabel(inst, inst);
    SetSemi(inst, dfsNum_);
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
    dfsNum_ = DEFAULT_DFS_VAL;
}

static bool PhiHasSameInputs(Inst *phi)
{
    auto input = phi->GetInput(0).GetInst();
    for (size_t i = 1; i < phi->GetInputsCount(); i++) {
        if (input != phi->GetInput(i).GetInst()) {
            return false;
        }
    }
    return true;
}

// Remove only phi with same inputs
bool Cleanup::PhiCheckerLight() const
{
    bool modified = false;
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto phi : bb->PhiInstsSafe()) {
            if (PhiHasSameInputs(phi)) {
                phi->ReplaceUsers(phi->GetInput(0).GetInst());
                bb->RemoveInst(phi);
                modified = true;
            }
        }
    }
    return modified;
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
            if (change == fakeRoot_) {
                continue;
            }
            while (GetIdom(change) != fakeRoot_) {
                change = GetIdom(change);
            }
            auto basicBlock = phi->GetBasicBlock();
            phi->ReplaceUsers(change);
            basicBlock->RemoveInst(phi);
            modified = true;
        }
    }
    map_.clear();
    return modified;
}

void Cleanup::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<LinearOrder>();
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}
}  // namespace ark::compiler
