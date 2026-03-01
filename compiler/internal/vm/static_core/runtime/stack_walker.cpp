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

#include "compiler/code_info/code_info.h"
#include "runtime/include/stack_walker-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/include/panda_vm.h"
#include "libarkbase/mem/mem.h"
#include "libarkfile/bytecode_instruction.h"
#include "runtime/interpreter/runtime_interface.h"

#include <iomanip>

namespace ark {

StackWalker StackWalker::Create(const ManagedThread *thread, UnwindPolicy policy)
{
    ASSERT(thread != nullptr);
#ifndef NDEBUG
    ASSERT(thread->IsRuntimeCallEnabled());
    if (Runtime::GetOptions().IsVerifyCallStack()) {
        StackWalker(thread->GetCurrentFrame(), thread->IsCurrentFrameCompiled(), thread->GetNativePc(), policy)
            .Verify();
    }
#endif
    return StackWalker(thread->GetCurrentFrame(), thread->IsCurrentFrameCompiled(), thread->GetNativePc(), policy);
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
StackWalker::StackWalker(void *fp, bool isFrameCompiled, uintptr_t npc, UnwindPolicy policy)
{
    frame_ = GetTopFrameFromFp(fp, isFrameCompiled, npc);
    if (policy == UnwindPolicy::SKIP_INLINED) {
        inlineDepth_ = -1;
    }
}

void StackWalker::Reset(const ManagedThread *thread)
{
    ASSERT(thread != nullptr);
    frame_ = GetTopFrameFromFp(thread->GetCurrentFrame(), thread->IsCurrentFrameCompiled(), thread->GetNativePc());
}

/* static */
typename StackWalker::FrameVariant StackWalker::GetTopFrameFromFp(void *ptr, bool isFrameCompiled, uintptr_t npc)
{
    if (isFrameCompiled) {
        if (IsBoundaryFrame<FrameKind::INTERPRETER>(ptr)) {
            auto bp = GetPrevFromBoundary<FrameKind::INTERPRETER>(ptr);
            if (GetBoundaryFrameMethod<FrameKind::COMPILER>(bp) == BYPASS) {
                return CreateCFrame(GetPrevFromBoundary<FrameKind::COMPILER>(bp),
                                    GetReturnAddressFromBoundary<FrameKind::COMPILER>(bp),
                                    GetCalleeStackFromBoundary<FrameKind::COMPILER>(bp));
            }
            return CreateCFrame(GetPrevFromBoundary<FrameKind::INTERPRETER>(ptr),
                                GetReturnAddressFromBoundary<FrameKind::INTERPRETER>(ptr),
                                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                                reinterpret_cast<SlotType *>(ptr) +
                                    BoundaryFrame<FrameKind::INTERPRETER>::CALLEES_OFFSET);  // NOLINT
        }
        return CreateCFrame(reinterpret_cast<SlotType *>(ptr), npc, nullptr);
    }
    return reinterpret_cast<Frame *>(ptr);
}

Method *StackWalker::GetInlinedMethod(CFrameType &cframe)
{
    ASSERT(IsInlined());
    auto methodVariant = codeInfo_.GetMethod(stackmap_, inlineDepth_);
    if (std::holds_alternative<uint32_t>(methodVariant)) {
        auto classLinker = Runtime::GetCurrent()->GetClassLinker();
        auto inlineInfo = codeInfo_.GetInlineInfo(stackmap_, inlineDepth_);
        auto entityId = panda_file::File::EntityId(std::get<uint32_t>(methodVariant));
        auto pandaFileIndex = inlineInfo.GetFileIndex();
        if (pandaFileIndex != panda_file::File::INVALID_FILE_INDEX) {
            pandaFileIndex -= panda_file::File::FILE_INDEX_BASE_OFFSET;
            auto pf = classLinker->GetAotManager()->GetPandaFileBySnapshotIndex(pandaFileIndex);
            return classLinker->GetMethod(*pf, entityId, cframe.GetMethod()->GetClass()->GetLoadContext());
        }
        return classLinker->GetMethod(*cframe.GetMethod(), entityId);
    }
    return reinterpret_cast<Method *>(std::get<void *>(methodVariant));
}

Method *StackWalker::GetMethod()
{
    ASSERT(HasFrame());
    if (!IsCFrame()) {
        ASSERT(GetIFrame() != nullptr);
        return GetIFrame()->GetMethod();
    }
    auto &cframe = GetCFrame();
    if (!cframe.IsNative()) {
        // NOTE(m.strizhak): replace this condition with assert after fixing JIT trampolines for sampler
        if (!stackmap_.IsValid()) {
            return nullptr;
        }
        if (IsInlined()) {
            return GetInlinedMethod(cframe);
        }
    }
    return cframe.GetMethod();
}

template <bool CREATE>
StackWalker::CFrameType StackWalker::CreateCFrameForC2IBridge(Frame *frame)
{
    auto prev = GetPrevFromBoundary<FrameKind::INTERPRETER>(frame);
    ASSERT(GetBoundaryFrameMethod<FrameKind::COMPILER>(prev) != FrameBridgeKind::BYPASS);
    // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
    if constexpr (CREATE) {
        return CreateCFrame(reinterpret_cast<SlotType *>(prev),
                            GetReturnAddressFromBoundary<FrameKind::INTERPRETER>(frame),
                            GetCalleeStackFromBoundary<FrameKind::INTERPRETER>(frame));
    }
    return CFrameType(prev);
}

StackWalker::CFrameType StackWalker::CreateCFrame(SlotType *ptr, uintptr_t npc, SlotType *calleeSlots,
                                                  CalleeStorage *prevCallees)
{
    CFrameType cframe(ptr);
    // NOTE(m.strizhak): replace this condition with assert after fixing JIT trampolines for sampler
    if (cframe.GetMethod() == nullptr) {
        return cframe;
    }
    if (cframe.IsNativeMethod()) {
        return cframe;
    }
    const void *codeEntry;
    if (cframe.ShouldDeoptimize()) {
        // When method was deoptimized due to speculation failure, regular code entry become invalid,
        // so we read entry from special backup field in the frame.
        codeEntry = cframe.GetDeoptCodeEntry();
    } else if (cframe.IsOsr()) {
        codeEntry = Thread::GetCurrent()->GetVM()->GetCompiler()->GetOsrCode(cframe.GetMethod());
    } else {
        codeEntry = cframe.GetMethod()->GetCompiledEntryPoint();
    }
    new (&codeInfo_) CodeInfo(CodeInfo::GetCodeOriginFromEntryPoint(codeEntry));
    // StackOverflow stackmap has zero address
    if (npc == 0) {
        stackmap_ = codeInfo_.FindStackMapForNativePc(npc);
    } else {
        auto code = reinterpret_cast<uintptr_t>(codeInfo_.GetCode());
        CHECK_GT(npc, code);
        CHECK_LT(npc - code, std::numeric_limits<uint32_t>::max());
        stackmap_ = codeInfo_.FindStackMapForNativePc(npc - code);
    }

    ASSERT_PRINT(stackmap_.IsValid(),
                 "Stackmap not found " << cframe.GetMethod()->GetFullName() << ": npc=0x" << std::hex << npc
                                       << ", code=[" << reinterpret_cast<const void *>(codeInfo_.GetCode())
                                       << ".."
                                       // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                                       << reinterpret_cast<const void *>(codeInfo_.GetCode() + codeInfo_.GetCodeSize())
                                       << "]" << std::dec);
    calleeStack_.intRegsMask = codeInfo_.GetHeader().GetCalleeRegMask();
    calleeStack_.fpRegsMask = codeInfo_.GetHeader().GetCalleeFpRegMask();
    inlineDepth_ = codeInfo_.GetInlineDepth(stackmap_);

    InitCalleeBuffer(calleeSlots, prevCallees);

    return cframe;
}

/**
 * If all callees are saved then callee-saved regs are placed on the stack as follows:
 *
 * ---------------------  <-- callee_slots
 * LastCalleeReg   (x28)
 * ---------------------  <-- callee_slots - 1
 *                  ...
 * ---------------------
 * FirstCalleeReg  (x19)
 * ---------------------  <-- callee_slots - CalleeRegsCount()
 * LastCalleeFpReg (d15)
 * ---------------------
 *                  ...
 * ---------------------
 * FirstCalleeFpReg (d8)
 * ---------------------  <-- callee_slots - CalleeRegsCount() - CalleeFpRegsCount()
 *
 * If only used callees are saved, then the space is reserved for all callee registers,
 * but only umasked regs are saved and there are no gaps between them.
 *
 * Suppose that regs masks are as follows:
 *
 * int_regs_mask = 0x00980000 (1001 1000 0000 0000 0000 0000, i.e. x19, x20 and x23 must be saved)
 * fp_regs_mask  = 0x0,
 *
 * then we have the following layout:
 *
 * --------------------  <-- callee_slots
 *                (x23)
 * --------------------  <-- callee_slots - 1
 *                (x20)
 * --------------------  <-- callee_slots - 2
 *                (x19)
 * --------------------  <-- callee_slots - 3
 *                 ...
 * --------------------
 *                (---)
 * --------------------  <-- callee_slots - CalleeIntRegsCount()
 *                 ...
 * --------------------
 *                (---)
 * --------------------  <-- callee_slots - CalleeIntRegsCount() - CalleeFpRegsCount()
 */
void StackWalker::InitCalleeBuffer(SlotType *calleeSlots, CalleeStorage *prevCallees)
{
    constexpr RegMask ARCH_INT_REGS_MASK(ark::GetCalleeRegsMask(RUNTIME_ARCH, false));
    constexpr RegMask ARCH_FP_REGS_MASK(ark::GetCalleeRegsMask(RUNTIME_ARCH, true));

    bool prevIsNative = IsCFrame() ? GetCFrame().IsNative() : false;
    if (calleeSlots != nullptr || prevCallees != nullptr) {
        // Process scalar integer callee registers
        for (size_t reg = FirstCalleeIntReg(); reg <= LastCalleeIntReg(); reg++) {
            size_t offset = reg - FirstCalleeIntReg();
            if (prevCallees == nullptr || prevIsNative) {
                size_t slot = ARCH_INT_REGS_MASK.GetDistanceFromHead(reg);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                calleeStack_.stack[offset] = bit_cast<uintptr_t *>(calleeSlots - slot - 1);
            } else if (prevCallees->intRegsMask.Test(reg)) {
                size_t slot = prevCallees->intRegsMask.GetDistanceFromHead(reg);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                calleeStack_.stack[offset] = bit_cast<uintptr_t *>(calleeSlots - slot - 1);
            } else {
                ASSERT(nullptr != prevCallees->stack[offset]);
                calleeStack_.stack[offset] = prevCallees->stack[offset];
            }
        }
        // Process SIMD and Floating-Point callee registers
        for (size_t reg = FirstCalleeFpReg(); reg <= LastCalleeFpReg(); reg++) {
            size_t offset = CalleeIntRegsCount() + reg - FirstCalleeFpReg();
            if (prevCallees == nullptr || prevIsNative) {
                size_t slot = ARCH_FP_REGS_MASK.GetDistanceFromHead(reg);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                calleeStack_.stack[offset] = bit_cast<uintptr_t *>(calleeSlots - CalleeIntRegsCount() - slot - 1);
            } else if (prevCallees->fpRegsMask.Test(reg)) {
                size_t slot = prevCallees->fpRegsMask.GetDistanceFromHead(reg);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                calleeStack_.stack[offset] = bit_cast<uintptr_t *>(calleeSlots - CalleeIntRegsCount() - slot - 1);
            } else {
                ASSERT(nullptr != prevCallees->stack[offset]);
                calleeStack_.stack[offset] = prevCallees->stack[offset];
            }
        }
    }
}

StackWalker::CalleeRegsBuffer &StackWalker::GetCalleeRegsForDeoptimize()
{
    // Process scalar integer callee registers
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    SlotType *calleeSrcSlots = GetCFrame().GetCalleeSaveStack() - 1;
    SlotType *calleeDstSlots = &deoptCalleeRegs_[CalleeFpRegsCount()];
    for (size_t reg = FirstCalleeIntReg(); reg <= LastCalleeIntReg(); reg++) {
        size_t offset = reg - FirstCalleeIntReg();
        if (calleeStack_.intRegsMask.Test(reg)) {
            size_t slot = calleeStack_.intRegsMask.GetDistanceFromHead(reg);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            calleeDstSlots[offset] = *(calleeSrcSlots - slot);
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            calleeDstSlots[offset] = *calleeStack_.stack[offset];
        }
    }
    // Process SIMD and Floating-Point callee registers
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    calleeSrcSlots = GetCFrame().GetCalleeSaveStack() - CalleeIntRegsCount() - 1;
    calleeDstSlots = deoptCalleeRegs_.begin();
    for (size_t reg = FirstCalleeFpReg(); reg <= LastCalleeFpReg(); reg++) {
        size_t offset = reg - FirstCalleeFpReg();
        if (calleeStack_.fpRegsMask.Test(reg)) {
            size_t slot = calleeStack_.fpRegsMask.GetDistanceFromHead(reg);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            calleeDstSlots[offset] = *(calleeSrcSlots - slot);
        } else {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            calleeDstSlots[offset] = *calleeStack_.stack[CalleeIntRegsCount() + offset];
        }
    }

    return deoptCalleeRegs_;
}

interpreter::VRegister StackWalker::GetVRegValue(size_t vregNum)
{
    if (IsCFrame()) {
        // NOTE(msherstennikov): we need to cache vregs_list within single cframe
        auto vregsList =
            codeInfo_.GetVRegList(stackmap_, inlineDepth_, mem::InternalAllocator<>::GetInternalAllocatorFromRuntime());
        ASSERT(vregsList[vregNum].GetIndex() == vregNum);
        interpreter::VRegister vreg0;
        [[maybe_unused]] interpreter::VRegister vreg1;
        GetCFrame().GetVRegValue(vregsList[vregNum], codeInfo_,
                                 reinterpret_cast<SlotType **>(calleeStack_.stack.data()),
                                 interpreter::StaticVRegisterRef(&vreg0, &vreg1));
        return vreg0;
    }
    ASSERT(vregNum < GetIFrame()->GetSize());
    return GetIFrame()->GetVReg(vregNum);
}

template <bool IS_DYNAMIC, typename T>
void StackWalker::SetVRegValue(VRegInfo regInfo, T value)
{
    if (IsCFrame()) {
        auto &cframe = GetCFrame();
        if (IsDynamicMethod()) {
            if constexpr (sizeof(T) == sizeof(uint64_t)) {  // NOLINT
                cframe.SetVRegValue<true>(regInfo, bit_cast<uint64_t>(value),
                                          bit_cast<SlotType **>(calleeStack_.stack.data()));
            } else {  // NOLINT
                static_assert(sizeof(T) == sizeof(uint32_t));
                cframe.SetVRegValue<true>(regInfo, static_cast<uint64_t>(bit_cast<uint32_t>(value)),
                                          reinterpret_cast<SlotType **>(calleeStack_.stack.data()));
            }
        } else {
            if constexpr (sizeof(T) == sizeof(uint64_t)) {  // NOLINT
                cframe.SetVRegValue(regInfo, bit_cast<uint64_t>(value),
                                    reinterpret_cast<SlotType **>(calleeStack_.stack.data()));
            } else {  // NOLINT
                static_assert(sizeof(T) == sizeof(uint32_t));
                cframe.SetVRegValue(regInfo, static_cast<uint64_t>(bit_cast<uint32_t>(value)),
                                    reinterpret_cast<SlotType **>(calleeStack_.stack.data()));
            }
        }
    } else {
        auto vreg = GetFrameHandler<IS_DYNAMIC>(GetIFrame()).GetVReg(regInfo.GetIndex());
        if constexpr (std::is_same_v<T, ObjectHeader *>) {  // NOLINT
            ASSERT(vreg.HasObject() && "Trying to change object variable by scalar value");
            vreg.SetReference(value);
        } else {  // NOLINT
            ASSERT(!vreg.HasObject() && "Trying to change object variable by scalar value");
            vreg.Set(value);
        }
    }
}

template void StackWalker::SetVRegValue(VRegInfo reg_info, uint32_t value);
template void StackWalker::SetVRegValue(VRegInfo reg_info, int32_t value);
template void StackWalker::SetVRegValue(VRegInfo reg_info, uint64_t value);
template void StackWalker::SetVRegValue(VRegInfo reg_info, int64_t value);
template void StackWalker::SetVRegValue(VRegInfo reg_info, float value);
template void StackWalker::SetVRegValue(VRegInfo reg_info, double value);
template void StackWalker::SetVRegValue(VRegInfo reg_info, ObjectHeader *value);
template void StackWalker::SetVRegValue<true>(VRegInfo reg_info, uint32_t value);
template void StackWalker::SetVRegValue<true>(VRegInfo reg_info, int32_t value);
template void StackWalker::SetVRegValue<true>(VRegInfo reg_info, uint64_t value);
template void StackWalker::SetVRegValue<true>(VRegInfo reg_info, int64_t value);
template void StackWalker::SetVRegValue<true>(VRegInfo reg_info, float value);
template void StackWalker::SetVRegValue<true>(VRegInfo reg_info, double value);
template void StackWalker::SetVRegValue<true>(VRegInfo reg_info, ObjectHeader *value);

void StackWalker::NextFrame()
{
    if (IsCFrame()) {
        NextFromCFrame();
    } else {
        NextFromIFrame();
    }
}

void StackWalker::NextFromCFrame()
{
    if (IsInlined()) {
        if (policy_ != UnwindPolicy::SKIP_INLINED) {
            inlineDepth_--;
            return;
        }
        inlineDepth_ = -1;
    }
    if (policy_ == UnwindPolicy::ONLY_INLINED) {
        frame_ = nullptr;
        return;
    }
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
                frame_ = CreateCFrameForC2IBridge<true>(prevFrame);
                break;
            }

            frame_ = reinterpret_cast<Frame *>(prevFrame);
            break;
        }
        case FrameBridgeKind::BYPASS: {
            auto prevFrame = reinterpret_cast<Frame *>(GetPrevFromBoundary<FrameKind::COMPILER>(prev));
            if (prevFrame != nullptr && IsBoundaryFrame<FrameKind::INTERPRETER>(prevFrame)) {
                frame_ = CreateCFrameForC2IBridge<true>(prevFrame);
                break;
            }
            frame_ = CreateCFrame(reinterpret_cast<SlotType *>(GetPrevFromBoundary<FrameKind::COMPILER>(prev)),
                                  GetReturnAddressFromBoundary<FrameKind::COMPILER>(prev),
                                  GetCalleeStackFromBoundary<FrameKind::COMPILER>(prev));
            break;
        }
        default:
            prevCalleeStack_ = calleeStack_;
            frame_ = CreateCFrame(reinterpret_cast<SlotType *>(prev), GetCFrame().GetLr(),
                                  GetCFrame().GetCalleeSaveStack(), &prevCalleeStack_);
            break;
    }
}

