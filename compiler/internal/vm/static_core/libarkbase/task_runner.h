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

#ifndef LIBPANDABASE_PANDA_TASK_RUNNER_H
#define LIBPANDABASE_PANDA_TASK_RUNNER_H

#include "libarkbase/macros.h"

#include <functional>
#include <thread>

// clang-format off
/**
*    ark::TaskRunner runner;
*
*    runner.AddFinalize(foo);
*    runner.AddCallbackOnSuccess(foo1);
*    runner.AddCallbackOnFail(foo2);
*    runner.SetTaskOnSuccess(next_task);
*    bool success = task();
*    TaskRunner::EndTask(runner, success); --> if success --
*                                              |           |
*                                              |           |
*                                              |           |
*                                              |           if has_task_on_success -->-- StartTask(runner, next_task);
*                                              |           |
*                                              |           |
*                                              |           |
*                                              |           else -->------ X ------ call foo1;
*                                              |                                   call foo;
*                                              |
*                                              |
*                                              |
*                                              else ------ if has_task_on_fail -->---- X
*                                                          |
*                                                          |
*                                                          |
*                                                          else -->---- call foo2;
*                                                                       call foo;
*/
// clang-format on

namespace ark {

/**
 * @brief TaskRunner class implements interface to add callbacks and set next tasks to be executed
 * @tparam RunnerT - an inherited class that contains ContextT and specifies StartTask and GetContext methods
 * @tparam ContexT - a class representing the context of the task
 */
template <typename RunnerT, typename ContextT>
class TaskRunner {
public:
    TaskRunner() = default;
    NO_COPY_SEMANTIC(TaskRunner);
    DEFAULT_MOVE_SEMANTIC(TaskRunner);
    ~TaskRunner() = default;

    // We must use && to avoid slicing
    using TaskFunc = std::function<void(RunnerT)>;
    using Callback = std::function<void(ContextT &)>;

    virtual ContextT &GetContext() = 0;
    static void StartTask(RunnerT taskRunner, TaskFunc taskFunc);

    class TaskCallback {
    public:
        TaskCallback() = default;
        NO_COPY_SEMANTIC(TaskCallback);
        NO_MOVE_OPERATOR(TaskCallback);
        TaskCallback(TaskCallback &&taskCb)
            : callbackOnSuccess_(std::move(taskCb.callbackOnSuccess_)),
              callbackOnFail_(std::move(taskCb.callbackOnFail_))
        {
            taskCb.SetNeedMakeCall(false);
        }
        // NOLINTNEXTLINE(modernize-use-equals-default)
        ~TaskCallback()
        {
            ASSERT(!needMakeCall_);
        }

        void RunOnSuccess(ContextT &taskCtx)
        {
            if (callbackOnSuccess_) {
                callbackOnSuccess_(taskCtx);
            }
            SetNeedMakeCall(false);
        }

        void RunOnFail(ContextT &taskCtx)
        {
            if (callbackOnFail_) {
                callbackOnFail_(taskCtx);
            }
            SetNeedMakeCall(false);
        }

        template <typename Foo>
        void AddOnSuccess(Foo foo)
        {
            if (LIKELY(callbackOnSuccess_)) {
                callbackOnSuccess_ = NextCallback(std::move(callbackOnSuccess_), std::move(foo));
            } else {
                callbackOnSuccess_ = std::move(foo);
            }
            SetNeedMakeCall(true);
        }

        template <typename Foo>
        void AddOnFail(Foo foo)
        {
            if (LIKELY(callbackOnFail_)) {
                callbackOnFail_ = NextCallback(std::move(callbackOnFail_), std::move(foo));
            } else {
                callbackOnFail_ = std::move(foo);
            }
            SetNeedMakeCall(true);
        }

        void SetNeedMakeCall([[maybe_unused]] bool needMakeCall)
        {
#ifndef NDEBUG
            needMakeCall_ = needMakeCall;
#endif
        }

    private:
        template <typename Foo>
        Callback NextCallback(Callback &&cb, Foo foo)
        {
            return [cb = std::move(cb), foo = std::move(foo)](ContextT &taskCtx) mutable {
                foo(taskCtx);
                cb(taskCtx);
            };
        }

