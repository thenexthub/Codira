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

#include "agent/debugger_impl.h"
#include "backend/js_pt_hooks.h"
#include "tooling/dynamic/base/pt_types.h"
#include "tooling/dynamic/base/pt_events.h"
#include "dispatcher.h"

#include "ecmascript/debugger/js_debugger.h"
#include "ecmascript/js_array.h"
#include "ecmascript/js_tagged_value-inl.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tests/test_helper.h"
#include "protocol_handler.h"

using namespace panda::ecmascript;
using namespace panda::ecmascript::tooling;

namespace panda::test {
class JSPtHooksTest : public testing::Test {
public:
    using EntityId = panda_file::File::EntityId;
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

HWTEST_F_L0(JSPtHooksTest, BreakpointTest)
{
    auto debugger = std::make_unique<DebuggerImpl>(ecmaVm, nullptr, nullptr);
    std::unique_ptr<JSPtHooks> jspthooks = std::make_unique<JSPtHooks>(debugger.get());
    EntityId methodId(0);
    uint32_t bytecodeOffset = 0;
    JSPtLocation ptLocation1(nullptr, methodId, bytecodeOffset);
    jspthooks->Breakpoint(ptLocation1);
    ASSERT_NE(jspthooks, nullptr);
}

HWTEST_F_L0(JSPtHooksTest, LoadModuleTest)
{
    auto debugger = std::make_unique<DebuggerImpl>(ecmaVm, nullptr, nullptr);
    std::unique_ptr<JSPtHooks> jspthooks = std::make_unique<JSPtHooks>(debugger.get());
    jspthooks->LoadModule("pandafile/test.abc", "func_main_0");
    ASSERT_NE(jspthooks, nullptr);
}

HWTEST_F_L0(JSPtHooksTest, ExceptionTest)
{
    auto debugger = std::make_unique<DebuggerImpl>(ecmaVm, nullptr, nullptr);
    std::unique_ptr<JSPtHooks> jspthooks = std::make_unique<JSPtHooks>(debugger.get());
    EntityId methodId(0);
    uint32_t bytecodeOffset = 0;
    JSPtLocation ptLocation2(nullptr, methodId, bytecodeOffset);
    jspthooks->Exception(ptLocation2);
    ASSERT_NE(jspthooks, nullptr);
}

HWTEST_F_L0(JSPtHooksTest, SingleStepTest)
{
    std::string outStrForCallbackCheck = "";
    std::function<void(const void*, const std::string &)> callback =
        [&outStrForCallbackCheck]([[maybe_unused]] const void *ptr, const std::string &inStrOfReply) {
            outStrForCallbackCheck = inStrOfReply;};
    ProtocolChannel *protocolChannel = new ProtocolHandler(callback, ecmaVm);
    auto runtimeImpl = std::make_unique<RuntimeImpl>(ecmaVm, protocolChannel);
    auto debuggerImpl = std::make_unique<DebuggerImpl>(ecmaVm, protocolChannel, runtimeImpl.get());

    std::unique_ptr<JSPtHooks> jspthooks = std::make_unique<JSPtHooks>(debuggerImpl.get());
    EntityId methodId(0);
    uint32_t bytecodeOffset = 0;
    JSPtLocation ptLocation4(nullptr, methodId, bytecodeOffset);
    jspthooks->SingleStep(ptLocation4);
    ASSERT_NE(jspthooks, nullptr);

    if (protocolChannel) {
        delete protocolChannel;
        protocolChannel = nullptr;
    }
}

HWTEST_F_L0(JSPtHooksTest, VmStartTest)
{
    auto debugger = std::make_unique<DebuggerImpl>(ecmaVm, nullptr, nullptr);
    std::unique_ptr<JSPtHooks> jspthooks = std::make_unique<JSPtHooks>(debugger.get());
    jspthooks->VmStart();
    ASSERT_NE(jspthooks, nullptr);
}

HWTEST_F_L0(JSPtHooksTest, VmDeathTest)
{
    auto debugger = std::make_unique<DebuggerImpl>(ecmaVm, nullptr, nullptr);
    std::unique_ptr<JSPtHooks> jspthooks = std::make_unique<JSPtHooks>(debugger.get());
    jspthooks->VmDeath();
    ASSERT_NE(jspthooks, nullptr);
}

HWTEST_F_L0(JSPtHooksTest, NativeCallingTest)
{
    auto debugger = std::make_unique<DebuggerImpl>(ecmaVm, nullptr, nullptr);
    std::unique_ptr<JSPtHooks> jspthooks = std::make_unique<JSPtHooks>(debugger.get());
    jspthooks->NativeCalling(nullptr);
    ASSERT_NE(jspthooks, nullptr);
}

HWTEST_F_L0(JSPtHooksTest, NativeReturnTest)
{
    auto debugger = std::make_unique<DebuggerImpl>(ecmaVm, nullptr, nullptr);
    std::unique_ptr<JSPtHooks> jspthooks = std::make_unique<JSPtHooks>(debugger.get());
    jspthooks->NativeReturn(nullptr);
    ASSERT_NE(jspthooks, nullptr);
}

HWTEST_F_L0(JSPtHooksTest, SetDebuggerAccessorTest)
{
    [[maybe_unused]] auto debugger = std::make_unique<DebuggerImpl>(ecmaVm, nullptr, nullptr);
    Local<JSValueRef> newContext = JSNApi::CreateContext(ecmaVm);
    Local<ObjectRef> globalObj = JSNApi::GetGlobalObject(ecmaVm, newContext);
    Local<JSValueRef> setStr = StringRef::NewFromUtf8(ecmaVm, "debuggerSetValue");
    Local<JSValueRef> getStr = StringRef::NewFromUtf8(ecmaVm, "debuggerGetValue");
    EXPECT_TRUE(globalObj->Has(ecmaVm, setStr));
    EXPECT_TRUE(globalObj->Has(ecmaVm, getStr));
}
}
