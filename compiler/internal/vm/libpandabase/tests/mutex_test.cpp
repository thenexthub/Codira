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

#include <pthread.h>

#include "gtest/gtest.h"
#include "os/mutex.h"
#include "os/thread.h"

namespace panda::test {

class MutexTest : public testing::Test {};

constexpr size_t N = 10;
constexpr size_t WRITE_LOCK_SLEEP_TIME = 50;
constexpr size_t READ_LOCK_SLEEP_TIME = 1000;

struct ThreadRWLockArgs {
    os::memory::RWLock *rwlock;
    size_t *index;
};

struct ThreadMutexArgs {
    os::memory::Mutex *lock;
    size_t *index;
};

static void *TestThread1(void *input)
{
    ASSERT(input != nullptr);
    ThreadRWLockArgs *arg = reinterpret_cast<ThreadRWLockArgs *>(input);
    arg->rwlock->WriteLock();
    (*(arg->index))++;
    os::thread::NativeSleep(WRITE_LOCK_SLEEP_TIME);
    arg->rwlock->Unlock();
    return nullptr;
}

static void *TestThread2(void *input)
{
    ASSERT(input != nullptr);
    ThreadRWLockArgs *arg = reinterpret_cast<ThreadRWLockArgs *>(input);
    arg->rwlock->ReadLock();
    os::thread::NativeSleep(READ_LOCK_SLEEP_TIME);
    arg->rwlock->Unlock();
    return nullptr;
}

static void *TestThread3(void *input)
{
    ASSERT(input != nullptr);
    ThreadRWLockArgs *arg = reinterpret_cast<ThreadRWLockArgs *>(input);
    bool res = arg->rwlock->TryReadLock();
    if (res) {
        (*(arg->index))++;
        arg->rwlock->Unlock();
    }

    res = arg->rwlock->TryWriteLock();
    if (res) {
        (*(arg->index))++;
        arg->rwlock->Unlock();
    }
    return nullptr;
}

static void *TestThread4(void *input)
{
    ASSERT(input != nullptr);
    ThreadMutexArgs *arg = reinterpret_cast<ThreadMutexArgs *>(input);
    bool res = arg->lock->TryLockWithSpinning();
    if (res) {
        (*(arg->index))++;
        arg->lock->Unlock();
    }
    return nullptr;
}

static void *TestThread5(void *input)
{
    ASSERT(input != nullptr);
    ThreadRWLockArgs *arg = reinterpret_cast<ThreadRWLockArgs *>(input);
    arg->rwlock->WriteLock();
    arg->rwlock->Unlock();
    return nullptr;
}

HWTEST_F(MutexTest, RWLockTest, testing::ext::TestSize.Level0)
{
    size_t res = 0;
    auto *rwlock = new os::memory::RWLock();
    ThreadRWLockArgs arg = {rwlock, &res};

    pthread_t t[N + N];

    for (size_t i = 0; i < N; i++) {
        pthread_create(&t[i], nullptr, TestThread1, reinterpret_cast<void *>(&arg));
    }
    for (size_t i = 0; i < N; i++) {
        pthread_create(&t[i + N], nullptr, TestThread2, reinterpret_cast<void *>(&arg));
    }

    for (size_t i = 0; i < N; i++) {
        pthread_join(t[i], nullptr);
        pthread_join(t[i + N], nullptr);
    }

    rwlock->WriteLock();
    rwlock->Unlock();

    ASSERT_EQ(res, N);
    delete rwlock;
}

HWTEST_F(MutexTest, TryLockTest, testing::ext::TestSize.Level0)
{
    auto *rwlock = new os::memory::RWLock();
    size_t res = 0;
    ThreadRWLockArgs arg = {rwlock, &res};

    pthread_t t1;
    rwlock->WriteLock();
    pthread_create(&t1, nullptr, TestThread3, reinterpret_cast<void *>(&arg));
    pthread_join(t1, nullptr);
    rwlock->Unlock();
    ASSERT_EQ(res, 0U);

    pthread_t t2;
    rwlock->ReadLock();
    res = 0;
    pthread_create(&t2, nullptr, TestThread3, reinterpret_cast<void *>(&arg));
    pthread_join(t2, nullptr);
    rwlock->Unlock();
    ASSERT_EQ(res, 1U);

    pthread_t t3;
    res = 0;
    pthread_create(&t3, nullptr, TestThread3, reinterpret_cast<void *>(&arg));
    pthread_join(t3, nullptr);
    ASSERT_EQ(res, 2U);
    delete rwlock;
}

HWTEST_F(MutexTest, TryLockWithSpiningTest, testing::ext::TestSize.Level0)
{
    auto *lock = new os::memory::Mutex();
    bool res = lock->TryLockWithSpinning();
    ASSERT_TRUE(res);

    size_t index = 0;
    ThreadMutexArgs arg = {lock, &index};
    pthread_t t;
    pthread_create(&t, nullptr, TestThread4, reinterpret_cast<void *>(&arg));
    pthread_join(t, nullptr);
    ASSERT_EQ(index, 0U);

    if (res) {
        lock->Unlock();
    }
    delete lock;
}

HWTEST_F(MutexTest, LockForOtherTest, testing::ext::TestSize.Level0)
{
    auto *rwlock = new os::memory::RWLock();
    ThreadRWLockArgs arg = {rwlock, nullptr};
    rwlock->WriteLock();
    auto *lock = new os::memory::Mutex();
    pthread_t t;
    pthread_create(&t, nullptr, TestThread5, reinterpret_cast<void *>(&arg));
    lock->LockForOther(t);
    bool res = lock->TryLock();
    if (res) {
        lock->Unlock();
    }
    lock->UnlockForOther(t);
    rwlock->Unlock();
    ASSERT_FALSE(res);
    pthread_join(t, nullptr);
    delete lock;
}

}  // namespace panda
