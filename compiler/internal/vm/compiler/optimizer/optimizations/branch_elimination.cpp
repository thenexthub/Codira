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

#include "branch_elimination.h"
#include "compiler_logger.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/ir/basicblock.h"

namespace panda::compiler {
/**
 * Branch Elimination optimization finds `if-true` blocks with resolvable conditional instruction.
 * Condition can be resolved in the following ways:
 * - Condition is constant;
 */
BranchElimination::BranchElimination(Graph *graph)
    : Optimization(graph)
{
}

bool BranchElimination::RunImpl()
{
    GetGraph()->RunPass<DominatorsTree>();
    isApplied_ = false;
    auto markerHolder = MarkerHolder(GetGraph());
    rmBlockMarker_ = markerHolder.GetMarker();
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (block->IsEmpty() || (block->IsTry() && GetGraph()->IsBytecodeOptimizer())) {
            continue;
        }
        if (block->GetLastInst()->GetOpcode() == Opcode::IfImm) {
            VisitBlock(block);
        }
    }
    DisconnectBlocks();

    COMPILER_LOG(DEBUG, BRANCH_ELIM) << "Branch elimination complete";
    return isApplied_;
}

void BranchElimination::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    InvalidateBlocksOrderAnalyzes(GetGraph());
}

void BranchElimination::BranchEliminationConst(BasicBlock *ifBlock)
{
    auto ifImm = ifBlock->GetLastInst()->CastToIfImm();
    auto conditionInst = ifBlock->GetLastInst()->GetInput(0).GetInst();
    COMPILER_LOG(DEBUG, BRANCH_ELIM) << "Block with constant if instruction input is visited, id = "
                                     << ifBlock->GetId();

    uint64_t constValue = conditionInst->CastToConstant()->GetIntValue();
    bool condResult = (constValue == ifImm->GetImm());
    if (ifImm->GetCc() == CC_NE) {
        condResult = !condResult;
    } else {
        ASSERT(ifImm->GetCc() == CC_EQ);
    }
    auto eliminatedSuccessor = ifBlock->GetFalseSuccessor();
    if (!condResult) {
        eliminatedSuccessor = ifBlock->GetTrueSuccessor();
    }
    EliminateBranch(ifBlock, eliminatedSuccessor);
    GetGraph()->GetEventWriter().EventBranchElimination(ifBlock->GetId(), ifBlock->GetGuestPc(), conditionInst->GetId(),
                                                        conditionInst->GetPc(), "const-condition",
                                                        eliminatedSuccessor == ifBlock->GetTrueSuccessor());
}

void BranchElimination::BranchEliminationIntrinsic(BasicBlock *ifBlock)
{
    auto ifImm = ifBlock->GetLastInst()->CastToIfImm();
    auto conditionInst = ifBlock->GetLastInst()->GetInput(0).GetInst()->CastToIntrinsic();
    if (conditionInst->CastToIntrinsic()->GetIntrinsicId() != IntrinsicInst::IntrinsicId::LDTRUE &&
        conditionInst->CastToIntrinsic()->GetIntrinsicId() != IntrinsicInst::IntrinsicId::LDFALSE) {
        return;
    }

    COMPILER_LOG(DEBUG, BRANCH_ELIM) << "Block with intrinsic ldtrue/ldfalse if instruction input is visited, id = "
                                     << ifBlock->GetId();

    bool constValue = conditionInst->GetIntrinsicId() == IntrinsicInst::IntrinsicId::LDTRUE;
    ASSERT(ifImm->GetImm() == 0);
    bool condResult = (constValue == ifImm->GetImm());
    if (ifImm->GetCc() == CC_NE) {
        condResult = !condResult;
    } else {
        ASSERT(ifImm->GetCc() == CC_EQ);
    }
    auto eliminatedSuccessor = ifBlock->GetFalseSuccessor();
    if (!condResult) {
        eliminatedSuccessor = ifBlock->GetTrueSuccessor();
    }
    EliminateBranch(ifBlock, eliminatedSuccessor);
    GetGraph()->GetEventWriter().EventBranchElimination(ifBlock->GetId(), ifBlock->GetGuestPc(), conditionInst->GetId(),
                                                        conditionInst->GetPc(), "const-condition",
                                                        eliminatedSuccessor == ifBlock->GetTrueSuccessor());
}

