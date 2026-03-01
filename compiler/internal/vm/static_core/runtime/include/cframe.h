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

#ifndef PANDA_CFRAME_H
#define PANDA_CFRAME_H
#include <array>

#include "compiler/code_info/code_info.h"
#include "libarkbase/utils/cframe_layout.h"
#include "runtime/interpreter/frame.h"
#include "libarkbase/macros.h"
namespace ark {

namespace compiler {
class CodeInfo;
}  // namespace compiler

class Method;

struct alignas(2U * alignof(uintptr_t)) C2IBridge {
    std::array<uintptr_t, 4> v;  // 4: means array length
};

/**
 * CFrame layout (in descending order):
 *
 * -----------------
 *    LR
 *    PREV_FRAME   <-- `fp_` points here
 *    METHOD
 *    PROPERTIES: [0]: Should deoptimize (1 - deoptimize)
 *                [1..2]: Frame type - NATIVE, OSR or DEFAULT
 * -----------------
 *   LOCALS     several slots used for internal needs
 * -----------------
 *    R_N       = CALLEE SAVED REGS <--- `callee_stack_` of the caller's frame points here
 *    ...
 *    R_0
 * -----------------
 *    VR_N      = CALLEE SAVED FP REGS
 *    ...
 *    VR_0
 * -----------------
 *    R_N       = CALLER SAVED REGS
 *    ...
 *    R_0
 * -----------------
 *    VR_N      = CALLER SAVED FP REGS
 *    ...
 *    VR_0
 * -----------------
 *    SLOT_N    = REGALLOC SPILL/FILLS
 *    ...
 *    SLOT_0
 * -----------------
 *    SLOT_M    = LANGUAGE EXTENSION SPILL SLOTS
 *    ...
 *    SLOT_0
 * -----------------
 *    SLOT_P    = PARAMETERS SLOTS
 *    ...
 *    SLOT_0
 * -----------------
 */
class CFrame final {
public:
    static constexpr Arch ARCH = RUNTIME_ARCH;

    using SlotType = std::conditional_t<ArchTraits<ARCH>::IS_64_BITS, uint64_t, uint32_t>;

    using CodeInfo = compiler::CodeInfo;
    using VRegInfo = compiler::VRegInfo;
    using StackMap = compiler::StackMap;

public:
    explicit CFrame(void *frameData) : fp_(reinterpret_cast<SlotType *>(frameData)) {}
    ~CFrame() = default;

    DEFAULT_COPY_SEMANTIC(CFrame);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(CFrame);

    bool IsOsr() const
    {
        return CFrameLayout::FrameKindField::Get(*GetPtr<SlotType>(CFrameLayout::FlagsSlot::Start())) ==
               CFrameLayout::FrameKind::OSR;
    }

    bool IsNative() const
    {
        return CFrameLayout::FrameKindField::Get(*GetPtr<SlotType>(CFrameLayout::FlagsSlot::Start())) ==
               CFrameLayout::FrameKind::NATIVE;
    }

    void SetFrameKind(CFrameLayout::FrameKind kind)
    {
        CFrameLayout::FrameKindField::Set(kind, GetPtr<SlotType>(CFrameLayout::FlagsSlot::Start()));
    }

    PANDA_PUBLIC_API bool IsNativeMethod() const;

    bool ShouldDeoptimize() const
    {
        return CFrameLayout::ShouldDeoptimizeFlag::Get(*GetPtr<SlotType>(CFrameLayout::FlagsSlot::Start()));
    }

    void SetShouldDeoptimize(bool v)
    {
        CFrameLayout::ShouldDeoptimizeFlag::Set(v, GetPtr<SlotType>(CFrameLayout::FlagsSlot::Start()));
    }

    void SetHasFloatRegs(bool has)
    {
        CFrameLayout::HasFloatRegsFlag::Set(has, GetPtr<SlotType>(CFrameLayout::FlagsSlot::Start()));
    }

    SlotType *GetPrevFrame()
    {
        return *GetPtr<SlotType *>(CFrameLayout::PrevFrameSlot::Start());
    }

    const SlotType *GetPrevFrame() const
    {
        return *GetPtr<SlotType *>(CFrameLayout::PrevFrameSlot::Start());
    }

    void SetPrevFrame(void *prevFrame)
    {
        *GetPtr<SlotType>(CFrameLayout::PrevFrameSlot::Start()) = bit_cast<SlotType>(prevFrame);
    }

    Method *GetMethod()
    {
        return *GetPtr<Method *>(CFrameLayout::MethodSlot::Start());
    }

    const Method *GetMethod() const
    {
        return const_cast<CFrame *>(this)->GetMethod();
    }

    void SetMethod(Method *method)
    {
        *GetPtr<SlotType>(CFrameLayout::MethodSlot::Start()) = bit_cast<SlotType>(method);
    }

    void *GetDeoptCodeEntry() const
    {
        return *GetPtr<void *>(CFrameData::Start());
    }

    /**
     * When method is deoptimizated due to it speculation fatal failure, its code entry is reset.
     * Therefore already executing methods will can't get proper code entry for stack walker,
     * thus we create this backup code entry.
     */
    void SetDeoptCodeEntry(const void *value)
    {
        *GetPtr<const void *>(CFrameData::Start()) = value;
    }

    template <typename RegRef>
    inline void GetVRegValue(const VRegInfo &vreg, const compiler::CodeInfo &codeInfo, SlotType **calleeStack,
                             RegRef &&reg)
    {
        auto val = GetVRegValue<false>(vreg, codeInfo, calleeStack).GetValue();
        if (vreg.IsObject()) {
            reg.SetReference(reinterpret_cast<ObjectHeader *>(static_cast<ObjectPointerType>(val)));
        } else {
            reg.SetPrimitive(val);
        }
    }

