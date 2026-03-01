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


#ifndef MRT_STACK_MANAGER_H
#define MRT_STACK_MANAGER_H

#include <map>

#include "Common/StackType.h"
#include "Common/TypeDef.h"
#include "StackMap/StackMap.h"
#include "UnwindStack/StackInfo.h"

namespace MapleRuntime {
class StackManager {
public:
    StackManager();
    ~StackManager() = default;

    // runtime required lifecycle interfaces
    void Init();
    void Fini() const;

#ifdef __arm__
    static uint32_t rtStartAddr;
    static uint32_t rtEndAddr;
    static uint32_t coreSoStartAddr;
    static uint32_t coreSoEndAddr;
    static uint32_t codecSoStartAddr;
    static uint32_t codecSoEndAddr;
    static uint32_t traceSoStartAddr;
    static uint32_t traceSoEndAddr;
#else
    static uint64_t rtStartAddr;
    static uint64_t rtEndAddr;
    static uint64_t coreSoStartAddr;
    static uint64_t coreSoEndAddr;
    static uint64_t codecSoStartAddr;
    static uint64_t codecSoEndAddr;
    static uint64_t traceSoStartAddr;
    static uint64_t traceSoEndAddr;
#endif

    // debug interface for runtime to get all stack trace.
    static void PrintStackTrace(UnwindContext* unContext = nullptr);

    static void PrintSignalStackTrace(UnwindContext* uwContext, uintptr_t pc, uintptr_t fa);

    static void PrintStackTraceForCpuProfile(UnwindContext* unContext, unsigned long long int codeThreadId);

    static void RecordLiteFrameInfos(std::vector<uint64_t>& liteFrameInfos, size_t steps = STACK_UNWIND_STEP_MAX);

    static void GetStackTraceByLiteFrameInfos(const std::vector<uint64_t>& liteFrameInfos,
                                              std::vector<StackTraceElement>& stackTrace);

    static void GetStackTraceByLiteFrameInfo(const uint64_t ip, const uint64_t pc, const uint64_t funcDesc,
                                             StackTraceElement& ste);

    // visit GC roots of current managed thread for tracing GC.
    static void VisitStackRoots(const UnwindContext& topFrame, const RootVisitor& func, Mutator& mutator);
    static void VisitHeapReferencesOnStack(const UnwindContext& topFrame, const RootVisitor& rootVisitor,
                                           const DerivedPtrVisitor& derivedPtrVisitor, Mutator& mutator);

    static void VisitStackPtrMap(const UnwindContext& topFrame, const StackPtrVisitor& traceAndFixPtrVisitor,
                                 const StackPtrVisitor& fixPtrVisitor, const DerivedPtrVisitor& derivedPtrVisitor,
                                 Mutator& mutator);

    static void InitStackGrowConfig();
#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
    static std::vector<FrameInfo> GetCurrentStack(StackMode mode);
#endif

    static bool IsRuntimeFrame(Uptr pc);
    friend class MachineFrame;

private:
    // Initialize the so load address defined below.
    void InitAddressScope();
};

void InitAddressScopeForCODEthreadTrace();

} // namespace MapleRuntime

#endif // MRT_STACK_MANAGER_H
