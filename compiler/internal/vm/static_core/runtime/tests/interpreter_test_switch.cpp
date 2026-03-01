/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "assembler/assembly-parser.h"
#include "libarkfile/bytecode_instruction.h"
#include "include/thread_scopes.h"
#include "libarkfile/bytecode_instruction-inl.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/interpreter/runtime_interface.h"

namespace ark::interpreter::test {

class InterpreterTestSwitch : public testing::Test {
public:
    InterpreterTestSwitch()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetRunGcInPlace(true);
        options.SetVerifyCallStack(false);
        options.SetInterpreterType("cpp");
        Runtime::Create(options);
    }

    ~InterpreterTestSwitch() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(InterpreterTestSwitch);
    NO_MOVE_SEMANTIC(InterpreterTestSwitch);
};

constexpr int32_t RET = 10;

static int32_t EntryPoint(Method * /* unused */)
{
    auto *thread = ManagedThread::GetCurrent();
    thread->SetCurrentDispatchTable<false>(thread->GetDebugDispatchTable());
    return RET;
}

class SwitchToDebugListener : public RuntimeListener {
public:
    struct Event {
        ManagedThread *thread;
        Method *method;
        uint32_t bcOffset;
    };

    void BytecodePcChanged(ManagedThread *thread, Method *method, uint32_t bcOffset) override
    {
        events_.push_back({thread, method, bcOffset});
    }

    auto &GetEvents() const
    {
        return events_;
    }

private:
    std::vector<Event> events_ {};
};

static auto g_switchToDebugSource = R"(
    .function i32 f() {
        ldai 10
        return
    }

    .function i32 g() {
        call f
        return
    }

    .function i32 main() {
        call g
        return
    }
)";

TEST_F(InterpreterTestSwitch, SwitchToDebug)
{
    pandasm::Parser p;

    auto source = g_switchToDebugSource;

    auto res = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());
    ASSERT_NE(pf, nullptr);

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));
    auto *extension = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY);

    SwitchToDebugListener listener {};

    auto *notificationManager = Runtime::GetCurrent()->GetNotificationManager();
    notificationManager->AddListener(&listener, RuntimeNotificationManager::BYTECODE_PC_CHANGED);

    std::vector<Value> args;
    Value v;
    Method *mainMethod;

    auto *thread = ManagedThread::GetCurrent();

    {
        ScopedManagedCodeThread smc(thread);
        PandaString descriptor;

        Class *klass = extension->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
        ASSERT_NE(klass, nullptr);

        mainMethod = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
        ASSERT_NE(mainMethod, nullptr);

        Method *fMethod = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
        ASSERT_NE(fMethod, nullptr);

        fMethod->SetCompiledEntryPoint(reinterpret_cast<const void *>(EntryPoint));

        v = mainMethod->Invoke(thread, args.data());
    }

    notificationManager->RemoveListener(&listener, RuntimeNotificationManager::BYTECODE_PC_CHANGED);

    ASSERT_EQ(v.GetAs<int32_t>(), RET);
    ASSERT_EQ(listener.GetEvents().size(), 1U);

    auto &event = listener.GetEvents()[0];
    EXPECT_EQ(event.thread, thread);
    EXPECT_EQ(event.method, mainMethod);
    EXPECT_EQ(event.bcOffset, BytecodeInstruction::Size(BytecodeInstruction::Format::V4_V4_ID16));
}

}  // namespace ark::interpreter::test
