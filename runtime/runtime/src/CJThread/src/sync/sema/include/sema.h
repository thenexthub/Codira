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

#ifndef MRT_SEMA_H
#define MRT_SEMA_H

#include <stdbool.h>
#include "mid.h"
#include "external.h"
#include "waitqueue.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/**
* @brief 0x100e0001 The semaphore passed in by the user does not exist
*/
#define ERRNO_SEMA_DOESNT_EXIST ((MID_SEMA) | 0x001)

/**
* @brief 0x100e0002 The parameters passed in by the user are incorrect
*/
#define ERRNO_SEMA_ARG_INVALID ((MID_SEMA) | 0x002)

/**
* @brief 0x100e0003 Signal count reaches maximum value
*/
#define ERRNO_SEMA_OVERFLOW ((MID_SEMA) | 0x003)

/**
 * @brief Create a semaphore
 * @par Create a semaphore and allocate space for the waiting queue in the semaphore, with an initial value of 1.
 * @param semaUser  [IN]  The pointer to the semaphore structure points to the memory
 * of the already allocated Sema structure.
 * @param semaVal   [IN]  Initial value of semaphore
 * @retval Success returns 0, failure returns error code
 */
int SemaNew(struct Sema *semaUser, unsigned int semaVal);

/**
 * @brief Delete a semaphore
 * @param  semHandle   [IN]  The target semaphore to be deleted
 * @retval Success returns 0, failure returns error code
 */
int SemaDelete(struct Sema *semHandle);

/**
 * @brief Attempt to obtain a signal once
 * @par Attempt to obtain a semaphore once, if unsuccessful, return directly without entering the waiting queue.
 * @param  semHandle   [IN]  Handle of the target semaphore.
 * @retval True means obtained, false means not obtained
 */
bool SemaTryAcquire(struct Sema *semHandle);

/**
 * @brief Obtain the specified semaphore
 * @par Obtain the target semaphore. If the semaphore is held by other codethreads,
 * the codethread will be suspended until the semaphore is obtained.
 * @param  semHandle   [IN]  The target signal quantity to be obtained.
 * @retval Success returns 0, failure returns error code
 */
int SemaAcquire(struct Sema *semHandle, bool head);

/**
 * @brief Release a semaphore
 * @par Release a semaphore, and if the waiting queue is not empty, wake up the codethread at the head of the queue
 * @param  semHandle   [IN]  The target semaphore to be deleted
 * @retval Success returns 0, failure returns error code
 */
int SemaRelease(struct Sema *semHandle);

/**
 * @brief Obtain the value of the semaphore
 * @attention  Once the semaphore value returns, it should be considered outdated
 * @param  semHandle   [IN]  To obtain the signal quantity of the value
 * @param  semVal      [OUT]  Signal value
 * @retval Success returns 0, failure returns error code
 */
int SemaGetValue(const struct Sema *semHandle, int *semVal);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_SEMA_H */
