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

#include <gtest/gtest.h>
#include "platforms/unix/libarkbase/signal.h"

namespace ark::os::unix {

bool operator==(const ::ark::os::unix::SignalCtl &l, const ::ark::os::unix::SignalCtl &r)
{
    return 0U == memcmp(&l, &r, sizeof(l));
}

bool operator!=(const ::ark::os::unix::SignalCtl &l, const ::ark::os::unix::SignalCtl &r)
{
    return !(l == r);
}

}  // namespace ark::os::unix

namespace ark::test {

class UnixSignal : public ::testing::Test {
protected:
    static void SetUpTestSuite() {}

    void SetUp() override
    {
        os::unix::SignalCtl::GetCurrent(signalCtl_);
        sigActionCount_ = 0;
    }

    void TearDown() override
    {
        os::unix::SignalCtl signalCtl;
        os::unix::SignalCtl::GetCurrent(signalCtl);
    }

    static void SigAction(int sig, UnixSignal *self)
    {
        self->sigActionCount_ += sig;
    }

    static void Delay()
    {
        usleep(TIME_TO_WAIT);
    }

    void CheckSignalsInsideCycle(int signals)
    {
        uint32_t timeoutCounter = 0;
        while (true) {
            if (sigActionCount_ == signals) {
                break;
            }

            ++timeoutCounter;
            Delay();

            ASSERT_NE(timeoutCounter, maxTimeoutCounterWait_) << "Timeout error: Signals not got in time";
        }
    }

    static const uint32_t TIME_TO_WAIT = 100U * 1000U;  // 0.1 second
                                                        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    const uint32_t maxTimeoutCounterWait_ = 5U * 60U * 1000U * 1000U / TIME_TO_WAIT;  // 5 minutes

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    os::unix::SignalCtl signalCtl_ {};
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    std::atomic_int sigActionCount_ {0U};
};

TEST_F(UnixSignal, CheckRestoringSigSet)
{
    // This check is performed by UnixSignal.TearDown()
}

TEST_F(UnixSignal, CheckRestoringSigSet2)
{
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &sigset, nullptr);

    os::unix::SignalCtl signalCtl;
    os::unix::SignalCtl::GetCurrent(signalCtl);
    ASSERT_NE(signalCtl_, signalCtl);

    pthread_sigmask(SIG_UNBLOCK, &sigset, nullptr);
}

TEST_F(UnixSignal, StartStopThread)
{
    os::unix::SignalCatcherThread catcherThread;

    catcherThread.StartThread(&UnixSignal::SigAction, this);
    catcherThread.StopThread();
}

TEST_F(UnixSignal, StartStopThreadCatchOnlyCatcherThread)
{
    os::unix::SignalCatcherThread catcherThread;
    catcherThread.CatchOnlyCatcherThread();

    catcherThread.StartThread(&UnixSignal::SigAction, this);
    catcherThread.StopThread();
}

TEST_F(UnixSignal, SendSignalsToMainThread)
{
    os::unix::SignalCatcherThread catcherThread({SIGUSR1, SIGUSR2});
    catcherThread.StartThread(&UnixSignal::SigAction, this);

    // Send signals to main thread
    kill(getpid(), SIGUSR1);
    kill(getpid(), SIGUSR2);

    // Wait for the signals inside the cycle
    CheckSignalsInsideCycle(SIGUSR1 + SIGUSR2);
    catcherThread.StopThread();

    ASSERT_EQ(sigActionCount_, SIGUSR1 + SIGUSR2);
}

// The test fails on WSL
#ifndef PANDA_TARGET_WSL
TEST_F(UnixSignal, SendSignalsToMainThread2)
{
    EXPECT_DEATH(
        {
            os::unix::SignalCatcherThread catcherThread({SIGUSR1});
            catcherThread.CatchOnlyCatcherThread();

            catcherThread.StartThread(&UnixSignal::SigAction, this);

            // Send signals to main thread
            catcherThread.SendSignal(SIGUSR1);
            CheckSignalsInsideCycle(SIGUSR1);

            // After kill process must die
            kill(getpid(), SIGUSR1);
            CheckSignalsInsideCycle(SIGUSR1 + SIGUSR1);
            catcherThread.StopThread();

            ASSERT_EQ(true, false) << "Error: Thread must die before this assert";
        },
        "");
}
#endif  // PANDA_TARGET_WSL

