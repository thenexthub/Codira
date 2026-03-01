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


#include "StackInfo.h"

#include <cstdint>

#include "Base/Log.h"
#include "Base/LogFile.h"
#include "Collector/TracingCollector.h"
#include "Common/StackType.h"
#include "Common/TypeDef.h"
#include "MangleNameHelper.h"
#include "StackMap/StackMap.h"
#include "UnwindStack/StackMetadataHelper.h"
#ifdef _WIN64
#include "UnwindWin.h"
#endif

namespace MapleRuntime {
const int StackInfo::NEED_FILTED_FLAG = -1;

#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
static uint64_t *stackFrameAlign(uint64_t *fa)
{
    auto addr = reinterpret_cast<uint64_t>(fa);
    return reinterpret_cast<uint64_t*>((addr + 0xf) & (~0xf));
}

void InitPtrAuthRAMod(FrameInfo& callerFrameInfo, FrameInfo& calleeFrameInfo)
{
    if (calleeFrameInfo.GetFrameType() == FrameType::MANAGED) {
        FuncDescRef funcDesc = MFuncDesc::GetFuncDesc(
            reinterpret_cast<Uptr>(FrameInfo::GetFuncStartPCFromFrameAddress(
                reinterpret_cast<FrameAddress*>(calleeFrameInfo.mFrame.GetFA()))));
        Uptr* stackMapEntry = funcDesc->GetStackMap();
        uint32_t validPos = 0;
        auto fa = calleeFrameInfo.mFrame.GetFA();
        uint32_t stackSize = EHFrameInfo::ReadVarInt(&stackMapEntry, validPos);
        (void)stackSize;
        (void)EHFrameInfo::ReadVarInt(&stackMapEntry, validPos);
        uint32_t calleeSavedBitmap = EHFrameInfo::ReadVarInt(&stackMapEntry, validPos);
        uint32_t count = 0;
        while (calleeSavedBitmap != 0) {
            count++;
            calleeSavedBitmap = (calleeSavedBitmap - 1) & calleeSavedBitmap;
        }
        callerFrameInfo.mFrame.SetPtrAuthRAMod(stackFrameAlign(reinterpret_cast<uint64_t*>(fa) + count));
    } else if (calleeFrameInfo.mFrame.IsC2RStubFrame() || calleeFrameInfo.mFrame.IsC2NExceptionStubFrame()) {
        auto fa = calleeFrameInfo.mFrame.GetFA();
        auto size = GetRuntimeFrameSize((MachineFrame&)calleeFrameInfo.mFrame);
        callerFrameInfo.mFrame.SetPtrAuthRAMod(
            stackFrameAlign(reinterpret_cast<uint64_t*>(reinterpret_cast<uint64_t>(fa) + size)));
    } else {
        return;
    }
}
#endif

void StackInfo::CheckTopUnwindContextAndInit(UnwindContext& uwContext)
{
    if (topContext == nullptr) {
        UnwindContext& localContext = Mutator::GetMutator()->GetUnwindContext();
        if (localContext.GetUnwindContextStatus() == UnwindContextStatus::RELIABLE) {
#ifdef _WIN64
            Runtime& runtime = Runtime::Current();
            WinModuleManager& winModuleManager = runtime.GetWinModuleManager();
            Uptr rip = 0;
            Uptr rsp = 0;
            GetContextWin64(&rip, &rsp);
            FrameInfo curFrame = GetCurFrameInfo(winModuleManager, rip, rsp);
            UnwindContextStatus ucs = UnwindContextStatus::UNKNOWN;
            uwContext.frameInfo = GetCallerFrameInfo(winModuleManager, curFrame.mFrame, ucs);
#else
            MRT_UNW_GETCALLERFRAME(uwContext.frameInfo);
#endif
            uwContext.frameInfo.SetFrameType(FrameType::RUNTIME);
            uwContext.SetUnwindContextStatus(UnwindContextStatus::RELIABLE);
        } else {
            uwContext = localContext;
        }
    } else {
        // must be runtime frame
        uwContext = *topContext;
    }

#ifdef _WIN64
    uwCtxStatus = uwContext.GetUnwindContextStatus();
#endif
}

void StackInfo::AnalyseAndSetFrameType(UnwindContext& uwContext)
{
    FrameInfo& frameInfo = uwContext.frameInfo;
    MachineFrame& mFrame = frameInfo.mFrame;
    if (mFrame.IsN2CStubFrame()) {
        MRT_Check(lastFrameType == FrameType::MANAGED, "");
        N2CSlotData* n2cSlotData = N2CFrame::GetSlotData(mFrame.GetFA());
        if (n2cSlotData->status == UnwindContextStatus::RELIABLE) {
            isReliableN2CStub = true;
        }
        frameInfo.SetFrameType(FrameType::N2C_STUB);
    } else if (mFrame.IsSafepointHandlerStubFrame()) {
        isReliableN2CStub = false;
        frameInfo.SetFrameType(FrameType::SAFEPOINT);
    } else if (mFrame.IsC2RStubFrame()) {
        isReliableN2CStub = false;
        frameInfo.SetFrameType(FrameType::C2R_STUB);
    } else if (mFrame.IsC2NStubFrame()) {
        // The lastFrameType cannot be checked here, because it may be unknow or runtime.
        isReliableN2CStub = false;
        frameInfo.SetFrameType(FrameType::C2N_STUB);
        // Runtime frame judgment needs to be placed after N2C ande C2N and C2R judgment.
        // Because they are belong to runtime frame.
    } else if (mFrame.IsStackGrowStubFrame()) {
        isReliableN2CStub = false;
        frameInfo.SetFrameType(FrameType::STACKGROW);
    } else if (mFrame.IsRuntimeFrame()) {
        frameInfo.SetFrameType(FrameType::RUNTIME);
        isReliableN2CStub = false;
    } else {
        // The judgment is mainly to identify the credible native call the managed code
        // through n2c in the macor. The so of macor is a mixed code library, which cannot
        // be directly identified by the runtime library address.
        if (isReliableN2CStub) {
            frameInfo.SetFrameType(FrameType::RUNTIME);
        } else {
            frameInfo.SetFrameType(FrameType::MANAGED);
            isReliableN2CStub = false;
            frameInfo.ResolveProcInfo();
        }
        return;
    }

    // When the stack is not empty, it is necessary to determine whether a current
    // context is the context of n2c.
    if (stack.size() != 0 && IsN2CContext(uwContext)) {
        // If the context has not been set to n2cstub before, you need to set it to
        // runtime frame.
        if (frameInfo.GetFrameType() == FrameType::UNKNOWN) {
            frameInfo.SetFrameType(FrameType::RUNTIME);
        }
        n2cCount++;
    }
}

// The current judgment of anchor context is made by comparing the previous stack
// frame type and the current stack frame type.
bool StackInfo::IsN2CContext(const UnwindContext& uwContext) const
{
    if ((lastFrameType == FrameType::MANAGED && uwContext.frameInfo.GetFrameType() == FrameType::RUNTIME) ||
        uwContext.frameInfo.GetFrameType() == FrameType::N2C_STUB) {
        return true;
    } else {
        return false;
    }
}

uint32_t* StackInfo::GetAnchorFAFromMutatorContext() const
{
    UnwindContext& localContext = Mutator::GetMutator()->GetUnwindContext();
    return localContext.anchorFA;
}

void StackInfo::ExtractLiteFrameInfoFromStack(std::vector<uint64_t>& liteFrameInfos, size_t steps) const
{
    size_t count = 1;
    for (auto frameInfo : stack) {
        if (count <= steps) {
            liteFrameInfos.push_back(reinterpret_cast<uint64_t>(frameInfo.mFrame.GetIP()));
            liteFrameInfos.push_back(reinterpret_cast<uint64_t>(frameInfo.GetFuncStartPC()));
#ifdef __APPLE__
            FuncDescRef funcDesc = MFuncDesc::GetFuncDesc(frameInfo.mFrame.GetFA());
#else
            FuncDescRef funcDesc = MFuncDesc::GetFuncDesc(reinterpret_cast<Uptr>(frameInfo.GetFuncStartPC()));
#endif
            liteFrameInfos.push_back(reinterpret_cast<uint64_t>(funcDesc));
            count++;
        }
    }
}

void StackInfo::GetStackTraceByLiteFrameInfos(const std::vector<uint64_t>& liteFrameInfos,
                                              std::vector<StackTraceElement>& stackTrace)
{
    constexpr int liteFrameInfoElementSize = 3;
    for (size_t i = 0; i < liteFrameInfos.size(); i += liteFrameInfoElementSize) {
        // Each function stack frame is represented by a triple {ip, startPC, funcDesc}
        StackMetadataHelper stackMetadataHelper(reinterpret_cast<uint32_t*>(liteFrameInfos[i]),
                                                reinterpret_cast<uint32_t*>(liteFrameInfos[i + 1]),
                                                reinterpret_cast<uint64_t*>(liteFrameInfos[i + 2]));

        StackTraceElement tmpElement;
        GetStackTraceByLiteFrameInfo(liteFrameInfos[i], liteFrameInfos[i + 1], liteFrameInfos[i + 2], tmpElement);
        if (tmpElement.lineNumber != NEED_FILTED_FLAG) {
            stackTrace.push_back(tmpElement);
        }
    }
}

void StackInfo::GetStackTraceByLiteFrameInfo(const uint64_t ip, const uint64_t pc, const uint64_t funcDesc,
                                             StackTraceElement& ste)
{
    StackMetadataHelper stackMetadataHelper(reinterpret_cast<uint32_t*>(ip), reinterpret_cast<uint32_t*>(pc),
                                            reinterpret_cast<uint64_t*>(funcDesc));
    stackMetadataHelper.GetMangleNameHelper()->Demangle();

    if (stackMetadataHelper.IsNeedFiltExceptionCreationLayer()) {
        ste.lineNumber = NEED_FILTED_FLAG;
        return;
    }

    ste.lineNumber = stackMetadataHelper.GetLineNumber();
    ste.methodName = stackMetadataHelper.GetMangleNameHelper()->GetMethodName();
    ste.className = stackMetadataHelper.GetMangleNameHelper()->GetPackClassName();
    ste.fileName = stackMetadataHelper.GetFilePathAndName();
}
} // namespace MapleRuntime
