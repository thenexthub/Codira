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

#include "ets_interop_js_gtest.h"
#include "plugins/ets/runtime/interop_js/sts_vm_interface_impl.h"

#include <vector>
#include <queue>

namespace ark::ets::interop::js::testing {

class STSVMInterfaceImplTest : public EtsInteropTest {
public:
    using VMBarrier = STSVMInterfaceImpl::VMBarrier;
};

class BarrierTests : public STSVMInterfaceImplTest {
public:
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr size_t BARRIER_USERS_COUNT = 7U;
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr size_t PHASES_COUNT = 1'000U;

    BarrierTests()
    {
        threads_.reserve(BARRIER_USERS_COUNT);
    }

    void StartThreads(const std::function<void()> &func)
    {
        ASSERT_TRUE(threads_.empty());
        ASSERT_TRUE(threads_.capacity() == BARRIER_USERS_COUNT);
        for (size_t i = 0; i < BARRIER_USERS_COUNT; i++) {
            threads_.emplace_back(func);
        }
    }

    void JoinThreads()
    {
        for (size_t i = 0; i < BARRIER_USERS_COUNT; i++) {
            threads_[i].join();
        }
        threads_.clear();
    }

    class ThreadSafeQueue {
    public:
        void Push(size_t val)
        {
            os::memory::LockHolder lh(mutex_);
            queue_.push(val);
        }

        std::optional<size_t> Pop()
        {
            os::memory::LockHolder lh(mutex_);
            if (queue_.empty()) {
                return std::nullopt;
            }
            auto val = queue_.front();
            queue_.pop();
            return val;
        }

        size_t Size() const
        {
            os::memory::LockHolder lh(mutex_);
            return queue_.size();
        }

