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

#ifndef PANDA_CFRAME_ITERATORS_H
#define PANDA_CFRAME_ITERATORS_H

#include "libarkbase/utils/arch.h"
#include "libarkbase/utils/range.h"
#include "libarkfile/shorty_iterator.h"
#include "runtime/arch/helpers.h"
#include "runtime/include/cframe.h"
#include "runtime/include/method.h"
#include "libarkbase/utils/bit_utils.h"

namespace ark {

template <Arch ARCH = RUNTIME_ARCH>
class CFrameStaticNativeMethodIterator {
    using SlotType = typename CFrame::SlotType;
    using VRegInfo = compiler::VRegInfo;

    static constexpr size_t ARG_FP_REGS_COUNG = arch::ExtArchTraits<ARCH>::NUM_FP_ARG_REGS;
    static constexpr size_t ARG_GP_REGS_COUNG = arch::ExtArchTraits<ARCH>::NUM_GP_ARG_REGS;

public:
    static auto MakeRange(CFrame *cframe)
    {
        CFrameLayout cframeLayout(ARCH, 0);

        const ptrdiff_t inRegsStartSlot = static_cast<int64_t>(cframeLayout.GetCallerRegsStartSlot()) -
                                          cframeLayout.GetStackStartSlot()
                                          // skipped the slot to align the stack
                                          + ((ARCH == Arch::X86_64) ? 1 : 0);
        const ptrdiff_t inStackStartSlot = cframeLayout.GetStackArgsStartSlot() - cframeLayout.GetStackStartSlot();

        ptrdiff_t fpEndSlot = inRegsStartSlot - 1;
        ptrdiff_t fpBeginSlot = fpEndSlot + ARG_FP_REGS_COUNG;
        ptrdiff_t gprEndSlot = fpBeginSlot;
        ptrdiff_t gprBeginSlot = gprEndSlot + ARG_GP_REGS_COUNG;
        ptrdiff_t stackBeginSlot = inStackStartSlot + 1;

        Method *method = cframe->GetMethod();
        bool isStatic = method->IsStatic();
        if (!isStatic) {
            --gprBeginSlot;  // skip Method*
        }

        uint32_t numArgs = method->GetNumArgs();
        uint32_t vregNum = numArgs + (isStatic ? 1 : 0);

        return Range<CFrameStaticNativeMethodIterator>(
            CFrameStaticNativeMethodIterator(0, vregNum, method->GetShorty(), gprBeginSlot, gprEndSlot, fpBeginSlot + 1,
                                             fpEndSlot, stackBeginSlot),
            CFrameStaticNativeMethodIterator(vregNum, vregNum, method->GetShorty(), 0, 0, 0, 0, 0));
    }

    VRegInfo operator*()
    {
        return VRegInfo(currentSlot_, VRegInfo::Location::SLOT, vregType_, VRegInfo::VRegType::VREG, vregIndex_);
    }

    auto operator++()
    {
        if (++vregIndex_ >= vregNum_) {
            return *this;
        }

        // Update vreg_type_
        vregType_ = ConvertType(*shortyIt_);
        ++shortyIt_;

        // Update current_slot_
        if (vregType_ == VRegInfo::Type::FLOAT32 || vregType_ == VRegInfo::Type::FLOAT64) {
            if ((fpCurrentSlot_ - 1) > fpEndSlot_) {
                --fpCurrentSlot_;
                currentSlot_ = fpCurrentSlot_;
            } else {
                --stackCurrentSlot_;
                currentSlot_ = stackCurrentSlot_;
            }
        } else {
            if ((gprCurrentSlot_ - 1) > gprEndSlot_) {
                --gprCurrentSlot_;
                currentSlot_ = gprCurrentSlot_;
            } else {
                --stackCurrentSlot_;
                currentSlot_ = stackCurrentSlot_;
            }
        }

        return *this;
    }

    auto operator++(int)  // NOLINT(cert-dcl21-cpp)
    {
        auto res = *this;
        this->operator++();
        return res;
    }

