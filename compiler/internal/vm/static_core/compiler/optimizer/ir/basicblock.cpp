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
#include "inst.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/dominators_tree.h"

namespace ark::compiler {
class Inst;
BasicBlock::BasicBlock(Graph *graph, uint32_t guestPc)
    : graph_(graph),
      preds_(graph_->GetAllocator()->Adapter()),
      succs_(graph_->GetAllocator()),
      domBlocks_(graph_->GetAllocator()->Adapter()),
      guestPc_(guestPc)
{
}

void BasicBlock::SetId(uint32_t id)
{
    bbId_ = id;
}
uint32_t BasicBlock::GetId() const
{
    return bbId_;
}

ArenaVector<BasicBlock *> &BasicBlock::GetPredsBlocks()
{
    return preds_;
}
const ArenaVector<BasicBlock *> &BasicBlock::GetPredsBlocks() const
{
    return preds_;
}

BasicBlock::SuccsVector &BasicBlock::GetSuccsBlocks()
{
    return succs_;
}
const BasicBlock::SuccsVector &BasicBlock::GetSuccsBlocks() const
{
    return succs_;
}

BasicBlock *BasicBlock::GetSuccessor(size_t index) const
{
    ASSERT(index < succs_.size());
    return succs_[index];
}

BasicBlock *BasicBlock::GetPredecessor(size_t index) const
{
    ASSERT(index < preds_.size());
    return preds_[index];
}

bool BasicBlock::IsInverted() const
{
    return inverted_;
}

void BasicBlock::SetHotness(int64_t hotness)
{
    hotness_ = hotness;
}

int64_t BasicBlock::GetHotness() const
{
    return hotness_;
}

BasicBlock *BasicBlock::GetTrueSuccessor() const
{
    ASSERT(!succs_.empty());
    return succs_[TRUE_SUCC_IDX];
}

BasicBlock *BasicBlock::GetFalseSuccessor() const
{
    ASSERT(succs_.size() > 1);
    return succs_[FALSE_SUCC_IDX];
}

size_t BasicBlock::GetPredBlockIndex(const BasicBlock *block) const
{
    auto it = std::find(preds_.begin(), preds_.end(), block);
    ASSERT(it != preds_.end());
    ASSERT(std::find(it + 1, preds_.end(), block) == preds_.end());
    return std::distance(preds_.begin(), it);
}

size_t BasicBlock::GetSuccBlockIndex(const BasicBlock *block) const
{
    auto it = std::find(succs_.begin(), succs_.end(), block);
    ASSERT(it != succs_.end());
    ASSERT(std::find(it + 1, succs_.end(), block) == succs_.end());
    return std::distance(succs_.begin(), it);
}

BasicBlock *BasicBlock::GetPredBlockByIndex(size_t index) const
{
    ASSERT(index < preds_.size());
    return preds_[index];
}

Graph *BasicBlock::GetGraph()
{
    return graph_;
}

const Graph *BasicBlock::GetGraph() const
{
    return graph_;
}

void BasicBlock::SetGraph(Graph *graph)
{
    graph_ = graph;
}

void BasicBlock::SetGuestPc(uint32_t guestPc)
{
    guestPc_ = guestPc;
}

uint32_t BasicBlock::GetGuestPc() const
{
    return guestPc_;
}

bool BasicBlock::HasSucc(BasicBlock *succ)
{
    return std::find(succs_.begin(), succs_.end(), succ) != succs_.end();
}

void BasicBlock::ReplacePred(BasicBlock *prevPred, BasicBlock *newPred)
{
    preds_[GetPredBlockIndex(prevPred)] = newPred;
    ASSERT(newPred != nullptr);
    newPred->succs_.push_back(this);
}

void BasicBlock::ReplaceTrueSuccessor(BasicBlock *newSucc)
{
    ASSERT(!succs_.empty());
    succs_[TRUE_SUCC_IDX] = newSucc;
    newSucc->preds_.push_back(this);
}

void BasicBlock::ReplaceFalseSuccessor(BasicBlock *newSucc)
{
    ASSERT(succs_.size() > 1);
    succs_[FALSE_SUCC_IDX] = newSucc;
    newSucc->preds_.push_back(this);
}

void BasicBlock::PrependInst(Inst *inst)
{
    AddInst<false>(inst);
}
// Insert new instruction(NOT PHI) in BasicBlock at the end of instructions
void BasicBlock::AppendInst(Inst *inst)
{
    AddInst<true>(inst);
}
void BasicBlock::AppendInsts(std::initializer_list<Inst *> &&insts)
{
    for (auto inst : insts) {
        AddInst<true>(inst);
    }
}

Inst *BasicBlock::GetLastInst() const
{
    return lastInst_;
}

bool BasicBlock::IsEndWithThrowOrDeoptimize() const
{
    if (lastInst_ == nullptr) {
        return false;
    }
    return lastInst_->IsControlFlow() && lastInst_->CanThrow() && lastInst_->IsTerminator();
}

bool BasicBlock::IsEndWithThrow() const
{
    if (lastInst_ == nullptr) {
        return false;
    }
    return lastInst_->GetOpcode() == Opcode::Throw;
}

Inst *BasicBlock::GetFirstInst() const
{
    return firstInst_;
}

Inst *BasicBlock::GetFirstPhi() const
{
    return firstPhi_;
}

bool BasicBlock::IsEmpty() const
{
    return firstInst_ == nullptr;
}

bool BasicBlock::HasPhi() const
{
    return firstPhi_ != nullptr;
}

void BasicBlock::SetDominator(BasicBlock *dominator)
{
    dominator_ = dominator;
}
void BasicBlock::ClearDominator()
{
    dominator_ = nullptr;
}

void BasicBlock::AddDominatedBlock(BasicBlock *block)
{
    domBlocks_.push_back(block);
}

void BasicBlock::RemoveDominatedBlock(BasicBlock *block)
{
    domBlocks_.erase(std::find(domBlocks_.begin(), domBlocks_.end(), block));
}

void BasicBlock::ClearDominatedBlocks()
{
    domBlocks_.clear();
}

void BasicBlock::RemovePred(const BasicBlock *pred)
{
    ASSERT(GetPredBlockIndex(pred) < preds_.size());
    preds_[GetPredBlockIndex(pred)] = preds_.back();
    preds_.pop_back();
}

void BasicBlock::RemovePred(size_t index)
{
    ASSERT(index < preds_.size());
    preds_[index] = preds_.back();
    preds_.pop_back();
}

void BasicBlock::RemoveSucc(BasicBlock *succ)
{
    auto it = std::find(succs_.begin(), succs_.end(), succ);
    ASSERT(it != succs_.end());
    succs_.erase(it);
}

Loop *BasicBlock::GetLoop() const
{
    return loop_;
}
void BasicBlock::SetLoop(Loop *loop)
{
    loop_ = loop;
}
bool BasicBlock::IsLoopValid() const
{
    return GetLoop() != nullptr;
}

void BasicBlock::SetNextLoop(Loop *loop)
{
    nextLoop_ = loop;
}

Loop *BasicBlock::GetNextLoop() const
{
    return nextLoop_;
}

bool BasicBlock::IsLoopPreHeader() const
{
    return (nextLoop_ != nullptr);
}

void BasicBlock::SetAllFields(uint32_t bitFields)
{
    bitFields_ = bitFields;
}

uint32_t BasicBlock::GetAllFields() const
{
    return bitFields_;
}

void BasicBlock::SetMonitorEntryBlock(bool v)
{
    SetField<MonitorEntryBlock>(v);
}

bool BasicBlock::GetMonitorEntryBlock()
{
    return GetField<MonitorEntryBlock>();
}

void BasicBlock::SetMonitorExitBlock(bool v)
{
    SetField<MonitorExitBlock>(v);
}

bool BasicBlock::GetMonitorExitBlock() const
{
    return GetField<MonitorExitBlock>();
}

void BasicBlock::SetMonitorBlock(bool v)
{
    SetField<MonitorBlock>(v);
}

bool BasicBlock::GetMonitorBlock() const
{
    return GetField<MonitorBlock>();
}

void BasicBlock::SetCatch(bool v)
{
    SetField<CatchBlock>(v);
}

bool BasicBlock::IsCatch() const
{
    return GetField<CatchBlock>();
}

void BasicBlock::SetCatchBegin(bool v)
{
    SetField<CatchBeginBlock>(v);
}

bool BasicBlock::IsCatchBegin() const
{
    return GetField<CatchBeginBlock>();
}

void BasicBlock::SetTry(bool v)
{
    SetField<TryBlock>(v);
}

bool BasicBlock::IsTry() const
{
    return GetField<TryBlock>();
}

void BasicBlock::SetTryBegin(bool v)
{
    SetField<TryBeginBlock>(v);
}

bool BasicBlock::IsTryBegin() const
{
    return GetField<TryBeginBlock>();
}

void BasicBlock::SetTryEnd(bool v)
{
    SetField<TryEndBlock>(v);
}

bool BasicBlock::IsTryEnd() const
{
    return GetField<TryEndBlock>();
}

void BasicBlock::SetOsrEntry(bool v)
{
    SetField<OsrEntry>(v);
}

bool BasicBlock::IsOsrEntry() const
{
    return GetField<OsrEntry>();
}

void BasicBlock::CopyTryCatchProps(BasicBlock *block)
{
    if (block->IsTry()) {
        SetTry(true);
        SetTryId(block->GetTryId());
    }
    if (block->IsCatch()) {
        SetCatch(true);
    }
}

uint32_t BasicBlock::GetTryId() const
{
    return tryId_;
}

void BasicBlock::SetTryId(uint32_t tryId)
{
    tryId_ = tryId;
}

void BasicBlock::SetNeedsJump(bool v)
{
    SetField<JumpFlag>(v);
}

bool BasicBlock::NeedsJump() const
{
    return GetField<JumpFlag>();
}

bool BasicBlock::IsIfBlock() const
{
    if (lastInst_ != nullptr) {
        if (lastInst_->GetOpcode() == Opcode::If || lastInst_->GetOpcode() == Opcode::IfImm) {
            return true;
        }
    }
    return false;
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
    return IsStartBlock() || IsEndBlock() || IsTryBegin() || IsTryEnd();
}

bool BasicBlock::IsLoopHeader() const
{
    ASSERT(GetLoop() != nullptr);
    return (GetLoop()->GetHeader() == this);
}

bool BasicBlock::IsLoopPostExit() const
{
    for (auto innerLoop : GetLoop()->GetInnerLoops()) {
        if (innerLoop->IsPostExitBlock(this)) {
            return true;
        }
    }

    return false;
}

bool BasicBlock::IsTryCatch() const
{
    return IsTry() || IsTryBegin() || IsTryEnd() || IsCatch() || IsCatchBegin();
}

BasicBlock *BasicBlock::SplitBlockAfterInstruction(Inst *inst, bool makeEdge)
{
    ASSERT(inst != nullptr);
    ASSERT(inst->GetBasicBlock() == this);
    ASSERT(!IsStartBlock() && !IsEndBlock());

    auto nextInst = inst->GetNext();
    auto newBb = GetGraph()->CreateEmptyBlock((nextInst != nullptr) ? nextInst->GetPc() : INVALID_PC);
    ASSERT(newBb != nullptr);
    newBb->SetAllFields(this->GetAllFields());
    newBb->SetOsrEntry(false);
    GetLoop()->AppendBlock(newBb);

    for (; nextInst != nullptr; nextInst = nextInst->GetNext()) {
        newBb->AppendInst(nextInst);
    }
    inst->SetNext(nullptr);
    lastInst_ = inst;
    if (inst->IsPhi()) {
        firstInst_ = nullptr;
    }
    for (auto succ : GetSuccsBlocks()) {
        succ->ReplacePred(this, newBb);
    }
    GetSuccsBlocks().clear();

    ASSERT(GetSuccsBlocks().empty());
    if (makeEdge) {
        AddSucc(newBb);
    }
    return newBb;
}

void BasicBlock::AddSucc(BasicBlock *succ, bool canAddEmptyBlock)
{
    auto it = std::find(succs_.begin(), succs_.end(), succ);
    ASSERT_PRINT(it == succs_.end() || canAddEmptyBlock, "Uncovered case where empty block needed to fix CFG");
    if (it != succs_.end() && canAddEmptyBlock) {
        // If edge already exists we create empty block on it
        auto emptyBb = GetGraph()->CreateEmptyBlock(GetGuestPc());
        ReplaceSucc(succ, emptyBb);
        ASSERT(succ != nullptr);
        succ->ReplacePred(this, emptyBb);
    }
    succs_.push_back(succ);
    succ->GetPredsBlocks().push_back(this);
}

void BasicBlock::ReplaceSucc(const BasicBlock *prevSucc, BasicBlock *newSucc, bool canAddEmptyBlock)
{
    auto it = std::find(succs_.begin(), succs_.end(), newSucc);
    ASSERT_PRINT(it == succs_.end() || canAddEmptyBlock, "Uncovered case where empty block needed to fix CFG");
    if (it != succs_.end() && canAddEmptyBlock) {
        // If edge already exists we create empty block on it
        auto emptyBb = GetGraph()->CreateEmptyBlock(GetGuestPc());
        ASSERT(newSucc != nullptr);
        ReplaceSucc(newSucc, emptyBb);
        newSucc->ReplacePred(this, emptyBb);
    }
    succs_[GetSuccBlockIndex(prevSucc)] = newSucc;
    newSucc->preds_.push_back(this);
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
    size_t numPreds = bb->GetPredsBlocks().size();

    for (auto phi : succ->PhiInsts()) {
        auto index = phi->CastToPhi()->GetPredBlockIndex(bb);
        auto inst = phi->GetInput(index).GetInst();
        if (inst->GetBasicBlock() == bb) {  // When INST is from empty basic block ...
            ASSERT(inst->IsPhi());
            // ... we have to copy it's inputs into corresponding inputs of PHI
            auto predBb = bb->GetPredBlockByIndex(0);
            phi->SetInput(index, inst->CastToPhi()->GetPhiInput(predBb));
            for (size_t i = 1; i < numPreds; i++) {
                predBb = bb->GetPredBlockByIndex(i);
                phi->AppendInput(inst->CastToPhi()->GetPhiInput(predBb));
            }
        } else {  // otherwise, just copy inputs for new arrived predecessors
            for (size_t i = 1; i < numPreds; i++) {
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
void BasicBlock::RemoveEmptyBlock(bool irrLoop)
{
    ASSERT(GetFirstInst() == nullptr);
    ASSERT(!GetPredsBlocks().empty());
    ASSERT(GetSuccsBlocks().size() == 1);
    auto succ = succs_[0];

    // Save old amount of predecessors in successor block
    size_t succPredsNum = succ->GetPredsBlocks().size();

    size_t numPreds = preds_.size();
    // If empty block had more than one predecessors
    if (numPreds > 1) {
        if (succPredsNum > 1) {
            // We have to process Phi instructions in successor block in a special way
            RemovePhiProcessing(this, succ);
        } else {  // successor didn't have other predecessors, we are moving Phi(s) into successor
            ASSERT(!succ->HasPhi());
            for (auto phi : PhiInstsSafe()) {
                succ->AppendPhi(phi);
            }
            firstPhi_ = nullptr;
            lastInst_ = nullptr;
        }
    }

    // Set successor for all predecessor blocks, at the same time in successor we replace one predecessor
    // and add others to the end (if there were many of them in empty block)
    auto pred = preds_[0];
    pred->succs_[pred->GetSuccBlockIndex(this)] = succ;
    succ->preds_[succ->GetPredBlockIndex(this)] = pred;
    for (size_t i = 1; i < numPreds; ++i) {
        pred = preds_[i];
        pred->succs_[pred->GetSuccBlockIndex(this)] = succ;
        succ->preds_.push_back(pred);
    }

    ASSERT(GetLastInst() == nullptr);
    ASSERT(GetLoop()->IsIrreducible() == irrLoop);
    // N.B. info about Irreducible loop can not be fixed on the fly
    if (!irrLoop) {
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
    auto firstPred = bb->GetPredBlockByIndex(0);
    // Do not dup back-edge
    if (loop->HasBackEdge(firstPred)) {
        loop->RemoveBackEdge(bb);
    } else {
        loop->ReplaceBackEdge(bb, firstPred);
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
    ASSERT(!IsLoopPreHeader());
    if (succ->IsLoopPreHeader()) {
        succ->GetNextLoop()->SetPreHeader(this);
    }

    // moving instructions from successor
    ASSERT(!succ->HasPhi());
    for (auto succInst : succ->Insts()) {
        AppendInst(succInst);
    }

    // moving successor blocks from the successor
    GetSuccsBlocks().clear();
    for (auto succSucc : succ->GetSuccsBlocks()) {
        succSucc->ReplacePred(succ, this);
    }

    if (GetGraph()->IsAnalysisValid<LoopAnalyzer>()) {
        // fixing loop information
        // invariant: succ has one predecessor, so it cannot be a loop header
        auto loop = succ->GetLoop();
        ASSERT(loop != nullptr);
        // Irreducible loop can not be fixed on the fly
        if (loop->IsIrreducible() || GetGraph()->IsThrowApplied()) {
            GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
        } else {
            // edge can have 2 successors, so it can be back-edge in 2 loops: own loop and outer loop
            ReplaceSuccessorLoopBackEdges(loop, succ);
            loop->RemoveBlock(succ);
        }
    }

    succ->firstInst_ = nullptr;
    succ->lastInst_ = nullptr;
    succ->GetPredsBlocks().clear();
    succ->GetSuccsBlocks().clear();

    this->bitFields_ |= succ->bitFields_;
    // NOTE (a.popov) replace by assert
    if (succ->tryId_ != INVALID_ID) {
        this->tryId_ = succ->tryId_;
    }
    GetGraph()->RemoveEmptyBlock(succ);
}

void BasicBlock::ReplaceSuccessorLoopBackEdges(Loop *loop, BasicBlock *succ)
{
    if (loop->HasBackEdge(succ)) {
        loop->ReplaceBackEdge(succ, this);
    }
    for (auto outerLoop = loop->GetOuterLoop(); outerLoop != nullptr; outerLoop = outerLoop->GetOuterLoop()) {
        if (outerLoop->HasBackEdge(succ)) {
            outerLoop->ReplaceBackEdge(succ, this);
        }
    }
    for (auto innerLoop : loop->GetInnerLoops()) {
        if (innerLoop->GetPreHeader() == succ) {
            innerLoop->SetPreHeader(this);
        }
    }
}

/**
 * Join successor block into the block, which have another successor;
 * Used in if-conversion pass and fixes dataflow using Select instructions.
 * @param succ is a successor block which must have one predecessor and
 * one successor, function will remove Phi(s) from the latter successor.
 * @param select_bb is a block to insert generated Select instructions.
 * When 'select_bb' is nullptr, we generate Select(s) in 'this' block.
 * @param swapped == true means 'succ' is False successor (instead of True).
 * How conversion works in the following two cases:
 *
 * EXAMPLE 1 (Comes from TryTriangle in if_conversion.cpp):
 *
 *                [this] ('select_bb' == nullptr)
 *                (last inst: if-jump)
 *                 |  \
 *                 |  [succ]
 *                 |  (one inst: arithmetic)
 *                 |  /
 *                (first inst: phi)
 *                [other]
 *
 * turns into this:
 *                [this]
 *                (arithmetic)
 *                (select)
 *                 |
 *                 |
 *                (may be phi if there are another predecessors)
 *                [other]
 *
 * EXAMPLE 2 (Comes from TryDiamond in if_conversion.cpp):
 *
 *                [this]
 *                (last inst: if-jump)
 *                /   \
 *       [select_bb]  [succ]
 *     (arithmetic2)  (arithmetic1)
 *                \   /
 *                (first inst: phi)
 *                [other]
 *
 * turns into this:
 *                [this]
 *                (arithmetic1)
 *                 |
 *                 |
 *                [select_bb]
 *                (arithmetic2)
 *                (select)
 *                 |
 *                 |
 *                (may be phi if there are another predecessors)
 *                [other]
 *
 * Function returns whether we need DCE for If inputs.
 */
void BasicBlock::JoinBlocksUsingSelect(BasicBlock *succ, BasicBlock *selectBb, bool swapped)
{
    ASSERT(!IsStartBlock());
    ASSERT(GetSuccsBlocks().size() == MAX_SUCCS_NUM);
    ASSERT(succ == GetSuccessor(0) || succ == GetSuccessor(1));
    ASSERT(!succ->IsEndBlock());
    ASSERT(succ->GetPredsBlocks().size() == 1);
    ASSERT(succ->GetPredBlockByIndex(0) == this);
    ASSERT(succ->GetSuccsBlocks().size() == 1);
    /**
     * There are 2 steps in Join operation.
     * Step 1. Move instructions from 'succ' into 'this'.
     */
    Inst *ifInst = GetLastInst();
    SavedIfInfo ifInfo {succ,
                        swapped,
                        0,
                        ConditionCode::CC_FIRST,
                        DataType::NO_TYPE,
                        ifInst->GetPc(),
                        ifInst->GetOpcode(),
                        ifInst->GetInput(0).GetInst(),
                        nullptr};

    // Save necessary info
    if (ifInfo.ifOpcode == Opcode::IfImm) {
        ifInfo.ifImm = ifInst->CastToIfImm()->GetImm();
        ifInfo.ifCc = ifInst->CastToIfImm()->GetCc();
        ifInfo.ifType = ifInst->CastToIfImm()->GetOperandsType();
    } else if (ifInfo.ifOpcode == Opcode::If) {
        ifInfo.ifInput1 = ifInst->GetInput(1).GetInst();
        ifInfo.ifCc = ifInst->CastToIf()->GetCc();
        ifInfo.ifType = ifInst->CastToIf()->GetOperandsType();
    } else {
        UNREACHABLE();
    }

    // Remove 'If' instruction
    RemoveInst(ifInst);

    // Remove incoming 'this->succ' edge
    RemoveSucc(succ);
    succ->GetPredsBlocks().clear();

    // Main loop in "Step 1", moving instructions from successor.
    ASSERT(!succ->HasPhi());
    for (auto inst : succ->Insts()) {
        AppendInst(inst);
    }
    succ->firstInst_ = nullptr;
    succ->lastInst_ = nullptr;

    auto other = succ->GetSuccessor(0);
    /**
     * Step 2. Generate Select(s).
     * We generate them in 'select_bb' if provided (another successor in Diamond case),
     * or in 'this' block otherwise (Triangle case).
     */
    if (selectBb == nullptr) {
        selectBb = this;
    }
    selectBb->GenerateSelects(&ifInfo);
    succ->SelectsFixLoopInfo(selectBb, other);
}

void BasicBlock::GenerateSelect(Inst *phi, Inst *inst1, Inst *inst2, const SavedIfInfo *ifInfo)
{
    auto other = ifInfo->succ->GetSuccessor(0);
    Inst *select = nullptr;
    ASSERT(GetGraph()->GetEncoder()->CanEncodeFloatSelect() || !DataType::IsFloatType(phi->GetType()));
    if (ifInfo->ifOpcode == Opcode::IfImm) {
        select = GetGraph()->CreateInstSelectImm(phi->GetType(), ifInfo->ifPc, ifInfo->swapped ? inst2 : inst1,
                                                 ifInfo->swapped ? inst1 : inst2, ifInfo->ifInput0, ifInfo->ifImm,
                                                 ifInfo->ifType, ifInfo->ifCc);
    } else if (ifInfo->ifOpcode == Opcode::If) {
        select = GetGraph()->CreateInstSelect(phi->GetType(), ifInfo->ifPc,
                                              std::array<Inst *, 4U> {ifInfo->swapped ? inst2 : inst1,
                                                                      ifInfo->swapped ? inst1 : inst2, ifInfo->ifInput0,
                                                                      ifInfo->ifInput1},
                                              ifInfo->ifType, ifInfo->ifCc);
    } else {
        UNREACHABLE();
    }

    AppendInst(select);

    if (other->GetPredsBlocks().size() > 2U) {
        // Change input (from this block) to new Select instruction
        auto index = phi->CastToPhi()->GetPredBlockIndex(this);
        phi->CastToPhi()->SetInput(index, select);
        // Remove input from 'succ'
        index = phi->CastToPhi()->GetPredBlockIndex(ifInfo->succ);
        phi->CastToPhi()->RemoveInput(index);
    } else {
        // Remove Phi
        phi->ReplaceUsers(select);
        other->RemoveInst(phi);
    }
    // Select now must have users
    ASSERT(select->HasUsers());
}

void BasicBlock::GenerateSelects(const SavedIfInfo *ifInfo)
{
    auto succ = ifInfo->succ;

    // The only successor whether we will check Phi(s)
    auto other = succ->GetSuccessor(0);
    constexpr auto TWO = 2;
    ASSERT(other->GetPredsBlocks().size() >= TWO);

    // Main loop in "Step 2", generate select(s) and drop phi(s) when possible
    for (auto phi : other->PhiInstsSafe()) {
        size_t index1 = phi->CastToPhi()->GetPredBlockIndex(succ);
        size_t index2 = phi->CastToPhi()->GetPredBlockIndex(this);
        ASSERT(index1 != index2);

        auto inst1 = phi->GetInput(index1).GetInst();
        auto inst2 = phi->GetInput(index2).GetInst();
        if (inst1 == inst2) {
            // No select needed
            if (other->GetPredsBlocks().size() > TWO) {
                // Remove input from 'succ'
                auto index = phi->CastToPhi()->GetPredBlockIndex(succ);
                phi->CastToPhi()->RemoveInput(index);
            } else {
                // Remove Phi
                phi->ReplaceUsers(inst1);
                other->RemoveInst(phi);
            }
            continue;
        }

        GenerateSelect(phi, inst1, inst2, ifInfo);
    }
}

void BasicBlock::SelectsFixLoopInfo(BasicBlock *selectBb, BasicBlock *other)
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
        for (auto innerLoop : loop->GetInnerLoops()) {
            if (innerLoop->GetPreHeader() == this) {
                innerLoop->SetPreHeader(selectBb);
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
    if (firstPhi_ == nullptr) {
        inst->SetNext(firstInst_);
        if (firstInst_ != nullptr) {
            firstInst_->SetPrev(inst);
        }
        firstPhi_ = inst;
        if (lastInst_ == nullptr) {
            lastInst_ = inst;
        }
    } else {
        if (firstInst_ != nullptr) {
            Inst *prev = firstInst_->GetPrev();
            ASSERT_PRINT(prev && prev->IsPhi(), "There is no phi in the block");
            inst->SetPrev(prev);
            prev->SetNext(inst);
            inst->SetNext(firstInst_);
            firstInst_->SetPrev(inst);
        } else {
            ASSERT_PRINT(lastInst_ && lastInst_->IsPhi(),
                         "If first_phi is defined and first_inst is undefined, last_inst must be phi");
            lastInst_->SetNext(inst);
            inst->SetPrev(lastInst_);
            lastInst_ = inst;
        }
    }
}

template <bool TO_END>
void BasicBlock::AddInst(Inst *inst)
{
    ASSERT_PRINT(!inst->IsPhi(), "Instruction mustn't be phi");
    inst->SetBasicBlock(this);
    if (firstInst_ == nullptr) {
        inst->SetPrev(lastInst_);
        if (lastInst_ != nullptr) {
            ASSERT(lastInst_->IsPhi());
            lastInst_->SetNext(inst);
        }
        firstInst_ = inst;
        lastInst_ = inst;
    } else {
        // NOLINTNEXTLINE(readability-braces-around-statements)
        if constexpr (TO_END) {
            ASSERT_PRINT(lastInst_, "Last instruction is undefined");
            inst->SetPrev(lastInst_);
            ASSERT(lastInst_ != nullptr);
            lastInst_->SetNext(inst);
            lastInst_ = inst;
            // NOLINTNEXTLINE(readability-misleading-indentation)
        } else {
            auto firstPrev = firstInst_->GetPrev();
            if (firstPrev != nullptr) {
                firstPrev->SetNext(inst);
            }
            inst->SetPrev(firstPrev);
            inst->SetNext(firstInst_);
            firstInst_->SetPrev(inst);
            firstInst_ = inst;
        }
    }
}

void BasicBlock::AppendRangeInst(Inst *rangeFirst, Inst *rangeLast)
{
#ifndef NDEBUG
    ASSERT(rangeFirst && rangeLast && rangeFirst->IsDominate(rangeLast));
    ASSERT(rangeFirst->GetPrev() == nullptr);
    ASSERT(rangeLast->GetNext() == nullptr);
    auto instDb = rangeFirst;
    while (instDb != rangeLast) {
        ASSERT_PRINT(!instDb->IsPhi(), "Instruction mustn't be phi");
        ASSERT_PRINT(instDb->GetBasicBlock() == this, "Inst::SetBasicBlock() should be called beforehand");
        instDb = instDb->GetNext();
    }
    ASSERT_PRINT(!instDb->IsPhi(), "Instruction mustn't be phi");
    ASSERT_PRINT(instDb->GetBasicBlock() == this, "Inst::SetBasicBlock() should be called beforehand");
#endif

    if (firstInst_ == nullptr) {
        rangeFirst->SetPrev(lastInst_);
        if (lastInst_ != nullptr) {
            ASSERT(lastInst_->IsPhi());
            lastInst_->SetNext(rangeFirst);
        }
        firstInst_ = rangeFirst;
        lastInst_ = rangeLast;
    } else {
        ASSERT_PRINT(lastInst_, "Last instruction is undefined");
        rangeFirst->SetPrev(lastInst_);
        lastInst_->SetNext(rangeFirst);
        lastInst_ = rangeLast;
    }
}

void BasicBlock::InsertAfter(Inst *inst, Inst *after)
{
    ASSERT(inst && after);
    ASSERT(inst->IsPhi() == after->IsPhi());
    ASSERT(after->GetBasicBlock() == this);
    ASSERT(inst->GetBasicBlock() == nullptr);
    ASSERT(inst != nullptr);
    inst->SetBasicBlock(this);
    Inst *next = after->GetNext();
    inst->SetPrev(after);
    inst->SetNext(next);
    after->SetNext(inst);
    if (next != nullptr) {
        next->SetPrev(inst);
    } else {
        ASSERT(after == lastInst_);
        lastInst_ = inst;
    }
}

void BasicBlock::InsertBefore(Inst *inst, Inst *before)
{
    ASSERT(inst && before);
    ASSERT(inst->IsPhi() == before->IsPhi());
    ASSERT(before->GetBasicBlock() == this);
    ASSERT(inst->GetBasicBlock() == nullptr);
    ASSERT(inst != nullptr);
    inst->SetBasicBlock(this);
    Inst *prev = before->GetPrev();
    inst->SetPrev(prev);
    inst->SetNext(before);
    before->SetPrev(inst);
    if (prev != nullptr) {
        prev->SetNext(inst);
    }
    if (before == firstPhi_) {
        firstPhi_ = inst;
    }
    if (before == firstInst_) {
        firstInst_ = inst;
    }
}

void BasicBlock::InsertRangeBefore(Inst *rangeFirst, Inst *rangeLast, Inst *before)
{
#ifndef NDEBUG
    ASSERT(rangeFirst && rangeLast && rangeFirst->IsDominate(rangeLast));
    ASSERT(before && !before->IsPhi());
    ASSERT(rangeFirst->GetPrev() == nullptr);
    ASSERT(rangeLast->GetNext() == nullptr);
    ASSERT(before->GetBasicBlock() == this);
    auto instDb = rangeFirst;
    while (instDb != rangeLast) {
        ASSERT_PRINT(!instDb->IsPhi(), "Instruction mustn't be phi");
        ASSERT_PRINT(instDb->GetBasicBlock() == this, "Inst::SetBasicBlock() should be called beforehand");
        instDb = instDb->GetNext();
    }
    ASSERT_PRINT(!instDb->IsPhi(), "Instruction mustn't be phi");
    ASSERT_PRINT(instDb->GetBasicBlock() == this, "Inst::SetBasicBlock() should be called beforehand");
#endif

    Inst *prev = before->GetPrev();
    rangeFirst->SetPrev(prev);
    rangeLast->SetNext(before);
    before->SetPrev(rangeLast);
    if (prev != nullptr) {
        prev->SetNext(rangeFirst);
    }
    if (before == firstInst_) {
        firstInst_ = rangeFirst;
    }
}

void BasicBlock::ReplaceInst(Inst *oldInst, Inst *newInst)
{
    ASSERT(oldInst && newInst);
    ASSERT(oldInst->IsPhi() == newInst->IsPhi());
    ASSERT(oldInst->GetBasicBlock() == this);
    ASSERT(newInst->GetBasicBlock() == nullptr);
    newInst->SetBasicBlock(this);
    Inst *prev = oldInst->GetPrev();
    Inst *next = oldInst->GetNext();

    oldInst->SetBasicBlock(nullptr);
    if (prev != nullptr) {
        prev->SetNext(newInst);
    }
    if (next != nullptr) {
        next->SetPrev(newInst);
    }
    newInst->SetPrev(prev);
    newInst->SetNext(next);
    if (firstPhi_ == oldInst) {
        firstPhi_ = newInst;
    }
    if (firstInst_ == oldInst) {
        firstInst_ = newInst;
    }
    if (lastInst_ == oldInst) {
        lastInst_ = newInst;
    }

    if (graph_->IsInstThrowable(oldInst)) {
        graph_->ReplaceThrowableInst(oldInst, newInst);
    }
}

void BasicBlock::ReplaceInstByDeoptimize(Inst *inst)
{
    ASSERT(inst != nullptr);
    ASSERT(inst->GetBasicBlock() == this);
    auto ss = inst->GetSaveState();
    ASSERT(ss != nullptr);
    auto callInst = ss->GetCallerInst();
    // if inst in inlined method, we need to build all return.inlined before deoptimize to correct restore registers.
    while (callInst != nullptr && callInst->IsInlined()) {
        ss = callInst->GetSaveState();
        ASSERT(ss != nullptr);
        auto retInl = GetGraph()->CreateInstReturnInlined(DataType::NO_TYPE, INVALID_PC, ss);
        retInl->SetExtendedLiveness();
        InsertBefore(retInl, inst);
        callInst = ss->GetCallerInst();
    }
    // Replace Inst
    auto deopt = GetGraph()->CreateInstDeoptimize(DataType::NO_TYPE, inst->GetPc(), inst->GetSaveState());
    deopt->SetDeoptimizeType(inst);
    inst->RemoveInputs();
    inst->RemoveUsers();
    ReplaceInst(inst, deopt);
    // Build control flow
    BasicBlock *succ = GetSuccsBlocks()[0];
    if (GetLastInst() != deopt) {
        succ = SplitBlockAfterInstruction(deopt, true);
    }
    ASSERT(GetSuccsBlocks().size() == 1);
    GetGraph()->RemoveSuccessors(this);
    auto endBlock = GetGraph()->HasEndBlock() ? GetGraph()->GetEndBlock() : GetGraph()->CreateEndBlock();
    ASSERT(endBlock->GetGraph() != nullptr);
    this->AddSucc(endBlock);
    ASSERT(GetGraph()->IsAnalysisValid<LoopAnalyzer>());
    GetGraph()->DisconnectBlockRec(succ, true, false);
}

void BasicBlock::RemoveOverflowCheck(Inst *inst)
{
    // replace by Add/Sub
    Inst *newInst = nullptr;
    switch (inst->GetOpcode()) {
        case Opcode::AddOverflowCheck:
            newInst = GetGraph()->CreateInstAdd(inst->GetType(), inst->GetPc());
            break;
        case Opcode::SubOverflowCheck:
            newInst = GetGraph()->CreateInstSub(inst->GetType(), inst->GetPc());
            break;
        case Opcode::NegOverflowAndZeroCheck:
            newInst = GetGraph()->CreateInstNeg(inst->GetType(), inst->GetPc());
            break;
        default:
            UNREACHABLE();
    }
    // clone inputs, except savestate
    for (size_t i = 0; i < inst->GetInputsCount() - 1; ++i) {
        newInst->SetInput(i, inst->GetInput(i).GetInst());
    }
    inst->ReplaceUsers(newInst);
    inst->RemoveInputs();
    ReplaceInst(inst, newInst);
}

void BasicBlock::EraseInst(Inst *inst, [[maybe_unused]] bool willBeMoved)
{
    ASSERT(willBeMoved || !GetGraph()->IsInstThrowable(inst));
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
    if (inst == firstPhi_) {
        firstPhi_ = (next != nullptr && next->IsPhi()) ? next : nullptr;
    }
    if (inst == firstInst_) {
        firstInst_ = next;
    }
    if (inst == lastInst_) {
        lastInst_ = prev;
    }
}

void BasicBlock::RemoveInst(Inst *inst)
{
    inst->RemoveUsers();
    ASSERT(!inst->HasUsers());
    inst->RemoveInputs();
    if (inst->GetOpcode() == Opcode::NullPtr) {
        graph_->UnsetNullPtrInst();
    } else if (inst->GetOpcode() == Opcode::LoadUniqueObject) {
        graph_->UnsetUniqueObjectInst();
    } else if (inst->GetOpcode() == Opcode::Constant) {
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
    BasicBlock *domBlock = other->GetDominator();
    while (domBlock != nullptr) {
        if (domBlock == this) {
            return true;
        }
        // Otherwise we are in infinite loop!?
        ASSERT(domBlock != domBlock->GetDominator());
        domBlock = domBlock->GetDominator();
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
        ASSERT(dominator != nullptr);
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
    return domBlocks_;
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

BasicBlock *BasicBlock::Clone(Graph *targetGraph) const
{
    BasicBlock *clone = nullptr;
#ifndef NDEBUG
    if (GetGraph() == targetGraph) {
        clone = targetGraph->CreateEmptyBlock();
    } else {
        clone = targetGraph->CreateEmptyBlock(GetId(), GetGuestPc());
    }
#else
    clone = targetGraph->CreateEmptyBlock();
#endif
    ASSERT(clone != nullptr);
    clone->SetAllFields(GetAllFields());
    clone->tryId_ = GetTryId();
    if (IsStartBlock()) {
        targetGraph->SetStartBlock(clone);
    } else if (IsEndBlock()) {
        targetGraph->SetEndBlock(clone);
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

Inst *BasicBlock::FindSaveStateDeoptimize() const
{
    for (auto inst : InstsReverse()) {
        if (inst->GetOpcode() == Opcode::SaveStateDeoptimize) {
            return inst;
        }
    }
    return nullptr;
}

SaveStateInst *BasicBlock::FindSaveState(Inst *start) const
{
    for (auto inst : InstReverseIter(*this, start)) {
        if (inst->GetOpcode() == Opcode::SaveState) {
            return inst->CastToSaveState();
        }
    }
    return nullptr;
}

void BasicBlock::InvalidateLoopIfIrreducible()
{
    auto loop = GetLoop();
    ASSERT(loop != nullptr);
    if (loop->IsIrreducible()) {
        GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    }
}

uint32_t BasicBlock::CountInsts() const
{
    uint32_t count = 0;
    for ([[maybe_unused]] auto inst : AllInsts()) {
        count++;
    }
    return count;
}

bool BlocksPathDfsSearch(Marker marker, BasicBlock *block, const BasicBlock *targetBlock,
                         const BasicBlock *excludeBlock)
{
    ASSERT(marker != UNDEF_MARKER);
    if (block == targetBlock) {
        return true;
    }
    block->SetMarker(marker);

    for (auto succBlock : block->GetSuccsBlocks()) {
        if (!succBlock->IsMarked(marker) && succBlock != excludeBlock) {
            if (BlocksPathDfsSearch(marker, succBlock, targetBlock, excludeBlock)) {
                return true;
            }
        }
    }
    return false;
}
}  // namespace ark::compiler
