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

#include "ecmascript/tests/test_helper.h"
#include "ecmascript/ecma_vm.h"
#include "debugger_service.h"
#include "protocol_handler.h"
#include "ecmascript/debugger/js_debugger_manager.h"
using namespace panda::ecmascript;
using namespace panda::ecmascript::tooling;

namespace panda::test {
class DebuggerServiceTest : public testing::Test {
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
        TestHelper::CreateEcmaVMWithScope(ecmaVm, thread, scope);
    }

    void TearDown() override
    {
        TestHelper::DestroyEcmaVMWithScope(ecmaVm, scope);
    }

protected:
    EcmaVM *ecmaVm {nullptr};
    EcmaHandleScope *scope {nullptr};
    JSThread *thread {nullptr};
};

HWTEST_F_L0(DebuggerServiceTest, InitializeDebuggerTest)
{
    ProtocolHandler *handler = ecmaVm->GetJsDebuggerManager()->GetDebuggerHandler();
    ASSERT_TRUE(handler == nullptr);
    InitializeDebugger(ecmaVm, nullptr);
    handler = ecmaVm->GetJsDebuggerManager()->GetDebuggerHandler();
    ASSERT_TRUE(handler != nullptr);
    InitializeDebugger(ecmaVm, nullptr);
    ASSERT_TRUE(handler != nullptr);
    EcmaVM *vm = nullptr;
    InitializeDebugger(vm, nullptr);
    ASSERT_TRUE(handler != nullptr);
}

HWTEST_F_L0(DebuggerServiceTest, UninitializeDebuggerTest)
{
    UninitializeDebugger(ecmaVm);
    InitializeDebugger(ecmaVm, nullptr);
    ProtocolHandler *handler = ecmaVm->GetJsDebuggerManager()->GetDebuggerHandler();
    ASSERT_TRUE(handler != nullptr);
    UninitializeDebugger(ecmaVm);
    handler = ecmaVm->GetJsDebuggerManager()->GetDebuggerHandler();
    ASSERT_TRUE(handler == nullptr);
    EcmaVM *vm = nullptr;
    UninitializeDebugger(vm);
    ASSERT_TRUE(handler == nullptr);
}

HWTEST_F_L0(DebuggerServiceTest, OnMessageTest)
{
    ProtocolHandler *handler = ecmaVm->GetJsDebuggerManager()->GetDebuggerHandler();
    OnMessage(ecmaVm, "");
    ASSERT_TRUE(handler == nullptr);
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    InitializeDebugger(ecmaVm, callback);
    std::string msg = std::string() + R"({"id":0,"method":"Tracing.Test","params":{}})";
    OnMessage(ecmaVm, msg + "");
    ProcessMessage(ecmaVm);
    ASSERT_TRUE(result.find("Unknown method: Test") != std::string::npos);
    EcmaVM *vm = nullptr;
    OnMessage(vm, "");
    ASSERT_TRUE(result.find("Unknown method: Test") != std::string::npos);
}

HWTEST_F_L0(DebuggerServiceTest, SetDebugAppDebuggerTest)
{
    InitializeDebugger(ecmaVm, nullptr);
    ProtocolHandler *handler = ecmaVm->GetJsDebuggerManager()->GetDebuggerHandler();
    ASSERT_TRUE(handler != nullptr);
    ecmaVm->GetJsDebuggerManager()->SetIsDebugApp(false);
    ASSERT_FALSE(ecmaVm->GetJsDebuggerManager()->IsDebugApp());
    SetDebugApp(ecmaVm);
    ASSERT_TRUE(ecmaVm->GetJsDebuggerManager()->IsDebugApp());
}

HWTEST_F_L0(DebuggerServiceTest, WaitForDebuggerTest)
{
    ProtocolHandler *handler = ecmaVm->GetJsDebuggerManager()->GetDebuggerHandler();
    WaitForDebugger(ecmaVm);
    ASSERT_TRUE(handler == nullptr);
}

HWTEST_F_L0(DebuggerServiceTest, ProcessMessageTest)
{
    ProtocolHandler *handler = ecmaVm->GetJsDebuggerManager()->GetDebuggerHandler();
    ProcessMessage(ecmaVm);
    ASSERT_TRUE(handler == nullptr);
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    InitializeDebugger(ecmaVm, callback);
    std::string msg = std::string() + R"({"id":0,"method":"Tracing.end","params":{}})";
    OnMessage(ecmaVm, msg + "");
    ProcessMessage(ecmaVm);
    ASSERT_TRUE(result.find("\"id\":0,") != std::string::npos);
    EcmaVM *vm = nullptr;
    ProcessMessage(vm);
    ASSERT_TRUE(result.find("Stop is failure") != std::string::npos);
}

HWTEST_F_L0(DebuggerServiceTest, GetDispatchStatusTest)
{
    ProtocolHandler::DispatchStatus status = ProtocolHandler::DispatchStatus(GetDispatchStatus(ecmaVm));
    ASSERT_TRUE(status == ProtocolHandler::DispatchStatus::UNKNOWN);
    std::string result = "";
    std::function<void(const void*, const std::string &)> callback =
        [&result]([[maybe_unused]] const void *ptr, const std::string &temp) { result = temp; };
    InitializeDebugger(ecmaVm, callback);
    status = ProtocolHandler::DispatchStatus(GetDispatchStatus(ecmaVm));
    ASSERT_TRUE(status == ProtocolHandler::DispatchStatus::DISPATCHED);
    EcmaVM *vm = nullptr;
    int32_t GetDispatchStatusTest = GetDispatchStatus(vm);
    ASSERT_TRUE(GetDispatchStatusTest == 0);
}