void StackWalker::NextFromIFrame()
{
    if (policy_ == UnwindPolicy::ONLY_INLINED) {
        frame_ = nullptr;
        return;
    }
    auto prev = GetIFrame()->GetPrevFrame();
    if (prev == nullptr) {
        frame_ = nullptr;
        return;
    }
    if (IsBoundaryFrame<FrameKind::INTERPRETER>(prev)) {
        auto bp = GetPrevFromBoundary<FrameKind::INTERPRETER>(prev);
        if (GetBoundaryFrameMethod<FrameKind::COMPILER>(bp) == BYPASS) {
            frame_ = CreateCFrame(GetPrevFromBoundary<FrameKind::COMPILER>(bp),
                                  GetReturnAddressFromBoundary<FrameKind::COMPILER>(bp),
                                  GetCalleeStackFromBoundary<FrameKind::COMPILER>(bp));
        } else {
            frame_ = CreateCFrameForC2IBridge<true>(prev);
        }
    } else {
        frame_ = reinterpret_cast<Frame *>(prev);
    }
}

FrameAccessor StackWalker::GetNextFrame()
{
    if (IsCFrame()) {
        if (IsInlined()) {
            return FrameAccessor(frame_);
        }
        auto prev = GetCFrame().GetPrevFrame();
        if (prev == nullptr) {
            return FrameAccessor(nullptr);
        }
        auto frameMethod = GetBoundaryFrameMethod<FrameKind::COMPILER>(prev);
        switch (frameMethod) {
            case FrameBridgeKind::INTERPRETER_TO_COMPILED_CODE: {
                auto prevFrame = reinterpret_cast<Frame *>(GetPrevFromBoundary<FrameKind::COMPILER>(prev));
                if (prevFrame != nullptr && IsBoundaryFrame<FrameKind::INTERPRETER>(prevFrame)) {
                    return FrameAccessor(CreateCFrameForC2IBridge<false>(prevFrame));
                }
                return FrameAccessor(prevFrame);
            }
            case FrameBridgeKind::BYPASS: {
                auto prevFrame = reinterpret_cast<Frame *>(GetPrevFromBoundary<FrameKind::COMPILER>(prev));
                if (prevFrame != nullptr && IsBoundaryFrame<FrameKind::INTERPRETER>(prevFrame)) {
                    return FrameAccessor(CreateCFrameForC2IBridge<false>(prevFrame));
                }
                return FrameAccessor(
                    CFrameType(reinterpret_cast<SlotType *>(GetPrevFromBoundary<FrameKind::COMPILER>(prev))));
            }
            default:
                return FrameAccessor(CFrameType(reinterpret_cast<SlotType *>(prev)));
        }
    } else {
        auto prev = GetIFrame()->GetPrevFrame();
        if (prev == nullptr) {
            return FrameAccessor(nullptr);
        }
        if (IsBoundaryFrame<FrameKind::INTERPRETER>(prev)) {
            auto bp = GetPrevFromBoundary<FrameKind::INTERPRETER>(prev);
            if (GetBoundaryFrameMethod<FrameKind::COMPILER>(bp) == BYPASS) {
                return FrameAccessor(CreateCFrame(GetPrevFromBoundary<FrameKind::COMPILER>(bp),
                                                  GetReturnAddressFromBoundary<FrameKind::COMPILER>(bp),
                                                  GetCalleeStackFromBoundary<FrameKind::COMPILER>(bp)));
            }
            return FrameAccessor(CreateCFrameForC2IBridge<false>(prev));
        }
        return FrameAccessor(reinterpret_cast<Frame *>(prev));
    }
}

