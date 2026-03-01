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

#ifndef PANDA_COMPILER_BACKGROUND_TASK_RUNNER_H
#define PANDA_COMPILER_BACKGROUND_TASK_RUNNER_H

#include <memory>

#include "libarkbase/task_runner.h"
#include "libarkbase/taskmanager/task.h"
#include "libarkbase/taskmanager/task_queue.h"
#include "libarkbase/mem/arena_allocator.h"
#include "compiler/optimizer/pipeline.h"
#include "compiler/optimizer/ir/runtime_interface.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/thread.h"

namespace ark {

class CompilerTask;
class Thread;
class Method;
class PandaVM;

}  // namespace ark

namespace ark::compiler {

class Graph;

class BackgroundCompilerContext {
public:
    using CompilerTask = std::unique_ptr<ark::CompilerTask, std::function<void(ark::CompilerTask *)>>;
    using CompilerThread = std::unique_ptr<ark::Thread, std::function<void(ark::Thread *)>>;

    void SetCompilerTask(CompilerTask compilerTask)
    {
        compilerTask_ = std::move(compilerTask);
    }

    void SetCompilerThread(CompilerThread compilerThread)
    {
        compilerThread_ = std::move(compilerThread);
    }

    void SetAllocator(std::unique_ptr<ArenaAllocator> allocator)
    {
        allocator_ = std::move(allocator);
    }

    void SetLocalAllocator(std::unique_ptr<ArenaAllocator> localAllocator)
    {
        localAllocator_ = std::move(localAllocator);
    }

    void SetMethodName(std::string methodName)
    {
        methodName_ = std::move(methodName);
    }

    void SetGraph(Graph *graph)
    {
        graph_ = graph;
    }

    void SetPipeline(std::unique_ptr<Pipeline> pipeline)
    {
        pipeline_ = std::move(pipeline);
    }

    void SetCompilationStatus(bool compilationStatus)
    {
        compilationStatus_ = compilationStatus;
    }

    Method *GetMethod() const
    {
        return compilerTask_->GetMethod();
    }

    bool IsOsr() const
    {
        return compilerTask_->IsOsr();
    }

    PandaVM *GetVM() const
    {
        return compilerTask_->GetVM();
    }

    ArenaAllocator *GetAllocator() const
    {
        return allocator_.get();
    }

    ArenaAllocator *GetLocalAllocator() const
    {
        return localAllocator_.get();
    }

    const std::string &GetMethodName() const
    {
        return methodName_;
    }

    Graph *GetGraph() const
    {
        return graph_;
    }

    Pipeline *GetPipeline() const
    {
        return pipeline_.get();
    }

    bool GetCompilationStatus() const
    {
        return compilationStatus_;
    }

private:
    CompilerTask compilerTask_;
    CompilerThread compilerThread_;
    std::unique_ptr<ArenaAllocator> allocator_;
    std::unique_ptr<ArenaAllocator> localAllocator_;
    std::string methodName_;
    Graph *graph_ {nullptr};
    std::unique_ptr<Pipeline> pipeline_;
    // Used only in JIT Compilation
    bool compilationStatus_ {false};
};

namespace copy_hooks {
class FailOnCopy {
public:
    FailOnCopy() = default;
    FailOnCopy(const FailOnCopy &other)
    {
        UNUSED_VAR(other);
        UNREACHABLE();
    }
    FailOnCopy &operator=(const FailOnCopy &other)
    {
        UNUSED_VAR(other);
        UNREACHABLE();
    }
    DEFAULT_MOVE_SEMANTIC(FailOnCopy);
    ~FailOnCopy() = default;
};

template <typename T>
class FakeCopyable : public FailOnCopy {
public:
    explicit FakeCopyable(T &&t) : target_(std::forward<T>(t)) {}
    FakeCopyable(const FakeCopyable &other)
        : FailOnCopy(other),                                  // this will fail
          target_(std::move(const_cast<T &>(other.target_)))  // never reached
    {
    }
    FakeCopyable &operator=(const FakeCopyable &other) = default;
    DEFAULT_MOVE_SEMANTIC(FakeCopyable);
    ~FakeCopyable() = default;

    template <typename... Args>
    auto operator()(Args &&...args)
    {
        return target_(std::forward<Args>(args)...);
    }

private:
    T target_;
};

template <typename T>
FakeCopyable<T> MakeFakeCopyable(T &&t)
{
    return FakeCopyable<T>(std::forward<T>(t));
}

}  // namespace copy_hooks

class BackgroundCompilerTaskRunner : public ark::TaskRunner<BackgroundCompilerTaskRunner, BackgroundCompilerContext> {
public:
    BackgroundCompilerTaskRunner(taskmanager::TaskQueueInterface *compilerQueue, Thread *compilerThread,
                                 RuntimeInterface *runtimeIface)
        : compilerQueue_(compilerQueue), compilerThread_(compilerThread), runtimeIface_(runtimeIface)
    {
    }

    BackgroundCompilerContext &GetContext() override
    {
        return taskCtx_;
    }

    /**
     * @brief Adds a task to the TaskManager queue. This task will run in the background
     * @param task_runner - Current TaskRunner containing context and callbacks
     * @param task_func - task which will be executed with @param task_runner
     */
    static void StartTask(BackgroundCompilerTaskRunner taskRunner, TaskRunner::TaskFunc taskFunc)
    {
        auto *compilerQueue = taskRunner.compilerQueue_;
        auto callback = [nextTask = std::move(taskFunc), nextRunner = std::move(taskRunner)]() mutable {
            auto *runtimeIface = nextRunner.runtimeIface_;
            runtimeIface->SetCurrentThread(nextRunner.compilerThread_);
            nextTask(std::move(nextRunner));
            runtimeIface->SetCurrentThread(nullptr);
        };
        compilerQueue->AddBackgroundTask(copy_hooks::MakeFakeCopyable(std::move(callback)));
    }

private:
    taskmanager::TaskQueueInterface *compilerQueue_ {nullptr};
    Thread *compilerThread_ {nullptr};
    RuntimeInterface *runtimeIface_;
    BackgroundCompilerContext taskCtx_;
};

}  // namespace ark::compiler

#endif  // PANDA_COMPILER_BACKGROUND_TASK_RUNNER_H
