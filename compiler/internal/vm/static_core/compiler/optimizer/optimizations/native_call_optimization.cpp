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
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/optimizations/native_call_optimization.h"

namespace ark::compiler {

void NativeCallOptimization::InvalidateAnalyses()
{
    GetGraph()->InvalidateAnalysis<BoundsAnalysis>();
    GetGraph()->InvalidateAnalysis<AliasAnalysis>();
}

bool NativeCallOptimization::RunImpl()
{
    if (!GetGraph()->CanOptimizeNativeMethods()) {
        COMPILER_LOG(DEBUG, NATIVE_CALL_OPT)
            << "Graph " << GetGraph()->GetRuntime()->GetMethodFullName(GetGraph()->GetMethod(), true)
            << " cannot optimize native methods, skip";
        return false;
    }

    VisitGraph();
    return IsApplied();
}

void NativeCallOptimization::VisitCallStatic(GraphVisitor *v, Inst *inst)
{
    CallInst *callInst = inst->CastToCallStatic();
    auto *that = static_cast<NativeCallOptimization *>(v);
    auto *runtime = that->GetGraph()->GetRuntime();

    if (!callInst->GetIsNative()) {
        COMPILER_LOG(DEBUG, NATIVE_CALL_OPT) << "CallStatic with id=" << callInst->GetId() << " is not native, skip";
        // NOTE: add event here!
        return;
    }

    // NOTE: workaround, need to enable back after fixing stack walker & gc roots issue
    // #24400 Compiler ANI segfault in StackWalker
    if (runtime->IsNecessarySwitchThreadState(callInst->GetCallMethod())) {
        COMPILER_LOG(DEBUG, NATIVE_CALL_OPT)
            << "CallStatic with id=" << callInst->GetId() << " needs to switch exec state, skip (workaround)";
        return;
    }

    // NOTE: workaround, need to enable back after fixing stack walker & gc roots issue
    // #24400 Compiler ANI segfault in StackWalker
    if (runtime->CanNativeMethodUseObjects(callInst->GetCallMethod())) {
        COMPILER_LOG(DEBUG, NATIVE_CALL_OPT)
            << "CallStatic with id=" << callInst->GetId() << " has objects, skip (workaround)";
        return;
    }

    if (runtime->IsMethodInModuleScope(callInst->GetCallMethod())) {
        COMPILER_LOG(DEBUG, NATIVE_CALL_OPT)
            << "CallStatic with id=" << callInst->GetId() << " is in module scope, skip (workaround)";
        return;
    }

    if (runtime->CanNativeMethodUseObjects(callInst->GetCallMethod())) {
        ASSERT(callInst->GetCanNativeException());
        OptimizeNativeCallWithObjects(v, callInst);
    } else {
        ASSERT(!callInst->GetCanNativeException());
        OptimizePrimitiveNativeCall(v, callInst);
    }
}

IntrinsicInst *NativeCallOptimization::CreateNativeApiIntrinsic(DataType::Type type, uint32_t pc,
                                                                RuntimeInterface::IntrinsicId id,
                                                                const MethodDataMixin *methodData)
{
    IntrinsicInst *intrinsic = GetGraph()->CreateInstIntrinsic(type, pc, id)->CastToIntrinsic();
    intrinsic->SetCallMethodId(methodData->GetCallMethodId());
    intrinsic->SetCallMethod(methodData->GetCallMethod());
    return intrinsic;
}

void NativeCallOptimization::OptimizePrimitiveNativeCall(GraphVisitor *v, CallInst *callInst)
{
    auto *that = static_cast<NativeCallOptimization *>(v);
    auto *graph = that->GetGraph();

    auto pc = callInst->GetPc();
    auto *methodData = static_cast<MethodDataMixin *>(callInst);
    auto *saveState = callInst->GetSaveState();
    ASSERT(saveState != nullptr);

    IntrinsicInst *getNativeMethod = that->CreateNativeApiIntrinsic(
        DataType::POINTER, pc, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_GET_NATIVE_METHOD, methodData);
    getNativeMethod->SetInputs(graph->GetAllocator(), {{saveState, saveState->GetType()}});
    if (graph->IsJitOrOsrMode()) {
        getNativeMethod->ClearFlag(inst_flags::Flags::RUNTIME_CALL);
    }

    IntrinsicInst *getNativePointer = that->CreateNativeApiIntrinsic(
        DataType::POINTER, pc, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_GET_METHOD_NATIVE_POINTER, methodData);
    getNativePointer->SetInputs(graph->GetAllocator(), {{getNativeMethod, getNativeMethod->GetType()}});

    callInst->InsertBefore(getNativeMethod);
    callInst->InsertBefore(getNativePointer);

    CallInst *callNative = callInst->Clone(graph)->CastToCallStatic();
    callNative->SetOpcode(Opcode::CallNative);
    callNative->GetInputTypes()->clear();
    callNative->AppendInput(getNativePointer, getNativePointer->GetType());
    for (auto input : callInst->GetInputs()) {
        auto *inputInst = input.GetInst();
        callNative->AppendInput(inputInst, inputInst->GetType());
    }

    callInst->ReplaceUsers(callNative);
    callInst->RemoveInputs();
    callInst->GetBasicBlock()->ReplaceInst(callInst, callNative);

    callNative->ClearFlag(inst_flags::Flags::CAN_THROW);
    callNative->ClearFlag(inst_flags::Flags::HEAP_INV);
    callNative->ClearFlag(inst_flags::Flags::REQUIRE_STATE);
    callNative->ClearFlag(inst_flags::Flags::RUNTIME_CALL);

    // remove save state from managed call
    callNative->RemoveInput(callNative->GetInputsCount() - 1U);

    that->SetIsApplied();
    COMPILER_LOG(DEBUG, NATIVE_CALL_OPT) << "CallStatic with id=" << callInst->GetId()
                                         << " is native and was replaced with CallNative with id="
                                         << callNative->GetId();
    // NOTE: add event here!
}

// CC-OFFNXT(huge_method[C++]) solid logic
void NativeCallOptimization::OptimizeNativeCallWithObjects(GraphVisitor *v, CallInst *callInst)
{
    auto *that = static_cast<NativeCallOptimization *>(v);
    auto *graph = that->GetGraph();
    auto *runtime = graph->GetRuntime();

    auto pc = callInst->GetPc();
    auto *methodData = static_cast<MethodDataMixin *>(callInst);
    auto *externalSaveState = callInst->GetSaveState();
    ASSERT(externalSaveState != nullptr);

    auto *internalSaveState = CopySaveState(graph, externalSaveState);
    internalSaveState->RemoveNumericInputs();
    for (size_t i = 0U; i < internalSaveState->GetInputsCount(); ++i) {
        internalSaveState->SetVirtualRegister(i, VirtualRegister(VirtualRegister::BRIDGE, VRegType::VREG));
    }
    internalSaveState->SetCallerInst(callInst);
    internalSaveState->SetInliningDepth(externalSaveState->GetInliningDepth() + 1U);
    internalSaveState->SetPc(INVALID_PC);

    Inst *thisInput = nullptr;
    if (!runtime->IsMethodStatic(callInst->GetCallMethod())) {
        auto *nullcheckInst = callInst->GetObjectInst();
        if (nullcheckInst->GetOpcode() == Opcode::NullCheck) {
            thisInput = nullcheckInst;
        }
    }

    IntrinsicInst *getNativeMethod = that->CreateNativeApiIntrinsic(
        DataType::POINTER, pc, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_GET_NATIVE_METHOD, methodData);
    getNativeMethod->SetInputs(graph->GetAllocator(), {{externalSaveState, externalSaveState->GetType()}});
    if (graph->IsJitOrOsrMode()) {
        getNativeMethod->ClearFlag(inst_flags::Flags::RUNTIME_CALL);
    }

    auto deoptComp = graph->CreateInstCompare(DataType::BOOL, pc, getNativeMethod, graph->FindOrCreateConstant(0),
                                              DataType::POINTER, CC_EQ);
    auto deoptimizeIf =
        graph->CreateInstDeoptimizeIf(pc, deoptComp, externalSaveState, DeoptimizeType::NOT_SUPPORTED_NATIVE);

    Inst *getManagedClass = nullptr;
    if (runtime->IsMethodStatic(callInst->GetCallMethod())) {
        getManagedClass = that->CreateNativeApiIntrinsic(
            DataType::REFERENCE, INVALID_PC,
            RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_GET_NATIVE_METHOD_MANAGED_CLASS, methodData);
        getManagedClass->CastToIntrinsic()->SetInputs(graph->GetAllocator(),
                                                      {{getNativeMethod, getNativeMethod->GetType()}});
    }

    IntrinsicInst *getNativePointer = that->CreateNativeApiIntrinsic(
        DataType::POINTER, INVALID_PC, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_GET_METHOD_NATIVE_POINTER,
        methodData);
    getNativePointer->SetInputs(graph->GetAllocator(), {{getNativeMethod, getNativeMethod->GetType()}});

    IntrinsicInst *getNativeApiEnv = that->CreateNativeApiIntrinsic(
        DataType::POINTER, INVALID_PC, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_GET_NATIVE_API_ENV,
        methodData);

    IntrinsicInst *beginNativeMethod = that->CreateNativeApiIntrinsic(
        DataType::VOID, INVALID_PC, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_BEGIN_NATIVE_METHOD, methodData);
    beginNativeMethod->SetInputs(graph->GetAllocator(), {{internalSaveState, internalSaveState->GetType()}});

    CallInst *callNative = callInst->Clone(graph)->CastToCallStatic();
    callNative->SetOpcode(Opcode::CallNative);
    callNative->GetInputTypes()->clear();
    callInst->ReplaceUsers(callNative);

    IntrinsicInst *endNativeMethod = nullptr;
    if (!DataType::IsReference(callNative->GetType())) {
        endNativeMethod = that->CreateNativeApiIntrinsic(
            DataType::VOID, INVALID_PC, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_END_NATIVE_METHOD_PRIM,
            methodData);
        endNativeMethod->SetInputs(graph->GetAllocator(), {{internalSaveState, internalSaveState->GetType()}});
    } else {
        callNative->SetType(DataType::POINTER);
        endNativeMethod = that->CreateNativeApiIntrinsic(
            DataType::REFERENCE, INVALID_PC, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_END_NATIVE_METHOD_OBJ,
            methodData);
        callNative->ReplaceUsers(endNativeMethod);
        endNativeMethod->SetInputs(graph->GetAllocator(), {{callNative, callNative->GetType()},
                                                           {internalSaveState, internalSaveState->GetType()}});
    }

    auto *checkException = that->CreateNativeApiIntrinsic(
        DataType::VOID, pc, RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_CHECK_NATIVE_EXCEPTION, methodData);
    checkException->SetInputs(graph->GetAllocator(), {{externalSaveState, externalSaveState->GetType()}});

    auto *returnInlined = graph->CreateInstReturnInlined(DataType::VOID, pc, externalSaveState);

    callInst->InsertBefore(getNativeMethod);
    callInst->InsertBefore(deoptComp);
    callInst->InsertBefore(deoptimizeIf);
    InstAppender appender(callInst->GetBasicBlock(), callInst);
    if (getManagedClass != nullptr) {
        appender.Append({internalSaveState, getManagedClass, getNativePointer, getNativeApiEnv, beginNativeMethod});
    } else {
        appender.Append({internalSaveState, getNativePointer, getNativeApiEnv, beginNativeMethod});
    }
    for (size_t i = 0U; i < callInst->GetInputsCount(); ++i) {
        Inst *input = callInst->GetInput(i).GetInst();
        if (DataType::IsReference(input->GetType())) {
            auto *wrapObjectNative = graph->CreateInstWrapObjectNative(DataType::POINTER, INVALID_PC, input);
            callInst->SetInput(i, wrapObjectNative);
            appender.Append(wrapObjectNative);
        }
    }
    if (getManagedClass != nullptr) {
        auto *wrapObjectNative = graph->CreateInstWrapObjectNative(DataType::POINTER, INVALID_PC, getManagedClass);
        appender.Append(wrapObjectNative);
        getManagedClass = wrapObjectNative;
    }
    appender.Append({callNative, endNativeMethod, returnInlined, checkException});

    callNative->AppendInput(getNativePointer, getNativePointer->GetType());
    callNative->AppendInput(getNativeApiEnv, getNativeApiEnv->GetType());
    if (getManagedClass != nullptr) {
        callNative->AppendInput(getManagedClass, getManagedClass->GetType());
    }
    for (auto input : callInst->GetInputs()) {
        auto *inputInst = input.GetInst();
        callNative->AppendInput(inputInst, inputInst->GetType());
    }
    callNative->SetSaveState(internalSaveState);

    callInst->RemoveInputs();
    if (thisInput != nullptr) {
        callInst->AppendInput(thisInput);
    }
    callInst->AppendInput(externalSaveState);

    callInst->SetInlined(true);
    callInst->SetFlag(inst_flags::NO_DST);
    callInst->SetFlag(inst_flags::NO_DCE);

    static_cast<NativeCallOptimization *>(v)->SetIsApplied();
    COMPILER_LOG(DEBUG, NATIVE_CALL_OPT) << "CallStatic with id=" << callInst->GetId()
                                         << " is native and was replaced with CallNative with id="
                                         << callNative->GetId();
    // NOTE: add event here!
}

}  // namespace ark::compiler
