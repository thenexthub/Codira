/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "unit_test.h"
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/adjust_arefs.h"

namespace ark::compiler {
class AdjustRefsTest : public GraphTest {
public:
    AdjustRefsTest() = default;

    Graph *BuildGraphOneBlockContinuousChain();
    Graph *BuildGraphOneBlockBrokenChain();
    Graph *BuildGraphMultipleBlockContinuousChain();
    Graph *BuildGraphMultipleBlockBrokenChain();
    Graph *BuildGraphProcessIndex(uint64_t offset1, uint64_t offset2, uint64_t offset3);
};

// NOLINTBEGIN(readability-magic-numbers)

Graph *AdjustRefsTest::BuildGraphOneBlockContinuousChain()
{
    auto graph = CreateEmptyFastpathGraph(RUNTIME_ARCH);
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).u64();
        CONSTANT(3U, 10U).s32();

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(10U, Opcode::Phi).s32().Inputs(1U, 40U);
            INST(11U, Opcode::LoadArray).u64().Inputs(0U, 1U);
            INST(12U, Opcode::LoadArray).u64().Inputs(0U, 1U);
            INST(13U, Opcode::StoreArray).u64().Inputs(0U, 1U, 2U);
            INST(14U, Opcode::StoreArray).u64().Inputs(0U, 1U, 2U);
            INST(15U, Opcode::Compare).b().Inputs(10U, 1U);
            INST(19U, Opcode::IfImm).CC(CC_NE).Inputs(15U).Imm(0U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(40U, Opcode::Add).s32().Inputs(10U, 3U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(50U, Opcode::ReturnVoid);
        }
    }
    return graph;
}

/* One block, continuous chain */
TEST_F(AdjustRefsTest, OneBlockContinuousChain)
{
    auto *graph = BuildGraphOneBlockContinuousChain();

    Graph *graphEt = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    GRAPH(graphEt)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).u64();
        CONSTANT(3U, 10U).s32();

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(10U, Opcode::Phi).s32().Inputs(1U, 40U);
            INST(11U, Opcode::AddI).ptr().Inputs(0U).Imm(graph->GetRuntime()->GetArrayDataOffset(graph->GetArch()));
            INST(12U, Opcode::Load).u64().Inputs(11U, 1U);
            INST(13U, Opcode::Load).u64().Inputs(11U, 1U);
            INST(14U, Opcode::Store).u64().Inputs(11U, 1U, 2U);
            INST(15U, Opcode::Store).u64().Inputs(11U, 1U, 2U);
            INST(16U, Opcode::Compare).b().Inputs(10U, 1U);
            INST(19U, Opcode::IfImm).CC(CC_NE).Inputs(16U).Imm(0U);
        }

        BASIC_BLOCK(4U, 3U)
        {
            INST(40U, Opcode::Add).s32().Inputs(10U, 3U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(50U, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(graph->RunPass<AdjustRefs>());

    GraphChecker(graph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, graphEt));
}

Graph *AdjustRefsTest::BuildGraphOneBlockBrokenChain()
{
    auto graph = CreateEmptyFastpathGraph(RUNTIME_ARCH);
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).u64();
        CONSTANT(3U, 10U).s32();

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(10U, Opcode::Phi).s32().Inputs(1U, 40U);
            INST(11U, Opcode::LoadArray).u64().Inputs(0U, 1U);
            INST(12U, Opcode::LoadArray).u64().Inputs(0U, 1U);
            INST(13U, Opcode::SafePoint).NoVregs();
            INST(14U, Opcode::StoreArray).u64().Inputs(0U, 1U, 2U);
            INST(15U, Opcode::StoreArray).u64().Inputs(0U, 1U, 2U);
            INST(16U, Opcode::Compare).b().Inputs(10U, 1U);
            INST(19U, Opcode::IfImm).CC(CC_NE).Inputs(16U).Imm(0U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(40U, Opcode::Add).s32().Inputs(10U, 3U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(50U, Opcode::ReturnVoid);
        }
    }
    return graph;
}

