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
#include "checks_elimination.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/object_type_propagation.h"
#include "optimizer/ir/graph_visitor.h"
#include "optimizer/ir/analysis.h"

namespace ark::compiler {

bool ChecksElimination::RunImpl()
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start ChecksElimination";
    GetGraph()->RunPass<DominatorsTree>();
    GetGraph()->RunPass<LoopAnalyzer>();
    GetGraph()->RunPass<ObjectTypePropagation>();

    VisitGraph();

    if (g_options.IsCompilerEnableReplacingChecksOnDeoptimization()) {
        if (!GetGraph()->IsOsrMode()) {
            ReplaceBoundsCheckToDeoptimizationBeforeLoop();
            MoveCheckOutOfLoop();
        }
        ReplaceBoundsCheckToDeoptimizationInLoop();

        ReplaceCheckMustThrowByUnconditionalDeoptimize();
    }
    if (IsLoopDeleted() && GetGraph()->IsOsrMode()) {
        CleanupGraphSaveStateOSR(GetGraph());
    }
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "ChecksElimination " << (IsApplied() ? "is" : "isn't") << " applied";
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Finish ChecksElimination";
    return isApplied_;
}

void ChecksElimination::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<DominatorsTree>();
    // Already "LoopAnalyzer" was ran in "CleanupGraphSaveStateOSR"
    // in case (IsLoopDeleted() && GetGraph()->IsOsrMode())
    if (!(IsLoopDeleted() && GetGraph()->IsOsrMode())) {
        GetGraph()->InvalidateAnalysis<LoopAnalyzer>();
    }
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

template <bool CHECK_FULL_DOM>
void ChecksElimination::VisitNullCheck(GraphVisitor *v, Inst *inst)
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start visit NullCheck with id = " << inst->GetId();

    auto ref = inst->GetInput(0).GetInst();
    static_cast<ChecksElimination *>(v)->TryRemoveDominatedNullChecks(inst, ref);

    if (!static_cast<ChecksElimination *>(v)->TryRemoveCheck<Opcode::NullCheck, CHECK_FULL_DOM>(inst)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "NullCheck couldn't be deleted";
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "NullCheck saved for further replacing on deoptimization";
        static_cast<ChecksElimination *>(v)->PushNewCheckForMoveOutOfLoop(inst);
    }
}

void ChecksElimination::VisitNegativeCheck(GraphVisitor *v, Inst *inst)
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start visit NegativeCheck with id = " << inst->GetId();
    if (!static_cast<ChecksElimination *>(v)->TryRemoveCheck<Opcode::NegativeCheck>(inst)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "NegativeCheck couldn't be deleted";
        static_cast<ChecksElimination *>(v)->PushNewCheckForMoveOutOfLoop(inst);
    }
}

void ChecksElimination::VisitNotPositiveCheck(GraphVisitor *v, Inst *inst)
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start visit NotPositiveCheck with id = " << inst->GetId();
    if (!static_cast<ChecksElimination *>(v)->TryRemoveCheck<Opcode::NotPositiveCheck>(inst)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "NotPositiveCheck couldn't be deleted";
        static_cast<ChecksElimination *>(v)->PushNewCheckForMoveOutOfLoop(inst);
    }
}

void ChecksElimination::VisitZeroCheck(GraphVisitor *v, Inst *inst)
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start visit ZeroCheck with id = " << inst->GetId();
    if (!static_cast<ChecksElimination *>(v)->TryRemoveCheck<Opcode::ZeroCheck>(inst)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "ZeroCheck couldn't be deleted";
        static_cast<ChecksElimination *>(v)->PushNewCheckForMoveOutOfLoop(inst);
    }
}

void ChecksElimination::VisitRefTypeCheck(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<ChecksElimination *>(v);
    auto storeInst = inst->GetDataFlowInput(inst->GetInput(1).GetInst());
    // Case: a[i] = nullptr
    if (storeInst->GetOpcode() == Opcode::NullPtr) {
        visitor->ReplaceUsersAndRemoveCheck(inst, storeInst);
        return;
    }
    auto arrayInst = inst->GetDataFlowInput(0);
    auto ref = inst->GetInput(1).GetInst();
    // Case:
    // a[1] = obj
    // a[2] = obj
    visitor->TryRemoveDominatedChecks<Opcode::RefTypeCheck>(inst, [arrayInst, ref](Inst *userInst) {
        return userInst->GetDataFlowInput(0) == arrayInst && userInst->GetInput(1) == ref;
    });
    auto arrayTypeInfo = arrayInst->GetObjectTypeInfo();
    if (arrayTypeInfo && arrayTypeInfo.IsExact()) {
        auto storeTypeInfo = storeInst->GetObjectTypeInfo();
        auto arrayClass = arrayTypeInfo.GetClass();
        auto storeClass = (storeTypeInfo) ? storeTypeInfo.GetClass() : nullptr;
        if (visitor->GetGraph()->GetRuntime()->CheckStoreArray(arrayClass, storeClass)) {
            visitor->ReplaceUsersAndRemoveCheck(inst, storeInst);
            return;
        }
    }
    visitor->PushNewCheckForMoveOutOfLoop(inst);
}

bool ChecksElimination::TryToEliminateAnyTypeCheck(Inst *inst, Inst *instToReplace, AnyBaseType type,
                                                   AnyBaseType prevType)
{
    auto language = GetGraph()->GetRuntime()->GetMethodSourceLanguage(GetGraph()->GetMethod());
    auto allowedType = inst->CastToAnyTypeCheck()->GetAllowedInputType();
    profiling::AnyInputType prevAllowedType;
    if (instToReplace->GetOpcode() == Opcode::AnyTypeCheck) {
        prevAllowedType = instToReplace->CastToAnyTypeCheck()->GetAllowedInputType();
    } else {
        prevAllowedType = instToReplace->CastToCastValueToAnyType()->GetAllowedInputType();
    }
    auto res = IsAnyTypeCanBeSubtypeOf(language, type, prevType, allowedType, prevAllowedType);
    if (!res) {
        return false;
    }
    if (res.value()) {
        ReplaceUsersAndRemoveCheck(inst, instToReplace);
    } else {
        PushNewCheckMustThrow(inst);
    }
    return true;
}

void ChecksElimination::UpdateHclassChecks(Inst *inst)
{
    // AnyTypeCheck HEAP_OBJECT => LoadObject Class => LoadObject Hclass => HclassCheck
    bool allUsersInlined = false;
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->IsCall()) {
            if (userInst->CastToCallDynamic()->IsInlined()) {
                allUsersInlined = true;
            } else {
                return;
            }
        } else if (userInst->GetOpcode() == Opcode::LoadObject) {
            if (userInst->CastToLoadObject()->GetObjectType() == ObjectType::MEM_DYN_CLASS) {
                continue;
            }
            ASSERT(userInst->CastToLoadObject()->GetObjectType() == ObjectType::MEM_DYN_METHOD);
            if (!IsInlinedCallLoadMethod(userInst)) {
                return;
            }
            allUsersInlined = true;
        } else {
            return;
        }
    }
    if (!allUsersInlined) {
        return;
    }
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        auto check = GetHclassCheckFromLoads(userInst);
        if (!check.has_value()) {
            continue;
        }
        check.value()->CastToHclassCheck()->SetCheckFunctionIsNotClassConstructor(false);
        SetApplied();
    }
}

std::optional<Inst *> ChecksElimination::GetHclassCheckFromLoads(Inst *loadClass)
{
    if (loadClass->GetOpcode() != Opcode::LoadObject ||
        loadClass->CastToLoadObject()->GetObjectType() != ObjectType::MEM_DYN_CLASS) {
        return std::nullopt;
    }
    for (auto &usersLoad : loadClass->GetUsers()) {
        auto loadHclass = usersLoad.GetInst();
        if (loadHclass->GetOpcode() != Opcode::LoadObject ||
            loadHclass->CastToLoadObject()->GetObjectType() != ObjectType::MEM_DYN_HCLASS) {
            continue;
        }
        for (auto &usersHclass : loadHclass->GetUsers()) {
            auto hclassCheck = usersHclass.GetInst();
            if (hclassCheck->GetOpcode() == Opcode::HclassCheck) {
                return hclassCheck;
            }
        }
    }
    return std::nullopt;
}