        Callback callbackOnSuccess_;
        Callback callbackOnFail_;
#ifndef NDEBUG
        bool needMakeCall_ {false};
#endif
    };

    class Task {
    public:
        Task() = default;
        NO_COPY_SEMANTIC(Task);
        NO_MOVE_OPERATOR(Task);
        Task(Task &&task) : taskFunc_(std::move(task.taskFunc_)), hasTask_(task.hasTask_)
        {
            task.hasTask_ = false;
        }
        ~Task() = default;

        explicit operator bool() const
        {
            return hasTask_;
        }

        template <typename Foo>
        void SetTaskFunc(Foo foo)
        {
            ASSERT(!hasTask_);
            taskFunc_ = std::move(foo);
            hasTask_ = true;
        }

        TaskFunc GetTaskFunc()
        {
            hasTask_ = false;
            return std::move(taskFunc_);
        }

    private:
        TaskFunc taskFunc_;
        bool hasTask_ {false};
    };

    /// @brief Starts a chain of callbacks on success
    void RunCallbackOnSuccess()
    {
        auto &context = GetContext();
        taskCb_.RunOnSuccess(context);
    }

    /// @brief Starts a chain of callbacks on fail
    void RunCallbackOnFail()
    {
        auto &context = GetContext();
        taskCb_.RunOnFail(context);
    }

    /**
     * @brief Adds callback to the chain on success
     * @param foo - callable object that gets ContextT &
     */
    template <typename Foo>
    void AddCallbackOnSuccess(Foo foo)
    {
        ASSERT(!taskOnSuccess_);
        taskCb_.AddOnSuccess(std::move(foo));
    }

    /**
     * @brief Adds callback to the chain on fail
     * @param foo - callable object that gets ContextT &
     */
    template <typename Foo>
    void AddCallbackOnFail(Foo foo)
    {
        ASSERT(!taskOnFail_);
        taskCb_.AddOnFail(std::move(foo));
    }

    /**
     * @brief Adds callback to the chain on success and on fail
     * @param foo - callable object that gets ContextT &
     */
    template <typename Foo>
    void AddFinalize(Foo foo)
    {
        AddCallbackOnSuccess(foo);
        AddCallbackOnFail(std::move(foo));
    }

    /**
     * @brief Sets next task on success. After this call you must not add callback on success
     * @param foo - callable object that gets RunnerT &&
     */
    template <typename Foo>
    void SetTaskOnSuccess(Foo foo)
    {
        ASSERT(!taskOnSuccess_);
        taskOnSuccess_.SetTaskFunc(std::move(foo));
    }

    /**
     * @brief Sets next task on fail. After this call you must not add callback on fail
     * @param foo - callable object that gets RunnerT &&
     */
    template <typename Foo>
    void SetTaskOnFail(Foo foo)
    {
        ASSERT(!taskOnFail_);
        taskOnFail_.SetTaskFunc(std::move(foo));
    }

    /**
     * @brief Completes the current task and starts the next one if it was set.
     * Otherwise, it calls callbacks depending on @param success
     * @param task_runner - Current TaskRunner
     * @param success - result of current task
     */
    static void EndTask(RunnerT taskRunner, bool success)
    {
        auto &baseRunner = static_cast<TaskRunner &>(taskRunner);
        auto taskOnSuccess = std::move(baseRunner.taskOnSuccess_);
        auto taskOnFail = std::move(baseRunner.taskOnFail_);
        ASSERT(!baseRunner.taskOnSuccess_ && !baseRunner.taskOnFail_);
        ContextT &taskCtx = taskRunner.GetContext();
        if (success) {
            if (taskOnSuccess) {
                RunnerT::StartTask(std::move(taskRunner), taskOnSuccess.GetTaskFunc());
                return;
            }
            baseRunner.taskCb_.RunOnSuccess(taskCtx);
        } else {
            if (taskOnFail) {
                RunnerT::StartTask(std::move(taskRunner), taskOnFail.GetTaskFunc());
                return;
            }
            baseRunner.taskCb_.RunOnFail(taskCtx);
        }
    }

private:
    TaskCallback taskCb_;
    Task taskOnSuccess_;
    Task taskOnFail_;
};

}  // namespace ark

#endif  // LIBPANDABASE_PANDA_TASK_RUNNER_H
