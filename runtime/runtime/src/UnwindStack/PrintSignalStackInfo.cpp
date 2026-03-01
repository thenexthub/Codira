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


#include "PrintSignalStackInfo.h"

#include "Common/StackType.h"

namespace MapleRuntime {
void PrintSignalStackInfo::FillInStackTrace()
{
    UnwindContext uwContext;
    // Top unwind context can only be runtime or Codira context.
    CheckTopUnwindContextAndInit(uwContext);
    while (!uwContext.frameInfo.mFrame.IsAnchorFrame(anchorFA)) {
        AnalyseAndSetFrameType(uwContext);
        if (uwContext.frameInfo.GetFrameType() == FrameType::C2N_STUB && lastFrameType == FrameType::N2C_STUB) {
            // Check whether out-of-bounds array occurs. If the value is about to exceed the limit,
            // increase the value of Index by 1 to print prompt information using comments.
            if (stackIndex >= MAX_SIGNAL_STACK_SIZE) {
                stackIndex++;
                break;
            }
            signalStack[stackIndex++] = SigHandlerFrameinfo(MachineFrame(), FrameType::NATIVE);
        }
        if (stackIndex >= MAX_SIGNAL_STACK_SIZE) {
            stackIndex++;
            break;
        }
        signalStack[stackIndex++] = uwContext.frameInfo;

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

void PrintSignalStackInfo::PrintStackTrace() const
{
    uint16_t stackSize;
    bool longStackFlag = false;

    if (stackIndex < MAX_SIGNAL_STACK_SIZE) {
        stackSize = stackIndex;
    } else {
        stackSize = MAX_SIGNAL_STACK_SIZE;
        longStackFlag = true;
    }
    for (size_t i = 0; i < stackSize; ++i) {
        signalStack[i].PrintFrameInfo(i);
    }
    if (longStackFlag) {
        FLOG(RTLOG_ERROR, "      ...");
    }
}
} // namespace MapleRuntime