bool ChecksElimination::IsInlinedCallLoadMethod(Inst *inst)
{
    bool isMethod = inst->GetOpcode() == Opcode::LoadObject &&
                    inst->CastToLoadObject()->GetObjectType() == ObjectType::MEM_DYN_METHOD;
    if (!isMethod) {
        return false;
    }
    for (auto &methodUser : inst->GetUsers()) {
        auto compare = methodUser.GetInst()->CastToCompare();
        bool isCompare = compare->GetOpcode() == Opcode::Compare && compare->GetCc() == ConditionCode::CC_NE &&
                         compare->GetInputType(0) == DataType::POINTER;
        if (!isCompare) {
            return false;
        }
        auto deoptimizeIf = compare->GetFirstUser()->GetInst()->CastToDeoptimizeIf();
        if (deoptimizeIf->GetDeoptimizeType() != DeoptimizeType::INLINE_DYN) {
            return false;
        }
    }
    return true;
}

void ChecksElimination::TryRemoveDominatedHclassCheck(Inst *inst)
{
    ASSERT(inst->GetOpcode() == Opcode::HclassCheck);
    // AnyTypeCheck HEAP_OBJECT => LoadObject Class => LoadObject Hclass => HclassCheck
    auto object = inst->GetInput(0).GetInst()->GetInput(0).GetInst()->GetInput(0).GetInst();

    for (auto &user : object->GetUsers()) {
        auto userInst = user.GetInst();
        auto hclassCheck = GetHclassCheckFromLoads(userInst);
        if (hclassCheck.has_value() && hclassCheck.value() != inst && inst->IsDominate(hclassCheck.value())) {
            ASSERT(inst->IsDominate(hclassCheck.value()));
            COMPILER_LOG(DEBUG, CHECKS_ELIM)
                << GetOpcodeString(Opcode::HclassCheck) << " with id = " << inst->GetId() << " dominate on "
                << GetOpcodeString(Opcode::HclassCheck) << " with id = " << hclassCheck.value()->GetId();
            auto block = hclassCheck.value()->GetBasicBlock();
            auto graph = block->GetGraph();
            hclassCheck.value()->RemoveInputs();
            // We don't should update because it's possible to delete one flag in check, when all DynCall is inlined
            inst->CastToHclassCheck()->ExtendFlags(hclassCheck.value());
            block->ReplaceInst(hclassCheck.value(), graph->CreateInstNOP());
            SetApplied();
        }
    }
}

void ChecksElimination::VisitAnyTypeCheck(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<ChecksElimination *>(v);
    auto inputInst = inst->GetInput(0).GetInst();
    auto type = inst->CastToAnyTypeCheck()->GetAnyType();
    if (type == AnyBaseType::UNDEFINED_TYPE) {
        visitor->ReplaceUsersAndRemoveCheck(inst, inputInst);
        return;
    }
    // from:
    //     2.any  CastValueToAnyType ANY_SUBTYPE v1 -> (v4)
    //     4.any  AnyTypeCheck ANY_SUBTYPE v2, v3 -> (....)
    // to:
    //     2.any  CastValueToAnyType ANY_SUBTYPE v1 -> (...)
    if (inputInst->GetOpcode() == Opcode::CastValueToAnyType) {
        visitor->TryToEliminateAnyTypeCheck(inst, inputInst, type, inputInst->CastToCastValueToAnyType()->GetAnyType());
        return;
    }
    // from:
    //     2.any  AnyTypeCheck ANY_SUBTYPE v1, v0 -> (v4)
    //     4.any  AnyTypeCheck ANY_SUBTYPE v2, v3 -> (....)
    // to:
    //     2.any  AnyTypeCheck ANY_SUBTYPE v1, v0 -> (...)
    if (inputInst->GetOpcode() == Opcode::AnyTypeCheck) {
        visitor->TryToEliminateAnyTypeCheck(inst, inputInst, type, inputInst->CastToAnyTypeCheck()->GetAnyType());
        return;
    }
    // from:
    //     2.any  AnyTypeCheck ANY_SUBTYPE v1, v0 -> (v4)
    //     4.any  AnyTypeCheck ANY_SUBTYPE v1, v3 -> (....)
    // to:
    //     2.any  AnyTypeCheck ANY_SUBTYPE v1, v0 -> (v4,...)
    bool applied = false;
    for (auto &user : inputInst->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst == inst) {
            continue;
        }
        if (userInst->GetOpcode() != Opcode::AnyTypeCheck) {
            continue;
        }
        if (!inst->IsDominate(userInst)) {
            continue;
        }

        if (visitor->TryToEliminateAnyTypeCheck(userInst, inst, userInst->CastToAnyTypeCheck()->GetAnyType(), type)) {
            applied = true;
        }
    }
    if (!applied) {
        visitor->PushNewCheckForMoveOutOfLoop(inst);
    }
    // from:
    //    39.any  AnyTypeCheck ECMASCRIPT_HEAP_OBJECT_TYPE v37, v38 -> (v65, v40)
    //    40.ref  LoadObject 4294967288 Class v39 -> (v41)
    //    41.ref  LoadObject 4294967287 Hclass v40
    //    42.     HclassCheck [IsFunc, IsNotClassConstr] v41, v38
    //      ...METHOD IS INLINED ...
    // to:
    //    39.any  AnyTypeCheck ECMASCRIPT_HEAP_OBJECT_TYPE v37, v38 -> (v65, v40)
    //    40.ref  LoadObject 4294967288 Class v39 -> (v41)
    //    41.ref  LoadObject 4294967287 Hclass v40
    //    42.     HclassCheck [IsFunc] v41, v38
    //      ...METHOD IS INLINED ...
    if (TryIsDynHeapObject(type)) {
        visitor->UpdateHclassChecks(inst);
    }
}

void ChecksElimination::VisitHclassCheck(GraphVisitor *v, Inst *inst)
{
    [[maybe_unused]] auto visitor = static_cast<ChecksElimination *>(v);
    visitor->TryRemoveDominatedHclassCheck(inst);
    visitor->PushNewCheckForMoveOutOfLoop(inst);
}

template <typename RangeChecker, typename... RangeCheckers>
bool NeedSetFlagNoHoist(Inst *check, Inst *input, RangeChecker rangeChecker, RangeCheckers... rangeCheckers)
{
    return NeedSetFlagNoHoist(check, input, rangeChecker) || (NeedSetFlagNoHoist(check, input, rangeCheckers) || ...);
}

template <typename RangeChecker>
bool NeedSetFlagNoHoist(Inst *check, Inst *input, RangeChecker rangeChecker)
{
    auto checkBlock = check->GetBasicBlock();
    auto inputBlock = input->GetBasicBlock();

    // 1. The `check` is in the root loop, further analysis is not needed.
    if (checkBlock->GetLoop()->IsRoot()) {
        return false;
    }

    // 2. If `input` isn't in the root loop and it is hoistable, conservatively enforce `NO_HOIST` flag on `check`'s
    //    users since `input` itself may be hoisted, allowing the users to be hoisted as well (with the danger of
    //    breaking bounds). See 'HoistableInput' checked test.
    if (((!inputBlock->GetLoop()->IsRoot()) && (!input->IsNotHoistable())) || input->IsPhi()) {
        return true;
    }

    // 3. `input` is not hoistable due to `2.` so users can be hoisted only into preheader of some inner loops, which
    //     are dominated by `checkBlock`. No need to set `NO_HOIST` flag explicitly.
    if ((inputBlock->GetLoop() == checkBlock->GetLoop()) || (inputBlock->GetLoop()->IsInside(checkBlock->GetLoop()))) {
        return false;
    }

    // Find outermost loop of `check` that is lying in the innermost loop, containing both `check` and `input`.
    auto cl = checkBlock->GetLoop();
    auto il = inputBlock->GetLoop();
    while (cl->GetOuterLoop() != il) {
        cl = cl->GetOuterLoop();
        if (cl == nullptr) {
            // Restart algorithm with an outer `input` loop.
            cl = checkBlock->GetLoop();
            il = il->GetOuterLoop();
        }
    }
    auto outermostPrehead = cl->GetPreHeader();

    // It is hoistable if bounds range are valid at the outermost prehead:
    auto bri = outermostPrehead->GetGraph()->GetBoundsRangeInfo();
    auto range = bri->FindBoundsRange(outermostPrehead, input);
    return !rangeChecker(range);
}

