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


#ifndef MRT_THREAD_H
#define MRT_THREAD_H

#include <pthread.h>
#include <semaphore.h>
#include <atomic>
#include <stdint.h>
#include <unistd.h>
#include "base.h"
#include "securec.h"
#include "list.h"
#include "codethread_context.h"

#ifdef MRT_WINDOWS
#include <winsock2.h>
#include <windows.h>
#else
#include <sys/syscall.h>
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* thread name size */
#define PTHREAD_NAME_SIZE             128

/**
 * @brief thread state
 */
enum ThreadState {
    THREAD_INIT,
    THREAD_CLOSE,
    THREAD_RUNNING,
    THREAD_PRE_SLEEP,
    THREAD_SLEEP,
};


/**
 * @brief thread structure
 */
struct Thread {
    struct Dulink link2schd;                /* free thread queue linked to scheduler */
    void *codethread;                         /* The current codethread. Note that the offset of
                                             * this field in the structure is used in the
                                             * codethread_context assembly. Therefore, do not
                                             * change the offset. */
    void *codethread0;                        /* codethread0 of thread. Note that the offset of
                                             * this field in the structure is used in the
                                             * codethread_context assembly. Therefore, do not
                                             * change the offset. */
    uintptr_t *preemptFlag;                 /* preemption flag */
    uintptr_t *preemptRequest;              /* preemption request */
    std::atomic<ThreadState> state;         /* thread state */
    void *processor;                        /* current processor of this thread */
    void *oldProcessor;                     /* previous bound processor */
    struct Semaphore sem;                   /* used to sleep this thread */
#ifdef MRT_MACOS
    unsigned long long tid;                 /* thread id */
#else
    pid_t tid;                              /* thread id */
#endif
    pthread_t osThread;                     /* pthread id */
    struct CODEThreadContext context;         /* Context when a thread enters the codethread,
                                             * which is used to the exit of scheduler. */
    bool isSearching;                       /* indicates whether the thread is in search */
    struct CODEThread *boundCODEThread;         /* bound codethread */
    void *nextProcessor;                    /* next processor to be bound to the thread */
    struct Dulink allThreadDulink;          /* link to thread management queue of the scheduler */
};


#ifdef MRT_MACOS
MRT_INLINE static unsigned long long GetSystemThreadId(void)
{
    unsigned long long tid;
    pthread_threadid_np(NULL, &tid);
    return tid;
}
#elif defined (MRT_WINDOWS)
MRT_INLINE static pid_t GetSystemThreadId(void)
{
    return GetCurrentThreadId();
}
#elif defined (MRT_LINUX)
MRT_INLINE static pid_t GetSystemThreadId(void)
{
    return syscall(SYS_gettid);
}
#endif

MRT_INLINE static long GetSystemProcessorsNums(void)
{
    long nums;
#ifdef MRT_WINDOWS
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    nums = systemInfo.dwNumberOfProcessors;
#else
    nums = sysconf(_SC_NPROCESSORS_ONLN);
#endif
    return nums;
}

extern size_t g_pageSize;

__attribute__((constructor)) static void GetSystemPageSize(void)
{
#ifdef MRT_WINDOWS
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    g_pageSize = systemInfo.dwPageSize;
#else
    g_pageSize = sysconf(_SC_PAGE_SIZE);
#endif
    if (g_pageSize == (size_t)-1) {
        LOG_ERROR(errno, "page_size wrong. size is: %u", g_pageSize);
    }
}

/**
 * @brief Set the current thread to sleep state.
 * @param thread    [IN] Thread to enter the sleep state
 * @retval Error code. The value 0 indicates success.
 */
int ThreadSleep(struct Thread *thread);

/**
 * @brief Create an idle thread.
 * @par Description: Create an idle thread, including the control block of the allocated
 * thread and the idle codethread0 control block, initialize related fields,
 * and create a physical thread.
 * @param schedule    [IN] Scheduler to which the thread belongs
 * @retval Pointer to the newly created thread.
 */
struct Thread *ThreadCreate(void *schedule);

/**
 * @brief Bind the thread to codethread0.
 * @par Description: Bind the thread to codethread0 and switch the context to codethread0.
 * @param arg [IN] Pointer to the thread to be bound to codethread0
 */
void *ThreadEntry(void *arg);

/**
 * @brief Stop the current thread.
 * @par Description: Stop the current thread and place the thread in the idle linked list.
 * @param handle     [IN] Put idle threads into the idle linked list of the scheduler.
 * @retval Error code. The value 0 indicates success.
 */
int ThreadStop(ScheduleHandle handle);

/**
 * @brief Allocate a thread and bind it to the specified processor.
 * @par Description: Allocate an idle thread, bind it to the processor in the parameter,
 * and start the thread.
 * @param processor    [IN] Processor to be bound to the new thread
 * @param searching    [IN] Whether to search
 * @retval 0 indicates success, and - 1 indicates failure.
 */
void ThreadAllocBindProcessor(void *processor, bool searching);

/**
 * @brief Initialize the preemption flag of the thread.
 */
void ThreadPreemptFlagInit(void);

/**
 * @brief Bind the processor to the current thread.
 * @param processor    [IN] Specify a processor for binding.
 * @retval Processor that is actually allocated
 */
void *ThreadBindProcessor(void *processor);

/**
 * @brief The non-default scheduler checks whether a codethread exists for the last time.
 * If no codethread exists, the thread is suspended.
 * @retval If successful, 0 is returned. If fails, an error code is returned.
 */
int ThreadWaitWithLastCheck(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_THREAD_H */
