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

#ifndef COMPILER_TESTS_CODEGEN_CODEGEN_TEST_H
#define COMPILER_TESTS_CODEGEN_CODEGEN_TEST_H

#include <random>

#include "optimizer/code_generator/codegen.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/if_conversion.h"
#include "optimizer/optimizations/lowering.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "optimizer_run.h"
#include "compiler/inplace_task_runner.h"
#include "compiler/compiler_task_runner.h"

#include "libarkbase/macros.h"
#include "libarkbase/utils/utils.h"
#include "gtest/gtest.h"
#include "compiler/tests/unit_test.h"
#include "libarkbase/utils/bit_utils.h"
#include "compiler/tests/vixl_exec_module.h"

namespace ark::compiler {

class CodegenTest : public GraphTest {
public:
    CodegenTest() : execModule_(GetAllocator(), GetGraph()->GetRuntime())
    {
#ifndef NDEBUG
        // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
        GetGraph()->SetLowLevelInstructionsEnabled();
#endif
    }
    ~CodegenTest() override = default;

    NO_COPY_SEMANTIC(CodegenTest);
    NO_MOVE_SEMANTIC(CodegenTest);

    VixlExecModule &GetExecModule()
    {
        return execModule_;
    }

    template <typename T>
    void CheckStoreArray();

    template <typename T>
    void CheckLoadArray();

    template <typename T>
    void CheckStoreArrayPair(bool imm);

    template <typename T>
    void CheckLoadArrayPair(bool imm);

    template <typename T>
    void CheckCmp(bool isFcmpg = false);

    template <typename T>
    void CheckReturnValue(Graph *graph, T expectedValue);

    template <typename T>
    void CheckBounds(uint64_t count);

    template <Opcode OPCODE, uint32_t L, uint32_t R, ShiftType SHIFT_TYPE, uint32_t SHIFT, uint32_t ERV>
    void TestBinaryOperationWithShiftedOperand();

    void CreateGraphForOverflowTest(Graph *graph, Opcode overflowOpcode);

private:
    VixlExecModule execModule_;
};

inline bool RunCodegen(Graph *graph)
{
    return graph->RunPass<Codegen>();
}

template <typename T>
void CodegenTest::CheckReturnValue(Graph *graph, [[maybe_unused]] T expectedValue)
{
    SetNumVirtRegs(0U);
    RegAlloc(graph);
    EXPECT_TRUE(RunCodegen(graph));

    auto codeEntry = reinterpret_cast<char *>(graph->GetCode().Data());
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto codeExit = codeEntry + graph->GetCode().Size();

    ASSERT(codeEntry != nullptr);
    ASSERT(codeExit != nullptr);

    GetExecModule().SetInstructions(codeEntry, codeExit);
    GetExecModule().SetDump(false);

    GetExecModule().Execute();
    auto rv = GetExecModule().GetRetValue<T>();
    EXPECT_EQ(rv, expectedValue);
}

}  // namespace ark::compiler

#endif  // COMPILER_TESTS_CODEGEN_CODEGEN_TEST_H
