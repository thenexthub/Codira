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

#ifndef LIBPANDABASE_TASKMANAGER_UTILS_TWO_LOCK_QUEUE
#define LIBPANDABASE_TASKMANAGER_UTILS_TWO_LOCK_QUEUE

#include <type_traits>
#include "libarkbase/taskmanager/utils/sp_sc_lock_free_queue.h"
#include "libarkbase/os/mutex.h"

namespace ark::taskmanager::internal {

template <class T, class Allocator, QueueElemAllocationType ALLOCATION_TYPE>
class TwoLockQueue {
public:
    void Push(T *val)
    {
        os::memory::LockHolder lh(pushLock_);
        queue_.Push(val);
    }

    template <class CreationFunc, class... Args>
    void Emplace(CreationFunc creationFunc, Args &&...args)
    {
        os::memory::LockHolder lh(pushLock_);
        queue_.Emplace(creationFunc, std::forward<Args>(args)...);
    }

    T Pop()
    {
        os::memory::LockHolder lh(popLock_);
        return queue_.Pop();
    }

    bool TryPop(T **val)
    {
        os::memory::LockHolder lh(popLock_);
        return queue_.TryPop(val);
    }

    bool IsEmpty() const
    {
        return queue_.IsEmpty();
    }

    size_t Size() const
    {
        return queue_.Size();
    }

    static void TryDeleteNode(void *node)
    {
        InternalTaskQueue::TryDeleteNode(node);
    }

private:
    using InternalTaskQueue = SPSCLockFreeQueue<T, Allocator, ALLOCATION_TYPE>;

    mutable os::memory::Mutex pushLock_;
    mutable os::memory::Mutex popLock_;
    InternalTaskQueue queue_;
};

}  // namespace ark::taskmanager::internal

#endif  // LIBPANDABASE_TASKMANAGER_UTILS_TWO_LOCK_QUEUE
