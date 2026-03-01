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

#include "libarkfile/file_items.h"
#include "ir_builder.h"
#include "compiler_logger.h"
#include "libarkbase/macros.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/runtime_interface.h"
#include "pbc_iterator.h"
#include "libarkfile/bytecode_instruction.h"
#include "libarkfile/code_data_accessor-inl.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/hotness_propagation.h"
#include "libarkfile/method_data_accessor-inl.h"

#ifdef ENABLE_LIBABCKIT
#include "abckit_inst_builder.h"
#define ABCKIT_INST_BUILDER(graph, method, callerInst, inliningDepth) \
    AbcKitInstBuilder(graph, method, callerInst, inliningDepth)
#else
// CC-OFFNXT(G.PRE.02) should be with define
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ABCKIT_INST_BUILDER(graph, method, callerInst, inliningDepth) \
    InstBuilder(graph, method, callerInst, inliningDepth)
#endif

namespace ark::compiler {

bool IrBuilder::RunImpl()
{
    COMPILER_LOG(INFO, IR_BUILDER) << "Start building ir for method: "
                                   << GetGraph()->GetRuntime()->GetClassNameFromMethod(GetMethod()) << "."
                                   << GetGraph()->GetRuntime()->GetMethodName(GetMethod())
                                   << "(args=" << GetGraph()->GetRuntime()->GetMethodTotalArgumentsCount(GetMethod())
                                   << ", regs=" << GetGraph()->GetRuntime()->GetMethodRegistersCount(GetMethod())
                                   << ")";

    auto instructionsBuf = GetGraph()->GetRuntime()->GetMethodCode(GetMethod());
    BytecodeInstructions pbcInstructions(instructionsBuf, GetGraph()->GetRuntime()->GetMethodCodeSize(GetMethod()));
    size_t vregsCount = GetGraph()->GetRuntime()->GetMethodRegistersCount(GetMethod()) +
                        GetGraph()->GetRuntime()->GetMethodTotalArgumentsCount(GetMethod()) + 1;
    if (!CheckMethodLimitations(pbcInstructions, vregsCount)) {
        return false;
    }
    GetGraph()->SetVRegsCount(vregsCount);
    BuildBasicBlocks(pbcInstructions);
    GetGraph()->RunPass<DominatorsTree>();
    GetGraph()->RunPass<LoopAnalyzer>();

    if (!BuildIr(vregsCount)) {
        return false;
    }

    if (g_options.IsCompilerPrintStats() || g_options.WasSetCompilerDumpStatsCsv()) {
        uint64_t pbcInstNum = 0;
        for ([[maybe_unused]] auto i : pbcInstructions) {
            pbcInstNum++;
        }
        GetGraph()->GetPassManager()->GetStatistics()->AddPbcInstNum(pbcInstNum);
    }

    COMPILER_LOG(INFO, IR_BUILDER) << "IR successfully built: " << GetGraph()->GetVectorBlocks().size()
                                   << " basic blocks, " << GetGraph()->GetCurrentInstructionId() << " instructions";

    if (!GetGraph()->IsAbcKit()) {
        HotnessPropagation(GetGraph()).Run();
    }

    return true;
}

bool IrBuilder::BuildIrImpl(InstBuilder &instBuilder, size_t vregsCount)
{
    instBuilder.Prepare(isInlinedGraph_);

    auto instructionsBuf = GetGraph()->GetRuntime()->GetMethodCode(GetMethod());

    instDefs_.resize(vregsCount + GetGraph()->GetEnvCount());
    COMPILER_LOG(INFO, IR_BUILDER) << "Start instructions building...";
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (!BuildBasicBlock(instBuilder, bb, instructionsBuf)) {
            return false;
        }
    }
    if (GetGraph()->IsBytecodeOptimizer() && !g_options.IsCompilerNonOptimizing() && !GetGraph()->IsDynamicMethod()) {
        ConnectThrowBlocks();
        if (GetGraph()->IsThrowApplied()) {
            InvalidateBlocksOrderAnalyzes(GetGraph());
        }
    }
    GetGraph()->RunPass<DominatorsTree>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->RunPass<LoopAnalyzer>();
    instBuilder.FixInstructions();
    if (GetGraph()->GetRuntime()->IsMemoryBarrierRequired(GetMethod())) {
        SetMemoryBarrierFlag();
    }
    return true;
}

