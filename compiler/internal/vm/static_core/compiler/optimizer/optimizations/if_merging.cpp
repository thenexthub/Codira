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

#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/optimizations/if_merging.h"
#include "compiler_logger.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/analysis.h"

namespace ark::compiler {
IfMerging::IfMerging(Graph *graph) : Optimization(graph) {}

bool IfMerging::RunImpl()
{
    GetGraph()->RunPass<DominatorsTree>();
    // Do not try to fix LoopAnalyzer during optimization
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    isApplied_ = false;
    trueBranchMarker_ = GetGraph()->NewMarker();
    ArenaVector<BasicBlock *> blocks(GetGraph()->GetVectorBlocks(), GetGraph()->GetLocalAllocator()->Adapter());
    for (auto block : blocks) {
        if (block == nullptr) {
            continue;
        }
        if (TryMergeEquivalentIfs(block) || TrySimplifyConstantPhi(block)) {
            isApplied_ = true;
        }
    }
    GetGraph()->RemoveUnreachableBlocks();

#ifndef NDEBUG
    CheckDomTreeValid();
#endif
    GetGraph()->EraseMarker(trueBranchMarker_);
    COMPILER_LOG(DEBUG, IF_MERGING) << "If merging complete";
    return isApplied_;
}

void IfMerging::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}

// Splits block while it contains If comparing phi with constant inputs to another constant
// (After removing one If and joining blocks a new If may go to this block)
bool IfMerging::TrySimplifyConstantPhi(BasicBlock *block)
{
    auto isApplied = false;
    while (TryRemoveConstantPhiIf(block)) {
        isApplied = true;
    }
    return isApplied;
}

// Tries to remove If that compares phi with constant inputs to another constant
bool IfMerging::TryRemoveConstantPhiIf(BasicBlock *ifBlock)
{
    auto ifImm = GetIfImm(ifBlock);
    if (ifImm == nullptr || ifBlock->GetGraph() == nullptr || ifBlock->IsTry()) {
        return false;
    }
    auto input = ifImm->GetInput(0).GetInst();
    if (input->GetOpcode() != Opcode::Compare) {
        return false;
    }
    auto compare = input->CastToCompare();
    auto lhs = compare->GetInput(0).GetInst();
    auto rhs = compare->GetInput(1).GetInst();
    if (rhs->IsConst() && lhs->GetOpcode() == Opcode::Phi &&
        TryRemoveConstantPhiIf(ifImm, lhs->CastToPhi(), rhs->CastToConstant()->GetRawValue(), compare->GetCc())) {
        return true;
    }
    if (lhs->IsConst() && rhs->GetOpcode() == Opcode::Phi &&
        TryRemoveConstantPhiIf(ifImm, rhs->CastToPhi(), lhs->CastToConstant()->GetRawValue(),
                               SwapOperandsConditionCode(compare->GetCc()))) {
        return true;
    }
    return false;
}

// Returns IfImm instruction terminating the block or nullptr
IfImmInst *IfMerging::GetIfImm(BasicBlock *block)
{
    if (block->IsEmpty() || block->GetLastInst()->GetOpcode() != Opcode::IfImm) {
        return nullptr;
    }
    auto ifImm = block->GetLastInst()->CastToIfImm();
    if ((ifImm->GetCc() != CC_EQ && ifImm->GetCc() != CC_NE) || ifImm->GetImm() != 0) {
        return nullptr;
    }
    return ifImm;
}

