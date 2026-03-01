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

#include "platforms/unix/libarkbase/futex/fmutex.h"

#include "gtest/gtest.h"
#include "libarkbase/utils/logger.h"

namespace ark {

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
struct ark::os::unix::memory::futex::fmutex g_futex;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
struct ark::os::unix::memory::futex::CondVar g_condvar;
volatile int g_global;

constexpr std::chrono::duration DELAY = std::chrono::milliseconds(10U);

// The thread just modifies shared data under locks
void Writer()
{
    MutexLock(&g_futex, false);
    int newVal = g_global + 1U;
    std::this_thread::sleep_for(DELAY);
    g_global = newVal;
    MutexUnlock(&g_futex);
}

// The thread modifies shared data (meaning it reached Wait function)
// After wake it modifies the shared data again
// There should not be any other writer in parallel
void Waiter()
{
    MutexLock(&g_futex, false);
    g_global++;
    int oldVal = g_global;
    do {
        // spurious wake is possible
        Wait(&g_condvar, &g_futex);
    } while (oldVal == g_global);
    MutexUnlock(&g_futex);
    int newVal = g_global + 1U;
    // Check that waiter is correctly waken
    ASSERT_EQ(newVal, g_global + 1U);
    g_global = newVal;
}

// The thread modifies shared data (meaning it reached Wait function)
// After wake it modifies the shared data again under lock
// There may be other writers in parallel
void Syncwaiter()
{
    MutexLock(&g_futex, false);
    g_global++;
    int oldVal = g_global;
    do {
        // spurious wake is possible
        Wait(&g_condvar, &g_futex);
    } while (oldVal == g_global);
    int newVal = g_global + 1U;
    // Check that waiter is correctly waken
    ASSERT_EQ(newVal, g_global + 1U);
    g_global = newVal;
    MutexUnlock(&g_futex);
}

// The thread modifies shared data (meaning it reached TimedWait function)
// Likely it is not waken by signal and is interrupted by timeout
void Timedwaiter()
{
    MutexLock(&g_futex, false);
    g_global++;
    bool ret = TimedWait(&g_condvar, &g_futex, 1U, 0U, false);
    ASSERT_TRUE(ret);
    if (ret) {
        // Timeout
        g_global++;
        MutexUnlock(&g_futex);
        return;
    }
    MutexUnlock(&g_futex);
}

class FutexTest : public testing::Test {
public:
    FutexTest()
    {
        MutexInit(&g_futex);
        ConditionVariableInit(&g_condvar);
    }

    ~FutexTest() override
    {
        MutexDestroy(&g_futex);
        ConditionVariableDestroy(&g_condvar);
    }

