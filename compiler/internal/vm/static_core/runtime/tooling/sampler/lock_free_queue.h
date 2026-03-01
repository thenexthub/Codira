/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_TOOLING_SAMPLER_LOCK_FREE_QUEUE_H
#define PANDA_RUNTIME_TOOLING_SAMPLER_LOCK_FREE_QUEUE_H

#include <atomic>
#include <memory>
#include "runtime/tooling/sampler/sample_info.h"

namespace ark::tooling::sampler {

class LockFreeQueue {
public:
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    LockFreeQueue()
    {
        std::atomic<Node *> dummyNode = new Node(std::make_unique<FileInfo>(), nullptr);
        // Atomic with relaxed order reason: there is no data race
        head_.store(dummyNode, std::memory_order_relaxed);
        // Atomic with relaxed order reason: there is no data race
        tail_.store(dummyNode, std::memory_order_relaxed);
        size_ = 0;
    }

    ~LockFreeQueue()
    {
        FileInfo module;
        while (size_ != 0) {
            Pop(module);
        }
        delete head_;
    }

    void Push(const FileInfo &data);
    bool FindValue(uintptr_t data) const;

    size_t Size()
    {
        return size_;
    }

    NO_MOVE_SEMANTIC(LockFreeQueue);
    NO_COPY_SEMANTIC(LockFreeQueue);

private:
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    void Pop(FileInfo &ret);

private:
    struct Node {
        // NOLINTBEGIN
        Node(std::unique_ptr<FileInfo> new_data, std::atomic<Node *> new_next) : data(std::move(new_data))
        {
            // Atomic with relaxed order reason: there is no data race
            next.store(new_next, std::memory_order_relaxed);
        }
        // NOLINTEND

        // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
        std::unique_ptr<FileInfo> data;
        std::atomic<Node *> next;
        // NOLINTEND(misc-non-private-member-variables-in-classes)
    };

    std::atomic<Node *> head_;
    std::atomic<Node *> tail_;
    std::atomic<size_t> size_;
};

}  // namespace ark::tooling::sampler

#endif  // PANDA_RUNTIME_TOOLING_SAMPLER_LOCK_FREE_QUEUE_H
