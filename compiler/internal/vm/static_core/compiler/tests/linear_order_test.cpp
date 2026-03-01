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

#include "libarkbase/macros.h"
#include "unit_test.h"
#include "jit/profiling_data.h"
#include "compiler/optimizer/analysis/linear_order.h"
#include "compiler/optimizer/optimizations/loop_peeling.h"
#include "compiler/optimizer/optimizations/loop_unroll.h"
#include "optimizer/optimizations/licm.h"
#include "optimizer/optimizations/licm_conditions.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"

namespace ark::compiler {

// NOLINTBEGIN(readability-magic-numbers)
class LinearOrderTest : public AsmTest {
public:
    LinearOrderTest() : defaultThreshold_(compiler::g_options.GetCompilerFreqBasedBranchReorderThreshold())
    {
        compiler::g_options.SetCompilerFreqBasedBranchReorderThreshold(10U);
    }

    ~LinearOrderTest() override
    {
        compiler::g_options.SetCompilerFreqBasedBranchReorderThreshold(defaultThreshold_);
    }

    NO_COPY_SEMANTIC(LinearOrderTest);
    NO_MOVE_SEMANTIC(LinearOrderTest);

    void StartProfiling()
    {
        auto method = reinterpret_cast<Method *>(GetGraph()->GetMethod());
        PandaVM::GetCurrent()->GetMutatorLock()->WriteLock();
        method->StartProfiling();
        PandaVM::GetCurrent()->GetMutatorLock()->Unlock();
    }

    void UpdateBranchTaken(uint32_t pc, uint32_t count = 1U)
    {
        auto method = reinterpret_cast<Method *>(GetGraph()->GetMethod());
        auto profilingData = method->GetProfilingData();
        for (uint32_t i = 0; i < count; i++) {
            profilingData->UpdateBranchTaken(pc);
        }
    }

    void UpdateBranchNotTaken(uint32_t pc, uint32_t count = 1U)
    {
        auto method = reinterpret_cast<Method *>(GetGraph()->GetMethod());
        auto profilingData = method->GetProfilingData();
        for (uint32_t i = 0; i < count; i++) {
            profilingData->UpdateBranchNotTaken(pc);
        }
    }

    void Reset()
    {
        GetGraph()->GetValidAnalysis<LinearOrder>().SetValid(false);
        auto blocks = GetGraph()->GetVectorBlocks();
        std::for_each(std::begin(blocks), std::end(blocks), [](BasicBlock *b) {
            if (b != nullptr) {
                b->SetNeedsJump(false);
            }
        });
    }

    const BasicBlock *GetOrderedBasicBlock(uint32_t id, uint32_t pos)
    {
        const auto &blocks = GetGraph()->GetBlocksLinearOrder();
        return *(std::find_if(std::begin(blocks), std::end(blocks), [id](BasicBlock *b) { return b->GetId() == id; }) +
                 pos);
    }

    bool CheckOrder(uint32_t id, std::initializer_list<uint32_t> expected)
    {
        Reset();
        const auto &blocks = GetGraph()->GetBlocksLinearOrder();
        auto actualIt =
            std::find_if(std::begin(blocks), std::end(blocks), [id](BasicBlock *b) { return b->GetId() == id; }) + 1U;
        auto expectedIt = std::begin(expected);
        while (actualIt != std::end(blocks) && expectedIt != std::end(expected)) {
            if ((*actualIt)->GetId() != *expectedIt) {
                return false;
            }
            ++actualIt;
            ++expectedIt;
        }
        return expectedIt == std::end(expected);
    }

private:
    uint32_t defaultThreshold_;
};

TEST_F(LinearOrderTest, RareLoopSideExit)
{
    auto source = R"(
    .record Test <> {
            u1 a <static>
    }
    .function i32 foo() <static> {
            movi v0, 0xa
            movi v1, 0x0
            mov v2, v1
            jump_label_2: lda v2
            jge v0, jump_label_0
            ldstatic Test.a
            jeqz jump_label_1
            lda v2
            return
            jump_label_1: lda v1
            addi 0x1
            sta v3
            lda v2
            addi 0x1
            sta v1
            mov v2, v1
            mov v1, v3
            jmp jump_label_2
            jump_label_0: lda v1
            return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "foo"));
    ASSERT_TRUE(CheckOrder(2U, {4U, 3U})) << "Unexpected initial order";

    StartProfiling();
    UpdateBranchTaken(0xFU);

    ASSERT_TRUE(CheckOrder(2U, {3U, 4U})) << "Unexpected order";
}