// If bb and its dominator end with the same IfImm instruction, removes IfImm in bb and returns true
bool IfMerging::TryMergeEquivalentIfs(BasicBlock *bb)
{
    auto ifImm = GetIfImm(bb);
    if (ifImm == nullptr || bb->GetGraph() == nullptr || bb->IsTry()) {
        return false;
    }
    if (bb->GetPredsBlocks().size() != MAX_SUCCS_NUM) {
        return false;
    }
    auto dom = bb->GetDominator();
    auto domIf = GetIfImm(dom);
    if (domIf == nullptr || domIf->GetInput(0).GetInst() != ifImm->GetInput(0).GetInst()) {
        return false;
    }
    auto invertedIf = IsIfInverted(bb, domIf);
    if (invertedIf == std::nullopt) {
        // Not applied, true and false branches of dominate If intersect
        return false;
    }
    auto trueBb = ifImm->GetEdgeIfInputTrue();
    auto falseBb = ifImm->GetEdgeIfInputFalse();
    if (!MarkInstBranches(bb, trueBb, falseBb)) {
        // Not applied, there are instructions in bb used both in true and false branches
        return false;
    }
    COMPILER_LOG(DEBUG, IF_MERGING) << "Equivalent IfImm instructions were found. Dominant block id = " << dom->GetId()
                                    << ", dominated block id = " << bb->GetId();
    bb->RemoveInst(ifImm);
    SplitBlockWithEquivalentIf(bb, trueBb, *invertedIf);
    return true;
}

// In case If has constant phi input and can be removed, removes it and returns true
bool IfMerging::TryRemoveConstantPhiIf(IfImmInst *ifImm, PhiInst *phi, uint64_t constant, ConditionCode cc)
{
    auto bb = phi->GetBasicBlock();
    if (bb != ifImm->GetBasicBlock()) {
        return false;
    }
    for (auto input : phi->GetInputs()) {
        if (!input.GetInst()->IsConst()) {
            return false;
        }
    }
    auto trueBb = ifImm->GetEdgeIfInputTrue();
    auto falseBb = ifImm->GetEdgeIfInputFalse();
    if (!MarkInstBranches(bb, trueBb, falseBb)) {
        // Not applied, there are instructions in bb used both in true and false branches
        return false;
    }
    for (auto inst : bb->PhiInsts()) {
        if (inst != phi) {
            return false;
        }
    }

    COMPILER_LOG(DEBUG, IF_MERGING) << "Comparison of constant and Phi instruction with constant inputs was found. "
                                    << "Phi id = " << phi->GetId() << ", IfImm id = " << ifImm->GetId();
    bb->RemoveInst(ifImm);
    SplitBlockWithConstantPhi(bb, trueBb, phi, constant, cc);
    return true;
}

// Marks instructions in bb for moving to true or false branch
// Returns true if it was possible
bool IfMerging::MarkInstBranches(BasicBlock *bb, BasicBlock *trueBb, BasicBlock *falseBb)
{
    for (auto inst : bb->AllInstsSafeReverse()) {
        if (inst->IsNotRemovable() && inst != bb->GetLastInst()) {
            return false;
        }
        auto trueBranch = false;
        auto falseBranch = false;
        for (auto &user : inst->GetUsers()) {
            auto userBranch = GetUserBranch(user.GetInst(), bb, trueBb, falseBb);
            if (!userBranch) {
                // If instruction has users not in true or false branch, than splitting is
                // impossible (even if the instruction is Phi)
                return false;
            }
            if (*userBranch) {
                trueBranch = true;
            } else {
                falseBranch = true;
            }
        }
        if (inst->IsPhi()) {
            // Phi instruction will be replaced with one of its inputs
            continue;
        }
        if (trueBranch && falseBranch) {
            // If instruction is used in both branches, it would need to be copied -> not applied
            return false;
        }
        if (trueBranch) {
            inst->SetMarker(trueBranchMarker_);
        } else {
            inst->ResetMarker(trueBranchMarker_);
        }
    }
    return true;
}

std::optional<bool> IfMerging::GetUserBranch(Inst *userInst, BasicBlock *bb, BasicBlock *trueBb, BasicBlock *falseBb)
{
    auto userBb = userInst->GetBasicBlock();
    if (userBb == bb) {
        return userInst->IsMarked(trueBranchMarker_);
    }
    if (IsDominateEdge(trueBb, userBb)) {
        // user_inst can be executed only in true branch of If
        return true;
    }
    if (IsDominateEdge(falseBb, userBb)) {
        // user_inst can be executed only in false branch of If
        return false;
    }
    // user_inst can be executed both in true and false branches
    return std::nullopt;
}

