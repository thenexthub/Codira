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

#ifndef MRT_WAITQUEUE_H
#define MRT_WAITQUEUE_H

#include <stdbool.h>
#include <pthread.h>
#include "mid.h"
#include "external.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
* @brief 0x100d0001 The waiting queue passed in by the user does not exist
*/
#define ERRNO_QUEUE_DOESNT_EXIST ((MID_WAITQUEUE) | 0x001)

/**
* @brief 0x100d0002 CODEThread returns early but cannot stop the timer
*/
#define ERRNO_TIMER_STOP_FAILED ((MID_WAITQUEUE) | 0x002)

/**
* @brief 0x100d0003 The user wants to wake up the codethread, but the waiting queue is empty
*/
#define ERRNO_QUEUE_IS_EMPTY ((MID_WAITQUEUE) | 0x003)

/**
* @brief 0x100d0004 The callback function returns true in advance
*/
#define ERRNO_CALLBACK_RETURN_TRUE ((MID_WAITQUEUE) | 0x004)

/**
* @brief 0x100d0005 memory allocation failed
*/
#define ERRNO_MALLOC_FAILED ((MID_WAITQUEUE) | 0x005)

/**
* @brief 0x100d0006 Timer state is invalid
*/
#define ERRNO_TIMER_STATE_INVALID ((MID_WAITQUEUE) | 0x006)

/**
* @brief 0x100d0007 A negative waiting time is considered illegal
*/
#define ERRNO_WQ_DURATION_INVALID ((MID_WAITQUEUE) | 0x007)

/**
* @brief 0x100d0008 Waitqueue has reached its limit
*/
#define ERRNO_WQ_MAX_NUM ((MID_WAITQUEUE) | 0x008)

/**
* @brief 0x100d1000 WaitqueuePark interface returned due to timeout
*/
#define  WAIT_QUEUE_TIMEOUT ((MID_WAITQUEUE) | 0x1000)

/**
* @brief The callback function to be executed before waiting for queue park and wake
*/
typedef bool (*WqCallbackFunc)(void *);

/**
 * @brief create a new waitqueue
 * @par create a new waitqueue
 * @param  wq   [IN]  waitqueue pointer
 * @retval 0 represents success, non-zero returns an error code.
 */
int WaitqueueNew(struct Waitqueue *wq);

/**
 * @brief Put the codethread into the waiting queue, and if the waiting time is exceeded, it will automatically wake up
 * @par use a callback function to determine whether to enter sleep mode. If necessary,
 * put the codethread into the waiting queue and sleep. If not awakened within the timeout
 * period, the codethread will automatically wake up. When ns is LLong_max, there is no timeout,
 * and the codethread will wait until another codethread wakes it up.
 * @attention
 * 1.If the callback function is NULL, it will not be judged
 * 2.The longest waiting time is LLong-MAX -1 nanosecond
 * 3.If ns <= 0, return failed.
 * @param  wq               [IN]  waitqueue for codethread
 * @param  ns               [IN]  timeout
 * @param  callbackFunc     [IN]  callback function
 * @param  arg              [IN]  callback parameters
 * @retval The value 0 indicates success.
 * The value WAIT_QUEUE_TIMEOUT(0x100d1000) indicates automatic wakeup upon timeout.
 * Other values indicate error scenarios.
 */
int WaitqueuePark(struct Waitqueue *wq, long long ns,
                  WqCallbackFunc callbackFunc, void *arg, bool head);

/**
 * @brief Wake up a codethread in the queue
 * @par Determine whether to wake up the codethread through the callback function.
 * If necessary, wake up the codethread of the queue head.
 * @param  wqHandle         [IN]  waitqueue handle
 * @param  callbackFunc     [IN]  callback function
 * @param  arg              [IN]  callback parameters
 * @retval The value 0 indicates success. A non-zero value indicates an error code.
 */
int WaitqueueWakeOne(struct Waitqueue *wqHandle, WqCallbackFunc callbackFunc,
                     void *arg);

/**
 * @brief Wake up all codethreads in the queue
 * @par Determine whether to wake up the codethread through the callback function.
 * If necessary, wake up all the codethreads of the queue head.
 * @param  wqHandle         [IN]  waitqueue handle
 * @param  callbackFunc     [IN]  callback function
 * @param  arg              [IN]  callback parameters
 * @retval The value 0 indicates success. A non-zero value indicates an error code.
 */
int WaitqueueWakeAll(struct Waitqueue *wqHandle, WqCallbackFunc callbackFunc,
                     void *arg);

/**
 * @brief Deletes and releases a specified waiting queue.
 * @par Release all nodes in the waiting queue, destroy the lock of the waiting queue,
 * and release the bidirectional queue of the waiting queue.
 * @param  wq               [IN]  waitqueue
 * @retval The value 0 indicates success. A non-zero value indicates an error code.
 */
int WaitqueueDelete(struct Waitqueue *wq);

/**
 * @brief Get the number of waits in the waitqueue.
 * @par Get the number of waits in the waitqueue.
 * @attention  If the queue is NULL, 0 is returned.
 * @param  wq               [IN]  waitqueue
 * @retval the number of waits
 */
unsigned int WaitqueueGetWaitNum(const struct Waitqueue *wq);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_WAITQUEUE_H */