bool IrBuilder::BuildIr(size_t vregsCount)
{
    if (GetGraph()->IsAbcKit()) {
        auto instBuilder = ABCKIT_INST_BUILDER(GetGraph(), GetMethod(), callerInst_, inliningDepth_);
        return BuildIrImpl(instBuilder, vregsCount);  // releases instBuilder's internal markers on ~InstBuilder
    }

    auto instBuilder = InstBuilder(GetGraph(), GetMethod(), callerInst_, inliningDepth_);
    return BuildIrImpl(instBuilder, vregsCount);  // releases instBuilder's internal markers on ~InstBuilder
}

void IrBuilder::SetMemoryBarrierFlag()
{
    COMPILER_LOG(INFO, IR_BUILDER) << "Setting memory barrier flag";
    for (auto preEnd : GetGraph()->GetEndBlock()->GetPredsBlocks()) {
        if (preEnd->IsTryEnd()) {
            ASSERT(preEnd->GetPredsBlocks().size() == 1U);
            preEnd = preEnd->GetPredecessor(0);
        }
        auto lastInst = preEnd->GetLastInst();
        ASSERT(lastInst != nullptr);
        if (lastInst->IsReturn()) {
            ASSERT(GetGraph()->GetRuntime()->IsInstanceConstructor(GetMethod()));
            lastInst->SetFlag(inst_flags::MEM_BARRIER);
            COMPILER_LOG(INFO, IR_BUILDER) << "Set memory barrier flag to " << *lastInst;
        }
    }
}

bool IrBuilder::CheckMethodLimitations(const BytecodeInstructions &instructions, size_t vregsCount)
{
    // NOTE(a.popov) Optimize catch-phi's memory consumption and get rid of this limitation
    static constexpr auto TRY_BLOCKS_LIMIT = 128U;

    size_t bytecodeSizeLimit = g_options.GetCompilerMaxBytecodeSize();

    // The option CompilerInlineFullIntrinsics increases the size of the code several times.
    // So the limit for this option is reduced
    if (g_options.IsCompilerInlineFullIntrinsics()) {
        ASSERT(GetGraph()->IsDynamicMethod());
        bytecodeSizeLimit >>= 2U;
    }

    if (g_options.GetCompilerMaxVregsNum() > VirtualRegister::MAX_NUM_VIRT_REGS) {
        COMPILER_LOG(INFO, IR_BUILDER) << "Big limit for virtual registers from program options: "
                                       << g_options.GetCompilerMaxVregsNum()
                                       << " max value:" << VirtualRegister::MAX_NUM_VIRT_REGS;
        ASSERT(g_options.GetCompilerMaxVregsNum() <= VirtualRegister::MAX_NUM_VIRT_REGS);
        return false;
    }

    if (instructions.GetSize() > bytecodeSizeLimit) {
        COMPILER_LOG(INFO, IR_BUILDER) << "Method is too big: size=" << instructions.GetSize()
                                       << ", limit=" << bytecodeSizeLimit;
        return false;
    }
    if (vregsCount >= g_options.GetCompilerMaxVregsNum()) {
        COMPILER_LOG(INFO, IR_BUILDER) << "Method has too many virtual registers: " << vregsCount
                                       << ", limit=" << g_options.GetCompilerMaxVregsNum();
        return false;
    }

    auto pandaFile = static_cast<panda_file::File *>(GetGraph()->GetRuntime()->GetBinaryFileForMethod(GetMethod()));
    ASSERT(pandaFile != nullptr);
    panda_file::MethodDataAccessor mda(*pandaFile,
                                       panda_file::File::EntityId(GetGraph()->GetRuntime()->GetMethodId(GetMethod())));
    panda_file::CodeDataAccessor cda(*pandaFile, mda.GetCodeId().value());
    if (cda.GetTriesSize() > TRY_BLOCKS_LIMIT) {
        COMPILER_LOG(INFO, IR_BUILDER) << "Method has too many try blocks: " << cda.GetTriesSize()
                                       << ", limit=" << TRY_BLOCKS_LIMIT;
        return false;
    }
    return true;
}

