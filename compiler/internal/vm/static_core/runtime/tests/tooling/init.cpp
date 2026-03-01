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

#include <memory>
#include <thread>

#include "runtime/include/tooling/debug_interface.h"
#include "runtime/tests/tooling/test_runner.h"
#include "test_util.h"

namespace ark::tooling::test {
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::thread g_gDebuggerThread;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static std::unique_ptr<TestRunner> g_gRunner {nullptr};

extern "C" int StartDebugger(uint32_t /* port */, bool /* break_on_start */)
{
    const char *testName = GetCurrentTestName();
    g_gRunner = std::make_unique<TestRunner>(testName);
    g_gDebuggerThread = std::thread([] {
        TestUtil::WaitForInit();
        g_gRunner->Run();
    });
    return 0;
}

extern "C" int StopDebugger()
{
    g_gDebuggerThread.join();
    g_gRunner->TerminateTest();
    g_gRunner.reset();
    return 0;
}
}  // namespace ark::tooling::test
