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

#include <fstream>
#include "bytecode_inst.h"
#include "static_core/compiler/optimizer/analysis/dominators_tree.h"
#include "static_core/compiler/optimizer/analysis/loop_analyzer.h"
#include "libabckit/src/irbuilder_dynamic/ir_builder_dyn.h"

namespace libabckit {

bool IrBuilderDynamic::RunImpl()
{
    auto instructionsBuf = GetGraph()->GetRuntime()->GetMethodCode(GetMethod());
    BytecodeInstructions pbcInstructions(instructionsBuf, GetGraph()->GetRuntime()->GetMethodCodeSize(GetMethod()));
    size_t vregsCount = GetGraph()->GetRuntime()->GetMethodRegistersCount(GetMethod()) +
                        GetGraph()->GetRuntime()->GetMethodTotalArgumentsCount(GetMethod()) + 1;
    if (!CheckMethodLimitations(pbcInstructions, vregsCount)) {
        return false;
    }
    GetGraph()->SetVRegsCount(vregsCount);
    BuildBasicBlocks(pbcInstructions);
    GetGraph()->RunPass<compiler::DominatorsTree>();
    GetGraph()->RunPass<compiler::LoopAnalyzer>();

    InstBuilder instBuilder(GetGraph(), GetMethod());
    instBuilder.Prepare();
    instDefs_.resize(vregsCount);

    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (!BuildBasicBlock(bb, &instBuilder, instructionsBuf)) {
            return false;
        }
    }

    GetGraph()->RunPass<compiler::DominatorsTree>();
    GetGraph()->InvalidateAnalysis<compiler::LoopAnalyzer>();
    GetGraph()->RunPass<compiler::LoopAnalyzer>();
    instBuilder.FixInstructions();
    return true;
}

bool IrBuilderDynamic::CheckMethodLimitations(const BytecodeInstructions & /*instructions*/, size_t /*vregsCount*/)
{
    return true;
}

bool IrBuilderDynamic::BuildBasicBlock(compiler::BasicBlock *bb, InstBuilder *instBuilder,
                                       const uint8_t *instructionsBuf)
{
    instBuilder->SetCurrentBlock(bb);
    instBuilder->UpdateDefs();
    ASSERT(bb->GetGuestPc() != ark::compiler::INVALID_PC);
    // If block is not in the `blocks_` vector, it's auxiliary block without instructions
    if (bb == blocks_[bb->GetGuestPc()]) {
        return BuildInstructionsForBB(bb, instBuilder, instructionsBuf);
    }

    return true;
}

bool IrBuilderDynamic::BuildInstructionsForBB(compiler::BasicBlock *bb, InstBuilder *instBuilder,
                                              const uint8_t *instructionsBuf)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    BytecodeInstructions instructions(instructionsBuf + bb->GetGuestPc(), std::numeric_limits<int>::max());
    for (auto inst : instructions) {
        auto pc = instBuilder->GetPc(inst.GetAddress());
        // Break if current pc is pc of some basic block, that means that it is the end of the current block.
        if (pc != bb->GetGuestPc() && GetBlockForPc(pc) != nullptr) {
            break;
        }
        // Copy current defs for assigning them to catch-phi if current inst is throwable
        ASSERT(instBuilder->GetCurrentDefs().size() == instDefs_.size());
        std::copy(instBuilder->GetCurrentDefs().begin(), instBuilder->GetCurrentDefs().end(), instDefs_.begin());
        auto currentLastInst = bb->GetLastInst();
        auto bbCount = GetGraph()->GetVectorBlocks().size();
        instBuilder->BuildInstruction(&inst);

        if (instBuilder->IsFailed()) {
            std::cerr << "Unsupported instruction\n";
            return false;
        }

        // One PBC instruction can be expanded to the group of IR's instructions, find first built instruction in
        // this group, and then mark all instructions as throwable; All instructions should be marked, since some of
        // them can be deleted during optimizations, unnecessary catch-phi moves will be resolved before Register
        // Allocator
        auto throwableInst = (currentLastInst == nullptr) ? bb->GetFirstInst() : currentLastInst->GetNext();
        ProcessThrowableInstructions(instBuilder, throwableInst);

        auto &vb = GetGraph()->GetVectorBlocks();
        for (size_t i = bbCount; i < vb.size(); i++) {
            ProcessThrowableInstructions(instBuilder, vb[i]->GetFirstInst());
        }

        // Break if we meet terminator instruction. If instruction in the middle of basic block we don't create
        // further dead instructions.
        if (inst.IsTerminator() && !inst.IsSuspend()) {
            break;
        }
    }
    return true;
}

