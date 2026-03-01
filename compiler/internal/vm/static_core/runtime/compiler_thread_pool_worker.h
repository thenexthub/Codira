/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#ifndef RUTNIME_COMPILER_THREAD_POOL_WORKER_H
#define RUTNIME_COMPILER_THREAD_POOL_WORKER_H

#include "runtime/compiler_worker.h"
#include "runtime/thread_pool.h"
#include "runtime/compiler_queue_interface.h"

namespace ark {

class CompilerProcessor : public ProcessorInterface<CompilerTask, Compiler *> {
public:
    explicit CompilerProcessor(Compiler *compiler) : compiler_(compiler) {}
    bool Process(CompilerTask &&task) override;

private:
    void InPlaceCompileMethod(CompilerTask &&ctx);
    Compiler *compiler_;
};

/// @brief Compiler worker task pool based on ThreadPool
class CompilerThreadPoolWorker : public CompilerWorker {
public:
    CompilerThreadPoolWorker(mem::InternalAllocatorPtr internalAllocator, Compiler *compiler, bool &noAsyncJit,
                             const RuntimeOptions &options);
    NO_COPY_SEMANTIC(CompilerThreadPoolWorker);
    NO_MOVE_SEMANTIC(CompilerThreadPoolWorker);
    ~CompilerThreadPoolWorker() override;

    void InitializeWorker() override
    {
        threadPool_ = internalAllocator_->New<ThreadPool<CompilerTask, CompilerProcessor, Compiler *>>(
            internalAllocator_, queue_, compiler_, 1, "JIT Thread");
    }

    void FinalizeWorker() override
    {
        if (threadPool_ != nullptr) {
            JoinWorker();
            internalAllocator_->Delete(threadPool_);
            threadPool_ = nullptr;
        }
    }

    void JoinWorker() override
    {
        if (threadPool_ != nullptr) {
            threadPool_->Shutdown(true);
        }
    }

    bool IsWorkerJoined() override
    {
        return !threadPool_->IsActive();
    }

    void AddTask(CompilerTask &&ctx) override
    {
        threadPool_->PutTask(std::move(ctx));
    }

    ThreadPool<CompilerTask, CompilerProcessor, Compiler *> *GetThreadPool()
    {
        return threadPool_;
    }

private:
    CompilerQueueInterface *CreateJITTaskQueue(const std::string &queueType, uint64_t maxLength, uint64_t taskLife,
                                               uint64_t deathCounter, uint64_t epochDuration);

    // This queue is used only in ThreadPool. Do not use it from this class.
    CompilerQueueInterface *queue_ {nullptr};
    ThreadPool<CompilerTask, CompilerProcessor, Compiler *> *threadPool_ {nullptr};
};

}  // namespace ark

#endif  // RUTNIME_COMPILER_THREAD_POOL_WORKER_H
