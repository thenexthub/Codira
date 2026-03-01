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

#include "object_type_check_elimination.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/object_type_propagation.h"

namespace ark::compiler {
bool ObjectTypeCheckElimination::RunImpl()
{
    GetGraph()->RunPass<LoopAnalyzer>();
    GetGraph()->RunPass<ObjectTypePropagation>();
    GetGraph()->RunPass<DominatorsTree>();
    VisitGraph();
    ReplaceCheckMustThrowByUnconditionalDeoptimize();
    return IsApplied();
}

void ObjectTypeCheckElimination::VisitIsInstance(GraphVisitor *visitor, Inst *inst)
{
    if (TryEliminateIsInstance(inst)) {
        static_cast<ObjectTypeCheckElimination *>(visitor)->SetApplied();
    }
}

void ObjectTypeCheckElimination::VisitCheckCast(GraphVisitor *visitor, Inst *inst)
{
    auto result = TryEliminateCheckCast(inst);
    if (result != CheckCastEliminateType::INVALID) {
        auto opt = static_cast<ObjectTypeCheckElimination *>(visitor);
        opt->SetApplied();
        if (result == CheckCastEliminateType::MUST_THROW) {
            opt->PushNewCheckMustThrow(inst);
        }
    }
}

void ObjectTypeCheckElimination::ReplaceCheckMustThrowByUnconditionalDeoptimize()
{
    for (auto &inst : checksMustThrow_) {
        auto block = inst->GetBasicBlock();
        if (block != nullptr) {
            COMPILER_LOG(DEBUG, CHECKS_ELIM)
                << "Replace check with id = " << inst->GetId() << " by uncondition deoptimize";
            block->ReplaceInstByDeoptimize(inst);
            SetApplied();
        }
    }
}

/**
 * This function try to replace IsInstance with a constant.
 * If input of IsInstance is Nullptr then it replaced by zero constant.
 */
bool ObjectTypeCheckElimination::TryEliminateIsInstance(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::IsInstance);
    if (!inst->HasUsers()) {
        return false;
    }
    auto block = inst->GetBasicBlock();
    auto graph = block->GetGraph();
    auto ref = inst->GetDataFlowInput(0);
    // Null isn't instance of any class.
    if (ref->GetOpcode() == Opcode::NullPtr) {
        auto newCnst = graph->FindOrCreateConstant(0);
        inst->ReplaceUsers(newCnst);
        return true;
    }
    auto isInstance = inst->CastToIsInstance();
    if (!graph->IsBytecodeOptimizer() && IsMember(ref, isInstance->GetTypeId(), isInstance)) {
        if (BoundsAnalysis::IsInstNotNull(ref, block)) {
            auto newCnst = graph->FindOrCreateConstant(true);
            inst->ReplaceUsers(newCnst);
            return true;
        }
    }
    auto tgtKlass = graph->GetRuntime()->GetClass(isInstance->GetMethod(), isInstance->GetTypeId());
    // If we can't resolve klass in runtime we must throw exception, so we check NullPtr after
    // But we can't change the IsInstance to Deoptimize, because we can resolve after compilation
    if (tgtKlass == nullptr) {
        return false;
    }
    auto refInfo = ref->GetObjectTypeInfo();
    if (refInfo) {
        auto refKlass = refInfo.GetClass();
        bool result = graph->GetRuntime()->IsAssignableFrom(tgtKlass, refKlass);
        // If ref can be null, IsInstance cannot be changed to true
        if (result) {
            if (graph->IsBytecodeOptimizer()) {
                // Cannot run BoundsAnalysis
                return false;
            }
            if (!BoundsAnalysis::IsInstNotNull(ref, block)) {
                return false;
            }
        }
        // If class of ref can be subclass of ref_klass, IsInstance cannot be changed to false
        if (!result && !refInfo.IsExact()) {
            return false;
        }
        auto newCnst = graph->FindOrCreateConstant(result);
        inst->ReplaceUsers(newCnst);
        return true;
    }
    return false;
}

ObjectTypeCheckElimination::CheckCastEliminateType ObjectTypeCheckElimination::TryEliminateCheckCast(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::CheckCast);
    auto block = inst->GetBasicBlock();
    auto graph = block->GetGraph();
    auto ref = inst->GetDataFlowInput(0);
    // Null can be cast to every type.
    if (ref->GetOpcode() == Opcode::NullPtr) {
        inst->RemoveInputs();
        block->ReplaceInst(inst, block->GetGraph()->CreateInstNOP());
        return CheckCastEliminateType::REDUNDANT;
    }
    auto checkCast = inst->CastToCheckCast();
    auto tgtKlass = graph->GetRuntime()->GetClass(checkCast->GetMethod(), checkCast->GetTypeId());
    auto refInfo = ref->GetObjectTypeInfo();
    // If we can't resolve klass in runtime we must throw exception, so we check NullPtr after
    // But we can't change the CheckCast to Deoptimize, because we can resolve after compilation
    if (tgtKlass != nullptr && refInfo) {
        auto refKlass = refInfo.GetClass();
        bool result = graph->GetRuntime()->IsAssignableFrom(tgtKlass, refKlass);
        if (result) {
            inst->RemoveInputs();
            block->ReplaceInst(inst, block->GetGraph()->CreateInstNOP());
            return CheckCastEliminateType::REDUNDANT;
        }
        if (BoundsAnalysis::IsInstNotNull(ref, block) && refInfo.IsExact()) {
            return CheckCastEliminateType::MUST_THROW;
        }
    }
    if (IsMember(ref, checkCast->GetTypeId(), inst)) {
        inst->RemoveInputs();
        block->ReplaceInst(inst, block->GetGraph()->CreateInstNOP());
        return CheckCastEliminateType::REDUNDANT;
    }
    return CheckCastEliminateType::INVALID;
}

// returns true if data flow input of inst is always member of class type_id when ref_user is executed
bool ObjectTypeCheckElimination::IsMember(Inst *inst, uint32_t typeId, Inst *refUser)
{
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst == refUser || !userInst->IsDominate(refUser)) {
            continue;
        }
        bool success = false;
        switch (userInst->GetOpcode()) {
            case Opcode::CheckCast:
                success = (userInst->CastToCheckCast()->GetTypeId() == typeId);
                break;
            case Opcode::IsInstance:
                success = IsSuccessfulIsInstance(userInst->CastToIsInstance(), typeId, refUser);
                break;
            case Opcode::NullCheck:
                success = IsMember(userInst, typeId, refUser);
                break;
            default:
                break;
        }
        if (success) {
            return true;
        }
    }
    return false;
}

// returns true if is_instance has given type_id and evaluates to true at ref_user
bool ObjectTypeCheckElimination::IsSuccessfulIsInstance(IsInstanceInst *isInstance, uint32_t typeId, Inst *refUser)
{
    ASSERT(isInstance->GetDataFlowInput(0) == refUser->GetDataFlowInput(0));
    if (isInstance->GetTypeId() != typeId) {
        return false;
    }
    for (auto &user : isInstance->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->GetOpcode() != Opcode::IfImm) {
            continue;
        }
        auto trueBlock = userInst->CastToIfImm()->GetEdgeIfInputTrue();
        if (trueBlock->GetPredsBlocks().size() == 1 && trueBlock->IsDominate(refUser->GetBasicBlock())) {
            return true;
        }
    }
    return false;
}

void ObjectTypeCheckElimination::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}
}  // namespace ark::compiler
