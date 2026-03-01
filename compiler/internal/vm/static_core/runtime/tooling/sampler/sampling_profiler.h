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

#ifndef PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLING_PROFILER_H
#define PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLING_PROFILER_H

#include <atomic>
#include <csignal>

#include "libarkbase/macros.h"
#include "libarkbase/os/pipe.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/mem/panda_containers.h"

#include "runtime/tooling/sampler/sample_info.h"
#include "runtime/tooling/sampler/sample_writer.h"
#include "runtime/tooling/sampler/samples_record.h"
#include "runtime/tooling/sampler/thread_communicator.h"
#include "runtime/tooling/sampler/lock_free_queue.h"

namespace ark::tooling::sampler {

namespace test {
class SamplerTest;
}  // namespace test

// Panda sampling profiler
class Sampler final : public RuntimeListener {
public:
    ~Sampler() override = default;

    static PANDA_PUBLIC_API Sampler *Create();
    static PANDA_PUBLIC_API void Destroy(Sampler *sampler);

    // Need to get comunicator inside the signal handler
    static const ThreadCommunicator &GetSampleCommunicator()
    {
        ASSERT(instance_ != nullptr);
        return instance_->GetCommunicator();
    }

    const ThreadCommunicator &GetCommunicator() const
    {
        return communicator_;
    }

    static const LockFreeQueue &GetSampleQueuePF()
    {
        ASSERT(instance_ != nullptr);
        return instance_->GetQueuePF();
    }

    const LockFreeQueue &GetQueuePF()
    {
        return loadedPfsQueue_;
    }

    void SetSampleInterval(uint32_t us)
    {
        // Atomic with acquire order reason: To ensure start/stop load correctly
        ASSERT(isActive_.load(std::memory_order_acquire) == false);
        sampleInterval_ = static_cast<std::chrono::microseconds>(us);
    }

    void SetSegvHandlerStatus(bool segvHandlerStatus)
    {
        isSegvHandlerEnable_ = segvHandlerStatus;
    }

    bool IsSegvHandlerEnable() const
    {
        return isSegvHandlerEnable_;
    }

    PANDA_PUBLIC_API bool Start(std::unique_ptr<StreamWriter> &&writer);
    PANDA_PUBLIC_API bool Stop();

    // Events: Notify profiler that managed thread created or finished
    void ThreadStart(ManagedThread *managedThread) override;
    void ThreadEnd(ManagedThread *managedThread) override;
    void LoadModule(std::string_view name) override;
    static PANDA_PUBLIC_API uint64_t GetMicrosecondsTimeStamp();

    static constexpr uint32_t DEFAULT_SAMPLE_INTERVAL_US = 500;

private:
    explicit Sampler();

    void SamplerThreadEntry();
    void ListenerThreadEntry(std::unique_ptr<StreamWriter> writerPtr);

    void AddThreadHandle(ManagedThread *thread);
    void EraseThreadHandle(ManagedThread *thread);

    void CollectThreads();
    void CollectModules();

    void WriteLoadedPandaFiles(StreamWriter *writerPtr);

    void ClearManagedThreadSet()
    {
        os::memory::LockHolder holder(managedThreadsLock_);
        managedThreads_.clear();
    }

    void ClearLoadedPfs()
    {
        os::memory::LockHolder holder(loadedPfsLock_);
        loadedPfs_.clear();
    }

    static Sampler *instance_;

    Runtime *runtime_ {nullptr};
    // Remember agent thread id for security
    os::thread::NativeHandleType listenerTid_ {0};
    os::thread::NativeHandleType samplerTid_ {0};
    std::unique_ptr<std::thread> samplerThread_ {nullptr};
    std::unique_ptr<std::thread> listenerThread_ {nullptr};
    ThreadCommunicator communicator_;

    std::atomic<bool> isActive_ {false};
    bool isSegvHandlerEnable_ {true};

    PandaSet<os::thread::ThreadId> managedThreads_ GUARDED_BY(managedThreadsLock_);
    os::memory::Mutex managedThreadsLock_;

    LockFreeQueue loadedPfsQueue_;

    PandaVector<FileInfo> loadedPfs_ GUARDED_BY(loadedPfsLock_);
    os::memory::Mutex loadedPfsLock_;

    std::chrono::microseconds sampleInterval_;

    friend class test::SamplerTest;

    NO_COPY_SEMANTIC(Sampler);
    NO_MOVE_SEMANTIC(Sampler);
};

}  // namespace ark::tooling::sampler

#endif  // PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLING_PROFILER_H
