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

#include "compiler/optimizer/code_generator/codegen.h"

namespace ark::compiler {

void Codegen::EtsGetNativeMethod(IntrinsicInst *inst, Reg dst, [[maybe_unused]] SRCREGS &src)
{
    auto *encoder = GetEncoder();
    ScopedTmpReg methodReg(encoder);

    if (GetGraph()->IsJitOrOsrMode()) {
        auto *method = inst->GetCallMethod();
        encoder->EncodeMov(methodReg, Imm(bit_cast<uintptr_t>(method)));
    } else {
        ScopedTmpReg addrReg(encoder);
        auto *aotData = GetGraph()->GetAotData();
        auto methodId = inst->GetCallMethodId();
        intptr_t offset = aotData->GetCommonSlotOffset(encoder->GetCursorOffset(), methodId);
        encoder->MakeLoadAotTableAddr(offset, addrReg, methodReg);

        auto skipLabel = encoder->CreateLabel();
        encoder->EncodeJump(skipLabel, methodReg, Condition::NE);
        LoadMethod(methodReg);
        CallRuntime(inst, EntrypointId::GET_CALLEE_METHOD, methodReg, RegMask::GetZeroMask(), methodReg,
                    GetTypeIdImm(inst, methodId));
        encoder->EncodeStr(methodReg, MemRef(addrReg));
        encoder->BindLabel(skipLabel);
    }

    auto successLabel = encoder->CreateLabel();
    auto failLabel = encoder->CreateLabel();

    // If native method is not registered, deoptimize
    ScopedTmpReg tmpReg(encoder);
    encoder->EncodeLdr(tmpReg, false, MemRef(methodReg, GetRuntime()->GetNativePointerOffset(GetArch())));
    encoder->EncodeJump(failLabel, tmpReg, Condition::EQ);

    encoder->EncodeMov(dst, methodReg);
    encoder->EncodeJump(successLabel);

    encoder->BindLabel(failLabel);
    encoder->EncodeMov(dst, Imm(0));

    encoder->BindLabel(successLabel);
}

void Codegen::EtsGetNativeMethodManagedClass(IntrinsicInst *inst, Reg dst, SRCREGS &src)
{
    if (GetGraph()->IsJitOrOsrMode()) {
        auto *method = inst->GetCallMethod();
        auto runtimeClass = bit_cast<uintptr_t>(GetRuntime()->GetClass(method));
        auto managedClass = runtimeClass - GetRuntime()->GetRuntimeClassOffset(GetArch());
        GetEncoder()->EncodeMov(dst, Imm(managedClass));
    } else {
        auto methodReg = src[0U];
        GetEncoder()->EncodeLdr(dst, false, MemRef(methodReg, GetRuntime()->GetClassOffset(GetArch())));
        GetEncoder()->EncodeSub(dst, dst, Imm(GetRuntime()->GetRuntimeClassOffset(GetArch())));
    }
}

void Codegen::EtsGetMethodNativePointer(IntrinsicInst *inst, Reg dst, SRCREGS &src)
{
    auto *method = inst->GetCallMethod();
    auto *nativePointer = GetRuntime()->GetMethodNativePointer(method);
    if (GetGraph()->IsJitOrOsrMode() && nativePointer != nullptr) {
        GetEncoder()->EncodeMov(dst, Imm(bit_cast<uintptr_t>(nativePointer)));
    } else {
        auto methodReg = src[0U];
        GetEncoder()->EncodeLdr(dst, false, MemRef(methodReg, GetRuntime()->GetNativePointerOffset(GetArch())));
    }
}

void Codegen::EtsGetNativeApiEnv([[maybe_unused]] IntrinsicInst *inst, Reg dst, [[maybe_unused]] SRCREGS &src)
{
    GetEncoder()->EncodeLdr(dst, false, MemRef(ThreadReg(), GetRuntime()->GetTlsNativeApiOffset(GetArch())));
}

void Codegen::EtsBeginNativeMethod(IntrinsicInst *inst, [[maybe_unused]] Reg dst, [[maybe_unused]] SRCREGS &src)
{
    ASSERT(!HasLiveCallerSavedRegs(inst));
    ASSERT(inst->GetSaveState()->GetRootsRegsMask().none());

    {
        SCOPED_DISASM_STR(this, "Prepare for begin native");
        GetEncoder()->EncodeStr(FpReg(), MemRef(ThreadReg(), GetRuntime()->GetTlsFrameOffset(GetArch())));

        CreateStackMap(inst);
        ScopedTmpReg pc(GetEncoder());
        GetEncoder()->EncodeGetCurrentPc(pc);
        GetEncoder()->EncodeStr(pc, MemRef(ThreadReg(), GetRuntime()->GetTlsNativePcOffset(GetArch())));
    }

    {
        SCOPED_DISASM_STR(this, "Call begin native intrinsic");
        bool switchesToNative = GetRuntime()->IsNecessarySwitchThreadState(inst->GetCallMethod());
        auto id =
            switchesToNative ? EntrypointId::BEGIN_GENERAL_NATIVE_METHOD : EntrypointId::BEGIN_QUICK_NATIVE_METHOD;
        ScopedTmpReg tmpReg(GetEncoder(), true);
        GetEntrypoint(tmpReg, id);
        GetEncoder()->MakeCall(tmpReg);
    }
}

void Codegen::EtsEndNativeMethodPrim(IntrinsicInst *inst, Reg dst, SRCREGS &src)
{
    EtsEndNativeMethod(inst, dst, src, false);
}

void Codegen::EtsEndNativeMethodObj(IntrinsicInst *inst, Reg dst, SRCREGS &src)
{
    EtsEndNativeMethod(inst, dst, src, true);
}

void Codegen::EtsEndNativeMethod(IntrinsicInst *inst, [[maybe_unused]] Reg dst, [[maybe_unused]] SRCREGS &src,
                                 bool isObject)
{
    ASSERT(!HasLiveCallerSavedRegs(inst));
    ASSERT(inst->GetSaveState()->GetRootsRegsMask().none());

    {
        SCOPED_DISASM_STR(this, "Call end native intrinsic");
        bool switchesToNative = GetRuntime()->IsNecessarySwitchThreadState(inst->GetCallMethod());
        EntrypointId id = EntrypointId::INVALID;
        if (switchesToNative) {
            id = isObject ? EntrypointId::END_GENERAL_NATIVE_METHOD_OBJ : EntrypointId::END_GENERAL_NATIVE_METHOD_PRIM;
        } else {
            id = isObject ? EntrypointId::END_QUICK_NATIVE_METHOD_OBJ : EntrypointId::END_QUICK_NATIVE_METHOD_PRIM;
        }
        ScopedTmpReg tmpReg(GetEncoder(), true);
        GetEntrypoint(tmpReg, id);
        GetEncoder()->MakeCall(tmpReg);
    }
}

void Codegen::EtsCheckNativeException(IntrinsicInst *inst, [[maybe_unused]] Reg dst, [[maybe_unused]] SRCREGS &src)
{
    ASSERT(inst->CanThrow());
    auto throwLabel = CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::THROW_NATIVE_EXCEPTION)->GetLabel();

