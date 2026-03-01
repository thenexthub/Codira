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

#ifndef COMMON_COMPONENTS_PLATFORM_MUTEX_H
#define COMMON_COMPONENTS_PLATFORM_MUTEX_H

#include "common_interfaces/base/common.h"
#include "common_interfaces/thread/thread_state_transition.h"

#ifdef DEBUG
#define FATAL_IF_ERROR(f, rc)                             \
    do {                                                  \
        if (rc != 0) {                                    \
            LOG_COMMON(FATAL)<< f << " failed: " << rc;   \
        }                                                 \
    } while (false)
#else
#define FATAL_IF_ERROR(f, rc) static_cast<void>(0)
#endif

namespace common {
class Mutex {
public:
    explicit Mutex(bool isInit = true);

    ~Mutex();

    void Lock();

    bool TryLock();

    void Unlock();

protected:
    void Init(pthread_mutexattr_t *attrs);

private:
    pthread_mutex_t mutex_;
    friend class ConditionVariable;
};

inline Mutex::Mutex(bool isInit) : mutex_()
{
    if (isInit) {
        Init(nullptr);
    }
}

inline Mutex::~Mutex()
{
    [[maybe_unused]] int rc = pthread_mutex_destroy(&mutex_);
    FATAL_IF_ERROR("pthread_mutex_destroy", rc);
}

inline void Mutex::Init(pthread_mutexattr_t *attrs)
{
    [[maybe_unused]] int rc = pthread_mutex_init(&mutex_, attrs);
    FATAL_IF_ERROR("pthread_mutex_init", rc);
}

inline void Mutex::Lock()
{
    [[maybe_unused]] int rc = pthread_mutex_lock(&mutex_);
    FATAL_IF_ERROR("pthread_mutex_lock", rc);
}

inline bool Mutex::TryLock()
{
    int rc = pthread_mutex_trylock(&mutex_);
    if (rc == EBUSY) {
        return false;
    }

    FATAL_IF_ERROR("pthread_mutex_trylock", rc);

    return true;
}

inline void Mutex::Unlock()
{
    [[maybe_unused]] int rc = pthread_mutex_unlock(&mutex_);
    FATAL_IF_ERROR("pthread_mutex_unlock", rc);
}

class LockHolder {
public:
    explicit LockHolder(Mutex &mtx) : lock_(mtx)
    {
        lock_.Lock();
    }

    ~LockHolder()
    {
        lock_.Unlock();
    }

private:
    Mutex &lock_;

    NO_COPY_SEMANTIC_CC(LockHolder);
    NO_MOVE_SEMANTIC_CC(LockHolder);
};

class PUBLIC_API ConditionVariable {
public:
    ConditionVariable();

    ~ConditionVariable();

    void Signal();

    void SignalAll();

    void Wait(Mutex *mutex);

    bool TimedWait(Mutex *mutex, uint64_t ms, uint64_t ns = 0, bool is_absolute = false);

private:
    pthread_cond_t cond_;
};

inline ConditionVariable::ConditionVariable() : cond_()
{
    [[maybe_unused]]int rc = pthread_cond_init(&cond_, nullptr);
    FATAL_IF_ERROR("pthread_cond_init", rc);
}

inline ConditionVariable::~ConditionVariable()
{
    [[maybe_unused]]int rc = pthread_cond_destroy(&cond_);
    FATAL_IF_ERROR("pthread_cond_destroy", rc);
}

inline void ConditionVariable::Signal()
{
    [[maybe_unused]]int rc = pthread_cond_signal(&cond_);
    FATAL_IF_ERROR("pthread_cond_signal", rc);
}

inline void ConditionVariable::SignalAll()
{
    [[maybe_unused]]int rc = pthread_cond_broadcast(&cond_);
    FATAL_IF_ERROR("pthread_cond_broadcast", rc);
}

inline void ConditionVariable::Wait(Mutex *mutex)
{
    [[maybe_unused]]int rc = pthread_cond_wait(&cond_, &mutex->mutex_);
    FATAL_IF_ERROR("pthread_cond_wait", rc);
}

}
#endif //COMMON_COMPONENTS_PLATFORM_MUTEX_H

