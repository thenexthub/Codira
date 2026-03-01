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

#include "codegen_fastpath.h"
#include "optimizer/ir/inst.h"
#include "relocations.h"

namespace ark::compiler {

static void SaveCallerRegistersInFrame(RegMask mask, Encoder *encoder, const CFrameLayout &fl, bool isFp)
{
    if (mask.none()) {
        return;
    }
    auto fpReg = Target(fl.GetArch()).GetFrameReg();

    mask &= GetCallerRegsMask(fl.GetArch(), isFp);
    auto startSlot = static_cast<size_t>(fl.GetStackStartSlot()) + fl.GetCallerLastSlot(isFp);
    encoder->SaveRegisters(mask, isFp, -startSlot, fpReg, GetCallerRegsMask(fl.GetArch(), isFp));
}

static void RestoreCallerRegistersFromFrame(RegMask mask, Encoder *encoder, const CFrameLayout &fl, bool isFp)
{
    if (mask.none()) {
        return;
    }
    auto fpReg = Target(fl.GetArch()).GetFrameReg();

    mask &= GetCallerRegsMask(fl.GetArch(), isFp);
    auto startSlot = static_cast<size_t>(fl.GetStackStartSlot()) + fl.GetCallerLastSlot(isFp);
    encoder->LoadRegisters(mask, isFp, -startSlot, fpReg, GetCallerRegsMask(fl.GetArch(), isFp));
}

static bool InstHasRuntimeCall(const Inst *inst)
{
    switch (inst->GetOpcode()) {
        case Opcode::Store:
            if (inst->CastToStore()->GetNeedBarrier()) {
                return true;
            }
            break;
        case Opcode::StoreI:
            if (inst->CastToStoreI()->GetNeedBarrier()) {
                return true;
            }
            break;
        case Opcode::StoreArray:
            if (inst->CastToStoreArray()->GetNeedBarrier()) {
                return true;
            }
            break;
        case Opcode::StoreObject:
            if (inst->CastToStoreObject()->GetNeedBarrier()) {
                return true;
            }
            break;
        case Opcode::LoadObjectDynamic:
        case Opcode::StoreObjectDynamic:
            return true;
        case Opcode::Cast:
            if (inst->CastToCast()->IsDynamicCast()) {
                return true;
            }
            break;
        default:
            break;
    }
    if (inst->IsRuntimeCall()) {
        if (!inst->IsIntrinsic()) {
            return true;
        }
        auto intrinsicId = inst->CastToIntrinsic()->GetIntrinsicId();
        if (intrinsicId != RuntimeInterface::IntrinsicId::INTRINSIC_SLOW_PATH_ENTRY &&
            intrinsicId != RuntimeInterface::IntrinsicId::INTRINSIC_TAIL_CALL) {
            return true;
        }
    }
    return false;
}

/*
 * We determine runtime calls manually, not using MethodProperties::HasRuntimeCalls, because we need to ignore
 * SLOW_PATH_ENTRY intrinsic, since it doesn't require LR to be preserved.
 */
static bool HasRuntimeCalls(const Graph &graph)
{
    for (auto bb : graph.GetBlocksRPO()) {
        for (auto inst : bb->Insts()) {
            if (InstHasRuntimeCall(inst)) {
                return true;
            }
        }
    }
    return false;
}

void CodegenFastPath::GeneratePrologue()
{
    SCOPED_DISASM_STR(this, "FastPath Prologue");

    auto callerRegs = RegMask(GetCallerRegsMask(GetArch(), false));
    auto argsNum = GetRuntime()->GetMethodArgumentsCount(GetGraph()->GetMethod());
    callerRegs &= GetUsedRegs() & ~GetTarget().GetParamRegsMask(argsNum);
    SaveCallerRegistersInFrame(callerRegs, GetEncoder(), GetFrameLayout(), false);

    auto hasRuntimeCalls = HasRuntimeCalls(*GetGraph());

    savedRegisters_ = GetUsedRegs() & RegMask(GetCalleeRegsMask(GetArch(), false));
    if (GetTarget().SupportLinkReg() && hasRuntimeCalls) {
        savedRegisters_ |= GetTarget().GetLinkReg().GetMask();
        GetEncoder()->EnableLrAsTempReg(true);
    }

    if (GetUsedVRegs().Any()) {
        SaveCallerRegistersInFrame(GetUsedVRegs() & GetCallerRegsMask(GetArch(), true), GetEncoder(), GetFrameLayout(),
                                   true);
        savedFpRegisters_ = GetUsedVRegs() & VRegMask(GetCalleeRegsMask(GetArch(), true));
    }

    GetEncoder()->PushRegisters(savedRegisters_, savedFpRegisters_, GetTarget().SupportLinkReg());

    if (GetFrameInfo()->GetSpillsCount() != 0) {
        GetEncoder()->EncodeSub(
            GetTarget().GetStackReg(), GetTarget().GetStackReg(),
            Imm(RoundUp(GetFrameInfo()->GetSpillsCount() * GetTarget().WordSize(), GetTarget().GetSpAlignment())));
    }
}

RegMask CodegenFastPath::GetCallerRegistersToRestore() const
{
    RegMask callerRegs = GetUsedRegs() & RegMask(GetCallerRegsMask(GetArch(), false));

    auto argsNum = GetRuntime()->GetMethodArgumentsCount(GetGraph()->GetMethod());
    callerRegs &= ~GetTarget().GetParamRegsMask(argsNum);

    if (auto retType {GetRuntime()->GetMethodReturnType(GetGraph()->GetMethod())};
        retType != DataType::VOID && retType != DataType::NO_TYPE) {
        ASSERT(!DataType::IsFloatType(retType));
        callerRegs.reset(GetTarget().GetReturnRegId());
    }
    return callerRegs;
}

void CodegenFastPath::GenerateEpilogue()
{
    SCOPED_DISASM_STR(this, "FastPath Epilogue");

    if (GetFrameInfo()->GetSpillsCount() != 0) {
        GetEncoder()->EncodeAdd(
            GetTarget().GetStackReg(), GetTarget().GetStackReg(),
            Imm(RoundUp(GetFrameInfo()->GetSpillsCount() * GetTarget().WordSize(), GetTarget().GetSpAlignment())));
    }

    RestoreCallerRegistersFromFrame(GetCallerRegistersToRestore(), GetEncoder(), GetFrameLayout(), false);

    if (GetUsedVRegs().Any()) {
        RestoreCallerRegistersFromFrame(GetUsedVRegs() & GetCallerRegsMask(GetArch(), true), GetEncoder(),
                                        GetFrameLayout(), true);
    }

    GetEncoder()->PopRegisters(savedRegisters_, savedFpRegisters_, GetTarget().SupportLinkReg());

    GetEncoder()->EncodeReturn();
}

void CodegenFastPath::CreateFrameInfo()
{
    auto frame = GetGraph()->GetLocalAllocator()->New<FrameInfo>(
        FrameInfo::PositionedCallers::Encode(true) | FrameInfo::PositionedCallees::Encode(false) |
        FrameInfo::CallersRelativeFp::Encode(true) | FrameInfo::CalleesRelativeFp::Encode(false) |
        FrameInfo::PushCallers::Encode(true));
    ASSERT(frame != nullptr);
    frame->SetSpillsCount(GetGraph()->GetStackSlotsCount());
    CFrameLayout fl(GetGraph()->GetArch(), GetGraph()->GetStackSlotsCount(), false);

    frame->SetCallersOffset(fl.GetOffset<CFrameLayout::OffsetOrigin::SP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCallerLastSlot(false)));
    frame->SetFpCallersOffset(fl.GetOffset<CFrameLayout::OffsetOrigin::SP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCallerLastSlot(true)));
    frame->SetCalleesOffset(-fl.GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCalleeLastSlot(false)));
    frame->SetFpCalleesOffset(-fl.GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::SLOTS>(
        fl.GetStackStartSlot() + fl.GetCalleeLastSlot(true)));

    SetFrameInfo(frame);
}

