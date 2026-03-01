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
#include <libarkbase/mem/ringbuf/lock_free_ring_buffer.h>
#include <queue>

namespace ark::mem {
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

    // NOLINTNEXTLINE(readability-magic-numbers)
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
    // NOLINTNEXTLINE(cert-msc51-cpp)
    srand(SEED);
    LockFreeBuffer<size_t, DEFAULT_BUFFER_SIZE> buffer;
    std::queue<size_t> queue;
    for (size_t i = 0; i < DEFAULT_BUFFER_SIZE - 1; i++) {
        buffer.Push(i);
        queue.push(i);
        // NOLINTNEXTLINE(cert-msc50-cpp,readability-magic-numbers)
        if (i % ((rand() % 100) + 1) == 0 && !queue.empty()) {
            size_t bufferPop = buffer.Pop();
            size_t queuePop = queue.front();
            queue.pop();
            ASSERT_EQ(bufferPop, queuePop);
        }
    }
    while (!queue.empty()) {
        size_t bufferPop = buffer.Pop();
        size_t queuePop = queue.front();
        queue.pop();
        ASSERT_EQ(bufferPop, queuePop);
    }
    size_t x = 0;
    bool pop = buffer.TryPop(&x);
    ASSERT_FALSE(pop);
    ASSERT_EQ(x, 0);
}

void PopElementsFromBuffer(LockFreeBuffer<size_t, DEFAULT_BUFFER_SIZE> *buffer, std::atomic<bool> *popThreadStarted,
                           std::atomic<bool> *popThreadFinished, size_t *popSum)
{
    popThreadStarted->store(true);
    ASSERT(*popSum == 0);

    while (!popThreadFinished->load()) {
        size_t x;
        bool popSuccess = buffer->TryPop(&x);
        if (popSuccess) {
            *popSum += x;
        }
    }
}

TEST(LockFreeRingBufferTest, MultiThreadingTest)
{
    // NOLINTNEXTLINE(cert-msc51-cpp)
    srand(SEED);
    LockFreeBuffer<size_t, DEFAULT_BUFFER_SIZE> buffer;
    std::atomic<bool> popThreadStarted = false;
    std::atomic<bool> popThreadFinished = false;
    size_t popSum = 0;
    auto popThread = std::thread(PopElementsFromBuffer, &buffer, &popThreadStarted, &popThreadFinished, &popSum);
    // wait until pop_thread starts to work
    while (!popThreadStarted.load()) {
    }

    size_t expectedSum = 0;
    size_t sum = 0;
    for (size_t i = 0; i < ITERATIONS; i++) {
        expectedSum += i;
        buffer.Push(i);
    }

    // wait pop_thread to process everything
    while (!buffer.IsEmpty()) {
    }
    popThreadFinished.store(true);
    popThread.join();
    sum += popSum;  // can be without atomics because we use it after .join only -> HB
    ASSERT_EQ(sum, expectedSum);
}
}  // namespace ark::mem
