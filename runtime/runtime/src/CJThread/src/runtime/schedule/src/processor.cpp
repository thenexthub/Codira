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


#include <cstdio>
#include <sched.h>
#include <unistd.h>
#include <ctime>
#include "schedule_impl.h"
#include "securec.h"
#include "schdpoll.h"
#include "basetime.h"
#include "log.h"
#if defined(CODIRA_SANITIZER_SUPPORT)
#include "Sanitizer/SanitizerInterface.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ProcessorThreadExit sleep_time */
const int PROCESSOR_RELEASE_SLEEP_TIME = 10;
/* undefault schedule processor_codethread_get round number */
const int PROCESSOR_CODETHREAD_GET_ROUND_NUM = 20;

unsigned int g_randSeed = 0;

int ProcessorGlobalWrite(struct CODEThread *codethreadList[], unsigned int num)
{
    struct Dulink tempDulink;
    struct Schedule *sch;
    unsigned long length;
    unsigned long i;
    void *buf[PROCESSOR_QUEUE_CAPACITY] = {nullptr};
    struct Processor *processor;

    processor = ProcessorGet();
    sch = static_cast<struct Schedule *>(processor->schedule);
    DulinkInit(&tempDulink);

    // Get 1/4 from the local queue and put it into the global queue.
    length = QueueLength(&processor->runq) / GLOBAL_ADD_RATIO;
    if (length != 0) {
        length = QueuePopHeadBatch(&processor->runq, buf, length);
    }

    // To reduce the occupation time of schdCODEThread.mutex, use tempDulink to temporarily
    // store the file, and then move the file to runq.
    for (i = 0; i < length; i++) {
        DulinkPushtail(&tempDulink, buf[i]);
    }

    for (i = 0; i < num; i++) {
        DulinkPushtail(&tempDulink, codethreadList[i]);
    }

    // Add to the global queue of the schedule.
    pthread_mutex_lock(&sch->schdCODEThread.mutex);
    DulinkMove(&sch->schdCODEThread.runq, &tempDulink, 0);
    sch->schdCODEThread.num += (length + num);
    pthread_mutex_unlock(&sch->schdCODEThread.mutex);

    return 0;
}

/* Add the codethread to the processor's run queue. */
int ProcessorLocalWriteBatch(struct CODEThread **codethread, unsigned int num)
{
    int ret;
    struct Queue *runq;
    unsigned int pushNum;
    struct Processor *processor;

    processor = ProcessorGet();
    runq = &processor->runq;
    pushNum = QueuePushTailBatch(runq, reinterpret_cast<void **>(codethread), num);
    if (pushNum == num) {
        return 0;
    }

    // If add to local queue fail, add to global queue
    ret = ProcessorGlobalWrite(codethread + pushNum, num - pushNum);
    if (ret) {
        LOG_ERROR(ret, "write global queue failed!");
        return ret;
    }

    return 0;
}

/* Bulk fetching schedulable codethreads from the global queue */
struct CODEThread *ProcessorGlobalRead(void *schedule, bool batch)
{
    struct ScheduleCODEThread *schdCODEThread;
    struct Dulink *runDulink;
    struct Dulink tempDulink;
    struct CODEThread *codethreadTemp;
    struct CODEThread *codethreadNext = nullptr;
    void *buf[PROCESSOR_QUEUE_CAPACITY];
    unsigned int i;
    unsigned int readNum = 1;
    struct Processor *processor;
    unsigned int processorNum = ((struct Schedule *)schedule)->schdProcessor.processorNum;

    schdCODEThread = &((struct Schedule *)schedule)->schdCODEThread;
    if (schdCODEThread->num == 0) {
        return nullptr;
    }
    runDulink = &schdCODEThread->runq;
    pthread_mutex_lock(&(schdCODEThread->mutex));

    if (schdCODEThread->num == 0) {
        pthread_mutex_unlock(&(schdCODEThread->mutex));
        return nullptr;
    }

    if (batch) {
        // The quantity is the number of processes in the global queue divided by the number
        // of processors. The value must be at least 1.
        readNum = (schdCODEThread->num + processorNum - 1) / processorNum;
        if (readNum > PROCESSOR_QUEUE_CAPACITY) {
            readNum = PROCESSOR_QUEUE_CAPACITY;
        }
    }
    schdCODEThread->num -= readNum;

    // Get one first
    codethreadNext = DULINK_ENTRY(runDulink->next, struct CODEThread, schdDulink);
    DulinkRemove(&(codethreadNext->schdDulink));

    if (readNum == 1) {
        pthread_mutex_unlock(&(schdCODEThread->mutex));
        return codethreadNext;
    }
    --readNum;
    DulinkInit(&tempDulink);
    DulinkMove(&tempDulink, runDulink, readNum);
    pthread_mutex_unlock(&(schdCODEThread->mutex));

    // The rest of the codethreads are placed in the local queue. If batch is false or only
    // one codethread is obtained, the loop will not be entered.
    processor = ProcessorGet();
    for (i = 0; i < readNum; i++) {
        codethreadTemp = DULINK_ENTRY(tempDulink.next, struct CODEThread, schdDulink);
        DulinkRemove(&(codethreadTemp->schdDulink));
        buf[i] = (void *)codethreadTemp;
    }

    QueuePushTailBatch(&processor->runq, buf, readNum);

    return codethreadNext;
}