TEST_F(UnixSignal, SendSignalsToCatcherThread)
{
    os::unix::SignalCatcherThread catcherThread({SIGUSR1, SIGUSR2});
    catcherThread.CatchOnlyCatcherThread();

    catcherThread.StartThread(&UnixSignal::SigAction, this);

    // Send signals to catcher thread
    catcherThread.SendSignal(SIGUSR1);
    catcherThread.SendSignal(SIGUSR2);

    // Wait for the signals inside the cycle
    CheckSignalsInsideCycle(SIGUSR1 + SIGUSR2);
    catcherThread.StopThread();

    ASSERT_EQ(sigActionCount_, SIGUSR1 + SIGUSR2);
}

TEST_F(UnixSignal, SendUnhandledSignal)
{
    EXPECT_DEATH(
        {
            os::unix::SignalCatcherThread catcherThread({SIGUSR1});
            catcherThread.StartThread(&UnixSignal::SigAction, this);

            // Send signals to catcher thread
            catcherThread.SendSignal(SIGUSR1);
            catcherThread.SendSignal(SIGUSR2);

            // Wait for the signals inside the cycle
            CheckSignalsInsideCycle(SIGUSR1 + SIGUSR2);
            catcherThread.StopThread();

            ASSERT_EQ(sigActionCount_, SIGUSR1 + SIGUSR2);
        },
        "");
}

TEST_F(UnixSignal, SendSomeSignalsToCatcherThread)
{
    os::unix::SignalCatcherThread catcherThread({SIGQUIT, SIGUSR1, SIGUSR2});
    catcherThread.CatchOnlyCatcherThread();

    catcherThread.StartThread(&UnixSignal::SigAction, this);

    // Send signals to catcher thread
    catcherThread.SendSignal(SIGQUIT);
    catcherThread.SendSignal(SIGUSR1);
    catcherThread.SendSignal(SIGUSR2);

    // Wait for the signals inside the cycle
    CheckSignalsInsideCycle(SIGQUIT + SIGUSR1 + SIGUSR2);
    catcherThread.StopThread();

    ASSERT_EQ(sigActionCount_, SIGQUIT + SIGUSR1 + SIGUSR2);
}

TEST_F(UnixSignal, AfterThreadStartCallback)
{
    os::unix::SignalCatcherThread catcherThread({SIGQUIT});

    // NOLINTNEXTLINE(readability-magic-numbers)
    catcherThread.SetupCallbacks([this]() { sigActionCount_ += 1000U; }, nullptr);

    catcherThread.StartThread(&UnixSignal::SigAction, this);

    // Send signals to catcher thread
    catcherThread.SendSignal(SIGQUIT);

    // Wait for the signals inside the cycle
    // NOLINTNEXTLINE(readability-magic-numbers)
    CheckSignalsInsideCycle(SIGQUIT + 1000U);
    catcherThread.StopThread();

    ASSERT_EQ(sigActionCount_, SIGQUIT + 1000U);
}

TEST_F(UnixSignal, BeforeThreadStopCallback)
{
    os::unix::SignalCatcherThread catcherThread({SIGQUIT});

    // NOLINTNEXTLINE(readability-magic-numbers)
    catcherThread.SetupCallbacks(nullptr, [this]() { sigActionCount_ += 2000U; });

    catcherThread.StartThread(&UnixSignal::SigAction, this);

    // Send signals to catcher thread
    catcherThread.SendSignal(SIGQUIT);

    // Wait for the signals inside the cycle
    CheckSignalsInsideCycle(SIGQUIT);
    catcherThread.StopThread();

    ASSERT_EQ(sigActionCount_, SIGQUIT + 2000U);
}

TEST_F(UnixSignal, ThreadCallbacks)
{
    os::unix::SignalCatcherThread catcherThread({SIGQUIT});

    // NOLINTNEXTLINE(readability-magic-numbers)
    catcherThread.SetupCallbacks([this]() { sigActionCount_ += 1000U; }, [this]() { sigActionCount_ += 2000U; });

    catcherThread.StartThread(&UnixSignal::SigAction, this);

    // Send signals to catcher thread
    catcherThread.SendSignal(SIGQUIT);

    // Wait for the signals inside the cycle
    // NOLINTNEXTLINE(readability-magic-numbers)
    CheckSignalsInsideCycle(1000U + SIGQUIT);
    catcherThread.StopThread();

    ASSERT_EQ(sigActionCount_, 1000U + SIGQUIT + 2000U);
}

}  // namespace ark::test
