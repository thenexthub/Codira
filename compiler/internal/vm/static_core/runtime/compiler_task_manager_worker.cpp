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

#include "runtime/compiler.h"
#include "runtime/compiler_task_manager_worker.h"
#include "compiler/background_task_runner.h"
#include "compiler/compiler_task_runner.h"

namespace ark {

CompilerTaskManagerWorker::CompilerTaskManagerWorker(mem::InternalAllocatorPtr internalAllocator, Compiler *compiler)
    : CompilerWorker(internalAllocator, compiler)
{
    compilerTaskManagerQueue_ = taskmanager::TaskManager::CreateTaskQueue<decltype(internalAllocator_->Adapter())>(
        taskmanager::MIN_QUEUE_PRIORITY);
    ASSERT(compilerTaskManagerQueue_ != nullptr);
}

void CompilerTaskManagerWorker::JoinWorker()
{
    {
        os::memory::LockHolder lock(taskQueueLock_);
        compilerWorkerJoined_ = true;
    }
    compilerTaskManagerQueue_->WaitBackgroundTasks();
}

void CompilerTaskManagerWorker::AddTask(CompilerTask &&task)
{
    {
        os::memory::LockHolder lock(taskQueueLock_);
        if (compilerWorkerJoined_) {
            return;
        }
        if (compilerTaskDeque_.empty()) {
            CompilerTask emptyTask;
            compilerTaskDeque_.emplace_back(std::move(emptyTask));
        } else {
            compilerTaskDeque_.emplace_back(std::move(task));
            return;
        }
    }
    // This means that this is first task in queue, so we can compile it in-place.
    BackgroundCompileMethod(std::move(task));
}

void CompilerTaskManagerWorker::BackgroundCompileMethod(CompilerTask &&ctx)
{
    auto threadDeleter = [this](Thread *thread) { internalAllocator_->Delete(thread); };
    compiler::BackgroundCompilerContext::CompilerThread compilerThread(
        internalAllocator_->New<Thread>(ctx.GetVM(), Thread::ThreadType::THREAD_TYPE_COMPILER),
        std::move(threadDeleter));

    auto taskDeleter = [this](CompilerTask *task) { internalAllocator_->Delete(task); };
    compiler::BackgroundCompilerContext::CompilerTask compilerTask(
        internalAllocator_->New<CompilerTask>(std::move(ctx)), std::move(taskDeleter));

    compiler::BackgroundCompilerTaskRunner taskRunner(compilerTaskManagerQueue_, compilerThread.get(),
                                                      compiler_->GetRuntimeInterface());
    auto &compilerCtx = taskRunner.GetContext();
    compilerCtx.SetCompilerThread(std::move(compilerThread));
    compilerCtx.SetCompilerTask(std::move(compilerTask));

    // Callback to compile next method from compiler_task_deque_
    taskRunner.AddFinalize([this]([[maybe_unused]] compiler::BackgroundCompilerContext &taskContext) {
        CompilerTask nextTask;
        {
            os::memory::LockHolder lock(taskQueueLock_);
            ASSERT(!compilerTaskDeque_.empty());
            // compilation of current task is complete
            compilerTaskDeque_.pop_front();
            if (compilerTaskDeque_.empty()) {
                return;
            }
            // now queue has empty task which will be popped at the end of compilation
            nextTask = std::move(compilerTaskDeque_.front());
        }
        BackgroundCompileMethod(std::move(nextTask));
    });

    auto backgroundTask = [this](compiler::BackgroundCompilerTaskRunner runner) {
        if (runner.GetContext().GetMethod()->AtomicSetCompilationStatus(Method::WAITING, Method::COMPILATION)) {
            compiler_->StartCompileMethod<compiler::BACKGROUND_MODE>(std::move(runner));
            return;
        }
        compiler::BackgroundCompilerTaskRunner::EndTask(std::move(runner), false);
    };
    compiler::BackgroundCompilerTaskRunner::StartTask(std::move(taskRunner), std::move(backgroundTask));
}

}  // namespace ark