FrameKind StackWalker::GetPreviousFrameKind() const
{
    if (IsCFrame()) {
        auto prev = GetCFrame().GetPrevFrame();
        if (prev == nullptr) {
            return FrameKind::NONE;
        }
        if (IsBoundaryFrame<FrameKind::COMPILER>(prev)) {
            return FrameKind::INTERPRETER;
        }
        return FrameKind::COMPILER;
    }
    auto prev = GetIFrame()->GetPrevFrame();
    if (prev == nullptr) {
        return FrameKind::NONE;
    }
    if (IsBoundaryFrame<FrameKind::INTERPRETER>(prev)) {
        return FrameKind::COMPILER;
    }
    return FrameKind::INTERPRETER;
}

bool StackWalker::IsCompilerBoundFrame(SlotType *prev)
{
    if (IsBoundaryFrame<FrameKind::COMPILER>(prev)) {
        return true;
    }
    if (GetBoundaryFrameMethod<FrameKind::COMPILER>(prev) == FrameBridgeKind::BYPASS) {
        auto prevFrame = reinterpret_cast<Frame *>(GetPrevFromBoundary<FrameKind::COMPILER>(prev));
        // Case for clinit:
        // Compiled code -> C2I -> InitializeClass -> call clinit -> I2C -> compiled code for clinit
        if (prevFrame != nullptr && IsBoundaryFrame<FrameKind::INTERPRETER>(prevFrame)) {
            return true;
        }
    }

    return false;
}

