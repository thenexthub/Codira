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

#ifndef SRC_JSVM_MUTEX_H_
#define SRC_JSVM_MUTEX_H_

#include <memory>  // std::shared_ptr<T>
#include <utility> // std::forward<T>

#include "jsvm_util.h"
#include "uv.h"

namespace jsvm {

template<typename Traits>
class ConditionVariableBase;
template<typename Traits>
class MutexBase;
struct LibuvMutexTraits;
struct LibuvRwlockTraits;

using ConditionVariable = ConditionVariableBase<LibuvMutexTraits>;
using Mutex = MutexBase<LibuvMutexTraits>;
using RwLock = MutexBase<LibuvRwlockTraits>;

template<typename T, typename MutexT = Mutex>
class ExclusiveAccess {
public:
    ExclusiveAccess() = default;

    template<typename... Args>
    explicit ExclusiveAccess(Args&&... args) : item(std::forward<Args>(args)...)
    {}

    ExclusiveAccess(const ExclusiveAccess&) = delete;
    ExclusiveAccess& operator=(const ExclusiveAccess&) = delete;

    class Scoped {
    public:
        // ExclusiveAccess will commonly be used in conjunction with std::shared_ptr
        // and without this constructor it's too easy to forget to keep a reference
        // around to the shared_ptr while operating on the ExclusiveAccess object.
        explicit Scoped(const std::shared_ptr<ExclusiveAccess>& shared)
            : shared(shared), scopedLock(shared->mutex), pointer(&shared->item)
        {}

        explicit Scoped(ExclusiveAccess* exclusiveAccess)
            : shared(), scopedLock(exclusiveAccess->mutex), pointer(&exclusiveAccess->item)
        {}

        T& operator*() const
        {
            return *pointer;
        }
        T* operator->() const
        {
            return pointer;
        }

        Scoped(const Scoped&) = delete;
        Scoped& operator=(const Scoped&) = delete;

    private:
        std::shared_ptr<ExclusiveAccess> shared;
        typename MutexT::ScopedLock scopedLock;
        T* const pointer;
    };

private:
    friend class ScopedLock;
    MutexT mutex;
    T item;
};

template<typename Traits>
class MutexBase {
public:
    inline MutexBase();

    inline ~MutexBase();

    MutexBase(const MutexBase&) = delete;

    MutexBase& operator=(const MutexBase&) = delete;

    inline void Lock();

    inline void Unlock();

    inline void RdLock();

    inline void RdUnlock();

    class ScopedUnlock;
    class ScopedLock;

    class ScopedLock {
    public:
        inline explicit ScopedLock(const MutexBase& mutex);

        inline explicit ScopedLock(const ScopedUnlock& scopedUnlock);

        ScopedLock(const ScopedLock&) = delete;

        ScopedLock& operator=(const ScopedLock&) = delete;

        inline ~ScopedLock();

    private:
        template<typename>
        friend class ConditionVariableBase;
        friend class ScopedUnlock;
        const MutexBase& mutex;
    };

    class ScopedReadLock {
    public:
        inline explicit ScopedReadLock(const MutexBase& mutex);

        inline ~ScopedReadLock();

        ScopedReadLock(const ScopedReadLock&) = delete;

        ScopedReadLock& operator=(const ScopedReadLock&) = delete;

    private:
        template<typename>
        friend class ConditionVariableBase;
        const MutexBase& mutex;
    };

    using ScopedWriteLock = ScopedLock;

    class ScopedUnlock {
    public:
        inline explicit ScopedUnlock(const ScopedLock& scopedLock);
        inline ~ScopedUnlock();

        ScopedUnlock(const ScopedUnlock&) = delete;
        ScopedUnlock& operator=(const ScopedUnlock&) = delete;

    private:
        friend class ScopedLock;
        const MutexBase& mutex;
    };

private:
    template<typename>
    friend class ConditionVariableBase;
    mutable typename Traits::MutexT mutex;
};

template<typename Traits>
class ConditionVariableBase {
public:
    inline ConditionVariableBase();

    inline ~ConditionVariableBase();

    using ScopedLock = typename MutexBase<Traits>::ScopedLock;

    ConditionVariableBase(const ConditionVariableBase&) = delete;
    ConditionVariableBase& operator=(const ConditionVariableBase&) = delete;

    inline void Broadcast(const ScopedLock&);

    inline void Signal(const ScopedLock&);

    inline void Wait(const ScopedLock& scopedLock);

private:
    typename Traits::CondT cond;
};

struct LibuvMutexTraits {
    using CondT = uv_cond_t;
    using MutexT = uv_mutex_t;

    static inline int CondInit(CondT* cond)
    {
        return uv_cond_init(cond);
    }

    static inline int MutexInit(MutexT* mutex)
    {
        return uv_mutex_init(mutex);
    }