    NO_COPY_SEMANTIC(FutexTest);
    NO_MOVE_SEMANTIC(FutexTest);

protected:
};

// The test checks basic lock protection
// If two values in critical section differ - it is an error
TEST(FutexTest, LockUnlockTest)
{
    g_global = 0;
    std::thread thr(Writer);
    MutexLock(&g_futex, false);
    int val1 = g_global;
    // Wait a bit, to get a chance to Writer
    std::this_thread::sleep_for(DELAY);
    int val2 = g_global;
    MutexUnlock(&g_futex);
    // Check that Writer cannot change GLOBAL in critical section
    ASSERT_EQ(val1, val2);
    thr.join();
    val1 = g_global;
    // Check that both Writer and main correctly incremented in critical section
    ASSERT_EQ(val1, 1U);
}

// The test checks trylock operation
TEST(FutexTest, TrylockTest)
{
    g_global = 0;
    std::thread thr(Writer);
    do {
    } while (!MutexLock(&g_futex, true));
    int newVal = g_global + 1U;
    // Wait a bit, to get a chance to Writer
    std::this_thread::sleep_for(DELAY);
    g_global = newVal;
    MutexUnlock(&g_futex);
    thr.join();
    int val1 = g_global;
    // Check that both Writer and main correctly incremented in critical section
    ASSERT_EQ(val1, 2U);
}

// The test checks work with multiple writers
TEST(FutexTest, MultiWriteTest)
{
    g_global = 0;
    std::thread thr(Writer);
    std::thread thr2(Writer);
    std::thread thr3(Writer);
    std::thread thr4(Writer);
    std::thread thr5(Writer);
    thr.join();
    thr2.join();
    thr3.join();
    thr4.join();
    thr5.join();
    int val1 = g_global;
    // Check that threads correctly incremented in critical section
    ASSERT_EQ(val1, 5U);
}

// The test checks work with recursive futexes
TEST(FutexTest, RecursiveFutexTest)
{
    g_global = 0;
    g_futex.recursiveMutex = true;
    std::thread thr(Writer);
    MutexLock(&g_futex, false);
    MutexLock(&g_futex, false);
    int val1 = g_global;
    MutexUnlock(&g_futex);
    // Wait a bit, to get a chance to Writer
    std::this_thread::sleep_for(DELAY);
    // Read after the first unlock to check, if we are still in critical section
    int val2 = g_global;
    MutexUnlock(&g_futex);
    thr.join();
    // Check that Writer cannot change GLOBAL in recursive critical section
    ASSERT_EQ(val1, val2);
    g_futex.recursiveMutex = false;
}

// The test checks basic wait-notify actions
// We start Waiter, which reaches Wait() function
// Then main send SignalCount
TEST(FutexTest, WaitNotifyTest)
{
    g_global = 0;
    std::thread thr(Waiter);
    bool cond = false;
    do {
        // MutexLock is neccessary to avoid interleaving before Wait()
        MutexLock(&g_futex, false);
        cond = g_global == 1;
        MutexUnlock(&g_futex);
        std::this_thread::sleep_for(DELAY);
    } while (!cond);
    // Waiter reached Wait()

    // Lock for TSAN
    MutexLock(&g_futex, false);
    g_global++;
    MutexUnlock(&g_futex);
    SignalCount(&g_condvar, ark::os::unix::memory::futex::WAKE_ONE);
    thr.join();
    g_global++;
    // Check that waiter is correctly finished
    ASSERT_EQ(g_global, 4U);
}

// The test checks basic timedwait-notify actions
// We start Waiter, which reaches Timedwait() function and then exits by timeout
// Then main send SignalCount
TEST(FutexTest, TimedwaitTest)
{
    g_global = 0;
    std::thread thr(Timedwaiter);
    // Just wait for timeout
    thr.join();
    g_global++;
    // Check that waiter is correctly finished
    ASSERT_EQ(g_global, 3U);
}

// The test checks wait-notifyAll operations in case of multiple waiters
// We start Waiters, which reach Wait() function
// Then main send SignalAll to waiters
TEST(FutexTest, WakeAllTest)
{
    g_global = 0;
    constexpr int NUM = 5;
    std::thread thr(Syncwaiter);
    std::thread thr2(Syncwaiter);
    std::thread thr3(Syncwaiter);
    std::thread thr4(Syncwaiter);
    std::thread thr5(Syncwaiter);

    bool cond = false;
    do {
        // MutexLock is neccessary to avoid interleaving before Wait()
        MutexLock(&g_futex, false);
        cond = g_global == NUM;
        MutexUnlock(&g_futex);
        std::this_thread::sleep_for(DELAY);
    } while (!cond);
    // All threads reached Wait

    // Lock for TSAN
    MutexLock(&g_futex, false);
    g_global++;
    MutexUnlock(&g_futex);
    SignalCount(&g_condvar, ark::os::unix::memory::futex::WAKE_ALL);

    thr.join();
    thr2.join();
    thr3.join();
    thr4.join();
    thr5.join();
    g_global++;
    // Check that waiter is correctly finished by timeout
    ASSERT_EQ(g_global, 2U * NUM + 2U);
}

}  // namespace ark
