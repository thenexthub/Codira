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

#include "linear_order.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"

namespace panda::compiler {
LinearOrder::LinearOrder(Graph *graph)
    : Analysis(graph),
      linear_blocks_(graph->GetAllocator()->Adapter()),
      rpo_blocks_(graph->GetAllocator()->Adapter()),
      reordered_blocks_(graph->GetAllocator()->Adapter())
{
}

void LinearOrder::HandleIfBlock(BasicBlock *if_true_block, BasicBlock *next_block)
{
    ASSERT(if_true_block != nullptr && next_block != nullptr);
    ASSERT(!if_true_block->IsEmpty());
    if (if_true_block->GetTrueSuccessor() == next_block) {
        auto if_inst = if_true_block->GetLastInst();
        if_true_block->SwapTrueFalseSuccessors<true>();
        if (if_inst->GetOpcode() == Opcode::IfImm) {
            if_inst->CastToIfImm()->InverseConditionCode();
        } else if (if_inst->GetOpcode() == Opcode::If) {
            if_inst->CastToIf()->InverseConditionCode();
        } else {
            LOG(FATAL, COMPILER) << "Unexpected `If` instruction: " << *if_inst;
        }
    } else if (if_true_block->GetFalseSuccessor() != next_block) {
        if_true_block->SetNeedsJump(true);
    }
}

void LinearOrder::HandlePrevInstruction(BasicBlock *block, BasicBlock *prev_block)
{
    ASSERT(block != nullptr && prev_block != nullptr);
    ASSERT(!prev_block->NeedsJump());
    if (!prev_block->IsEmpty()) {
        auto prev_inst = prev_block->GetLastInst();
        switch (prev_inst->GetOpcode()) {
            case Opcode::IfImm:
            case Opcode::If:
                ASSERT(prev_block->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
                HandleIfBlock(prev_block, block);
                break;

            default:
                ASSERT(prev_block->GetSuccsBlocks().size() == 1 || prev_block->IsTryBegin() || prev_block->IsTryEnd());
                if (block != prev_block->GetSuccessor(0) && !prev_block->GetLastInst()->IsControlFlow()) {
                    prev_block->SetNeedsJump(true);
                }
                break;
        }
    } else if (!prev_block->IsEndBlock() && block != prev_block->GetSuccessor(0) &&
               !prev_block->GetSuccessor(0)->IsEndBlock()) {
        ASSERT(prev_block->GetSuccsBlocks().size() == 1 || prev_block->IsTryEnd());
        prev_block->SetNeedsJump(true);
    }
}

static void AddSortedByPc(ArenaList<BasicBlock *> *rpo_blocks, BasicBlock *bb)
{
    auto cmp = [](BasicBlock *lhs, BasicBlock *rhs) { return lhs->GetGuestPc() >= rhs->GetGuestPc(); };

    if (rpo_blocks->empty()) {
        rpo_blocks->push_back(bb);
        return;
    }

    auto iter = rpo_blocks->end();
    --iter;
    while (true) {
        if (cmp(bb, *iter)) {
            rpo_blocks->insert(++iter, bb);
            break;
        }
        if (iter == rpo_blocks->begin()) {
            rpo_blocks->push_front(bb);
            break;
        }
        --iter;
    }
}

template <class T>
void LinearOrder::MakeLinearOrder(const T &blocks)
{
    linear_blocks_.clear();
    linear_blocks_.reserve(blocks.size());

    BasicBlock *prev = nullptr;
    for (auto block : blocks) {
        if (prev != nullptr) {
            HandlePrevInstruction(block, prev);
        }
        linear_blocks_.push_back(block);
        prev = block;
    }

    if (prev != nullptr && !prev->IsEndBlock()) {
        // Handle last block
        ASSERT(prev->GetSuccsBlocks().size() == 1 || prev->IsIfBlock());
        prev->SetNeedsJump(true);
    }
}

BasicBlock *LinearOrder::LeastLikelySuccessor(const BasicBlock *block)
{
    if (!options.IsCompilerFreqBasedBranchReorder()) {
        return nullptr;
    }

    if (block->GetSuccsBlocks().size() != MAX_SUCCS_NUM) {
        return nullptr;
    }

    auto counter0 = GetGraph()->GetBranchCounter(block, true);
    auto counter1 = GetGraph()->GetBranchCounter(block, false);
    if (counter0 > 0 || counter1 > 0) {
        auto denom = std::max(counter0, counter1);
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto r = (counter0 - counter1) * 100 / denom;
        if (std::abs(r) < options.GetCompilerFreqBasedBranchReorderThreshold()) {
            return nullptr;
        }
        return r < 0 ? block->GetTrueSuccessor() : block->GetFalseSuccessor();
    }

    return nullptr;
}

// Similar to DFS but move least frequent branch to the end.
// First time method is called with defer_least_frequent=true template param which moves least likely successors to the
// end. After all most likely successors are processed call method with defer_least_frequent=false and process least
// frequent successors with DFS.
template <bool defer_least_frequent>
void LinearOrder::DFSAndDeferLeastFrequentBranches(BasicBlock *block, size_t *blocks_count)
{
    ASSERT(block != nullptr);
    block->SetMarker(marker_);

    auto least_likely_successor = defer_least_frequent ? LeastLikelySuccessor(block) : nullptr;
    if (least_likely_successor == nullptr) {
        for (auto succ_block : block->GetSuccsBlocks()) {
            if (!succ_block->IsMarked(marker_)) {
                DFSAndDeferLeastFrequentBranches<defer_least_frequent>(succ_block, blocks_count);
            }
        }
    } else {
        linear_blocks_.push_back(least_likely_successor);
        auto most_likely_successor = least_likely_successor == block->GetTrueSuccessor() ? block->GetFalseSuccessor()
                                                                                         : block->GetTrueSuccessor();
        if (!most_likely_successor->IsMarked(marker_)) {
            DFSAndDeferLeastFrequentBranches<defer_least_frequent>(most_likely_successor, blocks_count);
        }
    }

    if constexpr (defer_least_frequent) {  // NOLINT(readability-braces-around-statements,bugprone-suspicious-semicolon)
        for (auto succ_block : linear_blocks_) {
            if (!succ_block->IsMarked(marker_)) {
                DFSAndDeferLeastFrequentBranches<false>(succ_block, blocks_count);
            }
        }
        linear_blocks_.clear();
    }

    ASSERT(blocks_count != nullptr && *blocks_count > 0);
    reordered_blocks_[--(*blocks_count)] = block;
}

bool LinearOrder::RunImpl()
{
    if (GetGraph()->IsBytecodeOptimizer()) {
        // Make blocks order sorted by bytecode PC
        rpo_blocks_.clear();
        for (auto bb : GetGraph()->GetBlocksRPO()) {
            ASSERT(bb->GetGuestPc() != INVALID_PC);
            AddSortedByPc(&rpo_blocks_, bb);
        }
        MakeLinearOrder(rpo_blocks_);
    } else {
        marker_ = GetGraph()->NewMarker();
        size_t blocks_count = GetGraph()->GetAliveBlocksCount();
        linear_blocks_.clear();
        reordered_blocks_.clear();
        reordered_blocks_.resize(blocks_count);
        DFSAndDeferLeastFrequentBranches<true>(GetGraph()->GetStartBlock(), &blocks_count);
#ifndef NDEBUG
        if (blocks_count != 0) {
            std::cerr << "There are unreachable blocks:\n";
            for (auto bb : *GetGraph()) {
                if (bb != nullptr && !bb->IsMarked(marker_)) {
                    bb->Dump(&std::cerr);
                }
            }
            UNREACHABLE();
        }
#endif  // NDEBUG
        MakeLinearOrder(reordered_blocks_);
        GetGraph()->EraseMarker(marker_);
    }
    return true;
}
}  // namespace panda::compiler
