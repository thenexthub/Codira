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

#ifndef PANDA_CFRAME_LAYOUT_H
#define PANDA_CFRAME_LAYOUT_H

#include "arch.h"
#include "bit_field.h"

namespace ark {

enum FrameBridgeKind {
    INTERPRETER_TO_COMPILED_CODE = 1,
    COMPILED_CODE_TO_INTERPRETER = 2,
    BYPASS = 3,
};

template <ssize_t START, ssize_t SIZE>
class StackRegion {
public:
    static_assert(SIZE >= 0);
    template <size_t SZ>
    using NextStackRegion = StackRegion<START + SIZE, SZ>;

    using NextStackSlot = StackRegion<START + SIZE, 1>;

    static constexpr ssize_t Start()
    {
        return START;
    }
    static constexpr ssize_t End()
    {
        return Start() + GetSize();
    }
    static constexpr ssize_t GetSize()
    {
        return SIZE;
    }
    template <class CFrameLayoutT>
    static constexpr ssize_t GetOffsetFromSpInSlots(const CFrameLayoutT &fl)
    {
        return fl.template GetFrameSize<CFrameLayoutT::OffsetUnit::SLOTS>() - START - 2U;
    }
    template <class CFrameLayoutT>
    static constexpr ssize_t GetOffsetFromSpInBytes(const CFrameLayoutT &fl)
    {
        return GetOffsetFromSpInSlots(fl) * static_cast<ssize_t>(fl.GetSlotSize());
    }
    template <class CFrameLayoutT>
    static constexpr ssize_t GetOffsetFromFpInSlots()
    {
        return START;
    }
    template <class CFrameLayoutT>
    static constexpr ssize_t GetOffsetFromFpInBytes(const CFrameLayoutT &fl)
    {
        return START * static_cast<ssize_t>(fl.GetSlotSize());
    }

private:
};

class CFrameLayout {
public:
    constexpr CFrameLayout(Arch arch, size_t spillsCount, bool alignSpills = true)
        : arch_(arch), spillsCount_(alignSpills ? AlignSpillCount(arch, spillsCount) : spillsCount)
    {
    }
    ~CFrameLayout() = default;
    DEFAULT_COPY_SEMANTIC(CFrameLayout);
    DEFAULT_MOVE_SEMANTIC(CFrameLayout);

    enum class OffsetOrigin { SP, FP };
    enum class OffsetUnit { BYTES, SLOTS };

    using StackArgSlot = StackRegion<-2, 1>;               // -2 slot
    using LrSlot = StackArgSlot::NextStackSlot;            // -1 slot
    using PrevFrameSlot = LrSlot::NextStackSlot;           //  0 slot
    using MethodSlot = PrevFrameSlot::NextStackSlot;       //  1 slot
    using FlagsSlot = MethodSlot::NextStackSlot;           //  2 slot
    using DataRegion = FlagsSlot::NextStackRegion<2>;      // [3..4] slots
    using LocalsRegion = DataRegion::NextStackRegion<4>;   // [5..8] slots
    using SlotsRegion = LocalsRegion::NextStackRegion<0>;  // [9...] slots
    using RegsRegion = SlotsRegion;

    static constexpr ssize_t HEADER_SIZE = FlagsSlot::End() - LrSlot::Start();

    enum class FrameKind : uint8_t { DEFAULT = 0, OSR = 1, NATIVE = 2, LAST = NATIVE };

    using ShouldDeoptimizeFlag = BitField<bool, 0, 1>;
    using HasFloatRegsFlag = ShouldDeoptimizeFlag::NextFlag;
    using FrameKindField = HasFloatRegsFlag::NextField<FrameKind, MinimumBitsToStore(FrameKind::LAST)>;

    // Current usage of the locals:
    //  [0..1] slots: internal spill slots for codegen
    //  [2..3] slots: fp and lr in osr mode
    // NOTE(msherstennikov): need to make flexible machinery to handle locals
    static constexpr ptrdiff_t STACK_ARGS_START = StackArgSlot::Start();
    static constexpr ptrdiff_t FRAME_START_SLOT = LrSlot::Start();
    static constexpr size_t LOCALS_START_SLOT = 5;
    static constexpr size_t STACK_START_SLOT = 9;
    static constexpr size_t CALLEE_REGS_START_SLOT = STACK_START_SLOT;

    // NB! Use getter below instead.
    static constexpr size_t CALLER_REGS_START_SLOT =
        CALLEE_REGS_START_SLOT + GetCalleeRegsCount(RUNTIME_ARCH, false) + GetCalleeRegsCount(RUNTIME_ARCH, true);

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    constexpr size_t GetCalleeRegsStartSlot() const
    {
        return STACK_START_SLOT;
    }

