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
#ifndef PANDA_RUNTIME_STACK_WALKER_H
#define PANDA_RUNTIME_STACK_WALKER_H

#include <variant>

#include "runtime/include/cframe.h"
#include "runtime/include/cframe_iterators.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/interpreter/frame.h"
#include "compiler/code_info/code_info.h"

namespace ark {

enum class UnwindPolicy {
    ALL,           // unwing all frames including inlined
    SKIP_INLINED,  // unwing all frames excluding inlined
    ONLY_INLINED,  // unwind all inlined frames within single cframe
};

class FrameAccessor {
public:
    using CFrameType = CFrame;
    using FrameVariant = std::variant<Frame *, CFrame>;

    explicit FrameAccessor(const FrameVariant &frame) : frame_(frame) {}

    bool IsValid() const
    {
        return IsCFrame() || GetIFrame() != nullptr;
    }

    bool IsCFrame() const
    {
        return std::holds_alternative<CFrameType>(frame_);
    }

    CFrameType &GetCFrame()
    {
        ASSERT(IsCFrame());
        return std::get<CFrameType>(frame_);
    }

    const CFrameType &GetCFrame() const
    {
        ASSERT(IsCFrame());
        return std::get<CFrameType>(frame_);
    }

    Frame *GetIFrame()
    {
        return std::get<Frame *>(frame_);
    }

    const Frame *GetIFrame() const
    {
        return std::get<Frame *>(frame_);
    }

private:
    FrameVariant frame_;
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
class StackWalker {
public:
    static constexpr size_t CALLEE_REGS_COUNT =
        GetCalleeRegsCount(RUNTIME_ARCH, false) + GetCalleeRegsCount(RUNTIME_ARCH, true);

    using SlotType = std::conditional_t<ArchTraits<RUNTIME_ARCH>::IS_64_BITS, uint64_t, uint32_t>;
    using FrameVariant = std::variant<Frame *, CFrame>;
    using CFrameType = CFrame;
    using CodeInfo = compiler::CodeInfo;
    using VRegInfo = compiler::VRegInfo;
    using CalleeRegsBuffer = std::array<SlotType, CALLEE_REGS_COUNT>;

    // Use static method to be able to ASSERT(thread->IsRuntimeCallEnabled()) before StackWalker construction to avoid
    // crash in constructor, e.g. while GetTopFrameFromFp()
    static PANDA_PUBLIC_API StackWalker Create(const ManagedThread *thread, UnwindPolicy policy = UnwindPolicy::ALL);

    StackWalker() = default;
    StackWalker(void *fp, bool isFrameCompiled, uintptr_t npc, UnwindPolicy policy = UnwindPolicy::ALL);

    virtual ~StackWalker() = default;

    NO_COPY_SEMANTIC(StackWalker);
    NO_MOVE_SEMANTIC(StackWalker);

    void Reset(const ManagedThread *thread);

    void Verify();

    PANDA_PUBLIC_API void NextFrame();

    PANDA_PUBLIC_API Method *GetMethod();

    const Method *GetMethod() const
    {
        return IsCFrame() ? GetCFrame().GetMethod() : GetIFrame()->GetMethod();
    }

    size_t GetBytecodePc() const
    {
        return IsCFrame() ? GetCFrameBytecodePc() : GetIFrame()->GetBytecodeOffset();
    }

    size_t GetNativePc() const
    {
        return IsCFrame() ? GetCFrameNativePc() : 0;
    }

    void *GetFp()
    {
        return IsCFrame() ? reinterpret_cast<void *>(GetCFrame().GetFrameOrigin())
                          : reinterpret_cast<void *>(GetIFrame());
    }

    bool HasFrame() const
    {
        return IsCFrame() || GetIFrame() != nullptr;
    }

    template <typename Func>
    bool IterateObjects(Func func)
    {
        return IterateRegs<true, false>(func);
    }

    template <typename Func>
    bool IterateObjectsWithInfo(Func func)
    {
        return IterateRegs<true, true>(func);
    }

    template <typename Func>
    bool IterateVRegsWithInfo(Func func)
    {
        return IterateRegs<false, true>(func);
    }

    bool IsCFrame() const
    {
        return std::holds_alternative<CFrameType>(frame_);
    }

    interpreter::VRegister GetVRegValue(size_t vregNum);

    template <bool IS_DYNAMIC = false, typename T>
    void SetVRegValue(VRegInfo regInfo, T value);

    CFrameType &GetCFrame()
    {
        ASSERT(IsCFrame());
        return std::get<CFrameType>(frame_);
    }