void IrBuilderDynamic::ProcessThrowableInstructions(InstBuilder *instBuilder, compiler::Inst *throwableInst)
{
    for (; throwableInst != nullptr; throwableInst = throwableInst->GetNext()) {
        if (throwableInst->IsSaveState()) {
            continue;
        }
        catchHandlers_.clear();
        EnumerateTryBlocksCoveredPc(throwableInst->GetPc(), [this](const TryCodeBlock &tryBlock) {
            auto tbb = tryBlock.beginBb;
            tbb->EnumerateCatchHandlers([this](compiler::BasicBlock *catchHandler, [[maybe_unused]] size_t typeId) {
                catchHandlers_.insert(catchHandler);
                return true;
            });
        });
        if (!catchHandlers_.empty()) {
            instBuilder->AddCatchPhiInputs(catchHandlers_, instDefs_, throwableInst);
        }
    }
}

static inline bool InstNotJump(BytecodeInst *inst)
{
    return inst->GetAddress() != nullptr && InstBuilder::GetInstructionJumpOffset(inst) == INVALID_OFFSET &&
           !inst->HasFlag(BytecodeInst::RETURN);
}

void IrBuilderDynamic::BuildBasicBlocks(const BytecodeInstructions &instructions)
{
    blocks_.resize(instructions.GetSize() + 1);
    bool fallthrough = false;

    CreateBlock(0);
    // Create basic blocks
    for (auto inst : instructions) {
        auto pc = instructions.GetPc(inst);

        if (fallthrough) {
            CreateBlock(pc);
            fallthrough = false;
        }
        auto offset = InstBuilder::GetInstructionJumpOffset(&inst);
        if (offset != INVALID_OFFSET) {
            auto targetPc = pc + static_cast<size_t>(offset);
            CreateBlock(targetPc);
            if (inst.HasFlag(BytecodeInst::CONDITIONAL)) {
                fallthrough = true;
            }
        }
    }
    CreateTryCatchBoundariesBlocks();
    GetGraph()->CreateStartBlock();
    GetGraph()->CreateEndBlock(instructions.GetSize());
    ConnectBasicBlocks(instructions);
    ResolveTryCatchBlocks();
}

template <class Callback>
void IrBuilderDynamic::EnumerateTryBlocksCoveredPc(uint32_t pc, const Callback &callback)
{
    for (const auto &[begin_pc, try_block] : tryBlocks_) {
        if (begin_pc <= pc && pc < try_block.boundaries.endPc) {
            callback(try_block);
        }
    }
}

/**
 * Return `TryCodeBlock` and flag if was created a new one
 */
IrBuilderDynamic::TryCodeBlock *IrBuilderDynamic::InsertTryBlockInfo(const Boundaries &tryBoundaries)
{
    auto tryId = static_cast<uint32_t>(tryBlocks_.size());
    auto range = tryBlocks_.equal_range(tryBoundaries.beginPc);
    for (auto iter = range.first; iter != range.second; ++iter) {
        // use try-block with the same boundaries
        if (tryBoundaries.endPc == iter->second.boundaries.endPc) {
            return &iter->second;
        }
        // insert in the increasing `end_pc` order
        if (tryBoundaries.endPc > iter->second.boundaries.endPc) {
            auto it = tryBlocks_.emplace_hint(iter, tryBoundaries.beginPc, TryCodeBlock {tryBoundaries});
            it->second.Init(GetGraph(), tryId);
            return &it->second;
        }
    }
    auto it = tryBlocks_.emplace(tryBoundaries.beginPc, TryCodeBlock {tryBoundaries});
    it->second.Init(GetGraph(), tryId);
    return &it->second;
}

