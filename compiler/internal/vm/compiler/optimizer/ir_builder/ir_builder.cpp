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

#include "code_data_accessor-inl.h"
#include "method_data_accessor-inl.h"
#include "ir_builder.h"

namespace panda::compiler {
bool IrBuilder::RunImpl()
{
    COMPILER_LOG(INFO, IR_BUILDER) << "Start building ir for method: "
                                   << GetGraph()->GetRuntime()->GetClassNameFromMethod(GetMethod()) << "."
                                   << GetGraph()->GetRuntime()->GetMethodName(GetMethod())
                                   << "(args=" << GetGraph()->GetRuntime()->GetMethodTotalArgumentsCount(GetMethod())
                                   << ", regs=" << GetGraph()->GetRuntime()->GetMethodRegistersCount(GetMethod())
                                   << ")";

    auto instructions_buf = GetGraph()->GetRuntime()->GetMethodCode(GetMethod());
    BytecodeInstructions pbc_instructions(instructions_buf, GetGraph()->GetRuntime()->GetMethodCodeSize(GetMethod()));
    size_t vregs_count = GetGraph()->GetRuntime()->GetMethodRegistersCount(GetMethod()) +
                         GetGraph()->GetRuntime()->GetMethodTotalArgumentsCount(GetMethod()) + 1;
    if (!CheckMethodLimitations(pbc_instructions, vregs_count)) {
        return false;
    }
    GetGraph()->SetVRegsCount(vregs_count);
    BuildBasicBlocks(pbc_instructions);
    GetGraph()->RunPass<DominatorsTree>();
    GetGraph()->RunPass<LoopAnalyzer>();

    InstBuilder inst_builder(GetGraph(), GetMethod());
    inst_builder.Prepare();
    inst_defs_.resize(vregs_count);
    COMPILER_LOG(INFO, IR_BUILDER) << "Start instructions building...";
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (!BuildBasicBlock(bb, &inst_builder, instructions_buf)) {
            return false;
        }
    }
    GetGraph()->RunPass<DominatorsTree>();
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->RunPass<LoopAnalyzer>();
    inst_builder.FixInstructions();

    if (options.IsCompilerPrintStats() || options.WasSetCompilerDumpStatsCsv()) {
        uint64_t pbc_inst_num = 0;
        for ([[maybe_unused]] auto i : pbc_instructions) {
            pbc_inst_num++;
        }
        GetGraph()->GetPassManager()->GetStatistics()->AddPbcInstNum(pbc_inst_num);
    }
    COMPILER_LOG(INFO, IR_BUILDER) << "IR successfully built: " << GetGraph()->GetVectorBlocks().size()
                                   << " basic blocks, " << GetGraph()->GetCurrentInstructionId() << " instructions";
    return true;
}

bool IrBuilder::CheckMethodLimitations(const BytecodeInstructions &instructions, size_t vregs_count)
{
    // TODO(a.popov) Optimize catch-phi's memory consumption and get rid of this limitation
    static constexpr auto TRY_BLOCKS_LIMIT = 128U;

    size_t bytecode_size_limit = options.GetCompilerMaxBytecodeSize();

    // The option CompilerInlineFullIntrinsics increases the size of the code several times.
    // So the limit for this option is reduced
    if (options.IsCompilerInlineFullIntrinsics()) {
        ASSERT(GetGraph()->IsDynamicMethod());
        bytecode_size_limit >>= 2U;
    }

    if (instructions.GetSize() > bytecode_size_limit) {
        COMPILER_LOG(INFO, IR_BUILDER) << "Method is too big: size=" << instructions.GetSize()
                                       << ", limit=" << bytecode_size_limit;
        return false;
    }
    if (vregs_count >= options.GetCompilerMaxVregsNum()) {
        COMPILER_LOG(INFO, IR_BUILDER) << "Method has too many virtual registers: " << vregs_count
                                       << ", limit=" << options.GetCompilerMaxVregsNum();
        return false;
    }

    auto panda_file = static_cast<panda_file::File *>(GetGraph()->GetRuntime()->GetBinaryFileForMethod(GetMethod()));
    CHECK_NOT_NULL(panda_file);
    panda_file::MethodDataAccessor mda(*panda_file,
                                       panda_file::File::EntityId(GetGraph()->GetRuntime()->GetMethodId(GetMethod())));
    panda_file::CodeDataAccessor cda(*panda_file, mda.GetCodeId().value());
    if (cda.GetTriesSize() > TRY_BLOCKS_LIMIT) {
        COMPILER_LOG(INFO, IR_BUILDER) << "Method has too many try blocks: " << cda.GetTriesSize()
                                       << ", limit=" << TRY_BLOCKS_LIMIT;
        return false;
    }
    return true;
}

