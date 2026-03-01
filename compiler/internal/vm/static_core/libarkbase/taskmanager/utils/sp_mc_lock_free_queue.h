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

#ifndef LIBPANDABASE_TASKMANAGER_UTILS_SP_MC_LOCK_FREE_QUEUE_H
#define LIBPANDABASE_TASKMANAGER_UTILS_SP_MC_LOCK_FREE_QUEUE_H

#include "libarkbase/generated/coherency_line_size.h"
#include "libarkbase/utils/math_helpers.h"
#include <optional>
#include <map>
#include <atomic>
#include <array>
#include <set>

namespace ark::taskmanager::internal {

static constexpr size_t SP_MC_LOCK_FREE_QUEUE_DEFAULT_QUEUE_NODE_SIZE = 1UL << 5U;

/**
 * @brief SPMCLockFreeQueue is single producer, multiple consumer lock free queue.
 * @tparam T: Type of class you want ot store in queue
 * @tparam QUEUE_NODE_SIZE: Size of one node in queue. Default value: SP_MC_LOCK_FREE_QUEUE_DEFAULT_QUEUE_NODE_SIZE
 */
template <class T, size_t QUEUE_NODE_SIZE = SP_MC_LOCK_FREE_QUEUE_DEFAULT_QUEUE_NODE_SIZE>
class SPMCLockFreeQueue {
    static_assert(ark::helpers::math::IsPowerOfTwo(QUEUE_NODE_SIZE));
    static constexpr size_t QUEUE_NODE_MASK = QUEUE_NODE_SIZE - 1U;
    static_assert(QUEUE_NODE_MASK > 0);
#ifdef __APPLE__
    static constexpr size_t QUEUE_NODE_SHIFT = ark::helpers::math::GetIntLog2(static_cast<uint64_t>(QUEUE_NODE_SIZE));
#else
    static constexpr size_t QUEUE_NODE_SHIFT = ark::helpers::math::GetIntLog2(QUEUE_NODE_SIZE);
#endif

    static constexpr size_t CONSUMER_MAX_COUNT = 1UL << 5U;
    static constexpr size_t DELETE_TRIGGER_PUSH_COUNT = QUEUE_NODE_SIZE;
    static_assert(ark::helpers::math::IsPowerOfTwo(DELETE_TRIGGER_PUSH_COUNT));
    static constexpr size_t DELETE_TRIGGER_PUSH_MASK = DELETE_TRIGGER_PUSH_COUNT - 1U;

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    template <class U, size_t NODE_SIZE>
    struct QueueNode {
        QueueNode() = default;
        ~QueueNode() = default;

        DEFAULT_COPY_SEMANTIC(QueueNode);
        DEFAULT_MOVE_SEMANTIC(QueueNode);

        size_t id {0};
        std::atomic<QueueNode *> next {nullptr};
        std::array<U, NODE_SIZE> buffer {};
    };
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    using NodeType = QueueNode<T, QUEUE_NODE_SIZE>;
    using NodePtr = NodeType *;

    class PopUserScope {
    public:
        explicit PopUserScope(SPMCLockFreeQueue *queue) : queue_(queue)
        {
            // Atomic with acq_rel order reason: other threads should see correct value
            queue_->popUserCount_.fetch_add(1U, std::memory_order_acq_rel);
        }
        ~PopUserScope()
        {
            // Atomic with acq_rel order reason: other threads should see correct value
            queue_->popUserCount_.fetch_sub(1U, std::memory_order_acq_rel);
        }

        NO_COPY_SEMANTIC(PopUserScope);
        NO_MOVE_SEMANTIC(PopUserScope);

    private:
        SPMCLockFreeQueue *queue_ = nullptr;
    };

public:
    using ThreadId = size_t;
    using ValueType = T;

    SPMCLockFreeQueue();
    ~SPMCLockFreeQueue();

    NO_COPY_SEMANTIC(SPMCLockFreeQueue);
    NO_MOVE_SEMANTIC(SPMCLockFreeQueue);

    /**
     * @brief Registers thread in queue, should be called only on time by one thread.
     * @returns Unique id for thread
     */
    size_t RegisterConsumer();

    /**
     * @brief Method pushes value in queue. Method can allocate new queue node.
     * @param val: value that should be pushed.
     */
    void Push(T &&val);

    /**
     * @brief Method pops value from queue.
     * @param consumer_id: unique consumer id that was gotten with RegisterConsumerMethod
     * @returns value if queue have it, otherwise std::nullopt.
     */
    std::optional<T> Pop(size_t id);

    /// @returns true if queue is empty.
    bool inline IsEmpty() const;

    /// @returns count of tasks inside queue.
    size_t inline Size() const;