/*
 * Safe call of the c++ function from the irtoc
 * */
void CodegenFastPath::CreateTailCall(IntrinsicInst *inst, bool isFastpath)
{
    auto encoder = GetEncoder();

    if (GetFrameInfo()->GetSpillsCount() != 0) {
        encoder->EncodeAdd(
            GetTarget().GetStackReg(), GetTarget().GetStackReg(),
            Imm(RoundUp(GetFrameInfo()->GetSpillsCount() * GetTarget().WordSize(), GetTarget().GetSpAlignment())));
    }

    /* Once we reach the slow path, we can release all temp registers, since slow path terminates execution */
    auto tempsMask = GetTarget().GetTempRegsMask();
    for (size_t reg = tempsMask.GetMinRegister(); reg <= tempsMask.GetMaxRegister(); reg++) {
        if (tempsMask.Test(reg)) {
            encoder->ReleaseScratchRegister(Reg(reg, INT32_TYPE));
        }
    }

    if (isFastpath) {
        RestoreCallerRegistersFromFrame(GetCallerRegistersToRestore(), encoder, GetFrameLayout(), false);
        if (GetUsedVRegs().Any()) {
            RestoreCallerRegistersFromFrame(GetUsedVRegs() & GetCallerRegsMask(GetArch(), true), encoder,
                                            GetFrameLayout(), true);
        }
    } else {
        RegMask callerRegs = ~GetUsedRegs() & RegMask(GetCallerRegsMask(GetArch(), false));
        auto argsNum = GetRuntime()->GetMethodArgumentsCount(GetGraph()->GetMethod());
        callerRegs &= ~GetTarget().GetParamRegsMask(argsNum);

        if (GetUsedVRegs().Any()) {
            VRegMask fpCallerRegs = ~GetUsedVRegs() & RegMask(GetCallerRegsMask(GetArch(), true));
            SaveCallerRegistersInFrame(fpCallerRegs, encoder, GetFrameLayout(), true);
        }

        SaveCallerRegistersInFrame(callerRegs, encoder, GetFrameLayout(), false);
    }
    encoder->PopRegisters(savedRegisters_, savedFpRegisters_, GetTarget().SupportLinkReg());

    /* First Imm is offset of the runtime entrypoint for Ark Irtoc */
    /* Second Imm is necessary for proper LLVM Irtoc FastPath compilation */
    CHECK_LE(inst->GetImms().size(), 2U);
    if (inst->GetRelocate()) {
        RelocationInfo relocation;
        encoder->EncodeJump(&relocation);
        GetGraph()->GetRelocationHandler()->AddRelocation(relocation);
    } else {
        ScopedTmpReg tmp(encoder);
        auto offset = inst->GetImms()[0];
        GetEntrypoint(tmp, offset);
        encoder->EncodeJump(tmp);
    }
}

