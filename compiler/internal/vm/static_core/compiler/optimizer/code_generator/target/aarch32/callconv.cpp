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
#include "target/aarch32/target.h"
#include <cmath>

namespace ark::compiler::aarch32 {
using vixl::aarch32::RegisterList;
using vixl::aarch32::SRegister;
using vixl::aarch32::SRegisterList;

constexpr size_t MAX_SCALAR_PARAM_ID = 3;                           // r0-r3
[[maybe_unused]] constexpr size_t MAX_VECTOR_SINGLE_PARAM_ID = 15;  // s0-s15
[[maybe_unused]] constexpr size_t MAX_VECTOR_DOUBLE_PARAM_ID = 7;   // d0-d7

Aarch32CallingConvention::Aarch32CallingConvention(ArenaAllocator *allocator, Encoder *enc, RegistersDescription *descr,
                                                   CallConvMode mode)
    : CallingConvention(allocator, enc, descr, mode)
{
}

ParameterInfo *Aarch32CallingConvention::GetParameterInfo(uint8_t regsOffset)
{
    auto paramInfo = GetAllocator()->New<aarch32::Aarch32ParameterInfo>();
    for (int i = 0; i < regsOffset; ++i) {
        paramInfo->GetNativeParam(INT32_TYPE);
    }
    return paramInfo;
}

void *Aarch32CallingConvention::GetCodeEntry()
{
    auto res = GetMasm()->GetBuffer()->GetOffsetAddress<uint32_t *>(0);
    return reinterpret_cast<void *>(res);
}

uint32_t Aarch32CallingConvention::GetCodeSize()
{
    return GetMasm()->GetSizeOfCodeGenerated();
}

uint8_t Aarch32CallingConvention::PushPopVRegs(VRegMask vregs, bool isPush = true)
{
    int8_t first = -1;
    uint8_t size = 0;
    bool isSequential = true;
    for (size_t i = 0; i < vregs.size(); ++i) {
        if (-1 == first && vregs.test(i)) {
            first = i;
            ++size;
            continue;
        }
        if (vregs.test(i)) {
            if (!vregs.test(i - 1)) {
                isSequential = false;
                break;
            }
            ++size;
        }
    }
    if (first == -1) {
        ASSERT(size == 0);
        return 0;
    }

    if (isSequential) {
        auto regList = vixl::aarch32::SRegisterList(vixl::aarch32::SRegister(first), size);
        if (isPush) {
            GetMasm()->Vpush(regList);
        } else {
            GetMasm()->Vpop(regList);
        }
        return size;
    }

    uint32_t realOffset = 0;
    if (isPush) {
        for (int32_t i = vregs.size() - 1; i >= 0; --i) {
            if (vregs.test(i)) {
                GetMasm()->PushRegister(VixlVReg(Reg(i, FLOAT32_TYPE)).S());
                ++realOffset;
            }
        }
    } else {
        constexpr auto VREG_SIZE = 1;
        for (size_t i = 0; i < vregs.size(); ++i) {
            if (vregs.test(i)) {
                GetMasm()->Vpop(vixl::aarch32::SRegisterList(VixlVReg(Reg(i, FLOAT32_TYPE)).S(), VREG_SIZE));
                ++realOffset;
            }
        }
    }
    return realOffset;
}

vixl::aarch32::MacroAssembler *Aarch32CallingConvention::GetMasm()
{
    return (static_cast<Aarch32Encoder *>(GetEncoder()))->GetMasm();
}
constexpr auto Aarch32CallingConvention::GetTarget()
{
    return ark::compiler::Target(Arch::AARCH32);
}

uint8_t Aarch32CallingConvention::PushRegs(RegMask regs, VRegMask vregs, bool isCallee)
{
    auto regdescr = static_cast<Aarch32RegisterDescription *>(GetRegfile());
    auto lr = GetTarget().GetLinkReg().GetId();
    if (regs.test(lr)) {
        regs.reset(lr);
    }
    auto fp = GetTarget().GetFrameReg().GetId();
    if (regs.test(fp)) {
        regs.reset(fp);
    }

    uint8_t realOffset = 0;
    uint32_t savedRegistersMask = 0;

    for (size_t i = 0; i < regs.size(); ++i) {
        if (regs.test(i)) {
            savedRegistersMask |= 1UL << i;
            ++realOffset;
        }
    }

    if (((regs.count() + vregs.count()) & 1U) == 1) {
        // NOTE(igorban) move them to Sub(sp)
        uint8_t alignReg = regdescr->GetAligmentReg(isCallee);
        GetMasm()->PushRegister(vixl::aarch32::Register(alignReg));
        ++realOffset;
    }

    if (savedRegistersMask != 0) {
        GetMasm()->Push(vixl::aarch32::RegisterList(savedRegistersMask));
    }
    realOffset += PushPopVRegs(vregs, true);
    ASSERT((realOffset & 1U) == 0);

    return realOffset;
}

uint8_t Aarch32CallingConvention::PopRegs(RegMask regs, VRegMask vregs, bool isCallee)
{
    auto regdescr = static_cast<Aarch32RegisterDescription *>(GetRegfile());

    auto fp = GetTarget().GetFrameReg().GetId();
    if (regs.test(fp)) {
        regs.reset(fp);
    }
    auto lr = GetTarget().GetLinkReg().GetId();
    if (regs.test(lr)) {
        regs.reset(lr);
    }

    uint8_t realOffset = 0;
    realOffset += PushPopVRegs(vregs, false);

    uint32_t savedRegistersMask = 0;

    for (size_t i = 0; i < regs.size(); ++i) {
        if (regs.test(i)) {
            savedRegistersMask |= 1UL << i;
            ++realOffset;
        }
    }

    if (savedRegistersMask != 0) {
        GetMasm()->Pop(vixl::aarch32::RegisterList(savedRegistersMask));
    }

    if (((regs.count() + vregs.count()) & 1U) == 1) {
        uint8_t alignReg = regdescr->GetAligmentReg(isCallee);
        GetMasm()->Pop(vixl::aarch32::Register(alignReg));
        ++realOffset;
    }
    ASSERT((realOffset & 1U) == 0);

    return realOffset;
}

std::variant<Reg, uint8_t> Aarch32ParameterInfo::GetNativeParam(const TypeInfo &type)
{
    constexpr int32_t STEP = 2;
#if (PANDA_TARGET_ARM32_ABI_HARD)
    // Use vector registers
    if (type == FLOAT32_TYPE) {
        if (currentVectorNumber_ > MAX_VECTOR_SINGLE_PARAM_ID) {
            return currentStackOffset_++;
        }
        return Reg(currentVectorNumber_++, FLOAT32_TYPE);
    }
    if (type == FLOAT64_TYPE) {
        // Allignment for 8 bytes (in stack and registers)
        if ((currentVectorNumber_ & 1U) == 1) {
            ++currentVectorNumber_;
        }
        if ((currentVectorNumber_ >> 1U) > MAX_VECTOR_DOUBLE_PARAM_ID) {
            if ((currentStackOffset_ & 1U) == 1) {
                ++currentStackOffset_;
            }
            auto stackOffset = currentStackOffset_;
            currentStackOffset_ += STEP;
            return stackOffset;
        }
        auto vectorNumber = currentVectorNumber_;
        currentVectorNumber_ += STEP;
        return Reg(vectorNumber, FLOAT64_TYPE);
    }
#endif  // PANDA_TARGET_ARM32_ABI_HARD
    if (type.GetSize() == DOUBLE_WORD_SIZE) {
        if ((currentScalarNumber_ & 1U) == 1) {
            ++currentScalarNumber_;
        }
        // Allignment for 8 bytes (in stack and registers)
        if (currentScalarNumber_ > MAX_SCALAR_PARAM_ID) {
            if ((currentStackOffset_ & 1U) == 1) {
                ++currentStackOffset_;
            }
            auto stackOffset = currentStackOffset_;
            currentStackOffset_ += STEP;
            return stackOffset;
        }
        auto scalarNumber = currentScalarNumber_;
        currentScalarNumber_ += STEP;
        return Reg(scalarNumber, INT64_TYPE);
    }
    if (currentScalarNumber_ > MAX_SCALAR_PARAM_ID) {
        return currentStackOffset_++;
    }
    ASSERT(!type.IsFloat() || type == FLOAT32_TYPE);
    return Reg(currentScalarNumber_++, type.IsFloat() ? INT32_TYPE : type);
}

Location Aarch32ParameterInfo::GetNextLocation(DataType::Type type)
{
    auto res = GetNativeParam(TypeInfo::FromDataType(type, Arch::AARCH32));
    if (std::holds_alternative<Reg>(res)) {
        auto reg = std::get<Reg>(res);
#if (PANDA_TARGET_ARM32_ABI_SOFT || PANDA_TARGET_ARM32_ABI_SOFTFP)
        if (DataType::IsFloatType(type)) {
            return Location::MakeRegister(reg.GetId());
        }
#endif
        return Location::MakeRegister(reg.GetId(), type);
    }
    return Location::MakeStackArgument(std::get<uint8_t>(res));
}

bool Aarch32CallingConvention::IsValid() const
{
    return true;
}

void Aarch32CallingConvention::GenerateNativePrologue(const FrameInfo &frameInfo)
{
    GeneratePrologue(frameInfo);
}

void Aarch32CallingConvention::GenerateNativeEpilogue(const FrameInfo &frameInfo, std::function<void()> postJob)
{
    GenerateEpilogue(frameInfo, postJob);
}

void Aarch32CallingConvention::GenerateEpilogueHead([[maybe_unused]] const FrameInfo &frameInfo,
                                                    [[maybe_unused]] std::function<void()> postJob)
{
    UNREACHABLE();
}

void Aarch32CallingConvention::GeneratePrologue([[maybe_unused]] const FrameInfo &frameInfo)
{
    auto encoder = GetEncoder();
    ASSERT(encoder->IsValid());
    ASSERT(encoder->InitMasm());
    const CFrameLayout &fl = encoder->GetFrameLayout();
    auto fpReg = GetTarget().GetFrameReg();
    auto spReg = GetTarget().GetStackReg();

    GetMasm()->Push(RegisterList(vixl::aarch32::r11, vixl::aarch32::lr));
    SET_CFI_OFFSET(pushFplr, encoder->GetCursorOffset());

    ASSERT(!IsDynCallMode());

    encoder->EncodeMov(fpReg, spReg);
    SET_CFI_OFFSET(setFp, encoder->GetCursorOffset());
    constexpr auto IMM_2 = 2;
    encoder->EncodeSub(spReg, spReg, Imm(WORD_SIZE_BYTES * IMM_2));
    encoder->EncodeStr(GetTarget().GetParamReg(0), MemRef(spReg, WORD_SIZE_BYTES));

    // Allocate space for locals
    auto localsSize = (CFrameSlots::Start() - CFrameData::Start()) * WORD_SIZE_BYTES;
    encoder->EncodeSub(spReg, spReg, Imm(localsSize));

    SET_CFI_CALLEE_REGS(GetCalleeRegsMask(Arch::AARCH32, false));
    SET_CFI_CALLEE_VREGS(GetCalleeRegsMask(Arch::AARCH32, true));
    GetMasm()->Push(RegisterList(GetCalleeRegsMask(Arch::AARCH32, false).GetValue()));
    GetMasm()->Vpush(
        SRegisterList(SRegister(GetFirstCalleeReg(Arch::AARCH32, true)), GetCalleeRegsCount(Arch::AARCH32, true)));
    SET_CFI_OFFSET(pushCallees, encoder->GetCursorOffset());

    // Reset OSR flag and set HasFloatRegsFlag
    auto calleeRegsSize =
        (GetCalleeRegsCount(Arch::AARCH32, true) + GetCalleeRegsCount(Arch::AARCH32, false)) * WORD_SIZE_BYTES;
    auto flags {static_cast<uint32_t>(frameInfo.GetHasFloatRegs()) << CFrameLayout::HasFloatRegsFlag::START_BIT};
    encoder->EncodeSti(flags, sizeof(flags), MemRef(spReg, calleeRegsSize + localsSize));

    encoder->EncodeSub(
        spReg, spReg,
        Imm((fl.GetSpillsCount() + fl.GetCallerRegistersCount(false) + fl.GetCallerRegistersCount(true)) *
            WORD_SIZE_BYTES));
}

void Aarch32CallingConvention::GenerateEpilogue([[maybe_unused]] const FrameInfo &frameInfo,
                                                std::function<void()> /* post_job */)
{
    auto encoder = GetEncoder();
    const CFrameLayout &fl = encoder->GetFrameLayout();
    auto spReg = GetTarget().GetStackReg();

    encoder->EncodeAdd(
        spReg, spReg,
        Imm((fl.GetSpillsCount() + fl.GetCallerRegistersCount(false) + fl.GetCallerRegistersCount(true)) *
            WORD_SIZE_BYTES));

    GetMasm()->Vpop(
        SRegisterList(SRegister(GetFirstCalleeReg(Arch::AARCH32, true)), GetCalleeRegsCount(Arch::AARCH32, true)));
    GetMasm()->Pop(RegisterList(GetCalleeRegsMask(Arch::AARCH32, false).GetValue()));
    SET_CFI_OFFSET(popCallees, encoder->GetCursorOffset());

    // ARM32 doesn't support OSR mode
    ASSERT(!IsOsrMode());
    // Support restoring of LR and FP registers once OSR is supported in arm32
    static_assert(!ArchTraits<Arch::AARCH32>::SUPPORT_OSR);
    constexpr auto IMM_2 = 2;
    encoder->EncodeAdd(spReg, spReg, Imm(WORD_SIZE_BYTES * IMM_2));
    encoder->EncodeAdd(spReg, spReg, Imm(WORD_SIZE_BYTES * (CFrameSlots::Start() - CFrameData::Start())));

    GetMasm()->Pop(RegisterList(vixl::aarch32::r11, vixl::aarch32::lr));
    SET_CFI_OFFSET(popFplr, encoder->GetCursorOffset());

    encoder->EncodeReturn();
}
}  // namespace ark::compiler::aarch32