void IrBuilderDynamic::CreateTryCatchBoundariesBlocks()
{
    auto *pfw = static_cast<FileWrapper *>(GetGraph()->GetRuntime()->GetBinaryFileForMethod(GetMethod()));
    pfw->EnumerateTryBlocks(GetMethod(), [pfw, this](void *tryBlock) {
        auto [start, end] = pfw->GetTryBlockBoundaries(tryBlock);
        auto *tryInfo = InsertTryBlockInfo({start, end});
        pfw->EnumerateCatchBlocksForTryBlock(tryBlock, [pfw, this, tryInfo](void *catchBlock) {
            auto pc = pfw->GetCatchBlockHandlerPc(catchBlock);
            catchesPc_.insert(pc);
            tryInfo->catches->emplace_back(CatchCodeBlock {pc, 0U});
        });
    });

    for (const auto &[pc, try_block] : tryBlocks_) {
        CreateBlock(pc);
        CreateBlock(try_block.boundaries.endPc);
    }
    for (auto pc : catchesPc_) {
        CreateBlock(pc);
    }
}

compiler::BasicBlock *IrBuilderDynamic::AddSuccs(BlocksConnectorInfo *info, compiler::BasicBlock *currBb,
                                                 compiler::BasicBlock *targetBlock, size_t &pc, BytecodeInst &inst)
{
    if (info->fallthrough) {
        ASSERT(targetBlock != nullptr);
        // May be the second edge between same blocks
        currBb->AddSucc(targetBlock, true);
        info->fallthrough = false;
        currBb = targetBlock;
    } else if (targetBlock != nullptr) {
        if (catchesPc_.count(pc) == 0) {
            if (InstNotJump(&info->prevInst) && !info->deadInstructions) {
                currBb->AddSucc(targetBlock);
            }
        }
        currBb = targetBlock;
        info->deadInstructions = false;
    } else if (info->deadInstructions) {
        // We are processing dead instructions now, skipping them until we meet the next block.
        return currBb;
    }
    if (auto jmpTargetBlock = GetBlockToJump(&inst, pc); jmpTargetBlock != nullptr) {
        currBb->AddSucc(jmpTargetBlock);
        // In case of unconditional branch, we reset curr_bb, so if next instruction won't start new block, then
        // we'll skip further dead instructions.
        info->fallthrough = inst.HasFlag(BytecodeInst::CONDITIONAL);
        if (!info->fallthrough) {
            info->deadInstructions = true;
        }
    }
    info->prevInst = inst;
    return currBb;
}

void IrBuilderDynamic::ConnectBasicBlocks(const BytecodeInstructions &instructions)
{
    BlocksConnectorInfo info;
    compiler::BasicBlock *currBb = blocks_[0];
    GetGraph()->GetStartBlock()->AddSucc(currBb);
    for (auto inst : instructions) {
        auto pc = instructions.GetPc(inst);
        auto targetBlock = blocks_[pc];
        TrackTryBoundaries(pc);
        currBb = AddSuccs(&info, currBb, targetBlock, pc, inst);
    }

    // Erase end block if it wasn't connected, should be infinite loop in the graph
    if (GetGraph()->GetEndBlock()->GetPredsBlocks().empty()) {
        GetGraph()->EraseBlock(GetGraph()->GetEndBlock());
        GetGraph()->SetEndBlock(nullptr);
    }
}

