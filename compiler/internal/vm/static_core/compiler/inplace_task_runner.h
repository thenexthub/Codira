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

#ifndef PANDA_COMPILER_INPLACE_TASK_RUNNER_H
#define PANDA_COMPILER_INPLACE_TASK_RUNNER_H

#include <memory>

#include "libarkbase/task_runner.h"
#include "libarkbase/mem/arena_allocator.h"

namespace ark {

class Method;
class PandaVM;
}  // namespace ark

namespace ark::compiler {

class Pipeline;
class Graph;

class InPlaceCompilerContext {
public:
    void SetMethod(Method *method)
    {
        method_ = method;
    }

    void SetOsr(bool isOsr)
    {
        isOsr_ = isOsr;
    }

    void SetVM(PandaVM *pandaVm)
    {
        pandaVm_ = pandaVm;
    }

    void SetAllocator(ArenaAllocator *allocator)
    {
        allocator_ = allocator;
    }

    void SetLocalAllocator(ArenaAllocator *localAllocator)
    {
        localAllocator_ = localAllocator;
    }

    void SetMethodName(std::string methodName)
    {
        methodName_ = std::move(methodName);
    }

    void SetGraph(Graph *graph)
    {
        graph_ = graph;
    }

    void SetPipeline(Pipeline *pipeline)
    {
        pipeline_ = pipeline;
    }

    void SetCompilationStatus(bool compilationStatus)
    {
        compilationStatus_ = compilationStatus;
    }

    Method *GetMethod() const
    {
        return method_;
    }

    bool IsOsr() const
    {
        return isOsr_;
    }

    PandaVM *GetVM() const
    {
        return pandaVm_;
    }

    ArenaAllocator *GetAllocator() const
    {
        return allocator_;
    }

    ArenaAllocator *GetLocalAllocator() const
    {
        return localAllocator_;
    }

    Graph *GetGraph() const
    {
        return graph_;
    }

    const std::string &GetMethodName() const
    {
        return methodName_;
    }

    Pipeline *GetPipeline() const
    {
        return pipeline_;
    }

    bool GetCompilationStatus() const
    {
        return compilationStatus_;
    }

private:
    Method *method_ {nullptr};
    bool isOsr_ {false};
    PandaVM *pandaVm_ {nullptr};
    ArenaAllocator *allocator_ {nullptr};
    ArenaAllocator *localAllocator_ {nullptr};
    Graph *graph_ {nullptr};
    std::string methodName_;
    Pipeline *pipeline_ {nullptr};
    // Used only in JIT Compilation
    bool compilationStatus_ {false};
};

class InPlaceCompilerTaskRunner : public ark::TaskRunner<InPlaceCompilerTaskRunner, InPlaceCompilerContext> {
public:
    InPlaceCompilerContext &GetContext() override
    {
        return taskCtx_;
    }

    /**
     * @brief Starts task in-place
     * @param task_runner - Current TaskRunner containing context and callbacks
     * @param task_func - task which will be executed with @param task_runner
     */
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    static void StartTask(InPlaceCompilerTaskRunner taskRunner, TaskRunner::TaskFunc taskFunc)
    {
        taskFunc(std::move(taskRunner));
    }

private:
    InPlaceCompilerContext taskCtx_;
};

}  // namespace ark::compiler

#endif  // PANDA_COMPILER_INPLACE_TASK_RUNNER_H
