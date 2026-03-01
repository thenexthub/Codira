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
#include "optimizer/optimizations/branch_elimination.h"
#include "compiler_logger.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/analysis.h"

namespace ark::compiler {
/**
 * Branch Elimination optimization finds `if-true` blocks with resolvable conditional instruction.
 * Condition can be resolved in the following ways:
 * - Condition is constant;
 * - Condition is dominated by the equal one condition with the same inputs and the only one successor of the dominant
 * reaches dominated condition
 */
BranchElimination::BranchElimination(Graph *graph)
    : Optimization(graph),
      sameInputCompares_(graph->GetLocalAllocator()->Adapter()),
      sameInputCompareAnyType_(graph->GetLocalAllocator()->Adapter())
{
}

bool BranchElimination::RunImpl()
{
    GetGraph()->RunPass<DominatorsTree>();
    isApplied_ = false;
    sameInputCompares_.clear();
    auto markerHolder = MarkerHolder(GetGraph());
    rmBlockMarker_ = markerHolder.GetMarker();
    // Additional removal marker for blocks that were NOT originally marked for elimination, but have ALL their
    // predecessors marked for elimination.
    auto auxMarkerHolder = MarkerHolder(GetGraph());
    auxRmMarker_ = auxMarkerHolder.GetMarker();
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (block->IsEmpty() || (block->IsTry() && GetGraph()->IsBytecodeOptimizer())) {
            continue;
        }
        if (block->GetLastInst()->GetOpcode() == Opcode::IfImm) {
            if (SkipForOsr(block)) {
                COMPILER_LOG(DEBUG, BRANCH_ELIM) << "Skip for OSR, id = " << block->GetId();
                continue;
            }
            /* skip branch() elimination at the end of the
             * preheader until LoopUnrolling pass is done */
            if (!GetGraph()->IsBytecodeOptimizer() && g_options.IsCompilerDeferPreheaderTransform() &&
                !GetGraph()->IsUnrollComplete() && block->IsLoopValid() && block->IsLoopPreHeader()) {
                continue;
            }
            VisitBlock(block);
        }
    }
    DisconnectBlocks();

    if (isApplied_ && GetGraph()->IsOsrMode()) {
        CleanupGraphSaveStateOSR(GetGraph());
    }
    COMPILER_LOG(DEBUG, BRANCH_ELIM) << "Branch elimination complete";
    return isApplied_;
}

bool BranchElimination::SkipForOsr(const BasicBlock *block)
{
    if (!block->IsLoopValid()) {
        return false;
    }
    auto loop = block->GetLoop();
    if (loop->IsRoot() || !loop->GetHeader()->IsOsrEntry()) {
        return false;
    }
    if (loop->GetBackEdges().size() > 1) {
        return false;
    }
    return block->IsDominate(loop->GetBackEdges()[0]);
}

void BranchElimination::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
    // Before in "CleanupGraphSaveStateOSR" already run "LoopAnalyzer"
    // in case (is_applied_ && GetGraph()->IsOsrMode())
    if (!(isApplied_ && GetGraph()->IsOsrMode())) {
        GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    }
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

void BranchElimination::BranchEliminationCompare(BasicBlock *ifBlock)
{
    auto ifImm = ifBlock->GetLastInst()->CastToIfImm();
    auto conditionInst = ifBlock->GetLastInst()->GetInput(0).GetInst();
    if (auto result = GetConditionResult(conditionInst)) {
        COMPILER_LOG(DEBUG, BRANCH_ELIM) << "Compare instruction result was resolved. Instruction id = "
                                         << conditionInst->GetId()
                                         << ", resolved result: " << (result.value() ? "true" : "false");
        auto eliminatedSuccessor = ifImm->GetEdgeIfInputFalse();
        if (!result.value()) {
            eliminatedSuccessor = ifImm->GetEdgeIfInputTrue();
        }
        EliminateBranch(ifBlock, eliminatedSuccessor);
        GetGraph()->GetEventWriter().EventBranchElimination(
            ifBlock->GetId(), ifBlock->GetGuestPc(), conditionInst->GetId(), conditionInst->GetPc(), "dominant-if",
            eliminatedSuccessor == ifBlock->GetTrueSuccessor());
    } else {
        ConditionOps ops {conditionInst->GetInput(0).GetInst(), conditionInst->GetInput(1).GetInst()};
        auto it = sameInputCompares_.try_emplace(ops, GetGraph()->GetLocalAllocator()->Adapter());
        it.first->second.push_back(conditionInst);
    }
}

