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

/**
 * @file
 *
 * This file declares some utility functions for Concurrent Task Queue.
 */

#ifndef CODIRA_UTILS_TASKQUEUE_H
#define CODIRA_UTILS_TASKQUEUE_H

#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include "Codira/Utils/CheckUtils.h"

namespace Codira::Utils {

template <typename TRes> using TaskResult = std::future<TRes>;

struct Task {
public:
    template <typename F> explicit Task(F&& fn, uint64_t priority) : fn(std::forward<F>(fn)), priority(priority)
    {
    }

    void operator()() const
    {
        fn();
    }

    bool operator<(const Task& other) const
    {
        return priority < other.priority;
    }

private:
    std::function<void()> fn;
    uint64_t priority; /**< A larger value indicates a higher priority. **/
};

/**
 * A heuristic parallel task queue. Each task needs to be created with a
 * specified weight. Each available thread selects the task at the head
 * of the queue from the task queue to execute. This means tasks with
 * higher weights can always be executed with higher priority.
 *
 * Note that **adding tasks** and **executing tasks** are phased and it
 * is not allowed to add tasks again after TaskQueue starts to execute.
 * Therefore, make sure all tasks have been added before TaskQueue is
 * executed. Otherwise, unintended behavior may occur.
 *
 * @tparam TRes The type of task result
 */
class TaskQueue {
public:
    /**
     * When the value of `threadsNum` is 0, the value of `threadsNum` is
     * changed to 1 to prevent queue tasks from having no executors.
     */
    explicit TaskQueue(size_t threadsNum) : threadsNum(threadsNum > 0 ? threadsNum : 1)
    {
    }

    ~TaskQueue()
    {
        threads.clear();
    }

    /**
     * Add a task into the queue. This is not a concurrency-safe method.
     * @tparam TRes The result type of the task.
     * @param fn Function to be executed by the task.
     * @param priority Priority of the task.
     * @return A place where stores the result of the task.
     */
    template <typename TRes> TaskResult<TRes> AddTask(const std::function<TRes()>& fn, uint64_t priority = 0U)
    {
        CODEC_ASSERT(!isStarted && "Do not add new tasks while executing.");
        auto task = std::make_shared<std::packaged_task<TRes()>>(fn);
        TaskResult<TRes> res = task->get_future();
        tasks.emplace([task]() mutable { (*task)(); }, priority);
        return res;
    }

    /**
     * Create threads and start executing tasks in the queue asynchronously
     * in the background.
     */
    void RunInBackground()
    {
        if (tasks.empty()) {
            return;
        }
        CreateThreads();
    }

    /**
     * Waiting for all threads to complete the tasks.
     */
    void WaitForAllTasksCompleted()
    {
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        threads.clear();
    }

    /**
     * Create threads to start asynchronously executing tasks in the queue and
     * waiting for all threads to complete the tasks.
     * Note: it will block the main thread.
     */
    void RunAndWaitForAllTasksCompleted()
    {
        if (tasks.empty()) {
            return;
        }
        CreateThreads();
        WaitForAllTasksCompleted();
    }

private:
    void CreateThreads()
    {
        isStarted = true;
        auto fixedThreadsNum = std::min(tasks.size(), threadsNum);
        for (size_t threadIdx = 0; threadIdx < fixedThreadsNum; ++threadIdx) {
            (void)threads.emplace_back([this] { DoTask(); });
        }
    }

    void DoTask()
    {
        // Once the thread is idle, it selects the task at the head of the queue to execute.
        while (true) {
            std::unique_lock<std::mutex> lock(mutex);
            // No remaining tasks. Thread exits polling.
            if (tasks.empty()) {
                return;
            }
            Task task = tasks.top();
            tasks.pop();
            lock.unlock();
            task();
        }
    }

    std::priority_queue<Task> tasks;
    std::mutex mutex;
    size_t threadsNum;
    std::vector<std::thread> threads;
    bool isStarted = false;
};
} // namespace Codira::Utils
#endif