Frame *StackWalker::GetFrameFromPrevFrame(Frame *prevFrame)
{
    auto vregList =
        codeInfo_.GetVRegList(stackmap_, inlineDepth_, mem::InternalAllocator<>::GetInternalAllocatorFromRuntime());
    auto *method = GetMethod();
    ASSERT(method != nullptr);
    Frame *frame;
    if (IsDynamicMethod()) {
        /* If there is a usage of rest arguments in dynamic function, then a managed object to contain actual arguments
         * is constructed in prologue. Thus there is no need to reconstruct rest arguments here
         */
        auto numActualArgs = method->GetNumArgs();
        /* If there are no arguments-keeping object construction in execution path, the number of actual args may be
         * retreived from cframe
         */
        size_t frameNumVregs = method->GetNumVregs() + numActualArgs;
        frame = interpreter::RuntimeInterface::CreateFrameWithActualArgs<true>(frameNumVregs, numActualArgs, method,
                                                                               prevFrame);
        ASSERT(frame != nullptr);
        frame->SetDynamic();
        DynamicFrameHandler frameHandler(frame);
        static constexpr uint8_t ACC_OFFSET = VRegInfo::ENV_COUNT + 1;
        for (size_t i = 0; i < vregList.size() - ACC_OFFSET; i++) {
            if (!vregList[i].IsLive()) {
                continue;
            }
            auto regRef = frameHandler.GetVReg(i);
            GetCFrame().GetPackVRegValue(vregList[i], codeInfo_,
                                         reinterpret_cast<SlotType **>(calleeStack_.stack.data()), regRef);
        }
        {
            auto vreg = vregList[vregList.size() - ACC_OFFSET];
            if (vreg.IsLive()) {
                auto regRef = frameHandler.GetAccAsVReg();
                GetCFrame().GetPackVRegValue(vreg, codeInfo_, reinterpret_cast<SlotType **>(calleeStack_.stack.data()),
                                             regRef);
            }
        }
        EnvData envData {vregList, GetCFrame(), codeInfo_, reinterpret_cast<SlotType **>(calleeStack_.stack.data())};
        Thread::GetCurrent()->GetVM()->GetLanguageContext().RestoreEnv(frame, envData);
    } else {
        auto frameNumVregs = method->GetNumVregs() + method->GetNumArgs();
        ASSERT((frameNumVregs + 1) >= vregList.size());
        frame = interpreter::RuntimeInterface::CreateFrame(frameNumVregs, method, prevFrame);
        StaticFrameHandler frameHandler(frame);
        for (size_t i = 0; i < vregList.size(); i++) {
            if (!vregList[i].IsLive()) {
                continue;
            }
            bool isAcc = i == (vregList.size() - 1);
            auto regRef = isAcc ? frame->GetAccAsVReg() : frameHandler.GetVReg(i);
            GetCFrame().GetVRegValue(vregList[i], codeInfo_, reinterpret_cast<SlotType **>(calleeStack_.stack.data()),
                                     regRef);
        }
    }
    return frame;
}

