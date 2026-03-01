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


#ifndef MRT_TRACE_H
#define MRT_TRACE_H

#include <cstdbool>
#include <schedule.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
* @brief 0x10150000 trace has already started
*/
#define ERRNO_TRACE_ALREADY_START ((MID_TRACE) | 0x000)

/**
* @brief 0x10150001 trace shutdown error
*/
#define ERRNO_TRACE_SHUTDOWN_WRONG ((MID_TRACE) | 0x001)

/**
* @brief 0x10150002 traceBuf alloc memory error
*/
#define ERRNO_TRACE_BUF_FLUSH_WRONG ((MID_TRACE) | 0x002)

/**
* @brief 0x10150003 Event recording is not performed when trace is disabled.
*/
#define ERRNO_TRACE_EVENT_WHEN_STOP ((MID_TRACE) | 0x003)

/**
* @brief 0x10150004 malloc fail
*/
#define ERRNO_TRACE_MALLOC_FAILED ((MID_TRACE) | 0x004)

/**
* @brief 0x10150005 trace reader abnormal wakeup
*/
#define ERRNO_TRACE_READER_SPURIOUS_WAKEUP ((MID_TRACE) | 0x005)

/**
* @brief 0x10150006 reader codethread is being executed
*/
#define ERRNO_TRACE_MULTIPLE_READER ((MID_TRACE) | 0x006)

/**
* @brief 0x10150007 traceStop exception
*/
#define ERRNO_TRACE_STOP_EXCEPTION ((MID_TRACE) | 0x007)

/**
* @brief 0x10150008 trace has been stopped
*/
#define ERRNO_TRACE_ALREADY_STOP ((MID_TRACE) | 0x008)

/**
* @brief 0x10150009 trace string event error
*/
#define ERRNO_TRACE_STRING_EVENT ((MID_TRACE) | 0x009)

/**
* @brief 0x1015000A trace stack event error
*/
#define ERRNO_TRACE_STACK_EVENT ((MID_TRACE) | 0x00A)

/**
 * @brief start trace
 * @par start trace
 * @attention only one trace exists globally
 * @param  traceType         [IN]  trace event
 * @retval true or false
 */
bool TraceStart(unsigned short traceType);

/**
 * @brief stop trace
 * @par stop trace
 * @attention only one trace exists globally
 * @retval true or false
 */
bool TraceStop(void);

/**
 * @brief Common methods for recording common trace events
 * @par Common methods for recording common trace events
 * @attention The events that need to store the call stack must be in the context of codethread.
 * Ensure that the number of argNum is the same as the number of input extra parameters.
 * @param  event             [IN]  event type
 * @param  skip              [IN]  when skip>=0,call stack needs to be stored.
 * The value of skip indicates the number of layers to be returned to the stack.
 * @param  mutator           [IN]  mutator is used for backstack
 * @param  argNum            [IN]  number of extra parameters
 * @param  args              [IN]  extra parameters
 * @retval null
 */
void TraceRecordEvent(TraceEvent event, int skip, void *mutator, int argNum, va_list args);

bool TraceRecordStackEvent(unsigned long long stackId, int skip, void *mutator);

unsigned long long TraceRecordStringEvent(const char* str);

void TraceRecordStartEvent();

bool TraceRecordSpecialStackEvent(unsigned long long stackId, const char* funcStr);

unsigned long long TraceStackId();

/**
 * @brief Outputs trace events stored in the buffer as byte arrays.
 * @par Outputs trace events stored in the buffer as byte arrays.
 * @param  len             [OUT]  Byte array length
 * @retval NULL or charArrayPointer
 */
unsigned char *TraceDump(int *len);

/**
 * @brief Obtains the codethread of the blocked dump trace event
 * @par Obtains the codethread of the blocked dump trace event.
 * NULL is returned when the traceReader is executing or fullTraceBuf is used.
 * @retval NULL or CODEThreadPointer
 */
struct CODEThread *TraceReaderGet(void);

/**
 * @brief Register the trace method with the function hook of the scheduleManager.
 * @par Register the trace method with the function hook of the scheduleManager.
 * @retval NULL
 */
void TraceRegister(void);

/**
 * @brief Deregister the trace method with the function hook of the scheduleManager.
 * @par Deregister the trace method with the function hook of the scheduleManager.
 * @retval NULL
 */
void TraceDeregister(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_TRACE_H */
