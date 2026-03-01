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

#include "configs/guard_context.h"

using namespace testing::ext;
using namespace panda;

namespace {
const std::string CONTEXT_TEST_01_CONFIG_FILE = PANDA_GUARD_UNIT_TEST_DIR "configs/context_test_01_config.json";
}

/**
 * @tc.name: guard_context_test_001
 * @tc.desc: test context init is right
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(GuardContextUnitTest, guard_context_test_001, TestSize.Level4)
{
    int argc = 3;
    char *argv[3];
    argv[0] = const_cast<char *>("xxx");
    argv[1] = const_cast<char *>("--debug");
    argv[2] = const_cast<char *>(CONTEXT_TEST_01_CONFIG_FILE.c_str());

    auto context = guard::GuardContext::GetInstance();
    context->Init(argc, const_cast<const char **>(argv));
    EXPECT_EQ(context->IsDebugMode(), true);

    auto nameMapping = context->GetNameMapping();
    std::string obfName = nameMapping->GetFileName("context_test");
    EXPECT_EQ(obfName, "h");

    obfName = nameMapping->GetFileName("test1");
    EXPECT_EQ(obfName, "a");

    obfName = nameMapping->GetFileName("xxx");
    EXPECT_EQ(obfName, "xxx");
}

/**
 * @tc.name: guard_context_test_002
 * @tc.desc: test context init with invalid config
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(GuardContextUnitTest, guard_context_test_002, TestSize.Level4)
{
    int argc = 2;
    char *argv[2];
    argv[0] = const_cast<char *>("xxx");
    argv[1] = const_cast<char *>("xxx");

    auto context = guard::GuardContext::GetInstance();
    EXPECT_DEATH(context->Init(argc, const_cast<const char **>(argv)), "");
}