Frame *StackWalker::ConvertToIFrame(FrameKind *prevFrameKind, uint32_t *numInlinedMethods)
{
    if (!IsCFrame()) {
        return GetIFrame();
    }
    auto &cframe = GetCFrame();

    auto inlineDepth = inlineDepth_;
    bool isInvoke = false;

    void *prevFrame;
    bool isInit = false;
    if (IsInlined()) {
        inlineDepth_--;
        *numInlinedMethods = *numInlinedMethods + 1;
        prevFrame = ConvertToIFrame(prevFrameKind, numInlinedMethods);
        auto iframe = static_cast<Frame *>(prevFrame);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto pc = iframe->GetMethod()->GetInstructions() + iframe->GetBytecodeOffset();
        if (BytecodeInstruction(pc).HasFlag(BytecodeInstruction::INIT_OBJ)) {
            isInit = true;
        }
    } else {
        auto prev = cframe.GetPrevFrame();
        if (prev == nullptr) {
            *prevFrameKind = FrameKind::NONE;
            prevFrame = nullptr;
        } else if (IsCompilerBoundFrame(prev)) {
            isInvoke = true;
            prevFrame =
                reinterpret_cast<Frame *>(StackWalker::GetPrevFromBoundary<FrameKind::COMPILER>(cframe.GetPrevFrame()));
            if (prevFrameKind != nullptr) {
                *prevFrameKind = FrameKind::INTERPRETER;
            }
        } else {
            prevFrame = cframe.GetPrevFrame();
            if (prevFrameKind != nullptr) {
                *prevFrameKind = FrameKind::COMPILER;
            }
        }
    }
    inlineDepth_ = inlineDepth;
    Frame *frame = GetFrameFromPrevFrame(reinterpret_cast<Frame *>(prevFrame));

    frame->SetDeoptimized();
    frame->SetBytecodeOffset(GetBytecodePc());
    if (isInit) {
        frame->SetInitobj();
    }
    if (isInvoke) {
        frame->SetInvoke();
    }
    return frame;
}

