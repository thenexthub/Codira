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

#include "lower_boxed_boolean.h"
#include "compiler_logger.h"
#include <optional>

#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_visitor.h"
#include "optimizer/ir/inst.h"
#include "libarkbase/utils/arena_containers.h"

namespace ark::compiler {

bool LowerBoxedBoolean::RunImpl()
{
    COMPILER_LOG(DEBUG, LOWER_BOXED_BOOLEAN) << "Run " << GetPassName();
    isApplied_ = false;
    visitedMarker_ = GetGraph()->NewMarker();
    VisitGraph();
    instReplacements_.clear();
    GetGraph()->EraseMarker(visitedMarker_);
    COMPILER_LOG(DEBUG, LOWER_BOXED_BOOLEAN) << "LowerBoxedBoolean " << (isApplied_ ? "is" : "isn't") << " applied";
    COMPILER_LOG(DEBUG, LOWER_BOXED_BOOLEAN) << "Finish LowerBoxedBoolean";
    return isApplied_;
}

void LowerBoxedBoolean::VisitCompare(GraphVisitor *v, Inst *inst)
{
    COMPILER_LOG(DEBUG, LOWER_BOXED_BOOLEAN) << "Start visit Compare with id = " << inst->GetId();
    auto visitor = static_cast<LowerBoxedBoolean *>(v);
    auto input = inst->GetInput(0).GetInst();

    if (!IsCompareWithNullPtr(inst)) {
        return;
    }

    ProcessInput(v, input);

    if (!visitor->HasReplacement(input)) {
        return;
    }

    COMPILER_LOG(DEBUG, LOWER_BOXED_BOOLEAN)
        << "Applied LowerBoxedBoolean optimization to Compare with id = " << inst->GetId();

    auto graph = inst->GetBasicBlock()->GetGraph();
    auto cc = inst->CastToCompare()->GetCc();
    ASSERT(cc == ConditionCode::CC_NE || cc == ConditionCode::CC_EQ);
    int value = cc == ConditionCode::CC_NE ? 1 : 0;
    inst->ReplaceUsers(graph->FindOrCreateConstant(value));
    visitor->SetApplied();
}

void LowerBoxedBoolean::VisitLoadObject(GraphVisitor *v, Inst *inst)
{
    COMPILER_LOG(DEBUG, LOWER_BOXED_BOOLEAN) << "Start visit LoadObject with id = " << inst->GetId();
    auto runtime = inst->GetBasicBlock()->GetGraph()->GetRuntime();
    auto fieldPtr = inst->CastToLoadObject()->GetObjField();
    auto visitor = static_cast<LowerBoxedBoolean *>(v);
    if (!runtime->IsFieldBooleanValue(fieldPtr)) {
        return;
    }

    auto input = inst->GetDataFlowInput(0);
    if (!IsValidLoadObjectInput(input)) {
        return;
    }

    ProcessInput(v, input);

    if (visitor->HasReplacement(input)) {
        COMPILER_LOG(DEBUG, LOWER_BOXED_BOOLEAN)
            << "Applied LowerBoxedBoolean optimization to LoadObject with id = " << inst->GetId();
        inst->ReplaceUsers(visitor->GetReplacement(input));
        visitor->SetApplied();
    }
}

bool LowerBoxedBoolean::IsValidLoadObjectInput(Inst *input)
{
    // For LoadObject std.core.Boolean, we only support cases where its input is either:
    //   - LoadStatic of a known Boolean constant (TRUE or FALSE), or
    //   - Phi node merging such values.
    return input->IsPhi() || GetBooleanFieldValue(input).has_value();
}

void LowerBoxedBoolean::ProcessInput(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<LowerBoxedBoolean *>(v);
    if (visitor->IsVisited(inst)) {
        return;
    }
    visitor->SetVisited(inst);

    switch (inst->GetOpcode()) {
        case Opcode::LoadStatic:
            ProcessLoadStatic(v, inst);
            break;
        case Opcode::Phi:
            ProcessPhi(v, inst);
            break;
        default:
            break;
    }
}

void LowerBoxedBoolean::ProcessPhi(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<LowerBoxedBoolean *>(v);
    if (visitor->HasReplacement(inst)) {
        return;
    }

    if (!HasOnlyKnownUsers(inst)) {
        return;
    }

    COMPILER_LOG(DEBUG, LOWER_BOXED_BOOLEAN) << "Process Phi with id = " << inst->GetId();

    // We must be sure that all Phi inputs are reducible to constant Boolean values
    // before replacing the Phi itself.
    for (auto input : inst->GetInputs()) {
        ProcessInput(v, input.GetInst());
        if (!visitor->HasReplacement(input.GetInst())) {
            return;
        }
    }

    // Clone Phi instruction, replace its inputs with optimized versions, and set its type to BOOL.
    auto graph = inst->GetBasicBlock()->GetGraph();
    auto clone = inst->Clone(graph);
    clone->SetType(DataType::BOOL);
    inst->InsertBefore(clone);

    visitor->SetInstReplacement(inst, clone);
    for (auto input : inst->GetInputs()) {
        clone->AppendInput(visitor->GetReplacement(input.GetInst()));
    }

    inst->SetFlag(inst_flags::NO_NULLPTR);
}

void LowerBoxedBoolean::ProcessLoadStatic(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<LowerBoxedBoolean *>(v);
    if (visitor->HasReplacement(inst)) {
        return;
    }

    if (!HasOnlyKnownUsers(inst)) {
        return;
    }

    COMPILER_LOG(DEBUG, LOWER_BOXED_BOOLEAN) << "Process LoadStatic with id = " << inst->GetId();

    auto graph = inst->GetBasicBlock()->GetGraph();
    if (auto fieldValue = GetBooleanFieldValue(inst)) {
        inst->SetFlag(inst_flags::NO_NULLPTR);
        auto constInst = graph->FindOrCreateConstant(*fieldValue);
        visitor->SetInstReplacement(inst, constInst);
    }
}

bool LowerBoxedBoolean::HasOnlyKnownUsers(Inst *inst)
{
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        auto opcode = userInst->GetOpcode();
        if (opcode == Opcode::SaveState) {
            if (ProcessSaveState(userInst, inst)) {
                continue;
            }
        }
        switch (opcode) {
            case Opcode::CallStatic:
            case Opcode::CheckCast:
            case Opcode::Compare:
            case Opcode::IfImm:
            case Opcode::Intrinsic:
            case Opcode::LoadObject:
            case Opcode::NullCheck:
            case Opcode::Phi:
                continue;
            default:
                return false;
        }
    }
    return true;
}