/* Add a single codethread to the local queue */
int ProcessorLocalWrite(struct CODEThread *codethread, bool isReschd)
{
    int error;
    struct Queue *runq;
    struct Processor *processor;

    processor = ProcessorGet();
    runq = &processor->runq;

    while (true) {
        if (isReschd) {
            error = QueuePushTail(runq, codethread);
            if (error == 0) {
                return 0;
            }
            error = ProcessorGlobalWrite(&codethread, 1);
            if (error) {
                LOG_ERROR(error, "write global queue failed!");
            }
            return error;
        }

        struct CODEThread *obj = atomic_load_explicit(&processor->codethreadNext, std::memory_order_relaxed);
        if (atomic_compare_exchange_weak(&processor->codethreadNext, &obj, codethread)) {
            if (obj == nullptr) {
                return 0;
            }
            error = QueuePushTail(runq, obj);
            if (error == 0) {
                return 0;
            }
            // If add codethread to local queue fail, add it to global queue
            error = ProcessorGlobalWrite(&obj, 1);
            if (error) {
                LOG_ERROR(error, "write global queue failed!");
            }
            return error;
        }
    }
    return 0;
}

MRT_STATIC_INLINE unsigned long ProcessorQueueSteal(struct Processor *processor,
                                                    void *buf[], unsigned long bufLen)
{
    struct Queue *stealRunq;
    unsigned long length;

    stealRunq = &processor->runq;
    length = (QueueLength(stealRunq) + PROCESSOR_STEAL_RATIO - 1) / PROCESSOR_STEAL_RATIO;
    if (length == 0) {
        return 0;
    }
    if (length > bufLen) {
        length = bufLen;
    }

    return QueuePopHeadBatch(stealRunq, buf, length);
}

struct CODEThread *ProcessorCODEThreadSteal(struct Processor *localProcessor, struct Processor *stealProcessor)
{
    unsigned long stealNum;
    unsigned long pushNum;
    void *buf[PROCESSOR_QUEUE_CAPACITY] = {nullptr};
    struct Queue *localRunq;

    // Stealing codethreads from other processor queues.
    stealNum = ProcessorQueueSteal(stealProcessor, buf, PROCESSOR_QUEUE_CAPACITY);
    if (stealNum == 0) {
        return nullptr;
    }

    // Put the stolen codethread into the local queue.
    localRunq = &localProcessor->runq;
    pushNum = QueuePushTailBatch(localRunq, buf + 1, stealNum - 1);
    if (pushNum != stealNum - 1) {
        LOG_ERROR(ERRNO_SCHD_LOCAL_QUEUE_PUSH_FAILED,
                  "push local queue failed! steal_num: %u, push_num: %u",
                  stealNum, pushNum);
        return nullptr;
    }

    // Return the local codethread to be run.
    return (struct CODEThread *)buf[0];
}

/* Releases processor resources. In the exit process of the scheduling framework, this
 * function may be invoked twice in the same processor. Therefore, the system checks whether
 * the current processor is NULL first. */
int ProcessorRelease(void)
{
    struct Processor *curProcessor;
    struct Thread *curThread;
    struct Schedule *schedule;

    curProcessor = ProcessorGet();
    if (curProcessor == nullptr) {
        return ERRNO_SCHD_PROCESSOR_STATE_INVALID;
    }

    schedule = static_cast<struct Schedule *>(curProcessor->schedule);

    if (g_scheduleManager.trace.openType && (g_scheduleManager.trace.openType & TRACE_EV_PROC_STOP)) {
        ScheduleTraceEventOrigin(TRACE_EV_PROC_STOP, -1, nullptr, 0);
    }
    // Unbind the processor from the thread.
    curThread = curProcessor->thread;

    curProcessor->thread = nullptr;
    curThread->processor = nullptr;

    // Set the status of the processor and increases the number of idle processors.
    if (schedule->state == SCHEDULE_EXITING) {
        atomic_store(&curProcessor->state, PROCESSOR_EXITING);
    } else {
        atomic_store(&curProcessor->state, PROCESSOR_IDLE);
        atomic_fetch_add(&schedule->schdProcessor.freeNum, 1u);
    }
    return 0;
}

/* check again before stop the processor */
int ProcessorStopWithLastCheck(void)
{
    int ret;
    struct Processor *oldProcessor = ProcessorGet();
    struct Processor *newProcessor;
    struct Schedule *schedule = static_cast<struct Schedule *>(oldProcessor->schedule);

    // release current processor
    ret = ProcessorRelease();
    if (ret != 0) {
        LOG_ERROR(ret, "process release failed!");
        return ret;
    }
    // Note: The processor has been released and may be immediately woken up and occupied
    // by other threads. After exiting the search, need to perform a check, because new
    // codethreads may be generated here. If we don't check here, there will be two situations
    // where the codethread cannot be executed:
    // 1. ProcessorWake in CODEThreadReady does not perform ProcessorWake because it detects
    // that searching_num is greater than or equal to 1, and at this time, the search thread
    // just exits search
    // 2. ProcessorWake in CODEThreadReady does not perform ProcessorWake because it detects
    // that the number of idle processors is 0, while all processors are currently undergoing
    // ProcessorRelease
    if (ScheduleAnyCODEThread(schedule)) {
        // get a new processor
        newProcessor = static_cast<struct Processor *>(ThreadBindProcessor(static_cast<void *>(oldProcessor)));
        if (newProcessor != nullptr) {
            // Enter the search state again and continue to search for tasks.
            newProcessor->thread->isSearching = true;
            atomic_fetch_add(&schedule->schdThread.searchingNum, 1u);
            if (g_scheduleManager.trace.openType && (g_scheduleManager.trace.openType & TRACE_EV_PROC_WAKE)) {
                ScheduleTraceEventOrigin(TRACE_EV_PROC_WAKE, -1, nullptr, 1,
                                         static_cast<unsigned long long>(ThreadGet()->tid));
            }
            return 0;
        }
    }

    // Release thread-related resources again. The thread has been unbound from the processor.
    // ScheduleGet cannot obtain the current schedule. Parameters need to be transferred.
    ret = ThreadStop(schedule);
    if (ret != 0) {
        LOG_ERROR(ret, "thread stop failed!");
        return ret;
    }

    return 0;
}

