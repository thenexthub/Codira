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


#include <cerrno>
#include "codethread.h"
#include "thread.h"
#include "processor.h"
#include "schedule_impl.h"
#if defined(CODIRA_ASAN_SUPPORT)
#include "Sanitizer/SanitizerInterface.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

void SyscallEnter(void)
{
    struct CODEThread *codethread;
    struct Processor *processor;
    struct Thread *thread;
    codethread = CODEThreadGet();
    if (codethread == nullptr) {
        return;
    }
    ScheduleTraceEvent(TRACE_EV_CODETHREAD_SYSCALL, TRACE_STACK_10, nullptr, 0);
    thread = static_cast<struct Thread *>(codethread->thread);
    processor = static_cast<struct Processor *>(thread->processor);
    atomic_store(&codethread->state, CODETHREAD_SYSCALL);
    atomic_store(&processor->state, PROCESSOR_SYSCALL);
    processor->thread = nullptr;
    thread->oldProcessor = processor;
    thread->processor = nullptr;
}

bool SyscallFastExit(struct Processor *oldProcessor)
{
    struct CODEThread *codethread;
    struct Thread *thread;
    ProcessorState expected = PROCESSOR_SYSCALL;
    codethread = CODEThreadGet();
    thread = static_cast<struct Thread *>(codethread->thread);
    // If the status of the processor remains unchanged, rebind it.
    if (oldProcessor != nullptr && oldProcessor->state == PROCESSOR_SYSCALL &&
        atomic_compare_exchange_strong(&oldProcessor->state, &expected, PROCESSOR_RUNNING)) {
        thread->processor = reinterpret_cast<void *>(oldProcessor);
        oldProcessor->thread = thread;
        ScheduleTraceEvent(TRACE_EV_CODETHREAD_SYSEXIT, -1, nullptr, 1,
                           CODEThreadGetId(static_cast<CODEThreadHandle>(codethread)));
        ScheduleTraceEvent(TRACE_EV_CODETHREAD_START, -1, nullptr, 1,
                           CODEThreadGetId(static_cast<CODEThreadHandle>(codethread)));
        return true;
    }
    return false;
}

void *SyscallExit0(struct CODEThread *codethread)
{
    struct Thread *thread;
    struct Processor *processor;
    thread = static_cast<struct Thread *>(CODEThreadGet()->thread);
#ifdef CODIRA_ASAN_SUPPORT
    MapleRuntime::Sanitizer::AsanEndSwitchThreadContext(CODEThreadGet());
#endif
    atomic_store(&codethread->state, CODETHREAD_READY);
    processor = ProcessorAlloc(codethread->schedule, nullptr);
    // If there are no idle processors, join the global running queue.
    if (processor == nullptr) {
        ScheduleTraceEvent(TRACE_EV_CODETHREAD_SYSEXIT, -1, nullptr, 1,
                           CODEThreadGetId(static_cast<CODEThreadHandle>(codethread)));
        thread->codethread = nullptr;
        codethread->thread = nullptr;
        ScheduleGlobalWrite(&codethread, 1);
        ThreadStop(codethread->schedule);
    } else {
        thread->processor = static_cast<void *>(processor);
        processor->thread = thread;
        ProcessorLocalWrite(codethread);
        ScheduleTraceEvent(TRACE_EV_CODETHREAD_SYSEXIT, -1, nullptr, 1,
                           CODEThreadGetId(static_cast<CODEThreadHandle>(codethread)));
        ScheduleTraceEvent(TRACE_EV_CODETHREAD_START, -1, nullptr, 1,
                           CODEThreadGetId(static_cast<CODEThreadHandle>(codethread)));
    }
    ProcessorSchedule();
    return nullptr;
}

void SyscallExit(void)
{
    struct CODEThread *codethread;
    struct Processor *oldProcessor;
    struct Thread *thread;
    codethread = CODEThreadGet();
    if (codethread == nullptr) {
        return;
    }
    thread = static_cast<struct Thread *>(codethread->thread);
    oldProcessor = static_cast<struct Processor *>(thread->oldProcessor);
    // Quick route: The previously bound processor is still in syscall state.
    if (SyscallFastExit(oldProcessor)) {
        atomic_store(&codethread->state, CODETHREAD_RUNNING);
        return;
    }
#ifdef CODIRA_ASAN_SUPPORT
    MapleRuntime::Sanitizer::AsanStartSwitchThreadContext(codethread, ThreadGet()->codethread0);
#endif
    CODEThreadMcall(reinterpret_cast<void *>(SyscallExit0), CODEThreadAddr());
}

#ifdef __cplusplus
}
#endif
