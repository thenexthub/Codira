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
#include "optimizer/ir/graph.h"
#include "optimizer/analysis/rpo.h"
#include "loop_analyzer.h"

namespace panda::compiler {
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
    SearchInfiniteLoops(GetGraph()->GetRootLoop());
    return true;
}

void LoopAnalyzer::ResetLoopInfo()
{
    for (auto block : GetGraph()->GetVectorBlocks()) {
        if (block != nullptr) {
            block->SetLoop(nullptr);
        }
    }
    GetGraph()->SetRootLoop(nullptr);
    GetGraph()->SetHasIrreducibleLoop(false);
    GetGraph()->SetHasInfiniteLoop(false);
    loop_counter_ = 0;
}

Loop *LoopAnalyzer::CreateNewLoop(BasicBlock *loop_header)
{
    auto loop = GetGraph()->GetAllocator()->New<Loop>(GetGraph()->GetAllocator(), loop_header, loop_counter_++);
    CHECK_NOT_NULL(loop);
    loop->AppendBlock(loop_header);
    return loop;
}

void LoopAnalyzer::CreateRootLoop()
{
    ASSERT(GetGraph()->GetRootLoop() == nullptr);
    auto root_loop = GetGraph()->GetAllocator()->New<Loop>(GetGraph()->GetAllocator(), nullptr, loop_counter_++);
    CHECK_NOT_NULL(root_loop);
    root_loop->SetAsRoot();
    GetGraph()->SetRootLoop(root_loop);
}

void LoopAnalyzer::CollectBackEdges()
{
    black_marker_ = GetGraph()->NewMarker();
    gray_marker_ = GetGraph()->NewMarker();
    BackEdgeSearch(GetGraph()->GetStartBlock());
    GetGraph()->EraseMarker(black_marker_);
    GetGraph()->EraseMarker(gray_marker_);
}

/*
 * Depth-first search to find back edges in the graph.
 * When a block is visited for the first time it is marked with a gray marker,
 * after visiting all his successors, a block is marked with a black marker.
 * While doing DFS, if we encounter a block with a gray mark, then edge to this block is back edge.
 */
void LoopAnalyzer::BackEdgeSearch(BasicBlock *block)
{
    block->SetMarker(gray_marker_);
    block->SetMarker(black_marker_);
    for (auto succ : block->GetSuccsBlocks()) {
        if (succ->IsMarked(gray_marker_)) {
            ProcessNewBackEdge(succ, block);
        } else if (!succ->IsMarked(black_marker_)) {
            BackEdgeSearch(succ);
        }
    }
    block->ResetMarker(gray_marker_);
}

/*
 * Create new Loop if it doesn't exists.
 * Append information about its header, back edge and check if this loop is irreducible.
 * Loop is irreducible when its header doesn't dominate back edge.
 */
