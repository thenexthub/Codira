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
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/deoptimize_elimination.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/ir/runtime_interface.h"

namespace ark::compiler {
class DeoptimizeEliminationTest : public CommonTest {
public:
    DeoptimizeEliminationTest()
        : graph_(CreateGraphStartEndBlocks()),
          defaultCompilerSafePointsRequireRegMap_(g_options.IsCompilerSafePointsRequireRegMap())
    {
    }

    ~DeoptimizeEliminationTest() override
    {
        g_options.SetCompilerSafePointsRequireRegMap(defaultCompilerSafePointsRequireRegMap_);
    }

    NO_COPY_SEMANTIC(DeoptimizeEliminationTest);
    NO_MOVE_SEMANTIC(DeoptimizeEliminationTest);

    Graph *GetGraph()
    {
        return graph_;
    }

private:
    Graph *graph_ {nullptr};
    bool defaultCompilerSafePointsRequireRegMap_;
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(DeoptimizeEliminationTest, DeoptimizeIfTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            // cleanup delete this
            INST(10U, Opcode::SaveState).Inputs(1U).SrcVregs({1U});

            INST(3U, Opcode::SaveStateDeoptimize).Inputs(1U).SrcVregs({1U});
            INST(4U, Opcode::Compare).b().Inputs(0U, 1U).CC(CC_GT);
            INST(5U, Opcode::DeoptimizeIf).Inputs(4U, 3U);
            // 5 is dominate by 7
            INST(6U, Opcode::SaveStateDeoptimize).Inputs(1U).SrcVregs({1U});
            INST(7U, Opcode::DeoptimizeIf).Inputs(4U, 6U);

            // redundant
            INST(8U, Opcode::DeoptimizeIf).Inputs(1U, 6U);

            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::SaveStateDeoptimize).Inputs(1U).SrcVregs({1U});
            INST(4U, Opcode::Compare).b().Inputs(0U, 1U).CC(CC_GT);
            INST(5U, Opcode::DeoptimizeIf).Inputs(4U, 3U);

            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardOneBlockTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(10U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(0U, Opcode::SaveStateDeoptimize).Inputs(10U).SrcVregs({10U});
            INST(1U, Opcode::IsMustDeoptimize).b();
            INST(2U, Opcode::DeoptimizeIf).Inputs(1U, 0U);

            INST(3U, Opcode::SaveStateDeoptimize).Inputs(10U).SrcVregs({10U});
            INST(4U, Opcode::IsMustDeoptimize).b();
            INST(5U, Opcode::DeoptimizeIf).Inputs(4U, 3U);

            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(10U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(0U, Opcode::SaveStateDeoptimize).Inputs(10U).SrcVregs({10U});
            INST(1U, Opcode::IsMustDeoptimize).b();
            INST(2U, Opcode::DeoptimizeIf).Inputs(1U, 0U);

            INST(3U, Opcode::NOP);
            INST(4U, Opcode::NOP);
            INST(5U, Opcode::NOP);

            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardOneBlockCallTest)
{
    // Not applied. Call to runtime between guards.
    GRAPH(GetGraph())
    {
        CONSTANT(10U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(0U, Opcode::SaveStateDeoptimize).Inputs(10U).SrcVregs({10U});
            INST(1U, Opcode::IsMustDeoptimize).b();
            INST(2U, Opcode::DeoptimizeIf).Inputs(1U, 0U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(20U);

            INST(3U, Opcode::SaveStateDeoptimize).Inputs(10U).SrcVregs({10U});
            INST(4U, Opcode::IsMustDeoptimize).b();
            INST(5U, Opcode::DeoptimizeIf).Inputs(4U, 3U);

            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<DeoptimizeElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardOneBlockSeveralGuardsTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(10U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(0U, Opcode::SaveStateDeoptimize).Inputs(10U).SrcVregs({10U});
            INST(1U, Opcode::IsMustDeoptimize).b();
            INST(2U, Opcode::DeoptimizeIf).Inputs(1U, 0U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(20U);

            INST(3U, Opcode::SaveStateDeoptimize).Inputs(10U).SrcVregs({10U});
            INST(4U, Opcode::IsMustDeoptimize).b();
            INST(5U, Opcode::DeoptimizeIf).Inputs(4U, 3U);

            INST(11U, Opcode::SaveStateDeoptimize).Inputs(10U).SrcVregs({10U});
            INST(12U, Opcode::IsMustDeoptimize).b();
            INST(13U, Opcode::DeoptimizeIf).Inputs(12U, 11U);

            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(10U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(0U, Opcode::SaveStateDeoptimize).Inputs(10U).SrcVregs({10U});
            INST(1U, Opcode::IsMustDeoptimize).b();
            INST(2U, Opcode::DeoptimizeIf).Inputs(1U, 0U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(20U);

            INST(3U, Opcode::SaveStateDeoptimize).Inputs(10U).SrcVregs({10U});
            INST(4U, Opcode::IsMustDeoptimize).b();
            INST(5U, Opcode::DeoptimizeIf).Inputs(4U, 3U);

            INST(11U, Opcode::NOP);
            INST(12U, Opcode::NOP);
            INST(13U, Opcode::NOP);

            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardOneBlockCallInlinedTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(10U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(0U, Opcode::SaveStateDeoptimize).Inputs(10U).SrcVregs({10U});
            INST(1U, Opcode::IsMustDeoptimize).b();
            INST(2U, Opcode::DeoptimizeIf).Inputs(1U, 0U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().Inlined().InputsAutoType(20U);
            INST(7U, Opcode::ReturnInlined).Inputs(20U);

            INST(3U, Opcode::SaveStateDeoptimize).Inputs(10U).SrcVregs({10U});
            INST(4U, Opcode::IsMustDeoptimize).b();
            INST(5U, Opcode::DeoptimizeIf).Inputs(4U, 3U);

            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(10U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(0U, Opcode::SaveStateDeoptimize).Inputs(10U).SrcVregs({10U});
            INST(1U, Opcode::IsMustDeoptimize).b();
            INST(2U, Opcode::DeoptimizeIf).Inputs(1U, 0U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).v0id().Inlined().InputsAutoType(20U);
            INST(7U, Opcode::ReturnInlined).Inputs(20U);

            INST(3U, Opcode::NOP);
            INST(4U, Opcode::NOP);
            INST(5U, Opcode::NOP);

            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardIfTest)
{
    /*
     * Not applied.
     * bb1
     * guard
     * |   \
     * |   bb2
     * |   runtime call
     * |  /
     * bb3
     * guard
     */
    GRAPH(GetGraph())
    {
        PARAMETER(10U, 0U).s32();
        CONSTANT(11U, 1U);
        BASIC_BLOCK(2U, 4U, 5U)
        {
            INST(0U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(1U, Opcode::IsMustDeoptimize).b();
            INST(2U, Opcode::DeoptimizeIf).Inputs(1U, 0U);
            INST(3U, Opcode::Compare).b().CC(CC_LT).Inputs(10U, 11U);
            INST(4U, Opcode::IfImm).CC(CC_NE).Inputs(3U).Imm(0U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallStatic).v0id().InputsAutoType(20U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(6U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(7U, Opcode::IsMustDeoptimize).b();
            INST(8U, Opcode::DeoptimizeIf).Inputs(7U, 6U);
            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<DeoptimizeElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

SRC_GRAPH(ChaGuardIfSeveralGuardsTest, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(10U, 0U).s32();
        CONSTANT(11U, 1U);
        BASIC_BLOCK(2U, 4U, 5U)
        {
            INST(0U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(1U, Opcode::IsMustDeoptimize).b();
            INST(2U, Opcode::DeoptimizeIf).Inputs(1U, 0U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(12U, Opcode::CallStatic).v0id().InputsAutoType(20U);
            INST(13U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(14U, Opcode::IsMustDeoptimize).b();
            INST(15U, Opcode::DeoptimizeIf).Inputs(14U, 13U);
            INST(3U, Opcode::Compare).b().CC(CC_LT).Inputs(10U, 11U);
            INST(4U, Opcode::IfImm).CC(CC_NE).Inputs(3U).Imm(0U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(21U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallStatic).v0id().Inlined().InputsAutoType(21U);
            INST(16U, Opcode::ReturnInlined).Inputs(21U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(6U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(7U, Opcode::IsMustDeoptimize).b();
            INST(8U, Opcode::DeoptimizeIf).Inputs(7U, 6U);
            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(ChaGuardIfSeveralGuardsTest, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(10U, 0U).s32();
        CONSTANT(11U, 1U);
        BASIC_BLOCK(2U, 4U, 5U)
        {
            INST(0U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(1U, Opcode::IsMustDeoptimize).b();
            INST(2U, Opcode::DeoptimizeIf).Inputs(1U, 0U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(12U, Opcode::CallStatic).v0id().InputsAutoType(20U);
            INST(13U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(14U, Opcode::IsMustDeoptimize).b();
            INST(15U, Opcode::DeoptimizeIf).Inputs(14U, 13U);
            INST(3U, Opcode::Compare).b().CC(CC_LT).Inputs(10U, 11U);
            INST(4U, Opcode::IfImm).CC(CC_NE).Inputs(3U).Imm(0U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(21U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallStatic).v0id().Inlined().InputsAutoType(21U);
            INST(16U, Opcode::ReturnInlined).Inputs(21U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(6U, Opcode::NOP);
            INST(7U, Opcode::NOP);
            INST(8U, Opcode::NOP);
            INST(9U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(DeoptimizeEliminationTest, ChaGuardIfSeveralGuardsTest)
{
    /*
     * bb1
     * guard
     * call
     * guard
     * |   \
     * |   bb2
     * |   some inst
     * |  /
     * bb3
     * guard
     */
    src_graph::ChaGuardIfSeveralGuardsTest::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    out_graph::ChaGuardIfSeveralGuardsTest::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(ChaGuardLoopTest, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(10U, 0U);
        CONSTANT(11U, 1U);
        CONSTANT(12U, 10U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(0U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(1U, Opcode::IsMustDeoptimize).b();
            INST(2U, Opcode::DeoptimizeIf).Inputs(1U, 0U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(3U, Opcode::Phi).s32().Inputs(10U, 7U);
            INST(8U, Opcode::Compare).b().CC(CC_LT).Inputs(3U, 12U);
            INST(9U, Opcode::IfImm).CC(CC_NE).Inputs(8U).Imm(0U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(4U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(5U, Opcode::IsMustDeoptimize).b();
            INST(6U, Opcode::DeoptimizeIf).Inputs(5U, 4U);
            INST(7U, Opcode::Add).s32().Inputs(3U, 11U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(13U, Opcode::Return).s32().Inputs(3U);
        }
    }
}

OUT_GRAPH(ChaGuardLoopTest, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(10U, 0U);
        CONSTANT(11U, 1U);
        CONSTANT(12U, 10U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(0U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(1U, Opcode::IsMustDeoptimize).b();
            INST(2U, Opcode::DeoptimizeIf).Inputs(1U, 0U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(3U, Opcode::Phi).s32().Inputs(10U, 7U);
            INST(8U, Opcode::Compare).b().CC(CC_LT).Inputs(3U, 12U);
            INST(9U, Opcode::IfImm).CC(CC_NE).Inputs(8U).Imm(0U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(4U, Opcode::NOP);
            INST(5U, Opcode::NOP);
            INST(6U, Opcode::NOP);
            INST(7U, Opcode::Add).s32().Inputs(3U, 11U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(13U, Opcode::Return).s32().Inputs(3U);
        }
    }
}

TEST_F(DeoptimizeEliminationTest, ChaGuardLoopTest)
{
    src_graph::ChaGuardLoopTest::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    out_graph::ChaGuardLoopTest::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardLoopWithCallAfterGuardTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(10U, 0U);
        CONSTANT(11U, 1U);
        CONSTANT(12U, 10U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(0U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(1U, Opcode::IsMustDeoptimize).b();
            INST(2U, Opcode::DeoptimizeIf).Inputs(1U, 0U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(3U, Opcode::Phi).s32().Inputs(10U, 7U);
            INST(8U, Opcode::Compare).b().CC(CC_LT).Inputs(3U, 12U);
            INST(9U, Opcode::IfImm).CC(CC_NE).Inputs(8U).Imm(0U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(4U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(5U, Opcode::IsMustDeoptimize).b();
            INST(6U, Opcode::DeoptimizeIf).Inputs(5U, 4U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).v0id().InputsAutoType(20U);
            INST(7U, Opcode::Add).s32().Inputs(3U, 11U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(13U, Opcode::Return).s32().Inputs(3U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<DeoptimizeElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardLoopWithCallBeforeGuardTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(10U, 0U);
        CONSTANT(11U, 1U);
        CONSTANT(12U, 10U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(0U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(1U, Opcode::IsMustDeoptimize).b();
            INST(2U, Opcode::DeoptimizeIf).Inputs(1U, 0U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(3U, Opcode::Phi).s32().Inputs(10U, 7U);
            INST(8U, Opcode::Compare).b().CC(CC_LT).Inputs(3U, 12U);
            INST(9U, Opcode::IfImm).CC(CC_NE).Inputs(8U).Imm(0U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).v0id().InputsAutoType(20U);
            INST(4U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(5U, Opcode::IsMustDeoptimize).b();
            INST(6U, Opcode::DeoptimizeIf).Inputs(5U, 4U);
            INST(7U, Opcode::Add).s32().Inputs(3U, 11U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(13U, Opcode::Return).s32().Inputs(3U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<DeoptimizeElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

SRC_GRAPH(ChaGuardLoopWithCallBetweenGuardsWithDomGuardTest, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(10U, 0U);
        CONSTANT(11U, 1U);
        CONSTANT(12U, 10U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(0U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(1U, Opcode::IsMustDeoptimize).b();
            INST(2U, Opcode::DeoptimizeIf).Inputs(1U, 0U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(3U, Opcode::Phi).s32().Inputs(10U, 7U);
            INST(8U, Opcode::Compare).b().CC(CC_LT).Inputs(3U, 12U);
            INST(9U, Opcode::IfImm).CC(CC_NE).Inputs(8U).Imm(0U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(15U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(16U, Opcode::IsMustDeoptimize).b();
            INST(17U, Opcode::DeoptimizeIf).Inputs(16U, 15U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).v0id().InputsAutoType(20U);
            INST(4U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(5U, Opcode::IsMustDeoptimize).b();
            INST(6U, Opcode::DeoptimizeIf).Inputs(5U, 4U);
            INST(7U, Opcode::Add).s32().Inputs(3U, 11U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(13U, Opcode::Return).s32().Inputs(3U);
        }
    }
}

OUT_GRAPH(ChaGuardLoopWithCallBetweenGuardsWithDomGuardTest, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(10U, 0U);
        CONSTANT(11U, 1U);
        CONSTANT(12U, 10U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(0U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(1U, Opcode::IsMustDeoptimize).b();
            INST(2U, Opcode::DeoptimizeIf).Inputs(1U, 0U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(3U, Opcode::Phi).s32().Inputs(10U, 7U);
            INST(8U, Opcode::Compare).b().CC(CC_LT).Inputs(3U, 12U);
            INST(9U, Opcode::IfImm).CC(CC_NE).Inputs(8U).Imm(0U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(15U, Opcode::NOP);
            INST(16U, Opcode::NOP);
            INST(17U, Opcode::NOP);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).v0id().InputsAutoType(20U);
            INST(4U, Opcode::SaveStateDeoptimize).Inputs(10U, 11U).SrcVregs({0U, 1U});
            INST(5U, Opcode::IsMustDeoptimize).b();
            INST(6U, Opcode::DeoptimizeIf).Inputs(5U, 4U);
            INST(7U, Opcode::Add).s32().Inputs(3U, 11U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(13U, Opcode::Return).s32().Inputs(3U);
        }
    }
}

TEST_F(DeoptimizeEliminationTest, ChaGuardLoopWithCallBetweenGuardsWithDomGuardTest)
{
    src_graph::ChaGuardLoopWithCallBetweenGuardsWithDomGuardTest::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    out_graph::ChaGuardLoopWithCallBetweenGuardsWithDomGuardTest::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(ChaGuardLoopWithDuplicatedGuardsTest, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        PARAMETER(2U, 0U).s32();
        BASIC_BLOCK(2U, 3U, 10U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 2U);
            INST(4U, Opcode::IfImm).CC(CC_NE).Inputs(3U).Imm(0U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(19U, Opcode::Phi).s32().Inputs(0U, 14U);
            INST(5U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::IsMustDeoptimize).b();
            INST(7U, Opcode::DeoptimizeIf).Inputs(6U, 5U);
            INST(8U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 2U);
            INST(9U, Opcode::IfImm).CC(CC_NE).Inputs(8U).Imm(0U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(10U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(11U, Opcode::IsMustDeoptimize).b();
            INST(12U, Opcode::DeoptimizeIf).Inputs(11U, 10U);
        }
        BASIC_BLOCK(5U, 3U, 10U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(13U, Opcode::CallStatic).v0id().InputsAutoType(20U);
            INST(14U, Opcode::Add).s32().Inputs(19U, 1U);
            INST(15U, Opcode::Compare).b().CC(CC_LT).Inputs(14U, 2U);
            INST(16U, Opcode::IfImm).CC(CC_NE).Inputs(15U).Imm(0U);
        }

        BASIC_BLOCK(10U, 1U)
        {
            INST(17U, Opcode::Phi).s32().Inputs(0U, 14U);
            INST(18U, Opcode::Return).s32().Inputs(17U);
        }
    }
}

OUT_GRAPH(ChaGuardLoopWithDuplicatedGuardsTest, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        PARAMETER(2U, 0U).s32();
        BASIC_BLOCK(2U, 3U, 10U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 2U);
            INST(4U, Opcode::IfImm).CC(CC_NE).Inputs(3U).Imm(0U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(19U, Opcode::Phi).s32().Inputs(0U, 14U);
            INST(5U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::IsMustDeoptimize).b();
            INST(7U, Opcode::DeoptimizeIf).Inputs(6U, 5U);
            INST(8U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 2U);
            INST(9U, Opcode::IfImm).CC(CC_NE).Inputs(8U).Imm(0U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(10U, Opcode::NOP);
            INST(11U, Opcode::NOP);
            INST(12U, Opcode::NOP);
        }
        BASIC_BLOCK(5U, 3U, 10U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(13U, Opcode::CallStatic).v0id().InputsAutoType(20U);
            INST(14U, Opcode::Add).s32().Inputs(19U, 1U);
            INST(15U, Opcode::Compare).b().CC(CC_LT).Inputs(14U, 2U);
            INST(16U, Opcode::IfImm).CC(CC_NE).Inputs(15U).Imm(0U);
        }

        BASIC_BLOCK(10U, 1U)
        {
            INST(17U, Opcode::Phi).s32().Inputs(0U, 14U);
            INST(18U, Opcode::Return).s32().Inputs(17U);
        }
    }
}

TEST_F(DeoptimizeEliminationTest, ChaGuardLoopWithDuplicatedGuardsTest)
{
    src_graph::ChaGuardLoopWithDuplicatedGuardsTest::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    out_graph::ChaGuardLoopWithDuplicatedGuardsTest::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ChaGuardNotDominateTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).CC(CC_NE).Inputs(2U).Imm(0U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(4U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::IsMustDeoptimize).b();
            INST(6U, Opcode::DeoptimizeIf).Inputs(5U, 4U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(7U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(8U, Opcode::IsMustDeoptimize).b();
            INST(9U, Opcode::DeoptimizeIf).Inputs(8U, 7U);
            INST(10U, Opcode::ReturnVoid).v0id();
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<DeoptimizeElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

SRC_GRAPH(ChaGuardIfWithGuardsTest, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(6U, Opcode::IfImm).CC(CC_NE).Inputs(5U).Imm(0U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(20U);
            INST(8U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(9U, Opcode::IsMustDeoptimize).b();
            INST(10U, Opcode::DeoptimizeIf).Inputs(9U, 8U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(21U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::CallStatic).v0id().InputsAutoType(21U);
            INST(12U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(13U, Opcode::IsMustDeoptimize).b();
            INST(14U, Opcode::DeoptimizeIf).Inputs(13U, 12U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(15U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(16U, Opcode::IsMustDeoptimize).b();
            INST(17U, Opcode::DeoptimizeIf).Inputs(16U, 15U);
            INST(18U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(ChaGuardIfWithGuardsTest, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(6U, Opcode::IfImm).CC(CC_NE).Inputs(5U).Imm(0U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(20U);
            INST(8U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(9U, Opcode::IsMustDeoptimize).b();
            INST(10U, Opcode::DeoptimizeIf).Inputs(9U, 8U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(21U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::CallStatic).v0id().InputsAutoType(21U);
            INST(12U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(13U, Opcode::IsMustDeoptimize).b();
            INST(14U, Opcode::DeoptimizeIf).Inputs(13U, 12U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(15U, Opcode::NOP);
            INST(16U, Opcode::NOP);
            INST(17U, Opcode::NOP);
            INST(18U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(DeoptimizeEliminationTest, ChaGuardIfWithGuardsTest)
{
    /*
     *    some code
     *    /       \
     *  call     call
     * guard    guard
     * \          /
     *  \        /
     *    guard(*, deleted)
     */
    src_graph::ChaGuardIfWithGuardsTest::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    out_graph::ChaGuardIfWithGuardsTest::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ReplaceByDeoptimizeTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::DeoptimizeIf).Inputs(0U, 2U);
            INST(4U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::INVALID).Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(DeoptimizeEliminationTest, ReplaceByDeoptimizeInliningTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::CallStatic).v0id().InputsAutoType(2U).Inlined();
            INST(4U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(7U, Opcode::DeoptimizeIf).Inputs(0U, 4U);
            INST(5U, Opcode::ReturnInlined).Inputs(2U);
            INST(6U, Opcode::ReturnVoid).v0id();
        }
    }
    INS(4U).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(3U)));
    INS(4U).CastToSaveState()->SetInliningDepth(1U);

    ASSERT_TRUE(GetGraph()->RunPass<DeoptimizeElimination>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::CallStatic).v0id().InputsAutoType(2U).Inlined();
            INST(4U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(5U, Opcode::ReturnInlined).Inputs(2U);
            INST(8U, Opcode::Deoptimize).Inputs(4U);
        }
    }
    INS(4U).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(3U)));
    INS(4U).CastToSaveState()->SetInliningDepth(1U);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