    private:
        std::queue<size_t> queue_;
        mutable os::memory::Mutex mutex_;
    };

private:
    std::vector<std::thread> threads_;
};

TEST_F(BarrierTests, WaitWithoutConditionTest)
{
    VMBarrier barrier(BARRIER_USERS_COUNT);
    std::atomic_size_t threadInPhaseCounter = 0U;
    auto barrierTestFunc = [&barrier, &threadInPhaseCounter] {
        barrier.InitialWait();
        for (size_t i = 0; i < PHASES_COUNT; i++) {
            auto count = threadInPhaseCounter++;
            // count should be less then BARRIER_USERS_COUNT, otherwise barrier is broken
            ASSERT_LT(count, BARRIER_USERS_COUNT);
            if (count + 1U == BARRIER_USERS_COUNT) {
                threadInPhaseCounter = 0U;
            }
            barrier.Wait();
        }
    };

    StartThreads(barrierTestFunc);
    // ... all the magic is happening ...
    JoinThreads();
}

TEST_F(BarrierTests, WaitWithConditionTest)
{
    VMBarrier barrier(BARRIER_USERS_COUNT + 1U);
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr size_t QUEUE_MAX_SIZE = 1'000U;
    std::atomic_bool goExecute = false;
    ThreadSafeQueue queue;

    auto barrierTestFunc = [&barrier, &queue, &goExecute] {
        barrier.InitialWait();
        while (!barrier.Wait([&goExecute, &queue]() { return !goExecute || queue.Size() == 0U; })) {
            if (queue.Size() != 0) {
                queue.Pop();
            }
        }
    };

    StartThreads(barrierTestFunc);

    barrier.InitialWait();
    for (size_t i = 0; i < QUEUE_MAX_SIZE; i++) {
        queue.Push(i);
    }
    ASSERT_EQ(queue.Size(), QUEUE_MAX_SIZE);
    goExecute = true;
    barrier.Signal();
    barrier.Wait();
    ASSERT_EQ(queue.Size(), 0U);

    JoinThreads();
}

TEST_F(BarrierTests, IncrementTest)
{
    VMBarrier barrier(BARRIER_USERS_COUNT);
    std::vector<size_t> valueStorage;
    std::atomic_size_t threadCounter = 0;
    os::memory::Mutex lock;

    auto barrierTestFunc = [&barrier, &valueStorage, &threadCounter, &lock] {
        auto subBarrierFunc = [&barrier, &threadCounter] {
            barrier.InitialWait();
            threadCounter++;
            barrier.Wait();
        };

        barrier.Increment();
        std::thread newThread(subBarrierFunc);
        barrier.InitialWait();
        threadCounter++;
        barrier.Wait();
        {
            os::memory::LockHolder lh(lock);
            valueStorage.push_back(threadCounter);
        }
        newThread.join();
    };

    StartThreads(barrierTestFunc);
    // ... all the magic is happening ...
    JoinThreads();

    ASSERT_EQ(valueStorage.size(), BARRIER_USERS_COUNT);
    for (const auto &value : valueStorage) {
        ASSERT_EQ(value, 2U * BARRIER_USERS_COUNT);
    }
}

TEST_F(BarrierTests, DecrementTest)
{
    VMBarrier barrier(BARRIER_USERS_COUNT + 1U);
    std::atomic_size_t threadNumber = 0;
    std::atomic_size_t waitCounter = 0;
    auto barrierTestFunc = [&barrier, &threadNumber, &waitCounter] {
        auto myNumber = threadNumber++;
        if (myNumber % 2 == 0) {
            barrier.Decrement();
        } else {
            waitCounter++;
            barrier.InitialWait();
        }
    };

    StartThreads(barrierTestFunc);

    barrier.InitialWait();
    ASSERT_EQ(waitCounter, BARRIER_USERS_COUNT / 2U);

    JoinThreads();
}

TEST_F(BarrierTests, TwoThreadScenario)
{
    STSVMInterfaceImpl stsVMIface;
    ThreadSafeQueue queue;
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr size_t PIPE_ELEMENT_COUNT = 100U;

    auto sthreadBody = [&stsVMIface, &queue] {
        stsVMIface.StartXGCBarrier(nullptr);
        // start pushing something for other thread
        for (size_t i = 0; i < PIPE_ELEMENT_COUNT; i++) {
            queue.Push(i);
        }
        stsVMIface.NotifyWaiters();
        stsVMIface.WaitForConcurrentMark(nullptr);
        stsVMIface.RemarkStartBarrier();
        for (size_t i = 0; i < PIPE_ELEMENT_COUNT; i++) {
            queue.Push(i);
        }
        stsVMIface.NotifyWaiters();
        stsVMIface.WaitForRemark(nullptr);
        stsVMIface.FinishXGCBarrier();
    };

    auto dthreadBody = [&stsVMIface, &queue] {
        stsVMIface.StartXGCBarrier(nullptr);
        size_t counter = 0;
        do {
            while (queue.Size() != 0) {
                counter++;
                queue.Pop();
            }
        } while (!stsVMIface.WaitForConcurrentMark([&queue] { return queue.Size() == 0; }));
        ASSERT_EQ(queue.Size(), 0);
        ASSERT_EQ(counter, PIPE_ELEMENT_COUNT);
        stsVMIface.RemarkStartBarrier();
        counter = 0;
        do {
            while (queue.Size() != 0) {
                counter++;
                queue.Pop();
            }
        } while (!stsVMIface.WaitForRemark([&queue] { return queue.Size() == 0; }));
        ASSERT_EQ(queue.Size(), 0);
        ASSERT_EQ(counter, PIPE_ELEMENT_COUNT);
        stsVMIface.FinishXGCBarrier();
    };
    std::thread sthread(sthreadBody);
    std::thread dthread(dthreadBody);
    // ... real magic is here ...
    sthread.join();
    dthread.join();
}

TEST_F(BarrierTests, StartXGCBarrierWithPredicate)
{
    STSVMInterfaceImpl stsVMIface;
    bool wasNotInterrupted = true;

    auto threadBody = [&stsVMIface, &wasNotInterrupted] {
        wasNotInterrupted = stsVMIface.StartXGCBarrier([] { return false; });
        ASSERT_FALSE(wasNotInterrupted);
        stsVMIface.NotifyWaiters();
    };

    std::thread thread(threadBody);

    ASSERT_FALSE(stsVMIface.StartXGCBarrier([&wasNotInterrupted] { return wasNotInterrupted; }));

    thread.join();
}

TEST_F(BarrierTests, StartXGCBarrierWithPredicate2)
{
    STSVMInterfaceImpl stsVMIface;

    auto threadBody = [&stsVMIface] {
        ASSERT_TRUE(stsVMIface.StartXGCBarrier([] { return true; }));
        stsVMIface.NotifyWaiters();
        stsVMIface.FinishXGCBarrier();
    };

    std::thread thread(threadBody);

    ASSERT_TRUE(stsVMIface.StartXGCBarrier([] { return true; }));
    stsVMIface.FinishXGCBarrier();

    thread.join();
}

TEST_F(BarrierTests, StartXGCBarrierWithoutPredicate)
{
    STSVMInterfaceImpl stsVMIface;

    auto threadBody = [&stsVMIface] {
        ASSERT_TRUE(stsVMIface.StartXGCBarrier(nullptr));
        stsVMIface.NotifyWaiters();
        stsVMIface.FinishXGCBarrier();
    };

    std::thread thread(threadBody);

    ASSERT_TRUE(stsVMIface.StartXGCBarrier(nullptr));
    stsVMIface.FinishXGCBarrier();

    thread.join();
}

}  // namespace ark::ets::interop::js::testing