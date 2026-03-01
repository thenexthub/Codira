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


#include "StackGrowStackInfo.h"

#include <stack>

#include "Collector/TracingCollector.h"
#include "Common/StackType.h"

namespace MapleRuntime {
void StackGrowStackInfo::FillInStackTrace()
{
    Mutator* mutator = Mutator::GetMutator();
    if (mutator == nullptr) {
        return;
    }
    UnwindContext uwContext;
    // Top unwind context can only be runtime or Codira context.
    CheckTopUnwindContextAndInit(uwContext);

#ifdef _WIN64
    // The offset on the window is unchanged.
    const int firstFrameBaseOffset = 48;
    while (reinterpret_cast<intptr_t>(uwContext.frameInfo.mFrame.GetFA()) !=
           mutator->GetStackBaseAddr() - firstFrameBaseOffset) {
#else
    // [rbp] of CODE_CODEThreadEntry is 0x0.
    while (reinterpret_cast<intptr_t>(uwContext.frameInfo.mFrame.GetFA()) != 0) {
#endif
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

void StackGrowStackInfo::RecordStackPtrsImpl(const StackPtrVisitor& traceAndFixPtrVisitor,
                                             const StackPtrVisitor& fixPtrVisitor,
                                             const DerivedPtrVisitor& derivedPtrVisitor,
                                             RegSlotsMap& regSlotsMap,
                                             const FrameInfo& frame, Mutator& mutator)
{
    uintptr_t startIP = reinterpret_cast<uintptr_t>(frame.GetStartProc());
    uintptr_t frameIP = reinterpret_cast<uintptr_t>(frame.mFrame.GetIP());
    uintptr_t frameAddress = reinterpret_cast<uintptr_t>(frame.mFrame.GetFA());
    StackPtrMap stackPtrMap = StackMapBuilder(startIP, frameIP, frameAddress).Build<StackPtrMap>();
    if (stackPtrMap.IsValid()) {
        if (!stackPtrMap.VisitReg(traceAndFixPtrVisitor, fixPtrVisitor, nullptr, regSlotsMap)) {
            LOG(RTLOG_FATAL, "wrong reg info, start ip: %p frame pc: %p", reinterpret_cast<void*>(startIP),
                reinterpret_cast<void*>(frameIP));
        }
        stackPtrMap.VisitSlot(traceAndFixPtrVisitor, fixPtrVisitor, nullptr);
        stackPtrMap.VisitDerivedPtr(derivedPtrVisitor, regSlotsMap);
    }
    stackPtrMap.RecordCalleeSaved(regSlotsMap);
}

void StackGrowStackInfo::RecordStackPtrs(const StackPtrVisitor& traceAndFixPtrVisitor,
                                         const StackPtrVisitor& fixPtrVisitor,
                                         const DerivedPtrVisitor& derivedPtrVisitor, Mutator& mutator)
{
    RegSlotsMap regSlotsMap;
    for (auto frame : stack) {
        ObjectRef* rbp = reinterpret_cast<ObjectRef*>(frame.GetMachineFrame().GetFA());
        fixPtrVisitor(*rbp);

        switch (frame.GetFrameType()) {
            case FrameType::MANAGED: {
                RecordStackPtrsImpl(traceAndFixPtrVisitor, fixPtrVisitor, derivedPtrVisitor,
                                    regSlotsMap, frame, mutator);
                break;
            }
            case FrameType::STACKGROW:
                RegRoot::RecordRegs(regSlotsMap, reinterpret_cast<Uptr>(frame.mFrame.GetFA()));
                break;
            default: {
                break;
            }
        }
    }
}
} // namespace MapleRuntime
