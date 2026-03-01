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
/*
Low-level calling convention
*/
#include "optimizer/code_generator/codegen_load_entrypoint.h"
#include "scoped_tmp_reg.h"
#include "target/amd64/target.h"

namespace ark::compiler::amd64 {

Amd64CallingConvention::Amd64CallingConvention(ArenaAllocator *allocator, Encoder *enc, RegistersDescription *descr,
                                               CallConvMode mode)
    : CallingConvention(allocator, enc, descr, mode)
{
}

constexpr auto Amd64CallingConvention::GetTarget()
{
    return ark::compiler::Target(Arch::X86_64);
}

bool Amd64CallingConvention::IsValid() const
{
    return true;
}

ParameterInfo *Amd64CallingConvention::GetParameterInfo(uint8_t regsOffset)
{
    auto paramInfo = GetAllocator()->New<amd64::Amd64ParameterInfo>();
    // reserve first parameter to method pointer
    for (int i = 0; i < regsOffset; ++i) {
        paramInfo->GetNativeParam(INT64_TYPE);
    }
    return paramInfo;
}

void *Amd64CallingConvention::GetCodeEntry()
{
    auto code = static_cast<Amd64Encoder *>(GetEncoder())->GetMasm()->code();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return reinterpret_cast<void *>(code->baseAddress());
}

uint32_t Amd64CallingConvention::GetCodeSize()
{
    return static_cast<Amd64Encoder *>(GetEncoder())->GetMasm()->code()->codeSize();
}

size_t Amd64CallingConvention::PushRegs(RegList regs, RegList vregs)
{
    size_t regsCount {0};
    size_t vregsCount {0};

    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        uint32_t ii {MAX_NUM_REGS - i - 1};
        if (vregs.Has(ii)) {
            ++vregsCount;
            GetMasm()->sub(asmjit::x86::rsp, asmjit::imm(DOUBLE_WORD_SIZE_BYTES));
            GetMasm()->movsd(asmjit::x86::ptr(asmjit::x86::rsp), asmjit::x86::xmm(ii));
        }
    }

    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        uint32_t ii {MAX_NUM_REGS - i - 1};
        if (regs.Has(ii)) {
            ++regsCount;
            GetMasm()->push(asmjit::x86::gpq(ConvertRegNumber(ii)));
        }
    }

    return vregsCount + regsCount;
}

size_t Amd64CallingConvention::PopRegs(RegList regs, RegList vregs)
{
    size_t regsCount {0};
    size_t vregsCount {0};

    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        if (regs.Has(i)) {
            ++regsCount;
            GetMasm()->pop(asmjit::x86::gpq(ConvertRegNumber(i)));
        }
    }

    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        if (vregs.Has(i)) {
            ++vregsCount;
            GetMasm()->movsd(asmjit::x86::xmm(i), asmjit::x86::ptr(asmjit::x86::rsp));
            GetMasm()->add(asmjit::x86::rsp, asmjit::imm(DOUBLE_WORD_SIZE_BYTES));
        }
    }

    return vregsCount + regsCount;
}

std::variant<Reg, uint8_t> Amd64ParameterInfo::GetNativeParam(const TypeInfo &type)
{
    if (type.IsFloat()) {
        if (currentVectorNumber_ > MAX_VECTOR_PARAM_ID) {
            return currentStackOffset_++;
        }
        return Reg(currentVectorNumber_++, type);
    }
    if (currentScalarNumber_ > MAX_SCALAR_PARAM_ID) {
        return currentStackOffset_++;
    }

    return Target(Arch::X86_64).GetParamReg(currentScalarNumber_++, type);
}

Location Amd64ParameterInfo::GetNextLocation(DataType::Type type)
{
    if (DataType::IsFloatType(type)) {
        if (currentVectorNumber_ > MAX_VECTOR_PARAM_ID) {
            return Location::MakeStackArgument(currentStackOffset_++);
        }
        return Location::MakeFpRegister(currentVectorNumber_++);
    }
    if (currentScalarNumber_ > MAX_SCALAR_PARAM_ID) {
        return Location::MakeStackArgument(currentStackOffset_++);
    }
    Target target(Arch::X86_64);
    return Location::MakeRegister(target.GetParamRegId(currentScalarNumber_++));
}

