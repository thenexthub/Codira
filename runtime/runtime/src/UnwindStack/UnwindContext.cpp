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


#include "Base/LogFile.h"
#include "Common/StackType.h"

namespace MapleRuntime {

#ifndef _WIN64
bool UnwindContext::UnwindToCallerContextFromN2CStub(UnwindContext& caller) const
{
    N2CSlotData* n2cSlotData = N2CFrame::GetSlotData(frameInfo.mFrame.GetFA());
    if (n2cSlotData->status == UnwindContextStatus::RISKY ||
        n2cSlotData->status == UnwindContextStatus::SIGNAL_STATUS ||
        n2cSlotData->status == UnwindContextStatus::UNKNOWN) {
        caller.frameInfo.mFrame.SetFA(n2cSlotData->fa);
        caller.frameInfo.mFrame.SetIP(n2cSlotData->pc);
    } else if (n2cSlotData->status == UnwindContextStatus::RELIABLE) {
        frameInfo.mFrame.UnwindToCallerMachineFrame(caller.frameInfo.mFrame);
    }

    caller.SetUnwindContextStatus(UnwindContextStatus::RELIABLE);
    return true;
}

bool UnwindContext::UnwindToCallerContext(UnwindContext& caller) const
{
    DLOG(UNWIND, "UnwindToCallerContext");
    DLOG(UNWIND, "Current UnwindContext status : %p",
         static_cast<std::underlying_type<UnwindContextStatus>::type>(status));
    DLOG(UNWIND, "FrameInfo->FA\t%p", frameInfo.mFrame.GetFA());
    DLOG(UNWIND, "FrameInfo->IP\t%p", frameInfo.mFrame.GetIP());
    DLOG(UNWIND, "FrameInfoType\t%d", frameInfo.GetFrameType());
    DLOG(UNWIND, "FrameInfo->lsda\t%p", reinterpret_cast<uintptr_t>(frameInfo.GetLsdaProc()));
    DLOG(UNWIND, "FrameInfo->startProc\t%p", reinterpret_cast<uintptr_t>(frameInfo.GetStartProc()));
    if (frameInfo.GetFrameType() == FrameType::N2C_STUB) {
        UnwindToCallerContextFromN2CStub(caller);
    } else {
#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
        auto res = frameInfo.mFrame.UnwindToCallerMachineFrame(caller.frameInfo.mFrame);
        if (res) {
            InitPtrAuthRAMod(caller.frameInfo, (FrameInfo&)frameInfo);
        }
        return res;
#else
        return frameInfo.mFrame.UnwindToCallerMachineFrame(caller.frameInfo.mFrame);
#endif
    }

    return true;
}

#else
// In Windows, the time is based on the same stack unwind logic,
// except that the rbp recorded in the slot is used for jump during N2CStub.
bool UnwindContext::UnwindToCallerContext(UnwindContext& caller, UnwindContextStatus& uwCtxStatus) const
{
    DLOG(UNWIND, "UnwindToCallerContext");
    DLOG(UNWIND, "Current UnwindContext status : %p",
         static_cast<std::underlying_type<UnwindContextStatus>::type>(status));
    DLOG(UNWIND, "FrameInfo->FA\t%p", frameInfo.mFrame.GetFA());
    DLOG(UNWIND, "FrameInfo->IP\t%p", frameInfo.mFrame.GetIP());
    DLOG(UNWIND, "FrameInfoType\t%d", frameInfo.GetFrameType());
    DLOG(UNWIND, "FrameInfo->lsda\t%p", reinterpret_cast<uintptr_t>(frameInfo.GetLsdaProc()));
    DLOG(UNWIND, "FrameInfo->startProc\t%p", reinterpret_cast<uintptr_t>(frameInfo.GetStartProc()));
    if (frameInfo.GetFrameType() == FrameType::N2C_STUB) {
        N2CSlotData* n2cSlotData = N2CFrame::GetSlotData(frameInfo.mFrame.GetFA());
        if (n2cSlotData->status == UnwindContextStatus::RISKY ||
            n2cSlotData->status == UnwindContextStatus::SIGNAL_STATUS ||
            n2cSlotData->status == UnwindContextStatus::UNKNOWN) {
            caller.frameInfo.mFrame.SetFA(n2cSlotData->fa);
            caller.frameInfo.mFrame.SetIP(n2cSlotData->pc);
        } else if (n2cSlotData->status == UnwindContextStatus::RELIABLE) {
            frameInfo.mFrame.UnwindToCallerMachineFrame(caller.frameInfo, uwCtxStatus);
        }
        caller.SetUnwindContextStatus(UnwindContextStatus::RELIABLE);
        return true;
    } else {
    return frameInfo.mFrame.UnwindToCallerMachineFrame(caller.frameInfo, uwCtxStatus);
    }
    return true;
}
#endif

void UnwindContext::GoIntoManagedCode()
{
    frameInfo.mFrame.SetIP(nullptr);
    frameInfo.mFrame.SetFA(nullptr);
    status = UnwindContextStatus::RELIABLE;
}

void UnwindContext::GoIntoNativeCode(const uint32_t* pc, void* fa)
{
    frameInfo.mFrame.SetIP(pc);
    frameInfo.mFrame.SetFA(reinterpret_cast<FrameAddress*>(fa));
    status = UnwindContextStatus::RISKY;
}
} // namespace MapleRuntime
