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

#include "runtime/mem/gc/g1/update_remset_thread.h"

#include "libarkbase/utils/logger.h"
#include "runtime/include/language_config.h"
#include "runtime/mem/gc/g1/g1-gc.h"
#include "runtime/mem/gc/g1/update_remset_worker.h"

namespace ark::mem {

template <class LanguageConfig>
UpdateRemsetThread<LanguageConfig>::UpdateRemsetThread(G1GC<LanguageConfig> *gc,
                                                       GCG1BarrierSet::ThreadLocalCardQueues *queue,
                                                       os::memory::Mutex *queueLock, size_t regionSize,
                                                       bool updateConcurrent, size_t minConcurrentCardsToProcess,
                                                       size_t hotCardsProcessingFrequency)
    : UpdateRemsetWorker<LanguageConfig>(gc, queue, queueLock, regionSize, updateConcurrent,
                                         minConcurrentCardsToProcess, hotCardsProcessingFrequency)
{
}

template <class LanguageConfig>
UpdateRemsetThread<LanguageConfig>::~UpdateRemsetThread() = default;

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::CreateWorkerImpl()
{
    LOG(DEBUG, GC) << "Start creating UpdateRemsetThread";
    os::memory::LockHolder lk(this->updateRemsetLock_);
    auto internalAllocator = this->GetGC()->GetInternalAllocator();
    ASSERT(updateThread_ == nullptr);
    updateThread_ = internalAllocator->template New<std::thread>(&UpdateRemsetThread::ThreadLoop, this);
    ASSERT(updateThread_ != nullptr);
    int res = os::thread::SetThreadName(updateThread_->native_handle(), "UpdateRemset");

    LOG_IF(res != 0, ERROR, RUNTIME) << "Failed to set a name for the UpdateRemset thread";
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::DestroyWorkerImpl()
{
    LOG(DEBUG, GC) << "Starting destroy UpdateRemsetThread";
    {
        os::memory::LockHolder holder(this->updateRemsetLock_);
        threadCondVar_.SignalAll();  // wake up updateremset worker & pause method
    }
    ASSERT(updateThread_ != nullptr);
    updateThread_->join();
    auto allocator = this->GetGC()->GetInternalAllocator();
    ASSERT(allocator != nullptr);
    allocator->Delete(updateThread_);
    updateThread_ = nullptr;
    LOG(DEBUG, GC) << "UpdateRemsetThread was destroyed";
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::ThreadLoop()
{
    LOG(DEBUG, GC) << "Entering UpdateRemsetThread ThreadLoop";

    this->updateRemsetLock_.Lock();
    while (true) {
        // Do one atomic load before checks
        auto iterationFlag = this->GetFlag();
        if (iterationFlag == UpdateRemsetWorker<LanguageConfig>::UpdateRemsetWorkerFlags::IS_STOP_WORKER) {
            LOG(DEBUG, GC) << "exit UpdateRemsetThread loop, because thread was stopped";
            break;
        }
        if (iterationFlag == UpdateRemsetWorker<LanguageConfig>::UpdateRemsetWorkerFlags::IS_PAUSED_BY_GC_THREAD) {
            // UpdateRemsetThread is paused by GC, wait until GC notifies to continue work
            threadCondVar_.Wait(&this->updateRemsetLock_);
            continue;
        }
        if (iterationFlag == UpdateRemsetWorker<LanguageConfig>::UpdateRemsetWorkerFlags::IS_INVALIDATE_REGIONS) {
            // wait until GC-Thread invalidates regions
            threadCondVar_.Wait(&this->updateRemsetLock_);
            continue;
        }
        ASSERT(!this->pausedByGcThread_);
        ASSERT(iterationFlag == UpdateRemsetWorker<LanguageConfig>::UpdateRemsetWorkerFlags::IS_PROCESS_CARD);
        auto processedCards = this->ProcessAllCards();
        if (processedCards < this->GetMinConcurrentCardsToProcess()) {
            Sleep();
        }
    }
    this->updateRemsetLock_.Unlock();
    LOG(DEBUG, GC) << "Exiting UpdateRemsetThread ThreadLoop";
}

template <class LanguageConfig>
void UpdateRemsetThread<LanguageConfig>::ContinueProcessCards()
{
    // Signal to continue UpdateRemsetThread
    threadCondVar_.Signal();
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(UpdateRemsetThread);
}  // namespace ark::mem
