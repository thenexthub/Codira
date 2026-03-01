/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_THREAD_PROXY_HYBRID_H
#define PANDA_RUNTIME_THREAD_PROXY_HYBRID_H

#ifdef ARK_HYBRID

#include "runtime/include/thread_interface.h"

#include "common_interfaces/thread/base_thread.h"

namespace ark {

class ThreadProxyHybrid : public ThreadInterface {
public:
    explicit ThreadProxyHybrid(MutatorLock *mutatorLock) : ThreadInterface(mutatorLock) {}

    bool TestAllFlags() const;

    enum ThreadStatus GetStatus() const;

    void InitializeThreadFlag();

    void UpdateStatus(enum ThreadStatus status) NO_THREAD_SAFETY_ANALYSIS;

    bool IsSuspended() const;

    bool IsRuntimeTerminated() const;

    void SetRuntimeTerminated();

    void SuspendCheck();

    void SuspendImpl(bool internalSuspend = false);

    void ResumeImpl(bool internalResume = false);

    void SafepointPoll();

    bool IsUserSuspended();

    void WaitSuspension();

    void MakeTSANHappyForThreadState();

    void CleanUpThreadStatus();

    static constexpr uint32_t GetFlagOffset()
    {
        return 0;
    };

    static void InitializeInitThreadFlag() {};

    void BindMutator();

    void UnbindMutator();

    /// @returns true if holder was created
    bool CreateExternalHolderIfNeeded(bool useSharedHolder);

    common::ThreadHolder *GetThreadHolder() const
    {
        ASSERT(threadHolder_ != nullptr);
        return threadHolder_;
    }

private:
    static void SetSharedExternalHolder(common::ThreadHolder *externalHolder);
    static common::ThreadHolder *GetSharedExternalHolder();

    common::ThreadHolder *threadHolder_ = nullptr;
};

}  // namespace ark

#endif

#endif  // PANDA_RUNTIME_THREAD_PROXY_HYBRID_H
