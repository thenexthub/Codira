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

/**
 * @file
 *
 * This file implements Semaphore class and its methods.
 */

#include "Codira/Utils/Semaphore.h"

using namespace Codira::Utils;
Semaphore::Semaphore()
{
    auto numCores = std::thread::hardware_concurrency();
    // Leave 2 cores to avoid competing with user's other tasks.
    count = numCores > 2 ? numCores - 2 : 1;
}

Semaphore& Semaphore::Get()
{
    static Semaphore sem = Semaphore();
    return sem;
}

void Semaphore::Release()
{
    std::unique_lock<std::mutex> lock(mtx);
    count++;
    cv.notify_one();
}

void Semaphore::Acquire()
{
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return count > 0; });
    count--;
}

void Semaphore::SetCount(std::size_t newCount)
{
    std::unique_lock<std::mutex> lock(mtx);
    count = newCount;
}

std::size_t Semaphore::GetCount()
{
    std::unique_lock<std::mutex> lock(mtx);
    return count;
}