bool IrBuilder::BuildBasicBlock(BasicBlock *bb, InstBuilder *inst_builder, const uint8_t *instructions_buf)
{
    inst_builder->SetCurrentBlock(bb);
    inst_builder->UpdateDefs();
    ASSERT(bb->GetGuestPc() != INVALID_PC);
    // If block is not in the `blocks_` vector, it's auxiliary block without instructions
    if (bb == blocks_[bb->GetGuestPc()]) {
        return BuildInstructionsForBB(bb, inst_builder, instructions_buf);
    }
    COMPILER_LOG(DEBUG, IR_BUILDER) << "Auxiliary block, skipping";
    return true;
}

bool IrBuilder::BuildInstructionsForBB(BasicBlock *bb, InstBuilder *inst_builder, const uint8_t *instructions_buf)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    BytecodeInstructions instructions(instructions_buf + bb->GetGuestPc(), std::numeric_limits<int>::max());
    for (auto inst : instructions) {
        auto pc = inst_builder->GetPc(inst.GetAddress());
        // Break if current pc is pc of some basic block, that means that it is the end of the current block.
        if (pc != bb->GetGuestPc() && GetBlockForPc(pc) != nullptr) {
            break;
        }
        COMPILER_LOG(DEBUG, IR_BUILDER) << "PBC instruction: " << inst << "  # "
                                        << reinterpret_cast<void *>(inst.GetAddress() - instructions_buf);
        // Copy current defs for assigning them to catch-phi if current inst is throwable
        ASSERT(inst_builder->GetCurrentDefs().size() == inst_defs_.size());
        std::copy(inst_builder->GetCurrentDefs().begin(), inst_builder->GetCurrentDefs().end(), inst_defs_.begin());
        auto current_last_inst = bb->GetLastInst();
        auto bb_count = GetGraph()->GetVectorBlocks().size();
        inst_builder->BuildInstruction(&inst);
        if (inst_builder->IsFailed()) {
            COMPILER_LOG(WARNING, IR_BUILDER) << "Unsupported instruction";
            return false;
        }

        // One PBC instruction can be expanded to the group of IR's instructions, find first built instruction in
        // this group, and then mark all instructions as throwable; All instructions should be marked, since some of
        // them can be deleted during optimizations, unnecessary catch-phi moves will be resolved before Register
        // Allocator
        auto throwable_inst = (current_last_inst == nullptr) ? bb->GetFirstInst() : current_last_inst->GetNext();
        ProcessThrowableInstructions(inst_builder, throwable_inst);

        auto &vb = GetGraph()->GetVectorBlocks();
        for (size_t i = bb_count; i < vb.size(); i++) {
            ProcessThrowableInstructions(inst_builder, vb[i]->GetFirstInst());
        }

        // Break if we meet terminator instruction. If instruction in the middle of basic block we don't create
        // further dead instructions.
        if (inst.IsTerminator() && !inst.IsSuspend()) {
            break;
        }
    }
    return true;
}

