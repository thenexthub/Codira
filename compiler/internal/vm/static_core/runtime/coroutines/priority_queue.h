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
#ifndef PANDA_RUNTIME_COROUTINES_PRIORITY_QUEUE_H
#define PANDA_RUNTIME_COROUTINES_PRIORITY_QUEUE_H

#include "libarkbase/macros.h"
#include "include/mem/panda_containers.h"
#include "runtime/coroutines/coroutine_worker.h"

#include <cstdint>
#include <limits>
#include <type_traits>

namespace ark {

class Coroutine;

enum class CoroutineOrder { STACK_ORDER, QUEUE_ORDER };

class PriorityQueue {
    class CoroutineWrapper {
    public:
        explicit CoroutineWrapper(Coroutine *coroutine, uint64_t priority) : coroutine_(coroutine), priority_(priority)
        {
            ASSERT((priority >> ORDER_BITS) != 0);
        }

        Coroutine *operator*() const
        {
            return coroutine_;
        }

        Coroutine *operator->() const
        {
            return coroutine_;
        }

        /// @return compares coroutine wrappers by priority
        bool operator<(const CoroutineWrapper &other) const
        {
            return priority_ < other.priority_;
        }

        /// @return priority of wrapped coroutine
        CoroutinePriority GetPriority();

    private:
        Coroutine *coroutine_;
        uint64_t priority_;
    };

public:
    using Queue = PandaDeque<CoroutineWrapper>;
    using CIterator = Queue::const_iterator;
    using Properties = std::unordered_map<CoroutinePriority, CoroutineOrder>;

    // NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
    static inline Properties defaultProps_ = {{CoroutinePriority::LOW_PRIORITY, CoroutineOrder::QUEUE_ORDER},
                                              {CoroutinePriority::MEDIUM_PRIORITY, CoroutineOrder::QUEUE_ORDER},
                                              {CoroutinePriority::HIGH_PRIORITY, CoroutineOrder::QUEUE_ORDER},
                                              {CoroutinePriority::CRITICAL_PRIORITY, CoroutineOrder::QUEUE_ORDER}};

    /// Creates PriorityQueue with the given properties (Priority -> Order mapping)
    explicit PriorityQueue(Properties props = defaultProps_) : properties_(std::move(props)) {}

    /**
     * Adds a coroutine to the queue in order of priority.
     * For the stack order, the coroutine is added at the top.
     * For the queue order - at the end of the priority subqueue.
     */
    void Push(Coroutine *coro, CoroutinePriority priority);

    /**
     * Checks that queue is not empty,
     * returns Coroutine with the highest priority.
     */
    std::pair<Coroutine *, CoroutinePriority> Pop();

    /**
     * Allows to iterate over Coroutines in the priority queue.
     * Guarantees priority ordrer during iteration.
     */
    template <typename Visitor, typename U = std::enable_if_t<std::is_invocable_v<Visitor, Coroutine *>>>
    void IterateOverCoroutines(const Visitor &visitor) const
    {
        for (auto &coroWrapper : coroutines_) {
            visitor(*coroWrapper);
        }
    }

    /**
     * Allows to apply callback for CoroutineWrappers in the priority queue.
     * Can be used in pair with RemoveCoroutines.
     */
    template <typename Visitor, typename U = std::enable_if_t<std::is_invocable_v<Visitor, CIterator, CIterator>>>
    void VisitCoroutines(const Visitor &visitor) const
    {
        visitor(coroutines_.cbegin(), coroutines_.cend());
    }

    /// Removes coroutines from the priority queue by given sequence of iterators
    template <typename Container,
              typename U = std::enable_if_t<std::is_same_v<typename Container::value_type, CIterator>>>
    void RemoveCoroutines(const Container &coroutinesIterators)
    {
        for (auto &iter : coroutinesIterators) {
            coroutines_.erase(iter);
        }
        std::make_heap(coroutines_.begin(), coroutines_.end(), Comparator);
    }

    /// @return size of the priority queue
    size_t Size() const
    {
        return coroutines_.size();
    }

    /// @return true if priority queue is empty
    bool Empty() const
    {
        return coroutines_.empty();
    }

private:
    uint64_t CalculatePriority(CoroutinePriority priority);

    static bool Comparator(const CoroutineWrapper &co1, const CoroutineWrapper &co2);

    static constexpr uint64_t PRIORITY_COUNT = static_cast<uint64_t>(CoroutinePriority::PRIORITY_COUNT);
    static constexpr uint64_t ORDER_BITS = std::numeric_limits<uint64_t>::digits - PRIORITY_COUNT;

    Queue coroutines_;
    Properties properties_;
    uint64_t stackOrder_ = 0;
    uint64_t queueOrder_ = std::numeric_limits<uint64_t>::max() >> PRIORITY_COUNT;
};
}  // namespace ark

#endif  // PANDA_RUNTIME_COROUTINES_PRIORITY_QUEUE_H
