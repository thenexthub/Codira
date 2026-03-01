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


#ifndef MRT_TIMER_H
#define MRT_TIMER_H

#include <stdbool.h>
#include "mid.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
 * @brief 0x100b0001
 * Memory application for the structure fails. (This problem may occur when the timer is created,
 * the timerheap is created, or the timerheap is expanded to the buffer.)
 * Create the reset_timer_para structure in timer_reset, apply for memory for the MoveItem.buf member,
 * and allocate memory for the timer_sleep interface parameters.
 * Failed to apply for memory in timer_handle_pool_init().
 */
#define ERROR_TIMER_ALLOC ((MID_TIMER) | 0x001)

/**
 * @brief 0x100b0002
 * The timer status must be locked. If the timer status fails to be changed,
 * the lock is invalid and a status exception error is returned.
 */
#define ERROR_TIMER_STATE ((MID_TIMER) | 0x002)

/**
 * @brief 0x100b0003
 * An error occurs in the pointer of index 0 of the register of the processor stored in the timer.
 * This pointer is assigned a value in the timer_new interface.
 */
#define ERROR_TIMER_PPTR   ((MID_TIMER) | 0x003)

/**
 * @brief 0x100b0004
 * The timer has been marked as being deleted. The stop operation is invalid.
 */
#define ERROR_TIMER_STOP_FAILED   ((MID_TIMER) | 0x004)

/**
 * @brief 0x100b0005
 * The input parameter of the timer_new function is invalid.
 */
#define ERROR_TIMER_NEW_PARA_INVALID   ((MID_TIMER) | 0x005)

/**
 * @brief 0x100b0006
 * If the pointer is null, the input parameter of timer_reset may be verified.
 * The return value of timer_new is incorrect. The input parameter of timer_try_lock is verified.
 */
#define ERROR_TIMER_PTR_INVALID   ((MID_TIMER) | 0x006)

/**
 * @brief 0x100b0007
 * The timer heap is not initialized.
 */
#define ERROR_TIMER_HEAP_NOT_INIT   ((MID_TIMER) | 0x007)

/**
 * @brief 0x100b0008
 * The timer has been executed. This error code is used in the timer_try_lock interface.
 * If the timer is not in the Waiting state, this error code is returned.
 */
#define ERROR_TIMER_IS_NOT_WAITING   ((MID_TIMER) | 0x008)

/**
 * @brief 0x100b0009
 * The timer still exists in the timer module. Failed to exit.
 * */
#define ERROR_TIMER_EXIT_FAILED   ((MID_TIMER) | 0x009)

/**
 * @brief 0x100b000a
 * Failed to initialize the lock in the timer.
 */
#define ERROR_TIMER_MUTEX_INVALID   ((MID_TIMER) | 0x000a)

/**
 * @brief 0x100b000b
 * timer_handle is invalid
 */
#define ERROR_TIMER_HANDLE_INVALID   ((MID_TIMER) | 0x000b)

/**
 * @brief 0x100b000c
 * The timer node has been released.
 * This error code is returned when the timer corresponding to the index held by the user has been released.
 * This error code is returned if the timer is in idle or removed state in timer_reset_heap_adjust.
 */
#define ERROR_TIMER_FREE   ((MID_TIMER) | 0x000c)

/**
 * @brief Handle returned when the timer node is created.
 */
typedef void* TimerHandle;

/**
 * @brief Function prototype restriction passed in TimerNew.
 */
typedef void (*TimerFunc)(void *args);

/**
 * @brief create a timer
 * @par Description: Create a timer and check whether the current processor initializes the timer heap structure.
 * If not, initialize the timer heap structure. If yes, insert the timer into the heap of the current processor.
 * You are advised to select whether to encapsulate the timer_new interface as required.
 * (that is, call couroutine_new() in the func of the input timer) encapsulate the function to be executed into
 * a codethread for execution. The advantage is that when the timer performs a time-consuming operation,
 * a processor is not occupied for a long time under the protection of codethread timeout mechanism.
 * @attention Invoke the timer created by timer_new. If the timer is no longer used, use timer_release to
 * release the timer.
 * @param dur [IN] The timer is triggered after the dur duration, expressed in nanoseconds.
 * @param period [IN] Indicates whether the timer is a cyclic timer. If period is greater than 0,
 * the timer is cyclic. After the timer is triggered, the period is transferred as the dur of the new timer.
 * period = 0 indicates a non-cyclic timer.
 * @param fun [IN] Callback function when the timer is triggered
 * @param args [IN] Input parameter of the callback function
 * @retval If successful, the index is returned. If fails, 0 is returned.
 */
TimerHandle TimerNew(unsigned long long dur, unsigned long long period, TimerFunc fun, void *args);

/**
 * @brief  stop the timer
 * @par Description: The timer is not stopped immediately. Instead, the state of the timer is
 * marked as TIMER_STOPMED and deleted when the timer is adjusted.
 * The single timer is automatically released after the execution is complete.
 * @param handle [IN] Timer handle
 * @retval If the stop fails, - 1 is returned. If the stop succeeds, 0 is returned.
 */
int TimerStop(TimerHandle handle);

/**
 * @brief Modify the structure of a specified timer.
 * @par Description: If the timer is a common timer, update its triggering time. If fun is
 * notempty, update fun and args,
 * For a cyclic timer, update its triggering time and period. If fun is not empty, update funand args.
 * @param handle [IN] Timer handle
 * @param dur [IN] The timer is triggered after the dur duration, expressed in nanoseconds.
 * @param period [IN] Indicates whether the timer is a cyclic timer. If period is greater than 0,
 * the timer is cyclic. After the timer is triggered, the period is transferred as the dur of the new timer,
 * period = 0 indicates a non-cyclic timer.
 * @param fun [IN] Callback function when the timer is triggered. If you do not change the value, the value is NULL.
 * @param args [IN] Input parameter of the callback function. If func does not need to be modified, the value is NULL.
 * @retval: If successful, 0 is returned. If fails, the corresponding error code is returned.
 */
int TimerReset(TimerHandle handle, unsigned long long ddl, unsigned long long period, TimerFunc fun, void *args);

/**
 * @brief provides codethread sleep function
 * @par Description: Wake up the codethread after entering the time
 * @param duration     [IN] codethread sleep time. The codethread wakes up after at least duration nanoseconds.
 * If duration is less than or equal to 0, the codethread performs lightweight sleep to release
 * execution resources and does not perform sleep.
 * @retval: If successful, 0 is returned. If fails, an error code is returned.
 */
int TimerSleep(long long duration);

/**
 * @brief Provides the timer release function.
 * @par Description: The corresponding timer is released through the input handle.
 * @attention This function can be called at any time in the timer life cycle to release the
 * space of the timer. The specific release time after this function is called is uncertain.
 * The timer module ensures memory security.
 * Do not use the input handle during or after the function is invoked. Otherwise, the wild
 * pointer will be accessed. This function must be called once after timer_new(),
 * Otherwise, a memory leak occurs. This function does not affect the timer function.After
 * timer_release is executed, callback is triggered for timers that are not triggered after
 * the timer expires. Then it's automatically released.
 * @param handle    [IN] Timer handle
 * @retval: If successful, 0 is returned. If fails, the corresponding error code is returned.
 */
int TimerRelease(TimerHandle handle);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* _TIMER_H */
