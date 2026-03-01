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
#include <gtest/gtest.h>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

#include "runtime_helpers.h"
#include "tooling/sampler/samples_record.h"
#include "ets_vm.h"
#include "include/runtime.h"
#include "include/runtime_options.h"
#include "libarkbase/test_utilities.h"
#include "libarkbase/os/filesystem.h"
#include "inspector/debugger_arkapi.h"

namespace ark::test {
class ArkDebugNativeAPITest : public testing::Test {
public:
    void SetUp() override
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(true);
        options.SetLoadRuntimes({"ets"});
        options.SetSamplingProfilerCreate(true);
        auto stdlib = std::getenv("PANDA_STD_LIB");
        if (stdlib == nullptr) {
            std::cerr << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc" << std::endl;
            std::abort();
        }
        options.SetBootPandaFiles({stdlib, "profile_arkapi_test.abc"});
        bool success = Runtime::Create(options);
        ASSERT_TRUE(success) << "Cannot create Runtime";
        std::ofstream file(OUTPUT);
        if (!file.is_open()) {
            std::cerr << "Cannot create file" << std::endl;
            std::abort();
        }
        file.close();
    }

    void TearDown() override
    {
        ArkDebugNativeAPI::StopProfiling();
        bool success = Runtime::Destroy();
        ASSERT_TRUE(success) << "Cannot destroy Runtime";
        std::filesystem::remove(OUTPUT);
    }
    static constexpr const char *MAIN_FUNC = "profile_arkapi_test.ETSGLOBAL::main";
    static constexpr const char *FILENAME = "profile_arkapi_test.abc";
    static constexpr const char *OUTPUT = "test.cpuprofiler";
    static const uint32_t INTERVAL = 100;
};

TEST_F(ArkDebugNativeAPITest, StartProfilingWithOutStop)
{
    bool result = ArkDebugNativeAPI::StartProfiling(OUTPUT, INTERVAL);
    EXPECT_TRUE(result);
}

TEST_F(ArkDebugNativeAPITest, StartProfilingRepeated)
{
    bool result = ArkDebugNativeAPI::StartProfiling(OUTPUT);
    EXPECT_TRUE(result);
    result = ArkDebugNativeAPI::StartProfiling(OUTPUT, INTERVAL);
    EXPECT_FALSE(result);
}

TEST_F(ArkDebugNativeAPITest, StopProfilingWithOutStartAndExePanda)
{
    bool result = ArkDebugNativeAPI::StopProfiling();
    EXPECT_FALSE(result);
}

TEST_F(ArkDebugNativeAPITest, StopProfilingWithOutStart)
{
    Runtime::GetCurrent()->ExecutePandaFile(FILENAME, MAIN_FUNC, {});
    bool result = ArkDebugNativeAPI::StopProfiling();
    EXPECT_FALSE(result);
}

TEST_F(ArkDebugNativeAPITest, StartAndStopProfilingWithOutExePanda)
{
    bool result = ArkDebugNativeAPI::StartProfiling(OUTPUT, INTERVAL);
    EXPECT_TRUE(result);
    result = ArkDebugNativeAPI::StopProfiling();
    EXPECT_TRUE(result);
    std::ifstream file(OUTPUT);
    EXPECT_TRUE(file.is_open());
    EXPECT_TRUE(file.peek() == std::ifstream::traits_type::eof());
}

TEST_F(ArkDebugNativeAPITest, ArkDebugNativeAPINormalTest)
{
    bool result = ArkDebugNativeAPI::StartProfiling(OUTPUT, INTERVAL);
    EXPECT_TRUE(result);
    Runtime::GetCurrent()->ExecutePandaFile(FILENAME, MAIN_FUNC, {});
    result = ArkDebugNativeAPI::StopProfiling();
    EXPECT_TRUE(result);
}

TEST_F(ArkDebugNativeAPITest, CheckDebugModeTest)
{
    ark::Runtime::GetCurrent()->SetDebugMode(false);
    EXPECT_FALSE(ArkDebugNativeAPI::IsDebugModeEnabled());
    ark::Runtime::GetCurrent()->SetDebugMode(true);
    EXPECT_TRUE(ArkDebugNativeAPI::IsDebugModeEnabled());
}

}  // namespace ark::test