bool StackWalker::IsDynamicMethod() const
{
    // Dynamic method may have no class
    return GetMethod()->GetClass() == nullptr ||
           ark::panda_file::IsDynamicLanguage(Runtime::GetCurrent()->GetLanguageContext(*GetMethod()).GetLanguage());
}

#ifndef NDEBUG
void StackWalker::DebugSingleFrameVerify()
{
    ASSERT(GetMethod() != nullptr);
    IterateVRegsWithInfo([this]([[maybe_unused]] const auto &regInfo, const auto &vreg) {
        if (regInfo.GetType() == compiler::VRegInfo::Type::ANY) {
            ASSERT(IsDynamicMethod());
            return true;
        }

        if (!vreg.HasObject()) {
            ASSERT(!regInfo.IsObject());
            vreg.GetLong();
            return true;
        }
        // Use Frame::VRegister::HasObject() to detect objects
        ASSERT(regInfo.IsObject());
        if (ObjectHeader *object = vreg.GetReference(); object != nullptr) {
            auto *cls = object->ClassAddr<Class>();
            if (!IsAddressInObjectsHeap(cls)) {
                StackWalker::Create(ManagedThread::GetCurrent()).Dump(std::cerr, true);
                // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
                LOG(FATAL, INTEROP) << "Wrong class " << cls << " for object " << object << "\n";
            } else {
                cls->GetDescriptor();
            }
        }
        return true;
    });

    if (IsCFrame()) {
        IterateObjects([this](const auto &vreg) {
            if (IsDynamicMethod()) {
                ASSERT(vreg.HasObject());
                return true;
            }

            ASSERT(vreg.HasObject());
            ObjectHeader *object = vreg.GetReference();
            if (object == nullptr) {
                return true;
            }
            ASSERT(IsAddressInObjectsHeap(object));
            auto *cls = object->ClassAddr<Class>();
            if (!IsAddressInObjectsHeap(cls)) {
                StackWalker::Create(ManagedThread::GetCurrent()).Dump(std::cerr, true);
                // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
                LOG(FATAL, INTEROP) << "Wrong class " << cls << " for object " << object << "\n";
            } else {
                cls->GetDescriptor();
            }
            return true;
        });
    }
}
#endif  // ifndef NDEBUG