    template <typename RegRef>
    inline void GetPackVRegValue(const VRegInfo &vreg, const compiler::CodeInfo &codeInfo, SlotType **calleeStack,
                                 RegRef &&reg)
    {
        auto val = GetVRegValue<true>(vreg, codeInfo, calleeStack).GetValue();
        reg.SetPrimitive(val);
    }

    template <bool NEED_PACK = false>
    void SetVRegValue(const VRegInfo &vreg, uint64_t value, SlotType **calleeStack);

    uintptr_t GetLr() const
    {
        return *GetPtr<uintptr_t>(CFrameLayout::LrSlot::Start());
    }

    SlotType *GetStackOrigin()
    {
        return GetPtr<SlotType>(CFrameLayout::STACK_START_SLOT);
    }
    const SlotType *GetStackOrigin() const
    {
        return GetPtr<const SlotType>(CFrameLayout::STACK_START_SLOT);
    }

    SlotType *GetCalleeSaveStack()
    {
        return GetPtr<SlotType>(CFrameLayout::CALLEE_REGS_START_SLOT - 1);
    }

    SlotType *GetCallerSaveStack()
    {
        return GetPtr<SlotType>(CFrameLayout::CALLER_REGS_START_SLOT - 1);
    }

    SlotType *GetFrameOrigin()
    {
        return fp_;
    }

    const SlotType *GetFrameOrigin() const
    {
        return fp_;
    }

    SlotType *GetStackArgsStart()
    {
        return GetPtr<SlotType>(CFrameLayout::STACK_ARGS_START);
    }

    const SlotType *GetStackArgsStart() const
    {
        return GetPtr<const SlotType>(CFrameLayout::STACK_ARGS_START);
    }

    SlotType *GetFrameStart()
    {
        return GetPtr<SlotType>(CFrameLayout::FRAME_START_SLOT);
    }

    const SlotType *GetFrameStart() const
    {
        return GetPtr<const SlotType>(CFrameLayout::FRAME_START_SLOT);
    }

    SlotType GetValueFromSlot(int slot) const
    {
        return *GetValuePtrFromSlot(slot);
    }

    const SlotType *GetValuePtrFromSlot(int slot) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<const SlotType *>(GetStackOrigin()) - slot;
    }

    void SetValueToSlot(int slot, SlotType value)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        *(reinterpret_cast<SlotType *>(GetStackOrigin()) - slot) = value;
    }

    void Dump(const CodeInfo &codeInfo, std::ostream &os);

    void Dump(std::ostream &os, uint32_t maxSlot = 0);

    template <bool NEED_PACK>
    PANDA_PUBLIC_API interpreter::VRegister GetVRegValue(const VRegInfo &vreg, const compiler::CodeInfo &codeInfo,
                                                         SlotType **calleeStack) const;

private:
    SlotType ReadCalleeSavedRegister(size_t reg, bool isFp, SlotType **calleeStack) const
    {
        ASSERT(reg >= GetFirstCalleeReg(ARCH, isFp));
        ASSERT(reg <= GetLastCalleeReg(ARCH, isFp));
        ASSERT(GetCalleeRegsCount(ARCH, isFp) != 0);
        size_t startSlot = GetCalleeRegsMask(ARCH, isFp).GetDistanceFromTail(reg);
        if (isFp) {
            startSlot += GetCalleeRegsCount(ARCH, false);
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ASSERT(calleeStack[startSlot] != nullptr);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return *calleeStack[startSlot];
    }

    void WriteCalleeSavedRegister(size_t reg, SlotType value, bool isFp, SlotType **calleeStack) const
    {
        ASSERT(reg >= GetFirstCalleeReg(ARCH, isFp));
        ASSERT(reg <= GetLastCalleeReg(ARCH, isFp));
        ASSERT(GetCalleeRegsCount(ARCH, isFp) != 0);
        size_t startSlot = reg - GetFirstCalleeReg(ARCH, isFp);
        if (isFp) {
            startSlot += GetCalleeRegsCount(ARCH, false);
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        ASSERT(calleeStack[startSlot] != nullptr);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        *calleeStack[startSlot] = value;
    }

    template <typename T>
    T *GetPtr(ptrdiff_t slot)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<T *>(GetFrameOrigin() - slot);
    }
    template <typename T>
    const T *GetPtr(ptrdiff_t slot) const
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<const T *>(GetFrameOrigin() - slot);
    }

    template <bool NEED_PACK>
    interpreter::VRegister GetVRegValueSlot(const VRegInfo &vreg) const;
    template <bool NEED_PACK>
    interpreter::VRegister GetVRegValueRegister(const VRegInfo &vreg, SlotType **calleeStack) const;
    template <bool NEED_PACK>
    interpreter::VRegister GetVRegValueConstant(const VRegInfo &vreg, const compiler::CodeInfo &codeInfo) const;

    uint64_t GetPackValue(VRegInfo::Type type, uint64_t val) const;

    using MemPrinter = void (*)(std::ostream &, void *, std::string_view, uintptr_t);
    void DumpCalleeRegs(std::ostream &os, MemPrinter printMem, PandaString *dscr, size_t *slot);
    void DumpCalleeFPRegs(std::ostream &os, MemPrinter printMem, PandaString *dscr, size_t *slot);
    void DumpCallerRegs(std::ostream &os, MemPrinter printMem, PandaString *dscr, size_t *slot);
    void DumpCallerFPRegs(std::ostream &os, MemPrinter printMem, PandaString *dscr, size_t *slot);
    void DumpLocals(std::ostream &os, MemPrinter printMem, PandaString *dscr, size_t *slot, int32_t maxSlot);

private:
    SlotType *fp_ {nullptr};
};

}  // namespace ark

#endif  // PANDA_CFRAME_H
