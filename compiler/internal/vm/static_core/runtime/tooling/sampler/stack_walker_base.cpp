/**
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

#include "runtime/tooling/sampler/stack_walker_base.h"

namespace ark {

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
StackWalkerBase::StackWalkerBase(void *fp, bool isFrameCompiled)
{
    frame_ = GetTopFrameFromFp(fp, isFrameCompiled);
}

StackWalkerBase::FrameVariant StackWalkerBase::GetTopFrameFromFp(void *ptr, bool isFrameCompiled)
{
    if (isFrameCompiled) {
        if (IsBoundaryFrame<FrameKind::INTERPRETER>(ptr)) {
            auto bp = GetPrevFromBoundary<FrameKind::INTERPRETER>(ptr);
            if (GetBoundaryFrameMethod<FrameKind::COMPILER>(bp) == BYPASS) {
                return CreateCFrame(GetPrevFromBoundary<FrameKind::COMPILER>(bp));
            }
            return CreateCFrame(GetPrevFromBoundary<FrameKind::INTERPRETER>(ptr));  // NOLINT
        }
        return CreateCFrame(reinterpret_cast<SlotType *>(ptr));
    }
    return reinterpret_cast<Frame *>(ptr);
}

StackWalkerBase::CFrameType StackWalkerBase::CreateCFrame(SlotType *ptr)
{
    CFrameType cframe(ptr);
    return cframe;
}

void StackWalkerBase::NextFrame()
{
    if (IsCFrame()) {
        NextFromCFrame();
    } else {
        NextFromIFrame();
    }
}

StackWalkerBase::CFrameType StackWalkerBase::CreateCFrameForC2IBridge(Frame *frame)
{
    auto prev = GetPrevFromBoundary<FrameKind::INTERPRETER>(frame);
    ASSERT(GetBoundaryFrameMethod<FrameKind::COMPILER>(prev) != FrameBridgeKind::BYPASS);
    return CreateCFrame(reinterpret_cast<SlotType *>(prev));
}

void StackWalkerBase::NextFromCFrame()
{
    auto prev = GetCFrame().GetPrevFrame();
    if (prev == nullptr) {
        frame_ = nullptr;
        return;
    }
    auto frameMethod = GetBoundaryFrameMethod<FrameKind::COMPILER>(prev);
    switch (frameMethod) {
        case FrameBridgeKind::INTERPRETER_TO_COMPILED_CODE: {
            auto prevFrame = reinterpret_cast<Frame *>(GetPrevFromBoundary<FrameKind::COMPILER>(prev));
            if (prevFrame != nullptr && IsBoundaryFrame<FrameKind::INTERPRETER>(prevFrame)) {
                frame_ = CreateCFrameForC2IBridge(prevFrame);
                break;
            }
            frame_ = reinterpret_cast<Frame *>(prevFrame);
            break;
        }
        case FrameBridgeKind::BYPASS: {
            auto prevFrame = reinterpret_cast<Frame *>(GetPrevFromBoundary<FrameKind::COMPILER>(prev));
            if (prevFrame != nullptr && IsBoundaryFrame<FrameKind::INTERPRETER>(prevFrame)) {
                frame_ = CreateCFrameForC2IBridge(prevFrame);
                break;
            }
            frame_ = CreateCFrame(reinterpret_cast<SlotType *>(GetPrevFromBoundary<FrameKind::COMPILER>(prev)));
            break;
        }
        default:
            frame_ = CreateCFrame(reinterpret_cast<SlotType *>(prev));
            break;
    }
}

void StackWalkerBase::NextFromIFrame()
{
    auto prev = GetIFrame()->GetPrevFrame();
    if (prev == nullptr) {
        frame_ = nullptr;
        return;
    }
    if (IsBoundaryFrame<FrameKind::INTERPRETER>(prev)) {
        auto bp = GetPrevFromBoundary<FrameKind::INTERPRETER>(prev);
        if (GetBoundaryFrameMethod<FrameKind::COMPILER>(bp) == BYPASS) {
            frame_ = CreateCFrame(GetPrevFromBoundary<FrameKind::COMPILER>(bp));
        } else {
            frame_ = CreateCFrameForC2IBridge(prev);
        }
    } else {
        frame_ = reinterpret_cast<Frame *>(prev);
    }
}

}  // namespace ark
