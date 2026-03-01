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


#ifndef MRT_PROCESSOR_H
#define MRT_PROCESSOR_H

#include <pthread.h>
#include <atomic>
#include "queue.h"
#include "list.h"
#include "codethread.h"
#include "trace_impl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* Attempts to access the global queue every 32 times. */
#define GLOBAL_SCH_NUM (1 << 5)

/* Length of the running queue in the processor */
#define PROCESSOR_QUEUE_CAPACITY 128

/* Steal ratio of the running queue in the processor */
#define PROCESSOR_STEAL_RATIO 2

/* Ratio for addring the local queue to the global queue when local is full */
#define GLOBAL_ADD_RATIO 4

/* Number of reserved positions for the p_array register of the processor */
#define PROCESSOR_PARRAY_NUM 4

/* Number of processor_steal loops */
#define PROCESSOR_STEAL_ROUNDS 2

/* Relationship between the number of processors in the running state and the multiple
 * of the search thread */
#define RUNNING_PROCESSOR_SEARCHING_NUM_MULTIPLE 2

/* The index of the local variable of the TimerHeap in the Processor. */
#define KEY_TIMER (0)

#define PROCESSOR_STEAL_SLEEP_THRESHOLD 2

#define PROCESSOR_SCHED_COUNT_THRESHOLD 100

/**
 * @brief processor state
 */
enum ProcessorState {
    PROCESSOR_IDLE,
    PROCESSOR_RUNNING,
    PROCESSOR_EXITING,
    PROCESSOR_SYSCALL,
};

/**
 * @brief processor free list
 */
struct ProcessorFreelist {
    struct Dulink freeList;
    unsigned int codethreadNum;
};

/**
 * @brief Status record structure, which records the latest status of the processor observed
 * by the monitor thread.
 */
struct ProcessorObservedRecord {
    unsigned long lastSchedCnt;
    unsigned long long lastTime;
};

/**
 * @brief processor structure
 */
struct Processor {
    unsigned int processorId;                    /* processor id */
    std::atomic<struct CODEThread *> codethreadNext; /* Next-priority scheduling codethread */
    void *schedule;                              /* scheduler */
    unsigned long schedCnt;                      /* schedule count */
    struct ProcessorObservedRecord obRecord;     /* latest state */
    struct Queue runq;                           /* lockless queue to run codethread */
    struct CODEthreadSpinLock lock;                /* lock of local codethread free list */
    struct ProcessorFreelist freelist;           /* local codethread free list */
    struct Thread *thread;                       /* bound thread */
    std::atomic<ProcessorState> state;           /* processor state */
    void *pArray[PROCESSOR_PARRAY_NUM];          /* processor reserved position. Index 0 is
                                                  *used to store the timer heap structure */
    struct TraceBuf *traceBuf;                   /* processor local trace buffer */
};

/**
 * @brief processor info structure
 */
struct ProcessorInfo {
    unsigned int processorId;                    /* processor id */
    int state;                                   /* processor state, 1 is running */
    int runqCnt;                                 /* codethread number of running queue */
    unsigned long schedCnt;                      /* schedule count */
    unsigned long threadId;                      /* thread id */
};

/**
 * @brief Add the specified codethread to the running queue under the processor.
 * @par Description: Adds a specified codethread to the running queue under the current processor.
 * @attention If the local queue fails to be added, the system joins the global running queue.
 * @param codethread    [IN] Array of codethreads to be added to the running queue
 * @param num    [IN] Number of codethreads
 * @retval 0 or error code
 */
int ProcessorLocalWriteBatch(struct CODEThread *codethread[], unsigned int num);

/**
 * @brief Put the specified codethread and some codethreads in the local queue of Processor into
 * the global queue.
 * @par Description: Place 1/4 of the local queue of a specified processor into the global
 * queue and add the specified codethread to the global queue.
 * @attention
 * @param codethread    [IN] The codethread to be added to the running queue.
 * @param num    [IN] Number of codethreads
 * @retval 0 or error code
 */
int ProcessorGlobalWrite(struct CODEThread *codethreadList[], unsigned int num);

/**
 * @brief Find the next codethread to be scheduled.
 * @par Description: Enable the processor to obtain the to-be-scheduled codethread from the
 * global queue and local queue. If the codethread is not stolen from the queues of other
 * processors, the processor will not sleep.
 * @retval Pointer to the codethread to be scheduled.
 */
struct CODEThread *ProcessorCODEThreadGet(void);

/**
* @brief Initialize a processor.
* @par Description: Initializes the scheduler, queue, and others of the specified processor.
* @param schedule     [IN] Initializes the scheduler of the processor.
* @param processor     [IN] Processor to be initialized
* @param processorId     [IN] Initialize the processor ID.
* @retval 0 or error code
 */