bool IrBuilder::CreateSaveStateForLoopBlocks(InstBuilder &instBuilder, BasicBlock *bb)
{
    if (GetGraph()->GetEndBlock() != bb && !GetGraph()->IsOsrMode() &&
        (bb->IsLoopPostExit() || bb->IsLoopPreHeader())) {
        // Prepend SaveState as a first instruction in the loop pre header and post exit
        // Set NO_DCE flag, so Cleanup pass won't delete yet unused instruction
        auto *ss = instBuilder.CreateSaveState(Opcode::SaveState, bb->GetGuestPc());
        bb->AppendInst(ss);
        ss->SetFlag(inst_flags::NO_DCE);
    }

    if (bb->IsLoopHeader() && !bb->GetLoop()->IsTryCatchLoop()) {
        // Prepend SaveSateOSR as a first instruction in the loop header
        // NOTE (a.popov) Support osr-entry for loops with catch-block back-edge
        if (GetGraph()->IsOsrMode()) {
            auto backedges = bb->GetLoop()->GetBackEdges();
            auto isCatch = [](BasicBlock *basicBlock) { return basicBlock->IsCatch(); };
            bool hasCatchBackedge = std::find_if(backedges.begin(), backedges.end(), isCatch) != backedges.end();
            if (hasCatchBackedge) {
                COMPILER_LOG(WARNING, IR_BUILDER)
                    << "Osr-entry for loops with catch-handler as back-edge is not supported";
                return false;
            }
            bb->SetOsrEntry(true);
            auto ss = instBuilder.CreateSaveStateOsr(bb);
            bb->AppendInst(ss);
            COMPILER_LOG(DEBUG, IR_BUILDER) << "create save state OSR: " << *ss;
#ifndef NDEBUG
            instBuilder.TryInsertSafepoint(bb, true);
#endif
        }

        if (g_options.IsCompilerUseSafepoint()) {
            auto sp = instBuilder.CreateSafePoint(bb);
            bb->AppendInst(sp);
            COMPILER_LOG(DEBUG, IR_BUILDER) << "create safepoint: " << *sp;
        }
    }

    return true;
}

bool IrBuilder::BuildBasicBlock(InstBuilder &instBuilder, BasicBlock *bb, const uint8_t *instructionsBuf)
{
    instBuilder.SetCurrentBlock(bb);
    instBuilder.UpdateDefs();
    if (GetGraph()->IsDynamicMethod() && !GetGraph()->IsBytecodeOptimizer() &&
        bb == GetGraph()->GetStartBlock()->GetSuccessor(0)) {
        instBuilder.InitEnv(bb);
    }

    // OSR needs additional design for try-catch processing.
    if ((bb->IsTryBegin() || bb->IsTryEnd()) && GetGraph()->IsOsrMode()) {
        return false;
    }

    if (!CreateSaveStateForLoopBlocks(instBuilder, bb)) {
        return false;
    }

    ASSERT(bb->GetGuestPc() != INVALID_PC);
    // If block is not in the `blocks_` vector, it's auxiliary block without instructions
    if (bb == blocks_[bb->GetGuestPc()]) {
        if (bb->IsLoopPreHeader() && !GetGraph()->IsOsrMode()) {
            return BuildInstructionsForBB<true>(instBuilder, bb, instructionsBuf);
        }
        return BuildInstructionsForBB<false>(instBuilder, bb, instructionsBuf);
    }
    if (bb->IsLoopPreHeader() && !GetGraph()->IsOsrMode()) {
        auto ss = instBuilder.CreateSaveStateDeoptimize(bb->GetGuestPc());
        bb->AppendInst(ss);
    }
    COMPILER_LOG(DEBUG, IR_BUILDER) << "Auxiliary block, skipping";
    return true;
}

template <bool NEED_SS_DEOPT>
bool IrBuilder::AddInstructionToBB(InstBuilder &instBuilder, BasicBlock *bb, BytecodeInstruction &inst,
                                   [[maybe_unused]] size_t pc, [[maybe_unused]] bool *ssDeoptWasBuilded)
{
    // Copy current defs for assigning them to catch-phi if current inst is throwable
    ASSERT(instBuilder.GetCurrentDefs().size() == instDefs_.size());
    std::copy(instBuilder.GetCurrentDefs().begin(), instBuilder.GetCurrentDefs().end(), instDefs_.begin());
    auto currentBb = instBuilder.GetCurrentBlock();
    auto currentLastInst = currentBb->GetLastInst();
    auto bbCount = GetGraph()->GetVectorBlocks().size();
    if constexpr (NEED_SS_DEOPT) {
        if (inst.IsJump()) {
            auto ss = instBuilder.CreateSaveStateDeoptimize(pc);
            bb->AppendInst(ss);
            *ssDeoptWasBuilded = true;
        }
    }
    instBuilder.BuildInstruction(&inst);
    if (instBuilder.IsFailed()) {
        COMPILER_LOG(WARNING, IR_BUILDER) << "Unsupported instruction";
        return false;
    }
    if (inst.CanThrow()) {
        // One PBC instruction can be expanded to the group of IR's instructions, find first built instruction
        // in this group, and then mark all instructions as throwable; All instructions should be marked, since
        // some of them can be deleted during optimizations, unnecessary catch-phi moves will be resolved before
        // Register Allocator
        auto throwableInst = (currentLastInst == nullptr) ? currentBb->GetFirstInst() : currentLastInst->GetNext();
        ProcessThrowableInstructions(instBuilder, throwableInst);

        auto &vb = GetGraph()->GetVectorBlocks();
        for (size_t i = bbCount; i < vb.size(); i++) {
            ProcessThrowableInstructions(instBuilder, vb[i]->GetFirstInst());
        }
    }
    return true;
}