    const CFrameType &GetCFrame() const
    {
        ASSERT(IsCFrame());
        return std::get<CFrameType>(frame_);
    }

    Frame *GetIFrame()
    {
        return std::get<Frame *>(frame_);
    }

    const Frame *GetIFrame() const
    {
        return std::get<Frame *>(frame_);
    }

    auto GetCompiledCodeEntry() const
    {
        ASSERT(IsCFrame());
        return codeInfo_.GetCode();
    }

    Frame *ConvertToIFrame(FrameKind *prevFrameKind, uint32_t *numInlinedMethods);

    bool IsCompilerBoundFrame(SlotType *prev);

    FrameKind GetPreviousFrameKind() const;

    FrameAccessor GetNextFrame();

    FrameAccessor GetCurrentFrame()
    {
        return FrameAccessor(frame_);
    }

    PANDA_PUBLIC_API bool IsDynamicMethod() const;

    void DumpFrame(std::ostream &os);

    template <FrameKind KIND>
    static SlotType *GetPrevFromBoundary(void *ptr)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return *(reinterpret_cast<SlotType **>(ptr) + BoundaryFrame<KIND>::FP_OFFSET);
    }

    template <FrameKind KIND>
    static uintptr_t GetBoundaryFrameMethod(const void *ptr)
    {
        auto frameMethod = reinterpret_cast<uintptr_t>(GetMethodFromBoundary<KIND>(ptr));
        return frameMethod;
    }

    template <FrameKind KIND>
    static bool IsBoundaryFrame(const void *ptr)
    {
        if constexpr (KIND == FrameKind::INTERPRETER) {  // NOLINT
            return GetBoundaryFrameMethod<KIND>(ptr) == COMPILED_CODE_TO_INTERPRETER;
        } else {  // NOLINT
            return GetBoundaryFrameMethod<KIND>(ptr) == INTERPRETER_TO_COMPILED_CODE;
        }
    }

    bool IsInlined() const
    {
        return inlineDepth_ != -1;
    }

    // Dump modify walker state - you must call it only for rvalue object
    void Dump(std::ostream &os, bool printVregs = false) &&;

    CalleeRegsBuffer &GetCalleeRegsForDeoptimize();

private:
    // These are shortenings for General-purpose register bank (scalar integer and pointer arithmetic)
    inline constexpr size_t FirstCalleeIntReg()
    {
        return GetFirstCalleeReg(RUNTIME_ARCH, false);
    }
    inline constexpr size_t LastCalleeIntReg()
    {
        return GetLastCalleeReg(RUNTIME_ARCH, false);
    }
    inline constexpr size_t CalleeIntRegsCount()
    {
        return GetCalleeRegsCount(RUNTIME_ARCH, false);
    }

    // These are shortenings for SIMD and Floating-Point register bank
    inline constexpr size_t FirstCalleeFpReg()
    {
        return GetFirstCalleeReg(RUNTIME_ARCH, true);
    }
    inline constexpr size_t LastCalleeFpReg()
    {
        return GetLastCalleeReg(RUNTIME_ARCH, true);
    }
    inline constexpr size_t CalleeFpRegsCount()
    {
        return GetCalleeRegsCount(RUNTIME_ARCH, true);
    }

    // Struct to keep pointers to stack slots holding callee-saved regs values
    // and corresponding callee-saved regs masks.
    struct CalleeStorage {
        std::array<uintptr_t *, CALLEE_REGS_COUNT> stack = {nullptr};
        RegMask intRegsMask {0};
        RegMask fpRegsMask {0};
    };

    void InitCalleeBuffer(SlotType *calleeSlots, CalleeStorage *prevCallees);
    CFrameType CreateCFrame(SlotType *ptr, uintptr_t npc, SlotType *calleeSlots, CalleeStorage *prevCallees = nullptr);

    template <bool CREATE>
    CFrameType CreateCFrameForC2IBridge(Frame *frame);

    template <bool OBJECTS, bool WITH_REG_INFO, typename Func>
    bool IterateRegs(Func func);

    template <bool WITH_REG_INFO, typename Func>
    bool IterateAllRegsForCFrame(Func func);

    template <bool OBJECTS, bool WITH_REG_INFO, typename Func>
    bool IterateRegsForCFrameStatic(Func func);

    template <bool OBJECTS, bool WITH_REG_INFO, typename Func>
    bool IterateRegsForCFrameDynamic(Func func);