    ScopedTmpReg tmpReg(GetEncoder());
    GetEncoder()->EncodeLdr(tmpReg, false, MemRef(ThreadReg(), GetRuntime()->GetExceptionOffset(GetArch())));
    GetEncoder()->EncodeJump(throwLabel, tmpReg, Condition::NE);
}

void Codegen::EtsWrapObjectNative(WrapObjectNativeInst *wrapObject)
{
    Inst *obj = wrapObject->GetDataFlowInput(0);
    ASSERT(obj != nullptr);
    CallInst *callNative = wrapObject->GetUsers().Front().GetInst()->CastToCallNative();
    ASSERT(callNative != nullptr);

    ASSERT(GetGraph()->IsAnalysisValid<LivenessAnalyzer>());
    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    auto targetLifeNumber = la.GetInstLifeIntervals(callNative)->GetBegin();

    auto *interval = la.GetInstLifeIntervals(obj)->FindSiblingAt(targetLifeNumber);
    ASSERT(interval != nullptr);
    auto location = interval->GetLocation();
    ASSERT(location.GetKind() == LocationType::STACK_PARAMETER || location.GetKind() == LocationType::STACK);

    auto dstReg = ConvertRegister(wrapObject->GetDstReg(), wrapObject->GetType());
    GetEncoder()->EncodeAdd(dstReg, SpReg(), Imm(GetStackOffset(location)));
    // don't apply stack ref mask, since it is empty
    ASSERT(GetRuntime()->GetStackReferenceMask() == 0U);
}

bool Codegen::ResolveCallByNameCodegen(ResolveVirtualInst *resolver)
{
    SCOPED_DISASM_STR(this, "Create runtime call to resolve a call by name");
    ASSERT(resolver->GetOpcode() == Opcode::ResolveByName);
    ASSERT(resolver->GetCallMethod() != nullptr);

    auto methodReg = ConvertRegister(resolver->GetDstReg(), resolver->GetType());
    auto objectReg = ConvertRegister(resolver->GetSrcReg(0), DataType::REFERENCE);
    ScopedTmpReg tmpMethodReg(GetEncoder());
    LoadMethod(tmpMethodReg);

    CallRuntime(resolver, EntrypointId::RESOLVE_CALL_BY_NAME, tmpMethodReg, {}, tmpMethodReg, objectReg,
                TypedImm(resolver->GetCallMethodId()));
    GetEncoder()->EncodeMov(methodReg, tmpMethodReg);
    return true;
}

}  // namespace ark::compiler