int ProcessorInit(void *schedule, struct Processor *processor, unsigned int processorId);

/**
 * @brief The context switches to a new schedulable codethread.
 * @par Description: Finds a schedulable codethread and switches context to this codethread.
 * If the current thread is bound to a codethread, the thread is switched out. If the
 * to-be-scheduled
 * If a codethread is bound to a thread, the codethread is switched to the corresponding thread
 * for execution.
 */
void ProcessorSchedule(void);

/**
 * @brief Stop the current processor. When a new codethread is available, the processor is
 * scheduled again. A check is performed before the processor is stopped.
 * @retval 0 or error code
 */
int ProcessorStopWithLastCheck(void);

/**
 * @brief Unbind the processor from the thread and exit.
 */
void ProcessorThreadExit(void);

/**
 * @brief Initialize the random seed.
 */
void RandSeedInit(void);

/**
 * @brief Read the global queue.
 * @attention Reads to the local queue in batches (batch is true) can be invoked only in the
 * codethread framework and only when the local queue is empty.
 * @param schedule    [IN] The scheduler used to initialize the processor.
 * @param batch    [IN] Indicates whether to read global queues in batches.
 * @retval codethread
 */
struct CODEThread *ProcessorGlobalRead(void *schedule, bool batch);

/**
 * @brief Get CODEthreadNext of the processor
 * @param  processor        [IN]  processor
 * @retval codethread
 */
MRT_INLINE static struct CODEThread *ProcessorCODEhreadNextRead(struct Processor *processor)
{
    while (true) {
        struct CODEThread *codethread = atomic_load_explicit(&processor->codethreadNext, std::memory_order_relaxed);
        if (codethread == nullptr) {
            return nullptr;
        }
        if (std::atomic_compare_exchange_weak(&processor->codethreadNext, &codethread, (struct CODEThread *)nullptr)) {
            return codethread;
        }
    }
}

/**
 * @brief Get the first codethread to run from the local queue
 * @param  processor        [IN]  processor
 * @retval codethread
 */
MRT_INLINE static struct CODEThread *ProcessorLocalRead(struct Processor *processor)
{
    return (struct CODEThread *)QueuePopHead(&processor->runq);
}
/**
 * @brief Add a single codethread to the local queue.
 * @par Description: If the local queue is full, add the local queue to the global queue.
 * @param codethread [IN] Pointer to the codethread to be added
 * @retval 0 or error code
 */
int ProcessorLocalWrite(struct CODEThread *codethread, bool isReschd = false);

/**
 * @brief Unbind processor and thread
 * @retval 0 or error code
 */
int ProcessorRelease(void);

/**
 * @brief Add the reusable codethread to the local free list or global free list of the processor,
 * and determine whether the global free list needs to be dynamically scaled in.
 * @par Description: Insert the codethread into the local free linked list of the processor.
 * When the number of nodes in the local free linked list is greater than or equal to
 * #PROCESSOR_FREE_LIST_CAPACITY, continuously fetching elements to the global linked list
 * until the number of remaining elements in the local free linked list is
 * #PROCESSOR_FREE_LIST_HALF_CAPACITY.
 * @param processor    [IN] Processor bound to the current codethread running
 * @param freeCODEThread    [IN] codethread to be reused
 */
void ProcessorFreelistPut(struct Processor *processor, struct CODEThread *freeCODEThread);

/**
 * @brief Obtain the codethread control block from the local linked list.
 * @par Description: If the local free list is not empty, the elements are extracted from the
 * local free list. If the local free list is empty but the global free list is not empty,
 * #PROCESSOR_FREE_LIST_HALF_CAPACITY (if global free links are obtained from the global free
 * link list). If the number of tables is less than #PROCESSOR_FREE_LIST_HALF_CAPACITY fetch
 * all) to the local free linked list, and then fetch from the local free linked list
 * @param processor    [IN] Processor bound to the current codethread running
 * @retval codethread
 */
struct CODEThread *ProcessorFreelistGet(struct Processor *processor);

/**
 * @brief Obtain the information about all running processors.
 * @attention Only information about running processors is obtained.
 * @param processorBuf    [OUT] Cache for loading processor information.
 * @param num    [IN] A maximum of num processors can be obtained.
 * @retval Return the number of obtained Processor information.
 */
int ProcessorGetInfoAll(struct ProcessorInfo processorBuf[], unsigned int num);

/**
 * @brief Get processor new id
 */
unsigned int ProcessorNewId(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_PROCESSOR_H */