void RandSeedInit(void)
{
    g_randSeed = CurrentNanotimeGet();
}

MRT_STATIC_INLINE unsigned int RandomPseudo(unsigned int range)
{
    if (range == 0) {
        return 0;
    }
#ifdef MRT_WINDOWS
    errno_t ret;
    unsigned int number;
    ret = rand_s(&number);
    if (ret != 0) {
        return 0;
    }
    return number % range;
#else
    return static_cast<unsigned int>(rand_r(&g_randSeed)) % range;
#endif
}

/* steal timer */
MRT_STATIC_INLINE struct CODEThread *ProcessorTimerSteal(struct Processor *stealProcessor,
                                                       unsigned long long now)
{
    bool run = false;
    struct CODEThread *stealCODEThread;
    ProcessorCheckFunc checkFunc;
    Processor *curProcessor;

    checkFunc = g_scheduleManager.check[PROCESSOR_TIMER_HOOK];
    if (checkFunc == nullptr) {
        return nullptr;
    }
    checkFunc(stealProcessor, &now, &run);

    if (run) {
        curProcessor = ProcessorGet();
        stealCODEThread = ProcessorCODEhreadNextRead(curProcessor);
        if (stealCODEThread != nullptr) {
            return stealCODEThread;
        }
        // check_func is timer_trigger. Here may generate new codethread
        stealCODEThread = ProcessorLocalRead(curProcessor);
        if (stealCODEThread != nullptr) {
            return stealCODEThread;
        }
    }

    return nullptr;
}

struct CODEThread *ProcessorSteal(unsigned long long now)
{
    unsigned long stealNum;
    unsigned long  processorId;
    struct Processor *localProcessor = ProcessorGet();
    struct Schedule *schedule = static_cast<struct Schedule *>(localProcessor->schedule);
    struct Processor *stealProcessor;
    struct CODEThread *stealCODEThread;
    struct ScheduleProcessor *schdProcessor = &schedule->schdProcessor;
    unsigned long processorNum = schdProcessor->processorNum;
    bool stealTimersOrRunNext;
    processorId = RandomPseudo(processorNum);
    for (unsigned int i = 0; i < PROCESSOR_STEAL_ROUNDS; ++i) {
        stealTimersOrRunNext = (i == (PROCESSOR_STEAL_ROUNDS - 1));
        for (stealNum = 0; stealNum < processorNum; stealNum++) {
            // get steal processor
            stealProcessor = &schdProcessor->processorGroup[((stealNum + processorId) % processorNum)];
            if (stealProcessor == localProcessor) {
                continue;
            }

            // Trying to steal codethreads from other processors
            stealCODEThread = ProcessorCODEThreadSteal(localProcessor, stealProcessor);
            if (stealCODEThread != nullptr) {
                return stealCODEThread;
            }

            // Determine whether to steal the timer or nextCODEthread
            if (!stealTimersOrRunNext) {
                continue;
            }
            stealCODEThread = ProcessorTimerSteal(stealProcessor, now);
            if (stealCODEThread != nullptr) {
                return stealCODEThread;
            }
            stealCODEThread = ProcessorCODEhreadNextRead(stealProcessor);
            if (stealCODEThread != nullptr) {
                return stealCODEThread;
            }
        }
        // In multi-thread scenario, the competition caused by theft of processors need be reduced.
        // In single-thread scenario, avoid sleep to improve read and write performance.
        if (schedule->schdThread.searchingNum.load(std::memory_order_relaxed) > PROCESSOR_STEAL_SLEEP_THRESHOLD) {
            usleep(1);
        }
    }
    return nullptr;
}

/* Stealing schedulable codethreads from other processors */
struct CODEThread *ProcessorSearchingSteal(unsigned long long now)
{
    struct CODEThread *stealCODEThread;
    struct Schedule *schedule = ScheduleGet();
    struct Thread *thread = ThreadGet();
    struct ScheduleProcessor *schdProcessor = &schedule->schdProcessor;
    unsigned long processorNum;

    // The total number of search threads is less than or equal to half of the number of
    // running processors. However, if the current thread is assigned the search state,
    // the system directly enters the search process regardless of the number of threads.
    processorNum = schdProcessor->processorNum;
    if (thread->isSearching == false &&
        RUNNING_PROCESSOR_SEARCHING_NUM_MULTIPLE * schedule->schdThread.searchingNum
        > processorNum - schdProcessor->freeNum) {
        return nullptr;
    }

    // enter searching
    if (!thread->isSearching) {
        thread->isSearching = true;
        atomic_fetch_add(&schedule->schdThread.searchingNum, 1u);
    }

    stealCODEThread = ProcessorSteal(now);

    // No matter whether it is stolen or not, you need to exit the search.
    thread->isSearching = false;
    atomic_fetch_sub(&schedule->schdThread.searchingNum, 1u);
    // If a task is stolen, a new thread needs to be waked to ensure the concurrency.
    if (stealCODEThread != nullptr) {
        ProcessorWake(schedule, nullptr);
        return stealCODEThread;
    }

    return nullptr;
}

/* By default, the scheduler searches for other processors and steals codethread tasks.
 * Because other schedulers have only one processor, they can only obtain tasks from the
 * global queue. */
