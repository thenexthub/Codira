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

#include <csignal>

#include "ecmascript/ecma_vm.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "ecmascript/tests/test_helper.h"
#include "tooling/dynamic/test/client_utils/test_list.h"
#include "tooling/dynamic/test/client_utils/test_util.h"

namespace panda::ecmascript::tooling::test {
using panda::test::TestHelper;
static int g_port = 9002;  // 9002: socket port
class DebuggerClientTest : public testing::TestWithParam<const char *> {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "SetUpTestCase";
        if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
            GTEST_LOG_(ERROR) << "Reset SIGPIPE failed.";
        }
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "TearDownCase";
    }

    void SetUp() override
    {
        g_port -= 1;
        SetCurrentTestName(GetParam());
        TestHelper::CreateEcmaVMWithScope(instance, thread, scope);
        TestUtil::ForkSocketClient(g_port, GetParam());
        JSNApi::DebugOption debugOption = {DEBUGGER_LIBRARY, true, g_port};
        JSNApi::StartDebugger(instance, debugOption);
        if (instance->GetJsDebuggerManager() != nullptr) {
            instance->GetJsDebuggerManager()->DisableObjectHashDisplay();
        }
    }

    void TearDown() override
    {
        std::this_thread::sleep_for(std::chrono::microseconds(500000)); // 500000: 500ms
        JSNApi::StopDebugger(instance);
        TestHelper::DestroyEcmaVMWithScope(instance, scope);
    }

    EcmaVM *instance {nullptr};
    EcmaHandleScope *scope {nullptr};
    JSThread *thread {nullptr};
};

//NOTE: enable (issue 18042)
HWTEST_P_L0(DebuggerClientTest, DebuggerSuite)
{
    std::string testName = GetCurrentTestName();
    std::cout << "Running " << testName << std::endl;
    ASSERT_NE(instance, nullptr);
    auto [pandaFile, entryPoint] = GetTestEntryPoint(testName);
    auto res = JSNApi::Execute(instance, pandaFile.c_str(), entryPoint.c_str());
    ASSERT_TRUE(res);
}

INSTANTIATE_TEST_SUITE_P(DebugClientAbcTest, DebuggerClientTest, testing::ValuesIn(GetTestList()));
}  // namespace panda::ecmascript::tooling::test
