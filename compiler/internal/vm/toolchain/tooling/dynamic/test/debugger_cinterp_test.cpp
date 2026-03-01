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

#include "ecmascript/ecma_vm.h"
#include "ecmascript/napi/include/jsnapi.h"
#include "ecmascript/tests/test_helper.h"
#include "tooling/dynamic/test/utils/test_list.h"
#include "ecmascript/debugger/js_debugger.h"

namespace panda::ecmascript::tooling::test {
using panda::test::TestHelper;

class DebuggerCInterpTest : public testing::TestWithParam<const char *> {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "TearDownCase";
    }

    void SetUp() override
    {
        SetCurrentTestName(GetParam());
        TestHelper::CreateEcmaVMWithScope(instance, thread, scope, false, true);
        JSNApi::DebugOption debugOption = {DEBUGGER_TEST_LIBRARY, true, -1};
        JSNApi::StartDebugger(instance, debugOption);
        if (instance->GetJsDebuggerManager() != nullptr) {
            instance->GetJsDebuggerManager()->DisableObjectHashDisplay();
        }
    }

    void TearDown() override
    {
        JSNApi::StopDebugger(instance);
        TestHelper::DestroyEcmaVMWithScope(instance, scope);
    }

    EcmaVM *instance {nullptr};
    EcmaHandleScope *scope {nullptr};
    JSThread *thread {nullptr};
};

HWTEST_P_L0(DebuggerCInterpTest, DebuggerSuite)
{
    std::string testName = GetCurrentTestName();
    std::cout << "Running " << testName << std::endl;
    ASSERT_NE(instance, nullptr);
    auto [pandaFile, entryPoint] = GetTestEntryPoint(testName);
    auto res = JSNApi::Execute(instance, pandaFile.c_str(), entryPoint.c_str());
    ASSERT_TRUE(res);
}

INSTANTIATE_TEST_SUITE_P(DebugAbcTest, DebuggerCInterpTest, testing::ValuesIn(GetTestList()));
}  // namespace panda::ecmascript::tooling::test