void IrBuilder::ProcessThrowableInstructions(InstBuilder *inst_builder, Inst *throwable_inst)
{
    for (; throwable_inst != nullptr; throwable_inst = throwable_inst->GetNext()) {
        if (throwable_inst->IsSaveState()) {
            continue;
        }
        COMPILER_LOG(DEBUG, IR_BUILDER) << "Throwable inst, Id = " << throwable_inst->GetId();
        catch_handlers_.clear();
        EnumerateTryBlocksCoveredPc(throwable_inst->GetPc(), [this](const TryCodeBlock &try_block) {
            auto tbb = try_block.begin_bb;
            tbb->EnumerateCatchHandlers([this](BasicBlock *catch_handler, [[maybe_unused]] size_t type_id) {
                catch_handlers_.insert(catch_handler);
                return true;
            });
        });
        if (!catch_handlers_.empty()) {
            inst_builder->AddCatchPhiInputs(catch_handlers_, inst_defs_, throwable_inst);
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
            auto target_pc = pc + static_cast<size_t>(offset);
            CreateBlock(target_pc);
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
    for (const auto &[begin_pc, try_block] : try_blocks_) {
        if (begin_pc <= pc && pc < try_block.boundaries.end_pc) {
            callback(try_block);
        }
    }
}

/**
 * Return `TryCodeBlock` and flag if was created a new one
 */
IrBuilder::TryCodeBlock *IrBuilder::InsertTryBlockInfo(const Boundaries &try_boundaries)
{
    auto try_id = static_cast<uint32_t>(try_blocks_.size());
    auto range = try_blocks_.equal_range(try_boundaries.begin_pc);
    for (auto iter = range.first; iter != range.second; ++iter) {
        // use try-block with the same boundaries
        if (try_boundaries.end_pc == iter->second.boundaries.end_pc) {
            return &iter->second;
        }
        // insert in the increasing `end_pc` order
        if (try_boundaries.end_pc > iter->second.boundaries.end_pc) {
            auto it = try_blocks_.emplace_hint(iter, try_boundaries.begin_pc, TryCodeBlock {try_boundaries});
            it->second.Init(GetGraph(), try_id);
            return &it->second;
        }
    }
    auto it = try_blocks_.emplace(try_boundaries.begin_pc, TryCodeBlock {try_boundaries});
    it->second.Init(GetGraph(), try_id);
    return &it->second;
}

void IrBuilder::CreateTryCatchBoundariesBlocks()
{
    auto panda_file = static_cast<panda_file::File *>(GetGraph()->GetRuntime()->GetBinaryFileForMethod(GetMethod()));
    CHECK_NOT_NULL(panda_file);
    panda_file::MethodDataAccessor mda(*panda_file,
                                       panda_file::File::EntityId(GetGraph()->GetRuntime()->GetMethodId(GetMethod())));
    panda_file::CodeDataAccessor cda(*panda_file, mda.GetCodeId().value());

    cda.EnumerateTryBlocks([this](panda_file::CodeDataAccessor::TryBlock &try_block) {
        auto start_pc = try_block.GetStartPc();
        auto end_pc = start_pc + try_block.GetLength();
        auto try_info = InsertTryBlockInfo({start_pc, end_pc});
        try_block.EnumerateCatchBlocks([this, try_info](panda_file::CodeDataAccessor::CatchBlock &catch_block) {
            auto pc = catch_block.GetHandlerPc();
            catches_pc_.insert(pc);
            try_info->catches->emplace_back(CatchCodeBlock {pc, 0u});
            return true;
        });

        return true;
    });

    COMPILER_LOG(INFO, IR_BUILDER) << "There are: " << try_blocks_.size() << " try-blocks in the method";
    COMPILER_LOG(INFO, IR_BUILDER) << "There are: " << catches_pc_.size() << " catch-handlers in the method";

    for (const auto &[pc, try_block] : try_blocks_) {
        CreateBlock(pc);
        CreateBlock(try_block.boundaries.end_pc);
    }
    for (auto pc : catches_pc_) {
        CreateBlock(pc);
    }
}

struct BlocksConnectorInfo {
    bool fallthrough {};
    bool dead_instructions {};
    BytecodeInstruction prev_inst {nullptr};
};

void IrBuilder::ConnectBasicBlocks(const BytecodeInstructions &instructions)
{
    BlocksConnectorInfo info;
    BasicBlock *curr_bb = blocks_[0];
    GetGraph()->GetStartBlock()->AddSucc(curr_bb);
    for (auto inst : instructions) {
        auto pc = instructions.GetPc(inst);
        auto target_block = blocks_[pc];
        TrackTryBoundaries(pc, inst);
        if (info.fallthrough) {
            ASSERT(target_block != nullptr);
            // May be the second edge between same blocks
            curr_bb->AddSucc(target_block, true);
            info.fallthrough = false;
            curr_bb = target_block;
        } else if (target_block != nullptr) {
            if (catches_pc_.count(pc) == 0) {
                if (InstNotJump(&info.prev_inst) && !info.dead_instructions) {
                    curr_bb->AddSucc(target_block);
                }
            }
            curr_bb = target_block;
            info.dead_instructions = false;
        } else if (info.dead_instructions) {
            // We are processing dead instructions now, skipping them until we meet the next block.
            continue;
        }
        if (auto jmp_target_block = GetBlockToJump(&inst, pc); jmp_target_block != nullptr) {
            curr_bb->AddSucc(jmp_target_block);
            // In case of unconditional branch, we reset curr_bb, so if next instruction won't start new block, then
            // we'll skip further dead instructions.
            info.fallthrough = inst.HasFlag(BytecodeInstruction::CONDITIONAL);
            if (!info.fallthrough) {
                info.dead_instructions = true;
            }
        }
        info.prev_inst = inst;
    }

    // Erase end block if it wasn't connected, should be infinite loop in the graph
    if (GetGraph()->GetEndBlock()->GetPredsBlocks().empty()) {
        GetGraph()->EraseBlock(GetGraph()->GetEndBlock());
        GetGraph()->SetEndBlock(nullptr);
        COMPILER_LOG(INFO, IR_BUILDER) << "Builded graph without end block";
    }
}

void IrBuilder::TrackTryBoundaries(size_t pc, const BytecodeInstruction &inst)
{
    opened_try_blocks_.remove_if([pc](TryCodeBlock *try_block) { return try_block->boundaries.end_pc == pc; });

    if (try_blocks_.count(pc) > 0) {
        auto range = try_blocks_.equal_range(pc);
        for (auto it = range.first; it != range.second; ++it) {
            auto &try_block = it->second;
            if (try_block.boundaries.end_pc > pc) {
                opened_try_blocks_.push_back(&try_block);
                auto allocator = GetGraph()->GetLocalAllocator();
                try_block.basic_blocks = allocator->New<ArenaVector<BasicBlock *>>(allocator->Adapter());
            } else {
                // Empty try-block
                ASSERT(try_block.boundaries.end_pc == pc);
            }
        }
    }

    if (opened_try_blocks_.empty()) {
        return;
    }

    if (auto bb = blocks_[pc]; bb != nullptr) {
        for (auto try_block : opened_try_blocks_) {
            try_block->basic_blocks->push_back(bb);
        }
    }

    for (auto &try_block : opened_try_blocks_) {
        try_block->contains_throwable_inst = true;
    }
}

BasicBlock *IrBuilder::GetBlockToJump(BytecodeInstruction *inst, size_t pc)
{
    if ((inst->HasFlag(BytecodeInstruction::RETURN) && !inst->HasFlag(BytecodeInstruction::SUSPEND)) ||
        inst->IsThrow(BytecodeInstruction::Exceptions::X_THROW)) {
        return GetGraph()->GetEndBlock();
    }

#ifdef ENABLE_BYTECODE_OPT
    if (inst->GetOpcode() == BytecodeInstruction::Opcode::RETURNUNDEFINED) {
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

    // Nested try-blocks can be removed, but referring to them basic blocks can be placed in the external try-blocks.
    // So `try` marks are added after removing unreachable blocks
    for (auto it : try_blocks_) {
        const auto &try_block = it.second;
        if (try_block.begin_bb->GetGraph() != try_block.end_bb->GetGraph()) {
            RestoreTryEnd(try_block);
        }
        try_block.begin_bb->SetTryId(try_block.id);
        try_block.end_bb->SetTryId(try_block.id);
        if (try_block.basic_blocks == nullptr) {
            continue;
        }
        for (auto bb : *try_block.basic_blocks) {
            bb->SetTryId(try_block.id);
            bb->SetTry(true);
        }
    }
}

/*
 * Connect catch-blocks to the graph.
 */
void IrBuilder::ResolveTryCatchBlocks()
{
    auto marker_holder = MarkerHolder(GetGraph());
    auto marker = marker_holder.GetMarker();
    MarkNormalControlFlow(GetGraph()->GetStartBlock(), marker);
    ConnectTryCatchBlocks();
    GetGraph()->RemoveUnreachableBlocks();
    MarkTryCatchBlocks(marker);
}

void IrBuilder::ConnectTryCatchBlocks()
{
    ArenaMap<uint32_t, BasicBlock *> catch_blocks(GetGraph()->GetLocalAllocator()->Adapter());
    // Firstly create catch_begin blocks, as they should precede try_begin blocks
    for (auto pc : catches_pc_) {
        auto catch_begin = GetGraph()->CreateEmptyBlock();
        catch_begin->SetGuestPc(pc);
        catch_begin->SetCatch(true);
        catch_begin->SetCatchBegin(true);
        auto first_catch_bb = GetBlockForPc(pc);
        catch_begin->AddSucc(first_catch_bb);
        catch_blocks.emplace(pc, catch_begin);
    }

    // Connect try_begin and catch_begin blocks
    for (auto it : try_blocks_) {
        const auto &try_block = it.second;
        if (try_block.contains_throwable_inst) {
            ConnectTryCodeBlock(try_block, catch_blocks);
        } else if (try_block.basic_blocks != nullptr) {
            try_block.basic_blocks->clear();
        }
    }
}

void IrBuilder::ConnectTryCodeBlock(const TryCodeBlock &try_block, const ArenaMap<uint32_t, BasicBlock *> &catch_blocks)
{
    auto try_begin = try_block.begin_bb;
    ASSERT(try_begin != nullptr);
    auto try_end = try_block.end_bb;
    ASSERT(try_end != nullptr);
    // Create auxiliary `Try` instruction
    auto try_inst = GetGraph()->CreateInstTry();
    try_inst->SetTryEndBlock(try_end);
    try_begin->AppendInst(try_inst);
    // Insert `try_begin` and `try_end`
    auto first_try_bb = GetBlockForPc(try_block.boundaries.begin_pc);
    auto last_try_bb = GetPrevBlockForPc(try_block.boundaries.end_pc);
    first_try_bb->InsertBlockBefore(try_begin);
    last_try_bb->InsertBlockBeforeSucc(try_end, last_try_bb->GetSuccessor(0));
    // Connect catch-handlers
    for (auto catch_block : *try_block.catches) {
        auto catch_begin = catch_blocks.at(catch_block.pc);
        if (!try_begin->HasSucc(catch_begin)) {
            try_begin->AddSucc(catch_begin, true);
            try_end->AddSucc(catch_begin, true);
        }
        try_inst->AppendCatchTypeId(catch_block.type_id, try_begin->GetSuccBlockIndex(catch_begin));
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
void IrBuilder::RestoreTryEnd(const TryCodeBlock &try_block)
{
    ASSERT(try_block.end_bb->GetGraph() == nullptr);
    ASSERT(try_block.end_bb->GetSuccsBlocks().empty());
    ASSERT(try_block.end_bb->GetPredsBlocks().empty());

    GetGraph()->RestoreBlock(try_block.end_bb);
    auto last_try_bb = GetPrevBlockForPc(try_block.boundaries.end_pc);
    last_try_bb->InsertBlockBeforeSucc(try_block.end_bb, last_try_bb->GetSuccessor(0));
    for (auto succ : try_block.begin_bb->GetSuccsBlocks()) {
        if (succ->IsCatchBegin()) {
            try_block.end_bb->AddSucc(succ);
        }
    }
}
}  // namespace panda::compiler
