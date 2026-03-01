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

#ifndef PANDA_LIBPANDABASE_MEM_RING_BUF_LOCK_FREE_BUFFER
#define PANDA_LIBPANDABASE_MEM_RING_BUF_LOCK_FREE_BUFFER

#include <atomic>
#include <cinttypes>
#include <thread>
#include "libarkbase/generated/coherency_line_size.h"
#include "libarkbase/utils/math_helpers.h"

namespace ark::mem {
/// Lock-free single-producer single-consumer ring-buffer. Push can take infinite amount of time if buffer is full.
template <typename T, size_t RING_BUFFER_SIZE>
class LockFreeBuffer {
public:
    static_assert(RING_BUFFER_SIZE > 0U, "0 is invalid size for ring buffer");
    static_assert(ark::helpers::math::IsPowerOfTwo(RING_BUFFER_SIZE));
    static constexpr size_t RING_BUFFER_SIZE_MASK = RING_BUFFER_SIZE - 1;
    static_assert(RING_BUFFER_SIZE_MASK > 0);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
    LockFreeBuffer()
    {
        // Atomic with release order reason: threads should see correct initialization
        tailIndex_.store(0, std::memory_order_release);
        // Atomic with release order reason: threads should see correct initialization
        headIndex_.store(0, std::memory_order_release);
        lastHeadIndex_ = 0;
        lastTailIndex_ = 0;
        CheckInvariant();
    }

    bool TryPush(const T val)
    {
        CheckInvariant();
        // Atomic with relaxed order reason: tailIndex_ is own atomic for with thread, so currect value is the last.
        // If this method can be called from different thread, usage of the method should be separated by time.
        const auto currentTail = tailIndex_.load(std::memory_order_relaxed);
        const auto nextTail = Increment(currentTail);
        if (nextTail == lastHeadIndex_) {
            // Atomic with acquire order reason: push should get the latest value
            lastHeadIndex_ = headIndex_.load(std::memory_order_acquire);
            if (nextTail == lastHeadIndex_) {
                return false;
            }
        }
        ASSERT(currentTail < RING_BUFFER_SIZE);
        buffer_[currentTail] = val;
        // Atomic with release order reason: to allow pop to see the latest value
        tailIndex_.store(nextTail, std::memory_order_release);
        return true;
    }

    void Push(const T val)
    {
        // NOLINTNEXTLINE(readability-braces-around-statements)
        while (!TryPush(val)) {
        };
    }

    bool IsEmpty()
    {
        CheckInvariant();
        // Atomic with acquire order reason: get the latest value
        auto localHead = headIndex_.load(std::memory_order_acquire);
        // Atomic with acquire order reason: get the latest value
        auto localTail = tailIndex_.load(std::memory_order_acquire);
        bool isEmpty = (localHead == localTail);
        return isEmpty;
    }

    bool TryPop(T *pval)
    {
        CheckInvariant();

        // Atomic with relaxed order reason: headIndex_ is own atomic for with thread, so currect value is the last.
        // If this method can be called from different thread, usage of the method should be separated by time.
        auto currentHead = headIndex_.load(std::memory_order_relaxed);
        if (currentHead == lastTailIndex_) {
            // Atomic with acquire order reason: get the latest value
            lastTailIndex_ = tailIndex_.load(std::memory_order_acquire);
            if (currentHead == lastTailIndex_) {
                return false;
            }
        }

        *pval = buffer_[currentHead];
        size_t newValue = Increment(currentHead);
        // Atomic with release order reason: let others threads to see the latest value
        headIndex_.store(newValue, std::memory_order_release);
        return true;
    }

    T Pop()
    {
        T ret;
        // NOLINTNEXTLINE(readability-braces-around-statements)
        while (!TryPop(&ret)) {
        }
        return ret;
    }

    static constexpr uint32_t GetTailIndexOffset()
    {
        return MEMBER_OFFSET(Self, tailIndex_);
    }

    static constexpr uint32_t GetHeadIndexOffset()
    {
        return MEMBER_OFFSET(Self, headIndex_);
    }

    static constexpr uint32_t GetBufferOffset()
    {
        return MEMBER_OFFSET(Self, buffer_);
    }

private:
    using Self = LockFreeBuffer<T, RING_BUFFER_SIZE>;

    alignas(ark::COHERENCY_LINE_SIZE) std::atomic<size_t> tailIndex_;
    std::atomic<size_t> headIndex_;
    alignas(ark::COHERENCY_LINE_SIZE) std::array<T, RING_BUFFER_SIZE> buffer_;
    alignas(ark::COHERENCY_LINE_SIZE) size_t lastHeadIndex_;
    alignas(ark::COHERENCY_LINE_SIZE) size_t lastTailIndex_;

    size_t Increment(size_t n)
    {
        return (n + 1) & RING_BUFFER_SIZE_MASK;
    }

    void CheckInvariant()
    {
#ifndef NDEBUG
        // Atomic with relaxed order reason: get the latest value
        [[maybe_unused]] auto localHead = headIndex_.load(std::memory_order_relaxed);
        ASSERT(localHead < RING_BUFFER_SIZE);

        // Atomic with relaxed order reason: get the latest value
        [[maybe_unused]] auto localTail = tailIndex_.load(std::memory_order_relaxed);
        ASSERT(localTail < RING_BUFFER_SIZE);
#endif
    }
};
}  // namespace ark::mem
#endif