void CodegenFastPath::EmitSimdIntrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto intrinsic = inst->GetIntrinsicId();
    if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_COMPRESS_EIGHT_UTF16_TO_UTF8_CHARS_USING_SIMD) {
        GetEncoder()->EncodeCompressEightUtf16ToUtf8CharsUsingSimd(src[FIRST_OPERAND], src[SECOND_OPERAND]);
    } else if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_COMPRESS_SIXTEEN_UTF16_TO_UTF8_CHARS_USING_SIMD) {
        GetEncoder()->EncodeCompressSixteenUtf16ToUtf8CharsUsingSimd(src[FIRST_OPERAND], src[SECOND_OPERAND]);
    } else if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_MEM_CHAR_U8_X32_USING_SIMD) {
        GetEncoder()->EncodeMemCharU8X32UsingSimd(dst, src[FIRST_OPERAND], src[SECOND_OPERAND],
                                                  ConvertInstTmpReg(inst, DataType::FLOAT64));
    } else if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_MEM_CHAR_U16_X16_USING_SIMD) {
        GetEncoder()->EncodeMemCharU16X16UsingSimd(dst, src[FIRST_OPERAND], src[SECOND_OPERAND],
                                                   ConvertInstTmpReg(inst, DataType::FLOAT64));
    } else if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_MEM_CHAR_U8_X16_USING_SIMD) {
        GetEncoder()->EncodeMemCharU8X16UsingSimd(dst, src[FIRST_OPERAND], src[SECOND_OPERAND],
                                                  ConvertInstTmpReg(inst, DataType::FLOAT64));
    } else if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_MEM_CHAR_U16_X8_USING_SIMD) {
        GetEncoder()->EncodeMemCharU16X8UsingSimd(dst, src[FIRST_OPERAND], src[SECOND_OPERAND],
                                                  ConvertInstTmpReg(inst, DataType::FLOAT64));
    } else if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_MEM_LAST_CHAR_U16_X8_USING_SIMD) {
        GetEncoder()->EncodeMemLastCharU16X8UsingSimd(dst, src[FIRST_OPERAND], src[SECOND_OPERAND],
                                                      ConvertInstTmpReg(inst, DataType::FLOAT64));
    } else if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_MEM_LAST_CHAR_U8_X16_USING_SIMD) {
        GetEncoder()->EncodeMemLastCharU8X16UsingSimd(dst, src[FIRST_OPERAND], src[SECOND_OPERAND],
                                                      ConvertInstTmpReg(inst, DataType::FLOAT64));
    } else if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_CHARS_TO_UPPER_CASE_U8_X16_USING_SIMD) {
        GetEncoder()->EncodeCharsToUpperCaseU8X16UsingSimd(src[FIRST_OPERAND], src[SECOND_OPERAND],
                                                           ConvertInstTmpReg(inst, DataType::FLOAT64));
    } else if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_CHARS_TO_UPPER_CASE_U8_X8_USING_SIMD) {
        GetEncoder()->EncodeCharsToUpperCaseU8X8UsingSimd(src[FIRST_OPERAND], src[SECOND_OPERAND],
                                                          ConvertInstTmpReg(inst, DataType::FLOAT64));
    } else if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_CHARS_TO_LOWER_CASE_U8_X16_USING_SIMD) {
        GetEncoder()->EncodeCharsToLowerCaseU8X16UsingSimd(src[FIRST_OPERAND], src[SECOND_OPERAND],
                                                           ConvertInstTmpReg(inst, DataType::FLOAT64));
    } else if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_CHARS_TO_LOWER_CASE_U8_X8_USING_SIMD) {
        GetEncoder()->EncodeCharsToLowerCaseU8X8UsingSimd(src[FIRST_OPERAND], src[SECOND_OPERAND],
                                                          ConvertInstTmpReg(inst, DataType::FLOAT64));
    } else {
        UNREACHABLE();
    }
}

