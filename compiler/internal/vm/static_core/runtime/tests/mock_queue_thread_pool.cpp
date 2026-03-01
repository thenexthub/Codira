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

#include <gtest/gtest.h>

#include "runtime/include/runtime.h"
#include "runtime/thread_pool.h"

namespace ark::test {

constexpr int TEN = 10;
constexpr int PERCENT = 100;

class MockThreadPoolTest : public testing::Test {
public:
    static const size_t TASK_NUMBER = 32;
    MockThreadPoolTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~MockThreadPoolTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(MockThreadPoolTest);
    NO_MOVE_SEMANTIC(MockThreadPoolTest);

private:
    ark::MTManagedThread *thread_;
};

class MockTask : public TaskInterface {
public:
    explicit MockTask(size_t identifier = 0) : identifier_(identifier) {}

    enum TaskStatus {
        NOT_STARTED,
        IN_QUEUE,
        PROCESSING,
        COMPLETED,
    };

    bool IsEmpty() const
    {
        return identifier_ == 0;
    }

    size_t GetId() const
    {
        return identifier_;
    }

    TaskStatus GetStatus() const
    {
        return status_;
    }

    void SetStatus(TaskStatus status)
    {
        status_ = status;
    }

private:
    size_t identifier_;
    TaskStatus status_ = NOT_STARTED;
};

class MockQueue : public TaskQueueInterface<MockTask> {
public:
    explicit MockQueue(mem::InternalAllocatorPtr allocator) : queue_(allocator->Adapter()) {}
    MockQueue(mem::InternalAllocatorPtr allocator, size_t queueSize)
        : TaskQueueInterface<MockTask>(queueSize), queue_(allocator->Adapter())
    {
    }

    MockTask GetTask() override
    {
        if (queue_.empty()) {
            LOG(DEBUG, RUNTIME) << "Cannot get an element, queue is empty";
            return MockTask();
        }
        auto task = queue_.front();
        queue_.pop_front();
        LOG(DEBUG, RUNTIME) << "Extract task " << task.GetId();
        return task;
    }

    // NOLINTNEXTLINE(google-default-arguments)
    void AddTask(MockTask &&task, [[maybe_unused]] size_t priority = 0) override
    {
        task.SetStatus(MockTask::IN_QUEUE);
        queue_.push_front(task);
    }

    void Finalize() override
    {
        queue_.clear();
    }

protected:
    size_t GetQueueSize() override
    {
        return queue_.size();
    }

private:
    PandaList<MockTask> queue_;
};

class MockTaskController {
public:
    explicit MockTaskController() = default;

    void SolveTask(MockTask task)
    {
        task.SetStatus(MockTask::PROCESSING);
        // This is required to distribute tasks between different workers rather than solve it instantly
        // on only one worker.
        std::this_thread::sleep_for(std::chrono::milliseconds(TEN));
        task.SetStatus(MockTask::COMPLETED);
        LOG(DEBUG, RUNTIME) << "Task " << task.GetId() << " has been solved";
        solvedTasks_++;
    }

    size_t GetSolvedTasks()
    {
        return solvedTasks_;
    }

private:
    std::atomic_size_t solvedTasks_ = 0;
};

class MockProcessor : public ProcessorInterface<MockTask, MockTaskController *> {
public:
    explicit MockProcessor(MockTaskController *controller) : controller_(controller) {}