template <typename... RangeCheckers>
void CheckAndSetFlagNoHoist(Inst *inst, Inst *input, RangeCheckers... rangeCheckers)
{
    if (NeedSetFlagNoHoist(inst, input, rangeCheckers...)) {
        for (auto &user : inst->GetUsers()) {
            user.GetInst()->SetFlag(inst_flags::NO_HOIST);
        }
    }
}

void ChecksElimination::VisitBoundsCheck(GraphVisitor *v, Inst *inst)
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start visit BoundsCheck with id = " << inst->GetId();
    auto block = inst->GetBasicBlock();
    auto lenArray = inst->GetInput(0).GetInst();
    auto index = inst->GetInput(1).GetInst();
    auto visitor = static_cast<ChecksElimination *>(v);

    visitor->TryRemoveDominatedChecks<Opcode::BoundsCheck>(inst, [lenArray, index](Inst *userInst) {
        return lenArray == userInst->GetInput(0) && index == userInst->GetInput(1);
    });

    auto bri = block->GetGraph()->GetBoundsRangeInfo();
    auto lenArrayRange = bri->FindBoundsRange(block, lenArray);
    auto upperChecker = [&lenArrayRange, &lenArray](const BoundsRange &idxRange) {
        return idxRange.IsLess(lenArrayRange) || idxRange.IsLess(lenArray);
    };
    auto lowerChecker = [](const BoundsRange &idxRange) { return idxRange.IsNotNegative(); };
    auto indexRange = bri->FindBoundsRange(block, index);
    auto correctUpper = upperChecker(indexRange);
    auto correctLower = lowerChecker(indexRange);
    if (correctUpper && correctLower) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Index of BoundsCheck have correct bounds";
        CheckAndSetFlagNoHoist(inst, index, upperChecker, lowerChecker);
        visitor->ReplaceUsersAndRemoveCheck(inst, index);
        return;
    }

    if (indexRange.IsNegative() || indexRange.IsMoreOrEqual(lenArrayRange)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM)
            << "BoundsCheck have incorrect bounds, saved for replace by unconditional deoptimize";
        visitor->PushNewCheckMustThrow(inst);
        return;
    }

    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "BoundsCheck saved for further replacing on deoptimization";
    auto loop = GetLoopForBoundsCheck(block, lenArray, index);
    visitor->PushNewBoundsCheck(loop, inst, {lenArray, index}, !correctUpper, !correctLower);
}

void ChecksElimination::VisitCheckCast(GraphVisitor *v, Inst *inst)
{
    auto visitor = static_cast<ChecksElimination *>(v);
    auto result = ObjectTypeCheckElimination::TryEliminateCheckCast(inst);
    if (result != ObjectTypeCheckElimination::CheckCastEliminateType::INVALID) {
        visitor->SetApplied();
        if (result == ObjectTypeCheckElimination::CheckCastEliminateType::MUST_THROW) {
            visitor->PushNewCheckMustThrow(inst);
        }
        return;
    }
    if (!inst->CastToCheckCast()->GetOmitNullCheck()) {
        auto block = inst->GetBasicBlock();
        auto bri = block->GetGraph()->GetBoundsRangeInfo();
        auto inputRange = bri->FindBoundsRange(block, inst->GetInput(0).GetInst());
        if (inputRange.IsMore(BoundsRange(0))) {
            visitor->SetApplied();
            inst->CastToCheckCast()->SetOmitNullCheck(true);
        }
    }
    visitor->PushNewCheckForMoveOutOfLoop(inst);
}

void ChecksElimination::VisitIsInstance(GraphVisitor *v, Inst *inst)
{
    if (inst->CastToIsInstance()->GetOmitNullCheck()) {
        return;
    }
    auto block = inst->GetBasicBlock();
    auto bri = block->GetGraph()->GetBoundsRangeInfo();
    auto inputRange = bri->FindBoundsRange(block, inst->GetInput(0).GetInst());
    if (inputRange.IsMore(BoundsRange(0))) {
        auto visitor = static_cast<ChecksElimination *>(v);
        visitor->SetApplied();
        inst->CastToIsInstance()->SetOmitNullCheck(true);
    }
}

void ChecksElimination::VisitAddOverflowCheck(GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetType() == DataType::INT32);
    auto input1 = inst->GetInput(0).GetInst();
    auto input2 = inst->GetInput(1).GetInst();
    auto visitor = static_cast<ChecksElimination *>(v);
    visitor->TryRemoveDominatedChecks<Opcode::AddOverflowCheck>(inst, [input1, input2](Inst *userInst) {
        return (userInst->GetInput(0) == input1 && userInst->GetInput(1) == input2) ||
               (userInst->GetInput(0) == input2 && userInst->GetInput(1) == input1);
    });
    visitor->TryOptimizeOverflowCheck<Opcode::AddOverflowCheck>(inst);
}

void ChecksElimination::VisitSubOverflowCheck(GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetType() == DataType::INT32);
    auto input1 = inst->GetInput(0).GetInst();
    auto input2 = inst->GetInput(1).GetInst();
    auto visitor = static_cast<ChecksElimination *>(v);
    visitor->TryRemoveDominatedChecks<Opcode::SubOverflowCheck>(inst, [input1, input2](Inst *userInst) {
        return (userInst->GetInput(0) == input1 && userInst->GetInput(1) == input2);
    });
    visitor->TryOptimizeOverflowCheck<Opcode::SubOverflowCheck>(inst);
}

void ChecksElimination::VisitNegOverflowAndZeroCheck(GraphVisitor *v, Inst *inst)
{
    ASSERT(inst->GetType() == DataType::INT32);
    auto input1 = inst->GetInput(0).GetInst();
    auto visitor = static_cast<ChecksElimination *>(v);
    visitor->TryRemoveDominatedChecks<Opcode::NegOverflowAndZeroCheck>(
        inst, [input1](Inst *userInst) { return (userInst->GetInput(0) == input1); });
    visitor->TryOptimizeOverflowCheck<Opcode::NegOverflowAndZeroCheck>(inst);
}

void ChecksElimination::ReplaceUsersAndRemoveCheck(Inst *instDel, Inst *instRep)
{
    auto block = instDel->GetBasicBlock();
    auto graph = block->GetGraph();
    if (HasOsrEntryBetween(instRep, instDel)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Check couldn't be deleted, because in OSR mode we can't replace "
                                            "instructions with instructions placed before OSR entry";
        return;
    }
    instDel->ReplaceUsers(instRep);
    instDel->RemoveInputs();
    block->ReplaceInst(instDel, graph->CreateInstNOP());
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Checks elimination delete " << GetOpcodeString(instDel->GetOpcode())
                                     << " with id " << instDel->GetId();
    graph->GetEventWriter().EventChecksElimination(GetOpcodeString(instDel->GetOpcode()), instDel->GetId(),
                                                   instDel->GetPc());
    SetApplied();
}

