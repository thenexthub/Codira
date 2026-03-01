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
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/ir/graph_cloner.h"

namespace panda::compiler {
GraphCloner::GraphCloner(Graph *graph, ArenaAllocator *allocator, ArenaAllocator *local_allocator)
    : graph_(graph),
      allocator_(allocator),
      local_allocator_(local_allocator),
      clone_blocks_(allocator->Adapter()),
      clone_instructions_(allocator->Adapter())
{
}

/**
 * Clone the whole graph
 */
Graph *GraphCloner::CloneGraph()
{
    auto new_graph =
        allocator_->New<Graph>(allocator_, local_allocator_, GetGraph()->GetArch(), GetGraph()->GetMethod(),
                               GetGraph()->GetRuntime(), GetGraph()->GetParentGraph(), GetGraph()->GetMode());
    CHECK_NOT_NULL(new_graph);
    new_graph->SetCurrentInstructionId(GetGraph()->GetCurrentInstructionId());
    CloneBlocksAndInstructions<InstCloneType::CLONE_ALL, false>(GetGraph()->GetVectorBlocks(), new_graph);
    BuildControlFlow();
    BuildDataFlow();
    new_graph->GetPassManager()->SetCheckMode(GetGraph()->GetPassManager()->IsCheckMode());
    // Clone all flags
    new_graph->SetBitFields(GetGraph()->GetBitFields());
    new_graph->InitUsedRegs<DataType::INT64>(GetGraph()->GetUsedRegs<DataType::INT64>());
    new_graph->InitUsedRegs<DataType::FLOAT64>(GetGraph()->GetUsedRegs<DataType::FLOAT64>());
#ifndef NDEBUG
    CloneAnalyses(new_graph);
#endif
    return new_graph;
}

void GraphCloner::CloneAnalyses(Graph *new_graph)
{
    // Clone dominators if analysis is valid to check dom-tree
    ASSERT(!new_graph->IsAnalysisValid<DominatorsTree>());
    if (GetGraph()->IsAnalysisValid<DominatorsTree>()) {
        new_graph->GetAnalysis<DominatorsTree>().SetValid(true);
        for (auto block : GetGraph()->GetBlocksRPO()) {
            auto clone = GetClone(block);
            if (block->GetDominator() != nullptr) {
                auto clone_dom = GetClone(block->GetDominator());
                clone->SetDominator(clone_dom);
            }
            for (auto dom_blocks : block->GetDominatedBlocks()) {
                clone->AddDominatedBlock(GetClone(dom_blocks));
            }
        }
    }

    // Clone loops if analysis is valid to check loop-tree
    ASSERT(!new_graph->IsAnalysisValid<LoopAnalyzer>());
    if (GetGraph()->IsAnalysisValid<LoopAnalyzer>()) {
        auto &cloned_la = new_graph->GetAnalysis<LoopAnalyzer>();
        cloned_la.SetValid(true);
        cloned_la.CreateRootLoop();
        CopyLoop(GetGraph()->GetRootLoop(), new_graph->GetRootLoop());
        new_graph->SetHasIrreducibleLoop(GetGraph()->HasIrreducibleLoop());
        new_graph->SetHasInfiniteLoop(GetGraph()->HasInfiniteLoop());
    }

    ASSERT(!new_graph->IsAnalysisValid<LinearOrder>());
    if (GetGraph()->IsAnalysisValid<LinearOrder>()) {
        new_graph->GetAnalysis<LinearOrder>().SetValid(true);
        CloneLinearOrder(new_graph);
    }
}

void GraphCloner::CopyLoop(Loop *loop, Loop *cloned_loop)
{
    if (!loop->IsRoot() && !loop->IsIrreducible() && !loop->IsTryCatchLoop()) {
        ASSERT(GetClone(loop->GetHeader()) == cloned_loop->GetHeader());
        cloned_loop->SetPreHeader(GetClone(loop->GetPreHeader()));
    }
    for (auto block : loop->GetBlocks()) {
        if (block->IsLoopHeader()) {
            continue;
        }
        cloned_loop->AppendBlock(GetClone(block));
    }

    for (auto back_edge : loop->GetBackEdges()) {
        cloned_loop->AppendBackEdge(GetClone(back_edge));
    }
    cloned_loop->SetIsIrreducible(loop->IsIrreducible());
    cloned_loop->SetIsInfinite(loop->IsInfinite());

    // clone inner loops
    for (const auto &inner_loop : loop->GetInnerLoops()) {
        auto cloned_header = GetClone(inner_loop->GetHeader());
        auto &cloned_la = cloned_header->GetGraph()->GetAnalysis<LoopAnalyzer>();
        auto cloned_inner_loop = cloned_la.CreateNewLoop(cloned_header);
        cloned_inner_loop->SetOuterLoop(cloned_loop);
        cloned_loop->AppendInnerLoop(cloned_inner_loop);
        CopyLoop(inner_loop, cloned_inner_loop);
    }
}

void GraphCloner::CloneLinearOrder([[maybe_unused]] Graph *new_graph)
{
    ASSERT(new_graph != nullptr);
    ASSERT(GetGraph()->IsAnalysisValid<LinearOrder>());
    auto &clone_linear_blocks = new_graph->GetAnalysis<LinearOrder>().GetBlocks();
    clone_linear_blocks.reserve(GetGraph()->GetBlocksLinearOrder().size());
    for (auto block : GetGraph()->GetBlocksLinearOrder()) {
        clone_linear_blocks.push_back(GetClone(block));
    }
}

/**
 * Clone the whole graph control-flow
 */
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

/**
 * Clone the whole graph data-flow
 */
void GraphCloner::BuildDataFlow()
{
    for (const auto &block : GetGraph()->GetVectorBlocks()) {
        if (block == nullptr) {
            continue;
        }
        auto block_clone = GetClone(block);
        for (const auto &inst : block->Insts()) {
            SetCloneInputs<false>(inst);
            GetClone(inst)->SetId(inst->GetId());
            UpdateCaller(inst);
        }
        for (const auto &inst : block->PhiInsts()) {
            auto phi = inst->CastToPhi();
            auto inst_clone = GetClone(inst);
            inst_clone->SetId(inst->GetId());
            for (const auto &clone_pred_block : block_clone->GetPredsBlocks()) {
                auto it = std::find(clone_blocks_.begin(), clone_blocks_.end(), clone_pred_block);
                ASSERT(it != clone_blocks_.end());
                size_t index = std::distance(clone_blocks_.begin(), it);
                ASSERT(GetGraph()->GetVectorBlocks().size() > index);
                auto pred_block = GetGraph()->GetVectorBlocks()[index];
                inst_clone->AppendInput(GetClone(phi->GetPhiInput(pred_block)));
            }
        }
    }
}

/*
 * Create resolver-block - common successor for all loop side-exits
 */
BasicBlock *GraphCloner::CreateResolverBlock(Loop *loop, BasicBlock *back_edge)
{
    auto outside_succ = GetLoopOutsideSuccessor(loop);
    auto resolver = back_edge->InsertNewBlockToSuccEdge(outside_succ);
    back_edge->GetLoop()->GetOuterLoop()->AppendBlock(resolver);
    // Populate resolver-block with phis for each instruction which has outside-loop user
    for (auto block : loop->GetBlocks()) {
        for (auto inst : block->AllInsts()) {
            Inst *phi_resolver = nullptr;
            auto user_it = inst->GetUsers().begin();
            while (user_it != inst->GetUsers().end()) {
                auto user = user_it->GetInst();
                auto input_idx = user_it->GetIndex();
                ++user_it;
                ASSERT(user->GetBasicBlock() != nullptr);
                if (user->GetBasicBlock()->GetLoop() != loop) {
                    if (phi_resolver == nullptr) {
                        phi_resolver = GetGraph()->CreateInstPhi(inst->GetType(), inst->GetPc());
                        phi_resolver->AppendInput(inst);
                        resolver->AppendPhi(phi_resolver);
                    }
                    user->SetInput(input_idx, phi_resolver);
                }
            }
        }
    }
    return resolver;
}

/*
 * Split back-edge for cloning without side exits - in order not to clone `Compare` and `IfImm` instructions
 */
BasicBlock *GraphCloner::SplitBackEdge(LoopUnrollData *unroll_data, Loop *loop, BasicBlock *back_edge)
{
    auto ifimm = back_edge->GetLastInst();
    ASSERT(ifimm->GetOpcode() == Opcode::IfImm);
    auto compare = ifimm->GetInput(0).GetInst();
    ASSERT(compare->GetOpcode() == Opcode::Compare);
    // If there are intructions between `Compare` and `IfImm`, clone `Compare` and insert before `IfImm`
    if (ifimm->GetPrev() != compare) {
        auto new_cmp = compare->Clone(compare->GetBasicBlock()->GetGraph());
        new_cmp->SetInput(0, compare->GetInput(0).GetInst());
        new_cmp->SetInput(1, compare->GetInput(1).GetInst());
        ifimm->InsertBefore(new_cmp);
        ifimm->SetInput(0, new_cmp);
        compare = new_cmp;
    }
    if (compare->GetPrev() != nullptr) {
        auto back_edge_split = back_edge->SplitBlockAfterInstruction(compare->GetPrev(), true);
        loop->ReplaceBackEdge(back_edge, back_edge_split);
        back_edge = back_edge_split;
    } else {
        ASSERT(back_edge->GetPredsBlocks().size() == 1);
        auto it = std::find(unroll_data->blocks->begin(), unroll_data->blocks->end(), back_edge);
        ASSERT(it != unroll_data->blocks->end());
        unroll_data->blocks->erase(it);
    }
    [[maybe_unused]] static constexpr auto BACK_EDGE_INST_COUNT = 2;
    ASSERT(std::distance(back_edge->AllInsts().begin(), back_edge->AllInsts().end()) == BACK_EDGE_INST_COUNT);
    return back_edge;
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
GraphCloner::LoopUnrollData *GraphCloner::PrepareLoopToUnroll(Loop *loop, bool clone_side_exits)
{
    ASSERT(loop != nullptr);
    // Populate `LoopUnrollData`
    auto allocator = loop->GetHeader()->GetGraph()->GetLocalAllocator();
    auto unroll_data = allocator->New<LoopUnrollData>();
    CHECK_NOT_NULL(unroll_data);
    unroll_data->blocks = allocator->New<ArenaVector<BasicBlock *>>(allocator->Adapter());
    CHECK_NOT_NULL(unroll_data->blocks);
    unroll_data->blocks->resize(loop->GetBlocks().size());
    std::copy(loop->GetBlocks().begin(), loop->GetBlocks().end(), unroll_data->blocks->begin());
    // Split loop-header
    ASSERT(loop->GetBackEdges().size() == 1U);
    auto back_edge = loop->GetBackEdges()[0];
    ASSERT(back_edge != nullptr);
    auto header_block = loop->GetHeader();
    ASSERT(!header_block->IsEmpty());
    if (header_block->HasPhi()) {
        auto last_phi = header_block->GetFirstInst()->GetPrev();
        ASSERT(last_phi != nullptr && last_phi->IsPhi());
        auto header_split = header_block->SplitBlockAfterInstruction(last_phi, true);
        ASSERT(loop->GetBlocks().front() == header_block);
        unroll_data->blocks->at(0) = header_split;

        if (back_edge == header_block) {
            loop->ReplaceBackEdge(header_block, header_split);
            back_edge = header_split;
        }
    }
    unroll_data->exit_block = back_edge;
    if (clone_side_exits) {
        unroll_data->outer = CreateResolverBlock(loop, back_edge);
    } else {
        back_edge = SplitBackEdge(unroll_data, loop, back_edge);
    }
    // Save replaceable phi inputs

    unroll_data->phi_update_inputs = allocator->New<InstVector>(allocator->Adapter());
    CHECK_NOT_NULL(unroll_data->phi_update_inputs);
    for (auto phi : header_block->PhiInsts()) {
        unroll_data->phi_update_inputs->push_back(phi->CastToPhi()->GetPhiInput(back_edge));
    }
    unroll_data->header = header_block;
    unroll_data->backedge = back_edge;
    return unroll_data;
}

/**
 * Update data-flow after unrolling without side-exits
 */
void GraphCloner::UpdateUsersAfterNoSideExitsUnroll(const LoopUnrollData *unroll_data)
{
    auto loop = unroll_data->header->GetLoop();
    // Update outloop users: replace inputs located in the original loop by theirs clones
    auto compare = unroll_data->backedge->GetLastInst()->GetPrev();
    ASSERT(compare->GetOpcode() == Opcode::Compare);
    for (size_t i = 0; i < compare->GetInputsCount(); i++) {
        auto input = compare->GetInput(i).GetInst();
        if (HasClone(input)) {
            compare->SetInput(i, GetClone(input));
        }
    }
    // update outloop users
    for (auto block : *unroll_data->blocks) {
        for (auto inst : block->AllInsts()) {
            auto user_it = inst->GetUsers().begin();
            while (user_it != inst->GetUsers().end()) {
                auto user = user_it->GetInst();
                auto input_idx = user_it->GetIndex();
                ++user_it;
                ASSERT(user->GetBasicBlock() != nullptr);
                if (user->GetBasicBlock()->GetLoop() != loop) {
                    user->SetInput(input_idx, GetClone(inst));
                }
            }
        }
    }

    // All header-phi's outloop users are placed in the outer_bb after cloning the original loop
    // So it's enough to to iterate outer_bb's phis and repalce header-phi by its backedge input
    auto outer_idx = 1U - unroll_data->backedge->GetSuccBlockIndex(unroll_data->header);
    auto outer_bb = unroll_data->backedge->GetSuccessor(outer_idx);
    for (auto outer_phi : outer_bb->PhiInsts()) {
        auto header_phi = outer_phi->CastToPhi()->GetPhiInput(unroll_data->backedge);
        if (header_phi->IsPhi() && header_phi->GetBasicBlock() == unroll_data->header) {
            outer_phi->ReplaceInput(header_phi, header_phi->CastToPhi()->GetPhiInput(unroll_data->backedge));
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
void GraphCloner::BuildLoopUnrollControlFlow(LoopUnrollData *unroll_data)
{
    auto front_block = unroll_data->blocks->front();
    auto loop = front_block->GetLoop();

    // Copy 'blocks' control-flow to the 'clones'
    for (auto block : *unroll_data->blocks) {
        ASSERT(block->GetLoop() == loop);
        loop->AppendBlock(GetClone(block));

        if (block != front_block) {
            CloneEdges<CloneEdgeType::EDGE_PRED>(block);
        }
        if (block != unroll_data->exit_block) {
            CloneEdges<CloneEdgeType::EDGE_SUCC>(block);
        }
    }

    auto front_clone = GetClone(front_block);
    auto exit_clone = GetClone(unroll_data->exit_block);
    if (unroll_data->outer == nullptr) {
        ASSERT(unroll_data->backedge->GetPredsBlocks().size() == 1);
        auto last_block = unroll_data->backedge->GetPredsBlocks()[0];
        last_block->ReplaceSucc(unroll_data->backedge, front_clone);
        unroll_data->backedge->ReplacePred(last_block, exit_clone);
    } else {
        ASSERT(!unroll_data->outer->GetPredsBlocks().empty());
        auto last_block = unroll_data->outer->GetPredsBlocks().back();
        last_block->ReplaceSucc(unroll_data->header, front_clone);
        unroll_data->header->ReplacePred(last_block, exit_clone);

        exit_clone->AddSucc(unroll_data->outer);
        if (exit_clone->GetSuccBlockIndex(unroll_data->outer) != last_block->GetSuccBlockIndex(unroll_data->outer)) {
            exit_clone->SwapTrueFalseSuccessors();
        }
        auto new_backedge = GetClone(unroll_data->exit_block);
        loop->ReplaceBackEdge(unroll_data->backedge, new_backedge);
        unroll_data->backedge = new_backedge;
    }
}

/**
 * Construct dataflow for the cloned instructions
 * if input of the original instruction is front-block phi - insert replaceable input of this phi
 */
void GraphCloner::BuildLoopUnrollDataFlow(LoopUnrollData *unroll_data)
{
    for (auto block : *unroll_data->blocks) {
        for (auto inst : block->AllInsts()) {
            if (inst->IsMarked(clone_marker_)) {
                SetCloneInputs<true>(inst, unroll_data->backedge);
                UpdateCaller(inst);
            }
        }
    }

    // Append input to the phi-resolver from outer block, it holds all instructions which have outside loop users
    auto loop = unroll_data->blocks->front()->GetLoop();
    if (unroll_data->outer != nullptr) {
        for (auto phi : unroll_data->outer->PhiInsts()) {
            auto inst = phi->GetInput(0).GetInst();
            if (IsInstLoopHeaderPhi(inst, loop)) {
                auto update = inst->CastToPhi()->GetPhiInput(unroll_data->backedge);
                phi->AppendInput(update);
            } else {
                phi->AppendInput(GetClone(inst));
            }
        }
    }

    // TODO (a.popov) use temp container after if would be possible to reset local allocator
    if (unroll_data->phi_replaced_inputs == nullptr) {
        unroll_data->phi_replaced_inputs = allocator_->New<PhiInputsMap>(allocator_->Adapter());
        CHECK_NOT_NULL(unroll_data->phi_replaced_inputs);
    } else {
        unroll_data->phi_replaced_inputs->clear();
    }

    // Set new update inputs for header phis
    size_t phi_count = 0;
    for (auto phi : loop->GetHeader()->PhiInsts()) {
        auto input = unroll_data->phi_update_inputs->at(phi_count);
        if (HasClone(input)) {
            input = GetClone(input);
        } else if (input->IsPhi() && input->GetBasicBlock()->GetLoop() == loop) {
            if (phi->IsDominate(input)) {
                input = input->CastToPhi()->GetPhiInput(unroll_data->backedge);
            } else {
                // phi should be visited and its input should be added to the map
                ASSERT(unroll_data->phi_replaced_inputs->count(input) == 1);
                input = unroll_data->phi_replaced_inputs->at(input);
            }
        }

        auto phi_update_input_idx = phi->CastToPhi()->GetPredBlockIndex(unroll_data->backedge);
        unroll_data->phi_replaced_inputs->emplace(phi, phi->GetInput(phi_update_input_idx).GetInst());
        phi->SetInput(phi_update_input_idx, input);
        phi_count++;
    }
}

void GraphCloner::RemoveLoopBackEdge(const LoopUnrollData *unroll_data)
{
    ASSERT(unroll_data->outer != nullptr);
    ASSERT(!unroll_data->outer->GetPredsBlocks().empty());
    auto last_block = unroll_data->outer->GetPredsBlocks().back();
    // Erase control-flow instruction
    auto ifimm = last_block->GetLastInst();
    ASSERT(ifimm->GetOpcode() == Opcode::IfImm);
    last_block->RemoveInst(ifimm);
    // Remove back-edge
    auto header = unroll_data->header;
    // Clear header block, it should only contain phi
    ASSERT(header->GetFirstInst() == nullptr);
    for (auto phi : header->PhiInstsSafe()) {
        auto remaining_inst = phi->CastToPhi()->GetPhiInput(header->GetLoop()->GetPreHeader());
        phi->ReplaceUsers(remaining_inst);
        header->RemoveInst(phi);
    }

    last_block->RemoveSucc(header);
    header->RemovePred(last_block);

    // Clear outer phis if it has single predecessor
    if (unroll_data->outer->GetPredsBlocks().size() == 1U) {
        for (auto phi : unroll_data->outer->PhiInstsSafe()) {
            auto remaining_inst = phi->GetInput(0).GetInst();
            phi->ReplaceUsers(remaining_inst);
            unroll_data->outer->RemoveInst(phi);
        }
    }
}

void GraphCloner::BuildClonedLoopHeaderDataFlow(const BasicBlock &block, BasicBlock *resolver, BasicBlock *clone)
{
    for (auto inst : block.Insts()) {
        if (inst->IsMarked(clone_marker_)) {
            SetCloneInputs<true>(inst, clone);
            UpdateUsersForClonedLoopHeader(inst, resolver);
            UpdateCaller(inst);
        }
    }
    for (auto phi : block.PhiInsts()) {
        ASSERT(phi->GetInputsCount() == 2U);
        auto preloop_input = phi->CastToPhi()->GetPhiInput(clone);
        // Create phi instruction in the `resolver` block with two inputs: current phi and phi's preloop_input
        auto resolver_phi = GetGraph()->CreateInstPhi(phi->GetType(), phi->GetPc());
        for (auto user_it = phi->GetUsers().begin(); user_it != phi->GetUsers().end();) {
            auto user = user_it->GetInst();
            auto input_index = user_it->GetIndex();
            ++user_it;
            ASSERT(user->GetBasicBlock() != nullptr);
            if (user->GetBasicBlock()->GetLoop() != block.GetLoop()) {
                user->SetInput(input_index, resolver_phi);
            }
        }
        if (resolver_phi->HasUsers()) {
            resolver_phi->AppendInput(phi);
            resolver_phi->AppendInput(preloop_input);
            resolver->AppendPhi(resolver_phi);
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
BasicBlock *GraphCloner::CloneLoopHeader(BasicBlock *block, BasicBlock *outer, BasicBlock *replaceable_pred)
{
    ASSERT(GetGraph()->IsAnalysisValid<DominatorsTree>());
    ASSERT(clone_marker_ == UNDEF_MARKER);
    auto marker_holder = MarkerHolder(GetGraph());
    clone_marker_ = marker_holder.GetMarker();
    clone_instructions_.clear();
    size_t inst_count = 0;
    // Build control-flow
    auto resolver = block->InsertNewBlockToSuccEdge(outer);
    outer->GetLoop()->AppendBlock(resolver);
    if (outer->GetLoop()->HasBackEdge(block)) {
        outer->GetLoop()->ReplaceBackEdge(block, resolver);
    }
    auto clone_block = replaceable_pred->InsertNewBlockToSuccEdge(block);
    ASSERT(block->GetLoop()->GetPreHeader() == replaceable_pred);
    block->GetLoop()->SetPreHeader(clone_block);
    replaceable_pred->GetLoop()->AppendBlock(clone_block);
    clone_block->AddSucc(resolver);
    // Check the order of true-false successors
    if (clone_block->GetSuccBlockIndex(resolver) != block->GetSuccBlockIndex(resolver)) {
        clone_block->SwapTrueFalseSuccessors();
    }
    // Fix Dominators info
    auto &dom_tree = GetGraph()->GetAnalysis<DominatorsTree>();
    dom_tree.SetValid(true);
    ASSERT(block->GetDominator() == replaceable_pred);
    replaceable_pred->RemoveDominatedBlock(block);
    dom_tree.SetDomPair(clone_block, block);
    dom_tree.SetDomPair(replaceable_pred, clone_block);
    dom_tree.SetDomPair(clone_block, resolver);
    if (outer->GetDominator() == block) {
        block->RemoveDominatedBlock(outer);
        dom_tree.SetDomPair(resolver, outer);
    }
    CloneInstructions<InstCloneType::CLONE_INSTS, true>(block, clone_block, &inst_count);
    BuildClonedLoopHeaderDataFlow(*block, resolver, clone_block);
    clone_block->SetAllFields(block->GetAllFields());
    return clone_block;
}

/**
 * Use the following logic cloning the users:
 * - replace inputs of all users, placed OUTSIDE cloneable loop, by the new `phi_out` instruction
 * - `phi_out` is appended to the outer-block
 * - replace inputs of all users, placed INSIDE cloneable loop, but not cloned, by the new `phi_in` instruction
 * - `phi_in` is appended to the `inst` basic block
 * - `phi_in\phi_out` have `inst` and its clone as inputs
 */
void GraphCloner::UpdateUsersForClonedLoopHeader(Inst *inst, BasicBlock *outer_block)
{
    if (!inst->HasUsers()) {
        return;
    }
    auto inst_block = inst->GetBasicBlock();
    auto clone = GetClone(inst);
    auto clone_block = clone->GetBasicBlock();
    ASSERT(clone_block != nullptr);
    // phi for inside users
    auto phi_in = GetGraph()->CreateInstPhi(inst->GetType(), inst->GetPc());
    // phi for outside users
    auto phi_out = GetGraph()->CreateInstPhi(inst->GetType(), inst->GetPc());
    auto user_it = inst->GetUsers().begin();
    while (user_it != inst->GetUsers().end()) {
        auto user = user_it->GetInst();
        auto input_idx = user_it->GetIndex();
        ++user_it;
        ASSERT(user->GetBasicBlock() != nullptr);
        if (user->GetBasicBlock()->GetLoop() == inst_block->GetLoop()) {
            // user inside loop
            // skip users that will be moved to the loop-exit block
            if (user->GetBasicBlock()->IsLoopHeader() && !user->IsPhi()) {
                continue;
            }
            ASSERT(user->GetBasicBlock() != inst_block || user->IsPhi());
            user->SetInput(input_idx, phi_in);
        } else {
            // user outside loop
            user->SetInput(input_idx, phi_out);
        }
    }

    if (phi_in->HasUsers()) {
        auto clone_index {inst_block->GetPredBlockIndex(clone_block)};
        phi_in->AppendInput(clone);
        phi_in->AppendInput(inst);
        phi_in->CastToPhi()->SetPhiInputBbNum(0, clone_index);
        phi_in->CastToPhi()->SetPhiInputBbNum(1, 1 - clone_index);

        auto first_phi = inst_block->GetFirstPhi();
        if (first_phi == nullptr) {
            inst_block->AppendPhi(phi_in);
        } else {
            inst_block->InsertBefore(phi_in, first_phi);
        }
    }

    if (phi_out->HasUsers()) {
        phi_out->AppendInput(inst);
        phi_out->AppendInput(clone);
        outer_block->AppendPhi(phi_out);
    }
}

inline bool GraphCloner::IsInstLoopHeaderPhi(Inst *inst, Loop *loop)
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
    ASSERT(clone_marker_ == UNDEF_MARKER);

    auto marker_holder = MarkerHolder(GetGraph());
    clone_marker_ = marker_holder.GetMarker();
    auto unroll_data = PrepareLoopToClone(loop);

    CloneBlocksAndInstructions<InstCloneType::CLONE_ALL, true>(*unroll_data->blocks, GetGraph());
    BuildLoopCloneControlFlow(unroll_data);
    BuildLoopCloneDataFlow(unroll_data);
    MakeLoopCloneInfo(unroll_data);
    GetGraph()->RunPass<DominatorsTree>();

    auto clone_loop = GetClone(loop->GetHeader())->GetLoop();
    ASSERT(clone_loop != loop && clone_loop->GetOuterLoop() == loop->GetOuterLoop());
    COMPILER_LOG(DEBUG, GRAPH_CLONER) << "Loop " << loop->GetId() << " is copied";
    COMPILER_LOG(DEBUG, GRAPH_CLONER) << "Created new loop, id = " << clone_loop->GetId();
    return clone_loop;
}

BasicBlock *GraphCloner::CreateNewOutsideSucc(BasicBlock *outside_succ, BasicBlock *back_edge, BasicBlock *pre_header)
{
    auto back_edge_idx = outside_succ->GetPredBlockIndex(back_edge);
    auto pre_header_idx = outside_succ->GetPredBlockIndex(pre_header);
    auto rm_idx_max = std::max(back_edge_idx, pre_header_idx);
    auto rm_idx_min = std::min(back_edge_idx, pre_header_idx);
    auto new_outside_succ = GetGraph()->CreateEmptyBlock();
    outside_succ->GetLoop()->AppendBlock(new_outside_succ);
    back_edge->ReplaceSucc(outside_succ, new_outside_succ);
    pre_header->ReplaceSucc(outside_succ, new_outside_succ);
    new_outside_succ->AddSucc(outside_succ);
    for (auto phi : outside_succ->PhiInsts()) {
        auto new_phi = GetGraph()->CreateInstPhi(phi->GetType(), phi->GetPc());
        new_phi->AppendInput(phi->CastToPhi()->GetPhiInput(back_edge));
        new_phi->AppendInput(phi->CastToPhi()->GetPhiInput(pre_header));
        phi->AppendInput(new_phi);
        auto phi_back_edge_idx {phi->CastToPhi()->GetPredBlockIndex(back_edge)};
        auto phi_pre_header_idx {phi->CastToPhi()->GetPredBlockIndex(pre_header)};
        phi->RemoveInput(rm_idx_max == back_edge_idx ? phi_back_edge_idx : phi_pre_header_idx);
        phi->RemoveInput(rm_idx_min == pre_header_idx ? phi_pre_header_idx : phi_back_edge_idx);
        new_outside_succ->AppendPhi(new_phi);
    }
    outside_succ->RemovePred(rm_idx_max);
    outside_succ->RemovePred(rm_idx_min);

    COMPILER_LOG(DEBUG, GRAPH_CLONER) << "New loop outside block created: " << new_outside_succ->GetId();
    return new_outside_succ;
}

/**
 * - Split pre-header to contain `Compare` and `IfImm` instructions only;
 * - Make sure `outside_succ` has 2 predecessors only: loop header and back-edge;
 * - Split `outside_succ` to contain phi-instructions only;
 */
GraphCloner::LoopClonerData *GraphCloner::PrepareLoopToClone(Loop *loop)
{
    auto pre_header = loop->GetPreHeader();
    auto ifimm = pre_header->GetLastInst();
    ASSERT(ifimm->GetOpcode() == Opcode::IfImm);
    auto compare = ifimm->GetInput(0).GetInst();
    ASSERT(compare->GetOpcode() == Opcode::Compare);
    if (ifimm->GetPrev() != compare) {
        auto new_cmp = compare->Clone(compare->GetBasicBlock()->GetGraph());
        new_cmp->SetInput(0, compare->GetInput(0).GetInst());
        new_cmp->SetInput(1, compare->GetInput(1).GetInst());
        ifimm->InsertBefore(new_cmp);
        ifimm->SetInput(0, new_cmp);
        compare = new_cmp;
    }
    if (compare->GetPrev() != nullptr) {
        auto new_pre_header = pre_header->SplitBlockAfterInstruction(compare->GetPrev(), true);
        loop->SetPreHeader(new_pre_header);
        pre_header = new_pre_header;
    }
    [[maybe_unused]] static constexpr auto PRE_HEADER_INST_COUNT = 2;
    ASSERT(std::distance(pre_header->AllInsts().begin(), pre_header->AllInsts().end()) == PRE_HEADER_INST_COUNT);
    // If `outside_succ` has more than 2 predecessors, create a new one
    // with loop header and back-edge predecessors only and insert it before `outside_succ`
    auto outside_succ = GetLoopOutsideSuccessor(loop);
    constexpr auto PREDS_NUM = 2;
    if (outside_succ->GetPredsBlocks().size() > PREDS_NUM) {
        auto back_edge = loop->GetBackEdges()[0];
        outside_succ = CreateNewOutsideSucc(outside_succ, back_edge, pre_header);
    }
    // Split outside succ after last phi
    // create empty block before outside succ if outside succ don't contain phi insts
    if (outside_succ->HasPhi() && outside_succ->GetFirstInst() != nullptr) {
        auto last_phi = outside_succ->GetFirstInst()->GetPrev();
        auto block = outside_succ->SplitBlockAfterInstruction(last_phi, true);
        // if `outside_succ` is pre-header replace it by `block`
        for (auto in_loop : loop->GetOuterLoop()->GetInnerLoops()) {
            if (in_loop->GetPreHeader() == outside_succ) {
                in_loop->SetPreHeader(block);
            }
        }
    } else if (outside_succ->GetFirstInst() != nullptr) {
        auto block = outside_succ->InsertEmptyBlockBefore();
        outside_succ->GetLoop()->AppendBlock(block);
        outside_succ = block;
    }
    return CreateLoopClonerData(loop, pre_header, outside_succ);
}

GraphCloner::LoopClonerData *GraphCloner::CreateLoopClonerData(Loop *loop, BasicBlock *pre_header,
                                                               BasicBlock *outside_succ)
{
    // Populate `LoopClonerData`
    auto allocator = GetGraph()->GetLocalAllocator();
    auto unroll_data = allocator->New<LoopClonerData>();
    CHECK_NOT_NULL(unroll_data);
    unroll_data->blocks = allocator->New<ArenaVector<BasicBlock *>>(allocator->Adapter());
    CHECK_NOT_NULL(unroll_data->blocks);
    unroll_data->blocks->resize(loop->GetBlocks().size() + 1);
    unroll_data->blocks->at(0) = pre_header;
    std::copy(loop->GetBlocks().begin(), loop->GetBlocks().end(), unroll_data->blocks->begin() + 1);
    unroll_data->blocks->push_back(outside_succ);
    unroll_data->outer = outside_succ;
    unroll_data->header = loop->GetHeader();
    unroll_data->pre_header = loop->GetPreHeader();
    return unroll_data;
}

/**
 * Create new loop, populate it with cloned blocks and build conrlow-flow
 */
void GraphCloner::BuildLoopCloneControlFlow(LoopClonerData *unroll_data)
{
    ASSERT(unroll_data != nullptr);
    auto outer_clone = GetClone(unroll_data->outer);
    auto pre_header_clone = GetClone(unroll_data->pre_header);

    while (!unroll_data->outer->GetSuccsBlocks().empty()) {
        auto succ = unroll_data->outer->GetSuccsBlocks().front();
        succ->ReplacePred(unroll_data->outer, outer_clone);
        unroll_data->outer->RemoveSucc(succ);
    }
    unroll_data->outer->AddSucc(pre_header_clone);

    for (auto &block : *unroll_data->blocks) {
        if (block != unroll_data->pre_header) {
            CloneEdges<CloneEdgeType::EDGE_PRED>(block);
        }
        if (block != unroll_data->outer) {
            CloneEdges<CloneEdgeType::EDGE_SUCC>(block);
        }
    }
    ASSERT(unroll_data->outer->GetPredBlockIndex(unroll_data->pre_header) ==
           outer_clone->GetPredBlockIndex(pre_header_clone));
    ASSERT(unroll_data->header->GetPredBlockIndex(unroll_data->pre_header) ==
           GetClone(unroll_data->header)->GetPredBlockIndex(pre_header_clone));
}

/**
 * Insert cloned loop into loop-tree and populated with cloned blocks
 */
void GraphCloner::MakeLoopCloneInfo(LoopClonerData *unroll_data)
{
    ASSERT(unroll_data != nullptr);
    // Update loop tree
    auto loop = unroll_data->header->GetLoop();
    auto header_clone = GetClone(loop->GetHeader());
    auto clone_loop = GetGraph()->GetAnalysis<LoopAnalyzer>().CreateNewLoop(header_clone);
    auto outer_loop = loop->GetOuterLoop();
    outer_loop->AppendInnerLoop(clone_loop);
    clone_loop->SetOuterLoop(outer_loop);

    // Populate cloned loop
    auto pre_loop_clone = GetClone(unroll_data->pre_header);
    auto outside_succ_clone = GetClone(unroll_data->outer);
    clone_loop->SetPreHeader(pre_loop_clone);
    outer_loop->AppendBlock(pre_loop_clone);
    outer_loop->AppendBlock(outside_succ_clone);
    for (auto &block : loop->GetBlocks()) {
        if (!block->IsLoopHeader()) {
            clone_loop->AppendBlock(GetClone(block));
        }
    }
    for (auto back_edge : loop->GetBackEdges()) {
        clone_loop->AppendBackEdge(GetClone(back_edge));
    }
}

/**
 * Find or create phi in the outside_succ block with the same inputs as `check_phi`
 */
Inst *GetPhiResolver(Inst *check_phi, BasicBlock *outside_succ, BasicBlock *pre_header)
{
    [[maybe_unused]] constexpr auto MAX_PREDS_NUM = 2;
    ASSERT(outside_succ->GetPredsBlocks().size() == MAX_PREDS_NUM);
    ASSERT(check_phi->GetBasicBlock()->IsLoopHeader());
    auto init_idx = check_phi->CastToPhi()->GetPredBlockIndex(pre_header);
    auto init_input = check_phi->GetInput(init_idx).GetInst();
    auto update_input = check_phi->GetInput(1 - init_idx).GetInst();

    for (auto phi : outside_succ->PhiInsts()) {
        auto idx {phi->CastToPhi()->GetPredBlockIndex(pre_header)};
        if (phi->GetInput(idx).GetInst() == init_input && phi->GetInput(1 - idx).GetInst() == update_input) {
            return phi;
        }
    }

    auto phi_resolver = outside_succ->GetGraph()->CreateInstPhi(check_phi->GetType(), check_phi->GetPc());
    auto out_init_idx = outside_succ->GetPredBlockIndex(pre_header);
    phi_resolver->AppendInput(init_input);
    phi_resolver->AppendInput(update_input);
    phi_resolver->SetPhiInputBbNum(0, out_init_idx);
    phi_resolver->SetPhiInputBbNum(1, 1 - out_init_idx);

    outside_succ->AppendPhi(phi_resolver);
    return phi_resolver;
}

/**
 * Build data-flow for cloned instructions
 */
void GraphCloner::BuildLoopCloneDataFlow(LoopClonerData *unroll_data)
{
    ASSERT(unroll_data != nullptr);
    for (const auto &block : *unroll_data->blocks) {
        for (const auto &inst : block->AllInsts()) {
            if (inst->IsMarked(clone_marker_)) {
                SetCloneInputs<false>(inst);
                UpdateCaller(inst);
            }
        }
    }

    auto pre_loop_clone = GetClone(unroll_data->pre_header);
    for (auto phi : unroll_data->outer->PhiInsts()) {
        auto phi_clone = GetClone(phi);
        phi->ReplaceUsers(phi_clone);
        auto idx = phi_clone->CastToPhi()->GetPredBlockIndex(pre_loop_clone);
        phi_clone->SetInput(idx, phi);
    }

    auto compare = pre_loop_clone->GetFirstInst();
    auto exit_block = unroll_data->outer->GetPredBlockByIndex(0);
    if (exit_block == unroll_data->pre_header) {
        exit_block = unroll_data->outer->GetPredBlockByIndex(1);
    } else {
        ASSERT(unroll_data->outer->GetPredBlockByIndex(1) == unroll_data->pre_header);
    }
    auto back_edge_compare = exit_block->GetLastInst()->GetInput(0).GetInst();
    ASSERT(compare->GetOpcode() == Opcode::Compare);
    ASSERT(back_edge_compare->GetOpcode() == Opcode::Compare);
    for (auto phi : unroll_data->header->PhiInsts()) {
        auto init_idx = phi->CastToPhi()->GetPredBlockIndex(unroll_data->pre_header);
        ASSERT(GetClone(phi)->CastToPhi()->GetPredBlockIndex(pre_loop_clone) == init_idx);

        auto init = phi->GetInput(init_idx).GetInst();
        auto update = phi->GetInput(1 - init_idx).GetInst();
        auto resolver_phi = GetPhiResolver(phi, unroll_data->outer, unroll_data->pre_header);
        auto clone_phi = GetClone(phi);
        ASSERT(clone_phi->GetInput(init_idx).GetInst() == init);
        clone_phi->SetInput(init_idx, resolver_phi);
        for (size_t i = 0; i < compare->GetInputsCount(); i++) {
            if (compare->GetInput(i).GetInst() == init && back_edge_compare->GetInput(i).GetInst() == update) {
                compare->SetInput(i, resolver_phi);
                break;
            }
        }
    }
}

void GraphCloner::UpdateCaller(Inst *inst)
{
}

bool GraphCloner::IsLoopClonable(Loop *loop, size_t inst_limit)
{
    // TODO(schernykh) : implement case when we have inner loops
    if (!loop->GetOuterLoop()->IsRoot() || !loop->GetInnerLoops().empty() || !IsLoopSingleBackEdgeExitPoint(loop)) {
        return false;
    }

    auto pre_header = loop->GetPreHeader();
    auto ifimm = pre_header->GetLastInst();
    ASSERT(ifimm->GetOpcode() == Opcode::IfImm);
    auto compare = ifimm->GetInput(0).GetInst();
    ASSERT(compare->GetOpcode() == Opcode::Compare);
    // TODO(schernykh) : Implement case when compare is not before of ifimm
    if (ifimm->GetPrev() != compare) {
        return false;
    }

    // Count instructions for copy
    // in pre header copied compare + ifimm inst
    uint32_t inst_count = 1;
    for (const auto &block : loop->GetBlocks()) {
        inst_count += std::distance(block->AllInsts().begin(), block->AllInsts().end());
        if (inst_count > inst_limit) {
            return false;
        }
    }
    for ([[maybe_unused]] auto phi : GetLoopOutsideSuccessor(loop)->PhiInsts()) {
        inst_count++;
    }
    return (inst_count <= inst_limit);
}
}  // namespace panda::compiler
