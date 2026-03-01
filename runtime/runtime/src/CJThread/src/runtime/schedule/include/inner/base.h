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

#ifndef MRT_SCHEDULE_BASE_H
#define MRT_SCHEDULE_BASE_H

#ifdef MRT_MACOS
#include <dispatch/dispatch.h>

struct Semaphore {
    dispatch_semaphore_t sem;
};

static inline int SemaphoreInit(struct Semaphore *sem, int pshared, unsigned value)
{
    (void)pshared;
    sem->sem = dispatch_semaphore_create(value);
    if (sem->sem == NULL) {
        return -1;
    }
    return 0;
}

static inline int SemaphoreWait(struct Semaphore *sem)
{
    dispatch_semaphore_wait(sem->sem, DISPATCH_TIME_FOREVER); // always succuess
    return 0;
}

static inline int SemaphoreWaitNoIntr(struct Semaphore *sem)
{
    dispatch_semaphore_wait(sem->sem, DISPATCH_TIME_FOREVER); // always succuess
    return 0;
}

static inline int SemaphorePost(struct Semaphore *sem)
{
    dispatch_semaphore_signal(sem->sem); // always succuess
    return 0;
}

/* Attention:Different from sem_destroy, dispatch_release can be released only when the number
 * of wait and post times is balanced. That is, no object is allowed to use it when it is
 * destroyed.
 **/
static inline int SemaphoreDestroy(struct Semaphore *sem)
{
    dispatch_release(sem->sem); // always succuess
    return 0;
}

struct CODEthreadSpinLock {
    pthread_mutex_t lock;
};

static inline int PthreadSpinInit(struct CODEthreadSpinLock *lock)
{
    return pthread_mutex_init(&lock->lock, nullptr);
}

static inline int PthreadSpinLock(struct CODEthreadSpinLock *lock)
{
    return pthread_mutex_lock(&lock->lock);
}

static inline int PthreadSpinUnlock(struct CODEthreadSpinLock *lock)
{
    return pthread_mutex_unlock(&lock->lock);
}

static inline int PthreadSpinDestroy(struct CODEthreadSpinLock *lock)
{
    return pthread_mutex_destroy(&lock->lock);
}

#else
#include <semaphore.h>

struct Semaphore {
    sem_t sem;
};

static inline int SemaphoreInit(struct Semaphore *sem, int pshared, unsigned value)
{
    return sem_init(&sem->sem, pshared, value);
}

static inline int SemaphoreWait(struct Semaphore *sem)
{
    return sem_wait(&sem->sem);
}

static inline int SemaphoreWaitNoIntr(struct Semaphore *sem)
{
    int error;
    do {
        error = sem_wait(&(sem->sem));
    } while (error != 0 && errno == EINTR);
    return error;
}

static inline int SemaphorePost(struct Semaphore *sem)
{
    return sem_post(&sem->sem);
}

static inline int SemaphoreDestroy(struct Semaphore *sem)
{
    return sem_destroy(&sem->sem);
}


struct CODEthreadSpinLock {
    pthread_spinlock_t lock;
};

static inline int PthreadSpinInit(struct CODEthreadSpinLock *lock)
{
    return pthread_spin_init(&lock->lock, PTHREAD_PROCESS_PRIVATE);
}

static inline int PthreadSpinLock(struct CODEthreadSpinLock *lock)
{
    return pthread_spin_lock(&lock->lock);
}

static inline int PthreadSpinUnlock(struct CODEthreadSpinLock *lock)
{
    return pthread_spin_unlock(&lock->lock);
}

static inline int PthreadSpinDestroy(struct CODEthreadSpinLock *lock)
{
    return pthread_spin_destroy(&lock->lock);
}

#endif
#endif