struct CODEThread *ProcessorSearchingGlobal(void)
{
    struct CODEThread *stealCODEThread;
    struct Schedule *schedule = ScheduleGet();
    struct Thread *thread = ThreadGet();

    if (!thread->isSearching) {
        thread->isSearching = true;
        atomic_fetch_add(&schedule->schdThread.searchingNum, 1u);
    }
    for (int i = 0; i < PROCESSOR_CODETHREAD_GET_ROUND_NUM; ++i) {
        stealCODEThread = ProcessorGlobalRead(schedule, true);
        if (stealCODEThread != nullptr) {
            break;
        }
    }
    thread->isSearching = false;
    atomic_fetch_sub(&schedule->schdThread.searchingNum, 1u);

    return stealCODEThread;
}

MRT_STATIC_INLINE unsigned long long ProcessorTimerCheck(void)
{
    ProcessorCheckFunc checkFunc;
    unsigned long long now = 0;

    checkFunc = g_scheduleManager.check[PROCESSOR_TIMER_HOOK];
    if (checkFunc != nullptr) {
        checkFunc(ProcessorGet(), &now, nullptr);
    }

    return now;
}

/* This function is similar to the wake function in ProcessorSearchingSteal. It exits the
 * search function and starts a new thread. However, the branch is different. Here is the
 * branch of the global queue and network IO, and the other is the branch of task theft. */
MRT_INLINE static void ProcessorSearchingMore(void)
{
    struct Schedule *schedule;
    struct Thread *thread;
    // If a task is obtained through search, a new thread needs to be started to execute the task.
    thread = ThreadGet();
    if (thread->isSearching) {
        thread->isSearching = false;
        schedule = ScheduleGet();
        atomic_fetch_sub(&schedule->schdThread.searchingNum, 1u);
        ProcessorWake(schedule, nullptr);
    }
}

/* Find the next codethread to be scheduled. */
struct CODEThread *ProcessorCODEThreadGet(void)
{
    int ret;
    int num;
    struct CODEThread *nextCODEThread = nullptr;
    struct Processor *curProcessor;
    struct Schedule *schedule;
    void *buf[SCHDPOLL_ACQUIRE_MAX_NUM];
    unsigned long long now;
    struct CODEThread *expected;

    do {
        // Note: The curProcessor may change in each loop. Therefore, the value must be
        // assigned in the loop and cannot be placed outside the loop.
        curProcessor = ProcessorGet();
        schedule = static_cast<struct Schedule *>(curProcessor->schedule);
        if (schedule->scheduleType != SCHEDULE_DEFAULT &&
            (atomic_load_explicit(&schedule->state, std::memory_order_relaxed) == SCHEDULE_EXITING ||
             atomic_load_explicit(&schedule->state, std::memory_order_relaxed) == SCHEDULE_EXITED)) {
            if (g_scheduleManager.trace.openType && (g_scheduleManager.trace.openType & TRACE_EV_PROC_STOP)) {
                ScheduleTraceEventOrigin(TRACE_EV_PROC_STOP, -1, nullptr, 0);
            }
            auto *thread = ThreadGet();
#ifdef CODIRA_ASAN_SUPPORT
            // return to ThreadEntry, switch to original thread
            MapleRuntime::Sanitizer::AsanExitCODEThread(thread);
#endif
            CODEThreadContextSet(&thread->context);
        }
        if (schedule->noWaitAttr.nowait && schedule->noWaitAttr.timeout != 0 &&
            (CurrentNanotimeGet() - schedule->noWaitAttr.startTime >= schedule->noWaitAttr.timeout)) {
            auto *thread = ThreadGet();
#ifdef CODIRA_ASAN_SUPPORT
            // return to ThreadEntry, switch to original thread
            MapleRuntime::Sanitizer::AsanExitCODEThread(thread);
#endif
            CODEThreadContextSet(&thread->context);
        }

        // check timer first
        now = ProcessorTimerCheck();

        if (g_scheduleManager.trace.openType || g_scheduleManager.trace.shutdown) {
            nextCODEThread = ScheduleGetTraceReader();
            if (nextCODEThread != nullptr) {
                ScheduleTraceEventOrigin(TRACE_EV_CODETHREAD_UNBLOCK, -1, nullptr, TraceArgNum::TRACE_ARGS_2,
                                         CODEThreadGetId(static_cast<CODEThreadHandle>(nextCODEThread)),
                                         SpecialStackId::UNKNOWN);
                break;
            }
        }

        // Obtain codethread to be scheduled from the global queue or local queue every 32 times.
        if ((curProcessor->schedCnt & (GLOBAL_SCH_NUM - 1)) == GLOBAL_SCH_NUM - 1) {
            nextCODEThread = ProcessorGlobalRead(schedule, false);
            if (nextCODEThread != nullptr) {
                break;
            }
            // Obtain the codethread to be scheduled from the local queue to prevent tasks in
            // the local queue from being executed due to continuous next invoking.
            nextCODEThread = reinterpret_cast<struct CODEThread *>(QueuePopHead(&curProcessor->runq));
            if (nextCODEThread != nullptr) {
                break;
            }
        }

        // Check the CODEthreadNext corresponding to the current processor. Do not need to execute
        // the cnt+1 because the processor does not need to be switched.
        nextCODEThread = ProcessorCODEhreadNextRead(curProcessor);
        if (nextCODEThread != nullptr) {
            ProcessorSearchingMore();
            return nextCODEThread;
        }

        // Attempt to get schedulable codethread from local queue.
        nextCODEThread = ProcessorLocalRead(curProcessor);
        if (nextCODEThread != nullptr) {
            break;
        }

        // Attempt to get the last codethread from the scheduler.
        expected = atomic_load_explicit(&schedule->lastCODEThread, std::memory_order_relaxed);
        if (expected && atomic_compare_exchange_strong(&schedule->lastCODEThread,
                                                       &expected, static_cast<struct CODEThread *>(nullptr))) {
            nextCODEThread = expected;
            break;
        }

        // Attempt to get schedulable codethread from global queue.
        nextCODEThread = ProcessorGlobalRead(schedule, true);
        if (nextCODEThread != nullptr) {
            break;
        }

        // Attempt to get ready events from netpoll. This interface may return a failure less
        // than zero. For example, fd is disabled when the scheduling framework exits.
        if (schedule->netpoll.npfd != nullptr) {
            num = SchdpollAcquire(schedule, buf, SCHDPOLL_ACQUIRE_MAX_NUM, 0);
            if (num > 0) {
                // After successfully fetching the codethread from netpoll, review the local
                // and global queues again.
                CODEThreadAddBatch(buf, num);
                continue;
            }
        }

        if (schedule->scheduleType == SCHEDULE_DEFAULT) {
            // Attempt to steal codethreads from other processors
            nextCODEThread = ProcessorSearchingSteal(now);
            if (nextCODEThread != nullptr) {
                break;
            }
            // If all previous attempts fail, only the current processor can be hibernated.
            // The last check is performed before hibernation.
            ret = ProcessorStopWithLastCheck();
            if (ret) {
                LOG_ERROR(-1, "ProcessorStopWithLastCheck failed");
            }
        } else {
            // Attempt to steal codethreads from global
            nextCODEThread = ProcessorSearchingGlobal();
            if (nextCODEThread != nullptr) {
                break;
            }
            ret = ThreadWaitWithLastCheck();
            if (ret) {
                LOG_ERROR(-1, "ThreadWaitWithLastCheck failed");
            }
        }
    } while (1);

