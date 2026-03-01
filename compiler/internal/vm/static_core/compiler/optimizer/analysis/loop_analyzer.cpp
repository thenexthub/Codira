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

#include "optimizer/ir/basicblock.h"
#include "loop_analyzer.h"
#include "optimizer/ir/graph.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"

namespace ark::compiler {
bool LoopAnalyzer::RunImpl()
{
    GetGraph()->RunPass<DominatorsTree>();
    ResetLoopInfo();
    CreateRootLoop();
    CollectBackEdges();
    PopulateLoops();
    for (auto loop : GetGraph()->GetRootLoop()->GetInnerLoops()) {
        FindAndInsertPreHeaders(loop);
    }
    SetLoopProperties(GetGraph()->GetRootLoop(), 0);
    return true;
}

void LoopAnalyzer::ResetLoopInfo()
{
    for (auto block : GetGraph()->GetVectorBlocks()) {
        if (block != nullptr) {
            block->SetLoop(nullptr);
            block->SetNextLoop(nullptr);
        }
    }
    GetGraph()->SetRootLoop(nullptr);
    GetGraph()->SetHasIrreducibleLoop(false);
    GetGraph()->SetHasInfiniteLoop(false);
    loopCounter_ = 0;
}

Loop *LoopAnalyzer::CreateNewLoop(BasicBlock *loopHeader)
{
    auto loop = GetGraph()->GetAllocator()->New<Loop>(GetGraph()->GetAllocator(), loopHeader, loopCounter_++);
    ASSERT(loop != nullptr);
    loop->AppendBlock(loopHeader);
    return loop;
}

void LoopAnalyzer::CreateRootLoop()
{
    ASSERT(GetGraph()->GetRootLoop() == nullptr);
    auto rootLoop = GetGraph()->GetAllocator()->New<Loop>(GetGraph()->GetAllocator(), nullptr, loopCounter_++);
    ASSERT(rootLoop != nullptr);
    rootLoop->SetAsRoot();
    GetGraph()->SetRootLoop(rootLoop);
}

void LoopAnalyzer::CollectBackEdges()
{
    blackMarker_ = GetGraph()->NewMarker();
    grayMarker_ = GetGraph()->NewMarker();
    BackEdgeSearch(GetGraph()->GetStartBlock());
    GetGraph()->EraseMarker(blackMarker_);
    GetGraph()->EraseMarker(grayMarker_);
}

/*
 * Depth-first search to find back edges in the graph.
 * When a block is visited for the first time it is marked with a gray marker,
 * after visiting all his successors, a block is marked with a black marker.
 * While doing DFS, if we encounter a block with a gray mark, then edge to this block is back edge.
 */
void LoopAnalyzer::BackEdgeSearch(BasicBlock *block)
{
    block->SetMarker(grayMarker_);
    block->SetMarker(blackMarker_);
    for (auto succ : block->GetSuccsBlocks()) {
        if (succ->IsMarked(grayMarker_)) {
            ProcessNewBackEdge(succ, block);
        } else if (!succ->IsMarked(blackMarker_)) {
            BackEdgeSearch(succ);
        }
    }
    block->ResetMarker(grayMarker_);
}

/*
 * Create new Loop if it doesn't exists.
 * Append information about its header, back edge and check if this loop is irreducible.
 * Loop is irreducible when its header doesn't dominate back edge.
 */
void LoopAnalyzer::ProcessNewBackEdge(BasicBlock *header, BasicBlock *backEdge)
{
    auto loop = header->GetLoop();
    if (loop == nullptr) {
        loop = CreateNewLoop(header);
    }

    loop->AppendBackEdge(backEdge);
    if (!header->IsDominate(backEdge)) {
        loop->SetIsIrreducible(true);
        GetGraph()->SetHasIrreducibleLoop(true);
    }
}

/*
 * Get vector of forward edges indexes in descending order
 */
ArenaVector<int> LoopAnalyzer::GetForwardEdgesIndexes(BasicBlock *header)
{
    // Mark back-edges
    auto markerHolder = compiler::MarkerHolder(GetGraph());
    auto backEdgeMarker = markerHolder.GetMarker();
    auto &backEdges = header->GetLoop()->GetBackEdges();
    for (auto backEdge : backEdges) {
        backEdge->SetMarker(backEdgeMarker);
    }

    ArenaVector<int> indexes(header->GetGraph()->GetAllocator()->Adapter());
    auto &predBlocks = header->GetPredsBlocks();
    for (int idx = static_cast<int>(predBlocks.size()) - 1; idx >= 0; idx--) {
        if (!predBlocks[idx]->IsMarked(backEdgeMarker)) {
            indexes.push_back(idx);
        }
    }
    ASSERT(indexes.size() + backEdges.size() == predBlocks.size());
    return indexes;
}

void LoopAnalyzer::MovePhiInputsToPreHeader(BasicBlock *header, BasicBlock *preHeader,
                                            const ArenaVector<int> &fwEdgesIndexes)
{
    for (auto phi : header->PhiInsts()) {
        auto newPhi = GetGraph()->CreateInstPhi(phi->GetType(), phi->GetPc());
        for (auto idx : fwEdgesIndexes) {
            auto pred {header->GetPredBlockByIndex(idx)};
            auto phiIdx {phi->CastToPhi()->GetPredBlockIndex(pred)};
            newPhi->AppendInput(phi->GetInput(phiIdx).GetInst());
            phi->RemoveInput(phiIdx);
        }
        preHeader->AppendPhi(newPhi);
        phi->AppendInput(newPhi);
    }
}

void LoopAnalyzer::UpdateControlFlowWithPreHeader(BasicBlock *header, BasicBlock *preHeader,
                                                  const ArenaVector<int> &fwEdgesIndexes)
{
    constexpr size_t IMM_2 = 2;
    if (fwEdgesIndexes.size() >= IMM_2) {
        for (auto predIdx : fwEdgesIndexes) {
            auto edge = header->GetPredBlockByIndex(predIdx);
            edge->ReplaceSucc(header, preHeader);
            header->RemovePred(edge);
        }
        preHeader->AddSucc(header);
    } else {
        ASSERT(fwEdgesIndexes.size() == 1);
        auto edge = header->GetPredBlockByIndex(fwEdgesIndexes[0]);
        edge->ReplaceSucc(header, preHeader);
        header->ReplacePred(edge, preHeader);
    }
    // Update RPO
    GetGraph()->GetAnalysis<Rpo>().SetValid(true);
    GetGraph()->GetAnalysis<Rpo>().AddBasicBlockBefore(header, preHeader);
}

/*
 * Create block with the same amount of phi instructions as in a `header` and insert it before a `header`.
 * Move relevant to forward edges phi inputs to pre-header.
 */
BasicBlock *LoopAnalyzer::CreatePreHeader(BasicBlock *header)
{
    auto fwEdgesIndexes = GetForwardEdgesIndexes(header);
    auto preHeader = header->CreateImmediateDominator();
    ASSERT(preHeader != nullptr);
    preHeader->SetGuestPc(header->GetGuestPc());
    if (fwEdgesIndexes.size() >= 2U) {
        MovePhiInputsToPreHeader(header, preHeader, fwEdgesIndexes);
    }
    UpdateControlFlowWithPreHeader(header, preHeader, fwEdgesIndexes);
    return preHeader;
}

bool LoopAnalyzer::PreHeaderExists(Loop *loop)
{
    auto header = loop->GetHeader();
    return header->GetPredsBlocks().size() - loop->GetBackEdges().size() == 1 &&
           header->GetDominator()->GetLoop() == loop->GetOuterLoop() &&
           header->GetDominator() != GetGraph()->GetStartBlock() && header->GetDominator()->GetNextLoop() == nullptr;
}

/*
 * Find all loop pre-headers. If loop doesn't have pre-header, insert it
 */
void LoopAnalyzer::FindAndInsertPreHeaders(Loop *loop)
{
    ASSERT(loop != nullptr && loop->GetHeader() != nullptr);
    auto header = loop->GetHeader();

    if (loop->IsTryCatchLoop()) {
        loop->SetPreHeader(nullptr);
    } else if (!loop->IsIrreducible()) {
        BasicBlock *preHeader = nullptr;
        if (PreHeaderExists(loop)) {
            preHeader = header->GetDominator();
        } else {
            preHeader = CreatePreHeader(header);
            preHeader->CopyTryCatchProps(header);
            loop->GetOuterLoop()->AppendBlock(preHeader);
        }
        ASSERT(preHeader->GetNextLoop() == nullptr);  // a preheader shouldn't relate to several loops.
        loop->SetPreHeader(preHeader);
    }

    for (auto innerLoop : loop->GetInnerLoops()) {
        FindAndInsertPreHeaders(innerLoop);
    }
}

BasicBlock *Loop::GetPreHeader() const
{
    ASSERT(!preHeader_ || preHeader_->GetNextLoop() == this);
    return preHeader_;
}

void Loop::SetPreHeader(BasicBlock *preHeader)
{
    if (preHeader_ != nullptr && preHeader_->GetNextLoop() == this) {
        preHeader_->SetNextLoop(nullptr);
    }
    preHeader_ = preHeader;
    preHeader_->SetNextLoop(this);
}
void Loop::SetPreHeader(std::nullptr_t preHeader)
{
    if (preHeader_ != nullptr && preHeader_->GetNextLoop() == this) {
        preHeader_->SetNextLoop(nullptr);
    }
    preHeader_ = preHeader;
}

void LoopAnalyzer::PopulateIrreducibleLoop(Loop *loop)
{
    // Add back-edges to the loop for further analysis
    // Note that other blocks of `loop` besides the header are not added to it,
    // and outer loop of inner loops will be not `loop`, but its outer reducible (maybe root) loop
    for (auto backEdge : loop->GetBackEdges()) {
        if (backEdge->GetLoop() != loop) {
            loop->AppendBlock(backEdge);
        }
    }
}

/*
 * Visiting existing loop headers to populate loops with blocks
 * Search algorithm starts from the loop back edge and recursively adds all predecessors until loop header not found
 */
void LoopAnalyzer::PopulateLoops()
{
    for (auto it = GetGraph()->GetBlocksRPO().rbegin(); it != GetGraph()->GetBlocksRPO().rend(); it++) {
        auto block = *it;
        if (block->GetLoop() == nullptr || !block->IsLoopHeader()) {
            continue;
        }
        auto loop = block->GetLoop();
        if (loop->IsIrreducible()) {
            PopulateIrreducibleLoop(loop);
        } else {
            blackMarker_ = GetGraph()->NewMarker();
            block->SetMarker(blackMarker_);
            for (auto backEdge : loop->GetBackEdges()) {
                NaturalLoopSearch(loop, backEdge);
            }
            GetGraph()->EraseMarker(blackMarker_);
        }
    }

    // Populate the root loop with blocks which are not assign to any loops
    // Link all outer loops with the root loop
    auto rootLoop = GetGraph()->GetRootLoop();
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (block->GetLoop() == nullptr) {
            rootLoop->AppendBlock(block);
        } else if (block->GetLoop()->GetOuterLoop() == nullptr) {
            block->GetLoop()->SetOuterLoop(rootLoop);
            rootLoop->AppendInnerLoop(block->GetLoop());
        }
    }
}

