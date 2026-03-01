/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include "ani_gtest.h"

#include "runtime/include/runtime.h"

namespace ark::ets::test {

class UnhandledRejectionRecursiveTest : public ani::testing::AniTest {
protected:
    void SetUp() override
    {
        ani::testing::AniTest::SetUp();
        [[maybe_unused]] bool listPromises = Runtime::GetOptions().IsListUnhandledOnExitPromises(
            plugins::LangToRuntimeType(panda_file::SourceLang::ETS));
        ASSERT(listPromises);
        BindNativeFunctions();
        successFlag_ = false;
    }

    void RunTest()
    {
        CallStaticVoidMethod(env_, "testRecursive", ":");
        ASSERT_TRUE(vm_->DestroyVM() == ANI_OK) << "Cannot destroy ANI VM";
        ASSERT(successFlag_);
    }

    void TearDown() override {}

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

private:
    static ani_function ResolveFunction(ani_env *env, std::string_view methodName, std::string_view signature)
    {
        ani_module md;
        [[maybe_unused]] auto status = env->FindModule("UnhandledRejectionRecursiveTest", &md);
        ASSERT(status == ANI_OK);
        ani_function func;
        status = env->Module_FindFunction(md, methodName.data(), signature.data(), &func);
        ASSERT(status == ANI_OK);
        return func;
    }

    void BindNativeFunctions()
    {
        ani_module module {};
        [[maybe_unused]] auto status = env_->FindModule("UnhandledRejectionRecursiveTest", &module);
        ASSERT(status == ANI_OK);
        std::array methods = {
            ani_native_function {"setFlag", ":", reinterpret_cast<void *>(SetFlag)},
        };
        status = env_->Module_BindNativeFunctions(module, methods.data(), methods.size());
        ASSERT(status == ANI_OK);
    }

    static void SetFlag()
    {
        successFlag_ = true;
    }

    static bool successFlag_;
};

// ISO C++ forbids in-class initialization of non-const static member
bool UnhandledRejectionRecursiveTest::successFlag_ = false;

/// Rejects promise inside a rejected promise handler
TEST_F(UnhandledRejectionRecursiveTest, Recursive)
{
    RunTest();
}

}  // namespace ark::ets::test
