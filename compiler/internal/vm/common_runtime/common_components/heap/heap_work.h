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

#ifndef COMMON_COMPONENTS_HEAP_HEAP_WORK_H
#define COMMON_COMPONENTS_HEAP_HEAP_WORK_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <pthread.h>
#include <queue>
#include <vector>

namespace common {
class HeapWork {
public:
    virtual void Execute(size_t threadId) = 0;
    HeapWork() = default;
    virtual ~HeapWork() = default;
};

class LambdaWork : public HeapWork {
public:
    explicit LambdaWork(const std::function<void(size_t)>& function) : func_(function) {}
    ~LambdaWork() = default;
    void Execute(size_t threadId) override { func_(threadId); }

private:
    std::function<void(size_t)> func_;
};
} // namespace common

#endif // COMMON_COMPONENTS_HEAP_HEAP_WORK_H
