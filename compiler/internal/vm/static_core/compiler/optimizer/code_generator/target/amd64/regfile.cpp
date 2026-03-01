/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
Register file implementation.
Reserve registers.
*/

#include "registers_description.h"
#include "target/amd64/target.h"
#include "regfile.h"

namespace ark::compiler::amd64 {
Amd64RegisterDescription::Amd64RegisterDescription(ArenaAllocator *allocator)
    : RegistersDescription(allocator, Arch::X86_64), usedRegs_(allocator->Adapter())
{
}

bool Amd64RegisterDescription::IsRegUsed(ArenaVector<Reg> vecReg, Reg reg)
{
    auto equality = [reg](Reg in) { return (reg.GetId() == in.GetId()) && (reg.GetType() == in.GetType()); };
    return (std::find_if(vecReg.begin(), vecReg.end(), equality) != vecReg.end());
}

ArenaVector<Reg> Amd64RegisterDescription::GetCalleeSaved()
{
    ArenaVector<Reg> out(GetAllocator()->Adapter());
    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        if (calleeSaved_.Has(i)) {
            out.emplace_back(Reg(i, INT64_TYPE));
        }
        if (calleeSavedv_.Has(i)) {
            out.emplace_back(Reg(i, FLOAT64_TYPE));
        }
    }
    return out;
}

void Amd64RegisterDescription::SetCalleeSaved(const ArenaVector<Reg> &regs)
{
    calleeSaved_ = RegList(GetCalleeRegsMask(Arch::X86_64, false).GetValue());
    calleeSavedv_ = RegList(GetCalleeRegsMask(Arch::X86_64, true).GetValue());  // empty

    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        bool scalarUsed = IsRegUsed(regs, Reg(i, INT64_TYPE));
        if (scalarUsed) {
            calleeSaved_.Add(i);
        } else {
            calleeSaved_.Remove(i);
        }
        bool vectorUsed = IsRegUsed(regs, Reg(i, FLOAT64_TYPE));
        if (vectorUsed) {
            calleeSavedv_.Add(i);
        } else {
            calleeSavedv_.Remove(i);
        }
    }
    // Remove return-value from callee
    calleeSaved_.Remove(ConvertRegNumber(asmjit::x86::rax.id()));
}

void Amd64RegisterDescription::SetUsedRegs(const ArenaVector<Reg> &regs)
{
    usedRegs_ = regs;

    // Update current lists - to do not use old data
    calleeSaved_ = RegList(GetCalleeRegsMask(Arch::X86_64, false).GetValue());
    callerSaved_ = RegList(GetCallerRegsMask(Arch::X86_64, false).GetValue());

    calleeSavedv_ = RegList(GetCalleeRegsMask(Arch::X86_64, true).GetValue());  // empty
    callerSavedv_ = RegList(GetCallerRegsMask(Arch::X86_64, true).GetValue());

    for (uint32_t i = 0; i < MAX_NUM_REGS; ++i) {
        // IsRegUsed use used_regs_ variable
        bool scalarUsed = IsRegUsed(usedRegs_, Reg(i, INT64_TYPE));
        if (!scalarUsed && calleeSaved_.Has(i)) {
            calleeSaved_.Remove(i);
        }
        if (!scalarUsed && callerSaved_.Has(i)) {
            callerSaved_.Remove(i);
        }

        bool vectorUsed = IsRegUsed(usedRegs_, Reg(i, FLOAT64_TYPE));
        if (!vectorUsed && calleeSavedv_.Has(i)) {
            calleeSavedv_.Remove(i);
        }
        if (!vectorUsed && callerSavedv_.Has(i)) {
            callerSavedv_.Remove(i);
        }
    }
}

RegMask Amd64RegisterDescription::GetCallerSavedRegMask() const
{
    return RegMask(callerSaved_.GetMask());
}

VRegMask Amd64RegisterDescription::GetCallerSavedVRegMask() const
{
    return VRegMask(callerSavedv_.GetMask());
}

bool Amd64RegisterDescription::IsCalleeRegister(Reg reg)
{
    bool isFp = reg.IsFloat();
    return reg.GetId() >= GetFirstCalleeReg(Arch::X86_64, isFp) && reg.GetId() <= GetLastCalleeReg(Arch::X86_64, isFp);
}

