/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "linear_order.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"

namespace ark::compiler {
LinearOrder::LinearOrder(Graph *graph)
    : Analysis(graph),
      linearBlocks_(graph->GetAllocator()->Adapter()),
      rpoBlocks_(graph->GetAllocator()->Adapter()),
      reorderedBlocks_(graph->GetAllocator()->Adapter())
{
}

void LinearOrder::HandleIfBlock(BasicBlock *ifTrueBlock, BasicBlock *nextBlock)
{
    ASSERT(ifTrueBlock != nullptr && nextBlock != nullptr);
    ASSERT(!ifTrueBlock->IsEmpty());
    if (ifTrueBlock->GetTrueSuccessor() == nextBlock) {
        // The following swap of successors could break loop analyzer results in the case of irreducible loop
        GetGraph()->InvalidateAnalysis<LoopAnalyzer>();

        auto ifInst = ifTrueBlock->GetLastInst();
        ifTrueBlock->SwapTrueFalseSuccessors<true>();
        if (ifInst->GetOpcode() == Opcode::IfImm) {
            ifInst->CastToIfImm()->InverseConditionCode();
        } else if (ifInst->GetOpcode() == Opcode::If) {
            ifInst->CastToIf()->InverseConditionCode();
        } else if (ifInst->GetOpcode() == Opcode::AddOverflow) {
            ifInst->CastToAddOverflow()->InverseConditionCode();
        } else if (ifInst->GetOpcode() == Opcode::SubOverflow) {
            ifInst->CastToSubOverflow()->InverseConditionCode();
        } else {
            LOG(FATAL, COMPILER) << "Unexpected `If` instruction: " << *ifInst;
        }
    } else if (ifTrueBlock->GetFalseSuccessor() != nextBlock && !ifTrueBlock->GetSuccessor(0)->IsEndBlock()) {
        ifTrueBlock->SetNeedsJump(true);
    }
}

void LinearOrder::HandlePrevInstruction(BasicBlock *block, BasicBlock *prevBlock)
{
    ASSERT(block != nullptr && prevBlock != nullptr);
    ASSERT(!prevBlock->NeedsJump());
    if (!prevBlock->IsEmpty()) {
        auto prevInst = prevBlock->GetLastInst();
        switch (prevInst->GetOpcode()) {
            case Opcode::IfImm:
            case Opcode::If:
            case Opcode::AddOverflow:
            case Opcode::SubOverflow:
                ASSERT(prevBlock->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
                HandleIfBlock(prevBlock, block);
                break;

            case Opcode::Throw:
                break;

            default:
                if (GetGraph()->IsAbcKit()) {
                    ABCKIT_MODE_CHECK(prevInst->GetOpcode() == Opcode::Intrinsic &&
                                          prevInst->CastToIntrinsic()->GetIntrinsicId() ==
                                              RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_THROW,
                                      break);
                }
                ASSERT(prevBlock->GetSuccsBlocks().size() == 1 || prevBlock->IsTryBegin() || prevBlock->IsTryEnd());
                if (block != prevBlock->GetSuccessor(0) && !prevBlock->GetLastInst()->IsControlFlow()) {
                    prevBlock->SetNeedsJump(true);
                }
                break;
        }
    } else if (!prevBlock->IsEndBlock() && block != prevBlock->GetSuccessor(0) &&
               !prevBlock->GetSuccessor(0)->IsEndBlock()) {
        ASSERT(prevBlock->GetSuccsBlocks().size() == 1 || prevBlock->IsTryEnd());
        prevBlock->SetNeedsJump(true);
    }
}

static void AddSortedByPc(ArenaList<BasicBlock *> *rpoBlocks, BasicBlock *bb)
{
    auto cmp = [](BasicBlock *lhs, BasicBlock *rhs) { return lhs->GetGuestPc() >= rhs->GetGuestPc(); };

    if (rpoBlocks->empty()) {
        rpoBlocks->push_back(bb);
        return;
    }

    auto iter = rpoBlocks->end();
    --iter;
    while (true) {
        if (cmp(bb, *iter)) {
            rpoBlocks->insert(++iter, bb);
            break;
        }
        if (iter == rpoBlocks->begin()) {
            rpoBlocks->push_front(bb);
            break;
        }
        --iter;
    }
}

template <class T>
void LinearOrder::MakeLinearOrder(const T &blocks)
{
    linearBlocks_.clear();
    linearBlocks_.reserve(blocks.size());

    BasicBlock *prev = nullptr;
    for (auto block : blocks) {
        if (prev != nullptr) {
            HandlePrevInstruction(block, prev);
        }
        linearBlocks_.push_back(block);
        prev = block;
    }

    if (prev != nullptr && !prev->IsEndBlock() && !prev->GetSuccessor(0)->IsEndBlock()) {
        // Handle last block
        ASSERT(prev->GetSuccsBlocks().size() == 1 || prev->IsIfBlock());
        prev->SetNeedsJump(true);
    }
}

BasicBlock *LinearOrder::LeastLikelySuccessor(const BasicBlock *block)
{
    auto leastLikelySuccessor = LeastLikelySuccessorByBranchCounter(block);
    if (leastLikelySuccessor != nullptr) {
        return leastLikelySuccessor;
    }

    leastLikelySuccessor = LeastLikelySuccessorByPreference(block);
    if (leastLikelySuccessor != nullptr) {
        return leastLikelySuccessor;
    }

    if (block->GetSuccsBlocks().size() != MAX_SUCCS_NUM) {
        return nullptr;
    }

    auto trueSucc = block->GetTrueSuccessor();
    auto falseSucc = block->GetFalseSuccessor();
    ASSERT(trueSucc != nullptr && falseSucc != nullptr);
    if (falseSucc->IsMarked(blocksMarker_) != trueSucc->IsMarked(blocksMarker_)) {
        return falseSucc->IsMarked(blocksMarker_) ? falseSucc : trueSucc;
    }
    return nullptr;
}

BasicBlock *LinearOrder::LeastLikelySuccessorByBranchCounter(const BasicBlock *block)
{
    if (!g_options.IsCompilerFreqBasedBranchReorder()) {
        return nullptr;
    }

    if (block->GetSuccsBlocks().size() != MAX_SUCCS_NUM) {
        return nullptr;
    }

    auto counter0 = GetBranchCounter(block, true);
    auto counter1 = GetBranchCounter(block, false);
    if (counter0 > 0 || counter1 > 0) {
        auto denom = std::max(counter0, counter1);
        ASSERT(denom != 0);
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto r = (counter0 - counter1) * 100 / denom;
        if (std::abs(r) < g_options.GetCompilerFreqBasedBranchReorderThreshold()) {
            return nullptr;
        }
        return r < 0 ? block->GetTrueSuccessor() : block->GetFalseSuccessor();
    }

    return nullptr;
}

int64_t LinearOrder::GetBranchCounter(const BasicBlock *block, bool trueSucc)
{
    auto counter = GetGraph()->GetBranchCounter(block, trueSucc);
    if (counter > 0) {
        return counter;
    }
    if (IsConditionChainCounter(block)) {
        return GetConditionChainCounter(block, trueSucc);
    }
    return 0;
}

bool LinearOrder::IsConditionChainCounter(const BasicBlock *block)
{
    auto lastInst = block->GetLastInst();
    if (lastInst->GetOpcode() != Opcode::IfImm) {
        return false;
    }
    auto lastInstInput = lastInst->GetInput(0).GetInst();
    if (!lastInstInput->IsPhi()) {
        return false;
    }
    for (auto &input : lastInstInput->GetInputs()) {
        if (!input.GetInst()->IsConst()) {
            return false;
        }
        if (input.GetInst()->GetType() != DataType::INT64) {
            return false;
        }
        auto val = input.GetInst()->CastToConstant()->GetIntValue();
        if (val != 0 && val != 1) {
            return false;
        }
    }
    return true;
}

int64_t LinearOrder::GetConditionChainCounter(const BasicBlock *block, bool trueSucc)
{
    if (trueSucc != block->IsInverted()) {
        return GetConditionChainTrueSuccessorCounter(block);
    }

    return GetConditionChainFalseSuccessorCounter(block);
}

int64_t LinearOrder::GetConditionChainTrueSuccessorCounter(const BasicBlock *block)
{
    auto lastInst = block->GetLastInst();
    auto lastInstInput = lastInst->GetInput(0).GetInst();
    int64_t counter = 0;
    for (size_t i = 0; i < lastInstInput->GetInputsCount(); i++) {
        auto input = lastInstInput->GetInput(i);
        auto val = input.GetInst()->CastToConstant()->GetIntValue();
        if (val != 1) {
            continue;
        }
        auto bb = lastInstInput->GetBasicBlock();
        auto pred = bb->GetPredBlockByIndex(i);
        while (pred->GetSuccsBlocks().size() != MAX_SUCCS_NUM) {
            bb = pred;
            if (pred->GetPredsBlocks().empty()) {
                return 0;
            }
            pred = pred->GetPredBlockByIndex(0);
        }
        counter += GetGraph()->GetBranchCounter(pred, pred->GetTrueSuccessor() == bb);
    }
    return counter;
}

int64_t LinearOrder::GetConditionChainFalseSuccessorCounter(const BasicBlock *block)
{
    auto lastInst = block->GetLastInst();
    auto lastInstInput = lastInst->GetInput(0).GetInst();
    auto bb = lastInstInput->GetBasicBlock();
    BasicBlock *falsePred = nullptr;
    for (size_t i = 0; i < lastInstInput->GetInputsCount(); i++) {
        auto input = lastInstInput->GetInput(i);
        auto val = input.GetInst()->CastToConstant()->GetIntValue();
        if (val == 0) {
            falsePred = bb->GetPredBlockByIndex(i);
            break;
        }
    }
    if (falsePred == nullptr) {
        return 0;
    }
    while (falsePred->GetSuccsBlocks().size() != MAX_SUCCS_NUM) {
        bb = falsePred;
        if (bb->GetPredsBlocks().empty()) {
            return 0;
        }
        falsePred = falsePred->GetPredBlockByIndex(0);
    }
    return GetGraph()->GetBranchCounter(falsePred, falsePred->GetTrueSuccessor() == bb);
}

BasicBlock *LinearOrder::LeastLikelySuccessorByPreference(const BasicBlock *block)
{
    if (block->GetSuccsBlocks().size() != MAX_SUCCS_NUM) {
        return nullptr;
    }

    auto lastInst = block->GetLastInst();
    switch (lastInst->GetOpcode()) {
        case Opcode::If: {
            auto ifInst = lastInst->CastToIf();
            if (ifInst->IsLikely()) {
                ASSERT(!ifInst->IsUnlikely());
                return block->GetFalseSuccessor();
            }
            if (ifInst->IsUnlikely()) {
                ASSERT(!ifInst->IsLikely());
                return block->GetTrueSuccessor();
            }
            return nullptr;
        }
        case Opcode::IfImm: {
            auto ifimmInst = lastInst->CastToIfImm();
            if (ifimmInst->IsLikely()) {
                ASSERT(!ifimmInst->IsUnlikely());
                return block->GetFalseSuccessor();
            }
            if (ifimmInst->IsUnlikely()) {
                ASSERT(!ifimmInst->IsLikely());
                return block->GetTrueSuccessor();
            }
            return nullptr;
        }
        default:
            return nullptr;
    }
}
// Similar to DFS but move least frequent branch to the end.
// First time method is called with defer_least_frequent=true template param which moves least likely successors to the
// end. After all most likely successors are processed call method with defer_least_frequent=false and process least
// frequent successors with DFS.
template <bool DEFER_LEAST_FREQUENT>
void LinearOrder::DFSAndDeferLeastFrequentBranches(BasicBlock *block, size_t *blocksCount)
{
    ASSERT(block != nullptr);
    block->SetMarker(marker_);

    auto leastLikelySuccessor = DEFER_LEAST_FREQUENT ? LeastLikelySuccessor(block) : nullptr;
    if (leastLikelySuccessor == nullptr) {
        for (auto succBlock : block->GetSuccsBlocks()) {
            if (!succBlock->IsMarked(marker_)) {
                DFSAndDeferLeastFrequentBranches<DEFER_LEAST_FREQUENT>(succBlock, blocksCount);
            }
        }
    } else {
        linearBlocks_.push_back(leastLikelySuccessor);
        auto mostLikelySuccessor =
            leastLikelySuccessor == block->GetTrueSuccessor() ? block->GetFalseSuccessor() : block->GetTrueSuccessor();
        if (!mostLikelySuccessor->IsMarked(marker_)) {
            DFSAndDeferLeastFrequentBranches<DEFER_LEAST_FREQUENT>(mostLikelySuccessor, blocksCount);
        }
    }

    if constexpr (DEFER_LEAST_FREQUENT) {  // NOLINT(readability-braces-around-statements,bugprone-suspicious-semicolon)
        for (auto succBlock : linearBlocks_) {
            if (!succBlock->IsMarked(marker_)) {
                DFSAndDeferLeastFrequentBranches<false>(succBlock, blocksCount);
            }
        }
        linearBlocks_.clear();
    }

    ASSERT(blocksCount != nullptr && *blocksCount > 0);
    reorderedBlocks_[--(*blocksCount)] = block;
}

void LinearOrder::MarkSideExitsBlocks()
{
    auto endBlock = GetGraph()->GetEndBlock();
    // Check on infinite loop
    if (endBlock == nullptr) {
        return;
    }
    for (auto predBlock : endBlock->GetPredsBlocks()) {
        if (predBlock->IsEmpty() || predBlock->IsStartBlock()) {
            continue;
        }
        ASSERT(predBlock->GetSuccsBlocks().size() == 1);
        auto lastInst = predBlock->GetLastInst();
        ASSERT(lastInst != nullptr);
        if (lastInst->IsReturn()) {
            continue;
        }
        predBlock->SetMarker(blocksMarker_);
    }
}

void LinearOrder::DumpUnreachableBlocks()
{
    std::cerr << "There are unreachable blocks:\n";
    for (auto bb : *GetGraph()) {
        if (bb != nullptr && !bb->IsMarked(marker_)) {
            bb->Dump(&std::cerr);
        }
    }
    UNREACHABLE();
}

bool LinearOrder::RunImpl()
{
    if (GetGraph()->IsBytecodeOptimizer()) {
        // Make blocks order sorted by bytecode PC
        rpoBlocks_.clear();
        for (auto bb : GetGraph()->GetBlocksRPO()) {
            ASSERT(bb->GetGuestPc() != INVALID_PC);
            AddSortedByPc(&rpoBlocks_, bb);
        }
        MakeLinearOrder(rpoBlocks_);
    } else {
        marker_ = GetGraph()->NewMarker();
        blocksMarker_ = GetGraph()->NewMarker();
        size_t blocksCount = GetGraph()->GetAliveBlocksCount();
        linearBlocks_.clear();
        reorderedBlocks_.clear();
        reorderedBlocks_.resize(blocksCount);
        MarkSideExitsBlocks();
        DFSAndDeferLeastFrequentBranches<true>(GetGraph()->GetStartBlock(), &blocksCount);
#ifndef NDEBUG
        if (blocksCount != 0) {
            DumpUnreachableBlocks();
        }
#endif  // NDEBUG
        MakeLinearOrder(reorderedBlocks_);

        GetGraph()->EraseMarker(marker_);
        GetGraph()->EraseMarker(blocksMarker_);
    }
    return true;
}
}  // namespace ark::compiler