    constexpr size_t GetCalleeFpRegsStartSlot() const
    {
        return GetCalleeRegsStartSlot() + ::ark::GetCalleeRegsCount(GetArch(), false);
    }

    constexpr size_t GetCallerRegsStartSlot() const
    {
        return GetCalleeFpRegsStartSlot() + ::ark::GetCalleeRegsCount(GetArch(), true);
    }

    constexpr size_t GetCallerFpRegsStartSlot() const
    {
        return GetCallerRegsStartSlot() + ::ark::GetCallerRegsCount(GetArch(), false);
    }

    constexpr size_t GetSpillsStartSlot() const
    {
        return GetCallerFpRegsStartSlot() + ::ark::GetCallerRegsCount(GetArch(), true);
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    constexpr ptrdiff_t GetStackArgsStartSlot() const
    {
        return StackArgSlot::Start();
    }

    constexpr Arch GetArch() const
    {
        return arch_;
    }

    template <OffsetUnit UNIT>
    constexpr size_t GetFrameSize() const
    {
        // +1 for LR slot
        size_t sizeInSlots = STACK_START_SLOT + GetFirstSpillSlot() + spillsCount_ + 1U;
        return UNIT == OffsetUnit::BYTES ? (sizeInSlots * GetSlotSize()) : sizeInSlots;
    }

    template <OffsetOrigin ORIGIN, OffsetUnit UNIT>
    constexpr ssize_t GetOffset(ssize_t slot) const
    {
        if constexpr (ORIGIN == OffsetOrigin::SP) {  // NOLINT(readability-braces-around-statements)
            const auto offset = GetFrameSize<OffsetUnit::SLOTS>() - slot - 2U;
            if constexpr (UNIT == OffsetUnit::BYTES) {  // NOLINT
                return offset * GetSlotSize();
            }
            return offset;
        } else {                                        // NOLINT
            if constexpr (UNIT == OffsetUnit::BYTES) {  // NOLINT
                return slot * PointerSize(arch_);
            }
            return slot;
        }
    }

    template <OffsetOrigin ORIGIN, OffsetUnit UNIT>
    constexpr ssize_t GetMethodOffset() const
    {
        return GetOffset<ORIGIN, UNIT>(MethodSlot::Start());
    }

    template <OffsetOrigin ORIGIN, OffsetUnit UNIT>
    constexpr ssize_t GetReturnAddressOffset() const
    {
        return GetOffset<ORIGIN, UNIT>(LrSlot::Start());
    }

    template <OffsetOrigin ORIGIN, OffsetUnit UNIT>
    constexpr ssize_t GetFreeSlotOffset() const
    {
        return GetOffset<ORIGIN, UNIT>(LOCALS_START_SLOT);
    }

    template <OffsetOrigin ORIGIN, OffsetUnit UNIT>
    constexpr ssize_t GetSpillOffset(size_t spillSlot) const
    {
        size_t shift = Is64BitsArch(arch_) ? 0 : 1;  // in arm32 one slot is 2 word and shifted by 1
        return GetOffset<ORIGIN, UNIT>(STACK_START_SLOT + GetFirstSpillSlot() + (spillSlot << shift) + shift);
    }

    constexpr ssize_t GetSpillOffsetFromSpInBytes(size_t spillSlot) const
    {
        return GetSpillOffset<OffsetOrigin::SP, OffsetUnit::BYTES>(spillSlot);
    }

    constexpr size_t GetSpillOffsetFromFpInBytes(size_t spillSlot) const
    {
        return GetSpillOffset<OffsetOrigin::FP, OffsetUnit::BYTES>(spillSlot);
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    static constexpr ssize_t GetStackStartSlot()
    {
        return STACK_START_SLOT;
    }

    constexpr size_t GetCalleeSlotsCount() const
    {
        return GetCalleeRegistersCount(false) + GetCalleeRegistersCount(true);
    }

    constexpr size_t GetCallerSlotsCount() const
    {
        return GetCallerRegistersCount(false) + GetCallerRegistersCount(true);
    }

    constexpr size_t GetFirstSpillSlot() const
    {
        return GetCalleeSlotsCount() + GetCallerSlotsCount();
    }
    constexpr size_t GetLastSpillSlot() const
    {
        return GetFirstSpillSlot() + spillsCount_ - 1;
    }

    constexpr size_t GetCalleeFirstSlot(bool isFp) const
    {
        return isFp ? GetCalleeRegistersCount(false) : 0;
    }

    constexpr size_t GetCalleeLastSlot(bool isFp) const
    {
        return GetCalleeFirstSlot(isFp) + GetCalleeRegistersCount(isFp) - 1;
    }

    constexpr size_t GetCallerFirstSlot(bool isFp) const
    {
        return GetCalleeLastSlot(true) + 1 + (isFp ? GetCallerRegistersCount(false) : 0);
    }

    constexpr size_t GetCallerLastSlot(bool isFp) const
    {
        return GetCallerFirstSlot(isFp) + GetCallerRegistersCount(isFp) - 1;
    }

    constexpr size_t GetCalleeRegistersCount(bool isFp) const
    {
        return ark::GetCalleeRegsCount(arch_, isFp);
    }

    constexpr size_t GetCallerRegistersCount(bool isFp) const
    {
        return ark::GetCallerRegsCount(arch_, isFp);
    }

    constexpr size_t GetSlotSize() const
    {
        return PointerSize(arch_);
    }

    constexpr size_t GetSpillsCount() const
    {
        return spillsCount_;
    }

    constexpr size_t GetRegsSlotsCount() const
    {
        return GetCalleeSlotsCount() + GetCallerSlotsCount() + GetSpillsCount();
    }

    static constexpr size_t GetLocalsCount()
    {
        return STACK_START_SLOT - LOCALS_START_SLOT;
    }

    static constexpr ssize_t GetOsrFpLrSlot()
    {
        return LocalsRegion::Start() + static_cast<ssize_t>(GetLocalsCount()) - 1;
    }

    constexpr ssize_t GetOsrFpLrOffset() const
    {
        return GetOffset<CFrameLayout::OffsetOrigin::FP, CFrameLayout::OffsetUnit::BYTES>(GetOsrFpLrSlot());
    }

    static constexpr size_t GetFpLrSlotsCount()
    {
        static_assert(MethodSlot::Start() > LrSlot::Start());
        return MethodSlot::Start() - LrSlot::Start();
    }

    static constexpr size_t GetTopToRegsSlotsCount()
    {
        static_assert(SlotsRegion::Start() > LrSlot::Start());
        return SlotsRegion::Start() - LrSlot::Start();
    }

private:
    constexpr size_t AlignSpillCount(Arch arch, size_t spillsCount)
    {
        // Allign by odd-number, because GetSpillsStartSlot begins from fp (+1 slot for lr)
        if (arch == Arch::AARCH64 || arch == Arch::X86_64) {
            if (((GetSpillsStartSlot() + spillsCount) % 2U) == 0) {
                spillsCount++;
            }
        } else if (arch == Arch::AARCH32) {
            // Additional slot for spill/fill <-> sf-registers ldrd miscorp
            spillsCount = (spillsCount + 1) * 2U;
            if (((GetSpillsStartSlot() + spillsCount) % 2U) == 0) {
                spillsCount++;
            }
        }
        return spillsCount;
    }

private:
    Arch arch_;
    size_t spillsCount_ {0};
};

static_assert(CFrameLayout::GetLocalsCount() >= 2U);
static_assert(CFrameLayout::GetFpLrSlotsCount() == 2U);

enum class FrameKind { NONE, INTERPRETER, COMPILER };

template <FrameKind KIND>
struct BoundaryFrame;

// Compiled to Interpreter
template <>
struct BoundaryFrame<FrameKind::INTERPRETER> {
    static constexpr ssize_t METHOD_OFFSET = 1;
    static constexpr ssize_t FP_OFFSET = 0;
    static constexpr ssize_t RETURN_OFFSET = 2;
    static constexpr ssize_t CALLEES_OFFSET = -1;
};

// Interpreter to Compiled
// Value at offset 0 is the FP of previous native frame, which is possibly different with that at FP_OFFSET below.
template <>
struct BoundaryFrame<FrameKind::COMPILER> {
    static constexpr ssize_t METHOD_OFFSET = -1;
    // Stores FP of previous managed frame.
    static constexpr ssize_t FP_OFFSET = -2;
    static constexpr ssize_t RETURN_OFFSET = 1;
    // NB: The actual offset value of callee-saved registers is -3.
    // Set to -2 due to ARM64 alignment requirements.
    static constexpr ssize_t CALLEES_OFFSET = -2;
};

using CFrameReturnAddr = CFrameLayout::LrSlot;
using CFramePrevFrame = CFrameLayout::PrevFrameSlot;
using CFrameMethod = CFrameLayout::MethodSlot;
using CFrameFlags = CFrameLayout::FlagsSlot;
using CFrameData = CFrameLayout::DataRegion;
using CFrameLocals = CFrameLayout::LocalsRegion;
using CFrameSlots = CFrameLayout::SlotsRegion;
using CFrameRegs = CFrameLayout::RegsRegion;

}  // namespace ark

#endif  // PANDA_CFRAME_LAYOUT_H