/*
 * Depth-first search to find blocks in the loop.
 * When a block is visited for the first time it is marked with a black marker, added to the loop
 * (if it hasn't been already added to the inner loop), and search runs for all its predecessors.
 * Header block is marked firstly to stop search on it.
 */
void LoopAnalyzer::NaturalLoopSearch(Loop *loop, BasicBlock *block)
{
    if (!block->IsMarked(blackMarker_)) {
        block->SetMarker(blackMarker_);

        if (block->GetLoop() == nullptr) {
            // `block` without assignment to any loop is found
            loop->AppendBlock(block);
        } else if (block->GetLoop()->GetHeader() != loop->GetHeader()) {
            // `block` from an inner loop id found, because its header differs from searching loop header
            if (block->GetLoop()->GetOuterLoop() == nullptr) {
                // Link outer loop and inner loop
                block->GetLoop()->SetOuterLoop(loop);
                loop->AppendInnerLoop(block->GetLoop());
            }
        }

        for (auto pred : block->GetPredsBlocks()) {
            NaturalLoopSearch(loop, pred);
        }
    }
}

void LoopAnalyzer::SetLoopProperties(Loop *loop, uint32_t depth)
{
    loop->CheckInfinity();
    loop->SetDepth(depth);
    if (loop->IsInfinite()) {
        GetGraph()->SetHasInfiniteLoop(true);
    }
    depth++;
    for (auto innerLoop : loop->GetInnerLoops()) {
        SetLoopProperties(innerLoop, depth);
    }
}