    bool Process(MockTask &&task) override
    {
        if (task.GetStatus() == MockTask::IN_QUEUE) {
            controller_->SolveTask(task);
            return true;
        }
        return false;
    }

private:
    MockTaskController *controller_;
};

void CreateTasks(ThreadPool<MockTask, MockProcessor, MockTaskController *> *threadPool, size_t numberOfElements)
{
    for (size_t i = 0; i < numberOfElements; i++) {
        MockTask task(i + 1);
        LOG(DEBUG, RUNTIME) << "Queue task " << task.GetId();
        // NOLINTNEXTLINE(performance-move-const-arg)
        threadPool->PutTask(std::move(task));
    }
}

void TestThreadPool(size_t initialNumberOfThreads, size_t scaledNumberOfThreads, float scaleThreshold)
{
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto queue = allocator->New<MockQueue>(allocator);
    auto controller = allocator->New<MockTaskController>();
    auto threadPool = allocator->New<ThreadPool<MockTask, MockProcessor, MockTaskController *>>(
        allocator, queue, controller, initialNumberOfThreads, "Test thread");

    CreateTasks(threadPool, MockThreadPoolTest::TASK_NUMBER);

    if (scaleThreshold < 1.0) {
        while (controller->GetSolvedTasks() < scaleThreshold * MockThreadPoolTest::TASK_NUMBER) {
        }
        threadPool->Scale(scaledNumberOfThreads);
    }

    for (;;) {
        auto solvedTasks = controller->GetSolvedTasks();
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto rate = static_cast<size_t>((static_cast<float>(solvedTasks) / MockThreadPoolTest::TASK_NUMBER) * 100);
        LOG(DEBUG, RUNTIME) << "Number of solved tasks is " << solvedTasks << " (" << rate << "%)";
        if (scaleThreshold == 1.0) {
            // NOLINTNEXTLINE(readability-magic-numbers)
            size_t dynamicScaling = rate / 10 + 1;
            threadPool->Scale(dynamicScaling);
        }

        if (solvedTasks == MockThreadPoolTest::TASK_NUMBER) {
            break;
        }
    }

    allocator->Delete(threadPool);
    allocator->Delete(controller);
    allocator->Delete(queue);
}

TEST_F(MockThreadPoolTest, SeveralThreads)
{
    constexpr size_t NUMBER_OF_THREADS_INITIAL = 8;
    constexpr size_t NUMBER_OF_THREADS_SCALED = 8;
    constexpr float SCALE_THRESHOLD = 0.0;
    TestThreadPool(NUMBER_OF_THREADS_INITIAL, NUMBER_OF_THREADS_SCALED, SCALE_THRESHOLD);
}

TEST_F(MockThreadPoolTest, ReduceThreads)
{
    constexpr size_t NUMBER_OF_THREADS_INITIAL = 8;
    constexpr size_t NUMBER_OF_THREADS_SCALED = 4;
    constexpr float SCALE_THRESHOLD = 0.25;
    TestThreadPool(NUMBER_OF_THREADS_INITIAL, NUMBER_OF_THREADS_SCALED, SCALE_THRESHOLD);
}

TEST_F(MockThreadPoolTest, IncreaseThreads)
{
    constexpr size_t NUMBER_OF_THREADS_INITIAL = 4;
    constexpr size_t NUMBER_OF_THREADS_SCALED = 8;
    constexpr float SCALE_THRESHOLD = 0.25;
    TestThreadPool(NUMBER_OF_THREADS_INITIAL, NUMBER_OF_THREADS_SCALED, SCALE_THRESHOLD);
}

TEST_F(MockThreadPoolTest, DifferentNumberOfThreads)
{
    constexpr size_t NUMBER_OF_THREADS_INITIAL = 8;
    constexpr size_t NUMBER_OF_THREADS_SCALED = 8;
    constexpr float SCALE_THRESHOLD = 1.0;
    TestThreadPool(NUMBER_OF_THREADS_INITIAL, NUMBER_OF_THREADS_SCALED, SCALE_THRESHOLD);
}

void ControllerThreadPutTask(ThreadPool<MockTask, MockProcessor, MockTaskController *> *threadPool,
                             size_t numberOfTasks)
{
    CreateTasks(threadPool, numberOfTasks);
}

void ControllerThreadTryPutTask(ThreadPool<MockTask, MockProcessor, MockTaskController *> *threadPool,
                                size_t numberOfTasks)
{
    for (size_t i = 0; i < numberOfTasks; i++) {
        for (;;) {
            if (threadPool->TryPutTask(MockTask {i + 1}) || !threadPool->IsActive()) {
                break;
            }
        }
    }
}

void ControllerThreadScale(ThreadPool<MockTask, MockProcessor, MockTaskController *> *threadPool,
                           size_t numberOfThreads)
{
    threadPool->Scale(numberOfThreads);
}

void ControllerThreadShutdown(ThreadPool<MockTask, MockProcessor, MockTaskController *> *threadPool, bool isShutdown,
                              bool isForceShutdown)
{
    if (isShutdown) {
        threadPool->Shutdown(isForceShutdown);
    }
}

void TestThreadPoolWithControllers(size_t numberOfThreadsInitial, size_t numberOfThreadsScaled, bool isShutdown,
                                   bool isForceShutdown)
{
    constexpr size_t NUMBER_OF_TASKS = MockThreadPoolTest::TASK_NUMBER / 4;
    constexpr size_t QUEUE_SIZE = 16;

    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();
    auto queue = allocator->New<MockQueue>(allocator, QUEUE_SIZE);
    auto controller = allocator->New<MockTaskController>();
    auto threadPool = allocator->New<ThreadPool<MockTask, MockProcessor, MockTaskController *>>(
        allocator, queue, controller, numberOfThreadsInitial, "Test thread");

    std::thread controllerThreadPutTask1(ControllerThreadPutTask, threadPool, NUMBER_OF_TASKS);
    std::thread controllerThreadPutTask2(ControllerThreadPutTask, threadPool, NUMBER_OF_TASKS);
    std::thread controllerThreadTryPutTask1(ControllerThreadTryPutTask, threadPool, NUMBER_OF_TASKS);
    std::thread controllerThreadTryPutTask2(ControllerThreadTryPutTask, threadPool, NUMBER_OF_TASKS);
    std::thread controllerThreadScale1(ControllerThreadScale, threadPool, numberOfThreadsScaled);
    std::thread controllerThreadScale2(ControllerThreadScale, threadPool,
                                       numberOfThreadsScaled + numberOfThreadsInitial);
    std::thread controllerThreadShutdown1(ControllerThreadShutdown, threadPool, isShutdown, isForceShutdown);
    std::thread controllerThreadShutdown2(ControllerThreadShutdown, threadPool, isShutdown, isForceShutdown);

    // Wait for tasks completion.
    for (;;) {
        auto solvedTasks = controller->GetSolvedTasks();
        auto rate = static_cast<size_t>((static_cast<float>(solvedTasks) / MockThreadPoolTest::TASK_NUMBER) * PERCENT);
        (void)rate;
        LOG(DEBUG, RUNTIME) << "Number of solved tasks is " << solvedTasks << " (" << rate << "%)";
        if (solvedTasks == MockThreadPoolTest::TASK_NUMBER || !threadPool->IsActive()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(TEN));
    }
    controllerThreadPutTask1.join();
    controllerThreadPutTask2.join();
    controllerThreadTryPutTask1.join();
    controllerThreadTryPutTask2.join();
    controllerThreadScale1.join();
    controllerThreadScale2.join();
    controllerThreadShutdown1.join();
    controllerThreadShutdown2.join();

    allocator->Delete(threadPool);
    allocator->Delete(controller);
    allocator->Delete(queue);
}

TEST_F(MockThreadPoolTest, Controllers)
{
    constexpr size_t NUMBER_OF_THREADS_INITIAL = 8;
    constexpr size_t NUMBER_OF_THREADS_SCALED = 4;
    constexpr bool IS_SHUTDOWN = false;
    constexpr bool IS_FORCE_SHUTDOWN = false;
    TestThreadPoolWithControllers(NUMBER_OF_THREADS_INITIAL, NUMBER_OF_THREADS_SCALED, IS_SHUTDOWN, IS_FORCE_SHUTDOWN);
}

TEST_F(MockThreadPoolTest, ControllersShutdown)
{
    constexpr size_t NUMBER_OF_THREADS_INITIAL = 8;
    constexpr size_t NUMBER_OF_THREADS_SCALED = 4;
    constexpr bool IS_SHUTDOWN = true;
    constexpr bool IS_FORCE_SHUTDOWN = false;
    TestThreadPoolWithControllers(NUMBER_OF_THREADS_INITIAL, NUMBER_OF_THREADS_SCALED, IS_SHUTDOWN, IS_FORCE_SHUTDOWN);
}

TEST_F(MockThreadPoolTest, ControllersForceShutdown)
{
    constexpr size_t NUMBER_OF_THREADS_INITIAL = 8;
    constexpr size_t NUMBER_OF_THREADS_SCALED = 4;
    constexpr bool IS_SHUTDOWN = true;
    constexpr bool IS_FORCE_SHUTDOWN = true;
    TestThreadPoolWithControllers(NUMBER_OF_THREADS_INITIAL, NUMBER_OF_THREADS_SCALED, IS_SHUTDOWN, IS_FORCE_SHUTDOWN);
}

}  // namespace ark::test
