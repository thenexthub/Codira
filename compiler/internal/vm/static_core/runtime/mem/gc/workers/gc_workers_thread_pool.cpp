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

#include <utility>

#include "libarkbase/os/cpu_affinity.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/workers/gc_workers_thread_pool.h"

namespace ark::mem {

bool GCWorkersProcessor::Init()
{
    return gcThreadsPools_->GetGC()->InitWorker(&workerData_);
}

bool GCWorkersProcessor::Destroy()
{
    gcThreadsPools_->GetGC()->DestroyWorker(workerData_);
    return true;
}

bool GCWorkersProcessor::Process(GCWorkersTask &&task)
{
    gcThreadsPools_->RunGCWorkersTask(&task, workerData_);
    return true;
}

GCWorkersThreadPool::GCWorkersThreadPool(GC *gc, size_t threadsCount)
    : GCWorkersTaskPool(gc), internalAllocator_(gc->GetInternalAllocator()), threadsCount_(threadsCount)
{
    ASSERT(gc->GetPandaVm() != nullptr);
    queue_ = internalAllocator_->New<GCWorkersQueueSimple>(internalAllocator_, QUEUE_SIZE_MAX_SIZE);
    workerIface_ = internalAllocator_->New<GCWorkersCreationInterface>(gc->GetPandaVm());
    threadPool_ = internalAllocator_->New<ThreadPool<GCWorkersTask, GCWorkersProcessor, GCWorkersThreadPool *>>(
        internalAllocator_, queue_, this, threadsCount, "GC_WORKER",
        static_cast<WorkerCreationInterface *>(workerIface_));
}

bool GCWorkersThreadPool::TryAddTask(GCWorkersTask &&task)
{
    return threadPool_->TryPutTask(std::forward<GCWorkersTask &&>(task));
}

static void SetAffinity(GCWorkersProcessor *proc, size_t gcThreadsCount)
{
    // For first GC, GC-workers can be not started
    if (UNLIKELY(!proc->IsStarted())) {
        return;
    }
    const auto &bestAndMiddle = os::CpuAffinityManager::GetBestAndMiddleCpuSet();
    if (gcThreadsCount < bestAndMiddle.Count()) {
        os::CpuAffinityManager::SetAffinityForThread(proc->GetTid(), bestAndMiddle);
    }
}

static void UnsetAffinity(GCWorkersProcessor *proc, [[maybe_unused]] size_t data)
{
    os::CpuAffinityManager::SetAffinityForThread(proc->GetTid(), os::CpuPower::WEAK);
}

void GCWorkersThreadPool::SetAffinityForGCWorkers()
{
    // Total GC threads count = GC Thread + GC workers
    threadPool_->EnumerateProcs(SetAffinity, threadsCount_ + 1U);
}

void GCWorkersThreadPool::UnsetAffinityForGCWorkers()
{
    threadPool_->EnumerateProcs(UnsetAffinity, 0U);
}

GCWorkersThreadPool::~GCWorkersThreadPool()
{
    internalAllocator_->Delete(threadPool_);
    internalAllocator_->Delete(workerIface_);
    queue_->Finalize();
    internalAllocator_->Delete(queue_);
}

void GCWorkersThreadPool::RunInCurrentThread()
{
    threadPool_->Help();
}

}  // namespace ark::mem
