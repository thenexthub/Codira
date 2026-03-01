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


#ifndef MRT_TIMER_IMP_H
#define MRT_TIMER_IMP_H

#include <atomic>
#include <pthread.h>
#include "timer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define FORKS_NUM (4)

/**
 * @brief Timer status
 **/
enum TimerStatus {
    TIMER_IDLE,                /* status after the timer runs. */
    TIMER_WAITING,             /* timer waits to run, which is the most common state. */
    TIMER_RUNNING,             /* timer is running. */
    TIMER_STOPPED,             /* timer is stopped. The timer will be checked and deleted in check_timer. */
    TIMER_MODIFYING,           /* timer is being modified by the stop or reset function. */
    TIMER_MODIFIED_AHEAD,      /* timer is changed to the time before the triggering time. */
    TIMER_MODIFIED_DELAYED,    /* timer is changed to the time after the triggering time. */
    TIMER_REMOVING,            /* timer marked in the halted state is being modified. */
    TIMER_REMOVED,             /* timer marked in the halted state has been removed from the heap. */
    TIMER_MOVING               /* timer marked as TIMER_MODIFIED_AHEAD or TIMER_MODIFIED_DELAYED is being modified. */
};

/**
 * @brief Timer Node
 **/
struct TimerNode {
    void *pPtr;                        /* Pointer to the processor where the timer is located */
    unsigned long long deadline;       /* Time when the timer is triggered */
    unsigned long long newDeadline;    /* Timer needs to be changed, the new trigger time is stored here */
    unsigned long long period;         /* 0: single timer; non-zero: cyclic timer */
    TimerFunc func;                    /* Function invoked when the timer is triggered */
    void *args;                        /* Timer callback function parameter */
    std::atomic<TimerStatus> status;   /* Timer status */
    bool autoReleasing;                /* True indicates that the timer is automatically released after execution */
};

/**
 * @brief Timer Minimum Heap Structure
 **/
struct TimerHeap {
    pthread_mutex_t mutex;                  /* Lock for timer heap */
    struct TimerNode **buf;                 /* Stores the quad-heap memory of the timer.
                                             * The requested memory is 256 temporarily. If the
                                             * memory exceeds 256, the system automatically
                                             * expands the memory to twice the current capacity. */
    unsigned long long int numTimers;       /* Timer number */
    std::atomic<unsigned long long> adjustTimers;       /* Number of timers whose status is TIMER_MODIFIED_AHEAD */
    std::atomic<unsigned long long> stoppedTimers;      /* Number of timers whose status is TIMER_STOPMED */
    unsigned long long int timer0Deadline;  /* Time when the heap top timer is triggered */
    unsigned long long int capacity;        /* heap capacity */
};

/**
 * @brief Structure for batch migration
 **/
struct MoveItem {
    void **buf;
    unsigned long long int len;
};

/**
 * @brief Reset parameter structure
 **/
struct TimerResetPara {
    unsigned long long ddl;           /* Trigger time of the timer to be modified. */
    unsigned long long period;        /* Interval for triggering the timer to be modified */
    TimerFunc func;                   /* Timer callback function to be modified */
    void *args;                       /* Input parameters of the timer callback function to be modified */
};

/**
 * @brief Stop parameter structure
 **/
struct TimerStoppedPara {
    unsigned long long int aheadcnt;             /* Number of timers to be triggered in advance */
    unsigned long long int delcnt;               /* Number of timers deleted from the heap during this adjustment */
    unsigned long long int bufIndex;             /* Index of the adjusted timer in the heap */
};


/**
 * @brief Hook provided for the schedule module to release the timer.
 * @param  processor    [IN]  Processor to be released
 * @retval If the operation is successful, 0 is returned. If the operation fails, an error code is returned.
 **/
int TimerExit(void *processor);

/**
 * @brief Hook provided for the schmon module for checking the timer
 * @param pro    [IN] Processor to be checked
 * @param now    [IN] Current time
 * @retval If the operation is successful, 0 is returned. If the operation fails, an error code is returned.
 **/
int TimerSchmonCheck(void *pro, unsigned long long now);

/**
 * @brief Check whether an executable timer exists in the processor and provide the timer to the non-default scheduler.
 * @param processor     [IN] Processor to be checked
 * @retval If yes, true is returned. If no, false is returned.
 **/
bool TimerCheckReady(struct Processor *processor);

/**
 * @brief Check whether a timer exists in the processor when exiting. The timer is provided for non-default scheduler.
 * @param processor    [IN] Processor to be checked
 * @retval If yes, true is returned. If no, false is returned.
 **/
bool TimerCheckExistence(void *processor);

/**
 * @brief Stop the timer.
 * @par Description: The TimerTryStop method is provided for cmutex. When the timer is in the Waiting state,
 * the timer is marked as the halted state. If the status is not Waiting, the status is Running
 * or Idle, and an error code is returned.
 * @param handle [IN] Timer handle
 * @retval: indicates whether the service is successfully stopped.
 * If the service is successfully stopped, 0 is returned.
 * If the service fails to be stopped, the corresponding error code is returned.
 **/
int TimerTryStop(TimerHandle handle);

/**
 * @brief Hook provided for the schedule module to obtain the number of timers.
 * @retval Returns the current number of timers.
 **/
int TimerNum(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_TIMER_IMPL_H */
