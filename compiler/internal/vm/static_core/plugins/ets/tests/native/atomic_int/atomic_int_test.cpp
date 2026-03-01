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

#include "plugins/ets/tests/ani/ani_gtest/ani_gtest.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_atomic_int.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"

namespace ark::ets::test {

class EtsNativeAtomicIntTest : public ani::testing::AniTest {
public:
    void SetUp() override
    {
        ani::testing::AniTest::SetUp();
        BindAtomicIntNativeFunctions();
    }

    template <typename... Args>
    static void CallStaticIntMethod(ani_env *env, std::string_view methodName, std::string_view signature,
                                    Args &&...args)
    {
        auto func = ResolveFunction(env, methodName, signature);

        int res = 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        env->Function_Call_Int(func, std::forward<Args>(args)..., &res);

        ani_boolean exceptionRaised {};
        env->ExistUnhandledError(&exceptionRaised);
        if (static_cast<bool>(exceptionRaised)) {
            env->DescribeError();
            env->Abort("The method call ended with an exception");
        }
        ASSERT_EQ(res, 0) << "The method call returned an unexpected value";
    }

    std::vector<ani_option> GetExtraAniOptions() override
    {
        return {
            ani_option {"--ext:gc-type=g1-gc", nullptr},
        };
    }

private:
    static ani_function ResolveFunction(ani_env *env, std::string_view methodName, std::string_view signature)
    {
        ani_module md;
        [[maybe_unused]] auto status = env->FindModule("atomic_int_test", &md);
        ASSERT(status == ANI_OK);
        ani_function func;
        status = env->Module_FindFunction(md, methodName.data(), signature.data(), &func);
        ASSERT(status == ANI_OK);
        return func;
    }

    void BindAtomicIntNativeFunctions()
    {
        ani_class cls = {};
        [[maybe_unused]] auto status = env_->FindClass("atomic_int_test.AtomicInt", &cls);
        ASSERT(status == ANI_OK);
        std::array nativeMethods = {
            ani_native_function {"set", "i:", reinterpret_cast<void *>(EtsAtomicIntSetValue)},
            ani_native_function {"get", ":i", reinterpret_cast<void *>(EtsAtomicIntGetValue)},
            ani_native_function {"exchange", "i:i", reinterpret_cast<void *>(EtsAtomicIntGetAndSet)},
            ani_native_function {"compareAndSwap", "ii:i", reinterpret_cast<void *>(EtsAtomicIntCompareAndSet)},
            ani_native_function {"fetchAndAdd", "i:i", reinterpret_cast<void *>(EtsAtomicIntFetchAndAdd)},
            ani_native_function {"fetchAndSub", "i:i", reinterpret_cast<void *>(EtsAtomicIntFetchAndSub)},
        };

        status = env_->Class_BindNativeMethods(cls, nativeMethods.data(), nativeMethods.size());
        ASSERT(status == ANI_OK);
    }

    static void EtsAtomicIntSetValue(ani_env *env, ani_object atomicInt, ani_int value)
    {
        ani::ScopedManagedCodeFix s(env);
        auto etsAtomicInt = EtsAtomicInt::FromEtsObject(s.ToInternalType(atomicInt));
        etsAtomicInt->SetValue(value);
    }

    static ani_int EtsAtomicIntGetValue(ani_env *env, ani_object atomicInt)
    {
        ani::ScopedManagedCodeFix s(env);
        auto etsAtomicInt = EtsAtomicInt::FromEtsObject(s.ToInternalType(atomicInt));
        return static_cast<ani_int>(etsAtomicInt->GetValue());
    }

    static ani_int EtsAtomicIntGetAndSet(ani_env *env, ani_object atomicInt, ani_int value)
    {
        ani::ScopedManagedCodeFix s(env);
        auto etsAtomicInt = EtsAtomicInt::FromEtsObject(s.ToInternalType(atomicInt));
        return static_cast<ani_int>(etsAtomicInt->GetAndSet(value));
    }

    static ani_int EtsAtomicIntCompareAndSet(ani_env *env, ani_object atomicInt, ani_int expected, ani_int newValue)
    {
        ani::ScopedManagedCodeFix s(env);
        auto etsAtomicInt = EtsAtomicInt::FromEtsObject(s.ToInternalType(atomicInt));
        return static_cast<ani_int>(etsAtomicInt->CompareAndSet(expected, newValue));
    }

    static ani_int EtsAtomicIntFetchAndAdd(ani_env *env, ani_object atomicInt, ani_int value)
    {
        ani::ScopedManagedCodeFix s(env);
        auto etsAtomicInt = EtsAtomicInt::FromEtsObject(s.ToInternalType(atomicInt));
        return static_cast<ani_int>(etsAtomicInt->FetchAndAdd(value));
    }

    static ani_int EtsAtomicIntFetchAndSub(ani_env *env, ani_object atomicInt, ani_int value)
    {
        ani::ScopedManagedCodeFix s(env);
        auto etsAtomicInt = EtsAtomicInt::FromEtsObject(s.ToInternalType(atomicInt));
        return static_cast<ani_int>(etsAtomicInt->FetchAndSub(value));
    }
};

TEST_F(EtsNativeAtomicIntTest, testAtomicInt)
{
    CallStaticIntMethod(env_, "main", ":i");
}

}  // namespace ark::ets::test