void CodegenFastPath::EmitReverseIntrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto intrinsic = inst->GetIntrinsicId();
    if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_REVERSE_BYTES_U64 ||
        intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_REVERSE_BYTES_U32) {
        GetEncoder()->EncodeReverseBytes(dst, src[0]);
    } else if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_REVERSE_HALF_WORDS) {
        GetEncoder()->EncodeReverseHalfWords(dst, src[0]);
    } else {
        UNREACHABLE();
    }
}

void CodegenFastPath::EmitMarkWordIntrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto intrinsic = inst->GetIntrinsicId();
    if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_LOAD_ACQUIRE_MARK_WORD_EXCLUSIVE) {
        GetEncoder()->EncodeLdrExclusive(dst, src[0], true);
    } else if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_STORE_RELEASE_MARK_WORD_EXCLUSIVE) {
        GetEncoder()->EncodeStrExclusive(dst, src[SECOND_OPERAND], src[0], true);
    } else if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_COMPARE_AND_SET_MARK_WORD) {
        GetEncoder()->EncodeCompareAndSwap(dst, src[0], src[SECOND_OPERAND], src[THIRD_OPERAND]);
    } else {
        UNREACHABLE();
    }
}

void CodegenFastPath::EmitDataMemoryBarrierFullIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                                         [[maybe_unused]] SRCREGS src)
{
    ASSERT(inst->GetIntrinsicId() == RuntimeInterface::IntrinsicId::INTRINSIC_DATA_MEMORY_BARRIER_FULL);
    GetEncoder()->EncodeMemoryBarrier(memory_order::FULL);
}

/*
 * Safe call of the c++ function from the irtoc
 */
void CodegenFastPath::EmitWriteTlabStatsSafeIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                                      SRCREGS src)
{
    ASSERT(inst->GetIntrinsicId() == RuntimeInterface::IntrinsicId::INTRINSIC_WRITE_TLAB_STATS_SAFE);
    ASSERT(!inst->HasUsers());

    auto src1 = src[FIRST_OPERAND];
    auto src2 = src[SECOND_OPERAND];

    auto regs = GetCallerRegsMask(GetArch(), false) | GetCalleeRegsMask(GetArch(), false);
    auto vregs = GetCallerRegsMask(GetArch(), true);
    GetEncoder()->PushRegisters(regs, vregs);

    FillCallParams(src1, src2);

    auto id = RuntimeInterface::EntrypointId::WRITE_TLAB_STATS_NO_BRIDGE;
    constexpr size_t NUM_OF_ARGS = 2;
    // Temp registers are not available because they are busy in irtoc tlab allocation. So it is faked with param reg.
    auto tmp = GetEncoder()->GetTarget().GetParamReg(NUM_OF_ARGS);
    GetEntrypoint(tmp, id);
    GetEncoder()->MakeCall(tmp);

    GetEncoder()->PopRegisters(regs, vregs);
}