void IrBuilderDynamic::TrackTryBoundaries(size_t pc)
{
    openedTryBlocks_.remove_if([pc](TryCodeBlock *tryBlock) { return tryBlock->boundaries.endPc == pc; });

    if (tryBlocks_.count(pc) > 0) {
        auto range = tryBlocks_.equal_range(pc);
        for (auto it = range.first; it != range.second; ++it) {
            auto &tryBlock = it->second;
            if (tryBlock.boundaries.endPc > pc) {
                openedTryBlocks_.push_back(&tryBlock);
                auto allocator = GetGraph()->GetLocalAllocator();
                tryBlock.basicBlocks = allocator->New<ArenaVector<compiler::BasicBlock *>>(allocator->Adapter());
            } else {
                // Empty try-block
                ASSERT(tryBlock.boundaries.endPc == pc);
            }
        }
    }

    if (openedTryBlocks_.empty()) {
        return;
    }

    if (auto bb = blocks_[pc]; bb != nullptr) {
        for (auto tryBlock : openedTryBlocks_) {
            tryBlock->basicBlocks->push_back(bb);
        }
    }

    for (auto &tryBlock : openedTryBlocks_) {
        tryBlock->containsThrowableInst = true;
    }
}

compiler::BasicBlock *IrBuilderDynamic::GetBlockToJump(BytecodeInst *inst, size_t pc)
{
    if ((inst->HasFlag(BytecodeInst::RETURN) && !inst->HasFlag(BytecodeInst::SUSPEND)) ||
        inst->IsThrow(BytecodeInst::Exceptions::X_THROW)) {
        return GetGraph()->GetEndBlock();
    }

#ifdef ENABLE_BYTECODE_OPT
    if (inst->GetOpcode() == BytecodeInst::Opcode::RETURNUNDEFINED) {
        return GetGraph()->GetEndBlock();
    }
#endif

    if (auto offset = InstBuilder::GetInstructionJumpOffset(inst); offset != INVALID_OFFSET) {
        ASSERT(blocks_[pc + static_cast<size_t>(offset)] != nullptr);
        return blocks_[pc + static_cast<size_t>(offset)];
    }
    return nullptr;
}

/**
 * Mark blocks which were connected to the graph.
 * Catch-handlers will not be marked, since they have not been connected yet.
 */
static void MarkNormalControlFlow(compiler::BasicBlock *block, compiler::Marker marker)
{
    block->SetMarker(marker);
    for (auto succ : block->GetSuccsBlocks()) {
        if (!succ->IsMarked(marker)) {
            MarkNormalControlFlow(succ, marker);
        }
    }
}

void IrBuilderDynamic::MarkTryCatchBlocks(compiler::Marker marker)
{
    // All blocks without `normal` mark are considered as catch-blocks
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (bb->IsMarked(marker)) {
            continue;
        }
        if (bb->IsTryBegin()) {
            bb->SetCatch(bb->GetSuccessor(0)->IsCatch());
        } else if (bb->IsTryEnd()) {
            bb->SetCatch(bb->GetPredecessor(0)->IsCatch());
        } else {
            bb->SetCatch(true);
        }
    }

    // Nested try-blocks can be removed, but referring to them basic blocks can be placed in the external try-blocks.
    // So `try` marks are added after removing unreachable blocks
    for (auto it : tryBlocks_) {
        const auto &tryBlock = it.second;
        if (tryBlock.beginBb->GetGraph() != tryBlock.endBb->GetGraph()) {
            RestoreTryEnd(tryBlock);
        }
        tryBlock.beginBb->SetTryId(tryBlock.id);
        tryBlock.endBb->SetTryId(tryBlock.id);
        if (tryBlock.basicBlocks == nullptr) {
            continue;
        }
        for (auto bb : *tryBlock.basicBlocks) {
            bb->SetTryId(tryBlock.id);
            bb->SetTry(true);
        }
    }
}

/*
 * Connect catch-blocks to the graph.
 */
void IrBuilderDynamic::ResolveTryCatchBlocks()
{
    auto markerHolder = compiler::MarkerHolder(GetGraph());
    auto marker = markerHolder.GetMarker();
    MarkNormalControlFlow(GetGraph()->GetStartBlock(), marker);
    ConnectTryCatchBlocks();
    GetGraph()->RemoveUnreachableBlocks();
    MarkTryCatchBlocks(marker);
}