void LoopAnalyzer::ProcessNewBackEdge(BasicBlock *header, BasicBlock *back_edge)
{
    auto loop = header->GetLoop();
    if (loop == nullptr) {
        loop = CreateNewLoop(header);
    }

    loop->AppendBackEdge(back_edge);
    if (!header->IsDominate(back_edge)) {
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
    auto marker_holder = compiler::MarkerHolder(GetGraph());
    auto back_edge_marker = marker_holder.GetMarker();
    auto &back_edges = header->GetLoop()->GetBackEdges();
    for (auto back_edge : back_edges) {
        back_edge->SetMarker(back_edge_marker);
    }

    ArenaVector<int> indexes(header->GetGraph()->GetAllocator()->Adapter());
    auto &pred_blocks = header->GetPredsBlocks();
    for (int idx = static_cast<int>(pred_blocks.size()) - 1; idx >= 0; idx--) {
        if (!pred_blocks[idx]->IsMarked(back_edge_marker)) {
            indexes.push_back(idx);
        }
    }
    ASSERT(indexes.size() + back_edges.size() == pred_blocks.size());
    return indexes;
}

void LoopAnalyzer::MovePhiInputsToPreHeader(BasicBlock *header, BasicBlock *pre_header,
                                            const ArenaVector<int> &fw_edges_indexes)
{
    for (auto phi : header->PhiInsts()) {
        auto new_phi = GetGraph()->CreateInstPhi(phi->GetType(), phi->GetPc());
        for (auto idx : fw_edges_indexes) {
            auto pred {header->GetPredBlockByIndex(idx)};
            auto phi_idx {phi->CastToPhi()->GetPredBlockIndex(pred)};
            new_phi->AppendInput(phi->GetInput(phi_idx).GetInst());
            phi->RemoveInput(phi_idx);
        }
        pre_header->AppendPhi(new_phi);
        phi->AppendInput(new_phi);
    }
}

void LoopAnalyzer::UpdateControlFlowWithPreHeader(BasicBlock *header, BasicBlock *pre_header,
                                                  const ArenaVector<int> &fw_edges_indexes)
{
    constexpr size_t IMM_2 = 2;
    if (fw_edges_indexes.size() >= IMM_2) {
        for (auto pred_idx : fw_edges_indexes) {
            auto edge = header->GetPredBlockByIndex(pred_idx);
            edge->ReplaceSucc(header, pre_header);
            header->RemovePred(edge);
        }
        pre_header->AddSucc(header);
    } else {
        ASSERT(fw_edges_indexes.size() == 1);
        auto edge = header->GetPredBlockByIndex(fw_edges_indexes[0]);
        edge->ReplaceSucc(header, pre_header);
        header->ReplacePred(edge, pre_header);
    }
    // Update RPO
    GetGraph()->GetAnalysis<Rpo>().SetValid(true);
    GetGraph()->GetAnalysis<Rpo>().AddBasicBlockBefore(header, pre_header);
}

/*
 * Create block with the same amount of phi instructions as in a `header` and insert it before a `header`.
 * Move relevant to forward edges phi inputs to pre-header.
 */
BasicBlock *LoopAnalyzer::CreatePreHeader(BasicBlock *header)
{
    auto fw_edges_indexes = GetForwardEdgesIndexes(header);
    auto pre_header = header->CreateImmediateDominator();
    pre_header->SetGuestPc(header->GetGuestPc());
    if (fw_edges_indexes.size() >= 2U) {
        MovePhiInputsToPreHeader(header, pre_header, fw_edges_indexes);
    }
    UpdateControlFlowWithPreHeader(header, pre_header, fw_edges_indexes);
    return pre_header;
}

bool LoopAnalyzer::PreHeaderExists(Loop *loop)
{
    auto header = loop->GetHeader();

    return header->GetPredsBlocks().size() - loop->GetBackEdges().size() == 1 &&
           header->GetDominator()->GetLoop() == loop->GetOuterLoop() &&
           header->GetDominator() != GetGraph()->GetStartBlock();
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
        BasicBlock *pre_header = nullptr;
        if (PreHeaderExists(loop)) {
            pre_header = header->GetDominator();
        } else {
            pre_header = CreatePreHeader(header);
            pre_header->CopyTryCatchProps(header);
            loop->GetOuterLoop()->AppendBlock(pre_header);
        }
        loop->SetPreHeader(pre_header);
        pre_header->SetNextLoop(loop);
    }

    for (auto inner_loop : loop->GetInnerLoops()) {
        FindAndInsertPreHeaders(inner_loop);
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
            // Add back-edges to the loop for further analysis
            for (auto back_edge : loop->GetBackEdges()) {
                if (back_edge->GetLoop() != loop) {
                    loop->AppendBlock(back_edge);
                }
            }
        } else {
            black_marker_ = GetGraph()->NewMarker();
            block->SetMarker(black_marker_);
            for (auto back_edge : loop->GetBackEdges()) {
                NaturalLoopSearch(loop, back_edge);
            }
            GetGraph()->EraseMarker(black_marker_);
        }
    }

    // Populate the root loop with blocks which are not assign to any loops
    // Link all outer loops with the root loop
    auto root_loop = GetGraph()->GetRootLoop();
    for (auto block : GetGraph()->GetBlocksRPO()) {
        if (block->GetLoop() == nullptr) {
            root_loop->AppendBlock(block);
        } else if (block->GetLoop()->GetOuterLoop() == nullptr) {
            block->GetLoop()->SetOuterLoop(root_loop);
            root_loop->AppendInnerLoop(block->GetLoop());
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
    if (!block->IsMarked(black_marker_)) {
        block->SetMarker(black_marker_);

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

void LoopAnalyzer::SearchInfiniteLoops(Loop *loop)
{
    loop->CheckInfinity();
    if (loop->IsInfinite()) {
        GetGraph()->SetHasInfiniteLoop(true);
    }
    for (auto inner_loop : loop->GetInnerLoops()) {
        SearchInfiniteLoops(inner_loop);
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
    for (auto inner_loop : GetInnerLoops()) {
        ASSERT(block != inner_loop->GetPreHeader());
    }
#endif

    auto block_it = std::find(blocks_.begin(), blocks_.end(), block);
    ASSERT(block_it != blocks_.end());
    blocks_.erase(block_it);
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
    is_infinite_ = false;
    if (is_root_) {
        return;
    }
    auto outer_loop = GetOuterLoop();
    for (auto block : GetBlocks()) {
        const auto &succs = block->GetSuccsBlocks();
        bool has_exit = std::find_if(succs.begin(), succs.end(), [&outer_loop](const BasicBlock *bb) {
                            return bb->GetLoop() == outer_loop;
                        }) != succs.end();
        if (has_exit) {
            return;
        }
    }
    is_infinite_ = true;
}

/*
 * Find outside block for the loop with single back-edge exit
 */
BasicBlock *GetLoopOutsideSuccessor(Loop *loop)
{
    ASSERT(loop->GetBackEdges().size() == 1);
    auto back_edge = loop->GetBackEdges()[0];
    auto header_succ_idx = back_edge->GetSuccBlockIndex(loop->GetHeader());
    ASSERT(back_edge->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
    auto outside_block = back_edge->GetSuccessor(1 - header_succ_idx);
    ASSERT(outside_block != nullptr);
    return outside_block;
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
    auto back_edge = loop->GetBackEdges()[0];
    // Check there are no side-exits
    for (auto block : loop->GetBlocks()) {
        if (block == back_edge) {
            continue;
        }
        for (auto succ : block->GetSuccsBlocks()) {
            if (succ->GetLoop() != loop) {
                return false;
            }
        }
    }
    return true;
}
}  // namespace panda::compiler
