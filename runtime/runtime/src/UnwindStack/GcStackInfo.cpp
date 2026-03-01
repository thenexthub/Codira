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


#include "GcStackInfo.h"

#include <stack>

#include "Collector/TracingCollector.h"
#include "Common/StackType.h"

namespace MapleRuntime {
void GCStackInfo::FillInStackTrace()
{
    UnwindContext uwContext;
    // Top unwind context can only be runtime or Codira context.
    
    CheckTopUnwindContextAndInit(uwContext);
    while (!uwContext.frameInfo.mFrame.IsAnchorFrame(anchorFA)) {
        AnalyseAndSetFrameType(uwContext);
        stack.emplace_back(uwContext.frameInfo);
        UnwindContext caller;
        lastFrameType = uwContext.frameInfo.GetFrameType();
#ifndef _WIN64
        if (uwContext.UnwindToCallerContext(caller) == false) {
#else
        if (uwContext.UnwindToCallerContext(caller, uwCtxStatus) == false) {
#endif
            return;
        }
        uwContext = caller;
    }
}

void GCStackInfo::VisitStackRoots(const RootVisitor& func, Mutator& mutator) const
{
    RegSlotsMap regSlotsMap;
    for (auto frame : stack) {
        switch (frame.GetFrameType()) {
            case FrameType::MANAGED: {
                TracingCollector::VisitStackRoots(func, regSlotsMap, frame, mutator);
                break;
            }
            case FrameType::SAFEPOINT:
            case FrameType::STACKGROW:
#ifdef __arm__
                TracingCollector::RecordStubCalleeSaved(regSlotsMap, reinterpret_cast<Uptr>(frame.mFrame.GetFA()));
#else
                TracingCollector::RecordStubAllRegister(regSlotsMap, reinterpret_cast<Uptr>(frame.mFrame.GetFA()));
#endif
                break;
            case FrameType::C2R_STUB:
            case FrameType::C2N_STUB:
                TracingCollector::RecordStubCalleeSaved(regSlotsMap, reinterpret_cast<Uptr>(frame.mFrame.GetFA()));
                break;
            default: {
                break;
            }
        }
    }
}

void GCStackInfo::VisitHeapReferencesOnStack(const RootVisitor& rootVisitor, const DerivedPtrVisitor& derivedPtrVisitor,
                                             Mutator& mutator) const
{
    RegSlotsMap regSlotsMap;
    for (auto frame : stack) {
        switch (frame.GetFrameType()) {
            case FrameType::MANAGED: {
                TracingCollector::VisitHeapReferencesOnStack(rootVisitor, derivedPtrVisitor, regSlotsMap, frame,
                                                             mutator);
                break;
            }
            case FrameType::C2R_STUB:
            case FrameType::C2N_STUB:
                TracingCollector::RecordStubCalleeSaved(regSlotsMap, reinterpret_cast<Uptr>(frame.mFrame.GetFA()));
                break;
            case FrameType::SAFEPOINT:
            case FrameType::STACKGROW:
#ifdef __arm__
                TracingCollector::RecordStubCalleeSaved(regSlotsMap, reinterpret_cast<Uptr>(frame.mFrame.GetFA()));
#else
                TracingCollector::RecordStubAllRegister(regSlotsMap, reinterpret_cast<Uptr>(frame.mFrame.GetFA()));
#endif
                break;
            default: {
                break;
            }
        }
    }
}

void RecordStackInfo::FillInStackTrace()
{
    UnwindContext uwContext;
    // Top unwind context can only be runtime or Codira context.
    CheckTopUnwindContextAndInit(uwContext);
    while (!uwContext.frameInfo.mFrame.IsAnchorFrame(anchorFA)) {
        AnalyseAndSetFrameType(uwContext);
        FrameInfo* f = new FrameInfo(uwContext.frameInfo);
        stacks.emplace_back(f);
        UnwindContext caller;
        lastFrameType = uwContext.frameInfo.GetFrameType();
#ifndef _WIN64
        if (uwContext.UnwindToCallerContext(caller) == false) {
#else
        if (uwContext.UnwindToCallerContext(caller, uwCtxStatus) == false) {
#endif
            return;
        }
        uwContext = caller;
    }
}

void RecordStackInfo::VisitStackRoots(const RootVisitor &func, Mutator &mutator)
{
    RegSlotsMap regSlotsMap;
    for (auto frame : stacks) {
        FrameInfo &ref = *frame;
        switch (frame->GetFrameType()) {
            case FrameType::MANAGED: {
                currentFrame++;
                TracingCollector::VisitStackRoots(func, regSlotsMap, ref, mutator);
                break;
            }
            case FrameType::SAFEPOINT:
            case FrameType::STACKGROW:
#ifdef __arm__
                TracingCollector::RecordStubCalleeSaved(regSlotsMap, reinterpret_cast<Uptr>(frame->mFrame.GetFA()));
#else
                TracingCollector::RecordStubAllRegister(regSlotsMap, reinterpret_cast<Uptr>(frame->mFrame.GetFA()));
#endif
                break;
            case FrameType::C2R_STUB:
            case FrameType::C2N_STUB:
                TracingCollector::RecordStubCalleeSaved(regSlotsMap, reinterpret_cast<Uptr>(frame->mFrame.GetFA()));
                break;
            default: {
                break;
            }
        }
    }
}

void CODEThreadStackInfo::FillInStackTrace()
{
    UnwindContext uwContext;
    // Top unwind context can only be runtime or Codira context.
    CheckTopUnwindContextAndInit(uwContext);
    while (!uwContext.frameInfo.mFrame.IsAnchorFrame(anchorFA)) {
        AnalyseAndSetFrameType(uwContext);
        stack.emplace_back(uwContext.frameInfo);
        UnwindContext caller;
        lastFrameType = uwContext.frameInfo.GetFrameType();
#ifndef _WIN64
        if (uwContext.UnwindToCallerContext(caller) == false) {
#else
        if (uwContext.UnwindToCallerContext(caller, uwCtxStatus) == false) {
#endif
            return;
        }
        uwContext = caller;
    }
    filledStackSize = stack.size();
}

char* CODEThreadStackInfo::GetFuncOrFileNameStr(CString originName)
{
    CString name;
    if (originName.Length() > maxStrSize) {
            name = originName.SubStr(originName.Length() - maxStrSize);
        } else {
            name = originName;
    }
    char* nameStr = reinterpret_cast<char*>(malloc(name.Length() + 1));
    int ret = strcpy_s(nameStr, name.Length() + 1, name.Str());
    if (ret != EOK) {
        LOG(RTLOG_ERROR, "strcpy_s failed when copy func or file names");
        return nullptr;
    }
    return nameStr;
}

void CODEThreadStackInfo::GetInfoFromStackTrace(uint32_t* framePcArr, char** funcNameArr,
                                              char** fileNameArr, uint32_t* lineNumberArr)
{
    int stackSize = static_cast<int>(stack.size());
    if (stackSize >= TRACE_STACK_MAX_DEPTH) {
        stackSize = TRACE_STACK_MAX_DEPTH;
    }
    int arrIdx = 0;
    for (int stackIdx = 0; stackIdx < stackSize; stackIdx++) {
        FrameInfo frame = stack[stackIdx];
        FrameType fType = frame.GetFrameType();
        /* During stack trace, runtime stack frame is not recorded,
         * or only the outermost two runtime stack frame is recorded at most.
         */
        if ((stackIdx > 0 && fType == FrameType::NATIVE) ||
            (stackIdx < stackSize - stackSkipThreshold && fType == FrameType::RUNTIME)) {
            continue;
        }
        CString funcName = frame.GetFuncName();
        CString fileName = frame.GetFileNameForTrace();
        if (funcName.IsEmpty() || fileName.IsEmpty()) {
            continue;
        }

        char* funcNameStr = GetFuncOrFileNameStr(funcName);
        if (funcNameStr == nullptr) {
            continue;
        }
        char* fileNameStr = GetFuncOrFileNameStr(fileName);
        if (fileNameStr == nullptr) {
            free(funcNameStr);
            continue;
        }

        framePcArr[arrIdx] = frame.GetFramePc();
        funcNameArr[arrIdx] = funcNameStr;
        fileNameArr[arrIdx] = fileNameStr;
        lineNumberArr[arrIdx] = frame.GetLineNum();
        arrIdx++;
    }
    /* Avoid empty records due to empty names or other reasons. */
    if (arrIdx == 0 && stackSize > 0) {
        funcNameArr[arrIdx] = GetFuncOrFileNameStr(CString("?"));
        fileNameArr[arrIdx] = GetFuncOrFileNameStr(CString("unknown"));
        arrIdx = 1;
    }
    realStackSize = arrIdx;
}

int InitCODEThreadStackInfoFromCurrFunc(uint32_t maxStrSize,
                                      uint32_t* framePcArr, char** funcNameArr,
                                      char** fileNameArr, uint32_t* lineNumberArr)
{
    FrameInfo frameInfo;
    UnwindContext unwindCxt;
#ifdef _WIN64
    Runtime& runtime = Runtime::Current();
    WinModuleManager& winModuleManager = runtime.GetWinModuleManager();
    Uptr rip = 0;
    Uptr rsp = 0;
    GetContextWin64(&rip, &rsp);
    frameInfo = GetCurFrameInfo(winModuleManager, rip, rsp);
    UnwindContextStatus ucs = UnwindContextStatus::UNKNOWN;
    unwindCxt.frameInfo = GetCallerFrameInfo(winModuleManager, frameInfo.mFrame, ucs);
#else
    void* ip = __builtin_return_address(0);
    void* fa = __builtin_frame_address(0);
    frameInfo.mFrame.SetIP(static_cast<uint32_t*>(ip));
    frameInfo.mFrame.SetFA(static_cast<FrameAddress*>(fa)->callerFrameAddress);
    unwindCxt.frameInfo = frameInfo;
#endif
    CODEThreadStackInfo stackInfo(&unwindCxt, maxStrSize);
    stackInfo.FillInStackTrace();
    stackInfo.GetInfoFromStackTrace(framePcArr, funcNameArr, fileNameArr, lineNumberArr);
    int realStackSize = stackInfo.GetRealStackSize();
    return realStackSize;
}

extern "C" MRT_EXPORT int CODE_MCC_InitCODEthreadStackInfo(uint32_t maxStrSize, void *mut,
                                                       uint32_t* framePcArr, char** funcNameArr,
                                                       char** fileNameArr, uint32_t* lineNumberArr)
{
    int realStackSize = -1;
    auto mutator = static_cast<Mutator *>(mut);
    if (mutator == nullptr) {
        realStackSize = InitCODEThreadStackInfoFromCurrFunc(maxStrSize, framePcArr, funcNameArr,
                                                          fileNameArr, lineNumberArr);
    } else {
        UnwindContext unwindCxt = mutator->GetUnwindContext();
        CODEThreadStackInfo stackInfo(&unwindCxt, maxStrSize);
        stackInfo.FillInStackTrace();
        stackInfo.GetInfoFromStackTrace(framePcArr, funcNameArr, fileNameArr, lineNumberArr);
        realStackSize = stackInfo.GetRealStackSize();
    }

    return realStackSize;
}

} // namespace MapleRuntime
