/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

#include "runtime_helpers.h"
#include "include/runtime.h"
#include "include/runtime_options.h"
#include "libarkbase/utils/logger.h"
#include "libarkbase/test_utilities.h"

namespace ark::test {
class SymbolExtractorTest : public testing::Test {
public:
    void SetUp() override {}

    void TearDown() override
    {
        bool success = Runtime::Destroy();
        ASSERT_TRUE(success) << "Cannot destroy Runtime";
    }
};

TEST_F(SymbolExtractorTest, SymbolExtractorTestCase)
{
    RuntimeOptions options;
    options.SetShouldLoadBootPandaFiles(true);
    options.SetLoadRuntimes({"ets"});
    auto stdlib = std::getenv("PANDA_STD_LIB");
    if (stdlib == nullptr) {
        LOG(FATAL, TOOLING) << "PANDA_STD_LIB env variable should be set and point to etsstdlib.abc";
    }
    auto pandafile = std::getenv("ARK_GTEST_ABC_PATH");
    if (pandafile == nullptr) {
        LOG(FATAL, TOOLING)
            << "ARK_ETS_GTEST_ABC_PATH env variable should be set and point to call_symbol_extractor.abc";
    }
    options.SetBootPandaFiles({stdlib});
    options.SetPandaFiles({pandafile});
    bool success = Runtime::Create(options);
    ASSERT_TRUE(success) << "Cannot create Runtime";
    auto result = Runtime::GetCurrent()->ExecutePandaFile(pandafile, "call_symbol_extractor.ETSGLOBAL::main", {});
    EXPECT_TRUE(result);
}

}  // namespace ark::test