void CodegenFastPath::EmitExpandU8ToU16Intrinsic([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(inst->GetIntrinsicId() == RuntimeInterface::IntrinsicId::INTRINSIC_EXPAND_U8_TO_U16);
    GetEncoder()->EncodeUnsignedExtendBytesToShorts(dst, src[0]);
}

void CodegenFastPath::EmitAtomicByteOrIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                                SRCREGS src)
{
    ASSERT(inst->GetIntrinsicId() == RuntimeInterface::IntrinsicId::INTRINSIC_ATOMIC_BYTE_OR);
    bool fastEncoding = true;
    if (GetArch() == Arch::AARCH64 && !g_options.IsCpuFeatureEnabled(CpuFeature::ATOMICS)) {
        fastEncoding = false;
    }
    GetEncoder()->EncodeAtomicByteOr(src[FIRST_OPERAND], src[SECOND_OPERAND], fastEncoding);
}

void CodegenFastPath::EmitSaveOrRestoreRegsEpIntrinsic(IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                                       [[maybe_unused]] SRCREGS src)
{
    RegMask calleeRegs = GetUsedRegs() & RegMask(GetCalleeRegsMask(GetArch(), false));
    // We need to restore all caller regs, since caller doesn't care about registers at all (except parameters)
    auto callerRegs = RegMask(GetCallerRegsMask(GetArch(), false));
    auto callerVregs = RegMask(GetCallerRegsMask(GetArch(), true));
    for (auto &input : inst->GetInputs()) {
        calleeRegs.reset(input.GetInst()->GetDstReg());
        callerRegs.reset(input.GetInst()->GetDstReg());
    }
    if (GetTarget().SupportLinkReg()) {
        callerRegs.set(GetTarget().GetLinkReg().GetId());
    }
    if (!inst->HasUsers()) {
        callerRegs.set(GetTarget().GetReturnReg(GetPtrRegType()).GetId());
    }
    auto intrinsic = inst->GetIntrinsicId();
    if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_SAVE_REGISTERS_EP) {
        GetEncoder()->PushRegisters(callerRegs | calleeRegs, callerVregs);
    } else if (intrinsic == RuntimeInterface::IntrinsicId::INTRINSIC_RESTORE_REGISTERS_EP) {
        GetEncoder()->PopRegisters(callerRegs | calleeRegs, callerVregs);
    } else {
        UNREACHABLE();
    }
}

void CodegenFastPath::EmitTailCallIntrinsic(IntrinsicInst *inst, [[maybe_unused]] Reg dst, [[maybe_unused]] SRCREGS src)
{
    ASSERT(inst->GetIntrinsicId() == RuntimeInterface::IntrinsicId::INTRINSIC_TAIL_CALL);
    CreateTailCall(inst, true);
}

void CodegenFastPath::EmitSlowPathEntryIntrinsic(IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                                 [[maybe_unused]] SRCREGS src)
{
    ASSERT(inst->GetIntrinsicId() == RuntimeInterface::IntrinsicId::INTRINSIC_SLOW_PATH_ENTRY);
    CreateTailCall(inst, false);
}

void CodegenFastPath::EmitJsCastDoubleToCharIntrinsic([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(inst->GetIntrinsicId() == RuntimeInterface::IntrinsicId::INTRINSIC_JS_CAST_DOUBLE_TO_CHAR);
    auto srcReg = src[FIRST_OPERAND];

    CHECK_EQ(srcReg.GetSize(), BITS_PER_UINT64);
    CHECK_EQ(dst.GetSize(), BITS_PER_UINT32);
    dst = dst.As(INT32_TYPE);

    auto enc {GetEncoder()};
    if (g_options.IsCpuFeatureEnabled(CpuFeature::JSCVT)) {
        // no failure may occur
        enc->EncodeJsDoubleToCharCast(dst, srcReg);
    } else {
        constexpr uint32_t FAILURE_RESULT_FLAG = (1U << 16U);
        ScopedTmpRegU32 tmp(enc);
        enc->EncodeJsDoubleToCharCast(dst, srcReg, tmp, FAILURE_RESULT_FLAG);
    }
}
}  // namespace ark::compiler
