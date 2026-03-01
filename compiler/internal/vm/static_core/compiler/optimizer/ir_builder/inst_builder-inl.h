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

#ifndef PANDA_INST_BUILDER_INL_H
#define PANDA_INST_BUILDER_INL_H

#include "inst_builder.h"
#include "libarkbase/macros.h"
#include "optimizer/code_generator/encode.h"
#include "runtime/include/coretypes/string.h"

namespace ark::compiler {

template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ, bool HAS_SAVE_STATE>
uint32_t InstBuilder::BuildCallHelper<OPCODE, IS_RANGE, ACC_READ, HAS_SAVE_STATE>::GetClassId()
{
    if (method_ == nullptr) {
        return 0;
    }
    if (GetGraph()->IsAotMode()) {
        return GetRuntime()->GetClassIdWithinFile(GetMethod(), GetRuntime()->GetClass(method_));
    }
    return GetRuntime()->GetClassIdForMethod(GetMethod(), methodId_);
}

// NOLINTNEXTLINE(misc-definitions-in-headers,readability-function-size)
template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ, bool HAS_SAVE_STATE>
InstBuilder::BuildCallHelper<OPCODE, IS_RANGE, ACC_READ, HAS_SAVE_STATE>::BuildCallHelper(
    const BytecodeInstruction *bcInst, InstBuilder *builder, Inst *additionalInput)
    : builder_(builder), bcInst_(bcInst)
{
    methodId_ = GetRuntime()->ResolveMethodIndex(Builder()->GetMethod(), bcInst->GetId(0).AsIndex());
    pc_ = Builder()->GetPc(bcInst->GetAddress());
    hasImplicitArg_ = !GetRuntime()->IsMethodStatic(Builder()->GetMethod(), methodId_);

    if (auto *intrinsic = GetRuntime()->GetMethodAsIntrinsic(Builder()->GetMethod(), methodId_)) {
        method_ = intrinsic;
        if (TryBuildIntrinsic()) {
            return;
        }
    }
    // Here "GetMethodById" can be used without additional checks, result may be nullptr and it is normal situation
    method_ = GetRuntime()->GetMethodById(Builder()->GetMethod(), methodId_);
    saveState_ = nullptr;
    uint32_t classId = 0;
    if constexpr (HAS_SAVE_STATE) {
        saveState_ = Builder()->CreateSaveState(Opcode::SaveState, pc_);
        if (hasImplicitArg_) {
            nullCheck_ = GetGraph()->CreateInstNullCheck(DataType::REFERENCE, pc_,
                                                         Builder()->GetArgDefinition(bcInst, 0, ACC_READ), saveState_);
        } else {
            classId = GetClassId();
        }
    } else {
        classId = GetClassId();
    }

    // NOLINTNEXTLINE(readability-magic-numbers)
    BuildCallInst(classId);
    SetCallArgs(additionalInput);

    if constexpr (HAS_SAVE_STATE) {
        // Add SaveState
        if (saveState_ != nullptr) {
            Builder()->AddInstruction(saveState_);
        }
        // Add NullCheck
        if (hasImplicitArg_) {
            ASSERT(nullCheck_ != nullptr);
            Builder()->AddInstruction(nullCheck_);
        } else if (!call_->IsUnresolved() && static_cast<CallInst *>(call_)->GetCallMethod() != nullptr) {
            // Initialize class as call is resolved
            BuildInitClassInstForCallStatic(classId);
        }
        // Add resolver
        if (resolver_ != nullptr) {
            if (call_->IsStaticCall()) {
                resolver_->SetInput(0, saveState_);
            } else {
                resolver_->SetInput(0, nullCheck_);
                resolver_->SetInput(1, saveState_);
            }
            Builder()->AddInstruction(resolver_);
        }
    }
    // Add Call
    AddCallInstruction();
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ, bool HAS_SAVE_STATE>
void InstBuilder::BuildCallHelper<OPCODE, IS_RANGE, ACC_READ, HAS_SAVE_STATE>::SetCallArgs(Inst *additionalInput)
{
    size_t hiddenArgsCount = hasImplicitArg_ ? 1 : 0;                 // +1 for non-static call
    size_t additionalArgsCount = additionalInput == nullptr ? 0 : 1;  // +1 for launch call
    size_t argsCount = Builder()->GetMethodArgumentsCount(methodId_);
    size_t totalArgsCount = hiddenArgsCount + argsCount + additionalArgsCount;
    size_t inputsCount = totalArgsCount + (call_->RequireState() ? 0 : 1) + (resolver_ == nullptr ? 0 : 1);
    call_->ReserveInputs(inputsCount);
    call_->AllocateInputTypes(GetGraph()->GetAllocator(), inputsCount);
    if (resolver_ != nullptr) {
        call_->AppendInput(resolver_);
        call_->AddInputType(DataType::POINTER);
    }
    if (additionalInput != nullptr) {
        ASSERT(call_->RequireState() && saveState_ != nullptr);
        call_->AppendInput(additionalInput);
        call_->AddInputType(DataType::REFERENCE);
        saveState_->AppendBridge(additionalInput);
    }
    if (hasImplicitArg_ && nullCheck_ != nullptr) {
        call_->AppendInput(nullCheck_);
        call_->AddInputType(DataType::REFERENCE);
    }
    if constexpr (!HAS_SAVE_STATE) {
        if (hasImplicitArg_) {
            call_->AppendInput(Builder()->GetArgDefinition(bcInst_, 0, ACC_READ));
            call_->AddInputType(DataType::REFERENCE);
        }
    }
    if constexpr (IS_RANGE) {
        auto startReg = bcInst_->GetVReg(0);
        // start reg for Virtual call was added
        if (hasImplicitArg_) {
            ++startReg;
        }
        for (size_t i = 0; i < argsCount; startReg++, i++) {
            call_->AppendInput(Builder()->GetDefinition(startReg));
            call_->AddInputType(Builder()->GetMethodArgumentType(methodId_, i));
        }
    } else {
        for (size_t i = 0; i < argsCount; i++) {
            call_->AppendInput(Builder()->GetArgDefinition(bcInst_, i + hiddenArgsCount, ACC_READ));
            call_->AddInputType(Builder()->GetMethodArgumentType(methodId_, i));
        }
    }
    if (call_->RequireState()) {
        call_->AppendInput(saveState_);
        call_->AddInputType(DataType::NO_TYPE);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ, bool HAS_SAVE_STATE>
void InstBuilder::BuildCallHelper<OPCODE, IS_RANGE, ACC_READ, HAS_SAVE_STATE>::BuildInitClassInstForCallStatic(
    uint32_t classId)
{
    if (Builder()->GetClassId() != classId) {
        auto initClass = GetGraph()->CreateInstInitClass(DataType::NO_TYPE, pc_, saveState_,
                                                         TypeIdMixin {classId, GetGraph()->GetMethod()},
                                                         GetRuntime()->GetClass(method_));
        Builder()->AddInstruction(initClass);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ, bool HAS_SAVE_STATE>
void InstBuilder::BuildCallHelper<OPCODE, IS_RANGE, ACC_READ, HAS_SAVE_STATE>::BuildCallStaticInst(uint32_t classId)
{
    constexpr auto SLOT_KIND = UnresolvedTypesInterface::SlotKind::METHOD;
    if (method_ == nullptr || (GetRuntime()->IsMethodStatic(GetMethod(), methodId_) && classId == 0) ||
        Builder()->ForceUnresolved()) {
        resolver_ = GetGraph()->CreateInstResolveStatic(DataType::POINTER, pc_, methodId_, nullptr);
        if constexpr (OPCODE == Opcode::CallStatic) {
            call_ = GetGraph()->CreateInstCallResolvedStatic(Builder()->GetMethodReturnType(methodId_), pc_, methodId_);
        }
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), methodId_, SLOT_KIND);
        }
    } else {
        if constexpr (OPCODE == Opcode::CallStatic) {
            call_ =
                GetGraph()->CreateInstCallStatic(Builder()->GetMethodReturnType(methodId_), pc_, methodId_, method_);
        }
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ, bool HAS_SAVE_STATE>
void InstBuilder::BuildCallHelper<OPCODE, IS_RANGE, ACC_READ, HAS_SAVE_STATE>::BuildCallVirtualInst()
{
    constexpr auto SLOT_KIND = UnresolvedTypesInterface::SlotKind::VIRTUAL_METHOD;
    ASSERT(!GetRuntime()->IsMethodStatic(Builder()->GetMethod(), methodId_));
    if (method_ != nullptr && (GetRuntime()->IsInterfaceMethod(method_) || GetGraph()->IsAotNoChaMode())) {
        resolver_ = GetGraph()->CreateInstResolveVirtual(DataType::POINTER, pc_, methodId_, method_);
        if constexpr (OPCODE == Opcode::CallVirtual) {
            call_ = GetGraph()->CreateInstCallResolvedVirtual(Builder()->GetMethodReturnType(methodId_), pc_, methodId_,
                                                              method_);
        }
    } else if (method_ == nullptr || Builder()->ForceUnresolved()) {
        resolver_ = GetGraph()->CreateInstResolveVirtual(DataType::POINTER, pc_, methodId_, nullptr);
        if constexpr (OPCODE == Opcode::CallVirtual) {
            call_ =
                GetGraph()->CreateInstCallResolvedVirtual(Builder()->GetMethodReturnType(methodId_), pc_, methodId_);
        }
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(Builder()->GetMethod(), methodId_, SLOT_KIND);
        }
    } else {
        ASSERT(method_ != nullptr);
        if constexpr (OPCODE == Opcode::CallVirtual) {
            call_ =
                GetGraph()->CreateInstCallVirtual(Builder()->GetMethodReturnType(methodId_), pc_, methodId_, method_);
        }
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ, bool HAS_SAVE_STATE>
void InstBuilder::BuildCallHelper<OPCODE, IS_RANGE, ACC_READ, HAS_SAVE_STATE>::BuildCallInst(
    // CC-OFFNXT(G.FMT.06) false positive
    [[maybe_unused]] uint32_t classId)
{
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (OPCODE == Opcode::CallStatic) {
        BuildCallStaticInst(classId);
    }
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (OPCODE == Opcode::CallVirtual) {
        BuildCallVirtualInst();
    }
    if (UNLIKELY(call_ == nullptr)) {
        UNREACHABLE();
    }
    builder_->SetCallNativeFlags(static_cast<CallInst *>(call_), method_);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildMonitor(const BytecodeInstruction *bcInst, Inst *def, bool isEnter)
{
    auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
    auto inst = GetGraph()->CreateInstMonitor(DataType::VOID, GetPc(bcInst->GetAddress()));
    AddInstruction(saveState);
    if (!isEnter) {
        inst->CastToMonitor()->SetExit();
    } else {
        // Create NullCheck instruction
        auto nullCheck = graph_->CreateInstNullCheck(DataType::REFERENCE, GetPc(bcInst->GetAddress()), def, saveState);
        def = nullCheck;
        AddInstruction(nullCheck);
    }
    inst->SetInput(0, def);
    inst->SetInput(1, saveState);

    AddInstruction(inst);
}

#include <intrinsics_ir_build.inl>

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ, bool HAS_SAVE_STATE>
void InstBuilder::BuildCallHelper<OPCODE, IS_RANGE, ACC_READ, HAS_SAVE_STATE>::BuildDefaultStaticIntrinsic(
    RuntimeInterface::IntrinsicId intrinsicId)
{
    ASSERT(intrinsicId != RuntimeInterface::IntrinsicId::COUNT);
    auto retType = Builder()->GetMethodReturnType(methodId_);
    call_ = GetGraph()->CreateInstIntrinsic(retType, pc_, intrinsicId);
    // If an intrinsic may call runtime then we need a SaveState
    saveState_ = call_->RequireState() ? Builder()->CreateSaveState(Opcode::SaveState, pc_) : nullptr;
    SetCallArgs();
    if (saveState_ != nullptr) {
        Builder()->AddInstruction(saveState_);
    }
    /* if there are reference type args to be checked for NULL ('need_nullcheck' intrinsic property) */
    Builder()->template AddArgNullcheckIfNeeded<false>(intrinsicId, call_, saveState_, pc_);
    AddCallInstruction();
    if (NeedSafePointAfterIntrinsic(intrinsicId)) {
        Builder()->AddInstruction(Builder()->CreateSafePoint(Builder()->GetCurrentBlock()));
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildAbsIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    auto inst = GetGraph()->CreateInstAbs(GetMethodReturnType(methodId), GetPc(bcInst->GetAddress()));
    ASSERT(GetMethodArgumentsCount(methodId) == 1);
    inst->SetInput(0, GetArgDefinition(bcInst, 0, accRead));
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

template <Opcode OPCODE>
static BinaryOperation *CreateBinaryOperation(Graph *graph, DataType::Type returnType, size_t pc) = delete;

template <>
BinaryOperation *CreateBinaryOperation<Opcode::Min>(Graph *graph, DataType::Type returnType, size_t pc)
{
    return graph->CreateInstMin(returnType, pc);
}

template <>
BinaryOperation *CreateBinaryOperation<Opcode::Max>(Graph *graph, DataType::Type returnType, size_t pc)
{
    return graph->CreateInstMax(returnType, pc);
}

template <>
BinaryOperation *CreateBinaryOperation<Opcode::Mod>(Graph *graph, DataType::Type returnType, size_t pc)
{
    return graph->CreateInstMod(returnType, pc);
}

template void InstBuilder::BuildBinaryOperationIntrinsic<Opcode::Mod>(const BytecodeInstruction *bcInst, bool accRead);

template <Opcode OPCODE>
void InstBuilder::BuildBinaryOperationIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    [[maybe_unused]] auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    ASSERT(GetMethodArgumentsCount(methodId) == 2U);
    // NOLINTNEXTLINE(readability-magic-numbers)
    auto inst = CreateBinaryOperation<OPCODE>(GetGraph(), GetMethodReturnType(methodId), GetPc(bcInst->GetAddress()));
    inst->SetInput(0, GetArgDefinition(bcInst, 0, accRead));
    inst->SetInput(1, GetArgDefinition(bcInst, 1, accRead));
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildSqrtIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    [[maybe_unused]] auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    auto inst = GetGraph()->CreateInstSqrt(GetMethodReturnType(methodId), GetPc(bcInst->GetAddress()));
    ASSERT(GetMethodArgumentsCount(methodId) == 1);
    Inst *def = GetArgDefinition(bcInst, 0, accRead);
    inst->SetInput(0, def);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildIsNanIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    // No need to create specialized node for isNaN. Since NaN != NaN, simple float compare node is fine.
    // Also, ensure that float comparison node is implemented for specific architecture
    auto vreg = GetArgDefinition(bcInst, 0, accRead);
    auto inst = GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bcInst->GetAddress()), vreg, vreg,
                                              GetMethodArgumentType(methodId, 0), ConditionCode::CC_NE);
    inst->SetOperandsType(GetMethodArgumentType(methodId, 0));
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildStringLengthIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, bcAddr);

    auto nullCheck =
        graph_->CreateInstNullCheck(DataType::REFERENCE, bcAddr, GetArgDefinition(bcInst, 0, accRead), saveState);
    auto arrayLength = graph_->CreateInstLenArray(DataType::INT32, bcAddr, nullCheck, false);

    AddInstruction(saveState);
    AddInstruction(nullCheck);
    AddInstruction(arrayLength);

    Inst *stringLength;
    if (graph_->GetRuntime()->IsCompressedStringsEnabled()) {
        auto constTwoInst = graph_->FindOrCreateConstant(ark::coretypes::String::STRING_LENGTH_SHIFT);
        stringLength = graph_->CreateInstShr(DataType::INT32, bcAddr, arrayLength, constTwoInst);
        AddInstruction(stringLength);
    } else {
        stringLength = arrayLength;
    }
    UpdateDefinitionAcc(stringLength);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildStringIsEmptyIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, bcAddr);
    auto nullCheck =
        graph_->CreateInstNullCheck(DataType::REFERENCE, bcAddr, GetArgDefinition(bcInst, 0, accRead), saveState);
    auto length = graph_->CreateInstLenArray(DataType::INT32, bcAddr, nullCheck, false);
    auto zeroConst = graph_->FindOrCreateConstant(0);
    auto checkZeroLength =
        graph_->CreateInstCompare(DataType::BOOL, bcAddr, length, zeroConst, DataType::INT32, ConditionCode::CC_EQ);
    AddInstruction(saveState);
    AddInstruction(nullCheck);
    AddInstruction(length);
    AddInstruction(checkZeroLength);
    UpdateDefinitionAcc(checkZeroLength);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCharIsUpperCaseIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    // IsUpperCase(char) = (char - 'A') < ('Z' - 'A')

    ASSERT(GetMethodArgumentsCount(GetRuntime()->ResolveMethodIndex(GetMethod(), bcInst->GetId(0).AsIndex())) == 1);

    // Adding InstCast here as the aternative compiler backend requires inputs of InstSub to be of the same type.
    // The InstCast instructon makes no harm as normally they are removed by the following compiler stages.
    auto argInput = GetArgDefinition(bcInst, 0, accRead);
    auto constInput = graph_->FindOrCreateConstant('A');
    auto arg = GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bcInst->GetAddress()), argInput, argInput->GetType());
    auto constA =
        GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bcInst->GetAddress()), constInput, constInput->GetType());

