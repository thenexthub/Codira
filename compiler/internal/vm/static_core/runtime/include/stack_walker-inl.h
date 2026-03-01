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

#ifndef PANDA_RUNTIME_STACK_WALKER_INL_H
#define PANDA_RUNTIME_STACK_WALKER_INL_H

#include "stack_walker.h"
#include "runtime/include/cframe_iterators.h"
#include "compiler/code_info/code_info.h"

namespace ark {

template <bool OBJECTS, bool WITH_REG_INFO, class VRegRef, typename Func>
// NOLINTNEXTLINE(google-runtime-references)
bool InvokeCallback(Func func, [[maybe_unused]] compiler::VRegInfo regInfo, VRegRef &vreg)
{
    if (OBJECTS && !vreg.HasObject()) {
        return true;
    }
    if constexpr (WITH_REG_INFO) {  // NOLINT
        if (!func(regInfo, vreg)) {
            return false;
        }
    } else {  // NOLINT
        if (!func(vreg)) {
            return false;
        }
    }
    return true;
}

template <bool WITH_REG_INFO, typename Func>
bool StackWalker::IterateAllRegsForCFrame(Func func)
{
    auto &cframe = GetCFrame();
    const auto regs =
        codeInfo_.GetVRegList(stackmap_, inlineDepth_, mem::InternalAllocator<>::GetInternalAllocatorFromRuntime());

    for (auto &regInfo : regs) {
        if (!regInfo.IsLive()) {
            continue;
        }
        interpreter::VRegister vreg0;
        interpreter::VRegister vreg1;
        interpreter::StaticVRegisterRef resReg(&vreg0, &vreg1);
        // Need cast as uintptr_t and uint64_t are not compatible on 64-bit macOS.
        cframe.GetVRegValue(regInfo, codeInfo_, reinterpret_cast<SlotType **>(calleeStack_.stack.data()), resReg);
        if (!InvokeCallback<false, WITH_REG_INFO>(func, regInfo, resReg)) {
            return false;
        }
    }

    return true;
}

template <bool OBJECTS, bool WITH_REG_INFO, typename Func>
bool StackWalker::IterateRegsForCFrameStatic(Func func)
{
    ASSERT(IsCFrame());
    ASSERT(!IsDynamicMethod());
    auto &cframe = GetCFrame();

    if (cframe.IsNativeMethod()) {
        for (auto regInfo : CFrameStaticNativeMethodIterator<RUNTIME_ARCH>::MakeRange(&cframe)) {
            interpreter::VRegister vreg0;
            interpreter::VRegister vreg1;
            interpreter::StaticVRegisterRef resReg(&vreg0, &vreg1);
            cframe.GetVRegValue(regInfo, codeInfo_, reinterpret_cast<SlotType **>(calleeStack_.stack.data()), resReg);
            if (!InvokeCallback<OBJECTS, WITH_REG_INFO>(func, regInfo, resReg)) {
                return false;
            }
        }
        return true;
    }

    if (OBJECTS) {
        codeInfo_.EnumerateStaticRoots(stackmap_, [this, &cframe, &func](VRegInfo regInfo) {
            interpreter::VRegister vreg0;
            interpreter::VRegister vreg1;
            interpreter::StaticVRegisterRef resReg(&vreg0, &vreg1);
            cframe.GetVRegValue(regInfo, codeInfo_, reinterpret_cast<SlotType **>(calleeStack_.stack.data()), resReg);
            return !resReg.HasObject() || InvokeCallback<OBJECTS, WITH_REG_INFO>(func, regInfo, resReg);
        });
        return true;
    }

    return IterateAllRegsForCFrame<WITH_REG_INFO>(func);
}

template <bool OBJECTS, bool WITH_REG_INFO, typename Func>
bool StackWalker::IterateRegsForCFrameDynamic(Func func)
{
    ASSERT(IsCFrame());
    ASSERT(IsDynamicMethod());
    auto &cframe = GetCFrame();

    if (cframe.IsNativeMethod()) {
        for (auto regInfo : CFrameDynamicNativeMethodIterator<RUNTIME_ARCH>::MakeRange(&cframe)) {
            interpreter::VRegister vreg0;
            interpreter::DynamicVRegisterRef resReg(&vreg0);
            cframe.GetVRegValue(regInfo, codeInfo_, reinterpret_cast<SlotType **>(calleeStack_.stack.data()), resReg);
            if (!InvokeCallback<OBJECTS, WITH_REG_INFO>(func, regInfo, resReg)) {
                return false;
            }
        }
        return true;
    }

    if (OBJECTS) {
        codeInfo_.EnumerateDynamicRoots(stackmap_, [this, &cframe, &func](VRegInfo regInfo) {
            interpreter::VRegister vreg0;
            interpreter::DynamicVRegisterRef resReg(&vreg0);
            cframe.GetVRegValue(regInfo, codeInfo_, reinterpret_cast<SlotType **>(calleeStack_.stack.data()), resReg);
            return !resReg.HasObject() || InvokeCallback<OBJECTS, WITH_REG_INFO>(func, regInfo, resReg);
        });
        return true;
    }

    return IterateAllRegsForCFrame<WITH_REG_INFO>(func);
}

template <bool OBJECTS, bool WITH_REG_INFO, StackWalker::VRegInfo::Type OBJ_TYPE,
          StackWalker::VRegInfo::Type PRIMITIVE_TYPE, class F, typename Func>
bool StackWalker::IterateRegsForIFrameInternal(F frameHandler, Func func)
{
    for (size_t i = 0; i < frameHandler.GetSize(); i++) {
        auto vreg = frameHandler.GetVReg(i);
        if (OBJECTS && !vreg.HasObject()) {
            continue;
        }
        auto type = vreg.HasObject() ? OBJ_TYPE : PRIMITIVE_TYPE;
        VRegInfo regInfo(0, VRegInfo::Location::SLOT, type, VRegInfo::VRegType::VREG, i);
        if (!InvokeCallback<OBJECTS, WITH_REG_INFO>(func, regInfo, vreg)) {
            return false;
        }
    }

    auto acc = frameHandler.GetAccAsVReg();
    auto type = acc.HasObject() ? OBJ_TYPE : PRIMITIVE_TYPE;
    VRegInfo regInfo(0, VRegInfo::Location::SLOT, type, VRegInfo::VRegType::ACC, 0);
    return InvokeCallback<OBJECTS, WITH_REG_INFO>(func, regInfo, acc);
}

template <bool OBJECTS, bool WITH_REG_INFO, typename Func>
bool StackWalker::IterateRegsForIFrame(Func func)
{
    auto frame = GetIFrame();
    if (frame->IsDynamic()) {
        DynamicFrameHandler frameHandler(frame);
        ASSERT(IsDynamicMethod());
        return IterateRegsForIFrameInternal<OBJECTS, WITH_REG_INFO, VRegInfo::Type::ANY, VRegInfo::Type::ANY>(
            frameHandler, func);
    }
    StaticFrameHandler frameHandler(frame);
    return IterateRegsForIFrameInternal<OBJECTS, WITH_REG_INFO, VRegInfo::Type::OBJECT, VRegInfo::Type::INT64>(
        frameHandler, func);
}

template <bool OBJECTS, bool WITH_REG_INFO, typename Func>
bool StackWalker::IterateRegs(Func func)
{
    if (IsCFrame()) {
        if (IsDynamicMethod()) {
            return IterateRegsForCFrameDynamic<OBJECTS, WITH_REG_INFO>(func);
        }
        return IterateRegsForCFrameStatic<OBJECTS, WITH_REG_INFO>(func);
    }
    return IterateRegsForIFrame<OBJECTS, WITH_REG_INFO>(func);
}

}  // namespace ark

#endif  // PANDA_RUNTIME_STACK_WALKER_INL_H