void Loop::AppendBlock(BasicBlock *block)
{
    ASSERT(std::find(blocks_.begin(), blocks_.end(), block) == blocks_.end());
    block->SetLoop(this);
    blocks_.push_back(block);
}

void Loop::RemoveBlock(BasicBlock *block)
{
    ASSERT(block != GetHeader());
    ASSERT(!HasBackEdge(block));
#ifndef NDEBUG
    for (auto innerLoop : GetInnerLoops()) {
        ASSERT(block != innerLoop->GetPreHeader());
    }
#endif

    auto blockIt = std::find(blocks_.begin(), blocks_.end(), block);
    ASSERT(blockIt != blocks_.end());
    blocks_.erase(blockIt);
}

bool Loop::IsOsrLoop() const
{
    return !IsRoot() && GetHeader()->IsOsrEntry();
}

bool Loop::IsTryCatchLoop() const
{
    return !IsRoot() && GetHeader()->IsCatchBegin();
}

/*
 * Check if this loop is inside other
 */
bool Loop::IsInside(Loop *other)
{
    auto outer = this->GetOuterLoop();
    while (outer != nullptr) {
        if (outer == other) {
            return true;
        }
        outer = outer->GetOuterLoop();
    }
    return false;
}

void Loop::MoveHeaderToSucc()
{
    ASSERT(header_->GetSuccsBlocks().size() == 1);
    header_ = header_->GetSuccessor(0);
    ASSERT(header_->GetLoop() == this);
    auto it = std::find(blocks_.begin(), blocks_.end(), header_);
    ASSERT(it != blocks_.end());
    std::swap(*it, *blocks_.begin());
}

