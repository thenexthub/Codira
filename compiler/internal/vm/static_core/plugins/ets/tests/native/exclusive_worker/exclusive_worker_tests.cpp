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

#include <atomic>
#include "plugins/ets/tests/ani/ani_gtest/ani_gtest.h"
#include "runtime/coroutines/coroutine.h"
#include "runtime/include/runtime.h"

namespace ark::ets::test {

class Event {
public:
    void Wait()
    {
        os::memory::LockHolder lh(lock_);
        while (!happened_) {
            cv_.Wait(&lock_);
        }
    }

    void Fire()
    {
        os::memory::LockHolder lh(lock_);
        happened_ = true;
        cv_.SignalAll();
    }

private:
    bool happened_ = false;
    os::memory::Mutex lock_;
    os::memory::ConditionVariable cv_;
};

class EtsNativeExclusiveWorkerTest : public ani::testing::AniTest {
public:
    template <typename... Args>
    void RunRoutineInExclusiveWorker(std::string_view routineName, std::string_view routineSignature, Args &&...args)
    {
        auto event = Event();

        std::thread worker([this, &event, routineName, routineSignature,
                            args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
            WorkerRoutine(event, routineName, routineSignature, std::forward<Args>(args)...);
        });

        event.Wait();
        worker.detach();
    }

    template <typename... Args>
    static bool CallStaticBooleanMethod(ani_env *env, std::string_view methodName, std::string_view signature,
                                        Args &&...args)
    {
        auto func = ResolveFunction(env, methodName, signature);
        ani_boolean result;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        env->Function_Call_Boolean(func, &result, std::forward<Args>(args)...);
        return result;
    }

    template <typename... Args>
    static void CallStaticVoidMethod(ani_env *env, std::string_view methodName, std::string_view signature,
                                     Args &&...args)
    {
        auto func = ResolveFunction(env, methodName, signature);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        env->Function_Call_Void(func, std::forward<Args>(args)...);
    }

    std::vector<ani_option> GetExtraAniOptions() override
    {
        return {
            ani_option {"--ext:gc-type=g1-gc", nullptr},
            ani_option {"--ext:compiler-enable-jit", nullptr},
            // Note: here set the coroutine-e-workers-limit to 2 to save system resources
            ani_option {"--ext:coroutine-e-workers-limit=2", nullptr},
            // Note: reduce the maximum the coroutine number here to save system resources in the limit test
            ani_option {"--ext:coroutines-stack-mem-limit=268435456", nullptr},
        };
    }

    void ReachLimitationOfCreatingEACoroutine()
    {
        auto event = Event();
        std::atomic_size_t count {0};
        // The eaworker limit is 5(2 is set in GetExtraAniOptions, 3 is for taskpool eaworker).
        std::thread worker1([this, &event, &count]() mutable { WaitUntilReachLimit(event, count); });
        std::thread worker2([this, &event, &count]() mutable { WaitUntilReachLimit(event, count); });
        std::thread worker3([this, &event, &count]() mutable { WaitUntilReachLimit(event, count); });
        std::thread worker4([this, &event, &count]() mutable { WaitUntilReachLimit(event, count); });
        std::thread worker5([this, &event, &count]() mutable { WaitUntilReachLimit(event, count); });
        std::thread worker6([this, &event, &count]() mutable { WaitUntilReachLimit(event, count); });

        worker1.join();
        worker2.join();
        worker3.join();
        worker4.join();
        worker5.join();
        worker6.join();
        event.Wait();
        ASSERT(count == 1);
    }

    template <typename... Args>
    void UseAfterDetach(std::string_view routineName, std::string_view routineSignature, Args &&...args)
    {
        auto event = Event();
        std::thread worker([this, &event, routineName, routineSignature,
                            args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
            WorkerRoutine(event, routineName, routineSignature, std::forward<Args>(args)...);
        });

        ani_boolean res {};
        res = CallStaticBooleanMethod(env_, routineName, routineSignature, std::forward<Args>(args)...);
        ASSERT_EQ(res, false);
        event.Wait();
        worker.detach();
    }

    template <typename... Args>
    void AttachSameThread(std::string_view routineName, std::string_view routineSignature, Args &&...args)
    {
        auto event = Event();
        std::thread worker([this, &event, routineName, routineSignature,
                            args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
            ASSERT(Thread::GetCurrent() == nullptr);
            [[maybe_unused]] ani_env *workerEnv = nullptr;
            ani_options aniArgs {0, nullptr};
            [[maybe_unused]] auto status = vm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &workerEnv);
            ASSERT(status == ANI_OK);
            event.Fire();

            // Cannot attach current thread, thread has already been attached
            status = vm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &workerEnv);
            ASSERT(status == ANI_ERROR);

            ani_int eWorkerId = Coroutine::GetCurrent()->GetWorker()->GetId();
            CallStaticVoidMethod(workerEnv, "setWorkerId", "i:", eWorkerId);

            ani_boolean res {};
            res = CallStaticBooleanMethod(workerEnv, routineName, routineSignature, std::forward<Args>(args)...);
            ASSERT_EQ(res, true);

            status = vm_->DetachCurrentThread();
            ASSERT(status == ANI_OK);
        });

