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
#include "runtime/coroutines/priority_queue.h"
#include "runtime/coroutines/coroutine.h"

#include <algorithm>

namespace ark {

CoroutinePriority PriorityQueue::CoroutineWrapper::GetPriority()
{
    auto internalPriority = priority_ >> ORDER_BITS;
    auto priority = 0U;
    while (internalPriority != 1) {
        internalPriority >>= 1U;
        priority++;
    }
    ASSERT(priority < PRIORITY_COUNT);
    return static_cast<CoroutinePriority>(priority);
}

void PriorityQueue::Push(Coroutine *coro, CoroutinePriority priority)
{
    coroutines_.emplace_back(coro, CalculatePriority(priority));
    std::push_heap(coroutines_.begin(), coroutines_.end(), Comparator);
}

std::pair<Coroutine *, CoroutinePriority> PriorityQueue::Pop()
{
    ASSERT(!Empty());
    std::pop_heap(coroutines_.begin(), coroutines_.end(), Comparator);
    auto coroWrapper = coroutines_.back();
    coroutines_.pop_back();
    return {*coroWrapper, coroWrapper.GetPriority()};
}

uint64_t PriorityQueue::CalculatePriority(CoroutinePriority priority)
{
    ASSERT(static_cast<uint32_t>(priority) < PRIORITY_COUNT);
    auto orderIt = properties_.find(priority);
    ASSERT(orderIt != properties_.end());
    uint64_t order = 0;
    switch (orderIt->second) {
        case CoroutineOrder::STACK_ORDER:
            order = stackOrder_++;
            break;
        case CoroutineOrder::QUEUE_ORDER:
            order = queueOrder_--;
            break;
        default:
            UNREACHABLE();
    }
    return (1ULL << (ORDER_BITS + static_cast<uint32_t>(priority))) | order;
}

bool PriorityQueue::Comparator(const CoroutineWrapper &co1, const CoroutineWrapper &co2)
{
    return co1 < co2;
}

}  // namespace ark