bool ChecksElimination::IsInstIncOrDec(Inst *inst)
{
    return inst->IsAddSub() && inst->GetInput(1).GetInst()->IsConst();
}

int64_t ChecksElimination::GetInc(Inst *inst)
{
    ASSERT(IsInstIncOrDec(inst));
    auto val = static_cast<int64_t>(inst->GetInput(1).GetInst()->CastToConstant()->GetIntValue());
    if (inst->IsSub()) {
        val = -val;
    }
    return val;
}

Loop *ChecksElimination::GetLoopForBoundsCheck(BasicBlock *block, Inst *lenArray, Inst *index)
{
    auto parentIndex = IsInstIncOrDec(index) ? index->GetInput(0).GetInst() : index;
    auto indexBlock = parentIndex->GetBasicBlock();
    ASSERT(indexBlock != nullptr);
    auto indexLoop = indexBlock->GetLoop();
    if (auto loopInfo = CountableLoopParser(*indexLoop).Parse()) {
        auto input = lenArray;
        if (lenArray->GetOpcode() == Opcode::LenArray) {
            // new lenArray can be inserted
            input = lenArray->GetDataFlowInput(0);
        }
        if (loopInfo->index == parentIndex && input->GetBasicBlock()->IsDominate(indexBlock)) {
            ASSERT(indexBlock == indexLoop->GetHeader());
            return indexLoop;
        }
    }
    return block->GetLoop();
}

void ChecksElimination::InitItemForNewIndex(GroupedBoundsChecks *place, Inst *index, Inst *inst, bool checkUpper,
                                            bool checkLower)
{
    ASSERT(inst->GetOpcode() == Opcode::BoundsCheck);
    InstVector insts(GetGraph()->GetLocalAllocator()->Adapter());
    insts.push_back(inst);
    int64_t val = 0;
    Inst *parentIndex = index;
    if (IsInstIncOrDec(index)) {
        val = GetInc(index);
        parentIndex = index->GetInput(0).GetInst();
    } else if (index->IsConst()) {
        val = static_cast<int64_t>(index->CastToConstant()->GetIntValue());
        parentIndex = nullptr;
    }
    auto maxVal = checkUpper ? val : std::numeric_limits<int64_t>::min();
    auto minVal = checkLower ? val : std::numeric_limits<int64_t>::max();
    place->emplace_back(std::make_tuple(parentIndex, insts, maxVal, minVal));
}

void ChecksElimination::PushNewBoundsCheck(Loop *loop, Inst *inst, InstPair helpers, bool checkUpper, bool checkLower)
{
    auto *lenArray = helpers.first;
    auto *index = helpers.second;
    ASSERT(loop != nullptr && lenArray != nullptr && index != nullptr && inst != nullptr);
    ASSERT(inst->GetOpcode() == Opcode::BoundsCheck);
    auto loopChecksIt =
        std::find_if(boundsChecks_.begin(), boundsChecks_.end(), [=](auto p) { return p.first == loop; });
    if (loopChecksIt == boundsChecks_.end()) {
        auto &loopLenArrays = boundsChecks_.emplace_back(loop, GetGraph()->GetLocalAllocator()->Adapter());
        auto &lenArrayIndexes = loopLenArrays.second.emplace_back(lenArray, GetGraph()->GetLocalAllocator()->Adapter());
        InitItemForNewIndex(&lenArrayIndexes.second, index, inst, checkUpper, checkLower);
    } else {
        auto *lenArrays = &loopChecksIt->second;
        auto lenArrayIt =
            std::find_if(lenArrays->begin(), lenArrays->end(), [lenArray](auto p) { return p.first == lenArray; });
        if (lenArrayIt == lenArrays->end()) {
            auto &lenArrayIndexes = lenArrays->emplace_back(lenArray, GetGraph()->GetLocalAllocator()->Adapter());
            InitItemForNewIndex(&lenArrayIndexes.second, index, inst, checkUpper, checkLower);
        } else {
            auto *indexes = &lenArrayIt->second;
            PushNewBoundsCheckAtExistingIndexes(indexes, index, inst, checkUpper, checkLower);
        }
    }
}

void ChecksElimination::PushNewBoundsCheckAtExistingIndexes(GroupedBoundsChecks *indexes, Inst *index, Inst *inst,
                                                            bool checkUpper, bool checkLower)
{
    auto indexIt = std::find_if(indexes->begin(), indexes->end(), [index](auto p) { return std::get<0>(p) == index; });
    if (indexIt == indexes->end()) {
        auto parentIndex = index;
        int64_t val {};
        if (IsInstIncOrDec(index)) {
            parentIndex = index->GetInput(0).GetInst();
            val = GetInc(index);
        } else if (index->IsConst()) {
            parentIndex = nullptr;
            val = static_cast<int64_t>(index->CastToConstant()->GetIntValue());
        }
        auto parentIndexIt =
            std::find_if(indexes->begin(), indexes->end(), [=](auto p) { return std::get<0>(p) == parentIndex; });
        if (parentIndex == index || parentIndexIt == indexes->end()) {
            InitItemForNewIndex(indexes, index, inst, checkUpper, checkLower);
        } else {
            auto &boundChecks = std::get<1U>(*parentIndexIt);
            auto &max = std::get<2U>(*parentIndexIt);
            auto &min = std::get<3U>(*parentIndexIt);
            boundChecks.push_back(inst);
            if (val > max && checkUpper) {
                max = val;
            } else if (val < min && checkLower) {
                min = val;
            }
        }
    } else {
        auto &boundChecks = std::get<1U>(*indexIt);
        auto &max = std::get<2U>(*indexIt);
        auto &min = std::get<3U>(*indexIt);
        boundChecks.push_back(inst);
        if (max < 0 && checkUpper) {
            max = 0;
        }
        if (min > 0 && checkLower) {
            min = 0;
        }
    }
}

void ChecksElimination::TryRemoveDominatedNullChecks(Inst *inst, Inst *ref)
{
    for (auto &user : ref->GetUsers()) {
        auto userInst = user.GetInst();
        if (((userInst->GetOpcode() == Opcode::IsInstance && !userInst->CastToIsInstance()->GetOmitNullCheck()) ||
             (userInst->GetOpcode() == Opcode::CheckCast && !userInst->CastToCheckCast()->GetOmitNullCheck())) &&
            inst->IsDominate(userInst)) {
            COMPILER_LOG(DEBUG, CHECKS_ELIM)
                << "NullCheck with id = " << inst->GetId() << " dominate on " << GetOpcodeString(userInst->GetOpcode())
                << " with id = " << userInst->GetId();
            if (userInst->GetOpcode() == Opcode::IsInstance) {
                userInst->CastToIsInstance()->SetOmitNullCheck(true);
            } else {
                userInst->CastToCheckCast()->SetOmitNullCheck(true);
            }
            SetApplied();
        }
    }
}

template <Opcode OPC, bool CHECK_FULL_DOM, typename CheckInputs>
void ChecksElimination::TryRemoveDominatedCheck(Inst *inst, Inst *userInst, CheckInputs checkInputs)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    if (userInst->GetOpcode() == OPC && userInst != inst && userInst->GetType() == inst->GetType() &&
        checkInputs(userInst) &&
        (CHECK_FULL_DOM ? inst->IsDominate(userInst) : inst->InSameBlockOrDominate(userInst))) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM)
            // NOLINTNEXTLINE(readability-magic-numbers)
            << GetOpcodeString(OPC) << " with id = " << inst->GetId() << " dominate on " << GetOpcodeString(OPC)
            << " with id = " << userInst->GetId();
        ReplaceUsersAndRemoveCheck(userInst, inst);
    }
}

