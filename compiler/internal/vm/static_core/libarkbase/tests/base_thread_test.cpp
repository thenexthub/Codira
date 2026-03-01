/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gtest/gtest.h"
#include "libarkbase/os/thread.h"
#include "libarkbase/os/mutex.h"

namespace ark::os::thread {
class ThreadTest : public testing::Test {};

uint32_t g_curThreadId = 0;
bool g_updated = false;
bool g_operated = false;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
os::memory::Mutex g_mu;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
os::memory::ConditionVariable g_cv;

#ifdef PANDA_TARGET_UNIX
constexpr int LOWER_PRIOIRITY = 1;
#elif defined(PANDA_TARGET_WINDOWS)
constexpr int LOWER_PRIOIRITY = -1;
#endif

void ThreadFunc()
{
    g_curThreadId = GetCurrentThreadId();
    {
        os::memory::LockHolder lk(g_mu);
        g_updated = true;
    }
    g_cv.Signal();
    {
        // wait for the main thread to Set/GetPriority
        os::memory::LockHolder lk(g_mu);
        while (!g_operated) {
            g_cv.Wait(&g_mu);
        }
    }
}

TEST_F(ThreadTest, SetCurrentThreadPriorityTest)
{
    // Since setting higher priority needs "sudo" right, we only test lower one here.
    auto ret1 = SetPriority(GetCurrentThreadId(), LOWER_PRIOIRITY);
    auto prio1 = GetPriority(GetCurrentThreadId());
    ASSERT_EQ(prio1, LOWER_PRIOIRITY);

    auto ret2 = SetPriority(GetCurrentThreadId(), LOWEST_PRIORITY);
    auto prio2 = GetPriority(GetCurrentThreadId());
    ASSERT_EQ(prio2, LOWEST_PRIORITY);

#ifdef PANDA_TARGET_UNIX
    ASSERT_EQ(ret1, 0U);
    ASSERT_EQ(ret2, 0U);
#elif defined(PANDA_TARGET_WINDOWS)
    ASSERT_NE(ret1, 0U);
    ASSERT_NE(ret2, 0U);
#endif
}

TEST_F(ThreadTest, SetOtherThreadPriorityTest)
{
    auto parentPid = GetCurrentThreadId();
    auto parentPrioBefore = GetPriority(parentPid);

    auto newThread = ThreadStart(ThreadFunc);
    // wait for the new_thread to update CUR_THREAD_ID
    g_mu.Lock();
    while (!g_updated) {
        g_cv.Wait(&g_mu);
    }
    auto childPid = g_curThreadId;

    auto childPrioBefore = GetPriority(childPid);
    (void)childPrioBefore;
    auto ret = SetPriority(childPid, LOWEST_PRIORITY);

    auto childPrioAfter = GetPriority(childPid);
    (void)childPrioAfter;
    auto parentPrioAfter = GetPriority(parentPid);

    g_operated = true;
    g_mu.Unlock();
    g_cv.Signal();
    void *res;
    ThreadJoin(newThread, &res);

    ASSERT_EQ(parentPrioBefore, parentPrioAfter);
#ifdef PANDA_TARGET_UNIX
    ASSERT_EQ(ret, 0U);
    ASSERT(childPrioBefore <= childPrioAfter);
#elif defined(PANDA_TARGET_WINDOWS)
    ASSERT_NE(ret, 0U);
    ASSERT(childPrioAfter <= childPrioBefore);
#endif
}
}  // namespace ark::os::thread
