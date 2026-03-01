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

#include "common_components/heap/collector/utils.h"

namespace common {

void ArrayTaskDispatcher::Dispatch(Taskpool *pool, int nThread)
{
    aliveTask_ = nThread;
    for (int i = 0; i < nThread; ++i) {
        pool->PostTask(std::make_unique<Runner>(this));
    }
}

void ArrayTaskDispatcher::RunImpl()
{
    void *end = reinterpret_cast<void *>(array_ + bytes_);
    size_t i = index_.fetch_add(1, std::memory_order_relaxed);
    auto iter = TaskAt(i);
    while (iter < end) {
        consumer_->Run(iter, std::min(TaskAt(i + 1), end));
        i = index_.fetch_add(1, std::memory_order_relaxed);
        iter = TaskAt(i);
    }
}

bool ArrayTaskDispatcher::Runner::Run(uint32_t)
{
    manager_->RunImpl();
    return true;
}

ArrayTaskDispatcher::Runner::~Runner()
{
    std::unique_lock<std::mutex> lock(manager_->mtx_);
    if ((--manager_->aliveTask_) == 0) {
        manager_->cv_.notify_all();
    }
}

void ArrayTaskDispatcher::JoinAndWait()
{
    RunImpl();
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this]() { return aliveTask_ == 0; });
}

}  // namespace common
