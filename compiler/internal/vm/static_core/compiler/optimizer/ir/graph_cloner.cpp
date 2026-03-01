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

#include "compiler_logger.h"
#include "inst.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_cloner.h"

namespace ark::compiler {
GraphCloner::GraphCloner(Graph *graph, ArenaAllocator *allocator, ArenaAllocator *localAllocator)
    : graph_(graph),
      allocator_(allocator),
      localAllocator_(localAllocator),
      cloneBlocks_(allocator->Adapter()),
      cloneInstructions_(allocator->Adapter()),
      cloneInstMap_(allocator->Adapter())
{
}

/// Clone the whole graph
Graph *GraphCloner::CloneGraph()
{
    auto newGraph = allocator_->New<Graph>(Graph::GraphArgs {allocator_, localAllocator_, GetGraph()->GetArch(),
                                                             GetGraph()->GetMethod(), GetGraph()->GetRuntime()},
                                           GetGraph()->GetParentGraph(), GetGraph()->GetMode());
    ASSERT(newGraph != nullptr);
    newGraph->SetCurrentInstructionId(GetGraph()->GetCurrentInstructionId());
    newGraph->SetAotData(GetGraph()->GetAotData());
    CloneBlocksAndInstructions<InstCloneType::CLONE_ALL, false>(GetGraph()->GetVectorBlocks(), newGraph);
    BuildControlFlow();
    BuildTryCatchLogic(newGraph);
    BuildDataFlow();
    CloneConstantsInfo(newGraph);
    newGraph->GetPassManager()->SetCheckMode(GetGraph()->GetPassManager()->IsCheckMode());
    // Clone all flags
    newGraph->SetBitFields(GetGraph()->GetBitFields());
    newGraph->InitUsedRegs<DataType::INT64>(GetGraph()->GetUsedRegs<DataType::INT64>());
    newGraph->InitUsedRegs<DataType::FLOAT64>(GetGraph()->GetUsedRegs<DataType::FLOAT64>());
#ifdef COMPILER_DEBUG_CHECKS
    CloneAnalyses(newGraph);
#endif
    return newGraph;
}

void GraphCloner::CloneAnalyses(Graph *newGraph)
{
    // Clone dominators if analysis is valid to check dom-tree
    ASSERT(!newGraph->IsAnalysisValid<DominatorsTree>());
    if (GetGraph()->IsAnalysisValid<DominatorsTree>()) {
        newGraph->GetAnalysis<DominatorsTree>().SetValid(true);
        for (auto block : GetGraph()->GetBlocksRPO()) {
            auto clone = GetClone(block);
            if (block->GetDominator() != nullptr) {
                auto cloneDom = GetClone(block->GetDominator());
                clone->SetDominator(cloneDom);
            }
            for (auto domBlocks : block->GetDominatedBlocks()) {
                clone->AddDominatedBlock(GetClone(domBlocks));
            }
        }
    }

    // Clone loops if analysis is valid to check loop-tree
    ASSERT(!newGraph->IsAnalysisValid<LoopAnalyzer>());
    if (GetGraph()->IsAnalysisValid<LoopAnalyzer>()) {
        auto &clonedLa = newGraph->GetAnalysis<LoopAnalyzer>();
        clonedLa.SetValid(true);
        clonedLa.CreateRootLoop();
        CopyLoop(GetGraph()->GetRootLoop(), newGraph->GetRootLoop());
        newGraph->SetHasIrreducibleLoop(GetGraph()->HasIrreducibleLoop());
        newGraph->SetHasInfiniteLoop(GetGraph()->HasInfiniteLoop());
    }

    ASSERT(!newGraph->IsAnalysisValid<LinearOrder>());
    if (GetGraph()->IsAnalysisValid<LinearOrder>()) {
        newGraph->GetAnalysis<LinearOrder>().SetValid(true);
        CloneLinearOrder(newGraph);
    }
}

void GraphCloner::CopyLoop(Loop *loop, Loop *clonedLoop)
{
    if (!loop->IsRoot() && !loop->IsIrreducible() && !loop->IsTryCatchLoop()) {
        ASSERT(GetClone(loop->GetHeader()) == clonedLoop->GetHeader());
        clonedLoop->SetPreHeader(GetClone(loop->GetPreHeader()));
    }
    for (auto block : loop->GetBlocks()) {
        if (block->IsLoopHeader()) {
            continue;
        }
        clonedLoop->AppendBlock(GetClone(block));
    }

    for (auto backEdge : loop->GetBackEdges()) {
        clonedLoop->AppendBackEdge(GetClone(backEdge));
    }
    clonedLoop->SetIsIrreducible(loop->IsIrreducible());
    clonedLoop->SetIsInfinite(loop->IsInfinite());

    // clone inner loops
    for (const auto &innerLoop : loop->GetInnerLoops()) {
        auto clonedHeader = GetClone(innerLoop->GetHeader());
        auto &clonedLa = clonedHeader->GetGraph()->GetAnalysis<LoopAnalyzer>();
        auto clonedInnerLoop = clonedLa.CreateNewLoop(clonedHeader);
        ASSERT(clonedInnerLoop != nullptr);
        clonedInnerLoop->SetOuterLoop(clonedLoop);
        clonedLoop->AppendInnerLoop(clonedInnerLoop);
        CopyLoop(innerLoop, clonedInnerLoop);
    }
}

void GraphCloner::CloneLinearOrder([[maybe_unused]] Graph *newGraph)
{
    ASSERT(newGraph != nullptr);
    ASSERT(GetGraph()->IsAnalysisValid<LinearOrder>());
    auto &cloneLinearBlocks = newGraph->GetAnalysis<LinearOrder>().GetBlocks();
    cloneLinearBlocks.reserve(GetGraph()->GetBlocksLinearOrder().size());
    for (auto block : GetGraph()->GetBlocksLinearOrder()) {
        cloneLinearBlocks.push_back(GetClone(block));
    }
}

/// Clone the whole graph control-flow
void GraphCloner::BuildControlFlow()
{
    for (const auto &block : GetGraph()->GetVectorBlocks()) {
        if (block == nullptr) {
            continue;
        }
        CloneEdges<CloneEdgeType::EDGE_PRED>(block);
        CloneEdges<CloneEdgeType::EDGE_SUCC>(block);
    }
}

void GraphCloner::CloneThrowableInstHandlers(Graph *newGraph)
{
    for (auto *block : GetGraph()->GetVectorBlocks()) {
        if (block == nullptr) {
            continue;
        }
        for (auto *inst : block->AllInsts()) {
            if (!GetGraph()->IsInstThrowable(inst)) {
                continue;
            }

            auto *clonedInst = GetClone(inst);
            for (auto *catchBlock : GetGraph()->GetThrowableInstHandlers(inst)) {
                auto *clonedCatchBlock = GetClone(catchBlock);
                newGraph->AppendThrowableInst(clonedInst, clonedCatchBlock);
            }
        }
    }
}

void GraphCloner::BuildTryCatchLogic(Graph *newGraph)
{
    // Connect every tryInst to corresponding tryEndBlock
    for (auto *tryBegin : newGraph->GetTryBeginBlocks()) {
        TryInst *tryInst = GetTryBeginInst(tryBegin);
        ASSERT(tryInst != nullptr);
        for (auto block : newGraph->GetBlocksRPO()) {
            if (block->IsTryEnd() && (block->GetTryId() == tryBegin->GetTryId())) {
                tryInst->SetTryEndBlock(block);
            }
        }
    }

    CloneThrowableInstHandlers(newGraph);

    auto fixThrowables = [this](const CatchPhiInst *originalInst, CatchPhiInst *clonedInst) {
        if (originalInst->GetThrowableInsts() != nullptr) {
            for (size_t i = 0, j = originalInst->GetThrowableInsts()->size(); i < j; ++i) {
                auto *clonedThrowable = cloneInstMap_[originalInst->GetThrowableInst(i)];
                clonedInst->AppendThrowableInst(clonedThrowable);
            }
        }
    };

    // Restore catchPhi's and throwables
    for (BasicBlock *clone : cloneBlocks_) {
        // Skip nullptrs and non-catch blocks
        if (clone == nullptr || !clone->IsCatch()) {
            continue;
        }
        Inst *inst = clone->GetFirstInst();
        while (inst != nullptr) {
            if (inst->IsCatchPhi() && !inst->CastToCatchPhi()->IsAcc()) {
                fixThrowables(cloneInstMap_[inst]->CastToCatchPhi(), inst->CastToCatchPhi());
            }
            inst = inst->GetNext();
        }
    }
}

/// Clone the whole graph data-flow
void GraphCloner::BuildDataFlow()
{
    for (const auto &block : GetGraph()->GetVectorBlocks()) {
        if (block == nullptr) {
            continue;
        }
        auto blockClone = GetClone(block);
        for (const auto &inst : block->Insts()) {
            SetCloneInputs<false>(inst);
            GetClone(inst)->SetId(inst->GetId());
            UpdateCaller(inst);
        }
        for (const auto &inst : block->PhiInsts()) {
            auto phi = inst->CastToPhi();
            auto instClone = GetClone(inst);
            instClone->SetId(inst->GetId());
            for (const auto &clonePredBlock : blockClone->GetPredsBlocks()) {
                auto it = std::find(cloneBlocks_.begin(), cloneBlocks_.end(), clonePredBlock);
                ASSERT(it != cloneBlocks_.end());
                size_t index = std::distance(cloneBlocks_.begin(), it);
                ASSERT(GetGraph()->GetVectorBlocks().size() > index);
                auto predBlock = GetGraph()->GetVectorBlocks()[index];
                instClone->AppendInput(GetClone(phi->GetPhiInput(predBlock)));
            }
        }
    }
}

void GraphCloner::CloneConstantsInfo(Graph *newGraph)
{
    // Restore firstConstInst_ logic
    BasicBlock *startBB = newGraph->GetStartBlock();
    Inst *inst = startBB->GetFirstInst();
    // Find first const if any
    while (inst != nullptr) {
        if (inst->IsConst()) {
            newGraph->SetFirstConstInst(inst->CastToConstant());
            break;
        }
        inst = inst->GetNext();
    }
    if (inst == nullptr) {
        return;
    }
    // Put another constants in chain if any
    ConstantInst *constInst = inst->CastToConstant();
    ASSERT(constInst->GetBasicBlock() == startBB);
    while ((inst = inst->GetNext()) != nullptr) {
        if (inst->IsConst()) {
            constInst->SetNextConst(inst->CastToConstant());
            constInst = inst->CastToConstant();
        }
    }
}

void PopulateResolverBlock(Loop *loop, BasicBlock *resolver, Inst *inst)
{
    Inst *phiResolver = nullptr;
    auto userIt = inst->GetUsers().begin();
    while (userIt != inst->GetUsers().end()) {
        auto user = userIt->GetInst();
        auto inputIdx = userIt->GetIndex();
        ++userIt;
        ASSERT(user->GetBasicBlock() != nullptr);
        if (user->GetBasicBlock()->GetLoop() != loop) {
            if (phiResolver == nullptr) {
                phiResolver = inst->GetBasicBlock()->GetGraph()->CreateInstPhi(inst->GetType(), inst->GetPc());
                phiResolver->AppendInput(inst);
                resolver->AppendPhi(phiResolver);
            }
            user->SetInput(inputIdx, phiResolver);
        }
    }
}

/*
 * Create resolver-block - common successor for all loop side-exits
 */
BasicBlock *GraphCloner::CreateResolverBlock(Loop *loop, BasicBlock *backEdge)
{
    auto outsideSucc = GetLoopOutsideSuccessor(loop);
    ASSERT(backEdge != nullptr);
    auto resolver = backEdge->InsertNewBlockToSuccEdge(outsideSucc);
    backEdge->GetLoop()->GetOuterLoop()->AppendBlock(resolver);
    // Populate resolver-block with phis for each instruction which has outside-loop user
    for (auto block : loop->GetBlocks()) {
        for (auto inst : block->AllInsts()) {
            PopulateResolverBlock(loop, resolver, inst);
        }
    }
    return resolver;
}

Inst *GraphCloner::GetCompareInst(Inst *ifimm)
{
    auto compare = ifimm->GetInput(0).GetInst();
    ASSERT(compare->GetOpcode() == Opcode::Compare);
    // If there are intructions between `Compare` and `IfImm`, clone `Compare` and insert before `IfImm`
    if (ifimm->GetPrev() != compare || !compare->HasSingleUser()) {
        auto newCmp = compare->Clone(compare->GetBasicBlock()->GetGraph());
        newCmp->SetInput(0, compare->GetInput(0).GetInst());
        newCmp->SetInput(1, compare->GetInput(1).GetInst());
        ifimm->InsertBefore(newCmp);
        ifimm->SetInput(0, newCmp);
        compare = newCmp;
    }
    return compare;
}

/*
 * Split back-edge for cloning without side exits - in order not to clone `Compare` and `IfImm` instructions
 */
BasicBlock *GraphCloner::SplitBackEdge(LoopUnrollData *unrollData, Loop *loop, BasicBlock *backEdge)
{
    auto *ifimm = backEdge->GetLastInst();
    ASSERT(ifimm->GetOpcode() == Opcode::IfImm);
    auto *compare = GetCompareInst(ifimm);
    if (compare->GetPrev() != nullptr) {
        auto backEdgeSplit = backEdge->SplitBlockAfterInstruction(compare->GetPrev(), true);
        loop->ReplaceBackEdge(backEdge, backEdgeSplit);
        backEdge = backEdgeSplit;
    } else {
        ASSERT(backEdge->GetPredsBlocks().size() == 1);
        auto it = std::find(unrollData->blocks->begin(), unrollData->blocks->end(), backEdge);
        ASSERT(it != unrollData->blocks->end());
        unrollData->blocks->erase(it);
        unrollData->exitBlock = backEdge->GetPredBlockByIndex(0);
    }
    [[maybe_unused]] static constexpr auto BACK_EDGE_INST_COUNT = 2;
    ASSERT(std::distance(backEdge->AllInsts().begin(), backEdge->AllInsts().end()) == BACK_EDGE_INST_COUNT);
    return backEdge;
}

/**
 *   - Split loop-header into two blocks, header phis will not be cloned:
 *   [phi-insts]
 *   -----------
 *   [all-insts]
 *
 *  - If loop is cloing with side-exits create common successor for them;
 *  - Otherwise split back-edge to cut `Compare` and `IfImm` instructions and not clone them;
 */
GraphCloner::LoopUnrollData *GraphCloner::PrepareLoopToUnroll(Loop *loop, bool cloneSideExits)
{
    ASSERT(loop != nullptr);
    // Populate `LoopUnrollData`
    auto allocator = loop->GetHeader()->GetGraph()->GetLocalAllocator();
    auto unrollData = allocator->New<LoopUnrollData>();
    ASSERT(unrollData != nullptr);
    unrollData->blocks = allocator->New<ArenaVector<BasicBlock *>>(allocator->Adapter());
    ASSERT(unrollData->blocks != nullptr);
    unrollData->blocks->resize(loop->GetBlocks().size());
    std::copy(loop->GetBlocks().begin(), loop->GetBlocks().end(), unrollData->blocks->begin());
    // Split loop-header
    ASSERT(loop->GetBackEdges().size() == 1U);
    auto backEdge = loop->GetBackEdges()[0];
    ASSERT(backEdge != nullptr);
    auto headerBlock = loop->GetHeader();
    ASSERT(!headerBlock->IsEmpty());
    if (headerBlock->HasPhi()) {
        auto lastPhi = headerBlock->GetFirstInst()->GetPrev();
        ASSERT(lastPhi != nullptr && lastPhi->IsPhi());
        auto headerSplit = headerBlock->SplitBlockAfterInstruction(lastPhi, true);
        ASSERT(loop->GetBlocks().front() == headerBlock);
        unrollData->blocks->at(0) = headerSplit;

        if (backEdge == headerBlock) {
            loop->ReplaceBackEdge(headerBlock, headerSplit);
            backEdge = headerSplit;
        }
    }
    unrollData->exitBlock = backEdge;
    if (cloneSideExits) {
        unrollData->outer = CreateResolverBlock(loop, backEdge);
    } else {
        backEdge = SplitBackEdge(unrollData, loop, backEdge);
    }
    // Save replaceable phi inputs

    unrollData->phiUpdateInputs = allocator->New<InstVector>(allocator->Adapter());
    for (auto phi : headerBlock->PhiInsts()) {
        ASSERT(unrollData->phiUpdateInputs != nullptr);
        unrollData->phiUpdateInputs->push_back(phi->CastToPhi()->GetPhiInput(backEdge));
    }
    unrollData->header = headerBlock;
    unrollData->backedge = backEdge;
    return unrollData;
}

/// Update data-flow after unrolling without side-exits
void GraphCloner::UpdateUsersAfterNoSideExitsUnroll(const LoopUnrollData *unrollData)
{
    auto loop = unrollData->header->GetLoop();
    // Update outloop users: replace inputs located in the original loop by theirs clones
    auto compare = unrollData->backedge->GetLastInst()->GetPrev();
    ASSERT(compare->GetOpcode() == Opcode::Compare);
    for (size_t i = 0; i < compare->GetInputsCount(); i++) {
        auto input = compare->GetInput(i).GetInst();
        if (HasClone(input)) {
            compare->SetInput(i, GetClone(input));
        }
    }
    // update outloop users
    for (auto block : *unrollData->blocks) {
        for (auto inst : block->AllInsts()) {
            UpdateOutloopUsers(loop, inst);
        }
    }

    // All header-phi's outloop users are placed in the outer_bb after cloning the original loop
    // So it's enough to to iterate outer_bb's phis and repalce header-phi by its backedge input
    auto outerIdx = 1U - unrollData->backedge->GetSuccBlockIndex(unrollData->header);
    auto outerBb = unrollData->backedge->GetSuccessor(outerIdx);
    for (auto outerPhi : outerBb->PhiInsts()) {
        auto headerPhi = outerPhi->CastToPhi()->GetPhiInput(unrollData->backedge);
        if (headerPhi->IsPhi() && headerPhi->GetBasicBlock() == unrollData->header) {
            outerPhi->ReplaceInput(headerPhi, headerPhi->CastToPhi()->GetPhiInput(unrollData->backedge));
        }
    }
}

void GraphCloner::UpdateOutloopUsers(Loop *loop, Inst *inst)
{
    auto userIt = inst->GetUsers().begin();
    while (userIt != inst->GetUsers().end()) {
        auto user = userIt->GetInst();
        auto inputIdx = userIt->GetIndex();
        ++userIt;
        ASSERT(user->GetBasicBlock() != nullptr);
        if (user->GetBasicBlock()->GetLoop() != loop) {
            user->SetInput(inputIdx, GetClone(inst));
        }
    }
}

/**
 * Link cloned blocks with each other and insert them to the graph between the last-block and the output-block
 *
 * - No-side-exits case:
 *  /---->[header]
 *  |        |
 *  |        v
 *  |     [loop-body]   << last-block << exit-block
 *  |        |
 *  |        v
 *  \-----[backedge]----> ...
 *
 *  New control-flow:
 *  /---->[header]
 *  |        |
 *  |        v
 *  |     [loop-body]   << exit-block
 *  |        |
 *  |        v
 *  |     [loop-body-clone] << last-block
 *  |        |
 *  |        v
 *  \-----[backedge]----> ...
 *
 *
 *  Side-exits case:
 *  /---->[header]
 *  |        |
 *  |        v
 *  |     [loop-body]
 *  |        |
 *  |        v
 *  \-----[backedge]    << last-block << exit-block
 *           |
 *           v
 *        [outer]-----> ...
 *
 *  New control-flow:
 *  /---->[header]
 *  |         |
 *  |         v
 *  |     [loop-body]
 *  |         |
 *  |         v
 *  |     [backedge]------------\   << exit-block
 *  |         |                 |
 *  |         v                 |
 *  |    [loop-body-clone]      |
 *  |         |                 |
 *  |         v                 |
 *  \-----[backedge-clone]----->|       << last-block
 *                              |
 *                              v
 *                           [outer]-----> ...
 */
void GraphCloner::BuildLoopUnrollControlFlow(LoopUnrollData *unrollData)
{
    auto frontBlock = unrollData->blocks->front();
    auto loop = frontBlock->GetLoop();

    // Copy 'blocks' control-flow to the 'clones'
    for (auto block : *unrollData->blocks) {
        ASSERT(block->GetLoop() == loop);
        loop->AppendBlock(GetClone(block));

        if (block != frontBlock) {
            CloneEdges<CloneEdgeType::EDGE_PRED>(block);
        }
        if (block != unrollData->exitBlock) {
            CloneEdges<CloneEdgeType::EDGE_SUCC>(block);
        }
    }

    auto frontClone = GetClone(frontBlock);
    auto exitClone = GetClone(unrollData->exitBlock);
    if (unrollData->outer == nullptr) {
        ASSERT(unrollData->backedge->GetPredsBlocks().size() == 1);
        auto lastBlock = unrollData->backedge->GetPredsBlocks()[0];
        lastBlock->ReplaceSucc(unrollData->backedge, frontClone);
        unrollData->backedge->ReplacePred(lastBlock, exitClone);
    } else {
        ASSERT(!unrollData->outer->GetPredsBlocks().empty());
        auto lastBlock = unrollData->outer->GetPredsBlocks().back();
        lastBlock->ReplaceSucc(unrollData->header, frontClone);
        unrollData->header->ReplacePred(lastBlock, exitClone);

        exitClone->AddSucc(unrollData->outer);
        if (exitClone->GetSuccBlockIndex(unrollData->outer) != lastBlock->GetSuccBlockIndex(unrollData->outer)) {
            exitClone->SwapTrueFalseSuccessors();
        }
        auto newBackedge = GetClone(unrollData->exitBlock);
        loop->ReplaceBackEdge(unrollData->backedge, newBackedge);
        unrollData->backedge = newBackedge;
    }
}

/**
 * Construct dataflow for the cloned instructions
 * if input of the original instruction is front-block phi - insert replaceable input of this phi
 */
void GraphCloner::BuildLoopUnrollDataFlow(LoopUnrollData *unrollData)
{
    for (auto block : *unrollData->blocks) {
        for (auto inst : block->AllInsts()) {
            if (inst->IsMarked(cloneMarker_)) {
                SetCloneInputs<true>(inst, unrollData->backedge);
                UpdateCaller(inst);
            }
        }
    }

    // Append input to the phi-resolver from outer block, it holds all instructions which have outside loop users
    auto loop = unrollData->blocks->front()->GetLoop();
    if (unrollData->outer != nullptr) {
        for (auto phi : unrollData->outer->PhiInsts()) {
            auto inst = phi->GetInput(0).GetInst();
            if (IsInstLoopHeaderPhi(inst, loop)) {
                auto update = inst->CastToPhi()->GetPhiInput(unrollData->backedge);
                phi->AppendInput(update);
            } else {
                phi->AppendInput(GetClone(inst));
            }
        }
    }

    // NOTE (a.popov) use temp container after if would be possible to reset local allocator
    if (unrollData->phiReplacedInputs == nullptr) {
        unrollData->phiReplacedInputs = allocator_->New<PhiInputsMap>(allocator_->Adapter());
    } else {
        unrollData->phiReplacedInputs->clear();
    }

    // Set new update inputs for header phis
    size_t phiCount = 0;
    for (auto phi : loop->GetHeader()->PhiInsts()) {
        auto input = unrollData->phiUpdateInputs->at(phiCount);
        if (HasClone(input)) {
            input = GetClone(input);
        } else if (input->IsPhi() && input->GetBasicBlock()->GetLoop() == loop) {
            if (phi->IsDominate(input)) {
                input = input->CastToPhi()->GetPhiInput(unrollData->backedge);
            } else {
                // phi should be visited and its input should be added to the map
                ASSERT(unrollData->phiReplacedInputs->count(input) == 1);
                ASSERT(unrollData->phiReplacedInputs != nullptr);
                input = unrollData->phiReplacedInputs->at(input);
            }
        }

        auto phiUpdateInputIdx = phi->CastToPhi()->GetPredBlockIndex(unrollData->backedge);
        unrollData->phiReplacedInputs->emplace(phi, phi->GetInput(phiUpdateInputIdx).GetInst());
        phi->SetInput(phiUpdateInputIdx, input);
        phiCount++;
    }
}

void GraphCloner::RemoveLoopBackEdge(const LoopUnrollData *unrollData)
{
    auto lastBlock = unrollData->backedge;
    ASSERT(unrollData->outer == nullptr || lastBlock == unrollData->outer->GetPredsBlocks().back());

    // Erase control-flow instruction
    auto ifimm = lastBlock->GetLastInst();
    ASSERT(ifimm->GetOpcode() == Opcode::IfImm);
    lastBlock->RemoveInst(ifimm);
    // Remove back-edge
    auto header = unrollData->header;
    // Clear header block, it should only contain phi
    ASSERT(header->GetFirstInst() == nullptr);
    for (auto phi : header->PhiInstsSafe()) {
        auto remainingInst = phi->CastToPhi()->GetPhiInput(header->GetLoop()->GetPreHeader());
        phi->ReplaceUsers(remainingInst);
        header->RemoveInst(phi);
    }

    lastBlock->RemoveSucc(header);
    header->RemovePred(lastBlock);

    // Clear outer phis if it has single predecessor
    if (unrollData->outer != nullptr && unrollData->outer->GetPredsBlocks().size() == 1U) {
        for (auto phi : unrollData->outer->PhiInstsSafe()) {
            auto remainingInst = phi->GetInput(0).GetInst();
            phi->ReplaceUsers(remainingInst);
            unrollData->outer->RemoveInst(phi);
        }
    }
}

void GraphCloner::RemoveLoopPreHeader(const LoopUnrollData *unrollData)
{
    ASSERT(unrollData->header->GetPredsBlocks().size() == 1);
    // Loop::GetPreHeader may be invalid here
    auto preHeader = unrollData->header->GetPredBlockByIndex(0);
    auto ifimm = preHeader->GetLastInst();
    ASSERT(ifimm->GetOpcode() == Opcode::IfImm);
    preHeader->RemoveInst(ifimm);

    // Loop header should be removed from backedge successors before call to this function
    ASSERT(unrollData->backedge->GetSuccsBlocks().size() == 1);
    auto removedSucc = unrollData->backedge->GetSuccessor(0);

    for (auto phi : removedSucc->PhiInstsSafe()) {
        if (removedSucc->GetPredsBlocks().size() == 2U) {
            auto remainingInst = phi->CastToPhi()->GetPhiInput(unrollData->backedge);
            phi->ReplaceUsers(remainingInst);
            removedSucc->RemoveInst(phi);
        } else {
            phi->RemoveInput(phi->CastToPhi()->GetPredBlockIndex(preHeader));
        }
    }

    preHeader->RemoveSucc(removedSucc);
    removedSucc->RemovePred(preHeader);
}

void GraphCloner::BuildClonedLoopHeaderDataFlow(const BasicBlock &block, BasicBlock *resolver, BasicBlock *clone)
{
    for (auto inst : block.Insts()) {
        if (inst->IsMarked(cloneMarker_)) {
            SetCloneInputs<true>(inst, clone);
            UpdateUsersForClonedLoopHeader(inst, resolver);
            UpdateCaller(inst);
        }
    }
    for (auto phi : block.PhiInsts()) {
        ASSERT(phi->GetInputsCount() == 2U);
        auto preloopInput = phi->CastToPhi()->GetPhiInput(clone);
        // Create phi instruction in the `resolver` block with two inputs: current phi and phi's preloop_input
        auto resolverPhi = GetGraph()->CreateInstPhi(phi->GetType(), phi->GetPc());
        for (auto userIt = phi->GetUsers().begin(); userIt != phi->GetUsers().end();) {
            auto user = userIt->GetInst();
            auto inputIndex = userIt->GetIndex();
            ++userIt;
            ASSERT(user->GetBasicBlock() != nullptr);
            auto userLoop = user->GetBasicBlock()->GetLoop();
            if (userLoop != block.GetLoop() && !userLoop->IsInside(block.GetLoop())) {
                user->SetInput(inputIndex, resolverPhi);
            }
        }
        if (resolverPhi->HasUsers()) {
            resolverPhi->AppendInput(phi);
            resolverPhi->AppendInput(preloopInput);
            resolver->AppendPhi(resolverPhi);
        }
    }
}

/**
 *  Used by Loop peeling to clone loop-header and insert this clone before loop-header.
 *  Created block-resolved - common succesor for loop-header and its clone
 *
 *      [replaceable_pred]
 *              |
 *              v
 *   ... --->[block]--------\
 *              |           |
 *              v           v
 *             ...       [outer]
 *
 *
 *      [replaceable_pred]
 *              |
 *              v
 *        [clone_block]---------\
 *              |               |
 *              v               |
 *   ... --->[block]--------\   |
 *              |           |   |
 *              v           v   v
 *             ...        [resolver]
 *                            |
 *                            v
 *                          [outer]
 */
BasicBlock *GraphCloner::CloneLoopHeader(BasicBlock *block, BasicBlock *outer, BasicBlock *replaceablePred)
{
    ASSERT(GetGraph()->IsAnalysisValid<DominatorsTree>());
    ASSERT(cloneMarker_ == UNDEF_MARKER);
    auto markerHolder = MarkerHolder(GetGraph());
    cloneMarker_ = markerHolder.GetMarker();
    cloneInstructions_.clear();
    size_t instCount = 0;
    // Build control-flow
    auto resolver = block->InsertNewBlockToSuccEdge(outer);
    outer->GetLoop()->AppendBlock(resolver);
    if (outer->GetLoop()->HasBackEdge(block)) {
        outer->GetLoop()->ReplaceBackEdge(block, resolver);
    }
    auto cloneBlock = replaceablePred->InsertNewBlockToSuccEdge(block);
    ASSERT(block->GetLoop()->GetPreHeader() == replaceablePred);
    block->GetLoop()->SetPreHeader(cloneBlock);
    ASSERT(replaceablePred->GetNextLoop() == nullptr);
    replaceablePred->GetLoop()->AppendBlock(cloneBlock);
    cloneBlock->AddSucc(resolver);
    // Check the order of true-false successors
    if (cloneBlock->GetSuccBlockIndex(resolver) != block->GetSuccBlockIndex(resolver)) {
        cloneBlock->SwapTrueFalseSuccessors();
    }
    // Fix Dominators info
    auto &domTree = GetGraph()->GetAnalysis<DominatorsTree>();
    domTree.SetValid(true);
    ASSERT(block->GetDominator() == replaceablePred);
    replaceablePred->RemoveDominatedBlock(block);
    domTree.SetDomPair(cloneBlock, block);
    domTree.SetDomPair(replaceablePred, cloneBlock);
    domTree.SetDomPair(cloneBlock, resolver);
    if (outer->GetDominator() == block) {
        block->RemoveDominatedBlock(outer);
        domTree.SetDomPair(resolver, outer);
    }
    CloneInstructions<InstCloneType::CLONE_INSTS, true>(block, cloneBlock, &instCount);
    BuildClonedLoopHeaderDataFlow(*block, resolver, cloneBlock);
    cloneBlock->SetAllFields(block->GetAllFields());
    return cloneBlock;
}

/**
 * Use the following logic cloning the users:
 * - replace inputs of all users, placed OUTSIDE cloneable loop, by the new `phi_out` instruction
 * - `phi_out` is appended to the outer-block
 * - replace inputs of all users, placed INSIDE cloneable loop, but not cloned, by the new `phi_in` instruction
 * - `phi_in` is appended to the `inst` basic block
 * - `phi_in\phi_out` have `inst` and its clone as inputs
 */
void GraphCloner::UpdateUsersForClonedLoopHeader(Inst *inst, BasicBlock *outerBlock)
{
    if (!inst->HasUsers()) {
        return;
    }
    auto instBlock = inst->GetBasicBlock();
    auto clone = GetClone(inst);
    auto cloneBlock = clone->GetBasicBlock();
    ASSERT(cloneBlock != nullptr);
    // phi for inside users
    auto phiIn = GetGraph()->CreateInstPhi(inst->GetType(), inst->GetPc());
    // phi for outside users
    auto phiOut = GetGraph()->CreateInstPhi(inst->GetType(), inst->GetPc());
    auto userIt = inst->GetUsers().begin();
    while (userIt != inst->GetUsers().end()) {
        auto user = userIt->GetInst();
        auto inputIdx = userIt->GetIndex();
        ++userIt;
        ASSERT(user->GetBasicBlock() != nullptr);
        auto userLoop = user->GetBasicBlock()->GetLoop();
        if (userLoop == instBlock->GetLoop() || userLoop->IsInside(instBlock->GetLoop())) {
            // user inside loop
            // skip users that will be moved to the loop-exit block
            if (user->GetBasicBlock() == instBlock && !user->IsPhi()) {
                continue;
            }
            user->SetInput(inputIdx, phiIn);
        } else {
            // user outside loop
            user->SetInput(inputIdx, phiOut);
        }
    }

    if (phiIn->HasUsers()) {
        auto cloneIndex {instBlock->GetPredBlockIndex(cloneBlock)};
        phiIn->AppendInput(clone);
        phiIn->AppendInput(inst);
        phiIn->CastToPhi()->SetPhiInputBbNum(0, cloneIndex);
        phiIn->CastToPhi()->SetPhiInputBbNum(1, 1 - cloneIndex);

        auto firstPhi = instBlock->GetFirstPhi();
        if (firstPhi == nullptr) {
            instBlock->AppendPhi(phiIn);
        } else {
            instBlock->InsertBefore(phiIn, firstPhi);
        }
    }

    if (phiOut->HasUsers()) {
        phiOut->AppendInput(inst);
        phiOut->AppendInput(clone);
        outerBlock->AppendPhi(phiOut);
    }
}

bool GraphCloner::IsInstLoopHeaderPhi(Inst *inst, Loop *loop)
{
    return inst->IsPhi() && inst->GetBasicBlock() == loop->GetHeader();
}

/**
 * Create clone of loop and insert it after original loop:
 *
 *      /----[pre-loop]
 *      |        |
 *      |        v
 *      |    [loop-body]<----\
 *      |        |   |       |
 *      |        |   \-------/
 *      |        |
 *      |        v
 *      \--->[outside-block]
 *               |
 *               v
 *      /----[pre-loop']
 *      |        |
 *      |        v
 *      |    [loop-body']<----\
 *      |        |   |       |
 *      |        |   \-------/
 *      |        |
 *      |        v
 *      \--->[outside-block']
 */
Loop *GraphCloner::CloneLoop(Loop *loop)
{
    ASSERT(loop != nullptr && !loop->IsRoot());
    ASSERT_PRINT(IsLoopSingleBackEdgeExitPoint(loop), "Cloning blocks doesn't have single entry/exit point");
    ASSERT(loop->GetPreHeader() != nullptr);
    ASSERT(cloneMarker_ == UNDEF_MARKER);

    auto markerHolder = MarkerHolder(GetGraph());
    cloneMarker_ = markerHolder.GetMarker();
    SplitPreHeader(loop);
    auto unrollData = PrepareLoopToClone(loop);

    ASSERT(unrollData != nullptr);
    CloneBlocksAndInstructions<InstCloneType::CLONE_ALL, true>(*unrollData->blocks, GetGraph());
    BuildLoopCloneControlFlow(unrollData);
    BuildLoopCloneDataFlow(unrollData);
    MakeLoopCloneInfo(unrollData);
    GetGraph()->RunPass<DominatorsTree>();

    auto cloneLoop = GetClone(loop->GetHeader())->GetLoop();
    ASSERT(cloneLoop != loop && cloneLoop->GetOuterLoop() == loop->GetOuterLoop());
    COMPILER_LOG(DEBUG, GRAPH_CLONER) << "Loop " << loop->GetId() << " is copied";
    COMPILER_LOG(DEBUG, GRAPH_CLONER) << "Created new loop, id = " << cloneLoop->GetId();
    return cloneLoop;
}

BasicBlock *GraphCloner::CreateNewOutsideSucc(BasicBlock *outsideSucc, BasicBlock *backEdge, BasicBlock *preHeader)
{
    auto backEdgeIdx = outsideSucc->GetPredBlockIndex(backEdge);
    auto preHeaderIdx = outsideSucc->GetPredBlockIndex(preHeader);
    auto rmIdxMax = std::max(backEdgeIdx, preHeaderIdx);
    auto rmIdxMin = std::min(backEdgeIdx, preHeaderIdx);
    auto newOutsideSucc = GetGraph()->CreateEmptyBlock();
    outsideSucc->GetLoop()->AppendBlock(newOutsideSucc);
    backEdge->ReplaceSucc(outsideSucc, newOutsideSucc);
    preHeader->ReplaceSucc(outsideSucc, newOutsideSucc);
    newOutsideSucc->AddSucc(outsideSucc);
    for (auto phi : outsideSucc->PhiInsts()) {
        auto newPhi = GetGraph()->CreateInstPhi(phi->GetType(), phi->GetPc());
        newPhi->AppendInput(phi->CastToPhi()->GetPhiInput(backEdge));
        newPhi->AppendInput(phi->CastToPhi()->GetPhiInput(preHeader));
        phi->AppendInput(newPhi);
        auto phiBackEdgeIdx {phi->CastToPhi()->GetPredBlockIndex(backEdge)};
        auto phiPreHeaderIdx {phi->CastToPhi()->GetPredBlockIndex(preHeader)};
        phi->RemoveInput(rmIdxMax == backEdgeIdx ? phiBackEdgeIdx : phiPreHeaderIdx);
        phi->RemoveInput(rmIdxMin == preHeaderIdx ? phiPreHeaderIdx : phiBackEdgeIdx);
        newOutsideSucc->AppendPhi(newPhi);
    }
    outsideSucc->RemovePred(rmIdxMax);
    outsideSucc->RemovePred(rmIdxMin);

    COMPILER_LOG(DEBUG, GRAPH_CLONER) << "New loop outside block created: " << newOutsideSucc->GetId();
    return newOutsideSucc;
}

GraphCloner::LoopClonerData *GraphCloner::PopulateLoopClonerData(Loop *loop, BasicBlock *preHeader,
                                                                 BasicBlock *outsideSucc)
{
    auto allocator = GetGraph()->GetLocalAllocator();
    auto clonerData = allocator->New<LoopClonerData>();
    ASSERT(clonerData != nullptr);
    clonerData->blocks = allocator->New<ArenaVector<BasicBlock *>>(allocator->Adapter());
    ASSERT(clonerData->blocks != nullptr);
    clonerData->blocks->resize(loop->GetBlocks().size() + 1);
    clonerData->blocks->at(0) = preHeader;
    std::copy(loop->GetBlocks().begin(), loop->GetBlocks().end(), clonerData->blocks->begin() + 1);
    clonerData->blocks->push_back(outsideSucc);
    clonerData->outer = outsideSucc;
    clonerData->header = loop->GetHeader();
    clonerData->preHeader = loop->GetPreHeader();
    return clonerData;
}

// Split pre-header to contain `Compare` and `IfImm` instructions only;
void GraphCloner::SplitPreHeader(Loop *loop)
{
    ASSERT(loop != nullptr);
    auto preHeader = loop->GetPreHeader();
    auto *ifimm = preHeader->GetLastInst();
    ASSERT(ifimm != nullptr && ifimm->GetOpcode() == Opcode::IfImm);
    auto *compare = GetCompareInst(ifimm);
    if (compare->GetPrev() != nullptr) {
        auto newPreHeader = preHeader->SplitBlockAfterInstruction(compare->GetPrev(), true);
        loop->SetPreHeader(newPreHeader);
        // NOLINTNEXTLINE(clang-analyzer-deadcode.DeadStores)
        preHeader = newPreHeader;
    }
    [[maybe_unused]] static constexpr auto PRE_HEADER_INST_COUNT = 2;
    ASSERT(std::distance(preHeader->AllInsts().begin(), preHeader->AllInsts().end()) == PRE_HEADER_INST_COUNT);
}

/**
 * - Make sure `outsideSucc` has 2 predecessors only: loop header and back-edge;
 * - Split `outsideSucc` to contain phi-instructions only;
 */
GraphCloner::LoopClonerData *GraphCloner::PrepareLoopToClone(Loop *loop)
{
    // If `outsideSucc` has more than 2 predecessors, create a new one
    // with loop header and back-edge predecessors only and insert it before `outsideSucc`
    auto preHeader = loop->GetPreHeader();
    auto outsideSucc = GetLoopOutsideSuccessor(loop);
    constexpr auto PREDS_NUM = 2;
    if (outsideSucc->GetPredsBlocks().size() > PREDS_NUM) {
        auto backEdge = loop->GetBackEdges()[0];
        outsideSucc = CreateNewOutsideSucc(outsideSucc, backEdge, preHeader);
    }
    // Split outside succ after last phi
    // create empty block before outside succ if outside succ don't contain phi insts
    ASSERT(outsideSucc != nullptr);
    if (outsideSucc->HasPhi() && outsideSucc->GetFirstInst() != nullptr) {
        auto lastPhi = outsideSucc->GetFirstInst()->GetPrev();
        auto block = outsideSucc->SplitBlockAfterInstruction(lastPhi, true);
        // if `outsideSucc` is pre-header replace it by `block`
        for (auto inLoop : loop->GetOuterLoop()->GetInnerLoops()) {
            if (inLoop->GetPreHeader() == outsideSucc) {
                inLoop->SetPreHeader(block);
            }
        }
    } else if (outsideSucc->GetFirstInst() != nullptr) {
        auto block = outsideSucc->InsertEmptyBlockBefore();
        outsideSucc->GetLoop()->AppendBlock(block);
        outsideSucc = block;
    }
    // Populate `LoopClonerData`
    return PopulateLoopClonerData(loop, preHeader, outsideSucc);
}

/// Create new loop, populate it with cloned blocks and build conrlow-flow
void GraphCloner::BuildLoopCloneControlFlow(LoopClonerData *unrollData)
{
    ASSERT(unrollData != nullptr);
    auto outerClone = GetClone(unrollData->outer);
    auto preHeaderClone = GetClone(unrollData->preHeader);

    while (!unrollData->outer->GetSuccsBlocks().empty()) {
        auto succ = unrollData->outer->GetSuccsBlocks().front();
        succ->ReplacePred(unrollData->outer, outerClone);
        unrollData->outer->RemoveSucc(succ);
    }
    unrollData->outer->AddSucc(preHeaderClone);

    for (auto &block : *unrollData->blocks) {
        if (block != unrollData->preHeader) {
            CloneEdges<CloneEdgeType::EDGE_PRED>(block);
        }
        if (block != unrollData->outer) {
            CloneEdges<CloneEdgeType::EDGE_SUCC>(block);
        }
    }
    ASSERT(unrollData->outer->GetPredBlockIndex(unrollData->preHeader) ==
           outerClone->GetPredBlockIndex(preHeaderClone));
    ASSERT(unrollData->header->GetPredBlockIndex(unrollData->preHeader) ==
           GetClone(unrollData->header)->GetPredBlockIndex(preHeaderClone));
}

/// Insert cloned loop into loop-tree and populated with cloned blocks
void GraphCloner::MakeLoopCloneInfo(LoopClonerData *unrollData)
{
    ASSERT(unrollData != nullptr);
    // Update loop tree
    auto loop = unrollData->header->GetLoop();
    auto headerClone = GetClone(loop->GetHeader());
    auto cloneLoop = GetGraph()->GetAnalysis<LoopAnalyzer>().CreateNewLoop(headerClone);
    auto outerLoop = loop->GetOuterLoop();
    outerLoop->AppendInnerLoop(cloneLoop);
    ASSERT(cloneLoop != nullptr);
    cloneLoop->SetOuterLoop(outerLoop);

    // Populate cloned loop
    auto preLoopClone = GetClone(unrollData->preHeader);
    auto outsideSuccClone = GetClone(unrollData->outer);
    cloneLoop->SetPreHeader(preLoopClone);
    outerLoop->AppendBlock(preLoopClone);
    outerLoop->AppendBlock(outsideSuccClone);
    for (auto &block : loop->GetBlocks()) {
        if (!block->IsLoopHeader()) {
            cloneLoop->AppendBlock(GetClone(block));
        }
    }
    for (auto backEdge : loop->GetBackEdges()) {
        cloneLoop->AppendBackEdge(GetClone(backEdge));
    }
}

/// Find or create phi in the outside_succ block with the same inputs as `check_phi`
Inst *GetPhiResolver(Inst *checkPhi, BasicBlock *outsideSucc, BasicBlock *preHeader)
{
    [[maybe_unused]] constexpr auto MAX_PREDS_NUM = 2;
    ASSERT(outsideSucc->GetPredsBlocks().size() == MAX_PREDS_NUM);
    ASSERT(checkPhi->GetBasicBlock()->IsLoopHeader());
    auto initIdx = checkPhi->CastToPhi()->GetPredBlockIndex(preHeader);
    auto initInput = checkPhi->GetInput(initIdx).GetInst();
    auto updateInput = checkPhi->GetInput(1 - initIdx).GetInst();

    for (auto phi : outsideSucc->PhiInsts()) {
        auto idx {phi->CastToPhi()->GetPredBlockIndex(preHeader)};
        if (phi->GetInput(idx).GetInst() == initInput && phi->GetInput(1 - idx).GetInst() == updateInput) {
            return phi;
        }
    }

    auto phiResolver = outsideSucc->GetGraph()->CreateInstPhi(checkPhi->GetType(), checkPhi->GetPc());
    auto outInitIdx = outsideSucc->GetPredBlockIndex(preHeader);
    phiResolver->AppendInput(initInput);
    phiResolver->AppendInput(updateInput);
    phiResolver->SetPhiInputBbNum(0, outInitIdx);
    phiResolver->SetPhiInputBbNum(1, 1 - outInitIdx);

    outsideSucc->AppendPhi(phiResolver);
    return phiResolver;
}

void GraphCloner::ProcessMarkedInsts(LoopClonerData *data)
{
    for (const auto &block : *data->blocks) {
        for (const auto &inst : block->AllInsts()) {
            if (inst->GetOpcode() == Opcode::NOP) {
                continue;
            }
            if (inst->IsMarked(cloneMarker_)) {
                SetCloneInputs<false>(inst);
                UpdateCaller(inst);
            }
        }
    }
}

/// Build data-flow for cloned instructions
void GraphCloner::BuildLoopCloneDataFlow(LoopClonerData *unrollData)
{
    ASSERT(unrollData != nullptr);
    ProcessMarkedInsts(unrollData);

    auto preLoopClone = GetClone(unrollData->preHeader);
    for (auto phi : unrollData->outer->PhiInsts()) {
        auto phiClone = GetClone(phi);
        phi->ReplaceUsers(phiClone);
        auto idx = phiClone->CastToPhi()->GetPredBlockIndex(preLoopClone);
        phiClone->SetInput(idx, phi);
    }

    auto compare = preLoopClone->GetFirstInst();
    auto exitBlock = unrollData->outer->GetPredBlockByIndex(0);
    if (exitBlock == unrollData->preHeader) {
        exitBlock = unrollData->outer->GetPredBlockByIndex(1);
    } else {
        ASSERT(unrollData->outer->GetPredBlockByIndex(1) == unrollData->preHeader);
    }
    auto backEdgeCompare = exitBlock->GetLastInst()->GetInput(0).GetInst();
    ASSERT(compare->GetOpcode() == Opcode::Compare);
    ASSERT(backEdgeCompare->GetOpcode() == Opcode::Compare);
    for (auto phi : unrollData->header->PhiInsts()) {
        auto initIdx = phi->CastToPhi()->GetPredBlockIndex(unrollData->preHeader);
        ASSERT(GetClone(phi)->CastToPhi()->GetPredBlockIndex(preLoopClone) == initIdx);

        auto init = phi->GetInput(initIdx).GetInst();
        auto update = phi->GetInput(1 - initIdx).GetInst();
        auto resolverPhi = GetPhiResolver(phi, unrollData->outer, unrollData->preHeader);
        auto clonePhi = GetClone(phi);
        ASSERT(clonePhi->GetInput(initIdx).GetInst() == init);
        clonePhi->SetInput(initIdx, resolverPhi);
        for (size_t i = 0; i < compare->GetInputsCount(); i++) {
            if (compare->GetInput(i).GetInst() == init && backEdgeCompare->GetInput(i).GetInst() == update) {
                compare->SetInput(i, resolverPhi);
                break;
            }
        }
    }
}

void GraphCloner::UpdateCaller(Inst *inst)
{
    if (inst->IsSaveState()) {
        auto caller = static_cast<SaveStateInst *>(inst)->GetCallerInst();
        if (caller != nullptr && caller->IsInlined() && HasClone(caller)) {
            auto ssClone = GetClone(inst);
            auto callerClone = GetClone(caller);
            static_cast<SaveStateInst *>(ssClone)->SetCallerInst(static_cast<CallInst *>(callerClone));
        }
    }
}
}  // namespace ark::compiler