TEST_F(LinearOrderTest, FrequencyThreshold)
{
    auto source = R"(
    .record Test <> {
            u1 f <static>
    }
    .function void foo() <static> {
            return.void
    }

    .function void bar() <static> {
            return.void
    }
    .function void main() <static> {
            movi v0, 0x64
            movi v1, 0x0
            jump_label_3: lda v1
            jge v0, jump_label_0
            ldstatic Test.f
            jeqz jump_label_1
            call.short foo
            jmp jump_label_2
            jump_label_1: call.short bar
            jump_label_2: inci v1, 0x1
            jmp jump_label_3
            jump_label_0: return.void
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    ASSERT_TRUE(CheckOrder(2U, {4U, 3U, 5U})) << "Unexpected initial order";

    StartProfiling();
    UpdateBranchNotTaken(0xDU, 90U);
    UpdateBranchTaken(0xDU, 99U);

    ASSERT_TRUE(CheckOrder(2U, {4U, 3U, 5U})) << "Unexpected order, threshold was not exceeded";

    UpdateBranchTaken(0xDU);
    ASSERT_TRUE(CheckOrder(2U, {3U, 5U, 4U})) << "Unexpected order, threshold was exceeded";

    UpdateBranchNotTaken(0xDU, 21U);
    ASSERT_TRUE(CheckOrder(2U, {3U, 4U, 5U})) << "Unexpected order, another branch didn't exceed threshold";

    UpdateBranchNotTaken(0xDU);
    ASSERT_TRUE(CheckOrder(2U, {4U, 5U, 3U})) << "Unexpected order, another branch exceeded threshold";
}

TEST_F(LinearOrderTest, LoopTransform)
{
    auto source = R"(
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
    ASSERT_TRUE(ParseToGraph(source, "foo"));
    ASSERT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>(false));
    ASSERT_TRUE(GetGraph()->RunPass<LoopUnroll>(100U, 3U));

    if (!CheckOrder(20U, {4U, 3U, 5U, 22U, 24U, 25U, 23U, 26U, 28U, 29U, 27U, 21U, 1U})) {
        FAIL() << "Unexpected initial order";
    }

    StartProfiling();
    UpdateBranchTaken(0x10U, 10U);
    UpdateBranchNotTaken(0x10U);

    if (!CheckOrder(20U, {3U, 5U, 22U, 25U, 23U, 26U, 29U, 27U, 21U, 28U, 24U, 4U, 1U})) {
        FAIL() << "Unexpected order, threshold was exceeded";
    }

    UpdateBranchNotTaken(0x10U, 20U);

    if (!CheckOrder(20U, {4U, 5U, 22U, 24U, 23U, 26U, 28U, 27U, 21U, 29U, 25U, 3U, 1U})) {
        FAIL() << "Unexpected order, another branch threshold was exceeded";
    }
}

TEST_F(LinearOrderTest, ThrowBlock1)
{
    auto graph = CreateGraphWithDefaultRuntime();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::Return).i32().Inputs(0U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(8U, Opcode::Throw).Inputs(1U, 7U);
        }
    }
    const auto &blocks = graph->GetBlocksLinearOrder();
    ASSERT_EQ(&BB(4U), blocks.back());

    auto graph1 = CreateGraphWithDefaultRuntime();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::Return).i32().Inputs(0U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(8U, Opcode::Throw).Inputs(1U, 7U);
        }
    }
    const auto &blocks1 = graph1->GetBlocksLinearOrder();
    ASSERT_EQ(&BB(4U), blocks1.back());
}

TEST_F(LinearOrderTest, ThrowBlock2)
{
    auto graph2 = CreateGraphWithDefaultRuntime();
    GRAPH(graph2)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(8U, Opcode::Throw).Inputs(1U, 7U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::Return).i32().Inputs(0U);
        }
    }
    const auto &blocks2 = graph2->GetBlocksLinearOrder();
    ASSERT_EQ(&BB(4U), blocks2.back());

    auto graph3 = CreateGraphWithDefaultRuntime();
    GRAPH(graph3)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(3U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(8U, Opcode::Throw).Inputs(1U, 7U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::Return).i32().Inputs(0U);
        }
    }
    const auto &blocks3 = graph3->GetBlocksLinearOrder();
    ASSERT_EQ(&BB(4U), blocks3.back());
}

TEST_F(LinearOrderTest, ConditionChainHoisting)
{
    auto source = R"(
    .function i32 foo(i32 a0, i32 a1) <static> {
        movi v0, 0x64
        movi v1, 0x0
        mov v3, a0
        mov v2, a1
        mov v4, v1
jump_label_4:
        lda v4
        jge v0, jump_label_0
        lda v3
        jnez jump_label_1
        lda v2
        jeqz jump_label_2
jump_label_1:
        add v4, v1
        sta v1
        jmp jump_label_3
jump_label_2:
        sub v1, v4
        sta v1
jump_label_3:
        inci v4, 0x1
        jmp jump_label_4
jump_label_0:
        lda v1
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "foo"));
    GetGraph()->RunPass<Cleanup>(false);
    GetGraph()->RunPass<Licm>(g_options.GetCompilerLicmHoistLimit());
    GetGraph()->RunPass<Cleanup>();
    ASSERT_TRUE(GetGraph()->RunPass<LicmConditions>());
    ASSERT_TRUE(GetGraph()->RunPass<LoopPeeling>());
    GetGraph()->RunPass<Cleanup>();
    ASSERT_TRUE(RegAlloc(GetGraph()));

    ASSERT_TRUE(CheckOrder(10U, {5U, 3U, 6U})) << "Unexpected initial order";

    StartProfiling();

    UpdateBranchTaken(0x12U, 10U);

    ASSERT_TRUE(CheckOrder(10U, {3U, 6U})) << "Unexpected order, threshold 1 was exceeded";

    UpdateBranchTaken(0x16U, 100U);
    ASSERT_TRUE(CheckOrder(10U, {5U, 6U})) << "Unexpected order, threshold 2 was exceeded";

    UpdateBranchNotTaken(0x16U, 200U);
    ASSERT_TRUE(CheckOrder(10U, {3U, 6U})) << "Unexpected order, threshold 3 was exceeded";
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
