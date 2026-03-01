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

#ifndef PANDA_LIBPANDABASE_OS_UNIX_SIGNAL_H_
#define PANDA_LIBPANDABASE_OS_UNIX_SIGNAL_H_

#include <csignal>
#include <functional>
#include <thread>
#include <condition_variable>
#include "libarkbase/macros.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/os/thread.h"
#include "libarkbase/os/failure_retry.h"

namespace ark::os::unix {

class SignalCtl {
public:
    SignalCtl(std::initializer_list<int> signalList = {})  // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        LOG_IF(::sigemptyset(&sigset_) == -1, FATAL, COMMON) << "sigemptyset failed";
        for (int sig : signalList) {
            Add(sig);
        }
    }
    ~SignalCtl() = default;
    NO_MOVE_SEMANTIC(SignalCtl);
    NO_COPY_SEMANTIC(SignalCtl);

    void Add(int sig)
    {
        LOG_IF(::sigaddset(&sigset_, sig) == -1, FATAL, COMMON) << "sigaddset failed";
    }

    void Delete(int sig)
    {
        LOG_IF(::sigdelset(&sigset_, sig) == -1, FATAL, COMMON) << "sigaddset failed";
    }

    bool IsExist(int sig) const
    {
        int ret = ::sigismember(&sigset_, sig);
        LOG_IF(ret == -1, FATAL, COMMON) << "sigismember failed";
        return ret == 1;
    }

    void Block()
    {
        LOG_IF(::pthread_sigmask(SIG_BLOCK, &sigset_, nullptr) != 0, FATAL, COMMON) << "pthread_sigmask failed";
    }

    void Unblock()
    {
        LOG_IF(::pthread_sigmask(SIG_UNBLOCK, &sigset_, nullptr) != 0, FATAL, COMMON) << "pthread_sigmask failed";
    }

    int Wait() const
    {
        int sig = 0;
        LOG_IF(PANDA_FAILURE_RETRY(sigwait(&sigset_, &sig)) != 0, FATAL, COMMON) << "sigwait failed";
        return sig;
    }

    static void GetCurrent(SignalCtl &out)  // NOLINT(google-runtime-references)
    {
        LOG_IF(::pthread_sigmask(SIG_SETMASK, nullptr, &out.sigset_) != 0, FATAL, COMMON) << "pthread_sigmask failed";
    }

private:
    sigset_t sigset_;
};

class SignalCatcherThread {
public:
    SignalCatcherThread(std::initializer_list<int> signalsList = {SIGUSR1}) : signalCtl_(signalsList)
    {
        ASSERT(signalsList.size() > 0);

        // Use the first signal as the stop catcher thread signal
        stopChatcherThreadSignal_ = *signalsList.begin();
    }
    ~SignalCatcherThread() = default;
    NO_MOVE_SEMANTIC(SignalCatcherThread);
    NO_COPY_SEMANTIC(SignalCatcherThread);

    void CatchOnlyCatcherThread()
    {
        ASSERT(catcherThread_ == 0 && "Use CatchOnlyCatcherThread() before StartThread()");
        catchOnlyCatcherThread_ = true;
    }

    void SetupCallbacks(std::function<void()> afterThreadStartCallback, std::function<void()> beforeThreadStopCallback)
    {
        afterThreadStartCallback_ = std::move(afterThreadStartCallback);
        beforeThreadStopCallback_ = std::move(beforeThreadStopCallback);
    }

    void SendSignal(int sig)
    {
        ASSERT(catcherThread_ != 0);
        thread::ThreadSendSignal(catcherThread_, sig);
    }

    template <typename SigAction, typename... Args>
    void StartThread(SigAction *sigAction, Args... args)
    {
        ASSERT(catcherThread_ == 0);
        ASSERT(!isRunning_);

        if (!catchOnlyCatcherThread_) {
            signalCtl_.Block();
        }

        // Start catcher_thread_
        catcherThread_ = thread::ThreadStart(&SignalCatcherThread::Run<SigAction, Args...>, this, sigAction, args...);

        // Wait until the catcher_thread_ is started
        std::unique_lock<std::mutex> cvUniqueLock(cvLock_);
        cv_.wait(cvUniqueLock, [this]() -> bool { return isRunning_; });
    }

    void StopThread()
    {
        ASSERT(catcherThread_ != 0);
        ASSERT(isRunning_);

        // Stop catcher_thread_
        isRunning_ = false;
        SendSignal(stopChatcherThreadSignal_);

        // Wait for catcher_thread_ to finish
        void **retVal = nullptr;
        thread::ThreadJoin(catcherThread_, retVal);
        catcherThread_ = 0;

        if (!catchOnlyCatcherThread_) {
            signalCtl_.Unblock();
        }
    }

private:
    template <typename SigAction, typename... Args>
    static void Run(SignalCatcherThread *self, SigAction *sigAction, Args... args)
    {
        LOG(DEBUG, COMMON) << "SignalCatcherThread::Run: Starting the signal catcher thread";

        if (self->afterThreadStartCallback_ != nullptr) {
            self->afterThreadStartCallback_();
        }

        if (self->catchOnlyCatcherThread_) {
            self->signalCtl_.Block();
        }

        {
            std::lock_guard<std::mutex> lockGuard(self->cvLock_);
            self->isRunning_ = true;
        }
        self->cv_.notify_one();
        while (true) {
            LOG(DEBUG, COMMON) << "SignalCatcherThread::Run: waiting";

            int sig = self->signalCtl_.Wait();
            if (!self->isRunning_) {
                LOG(DEBUG, COMMON) << "SignalCatcherThread::Run: exit loop, cause signal catcher thread was stopped";
                break;
            }

            LOG(DEBUG, COMMON) << "SignalCatcherThread::Run: signal[" << sig << "] handling begins";
            sigAction(sig, args...);
            LOG(DEBUG, COMMON) << "SignalCatcherThread::Run: signal[" << sig << "] handling ends";
        }

        if (self->catchOnlyCatcherThread_) {
            self->signalCtl_.Unblock();
        }

        if (self->beforeThreadStopCallback_ != nullptr) {
            self->beforeThreadStopCallback_();
        }

        LOG(DEBUG, COMMON) << "SignalCatcherThread::Run: Finishing the signal catcher thread";
    }

    std::mutex cvLock_;
    std::condition_variable cv_;

    SignalCtl signalCtl_;
    thread::NativeHandleType catcherThread_ {0};
    int stopChatcherThreadSignal_ {SIGUSR1};
    bool catchOnlyCatcherThread_ {false};
    std::atomic_bool isRunning_ {false};

    std::function<void()> afterThreadStartCallback_ {nullptr};
    std::function<void()> beforeThreadStopCallback_ {nullptr};
};

}  // namespace ark::os::unix

#endif  // PANDA_LIBPANDABASE_OS_UNIX_SIGNAL_H_
