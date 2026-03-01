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

#include "unit_test.h"
#include "panda_runner.h"
#include "runtime/jit/profiling_data.h"
#include "runtime/jit/profiling_saver.h"

namespace ark::test {
class ProfilingRunnerTest : public testing::Test {};

// Test constants
static constexpr const char *INTERPRETER_TYPE_CPP = "cpp";
static constexpr const char *METHOD_NAME_FOO = "foo";

static constexpr auto SOURCE = R"(
.function void main() <static> {
    movi v0, 0x2
    movi v1, 0x0
    jump_label_1: lda v1
    jge v0, jump_label_0
    call.short foo
    inci v1, 0x1
    jmp jump_label_1
    jump_label_0: return.void
}

.function i32 foo() <static> {
    movi v0, 0x64
    movi v1, 0x0
    mov v2, v1
    jump_label_3: lda v2
    jge v0, jump_label_0
    lda v2
    modi 0x3
    jnez jump_label_1
    lda v1
    addi 0x2
    sta v3
    mov v1, v3
    jmp jump_label_2
    jump_label_1: lda v1
    addi 0x3
    sta v3
    mov v1, v3
    jump_label_2: inci v2, 0x1
    jmp jump_label_3
    jump_label_0: lda v1
    return
}
)";

static constexpr int16_t SAVER_COUNTER_INIT = 4096;

void CheckSaverCounter(ark::Method *method)
{
    while (method->GetSaverTryCounter() > 0) {
        method->TryCreateSaverTask();
    }
    ASSERT_EQ(0, method->GetSaverTryCounter());
    method->TryCreateSaverTask();
    ASSERT_EQ(SAVER_COUNTER_INIT, method->GetSaverTryCounter());
}

TEST_F(ProfilingRunnerTest, BranchStatisticsCpp)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerProfilingThreshold(1U);
    runner.GetRuntimeOptions().SetInterpreterType(INTERPRETER_TYPE_CPP);
    runner.GetRuntimeOptions().SetProfileBranches(true);  // Enable branch profiling
    auto runtime = runner.CreateRuntime();
    runner.Run(runtime, SOURCE, std::vector<std::string> {});
    auto method = runner.GetMethod(METHOD_NAME_FOO);
    auto profilingData = method->GetProfilingData();
    ASSERT_EQ(132U, profilingData->GetBranchTakenCounter(0x10U));
    ASSERT_EQ(199U, profilingData->GetBranchNotTakenCounter(0x09U));
    ASSERT_EQ(67U, profilingData->GetBranchNotTakenCounter(0x10U));
    CheckSaverCounter(method);
    Runtime::Destroy();
}

TEST_F(ProfilingRunnerTest, ProfilingDataNullptTestCpp)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetInterpreterType(INTERPRETER_TYPE_CPP);
    auto runtime = runner.CreateRuntime();
    runner.Run(runtime, SOURCE, std::vector<std::string> {});
    auto method = runner.GetMethod(METHOD_NAME_FOO);
    auto profilingData = method->GetProfilingData();
    ASSERT_EQ(nullptr, profilingData);
    ProfilingSaver saver;
    pgo::AotProfilingData profData;
    saver.AddMethod(&profData, method, 0);
    Runtime::Destroy();
}

#ifndef PANDA_COMPILER_TARGET_AARCH32
TEST_F(ProfilingRunnerTest, BranchStatistics)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerProfilingThreshold(1U);
    runner.GetRuntimeOptions().SetProfileBranches(true);  // Enable branch profiling
    auto runtime = runner.CreateRuntime();
    runner.Run(runtime, SOURCE, std::vector<std::string> {});
    auto method = runner.GetMethod(METHOD_NAME_FOO);
    auto profilingData = method->GetProfilingData();
    ASSERT_EQ(132U, profilingData->GetBranchTakenCounter(0x10U));
    ASSERT_EQ(199U, profilingData->GetBranchNotTakenCounter(0x09U));
    ASSERT_EQ(67U, profilingData->GetBranchNotTakenCounter(0x10U));
    CheckSaverCounter(method);
    Runtime::Destroy();
}

TEST_F(ProfilingRunnerTest, ProfilingDataNullptTest)
{
    PandaRunner runner;
    auto runtime = runner.CreateRuntime();
    runner.Run(runtime, SOURCE, std::vector<std::string> {});
    auto method = runner.GetMethod(METHOD_NAME_FOO);
    auto profilingData = method->GetProfilingData();
    ASSERT_EQ(nullptr, profilingData);
    ProfilingSaver saver;
    pgo::AotProfilingData profData;
    saver.AddMethod(&profData, method, 0);
    Runtime::Destroy();
}

// Tests for branch profiling flag functionality - Assembly Interpreter
TEST_F(ProfilingRunnerTest, BranchProfilingDisabled)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerProfilingThreshold(1U);
    runner.GetRuntimeOptions().SetProfileBranches(false);  // Disable branch profiling
    // Use assembly interpreter (default)
    auto runtime = runner.CreateRuntime();
    runner.Run(runtime, SOURCE, std::vector<std::string> {});
    auto method = runner.GetMethod(METHOD_NAME_FOO);
    auto profilingData = method->GetProfilingData();

    // Verify ProfilingData exists but branch profiling is disabled
    ASSERT_NE(nullptr, profilingData);
    ASSERT_FALSE(profilingData->IsBranchProfilingEnabled());

    // Branch counters should be zero since profiling is disabled
    ASSERT_EQ(0U, profilingData->GetBranchTakenCounter(0x10U));
    ASSERT_EQ(0U, profilingData->GetBranchNotTakenCounter(0x09U));
    ASSERT_EQ(0U, profilingData->GetBranchNotTakenCounter(0x10U));

    Runtime::Destroy();
}