template <Opcode OPC, bool CHECK_FULL_DOM, typename CheckInputs>
void ChecksElimination::TryRemoveDominatedChecks(Inst *inst, CheckInputs checkInputs)
{
    for (auto &directUser : inst->GetInput(0).GetInst()->GetUsers()) {
        auto directUserInst = directUser.GetInst();
        if ((OPC != Opcode::NullCheck) && (directUserInst->GetOpcode() == Opcode::NullCheck)) {
            for (auto &actualUser : directUserInst->GetUsers()) {
                auto actualUserInst = actualUser.GetInst();
                TryRemoveDominatedCheck<OPC, CHECK_FULL_DOM>(inst, actualUserInst, checkInputs);
            }
        } else {
            TryRemoveDominatedCheck<OPC, CHECK_FULL_DOM>(inst, directUserInst, checkInputs);
        }
    }
}

// Remove consecutive checks: NullCheck -> NullCheck -> NullCheck
template <Opcode OPC>
void ChecksElimination::TryRemoveConsecutiveChecks(Inst *inst)
{
    auto end = inst->GetUsers().end();
    for (auto user = inst->GetUsers().begin(); user != end;) {
        auto userInst = (*user).GetInst();
        // NOLINTNEXTLINE(readability-magic-numbers)
        if (userInst->GetOpcode() == OPC) {
            // NOLINTNEXTLINE(readability-magic-numbers)
            COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Remove consecutive " << GetOpcodeString(OPC);
            ReplaceUsersAndRemoveCheck(userInst, inst);
            // Start iteration from beginning, because the new successors may be added.
            user = inst->GetUsers().begin();
            end = inst->GetUsers().end();
        } else {
            ++user;
        }
    }
}

template <Opcode OPC>
bool ChecksElimination::TryRemoveCheckByBounds(Inst *inst, Inst *input)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    static_assert(OPC == Opcode::ZeroCheck || OPC == Opcode::NegativeCheck || OPC == Opcode::NotPositiveCheck ||
                  OPC == Opcode::NullCheck);
    ASSERT(inst->GetOpcode() == OPC);
    auto block = inst->GetBasicBlock();
    auto bri = block->GetGraph()->GetBoundsRangeInfo();

    auto range = bri->FindBoundsRange(block, input);
    auto rangeChecker = [](const BoundsRange &r) {
        // NOLINTNEXTLINE(readability-magic-numbers, readability-braces-around-statements, bugprone-branch-clone)
        if constexpr (OPC == Opcode::ZeroCheck) {
            return r.IsLess(BoundsRange(0)) || r.IsMore(BoundsRange(0));
        } else if constexpr (OPC == Opcode::NullCheck) {  // NOLINT
            return r.IsMore(BoundsRange(0));
        } else if constexpr (OPC == Opcode::NegativeCheck) {  // NOLINT
            return r.IsNotNegative();
        } else if constexpr (OPC == Opcode::NotPositiveCheck) {  // NOLINT
            return r.IsPositive();
        }
    };
    bool result = rangeChecker(range);
    if (result) {
        // NOLINTNEXTLINE(readability-magic-numbers)
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << GetOpcodeString(OPC) << " have correct bounds";

        // As bounds analysis was used to prove the check's excessiveness, users should be moved with care:
        CheckAndSetFlagNoHoist(inst, input, rangeChecker);
        ReplaceUsersAndRemoveCheck(inst, input);
    } else {
        // NOLINTNEXTLINE(readability-magic-numbers, readability-braces-around-statements)
        if constexpr (OPC == Opcode::ZeroCheck || OPC == Opcode::NullCheck) {
            result = range.IsEqual(BoundsRange(0));
        } else if constexpr (OPC == Opcode::NegativeCheck) {  // NOLINT
            result = range.IsNegative();
        } else if constexpr (OPC == Opcode::NotPositiveCheck) {  // NOLINT
            result = range.IsNotPositive();
        }
        if (result) {
            COMPILER_LOG(DEBUG, CHECKS_ELIM)
                // NOLINTNEXTLINE(readability-magic-numbers)
                << GetOpcodeString(OPC) << " must throw, saved for replace by unconditional deoptimize";
            PushNewCheckMustThrow(inst);
        }
    }
    return result;
}

template <Opcode OPC, bool CHECK_FULL_DOM>
bool ChecksElimination::TryRemoveCheck(Inst *inst)
{
    // NOLINTNEXTLINE(readability-magic-numbers)
    static_assert(OPC == Opcode::ZeroCheck || OPC == Opcode::NegativeCheck || OPC == Opcode::NotPositiveCheck ||
                  OPC == Opcode::NullCheck);
    ASSERT(inst->GetOpcode() == OPC);

    // NOLINTNEXTLINE(readability-magic-numbers)
    TryRemoveDominatedChecks<OPC, CHECK_FULL_DOM>(inst);
    // NOLINTNEXTLINE(readability-magic-numbers)
    TryRemoveConsecutiveChecks<OPC>(inst);

    auto input = inst->GetInput(0).GetInst();
    // NOLINTNEXTLINE(readability-magic-numbers)
    return TryRemoveCheckByBounds<OPC>(inst, input);
}

template <Opcode OPC>
void ChecksElimination::TryOptimizeOverflowCheck(Inst *inst)
{
    auto block = inst->GetBasicBlock();
    auto bri = block->GetGraph()->GetBoundsRangeInfo();
    auto range = bri->FindBoundsRange(block, inst);
    bool canOverflow = true;
    if constexpr (OPC == Opcode::NegOverflowAndZeroCheck) {
        canOverflow = range.CanOverflowNeg(DataType::INT32);
    } else {
        canOverflow = range.CanOverflow(DataType::INT32);
    }
    if (!canOverflow) {
        block->RemoveOverflowCheck(inst);
        SetApplied();
        return;
    }
    bool constInputs = true;
    for (size_t i = 0; i < inst->GetInputsCount() - 1; ++i) {
        constInputs &= inst->GetInput(i).GetInst()->IsConst();
    }
    if (constInputs) {
        // replace by deopt
        PushNewCheckMustThrow(inst);
        return;
    }
    PushNewCheckForMoveOutOfLoop(inst);
}

Inst *ChecksElimination::FindSaveState(Loop *loop)
{
    auto block = loop->GetPreHeader();
    while (block != nullptr) {
        for (const auto &inst : block->InstsSafeReverse()) {
            if (inst->GetOpcode() == Opcode::SaveStateDeoptimize || inst->GetOpcode() == Opcode::SaveState) {
                return inst;
            }
        }
        auto next = block->GetDominator();
        // The case when the dominant block is the head of a inner loop
        if (next != nullptr && next->GetLoop()->GetOuterLoop() == block->GetLoop()) {
            return nullptr;
        }
        block = next;
    }
    return nullptr;
}

Inst *ChecksElimination::FindOptimalSaveStateForHoist(Inst *inst, Inst **optimalInsertAfter)
{
    ASSERT(inst->RequireState());
    auto block = inst->GetBasicBlock();
    if (block == nullptr) {
        return nullptr;
    }
    auto loop = block->GetLoop();
    *optimalInsertAfter = nullptr;

    while (!loop->IsRoot() && !loop->GetHeader()->IsOsrEntry() && !loop->IsIrreducible()) {
        for (auto backEdge : loop->GetBackEdges()) {
            if (!block->IsDominate(backEdge)) {
                // avoid taking checks out of slowpath
                return *optimalInsertAfter;
            }
        }
        // Find save state
        Inst *ss = FindSaveState(loop);
        if (ss == nullptr) {
            return *optimalInsertAfter;
        }
        auto insertAfter = ss;

        // Check that inputs are dominate on ss
        bool inputsAreDominate = true;
        for (size_t i = 0; i < inst->GetInputsCount() - 1; ++i) {
            auto input = inst->GetInput(i).GetInst();
            if (input->IsDominate(insertAfter)) {
                continue;
            }
            if (insertAfter->GetBasicBlock() == input->GetBasicBlock()) {
                insertAfter = input;
            } else {
                inputsAreDominate = false;
                break;
            }
        }

        if (!inputsAreDominate) {
            return *optimalInsertAfter;
        }
        *optimalInsertAfter = insertAfter;
        if (insertAfter != ss) {
            // some inputs are dominate on insert_after but not dominate on ss, stop here
            // the only case when return value is not equal to *optimal_insert_after
            return ss;
        }
        block = loop->GetHeader();  // block will be used to check for hot path
        loop = loop->GetOuterLoop();
    }
    return *optimalInsertAfter;
}