    ProcessorSearchingMore();

    // Update the number of processor switchover times.
    curProcessor->schedCnt++;

    // nextCODEThread cannont be null.
    return nextCODEThread;
}

/* Switch out the bound thread and replace it with a common thread. */
void ProcessorStopBoundCODEThread(void)
{
    int error;
    struct Thread *thread;
    struct Processor *processor;
    struct Processor *nextProcessor;

    thread = ThreadGet();
    processor = static_cast<struct Processor *>(thread->processor);
    if (g_scheduleManager.trace.openType && (g_scheduleManager.trace.openType & TRACE_EV_PROC_STOP)) {
        ScheduleTraceEventOrigin(TRACE_EV_PROC_STOP, -1, nullptr, 0);
    }
    // The processor and thread are unbound from each other.
    processor->thread = nullptr;
    thread->processor = nullptr;

    // Bind a new thread for processor and start it.
    ThreadAllocBindProcessor(processor, false);

    // Sleep current thread
    error = ThreadSleep(thread);
    if (error != 0) {
        LOG_ERROR(error, "ThreadSleep failed");
        return;
    }
    // After the current thread wakes up, bind the processor.
    nextProcessor = static_cast<struct Processor *>(thread->nextProcessor);
    thread->processor = nextProcessor;
    nextProcessor->thread = thread;
    thread->nextProcessor = nullptr;
}

/* Switch to the thread bound to codethread and execute the binding codethread. */
void ProcessorStartBoundCODEThread(struct CODEThread *codethread)
{
    int error;
    struct Thread *thread;
    struct Thread *curThread = ThreadGet();
    struct Processor *processor = nullptr;
    struct Processor *curProcessor = ProcessorGet();
    struct Schedule *schedule = ScheduleGet();

    // When there is a free processor, it is used to execute the binding codethread.
    if (schedule->schdProcessor.freeNum != 0) {
        processor = ProcessorAlloc(schedule, nullptr);
    }
    // When there is no free processor, free the current processor to execute the
    // binding codethread.
    if (processor == nullptr) {
        processor = curProcessor;
        curThread->processor = nullptr;
        curThread->codethread = nullptr;
    }

    thread = codethread->boundThread;
    thread->nextProcessor = processor;
    // Wakes up the thread and executes the bound codethread.
    error = SemaphorePost(&(thread->sem));
    if (error != 0) {
        LOG_ERROR(error, "sem post failed");
    }
    // If the current processor is released, the current thread needs to be stopped and the
    // thread is put in the free list.
    if (processor == curProcessor) {
        error = ThreadStop(schedule);
        if (error != 0) {
            LOG_ERROR(error, "thread stop failed!");
        }
    }
}