    static inline void CondBroadcast(CondT* cond)
    {
        uv_cond_broadcast(cond);
    }

    static inline void CondDestroy(CondT* cond)
    {
        uv_cond_destroy(cond);
    }

    static inline void CondSignal(CondT* cond)
    {
        uv_cond_signal(cond);
    }

    static inline void CondWait(CondT* cond, MutexT* mutex)
    {
        uv_cond_wait(cond, mutex);
    }

    static inline void MutexDestroy(MutexT* mutex)
    {
        uv_mutex_destroy(mutex);
    }

    static inline void MutexLock(MutexT* mutex)
    {
        uv_mutex_lock(mutex);
    }

    static inline void MutexUnlock(MutexT* mutex)
    {
        uv_mutex_unlock(mutex);
    }

    static inline void MutexRdlock(MutexT* mutex)
    {
        uv_mutex_lock(mutex);
    }

    static inline void MutexRdunlock(MutexT* mutex)
    {
        uv_mutex_unlock(mutex);
    }
};

struct LibuvRwlockTraits {
    using MutexT = uv_rwlock_t;

    static inline int MutexInit(MutexT* mutex)
    {
        return uv_rwlock_init(mutex);
    }

    static inline void MutexDestroy(MutexT* mutex)
    {
        uv_rwlock_destroy(mutex);
    }

    static inline void MutexLock(MutexT* mutex)
    {
        uv_rwlock_wrlock(mutex);
    }

    static inline void MutexUnlock(MutexT* mutex)
    {
        uv_rwlock_wrunlock(mutex);
    }

    static inline void MutexRdlock(MutexT* mutex)
    {
        uv_rwlock_rdlock(mutex);
    }

    static inline void MutexRdunlock(MutexT* mutex)
    {
        uv_rwlock_rdunlock(mutex);
    }
};

template<typename Traits>
ConditionVariableBase<Traits>::ConditionVariableBase()
{
    CHECK_EQ(0, Traits::CondInit(&cond));
}

template<typename Traits>
ConditionVariableBase<Traits>::~ConditionVariableBase()
{
    Traits::CondDestroy(&cond);
}

template<typename Traits>
void ConditionVariableBase<Traits>::Broadcast(const ScopedLock&)
{
    Traits::CondBroadcast(&cond);
}

template<typename Traits>
void ConditionVariableBase<Traits>::Signal(const ScopedLock&)
{
    Traits::CondSignal(&cond);
}

template<typename Traits>
void ConditionVariableBase<Traits>::Wait(const ScopedLock& scopedLock)
{
    Traits::CondWait(&cond, &scopedLock.mutex.mutex);
}

template<typename Traits>
MutexBase<Traits>::MutexBase()
{
    CHECK_EQ(0, Traits::MutexInit(&mutex));
}

template<typename Traits>
MutexBase<Traits>::~MutexBase()
{
    Traits::MutexDestroy(&mutex);
}

template<typename Traits>
void MutexBase<Traits>::Lock()
{
    Traits::MutexLock(&mutex);
}

template<typename Traits>
void MutexBase<Traits>::Unlock()
{
    Traits::MutexUnlock(&mutex);
}

template<typename Traits>
void MutexBase<Traits>::RdLock()
{
    Traits::MutexRdlock(&mutex);
}

template<typename Traits>
void MutexBase<Traits>::RdUnlock()
{
    Traits::MutexRdunlock(&mutex);
}

template<typename Traits>
MutexBase<Traits>::ScopedLock::ScopedLock(const MutexBase& mutex) : mutex(mutex)
{
    Traits::MutexLock(&mutex.mutex);
}

template<typename Traits>
MutexBase<Traits>::ScopedLock::ScopedLock(const ScopedUnlock& scopedUnlock) : MutexBase(scopedUnlock.mutex)
{}

template<typename Traits>
MutexBase<Traits>::ScopedLock::~ScopedLock()
{
    Traits::MutexUnlock(&mutex.mutex);
}

template<typename Traits>
MutexBase<Traits>::ScopedReadLock::ScopedReadLock(const MutexBase& mutex) : mutex(mutex)
{
    Traits::MutexRdlock(&mutex.mutex);
}

template<typename Traits>
MutexBase<Traits>::ScopedReadLock::~ScopedReadLock()
{
    Traits::MutexRdunlock(&mutex.mutex);
}

template<typename Traits>
MutexBase<Traits>::ScopedUnlock::ScopedUnlock(const ScopedLock& scopedLock) : mutex(scopedLock.mutex)
{
    Traits::MutexUnlock(&mutex.mutex);
}

template<typename Traits>
MutexBase<Traits>::ScopedUnlock::~ScopedUnlock()
{
    Traits::MutexLock(&mutex.mutex);
}

} // namespace jsvm

#endif // SRC_JSVM_MUTEX_H_