void IrBuilderDynamic::ConnectTryCatchBlocks()
{
    ArenaMap<uint32_t, compiler::BasicBlock *> catchBlocks(GetGraph()->GetLocalAllocator()->Adapter());
    // Firstly create catch_begin blocks, as they should precede try_begin blocks
    for (auto pc : catchesPc_) {
        auto catchBegin = GetGraph()->CreateEmptyBlock();
        catchBegin->SetGuestPc(pc);
        catchBegin->SetCatch(true);
        catchBegin->SetCatchBegin(true);
        auto firstCatchBb = GetBlockForPc(pc);
        catchBegin->AddSucc(firstCatchBb);
        catchBlocks.emplace(pc, catchBegin);
    }

    // Connect try_begin and catch_begin blocks
    for (auto it : tryBlocks_) {
        const auto &tryBlock = it.second;
        if (tryBlock.containsThrowableInst) {
            ConnectTryCodeBlock(tryBlock, catchBlocks);
        } else if (tryBlock.basicBlocks != nullptr) {
            tryBlock.basicBlocks->clear();
        }
    }
}

void IrBuilderDynamic::ConnectTryCodeBlock(const TryCodeBlock &tryBlock,
                                           const ArenaMap<uint32_t, compiler::BasicBlock *> &catchBlocks)
{
    auto tryBegin = tryBlock.beginBb;
    ASSERT(tryBegin != nullptr);
    auto tryEnd = tryBlock.endBb;
    ASSERT(tryEnd != nullptr);
    // Create auxiliary `Try` instruction
    auto tryInst = GetGraph()->CreateInstTry();
    tryInst->SetTryEndBlock(tryEnd);
    tryBegin->AppendInst(tryInst);
    // Insert `try_begin` and `try_end`
    auto firstTryBb = GetBlockForPc(tryBlock.boundaries.beginPc);
    auto lastTryBb = GetPrevBlockForPc(tryBlock.boundaries.endPc);
    firstTryBb->InsertBlockBefore(tryBegin);
    lastTryBb->InsertBlockBeforeSucc(tryEnd, lastTryBb->GetSuccessor(0));
    // Connect catch-handlers
    for (auto catchBlock : *tryBlock.catches) {
        auto catchBegin = catchBlocks.at(catchBlock.pc);
        if (!tryBegin->HasSucc(catchBegin)) {
            tryBegin->AddSucc(catchBegin, true);
            tryEnd->AddSucc(catchBegin, true);
        }
        tryInst->AppendCatchTypeId(catchBlock.typeId, tryBegin->GetSuccBlockIndex(catchBegin));
    }
}

/**
 * `try_end` restoring is required in the following case:
 * try {
 *       try { a++;}
 *       catch { a++; }
 * }
 *
 * Nested try doesn't contain throwable instructions and related catch-handler will not be connected to the graph.
 * As a result all `catch` basic blocks will be eliminated together with outer's `try_end`, since it was inserted just
 * after `catch`
 */
void IrBuilderDynamic::RestoreTryEnd(const TryCodeBlock &tryBlock)
{
    ASSERT(tryBlock.endBb->GetGraph() == nullptr);
    ASSERT(tryBlock.endBb->GetSuccsBlocks().empty());
    ASSERT(tryBlock.endBb->GetPredsBlocks().empty());

    GetGraph()->RestoreBlock(tryBlock.endBb);
    auto lastTryBb = GetPrevBlockForPc(tryBlock.boundaries.endPc);
    lastTryBb->InsertBlockBeforeSucc(tryBlock.endBb, lastTryBb->GetSuccessor(0));
    for (auto succ : tryBlock.beginBb->GetSuccsBlocks()) {
        if (succ->IsCatchBegin()) {
            tryBlock.endBb->AddSucc(succ);
        }
    }
}

}  // namespace libabckit