    /**
     * @brief Method deletes all retired nodes. Use this method if you know that no consumer
     * threads will use Pop method until method execution ends
     */
    void TryDeleteRetiredPtrs()
    {
        if (LIKELY(GetPopUserCount() == 0)) {
            // Atomic with acquire order reason: get the latest value
            auto currentCountOfThreads = consumerCount_.load(std::memory_order_acquire);
            for (size_t id = 0; id < currentCountOfThreads; id++) {
                auto &retiredPtrs = perConsumerRetiredPtrs_[id];
                for (auto node : retiredPtrs) {
                    delete node;
                }
                retiredPtrs.clear();
            }
        }
    }

private:
    size_t inline GetBufferIndex(size_t index) const
    {
        return index & QUEUE_NODE_MASK;
    }

    size_t inline GetNodeId(size_t index) const
    {
        return index >> QUEUE_NODE_SHIFT;
    }

    /// Method @returns value of popUserCount_
    size_t GetPopUserCount() const
    {
        // Atomic with acquire order reason: getting correct value of popUserCount
        return popUserCount_.load(std::memory_order_acquire);
    }

    /**
     * @brief Method load atomic ptr to QueueNode
     * @param atomicPtr: ref to prt that should be load
     * @return correct value of atomicPtr
     */
    NodePtr LoadAtomicPtr(std::atomic<NodePtr> &atomicPtr)
    {
        // Atomic with acquire order reason: get the latest value
        return atomicPtr.load(std::memory_order_acquire);
    }

    /**
     * @brief Writes ptr to list of retired ptrs to delete them later
     * @param consumerId: id of consumer that retire ptr
     * @param ptr: pointer to QueueNode that should be retired
     */
    void RetirePtr(size_t consumerId, NodePtr ptr)
    {
        auto &retiredPtrs = perConsumerRetiredPtrs_[consumerId];
        retiredPtrs.push_back(ptr);
    }

    /**
     * @brief Method tries move pop_index_ on next position.
     * @param current_pop_index: value of pop_index_ that are owned by thread
     * @returns true if currant_pop_index==pop_index and CAS was done, else false
     */
    inline bool TryMovePopIndex(size_t popIndex)
    {
        return popIndex_.compare_exchange_strong(
            popIndex, popIndex + 1U,
            // Atomic with acq_rel order reason: other threads should be correct value
            std::memory_order_acq_rel);
    }

    /**
     * @brief Method tries sets next head based on current. Also it's move old ptr to retired list
     * @param currentHead: value of head that was loaded
     * @param val: ref to T type variable to write first element of next node
     * @param consumerId: id of consumer that want to set new head
     * @returns true if head was changed, otherwise returns false
     */
    bool CompareAndSetNextHead(NodePtr currentHead, T &val, size_t consumerId)
    {
        auto nextHead = LoadAtomicPtr(currentHead->next);
        ASSERT(nextHead != nullptr);
        //   Atomic with acq_rel order reason: other threads should be correct value
        if (LIKELY(head_.compare_exchange_strong(currentHead, nextHead, std::memory_order_acq_rel))) {
            // Now we should correctly delete a current_head.
            RetirePtr(consumerId, currentHead);
            // Finally we can return first value of new head
            val = std::move(nextHead->buffer[0]);
            return true;
        }
        return false;
    }

    alignas(ark::COHERENCY_LINE_SIZE) std::atomic_size_t pushIndex_ {0};
    alignas(ark::COHERENCY_LINE_SIZE) std::atomic_size_t popIndex_ {0};

    alignas(ark::COHERENCY_LINE_SIZE) std::atomic<NodePtr> head_ {nullptr};
    alignas(ark::COHERENCY_LINE_SIZE) std::atomic<NodePtr> tail_ {nullptr};

