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

#pragma clang diagnostic ignored "-Wc11-extensions"

#include <cstdlib>

#include <gtest/gtest.h>
#include <pthread.h>

#define MC_ON
#include "unix/libpandabase/futex/fmutex.cpp"

pthread_t pthread_self(void);
// Copy of mutex storage, after complete implementation should totally replace mutex::current_tid
thread_local pthread_t current_tid;

namespace panda::test {
static struct fmutex g_x;
static int g_shared;

static void *ThreadN(void *arg)
{
    intptr_t index = reinterpret_cast<intptr_t>(arg);

    bool ret;
    do {
        ret = MutexLock(&g_x, true);
    } while (!ret);
    EXPECT_EQ(g_x.recursiveCount, 1);

    g_shared = index;
    int r = g_shared;
    EXPECT_TRUE(r == index);

    MutexUnlock(&g_x);
    EXPECT_EQ(g_x.recursiveCount, 0);
    return nullptr;
}

HWTEST(FMutexTest, TryLockTest, testing::ext::TestSize.Level0)
{
    // The tests check mutex try lock
    constexpr int N = 2;

    MutexInit(&g_x);
    ASSERT_EQ(g_x.recursiveCount, 0);
    ASSERT_EQ(g_x.state_and_waiters_, 0);
    ASSERT_EQ(g_x.exclusive_owner_, 0U);
    ASSERT_FALSE(g_x.recursive_mutex_);

    pthread_t t[N];
    for (long i = 0u; i < N; i++) {
        pthread_create(&t[i], nullptr, ThreadN, reinterpret_cast<void *>(i));
    }

    for (int i = 0u; i < N; i++) {
        pthread_join(t[i], nullptr);
    }

    ASSERT_TRUE(MutexTryLockWithSpinning(&g_x));
    MutexUnlock(&g_x);

    MutexDestroy(&g_x);
    ASSERT_FALSE(MutexDoNotCheckOnDeadlock());
}

}
