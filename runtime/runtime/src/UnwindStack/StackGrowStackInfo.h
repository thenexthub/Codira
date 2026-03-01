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


#ifndef MRT_SE_STACKINFO_H
#define MRT_SE_STACKINFO_H

#include "Base/LogFile.h"
#include "StackInfo.h"
#include "StackMap/StackMapTypeDef.h"

namespace MapleRuntime {
class StackGrowStackInfo : public StackInfo {
public:
    explicit StackGrowStackInfo(const UnwindContext* context = nullptr) : StackInfo(context)
    {
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
        DLOG(UNWIND, "SE Stack Info");
        DLOG(UNWIND, "TopContext : %x", context);
        if (context != nullptr) {
            DLOG(UNWIND, "Top Context ip : %x Top Context fa : %x", context->frameInfo.mFrame.GetIP(),
                 context->frameInfo.mFrame.GetFA());
        }
#endif
    }

    ~StackGrowStackInfo() override = default;
    void FillInStackTrace() override;
    void RecordStackPtrs(const StackPtrVisitor& traceAndFixPtrVisitor,
                         const StackPtrVisitor& fixPtrVisitor,
                         const DerivedPtrVisitor& derivedPtrVisitor,
                         Mutator& mutator);
    void RecordStackPtrsImpl(const StackPtrVisitor& traceAndFixPtrVisitor,
                             const StackPtrVisitor& fixPtrVisitor,
                             const DerivedPtrVisitor& derivedPtrVisitor,
                             RegSlotsMap& regSlotsMap,
                             const FrameInfo& frame, Mutator& mutator);
};
} // namespace MapleRuntime
#endif // MRT_SE_STACKINFO_H