void ChecksElimination::InsertInstAfter(Inst *inst, Inst *after, BasicBlock *block)
{
    if (after->IsPhi()) {
        block->PrependInst(inst);
    } else {
        block->InsertAfter(inst, after);
    }
}

void ChecksElimination::InsertBoundsCheckDeoptimization(ConditionCode cc, InstPair args, int64_t val, InstPair helpers,
                                                        Opcode newLeftOpcode)
{
    auto [left, right] = args;
    auto [ss, insertAfter] = helpers;
    auto block = insertAfter->GetBasicBlock();
    Inst *newLeft = nullptr;
    if (val == 0 && left != nullptr) {
        newLeft = left;
    } else if (left == nullptr) {
        ASSERT(newLeftOpcode == Opcode::Add);
        newLeft = GetGraph()->FindOrCreateConstant(val);
    } else {
        auto cnst = GetGraph()->FindOrCreateConstant(val);
        newLeft = GetGraph()->CreateInst(newLeftOpcode);
        newLeft->SetType(DataType::INT32);
        newLeft->SetInput(0, left);
        newLeft->SetInput(1, cnst);
        if (newLeft->RequireState()) {
            newLeft->SetSaveState(ss);
        }
        InsertInstAfter(newLeft, insertAfter, block);
        insertAfter = newLeft;
    }
    auto deoptComp = GetGraph()->CreateInstCompare(DataType::BOOL, INVALID_PC, newLeft, right, DataType::INT32, cc);
    auto deopt = GetGraph()->CreateInstDeoptimizeIf(ss->GetPc(), deoptComp, ss, DeoptimizeType::BOUNDS_CHECK);
    InsertInstAfter(deoptComp, insertAfter, block);
    block->InsertAfter(deopt, deoptComp);
}

std::optional<LoopInfo> ChecksElimination::FindLoopInfo(Loop *loop)
{
    Inst *ss = FindSaveState(loop);
    if (ss == nullptr) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "SaveState isn't founded";
        return std::nullopt;
    }
    auto loopParser = CountableLoopParser(*loop);
    if (auto loopInfo = loopParser.Parse()) {
        auto loopInfoValue = loopInfo.value();
        if (loopInfoValue.normalizedCc == CC_NE) {
            return std::nullopt;
        }
        bool isHeadLoopExit = loopInfoValue.ifImm->GetBasicBlock() == loop->GetHeader();
        bool hasPreHeaderCompare = CountableLoopParser::HasPreHeaderCompare(loop, loopInfoValue);
        ASSERT(loopInfoValue.index->GetOpcode() == Opcode::Phi);
        if (loopInfoValue.isInc) {
            return std::make_tuple(loopInfoValue, ss, loopInfoValue.init, loopInfoValue.test,
                                   loopInfoValue.normalizedCc == CC_LE ? CC_LE : CC_LT, isHeadLoopExit,
                                   hasPreHeaderCompare);
        }
        return std::make_tuple(loopInfoValue, ss, loopInfoValue.test, loopInfoValue.init, CC_LE, isHeadLoopExit,
                               hasPreHeaderCompare);
    }
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Not countable loop isn't supported";
    return std::nullopt;
}