    bool operator==(const CFrameStaticNativeMethodIterator &it) const
    {
        return vregIndex_ == it.vregIndex_;
    }

    bool operator!=(const CFrameStaticNativeMethodIterator &it) const
    {
        return !(*this == it);
    }

private:
    CFrameStaticNativeMethodIterator(uint32_t vregIndex, uint32_t vregNum, const uint16_t *shorty,
                                     ptrdiff_t gprBeginSlot, ptrdiff_t gprEndSlot, ptrdiff_t fpBeginSlot,
                                     ptrdiff_t fpEndSlot, ptrdiff_t stackCurrentSlot)
        : vregIndex_(vregIndex),
          vregNum_(vregNum),
          shortyIt_(shorty),
          currentSlot_(gprBeginSlot),
          gprCurrentSlot_(gprBeginSlot),
          gprEndSlot_(gprEndSlot),
          fpCurrentSlot_(fpBeginSlot),
          fpEndSlot_(fpEndSlot),
          stackCurrentSlot_(stackCurrentSlot)
    {
        ++shortyIt_;  // skip return value
    }

    VRegInfo::Type ConvertType(panda_file::Type type) const
    {
        switch (type.GetId()) {
            case panda_file::Type::TypeId::U1:
                return VRegInfo::Type::BOOL;
            case panda_file::Type::TypeId::I8:
            case panda_file::Type::TypeId::U8:
            case panda_file::Type::TypeId::I16:
            case panda_file::Type::TypeId::U16:
            case panda_file::Type::TypeId::I32:
            case panda_file::Type::TypeId::U32:
                return VRegInfo::Type::INT32;
            case panda_file::Type::TypeId::F32:
                return VRegInfo::Type::FLOAT32;
            case panda_file::Type::TypeId::F64:
                return VRegInfo::Type::FLOAT64;
            case panda_file::Type::TypeId::I64:
            case panda_file::Type::TypeId::U64:
                return VRegInfo::Type::INT64;
            case panda_file::Type::TypeId::REFERENCE:
                return VRegInfo::Type::OBJECT;
            case panda_file::Type::TypeId::TAGGED:
                return VRegInfo::Type::INT64;
            default:
                UNREACHABLE();
        }
        return VRegInfo::Type::INT32;
    }

private:
    uint32_t vregIndex_;
    uint32_t vregNum_;
    panda_file::ShortyIterator shortyIt_;
    ptrdiff_t currentSlot_;
    ptrdiff_t gprCurrentSlot_;
    ptrdiff_t gprEndSlot_;
    ptrdiff_t fpCurrentSlot_;
    ptrdiff_t fpEndSlot_;
    ptrdiff_t stackCurrentSlot_;
    VRegInfo::Type vregType_ = VRegInfo::Type::OBJECT;
};

template <>
class CFrameStaticNativeMethodIterator<Arch::AARCH32> {
    using SlotType = typename CFrame::SlotType;
    using VRegInfo = compiler::VRegInfo;

    static constexpr size_t ARG_FP_REGS_COUNG = arch::ExtArchTraits<Arch::AARCH32>::NUM_FP_ARG_REGS;
    static constexpr size_t ARG_GP_REGS_COUNG = arch::ExtArchTraits<Arch::AARCH32>::NUM_GP_ARG_REGS;