    alignas(ark::COHERENCY_LINE_SIZE) std::atomic_size_t popUserCount_ {0};
    alignas(ark::COHERENCY_LINE_SIZE) std::atomic_size_t consumerCount_ {0};
    alignas(ark::COHERENCY_LINE_SIZE) std::array<std::vector<NodePtr>, CONSUMER_MAX_COUNT> perConsumerRetiredPtrs_;
};

template <class T, size_t QUEUE_NODE_SIZE>
SPMCLockFreeQueue<T, QUEUE_NODE_SIZE>::SPMCLockFreeQueue()
{
    for (size_t id = 0; id < CONSUMER_MAX_COUNT; id++) {
        perConsumerRetiredPtrs_[id] = {};
    }
    auto *node = new NodeType();
    // Atomic with release order reason: other threads should see correct value
    head_.store(node, std::memory_order_release);
    // Atomic with release order reason: other threads should see correct value
    tail_.store(node, std::memory_order_release);
}

template <class T, size_t QUEUE_NODE_SIZE>
SPMCLockFreeQueue<T, QUEUE_NODE_SIZE>::~SPMCLockFreeQueue()
{
    // Check if now there are no elements in queue
    ASSERT(IsEmpty());
    ASSERT(GetPopUserCount() == 0);
    TryDeleteRetiredPtrs();
    // Atomic with acquire order reason: get the latest value
    auto head = head_.load(std::memory_order_acquire);
    delete head;
}

template <class T, size_t QUEUE_NODE_SIZE>
size_t SPMCLockFreeQueue<T, QUEUE_NODE_SIZE>::RegisterConsumer()
{
    // Atomic with acq_rel order reason: other threads should be correct value
    auto id = consumerCount_.fetch_add(1UL, std::memory_order_acq_rel);
    ASSERT(id < CONSUMER_MAX_COUNT);
    return id;
}

template <class T, size_t QUEUE_NODE_SIZE>
size_t inline SPMCLockFreeQueue<T, QUEUE_NODE_SIZE>::Size() const
{
    while (true) {
        // Atomic with acquire order reason: get the latest value
        auto pushIndex = pushIndex_.load(std::memory_order_acquire);
        // Atomic with acquire order reason: get the latest value
        auto popIndex = popIndex_.load(std::memory_order_acquire);
        // Atomic with acquire order reason: get the latest value
        if (UNLIKELY(pushIndex < popIndex && popIndex != pushIndex_.load(std::memory_order_acquire))) {
            continue;
        }
        return pushIndex - popIndex;
    }
}

template <class T, size_t QUEUE_NODE_SIZE>
bool inline SPMCLockFreeQueue<T, QUEUE_NODE_SIZE>::IsEmpty() const
{
    return Size() == 0;
}

template <class T, size_t QUEUE_NODE_SIZE>
void SPMCLockFreeQueue<T, QUEUE_NODE_SIZE>::Push(T &&val)
{
    // Atomic with acquire order reason: get the latest value
    auto pushIndex = pushIndex_.load(std::memory_order_acquire);

    // Atomic with acquire order reason: get the latest value
    auto currentTail = tail_.load(std::memory_order_acquire);
    auto bufferIndex = GetBufferIndex(pushIndex);
    if (UNLIKELY(bufferIndex == 0 && pushIndex != 0)) {
        // Creating new node
        auto newNode = new NodeType();
        // We set new id that define a order of nodes
        newNode->id = currentTail->id + 1UL;
        newNode->buffer[0] = std::move(val);
        // Atomic with release order reason: other threads should see correct value
        currentTail->next.store(newNode, std::memory_order_release);
        // Atomic with release order reason: other threads should see correct value
        tail_.store(newNode, std::memory_order_release);
    } else {
        // otherwise we can write new val in buffer
        currentTail->buffer[bufferIndex] = std::move(val);
    }
    ASSERT(pushIndex != SIZE_MAX);
    // We have finished with pushing so we need to mode push_index_
    // Atomic with release order reason: other threads should see correct value
    pushIndex_.store(pushIndex + 1UL, std::memory_order_release);
}

template <class T, size_t QUEUE_NODE_SIZE>
std::optional<T> SPMCLockFreeQueue<T, QUEUE_NODE_SIZE>::Pop(size_t consumerId)
{
    // Atomic with acquire order reason: get the latest value
    ASSERT(consumerId <= consumerCount_.load(std::memory_order_acquire));
    PopUserScope popUser(this);
    while (true) {
        // Atomic with acquire order reason: get the latest value
        auto pushIndex = pushIndex_.load(std::memory_order_acquire);
        // Atomic with acquire order reason: get the latest value
        auto popIndex = popIndex_.load(std::memory_order_acquire);
        // Check if we loaded correct indexes
        if (UNLIKELY(pushIndex < popIndex)) {
            // Retry
            continue;
        }
        // if no tasks we can return nullopt
        if (UNLIKELY(pushIndex == popIndex)) {
            // Atomic with acquire order reason: get the latest value
            if (pushIndex == pushIndex_.load(std::memory_order_acquire)) {
                return std::nullopt;
            }
            continue;
        }
        auto bufferIndex = GetBufferIndex(popIndex);
        auto nodeId = GetNodeId(popIndex);
        auto *currentHead = LoadAtomicPtr(head_);
        // Check if current_head is correct indexes corresponds to current_head.
        if (UNLIKELY(currentHead == nullptr)) {
            // Retry
            continue;
        }
        if (UNLIKELY(currentHead->id > nodeId)) {
            // Retry
            continue;
        }
        T val;
        // Check if need to change head_.
        if (UNLIKELY(bufferIndex == 0 && popIndex != 0)) {
            // We should move pop_index_ first to occupy current_pop_index.
            if (UNLIKELY(!TryMovePopIndex(popIndex))) {
                // if we can not occupy it we should retry.
                continue;
            }
            if (LIKELY(CompareAndSetNextHead(currentHead, val, consumerId))) {
                return val;
            }
            continue;
        }
        // Else we can assume that pop_index was moved but head_ may still be old one, we should check it
        if (UNLIKELY(currentHead->id < nodeId)) {
            // Help to change head_ to a new one and return it's first element
            if (LIKELY(CompareAndSetNextHead(currentHead, val, consumerId))) {
                return val;
            }
            continue;
        }
        // Otherwise we try to occupy index
        if (UNLIKELY(!TryMovePopIndex(popIndex))) {
            // if we can not occupy it we should retry.
            continue;
        }
        return std::move(currentHead->buffer[bufferIndex]);
    }
}

}  // namespace ark::taskmanager::internal

#endif  // LIBPANDABASE_TASKMANAGER_UTILS_SP_MC_LOCK_FREE_QUEUE_H