/* One block, broken chain */
TEST_F(AdjustRefsTest, OneBlockBrokenChain)
{
    auto *graph = BuildGraphOneBlockBrokenChain();
    Graph *graphEt = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    GRAPH(graphEt)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).u64();
        CONSTANT(3U, 10U).s32();

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(10U, Opcode::Phi).s32().Inputs(1U, 40U);
            INST(11U, Opcode::AddI).ptr().Inputs(0U).Imm(graph->GetRuntime()->GetArrayDataOffset(graph->GetArch()));
            INST(12U, Opcode::Load).u64().Inputs(11U, 1U);
            INST(13U, Opcode::Load).u64().Inputs(11U, 1U);

            INST(14U, Opcode::SafePoint).NoVregs();

            INST(15U, Opcode::AddI).ptr().Inputs(0U).Imm(graph->GetRuntime()->GetArrayDataOffset(graph->GetArch()));
            INST(16U, Opcode::Store).u64().Inputs(15U, 1U, 2U);
            INST(17U, Opcode::Store).u64().Inputs(15U, 1U, 2U);

            INST(18U, Opcode::Compare).b().Inputs(10U, 1U);
            INST(19U, Opcode::IfImm).CC(CC_NE).Inputs(18U).Imm(0U);
        }

        BASIC_BLOCK(4U, 3U)
        {
            INST(40U, Opcode::Add).s32().Inputs(10U, 3U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(50U, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(graph->RunPass<AdjustRefs>());

    GraphChecker(graph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, graphEt));
}

Graph *AdjustRefsTest::BuildGraphMultipleBlockContinuousChain()
{
    auto graph = CreateEmptyFastpathGraph(RUNTIME_ARCH);
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).u64();
        CONSTANT(3U, 10U).s32();

        BASIC_BLOCK(3U, 4U, 10U)
        {
            INST(10U, Opcode::Phi).s32().Inputs(1U, 90U);
            INST(11U, Opcode::LoadArray).u64().Inputs(0U, 1U);
            INST(15U, Opcode::Compare).b().Inputs(10U, 1U);
            INST(19U, Opcode::IfImm).CC(CC_NE).Inputs(15U).Imm(0U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(41U, Opcode::Compare).b().Inputs(10U, 1U);
            INST(42U, Opcode::IfImm).CC(CC_NE).Inputs(41U).Imm(0U);
        }
        BASIC_BLOCK(5U, 9U)
        {
            INST(51U, Opcode::LoadArray).u64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(6U, 9U)
        {
            INST(61U, Opcode::LoadArray).u64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(9U, 3U)
        {
            INST(90U, Opcode::Add).s32().Inputs(10U, 3U);
        }
        BASIC_BLOCK(10U, 1U)
        {
            INST(100U, Opcode::ReturnVoid);
        }
    }
    return graph;
}

/* one head, the chain spans across multiple blocks, not broken */
TEST_F(AdjustRefsTest, MultipleBlockContinuousChain)
{
    auto *graph = BuildGraphMultipleBlockContinuousChain();

    Graph *graphEt = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    GRAPH(graphEt)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).u64();
        CONSTANT(3U, 10U).s32();
        BASIC_BLOCK(3U, 4U, 10U)
        {
            INST(10U, Opcode::Phi).s32().Inputs(1U, 90U);
            INST(11U, Opcode::AddI).ptr().Inputs(0U).Imm(graph->GetRuntime()->GetArrayDataOffset(graph->GetArch()));
            INST(12U, Opcode::Load).u64().Inputs(11U, 1U);
            INST(15U, Opcode::Compare).b().Inputs(10U, 1U);
            INST(19U, Opcode::IfImm).CC(CC_NE).Inputs(15U).Imm(0U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(41U, Opcode::Compare).b().Inputs(10U, 1U);
            INST(42U, Opcode::IfImm).CC(CC_NE).Inputs(41U).Imm(0U);
        }
        BASIC_BLOCK(5U, 9U)
        {
            INST(51U, Opcode::Load).u64().Inputs(11U, 1U);
        }
        BASIC_BLOCK(6U, 9U)
        {
            INST(61U, Opcode::Load).u64().Inputs(11U, 1U);
        }
        BASIC_BLOCK(9U, 3U)
        {
            INST(90U, Opcode::Add).s32().Inputs(10U, 3U);
        }
        BASIC_BLOCK(10U, 1U)
        {
            INST(100U, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(graph->RunPass<AdjustRefs>());

    GraphChecker(graph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, graphEt));
}

Graph *AdjustRefsTest::BuildGraphMultipleBlockBrokenChain()
{
    auto graph = CreateEmptyFastpathGraph(RUNTIME_ARCH);
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).u64();
        CONSTANT(3U, 10U).s32();

        BASIC_BLOCK(3U, 4U, 10U)
        {
            INST(10U, Opcode::Phi).s32().Inputs(1U, 90U);
            INST(11U, Opcode::LoadArray).u64().Inputs(0U, 1U);
            INST(15U, Opcode::Compare).b().Inputs(10U, 1U);
            INST(19U, Opcode::IfImm).CC(CC_NE).Inputs(15U).Imm(0U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(41U, Opcode::Compare).b().Inputs(10U, 1U);
            INST(42U, Opcode::IfImm).CC(CC_NE).Inputs(41U).Imm(0U);
        }
        BASIC_BLOCK(5U, 9U)
        {
            INST(51U, Opcode::LoadArray).u64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(6U, 9U)
        {
            INST(13U, Opcode::SafePoint).NoVregs();
            INST(61U, Opcode::LoadArray).u64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(9U, 3U)
        {
            INST(90U, Opcode::Add).s32().Inputs(10U, 3U);
        }
        BASIC_BLOCK(10U, 1U)
        {
            INST(100U, Opcode::ReturnVoid);
        }
    }
    return graph;
}

/* one head, the chain spans across multiple blocks,
 * broken in one of the dominated basic blocks */
TEST_F(AdjustRefsTest, MultipleBlockBrokenChain)
{
    auto *graph = BuildGraphMultipleBlockBrokenChain();

    Graph *graphEt = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    GRAPH(graphEt)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).u64();
        CONSTANT(3U, 10U).s32();
        BASIC_BLOCK(3U, 4U, 10U)
        {
            INST(10U, Opcode::Phi).s32().Inputs(1U, 90U);
            INST(11U, Opcode::AddI).ptr().Inputs(0U).Imm(graph->GetRuntime()->GetArrayDataOffset(graph->GetArch()));
            INST(12U, Opcode::Load).u64().Inputs(11U, 1U);
            INST(15U, Opcode::Compare).b().Inputs(10U, 1U);
            INST(19U, Opcode::IfImm).CC(CC_NE).Inputs(15U).Imm(0U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(41U, Opcode::Compare).b().Inputs(10U, 1U);
            INST(42U, Opcode::IfImm).CC(CC_NE).Inputs(41U).Imm(0U);
        }
        BASIC_BLOCK(5U, 9U)
        {
            INST(51U, Opcode::Load).u64().Inputs(11U, 1U);
        }
        BASIC_BLOCK(6U, 9U)
        {
            INST(13U, Opcode::SafePoint).NoVregs();
            INST(61U, Opcode::LoadArray).u64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(9U, 3U)
        {
            INST(90U, Opcode::Add).s32().Inputs(10U, 3U);
        }
        BASIC_BLOCK(10U, 1U)
        {
            INST(100U, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(graph->RunPass<AdjustRefs>());

    GraphChecker(graph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, graphEt));
}

Graph *AdjustRefsTest::BuildGraphProcessIndex(uint64_t offset1, uint64_t offset2, uint64_t offset3)
{
    auto graph = CreateEmptyFastpathGraph(RUNTIME_ARCH);
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        PARAMETER(2U, 2U).u64();
        PARAMETER(3U, 3U).u8();
        PARAMETER(4U, 4U).u32();
        BASIC_BLOCK(3U, 1U)
        {
            INST(6U, Opcode::AddI).s32().Inputs(1U).Imm(offset1);
            INST(7U, Opcode::StoreArray).u64().Inputs(0U, 6U, 2U);
            INST(8U, Opcode::SubI).s32().Inputs(1U).Imm(offset2);
            INST(9U, Opcode::StoreArray).u8().Inputs(0U, 8U, 3U);
            INST(10U, Opcode::SubI).s32().Inputs(1U).Imm(offset3);
            INST(11U, Opcode::StoreArray).u32().Inputs(0U, 10U, 4U);
            INST(100U, Opcode::ReturnVoid);
        }
    }
    return graph;
}

TEST_F(AdjustRefsTest, ProcessIndex)
{
    uint64_t offset1 = 10;
    uint64_t offset2 = 1;
    uint64_t offset3 = 20;
    auto *graph = BuildGraphProcessIndex(offset1, offset2, offset3);

    Graph *graphEt = CreateEmptyFastpathGraph(RUNTIME_ARCH);
    auto arrData = graph->GetRuntime()->GetArrayDataOffset(graph->GetArch());
    uint64_t newOffset1 = arrData + (offset1 << 3U);
    uint64_t newOffset2 = arrData - (offset2 << 0U);
    uint64_t newOffset3 = (offset3 << 2U) - arrData;
    GRAPH(graphEt)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        PARAMETER(2U, 2U).u64();
        PARAMETER(3U, 3U).u8();
        PARAMETER(4U, 4U).u32();
        BASIC_BLOCK(3U, 1U)
        {
            INST(56U, Opcode::AddI).ptr().Inputs(0U).Imm(newOffset1);
            INST(57U, Opcode::Store).u64().Inputs(56U, 1U, 2U);
            INST(58U, Opcode::AddI).ptr().Inputs(0U).Imm(newOffset2);
            INST(59U, Opcode::Store).u8().Inputs(58U, 1U, 3U);
            INST(60U, Opcode::SubI).ptr().Inputs(0U).Imm(newOffset3);
            INST(61U, Opcode::Store).u32().Inputs(60U, 1U, 4U);
            INST(100U, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(graph->RunPass<AdjustRefs>());
    ASSERT_TRUE(graph->RunPass<Cleanup>());

    GraphChecker(graph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, graphEt));
}

// BB 4 dominates BB 9, but there is SafePoint in BB 6 and reference cannot be adjusted
TEST_F(AdjustRefsTest, TriangleBrokenChain)
{
    auto graph = CreateEmptyFastpathGraph(RUNTIME_ARCH);
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).u64();
        CONSTANT(3U, 10U);

        BASIC_BLOCK(3U, 4U, 10U)
        {
            INST(10U, Opcode::Phi).s32().Inputs(1U, 90U);
            INST(11U, Opcode::LoadArray).u64().Inputs(0U, 1U);
            INST(15U, Opcode::Compare).b().Inputs(10U, 1U);
            INST(19U, Opcode::IfImm).CC(CC_NE).Inputs(15U).Imm(0U);
        }
        BASIC_BLOCK(4U, 9U, 6U)
        {
            INST(41U, Opcode::Compare).b().Inputs(10U, 1U);
            INST(42U, Opcode::IfImm).CC(CC_NE).Inputs(41U).Imm(0U);
        }
        BASIC_BLOCK(6U, 9U)
        {
            INST(13U, Opcode::SafePoint).NoVregs();
        }
        BASIC_BLOCK(9U, 3U)
        {
            INST(61U, Opcode::LoadArray).u64().Inputs(0U, 1U);
            INST(90U, Opcode::Add).s32().Inputs(10U, 3U);
        }
        BASIC_BLOCK(10U, 1U)
        {
            INST(100U, Opcode::ReturnVoid);
        }
    }

    auto clone = GraphCloner(graph, graph->GetAllocator(), graph->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(graph->RunPass<AdjustRefs>());
    GraphChecker(graph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, clone));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
