/*
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

#include "unit_test.h"
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/memory_coalescing.h"
#include "optimizer/optimizations/scheduler.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"

namespace ark::compiler {
class MemoryCoalescingTest : public GraphTest {
#ifndef NDEBUG
public:
    MemoryCoalescingTest()
    {
        // GraphChecker hack: LowLevel instructions may appear only after Lowering pass:
        GetGraph()->SetLowLevelInstructionsEnabled();
    }
#endif
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(MemoryCoalescingTest, ImmidiateLoads)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0x2aU).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 0U).TypeId(77U);
            INST(41U, Opcode::SaveState).Inputs(3U).SrcVregs({7U});
            INST(42U, Opcode::NullCheck).ref().Inputs(3U, 41U);
            INST(225U, Opcode::LoadArrayI).s64().Inputs(42U).Imm(0x0U);
            INST(227U, Opcode::LoadArrayI).s64().Inputs(42U).Imm(0x1U);

            INST(51U, Opcode::Add).s64().Inputs(225U, 227U);
            INST(229U, Opcode::StoreArrayI).s64().Inputs(42U, 51U).Imm(0x0U);
            INST(230U, Opcode::StoreArrayI).s64().Inputs(42U, 51U).Imm(0x1U);
            INST(40U, Opcode::Return).s64().Inputs(51U);
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        CONSTANT(0U, 0x2aU).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 0U).TypeId(77U);
            INST(41U, Opcode::SaveState).Inputs(3U).SrcVregs({7U});
            INST(42U, Opcode::NullCheck).ref().Inputs(3U, 41U);
            INST(231U, Opcode::LoadArrayPairI).s64().Inputs(42U).Imm(0x0U);
            INST(232U, Opcode::LoadPairPart).s64().Inputs(231U).Imm(0x0U);
            INST(233U, Opcode::LoadPairPart).s64().Inputs(231U).Imm(0x1U);

            INST(51U, Opcode::Add).s64().Inputs(232U, 233U);
            INST(234U, Opcode::StoreArrayPairI).s64().Inputs(42U, 51U, 51U).Imm(0x0U);
            INST(40U, Opcode::Return).s64().Inputs(51U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

SRC_GRAPH(LoopLoadCoalescing, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(5U, 0U).ref();
        CONSTANT(6U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(35U, Opcode::SaveState).Inputs(5U).SrcVregs({1U});
            INST(36U, Opcode::NullCheck).ref().Inputs(5U, 35U);
        }
        BASIC_BLOCK(3U, 3U, 7U)
        {
            INST(9U, Opcode::Phi).s32().Inputs({{2U, 6U}, {3U, 64U}});
            INST(10U, Opcode::Phi).s64().Inputs({{2U, 6U}, {3U, 28U}});
            INST(19U, Opcode::LoadArray).s64().Inputs(36U, 9U);
            INST(63U, Opcode::AddI).s32().Inputs(9U).Imm(0x1U);
            INST(26U, Opcode::LoadArray).s64().Inputs(36U, 63U);
            INST(27U, Opcode::Add).s64().Inputs(26U, 19U);
            INST(28U, Opcode::Add).s64().Inputs(27U, 10U);
            INST(64U, Opcode::AddI).s32().Inputs(9U).Imm(0x2U);
            INST(31U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_LT).Imm(0x4U).Inputs(64U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(33U, Opcode::Return).s32().Inputs(28U);
        }
    }
}

OUT_GRAPH(LoopLoadCoalescing, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(5U, 0U).ref();
        CONSTANT(6U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(35U, Opcode::SaveState).Inputs(5U).SrcVregs({1U});
            INST(36U, Opcode::NullCheck).ref().Inputs(5U, 35U);
        }
        BASIC_BLOCK(3U, 3U, 7U)
        {
            INST(9U, Opcode::Phi).s32().Inputs({{2U, 6U}, {3U, 64U}});
            INST(10U, Opcode::Phi).s64().Inputs({{2U, 6U}, {3U, 28U}});
            INST(65U, Opcode::LoadArrayPair).s64().Inputs(36U, 9U);
            INST(66U, Opcode::LoadPairPart).s64().Inputs(65U).Imm(0x0U);
            INST(67U, Opcode::LoadPairPart).s64().Inputs(65U).Imm(0x1U);
            INST(63U, Opcode::AddI).s32().Inputs(9U).Imm(0x1U);
            INST(27U, Opcode::Add).s64().Inputs(67U, 66U);
            INST(28U, Opcode::Add).s64().Inputs(27U, 10U);
            INST(64U, Opcode::AddI).s32().Inputs(9U).Imm(0x2U);
            INST(31U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_LT).Imm(0x4U).Inputs(64U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(33U, Opcode::Return).s32().Inputs(28U);
        }
    }
}

TEST_F(MemoryCoalescingTest, LoopLoadCoalescing)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    src_graph::LoopLoadCoalescing::CREATE(GetGraph());
    Graph *optGraph = CreateEmptyGraph();
    out_graph::LoopLoadCoalescing::CREATE(optGraph);
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

SRC_GRAPH(LoopStoreCoalescing, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(4U, 0x0U).s64();
        CONSTANT(5U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::LenArray).s32().Inputs(0U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(6U, Opcode::Phi).s32().Inputs({{2U, 4U}, {3U, 34U}});
            INST(7U, Opcode::Phi).s32().Inputs({{2U, 5U}, {3U, 24U}});
            INST(8U, Opcode::Phi).s32().Inputs({{2U, 5U}, {3U, 25U}});
            INST(17U, Opcode::StoreArray).s32().Inputs(0U, 6U, 7U);
            INST(33U, Opcode::AddI).s32().Inputs(6U).Imm(0x1U);
            INST(23U, Opcode::StoreArray).s32().Inputs(0U, 33U, 8U);
            INST(24U, Opcode::Add).s32().Inputs(7U, 8U);
            INST(25U, Opcode::Add).s32().Inputs(8U, 24U);
            INST(34U, Opcode::AddI).s32().Inputs(6U).Imm(0x2U);
            INST(35U, Opcode::If).SrcType(DataType::INT32).CC(CC_LT).Inputs(34U, 3U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(29U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(LoopStoreCoalescing, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(4U, 0x0U).s64();
        CONSTANT(5U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::LenArray).s32().Inputs(0U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(6U, Opcode::Phi).s32().Inputs({{2U, 4U}, {3U, 34U}});
            INST(7U, Opcode::Phi).s32().Inputs({{2U, 5U}, {3U, 24U}});
            INST(8U, Opcode::Phi).s32().Inputs({{2U, 5U}, {3U, 25U}});
            INST(33U, Opcode::AddI).s32().Inputs(6U).Imm(0x1U);
            INST(36U, Opcode::StoreArrayPair).s32().Inputs(0U, 6U, 7U, 8U);
            INST(24U, Opcode::Add).s32().Inputs(7U, 8U);
            INST(25U, Opcode::Add).s32().Inputs(8U, 24U);
            INST(34U, Opcode::AddI).s32().Inputs(6U).Imm(0x2U);
            INST(35U, Opcode::If).SrcType(DataType::INT32).CC(CC_LT).Inputs(34U, 3U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(29U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(MemoryCoalescingTest, LoopStoreCoalescing)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    src_graph::LoopStoreCoalescing::CREATE(GetGraph());
    Graph *optGraph = CreateEmptyGraph();
    out_graph::LoopStoreCoalescing::CREATE(optGraph);
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, AliasedAccess)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(26U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x0U);
            INST(28U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);
            INST(21U, Opcode::Add).s64().Inputs(28U, 26U);
            INST(22U, Opcode::Return).s64().Inputs(21U);
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(29U, Opcode::LoadArrayPairI).s64().Inputs(0U).Imm(0x0U);
            INST(30U, Opcode::LoadPairPart).s64().Inputs(29U).Imm(0x0U);
            INST(31U, Opcode::LoadPairPart).s64().Inputs(29U).Imm(0x1U);
            INST(21U, Opcode::Add).s64().Inputs(31U, 30U);
            INST(22U, Opcode::Return).s64().Inputs(21U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, OnlySingleCoalescing)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(26U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x0U);
            INST(28U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);

            INST(30U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);
            INST(21U, Opcode::Add).s64().Inputs(28U, 26U);
            INST(22U, Opcode::Add).s64().Inputs(30U, 21U);
            INST(23U, Opcode::Return).s64().Inputs(22U);
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(31U, Opcode::LoadArrayPairI).s64().Inputs(0U).Imm(0x0U);
            INST(33U, Opcode::LoadPairPart).s64().Inputs(31U).Imm(0x0U);
            INST(32U, Opcode::LoadPairPart).s64().Inputs(31U).Imm(0x1U);

            INST(30U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);
            INST(21U, Opcode::Add).s64().Inputs(32U, 33U);
            INST(22U, Opcode::Add).s64().Inputs(30U, 21U);
            INST(23U, Opcode::Return).s64().Inputs(22U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, PseudoParts)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x0U);
            INST(2U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);

            INST(3U, Opcode::StoreArrayI).s64().Inputs(0U, 2U).Imm(0x0U);
            INST(4U, Opcode::StoreArrayI).s64().Inputs(0U, 1U).Imm(0x1U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::LoadArrayPairI).s64().Inputs(0U).Imm(0x0U);
            INST(7U, Opcode::LoadPairPart).s64().Inputs(6U).Imm(0x0U);
            INST(8U, Opcode::LoadPairPart).s64().Inputs(6U).Imm(0x1U);

            INST(9U, Opcode::StoreArrayPairI).s64().Inputs(0U, 8U, 7U).Imm(0x0U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GetGraph()->RunPass<Scheduler>();
    RegAlloc(GetGraph());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

SRC_GRAPH(UnalignedStores, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        PARAMETER(3U, 3U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::AddI).s32().Inputs(1U).Imm(0x4U);
            INST(5U, Opcode::StoreArray).s32().Inputs(0U, 4U, 2U);
            INST(6U, Opcode::AddI).s32().Inputs(1U).Imm(0x5U);
            INST(7U, Opcode::StoreArray).s32().Inputs(0U, 6U, 3U);

            INST(8U, Opcode::StoreArray).s32().Inputs(0U, 4U, 3U);
            INST(9U, Opcode::AddI).s32().Inputs(4U).Imm(0x1U);
            INST(10U, Opcode::StoreArray).s32().Inputs(0U, 9U, 2U);
            INST(11U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(UnalignedStores, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        PARAMETER(3U, 3U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::AddI).s32().Inputs(1U).Imm(0x4U);
            INST(6U, Opcode::AddI).s32().Inputs(1U).Imm(0x5U);
            INST(12U, Opcode::StoreArrayPair).s32().Inputs(0U, 1U, 2U, 3U).Imm(0x4U);

            INST(9U, Opcode::AddI).s32().Inputs(4U).Imm(0x1U);
            INST(13U, Opcode::StoreArrayPair).s32().Inputs(0U, 1U, 3U, 2U).Imm(0x4U);
            INST(11U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(MemoryCoalescingTest, UnalignedStores)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    src_graph::UnalignedStores::CREATE(GetGraph());
    Graph *optGraph = CreateEmptyGraph();
    out_graph::UnalignedStores::CREATE(optGraph);
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>(true));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));

    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, NoAlignmentTestI)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x2U);
            INST(2U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);
            INST(3U, Opcode::Add).s64().Inputs(2U, 1U);
            INST(4U, Opcode::Return).s64().Inputs(3U);
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::LoadArrayPairI).s64().Inputs(0U).Imm(0x1U);
            INST(6U, Opcode::LoadPairPart).s64().Inputs(5U).Imm(0x0U);
            INST(7U, Opcode::LoadPairPart).s64().Inputs(5U).Imm(0x1U);
            INST(3U, Opcode::Add).s64().Inputs(6U, 7U);
            INST(4U, Opcode::Return).s64().Inputs(3U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>(true));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));

    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, StoresRoundedByLoads)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x2U);
            INST(4U, Opcode::StoreArrayI).s64().Inputs(0U, 1U).Imm(0x2U);
            INST(5U, Opcode::StoreArrayI).s64().Inputs(0U, 2U).Imm(0x3U);
            INST(6U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x3U);
            INST(7U, Opcode::Add).s64().Inputs(3U, 6U);
            INST(8U, Opcode::Return).s64().Inputs(7U);
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x2U);
            INST(9U, Opcode::StoreArrayPairI).s64().Inputs(0U, 1U, 2U).Imm(0x2U);
            INST(6U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x3U);
            INST(7U, Opcode::Add).s64().Inputs(3U, 6U);
            INST(8U, Opcode::Return).s64().Inputs(7U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, LoadsRoundedByStores)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::StoreArrayI).s64().Inputs(0U, 1U).Imm(0x3U);
            INST(3U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x2U);
            INST(6U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x3U);
            INST(5U, Opcode::StoreArrayI).s64().Inputs(0U, 2U).Imm(0x2U);
            INST(7U, Opcode::Add).s64().Inputs(3U, 6U);
            INST(8U, Opcode::Return).s64().Inputs(7U);
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::StoreArrayI).s64().Inputs(0U, 1U).Imm(0x3U);
            INST(9U, Opcode::LoadArrayPairI).s64().Inputs(0U).Imm(0x2U);
            INST(10U, Opcode::LoadPairPart).s64().Inputs(9U).Imm(0x0U);
            INST(11U, Opcode::LoadPairPart).s64().Inputs(9U).Imm(0x1U);
            INST(5U, Opcode::StoreArrayI).s64().Inputs(0U, 2U).Imm(0x2U);
            INST(7U, Opcode::Add).s64().Inputs(10U, 11U);
            INST(8U, Opcode::Return).s64().Inputs(7U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

SRC_GRAPH(UnalignedInLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        CONSTANT(3U, 0xaU).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(6U, Opcode::NewArray).ref().Inputs(44U, 3U).TypeId(77U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(7U, Opcode::Phi).s32().Inputs({{2U, 0U}, {3U, 33U}});
            INST(19U, Opcode::StoreArray).s32().Inputs(6U, 7U, 7U);
            INST(33U, Opcode::AddI).s32().Inputs(7U).Imm(0x1U);
            INST(25U, Opcode::StoreArray).s32().Inputs(6U, 33U, 7U);
            INST(34U, Opcode::If).SrcType(DataType::INT32).CC(CC_GT).Inputs(1U, 33U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(29U, Opcode::Return).s32().Inputs(33U);
        }
    }
}

OUT_GRAPH(UnalignedInLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        CONSTANT(3U, 0xaU).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(6U, Opcode::NewArray).ref().Inputs(44U, 3U).TypeId(77U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(7U, Opcode::Phi).s32().Inputs({{2U, 0U}, {3U, 33U}});
            INST(33U, Opcode::AddI).s32().Inputs(7U).Imm(0x1U);
            INST(35U, Opcode::StoreArrayPair).s32().Inputs(6U, 7U, 7U, 7U);
            INST(34U, Opcode::If).SrcType(DataType::INT32).CC(CC_GT).Inputs(1U, 33U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(29U, Opcode::Return).s32().Inputs(33U);
        }
    }
}

TEST_F(MemoryCoalescingTest, UnalignedInLoop)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    src_graph::UnalignedInLoop::CREATE(GetGraph());
    Graph *optGraph = CreateEmptyGraph();
    out_graph::UnalignedInLoop::CREATE(optGraph);
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>(true));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));

    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, IndexInference)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::AddI).s32().Inputs(1U).Imm(0x1U);
            INST(7U, Opcode::LoadArray).s32().Inputs(0U, 1U);
            INST(8U, Opcode::LoadArray).s32().Inputs(0U, 6U);
            INST(9U, Opcode::StoreArray).s32().Inputs(0U, 1U, 7U);
            INST(10U, Opcode::StoreArray).s32().Inputs(0U, 6U, 8U);
            INST(11U, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::AddI).s32().Inputs(1U).Imm(0x1U);
            INST(12U, Opcode::LoadArrayPair).s32().Inputs(0U, 1U);
            INST(14U, Opcode::LoadPairPart).s32().Inputs(12U).Imm(0x0U);
            INST(13U, Opcode::LoadPairPart).s32().Inputs(12U).Imm(0x1U);
            INST(15U, Opcode::StoreArrayPair).s32().Inputs(0U, 1U, 14U, 13U);
            INST(11U, Opcode::ReturnVoid).v0id();
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>(true));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));

    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, SimplePlaceFinding)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::LoadArray).s32().Inputs(0U, 1U);
            INST(6U, Opcode::SubI).s32().Inputs(1U).Imm(1U);
            INST(8U, Opcode::LoadArray).s32().Inputs(0U, 6U);
            INST(9U, Opcode::Add).s32().Inputs(7U, 8U);
            INST(11U, Opcode::Return).s32().Inputs(9U);
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::SubI).s32().Inputs(1U).Imm(1U);
            INST(12U, Opcode::LoadArrayPair).s32().Inputs(0U, 6U);
            INST(14U, Opcode::LoadPairPart).s32().Inputs(12U).Imm(0x0U);
            INST(13U, Opcode::LoadPairPart).s32().Inputs(12U).Imm(0x1U);
            INST(9U, Opcode::Add).s32().Inputs(13U, 14U);
            INST(11U, Opcode::Return).s32().Inputs(9U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, ObjectAccessesCoalescing)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0x2U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(50U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(50U).TypeId(68U);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 1U, 50U).TypeId(88U);
            INST(38U, Opcode::LoadArrayI).ref().Inputs(0U).Imm(0x0U);
            INST(40U, Opcode::LoadArrayI).ref().Inputs(0U).Imm(0x1U);
            INST(41U, Opcode::StoreArrayI).ref().Inputs(4U, 38U).Imm(0x0U);
            INST(42U, Opcode::StoreArrayI).ref().Inputs(4U, 40U).Imm(0x1U);
            INST(27U, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0x2U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(50U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(444U, Opcode::LoadAndInitClass).ref().Inputs(50U).TypeId(68U);
            INST(4U, Opcode::NewArray).ref().Inputs(444U, 1U, 50U).TypeId(88U);
            INST(43U, Opcode::LoadArrayPairI).ref().Inputs(0U).Imm(0x0U);
            INST(45U, Opcode::LoadPairPart).ref().Inputs(43U).Imm(0x0U);
            INST(44U, Opcode::LoadPairPart).ref().Inputs(43U).Imm(0x1U);
            INST(46U, Opcode::StoreArrayPairI).ref().Inputs(4U, 45U, 44U).Imm(0x0U);
            INST(27U, Opcode::ReturnVoid).v0id();
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    g_options.SetCompilerMemoryCoalescingObjects(false);
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>(true));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));

    g_options.SetCompilerMemoryCoalescingObjects(true);
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>(true));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

SRC_GRAPH(AllowedVolatileReordering, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0x2aU).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).SrcVregs({});
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U).TypeId(0U);

            INST(3U, Opcode::NewArray).ref().Inputs(5U, 0U).TypeId(77U);
            // Can reorder Volatile Store (v226) and Normal Load (v227)
            INST(225U, Opcode::LoadArrayI).s64().Inputs(3U).Imm(0x0U);
            INST(226U, Opcode::StoreStatic).s64().Volatile().Inputs(5U, 225U).TypeId(103U);
            INST(227U, Opcode::LoadArrayI).s64().Inputs(3U).Imm(0x1U);

            INST(51U, Opcode::Add).s64().Inputs(225U, 227U);
            // Can reorder Normal Store (v229) and Volatile Load (v230)
            INST(229U, Opcode::StoreArrayI).s64().Inputs(3U, 51U).Imm(0x0U);
            INST(230U, Opcode::LoadStatic).s64().Inputs(5U).Volatile().TypeId(107U);
            INST(231U, Opcode::StoreArrayI).s64().Inputs(3U, 230U).Imm(0x1U);
            INST(40U, Opcode::Return).s64().Inputs(51U);
        }
    }
}

OUT_GRAPH(AllowedVolatileReordering, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0x2aU).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).SrcVregs({});
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U).TypeId(0U);

            INST(3U, Opcode::NewArray).ref().Inputs(5U, 0U).TypeId(77U);
            INST(232U, Opcode::LoadArrayPairI).s64().Inputs(3U).Imm(0x0U);
            INST(234U, Opcode::LoadPairPart).s64().Inputs(232U).Imm(0x0U);
            INST(233U, Opcode::LoadPairPart).s64().Inputs(232U).Imm(0x1U);
            INST(226U, Opcode::StoreStatic).s64().Volatile().Inputs(5U, 234U).TypeId(103U);

            INST(51U, Opcode::Add).s64().Inputs(234U, 233U);
            INST(230U, Opcode::LoadStatic).s64().Inputs(5U).Volatile().TypeId(107U);
            INST(235U, Opcode::StoreArrayPairI).s64().Inputs(3U, 51U, 230U).Imm(0x0U);
            INST(40U, Opcode::Return).s64().Inputs(51U);
        }
    }
}

TEST_F(MemoryCoalescingTest, AllowedVolatileReordering)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    src_graph::AllowedVolatileReordering::CREATE(GetGraph());
    Graph *optGraph = CreateEmptyGraph();
    out_graph::AllowedVolatileReordering::CREATE(optGraph);
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

SRC_GRAPH(AllowedVolatileReordering2, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0x2aU).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).SrcVregs({});
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U).TypeId(0U);

            INST(3U, Opcode::NewArray).ref().Inputs(5U, 0U).TypeId(77U);
            // We can reorder v225 and v226 but not v226 and v227
            INST(225U, Opcode::LoadArrayI).s64().Inputs(3U).Imm(0x0U);
            INST(226U, Opcode::LoadStatic).s64().Inputs(5U).Volatile().TypeId(103U);
            INST(227U, Opcode::LoadArrayI).s64().Inputs(3U).Imm(0x1U);

            INST(51U, Opcode::Add).s64().Inputs(225U, 227U);
            // We can reorder v230 and v231 but not v229 and v230
            INST(229U, Opcode::StoreArrayI).s64().Inputs(3U, 51U).Imm(0x0U);
            INST(230U, Opcode::StoreStatic).s64().Inputs(5U).Volatile().Inputs(226U).TypeId(105U);
            INST(231U, Opcode::StoreArrayI).s64().Inputs(3U, 51U).Imm(0x1U);
            INST(40U, Opcode::Return).s64().Inputs(51U);
        }
    }
}

OUT_GRAPH(AllowedVolatileReordering2, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0x2aU).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).SrcVregs({});
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U).TypeId(0U);

            INST(3U, Opcode::NewArray).ref().Inputs(5U, 0U).TypeId(77U);
            INST(226U, Opcode::LoadStatic).s64().Inputs(5U).Volatile().TypeId(103U);
            INST(235U, Opcode::LoadArrayPairI).s64().Inputs(3U).Imm(0x0U);
            INST(237U, Opcode::LoadPairPart).s64().Inputs(235U).Imm(0x0U);
            INST(236U, Opcode::LoadPairPart).s64().Inputs(235U).Imm(0x1U);

            INST(51U, Opcode::Add).s64().Inputs(237U, 236U);
            INST(238U, Opcode::StoreArrayPairI).s64().Inputs(3U, 51U, 51U).Imm(0x0U);
            INST(230U, Opcode::StoreStatic).s64().Inputs(5U).Volatile().Inputs(226U).TypeId(105U);
            INST(40U, Opcode::Return).s64().Inputs(51U);
        }
    }
}

TEST_F(MemoryCoalescingTest, AllowedVolatileReordering2)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    src_graph::AllowedVolatileReordering2::CREATE(GetGraph());
    Graph *optGraph = CreateEmptyGraph();
    out_graph::AllowedVolatileReordering2::CREATE(optGraph);
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

SRC_GRAPH(UnrolledLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).ref();
        CONSTANT(3U, 0x2aU).s64();
        CONSTANT(4U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(50U, Opcode::SaveState).Inputs(1U).SrcVregs({0U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(50U).TypeId(68U);
            INST(2U, Opcode::NewArray).ref().Inputs(44U, 3U, 50U).TypeId(77U);
            INST(10U, Opcode::IfImm).SrcType(DataType::INT64).CC(CC_LE).Imm(0x0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(11U, Opcode::Phi).s32().Inputs({{2U, 4U}, {3U, 17U}});
            INST(12U, Opcode::LoadArray).s32().Inputs(1U, 11U);
            INST(13U, Opcode::StoreArray).s32().Inputs(2U, 11U, 12U);
            INST(14U, Opcode::AddI).s32().Inputs(11U).Imm(1U);

            INST(15U, Opcode::LoadArray).s32().Inputs(1U, 14U);
            INST(16U, Opcode::StoreArray).s32().Inputs(2U, 14U, 15U);
            INST(17U, Opcode::AddI).s32().Inputs(11U).Imm(2U);

            INST(30U, Opcode::If).SrcType(DataType::INT32).CC(CC_GE).Inputs(17U, 3U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(40U, Opcode::ReturnVoid);
        }
    }
}

OUT_GRAPH(UnrolledLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).ref();
        CONSTANT(3U, 0x2aU).s64();
        CONSTANT(4U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(50U, Opcode::SaveState).Inputs(1U).SrcVregs({0U});
            INST(444U, Opcode::LoadAndInitClass).ref().Inputs(50U).TypeId(68U);
            INST(2U, Opcode::NewArray).ref().Inputs(444U, 3U, 50U).TypeId(77U);
            INST(10U, Opcode::IfImm).SrcType(DataType::INT64).CC(CC_LE).Imm(0x0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(11U, Opcode::Phi).s32().Inputs({{2U, 4U}, {3U, 17U}});
            INST(44U, Opcode::LoadArrayPair).s32().Inputs(1U, 11U);
            INST(46U, Opcode::LoadPairPart).s32().Inputs(44U).Imm(0x0U);
            INST(45U, Opcode::LoadPairPart).s32().Inputs(44U).Imm(0x1U);
            INST(14U, Opcode::AddI).s32().Inputs(11U).Imm(1U);

            INST(47U, Opcode::StoreArrayPair).s32().Inputs(2U, 11U, 46U, 45U);
            INST(17U, Opcode::AddI).s32().Inputs(11U).Imm(2U);

            INST(30U, Opcode::If).SrcType(DataType::INT32).CC(CC_GE).Inputs(17U, 3U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(40U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(MemoryCoalescingTest, UnrolledLoop)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    src_graph::UnrolledLoop::CREATE(GetGraph());
    Graph *optGraph = CreateEmptyGraph();
    out_graph::UnrolledLoop::CREATE(optGraph);
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

SRC_GRAPH(CoalescingOverSaveState, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({6U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);

            INST(41U, Opcode::SaveState).Inputs(0U).SrcVregs({6U});
            INST(43U, Opcode::LenArray).s32().Inputs(2U);
            INST(241U, Opcode::BoundsCheckI).s32().Inputs(43U, 41U).Imm(0x0U);
            INST(242U, Opcode::LoadArrayI).s64().Inputs(2U).Imm(0x0U);

            INST(47U, Opcode::SaveState).Inputs(242U, 0U).SrcVregs({3U, 6U});
            INST(49U, Opcode::LenArray).s32().Inputs(2U);
            INST(244U, Opcode::BoundsCheckI).s32().Inputs(49U, 47U).Imm(0x1U);
            INST(245U, Opcode::LoadArrayI).s64().Inputs(2U).Imm(0x1U);

            INST(53U, Opcode::Add).s64().Inputs(242U, 245U);
            INST(40U, Opcode::Return).s64().Inputs(53U);
        }
    }
}

OUT_GRAPH(CoalescingOverSaveState, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({6U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);

            INST(41U, Opcode::SaveState).Inputs(0U).SrcVregs({6U});
            INST(43U, Opcode::LenArray).s32().Inputs(2U);
            INST(241U, Opcode::BoundsCheckI).s32().Inputs(43U, 41U).Imm(0x0U);

            INST(246U, Opcode::LoadArrayPairI).s64().Inputs(2U).Imm(0x0U);
            INST(248U, Opcode::LoadPairPart).s64().Inputs(246U).Imm(0x0U);
            INST(247U, Opcode::LoadPairPart).s64().Inputs(246U).Imm(0x1U);

            INST(47U, Opcode::SaveState).Inputs(248U, 0U).SrcVregs({3U, 6U});
            INST(49U, Opcode::LenArray).s32().Inputs(2U);
            INST(244U, Opcode::BoundsCheckI).s32().Inputs(49U, 47U).Imm(0x1U);

            INST(53U, Opcode::Add).s64().Inputs(248U, 247U);
            INST(40U, Opcode::Return).s64().Inputs(53U);
        }
    }
}

TEST_F(MemoryCoalescingTest, CoalescingOverSaveState)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    src_graph::CoalescingOverSaveState::CREATE(GetGraph());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();

    Graph *optGraph = CreateEmptyGraph();
    out_graph::CoalescingOverSaveState::CREATE(optGraph);
    GraphChecker(optGraph).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, AlignmentTest)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(26U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x2U);
            INST(28U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);
            INST(21U, Opcode::Add).s64().Inputs(28U, 26U);
            INST(22U, Opcode::Return).s64().Inputs(21U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(MemoryCoalescingTest, AliasedStore)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(26U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x0U);
            INST(13U, Opcode::StoreArray).s64().Inputs(0U, 1U, 2U);
            INST(28U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x5U);
            INST(29U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);
            INST(21U, Opcode::Add).s64().Inputs(28U, 26U);
            INST(24U, Opcode::Add).s64().Inputs(21U, 29U);
            INST(22U, Opcode::Return).s64().Inputs(24U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(MemoryCoalescingTest, TypeCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::AddI).s32().Inputs(1U).Imm(0x1U);
            INST(7U, Opcode::LoadArray).s8().Inputs(0U, 1U);
            INST(8U, Opcode::LoadArray).s8().Inputs(0U, 6U);
            INST(9U, Opcode::LoadArrayI).s16().Inputs(0U).Imm(0x4U);
            INST(10U, Opcode::LoadArrayI).s16().Inputs(0U).Imm(0x5U);
            INST(11U, Opcode::StoreArray).s16().Inputs(0U, 1U, 9U);
            INST(12U, Opcode::StoreArray).s16().Inputs(0U, 6U, 10U);
            INST(13U, Opcode::StoreArrayI).s8().Inputs(0U, 7U).Imm(0x4U);
            INST(14U, Opcode::StoreArrayI).s8().Inputs(0U, 8U).Imm(0x5U);
            INST(15U, Opcode::ReturnVoid).v0id();
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(MemoryCoalescingTest, ProhibitedVolatileReordering)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0x2aU).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).SrcVregs({});
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U).TypeId(0U);

            INST(3U, Opcode::NewArray).ref().Inputs(5U, 0U).TypeId(77U);
            INST(225U, Opcode::LoadArrayI).s64().Inputs(3U).Imm(0x0U);
            // v50 is needed to prevent reordering v225 with v226
            INST(50U, Opcode::Add).s64().Inputs(225U, 225U);
            INST(226U, Opcode::LoadStatic).s64().Inputs(5U).Volatile().TypeId(103U);
            INST(227U, Opcode::LoadArrayI).s64().Inputs(3U).Imm(0x1U);

            INST(51U, Opcode::Add).s64().Inputs(50U, 227U);
            INST(229U, Opcode::StoreArrayI).s64().Inputs(3U, 51U).Imm(0x0U);
            INST(230U, Opcode::StoreStatic).s64().Inputs(5U).Volatile().Inputs(226U).TypeId(105U);
            // v51 is needed to prevent reordering v231 and v230
            INST(52U, Opcode::Add).s64().Inputs(50U, 51U);
            INST(231U, Opcode::StoreArrayI).s64().Inputs(3U, 52U).Imm(0x1U);
            INST(40U, Opcode::Return).s64().Inputs(51U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(MemoryCoalescingTest, LoweringDominance)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::SubI).s32().Inputs(1U).Imm(1U);

            INST(9U, Opcode::LoadArray).s32().Inputs(0U, 1U);
            INST(10U, Opcode::ShlI).s32().Inputs(9U).Imm(24U);
            INST(16U, Opcode::StoreArray).s32().Inputs(0U, 8U, 10U);
            INST(11U, Opcode::AShrI).s32().Inputs(9U).Imm(28U);

            INST(12U, Opcode::AddI).s64().Inputs(1U).Imm(1U);
            INST(13U, Opcode::LoadArray).s32().Inputs(0U, 12U);

            INST(14U, Opcode::Add).s32().Inputs(10U, 11U);
            INST(15U, Opcode::Return).s32().Inputs(14U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(MemoryCoalescingTest, LoweringDominance2)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).ref();
        PARAMETER(3U, 3U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::SubI).s32().Inputs(3U).Imm(1U);

            INST(9U, Opcode::LoadArrayI).s32().Inputs(0U).Imm(0x0U);
            INST(10U, Opcode::StoreArray).s32().Inputs(1U, 8U, 9U);

            INST(11U, Opcode::LoadArrayI).s32().Inputs(0U).Imm(0x1U);
            INST(12U, Opcode::StoreArray).s32().Inputs(2U, 8U, 11U);

            INST(15U, Opcode::Return).s32().Inputs(3U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(MemoryCoalescingTest, CoalescingLoadsOverSaveState)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(9U, Opcode::LoadArray).ref().Inputs(0U, 1U);
            INST(10U, Opcode::SaveState).SrcVregs({9U});
            INST(11U, Opcode::AddI).s64().Inputs(1U).Imm(1U);
            INST(12U, Opcode::LoadArray).ref().Inputs(0U, 11U);
            INST(13U, Opcode::SaveState).SrcVregs({9U, 12U});
            INST(15U, Opcode::Return).s32().Inputs(1U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

TEST_F(MemoryCoalescingTest, CoalescingPhiAsUser)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s32();
        PARAMETER(3U, 3U).s32();
        BASIC_BLOCK(2U, 3U, 2U)
        {
            INST(9U, Opcode::Phi).s32().Inputs({{0U, 3U}, {2U, 10U}});
            INST(10U, Opcode::LoadArray).s32().Inputs(0U, 9U);
            INST(11U, Opcode::AddI).s32().Inputs(9U).Imm(1U);
            INST(12U, Opcode::LoadArray).s32().Inputs(0U, 11U);
            INST(18U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0U).Inputs(12U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(20U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>(false));
    GraphChecker(GetGraph()).Check();
}

TEST_F(MemoryCoalescingTest, NestedLoadCoalescing)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(26U, Opcode::LoadArrayI).ref().Inputs(0U).Imm(0x0U);
            INST(28U, Opcode::LoadArrayI).ref().Inputs(0U).Imm(0x1U);

            INST(30U, Opcode::LoadArrayI).s64().Inputs(26U).Imm(0x0U);
            INST(31U, Opcode::LoadArrayI).s64().Inputs(26U).Imm(0x1U);
            INST(32U, Opcode::LoadArrayI).s64().Inputs(28U).Imm(0x0U);

            INST(21U, Opcode::Add).s64().Inputs(30U, 31U);
            INST(22U, Opcode::Add).s64().Inputs(21U, 32U);
            INST(23U, Opcode::Return).s64().Inputs(22U);
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(31U, Opcode::LoadArrayPairI).ref().Inputs(0U).Imm(0x0U);
            INST(32U, Opcode::LoadPairPart).ref().Inputs(31U).Imm(0x0U);
            INST(33U, Opcode::LoadPairPart).ref().Inputs(31U).Imm(0x1U);

            INST(34U, Opcode::LoadArrayPairI).s64().Inputs(32U).Imm(0x0U);
            INST(35U, Opcode::LoadPairPart).s64().Inputs(34U).Imm(0x0U);
            INST(36U, Opcode::LoadPairPart).s64().Inputs(34U).Imm(0x1U);
            INST(30U, Opcode::LoadArrayI).s64().Inputs(33U).Imm(0x0U);

            INST(21U, Opcode::Add).s64().Inputs(35U, 36U);
            INST(22U, Opcode::Add).s64().Inputs(21U, 30U);
            INST(23U, Opcode::Return).s64().Inputs(22U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
}

TEST_F(MemoryCoalescingTest, OnlySingleCoalescingOverSafePoint)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(26U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x0U);
            INST(27U, Opcode::SafePoint).NoVregs();
            INST(28U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);

            INST(30U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);
            INST(21U, Opcode::Add).s64().Inputs(28U, 26U);
            INST(22U, Opcode::Add).s64().Inputs(30U, 21U);
            INST(23U, Opcode::Return).s64().Inputs(22U);
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(31U, Opcode::LoadArrayPairI).s64().Inputs(0U).Imm(0x0U);
            INST(33U, Opcode::LoadPairPart).s64().Inputs(31U).Imm(0x0U);
            INST(32U, Opcode::LoadPairPart).s64().Inputs(31U).Imm(0x1U);
            INST(27U, Opcode::SafePoint).Inputs(0U).SrcVregs({VirtualRegister::BRIDGE});

            INST(30U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);
            INST(21U, Opcode::Add).s64().Inputs(32U, 33U);
            INST(22U, Opcode::Add).s64().Inputs(30U, 21U);
            INST(23U, Opcode::Return).s64().Inputs(22U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, PseudoPartsOverSafePoints)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x0U);
            INST(27U, Opcode::SafePoint).NoVregs();
            INST(2U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);

            INST(3U, Opcode::StoreArrayI).s64().Inputs(0U, 2U).Imm(0x0U);
            INST(28U, Opcode::SafePoint).NoVregs();
            INST(4U, Opcode::StoreArrayI).s64().Inputs(0U, 1U).Imm(0x1U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::LoadArrayPairI).s64().Inputs(0U).Imm(0x0U);
            INST(7U, Opcode::LoadPairPart).s64().Inputs(6U).Imm(0x0U);
            INST(8U, Opcode::LoadPairPart).s64().Inputs(6U).Imm(0x1U);
            INST(27U, Opcode::SafePoint).Inputs(0U).SrcVregs({VirtualRegister::BRIDGE});

            INST(9U, Opcode::StoreArrayPairI).s64().Inputs(0U, 8U, 7U).Imm(0x0U);
            INST(28U, Opcode::SafePoint).NoVregs();
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, OnlySingleCoalescingOverSaveState)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(26U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x0U);
            INST(2U, Opcode::SaveState).NoVregs();
            INST(28U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);

            INST(30U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);
            INST(21U, Opcode::Add).s64().Inputs(28U, 26U);
            INST(22U, Opcode::Add).s64().Inputs(30U, 21U);
            INST(23U, Opcode::Return).s64().Inputs(22U);
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(31U, Opcode::LoadArrayPairI).s64().Inputs(0U).Imm(0x0U);
            INST(33U, Opcode::LoadPairPart).s64().Inputs(31U).Imm(0x0U);
            INST(32U, Opcode::LoadPairPart).s64().Inputs(31U).Imm(0x1U);
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({VirtualRegister::BRIDGE});

            INST(30U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);
            INST(21U, Opcode::Add).s64().Inputs(32U, 33U);
            INST(22U, Opcode::Add).s64().Inputs(30U, 21U);
            INST(23U, Opcode::Return).s64().Inputs(22U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, OverSaveState)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x0U);
            INST(11U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);

            INST(3U, Opcode::StoreArrayI).s64().Inputs(0U, 2U).Imm(0x0U);
            INST(12U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::StoreArrayI).s64().Inputs(0U, 1U).Imm(0x1U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::LoadArrayPairI).s64().Inputs(0U).Imm(0x0U);
            INST(7U, Opcode::LoadPairPart).s64().Inputs(6U).Imm(0x0U);
            INST(8U, Opcode::LoadPairPart).s64().Inputs(6U).Imm(0x1U);
            INST(27U, Opcode::SaveState).Inputs(0U).SrcVregs({VirtualRegister::BRIDGE});

            INST(9U, Opcode::StoreArrayPairI).s64().Inputs(0U, 8U, 7U).Imm(0x0U);
            INST(28U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, OnlySingleCoalescingOverSaveStateDeoptimize)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(26U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x0U);
            INST(2U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(28U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);

            INST(30U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);
            INST(21U, Opcode::Add).s64().Inputs(28U, 26U);
            INST(22U, Opcode::Add).s64().Inputs(30U, 21U);
            INST(23U, Opcode::Return).s64().Inputs(22U);
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(31U, Opcode::LoadArrayPairI).s64().Inputs(0U).Imm(0x0U);
            INST(33U, Opcode::LoadPairPart).s64().Inputs(31U).Imm(0x0U);
            INST(32U, Opcode::LoadPairPart).s64().Inputs(31U).Imm(0x1U);
            INST(2U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});

            INST(30U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);
            INST(21U, Opcode::Add).s64().Inputs(32U, 33U);
            INST(22U, Opcode::Add).s64().Inputs(30U, 21U);
            INST(23U, Opcode::Return).s64().Inputs(22U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, OverSaveStateDeoptimize)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x0U);
            INST(11U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);

            INST(3U, Opcode::StoreArrayI).s64().Inputs(0U, 2U).Imm(0x0U);
            INST(12U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::StoreArrayI).s64().Inputs(0U, 1U).Imm(0x1U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();

    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::LoadArrayPairI).s64().Inputs(0U).Imm(0x0U);
            INST(7U, Opcode::LoadPairPart).s64().Inputs(6U).Imm(0x0U);
            INST(8U, Opcode::LoadPairPart).s64().Inputs(6U).Imm(0x1U);
            INST(27U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});

            INST(3U, Opcode::StoreArrayI).s64().Inputs(0U, 8U).Imm(0x0U);
            INST(12U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::StoreArrayI).s64().Inputs(0U, 7U).Imm(0x1U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, OnlySingleCoalescingOverSaveStateWithCheckUser)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(26U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x0U);
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(28U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);

            INST(30U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);
            INST(21U, Opcode::Add).s64().Inputs(28U, 26U);
            INST(22U, Opcode::Add).s64().Inputs(30U, 21U);
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(23U, Opcode::Return).s64().Inputs(22U);
        }
    }
    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(31U, Opcode::LoadArrayPairI).s64().Inputs(0U).Imm(0x0U);
            INST(33U, Opcode::LoadPairPart).s64().Inputs(31U).Imm(0x0U);
            INST(32U, Opcode::LoadPairPart).s64().Inputs(31U).Imm(0x1U);
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});

            INST(30U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);
            INST(21U, Opcode::Add).s64().Inputs(32U, 33U);
            INST(22U, Opcode::Add).s64().Inputs(30U, 21U);
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(23U, Opcode::Return).s64().Inputs(22U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, OverSaveStateWithCheckUser)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x0U);
            INST(11U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);

            INST(3U, Opcode::StoreArrayI).s64().Inputs(0U, 2U).Imm(0x0U);
            INST(12U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::StoreArrayI).s64().Inputs(0U, 1U).Imm(0x1U);
            INST(13U, Opcode::NullCheck).ref().Inputs(0U, 11U);
            INST(14U, Opcode::NullCheck).ref().Inputs(0U, 12U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();

    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::LoadArrayPairI).s64().Inputs(0U).Imm(0x0U);
            INST(7U, Opcode::LoadPairPart).s64().Inputs(6U).Imm(0x0U);
            INST(8U, Opcode::LoadPairPart).s64().Inputs(6U).Imm(0x1U);
            INST(11U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});

            INST(3U, Opcode::StoreArrayI).s64().Inputs(0U, 8U).Imm(0x0U);
            INST(12U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::StoreArrayI).s64().Inputs(0U, 7U).Imm(0x1U);
            INST(13U, Opcode::NullCheck).ref().Inputs(0U, 11U);
            INST(14U, Opcode::NullCheck).ref().Inputs(0U, 12U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}

TEST_F(MemoryCoalescingTest, OverSaveStateWithDeoptimizeUser)
{
    // Coalescing is supported only for aarch64
    if (GetGraph()->GetArch() != Arch::AARCH64) {
        return;
    }
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x0U);
            INST(11U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadArrayI).s64().Inputs(0U).Imm(0x1U);

            INST(3U, Opcode::StoreArrayI).s64().Inputs(0U, 2U).Imm(0x0U);
            INST(12U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::StoreArrayI).s64().Inputs(0U, 1U).Imm(0x1U);
            INST(13U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::NEGATIVE_CHECK).Inputs(11U);
            INST(14U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::NEGATIVE_CHECK).Inputs(12U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<MemoryCoalescing>());
    GraphChecker(GetGraph()).Check();

    Graph *optGraph = CreateEmptyGraph();
    GRAPH(optGraph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::LoadArrayPairI).s64().Inputs(0U).Imm(0x0U);
            INST(7U, Opcode::LoadPairPart).s64().Inputs(6U).Imm(0x0U);
            INST(8U, Opcode::LoadPairPart).s64().Inputs(6U).Imm(0x1U);
            INST(11U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});

            INST(3U, Opcode::StoreArrayI).s64().Inputs(0U, 8U).Imm(0x0U);
            INST(12U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::StoreArrayI).s64().Inputs(0U, 7U).Imm(0x1U);
            INST(13U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::NEGATIVE_CHECK).Inputs(11U);
            INST(14U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::NEGATIVE_CHECK).Inputs(12U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), optGraph));
}
// NOLINTEND(readability-magic-numbers)

}  //  namespace ark::compiler
