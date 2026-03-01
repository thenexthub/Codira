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
#ifndef PANDA_RUNTIME_TOOLING_SAMPLER_STACK_WALKER__BASE_H
#define PANDA_RUNTIME_TOOLING_SAMPLER_STACK_WALKER__BASE_H

#include <variant>

#include "runtime/include/cframe.h"
#include "runtime/interpreter/frame.h"

namespace ark {

class StackWalkerBase {
public:
    using SlotType = std::conditional_t<ArchTraits<RUNTIME_ARCH>::IS_64_BITS, uint64_t, uint32_t>;
    using FrameVariant = std::variant<Frame *, CFrame>;
    using CFrameType = CFrame;

    StackWalkerBase() = default;
    StackWalkerBase(void *fp, bool isFrameCompiled);

    virtual ~StackWalkerBase() = default;

    NO_COPY_SEMANTIC(StackWalkerBase);
    NO_MOVE_SEMANTIC(StackWalkerBase);

    void NextFrame();

    Frame *GetIFrame()
    {
        return std::get<Frame *>(frame_);
    }

    const Frame *GetIFrame() const
    {
        return std::get<Frame *>(frame_);
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

    bool IsCFrame() const
    {
        return std::holds_alternative<CFrameType>(frame_);
    }

    bool HasFrame() const
    {
        return IsCFrame() || GetIFrame() != nullptr;
    }

    Method *GetMethod()
    {
        ASSERT(HasFrame());
        return IsCFrame() ? GetCFrame().GetMethod() : GetIFrame()->GetMethod();
    }

    const Method *GetMethod() const
    {
        ASSERT(HasFrame());
        return IsCFrame() ? GetCFrame().GetMethod() : GetIFrame()->GetMethod();
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

    template <FrameKind KIND>
    static uintptr_t GetBoundaryFrameMethod(const void *ptr)
    {
        auto frameMethod = reinterpret_cast<uintptr_t>(GetMethodFromBoundary<KIND>(ptr));
        return frameMethod;
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
    static SlotType *GetPrevFromBoundary(void *ptr)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        return *(reinterpret_cast<SlotType **>(ptr) + BoundaryFrame<KIND>::FP_OFFSET);
    }

    static bool IsMethodInBoundaryFrame(const Method *method)
    {
        return IsMethodInI2CFrame(method) || IsMethodInC2IFrame(method) || IsMethodInBPFrame(method);
    }

    static bool IsMethodInI2CFrame(const Method *method)
    {
        return method == reinterpret_cast<void *>(FrameBridgeKind::INTERPRETER_TO_COMPILED_CODE);
    }

    static bool IsMethodInC2IFrame(const Method *method)
    {
        return method == reinterpret_cast<void *>(FrameBridgeKind::COMPILED_CODE_TO_INTERPRETER);
    }

    static bool IsMethodInBPFrame(const Method *method)
    {
        return method == reinterpret_cast<void *>(FrameBridgeKind::BYPASS);
    }

    size_t GetBytecodePc() const
    {
        return IsCFrame() ? 0 : GetIFrame()->GetBytecodeOffset();
    }

private:
    FrameVariant GetTopFrameFromFp(void *ptr, bool isFrameCompiled);

    CFrameType CreateCFrame(SlotType *ptr);

    CFrameType CreateCFrameForC2IBridge(Frame *frame);

    void NextFromCFrame();

    void NextFromIFrame();

private:
    FrameVariant frame_ {nullptr};
};

}  // namespace ark

#endif  // PANDA_RUNTIME_TOOLING_SAMPLER_STACK_WALKER__BASE_H
