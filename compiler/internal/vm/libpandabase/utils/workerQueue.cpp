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

#include "utils/workerQueue.h"

namespace panda {

void WorkerJob::DependsOn(WorkerJob *job)
{
    job->dependants_.push_back(this);
    dependencies_++;
}

void WorkerJob::Signal()
{
    {
        std::lock_guard<std::mutex> lock(m_);
        dependencies_--;
    }

    cond_.notify_one();
}

WorkerQueue::WorkerQueue(size_t threadCount)
{
    threads_.reserve(threadCount);

    for (size_t i = 0; i < threadCount; i++) {
        threads_.push_back(os::thread::ThreadStart(Worker, this));
    }
}

WorkerQueue::~WorkerQueue()
{
    void *retval = nullptr;

    std::unique_lock<std::mutex> lock(m_);
    terminate_ = true;
    lock.unlock();
    jobsAvailable_.notify_all();

    for (const auto handle_id : threads_) {
        os::thread::ThreadJoin(handle_id, &retval);
    }
}

bool WorkerQueue::Worker(WorkerQueue *queue)
{
    while (true) {
        std::unique_lock<std::mutex> lock(queue->m_);
        queue->jobsAvailable_.wait(lock, [queue]() { return queue->terminate_ || queue->jobsCount_ != 0; });

        if (queue->terminate_) {
            return false;
        }

        lock.unlock();
        if (!queue->Consume()) {
            return false;
        }
        
        queue->jobsFinished_.notify_one();
        return true;
    }
}

bool WorkerQueue::Consume()
{
    std::unique_lock<std::mutex> lock(m_);
    activeWorkers_++;

    while (jobsCount_ > 0) {
        --jobsCount_;
        auto &job = *(jobs_[jobsCount_]);

        lock.unlock();
        if (!job.Run()) {
            return false;
        }
        lock.lock();
    }

    activeWorkers_--;
    return true;
}

void WorkerQueue::Wait()
{
    std::unique_lock<std::mutex> lock(m_);
    jobsFinished_.wait(lock, [this]() { return activeWorkers_ == 0 && jobsCount_ == 0; });
    for (auto it = jobs_.begin(); it != jobs_.end(); it++) {
        if (*it != nullptr) {
            delete *it;
            *it = nullptr;
        }
    }
    jobs_.clear();
}
}  // namespace panda