        event.Wait();
        worker.detach();
    }

private:
    void WaitUntilReachLimit(Event &event, std::atomic_size_t &count)
    {
        ASSERT(Thread::GetCurrent() == nullptr);
        [[maybe_unused]] ani_env *workerEnv = nullptr;
        ani_options aniArgs {0, nullptr};
        [[maybe_unused]] auto status = vm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &workerEnv);
        if (status == ANI_OK) {
            event.Wait();
            status = vm_->DetachCurrentThread();
            ASSERT(status == ANI_OK);
        } else {
            count++;
            event.Fire();
        }
    }

    template <typename... Args>
    void WorkerRoutine(Event &event, std::string_view routineName, std::string_view routineSignature, Args &&...args)
    {
        ASSERT(Thread::GetCurrent() == nullptr);
        [[maybe_unused]] ani_env *workerEnv = nullptr;
        ani_options aniArgs {0, nullptr};
        [[maybe_unused]] auto status = vm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &workerEnv);
        ASSERT(status == ANI_OK);

        event.Fire();

        ani_int eWorkerId = Coroutine::GetCurrent()->GetWorker()->GetId();
        CallStaticVoidMethod(workerEnv, "setWorkerId", "i:", eWorkerId);

        ani_boolean res {};
        res = CallStaticBooleanMethod(workerEnv, routineName, routineSignature, std::forward<Args>(args)...);
        ASSERT_EQ(res, true);

        status = vm_->DetachCurrentThread();
        ASSERT(status == ANI_OK);
    }

    static ani_function ResolveFunction(ani_env *env, std::string_view methodName, std::string_view signature)
    {
        ani_module md;
        [[maybe_unused]] auto status = env->FindModule("exclusive_worker_tests", &md);
        ASSERT(status == ANI_OK);
        ani_function func;
        status = env->Module_FindFunction(md, methodName.data(), signature.data(), &func);
        ASSERT(status == ANI_OK);
        return func;
    }
};

TEST_F(EtsNativeExclusiveWorkerTest, CallMethod)
{
    RunRoutineInExclusiveWorker("call", ":z");
}

TEST_F(EtsNativeExclusiveWorkerTest, AsyncCallMethod)
{
    RunRoutineInExclusiveWorker("asyncCall", ":z");
}

TEST_F(EtsNativeExclusiveWorkerTest, LaunchCallMethod)
{
    RunRoutineInExclusiveWorker("launchCall", ":z");
}

TEST_F(EtsNativeExclusiveWorkerTest, ConcurrentWorkerAndRuntimeDestroy)
{
    RunRoutineInExclusiveWorker("eWorkerRoutine", ":z");
    CallStaticVoidMethod(env_, "mainRoutine", ":");
}

TEST_F(EtsNativeExclusiveWorkerTest, AttachSameThread)
{
    AttachSameThread("call", ":z");
}

TEST_F(EtsNativeExclusiveWorkerTest, CreateLimitExclusiveWorkers)
{
    ReachLimitationOfCreatingEACoroutine();
}

TEST_F(EtsNativeExclusiveWorkerTest, DetachWithoutAttach)
{
    auto status = vm_->DetachCurrentThread();
    ASSERT_EQ(status, ANI_ERROR);
}

TEST_F(EtsNativeExclusiveWorkerTest, UseAfterDetach)
{
    UseAfterDetach("call", ":z");
}

TEST_F(EtsNativeExclusiveWorkerTest, ScheduleACoroutine)
{
    RunRoutineInExclusiveWorker("scheduleACoroutine", ":z");
}

TEST_F(EtsNativeExclusiveWorkerTest, RecursiveLaunch)
{
    RunRoutineInExclusiveWorker("recursiveLaunch", ":z");
}

TEST_F(EtsNativeExclusiveWorkerTest, TestOOMinEACoroutine)
{
    RunRoutineInExclusiveWorker("testOOM", ":z");
}

TEST_F(EtsNativeExclusiveWorkerTest, TestExceptionsInEACoroutine)
{
    RunRoutineInExclusiveWorker("throwExceptions", ":z");
}

TEST_F(EtsNativeExclusiveWorkerTest, ScheduleAJCoroutine)
{
    RunRoutineInExclusiveWorker("scheduleAJCoroutine", ":z");
}

TEST_F(EtsNativeExclusiveWorkerTest, RecursiveAsync)
{
    RunRoutineInExclusiveWorker("recursiveAsync", ":z");
}

TEST_F(EtsNativeExclusiveWorkerTest, ACoroutineCallAsyncFunctions)
{
    RunRoutineInExclusiveWorker("ACoroutineCallAsyncFunctions", ":z");
}

TEST_F(EtsNativeExclusiveWorkerTest, AsyncFunctionLaunchACoroutine)
{
    RunRoutineInExclusiveWorker("asyncFunctionLaunchACoroutine", ":z");
}

TEST_F(EtsNativeExclusiveWorkerTest, CommunicateBetweenEACoroutines)
{
    RunRoutineInExclusiveWorker("EACoroutineSendToMain", ":z");
    CallStaticVoidMethod(env_, "mainSendToEACoroutine", ":");
}

}  // namespace ark::ets::test