bool LowerBoxedBoolean::ProcessSaveState(Inst *saveState, Inst *inst)
{
    auto saveStateInst = saveState->CastToSaveState();
    auto callerInst = saveStateInst->GetCallerInst();
    if (callerInst != nullptr && callerInst->IsInlined()) {
        return true;
    }

    if (!saveStateInst->HasUsers()) {
        return false;
    }

    auto firstUser = saveStateInst->GetFirstUser()->GetInst();
    if (firstUser->GetOpcode() == Opcode::ReturnInlined) {
        return true;
    }

    return CheckSaveStateUsers(saveState, inst);
}

bool LowerBoxedBoolean::CheckSaveStateUsers(Inst *saveStateInst, Inst *inst)
{
    for (auto &user : saveStateInst->GetUsers()) {
        if (!IsNullCheckUsingInput(user.GetInst(), inst)) {
            continue;
        }
        return true;
    }
    return false;
}

bool LowerBoxedBoolean::IsNullCheckUsingInput(Inst *inst, Inst *input)
{
    if (inst->GetOpcode() != Opcode::NullCheck) {
        return false;
    }

    for (auto &in : inst->GetInputs()) {
        if (in.GetInst() == input) {
            return true;
        }
    }
    return false;
}

std::optional<uint32_t> LowerBoxedBoolean::GetBooleanFieldValue(Inst *inst)
{
    if (inst->GetOpcode() != Opcode::LoadStatic) {
        return std::nullopt;
    }

    auto graph = inst->GetBasicBlock()->GetGraph();
    auto runtime = graph->GetRuntime();
    auto fieldPtr = inst->CastToLoadStatic()->GetObjField();

    bool isTrue = runtime->IsFieldBooleanTrue(fieldPtr);
    bool isFalse = runtime->IsFieldBooleanFalse(fieldPtr);
    if (isTrue == isFalse) {
        return std::nullopt;
    }

    return isTrue ? 1 : 0;
}

bool LowerBoxedBoolean::IsCompareWithNullPtr(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::Compare);
    auto input = inst->GetInput(1).GetInst();
    return input->GetOpcode() == Opcode::NullPtr;
}

void LowerBoxedBoolean::SetInstReplacement(Inst *oldInst, Inst *newInst)
{
    instReplacements_[oldInst] = newInst;
}

Inst *LowerBoxedBoolean::GetReplacement(Inst *inst)
{
    if (HasReplacement(inst)) {
        return instReplacements_[inst];
    }

    return nullptr;
}

bool LowerBoxedBoolean::HasReplacement(Inst *inst) const
{
    return instReplacements_.find(inst) != instReplacements_.end();
}

void LowerBoxedBoolean::SetVisited(Inst *inst)
{
    inst->SetMarker(visitedMarker_);
}

bool LowerBoxedBoolean::IsVisited(Inst *inst) const
{
    return inst->IsMarked(visitedMarker_);
}

void LowerBoxedBoolean::SetApplied()
{
    isApplied_ = true;
}
}  // namespace ark::compiler