    auto inst1 = GetGraph()->CreateInstSub(DataType::UINT16, GetPc(bcInst->GetAddress()), arg, constA);
    auto inst2 =
        GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bcInst->GetAddress()), inst1,
                                      graph_->FindOrCreateConstant('Z' - 'A'), DataType::UINT16, ConditionCode::CC_BE);

    AddInstruction(arg);
    AddInstruction(constA);
    AddInstruction(inst1);
    AddInstruction(inst2);
    UpdateDefinitionAcc(inst2);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCharToUpperCaseIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    // ToUpperCase(char) = ((char - 'a') < ('z' - 'a')) * ('Z' - 'z') + char

    ASSERT(GetMethodArgumentsCount(GetRuntime()->ResolveMethodIndex(GetMethod(), bcInst->GetId(0).AsIndex())) == 1);

    auto argInput = GetArgDefinition(bcInst, 0, accRead);
    auto constInput = graph_->FindOrCreateConstant('a');
    auto arg = GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bcInst->GetAddress()), argInput, argInput->GetType());
    auto constA =
        GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bcInst->GetAddress()), constInput, constInput->GetType());

    auto inst1 = GetGraph()->CreateInstSub(DataType::UINT16, GetPc(bcInst->GetAddress()), arg, constA);
    auto inst2 =
        GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bcInst->GetAddress()), inst1,
                                      graph_->FindOrCreateConstant('z' - 'a'), DataType::UINT16, ConditionCode::CC_BE);
    auto inst3 = GetGraph()->CreateInstMul(DataType::UINT16, GetPc(bcInst->GetAddress()), inst2,
                                           graph_->FindOrCreateConstant('Z' - 'z'));
    auto inst4 = GetGraph()->CreateInstAdd(DataType::UINT16, GetPc(bcInst->GetAddress()), arg, inst3);

    AddInstruction(arg);
    AddInstruction(constA);
    AddInstruction(inst1);
    AddInstruction(inst2);
    AddInstruction(inst3);
    AddInstruction(inst4);
    UpdateDefinitionAcc(inst4);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCharIsLowerCaseIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    ASSERT(GetMethodArgumentsCount(GetRuntime()->ResolveMethodIndex(GetMethod(), bcInst->GetId(0).AsIndex())) == 1);

    auto argInput = GetArgDefinition(bcInst, 0, accRead);
    auto constInput = graph_->FindOrCreateConstant('a');
    auto arg = GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bcInst->GetAddress()), argInput, argInput->GetType());
    auto constA =
        GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bcInst->GetAddress()), constInput, constInput->GetType());

    auto inst1 = GetGraph()->CreateInstSub(DataType::UINT16, GetPc(bcInst->GetAddress()), arg, constA);
    auto inst2 =
        GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bcInst->GetAddress()), inst1,
                                      graph_->FindOrCreateConstant('z' - 'a'), DataType::UINT16, ConditionCode::CC_BE);

    AddInstruction(arg);
    AddInstruction(constA);
    AddInstruction(inst1);
    AddInstruction(inst2);
    UpdateDefinitionAcc(inst2);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCharToLowerCaseIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    ASSERT(GetMethodArgumentsCount(GetRuntime()->ResolveMethodIndex(GetMethod(), bcInst->GetId(0).AsIndex())) == 1);

    auto argInput = GetArgDefinition(bcInst, 0, accRead);
    auto constInput = graph_->FindOrCreateConstant('A');
    auto arg = GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bcInst->GetAddress()), argInput, argInput->GetType());
    auto constA =
        GetGraph()->CreateInstCast(DataType::UINT16, GetPc(bcInst->GetAddress()), constInput, constInput->GetType());

    auto inst1 = GetGraph()->CreateInstSub(DataType::UINT16, GetPc(bcInst->GetAddress()), arg, constA);
    auto inst2 =
        GetGraph()->CreateInstCompare(DataType::BOOL, GetPc(bcInst->GetAddress()), inst1,
                                      graph_->FindOrCreateConstant('Z' - 'A'), DataType::UINT16, ConditionCode::CC_BE);
    auto inst3 = GetGraph()->CreateInstMul(DataType::UINT16, GetPc(bcInst->GetAddress()), inst2,
                                           graph_->FindOrCreateConstant('z' - 'Z'));
    auto inst4 = GetGraph()->CreateInstAdd(DataType::UINT16, GetPc(bcInst->GetAddress()), arg, inst3);

    AddInstruction(arg);
    AddInstruction(constA);
    AddInstruction(inst1);
    AddInstruction(inst2);
    AddInstruction(inst3);
    AddInstruction(inst4);
    UpdateDefinitionAcc(inst4);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::GetArgDefinition(const BytecodeInstruction *bcInst, size_t idx, bool accRead, bool isRange)
{
    if (isRange) {
        return GetArgDefinitionRange(bcInst, idx);
    }
    if (accRead) {
        auto accPos = static_cast<size_t>(bcInst->GetImm64());
        if (idx < accPos) {
            return GetDefinition(bcInst->GetVReg(idx));
        }
        if (accPos == idx) {
            return GetDefinitionAcc();
        }
        return GetDefinition(bcInst->GetVReg(idx - 1));
    }
    return GetDefinition(bcInst->GetVReg(idx));
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::GetArgDefinitionRange(const BytecodeInstruction *bcInst, size_t idx)
{
    auto startReg = bcInst->GetVReg(0);
    return GetDefinition(startReg + idx);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ, bool HAS_SAVE_STATE>
void InstBuilder::BuildCallHelper<OPCODE, IS_RANGE, ACC_READ, HAS_SAVE_STATE>::BuildMonitorIntrinsic(bool isEnter)
{
    ASSERT(Builder()->GetMethodReturnType(methodId_) == DataType::VOID);
    ASSERT(Builder()->GetMethodArgumentsCount(methodId_) == 1);
    Builder()->BuildMonitor(bcInst_, Builder()->GetArgDefinition(bcInst_, 0, ACC_READ), isEnter);
}

template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ, bool HAS_SAVE_STATE>
bool InstBuilder::BuildCallHelper<OPCODE, IS_RANGE, ACC_READ, HAS_SAVE_STATE>::TryBuildIntrinsic()
{
    auto runtime = GetRuntime();
    auto isMethodStatic = runtime->IsMethodStatic(method_);
    auto isMethodFinal = runtime->IsMethodFinal(method_);
    auto isClassFinalOrString = runtime->IsClassFinalOrString(runtime->GetClass(method_));
    if (isMethodStatic || isMethodFinal || isClassFinalOrString) {
        BuildIntrinsic();
        return true;
    }
    COMPILER_LOG(DEBUG, IR_BUILDER) << "Skips building intrinsic '"
                                    << GetIntrinsicName(runtime->GetIntrinsicId(method_))
                                    << "' since the method and class is not final.";
    return false;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ, bool HAS_SAVE_STATE>
void InstBuilder::BuildCallHelper<OPCODE, IS_RANGE, ACC_READ, HAS_SAVE_STATE>::BuildIntrinsic()
{
    ASSERT(method_ != nullptr);
    auto intrinsicId = GetRuntime()->GetIntrinsicId(method_);
    auto isVirtual = IsVirtual(intrinsicId);
    if (GetGraph()->IsBytecodeOptimizer() || !g_options.IsCompilerEncodeIntrinsics()) {
        BuildDefaultIntrinsic(intrinsicId, isVirtual);
        return;
    }
    if (!isVirtual) {
        return BuildStaticCallIntrinsic(intrinsicId);
    }
    return BuildVirtualCallIntrinsic(intrinsicId);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ, bool HAS_SAVE_STATE>
void InstBuilder::BuildCallHelper<OPCODE, IS_RANGE, ACC_READ, HAS_SAVE_STATE>::BuildDefaultIntrinsic(
    RuntimeInterface::IntrinsicId intrinsicId, bool isVirtual)
{
    if (intrinsicId == RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_ENTER ||
        intrinsicId == RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_EXIT) {
        BuildMonitorIntrinsic(intrinsicId == RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_ENTER);
        return;
    }
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if (!isVirtual) {
        BuildDefaultStaticIntrinsic(intrinsicId);
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        BuildDefaultVirtualCallIntrinsic(intrinsicId);
    }
}

// do not specify reason for tidy suppression because comment does not fit single line
// NOLINTNEXTLINE
template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ, bool HAS_SAVE_STATE>
void InstBuilder::BuildCallHelper<OPCODE, IS_RANGE, ACC_READ, HAS_SAVE_STATE>::BuildStaticCallIntrinsic(
    RuntimeInterface::IntrinsicId intrinsicId)
{
    switch (intrinsicId) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_ENTER:
        case RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_EXIT: {
            BuildMonitorIntrinsic(intrinsicId == RuntimeInterface::IntrinsicId::INTRINSIC_OBJECT_MONITOR_ENTER);
            break;
        }
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_I32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_I64:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_F32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_ABS_F64: {
            Builder()->BuildAbsIntrinsic(bcInst_, ACC_READ);
            break;
        }
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_SQRT_F32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_SQRT_F64: {
            Builder()->BuildSqrtIntrinsic(bcInst_, ACC_READ);
            break;
        }
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_I32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_I64:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_F32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MIN_F64: {
            Builder()->template BuildBinaryOperationIntrinsic<Opcode::Min>(bcInst_, ACC_READ);
            break;
        }
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_I32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_I64:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_F32:
        case RuntimeInterface::IntrinsicId::INTRINSIC_MATH_MAX_F64: {
            Builder()->template BuildBinaryOperationIntrinsic<Opcode::Max>(bcInst_, ACC_READ);
            break;
        }
#include "intrinsics_ir_build_static_call.inl"
        default: {
            BuildDefaultStaticIntrinsic(intrinsicId);
        }
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ, bool HAS_SAVE_STATE>
void InstBuilder::BuildCallHelper<OPCODE, IS_RANGE, ACC_READ, HAS_SAVE_STATE>::AddCallInstruction()
{
    Builder()->AddInstruction(call_);
    if (call_->GetType() != DataType::VOID) {
        Builder()->UpdateDefinitionAcc(call_);
    } else {
        Builder()->UpdateDefinitionAcc(nullptr);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE, bool IS_RANGE, bool ACC_READ, bool HAS_SAVE_STATE>
void InstBuilder::BuildCallHelper<OPCODE, IS_RANGE, ACC_READ, HAS_SAVE_STATE>::BuildDefaultVirtualCallIntrinsic(
    RuntimeInterface::IntrinsicId intrinsicId)
{
    saveState_ = Builder()->CreateSaveState(Opcode::SaveState, pc_);
    nullCheck_ = GetGraph()->CreateInstNullCheck(DataType::REFERENCE, pc_,
                                                 Builder()->GetArgDefinition(bcInst_, 0, ACC_READ), saveState_);

    call_ = GetGraph()->CreateInstIntrinsic(Builder()->GetMethodReturnType(methodId_), pc_, intrinsicId);
    SetCallArgs();

    Builder()->AddInstruction(saveState_);
    Builder()->AddInstruction(nullCheck_);

    /* if there are reference type args to be checked for NULL */
    Builder()->template AddArgNullcheckIfNeeded<true>(intrinsicId, call_, saveState_, pc_);

    AddCallInstruction();
    if (NeedSafePointAfterIntrinsic(intrinsicId)) {
        Builder()->AddInstruction(Builder()->CreateSafePoint(Builder()->GetCurrentBlock()));
    }
}

template InstBuilder::BuildCallHelper<Opcode::CallResolvedStatic, false, false>::BuildCallHelper(
    const BytecodeInstruction *bcInst, InstBuilder *builder, Inst *additionalInput);
template InstBuilder::BuildCallHelper<Opcode::CallResolvedStatic, false, false, false>::BuildCallHelper(
    const BytecodeInstruction *bcInst, InstBuilder *builder, Inst *additionalInput);

// NOLINTNEXTLINE(readability-function-size,misc-definitions-in-headers)
template <bool IS_ACC_WRITE>
void InstBuilder::BuildLoadObject(const BytecodeInstruction *bcInst, DataType::Type type)
{
    auto pc = GetPc(bcInst->GetAddress());
    // Create SaveState instruction
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    // Create NullCheck instruction
    auto nullCheck = graph_->CreateInstNullCheck(DataType::REFERENCE, pc,
                                                 GetDefinition(bcInst->GetVReg(IS_ACC_WRITE ? 0 : 1)), saveState);
    auto runtime = GetRuntime();
    auto fieldIndex = bcInst->GetId(0).AsIndex();
    auto fieldId = runtime->ResolveFieldIndex(GetMethod(), fieldIndex);
    auto field = runtime->ResolveField(GetMethod(), fieldId, false, !GetGraph()->IsAotMode(), nullptr);
    if (type != DataType::REFERENCE) {
        type = runtime->GetFieldTypeById(GetMethod(), fieldId);
    }

    // Create LoadObject instruction
    Inst *inst;
    AddInstruction(saveState, nullCheck);
    if (field == nullptr || ForceUnresolved()) {
        // 1. Create an instruction to resolve an object's field
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), fieldId,
                                                             UnresolvedTypesInterface::SlotKind::FIELD);
        }
        auto *resolveField = graph_->CreateInstResolveObjectField(DataType::UINT32, pc, saveState,
                                                                  TypeIdMixin {fieldId, GetGraph()->GetMethod()});
        AddInstruction(resolveField);
        // 2. Create an instruction to load a value from the resolved field
        auto loadField = graph_->CreateInstLoadResolvedObjectField(type, pc, nullCheck, resolveField,
                                                                   TypeIdMixin {fieldId, GetGraph()->GetMethod()});
        inst = loadField;
    } else {
        auto loadField =
            graph_->CreateInstLoadObject(type, pc, nullCheck, TypeIdMixin {fieldId, GetGraph()->GetMethod()}, field,
                                         runtime->IsFieldVolatile(field));
        // 'final' field can be reassigned e. g. with reflection, but 'readonly' cannot
        // `IsInConstructor` check should not be necessary, but need proper frontend support first
        constexpr bool IS_STATIC = false;
        if (runtime->IsFieldReadonly(field) && !IsInConstructor<IS_STATIC>()) {
            loadField->ClearFlag(inst_flags::NO_CSE);
        }
        inst = loadField;
    }
    if (type == DataType::REFERENCE && GetRuntime()->NeedsPreReadBarrier()) {
        static_cast<LoadInst *>(inst)->SetNeedBarrier(true);
    }

    AddInstruction(inst);
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (IS_ACC_WRITE) {
        UpdateDefinitionAcc(inst);
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        UpdateDefinition(bcInst->GetVReg(0), inst);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildStoreObjectInst(const BytecodeInstruction *bcInst, DataType::Type type,
                                        RuntimeInterface::FieldPtr field, uint32_t fieldId, Inst **resolveInst)
{
    auto pc = GetPc(bcInst->GetAddress());
    if (field == nullptr || ForceUnresolved()) {
        // The field is unresolved, so we have to resolve it and then store
        // 1. Create an instruction to resolve an object's field
        auto resolveField = graph_->CreateInstResolveObjectField(DataType::UINT32, pc, nullptr,
                                                                 TypeIdMixin {fieldId, GetGraph()->GetMethod()});
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), fieldId,
                                                             UnresolvedTypesInterface::SlotKind::FIELD);
        }
        // 2. Create an instruction to store a value to the resolved field
        auto storeField = graph_->CreateInstStoreResolvedObjectField(type, pc, nullptr, nullptr, nullptr,
                                                                     TypeIdMixin {fieldId, GetGraph()->GetMethod()},
                                                                     false, type == DataType::REFERENCE);
        *resolveInst = resolveField;
        return storeField;
    }

    ASSERT(field != nullptr);
    auto storeField =
        graph_->CreateInstStoreObject(type, pc, nullptr, nullptr, TypeIdMixin {fieldId, GetGraph()->GetMethod()}, field,
                                      GetRuntime()->IsFieldVolatile(field), type == DataType::REFERENCE);
    *resolveInst = nullptr;  // resolver is not needed in this case
    return storeField;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <bool IS_ACC_READ>
void InstBuilder::BuildStoreObject(const BytecodeInstruction *bcInst, DataType::Type type)
{
    // Create SaveState instruction
    auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));

    // Create NullCheck instruction
    auto nullCheck = graph_->CreateInstNullCheck(DataType::REFERENCE, GetPc(bcInst->GetAddress()),
                                                 GetDefinition(bcInst->GetVReg(IS_ACC_READ ? 0 : 1)), saveState);

    auto runtime = GetRuntime();
    auto fieldIndex = bcInst->GetId(0).AsIndex();
    auto fieldId = runtime->ResolveFieldIndex(GetMethod(), fieldIndex);
    auto field = runtime->ResolveField(GetMethod(), fieldId, false, !GetGraph()->IsAotMode(), nullptr);
    if (type != DataType::REFERENCE) {
        type = runtime->GetFieldTypeById(GetMethod(), fieldId);
    }

    // Get a value to store
    Inst *storeVal = nullptr;
    // NOLINTNEXTLINE(readability-braces-around-statements)
    if constexpr (IS_ACC_READ) {
        storeVal = GetDefinitionAcc();
    } else {  // NOLINT(readability-misleading-indentation)
        storeVal = GetDefinition(bcInst->GetVReg(0));
    }

    // Create StoreObject instruction
    Inst *resolveField = nullptr;
    Inst *storeField = BuildStoreObjectInst(bcInst, type, field, fieldId, &resolveField);
    storeField->SetInput(0, nullCheck);
    storeField->SetInput(1, storeVal);

    AddInstruction(saveState);
    AddInstruction(nullCheck);
    if (resolveField != nullptr) {
        ASSERT(field == nullptr || ForceUnresolved());
        resolveField->SetInput(0, saveState);
        storeField->SetInput(2U, resolveField);
        AddInstruction(resolveField);
    }
    AddInstruction(storeField);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildLoadStaticInst(size_t pc, DataType::Type type, uint32_t typeId, Inst *saveState)
{
    uint32_t classId;
    auto field = GetRuntime()->ResolveField(GetMethod(), typeId, true, !GetGraph()->IsAotMode(), &classId);
    if (field == nullptr || ForceUnresolved()) {
        // The static field is unresolved, so we have to resolve it and then load
        // 1. Create an instruction to resolve an object's static field.
        //    Its result is a static field memory address (not an offset as there is no object)
        auto resolveField = graph_->CreateInstResolveObjectFieldStatic(DataType::REFERENCE, pc, saveState,
                                                                       TypeIdMixin {typeId, GetGraph()->GetMethod()});
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), typeId,
                                                             UnresolvedTypesInterface::SlotKind::FIELD);
        }
        AddInstruction(resolveField);
        // 2. Create an instruction to load a value from the resolved static field address
        auto needsBarrier = (type == DataType::REFERENCE) && GetRuntime()->NeedsPreReadBarrier();
        auto loadField = graph_->CreateInstLoadResolvedObjectFieldStatic(
            type, pc, resolveField, TypeIdMixin {typeId, GetGraph()->GetMethod()}, false, needsBarrier);
        return loadField;
    }

    ASSERT(field != nullptr);
    ASSERT(classId != 0);
    auto initClass = graph_->CreateInstLoadAndInitClass(DataType::REFERENCE, pc, saveState,
                                                        TypeIdMixin {classId, GetGraph()->GetMethod()},
                                                        GetRuntime()->GetClassForField(field));
    auto needsBarrier = (type == DataType::REFERENCE) && GetRuntime()->NeedsPreReadBarrier();
    auto loadField = graph_->CreateInstLoadStatic(type, pc, initClass, TypeIdMixin {typeId, GetGraph()->GetMethod()},
                                                  field, GetRuntime()->IsFieldVolatile(field), needsBarrier);
    // 'final' field can be reassigned e. g. with reflection, but 'readonly' cannot
    // `IsInConstructor` check should not be necessary, but need proper frontend support first
    constexpr bool IS_STATIC = true;
    if (GetRuntime()->IsFieldReadonly(field) && !IsInConstructor<IS_STATIC>()) {
        loadField->ClearFlag(inst_flags::NO_CSE);
    }
    AddInstruction(initClass);
    return loadField;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
AnyTypeCheckInst *InstBuilder::BuildAnyTypeCheckInst(size_t bcAddr, Inst *input, Inst *saveState, AnyBaseType type)
{
    auto anyCheck = graph_->CreateInstAnyTypeCheck(DataType::ANY, bcAddr, input, saveState, type);
    AddInstruction(anyCheck);
    return anyCheck;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLoadStatic(const BytecodeInstruction *bcInst, DataType::Type type)
{
    auto fieldIndex = bcInst->GetId(0).AsIndex();
    auto fieldId = GetRuntime()->ResolveFieldIndex(GetMethod(), fieldIndex);
    if (type != DataType::REFERENCE) {
        type = GetRuntime()->GetFieldTypeById(GetMethod(), fieldId);
    }
    auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
    AddInstruction(saveState);
    auto *inst = BuildLoadStaticInst(GetPc(bcInst->GetAddress()), type, fieldId, saveState);
    if (type == DataType::REFERENCE && GetRuntime()->NeedsPreReadBarrier()) {
        static_cast<LoadStaticInst *>(inst)->SetNeedBarrier(true);
    }
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(readability-function-size,misc-definitions-in-headers)
Inst *InstBuilder::BuildStoreStaticInst(const BytecodeInstruction *bcInst, DataType::Type type, uint32_t typeId,
                                        Inst *storeInput, Inst *saveState)
{
    AddInstruction(saveState);

    uint32_t classId;
    auto field = GetRuntime()->ResolveField(GetMethod(), typeId, true, !GetGraph()->IsAotMode(), &classId);
    auto pc = GetPc(bcInst->GetAddress());

    if (field == nullptr || ForceUnresolved()) {
        if (type == DataType::REFERENCE) {
            // 1. Class initialization is needed.
            // 2. GC Pre/Post write barriers may be needed.
            // Just call runtime EntrypointId::UNRESOLVED_STORE_STATIC_BARRIERED,
            // which performs all the necessary steps (see codegen.cpp for the details).
            auto inst = graph_->CreateInstUnresolvedStoreStatic(type, pc, storeInput, saveState,
                                                                TypeIdMixin {typeId, GetGraph()->GetMethod()}, true);
            return inst;
        }
        ASSERT(type != DataType::REFERENCE);
        // 1. Create an instruction to resolve an object's static field.
        //    Its result is a static field memory address (REFERENCE)
        auto resolveField = graph_->CreateInstResolveObjectFieldStatic(DataType::REFERENCE, pc, saveState,
                                                                       TypeIdMixin {typeId, GetGraph()->GetMethod()});
        AddInstruction(resolveField);
        // 2. Create an instruction to store a value to the resolved static field address
        auto storeField = graph_->CreateInstStoreResolvedObjectFieldStatic(
            type, pc, resolveField, storeInput, TypeIdMixin {typeId, GetGraph()->GetMethod()});
        if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), typeId,
                                                             UnresolvedTypesInterface::SlotKind::FIELD);
        }
        return storeField;
    }

    ASSERT(field != nullptr);
    ASSERT(classId != 0);
    auto initClass = graph_->CreateInstLoadAndInitClass(DataType::REFERENCE, pc, saveState,
                                                        TypeIdMixin {classId, GetGraph()->GetMethod()},
                                                        GetRuntime()->GetClassForField(field));
    auto storeField =
        graph_->CreateInstStoreStatic(type, pc, initClass, storeInput, TypeIdMixin {typeId, GetGraph()->GetMethod()},
                                      field, GetRuntime()->IsFieldVolatile(field), type == DataType::REFERENCE);
    AddInstruction(initClass);
    return storeField;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildStoreStatic(const BytecodeInstruction *bcInst, DataType::Type type)
{
    auto fieldIndex = bcInst->GetId(0).AsIndex();
    auto fieldId = GetRuntime()->ResolveFieldIndex(GetMethod(), fieldIndex);
    if (type != DataType::REFERENCE) {
        type = GetRuntime()->GetFieldTypeById(GetMethod(), fieldId);
    }
    auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
    Inst *storeInput = GetDefinitionAcc();
    Inst *inst = BuildStoreStaticInst(bcInst, type, fieldId, storeInput, saveState);
    AddInstruction(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
std::tuple<SaveStateInst *, Inst *, LengthMethodInst *, BoundsCheckInst *> InstBuilder::BuildChecksBeforeArray(
    size_t pc, Inst *arrayRef, bool withNullcheck)
{
    // Create SaveState instruction
    auto saveState = CreateSaveState(Opcode::SaveState, pc);

    // Create NullCheck instruction
    Inst *nullCheck = nullptr;
    if (withNullcheck) {
        nullCheck = graph_->CreateInstNullCheck(DataType::REFERENCE, pc, arrayRef, saveState);
    } else {
        nullCheck = arrayRef;
    }

    // Create LenArray instruction
    auto arrayLength = graph_->CreateInstLenArray(DataType::INT32, pc, nullCheck);

    // Create BoundCheck instruction
    auto boundsCheck = graph_->CreateInstBoundsCheck(DataType::INT32, pc, arrayLength, nullptr, saveState);

    return std::make_tuple(saveState, nullCheck, arrayLength, boundsCheck);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLoadArray(const BytecodeInstruction *bcInst, DataType::Type type)
{
    ASSERT(type != DataType::NO_TYPE);
    auto pc = GetPc(bcInst->GetAddress());

    auto [saveState, nullCheck, arrayLength, boundsCheck] =
        BuildChecksBeforeArray(pc, GetDefinition(bcInst->GetVReg(0)));
    ASSERT(saveState != nullptr && nullCheck != nullptr && arrayLength != nullptr && boundsCheck != nullptr);

    // Create instruction
    auto inst = graph_->CreateInstLoadArray(type, pc, nullCheck, boundsCheck);
    boundsCheck->SetInput(1, GetDefinitionAcc());
    if (type == DataType::REFERENCE && GetRuntime()->NeedsPreReadBarrier()) {
        inst->SetNeedBarrier(true);
    }
    AddInstruction(saveState, nullCheck, arrayLength, boundsCheck, inst);
    UpdateDefinitionAcc(inst);
}

template <typename T>
void InstBuilder::BuildUnfoldLoadConstPrimitiveArray(const BytecodeInstruction *bcInst, DataType::Type type,
                                                     const pandasm::LiteralArray &litArray, NewArrayInst *arrayInst)
{
    [[maybe_unused]] auto tag = litArray.literals[0].tag;
    auto arraySize = litArray.literals.size();
    for (size_t i = 0; i < arraySize; i++) {
        auto indexInst = graph_->FindOrCreateConstant(i);
        ConstantInst *valueInst;
        if constexpr (std::is_same_v<T, float>) {
            ASSERT(tag == panda_file::LiteralTag::ARRAY_F32);
            valueInst = FindOrCreateFloatConstant(std::get<T>(litArray.literals[i].value));
        } else if constexpr (std::is_same_v<T, double>) {
            ASSERT(tag == panda_file::LiteralTag::ARRAY_F64);
            valueInst = FindOrCreateDoubleConstant(std::get<T>(litArray.literals[i].value));
        } else if constexpr (std::is_same_v<T, bool>) {
            ASSERT(tag == panda_file::LiteralTag::ARRAY_U1);
            valueInst = FindOrCreateConstant(std::get<T>(litArray.literals[i].value));
        } else {
            auto lit = litArray.literals[i];
            T val = std::get<T>(lit.value);
            if (lit.IsSigned()) {
                valueInst = FindOrCreateConstant(static_cast<std::make_signed_t<T>>(val));
            } else {
                valueInst = FindOrCreateConstant(val);
            }
        }

        BuildStoreArrayInst<false>(bcInst, type, arrayInst, indexInst, valueInst);
    }
}

template <typename T>
void InstBuilder::BuildUnfoldLoadConstStringArray(const BytecodeInstruction *bcInst, DataType::Type type,
                                                  const pandasm::LiteralArray &litArray, NewArrayInst *arrayInst)
{
    auto method = GetGraph()->GetMethod();
    auto arraySize = litArray.literals.size();
    for (size_t i = 0; i < arraySize; i++) {
        auto indexInst = graph_->FindOrCreateConstant(i);
        auto save = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
        auto typeId = static_cast<uint32_t>(std::get<T>(litArray.literals[i].value));
        auto loadStringInst = GetGraph()->CreateInstLoadString(DataType::REFERENCE, GetPc(bcInst->GetAddress()), save,
                                                               TypeIdMixin {typeId, method});
        AddInstruction(save);
        AddInstruction(loadStringInst);
        if (GetGraph()->IsDynamicMethod()) {
            BuildCastToAnyString(bcInst);
        }

        BuildStoreArrayInst<false>(bcInst, type, arrayInst, indexInst, loadStringInst);
    }
}

template <typename T>
void InstBuilder::BuildUnfoldLoadConstArray(const BytecodeInstruction *bcInst, DataType::Type type,
                                            const pandasm::LiteralArray &litArray)
{
    auto method = GetGraph()->GetMethod();
    auto arraySize = litArray.literals.size();
    auto typeId = GetRuntime()->GetLiteralArrayClassIdWithinFile(method, litArray.literals[0].tag);

    // Create NewArray instruction
    auto sizeInst = graph_->FindOrCreateConstant(arraySize);
    auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
    auto negCheck = graph_->CreateInstNegativeCheck(DataType::INT32, GetPc(bcInst->GetAddress()), sizeInst, saveState);
    auto initClass = CreateLoadAndInitClassGeneric(typeId, GetPc(bcInst->GetAddress()));
    initClass->SetInput(0, saveState);
    auto arrayInst = graph_->CreateInstNewArray(DataType::REFERENCE, GetPc(bcInst->GetAddress()), initClass, negCheck,
                                                saveState, TypeIdMixin {typeId, GetGraph()->GetMethod()});
    AddInstruction(saveState);
    AddInstruction(initClass);
    AddInstruction(negCheck);
    AddInstruction(arrayInst);
    UpdateDefinition(bcInst->GetVReg(0), arrayInst);

    if (arraySize > g_options.GetCompilerUnfoldConstArrayMaxSize()) {
        // Create LoadConstArray instruction
        auto ss = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
        auto inst = GetGraph()->CreateInstFillConstArray(type, GetPc(bcInst->GetAddress()), arrayInst, ss,
                                                         TypeIdMixin {bcInst->GetId(0).AsFileId().GetOffset(), method},
                                                         arraySize);
        AddInstruction(ss);
        AddInstruction(inst);
        return;
    }

    // Create instructions for array filling
    auto tag = litArray.literals[0].tag;
    if (tag != panda_file::LiteralTag::ARRAY_STRING) {
        BuildUnfoldLoadConstPrimitiveArray<T>(bcInst, type, litArray, arrayInst);
        return;
    }

    [[maybe_unused]] auto arrayClass = GetRuntime()->ResolveType(method, typeId);
    ASSERT(GetRuntime()->CheckStoreArray(arrayClass, GetRuntime()->GetLineStringClass(method, nullptr)));
    // Special case for string array
    BuildUnfoldLoadConstStringArray<T>(bcInst, type, litArray, arrayInst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLoadConstStringArray(const BytecodeInstruction *bcInst)
{
    auto literalArrayIdx = bcInst->GetId(0).AsIndex();
    auto litArray = GetRuntime()->GetLiteralArray(GetMethod(), literalArrayIdx);
    auto arraySize = litArray.literals.size();
    ASSERT(arraySize > 0);
    if (arraySize > g_options.GetCompilerUnfoldConstArrayMaxSize()) {
        // Create LoadConstArray instruction for String array, because we calls runtime for the case.
        auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
        auto method = GetGraph()->GetMethod();
        auto inst = GetGraph()->CreateInstLoadConstArray(DataType::REFERENCE, GetPc(bcInst->GetAddress()), saveState,
                                                         TypeIdMixin {literalArrayIdx, method});
        AddInstruction(saveState);
        AddInstruction(inst);
        UpdateDefinition(bcInst->GetVReg(0), inst);
    } else {
        BuildUnfoldLoadConstArray<uint32_t>(bcInst, DataType::REFERENCE, litArray);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLoadConstArray(const BytecodeInstruction *bcInst)
{
    auto literalArrayIdx = bcInst->GetId(0).AsIndex();
    auto litArray = GetRuntime()->GetLiteralArray(GetMethod(), literalArrayIdx);
    // Unfold LoadConstArray instruction
    auto tag = litArray.literals[0].tag;
    switch (tag) {
        case panda_file::LiteralTag::ARRAY_U1: {
            BuildUnfoldLoadConstArray<bool>(bcInst, DataType::INT8, litArray);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I8:
        case panda_file::LiteralTag::ARRAY_U8: {
            BuildUnfoldLoadConstArray<uint8_t>(bcInst, DataType::INT8, litArray);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I16:
        case panda_file::LiteralTag::ARRAY_U16: {
            BuildUnfoldLoadConstArray<uint16_t>(bcInst, DataType::INT16, litArray);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I32:
        case panda_file::LiteralTag::ARRAY_U32: {
            BuildUnfoldLoadConstArray<uint32_t>(bcInst, DataType::INT32, litArray);
            break;
        }
        case panda_file::LiteralTag::ARRAY_I64:
        case panda_file::LiteralTag::ARRAY_U64: {
            BuildUnfoldLoadConstArray<uint64_t>(bcInst, DataType::INT64, litArray);
            break;
        }
        case panda_file::LiteralTag::ARRAY_F32: {
            BuildUnfoldLoadConstArray<float>(bcInst, DataType::FLOAT32, litArray);
            break;
        }
        case panda_file::LiteralTag::ARRAY_F64: {
            BuildUnfoldLoadConstArray<double>(bcInst, DataType::FLOAT64, litArray);
            break;
        }
        case panda_file::LiteralTag::ARRAY_STRING: {
            BuildLoadConstStringArray(bcInst);
            break;
        }
        default: {
            UNREACHABLE();
            break;
        }
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildStoreArray(const BytecodeInstruction *bcInst, DataType::Type type)
{
    BuildStoreArrayInst<true>(bcInst, type, GetDefinition(bcInst->GetVReg(0)), GetDefinition(bcInst->GetVReg(1)),
                              GetDefinitionAcc());
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <bool CREATE_REF_CHECK>
void InstBuilder::BuildStoreArrayInst(const BytecodeInstruction *bcInst, DataType::Type type, Inst *arrayRef,
                                      Inst *index, Inst *value)
{
    ASSERT(type != DataType::NO_TYPE);
    Inst *refCheck = nullptr;

    auto pc = GetPc(bcInst->GetAddress());
    auto [saveState, nullCheck, arrayLength, boundsCheck] = BuildChecksBeforeArray(pc, arrayRef);
    ASSERT(saveState != nullptr && nullCheck != nullptr && arrayLength != nullptr && boundsCheck != nullptr);

    // Create instruction
    auto inst = graph_->CreateInstStoreArray(type, pc);
    boundsCheck->SetInput(1, index);
    auto storeDef = value;
    if (type == DataType::REFERENCE) {
        // NOLINTNEXTLINE(readability-braces-around-statements,bugprone-suspicious-semicolon)
        if constexpr (CREATE_REF_CHECK) {
            refCheck = graph_->CreateInstRefTypeCheck(DataType::REFERENCE, pc, nullCheck, storeDef, saveState);
            storeDef = refCheck;
        }
        inst->CastToStoreArray()->SetNeedBarrier(true);
    }
    inst->SetInput(0, nullCheck);
    inst->SetInput(1, boundsCheck);
    inst->SetInput(2U, storeDef);
    if (refCheck != nullptr) {
        AddInstruction(saveState, nullCheck, arrayLength, boundsCheck, refCheck, inst);
    } else {
        AddInstruction(saveState, nullCheck, arrayLength, boundsCheck, inst);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLenArray(const BytecodeInstruction *bcInst)
{
    auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
    auto nullCheck = graph_->CreateInstNullCheck(DataType::REFERENCE, GetPc(bcInst->GetAddress()));
    nullCheck->SetInput(0, GetDefinition(bcInst->GetVReg(0)));
    nullCheck->SetInput(1, saveState);
    auto inst = graph_->CreateInstLenArray(DataType::INT32, GetPc(bcInst->GetAddress()), nullCheck);
    AddInstruction(saveState);
    AddInstruction(nullCheck);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildNewArray(const BytecodeInstruction *bcInst)
{
    auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
    auto negCheck = graph_->CreateInstNegativeCheck(DataType::INT32, GetPc(bcInst->GetAddress()),
                                                    GetDefinition(bcInst->GetVReg(1)), saveState);

    auto typeIndex = bcInst->GetId(0).AsIndex();
    auto typeId = GetRuntime()->ResolveTypeIndex(GetMethod(), typeIndex);

    auto initClass = CreateLoadAndInitClassGeneric(typeId, GetPc(bcInst->GetAddress()));
    initClass->SetInput(0, saveState);

    auto inst = graph_->CreateInstNewArray(DataType::REFERENCE, GetPc(bcInst->GetAddress()), initClass, negCheck,
                                           saveState, TypeIdMixin {typeId, GetGraph()->GetMethod()});
    AddInstruction(saveState, initClass, negCheck, inst);
    UpdateDefinition(bcInst->GetVReg(0), inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildNewObject(const BytecodeInstruction *bcInst)
{
    auto classId = GetRuntime()->ResolveTypeIndex(GetMethod(), bcInst->GetId(0).AsIndex());
    auto pc = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto initClass = CreateLoadAndInitClassGeneric(classId, pc);
    auto inst = CreateNewObjectInst(pc, classId, saveState, initClass);
    initClass->SetInput(0, saveState);
    AddInstruction(saveState, initClass, inst);
    UpdateDefinition(bcInst->GetVReg(0), inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildMultiDimensionalArrayObject(const BytecodeInstruction *bcInst, bool isRange)
{
    auto methodIndex = bcInst->GetId(0).AsIndex();
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    auto pc = GetPc(bcInst->GetAddress());
    auto classId = GetRuntime()->GetClassIdForMethod(GetMethod(), methodId);
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto initClass = CreateLoadAndInitClassGeneric(classId, pc);
    size_t argsCount = GetMethodArgumentsCount(methodId);
    auto inst = GetGraph()->CreateInstMultiArray(DataType::REFERENCE, pc, methodId);

    initClass->SetInput(0, saveState);

    inst->ReserveInputs(ONE_FOR_OBJECT + argsCount + ONE_FOR_SSTATE);
    inst->AllocateInputTypes(GetGraph()->GetAllocator(), ONE_FOR_OBJECT + argsCount + ONE_FOR_SSTATE);
    inst->AppendInput(initClass);
    inst->AddInputType(DataType::REFERENCE);

    AddInstruction(saveState, initClass);

    if (isRange) {
        auto startReg = bcInst->GetVReg(0);
        for (size_t i = 0; i < argsCount; startReg++, i++) {
            auto negCheck = graph_->CreateInstNegativeCheck(DataType::INT32, pc, GetDefinition(startReg), saveState);
            AddInstruction(negCheck);
            inst->AppendInput(negCheck);
            inst->AddInputType(DataType::INT32);
        }
    } else {
        for (size_t i = 0; i < argsCount; i++) {
            auto negCheck =
                graph_->CreateInstNegativeCheck(DataType::INT32, pc, GetDefinition(bcInst->GetVReg(i)), saveState);
            AddInstruction(negCheck);
            inst->AppendInput(negCheck);
            inst->AddInputType(DataType::INT32);
        }
    }
    inst->AppendInput(saveState);
    inst->AddInputType(DataType::NO_TYPE);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildInitObjectMultiDimensionalArray(const BytecodeInstruction *bcInst, bool isRange)
{
    auto pc = GetPc(bcInst->GetAddress());
    auto methodIndex = bcInst->GetId(0).AsIndex();
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), methodIndex);
    auto classId = GetRuntime()->GetClassIdForMethod(GetMethod(), methodId);
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    ASSERT(classId != 0);
    auto initClass = graph_->CreateInstLoadAndInitClass(DataType::REFERENCE, pc, saveState,
                                                        TypeIdMixin {classId, GetGraph()->GetMethod()},
                                                        GetRuntime()->ResolveType(GetGraph()->GetMethod(), classId));
    auto inst = GetGraph()->CreateInstInitObject(DataType::REFERENCE, pc, methodId);

    size_t argsCount = GetMethodArgumentsCount(methodId);

    inst->ReserveInputs(ONE_FOR_OBJECT + argsCount + ONE_FOR_SSTATE);
    inst->AllocateInputTypes(GetGraph()->GetAllocator(), ONE_FOR_OBJECT + argsCount + ONE_FOR_SSTATE);
    inst->AppendInput(initClass);
    inst->AddInputType(DataType::REFERENCE);
    if (isRange) {
        auto startReg = bcInst->GetVReg(0);
        for (size_t i = 0; i < argsCount; startReg++, i++) {
            inst->AppendInput(GetDefinition(startReg));
            inst->AddInputType(GetMethodArgumentType(methodId, i));
        }
    } else {
        for (size_t i = 0; i < argsCount; i++) {
            inst->AppendInput(GetDefinition(bcInst->GetVReg(i)));
            inst->AddInputType(GetMethodArgumentType(methodId, i));
        }
    }
    inst->AppendInput(saveState);
    inst->AddInputType(DataType::NO_TYPE);
    inst->SetCallMethod(GetRuntime()->GetMethodById(GetGraph()->GetMethod(), methodId));

    AddInstruction(saveState, initClass, inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
CallInst *InstBuilder::BuildCallStaticForInitObject(const BytecodeInstruction *bcInst, uint32_t methodId,
                                                    Inst **resolver)
{
    auto pc = GetPc(bcInst->GetAddress());
    size_t argsCount = GetMethodArgumentsCount(methodId);
    size_t inputsCount = ONE_FOR_OBJECT + argsCount + ONE_FOR_SSTATE;
    auto method = GetRuntime()->GetMethodById(graph_->GetMethod(), methodId);
    CallInst *call = nullptr;
    if (method == nullptr || ForceUnresolved()) {
        ResolveStaticInst *resolveStatic = graph_->CreateInstResolveStatic(DataType::POINTER, pc, methodId, nullptr);
        *resolver = resolveStatic;
        call = graph_->CreateInstCallResolvedStatic(GetMethodReturnType(methodId), pc, methodId);
        if (!graph_->IsAotMode() && !graph_->IsBytecodeOptimizer()) {
            GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetMethod(), methodId,
                                                             UnresolvedTypesInterface::SlotKind::METHOD);
        }
        inputsCount += 1;  // resolver
    } else {
        call = graph_->CreateInstCallStatic(GetMethodReturnType(methodId), pc, methodId, method);
    }
    call->ReserveInputs(inputsCount);
    call->AllocateInputTypes(graph_->GetAllocator(), inputsCount);
    SetCallNativeFlags(call, method);
    return call;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildInitString(const BytecodeInstruction *bcInst)
{
    auto pc = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    AddInstruction(saveState);

    auto ctorMethodIndex = bcInst->GetId(0).AsIndex();
    auto ctorMethodId = GetRuntime()->ResolveMethodIndex(GetMethod(), ctorMethodIndex);
    size_t argsCount = GetMethodArgumentsCount(ctorMethodId);

    Inst *inst = nullptr;
    if (argsCount == 0) {
        inst = GetGraph()->CreateInstInitEmptyString(DataType::REFERENCE, pc, saveState);
    } else {
        ASSERT(argsCount == 1);
        auto nullCheck =
            graph_->CreateInstNullCheck(DataType::REFERENCE, pc, GetDefinition(bcInst->GetVReg(0)), saveState);
        AddInstruction(nullCheck);

        auto ctorMethod = GetRuntime()->GetMethodById(GetMethod(), ctorMethodId);
        auto ctorType = GetRuntime()->GetStringCtorType(ctorMethod);
        inst = GetGraph()->CreateInstInitString(DataType::REFERENCE, pc, nullCheck, saveState, ctorType);
    }
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildInitObject(const BytecodeInstruction *bcInst, bool isRange)
{
    auto methodId = GetRuntime()->ResolveMethodIndex(GetMethod(), bcInst->GetId(0).AsIndex());
    auto typeId = GetRuntime()->GetClassIdForMethod(GetMethod(), methodId);
    if (GetRuntime()->IsArrayClass(GetMethod(), typeId)) {
        if (GetGraph()->IsBytecodeOptimizer()) {
            BuildInitObjectMultiDimensionalArray(bcInst, isRange);
        } else {
            BuildMultiDimensionalArrayObject(bcInst, isRange);
        }
        return;
    }

    if (GetRuntime()->IsStringClass(GetMethod(), typeId) && !GetGraph()->IsBytecodeOptimizer()) {
        BuildInitString(bcInst);
        return;
    }

    auto pc = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto initClass = CreateLoadAndInitClassGeneric(typeId, pc);
    initClass->SetInput(0, saveState);
    auto newObj = CreateNewObjectInst(pc, typeId, saveState, initClass);
    UpdateDefinitionAcc(newObj);
    Inst *resolver = nullptr;
    CallInst *call = BuildCallStaticForInitObject(bcInst, methodId, &resolver);
    if (resolver != nullptr) {
        call->AppendInput(resolver, DataType::POINTER);
    }
    call->AppendInput(newObj, DataType::REFERENCE);

    size_t argsCount = GetMethodArgumentsCount(methodId);
    if (isRange) {
        auto startReg = bcInst->GetVReg(0);
        for (size_t i = 0; i < argsCount; startReg++, i++) {
            call->AppendInput(GetDefinition(startReg), GetMethodArgumentType(methodId, i));
        }
    } else {
        for (size_t i = 0; i < argsCount; i++) {
            call->AppendInput(GetDefinition(bcInst->GetVReg(i)), GetMethodArgumentType(methodId, i));
        }
    }
    auto saveStateForCall = CreateSaveState(Opcode::SaveState, pc);
    call->AppendInput(saveStateForCall, DataType::NO_TYPE);
    if (resolver != nullptr) {
        resolver->SetInput(0, saveStateForCall);
        AddInstruction(saveState, initClass, newObj, saveStateForCall, resolver, call);
    } else {
        AddInstruction(saveState, initClass, newObj, saveStateForCall, call);
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCheckCast(const BytecodeInstruction *bcInst)
{
    auto typeIndex = bcInst->GetId(0).AsIndex();
    auto typeId = GetRuntime()->ResolveTypeIndex(GetMethod(), typeIndex);
    auto klassType = GetRuntime()->GetClassType(GetGraph()->GetMethod(), typeId);
    auto pc = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto loadClass = BuildLoadClass(typeId, pc, saveState);
    auto inst = GetGraph()->CreateInstCheckCast(DataType::NO_TYPE, pc, GetDefinitionAcc(), loadClass, saveState,
                                                TypeIdMixin {typeId, GetGraph()->GetMethod()}, klassType);
    AddInstruction(saveState, loadClass, inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildIsInstance(const BytecodeInstruction *bcInst)
{
    auto typeIndex = bcInst->GetId(0).AsIndex();
    auto typeId = GetRuntime()->ResolveTypeIndex(GetMethod(), typeIndex);
    auto klassType = GetRuntime()->GetClassType(GetGraph()->GetMethod(), typeId);
    auto pc = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, pc);
    auto loadClass = BuildLoadClass(typeId, pc, saveState);
    auto inst = GetGraph()->CreateInstIsInstance(DataType::BOOL, pc, GetDefinitionAcc(), loadClass, saveState,
                                                 TypeIdMixin {typeId, GetGraph()->GetMethod()}, klassType);
    AddInstruction(saveState, loadClass, inst);
    UpdateDefinitionAcc(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
Inst *InstBuilder::BuildLoadClass(RuntimeInterface::IdType typeId, size_t pc, Inst *saveState)
{
    auto inst = GetGraph()->CreateInstLoadClass(DataType::REFERENCE, pc, saveState,
                                                TypeIdMixin {typeId, GetGraph()->GetMethod()}, nullptr);
    auto klass = GetRuntime()->ResolveType(GetGraph()->GetMethod(), typeId);
    if (klass != nullptr) {
        inst->SetClass(klass);
    } else if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
        GetRuntime()->GetUnresolvedTypes()->AddTableSlot(GetGraph()->GetMethod(), typeId,
                                                         UnresolvedTypesInterface::SlotKind::CLASS);
    }
    return inst;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildThrow(const BytecodeInstruction *bcInst)
{
    auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
    auto inst = graph_->CreateInstThrow(DataType::NO_TYPE, GetPc(bcInst->GetAddress()),
                                        GetDefinition(bcInst->GetVReg(0)), saveState);
    inst->SetCallMethod(GetMethod());
    AddInstruction(saveState);
    AddInstruction(inst);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <Opcode OPCODE>
void InstBuilder::BuildLoadFromPool(const BytecodeInstruction *bcInst)
{
    auto method = GetGraph()->GetMethod();
    uint32_t typeId;
    Inst *inst;
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements)
    if constexpr (OPCODE == Opcode::LoadType) {
        auto typeIndex = bcInst->GetId(0).AsIndex();
        typeId = GetRuntime()->ResolveTypeIndex(method, typeIndex);
        if (GetRuntime()->ResolveType(method, typeId) == nullptr) {
            inst = GetGraph()->CreateInstUnresolvedLoadType(DataType::REFERENCE, GetPc(bcInst->GetAddress()));
            if (!GetGraph()->IsAotMode() && !GetGraph()->IsBytecodeOptimizer()) {
                GetRuntime()->GetUnresolvedTypes()->AddTableSlot(method, typeId,
                                                                 UnresolvedTypesInterface::SlotKind::MANAGED_CLASS);
            }
        } else {
            inst = GetGraph()->CreateInstLoadType(DataType::REFERENCE, GetPc(bcInst->GetAddress()));
        }
        // NOLINTNEXTLINE(readability-misleading-indentation)
    } else {
        // NOLINTNEXTLINE(readability-magic-numbers)
        static_assert(OPCODE == Opcode::LoadString);
        typeId = bcInst->GetId(0).AsFileId().GetOffset();
        if (!GetGraph()->IsDynamicMethod() || GetGraph()->IsBytecodeOptimizer()) {
            inst = GetGraph()->CreateInstLoadString(DataType::REFERENCE, GetPc(bcInst->GetAddress()));
        } else {
            inst = GetGraph()->CreateInstLoadFromConstantPool(DataType::ANY, GetPc(bcInst->GetAddress()));
            inst->CastToLoadFromConstantPool()->SetString(true);
        }
    }
    if (!GetGraph()->IsDynamicMethod() || GetGraph()->IsBytecodeOptimizer()) {
        // Create SaveState instruction
        auto saveState = CreateSaveState(Opcode::SaveState, GetPc(bcInst->GetAddress()));
        inst->SetInput(0, saveState);
        static_cast<LoadFromPool *>(inst)->SetTypeId(typeId);
        static_cast<LoadFromPool *>(inst)->SetMethod(method);
        AddInstruction(saveState);
    } else {
        inst->SetInput(0, GetEnvDefinition(CONST_POOL_IDX));
        inst->CastToLoadFromConstantPool()->SetTypeId(typeId);
        inst->CastToLoadFromConstantPool()->SetMethod(method);
    }

    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (OPCODE == Opcode::LoadString) {
        if (GetGraph()->IsDynamicMethod() && GetGraph()->IsBytecodeOptimizer()) {
            BuildCastToAnyString(bcInst);
        }
    }
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCastToAnyString(const BytecodeInstruction *bcInst)
{
    auto input = GetDefinitionAcc();
    ASSERT(input->GetType() == DataType::REFERENCE);

    auto language = GetRuntime()->GetMethodSourceLanguage(GetMethod());
    auto anyType = GetAnyStringType(language);
    ASSERT(anyType != AnyBaseType::UNDEFINED_TYPE);

    auto box = graph_->CreateInstCastValueToAnyType(GetPc(bcInst->GetAddress()), anyType, input);
    UpdateDefinitionAcc(box);
    AddInstruction(box);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildCastToAnyNumber(const BytecodeInstruction *bcInst)
{
    auto input = GetDefinitionAcc();
    auto type = input->GetType();
    if (input->IsConst() && !DataType::IsFloatType(type)) {
        auto constInsn = input->CastToConstant();
        if (constInsn->GetType() == DataType::INT64) {
            auto value = input->CastToConstant()->GetInt64Value();
            if (value == static_cast<uint32_t>(value)) {
                type = DataType::INT32;
            }
        }
    }

    auto language = GetRuntime()->GetMethodSourceLanguage(GetMethod());
    auto anyType = NumericDataTypeToAnyType(type, language);
    ASSERT(anyType != AnyBaseType::UNDEFINED_TYPE);

    auto box = graph_->CreateInstCastValueToAnyType(GetPc(bcInst->GetAddress()), anyType, input);
    UpdateDefinitionAcc(box);
    AddInstruction(box);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
bool InstBuilder::TryBuildStringCharAtIntrinsic(const BytecodeInstruction *bcInst, bool accRead)
{
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto compressionEnabled = graph_->GetRuntime()->IsCompressedStringsEnabled();
    auto canEncodeCompressedStrCharAt = graph_->GetEncoder()->CanEncodeCompressedStringCharAt();
    if (compressionEnabled && !canEncodeCompressedStrCharAt) {
        return false;
    }
    auto saveStateNullcheck = CreateSaveState(Opcode::SaveState, bcAddr);
    auto nullCheck = graph_->CreateInstNullCheck(DataType::REFERENCE, bcAddr, GetArgDefinition(bcInst, 0, accRead),
                                                 saveStateNullcheck);
    auto arrayLength = graph_->CreateInstLenArray(DataType::INT32, bcAddr, nullCheck, false);

    AddInstruction(saveStateNullcheck);
    AddInstruction(nullCheck);
    AddInstruction(arrayLength);

    Inst *stringLength = nullptr;
    if (compressionEnabled) {
        auto constTwoInst = graph_->FindOrCreateConstant(ark::coretypes::String::STRING_LENGTH_SHIFT);
        stringLength = graph_->CreateInstShr(DataType::INT32, bcAddr, arrayLength, constTwoInst);
        AddInstruction(stringLength);
    } else {
        stringLength = arrayLength;
    }

    auto boundsCheck = graph_->CreateInstBoundsCheck(DataType::INT32, bcAddr, stringLength,
                                                     GetArgDefinition(bcInst, 1, accRead), saveStateNullcheck, false);
    AddInstruction(boundsCheck);

    Inst *inst = nullptr;
    if (compressionEnabled) {
        inst =
            graph_->CreateInstLoadCompressedStringChar(DataType::UINT16, bcAddr, nullCheck, boundsCheck, arrayLength);
    } else {
        inst = graph_->CreateInstLoadArray(DataType::UINT16, bcAddr, nullCheck, boundsCheck);
    }

    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
    return true;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildStringGetIntrinsic(const BytecodeInstruction *bcInst, bool accRead,
                                          RuntimeInterface::IntrinsicId intrinsicId)
{
    auto bcAddr = GetPc(bcInst->GetAddress());
    auto saveState = CreateSaveState(Opcode::SaveState, bcAddr);

    auto stringGet = graph_->CreateInstIntrinsic(DataType::REFERENCE, bcAddr, intrinsicId);
    stringGet->AllocateInputTypes(GetGraph()->GetAllocator(), 3);  // 3: number of inputs

    stringGet->AppendInput(GetArgDefinition(bcInst, 0, accRead));
    stringGet->AddInputType(DataType::REFERENCE);
    stringGet->AppendInput(GetArgDefinition(bcInst, 1, accRead));
    stringGet->AddInputType(DataType::INT32);
    stringGet->AppendInput(saveState);
    stringGet->AddInputType(DataType::NO_TYPE);

    AddInstruction(saveState, stringGet);
    UpdateDefinitionAcc(stringGet);
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <bool IS_ACC_WRITE>
void InstBuilder::BuildLoadFromAnyByName([[maybe_unused]] const BytecodeInstruction *bcInst,
                                         [[maybe_unused]] DataType::Type type)
{
    // NOTE(zhaoziming_hw, #ICFLYC) Support any bytecode in InstBuilder
    COMPILER_LOG(DEBUG, IR_BUILDER) << "Any bytecode not supported yet, skip current function";
    failed_ = true;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <bool IS_ACC_WRITE>
void InstBuilder::BuildStoreFromAnyByName([[maybe_unused]] const BytecodeInstruction *bcInst,
                                          [[maybe_unused]] DataType::Type type)
{
    // NOTE(zhaoziming_hw, #ICFLYC) Support any bytecode in InstBuilder
    COMPILER_LOG(DEBUG, IR_BUILDER) << "Any bytecode not supported yet, skip current function";
    failed_ = true;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLoadFromAnyByIdx([[maybe_unused]] const BytecodeInstruction *bcInst,
                                        [[maybe_unused]] DataType::Type type)
{
    // NOTE(zhaoziming_hw, #ICFLYC) Support any bytecode in InstBuilder
    COMPILER_LOG(DEBUG, IR_BUILDER) << "Any bytecode not supported yet, skip current function";
    failed_ = true;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildStoreFromAnyByIdx([[maybe_unused]] const BytecodeInstruction *bcInst,
                                         [[maybe_unused]] DataType::Type type)
{
    // NOTE(zhaoziming_hw, #ICFLYC) Support any bytecode in InstBuilder
    COMPILER_LOG(DEBUG, IR_BUILDER) << "Any bytecode not supported yet, skip current function";
    failed_ = true;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildLoadFromAnyByVal([[maybe_unused]] const BytecodeInstruction *bcInst,
                                        [[maybe_unused]] DataType::Type type)
{
    // NOTE(zhaoziming_hw, #ICFLYC) Support any bytecode in InstBuilder
    COMPILER_LOG(DEBUG, IR_BUILDER) << "Any bytecode not supported yet, skip current function";
    failed_ = true;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildStoreFromAnyByVal([[maybe_unused]] const BytecodeInstruction *bcInst,
                                         [[maybe_unused]] DataType::Type type)
{
    // NOTE(zhaoziming_hw, #ICFLYC) Support any bytecode in InstBuilder
    COMPILER_LOG(DEBUG, IR_BUILDER) << "Any bytecode not supported yet, skip current function";
    failed_ = true;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
template <bool IS_ACC_WRITE>
void InstBuilder::BuildAnyCall([[maybe_unused]] const BytecodeInstruction *bcInst)
{
    // NOTE(zhaoziming_hw, #ICFLYC) Support any bytecode in InstBuilder
    COMPILER_LOG(DEBUG, IR_BUILDER) << "Any bytecode not supported yet, skip current function";
    failed_ = true;
}

// NOLINTNEXTLINE(misc-definitions-in-headers)
void InstBuilder::BuildIsInstanceAny([[maybe_unused]] const BytecodeInstruction *bcInst,
                                     [[maybe_unused]] DataType::Type type)
{
    // NOTE(zhaoziming_hw, #ICFLYC) Support any bytecode in InstBuilder
    COMPILER_LOG(DEBUG, IR_BUILDER) << "Any bytecode not supported yet, skip current function";
    failed_ = true;
}

}  // namespace ark::compiler

#endif  // PANDA_INST_BUILDER_INL_H