    template <bool OBJECTS, bool WITH_REG_INFO, typename Func>
    bool IterateRegsForIFrame(Func func);

    template <bool OBJECTS, bool WITH_REG_INFO, VRegInfo::Type OBJ_TYPE, VRegInfo::Type PRIMITIVE_TYPE, class F,
              typename Func>
    bool IterateRegsForIFrameInternal(F frameHandler, Func func);

    FrameVariant GetTopFrameFromFp(void *ptr, bool isFrameCompiled, uintptr_t npc);

    void NextFromCFrame();
    void NextFromIFrame();

    static Method *GetMethodFromCBoundary(void *ptr)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return *reinterpret_cast<Method **>(reinterpret_cast<SlotType *>(ptr) - CFrameLayout::MethodSlot::Start());
    }

    template <FrameKind KIND>
    static Method *GetMethodFromBoundary(void *ptr)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return *(reinterpret_cast<Method **>(ptr) + BoundaryFrame<KIND>::METHOD_OFFSET);
    }

    template <FrameKind KIND>
    static const Method *GetMethodFromBoundary(const void *ptr)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return *(reinterpret_cast<Method *const *>(ptr) + BoundaryFrame<KIND>::METHOD_OFFSET);
    }

    template <FrameKind KIND>
    static uintptr_t GetReturnAddressFromBoundary(const void *ptr)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return *(reinterpret_cast<const uintptr_t *>(ptr) + BoundaryFrame<KIND>::RETURN_OFFSET);
    }

    template <FrameKind KIND>
    static SlotType *GetCalleeStackFromBoundary(void *ptr)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return reinterpret_cast<SlotType *>(ptr) + BoundaryFrame<KIND>::CALLEES_OFFSET;
    }

    uintptr_t GetCFrameBytecodePc() const
    {
        if (GetCFrame().IsNative()) {
            return 0;
        }
        if (IsInlined()) {
            auto ii = codeInfo_.GetInlineInfo(stackmap_, inlineDepth_);
            return ii.GetBytecodePc();
        }
        return stackmap_.GetBytecodePc();
    }
    uintptr_t GetCFrameNativePc() const
    {
        return GetCFrame().IsNative() ? 0 : stackmap_.GetNativePcUnpacked();
    }

    bool HandleAddingAsCFrame();

    bool HandleAddingAsIFrame();

    void SetPrevFrame(FrameKind *prevFrameKind, void **prevFrame, CFrameType *cframe);

    Frame *GetFrameFromPrevFrame(Frame *prevFrame);

    void DumpVRegs(std::ostream &os);

#ifndef NDEBUG
    void DebugSingleFrameVerify();
#endif

    void DumpVRegLocation(std::ostream &os, VRegInfo &regInfo);

    Method *GetInlinedMethod(CFrameType &cframe);

public:
    class EnvData {
        using EnvType = coretypes::TaggedType;

    public:
        template <typename Allocator>
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
        explicit EnvData(const compiler::CodeInfo::VRegList<Allocator> &vregList, const CFrameType &cframe,
                         const CodeInfo &codeInfo, SlotType **calleeStack)
        {
            for (size_t i = 0; i < VRegInfo::ENV_COUNT; i++) {
                auto vreg = vregList[vregList.size() - (VRegInfo::ENV_COUNT - i)];
                envData_[i] = static_cast<EnvType>(cframe.GetVRegValue<true>(vreg, codeInfo, calleeStack).GetValue());
            }
        }

        const EnvType &operator[](VRegInfo::VRegType type) const
        {
            return envData_[type - VRegInfo::VRegType::ENV_BEGIN];
        };

    private:
        std::array<EnvType, VRegInfo::ENV_COUNT> envData_;
    };

private:
    FrameVariant frame_ {nullptr};
    UnwindPolicy policy_ {UnwindPolicy::ALL};
    CodeInfo codeInfo_;
    compiler::StackMap stackmap_;
    int inlineDepth_ {-1};
    CalleeStorage calleeStack_;
    CalleeStorage prevCalleeStack_;
    CalleeRegsBuffer deoptCalleeRegs_ = {0};
};

static_assert((BoundaryFrame<FrameKind::INTERPRETER>::METHOD_OFFSET) * sizeof(uintptr_t) == Frame::GetMethodOffset());
static_assert((BoundaryFrame<FrameKind::INTERPRETER>::FP_OFFSET) * sizeof(uintptr_t) == Frame::GetPrevFrameOffset());

}  // namespace ark

#endif  // PANDA_RUNTIME_STACK_WALKER_H
