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

#include <gtest/gtest.h>
#include <libpandabase/mem/ringbuf/lock_free_ring_buffer.h>
#include <queue>

namespace panda::mem {
static constexpr size_t ITERATIONS = 1000000;

#ifdef PANDA_NIGHTLY_TEST_ON
const uint64_t SEED = std::time(NULL);
#else
const uint64_t SEED = 1234;
#endif

constexpr auto DEFAULT_BUFFER_SIZE = 1024;

TEST(LockFreeRingBufferTest, EmptyTest)
{
    LockFreeBuffer<size_t, DEFAULT_BUFFER_SIZE> buffer;
    ASSERT_TRUE(buffer.IsEmpty());

    buffer.Push(123);
    ASSERT_FALSE(buffer.IsEmpty());
    buffer.Pop();
    ASSERT_TRUE(buffer.IsEmpty());
}

TEST(LockFreeRingBufferTest, FullTest)
{
    LockFreeBuffer<size_t, DEFAULT_BUFFER_SIZE> buffer;
    for (size_t i = 0; i < DEFAULT_BUFFER_SIZE - 1; i++) {
        buffer.Push(i);
    }
    // in buffer can be maximum RING_BUFFER_SIZE - 1 elements
    ASSERT_FALSE(buffer.TryPush(666));
    buffer.Pop();
    ASSERT_TRUE(buffer.TryPush(666));
}

TEST(LockFreeRingBufferTest, PushPopTest)
{
    srand(SEED);
    LockFreeBuffer<size_t, DEFAULT_BUFFER_SIZE> buffer;
    std::queue<size_t> queue;
    for (size_t i = 0; i < DEFAULT_BUFFER_SIZE - 1; i++) {
        buffer.Push(i);
        queue.push(i);
        if (i % ((rand() % 100) + 1) == 0 && !queue.empty()) {
            size_t buffer_pop = buffer.Pop();
            size_t queue_pop = queue.front();
            queue.pop();
            ASSERT_EQ(buffer_pop, queue_pop);
        }
    }
    while (!queue.empty()) {
        size_t buffer_pop = buffer.Pop();
        size_t queue_pop = queue.front();
        queue.pop();
        ASSERT_EQ(buffer_pop, queue_pop);
    }
    size_t x = 0;
    bool pop = buffer.TryPop(&x);
    ASSERT_FALSE(pop);
    ASSERT_EQ(x, 0);
}

void PopElementsFromBuffer(LockFreeBuffer<size_t, DEFAULT_BUFFER_SIZE> *buffer, std::atomic<bool> *pop_thread_started,
                           std::atomic<bool> *pop_thread_finished, size_t *pop_sum)
{
    pop_thread_started->store(true);
    ASSERT(*pop_sum == 0);

    while (!pop_thread_finished->load()) {
        size_t x;
        bool pop_success = buffer->TryPop(&x);
        if (pop_success) {
            *pop_sum += x;
        }
    }
}

TEST(LockFreeRingBufferTest, MultiThreadingTest)
{
    srand(SEED);
    LockFreeBuffer<size_t, DEFAULT_BUFFER_SIZE> buffer;
    std::atomic<bool> pop_thread_started = false;
    std::atomic<bool> pop_thread_finished = false;
    size_t pop_sum = 0;
    auto pop_thread = std::thread(PopElementsFromBuffer, &buffer, &pop_thread_started, &pop_thread_finished, &pop_sum);
    // wait until pop_thread starts to work
    while (!pop_thread_started.load()) {
    }

    size_t expected_sum = 0;
    size_t sum = 0;
    for (size_t i = 0; i < ITERATIONS; i++) {
        expected_sum += i;
        buffer.Push(i);
    }

    // wait pop_thread to process everything
    while (!buffer.IsEmpty()) {
    }
    pop_thread_finished.store(true);
    pop_thread.join();
    sum += pop_sum;  // can be without atomics because we use it after .join only -> HB
    ASSERT_EQ(sum, expected_sum);
}
}  // namespace panda::mem