// Returns true if edge_bb is always the first block in the path
// from its predecessor to target_bb
bool IfMerging::IsDominateEdge(BasicBlock *edgeBb, BasicBlock *targetBb)
{
    ASSERT(edgeBb != nullptr);
    return edgeBb->IsDominate(targetBb) && edgeBb->GetPredsBlocks().size() == 1;
}

void IfMerging::SplitBlockWithEquivalentIf(BasicBlock *bb, BasicBlock *trueBb, bool invertedIf)
{
    auto trueBranchBb = SplitBlock(bb);
    // Phi instructions are replaced with one of their inputs
    for (auto inst : bb->PhiInstsSafe()) {
        for (auto it = inst->GetUsers().begin(); it != inst->GetUsers().end(); it = inst->GetUsers().begin()) {
            auto userInst = it->GetInst();
            auto userBb = userInst->GetBasicBlock();
            auto phiInputBlockIndex = (userBb == trueBranchBb || IsDominateEdge(trueBb, userBb)) ? 0 : 1;
            if (invertedIf) {
                phiInputBlockIndex = 1 - phiInputBlockIndex;
            }
            userInst->ReplaceInput(inst, inst->CastToPhi()->GetPhiInput(bb->GetPredecessor(phiInputBlockIndex)));
        }
        bb->RemoveInst(inst);
    }

    // True branches of both Ifs are disconnected from bb and connected to the new block
    trueBb->ReplacePred(bb, trueBranchBb);
    bb->RemoveSucc(trueBb);
    auto truePred = bb->GetPredecessor(invertedIf ? 1 : 0);
    truePred->ReplaceSucc(bb, trueBranchBb);
    bb->RemovePred(truePred);

    FixDominatorsTree(trueBranchBb, bb);
}

void IfMerging::SplitBlockWithConstantPhi(BasicBlock *bb, BasicBlock *trueBb, PhiInst *phi, uint64_t constant,
                                          ConditionCode cc)
{
    if (trueBb == bb) {
        // Avoid case when true_bb is bb itself
        trueBb = bb->InsertNewBlockToSuccEdge(trueBb);
    }
    auto trueBranchBb = SplitBlock(bb);
    // Move Phi inputs corresponding to true branch to a new Phi instruction
    auto trueBranchPhi = GetGraph()->CreateInstPhi(phi->GetType(), phi->GetPc());
    for (auto i = phi->GetInputsCount(); i > 0U; --i) {
        auto inst = phi->GetInput(i - 1).GetInst();
        if (Compare(cc, inst->CastToConstant()->GetRawValue(), constant)) {
            auto bbIndex = phi->GetPhiInputBbNum(i - 1);
            phi->RemoveInput(i - 1);
            bb->GetPredBlockByIndex(bbIndex)->ReplaceSucc(bb, trueBranchBb);
            bb->RemovePred(bbIndex);
            trueBranchPhi->AppendInput(inst);
        }
    }
    for (auto it = phi->GetUsers().begin(); it != phi->GetUsers().end();) {
        auto userInst = it->GetInst();
        auto userBb = userInst->GetBasicBlock();
        // Skip list nodes that can be deleted inside ReplaceInput
        while (it != phi->GetUsers().end() && it->GetInst() == userInst) {
            ++it;
        }
        if (userBb == trueBranchBb || IsDominateEdge(trueBb, userBb)) {
            userInst->ReplaceInput(phi, trueBranchPhi);
        }
    }
    // Phi instructions with single inputs need to be removed
    if (trueBranchPhi->GetInputsCount() == 1) {
        trueBranchPhi->ReplaceUsers(trueBranchPhi->GetInput(0).GetInst());
        trueBranchPhi->RemoveInputs();
    } else {
        trueBranchBb->AppendPhi(trueBranchPhi);
    }
    if (phi->GetInputsCount() == 1) {
        phi->ReplaceUsers(phi->GetInput(0).GetInst());
        bb->RemoveInst(phi);
    }

    // True branches of both Ifs are disconnected from bb and connected to the new block
    trueBb->ReplacePred(bb, trueBranchBb);
    bb->RemoveSucc(trueBb);
    FixDominatorsTree(trueBranchBb, bb);
}