void Amd64CallingConvention::GeneratePrologue([[maybe_unused]] const FrameInfo &frameInfo)
{
    auto encoder = GetEncoder();
    const CFrameLayout &fl = encoder->GetFrameLayout();
    auto fpReg = GetTarget().GetFrameReg();
    auto spReg = GetTarget().GetStackReg();

    // we do not push return address, because in amd64 call instruction already pushed it
    GetMasm()->push(asmjit::x86::rbp);  // frame pointer
    SET_CFI_OFFSET(pushFplr, encoder->GetCursorOffset());

    encoder->EncodeMov(fpReg, spReg);
    SET_CFI_OFFSET(setFp, encoder->GetCursorOffset());

    if (IsDynCallMode() && GetDynInfo().IsCheckRequired()) {
        static_assert(CallConvDynInfo::REG_NUM_ARGS == 1);
        static_assert(CallConvDynInfo::REG_COUNT == CallConvDynInfo::REG_NUM_ARGS + 1);

        constexpr auto NUM_ACTUAL_REG = GetTarget().GetParamReg(CallConvDynInfo::REG_NUM_ARGS);
        constexpr auto NUM_EXPECTED_REG = GetTarget().GetParamReg(CallConvDynInfo::REG_COUNT);
        auto numExpected = GetDynInfo().GetNumExpectedArgs();

        auto expandDone = encoder->CreateLabel();
        encoder->EncodeJump(expandDone, NUM_ACTUAL_REG, Imm(numExpected), Condition::GE);
        encoder->EncodeMov(NUM_EXPECTED_REG, Imm(numExpected));

        ScopedTmpRegU64 tmpReg(encoder);
        MemRef epTable = MemRef(Reg(GetThreadReg(Arch::X86_64), GetTarget().GetPtrRegType()),
                                GetDynInfo().GetExpandEntrypointsTableTlsOffset());
        LoadEntrypoint(tmpReg, encoder, epTable, GetDynInfo().GetExpandEntrypointOffset());
        encoder->MakeCall(tmpReg);
        encoder->BindLabel(expandDone);
    }

    encoder->EncodeSub(spReg, spReg, Imm(2U * DOUBLE_WORD_SIZE_BYTES));
    encoder->EncodeStr(GetTarget().GetParamReg(0), MemRef(spReg, DOUBLE_WORD_SIZE_BYTES));

    // Reset OSR flag and set HasFloatRegsFlag
    auto flags {static_cast<uint64_t>(frameInfo.GetHasFloatRegs()) << CFrameLayout::HasFloatRegsFlag::START_BIT};
    encoder->EncodeSti(flags, sizeof(flags), MemRef(spReg));
    // Allocate space for locals
    encoder->EncodeSub(spReg, spReg, Imm(DOUBLE_WORD_SIZE_BYTES * (CFrameSlots::Start() - CFrameData::Start())));
    static_assert((CFrameLayout::GetLocalsCount() & 1U) == 0);

    RegList calleeRegs {GetCalleeRegsMask(Arch::X86_64, false).GetValue()};
    RegList calleeVregs {GetCalleeRegsMask(Arch::X86_64, true).GetValue()};
    SET_CFI_CALLEE_REGS(RegMask(static_cast<size_t>(calleeRegs)));
    SET_CFI_CALLEE_VREGS(VRegMask(static_cast<size_t>(calleeVregs)));
    PushRegs(calleeRegs, calleeVregs);
    SET_CFI_OFFSET(pushCallees, encoder->GetCursorOffset());

    encoder->EncodeSub(
        spReg, spReg,
        Imm((fl.GetSpillsCount() + fl.GetCallerRegistersCount(false) + fl.GetCallerRegistersCount(true)) *
            DOUBLE_WORD_SIZE_BYTES));
}

void Amd64CallingConvention::GenerateEpilogueImpl([[maybe_unused]] const FrameInfo &frameInfo,
                                                  const std::function<void()> &postJob)
{
    auto encoder = GetEncoder();
    const CFrameLayout &fl = encoder->GetFrameLayout();
    auto spReg = GetTarget().GetStackReg();

    if (postJob) {
        postJob();
    }

    encoder->EncodeAdd(
        spReg, spReg,
        Imm((fl.GetSpillsCount() + fl.GetCallerRegistersCount(false) + fl.GetCallerRegistersCount(true)) *
            DOUBLE_WORD_SIZE_BYTES));

    PopRegs(RegList(GetCalleeRegsMask(Arch::X86_64, false).GetValue()),
            RegList(GetCalleeRegsMask(Arch::X86_64, true).GetValue()));
    SET_CFI_OFFSET(popCallees, encoder->GetCursorOffset());

    // X86_64 doesn't support OSR mode
    ASSERT(!IsOsrMode());
    // Support restoring of LR and FP registers once OSR is supported in x86_64
    static_assert(!ArchTraits<Arch::X86_64>::SUPPORT_OSR);
    constexpr auto SHIFT = DOUBLE_WORD_SIZE_BYTES * (2 + CFrameSlots::Start() - CFrameData::Start());
    encoder->EncodeAdd(spReg, spReg, Imm(SHIFT));

    GetMasm()->pop(asmjit::x86::rbp);  // frame pointer
    SET_CFI_OFFSET(popFplr, encoder->GetCursorOffset());
}

void Amd64CallingConvention::GenerateNativePrologue(const FrameInfo &frameInfo)
{
    GeneratePrologue(frameInfo);
}

void Amd64CallingConvention::GenerateEpilogue(const FrameInfo &frameInfo, std::function<void()> postJob)
{
    GenerateEpilogueImpl(frameInfo, postJob);
    GetMasm()->ret();
}

void Amd64CallingConvention::GenerateNativeEpilogue(const FrameInfo &frameInfo, std::function<void()> postJob)
{
    GenerateEpilogueImpl(frameInfo, postJob);
    GetMasm()->ret();
}

void Amd64CallingConvention::GenerateEpilogueHead(const FrameInfo &frameInfo, std::function<void()> postJob)
{
    GenerateEpilogueImpl(frameInfo, postJob);
}

asmjit::x86::Assembler *Amd64CallingConvention::GetMasm()
{
    return (static_cast<Amd64Encoder *>(GetEncoder()))->GetMasm();
}

}  // namespace ark::compiler::amd64