void BranchElimination::BranchEliminationCompareAnyType(BasicBlock *ifBlock)
{
    auto ifImm = ifBlock->GetLastInst()->CastToIfImm();
    auto compareAny = ifBlock->GetLastInst()->GetInput(0).GetInst()->CastToCompareAnyType();
    if (auto result = GetCompareAnyTypeResult(ifImm)) {
        COMPILER_LOG(DEBUG, BRANCH_ELIM) << "CompareAnyType instruction result was resolved. Instruction id = "
                                         << compareAny->GetId()
                                         << ", resolved result: " << (result.value() ? "true" : "false");
        auto eliminatedSuccessor = ifImm->GetEdgeIfInputFalse();
        if (!result.value()) {
            eliminatedSuccessor = ifImm->GetEdgeIfInputTrue();
        }
        EliminateBranch(ifBlock, eliminatedSuccessor);
        GetGraph()->GetEventWriter().EventBranchElimination(ifBlock->GetId(), ifBlock->GetGuestPc(),
                                                            compareAny->GetId(), compareAny->GetPc(), "dominant-if",
                                                            eliminatedSuccessor == ifBlock->GetTrueSuccessor());
        return;
    }

    Inst *input = compareAny->GetInput(0).GetInst();
    auto it = sameInputCompareAnyType_.try_emplace(input, GetGraph()->GetLocalAllocator()->Adapter());
    it.first->second.push_back(compareAny);
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
        case Opcode::Compare:
            BranchEliminationCompare(ifBlock);
            break;
        case Opcode::CompareAnyType:
            BranchEliminationCompareAnyType(ifBlock);
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
        GetGraph()->RemovePredecessorUpdateDF(eliminatedBlock, ifBlock);
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
        dom->SetMarker(auxRmMarker_);
        MarkUnreachableBlocks(dom);
    }
}

bool AllPredecessorsMarked(BasicBlock *block, Marker rmBlockMarker, Marker auxRmMarker)
{
    if (block->GetPredsBlocks().empty()) {
        ASSERT(block->IsStartBlock());
        return false;
    }

    for (auto pred : block->GetPredsBlocks()) {
        if (!pred->IsMarked(rmBlockMarker)) {
            return false;
        }
    }
    block->SetMarker(rmBlockMarker);
    block->SetMarker(auxRmMarker);
    return true;
}

void BranchElimination::UnmarkConnectionPath(BasicBlock *current, BasicBlock *target)
{
    if (current == target || !current->IsMarked(rmBlockMarker_)) {
        return;
    }

    current->ResetMarker(rmBlockMarker_);
    current->ResetMarker(auxRmMarker_);

    // Unmark all predecessors of the current block.
    for (auto *pred : current->GetPredsBlocks()) {
        if (pred->IsMarked(rmBlockMarker_)) {
            UnmarkConnectionPath(pred, target);
        }
    }
}

void BranchElimination::UnmarkConnectionToLiveCode(BasicBlock *block)
{
    if (!block->IsMarked(rmBlockMarker_)) {
        return;
    }

    block->ResetMarker(rmBlockMarker_);
    block->ResetMarker(auxRmMarker_);

    for (auto *pred : block->GetSuccsBlocks()) {
        UnmarkConnectionToLiveCode(pred);
    }
}

bool BranchElimination::HasThrowInBlock(const BasicBlock *block) const
{
    if (block->IsEmpty()) {
        return false;
    }

    return block->GetLastInst()->GetOpcode() == Opcode::Throw;
}

bool BranchElimination::IsNeedToProtect(BasicBlock *block) const
{
    if (HasThrowInBlock(block)) {
        return false;
    }

    MarkerHolder markerHolder(GetGraph());
    auto marker = markerHolder.GetMarker();
    if (HasThrowInSuccessors(block, marker)) {
        return false;
    }

    block->ResetMarker(marker);

    return !HasThrowInPredecessors(block, marker);
}