template <bool NEED_SS_DEOPT>
bool IrBuilder::BuildInstructionsForBB(InstBuilder &instBuilder, BasicBlock *bb, const uint8_t *instructionsBuf)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    BytecodeInstructions instructions(instructionsBuf + bb->GetGuestPc(), std::numeric_limits<int>::max());
    bool ssDeoptWasBuilded = false;
#ifndef NDEBUG
    instBuilder.TryInsertSafepoint(nullptr, true);
#endif
    for (auto inst : instructions) {
#ifndef NDEBUG
        instBuilder.TryInsertSafepoint();
#endif
        auto pc = instBuilder.GetPc(inst.GetAddress());
        // Break if current pc is pc of some basic block, that means that it is the end of the current block.
        if (pc != bb->GetGuestPc() && GetBlockForPc(pc) != nullptr) {
            break;
        }
        COMPILER_LOG(DEBUG, IR_BUILDER) << "[PBC] " << inst << "  # "
                                        << reinterpret_cast<void *>(inst.GetAddress() - instructionsBuf);
        if (!AddInstructionToBB<NEED_SS_DEOPT>(instBuilder, bb, inst, pc, &ssDeoptWasBuilded)) {
            return false;
        }
        // Break if we meet terminator instruction. If instruction in the middle of basic block we don't create
        // further dead instructions.
        if (inst.IsTerminator()) {
            break;
        }
    }
    if (NEED_SS_DEOPT && !ssDeoptWasBuilded) {
        ASSERT(bb->GetSuccsBlocks().size() == 1);
        auto pc = bb->GetSuccsBlocks()[0]->GetGuestPc();
        auto ss = instBuilder.CreateSaveStateDeoptimize(pc);
        bb->AppendInst(ss);
    }
    return true;
}

void IrBuilder::ProcessThrowableInstructions(InstBuilder &instBuilder, Inst *throwableInst)
{
    for (; throwableInst != nullptr; throwableInst = throwableInst->GetNext()) {
        if (throwableInst->IsSaveState()) {
            continue;
        }
        COMPILER_LOG(DEBUG, IR_BUILDER) << "Throwable inst, Id = " << throwableInst->GetId();
        catchHandlers_.clear();
        EnumerateTryBlocksCoveredPc(throwableInst->GetPc(), [this](const TryCodeBlock &tryBlock) {
            auto tbb = tryBlock.beginBb;
            tbb->EnumerateCatchHandlers([this](BasicBlock *catchHandler, [[maybe_unused]] size_t typeId) {
                catchHandlers_.insert(catchHandler);
                return true;
            });
        });
        if (!catchHandlers_.empty()) {
            instBuilder.AddCatchPhiInputs(catchHandlers_, instDefs_, throwableInst);
        }
    }
}

static inline bool InstNotJump(BytecodeInstruction *inst)
{
    return inst->GetAddress() != nullptr && InstBuilder::GetInstructionJumpOffset(inst) == INVALID_OFFSET &&
           !inst->HasFlag(BytecodeInstruction::RETURN);
}

void IrBuilder::BuildBasicBlocks(const BytecodeInstructions &instructions)
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
            auto targetPc = static_cast<uint64_t>(static_cast<int64_t>(pc) + offset);
            CreateBlock(targetPc);
            if (inst.HasFlag(BytecodeInstruction::CONDITIONAL)) {
                fallthrough = true;
            }
        }
    }
    CreateTryCatchBoundariesBlocks();
    GetGraph()->CreateStartBlock();
    GetGraph()->CreateEndBlock(instructions.GetSize());
    ConnectBasicBlocks(instructions);
    ResolveTryCatchBlocks();
    COMPILER_LOG(DEBUG, IR_BUILDER) << "Created " << GetGraph()->GetVectorBlocks().size() << " basic blocks";
}