Inst *ChecksElimination::InsertNewLenArray(Inst *lenArray, Inst *ss)
{
    if (lenArray->IsDominate(ss)) {
        return lenArray;
    }
    if (lenArray->GetOpcode() == Opcode::LenArray) {
        auto nullCheck = lenArray->GetInput(0).GetInst();
        auto ref = lenArray->GetDataFlowInput(nullCheck);
        if (ref->IsDominate(ss)) {
            // Build nullcheck + lenarray before loop
            auto nullcheck = GetGraph()->CreateInstNullCheck(DataType::REFERENCE, ss->GetPc(), ref, ss);
            nullcheck->SetFlag(inst_flags::CAN_DEOPTIMIZE);
            auto newLenArray = lenArray->Clone(GetGraph());
            newLenArray->SetInput(0, nullcheck);
            auto block = ss->GetBasicBlock();
            block->InsertAfter(newLenArray, ss);
            block->InsertAfter(nullcheck, ss);
            COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Builded new NullCheck(id=" << nullcheck->GetId()
                                             << ") and LenArray(id=" << newLenArray->GetId() << ") before loop";
            ChecksElimination::VisitNullCheck<true>(this, nullcheck);
            return newLenArray;
        }
    }
    return nullptr;
}

void ChecksElimination::InsertDeoptimizationForIndexOverflow(CountableLoopInfo *countableLoopInfo,
                                                             BoundsRange indexUpperRange, Inst *ss)
{
    auto loopCc = countableLoopInfo->normalizedCc;
    if (loopCc == CC_LT || loopCc == CC_LE) {
        auto loopUpper = countableLoopInfo->test;
        ASSERT(countableLoopInfo->constStep < INT64_MAX);
        auto step = static_cast<int64_t>(countableLoopInfo->constStep);
        auto indexType = countableLoopInfo->index->GetType();
        ASSERT(indexType == DataType::INT32);
        auto maxUpper = BoundsRange::GetMax(indexType) - step + (loopCc == CC_LT ? 1 : 0);
        auto bri = loopUpper->GetBasicBlock()->GetGraph()->GetBoundsRangeInfo();
        auto loopUpperRange = bri->FindBoundsRange(countableLoopInfo->index->GetBasicBlock(), loopUpper);
        // Upper bound of loop index assuming (index + maxAdd < lenArray)
        indexUpperRange = indexUpperRange.Add(BoundsRange(step)).FitInType(indexType);
        if (!BoundsRange(maxUpper).IsMoreOrEqual(loopUpperRange) && indexUpperRange.IsMaxRange(indexType)) {
            // loop index can overflow
            Inst *insertAfter = loopUpper->IsDominate(ss) ? ss : loopUpper;
            COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Build deoptimize for loop index overflow";
            // Create deoptimize if loop index can become negative
            InsertBoundsCheckDeoptimization(ConditionCode::CC_LT, {nullptr, loopUpper}, maxUpper, {ss, insertAfter});
        }
    }
}

bool ChecksElimination::NeedUpperDeoptimization(BasicBlock *header, InstPair insts, FlagPair flags, int64_t maxAdd,
                                                bool *insertNewLenArray)
{
    auto *lenArray = insts.first;
    auto *upper = insts.second;
    auto [ccIsLt, upperRangeIsMax] = flags;
    auto bri = GetGraph()->GetBoundsRangeInfo();
    auto newUpper = upper;
    auto newMaxAdd = maxAdd;
    int64_t upperAdd = 0;
    if (IsInstIncOrDec(upper)) {
        upperAdd = GetInc(upper);
        newMaxAdd += upperAdd;
        newUpper = upper->GetInput(0).GetInst();
    }
    if (lenArray == newUpper) {
        if (newMaxAdd < 0 || (newMaxAdd == 0 && ccIsLt)) {
            return false;
        }
    }
    auto useUpperLen = upperAdd >= 0 || !upperRangeIsMax;
    if (useUpperLen && newMaxAdd <= 0) {
        auto newUpperRange = bri->FindBoundsRange(header, newUpper);
        if (newUpperRange.GetLenArray() == lenArray) {
            return false;
        }
    }
    *insertNewLenArray = newUpper != lenArray;
    return true;
}

bool ChecksElimination::TryInsertDeoptimizationForLargeStep(ConditionCode cc, InstPair bounds, InstTriple helpers,
                                                            int64_t maxAdd, uint64_t constStep)
{
    auto [lower, upper] = bounds;
    auto [insertDeoptAfter, ss, resultLenArray] = helpers;
    auto block = insertDeoptAfter->GetBasicBlock();
    if (!lower->IsDominate(insertDeoptAfter)) {
        if (lower->GetBasicBlock() == block) {
            insertDeoptAfter = lower;
        } else {
            return false;
        }
    }
    auto subValue = lower;
    if (cc == CC_LT) {
        subValue = GetGraph()->CreateInstAdd(DataType::INT32, INVALID_PC, lower, GetGraph()->FindOrCreateConstant(1));
        InsertInstAfter(subValue, insertDeoptAfter, block);
        insertDeoptAfter = subValue;
    }
    auto sub = GetGraph()->CreateInstSub(DataType::INT32, INVALID_PC, upper, subValue);
    InsertInstAfter(sub, insertDeoptAfter, block);
    auto mod = GetGraph()->CreateInstMod(DataType::INT32, INVALID_PC, sub, GetGraph()->FindOrCreateConstant(constStep));
    block->InsertAfter(mod, sub);
    if (resultLenArray == upper) {
        auto maxAddConst = GetGraph()->FindOrCreateConstant(maxAdd);
        // (upper - lower [- 1]) % step </<= maxAdd
        InsertBoundsCheckDeoptimization(cc, {mod, maxAddConst}, 0, {ss, mod}, Opcode::NOP);
    } else {
        // result_len_array - maxAdd </<= upper - (upper - lower [- 1]) % step
        auto maxIndexValue = GetGraph()->CreateInstSub(DataType::INT32, INVALID_PC, upper, mod);
        block->InsertAfter(maxIndexValue, mod);
        auto opcode = maxAdd > 0 ? Opcode::Sub : Opcode::SubOverflowCheck;
        InsertBoundsCheckDeoptimization(cc, {resultLenArray, maxIndexValue}, maxAdd, {ss, maxIndexValue}, opcode);
    }
    return true;
}

bool ChecksElimination::TryInsertDeoptimization(LoopInfo loopInfo, Inst *lenArray, int64_t maxAdd, int64_t minAdd,
                                                bool hasCheckInHeader)
{
    auto [countableLoopInfo, ss, lower, upper, cc, isHeadLoopExit, hasPreHeaderCompare] = loopInfo;
    ASSERT(cc == CC_LT || cc == CC_LE);
    auto bri = GetGraph()->GetBoundsRangeInfo();
    auto header = countableLoopInfo.index->GetBasicBlock();
    auto upperRange = bri->FindBoundsRange(header, upper);
    auto lowerRange = bri->FindBoundsRange(header, lower);
    auto lenArrayRange = bri->FindBoundsRange(header, lenArray);
    auto hasCheckBeforeExit = hasCheckInHeader || !isHeadLoopExit;
    if (!hasPreHeaderCompare && !lowerRange.IsLess(upperRange) && hasCheckBeforeExit) {
        // if lower > upper, removing BoundsCheck may be wrong for the first iteration
        return false;
    }
    uint64_t lowerInc = (countableLoopInfo.normalizedCc == CC_GT ? 1 : 0);
    bool needLowerDeopt = (minAdd != std::numeric_limits<int64_t>::max()) &&
                          !lowerRange.Add(BoundsRange(minAdd)).Add(BoundsRange(lowerInc)).IsNotNegative();
    bool insertLowerDeopt = lower->IsDominate(ss);
    if (needLowerDeopt && !insertLowerDeopt) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Unable to build deoptimize for lower value";
        return false;
    }

    bool insertNewLenArray = false;
    bool correctMax = (maxAdd != std::numeric_limits<int64_t>::min());
    bool correctUpperRange = upperRange.Add(BoundsRange(maxAdd)).IsLess(lenArrayRange);
    bool upperRangeIsMax = upperRange.IsMaxRange(upper->GetType());
    if (correctMax && !correctUpperRange &&
        NeedUpperDeoptimization(header, {lenArray, upper}, {cc == CC_LT, upperRangeIsMax}, maxAdd,
                                &insertNewLenArray)) {
        if (!TryInsertUpperDeoptimization(loopInfo, lenArray, lowerRange, maxAdd, insertNewLenArray)) {
            return false;
        }
    }
    InsertDeoptimizationForIndexOverflow(&countableLoopInfo, lenArrayRange.Sub(BoundsRange(maxAdd)), ss);
    if (needLowerDeopt) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Build deoptimize for lower value";
        // Create deoptimize if lower < 0 (or -1 for loop with CC_GT)
        auto lowerConst = GetGraph()->FindOrCreateConstant(-lowerInc);
        InsertBoundsCheckDeoptimization(ConditionCode::CC_LT, {lower, lowerConst}, minAdd, {ss, ss});
    }
    return true;
}

bool ChecksElimination::TryInsertUpperDeoptimization(LoopInfo loopInfo, Inst *lenArray, BoundsRange lowerRange,
                                                     int64_t maxAdd, bool insertNewLenArray)
{
    [[maybe_unused]] auto [countableLoopInfo, ss, lower, upper, cc, isHeadLoopExit, hasPreHeaderCompare] = loopInfo;
    auto header = countableLoopInfo.index->GetBasicBlock();
    auto resultLenArray = insertNewLenArray ? InsertNewLenArray(lenArray, ss) : lenArray;
    if (resultLenArray == nullptr) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Unable to build deoptimize for upper value";
        return false;
    }
    auto insertDeoptAfter = lenArray != resultLenArray ? resultLenArray : ss;
    if (!upper->IsDominate(insertDeoptAfter)) {
        insertDeoptAfter = upper;
    }
    ASSERT(insertDeoptAfter->GetBasicBlock()->IsDominate(header));
    if (insertDeoptAfter->GetBasicBlock() == header) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Unable to build deoptimize for upper value";
        return false;
    }
    auto constStep = countableLoopInfo.constStep;
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Try to build deoptimize for upper value";
    if (constStep == 1 || (countableLoopInfo.normalizedCc == CC_GT || countableLoopInfo.normalizedCc == CC_GE)) {
        auto opcode = maxAdd > 0 ? Opcode::Sub : Opcode::SubOverflowCheck;
        // Create deoptimize if resultLenArray - maxAdd <=(<) upper
        // resultLenArray is >= 0, so if maxAdd > 0, overflow is not possible
        // that's why we do not add maxAdd to upper instead
        InsertBoundsCheckDeoptimization(cc, {resultLenArray, upper}, maxAdd, {ss, insertDeoptAfter}, opcode);
    } else if (lowerRange.IsConst() && lowerRange.GetLeft() == 0 && countableLoopInfo.normalizedCc == CC_LT &&
               resultLenArray == upper && maxAdd == static_cast<int64_t>(constStep) - 1) {
        // Example: for (int i = 0; i < len; i += x) process(a[i], ..., a[i + x - 1])
        // deoptimize if len % x != 0
        auto zeroConst = GetGraph()->FindOrCreateConstant(0);
        InsertBoundsCheckDeoptimization(ConditionCode::CC_NE, {resultLenArray, zeroConst}, constStep,
                                        {ss, insertDeoptAfter}, Opcode::Mod);
    } else if (!TryInsertDeoptimizationForLargeStep(cc, {lower, upper}, {insertDeoptAfter, ss, resultLenArray}, maxAdd,
                                                    constStep)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Unable to build deoptimize for upper value with step > 1";
        return false;
    }
    return true;
}

void ChecksElimination::HoistLoopInvariantBoundsChecks(Inst *lenArray, GroupedBoundsChecks *indexBoundschecks,
                                                       Loop *loop)
{
    auto lenArrLoop = lenArray->GetBasicBlock()->GetLoop();
    // lenArray isn't loop invariant
    if (lenArrLoop == loop) {
        return;
    }
    for (auto &[index, boundChecks, max, min] : *indexBoundschecks) {
        // Check that index is loop invariant, if index is nullptr it means that it was a constant
        if (index != nullptr && index->GetBasicBlock()->GetLoop() == loop) {
            continue;
        }
        for (auto boundsCheck : boundChecks) {
            PushNewCheckForMoveOutOfLoop(boundsCheck);
        }
    }
}