/**
 * Select unreachable successor and run elimination process
 * @param blocks - list of blocks with constant `if` instruction input
 */
void BranchElimination::VisitBlock(BasicBlock *ifBlock)
{
    ASSERT(ifBlock != nullptr);
    ASSERT(ifBlock->GetGraph() == GetGraph());
    ASSERT(ifBlock->GetLastInst()->GetOpcode() == Opcode::IfImm);
    ASSERT(ifBlock->GetSuccsBlocks().size() == MAX_SUCCS_NUM);

    auto conditionInst = ifBlock->GetLastInst()->GetInput(0).GetInst();
    switch (conditionInst->GetOpcode()) {
        case Opcode::Constant:
            BranchEliminationConst(ifBlock);
            break;
        case Opcode::Intrinsic:
            BranchEliminationIntrinsic(ifBlock);
            break;
        default:
            break;
    }
}

/**
 * If `eliminated_block` has no other predecessor than `if_block`, mark `eliminated_block` to be disconnected
 * If `eliminated_block` dominates all predecessors, exclude `if_block`, mark `eliminated_block` to be disconneted
 * Else remove edge between these blocks
 * @param if_block - block with constant 'if' instruction input
 * @param eliminated_block - unreachable form `if_block`
 */
void BranchElimination::EliminateBranch(BasicBlock *ifBlock, BasicBlock *eliminatedBlock)
{
    ASSERT(ifBlock != nullptr && ifBlock->GetGraph() == GetGraph());
    ASSERT(eliminatedBlock != nullptr && eliminatedBlock->GetGraph() == GetGraph());
    // find predecessor which is not dominated by `eliminated_block`
    auto preds = eliminatedBlock->GetPredsBlocks();
    auto it = std::find_if(preds.begin(), preds.end(), [ifBlock, eliminatedBlock](BasicBlock *pred) {
        return pred != ifBlock && !eliminatedBlock->IsDominate(pred);
    });
    bool dominatesAllPreds = (it == preds.cend());
    if (preds.size() > 1 && !dominatesAllPreds) {
        RemovePredecessorUpdateDF(eliminatedBlock, ifBlock);
        ifBlock->RemoveSucc(eliminatedBlock);
        ifBlock->RemoveInst(ifBlock->GetLastInst());
        GetGraph()->GetAnalysis<Rpo>().SetValid(true);
        // NOTE (a.popov) DominatorsTree could be restored inplace
        GetGraph()->RunPass<DominatorsTree>();
    } else {
        eliminatedBlock->SetMarker(rmBlockMarker_);
    }
    isApplied_ = true;
}

/**
 * Before disconnecting the `block` find and disconnect all its successors dominated by it.
 * Use DFS to disconnect blocks in the PRO
 * @param block - unreachable block to disconnect from the graph
 */
void BranchElimination::MarkUnreachableBlocks(BasicBlock *block)
{
    for (auto dom : block->GetDominatedBlocks()) {
        dom->SetMarker(rmBlockMarker_);
        MarkUnreachableBlocks(dom);
    }
}

bool AllPredecessorsMarked(BasicBlock *block, Marker marker)
{
    if (block->GetPredsBlocks().empty()) {
        ASSERT(block->IsStartBlock());
        return false;
    }

    for (auto pred : block->GetPredsBlocks()) {
        if (!pred->IsMarked(marker)) {
            return false;
        }
    }
    block->SetMarker(marker);
    return true;
}

/// Disconnect selected blocks
void BranchElimination::DisconnectBlocks()
{
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (block->IsMarked(rmBlockMarker_) || AllPredecessorsMarked(block, rmBlockMarker_)) {
            MarkUnreachableBlocks(block);
        }
    }

    const auto &rpoBlocks = GetGraph()->GetBlocksRPO();
    for (auto it = rpoBlocks.rbegin(); it != rpoBlocks.rend(); it++) {
        auto block = *it;
        if (block != nullptr && block->IsMarked(rmBlockMarker_)) {
            GetGraph()->DisconnectBlock(block);
            COMPILER_LOG(DEBUG, BRANCH_ELIM) << "Block was disconnected, id = " << block->GetId();
        }
    }
}
}  // namespace panda::compiler

