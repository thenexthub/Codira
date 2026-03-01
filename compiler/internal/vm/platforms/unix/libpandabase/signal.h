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

#ifndef PLATFORMS_UNIX_LIBPANDABASE_SIGNAL_H
#define PLATFORMS_UNIX_LIBPANDABASE_SIGNAL_H

#include <csignal>
#include <functional>
#include <thread>
#include <condition_variable>
#include "libpandabase/macros.h"
#include "libpandabase/utils/logger.h"
#include "libpandabase/os/thread.h"
#include "libpandabase/os/failure_retry.h"

namespace panda::os::unix {

class SignalCtl {
public:
    SignalCtl(std::initializer_list<int> signal_list = {})  // NOLINT(cppcoreguidelines-pro-type-member-init)
    {
        LOG_IF(::sigemptyset(&sigset_) == -1, FATAL, COMMON) << "sigemptyset failed";
        for (int sig : signal_list) {
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
        LOG_IF(::pthread_sigmask(SIG_BLOCK, &sigset_, nullptr) == -1, FATAL, COMMON) << "pthread_sigmask failed";
    }

    void Unblock()
    {
        LOG_IF(::pthread_sigmask(SIG_UNBLOCK, &sigset_, nullptr) == -1, FATAL, COMMON) << "pthread_sigmask failed";
    }

    int Wait() const
    {
        int sig = 0;
        LOG_IF(PANDA_FAILURE_RETRY(sigwait(&sigset_, &sig)) == -1, FATAL, COMMON) << "sigwait failed";
        return sig;
    }

    static void GetCurrent(SignalCtl &out)  // NOLINT(google-runtime-references)
    {
        LOG_IF(::pthread_sigmask(SIG_SETMASK, nullptr, &out.sigset_) == -1, FATAL, COMMON) << "pthread_sigmask failed";
    }

private:
    sigset_t sigset_;
};

class SignalCatcherThread {
public:
    SignalCatcherThread(std::initializer_list<int> signals_list = {SIGUSR1}) : signal_ctl_(signals_list)
    {
        ASSERT(signals_list.size() > 0);

        // Use the first signal as the stop catcher thread signal
        stop_chatcher_thread_signal_ = *signals_list.begin();
    }
    ~SignalCatcherThread() = default;
    NO_MOVE_SEMANTIC(SignalCatcherThread);
    NO_COPY_SEMANTIC(SignalCatcherThread);

    void CatchOnlyCatcherThread()
    {
        ASSERT(catcher_thread_ == 0 && "Use CatchOnlyCatcherThread() before StartThread()");
        catch_only_catcher_thread_ = true;
    }

    void SetupCallbacks(std::function<void()> after_thread_start_callback,
                        std::function<void()> before_thread_stop_callback)
    {
        after_thread_start_callback_ = std::move(after_thread_start_callback);
        before_thread_stop_callback_ = std::move(before_thread_stop_callback);
    }

    void SendSignal(int sig)
    {
        ASSERT(catcher_thread_ != 0);
        thread::ThreadSendSignal(catcher_thread_, sig);
    }

    template <typename SigAction, typename... Args>
    void StartThread(SigAction *sig_action, Args... args)
    {
        ASSERT(catcher_thread_ == 0);
        ASSERT(!is_running_);

        if (!catch_only_catcher_thread_) {
            signal_ctl_.Block();
        }

        // Start catcher_thread_
        catcher_thread_ = thread::ThreadStart(&SignalCatcherThread::Run<SigAction, Args...>, this, sig_action, args...);

        // Wait until the catcher_thread_ is started
        std::unique_lock<std::mutex> cv_unique_lock(cv_lock_);
        cv_.wait(cv_unique_lock, [this]() -> bool { return is_running_; });
    }

    void StopThread()
    {
        ASSERT(catcher_thread_ != 0);
        ASSERT(is_running_);

        // Stop catcher_thread_
        is_running_ = false;
        SendSignal(stop_chatcher_thread_signal_);

        // Wait for catcher_thread_ to finish
        void **ret_val = nullptr;
        thread::ThreadJoin(catcher_thread_, ret_val);
        catcher_thread_ = 0;

        if (!catch_only_catcher_thread_) {
            signal_ctl_.Unblock();
        }
    }

private:
    template <typename SigAction, typename... Args>
    static void Run(SignalCatcherThread *self, SigAction *sig_action, Args... args)
    {
        LOG(DEBUG, COMMON) << "SignalCatcherThread::Run: Starting the signal catcher thread";

        if (self->after_thread_start_callback_ != nullptr) {
            self->after_thread_start_callback_();
        }

        if (self->catch_only_catcher_thread_) {
            self->signal_ctl_.Block();
        }

        {
            std::lock_guard<std::mutex> lock_guard(self->cv_lock_);
            self->is_running_ = true;
        }
        self->cv_.notify_one();
        while (true) {
            LOG(DEBUG, COMMON) << "SignalCatcherThread::Run: waiting";

            int sig = self->signal_ctl_.Wait();
            if (!self->is_running_) {
                LOG(DEBUG, COMMON) << "SignalCatcherThread::Run: exit loop, cause signal catcher thread was stopped";
                break;
            }

            LOG(DEBUG, COMMON) << "SignalCatcherThread::Run: signal[" << sig << "] handling begins";
            sig_action(sig, args...);
            LOG(DEBUG, COMMON) << "SignalCatcherThread::Run: signal[" << sig << "] handling ends";
        }

        if (self->catch_only_catcher_thread_) {
            self->signal_ctl_.Unblock();
        }

        if (self->before_thread_stop_callback_ != nullptr) {
            self->before_thread_stop_callback_();
        }

        LOG(DEBUG, COMMON) << "SignalCatcherThread::Run: Finishing the signal catcher thread";
    }

    std::mutex cv_lock_;
    std::condition_variable cv_;

    SignalCtl signal_ctl_;
    thread::native_handle_type catcher_thread_ {0};
    int stop_chatcher_thread_signal_ {SIGUSR1};
    bool catch_only_catcher_thread_ {false};
    std::atomic_bool is_running_ {false};

    std::function<void()> after_thread_start_callback_ {nullptr};
    std::function<void()> before_thread_stop_callback_ {nullptr};
};

}  // namespace panda::os::unix

#endif  // PLATFORMS_UNIX_LIBPANDABASE_SIGNAL_H