HWTEST_F_L0(DebuggerServiceTest, SaveAllBreakpointsTest)
{
    const char *message = "{\"id\":0,\"method\":\"Debugger.saveAllPossibleBreakpoints\","
        "\"params\":{\"locations\":{\"entry|entry|1.0.0|src/main/ets/pages/Index.ts\":"
        "[{\"lineNumber\":59,\"columnNumber\":16}]}}}";
    DebugResponse debugMessage = OperateDebugMessage(nullptr, message);
    ASSERT_TRUE(debugMessage.response == nullptr && debugMessage.size == 0);

    debugMessage = OperateDebugMessage(ecmaVm, message);
    ASSERT_TRUE(debugMessage.response == nullptr && debugMessage.size == 0);

    InitializeDebugger(ecmaVm, nullptr);
    debugMessage = OperateDebugMessage(ecmaVm, message);
    ASSERT_TRUE(debugMessage.response != nullptr && debugMessage.size != 0);
    UninitializeDebugger(ecmaVm);
}

HWTEST_F_L0(DebuggerServiceTest, RemoveBreakpointTest)
{
    const char *message = "{\"id\":0,\"method\":\"Debugger.removeBreakpointsByUrl\","
        "\"params\":{\"url\":\"entry|entry|1.0.0|src/main/ets/pages/Index.ts\"}}";
    DebugResponse debugMessage = OperateDebugMessage(nullptr, message);
    ASSERT_TRUE(debugMessage.response == nullptr && debugMessage.size == 0);

    debugMessage = OperateDebugMessage(ecmaVm, message);
    ASSERT_TRUE(debugMessage.response == nullptr && debugMessage.size == 0);

    InitializeDebugger(ecmaVm, nullptr);
    debugMessage = OperateDebugMessage(ecmaVm, message);
    ASSERT_TRUE(debugMessage.response != nullptr && debugMessage.size != 0);
    UninitializeDebugger(ecmaVm);
}

HWTEST_F_L0(DebuggerServiceTest, SetBreakpointTest)
{
    const char *message = "{\"id\":0,\"method\":\"Debugger.getPossibleAndSetBreakpointByUrl\","
        "\"params\":{\"locations\":[{\"url\":\"entry|entry|1.0.0|src/main/ets/pages/Index.ts\","
        "\"lineNumber\":59,\"columnNumber\":16}]}}";
    DebugResponse debugMessage = OperateDebugMessage(nullptr, message);
    ASSERT_TRUE(debugMessage.response == nullptr && debugMessage.size == 0);

    debugMessage = OperateDebugMessage(ecmaVm, message);
    ASSERT_TRUE(debugMessage.response == nullptr && debugMessage.size == 0);

    InitializeDebugger(ecmaVm, nullptr);
    debugMessage = OperateDebugMessage(ecmaVm, message);
    ASSERT_TRUE(debugMessage.response != nullptr && debugMessage.size != 0);
    UninitializeDebugger(ecmaVm);
}

HWTEST_F_L0(DebuggerServiceTest, GetCallFramesTest)
{
    DebugResponse debugMessage = GetCallFrames(nullptr);
    ASSERT_TRUE(debugMessage.response == nullptr && debugMessage.size == 0);

    debugMessage = GetCallFrames(ecmaVm);
    ASSERT_TRUE(debugMessage.response == nullptr && debugMessage.size == 0);

    InitializeDebugger(ecmaVm, nullptr);
    debugMessage = GetCallFrames(ecmaVm);
    ASSERT_FALSE(debugMessage.response != nullptr && debugMessage.size != 0);
    UninitializeDebugger(ecmaVm);
}

HWTEST_F_L0(DebuggerServiceTest, OperateDebugMessageTest_0)
{
    DebugResponse debugMessage = OperateDebugMessage(nullptr, nullptr);
    ASSERT_TRUE(debugMessage.response == nullptr && debugMessage.size == 0);

    debugMessage = OperateDebugMessage(ecmaVm, nullptr);
    ASSERT_TRUE(debugMessage.response == nullptr && debugMessage.size == 0);

    std::string msg = std::string() +
        R"({
            "id":0,
            "method":"Debugger.clientDisconnect",
            "params":{}
        })";
    const char *message1 = msg.c_str();
    InitializeDebugger(ecmaVm, nullptr);
    debugMessage = OperateDebugMessage(ecmaVm, message1);
    ASSERT_TRUE(debugMessage.response == nullptr && debugMessage.size == 0);

    const char *message2 = "{\"id\":0,\"method\":\"Debugger.getPossibleAndSetBreakpointByUrl\","
        "\"params\":{\"locations\":[{\"url\":\"entry|entry|1.0.0|src/main/ets/pages/Index.ts\","
        "\"lineNumber\":59,\"columnNumber\":16}]}}";
    debugMessage = OperateDebugMessage(ecmaVm, message2);
    ASSERT_TRUE(debugMessage.response != nullptr && debugMessage.size != 0);
    UninitializeDebugger(ecmaVm);
}

HWTEST_F_L0(DebuggerServiceTest, OperateDebugMessageTest_1)
{
    const char *message = "{\"id\":0,\"method\":\"Runtime.getProperties\",\"params\":{\"objectId\":\"1\"}}";
    DebugResponse debugMessage = OperateDebugMessage(nullptr, message);
    ASSERT_TRUE(debugMessage.response == nullptr && debugMessage.size == 0);

    debugMessage = OperateDebugMessage(ecmaVm, message);
    ASSERT_TRUE(debugMessage.response == nullptr && debugMessage.size == 0);

    InitializeDebugger(ecmaVm, nullptr);
    debugMessage = OperateDebugMessage(ecmaVm, message);
    ASSERT_TRUE(debugMessage.response != nullptr && debugMessage.size != 0);
    UninitializeDebugger(ecmaVm);
}
}  // namespace panda::test