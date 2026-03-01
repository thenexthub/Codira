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

#include "libarkbase/os/mutex.h"
#include <gtest/gtest.h>
#include <array>
#include <cstdint>

namespace ark::os::memory::test {

class ScopedLockTest : public testing::Test {};

constexpr uint32_t N_ITERATIONS = 20;

template <class... Locks>
static void LockMutexesAndIncrement(uint32_t &var, Locks &...locks)
{
    constexpr uint32_t N {4};
    for (uint32_t i = 0; i < N; ++i) {
        os::memory::ScopedLock l(locks...);
        ++var;
    }
}

template <class... Locks>
static void LockWriteMutexesAndIncrement(uint32_t &var, Locks &...locks)
{
    constexpr uint32_t N {4};
    for (uint32_t i = 0; i < N; ++i) {
        os::memory::WriteScopedLock l(locks...);
        ++var;
    }
}

template <class... Locks>
static void LockReadMutexes(Locks &...locks)
{
    constexpr uint32_t N {4};
    for (uint32_t i = 0; i < N; ++i) {
        os::memory::ReadScopedLock l(locks...);
    }
}

TEST_F(ScopedLockTest, OneLockTest)
{
    constexpr uint32_t TEST_THREADS = 2;
    uint32_t var = 0;
    std::array<std::thread, TEST_THREADS> threads;
    Mutex lock;
    for (uint32_t i = 0; i < TEST_THREADS; ++i) {
        threads[i] = std::thread(LockMutexesAndIncrement<Mutex>, std::ref(var), std::ref(lock));
    }

    for (uint32_t i = 0; i < TEST_THREADS; ++i) {
        threads[i].join();
    }

    ASSERT_EQ(var, 8U);
}

TEST_F(ScopedLockTest, TwoLockTest)
{
    constexpr uint32_t TEST_THREADS = 2;

    std::array<std::thread, TEST_THREADS> threads;
    Mutex lock0;
    Mutex lock1;

    for (uint32_t i = 0; i < N_ITERATIONS; ++i) {
        uint32_t var = 0;
        threads[0U] =
            std::thread(LockMutexesAndIncrement<Mutex, Mutex>, std::ref(var), std::ref(lock0), std::ref(lock1));
        threads[1U] =
            std::thread(LockMutexesAndIncrement<Mutex, Mutex>, std::ref(var), std::ref(lock1), std::ref(lock0));

        for (uint32_t j = 0; j < TEST_THREADS; ++j) {
            threads[j].join();
        }

        ASSERT_EQ(var, 8U);
    }

    RecursiveMutex recursiveLock;

    for (uint32_t i = 0; i < N_ITERATIONS; ++i) {
        uint32_t var = 0;
        threads[0U] = std::thread(LockMutexesAndIncrement<Mutex, RecursiveMutex>, std::ref(var), std::ref(lock0),
                                  std::ref(recursiveLock));
        threads[1U] = std::thread(LockMutexesAndIncrement<RecursiveMutex, Mutex>, std::ref(var),
                                  std::ref(recursiveLock), std::ref(lock0));

        for (uint32_t j = 0; j < TEST_THREADS; ++j) {
            threads[j].join();
        }

        ASSERT_EQ(var, 8U);
    }
}

TEST_F(ScopedLockTest, ThreeLockTest)
{
    constexpr uint32_t TEST_THREADS = 6;

    std::array<std::thread, TEST_THREADS> threads;
    Mutex lock0;
    Mutex lock1;
    RecursiveMutex recursiveLock;

    for (uint32_t i = 0; i < N_ITERATIONS; ++i) {
        uint32_t var = 0;
        threads[0U] = std::thread(LockMutexesAndIncrement<Mutex, Mutex, RecursiveMutex>, std::ref(var), std::ref(lock0),
                                  std::ref(lock1), std::ref(recursiveLock));
        threads[1U] = std::thread(LockMutexesAndIncrement<Mutex, RecursiveMutex, Mutex>, std::ref(var), std::ref(lock0),
                                  std::ref(recursiveLock), std::ref(lock1));
        threads[2U] = std::thread(LockMutexesAndIncrement<Mutex, Mutex, RecursiveMutex>, std::ref(var), std::ref(lock1),
                                  std::ref(lock0), std::ref(recursiveLock));
        threads[3U] = std::thread(LockMutexesAndIncrement<Mutex, RecursiveMutex, Mutex>, std::ref(var), std::ref(lock1),
                                  std::ref(recursiveLock), std::ref(lock0));
        threads[4U] = std::thread(LockMutexesAndIncrement<RecursiveMutex, Mutex, Mutex>, std::ref(var),
                                  std::ref(recursiveLock), std::ref(lock0), std::ref(lock1));
        threads[5U] = std::thread(LockMutexesAndIncrement<RecursiveMutex, Mutex, Mutex>, std::ref(var),
                                  std::ref(recursiveLock), std::ref(lock1), std::ref(lock0));

        for (uint32_t j = 0; j < TEST_THREADS; ++j) {
            threads[j].join();
        }

        ASSERT_EQ(var, 24U);
    }
}

TEST_F(ScopedLockTest, TwoWriteLockTest)
{
    constexpr uint32_t TEST_THREADS = 2;

    std::array<std::thread, TEST_THREADS> threads;
    RWLock lock0;
    RWLock lock1;

    for (uint32_t i = 0; i < N_ITERATIONS; ++i) {
        uint32_t var = 0;
        threads[0U] =
            std::thread(LockWriteMutexesAndIncrement<RWLock, RWLock>, std::ref(var), std::ref(lock0), std::ref(lock1));
        threads[1U] =
            std::thread(LockWriteMutexesAndIncrement<RWLock, RWLock>, std::ref(var), std::ref(lock1), std::ref(lock0));

        for (uint32_t j = 0; j < TEST_THREADS; ++j) {
            threads[j].join();
        }

        ASSERT_EQ(var, 8U);
    }
}

TEST_F(ScopedLockTest, ThreeWriteLockTest)
{
    constexpr uint32_t TEST_THREADS = 6;

    std::array<std::thread, TEST_THREADS> threads;
    RWLock lock0;
    RWLock lock1;
    RWLock lock2;

    for (uint32_t i = 0; i < N_ITERATIONS; ++i) {
        uint32_t var = 0;
        threads[0U] = std::thread(LockWriteMutexesAndIncrement<RWLock, RWLock, RWLock>, std::ref(var), std::ref(lock0),
                                  std::ref(lock1), std::ref(lock2));
        threads[1U] = std::thread(LockWriteMutexesAndIncrement<RWLock, RWLock, RWLock>, std::ref(var), std::ref(lock0),
                                  std::ref(lock2), std::ref(lock1));
        threads[2U] = std::thread(LockWriteMutexesAndIncrement<RWLock, RWLock, RWLock>, std::ref(var), std::ref(lock1),
                                  std::ref(lock0), std::ref(lock2));
        threads[3U] = std::thread(LockWriteMutexesAndIncrement<RWLock, RWLock, RWLock>, std::ref(var), std::ref(lock1),
                                  std::ref(lock2), std::ref(lock0));
        threads[4U] = std::thread(LockWriteMutexesAndIncrement<RWLock, RWLock, RWLock>, std::ref(var), std::ref(lock2),
                                  std::ref(lock0), std::ref(lock1));
        threads[5U] = std::thread(LockWriteMutexesAndIncrement<RWLock, RWLock, RWLock>, std::ref(var), std::ref(lock2),
                                  std::ref(lock1), std::ref(lock0));

        for (uint32_t j = 0; j < TEST_THREADS; ++j) {
            threads[j].join();
        }

        ASSERT_EQ(var, 24U);
    }
}

TEST_F(ScopedLockTest, TwoReadLockTest)
{
    constexpr uint32_t TEST_THREADS = 2;

    std::array<std::thread, TEST_THREADS> threads;
    RWLock lock0;
    RWLock lock1;

    for (uint32_t i = 0; i < N_ITERATIONS; ++i) {
        threads[0U] = std::thread(LockReadMutexes<RWLock, RWLock>, std::ref(lock0), std::ref(lock1));
        threads[1U] = std::thread(LockReadMutexes<RWLock, RWLock>, std::ref(lock1), std::ref(lock0));

        for (uint32_t j = 0; j < TEST_THREADS; ++j) {
            threads[j].join();
        }
    }
}

TEST_F(ScopedLockTest, ThreeReadLockTest)
{
    constexpr uint32_t TEST_THREADS = 6;

    std::array<std::thread, TEST_THREADS> threads;
    RWLock lock0;
    RWLock lock1;
    RWLock lock2;

    for (uint32_t i = 0; i < N_ITERATIONS; ++i) {
        threads[0U] =
            std::thread(LockReadMutexes<RWLock, RWLock, RWLock>, std::ref(lock0), std::ref(lock1), std::ref(lock2));
        threads[1U] =
            std::thread(LockReadMutexes<RWLock, RWLock, RWLock>, std::ref(lock0), std::ref(lock2), std::ref(lock1));
        threads[2U] =
            std::thread(LockReadMutexes<RWLock, RWLock, RWLock>, std::ref(lock1), std::ref(lock0), std::ref(lock2));
        threads[3U] =
            std::thread(LockReadMutexes<RWLock, RWLock, RWLock>, std::ref(lock1), std::ref(lock2), std::ref(lock0));
        threads[4U] =
            std::thread(LockReadMutexes<RWLock, RWLock, RWLock>, std::ref(lock2), std::ref(lock0), std::ref(lock1));
        threads[5U] =
            std::thread(LockReadMutexes<RWLock, RWLock, RWLock>, std::ref(lock2), std::ref(lock1), std::ref(lock0));

        for (uint32_t j = 0; j < TEST_THREADS; ++j) {
            threads[j].join();
        }
    }
}

TEST_F(ScopedLockTest, TestLockCapability)
{
    Mutex lock0;
    Mutex lock1;
    RecursiveMutex recursiveLock;
    RWLock rwLock0;
    RWLock rwLock1;

    {
        ScopedLock scopedLock(lock0);
        std::thread thread {[&lock0] { ASSERT_FALSE(lock0.TryLock()); }};
        thread.join();
    }

    {
        ScopedLock scopedLock(lock0, recursiveLock, lock1);
        std::thread thread {[&lock0, &recursiveLock, &lock1] {
            ASSERT_FALSE(lock0.TryLock());
            ASSERT_FALSE(lock1.TryLock());
            ASSERT_FALSE(recursiveLock.TryLock());
        }};
        thread.join();
    }

    {
        ReadScopedLock scopedReadLock(rwLock0, rwLock1);

        std::thread writeThread {[&rwLock0, &rwLock1] {
            ASSERT_FALSE(rwLock0.TryWriteLock());
            ASSERT_FALSE(rwLock1.TryWriteLock());
        }};

        std::thread readThread {[&rwLock0, &rwLock1] { ReadScopedLock readLock(rwLock0, rwLock1); }};
        writeThread.join();
        readThread.join();
    }

    {
        WriteScopedLock scopedWriteLock(rwLock0, rwLock1);

        std::thread writeThread {[&rwLock0, &rwLock1] {
            ASSERT_FALSE(rwLock0.TryWriteLock());
            ASSERT_FALSE(rwLock1.TryWriteLock());
        }};

        std::thread readThread {[&rwLock0, &rwLock1] {
            ASSERT_FALSE(rwLock0.TryReadLock());
            ASSERT_FALSE(rwLock1.TryReadLock());
        }};

        writeThread.join();
        readThread.join();
    }
}

}  // namespace ark::os::memory::test
