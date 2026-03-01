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


#include <cstdlib>
#include <atomic>
#include "sema_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

int SemaNew(struct Sema *semaUser, unsigned int semaVal)
{
    int error;
    struct SemaInner *sema = reinterpret_cast<struct SemaInner*>(semaUser);

    if (sema == nullptr) {
        return ERRNO_SCHD_ARG_INVALID;
    }
    sema->semaVal = semaVal;
    error = WaitqueueNew(&sema->queue);
    if (error) {
        return error;
    }
    return 0;
}

bool SemaTryAcquire(struct Sema *semHandle)
{
    unsigned int semVal;
    struct SemaInner *sema = reinterpret_cast<struct SemaInner*>(semHandle);

    if (sema == nullptr) {
        return false;
    }

    while (1) {
        semVal = atomic_load(&sema->semaVal);
        // 0 indicates locking, unable to obtain semaphore.
        if (semVal == 0) {
            return false;
        }
        // If it is not 0, try subtracting 1.
        // If successful, it means you have received the signal quantity.
        if (atomic_compare_exchange_weak(&sema->semaVal, &semVal, semVal - 1)) {
            return true;
        }
    }
}

int SemaAcquire(struct Sema *semHandle, bool head)
{
    int ret;
    struct SemaInner *sema = reinterpret_cast<struct SemaInner*>(semHandle);

    if (sema == nullptr) {
        return ERRNO_SEMA_DOESNT_EXIST;
    }

    // Attempt to quickly obtain semaphore
    if (SemaTryAcquire(semHandle)) {
        return 0;
    }
    while (1) {
        // Put the codethread into the waiting queue and sleep
        ret = WaitqueuePark(&sema->queue, LLONG_MAX, (WqCallbackFunc) SemaTryAcquire, (void *) sema, head);
        // The semaphore was obtained in advance through a callback function,
        // and the codethread did not enter sleep mode.
        if (ret == ERRNO_CALLBACK_RETURN_TRUE) {
            break;
        }
        if (ret != 0) {
            return ret;
        }
        // This has been awakened by other codethreads, attempting to retrieve the lock again.
        if (SemaTryAcquire(semHandle)) {
            break;
        }
    }
    return 0;
}

int SemaRelease(struct Sema *semHandle)
{
    int ret;
    struct SemaInner *sema = reinterpret_cast<struct SemaInner*>(semHandle);
    unsigned int oldValue;

    if (sema == nullptr) {
        return ERRNO_SEMA_DOESNT_EXIST;
    }
    // Increase the signal quantity by 1.
    while (1) {
        oldValue = sema->semaVal;
        if (oldValue == UINT32_MAX) {
            return ERRNO_SEMA_OVERFLOW;
        }
        if (atomic_compare_exchange_strong(&sema->semaVal, &oldValue, oldValue + 1)) {
            break;
        }
    }
    // If there are no waiting codethreads, return directly.
    if (WaitqueueGetWaitNum(&sema->queue) == 0) {
        return 0;
    }
    // Wakeup a codethread.
    ret = WaitqueueWakeOne(&sema->queue, nullptr, nullptr);
    if (ret != 0 && ret != ERRNO_QUEUE_IS_EMPTY) {
        return ret;
    }
    return 0;
}

int SemaDelete(struct Sema *semHandle)
{
    struct SemaInner *sema = reinterpret_cast<struct SemaInner*>(semHandle);

    if (sema == nullptr) {
        return ERRNO_SEMA_DOESNT_EXIST;
    }
    WaitqueueDelete(&sema->queue);
    return 0;
}

int SemaGetValue(const struct Sema *semHandle, int *semVal)
{
    const struct SemaInner *sema;
    const struct Waitqueue *wq;

    sema = reinterpret_cast<const struct SemaInner*>(semHandle);
    if (sema == nullptr) {
        return ERRNO_SEMA_DOESNT_EXIST;
    }

    if (semVal == nullptr) {
        return ERRNO_SEMA_ARG_INVALID;
    }

    wq = &sema->queue;
    *semVal = atomic_load(&sema->semaVal) - WaitqueueGetWaitNum(wq);

    return 0;
}

#ifdef __cplusplus
}
#endif