bool BranchElimination::HasThrowInSuccessors(BasicBlock *block, Marker marker) const
{
    if (block->IsMarked(marker)) {
        return false;
    }

    block->SetMarker(marker);

    if (HasThrowInBlock(block)) {
        return true;
    }

    for (auto succ : block->GetSuccsBlocks()) {
        if (HasThrowInSuccessors(succ, marker)) {
            return true;
        }
    }

    return false;
}

bool BranchElimination::HasThrowInPredecessors(BasicBlock *block, Marker marker) const
{
    if (block->IsMarked(marker)) {
        return false;
    }

    block->SetMarker(marker);

    if (HasThrowInBlock(block)) {
        return true;
    }

    for (auto pred : block->GetPredsBlocks()) {
        if (pred->IsMarked(rmBlockMarker_)) {
            if (HasThrowInPredecessors(pred, marker)) {
                return true;
            }
        }
    }

    return false;
}

void BranchElimination::ProcessReturnInlinedInstruction(Inst *inst, BasicBlock *block)
{
    // If BE has marked a block with ReturnInlined as unreachable (flagged with rmBlockMarker_), we can safely remove
    // such block or if either its successor or predecessor references a throw block.
    if (!block->IsMarked(auxRmMarker_) || !IsNeedToProtect(block)) {
        return;
    }

    // Only proceed if the calling block isn't marked for deletion.
    auto *callBlock = const_cast<BasicBlock *>(inst->CastToReturnInlined()->GetSaveState()->GetBasicBlock());
    if (callBlock->IsMarked(rmBlockMarker_) || callBlock->IsMarked(auxRmMarker_)) {
        return;
    }

    // Restore the execution path between the call and return: unmark all blocks along the call-inlined
    // to return-inlined path. This maintains the inlined function's control flow.
    UnmarkConnectionPath(block, callBlock);

    // We also need to restore the path to the first live successor.
    for (auto *succ : block->GetSuccsBlocks()) {
        if (succ->IsMarked(rmBlockMarker_)) {
            UnmarkConnectionToLiveCode(succ);
        }
    }
}

void BranchElimination::ProtectReturnInlinedPaths()
{
    for (auto *block : GetGraph()->GetBlocksRPO()) {
        for (auto *inst : block->AllInsts()) {
            if (inst->GetOpcode() == Opcode::ReturnInlined) {
                ProcessReturnInlinedInstruction(inst, block);
            }
        }
    }
}

/// Disconnect selected blocks
void BranchElimination::DisconnectBlocks()
{
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (block->IsMarked(rmBlockMarker_) || AllPredecessorsMarked(block, rmBlockMarker_, auxRmMarker_)) {
            MarkUnreachableBlocks(block);
        }
    }

    // Protect some blocks containing ReturnInlined instructions from being deleted.
    ProtectReturnInlinedPaths();

    const auto &rpoBlocks = GetGraph()->GetBlocksRPO();
    for (auto it = rpoBlocks.rbegin(); it != rpoBlocks.rend(); it++) {
        auto block = *it;
        if (block != nullptr && block->IsMarked(rmBlockMarker_)) {
            GetGraph()->DisconnectBlock(block);
            COMPILER_LOG(DEBUG, BRANCH_ELIM) << "Block was disconnected, id = " << block->GetId();
        }
    }
}

/**
 * Check if the `target_block` is reachable from one successor only of the `dominant_block`
 *
 * If `target_block` is dominated by one of the successors, need to check that `target_block`
 * is NOT reachable by the other successor
 */
bool BlockIsReachedFromOnlySuccessor(BasicBlock *targetBlock, BasicBlock *dominantBlock)
{
    ASSERT(dominantBlock->IsDominate(targetBlock));
    BasicBlock *otherSuccesor = nullptr;
    if (dominantBlock->GetTrueSuccessor()->IsDominate(targetBlock)) {
        otherSuccesor = dominantBlock->GetFalseSuccessor();
    } else if (dominantBlock->GetFalseSuccessor()->IsDominate(targetBlock)) {
        otherSuccesor = dominantBlock->GetTrueSuccessor();
    } else {
        return false;
    }

    auto markerHolder = MarkerHolder(targetBlock->GetGraph());
    if (BlocksPathDfsSearch(markerHolder.GetMarker(), otherSuccesor, targetBlock)) {
        return false;
    }
    ASSERT(!otherSuccesor->IsDominate(targetBlock));
    return true;
}