    static constexpr ptrdiff_t IN_REGS_START_SLOT = 24;
    static constexpr ptrdiff_t IN_STACK_START_SLOT = -11;
    static constexpr ptrdiff_t FP_END_SLOT = IN_REGS_START_SLOT - 1;
    static constexpr ptrdiff_t FP_BEGIN_SLOT = FP_END_SLOT + ARG_FP_REGS_COUNG;
    static constexpr ptrdiff_t GPR_END_SLOT = FP_BEGIN_SLOT;
    static constexpr ptrdiff_t GPR_BEGIN_SLOT = GPR_END_SLOT + ARG_GP_REGS_COUNG;
    static constexpr ptrdiff_t STACK_BEGIN_SLOT = IN_STACK_START_SLOT + 1;

public:
    static auto MakeRange(CFrame *cframe)
    {
        ptrdiff_t gprBeginSlot = GPR_BEGIN_SLOT;
        Method *method = cframe->GetMethod();
        bool isStatic = method->IsStatic();
        if (!isStatic) {
            --gprBeginSlot;  // skip Method*
        }

        uint32_t numArgs = method->GetNumArgs();
        uint32_t vregNum = numArgs + (isStatic ? 1 : 0);

        return Range<CFrameStaticNativeMethodIterator>(
            CFrameStaticNativeMethodIterator(0, vregNum, method->GetShorty(), gprBeginSlot, GPR_END_SLOT, FP_BEGIN_SLOT,
                                             FP_END_SLOT, STACK_BEGIN_SLOT),
            CFrameStaticNativeMethodIterator(vregNum, vregNum, method->GetShorty(), 0, 0, 0, 0, 0));
    }

    VRegInfo operator*()
    {
        return VRegInfo(currentSlot_, VRegInfo::Location::SLOT, vregType_, VRegInfo::VRegType::VREG, vregIndex_);
    }

    uint32_t GetSlotsCountForType(VRegInfo::Type vregType)
    {
        static_assert(arch::ExtArchTraits<Arch::AARCH32>::GPR_SIZE == 4);  // 4 bytes -- register size on AARCH32

        if (vregType == VRegInfo::Type::INT64 || vregType == VRegInfo::Type::FLOAT64) {
            return 2;  // 2 slots
        }
        return 1;
    }

    auto operator++()
    {
        if (++vregIndex_ >= vregNum_) {
            return *this;
        }

        // Update type
        vregType_ = ConvertType(*shortyIt_);
        ++shortyIt_;

        // Update slots
        auto inc = static_cast<ptrdiff_t>(GetSlotsCountForType(vregType_));
        ASSERT(inc == 1 || inc == 2);  // 1 or 2 slots
        if (inc == 1) {
            if constexpr (arch::ExtArchTraits<Arch::AARCH32>::HARDFP) {
                if (vregType_ == VRegInfo::Type::FLOAT32) {  // in this case one takes 1 slots
                    return HandleHardFloat();
                }
            }
            if ((gprCurrentSlot_ - 1) > gprEndSlot_) {
                --gprCurrentSlot_;
                currentSlot_ = gprCurrentSlot_;
            } else {
                gprCurrentSlot_ = gprEndSlot_;

                --stackCurrentSlot_;
                currentSlot_ = stackCurrentSlot_;
            }
        } else {
            if constexpr (arch::ExtArchTraits<Arch::AARCH32>::HARDFP) {
                if (vregType_ == VRegInfo::Type::FLOAT64) {  // in this case one takes 2 slots
                    return HandleHardDouble();
                }
            }
            gprCurrentSlot_ = RoundUp(gprCurrentSlot_ - 1, 2) - 1;  // 2
            if (gprCurrentSlot_ > gprEndSlot_) {
                currentSlot_ = gprCurrentSlot_;
                gprCurrentSlot_ -= 1;
            } else {
                stackCurrentSlot_ = RoundUp(stackCurrentSlot_ - 1, 2) - 1;  // 2
                currentSlot_ = stackCurrentSlot_;
                stackCurrentSlot_ -= 1;
            }
        }

        return *this;
    }

    auto operator++(int)  // NOLINT(cert-dcl21-cpp)
    {
        auto res = *this;
        this->operator++();
        return res;
    }

    bool operator==(const CFrameStaticNativeMethodIterator &it) const
    {
        return vregIndex_ == it.vregIndex_;
    }