void ProcessorSchedule(void)
{
    struct CODEThread *nextCODEThread;
    struct Schedule *schedule;
    struct Processor *processor;
    struct Thread *thread;
    do {
        processor = ProcessorGet();
        schedule = static_cast<struct Schedule *>(processor->schedule);
        thread = processor->thread;
        if (schedule->scheduleType == SCHEDULE_DEFAULT &&
            atomic_load_explicit(&schedule->state, std::memory_order_relaxed) == SCHEDULE_EXITING) {
            ProcessorThreadExit();
        }
        // Check whether the current thread has a bound codethread. If yes, it indicates that
        // the codethread has been suspended and the current thread needs to be suspended.
        nextCODEThread = thread->boundCODEThread;
        if (nextCODEThread != nullptr) {
            ProcessorStopBoundCODEThread();
            // Because the binding thread is switched, preemption flag needs to be reset.
            ProtectAddrSet((uintptr_t)nextCODEThread->stack.stackGuard);
            MapleRuntime::Mutator* mutator = nextCODEThread->mutator;
            MapleRuntime::ThreadLocalData* tlData = MapleRuntime::ThreadLocal::GetThreadLocalData();
            tlData->mutator = mutator;
            mutator->PreparedToRun(tlData);
            if (g_scheduleManager.trace.openType && (g_scheduleManager.trace.openType & TRACE_EV_CODETHREAD_START)) {
                ScheduleTraceEventOrigin(TRACE_EV_CODETHREAD_START, -1, nullptr, 1,
                                         CODEThreadGetId(static_cast<CODEThreadHandle>(nextCODEThread)));
            }
#ifdef CODIRA_ASAN_SUPPORT
            // target to next code thread, just switch
            MapleRuntime::Sanitizer::AsanStartSwitchThreadContext(CODEThreadGet(), nextCODEThread);
            MapleRuntime::Sanitizer::AsanEndSwitchThreadContext(nextCODEThread);
#endif
#ifdef __OHOS__
            TRACE_START_ASYNC(TRACE_CODETHREAD_EXEC, nextCODEThread->id);
#elif defined (__ANDROID__)
            TRACE_START(MapleRuntime::TraceInfoFormat(TRACE_CODETHREAD_EXEC, nextCODEThread->id));
#endif
#ifdef __OHOS__
            if (nextCODEThread->schedule->scheduleType == SCHEDULE_UI_THREAD) {
                SingleCODEThreadStoreSP();
            }
#endif
            CODEThreadExecute(nextCODEThread, (void**)&tlData->codethread);
            continue;
        }
        // Finds a dispatchable codethread and switches its state to RUNNING.
        nextCODEThread = ProcessorCODEThreadGet();
        atomic_store_explicit(&nextCODEThread->state, CODETHREAD_RUNNING, std::memory_order_relaxed);
        ProtectAddrSet((uintptr_t)nextCODEThread->stack.stackGuard);
        if (nextCODEThread->boundThread != nullptr) {
            ProcessorStartBoundCODEThread(nextCODEThread);
        } else {
            MapleRuntime::Mutator* mutator = nextCODEThread->mutator;
            MapleRuntime::ThreadLocalData* tlData = MapleRuntime::ThreadLocal::GetThreadLocalData();
            tlData->mutator = mutator;
            mutator->PreparedToRun(tlData);
            if (g_scheduleManager.trace.openType && (g_scheduleManager.trace.openType & TRACE_EV_CODETHREAD_START)) {
                ScheduleTraceEventOrigin(TRACE_EV_CODETHREAD_START, -1, nullptr, 1,
                                         CODEThreadGetId(static_cast<CODEThreadHandle>(nextCODEThread)));
            }
#ifdef CODIRA_ASAN_SUPPORT
            // target to next code thread, just switch
            MapleRuntime::Sanitizer::AsanStartSwitchThreadContext(CODEThreadGet(), nextCODEThread);
            MapleRuntime::Sanitizer::AsanEndSwitchThreadContext(nextCODEThread);
#endif
#ifdef __OHOS__
            TRACE_START_ASYNC(TRACE_CODETHREAD_EXEC, nextCODEThread->id);
#elif defined (__ANDROID__)
            TRACE_START(MapleRuntime::TraceInfoFormat(TRACE_CODETHREAD_EXEC, nextCODEThread->id));
#endif
#ifdef __OHOS__
            if (nextCODEThread->schedule->scheduleType == SCHEDULE_UI_THREAD) {
                SingleCODEThreadStoreSP();
            }
#endif
            // The execution context of ProcessorWake must be in codethread0. Do not need to
            // change the status of the current codethread and directly switch to nextCODEThread.
            CODEThreadExecute(nextCODEThread, (void**)&tlData->codethread);
            return;
        }
    } while (true);
}

/* Init a processor */
int ProcessorInit(void *schedule, struct Processor *processor, unsigned int processorId)
{
    memset_s(processor, sizeof(struct Processor),
             0, sizeof(struct Processor));
    processor->processorId = processorId;
    processor->state = PROCESSOR_IDLE;
    processor->schedule = schedule;
    std::atomic_store_explicit(&processor->codethreadNext, (CODEThread *)nullptr, std::memory_order_relaxed);

    // Init processor running queue
    QueueInit(&processor->runq, PROCESSOR_QUEUE_CAPACITY);

    // Init local codethread free list
    processor->freelist.codethreadNum = 0;
    DulinkInit(&processor->freelist.freeList);
    PthreadSpinInit(&processor->lock);

#if defined(CODIRA_TSAN_SUPPORT)
    MapleRuntime::Sanitizer::TsanNewRaceProc(processor);
#endif

    return 0;
}

/* Free a processor */
void ProcessorFree(struct Schedule *schedule, struct Processor *processor)
{
    struct ScheduleProcessor *schdProcessor;

    schdProcessor = &(schedule->schdProcessor);

    atomic_store(&processor->state, PROCESSOR_IDLE);
    atomic_fetch_add(&schdProcessor->freeNum, 1u);
}

void ProcessorNonDefaultScheduleWake(struct Schedule *schedule)
{
    ThreadState expectedThreadState = THREAD_SLEEP;

    while (atomic_load(&schedule->thread0->state) == THREAD_PRE_SLEEP) {}
    if (atomic_load(&schedule->thread0->state) == THREAD_SLEEP) {
        if (atomic_compare_exchange_strong(&schedule->thread0->state, &expectedThreadState, THREAD_RUNNING)) {
            int error = SemaphorePost(&(schedule->thread0->sem));
            if (error != 0) {
                LOG_ERROR(error, "sem post failed");
            }
        }
    }
}

