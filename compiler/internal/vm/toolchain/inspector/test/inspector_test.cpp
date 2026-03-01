/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gtest/gtest.h"
#include "inspector/inspector.h"

using namespace OHOS::ArkCompiler::Toolchain;

namespace panda::test {

class InspectorTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "InspectorTest::SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "InspectorTest::TearDownTestCase";
    }

    void SetUp() override {}

    void TearDown() override {}

#if defined(OHOS_PLATFORM)
static constexpr char COMPONENT_NAME[] = "connected";
static constexpr int  UNSTANCE_ID = 12345;
static constexpr int  UNSTANCE_ID_0 = 0;
static constexpr bool IS_DEBUG_MODE_TRUE = true;
static constexpr bool IS_DEBUG_MODE_FALSE = false;
static constexpr int PORT_8080 = 8080;
static constexpr int PORT_65535 = 65535;
static constexpr int PORT_0 = 0;
static constexpr int TID = 12345;
static constexpr int SOCKETFD = 10;
static constexpr int TID_0 = 0;
static constexpr int SOCKETFD_0 = 0;
static constexpr int TID_MINUS_ONE = -1;
static constexpr int SOCKETFD_MINUS_ONE = -1;
#endif
};

HWTEST_F(InspectorTest, StartDebugTest001, testing::ext::TestSize.Level0)
{
#if defined(OHOS_PLATFORM)
    void* vm = nullptr;
    DebuggerPostTask debuggerPostTask = [](std::function<void()>&& task) {
        task();
    };

    auto res = StartDebug(COMPONENT_NAME, vm, IS_DEBUG_MODE_TRUE, UNSTANCE_ID, debuggerPostTask, PORT_8080);
    EXPECT_TRUE(res);

    res = StartDebug(COMPONENT_NAME, vm, IS_DEBUG_MODE_FALSE, UNSTANCE_ID, debuggerPostTask, PORT_8080);
    EXPECT_TRUE(res);

    res = StartDebug(COMPONENT_NAME, vm, IS_DEBUG_MODE_TRUE, UNSTANCE_ID, debuggerPostTask, PORT_0);
    EXPECT_TRUE(res);

    res = StartDebug("", vm, IS_DEBUG_MODE_TRUE, UNSTANCE_ID, debuggerPostTask, PORT_8080);
    EXPECT_TRUE(res);

    res = StartDebug(COMPONENT_NAME, vm, IS_DEBUG_MODE_TRUE, UNSTANCE_ID_0, debuggerPostTask, PORT_8080);
    EXPECT_TRUE(res);

    res = StartDebug(COMPONENT_NAME, vm, IS_DEBUG_MODE_TRUE, UNSTANCE_ID, {}, PORT_8080);
    EXPECT_TRUE(res);

    res = StartDebug(COMPONENT_NAME, vm, IS_DEBUG_MODE_TRUE, UNSTANCE_ID, debuggerPostTask, PORT_65535);
    EXPECT_TRUE(res);
#endif
}

HWTEST_F(InspectorTest, StartDebugForSocketpairTest001, testing::ext::TestSize.Level0)
{
#if defined(OHOS_PLATFORM)
    auto res = StartDebugForSocketpair(TID, SOCKETFD, false);
    EXPECT_FALSE(res);

    res = StartDebugForSocketpair(TID, SOCKETFD, true);
    EXPECT_FALSE(res);

    res = StartDebugForSocketpair(TID_0, SOCKETFD, false);
    EXPECT_FALSE(res);

    res = StartDebugForSocketpair(TID, SOCKETFD_0, false);
    EXPECT_FALSE(res);

    res = StartDebugForSocketpair(TID_MINUS_ONE, SOCKETFD, false);
    EXPECT_FALSE(res);

    res = StartDebugForSocketpair(TID, SOCKETFD_MINUS_ONE, false);
    EXPECT_FALSE(res);
#endif
}

HWTEST_F(InspectorTest, InitializeDebuggerForSocketpairTest001, testing::ext::TestSize.Level0)
{
#if defined(OHOS_PLATFORM)
    void* vm = nullptr;
    
    auto res = InitializeDebuggerForSocketpair(vm, false);
    EXPECT_TRUE(res);
    
    res = InitializeDebuggerForSocketpair(vm, true);
    EXPECT_TRUE(res);
    
    res = InitializeDebuggerForSocketpair(nullptr, false);
    EXPECT_TRUE(res);
#endif
}

HWTEST_F(InspectorTest, StopDebuggerTest001, testing::ext::TestSize.Level0)
{
#if defined(OHOS_PLATFORM)
    auto res = StopDebugger();
    EXPECT_TRUE(res);
#endif
}

HWTEST_F(InspectorTest, GetJsBacktraceTest001, testing::ext::TestSize.Level0)
{
#if defined(OHOS_PLATFORM)
    const char* backtrace = GetJsBacktrace();
    EXPECT_TRUE(backtrace != nullptr);
    if (backtrace != nullptr && backtrace[0] != '\0') {
        free(const_cast<char*>(backtrace));
    }
#endif
}

HWTEST_F(InspectorTest, OperateJsDebugMessageTest001, testing::ext::TestSize.Level0)
{
#if defined(OHOS_PLATFORM)
    const char* message = "{\"type\":\"Debugger.enable\"}";
    const char* response = OperateJsDebugMessage(message);
    EXPECT_TRUE(response != nullptr);

    if (response != nullptr && response[0] != '\0') {
        free(const_cast<char*>(response));
    }

    response = OperateJsDebugMessage("");
    EXPECT_TRUE(response != nullptr);
    if (response != nullptr && response[0] != '\0') {
        free(const_cast<char*>(response));
    }

    response = OperateJsDebugMessage(nullptr);
    EXPECT_TRUE(response != nullptr);
    if (response != nullptr && response[0] != '\0') {
        free(const_cast<char*>(response));
    }

    response = OperateJsDebugMessage("invalid json");
    EXPECT_TRUE(response != nullptr);
    if (response != nullptr && response[0] != '\0') {
        free(const_cast<char*>(response));
    }
#endif
}
} // namespace panda::test