template <class Callback>
void IrBuilder::EnumerateTryBlocksCoveredPc(uint32_t pc, const Callback &callback)
{
    for (const auto &[begin_pc, try_block] : tryBlocks_) {
        if (begin_pc <= pc && pc < try_block.boundaries.endPc) {
            callback(try_block);
        }
    }
}

/// Return `TryCodeBlock` and flag if was created a new one
IrBuilder::TryCodeBlock *IrBuilder::InsertTryBlockInfo(const Boundaries &tryBoundaries)
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

void IrBuilder::CreateTryCatchBoundariesBlocks()
{
    auto pandaFile = static_cast<panda_file::File *>(GetGraph()->GetRuntime()->GetBinaryFileForMethod(GetMethod()));
    ASSERT(pandaFile != nullptr);
    panda_file::MethodDataAccessor mda(*pandaFile,
                                       panda_file::File::EntityId(GetGraph()->GetRuntime()->GetMethodId(GetMethod())));
    panda_file::CodeDataAccessor cda(*pandaFile, mda.GetCodeId().value());

    cda.EnumerateTryBlocks([this](panda_file::CodeDataAccessor::TryBlock &tryBlock) {
        auto startPc = tryBlock.GetStartPc();
        auto endPc = startPc + tryBlock.GetLength();
        auto tryInfo = InsertTryBlockInfo({startPc, endPc});
        tryBlock.EnumerateCatchBlocks([this, tryInfo](panda_file::CodeDataAccessor::CatchBlock &catchBlock) {
            auto pc = catchBlock.GetHandlerPc();
            catchesPc_.insert(pc);
            auto typeIdx = catchBlock.GetTypeIdx();
            auto typeId = typeIdx == panda_file::INVALID_INDEX
                              ? 0
                              : GetGraph()->GetRuntime()->ResolveTypeIndex(GetMethod(), typeIdx);
            tryInfo->catches->emplace_back(CatchCodeBlock {pc, typeId});
            return true;
        });

        return true;
    });

    COMPILER_LOG(INFO, IR_BUILDER) << "There are: " << tryBlocks_.size() << " try-blocks in the method";
    COMPILER_LOG(INFO, IR_BUILDER) << "There are: " << catchesPc_.size() << " catch-handlers in the method";

    for (const auto &[pc, try_block] : tryBlocks_) {
        CreateBlock(pc);
        CreateBlock(try_block.boundaries.endPc);
    }
    for (auto pc : catchesPc_) {
        CreateBlock(pc);
    }
}

void IrBuilder::ConnectBasicBlocks(const BytecodeInstructions &instructions)
{
    BlocksConnectorInfo info;
    BasicBlock *currBb = blocks_[0];
    GetGraph()->GetStartBlock()->AddSucc(currBb);
    for (auto inst : instructions) {
        auto pc = instructions.GetPc(inst);
        auto targetBlock = blocks_[pc];
        TrackTryBoundaries(pc, inst, targetBlock != nullptr ? targetBlock : currBb, info);
        if (info.fallthrough) {
            ASSERT(targetBlock != nullptr);
            // May be the second edge between same blocks
            currBb->AddSucc(targetBlock, true);
            info.fallthrough = false;
            currBb = targetBlock;
        } else if (targetBlock != nullptr) {
            if (catchesPc_.count(pc) == 0 && InstNotJump(&info.prevInst) && !info.deadInstructions) {
                currBb->AddSucc(targetBlock);
            }
            currBb = targetBlock;
            info.deadInstructions = false;
        } else if (info.deadInstructions) {
            // We are processing dead instructions now, skipping them until we meet the next block.
            continue;
        }
        if (auto jmpTargetBlock = GetBlockToJump(&inst, pc); jmpTargetBlock != nullptr) {
            currBb->AddSucc(jmpTargetBlock);
            // In case of unconditional branch, we reset curr_bb, so if next instruction won't start new block, then
            // we'll skip further dead instructions.
            info.fallthrough = inst.HasFlag(BytecodeInstruction::CONDITIONAL);
            if (!info.fallthrough) {
                info.deadInstructions = true;
            }
        }
        info.prevInst = inst;
    }

    // Erase end block if it wasn't connected, should be infinite loop in the graph
    if (GetGraph()->GetEndBlock()->GetPredsBlocks().empty()) {
        GetGraph()->EraseBlock(GetGraph()->GetEndBlock());
        GetGraph()->SetEndBlock(nullptr);
        COMPILER_LOG(INFO, IR_BUILDER) << "Builded graph without end block";
    }
}