void StackWalker::Verify()
{
    for (; HasFrame(); NextFrame()) {
#ifndef NDEBUG
        DebugSingleFrameVerify();
#endif  // ifndef NDEBUG
    }
}

void StackWalker::DumpVRegLocation(std::ostream &os, VRegInfo &regInfo)
{
    [[maybe_unused]] static constexpr size_t WIDTH_LOCATION = 12;
    os << std::setw(WIDTH_LOCATION) << std::setfill(' ') << regInfo.GetTypeString();  // NOLINT
    if (IsCFrame()) {
        os << regInfo.GetLocationString() << ":" << std::dec << helpers::ToSigned(regInfo.GetValue());
    } else {
        os << '-';
    }
}

void StackWalker::DumpVRegs(std::ostream &os)
{
    [[maybe_unused]] static constexpr size_t WIDTH_REG = 10;
    [[maybe_unused]] static constexpr size_t WIDTH_TYPE = 20;

    IterateVRegsWithInfo([this, &os](auto regInfo, const auto &vreg) {
        os << "     " << std::setw(WIDTH_REG) << std::setfill(' ') << std::right
           << (regInfo.IsSpecialVReg() ? VRegInfo::VRegTypeToString(regInfo.GetVRegType())
                                       : (std::string("v") + std::to_string(regInfo.GetIndex())));
        os << " = ";
        if (regInfo.GetType() == compiler::VRegInfo::Type::ANY) {
            os << "0x";
        }
        os << std::left;
        os << std::setw(WIDTH_TYPE) << std::setfill(' ');
        switch (regInfo.GetType()) {
            case compiler::VRegInfo::Type::INT64:
            case compiler::VRegInfo::Type::INT32:
                os << std::dec << vreg.GetLong();
                break;
            case compiler::VRegInfo::Type::FLOAT64:
                os << vreg.GetDouble();
                break;
            case compiler::VRegInfo::Type::FLOAT32:
                os << vreg.GetFloat();
                break;
            case compiler::VRegInfo::Type::BOOL:
                os << (vreg.Get() ? "true" : "false");
                break;
            case compiler::VRegInfo::Type::OBJECT:
                os << vreg.GetReference();
                break;
            case compiler::VRegInfo::Type::ANY: {
                os << std::hex << static_cast<uint64_t>(vreg.GetValue());
                break;
            }
            case compiler::VRegInfo::Type::UNDEFINED:
                os << "undfined";
                break;
            default:
                os << "unknown";
                break;
        }
        DumpVRegLocation(os, regInfo);
        os << std::endl;
        return true;
    });
}

