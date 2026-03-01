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


#include "PrintStackInfo.h"

#include "Common/StackType.h"

namespace MapleRuntime {
void PrintStackInfo::FillInStackTrace()
{
    UnwindContext uwContext;
    // Top unwind context can only be runtime or Codira context.
    CheckTopUnwindContextAndInit(uwContext);
    while (!uwContext.frameInfo.mFrame.IsAnchorFrame(anchorFA)) {
        AnalyseAndSetFrameType(uwContext);
        if (uwContext.frameInfo.GetFrameType() == FrameType::MANAGED) {
            stack.emplace_back(uwContext.frameInfo);
        }

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

void PrintStackInfo::PrintStackTrace() const
{
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    size_t frameSize = stack.size();
    for (size_t i = 0; i < frameSize; ++i) {
        stack[i].PrintFrameInfo(i);
    }
#endif
}

#if defined(__IOS__)
CString PrintStackInfo::GetStackTraceString()
{
    UnwindContext uwContext;
    // Top unwind context can only be runtime or Codira context.
    CheckTopUnwindContextAndInit(uwContext);
    while (!uwContext.frameInfo.mFrame.IsAnchorFrame(anchorFA)) {
        AnalyseAndSetFrameType(uwContext);
        stack.emplace_back(uwContext.frameInfo);

        UnwindContext caller;
        lastFrameType = uwContext.frameInfo.GetFrameType();
        if (uwContext.UnwindToCallerContext(caller) == false) {
            break;
        }
        uwContext = caller;
    }

    CString res;
    size_t frameSize = stack.size();
    for (size_t i = 0; i < frameSize; ++i) {
        res.Append(stack[i].GetFrameInfo(i));
    }
    return res;
}
#endif
} // namespace MapleRuntime
