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


#ifndef MRT_TASK_QUEUE_H
#define MRT_TASK_QUEUE_H

#include <condition_variable>
#include <cstdint>
#include <list>

#include "Base/Panic.h"
#include "Common/PageAllocator.h"
#include "GcRequest.h"
#include "Heap/Heap.h"
#include "Inspector/CodeHeapData.h"
#include "Inspector/HeapSnapshotJsonSerializer.h"

// gc task and task queue implementation
namespace MapleRuntime {
class GCTask {
public:
    enum class TaskType : uint32_t {
        TASK_TYPE_INVALID = 0,
        TASK_TYPE_TERMINATE_GC = 1,  // terminate gc
        TASK_TYPE_TIMEOUT_GC = 2,    // timeout gc
        TASK_TYPE_INVOKE_GC = 3,     // invoke gc
        TASK_TYPE_DUMP_HEAP = 4,     // dump heap
        TASK_TYPE_DUMP_HEAP_OOM = 5, // dump heap after oom
        TASK_TYPE_DUMP_HEAP_IDE = 6, // dump heap for IDE
    };

    enum TaskIndex : uint64_t {
        INVALID_TASK_INDEX = 0,
        TASK_INDEX_FOR_EXIT = 1,
        SYNC_TASK_MIN_INDEX = 2,
        // other task index among (SYNC_TASK_MIN_INDEX, ASYNC_TASK_INDEX).
        ASYNC_TASK_INDEX = std::numeric_limits<uint64_t>::max(),
    };

    explicit GCTask(TaskType type) : taskType(type), taskIndex(ASYNC_TASK_INDEX) {}
    virtual ~GCTask() = default;

    TaskType GetType() const { return taskType; }

    void SetRegionType(TaskType type) { taskType = type; }

    TaskIndex GetTaskIndex() const { return taskIndex; }

    void SetTaskIndex(TaskIndex index) { taskIndex = index; }

    virtual bool NeedFilter() const { return false; }

    virtual bool Execute(void* owner) = 0;

protected:
    GCTask(const GCTask& task) = default;
    virtual GCTask& operator=(const GCTask&) = default;
    TaskType taskType;
    TaskIndex taskIndex;
};

class GCExecutor : public GCTask {
public:
    // For a task, we give it a priority based on schedule type and gc reason.
    // Termination and timeout events get highest prio, and override lower-prio tasks.
    // Each gc invocation task gets its prio relative to its reason.
    // This prio is used by the async task queue.
    static constexpr uint32_t PRIO_TERMINATE = 0;
    static constexpr uint32_t PRIO_TIMEOUT = 1;
    static constexpr uint32_t PRIO_INVOKE_GC = 2;

    static_assert(PRIO_INVOKE_GC + static_cast<uint32_t>(GC_REASON_MAX) <= std::numeric_limits<uint32_t>::digits,
                  "task queue reached max capacity");

    GCExecutor() : GCTask(TaskType::TASK_TYPE_INVALID), gcReason(GC_REASON_INVALID) {}

    explicit GCExecutor(TaskType type) : GCTask(type), gcReason(GC_REASON_INVALID)
    {
        MRT_ASSERT(type != TaskType::TASK_TYPE_INVOKE_GC, "invalid gc task!");
    }

    GCExecutor(TaskType type, GCReason reason) : GCTask(type), gcReason(reason)
    {
        MRT_ASSERT(gcReason < GC_REASON_MAX, "invalid reason");
    }

    GCExecutor(const GCExecutor& task) = default;
    ~GCExecutor() override = default;
    GCExecutor& operator=(const GCExecutor&) = default;

    static inline GCExecutor GetGCExecutor(uint32_t prio)
    {
        if (prio == PRIO_TERMINATE) {
            return GCExecutor(TaskType::TASK_TYPE_TERMINATE_GC);
        } else if (prio == PRIO_TIMEOUT) {
            return GCExecutor(TaskType::TASK_TYPE_TIMEOUT_GC);
        } else if (prio - PRIO_INVOKE_GC < GC_REASON_MAX) {
            return GCExecutor(TaskType::TASK_TYPE_INVOKE_GC, static_cast<GCReason>(prio - PRIO_INVOKE_GC));
        } else {
            LOG(RTLOG_FATAL, "Invalid priority in GetGCRequestByPrio function");
            return GCExecutor();
        }
    }

    inline uint32_t GetPriority() const
    {
        if (taskType == TaskType::TASK_TYPE_TERMINATE_GC) {
            return PRIO_TERMINATE;
        } else if (taskType == TaskType::TASK_TYPE_TIMEOUT_GC) {
            return PRIO_TIMEOUT;
        } else if (taskType == TaskType::TASK_TYPE_INVOKE_GC) {
            return PRIO_INVOKE_GC + gcReason;
        }
        LOG(RTLOG_FATAL, "Invalid task in GetPriority function");
        return 0;
    }

    static inline GCExecutor GetInvalidExecutor() { return GCExecutor(); }

    inline bool IsInvalid() const
    {
        return (taskType == TaskType::TASK_TYPE_INVALID) && (gcReason == GC_REASON_INVALID);
    }

    // Only for asyn gc task queues,
    // the TaskType::TASK_TYPE_TIMEOUT_GC and TaskType::TASK_TYPE_TERMINATE_GC gc task will remove all others
    inline bool IsOverriding() const { return (taskType != TaskType::TASK_TYPE_INVOKE_GC); }

    inline GCReason GetGCReason() const { return gcReason; }

    inline void SetGCReason(GCReason reason) { gcReason = reason; }