/**
 * NOTE (a.popov) Here can be supported more complex case:
 * when `dom_compare` has 2 or more `if_imm` users and `target_compare` is reachable from the same successors of these
 * if_imms
 */
Inst *FindIfImmDominatesCondition(Inst *domCompare, Inst *targetCompare)
{
    for (auto &user : domCompare->GetUsers()) {
        auto inst = user.GetInst();
        if (inst->GetOpcode() == Opcode::IfImm && inst->IsDominate(targetCompare)) {
            return inst;
        }
    }
    return nullptr;
}

/// Resolve condition result if there is a dominant equal condition
std::optional<bool> BranchElimination::GetConditionResult(Inst *condition)
{
    ConditionOps ops {condition->GetInput(0).GetInst(), condition->GetInput(1).GetInst()};
    if (sameInputCompares_.count(ops) <= 0) {
        return std::nullopt;
    }
    auto instructions = sameInputCompares_.at(ops);
    ASSERT(!instructions.empty());
    for (auto domCond : instructions) {
        // Find dom_cond's if_imm, that dominates target condition
        auto ifImm = FindIfImmDominatesCondition(domCond, condition);
        if (ifImm == nullptr) {
            continue;
        }
        if (BlockIsReachedFromOnlySuccessor(condition->GetBasicBlock(), ifImm->GetBasicBlock())) {
            if (auto result = TryResolveResult(condition, domCond, ifImm->CastToIfImm())) {
                COMPILER_LOG(DEBUG, BRANCH_ELIM)
                    << "Equal compare instructions were found. Dominant id = " << domCond->GetId()
                    << ", dominated id = " << condition->GetId();
                return result;
            }
        }
    }
    return std::nullopt;
}

/// Resolve condition result if there is a dominant IfImmInst after CompareAnyTypeInst.
std::optional<bool> BranchElimination::GetCompareAnyTypeResult(IfImmInst *ifImm)
{
    auto compareAny = ifImm->GetInput(0).GetInst()->CastToCompareAnyType();
    Inst *input = compareAny->GetInput(0).GetInst();
    const auto it = sameInputCompareAnyType_.find(input);
    if (it == sameInputCompareAnyType_.end()) {
        return std::nullopt;
    }

    const ArenaVector<CompareAnyTypeInst *> &instructions = it->second;
    ASSERT(!instructions.empty());
    for (const auto domCompareAny : instructions) {
        // Find dom_cond's if_imm, that dominates target condition.
        auto ifImmDomBlock = FindIfImmDominatesCondition(domCompareAny, ifImm);
        if (ifImmDomBlock == nullptr) {
            continue;
        }

        if (!BlockIsReachedFromOnlySuccessor(ifImm->GetBasicBlock(), ifImmDomBlock->GetBasicBlock())) {
            continue;
        }

        auto result = TryResolveCompareAnyTypeResult(compareAny, domCompareAny, ifImmDomBlock->CastToIfImm());
        if (!result) {
            continue;
        }

        COMPILER_LOG(DEBUG, BRANCH_ELIM) << "Equal CompareAnyType instructions were found. Dominant id = "
                                         << domCompareAny->GetId() << ", dominated id = " << compareAny->GetId();
        return result;
    }
    return std::nullopt;
}

/**
 * Try to resolve CompareAnyTypeInst result with the information
 * about dominant CompareAnyTypeInst with the same inputs.
 */
