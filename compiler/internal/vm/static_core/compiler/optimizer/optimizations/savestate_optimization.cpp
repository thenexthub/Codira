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
#include "savestate_optimization.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/ir/analysis.h"

namespace ark::compiler {

bool SaveStateOptimization::RunImpl()
{
    if (g_options.IsCompilerEnforceSafepointPlacement()) {
        return false;
    }
    uint64_t instsNumber = VisitGraphAndCount();
    if (!HaveCalls() && instsNumber <= g_options.GetCompilerSafepointEliminationLimit()) {
        RemoveSafePoints();
    }
    return IsApplied();
}

void SaveStateOptimization::RemoveSafePoints()
{
    auto block = GetGraph()->GetStartBlock();
    ASSERT(block != nullptr && block->IsStartBlock());
    for (auto sp : block->Insts()) {
        if (sp->GetOpcode() == Opcode::SafePoint) {
            sp->ClearFlag(inst_flags::NO_DCE);
            SetApplied();
            COMPILER_LOG(DEBUG, SAVESTATE_OPT) << "SafePoint " << sp->GetId() << " is deleted from start block";
            block->GetGraph()->GetEventWriter().EventSaveStateOptimization(GetOpcodeString(sp->GetOpcode()),
                                                                           sp->GetId(), sp->GetPc());
        }
    }
}

bool SaveStateOptimization::RequireRegMap(Inst *inst)
{
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->RequireRegMap()) {
            return true;
        }
        auto opcode = userInst->GetOpcode();
        if (opcode == Opcode::CallStatic || opcode == Opcode::CallVirtual || opcode == Opcode::CallResolvedVirtual ||
            opcode == Opcode::CallDynamic || opcode == Opcode::CallResolvedStatic) {
            // Inlined method can contain Deoptimize or DeoptimizeIf
            if (static_cast<CallInst *>(userInst)->IsInlined()) {
                return true;
            }
        }
    }
    return false;
}

void SaveStateOptimization::VisitDefault(Inst *inst)
{
    if (inst->GetType() != DataType::REFERENCE) {
        return;
    }
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (!userInst->IsSaveState()) {
            return;
        }
        if (userInst->GetOpcode() == Opcode::SafePoint) {
            if (g_options.IsCompilerSafePointsRequireRegMap()) {
                return;
            }
            continue;
        }
        if (RequireRegMap(userInst)) {
            return;
        }
    }

    inst->RemoveUsers<true>();

    SetApplied();
    COMPILER_LOG(DEBUG, SAVESTATE_OPT) << "All users of the instructions " << inst->GetId() << " are SaveStates";
    inst->GetBasicBlock()->GetGraph()->GetEventWriter().EventSaveStateOptimization(GetOpcodeString(inst->GetOpcode()),
                                                                                   inst->GetId(), inst->GetPc());
}

void SaveStateOptimization::VisitSaveState(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<SaveStateOptimization *>(v);
    if (visitor->TryToRemoveRedundantSaveState(inst)) {
        return;
    }

    if (visitor->RequireRegMap(inst)) {
        return;
    }

    auto ss = inst->CastToSaveState();
    if (ss->RemoveNumericInputs()) {
        visitor->SetApplied();
        COMPILER_LOG(DEBUG, SAVESTATE_OPT) << "SaveState " << ss->GetId() << " numeric inputs were deleted";
        ss->GetBasicBlock()->GetGraph()->GetEventWriter().EventSaveStateOptimization(GetOpcodeString(ss->GetOpcode()),
                                                                                     ss->GetId(), ss->GetPc());
        ss->SetInputsWereDeleted();
    }
}

void SaveStateOptimization::VisitSaveStateDeoptimize(GraphVisitor *v, Inst *inst)
{
    static_cast<SaveStateOptimization *>(v)->TryToRemoveRedundantSaveState(inst);
}

bool SaveStateOptimization::TryToRemoveRedundantSaveState(Inst *inst)
{
    if (!inst->HasUsers()) {
        auto block = inst->GetBasicBlock();
        block->ReplaceInst(inst, block->GetGraph()->CreateInstNOP());
        inst->RemoveInputs();
        SetApplied();
        COMPILER_LOG(DEBUG, SAVESTATE_OPT) << "SaveState " << inst->GetId() << " without users is deleted";
        block->GetGraph()->GetEventWriter().EventSaveStateOptimization(GetOpcodeString(inst->GetOpcode()),
                                                                       inst->GetId(), inst->GetPc());
        return true;
    }
    return false;
}
}  // namespace ark::compiler
