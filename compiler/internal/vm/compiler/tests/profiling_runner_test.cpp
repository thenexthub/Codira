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

namespace panda::test {
class ProfilingRunnerTest : public testing::Test {
};

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

TEST_F(ProfilingRunnerTest, BranchStatistics)
{
    PandaRunner runner;
    runner.GetRuntimeOptions().SetCompilerProfilingThreshold(1);
    auto runtime = runner.CreateRuntime();
    runner.Run(runtime, SOURCE, std::vector<std::string> {});
    auto method = runner.GetMethod("foo");
    auto profiling_data = method->GetProfilingData();
    ASSERT_EQ(132, profiling_data->GetBranchTakenCounter(0x10));
    ASSERT_EQ(199, profiling_data->GetBranchNotTakenCounter(0x09));
    ASSERT_EQ(67, profiling_data->GetBranchNotTakenCounter(0x10));
    Runtime::Destroy();
}
}  // namespace panda::test