void IrBuilder::TrackTryBoundaries(size_t pc, const BytecodeInstruction &inst, BasicBlock *targetBb,
                                   BlocksConnectorInfo &info)
{
    openedTryBlocks_.remove_if([pc](TryCodeBlock *tryBlock) { return tryBlock->boundaries.endPc == pc; });

    if (tryBlocks_.count(pc) > 0) {
        auto range = tryBlocks_.equal_range(pc);
        for (auto it = range.first; it != range.second; ++it) {
            auto &tryBlock = it->second;
            if (tryBlock.boundaries.endPc > pc) {
                openedTryBlocks_.push_back(&tryBlock);
                auto allocator = GetGraph()->GetLocalAllocator();
                tryBlock.basicBlocks = allocator->New<ArenaVector<BasicBlock *>>(allocator->Adapter());
                tryBlock.throwBlocks = allocator->New<ArenaSet<BasicBlock *>>(allocator->Adapter());
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

    if (inst.CanThrow()) {
        for (auto &tryBlock : openedTryBlocks_) {
            tryBlock->containsThrowableInst = true;
        }
    }
    if (GetGraph()->IsBytecodeOptimizer() && !g_options.IsCompilerNonOptimizing()) {
        if (!info.deadInstructions && inst.IsThrow(BytecodeInstruction::Exceptions::X_THROW) &&
            !openedTryBlocks_.empty()) {
            auto &tryBlock = *openedTryBlocks_.rbegin();
            tryBlock->throwBlocks->insert(targetBb);
        }
    }
}

BasicBlock *IrBuilder::GetBlockToJump(BytecodeInstruction *inst, size_t pc)
{
    if (inst->HasFlag(BytecodeInstruction::RETURN) || inst->IsThrow(BytecodeInstruction::Exceptions::X_THROW)) {
        return GetGraph()->GetEndBlock();
    }

    if (auto offset = InstBuilder::GetInstructionJumpOffset(inst); offset != INVALID_OFFSET) {
        ASSERT(blocks_[static_cast<uint64_t>(static_cast<int64_t>(pc) + offset)] != nullptr);
        return blocks_[static_cast<uint64_t>(static_cast<int64_t>(pc) + offset)];
    }
    return nullptr;
}

/**
 * Mark blocks which were connected to the graph.
 * Catch-handlers will not be marked, since they have not been connected yet.
 */
static void MarkNormalControlFlow(BasicBlock *block, Marker marker)
{
    block->SetMarker(marker);
    for (auto succ : block->GetSuccsBlocks()) {
        if (!succ->IsMarked(marker)) {
            MarkNormalControlFlow(succ, marker);
        }
    }
}

void IrBuilder::MarkTryCatchBlocks(Marker marker)
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

    // Nested try-blocks can be removed, but referring to them basic blocks can be placed in the external
    // try-blocks. So `try` marks are added after removing unreachable blocks
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
void IrBuilder::ResolveTryCatchBlocks()
{
    auto markerHolder = MarkerHolder(GetGraph());
    auto marker = markerHolder.GetMarker();
    MarkNormalControlFlow(GetGraph()->GetStartBlock(), marker);
    ConnectTryCatchBlocks();
    GetGraph()->RemoveUnreachableBlocks();
    MarkTryCatchBlocks(marker);
}

void IrBuilder::ConnectTryCatchBlocks()
{
    ArenaMap<uint32_t, BasicBlock *> catchBlocks(GetGraph()->GetLocalAllocator()->Adapter());
    // Firstly create catch_begin blocks, as they should precede try_begin blocks
    for (auto pc : catchesPc_) {
        auto catchBegin = GetGraph()->CreateEmptyBlock();
        ASSERT(catchBegin != nullptr);
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

void IrBuilder::ConnectTryCodeBlock(const TryCodeBlock &tryBlock, const ArenaMap<uint32_t, BasicBlock *> &catchBlocks)
{
    auto tryBegin = tryBlock.beginBb;
    ASSERT(tryBegin != nullptr);
    auto tryEnd = tryBlock.endBb;
    ASSERT(tryEnd != nullptr);
    // Create auxiliary `Try` instruction
    auto tryInst = GetGraph()->CreateInstTry(tryEnd);
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
 * As a result all `catch` basic blocks will be eliminated together with outer's `try_end`, since it was inserted
 * just after `catch`
 */
void IrBuilder::RestoreTryEnd(const TryCodeBlock &tryBlock)
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

bool IrBuilderInliningAnalysis::RunImpl()
{
    auto pandaFile = static_cast<panda_file::File *>(GetGraph()->GetRuntime()->GetBinaryFileForMethod(GetMethod()));
    ASSERT(pandaFile != nullptr);
    panda_file::MethodDataAccessor mda(*pandaFile,
                                       panda_file::File::EntityId(GetGraph()->GetRuntime()->GetMethodId(GetMethod())));
    auto codeId = mda.GetCodeId();
    if (!codeId.has_value()) {
        return false;
    }
    panda_file::CodeDataAccessor cda(*pandaFile, codeId.value());

    // NOTE(msherstennikov): Support inlining methods with try/catch
    if (cda.GetTriesSize() != 0) {
        return false;
    }

    BytecodeInstructions instructions(GetGraph()->GetRuntime()->GetMethodCode(GetMethod()),
                                      GetGraph()->GetRuntime()->GetMethodCodeSize(GetMethod()));

    for (auto inst : instructions) {
        if (!IsSuitableForInline(&inst)) {
            return false;
        }
    }
    return true;
}

uint32_t IrBuilder::FindCatchBlockInPandaFile(Class *cls, uint32_t pc) const
{
    if (cls == nullptr) {
        return panda_file::INVALID_OFFSET;
    }
    auto pandaFile = static_cast<panda_file::File *>(GetGraph()->GetRuntime()->GetBinaryFileForMethod(GetMethod()));
    auto *rta = GetGraph()->GetRuntime();
    ASSERT(pandaFile != nullptr);
    panda_file::MethodDataAccessor mda(*(pandaFile),
                                       panda_file::File::EntityId(GetGraph()->GetRuntime()->GetMethodId(GetMethod())));
    panda_file::CodeDataAccessor cda(*(pandaFile), mda.GetCodeId().value());

    auto method = GetMethod();

    auto *descriptorStdCoreObject = reinterpret_cast<const uint8_t *>("Lstd/core/Object;");
    ark::panda_file::File::EntityId stdCoreObjectId = pandaFile->GetClassId(descriptorStdCoreObject);
    auto *stdCoreObject = rta->ResolveType(method, stdCoreObjectId.GetOffset());

    uint32_t pcOffset = panda_file::INVALID_OFFSET;

    cda.EnumerateTryBlocks(
        [&pcOffset, cls, pc, rta, method, stdCoreObject](panda_file::CodeDataAccessor::TryBlock &tryBlock) {
            if ((tryBlock.GetStartPc() > pc) || ((tryBlock.GetStartPc() + tryBlock.GetLength()) <= pc)) {
                return pcOffset == panda_file::INVALID_OFFSET;
            }
            tryBlock.EnumerateCatchBlocks(
                [&pcOffset, &cls, &rta, &method, &stdCoreObject](panda_file::CodeDataAccessor::CatchBlock &catchBlock) {
                    auto typeIdx = catchBlock.GetTypeIdx();
                    if (typeIdx == panda_file::INVALID_INDEX) {
                        return true;
                    }

                    auto typeId = rta->ResolveTypeIndex(method, typeIdx);
                    auto *handlerClass = rta->ResolveType(method, typeId);

                    if (handlerClass == stdCoreObject) {
                        pcOffset = catchBlock.GetHandlerPc();
                        return false;
                    }

                    // bytecode_optimizer should have class linker to determine subclass
                    // then the following condition could be replaced by cls->IsSubClassOf(handlerClass)
                    if (cls == handlerClass) {
                        pcOffset = catchBlock.GetHandlerPc();
                        return false;
                    }
                    return true;
                });
            return false;
        });
    return pcOffset;
}

RuntimeInterface::ClassPtr IrBuilder::FindExceptionClass(BasicBlock *throwBlock, int32_t *throwPc)
{
    if (throwBlock->GetGraph() != GetGraph()) {
        return nullptr;
    }
    if (throwBlock->GetSuccsBlocks().size() > 1 || throwBlock->GetSuccessor(0)->IsTryEnd()) {
        return nullptr;
    }
    auto thr0w = throwBlock->GetLastInst();
    if (thr0w->GetOpcode() != Opcode::Throw) {
        return nullptr;
    }
    *throwPc = thr0w->GetPc();
    auto throwInst = thr0w->CastToThrow();
    auto newObj = throwInst->GetInput(0).GetInst();
    RuntimeInterface::ClassPtr cls = nullptr;
    if (newObj->GetOpcode() != Opcode::NewObject) {
        return nullptr;
    }
    auto initClass = newObj->GetInput(0).GetInst();
    if (initClass->GetOpcode() == Opcode::LoadAndInitClass) {
        cls = initClass->CastToLoadAndInitClass()->GetClass();
    } else {
        ASSERT(initClass->GetOpcode() == Opcode::LoadImmediate);
        cls = initClass->CastToLoadImmediate()->GetClass();
    }
    return cls;
}

BasicBlock *IrBuilder::FindCatchBegin(BasicBlock *bb)
{
    if (bb->IsCatchBegin()) {
        return bb;
    }
    for (auto pred : bb->GetPredsBlocks()) {
        if (pred->IsCatch() || pred->IsTryBegin()) {
            return FindCatchBegin(pred);
        }
    }
    UNREACHABLE();
    return nullptr;
}

bool IrBuilder::FindAppropriateCatchBlock(const TryCodeBlock &tryBlock, BasicBlock *throwBlock, uint32_t catchPc)
{
    bool result = false;
    if (catchPc == panda_file::INVALID_OFFSET) {
        return result;
    }
    for (auto catchBlock : *tryBlock.catches) {
        if (catchBlock.pc == catchPc) {
            auto cBb = GetBlockForPc(catchPc);
            ASSERT(cBb != nullptr);
            cBb = FindCatchBegin(cBb);
            ASSERT(cBb != nullptr && cBb->IsCatchBegin() && cBb->GetGuestPc() == catchPc);
            auto endBlock = GetGraph()->GetEndBlock();
            endBlock->RemovePred(throwBlock);
            throwBlock->RemoveSucc(endBlock);
            COMPILER_LOG(DEBUG, IR_BUILDER)
                << "throwBlock " << throwBlock->GetId() << " is disconnected with endBlock " << endBlock->GetId();
            throwBlock->AddSucc(cBb);
            COMPILER_LOG(DEBUG, IR_BUILDER)
                << "throwBlock " << throwBlock->GetId() << " is connected with catchBeginBlock " << cBb->GetId();
            [[maybe_unused]] auto res = GetGraph()->AppendThrowBlock(throwBlock);
            ASSERT(res);
            GetGraph()->SetThrowApplied();
            result = true;
            break;
        }
    }
    return result;
}

void IrBuilder::ConnectThrowBlock(BasicBlock *throwBlock, const TryCodeBlock &tryBlock)
{
    if (throwBlock->GetGraph() != GetGraph()) {
        return;
    }
    auto endBlock = GetGraph()->GetEndBlock();
    if (throwBlock->GetSuccsBlocks().size() != 1 || throwBlock->GetSuccessor(0) != endBlock) {
        return;
    }
    int32_t throwPc;
    auto cls = FindExceptionClass(throwBlock, &throwPc);
    if (cls != nullptr &&
        FindAppropriateCatchBlock(tryBlock, throwBlock,
                                  FindCatchBlockInPandaFile(static_cast<ark::Class *>(cls), throwPc))) {
        return;
    }
    auto tryEnd = tryBlock.endBb;
    ASSERT(tryEnd != nullptr);
    if (tryEnd->GetGraph() != GetGraph()) {
        return;
    }
    BasicBlock *catchBegin = nullptr;
    if (tryEnd->IsCatch()) {
        catchBegin = FindCatchBegin(tryEnd);
    } else {
        for (auto succ : tryEnd->GetSuccsBlocks()) {
            if (succ->IsCatchBegin()) {
                catchBegin = succ;
                break;
            }
        }
    }
    if (catchBegin != nullptr) {
        throwBlock->AddSucc(catchBegin);
        COMPILER_LOG(DEBUG, IR_BUILDER) << "throwBlock " << throwBlock->GetId() << " is connected with catchBegin "
                                        << catchBegin->GetId();
        [[maybe_unused]] auto result = GetGraph()->AppendThrowBlock(throwBlock);
        ASSERT(result);
        GetGraph()->SetThrowApplied();
    }
}

void IrBuilder::ConnectThrowBlocks()
{
    for (auto it : tryBlocks_) {
        const auto &tryBlock = it.second;
        if (tryBlock.containsThrowableInst) {
            for (auto throwBlock : *tryBlock.throwBlocks) {
                ConnectThrowBlock(throwBlock, tryBlock);
            }
        }
    }
}
}  // namespace ark::compiler
