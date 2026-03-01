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

#ifndef LSPSERVER_THRDPOOL_H
#define LSPSERVER_THRDPOOL_H

#include <functional>
#include <queue>
#include <thread>
#include <unordered_set>
#ifdef __APPLE__
#include <pthread.h>
#include "common/Constants.h"
#endif

namespace ark {
/**
 * @class ThrdPool
 * @brief A thread pool for task execution, supporting synchronous or asynchronous task execution
 *
 */
class ThrdPool {
public:
    using Task = std::function<void()>;
#ifdef __APPLE__
    explicit ThrdPool(const size_t threads) : stop(false)
    {
        for (size_t i = 0; i < threads; ++i) {
            auto wrapper = new TaskWrapper(this);
            pthread_t thread;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setstacksize(&attr, CONSTANTS::MAC_THREAD_STACK_SIZE);

            int result = pthread_create(&thread, &attr, ThreadRoutine, wrapper);
            if (result != 0) {
                Trace::Elog("Failed to create thread");
            }

            pthread_attr_destroy(&attr);
            workers.emplace_back(thread);
        }
    }
#else
    explicit ThrdPool(const size_t threads) : stop(false)
    {
        for (size_t i = 0; i < threads; ++i) {
            workers.emplace_back([this] {
                for (;;) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queueMu);
                        this->cv.wait(lock, [this] { return this->stop || !this->readyTasks.empty(); });
                        if (this->stop && this->readyTasks.empty()) {
                            return;
                        }
                        task = std::move(this->readyTasks.front());
                        this->readyTasks.pop();
                    }
                    task();
                }
            });
        }
    }
#endif

    template <class F, class... Args>
    void AddTask(const uint64_t taskId, const std::unordered_set<uint64_t> &dependencies, F &&func, Args &&...args)
    {
        {
            std::unique_lock<std::mutex> lock(queueMu);
            ++tasksRemaining; // Increment the count of remaining tasks
        }

        using TaskType = std::function<void()>;
        TaskType task = std::bind(std::forward<F>(func), std::forward<Args>(args)...);

        {
            std::unique_lock<std::mutex> lock(queueMu);
            taskMap[taskId] = task;

            for (uint64_t dep : dependencies) {
                taskDependencies[taskId].insert(dep);
                dependentTasks[dep].insert(taskId);
            }

            if (taskDependencies[taskId].empty()) {
                readyTasks.push(task);
                cv.notify_one();
            }
        }
    }

    void TaskCompleted(const uint64_t taskId, bool inPool = true)
    {
        std::unique_lock<std::mutex> lock(queueMu);
        for (uint64_t dep : dependentTasks[taskId]) {
            taskDependencies[dep].erase(taskId);
            if (taskDependencies[dep].empty()) {
                readyTasks.push(taskMap[dep]);
                cv.notify_one();
            }
        }

        // Clean up the completed task
        dependentTasks.erase(taskId);
        taskDependencies.erase(taskId);
        taskMap.erase(taskId);

        if (inPool && --tasksRemaining == 0) {
            completionCv.notify_all(); // Ensure to notify when all tasks complete
        }
    }

    void WaitUntilAllTasksComplete()
    {
        std::unique_lock<std::mutex> lock(queueMu);
        completionCv.wait(lock, [this] { return tasksRemaining == 0; });
    }

    ~ThrdPool()
    {
        {
            std::unique_lock<std::mutex> lock(queueMu);
            stop = true;
        }
        cv.notify_all();
#ifdef __APPLE__
        for (pthread_t &worker : workers) {
            pthread_join(worker, NULL);
        }
#else
        for (std::thread &worker : workers) {
            worker.join();
        }
#endif
    }

private:
#ifdef __APPLE__
    class TaskWrapper {
    public:
        explicit TaskWrapper(ThrdPool *pool) : pool_(pool) {}
        void operator()()
        {
            for (;;) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(pool_->queueMu);
                    pool_->cv.wait(lock, [this] { return pool_->stop || !pool_->readyTasks.empty(); });
                    if (pool_->stop && pool_->readyTasks.empty()) {
                        return;
                    }
                    task = std::move(pool_->readyTasks.front());
                    pool_->readyTasks.pop();
                }
                task();
            }
        }

    private:
        ThrdPool *pool_;
    };

    static void *ThreadRoutine(void *arg)
    {
        TaskWrapper *taskWrapper = static_cast<TaskWrapper *>(arg);
        (*taskWrapper)();
        delete taskWrapper;
        return nullptr;
    }

    std::vector<pthread_t> workers;
#else
    std::vector<std::thread> workers;
#endif
    std::unordered_map<uint64_t, Task> taskMap;
    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> taskDependencies;
    std::unordered_map<uint64_t, std::unordered_set<uint64_t>> dependentTasks;
    std::queue<Task> readyTasks;
    std::mutex queueMu;
    std::condition_variable cv;
    std::condition_variable completionCv;
    bool stop;
    size_t tasksRemaining = 0;
};
} // namespace ark

#endif // LSPSERVER_THRDPOOL_H
