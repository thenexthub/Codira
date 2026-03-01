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

#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_sync_primitives.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/tests/ani/ani_gtest/ani_gtest.h"

namespace ark::ets::test {

class EtsNativeQueueSpinlockTest : public ani::testing::AniTest {
public:
    void SetUp() override
    {
        ani::testing::AniTest::SetUp();
        BindSpinlockNativeFunctions();
    }

    template <typename... Args>
    static void CallStaticVoidMethod(ani_env *env, std::string_view methodName, std::string_view signature,
                                     Args &&...args)
    {
        auto func = ResolveFunction(env, methodName, signature);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        env->Function_Call_Void(func, std::forward<Args>(args)...);

        ani_boolean exceptionRaised {};
        env->ExistUnhandledError(&exceptionRaised);
        if (static_cast<bool>(exceptionRaised)) {
            env->DescribeError();
            env->Abort("The method call ended with an exception");
        }
    }

    std::vector<ani_option> GetExtraAniOptions() override
    {
        return {
            ani_option {"--ext:gc-type=g1-gc", nullptr},
            ani_option {"--ext:compiler-enable-jit", nullptr},
        };
    }

private:
    static ani_function ResolveFunction(ani_env *env, std::string_view methodName, std::string_view signature)
    {
        ani_module md;
        [[maybe_unused]] auto status = env->FindModule("queue_spinlock_tests", &md);
        ASSERT(status == ANI_OK);
        ani_function func;
        status = env->Module_FindFunction(md, methodName.data(), signature.data(), &func);
        ASSERT(status == ANI_OK);
        return func;
    }

    void BindSpinlockNativeFunctions()
    {
        ani_module module {};
        [[maybe_unused]] auto status = env_->FindModule("queue_spinlock_tests", &module);
        ASSERT(status == ANI_OK);
        std::array methods = {
            ani_native_function {"spinlockCreate", ":C{std.core.Object}", reinterpret_cast<void *>(SpinlockCreate)},
            ani_native_function {"spinlockGuard",
                                 "C{std.core.Object}C{std.core.Object}:", reinterpret_cast<void *>(SpinlockGuard)},
            ani_native_function {"spinlockIsHeld", "C{std.core.Object}:z", reinterpret_cast<void *>(SpinlockIsHeld)},
        };
        status = env_->Module_BindNativeFunctions(module, methods.data(), methods.size());
        ASSERT(status == ANI_OK);
    }

    static ani_object SpinlockCreate(ani_env *env)
    {
        ani::ScopedManagedCodeFix s(env);
        auto *coreSpinlock = EtsQueueSpinlock::Create(EtsCoroutine::GetCurrent());
        ani_object spinlock {};
        s.AddLocalRef(coreSpinlock, reinterpret_cast<ani_ref *>(&spinlock));
        return spinlock;
    }

    static void SpinlockGuard(ani_env *env, ani_object spinlock, ani_object callback)
    {
        ani::ScopedManagedCodeFix s(env);
        auto *coro = EtsCoroutine::GetCurrent();
        auto coreSpinlock = EtsQueueSpinlock::FromEtsObject(s.ToInternalType(spinlock));
        EtsHandleScope scope(coro);
        EtsHandle<EtsQueueSpinlock> hSpinlock(coro, coreSpinlock);
        EtsHandle<EtsObject> hCallback(coro, s.ToInternalType(callback));
        EtsQueueSpinlock::Guard guard(hSpinlock);
        LambdaUtils::InvokeVoid(coro, hCallback.GetPtr());
    }

    static ani_boolean SpinlockIsHeld(ani_env *env, ani_object spinlock)
    {
        ani::ScopedManagedCodeFix s(env);
        auto coreSpinlock = EtsQueueSpinlock::FromEtsObject(s.ToInternalType(spinlock));
        return static_cast<ani_boolean>(coreSpinlock->IsHeld());
    }
};

// Checks that the coroutine has exclusive access to the critical section
TEST_F(EtsNativeQueueSpinlockTest, ExclusiveAccess)
{
    CallStaticVoidMethod(env_, "exclusiveAccess", ":");
}

// Checks that the deadlock did not occur due to GC
TEST_F(EtsNativeQueueSpinlockTest, ObjectAllocationsUnderLock)
{
    CallStaticVoidMethod(env_, "objectAllocationsUnderLock", ":");
}

// Checks that the spinlock has been released in case of raised exception in callback
TEST_F(EtsNativeQueueSpinlockTest, ThrowingExceptionUnderLock)
{
    CallStaticVoidMethod(env_, "throwingExceptionUnderLock", ":");
}

}  // namespace ark::ets::test