void ChecksElimination::ProcessingGroupBoundsCheck(GroupedBoundsChecks *indexBoundschecks, LoopInfo loopInfo,
                                                   Inst *lenArray)
{
    auto phiIndex = std::get<0>(loopInfo).index;
    auto phiIndexIt = std::find_if(indexBoundschecks->begin(), indexBoundschecks->end(),
                                   [=](auto p) { return std::get<0>(p) == phiIndex; });
    if (phiIndexIt == indexBoundschecks->end()) {
        HoistLoopInvariantBoundsChecks(lenArray, indexBoundschecks, phiIndex->GetBasicBlock()->GetLoop());
        return;
    }
    const auto &instsToDelete = std::get<1U>(*phiIndexIt);
    auto maxAdd = std::get<2U>(*phiIndexIt);
    auto minAdd = std::get<3U>(*phiIndexIt);
    ASSERT(!instsToDelete.empty());
    bool hasCheckInHeader = false;
    for (const auto &inst : instsToDelete) {
        if (inst->GetBasicBlock() == phiIndex->GetBasicBlock()) {
            hasCheckInHeader = true;
        }
    }
    if (TryInsertDeoptimization(loopInfo, lenArray, maxAdd, minAdd, hasCheckInHeader)) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Delete group of BoundsChecks";
        // Delete bounds checks instructions
        for (const auto &inst : instsToDelete) {
            ReplaceUsersAndRemoveCheck(inst, inst->GetInput(1).GetInst());
            auto it = std::find_if(indexBoundschecks->begin(), indexBoundschecks->end(),
                                   [=](auto p) { return std::get<0>(p) == inst->GetInput(1).GetInst(); });
            if (it != indexBoundschecks->end()) {
                std::get<0>(*it) = nullptr;
                std::get<1>(*it).clear();
            }
        }
    }
}

void ChecksElimination::ProcessingLoop(Loop *loop, LoopNotFullyRedundantBoundsCheck *lenarrIndexChecks)
{
    auto loopInfo = FindLoopInfo(loop);
    if (loopInfo == std::nullopt) {
        return;
    }
    for (auto it = lenarrIndexChecks->rbegin(); it != lenarrIndexChecks->rend(); it++) {
        ProcessingGroupBoundsCheck(&it->second, loopInfo.value(), it->first);
    }
}

void ChecksElimination::ReplaceBoundsCheckToDeoptimizationBeforeLoop()
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start ReplaceBoundsCheckToDeoptimizationBeforeLoop";
    for (auto &[loop, lenarrIndexChecks] : boundsChecks_) {
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Processing loop with id = " << loop->GetId();
        if (loop->IsRoot()) {
            COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Skip root loop";
            continue;
        }
        ProcessingLoop(loop, &lenarrIndexChecks);
    }
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Finish ReplaceBoundsCheckToDeoptimizationBeforeLoop";
}

void ChecksElimination::MoveCheckOutOfLoop()
{
    for (auto inst : checksForMoveOutOfLoop_) {
        Inst *insertAfter = nullptr;
        auto ss = FindOptimalSaveStateForHoist(inst, &insertAfter);
        if (ss == nullptr) {
            continue;
        }
        if (inst->GetOpcode() == Opcode::CheckCast) {
            // input object can become null after hoisting
            inst->CastToCheckCast()->SetOmitNullCheck(false);
        }
        ASSERT(insertAfter != nullptr);
        ASSERT(ss->GetBasicBlock() == insertAfter->GetBasicBlock());
        auto block = inst->GetBasicBlock();
        COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Move check " << GetOpcodeString(inst->GetOpcode())
                                         << " with id = " << inst->GetId() << " from bb " << block->GetId() << " to bb "
                                         << ss->GetBasicBlock()->GetId();
        block->EraseInst(inst);
        ss->GetBasicBlock()->InsertAfter(inst, insertAfter);
        inst->SetSaveState(ss);
        inst->SetPc(ss->GetPc());
        inst->SetFlag(inst_flags::CAN_DEOPTIMIZE);
        SetApplied();
    }
}

Inst *ChecksElimination::FindSaveState(const InstVector &instsToDelete)
{
    for (auto boundsCheck : instsToDelete) {
        bool isDominate = true;
        for (auto boundsCheckTest : instsToDelete) {
            if (boundsCheck == boundsCheckTest) {
                continue;
            }
            isDominate &= boundsCheck->IsDominate(boundsCheckTest);
        }
        if (isDominate) {
            constexpr auto IMM_2 = 2;
            return boundsCheck->GetInput(IMM_2).GetInst();
        }
    }
    return nullptr;
}

void ChecksElimination::ReplaceOneBoundsCheckToDeoptimizationInLoop(
    std::pair<Loop *, LoopNotFullyRedundantBoundsCheck> &item)
{
    for (auto &[lenArray, indexBoundchecks] : item.second) {
        for (auto &[index, instsToDelete, max, min] : indexBoundchecks) {
            constexpr auto MIN_BOUNDSCHECKS_NUM = 2;
            if (instsToDelete.size() <= MIN_BOUNDSCHECKS_NUM) {
                COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Skip small group of BoundsChecks";
                continue;
            }
            // Try to replace more than 2 bounds checks to deoptimization in loop
            auto saveState = FindSaveState(instsToDelete);
            if (saveState == nullptr) {
                COMPILER_LOG(DEBUG, CHECKS_ELIM) << "SaveState isn't founded";
                continue;
            }
            COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Replace group of BoundsChecks on deoptimization in loop";
            auto insertAfter = lenArray->IsDominate(saveState) ? saveState : lenArray;
            if (max != std::numeric_limits<int64_t>::min()) {
                // Create deoptimize if max_index >= lenArray
                InsertBoundsCheckDeoptimization(ConditionCode::CC_GE, {index, lenArray}, max, {saveState, insertAfter});
            }
            if (index != nullptr && min != std::numeric_limits<int64_t>::max()) {
                // Create deoptimize if min_index < 0
                auto zeroConst = GetGraph()->FindOrCreateConstant(0);
                InsertBoundsCheckDeoptimization(ConditionCode::CC_LT, {index, zeroConst}, min,
                                                {saveState, insertAfter});
            } else {
                // No lower check needed based on BoundsAnalysis
                // if index is null, group of bounds checks consists of constants
                ASSERT(min >= 0);
            }
            for (const auto &inst : instsToDelete) {
                COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Delete group of BoundsChecks";
                ReplaceUsersAndRemoveCheck(inst, inst->GetInput(1).GetInst());
            }
        }
    }
}

void ChecksElimination::ReplaceBoundsCheckToDeoptimizationInLoop()
{
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Start ReplaceBoundsCheckToDeoptimizationInLoop";
    for (auto &item : boundsChecks_) {
        ReplaceOneBoundsCheckToDeoptimizationInLoop(item);
    }
    COMPILER_LOG(DEBUG, CHECKS_ELIM) << "Finish ReplaceBoundsCheckToDeoptimizationInLoop";
}

void ChecksElimination::ReplaceCheckMustThrowByUnconditionalDeoptimize()
{
    for (auto &inst : checksMustThrow_) {
        auto block = inst->GetBasicBlock();
        if (block != nullptr) {
            COMPILER_LOG(DEBUG, CHECKS_ELIM)
                << "Replace check with id = " << inst->GetId() << " by uncondition deoptimize";
            block->ReplaceInstByDeoptimize(inst);
            SetApplied();
            SetLoopDeleted();
        }
    }
}

}  // namespace ark::compiler