    bool NeedFilter() const override { return true; }

    bool Execute(void* owner) override;

private:
    GCReason gcReason;
};

// Lockless async task queue implementation.
// This queue manages a list of deduplicated tasks.
// Each bit of the queueWord indicates the corresponding priority task.
// Lower bit indicates higher priority task.
template<typename T>
class LocklessTaskQueue {
public:
    // Add one async task to asyncTaskQueue
    // One higher priority task might erase all lower-priority tasks in queueWord
    void Push(const T& task)
    {
        bool overriding = task.IsOverriding();
        uint32_t taskMask = (1U << task.GetPriority());
        uint32_t oldWord = queueWord.load(std::memory_order_relaxed);
        uint32_t newWord = 0;
        do {
            if (overriding) {
                newWord = taskMask | ((taskMask - 1) & oldWord);
            } else {
                newWord = taskMask | oldWord;
            }
        } while (!queueWord.compare_exchange_weak(oldWord, newWord, std::memory_order_relaxed));
    }

    // Get the highest priority task in queueWord, or get one invalid task if queueWord is empty
    T Pop()
    {
        uint32_t oldWord = queueWord.load(std::memory_order_relaxed);
        uint32_t newWord = 0;
        uint32_t dequeued = oldWord;
        do {
            newWord = oldWord & (oldWord - 1);
            dequeued = oldWord;
        } while (!queueWord.compare_exchange_weak(oldWord, newWord, std::memory_order_relaxed));
        if (oldWord == 0) {
            return T::GetInvalidExecutor();
        }
        // count the number of trailing zeros
        return T::GetGCExecutor(__builtin_ctz(dequeued));
    }

    // When gc thread exits, clear all tasks in queueWord
    void Clear() { queueWord.store(0, std::memory_order_relaxed); }

private:
    std::atomic<uint32_t> queueWord = {};
};

template<typename T>
class TaskQueue {
    static_assert(std::is_base_of<GCTask, T>::value, "T is not a subclass of MapleRuntime::GCTask");

public:
    using TaskFilter = std::function<bool(T& oldTask, T& newTask)>;
    using TaskQueueType = std::list<T, StdContainerAllocator<T, GC_TASK_QUEUE>>;

    void Init() { syncTaskIndex = GCTask::SYNC_TASK_MIN_INDEX; }

    void Fini()
    {
        std::lock_guard<std::recursive_mutex> lock(taskQueueLock);
        asyncTaskQueue.Clear();
        syncTaskQueue.clear();
    }

    // Add one task to syncTaskQueue
    // Return the accumulated gc times
    uint64_t EnqueueSync(T& task, TaskFilter& filter)
    {
        std::unique_lock<std::recursive_mutex> lock(taskQueueLock);
        TaskQueueType& queue = syncTaskQueue;

        if (!queue.empty() && task.NeedFilter()) {
            for (auto iter = queue.rbegin(); iter != queue.rend(); ++iter) {
                if (filter(*iter, task)) {
                    return (*iter).GetTaskIndex();
                }
            }
        }
        task.SetTaskIndex(static_cast<GCTask::TaskIndex>(++syncTaskIndex));
        queue.push_back(task);
        taskQueueCondVar.notify_all();
        return task.GetTaskIndex();
    }

    // Add one task to asyncTaskQueue
    void EnqueueAsync(const T& task)
    {
        asyncTaskQueue.Push(task);
        std::unique_lock<std::recursive_mutex> lock(taskQueueLock);
        taskQueueCondVar.notify_all();
    }

    // Get one gc task from task queue
    // Firstly get from syncTaskQueue, then get from asyncTaskQueue
    T Dequeue()
    {
        std::cv_status cvResult = std::cv_status::no_timeout;
        std::chrono::nanoseconds waitTime(DEFAULT_GC_TASK_INTERVAL_TIMEOUT_NS);
        while (true) {
            std::unique_lock<std::recursive_mutex> lock(taskQueueLock);
            // check sync queue firstly
            if (!syncTaskQueue.empty()) {
                T curTask(syncTaskQueue.front());
                syncTaskQueue.pop_front();
                return curTask;
            }

            if (cvResult == std::cv_status::timeout && Heap::GetHeap().IsGCEnabled()) {
                asyncTaskQueue.Push(T(GCTask::TaskType::TASK_TYPE_TIMEOUT_GC));
            }

            T task = asyncTaskQueue.Pop();
            if (!task.IsInvalid()) {
                VLOG(GCPHASE, "dequeue gc task: type %u. reason %u", task.GetType(), task.GetGCReason());
                return task;
            } else {
                VLOG(GCPHASE, "invalid gc task: type %u, reason %u", task.GetType(), task.GetGCReason());
            }

            cvResult = taskQueueCondVar.wait_for(lock, waitTime);
        }
    }

    // GC thread polling task queue and execute gc task
    void DrainTaskQueue(void* owner)
    {
        while (true) {
            T task = Dequeue();
            if (!task.Execute(owner)) {
                Fini();
                break;
            }
        }
    }

private:
    static constexpr uint64_t DEFAULT_GC_TASK_INTERVAL_TIMEOUT_NS = 1000L * 1000 * 1000; // default 1s
    std::recursive_mutex taskQueueLock;
    std::condition_variable_any taskQueueCondVar;
    uint64_t syncTaskIndex = 0;
    TaskQueueType syncTaskQueue;
    LocklessTaskQueue<T> asyncTaskQueue;
};
} // namespace MapleRuntime
#endif // MRT_TASK_QUEUE_H