TEST_F(ProfilingRunnerTest, BranchProfilingEnabled)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerProfilingThreshold(1U);
    runner.GetRuntimeOptions().SetProfileBranches(true);  // Enable branch profiling
    // Use assembly interpreter (default)
    auto runtime = runner.CreateRuntime();
    runner.Run(runtime, SOURCE, std::vector<std::string> {});
    auto method = runner.GetMethod(METHOD_NAME_FOO);
    auto profilingData = method->GetProfilingData();

    // Verify ProfilingData exists and branch profiling is enabled
    ASSERT_NE(nullptr, profilingData);
    ASSERT_TRUE(profilingData->IsBranchProfilingEnabled());

    // Branch counters should have expected values when profiling is enabled
    ASSERT_EQ(132U, profilingData->GetBranchTakenCounter(0x10U));
    ASSERT_EQ(199U, profilingData->GetBranchNotTakenCounter(0x09U));
    ASSERT_EQ(67U, profilingData->GetBranchNotTakenCounter(0x10U));

    CheckSaverCounter(method);
    Runtime::Destroy();
}

// Tests for branch profiling flag functionality - C++ Interpreter
TEST_F(ProfilingRunnerTest, BranchProfilingDisabledCpp)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetInterpreterType(INTERPRETER_TYPE_CPP);
    runner.GetRuntimeOptions().SetCompilerProfilingThreshold(1U);
    // Disable branch profiling
    runner.GetRuntimeOptions().SetProfileBranches(false);
    auto runtime = runner.CreateRuntime();
    runner.Run(runtime, SOURCE, std::vector<std::string> {});
    auto method = runner.GetMethod(METHOD_NAME_FOO);
    auto profilingData = method->GetProfilingData();

    // Verify ProfilingData exists but branch profiling is disabled
    ASSERT_NE(nullptr, profilingData);
    ASSERT_FALSE(profilingData->IsBranchProfilingEnabled());

    // Branch counters should be zero since profiling is disabled
    ASSERT_EQ(0U, profilingData->GetBranchTakenCounter(0x10U));
    ASSERT_EQ(0U, profilingData->GetBranchNotTakenCounter(0x09U));
    ASSERT_EQ(0U, profilingData->GetBranchNotTakenCounter(0x10U));

    Runtime::Destroy();
}

TEST_F(ProfilingRunnerTest, BranchProfilingEnabledCpp)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetInterpreterType(INTERPRETER_TYPE_CPP);
    runner.GetRuntimeOptions().SetCompilerProfilingThreshold(1U);
    // Enable branch profiling
    runner.GetRuntimeOptions().SetProfileBranches(true);
    auto runtime = runner.CreateRuntime();
    runner.Run(runtime, SOURCE, std::vector<std::string> {});
    auto method = runner.GetMethod(METHOD_NAME_FOO);
    auto profilingData = method->GetProfilingData();

    // Verify ProfilingData exists and branch profiling is enabled
    ASSERT_NE(nullptr, profilingData);
    ASSERT_TRUE(profilingData->IsBranchProfilingEnabled());

    // Branch counters should have expected values when profiling is enabled
    ASSERT_EQ(132U, profilingData->GetBranchTakenCounter(0x10U));
    ASSERT_EQ(199U, profilingData->GetBranchNotTakenCounter(0x09U));
    ASSERT_EQ(67U, profilingData->GetBranchNotTakenCounter(0x10U));

    CheckSaverCounter(method);
    Runtime::Destroy();
}

TEST_F(ProfilingRunnerTest, ProfilingDataFlagsConstruction)
{
    // Test ProfilingData construction with different flags
    PandaRunner runner;
    [[maybe_unused]] auto runtime = runner.CreateRuntime();
    auto allocator = Runtime::GetCurrent()->GetInternalAllocator();

    // Test with branch profiling enabled
    auto profilingDataEnabled = ProfilingData::Make(
        allocator, 0, 0, 0,
        [&](void *data, [[maybe_unused]] void *vcallsMem, [[maybe_unused]] void *branchesMem,
            [[maybe_unused]] void *throwsMem) { return new (data) ProfilingData({}, {}, {}, true); });

    ASSERT_NE(nullptr, profilingDataEnabled);
    ASSERT_TRUE(profilingDataEnabled->IsBranchProfilingEnabled());

    // Test with branch profiling disabled
    auto profilingDataDisabled = ProfilingData::Make(
        allocator, 0, 0, 0,
        [&](void *data, [[maybe_unused]] void *vcallsMem, [[maybe_unused]] void *branchesMem,
            [[maybe_unused]] void *throwsMem) { return new (data) ProfilingData({}, {}, {}, false); });

    ASSERT_NE(nullptr, profilingDataDisabled);
    ASSERT_FALSE(profilingDataDisabled->IsBranchProfilingEnabled());

    // Test with default flags (should be disabled)
    auto profilingDataDefault =
        ProfilingData::Make(allocator, 0, 0, 0,
                            [&](void *data, [[maybe_unused]] void *vcallsMem, [[maybe_unused]] void *branchesMem,
                                [[maybe_unused]] void *throwsMem) {
                                return new (data) ProfilingData({}, {}, {});  // No flags parameter - use default
                            });

    ASSERT_NE(nullptr, profilingDataDefault);
    ASSERT_FALSE(profilingDataDefault->IsBranchProfilingEnabled());  // Default should be disabled

    // Cleanup
    allocator->Free(profilingDataEnabled);
    allocator->Free(profilingDataDisabled);
    allocator->Free(profilingDataDefault);

    Runtime::Destroy();
}

#endif
}  // namespace ark::test
