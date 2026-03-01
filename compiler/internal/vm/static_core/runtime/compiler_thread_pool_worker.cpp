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

#include "runtime/compiler.h"
#include "runtime/compiler_thread_pool_worker.h"
#include "runtime/compiler_queue_simple.h"
#include "runtime/compiler_queue_aged_counter_priority.h"
#include "compiler/inplace_task_runner.h"

namespace ark {

CompilerThreadPoolWorker::CompilerThreadPoolWorker(mem::InternalAllocatorPtr internalAllocator, Compiler *compiler,
                                                   bool &noAsyncJit, const RuntimeOptions &options)
    : CompilerWorker(internalAllocator, compiler)
{
    queue_ = CreateJITTaskQueue(noAsyncJit ? "simple" : options.GetCompilerQueueType(),
                                options.GetCompilerQueueMaxLength(), options.GetCompilerTaskLifeSpan(),
                                options.GetCompilerDeathCounterValue(), options.GetCompilerEpochDuration());
    if (queue_ == nullptr) {
        // Because of problems (no memory) in allocator
        LOG(ERROR, COMPILER) << "Cannot create a compiler queue";
        noAsyncJit = true;
    }
}

CompilerThreadPoolWorker::~CompilerThreadPoolWorker()
{
    if (queue_ != nullptr) {
        queue_->Finalize();
        internalAllocator_->Delete(queue_);
    }
}

CompilerQueueInterface *CompilerThreadPoolWorker::CreateJITTaskQueue(const std::string &queueType, uint64_t maxLength,
                                                                     uint64_t taskLife, uint64_t deathCounter,
                                                                     uint64_t epochDuration)
{
    LOG(DEBUG, COMPILER) << "Creating " << queueType << " task queue";
    if (queueType == "simple") {
        return internalAllocator_->New<CompilerQueueSimple>(internalAllocator_);
    }
    if (queueType == "counter-priority") {
        return internalAllocator_->New<CompilerPriorityCounterQueue>(internalAllocator_, maxLength, taskLife);
    }
    if (queueType == "aged-counter-priority") {
        return internalAllocator_->New<CompilerPriorityAgedCounterQueue>(internalAllocator_, taskLife, deathCounter,
                                                                         epochDuration);
    }
    LOG(FATAL, COMPILER) << "Unknown queue type";
    return nullptr;
}

bool CompilerProcessor::Process(CompilerTask &&task)
{
    InPlaceCompileMethod(std::move(task));
    return true;
}

void CompilerProcessor::InPlaceCompileMethod(CompilerTask &&ctx)
{
    compiler::InPlaceCompilerTaskRunner taskRunner;
    auto &compilerCtx = taskRunner.GetContext();
    compilerCtx.SetMethod(ctx.GetMethod());
    compilerCtx.SetOsr(ctx.IsOsr());
    compilerCtx.SetVM(ctx.GetVM());

    // Set current thread to have access to vm during compilation
    Thread compilerThread(ctx.GetVM(), Thread::ThreadType::THREAD_TYPE_COMPILER);
    ScopedCurrentThread sct(&compilerThread);

    if (compilerCtx.GetMethod()->AtomicSetCompilationStatus(Method::WAITING, Method::COMPILATION)) {
        compiler_->CompileMethodLocked<compiler::INPLACE_MODE>(std::move(taskRunner));
    }
}

}  // namespace ark
