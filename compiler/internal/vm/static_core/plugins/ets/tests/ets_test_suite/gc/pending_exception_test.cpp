/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
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

#include "ets_vm.h"
#include "libarkbase/test_utilities.h"

namespace ark::test {
class PendingEtsExceptionTest : public testing::TestWithParam<int> {
public:
    void SetUp() override
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetCompilerEnableJit(false);
        options.SetLoadRuntimes({"ets"});
        options.SetGcType("g1-gc");
        options.SetRunGcInPlace(false);
        options.SetGcTriggerType("debug-never");

        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib});

        bool success = Runtime::Create(options);
        ASSERT_TRUE(success) << "Cannot create Runtime";
    }

    void TearDown() override
    {
        bool success = Runtime::Destroy();
        ASSERT_TRUE(success) << "Cannot destroy Runtime";
    }
};

DEATH_TEST_P(PendingEtsExceptionTest, MovingGcException)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    const std::string mainFunc = "pending_exception_gc_test.ETSGLOBAL::main";
    const std::string fileName = "pending_exception_gc_test.abc";
    EXPECT_EXIT(Runtime::GetCurrent()->ExecutePandaFile(fileName.c_str(), mainFunc.c_str(), {}),
                testing::ExitedWithCode(1), "");
}

INSTANTIATE_TEST_SUITE_P(MovingGcExceptionTests, PendingEtsExceptionTest, ::testing::Range(0, 8));
}  // namespace ark::test