// Dump function change StackWalker object-state, that's why it may be called only
// with rvalue reference.
void StackWalker::Dump(std::ostream &os, bool printVregs /* = false */) &&
{
    [[maybe_unused]] static constexpr size_t WIDTH_INDEX = 4;
    [[maybe_unused]] static constexpr size_t WIDTH_FRAME = 8;

    size_t frameIndex = 0;
    os << "Panda call stack:\n";
    for (; HasFrame(); NextFrame()) {
        os << std::setw(WIDTH_INDEX) << std::setfill(' ') << std::right << std::dec << frameIndex << ": "
           << std::setfill('0');
        os << std::setw(WIDTH_FRAME) << std::hex;
        os << (IsCFrame() ? reinterpret_cast<Frame *>(GetCFrame().GetFrameOrigin()) : GetIFrame()) << " in ";
        DumpFrame(os);
        os << std::endl;
        if (printVregs) {
            DumpVRegs(os);
        }
        if (IsCFrame() && printVregs) {
            os << "roots:";
            IterateObjectsWithInfo([&os](auto &regInfo, const auto &vreg) {
                ASSERT(vreg.HasObject());
                os << " " << regInfo.GetLocationString() << "[" << std::dec << regInfo.GetValue() << "]=" << std::hex
                   << vreg.GetReference();
                return true;
            });
            os << std::endl;
        }
        frameIndex++;
    }
}

void StackWalker::DumpFrame(std::ostream &os)
{
    ASSERT(GetMethod() != nullptr);
    os << GetMethod()->GetFullName();
    if (IsCFrame()) {
        if (GetCFrame().IsNative()) {
            os << " (native)";
        } else {
            os << " (compiled" << (GetCFrame().IsOsr() ? "/osr" : "") << ": npc=" << GetNativePc()
               << (IsInlined() ? ", inlined) " : ") ");
            if (IsInlined()) {
                codeInfo_.DumpInlineInfo(os, stackmap_, inlineDepth_);
            } else {
                codeInfo_.Dump(os, stackmap_);
            }
        }
    } else {
        os << " (managed)";
    }
}

}  // namespace ark