    bool operator!=(const CFrameStaticNativeMethodIterator &it) const
    {
        return !(*this == it);
    }

private:
    CFrameStaticNativeMethodIterator(uint32_t vregIndex, uint32_t vregNum, const uint16_t *shorty,
                                     ptrdiff_t gprBeginSlot, ptrdiff_t gprEndSlot, ptrdiff_t fpBeginSlot,
                                     ptrdiff_t fpEndSlot, ptrdiff_t stackCurrentSlot)
        : vregIndex_(vregIndex),
          vregNum_(vregNum),
          shortyIt_(shorty),
          currentSlot_(gprBeginSlot),
          gprCurrentSlot_(gprBeginSlot),
          gprEndSlot_(gprEndSlot),
          fpCurrentSlot_(fpBeginSlot),
          fpEndSlot_(fpEndSlot),
          stackCurrentSlot_(stackCurrentSlot)
    {
        ++shortyIt_;  // skip return value
    }

    VRegInfo::Type ConvertType(panda_file::Type type) const
    {
        switch (type.GetId()) {
            case panda_file::Type::TypeId::U1:
                return VRegInfo::Type::BOOL;
            case panda_file::Type::TypeId::I8:
            case panda_file::Type::TypeId::U8:
            case panda_file::Type::TypeId::I16:
            case panda_file::Type::TypeId::U16:
            case panda_file::Type::TypeId::I32:
            case panda_file::Type::TypeId::U32:
                return VRegInfo::Type::INT32;
            case panda_file::Type::TypeId::F32:
                return VRegInfo::Type::FLOAT32;
            case panda_file::Type::TypeId::F64:
                return VRegInfo::Type::FLOAT64;
            case panda_file::Type::TypeId::I64:
            case panda_file::Type::TypeId::U64:
                return VRegInfo::Type::INT64;
            case panda_file::Type::TypeId::REFERENCE:
                return VRegInfo::Type::OBJECT;
            case panda_file::Type::TypeId::TAGGED:
                return VRegInfo::Type::INT64;
            default:
                UNREACHABLE();
        }
        return VRegInfo::Type::INT32;
    }

    CFrameStaticNativeMethodIterator &HandleHardFloat()
    {
        ASSERT(vregType_ == VRegInfo::Type::FLOAT32);
        if (fpCurrentSlot_ > fpEndSlot_) {
            currentSlot_ = fpCurrentSlot_;
            --fpCurrentSlot_;
        } else {
            --stackCurrentSlot_;
            currentSlot_ = stackCurrentSlot_;
        }
        return *this;
    }

    CFrameStaticNativeMethodIterator &HandleHardDouble()
    {
        ASSERT(vregType_ == VRegInfo::Type::FLOAT64);
        fpCurrentSlot_ = static_cast<ptrdiff_t>(RoundDown(static_cast<uintptr_t>(fpCurrentSlot_) + 1, 2U) - 1);
        if (fpCurrentSlot_ > fpEndSlot_) {
            currentSlot_ = fpCurrentSlot_;
            fpCurrentSlot_ -= 2U;
        } else {
            stackCurrentSlot_ = RoundUp(stackCurrentSlot_ - 1, 2U) - 1;
            currentSlot_ = stackCurrentSlot_;
            stackCurrentSlot_ -= 1;
        }
        return *this;
    }

private:
    uint32_t vregIndex_;
    uint32_t vregNum_;
    panda_file::ShortyIterator shortyIt_;
    ptrdiff_t currentSlot_;
    ptrdiff_t gprCurrentSlot_;
    ptrdiff_t gprEndSlot_;
    ptrdiff_t fpCurrentSlot_;
    ptrdiff_t fpEndSlot_;
    ptrdiff_t stackCurrentSlot_;
    VRegInfo::Type vregType_ = VRegInfo::Type::OBJECT;
};

template <Arch ARCH = RUNTIME_ARCH>
class CFrameDynamicNativeMethodIterator {
    using SlotType = typename CFrame::SlotType;
    using VRegInfo = compiler::VRegInfo;

public:
    static auto MakeRange(CFrame *cframe)
    {
        size_t constexpr GPR_ARGS_MAX = arch::ExtArchTraits<ARCH>::NUM_GP_ARG_REGS;
        size_t constexpr GPR_ARGS_INTERNAL = 2U;  // Depends on dyn callconv
        size_t constexpr GPR_FN_ARGS_NUM = 0U;    // Depends on dyn callconv
        CFrameLayout cframeLayout(ARCH, 0);

        ptrdiff_t const gprEndSlot =
            static_cast<int64_t>(cframeLayout.GetCallerRegsStartSlot()) - 1 - cframeLayout.GetStackStartSlot();
        ptrdiff_t const gprStartSlot = gprEndSlot + GPR_ARGS_MAX;

        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        Span<SlotType const> gprSlots(cframe->GetValuePtrFromSlot(gprStartSlot), GPR_ARGS_MAX);

        auto const actualArgsNum = static_cast<uint32_t>(gprSlots[1]);
        auto const stackArgsNum = actualArgsNum - GPR_FN_ARGS_NUM;

        ptrdiff_t const gprTaggedEndSlot = gprEndSlot + GPR_ARGS_INTERNAL;
        ptrdiff_t const gprTaggedStartSlot = gprTaggedEndSlot + GPR_FN_ARGS_NUM;

        ptrdiff_t const stackStartSlot = cframeLayout.GetStackArgsStartSlot() - cframeLayout.GetStackStartSlot();
        ptrdiff_t const stackEndSlot = stackStartSlot - stackArgsNum;

        return Range<CFrameDynamicNativeMethodIterator>(
            CFrameDynamicNativeMethodIterator(cframe, gprTaggedStartSlot, gprTaggedEndSlot, stackStartSlot,
                                              stackEndSlot),
            CFrameDynamicNativeMethodIterator(cframe, gprTaggedEndSlot, gprTaggedEndSlot, stackEndSlot, stackEndSlot));
    }