/* Wakes up a processor or randomly selects a free processor for startup. */
void ProcessorWake(struct Schedule *schedule, void *specPro)
{
    struct Processor *processor;
    unsigned int expected = 0;

    if (schedule->state == SCHEDULE_INIT || schedule->state == SCHEDULE_EXITING  ||
        schedule->state == SCHEDULE_EXITED) {
        return;
    }
    if (schedule->scheduleType != SCHEDULE_DEFAULT) {
        ProcessorNonDefaultScheduleWake(schedule);
        return;
    }

    // Check whether the processor is free.
    if (schedule->schdProcessor.freeNum == 0) {
        return;
    }
    // The search thread exists. Do not need to start a new thread.
    if (atomic_load(&schedule->schdThread.searchingNum) != 0 ||
        atomic_compare_exchange_strong(&schedule->schdThread.searchingNum, &expected, 1u) == false) {
        return;
    }

    // Allocate a new processor and set the processor status to running.
    processor = ProcessorAlloc(schedule, (struct Processor *)specPro);
    if (processor == nullptr) {
        atomic_fetch_sub(&schedule->schdThread.searchingNum, 1u);
        return;
    }
    if (processor->thread == nullptr) {
        ThreadAllocBindProcessor(processor, true);
    } else {
        // During sleep, the processor is unbound from the thread. Therefore, the processor
        // should not go to this branch.
        LOG_ERROR(ERRNO_SCHD_PROCESSOR_INVALID, "Processor's thread is not null");
    }
}

/* Randomly allocate an idle processor. or assign a processor. */
struct Processor *ProcessorAlloc(struct Schedule *schedule, struct Processor *specPro)
{
    unsigned long idx;
    struct Processor *newProcessor;
    struct ScheduleProcessor *schdProcessor;
    ProcessorState pidle = PROCESSOR_IDLE;

    schdProcessor = &(schedule->schdProcessor);
    if (schdProcessor->freeNum == 0) {
        return nullptr;
    }

    if (specPro != nullptr && atomic_compare_exchange_strong(&specPro->state, &pidle, PROCESSOR_RUNNING)) {
        // Specify to retrieve a certain processor
        atomic_fetch_sub(&schdProcessor->freeNum, 1u);
        return specPro;
    }

    // Randomly allocate an idle processor
    for (idx = 0; idx < schdProcessor->processorNum; ++idx) {
        newProcessor = &(schdProcessor->processorGroup[idx]);
        pidle = PROCESSOR_IDLE;
        if (atomic_compare_exchange_strong(&newProcessor->state, &pidle, PROCESSOR_RUNNING)) {
            atomic_fetch_sub(&schdProcessor->freeNum, 1u);
            return newProcessor;
        }
    }
    return nullptr;
}

/* Unbind processor and thread and exit */
void ProcessorThreadExit(void)
{
    struct Thread *thread;
    const int sleepTime = PROCESSOR_RELEASE_SLEEP_TIME;

    thread = ThreadGet();

    // Return to the context of the thread_dentry thread and exit safely
    if (ScheduleGet()->schdThread.threadExit) {
#ifdef CODIRA_ASAN_SUPPORT
        // return to ThreadEntry, switch to original thread
        MapleRuntime::Sanitizer::AsanExitCODEThread(thread);
#endif
        CODEThreadContextSet(&thread->context);
    } else {
        // Suspend only, do not exit, exit automatically with the process.
        ProcessorRelease();
        while (1) {
            sleep(sleepTime);
        }
    }
}

/* Retrieve the address of the register array in the current processor */
void *ProcessorGetspecific(const struct Processor *processor, int key)
{
    return processor->pArray[key];
}

void ProcessorSetspecific(struct Processor *processor, int key, void *value)
{
    processor->pArray[key] = value;
}

int ProcessorGetInfoAll(struct ProcessorInfo *processorBuf, unsigned int num)
{
    struct ScheduleProcessor *schdProcessor;
    struct ProcessorInfo *info;
    struct Processor *processor;
    unsigned int index = 0;
    unsigned int count = 0;
    struct Schedule *schedule = ScheduleGet();

    if (schedule == nullptr || processorBuf == nullptr) {
        return 0;
    }
    schdProcessor = &(schedule->schdProcessor);

    while (count < num && index < schdProcessor->processorNum) {
        processor = &(schdProcessor->processorGroup[index]);
        index++;
        if (processor->state == PROCESSOR_IDLE) {
            continue;
        }
        info = &processorBuf[count];
        info->processorId = processor->processorId;
        info->schedCnt = processor->schedCnt;
        if (processor->thread != nullptr) {
#ifdef MRT_MACOS
            info->threadId = 0;
#else
            info->threadId = static_cast<unsigned long>(processor->thread->osThread);
#endif
        } else {
            info->threadId = 0;
        }
        info->state = processor->state;
        info->runqCnt = static_cast<int>(QueueLength(&processor->runq));
        count++;
    }

    return count;
}

MRT_STATIC_INLINE void ProcessorFreelistPush(struct ProcessorFreelist *pfreelist,
                                             struct CODEThread *codethread)
{
    DulinkAdd(&codethread->schdDulink, &pfreelist->freeList);
    pfreelist->codethreadNum++;
}

/* The caller needs to ensure that the pfreelist is not empty */
MRT_STATIC_INLINE struct CODEThread *ProcessorFreelistPop(struct ProcessorFreelist *pfreelist)
{
    struct CODEThread *codethread;
    codethread = DULINK_ENTRY(pfreelist->freeList.next, struct CODEThread, schdDulink);
    DulinkRemove(&(codethread->schdDulink));
    pfreelist->codethreadNum--;
    return codethread;
}

