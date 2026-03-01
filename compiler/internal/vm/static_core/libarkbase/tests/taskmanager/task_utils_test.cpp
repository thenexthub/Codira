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

#include "libarkbase/taskmanager/utils/sp_mc_lock_free_queue.h"
#include "libarkbase/taskmanager/utils/wait_list.h"
#include "libarkbase/os/thread.h"
#include <gtest/gtest.h>
#include <thread>

namespace ark::taskmanager {

class TaskUtilityTest : public testing::Test {
public:
    static constexpr size_t ELEMENTS_MAX_COUNT = 10'000UL;
    TaskUtilityTest() = default;
    ~TaskUtilityTest() override = default;

    NO_COPY_SEMANTIC(TaskUtilityTest);
    NO_MOVE_SEMANTIC(TaskUtilityTest);
};

TEST_F(TaskUtilityTest, SPMCLockFreeQueueSPSCTest)
{
    internal::SPMCLockFreeQueue<int> queue;
    std::atomic_size_t consumerCounter = {0};
    std::atomic_size_t producerCounter = {0};

    std::thread producer([&queue, &producerCounter]() {
        for (size_t i = 0; i < ELEMENTS_MAX_COUNT; i++) {
            queue.Push(i);
            producerCounter += i;
        }
    });

    std::thread consumer([&queue, &consumerCounter]() {
        size_t id = queue.RegisterConsumer();
        for (size_t i = 0; i < ELEMENTS_MAX_COUNT;) {
            auto optVal = queue.Pop(id);
            if (optVal.has_value()) {
                i++;
                consumerCounter += optVal.value();
            }
        }
    });

    producer.join();
    consumer.join();

    ASSERT_EQ(producerCounter, consumerCounter);
}

TEST_F(TaskUtilityTest, SPMCLockFreeQueueSPMCTest)
{
    internal::SPMCLockFreeQueue<int> queue;
    std::atomic_size_t consumerCounter = {0};
    std::atomic_size_t producerCounter = {0};

    std::thread producer([&queue, &producerCounter]() {
        for (size_t i = 1; i < ELEMENTS_MAX_COUNT; i++) {
            queue.Push(i);
            producerCounter += i;
        }
    });

    std::vector<std::thread> consumers;
    constexpr size_t CONSUMER_COUNT = 5;
    std::atomic_size_t countOfPoppedTasks = 0;

    auto consumerFunc = [&queue, &consumerCounter, &countOfPoppedTasks]() {
        size_t id = queue.RegisterConsumer();

        while (countOfPoppedTasks < ELEMENTS_MAX_COUNT - 1) {
            auto optVal = queue.Pop(id);
            if (optVal.has_value()) {
                countOfPoppedTasks++;
                consumerCounter += optVal.value();
            }
        }
    };

    for (size_t i = 0; i < CONSUMER_COUNT; i++) {
        consumers.emplace_back(consumerFunc);
    }

    producer.join();
    for (auto &consumer : consumers) {
        consumer.join();
    }
    ASSERT_EQ(producerCounter, consumerCounter);
}

TEST_F(TaskUtilityTest, WaitListTests)
{
    WaitList<size_t> waitList;
    constexpr size_t RETURN_VALUE_1 = 42U;
    constexpr size_t WAIT_TIME_MS = 10U;
    // First test should show correct work of WaitList with waiting
    waitList.AddValueToWait(RETURN_VALUE_1, WAIT_TIME_MS);
    os::thread::NativeSleep(WAIT_TIME_MS);
    auto value = waitList.GetReadyValue();
    ASSERT_TRUE(value.has_value() && value.value() == RETURN_VALUE_1 && !waitList.HaveReadyValue());

    // Second test should show correct work of WaitList with large time to wait and correct signal
    constexpr size_t RETURN_VALUE_2 = 12345U;
    constexpr size_t LARG_TIME_TO_WAIT_MS = 1'000'000'000;  // it's about 11.5 days
    auto id = waitList.AddValueToWait(RETURN_VALUE_2, LARG_TIME_TO_WAIT_MS);
    value = waitList.GetReadyValue();
    ASSERT_TRUE(!value.has_value());

    ASSERT_TRUE(id != INVALID_WAITER_ID);
    value = waitList.GetValueById(id);
    ASSERT_TRUE(value.has_value() && value.value() == RETURN_VALUE_2 && !waitList.HaveReadyValue());

    // Final test: adding of 2 values with both cases that was showed previously
    id = waitList.AddValueToWait(RETURN_VALUE_2, LARG_TIME_TO_WAIT_MS);
    waitList.AddValueToWait(RETURN_VALUE_1, WAIT_TIME_MS);

    os::thread::NativeSleep(WAIT_TIME_MS);
    value = waitList.GetReadyValue();
    ASSERT_TRUE(value.has_value() && value.value() == RETURN_VALUE_1 && !waitList.HaveReadyValue());
    value = waitList.GetReadyValue();
    ASSERT_TRUE(!value.has_value());

    ASSERT_TRUE(id != INVALID_WAITER_ID);
    value = waitList.GetValueById(id);

    ASSERT_TRUE(value.has_value() && value.value() == RETURN_VALUE_2 && !waitList.HaveReadyValue());
}

}  // namespace ark::taskmanager
