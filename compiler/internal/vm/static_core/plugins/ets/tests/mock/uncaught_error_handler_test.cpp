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

#include <gtest/gtest.h>

#include "runtime/include/runtime.h"

namespace ark::ets::test {

class UncaughtErrorHandlerTest : public testing::Test {
protected:
    void SetUp() override
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});
        options.SetListUnhandledOnExitPromises(true);
        options.SetCoroutineWorkersCount(3U);
        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib, "UncaughtErrorHandlerTest.abc"});
        options.SetCoroutineImpl("stackful");

        Runtime::Create(options);
        if (Runtime::GetCurrent() == nullptr) {
            std::cerr << "Can't create runtime" << std::endl;
            std::abort();
        }
    }

    void RunTest(const std::string &type)
    {
        const std::string mainFunc = "UncaughtErrorHandlerTest.ETSGLOBAL::main";
        Runtime::GetCurrent()->ExecutePandaFile(abcFile_.c_str(), mainFunc.c_str(), {type});
        Runtime::Destroy();
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    const std::string abcFile_ = "UncaughtErrorHandlerTest.abc";
};

using UncaughtErrorHandlerDeathTest = UncaughtErrorHandlerTest;

/// Death test. Throws an exception and doesn't catch it.
TEST_F(UncaughtErrorHandlerDeathTest, SimpleDeath)
{
    EXPECT_EXIT(RunTest("Simple"), testing::ExitedWithCode(2U), ".*");
}

/// Throws an exception from SetTimeout callback.
TEST_F(UncaughtErrorHandlerTest, SetTimeout)
{
    RunTest("SetTimeout");
}

/// Death test. Checks if `on('unhandledPromiseRejection)` and `on('uncaughtError')` don't collide.
TEST_F(UncaughtErrorHandlerDeathTest, NoDeadlockDeath)
{
    EXPECT_EXIT(RunTest("NoDeadlock"), testing::ExitedWithCode(2U), ".*");
}

TEST_F(UncaughtErrorHandlerDeathTest, SimpleConurrent)
{
    EXPECT_EXIT(RunTest("SimpleConcurrent"), testing::ExitedWithCode(2U), ".*");
}

/// Throws an exception an handles it with a catch block.
TEST_F(UncaughtErrorHandlerTest, NoAbort)
{
    RunTest("NoAbort");
}

}  // namespace ark::ets::test
