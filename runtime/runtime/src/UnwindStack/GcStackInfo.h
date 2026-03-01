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


#ifndef MRT_GC_STACKINFO_H
#define MRT_GC_STACKINFO_H

#include "Base/LogFile.h"
#include "StackInfo.h"
#include "StackMap/StackMapTypeDef.h"

namespace MapleRuntime {
#define TRACE_STACK_MAX_DEPTH 15

class GCStackInfo : public StackInfo {
public:
    explicit GCStackInfo(const UnwindContext* context = nullptr) : StackInfo(context)
    {
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
        DLOG(UNWIND, "GC Stack Info");
        DLOG(UNWIND, "TopContext : %x", context);
        if (context != nullptr) {
            DLOG(UNWIND, "Top Context ip : %x Top Context fa : %x", context->frameInfo.mFrame.GetIP(),
                 context->frameInfo.mFrame.GetFA());
        }
#endif
    }

    ~GCStackInfo() override = default;
    void FillInStackTrace() override;
    void VisitStackRoots(const RootVisitor& func, Mutator& mutator) const;
    void VisitHeapReferencesOnStack(const RootVisitor& rootVisitor, const DerivedPtrVisitor& derivedPtrVisitor,
                                    Mutator& mutator) const;
};

class RecordStackInfo : public GCStackInfo {
public:
    explicit RecordStackInfo(const UnwindContext* context = nullptr, uint32_t threadId = 0,
                             CString threadname = nullptr, int threadState = -1)
        : GCStackInfo(context), tid(threadId), name(threadname), state(threadState) {}
    ~RecordStackInfo() override
    {
        for (auto f:stacks) {
            if (f==nullptr) {
                break;
            }
            delete f;
            f = nullptr;
        }
    }
    uint32_t GetStackTid() { return tid; }
    CString GetThreadName() { return name; }
    int GetThreadState() { return state; }
    uint32_t GetCurrentFrame() { return currentFrame; }
    void FillInStackTrace() override;
    void VisitStackRoots(const RootVisitor &func, Mutator &mutator);

    std::vector<FrameInfo* > stacks;

private:
    uint32_t tid = 0;
    CString name;
    uint32_t currentFrame = 0;
    int state;
};

class CODEThreadStackInfo : public GCStackInfo {
public:
    explicit CODEThreadStackInfo(const UnwindContext* context = nullptr, uint32_t pMaxStrSize = 0)
        : GCStackInfo(context), maxStrSize(pMaxStrSize) {}
    
    ~CODEThreadStackInfo() override = default;

    void FillInStackTrace() override;
    void GetInfoFromStackTrace(uint32_t* framePcArr, char** funcNameArr, char** fileNameArr, uint32_t* lineNumbersArr);
    char* GetFuncOrFileNameStr(CString originName);
    int GetRealStackSize() { return realStackSize; }
    int GetFilledStackSize() { return filledStackSize; }

private:
    uint32_t maxStrSize;
    int filledStackSize = 0;
    int realStackSize = -1;
    int stackSkipThreshold = 2;
};

extern "C" MRT_EXPORT int CODE_MCC_InitCODEthreadStackInfo(uint32_t maxStrSize, void *mut, uint32_t* framePcArr,
                            char** funcNameArr, char** fileNameArr, uint32_t* lineNumberArr);
int InitCODEThreadStackInfoFromCurrFunc(uint32_t maxStrSize, uint32_t* framePcArr, char** funcNameArr,
                                      char** fileNameArr, uint32_t* lineNumberArr);

} // namespace MapleRuntime
#endif // MRT_GC_STACKINFO_H