    VRegInfo operator*()
    {
        if (gprStartSlot_ > gprEndSlot_) {
            return VRegInfo(gprStartSlot_, VRegInfo::Location::SLOT, VRegInfo::Type::ANY, VRegInfo::VRegType::VREG,
                            vregNum_);
        }
        ASSERT(stackStartSlot_ > stackEndSlot_);
        return VRegInfo(stackStartSlot_, VRegInfo::Location::SLOT, VRegInfo::Type::ANY, VRegInfo::VRegType::VREG,
                        vregNum_);
    }

    CFrameDynamicNativeMethodIterator &operator++()
    {
        auto inc = static_cast<int64_t>(sizeof(interpreter::VRegister) / sizeof(SlotType));
        if (gprStartSlot_ > gprEndSlot_) {
            gprStartSlot_ -= inc;
            ++vregNum_;
        } else if (stackStartSlot_ > stackEndSlot_) {
            stackStartSlot_ -= inc;
            ++vregNum_;
        }
        return *this;
    }

    // NOLINTNEXTLINE(cert-dcl21-cpp)
    CFrameDynamicNativeMethodIterator operator++(int)
    {
        auto res = *this;
        this->operator++();
        return res;
    }

    bool operator==(const CFrameDynamicNativeMethodIterator &it) const
    {
        return gprStartSlot_ == it.gprStartSlot_ && stackStartSlot_ == it.stackStartSlot_;
    }

    bool operator!=(const CFrameDynamicNativeMethodIterator &it) const
    {
        return !(*this == it);
    }

private:
    CFrameDynamicNativeMethodIterator(CFrame *cframe, ptrdiff_t gprStartSlot, ptrdiff_t gprEndSlot,
                                      ptrdiff_t stackStartSlot, ptrdiff_t stackEndSlot)
        : cframe_(cframe),
          gprStartSlot_(gprStartSlot),
          gprEndSlot_(gprEndSlot),
          stackStartSlot_(stackStartSlot),
          stackEndSlot_(stackEndSlot)
    {
    }

private:
    CFrame *cframe_;
    uint32_t vregNum_ = 0;
    ptrdiff_t gprStartSlot_;
    ptrdiff_t gprEndSlot_;
    ptrdiff_t stackStartSlot_;
    ptrdiff_t stackEndSlot_;
};

}  // namespace ark

#endif  // PANDA_CFRAME_ITERATORS
