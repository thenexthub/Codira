/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

class CoroInCctorTest : public testing::Test {
protected:
    void SetUp() override
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetShouldInitializeIntrinsics(true);
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        options.SetLoadRuntimes({"ets"});
        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to mock_stdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib, "coroswitch_in_cctor.abc"});
        options.SetCoroutineImpl("stackful");

        Runtime::Create(options);
    }

    void TearDown() override
    {
        Runtime::Destroy();
    }

    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    const std::string abcFile_ = "coroswitch_in_cctor.abc";
};

using CoroInCctorDeathTest = CoroInCctorTest;
/**
 * Death test. Tries to call an async function from a static ctor, hence creates a coroutine and switches to it, which
 * is prohibited. Should cause the program to be abort()-ed
 */
TEST_F(CoroInCctorDeathTest, FatalOnCoroSwitchInCctor)
{
    const std::string mainFunc = "coroswitch_in_cctor.ETSGLOBAL::main";
    EXPECT_EXIT(Runtime::GetCurrent()->ExecutePandaFile(abcFile_.c_str(), mainFunc.c_str(), {}),
                testing::ExitedWithCode(1), ".*");
}

}  // namespace ark::ets::test
