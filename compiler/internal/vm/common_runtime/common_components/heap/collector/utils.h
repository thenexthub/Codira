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
#ifndef COMMON_COMPONENTS_HEAP_ARK_COLLECTOR_UTILS_H
#define COMMON_COMPONENTS_HEAP_ARK_COLLECTOR_UTILS_H

#include "common_components/taskpool/taskpool.h"

namespace common {

class ArrayTaskDispatcher {
public:
    class ArrayTask {
    public:
        virtual ~ArrayTask() = default;
        virtual void Run(void *begin, void *end) = 0;
    };
    ArrayTaskDispatcher(void *memory, size_t bytes, size_t taskBytes, ArrayTask *callback)
        : index_(0),
          array_(reinterpret_cast<uintptr_t>(memory)),
          bytes_(bytes),
          batch_(taskBytes),
          consumer_(callback),
          aliveTask_(0)
    {
    }
    ~ArrayTaskDispatcher() { DCHECK_CC(aliveTask_ == 0); }
    void Dispatch(Taskpool *pool, int nThread);
    void JoinAndWait();

private:
    class Runner : public common::Task {
    public:
        explicit Runner(ArrayTaskDispatcher *dispatcher) : Task(0), manager_(dispatcher) {}
        ~Runner();
        bool Run(uint32_t) override;
        ArrayTaskDispatcher *manager_;
    };
    void *TaskAt(size_t i) const { return reinterpret_cast<void *>(array_ + (i * batch_)); }
    void RunImpl();

    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic_size_t index_;
    const uintptr_t array_;
    const size_t bytes_;
    const size_t batch_;
    ArrayTask *consumer_;
    int aliveTask_;
};

}  // namespace common

#endif
