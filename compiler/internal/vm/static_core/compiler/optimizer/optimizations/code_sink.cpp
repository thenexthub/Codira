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

#include "compiler_logger.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/optimizations/code_sink.h"

namespace ark::compiler {
/**
 * Code sinking optimization attempts to sink operations that is needed only in
 * a particular execution path but are performed on paths that do not need
 * them.
 *
 * [BB 0] ---------\
 *  | v3 = v1 + v2  \
 *  |                \
 * [BB 1]            [BB 2]
 *    return v1         return v3
 *
 * Transforms into
 *
 * [BB 0] ------\
 *  |            \
 * [BB 1]       [BB 2]
 *    return v1    v3 = v1 + v2
 *                 return v3
 *
 * Generally we can move only instructions with no side-effects.
 * We can't move:
 * - Allocations
 * - Control flow instructions
 * - Instructions that can throw an exception
 * - Function calls (they may have side effects and/or throw exceptions as well)
 * - Monitors
 * - Store instructions
 *
 * We do not move:
 * - Loads that may be aliased by following stores in a basic block
 * - Loads if there is any presence of monitors in a function
 * - Instructions into inner loops
 */
bool CodeSink::RunImpl()
{
    isApplied_ = false;
    GetGraph()->RunPass<LoopAnalyzer>();
    // Iteratively sink instructions.  On each iteration an instruction can be
    // sunk to it's basic block dominatee.  Iterate sinking until no changes
    // happens.
    bool changed = true;
    for (int i = 0; changed; ++i) {
        changed = false;
        COMPILER_LOG(DEBUG, CODE_SINK) << "Sinking iteration " << i;
        for (auto block : GetGraph()->GetBlocksRPO()) {
            changed |= ProcessBlock(block);
        }
        isApplied_ |= changed;
    }

    COMPILER_LOG(DEBUG, CODE_SINK) << "Code Sink complete";
    return isApplied_;
}

void CodeSink::InvalidateAnalyses()
{
    if (isApplied_) {
        // Bounds analysis works with instructions in a particular basic block
        // reordering breaks it
        GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    }
}

/**
 * Iterate instructions in reverse order to decrease the number of iterations.
 * If instruction can be sunk, it is erased from it's basic block and inserted
 * into appropriate successor.
 */
bool CodeSink::ProcessBlock(BasicBlock *block)
{
    if (block->IsStartBlock() || block->IsEndBlock()) {
        return false;
    }
    bool madeChange = false;
    bool memBarrier = false;
    InstVector stores(GetGraph()->GetLocalAllocator()->Adapter());
    for (auto inst : block->InstsSafeReverse()) {
        if (inst->IsCatchPhi() || !inst->HasUsers()) {
            continue;
        }
        if (inst->GetOpcode() == Opcode::Monitor || (inst->IsStore() && IsVolatileMemInst(inst)) ||
            inst->GetFlag(compiler::inst_flags::HEAP_INV)) {
            // Ensures that we do not move in or out monitored section
            // Also ensures we do not sink over volatile store
            memBarrier = true;
            continue;
        }
        BasicBlock *candidate = SinkInstruction(inst, &stores, memBarrier);
        if (candidate != nullptr) {
            COMPILER_LOG(DEBUG, CODE_SINK) << "Sunk v" << inst->GetId() << " to BB " << candidate->GetId();
            GetGraph()->GetEventWriter().EventCodeSink(inst->GetId(), inst->GetPc(), block->GetId(),
                                                       candidate->GetId());
            block->EraseInst(inst, true);
            // Insertion in the beginning of the block guaranties we do not
            // enter or exit monitor in candidate block
            candidate->PrependInst(inst);
            madeChange = true;
        }
    }
    return madeChange;
}

static bool InstHasRefInput(Inst *inst)
{
    for (size_t i = 0; i < inst->GetInputsCount(); ++i) {
        if (!DataType::IsTypeNumeric(inst->GetInputType(i))) {
            return true;
        }
    }
    return false;
}

/**
 * Check that an instruction can be sunk.  Then check that dominated blocks are acceptable for sinking.
 * Finally, return the candidate.
 */
BasicBlock *CodeSink::SinkInstruction(Inst *inst, InstVector *stores, bool barriered)
{
    ASSERT(inst->GetOpcode() != Opcode::Phi && inst->GetOpcode() != Opcode::CatchPhi);
    // Save stores to be sure we do not sink a load instruction that may be
    // overwritten by later store instruction
    if (inst->IsStore()) {
        stores->push_back(inst);
        return nullptr;
    }
    // Check that instruction can be sunk
    // Volatile memory operations are barriers
    // We can't move instruction with REFERENCE input throw SaveState(GC can moved or delete the object)
    // For Bytecode Optimizer, it is not allowed to mix LoadStatic and class initialization (for another LoadStatic),
    // so LoadStatic cannot be sunk
    if (inst->IsAllocation() || inst->IsControlFlow() || inst->CanThrow() || inst->IsBarrier() || inst->IsSaveState() ||
        (!GetGraph()->IsBytecodeOptimizer() && InstHasRefInput(inst)) ||
        (GetGraph()->IsBytecodeOptimizer() && (inst->GetOpcode() == Opcode::LoadStatic))) {
        return nullptr;
    }
    if (inst->IsLoad()) {
        // Do not sink over monitors or volatile stores
        if (barriered) {
            return nullptr;
        }
        for (auto store : *stores) {
            if (GetGraph()->CheckInstAlias(inst, store) != AliasType::NO_ALIAS) {
                return nullptr;
            }
        }
    }

    BasicBlock *block = inst->GetBasicBlock();
    auto dominated = block->GetDominatedBlocks();
    for (auto cand : dominated) {
        if (IsAcceptableTarget(inst, cand)) {
            return cand;
        }
    }

    return nullptr;
}

/// Check that candidate dominates all users of inst.
bool CodeSink::IsAcceptableTarget(Inst *inst, BasicBlock *candidate)
{
    ASSERT(inst != nullptr);
    ASSERT(candidate != nullptr);
    ASSERT(inst->GetBasicBlock() != candidate);
    ASSERT(inst->GetBasicBlock()->IsDominate(candidate));

    if (candidate->IsEndBlock() || candidate->IsTryBegin() || candidate->IsTryEnd()) {
        return false;
    }

    Loop *candLoop = candidate->GetLoop();
    // Do not sink into irreducible loops
    if (candLoop->IsIrreducible()) {
        return false;
    }

    BasicBlock *block = inst->GetBasicBlock();
    Loop *loop = block->GetLoop();
    if (candidate->GetPredsBlocks().size() > 1) {
        // Do not sink loads across a critical edge there may be stores in
        // other code paths.
        if (inst->IsLoad()) {
            return false;
        }
        if (loop != candLoop) {
            return false;
        }
    }
    ASSERT_PRINT(loop == candLoop || loop->IsInside(candLoop), "Can sink only into outer loop");

    // Check that all uses are dominated by the candidate
    for (auto &user : inst->GetUsers()) {
        Inst *uinst = user.GetInst();
        auto ublock = uinst->GetBasicBlock();
        // We can't insert instruction before Phi, therefore do not sink into blocks where one of users is a Phi
        if (!candidate->IsDominate(ublock) || (uinst->IsPhi() && ublock == candidate)) {
            return false;
        }
    }
    return true;
}
}  // namespace ark::compiler