std::optional<bool> BranchElimination::TryResolveCompareAnyTypeResult(CompareAnyTypeInst *compareAny,
                                                                      CompareAnyTypeInst *domCompareAny,
                                                                      IfImmInst *ifImmDomBlock)
{
    auto compareAnyBb = compareAny->GetBasicBlock();
    bool isTrueDomBranch = ifImmDomBlock->GetEdgeIfInputTrue()->IsDominate(compareAnyBb);

    auto graph = compareAnyBb->GetGraph();
    auto language = graph->GetRuntime()->GetMethodSourceLanguage(graph->GetMethod());
    auto res = IsAnyTypeCanBeSubtypeOf(language, compareAny->GetAnyType(), domCompareAny->GetAnyType());
    if (!res) {
        // We cannot compare types in compile-time
        return std::nullopt;
    }

    if (res.value()) {
        // If CompareAnyTypeInst has the same types for any, then it can be optimized in any case.
        return isTrueDomBranch;
    }

    // If CompareAnyTypeInst has the different types for any, then it can be optimized only in true-branch.
    if (!isTrueDomBranch) {
        return std::nullopt;
    }

    return false;
}

/**
 * Try to resolve condition result with the information about dominant condition with the same inputs
 *
 * - The result of the dominant condition is counted using dom-tree by checking which successor of if_imm_block
 * (true/false) dominates the current block;
 *
 * - Then this result is applied to the current condition, if it is possible, using the table of condition codes
 * relation
 */
std::optional<bool> BranchElimination::TryResolveResult(Inst *condition, Inst *dominantCondition, IfImmInst *ifImm)
{
    using std::nullopt;

    // Table keeps the result of the condition in the row if the condition in the column is true
    // Order must be same as in "ir/inst.h"
    static constexpr std::array<std::array<std::optional<bool>, ConditionCode::CC_LAST + 1>, ConditionCode::CC_LAST + 1>
        // clang-format off
        COND_RELATION = {{
        //  CC_EQ     CC_NE    CC_LT    CC_LE    CC_GT    CC_GE    CC_B     CC_BE    CC_A     CC_AE
            {true,    false,   false,   nullopt, false,   nullopt, false,   nullopt, false,   nullopt}, // CC_EQ
            {false,   true,    true,    nullopt, true,    nullopt, true,    nullopt, true,    nullopt}, // CC_NE
            {false,   nullopt, true,    nullopt, false,   false,   nullopt, nullopt, nullopt, nullopt}, // CC_LT
            {true,    nullopt, true,    true,    false,   nullopt, nullopt, nullopt, nullopt, nullopt}, // CC_LE
            {false,   nullopt, false,   false,   true,    nullopt, nullopt, nullopt, nullopt, nullopt}, // CC_GT
            {true,    nullopt, false,   nullopt, true,    true,    nullopt, nullopt, nullopt, nullopt}, // CC_GE
            {false,   nullopt, nullopt, nullopt, nullopt, nullopt, true,    nullopt, false,   false},   // CC_B
            {true,    nullopt, nullopt, nullopt, nullopt, nullopt, true,    true,    false,   nullopt}, // CC_BE
            {false,   nullopt, nullopt, nullopt, nullopt, nullopt, false,   false,   true,    nullopt}, // CC_A
            {true,    nullopt, nullopt, nullopt, nullopt, nullopt, false,   nullopt, true,    true},    // CC_AE
        }};
    // clang-format on

    auto dominantCc = dominantCondition->CastToCompare()->GetCc();
    // Swap the dominant condition code, if inputs are reversed: 'if (a < b)' -> 'if (b > a)'
    if (condition->GetInput(0) != dominantCondition->GetInput(0)) {
        ASSERT(condition->GetInput(0) == dominantCondition->GetInput(1));
        dominantCc = SwapOperandsConditionCode(dominantCc);
    }

    // Reverse the `dominant_cc` if the `condition` is reached after branching the false succesor of the
    // if_imm's basic block
    auto conditionBb = condition->GetBasicBlock();
    if (ifImm->GetEdgeIfInputFalse()->IsDominate(conditionBb)) {
        dominantCc = GetInverseConditionCode(dominantCc);
    } else {
        ASSERT(ifImm->GetEdgeIfInputTrue()->IsDominate(conditionBb));
    }
    // After these transformations dominant condition with current `dominant_cc` is equal to `true`
    // So `condition` result is resolved via table
    return COND_RELATION[condition->CastToCompare()->GetCc()][dominantCc];
}
}  // namespace ark::compiler
