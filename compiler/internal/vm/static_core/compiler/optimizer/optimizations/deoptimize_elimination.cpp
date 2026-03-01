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
#include "deoptimize_elimination.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/ir/analysis.h"

namespace ark::compiler {

bool DeoptimizeElimination::RunImpl()
{
    GetGraph()->RunPass<LoopAnalyzer>();

    VisitGraph();

    ReplaceDeoptimizeIfByUnconditionalDeoptimize();

    if (IsLoopDeleted() && GetGraph()->IsOsrMode()) {
        CleanupGraphSaveStateOSR(GetGraph());
    }
    return IsApplied();
}

void DeoptimizeElimination::ReplaceDeoptimizeIfByUnconditionalDeoptimize()
{
    for (auto &inst : deoptimizeMustThrow_) {
        auto block = inst->GetBasicBlock();
        if (block != nullptr) {
            block->ReplaceInstByDeoptimize(inst);
            SetApplied();
            SetLoopDeleted();
        }
    }
}

void DeoptimizeElimination::VisitDeoptimizeIf(GraphVisitor *v, Inst *inst)
{
    auto input = inst->GetInput(0).GetInst();
    auto block = inst->GetBasicBlock();
    auto graph = block->GetGraph();
    auto visitor = static_cast<DeoptimizeElimination *>(v);
    if (input->IsConst()) {
        if (input->CastToConstant()->GetIntValue() == 0) {
            visitor->RemoveDeoptimizeIf(inst);
        } else {
            visitor->PushNewDeoptimizeIf(inst);
        }
    } else if (input->GetOpcode() == Opcode::IsMustDeoptimize) {
        if (visitor->CanRemoveGuard(input)) {
            visitor->RemoveGuard(input);
        }
    } else {
        for (auto &user : input->GetUsers()) {
            auto userInst = user.GetInst();
            if (userInst != inst && userInst->GetOpcode() == Opcode::DeoptimizeIf &&
                !(graph->IsOsrMode() && block->GetLoop() != userInst->GetBasicBlock()->GetLoop()) &&
                inst->InSameBlockOrDominate(userInst)) {
                ASSERT(inst->IsDominate(userInst));
                visitor->RemoveDeoptimizeIf(userInst);
            }
        }
    }
}

bool DeoptimizeElimination::CanRemoveGuard(Inst *guard)
{
    auto guardBlock = guard->GetBasicBlock();
    auto it = InstSafeIterator<IterationType::INST, IterationDirection::BACKWARD>(*guardBlock, guard);
    for (++it; it != guardBlock->InstsSafeReverse().end(); ++it) {
        auto inst = *it;
        if (inst->IsRuntimeCall()) {
            return false;
        }
        if (inst->GetOpcode() == Opcode::IsMustDeoptimize) {
            return true;
        }
    }
    auto mrk = guardBlock->GetGraph()->NewMarker();
    auto removeMrk = guardBlock->GetGraph()->NewMarker();

    /*
     * Run search recursively from current block to start block.
     * We can remove guard, if guard is met in all ways and there should be no call instructions between current
     * guard and found guards.
     */
    bool canRemove = true;
    for (auto succBlock : guardBlock->GetPredsBlocks()) {
        canRemove &= CanRemoveGuardRec(succBlock, guard, mrk, removeMrk);
        if (!canRemove) {
            break;
        }
    }
    guardBlock->GetGraph()->EraseMarker(mrk);
    guardBlock->GetGraph()->EraseMarker(removeMrk);
    return canRemove;
}

bool DeoptimizeElimination::CanRemoveGuardRec(BasicBlock *block, Inst *guard, const Marker &mrk,
                                              const Marker &removeMrk)
{
    if (block->IsStartBlock()) {
        return false;
    }
    auto blockType = GetBlockType(block);
    if (block->SetMarker(mrk)) {
        return block->IsMarked(removeMrk);
    }
    if (blockType == BlockType::INVALID) {
        for (auto inst : block->InstsSafeReverse()) {
            if (inst->IsRuntimeCall()) {
                PushNewBlockType(block, BlockType::RUNTIME_CALL);
                return false;
            }
            if (inst->GetOpcode() == Opcode::IsMustDeoptimize) {
                [[maybe_unused]] auto result = block->SetMarker(removeMrk);
                ASSERT(!result);
                PushNewBlockType(block, BlockType::GUARD);
                return true;
            }
        }
        PushNewBlockType(block, BlockType::NOTHING);
    } else if (blockType != BlockType::NOTHING) {
        if (blockType == BlockType::GUARD) {
            [[maybe_unused]] auto result = block->SetMarker(removeMrk);
            ASSERT(!result);
            return true;
        }
        return false;
    }
    for (const auto &succBlock : block->GetPredsBlocks()) {
        if (!CanRemoveGuardRec(succBlock, guard, mrk, removeMrk)) {
            return false;
        }
    }
    [[maybe_unused]] auto result = block->SetMarker(removeMrk);
    ASSERT(!result);
    return true;
}

void DeoptimizeElimination::RemoveGuard(Inst *guard)
{
    ASSERT(guard->GetOpcode() == Opcode::IsMustDeoptimize);
    ASSERT(guard->HasSingleUser());

    auto deopt = guard->GetNext();
    ASSERT(deopt->GetOpcode() == Opcode::DeoptimizeIf);
    auto block = guard->GetBasicBlock();
    auto graph = block->GetGraph();
    guard->RemoveInputs();
    block->ReplaceInst(guard, graph->CreateInstNOP());

    COMPILER_LOG(DEBUG, DEOPTIMIZE_ELIM) << "Dublicated Guard " << guard->GetId() << " is deleted";
    graph->GetEventWriter().EventDeoptimizeElimination(GetOpcodeString(guard->GetOpcode()), guard->GetId(),
                                                       guard->GetPc());
    RemoveDeoptimizeIf(deopt);
}

void DeoptimizeElimination::RemoveDeoptimizeIf(Inst *inst)
{
    auto block = inst->GetBasicBlock();
    auto graph = block->GetGraph();
    auto savestate = inst->GetInput(1).GetInst();

    inst->RemoveInputs();
    block->ReplaceInst(inst, graph->CreateInstNOP());

    COMPILER_LOG(DEBUG, DEOPTIMIZE_ELIM) << "Dublicated or redundant DeoptimizeIf " << inst->GetId() << " is deleted";
    graph->GetEventWriter().EventDeoptimizeElimination(GetOpcodeString(inst->GetOpcode()), inst->GetId(),
                                                       inst->GetPc());

    if (savestate->GetUsers().Empty()) {
        savestate->GetBasicBlock()->ReplaceInst(savestate, graph->CreateInstNOP());
        savestate->RemoveInputs();

        COMPILER_LOG(DEBUG, DEOPTIMIZE_ELIM) << "SaveState " << savestate->GetId() << " without users is deleted";
        graph->GetEventWriter().EventDeoptimizeElimination(GetOpcodeString(savestate->GetOpcode()), savestate->GetId(),
                                                           savestate->GetPc());
    }
    SetApplied();
}

void DeoptimizeElimination::InvalidateAnalyses()
{
    // Before in "CleanupGraphSaveStateOSR" already run "LoopAnalyzer"
    // in case (IsLoopDeleted() && GetGraph()->IsOsrMode())
    if (!(IsLoopDeleted() && GetGraph()->IsOsrMode())) {
        GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    }
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}
}  // namespace ark::compiler