Reg Amd64RegisterDescription::GetZeroReg() const
{
    return INVALID_REGISTER;  // there is no one
}

bool Amd64RegisterDescription::IsZeroReg([[maybe_unused]] Reg reg) const
{
    return false;
}

Reg::RegIDType Amd64RegisterDescription::GetTempReg()
{
    return compiler::arch_info::x86_64::TEMP_REGS.GetMaxRegister();
}

Reg::RegIDType Amd64RegisterDescription::GetTempVReg()
{
    return compiler::arch_info::x86_64::TEMP_FP_REGS.GetMaxRegister();
}

RegMask Amd64RegisterDescription::GetDefaultRegMask() const
{
    static constexpr size_t HIGH_MASK {0xFFFF0000};

    RegMask regMask(HIGH_MASK);
    regMask |= compiler::arch_info::x86_64::TEMP_REGS;
    regMask.set(ConvertRegNumber(asmjit::x86::rbp.id()));
    regMask.set(ConvertRegNumber(asmjit::x86::rsp.id()));
    regMask.set(GetThreadReg(Arch::X86_64));
    return regMask;
}

VRegMask Amd64RegisterDescription::GetVRegMask()
{
    static constexpr size_t HIGH_MASK {0xFFFF0000};

    VRegMask vregMask(HIGH_MASK);
    vregMask |= compiler::arch_info::x86_64::TEMP_FP_REGS;
    return vregMask;
}

// Check register mapping
bool Amd64RegisterDescription::SupportMapping(uint32_t type)
{
    // Current implementation does not support reg-reg mapping
    if ((type & (RegMapping::VECTOR_VECTOR | RegMapping::FLOAT_FLOAT)) != 0U) {
        return false;
    }
    // Scalar and float registers lay in different registers
    if ((type & (RegMapping::SCALAR_VECTOR | RegMapping::SCALAR_FLOAT)) != 0U) {
        return false;
    }
    return true;
}

bool Amd64RegisterDescription::IsValid() const
{
    return true;
}

size_t Amd64RegisterDescription::GetCalleeSavedR()
{
    return static_cast<size_t>(calleeSaved_);
}

size_t Amd64RegisterDescription::GetCalleeSavedV()
{
    return static_cast<size_t>(calleeSavedv_);
}

size_t Amd64RegisterDescription::GetCallerSavedR()
{
    return static_cast<size_t>(callerSaved_);
}

size_t Amd64RegisterDescription::GetCallerSavedV()
{
    return static_cast<size_t>(callerSavedv_);
}

Reg Amd64RegisterDescription::AcquireScratchRegister(TypeInfo type)
{
    if (type.IsFloat()) {
        return Reg(scratchv_.Pop(), type);
    }
    return Reg(scratch_.Pop(), type);
}

void Amd64RegisterDescription::AcquireScratchRegister(Reg reg)
{
    if (reg.GetType().IsFloat()) {
        ASSERT(scratchv_.Has(reg.GetId()));
        scratchv_.Remove(reg.GetId());
    } else {
        ASSERT(scratch_.Has(reg.GetId()));
        scratch_.Remove(reg.GetId());
    }
}

void Amd64RegisterDescription::ReleaseScratchRegister(Reg reg)
{
    if (reg.IsFloat()) {
        scratchv_.Add(reg.GetId());
    } else {
        scratch_.Add(reg.GetId());
    }
}

bool Amd64RegisterDescription::IsScratchRegisterReleased(Reg reg) const
{
    if (reg.GetType().IsFloat()) {
        return scratchv_.Has(reg.GetId());
    }
    return scratch_.Has(reg.GetId());
}

RegList Amd64RegisterDescription::GetScratchRegisters() const
{
    return scratch_;
}

RegList Amd64RegisterDescription::GetScratchFPRegisters() const
{
    return scratchv_;
}

size_t Amd64RegisterDescription::GetScratchRegistersCount() const
{
    return scratch_.GetCount();
}

size_t Amd64RegisterDescription::GetScratchFPRegistersCount() const
{
    return scratchv_.GetCount();
}

RegMask Amd64RegisterDescription::GetScratchRegistersMask() const
{
    return RegMask(scratch_.GetMask());
}

RegMask Amd64RegisterDescription::GetScratchFpRegistersMask() const
{
    return RegMask(scratchv_.GetMask());
}

}  // namespace ark::compiler::amd64
