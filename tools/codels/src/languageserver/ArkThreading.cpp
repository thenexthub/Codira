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

#include "ArkThreading.h"
#include <atomic>

namespace ark {

Semaphore::Semaphore(std::size_t maxLocks) : freeSlots(maxLocks) {}

bool Semaphore::try_lock()
{
    std::unique_lock<std::mutex> lock(mutexSemaphore);
    if (freeSlots > 0) {
        --freeSlots;
        return true;
    }
    return false;
}

void Semaphore::lock()
{
    std::unique_lock<std::mutex> lock(mutexSemaphore);
    slotsChanged.wait(lock, [this]() { return freeSlots > 0; });
    --freeSlots;
}

void Semaphore::unlock()
{
    std::unique_lock<std::mutex> lock(mutexSemaphore);
    ++freeSlots;
    lock.unlock();
    slotsChanged.notify_one();
}

void Wait(std::unique_lock<std::mutex> &lock, std::condition_variable &cv, const Deadline deadline)
{
    if (deadline == Deadline::Zero()) {
        return;
    }
    if (deadline == Deadline::Infinity()) {
        return cv.wait(lock);
    }
    (void)cv.wait_until(lock, deadline.Time());
}
} // namespace ark