// Moves instructions that should be in the true branch to a new block and returns it
BasicBlock *IfMerging::SplitBlock(BasicBlock *bb)
{
    auto trueBranchBb = GetGraph()->CreateEmptyBlock(bb->GetGuestPc());
    // Dominators tree will be fixed after connecting the new block
    GetGraph()->GetAnalysis<DominatorsTree>().SetValid(true);
    for (auto inst : bb->InstsSafeReverse()) {
        if (inst->IsMarked(trueBranchMarker_)) {
            bb->EraseInst(inst);
            ASSERT(trueBranchBb != nullptr);
            trueBranchBb->PrependInst(inst);
        }
    }
    return trueBranchBb;
}

// Makes dominator tree valid after false_branch_bb was split into two blocks
void IfMerging::FixDominatorsTree(BasicBlock *trueBranchBb, BasicBlock *falseBranchBb)
{
    auto &domTree = GetGraph()->GetAnalysis<DominatorsTree>();
    auto dom = falseBranchBb->GetDominator();
    // Dominator of bb will remain (maybe not immediate) dominator of blocks dominated by bb
    for (auto dominatedBlock : falseBranchBb->GetDominatedBlocks()) {
        domTree.SetDomPair(dom, dominatedBlock);
    }
    falseBranchBb->ClearDominatedBlocks();

    TryJoinSuccessorBlock(trueBranchBb);
    TryJoinSuccessorBlock(falseBranchBb);
    TryUpdateDominator(trueBranchBb);
    TryUpdateDominator(falseBranchBb);
}

void IfMerging::TryJoinSuccessorBlock(BasicBlock *bb)
{
    if (bb->GetSuccsBlocks().size() == 1 && bb->GetSuccessor(0)->GetPredsBlocks().size() == 1 &&
        !bb->GetSuccessor(0)->IsPseudoControlFlowBlock() && bb->IsTry() == bb->GetSuccessor(0)->IsTry() &&
        bb->GetSuccessor(0) != bb) {
        bb->JoinSuccessorBlock();
    }
}

// If bb has only one precessor, set it as dominator of bb
void IfMerging::TryUpdateDominator(BasicBlock *bb)
{
    if (bb->GetPredsBlocks().size() != 1) {
        return;
    }
    auto pred = bb->GetPredecessor(0);
    auto dom = bb->GetDominator();
    if (dom != pred) {
        if (dom != nullptr) {
            dom->RemoveDominatedBlock(bb);
        }
        GetGraph()->GetAnalysis<DominatorsTree>().SetDomPair(pred, bb);
    }
}

#ifndef NDEBUG
// Checks if dominators in dominators tree are indeed dominators (but not necessary immediate)
void IfMerging::CheckDomTreeValid()
{
    ArenaVector<BasicBlock *> dominators(GetGraph()->GetVectorBlocks().size(),
                                         GetGraph()->GetLocalAllocator()->Adapter());
    GetGraph()->GetAnalysis<DominatorsTree>().SetValid(true);
    for (auto block : GetGraph()->GetBlocksRPO()) {
        dominators[block->GetId()] = block->GetDominator();
    }
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
    GetGraph()->RunPass<DominatorsTree>();

    for (auto block : GetGraph()->GetBlocksRPO()) {
        auto dom = dominators[block->GetId()];
        ASSERT_DO(dom == nullptr || dom->IsDominate(block),
                  (std::cerr << "Basic block with id " << block->GetId() << " has incorrect dominator with id "
                             << dom->GetId() << std::endl));
    }
}
#endif
}  // namespace ark::compiler
