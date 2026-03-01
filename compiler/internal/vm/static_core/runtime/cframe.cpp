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

#include "runtime/include/cframe.h"
#include "compiler/code_info/code_info.h"
#include "runtime/include/runtime.h"
#include "runtime/include/stack_walker.h"
#include "libarkbase/utils/regmask.h"

namespace ark {

bool CFrame::IsNativeMethod() const
{
    return GetMethod()->IsNative();
}

template <bool NEED_PACK>
interpreter::VRegister CFrame::GetVRegValue(const VRegInfo &vreg, const compiler::CodeInfo &codeInfo,
                                            SlotType **calleeStack) const
{
    switch (vreg.GetLocation()) {
        case VRegInfo::Location::SLOT:
            return GetVRegValueSlot<NEED_PACK>(vreg);
        case VRegInfo::Location::REGISTER:
        case VRegInfo::Location::FP_REGISTER:
            return GetVRegValueRegister<NEED_PACK>(vreg, calleeStack);
        case VRegInfo::Location::CONSTANT:
            return GetVRegValueConstant<NEED_PACK>(vreg, codeInfo);
        default:
            return interpreter::VRegister {};
    }
}

template interpreter::VRegister CFrame::GetVRegValue<true>(const VRegInfo &vreg, const compiler::CodeInfo &code_info,
                                                           SlotType **callee_stack) const;
template interpreter::VRegister CFrame::GetVRegValue<false>(const VRegInfo &vreg, const compiler::CodeInfo &code_info,
                                                            SlotType **callee_stack) const;

template void CFrame::SetVRegValue<true>(const VRegInfo &vreg, uint64_t value, SlotType **callee_stack);
template void CFrame::SetVRegValue<false>(const VRegInfo &vreg, uint64_t value, SlotType **callee_stack);

uint64_t CFrame::GetPackValue(VRegInfo::Type type, uint64_t val) const
{
    if (type == VRegInfo::Type::ANY) {
        return val;
    }
    if (type == VRegInfo::Type::FLOAT64) {
        return coretypes::TaggedValue::GetDoubleTaggedValue(val);
    }
    if (type == VRegInfo::Type::INT32) {
        return coretypes::TaggedValue::GetIntTaggedValue(val);
    }
    if (type == VRegInfo::Type::BOOL) {
        return coretypes::TaggedValue::GetBoolTaggedValue(val);
    }
    if (type == VRegInfo::Type::OBJECT) {
        return coretypes::TaggedValue::GetObjectTaggedValue(val);
    }
    UNREACHABLE();
    return val;
}

template <bool NEED_PACK>
interpreter::VRegister CFrame::GetVRegValueSlot(const VRegInfo &vreg) const
{
    interpreter::VRegister resReg;
    uint64_t val = GetValueFromSlot(vreg.GetValue());
    // NOLINTNEXTLINE(bugprone-suspicious-semicolon,readability-braces-around-statements)
    if constexpr (!ArchTraits<ARCH>::IS_64_BITS) {
        if (vreg.Has64BitValue()) {
            ASSERT(!vreg.IsObject());
            val |= static_cast<uint64_t>(GetValueFromSlot(helpers::ToSigned(vreg.GetValue()) - 1)) << BITS_PER_UINT32;
        }
    }
    // NOLINTNEXTLINE(bugprone-suspicious-semicolon,readability-braces-around-statements)
    if constexpr (NEED_PACK) {
        val = GetPackValue(vreg.GetType(), val);
    }
    resReg.Set(val);
    return resReg;
}

template <bool NEED_PACK>
interpreter::VRegister CFrame::GetVRegValueRegister(const VRegInfo &vreg, SlotType **calleeStack) const
{
    interpreter::VRegister resReg;
    bool isFp = vreg.GetLocation() == VRegInfo::Location::FP_REGISTER;
    if ((GetCallerRegsMask(ARCH, isFp) & (1U << vreg.GetValue())).Any()) {
        CFrameLayout fl(ARCH, 0);
        RegMask mask(GetCallerRegsMask(RUNTIME_ARCH, isFp));
        auto regNum = mask.GetDistanceFromTail(vreg.GetValue());
        regNum = fl.GetCallerLastSlot(isFp) - regNum;
        uint64_t val = GetValueFromSlot(regNum);
        // NOLINTNEXTLINE(bugprone-suspicious-semicolon,readability-magic-numbers,readability-braces-around-statements)
        if constexpr (!ArchTraits<ARCH>::IS_64_BITS) {
            if (vreg.Has64BitValue()) {
                ASSERT(!vreg.IsObject());
                val |= static_cast<uint64_t>(GetValueFromSlot(static_cast<int>(regNum) - 1)) << BITS_PER_UINT32;
            }
        }
        // NOLINTNEXTLINE(bugprone-suspicious-semicolon,readability-braces-around-statements)
        if constexpr (NEED_PACK) {
            val = GetPackValue(vreg.GetType(), val);
        }
        resReg.Set(val);
        return resReg;
    }

    uint64_t val = ReadCalleeSavedRegister(vreg.GetValue(), isFp, calleeStack);
    if (!ArchTraits<ARCH>::IS_64_BITS && vreg.Has64BitValue()) {
        val |= static_cast<uint64_t>(ReadCalleeSavedRegister(vreg.GetValue() + 1, isFp, calleeStack))
               << BITS_PER_UINT32;
    }
    // NOLINTNEXTLINE(bugprone-suspicious-semicolon,readability-braces-around-statements)
    if constexpr (NEED_PACK) {
        val = GetPackValue(vreg.GetType(), val);
    }
    if (isFp) {
        resReg.Set(val);
        if (vreg.Has64BitValue()) {
            resReg.Set(bit_cast<double>(val));
            return resReg;
        }
        resReg.Set(bit_cast<float>(static_cast<uint32_t>(val)));
        return resReg;
    }
    resReg.Set(val);
    return resReg;
}

template <bool NEED_PACK>
interpreter::VRegister CFrame::GetVRegValueConstant(const VRegInfo &vreg, const compiler::CodeInfo &codeInfo) const
{
    interpreter::VRegister resReg;
    auto val = codeInfo.GetConstant(vreg);
    // NOLINTNEXTLINE(bugprone-suspicious-semicolon,readability-braces-around-statements)
    if constexpr (NEED_PACK) {
        val = GetPackValue(vreg.GetType(), val);
    }
    resReg.Set(val);
    return resReg;
}

template <bool NEED_PACK>
void CFrame::SetVRegValue(const VRegInfo &vreg, uint64_t value, SlotType **calleeStack)
{
    auto locationValue = static_cast<int>(vreg.GetValue());
    // NOLINTNEXTLINE(bugprone-suspicious-semicolon,readability-braces-around-statements)
    if constexpr (NEED_PACK) {
        value = GetPackValue(vreg.GetType(), value);
    }
    switch (vreg.GetLocation()) {
        case VRegInfo::Location::SLOT: {
            SetValueToSlot(locationValue, value);
            if (!ArchTraits<ARCH>::IS_64_BITS && vreg.Has64BitValue()) {
                SetValueToSlot(locationValue - 1, value >> BITS_PER_UINT32);
            }
            break;
        }
        case VRegInfo::Location::REGISTER:
        case VRegInfo::Location::FP_REGISTER: {
            bool isFp = vreg.GetLocation() == VRegInfo::Location::FP_REGISTER;
            if ((GetCallerRegsMask(ARCH, isFp) & (1U << vreg.GetValue())).Any()) {
                CFrameLayout fl(ARCH, 0);
                auto regNum = locationValue - static_cast<int>(GetFirstCallerReg(ARCH, isFp));
                regNum = static_cast<int>(fl.GetCallerLastSlot(isFp)) - regNum;
                SetValueToSlot(regNum, value);
                if (!ArchTraits<ARCH>::IS_64_BITS && vreg.Has64BitValue()) {
                    SetValueToSlot(regNum - 1, value >> BITS_PER_UINT32);
                }
                break;
            }
            WriteCalleeSavedRegister(locationValue, value, isFp, calleeStack);
            // NOLINTNEXTLINE(bugprone-suspicious-semicolon,readability-braces-around-statements)
            if constexpr (!ArchTraits<ARCH>::IS_64_BITS) {
                if (vreg.Has64BitValue()) {
                    WriteCalleeSavedRegister(locationValue + 1, value >> BITS_PER_UINT32, isFp, calleeStack);
                }
                break;
            }
            break;
        }
        case VRegInfo::Location::CONSTANT:
            ASSERT(false && "Modifying constants is not permitted");  // NOLINT(misc-static-assert)
            break;
        default:
            UNREACHABLE();
    }
}

void CFrame::Dump(const CodeInfo &codeInfo, std::ostream &os)
{
    auto maxSlot = codeInfo.GetHeader().GetFrameSize() / ArchTraits<RUNTIME_ARCH>::POINTER_SIZE;
    Dump(os, maxSlot);
}

void CFrame::Dump(std::ostream &os, uint32_t maxSlot)
{
    if (IsNative()) {
        os << "NATIVE CFRAME: fp=" << fp_ << std::endl;
        return;
    }
    auto spillStartSlot = GetCalleeRegsCount(ARCH, false) + GetCalleeRegsCount(ARCH, true) +
                          GetCallerRegsCount(ARCH, false) + GetCallerRegsCount(ARCH, true);
    maxSlot = (maxSlot > spillStartSlot) ? (maxSlot - spillStartSlot) : 0;

    auto printMem = [](std::ostream &stream, void *addr, std::string_view dscr, uintptr_t value) {
        constexpr size_t WIDTH = 16;
        stream << ' ' << addr << ": " << std::setw(WIDTH) << std::setfill(' ') << dscr << " 0x" << std::hex << value
               << std::dec << std::endl;
    };
    os << "****************************************\n";
    os << "* CFRAME: fp=" << fp_ << ", max_spill_slot=" << maxSlot << '\n';
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    printMem(os, fp_ - CFrameLayout::LrSlot::Start(), "lr", GetLr());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    printMem(os, fp_ - CFrameLayout::PrevFrameSlot::Start(), "prev", reinterpret_cast<uintptr_t>(GetPrevFrame()));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    printMem(os, fp_ - CFrameLayout::MethodSlot::Start(), "method", reinterpret_cast<uintptr_t>(GetMethod()));
    PandaString dscr;
    size_t slot = 0;
    DumpCalleeRegs(os, printMem, &dscr, &slot);
    DumpCalleeFPRegs(os, printMem, &dscr, &slot);
    DumpCallerRegs(os, printMem, &dscr, &slot);
    DumpCallerFPRegs(os, printMem, &dscr, &slot);
    DumpLocals(os, printMem, &dscr, &slot, maxSlot);

    os << "* CFRAME END\n";
    os << "****************************************\n";
}

void CFrame::DumpCalleeRegs(std::ostream &os, MemPrinter printMem, PandaString *dscr, size_t *slot)
{
    os << " [Callee saved registers]\n";
    for (auto i = ark::helpers::ToSigned(GetLastCalleeReg(ARCH, false));
         i >= ark::helpers::ToSigned(GetFirstCalleeReg(ARCH, false)); i--, (*slot)++) {
        *dscr = "x" + ToPandaString(i) + ":" + ToPandaString(*slot);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        printMem(os, fp_ - CFrameLayout::STACK_START_SLOT - *slot, *dscr, GetValueFromSlot(*slot));
    }
}

void CFrame::DumpCalleeFPRegs(std::ostream &os, MemPrinter printMem, PandaString *dscr, size_t *slot)
{
    os << " [Callee saved FP registers]\n";
    for (auto i = ark::helpers::ToSigned(GetLastCalleeReg(ARCH, true));
         i >= ark::helpers::ToSigned(GetFirstCalleeReg(ARCH, true)); i--, (*slot)++) {
        *dscr = "d" + ToPandaString(i) + ":" + ToPandaString(*slot);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        printMem(os, fp_ - CFrameLayout::STACK_START_SLOT - *slot, *dscr, GetValueFromSlot(*slot));
    }
}

void CFrame::DumpCallerRegs(std::ostream &os, MemPrinter printMem, PandaString *dscr, size_t *slot)
{
    os << " [Caller saved registers] " << GetLastCallerReg(ARCH, false) << " " << GetFirstCallerReg(ARCH, false)
       << "\n";
    for (auto i = ark::helpers::ToSigned(GetLastCallerReg(ARCH, false));
         i >= ark::helpers::ToSigned(GetFirstCallerReg(ARCH, false)); i--, (*slot)++) {
        *dscr = "x" + ToPandaString(i) + ":" + ToPandaString(*slot);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        printMem(os, fp_ - CFrameLayout::STACK_START_SLOT - *slot, *dscr, GetValueFromSlot(*slot));
    }
}

void CFrame::DumpCallerFPRegs(std::ostream &os, MemPrinter printMem, PandaString *dscr, size_t *slot)
{
    os << " [Caller saved FP registers]\n";
    for (auto i = ark::helpers::ToSigned(GetLastCallerReg(ARCH, true));
         i >= ark::helpers::ToSigned(GetFirstCallerReg(ARCH, true)); i--, (*slot)++) {
        *dscr = "d" + ToPandaString(i) + ":" + ToPandaString(*slot);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        printMem(os, fp_ - CFrameLayout::STACK_START_SLOT - *slot, *dscr, GetValueFromSlot(*slot));
    }
}

void CFrame::DumpLocals(std::ostream &os, MemPrinter printMem, PandaString *dscr, size_t *slot, int32_t maxSlot)
{
    os << " [Locals]\n";
    for (auto i = 0; i <= maxSlot; i++, (*slot)++) {
        *dscr = "s" + ToPandaString(i) + ":" + ToPandaString(*slot);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        printMem(os, fp_ - CFrameLayout::STACK_START_SLOT - *slot, *dscr, GetValueFromSlot(*slot));
    }
}

}  // namespace ark