void Loop::CheckInfinity()
{
    isInfinite_ = false;
    if (isRoot_) {
        return;
    }
    auto outerLoop = GetOuterLoop();
    for (auto block : GetBlocks()) {
        const auto &succs = block->GetSuccsBlocks();
        bool hasExit = std::find_if(succs.begin(), succs.end(), [&outerLoop](const BasicBlock *bb) {
                           return bb->GetLoop() == outerLoop;
                       }) != succs.end();
        if (hasExit) {
            return;
        }
    }
    isInfinite_ = true;
}

bool Loop::IsPostExitBlock(const BasicBlock *block) const
{
    for (auto loopBlock : GetBlocks()) {
        for (auto succ : loopBlock->GetSuccsBlocks()) {
            if (succ->GetLoop() != GetOuterLoop()) {
                continue;
            }

            if (succ == block) {
                return true;
            }
        }
    }

    return false;
}

/*
 * Find outside block for the loop with single back-edge exit
 */
BasicBlock *GetLoopOutsideSuccessor(Loop *loop)
{
    ASSERT(loop->GetBackEdges().size() == 1);
    auto backEdge = loop->GetBackEdges()[0];
    auto headerSuccIdx = backEdge->GetSuccBlockIndex(loop->GetHeader());
    ASSERT(backEdge->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
    auto outsideBlock = backEdge->GetSuccessor(1 - headerSuccIdx);
    ASSERT(outsideBlock != nullptr);
    return outsideBlock;
}

/**
 * Check if the loop block sequence meets the requirements:
 * - there is only one back-edge;
 * - there is only one exit-point - from the back-edge;
 */
bool IsLoopSingleBackEdgeExitPoint(Loop *loop)
{
    ASSERT(loop != nullptr);
    if (loop->IsIrreducible()) {
        return false;
    }
    if (loop->GetBackEdges().size() != 1) {
        return false;
    }
    auto backEdge = loop->GetBackEdges()[0];
    // Check there are no side-exits
    for (auto block : loop->GetBlocks()) {
        if (block == backEdge) {
            continue;
        }
        for (auto succ : block->GetSuccsBlocks()) {
            if (succ->GetLoop() != loop && !succ->GetLoop()->IsInside(loop)) {
                return false;
            }
        }
    }
    return true;
}
}  // namespace ark::compiler