void ProcessorFreelistPut(struct Processor *processor, struct CODEThread *freeCODEThread)
{
    struct Schedule *schedule = static_cast<struct Schedule *>(processor->schedule);
    struct ScheduleGfreeList *gfreelist;
    struct ProcessorFreelist *pfreelist = &processor->freelist;

    if (schedule->state == SCHEDULE_WAITING || schedule->state == SCHEDULE_SUSPENDING) {
        CODEThreadFree(freeCODEThread, false);
        return;
    }
    PthreadSpinLock(&processor->lock);

    ProcessorFreelistPush(pfreelist, freeCODEThread);

    if (pfreelist->codethreadNum >= PROCESSOR_FREE_LIST_CAPACITY) {
        gfreelist = &schedule->schdCODEThread.gfreelist;
        pthread_mutex_lock(&gfreelist->gfreeLock);
        unsigned int count = PROCESSOR_FREE_LIST_HALF_CAPACITY;

        DulinkMove(&gfreelist->gfreeList, &pfreelist->freeList, static_cast<int>(count));

        gfreelist->freeCODEThreadNum += count;
        pfreelist->codethreadNum -= count;

        pthread_mutex_unlock(&gfreelist->gfreeLock);
    }

    PthreadSpinUnlock(&processor->lock);
}

struct CODEThread *ProcessorFreelistGet(struct Processor *processor)
{
    struct CODEThread *codethread = nullptr;
    struct Schedule *schedule = static_cast<struct Schedule *>(processor->schedule);
    struct ScheduleGfreeList *gfreelist = &schedule->schdCODEThread.gfreelist;
    struct ProcessorFreelist *pfreelist = &processor->freelist;
    unsigned int count;
    unsigned int rest = 0;

    if (pfreelist->codethreadNum == 0 && gfreelist->freeCODEThreadNum == 0) {
        return nullptr;
    }

    PthreadSpinLock(&processor->lock);

    if (pfreelist->codethreadNum == 0 && gfreelist->freeCODEThreadNum != 0) {
        pthread_mutex_lock(&gfreelist->gfreeLock);
        count = gfreelist->freeCODEThreadNum;
        // Local/global resource pools are both empty, return directly
        if (count == 0) {
            pthread_mutex_unlock(&gfreelist->gfreeLock);
            PthreadSpinUnlock(&processor->lock);
            return nullptr;
        }
        if (count > PROCESSOR_FREE_LIST_HALF_CAPACITY) {
            count = PROCESSOR_FREE_LIST_HALF_CAPACITY;
            rest = gfreelist->freeCODEThreadNum - count;
        }
        if (count < rest) {
            DulinkMove(&pfreelist->freeList, &gfreelist->gfreeList, static_cast<int>(count));
        } else {
            DulinkMove(&pfreelist->freeList, &gfreelist->gfreeList, -static_cast<int>(rest));
        }
        gfreelist->freeCODEThreadNum -= count;
        pthread_mutex_unlock(&gfreelist->gfreeLock);
        pfreelist->codethreadNum += count;
    }
    if (pfreelist->codethreadNum != 0) {
        codethread = ProcessorFreelistPop(pfreelist);
    }
    PthreadSpinUnlock(&processor->lock);
    return codethread;
}

/* Atomic method to obtain the ID of the processor, using a unified global variable to obtain
 * the processorId under multiple schedulers. */
unsigned int ProcessorNewId(void)
{
    return atomic_fetch_add(&g_scheduleManager.processorIdGen, 1u);
}

unsigned int ProcessorId(void)
{
    struct CODEThread *codethread = CODEThreadGet();
    struct Thread *thread;
    struct Processor *processor;
    if (codethread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_PROCESSOR_GET_FAILED, "codethread is nullptr");
        return 0;
    }
    thread = codethread->thread;
    if (thread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_PROCESSOR_GET_FAILED, "thread is nullptr");
        return 0;
    }
    processor = static_cast<struct Processor *>(thread->processor);
    if (processor == nullptr) {
        LOG_ERROR(ERRNO_SCHD_PROCESSOR_GET_FAILED, "processor is nullptr");
        return 0;
    }
    return processor->processorId;
}

#if defined(CODIRA_TSAN_SUPPORT)

void* ProcessorGetHandle(void)
{
    return ProcessorGetWithCheck();
}

#endif

bool ProcessorCanSpin(void)
{
    struct CODEThread *codethread = CODEThreadGet();
    struct Schedule *schedule;
    struct Thread *thread;
    struct Processor *processor;

    if (codethread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_PROCESSOR_GET_FAILED, "codethread is nullptr");
        return false;
    }
    schedule = codethread->schedule;
    unsigned int totalProcessorNum = schedule->schdProcessor.processorNum;
    unsigned int freeProcessorNum =
        atomic_load(&schedule->schdProcessor.freeNum) + atomic_load(&schedule->schdThread.searchingNum);
    if (totalProcessorNum <= 1 || // Single processor shuold not spin
        totalProcessorNum - 1 <= freeProcessorNum) { // No other running processors
        return false;
    }

    thread = codethread->thread;
    if (thread == nullptr) {
        LOG_ERROR(ERRNO_SCHD_PROCESSOR_GET_FAILED, "thread is nullptr");
        return false;
    }
    processor = static_cast<struct Processor *>(thread->processor);
    if (processor == nullptr) {
        LOG_ERROR(ERRNO_SCHD_PROCESSOR_GET_FAILED, "processor is nullptr");
        return false;
    }
    // If the current processor has other codethreads to run,
    // there is no need to spin.
    if (QueueLength(&processor->runq) != 0) {
        return false;
    }
    return true;
}

#ifdef __cplusplus
}
#endif
