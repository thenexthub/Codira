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
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/ir/inst.h"
#include "optimizer/optimizations/lse.h"
#include "optimizer/optimizations/peepholes.h"

namespace ark::compiler {
class LSETest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers)
/// Two consecutive loads of the same array element with leading store
TEST_F(LSETest, SimpleLoad)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({2U, 5U});
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(5U, Opcode::LenArray).s32().Inputs(4U);
            INST(6U, Opcode::BoundsCheck).s32().Inputs(5U, 2U, 3U);
            INST(7U, Opcode::StoreArray).u32().Inputs(4U, 6U, 1U);
            INST(8U, Opcode::SaveState).Inputs(0U, 2U).SrcVregs({2U, 5U});
            INST(11U, Opcode::BoundsCheck).s32().Inputs(5U, 2U, 8U);
            INST(12U, Opcode::LoadArray).s32().Inputs(4U, 11U);
            INST(13U, Opcode::SaveState).Inputs(12U, 2U, 0U).SrcVregs({0U, 5U, 1U});
            INST(16U, Opcode::BoundsCheck).s32().Inputs(5U, 2U, 13U);
            INST(17U, Opcode::LoadArray).s32().Inputs(4U, 16U);
            INST(18U, Opcode::Add).s32().Inputs(12U, 17U);
            INST(19U, Opcode::Return).s32().Inputs(18U);
        }
    }

    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({2U, 5U});
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(5U, Opcode::LenArray).s32().Inputs(4U);
            INST(6U, Opcode::BoundsCheck).s32().Inputs(5U, 2U, 3U);
            INST(7U, Opcode::StoreArray).u32().Inputs(4U, 6U, 1U);
            INST(18U, Opcode::Add).s32().Inputs(1U, 1U);
            INST(19U, Opcode::Return).s32().Inputs(18U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Two consecutive stores to the same array element
TEST_F(LSETest, SimpleStore)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 3U});
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(5U, Opcode::LenArray).s32().Inputs(4U);
            INST(6U, Opcode::BoundsCheck).s32().Inputs(5U, 2U, 3U);
            INST(7U, Opcode::StoreArray).u32().Inputs(4U, 6U, 1U);
            INST(8U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 3U});
            INST(11U, Opcode::BoundsCheck).s32().Inputs(5U, 2U, 8U);
            INST(12U, Opcode::StoreArray).u32().Inputs(4U, 11U, 1U);
            INST(13U, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 3U});
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(5U, Opcode::LenArray).s32().Inputs(4U);
            INST(6U, Opcode::BoundsCheck).s32().Inputs(5U, 2U, 3U);
            INST(7U, Opcode::StoreArray).u32().Inputs(4U, 6U, 1U);
            INST(13U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Store comes from previous basic block
SRC_GRAPH(PreviousBlocks, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(8U, 0x8U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(7U, Opcode::StoreArray).s32().Inputs(0U, 2U, 1U);
            INST(9U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 8U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(15U, Opcode::LoadArray).s32().Inputs(0U, 2U);
            INST(20U, Opcode::LoadArray).s32().Inputs(0U, 2U);
            INST(21U, Opcode::Add).s32().Inputs(15U, 20U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(22U, Opcode::Phi).s32().Inputs({{2U, 1U}, {4U, 21U}});
            INST(23U, Opcode::Return).s32().Inputs(22U);
        }
    }
}

OUT_GRAPH(PreviousBlocks, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(8U, 0x8U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(7U, Opcode::StoreArray).s32().Inputs(0U, 2U, 1U);
            INST(9U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 8U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(21U, Opcode::Add).s32().Inputs(1U, 1U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(22U, Opcode::Phi).s32().Inputs({{2U, 1U}, {4U, 21U}});
            INST(23U, Opcode::Return).s32().Inputs(22U);
        }
    }
}

TEST_F(LSETest, PreviousBlocks)
{
    src_graph::PreviousBlocks::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::PreviousBlocks::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Loading unknown value twice
TEST_F(LSETest, LoadWithoutStore)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::LoadArray).s64().Inputs(0U, 1U);
            INST(11U, Opcode::LoadArray).s64().Inputs(0U, 1U);
            INST(12U, Opcode::Add).s32().Inputs(6U, 11U);
            INST(13U, Opcode::Return).s32().Inputs(12U);
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::LoadArray).s64().Inputs(0U, 1U);
            INST(12U, Opcode::Add).s32().Inputs(6U, 6U);
            INST(13U, Opcode::Return).s32().Inputs(12U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Empty basic block after elimination
TEST_F(LSETest, EmptyBB)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::LoadArray).s64().Inputs(0U, 1U);
            INST(7U, Opcode::Compare).b().CC(CC_GT).Inputs(6U, 1U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(13U, Opcode::LoadArray).s64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(14U, Opcode::Phi).s64().Inputs({{2U, 6U}, {4U, 13U}});
            INST(15U, Opcode::Return).s64().Inputs(14U);
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::LoadArray).s64().Inputs(0U, 1U);
            INST(7U, Opcode::Compare).b().CC(CC_GT).Inputs(6U, 1U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, 3U) {}
        BASIC_BLOCK(3U, -1L)
        {
            INST(14U, Opcode::Phi).s64().Inputs({{2U, 6U}, {4U, 6U}});
            INST(15U, Opcode::Return).s64().Inputs(14U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Do not load constant multiple times
TEST_F(LSETest, DISABLED_PoolConstants)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(9U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(9U);

            INST(12U, Opcode::SaveState).NoVregs();
            INST(0U, Opcode::LoadString).ref().Inputs(12U).TypeId(42U);
            INST(1U, Opcode::StoreStatic).ref().Inputs(6U, 0U);

            INST(13U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadString).ref().Inputs(13U).TypeId(27U);
            INST(3U, Opcode::StoreStatic).ref().Inputs(6U, 2U);

            INST(14U, Opcode::SaveState).Inputs(2U).SrcVregs({0U});
            INST(4U, Opcode::LoadString).ref().Inputs(14U).TypeId(42U);
            INST(5U, Opcode::StoreStatic).ref().Inputs(6U, 4U);

            INST(21U, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(9U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(9U);

            INST(12U, Opcode::SaveState).NoVregs();
            INST(0U, Opcode::LoadString).ref().Inputs(12U).TypeId(42U);
            INST(1U, Opcode::StoreStatic).ref().Inputs(6U, 0U);

            INST(13U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadString).ref().Inputs(13U).TypeId(27U);
            INST(3U, Opcode::StoreStatic).ref().Inputs(6U, 2U);

            INST(5U, Opcode::StoreStatic).ref().Inputs(6U, 0U);

            INST(21U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Eliminate object field's operations.
TEST_F(LSETest, InstanceFields)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::StoreObject).s64().Inputs(3U, 1U).TypeId(122U);
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 2U});
            INST(6U, Opcode::NullCheck).ref().Inputs(0U, 5U);
            INST(7U, Opcode::LoadObject).s64().Inputs(6U).TypeId(122U);
            INST(8U, Opcode::Return).s64().Inputs(7U);
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::StoreObject).s64().Inputs(3U, 1U).TypeId(122U);
            INST(8U, Opcode::Return).s64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// A store before branch eliminates a load after branch
TEST_F(LSETest, DominationHere)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::StoreObject).s64().Inputs(0U, 1U).TypeId(122U);
            INST(7U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 1U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(11U, Opcode::StoreObject).s64().Inputs(0U, 1U).TypeId(136U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(14U, Opcode::LoadObject).s64().Inputs(0U).TypeId(122U);
            INST(15U, Opcode::Return).s64().Inputs(14U);
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::StoreObject).s64().Inputs(0U, 1U).TypeId(122U);
            INST(7U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 1U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(11U, Opcode::StoreObject).s64().Inputs(0U, 1U).TypeId(136U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(15U, Opcode::Return).s64().Inputs(1U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Infer the stored value through the heap
TEST_F(LSETest, TransitiveValues)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::StoreObject).s64().Inputs(0U, 1U).TypeId(243U);
            INST(7U, Opcode::LoadObject).s64().Inputs(0U).TypeId(243U);
            INST(10U, Opcode::StoreObject).s64().Inputs(0U, 7U).TypeId(243U);
            INST(13U, Opcode::LoadObject).s64().Inputs(0U).TypeId(243U);
            INST(16U, Opcode::StoreObject).s64().Inputs(0U, 13U).TypeId(243U);
            INST(17U, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::StoreObject).s64().Inputs(0U, 1U).TypeId(243U);
            INST(17U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/**
 * Tests that store of an eliminated value and store of an replacement for this
 * eliminated value are treated as same stores.
 */
TEST_F(LSETest, StoreEliminableDominatesStoreReplacement)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::StoreObject).s64().Inputs(0U, 1U).TypeId(243U);
            // Should be replaced with v1
            INST(7U, Opcode::LoadObject).s64().Inputs(0U).TypeId(243U);
            // v5-v6 to invalidate the whole heap
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 2U});
            INST(6U, Opcode::CallStatic).s64().Inputs({{DataType::REFERENCE, 0U}, {DataType::NO_TYPE, 5U}});
            // Here v7 and v1 are the same values. v1 is a replacement for v7
            INST(10U, Opcode::StoreObject).s64().Inputs(2U, 7U).TypeId(243U);
            INST(11U, Opcode::StoreObject).s64().Inputs(2U, 1U).TypeId(243U);
            INST(17U, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::StoreObject).s64().Inputs(0U, 1U).TypeId(243U);
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 2U});
            INST(6U, Opcode::CallStatic).s64().Inputs({{DataType::REFERENCE, 0U}, {DataType::NO_TYPE, 5U}});
            INST(10U, Opcode::StoreObject).s64().Inputs(2U, 1U).TypeId(243U);
            INST(17U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/**
 * Tests that store of an eliminated value and store of an replacement for this
 * eliminated value are treated as same stores.
 */
TEST_F(LSETest, StoreReplacementDominatesStoreEliminable)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::StoreObject).s64().Inputs(0U, 1U).TypeId(243U);
            // Should be replaced with v1
            INST(7U, Opcode::LoadObject).s64().Inputs(0U).TypeId(243U);
            // v5-v6 to invalidate the whole heap
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 2U});
            INST(6U, Opcode::CallStatic).s64().Inputs({{DataType::REFERENCE, 0U}, {DataType::NO_TYPE, 5U}});
            // Here v7 and v1 are the same values. v1 is a replacement for v7
            INST(10U, Opcode::StoreObject).s64().Inputs(2U, 1U).TypeId(243U);
            INST(11U, Opcode::StoreObject).s64().Inputs(2U, 7U).TypeId(243U);
            INST(17U, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::StoreObject).s64().Inputs(0U, 1U).TypeId(243U);
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 2U});
            INST(6U, Opcode::CallStatic).s64().Inputs({{DataType::REFERENCE, 0U}, {DataType::NO_TYPE, 5U}});
            INST(10U, Opcode::StoreObject).s64().Inputs(2U, 1U).TypeId(243U);
            INST(17U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Load elimination in loop by means of a dominated load
SRC_GRAPH(LoopElimination, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(7U, 0x0U).s64();
        CONSTANT(28U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::LoadObject).ref().Inputs(0U).TypeId(242U);
            INST(6U, Opcode::LenArray).s32().Inputs(3U);
            INST(30U, Opcode::Cmp).s32().Inputs(7U, 6U);
            INST(31U, Opcode::Compare).b().CC(CC_GE).Inputs(30U, 7U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        // For (v10 = 0, v10 < lenarr(v3), ++v10)
        //     v11 += v3[v10]
        BASIC_BLOCK(4U, 3U, 4U)
        {
            INST(10U, Opcode::Phi).s32().Inputs({{2U, 7U}, {4U, 27U}});
            INST(11U, Opcode::Phi).s32().Inputs({{2U, 7U}, {4U, 26U}});

            INST(20U, Opcode::LoadObject).ref().Inputs(0U).TypeId(242U);  // Eliminable due to checked access above

            INST(25U, Opcode::LoadArray).s32().Inputs(20U, 10U);

            INST(26U, Opcode::Add).s32().Inputs(25U, 11U);
            INST(27U, Opcode::Add).s32().Inputs(10U, 28U);
            INST(16U, Opcode::Compare).b().CC(CC_GE).Inputs(27U, 6U);
            INST(17U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(16U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(35U, Opcode::Phi).s32().Inputs({{4U, 26U}, {2U, 7U}});
            INST(29U, Opcode::Return).s32().Inputs(35U);
        }
    }
}

OUT_GRAPH(LoopElimination, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(7U, 0x0U).s64();
        CONSTANT(28U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::LoadObject).ref().Inputs(0U).TypeId(242U);
            INST(6U, Opcode::LenArray).s32().Inputs(3U);
            INST(30U, Opcode::Cmp).s32().Inputs(7U, 6U);
            INST(31U, Opcode::Compare).b().CC(CC_GE).Inputs(30U, 7U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        // For (v10 = 0, v10 < lenarr(v3), ++v10)
        //     v11 += v3[v10]
        BASIC_BLOCK(4U, 3U, 4U)
        {
            INST(10U, Opcode::Phi).s32().Inputs({{2U, 7U}, {4U, 27U}});
            INST(11U, Opcode::Phi).s32().Inputs({{2U, 7U}, {4U, 26U}});

            INST(25U, Opcode::LoadArray).s32().Inputs(3U, 10U);

            INST(26U, Opcode::Add).s32().Inputs(25U, 11U);
            INST(27U, Opcode::Add).s32().Inputs(10U, 28U);
            INST(16U, Opcode::Compare).b().CC(CC_GE).Inputs(27U, 6U);
            INST(17U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(16U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(35U, Opcode::Phi).s32().Inputs({{4U, 26U}, {2U, 7U}});
            INST(29U, Opcode::Return).s32().Inputs(35U);
        }
    }
}

TEST_F(LSETest, LoopElimination)
{
    src_graph::LoopElimination::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::LoopElimination::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Loop of multiple blocks
SRC_GRAPH(LoopBranches, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(7U, 0x0U).s64();
        CONSTANT(8U, 0x2U).s64();
        CONSTANT(37U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::LoadObject).ref().Inputs(0U).TypeId(242U);
            INST(6U, Opcode::LenArray).s32().Inputs(3U);
            INST(48U, Opcode::Compare).b().CC(CC_GE).Inputs(7U, 6U);
            INST(49U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(48U);
        }
        // For (v11 = 0, v11 < lenarr(v3), ++v11)
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(11U, Opcode::Phi).s32().Inputs({{2U, 7U}, {7U, 46U}});
            INST(12U, Opcode::Phi).s32().Inputs({{2U, 7U}, {7U, 45U}});
            INST(22U, Opcode::Mod).s32().Inputs(11U, 8U);
            INST(23U, Opcode::Compare).b().CC(CC_EQ).Inputs(22U, 7U);
            INST(24U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(23U);
        }
        //     If (v11 % 2 == 1)
        //          v44 = v3[v11]
        BASIC_BLOCK(6U, 7U)
        {
            INST(27U, Opcode::LoadObject).ref().Inputs(0U).TypeId(242U);  // Eliminable due to checked access above
            INST(32U, Opcode::LoadArray).s32().Inputs(27U, 11U);
        }
        //     else
        //          v44 = v3[v11 + 1]
        BASIC_BLOCK(5U, 7U)
        {
            INST(35U, Opcode::LoadObject).ref().Inputs(0U).TypeId(242U);  // Eliminable due to checked access above
            INST(36U, Opcode::Add).s32().Inputs(11U, 37U);
            INST(42U, Opcode::LoadArray).s32().Inputs(35U, 36U);
        }
        //     v12 += v44
        BASIC_BLOCK(7U, 3U, 4U)
        {
            INST(44U, Opcode::Phi).s32().Inputs({{6U, 32U}, {5U, 42U}});
            INST(45U, Opcode::Add).s32().Inputs(44U, 12U);
            INST(46U, Opcode::Add).s32().Inputs(11U, 37U);
            INST(18U, Opcode::Compare).b().CC(CC_GE).Inputs(46U, 6U);
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(18U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(51U, Opcode::Phi).s32().Inputs({{7U, 45U}, {2U, 7U}});
            INST(47U, Opcode::Return).s32().Inputs(51U);
        }
    }
}

OUT_GRAPH(LoopBranches, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(7U, 0x0U).s64();
        CONSTANT(8U, 0x2U).s64();
        CONSTANT(37U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::LoadObject).ref().Inputs(0U).TypeId(242U);
            INST(6U, Opcode::LenArray).s32().Inputs(3U);
            INST(48U, Opcode::Compare).b().CC(CC_GE).Inputs(7U, 6U);
            INST(49U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(48U);
        }
        // For (v11 = 0, v11 < lenarr(v3), ++v11)
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(11U, Opcode::Phi).s32().Inputs({{2U, 7U}, {7U, 46U}});
            INST(12U, Opcode::Phi).s32().Inputs({{2U, 7U}, {7U, 45U}});
            INST(22U, Opcode::Mod).s32().Inputs(11U, 8U);
            INST(23U, Opcode::Compare).b().CC(CC_EQ).Inputs(22U, 7U);
            INST(24U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(23U);
        }
        //     If (v11 % 2 == 1)
        //          v44 = v3[v11]
        BASIC_BLOCK(6U, 7U)
        {
            INST(32U, Opcode::LoadArray).s32().Inputs(3U, 11U);
        }
        //     else
        //          v44 = v3[v11 + 1]
        BASIC_BLOCK(5U, 7U)
        {
            INST(36U, Opcode::Add).s32().Inputs(11U, 37U);
            INST(42U, Opcode::LoadArray).s32().Inputs(3U, 36U);
        }
        //     v12 += v44
        BASIC_BLOCK(7U, 3U, 4U)
        {
            INST(44U, Opcode::Phi).s32().Inputs({{6U, 32U}, {5U, 42U}});
            INST(45U, Opcode::Add).s32().Inputs(44U, 12U);
            INST(46U, Opcode::Add).s32().Inputs(11U, 37U);
            INST(18U, Opcode::Compare).b().CC(CC_GE).Inputs(46U, 6U);
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(18U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(51U, Opcode::Phi).s32().Inputs({{7U, 45U}, {2U, 7U}});
            INST(47U, Opcode::Return).s32().Inputs(51U);
        }
    }
}

TEST_F(LSETest, LoopBranches)
{
    src_graph::LoopBranches::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::LoopBranches::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Nested loop elimination
// CC-OFFNXT(huge_method, G.FUN.01) graph creation
SRC_GRAPH(NestedLoopElimination, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        CONSTANT(11U, 0x0U).s64();
        CONSTANT(62U, 0x1U).s64();
        BASIC_BLOCK(2U, 8U)
        {
            INST(4U, Opcode::StoreObject).ref().Inputs(0U, 1U).TypeId(181U);
            INST(7U, Opcode::LoadObject).ref().Inputs(0U).TypeId(195U);
            INST(10U, Opcode::LenArray).s32().Inputs(7U);
        }
        // For (v14 = 0, v14 < lenarr(v7), v14++)
        BASIC_BLOCK(8U, 3U, 4U)
        {
            INST(14U, Opcode::Phi).s32().Inputs({{2U, 11U}, {5U, 63U}});
            INST(15U, Opcode::Phi).s32().Inputs({{2U, 11U}, {5U, 69U}});
            INST(20U, Opcode::Cmp).s32().Inputs(14U, 10U);
            INST(21U, Opcode::Compare).b().CC(CC_GE).Inputs(20U, 11U);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(21U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(25U, Opcode::LoadObject).ref().Inputs(0U).TypeId(209U);
            INST(28U, Opcode::LenArray).s32().Inputs(25U);
            INST(65U, Opcode::Cmp).s32().Inputs(11U, 28U);
            INST(66U, Opcode::Compare).b().CC(CC_GE).Inputs(65U, 11U);
            INST(67U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(66U);
        }
        //     For (v35 = 0, v35 < lenarr(v25), v35++)
        //         v32 += (v7[v14] * v25[v35])
        BASIC_BLOCK(6U, 5U, 6U)
        {
            INST(32U, Opcode::Phi).s32().Inputs({{4U, 15U}, {6U, 60U}});
            INST(35U, Opcode::Phi).s32().Inputs({{4U, 11U}, {6U, 61U}});
            INST(45U, Opcode::LoadObject).ref().Inputs(0U).TypeId(195U);  // Already loaded in BB2
            INST(50U, Opcode::LoadArray).s32().Inputs(45U, 14U);
            INST(53U, Opcode::LoadObject).ref().Inputs(0U).TypeId(209U);  // Already loaded in BB4
            INST(58U, Opcode::LoadArray).s32().Inputs(53U, 35U);
            INST(59U, Opcode::Mul).s32().Inputs(50U, 58U);
            INST(60U, Opcode::Add).s32().Inputs(59U, 32U);
            INST(61U, Opcode::Add).s32().Inputs(35U, 62U);
            INST(40U, Opcode::Cmp).s32().Inputs(61U, 28U);
            INST(41U, Opcode::Compare).b().CC(CC_GE).Inputs(40U, 11U);
            INST(42U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(41U);
        }
        BASIC_BLOCK(5U, 8U)
        {
            INST(69U, Opcode::Phi).s32().Inputs({{6U, 60U}, {4U, 15U}});
            INST(63U, Opcode::Add).s32().Inputs(14U, 62U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(64U, Opcode::Return).s32().Inputs(15U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(NestedLoopElimination, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        CONSTANT(11U, 0x0U).s64();
        CONSTANT(62U, 0x1U).s64();
        BASIC_BLOCK(2U, 8U)
        {
            INST(4U, Opcode::StoreObject).ref().Inputs(0U, 1U).TypeId(181U);
            INST(7U, Opcode::LoadObject).ref().Inputs(0U).TypeId(195U);
            INST(10U, Opcode::LenArray).s32().Inputs(7U);
        }
        // For (v14 = 0, v14 < lenarr(v7), v14++)
        BASIC_BLOCK(8U, 3U, 4U)
        {
            INST(14U, Opcode::Phi).s32().Inputs({{2U, 11U}, {5U, 63U}});
            INST(15U, Opcode::Phi).s32().Inputs({{2U, 11U}, {5U, 69U}});
            INST(20U, Opcode::Cmp).s32().Inputs(14U, 10U);
            INST(21U, Opcode::Compare).b().CC(CC_GE).Inputs(20U, 11U);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(21U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(25U, Opcode::LoadObject).ref().Inputs(0U).TypeId(209U);
            INST(28U, Opcode::LenArray).s32().Inputs(25U);
            INST(65U, Opcode::Cmp).s32().Inputs(11U, 28U);
            INST(66U, Opcode::Compare).b().CC(CC_GE).Inputs(65U, 11U);
            INST(67U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(66U);
        }
        //     For (v35 = 0, v35 < lenarr(v25), v35++)
        //         v32 += (v7[v14] * v25[v35])
        BASIC_BLOCK(6U, 5U, 6U)
        {
            INST(32U, Opcode::Phi).s32().Inputs({{4U, 15U}, {6U, 60U}});
            INST(35U, Opcode::Phi).s32().Inputs({{4U, 11U}, {6U, 61U}});
            INST(50U, Opcode::LoadArray).s32().Inputs(7U, 14U);
            INST(58U, Opcode::LoadArray).s32().Inputs(25U, 35U);
            INST(59U, Opcode::Mul).s32().Inputs(50U, 58U);
            INST(60U, Opcode::Add).s32().Inputs(59U, 32U);
            INST(61U, Opcode::Add).s32().Inputs(35U, 62U);
            INST(40U, Opcode::Cmp).s32().Inputs(61U, 28U);
            INST(41U, Opcode::Compare).b().CC(CC_GE).Inputs(40U, 11U);
            INST(42U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(41U);
        }
        BASIC_BLOCK(5U, 8U)
        {
            INST(69U, Opcode::Phi).s32().Inputs({{6U, 60U}, {4U, 15U}});
            INST(63U, Opcode::Add).s32().Inputs(14U, 62U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(64U, Opcode::Return).s32().Inputs(15U);
        }
    }
}

TEST_F(LSETest, NestedLoopElimination)
{
    src_graph::NestedLoopElimination::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::NestedLoopElimination::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

// Replace MUST_ALIASed accesses
// Move out of loop NO_ALIASed accesses
SRC_GRAPH(LoopWithMayAliases, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();  // i32[][]
        CONSTANT(1U, 0x0U).s64();
        CONSTANT(30U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::LoadArray).ref().Inputs(0U, 1U);
            INST(9U, Opcode::LenArray).s32().Inputs(6U);
            INST(45U, Opcode::Compare).b().CC(CC_GE).Inputs(1U, 9U);
            INST(46U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(45U);
        }
        // v6 = v[0]
        // For (v12 = 0, v12 < lenarr(v0[0]), v12++)
        //     v13 += v0[0][0] + v0[1][0]
        BASIC_BLOCK(4U, 3U, 4U)
        {
            INST(12U, Opcode::Phi).s32().Inputs({{2U, 1U}, {4U, 43U}});
            INST(13U, Opcode::Phi).s32().Inputs({{2U, 1U}, {4U, 42U}});
            INST(24U, Opcode::LoadArray).ref().Inputs(0U, 1U);  // Eliminated due to v6
            INST(29U, Opcode::LoadArray).s32().Inputs(24U, 1U);
            INST(35U, Opcode::LoadArray).ref().Inputs(0U, 30U);  // Move out of loop
            INST(40U, Opcode::LoadArray).s32().Inputs(35U, 1U);
            INST(41U, Opcode::Add).s32().Inputs(40U, 29U);
            INST(42U, Opcode::Add).s32().Inputs(41U, 13U);
            INST(43U, Opcode::Add).s32().Inputs(12U, 30U);
            INST(18U, Opcode::Compare).b().CC(CC_GE).Inputs(43U, 9U);
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(18U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(48U, Opcode::Phi).s32().Inputs({{4U, 42U}, {2U, 1U}});
            INST(44U, Opcode::Return).s32().Inputs(48U);
        }
    }
}

OUT_GRAPH(LoopWithMayAliases, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();  // i32[][]
        CONSTANT(1U, 0x0U).s64();
        CONSTANT(30U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::LoadArray).ref().Inputs(0U, 1U);
            INST(9U, Opcode::LenArray).s32().Inputs(6U);
            INST(45U, Opcode::Compare).b().CC(CC_GE).Inputs(1U, 9U);
            INST(35U, Opcode::LoadArray).ref().Inputs(0U, 30U);
            INST(46U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(45U);
        }
        // v6 = v0[0]
        // v35 = v0[1]
        // For (v12 = 0, v12 < lenarr(v0[0]), v12++)
        //     v13 += v6[0] + v35[0]
        BASIC_BLOCK(4U, 3U, 4U)
        {
            INST(12U, Opcode::Phi).s32().Inputs({{2U, 1U}, {4U, 43U}});
            INST(13U, Opcode::Phi).s32().Inputs({{2U, 1U}, {4U, 42U}});
            INST(29U, Opcode::LoadArray).s32().Inputs(6U, 1U);
            INST(40U, Opcode::LoadArray).s32().Inputs(35U, 1U);
            INST(41U, Opcode::Add).s32().Inputs(40U, 29U);
            INST(42U, Opcode::Add).s32().Inputs(41U, 13U);
            INST(43U, Opcode::Add).s32().Inputs(12U, 30U);
            INST(18U, Opcode::Compare).b().CC(CC_GE).Inputs(43U, 9U);
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(18U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(48U, Opcode::Phi).s32().Inputs({{4U, 42U}, {2U, 1U}});
            INST(44U, Opcode::Return).s32().Inputs(48U);
        }
    }
}

TEST_F(LSETest, LoopWithMayAliases)
{
    src_graph::LoopWithMayAliases::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::LoopWithMayAliases::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Loop elimination combined with regular elimination
SRC_GRAPH(CombinedWithLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();  // i32[][]
        CONSTANT(1U, 0x0U).s64();
        CONSTANT(43U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::LoadArray).ref().Inputs(0U, 1U);
            INST(9U, Opcode::LenArray).s32().Inputs(6U);
            INST(45U, Opcode::Compare).b().CC(CC_GE).Inputs(1U, 9U);
            INST(46U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(45U);
        }
        // For (v12 = 0, v12 < lenarr(v0[0]), v12++)
        //     v13 += v0[0][0] + v0[0][0]
        BASIC_BLOCK(4U, 3U, 4U)
        {
            INST(12U, Opcode::Phi).s32().Inputs({{2U, 1U}, {4U, 42U}});
            INST(13U, Opcode::Phi).s32().Inputs({{2U, 1U}, {4U, 41U}});
            INST(24U, Opcode::LoadArray).ref().Inputs(0U, 1U);  // Eliminated due to v6
            INST(29U, Opcode::LoadArray).s32().Inputs(24U, 1U);
            INST(34U, Opcode::LoadArray).ref().Inputs(0U, 1U);   // Eliminated due to v24
            INST(39U, Opcode::LoadArray).s32().Inputs(34U, 1U);  // Eliminated due to v29
            INST(40U, Opcode::Add).s32().Inputs(39U, 29U);
            INST(41U, Opcode::Add).s32().Inputs(40U, 13U);
            INST(42U, Opcode::Add).s32().Inputs(12U, 43U);
            INST(18U, Opcode::Compare).b().CC(CC_GE).Inputs(42U, 9U);
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(18U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(48U, Opcode::Phi).s32().Inputs({{4U, 41U}, {2U, 1U}});
            INST(44U, Opcode::Return).s32().Inputs(48U);
        }
    }
}

OUT_GRAPH(CombinedWithLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();  // i32[][]
        CONSTANT(1U, 0x0U).s64();
        CONSTANT(43U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(6U, Opcode::LoadArray).ref().Inputs(0U, 1U);
            INST(9U, Opcode::LenArray).s32().Inputs(6U);
            INST(45U, Opcode::Compare).b().CC(CC_GE).Inputs(1U, 9U);
            INST(46U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(45U);
        }
        // For (v12 = 0, v12 < lenarr(v0[0]), v12++)
        //     v13 += v0[0][0] + v0[0][0]
        BASIC_BLOCK(4U, 3U, 4U)
        {
            INST(12U, Opcode::Phi).s32().Inputs({{2U, 1U}, {4U, 42U}});
            INST(13U, Opcode::Phi).s32().Inputs({{2U, 1U}, {4U, 41U}});
            INST(29U, Opcode::LoadArray).s32().Inputs(6U, 1U);
            INST(40U, Opcode::Add).s32().Inputs(29U, 29U);
            INST(41U, Opcode::Add).s32().Inputs(40U, 13U);
            INST(42U, Opcode::Add).s32().Inputs(12U, 43U);
            INST(18U, Opcode::Compare).b().CC(CC_GE).Inputs(42U, 9U);
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(18U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(48U, Opcode::Phi).s32().Inputs({{4U, 41U}, {2U, 1U}});
            INST(44U, Opcode::Return).s32().Inputs(48U);
        }
    }
}

TEST_F(LSETest, CombinedWithLoop)
{
    src_graph::CombinedWithLoop::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::CombinedWithLoop::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Phi candidates are the origins of values on the heap
SRC_GRAPH(OverwrittenPhiCandidates, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        CONSTANT(2U, 0x0U).s64();
        CONSTANT(50U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(7U, Opcode::LoadArray).ref().Inputs(0U, 2U);
            INST(12U, Opcode::StoreArray).ref().Inputs(0U, 2U, 1U);
            INST(15U, Opcode::LenArray).s32().Inputs(7U);
            INST(52U, Opcode::Compare).b().CC(CC_GE).Inputs(2U, 15U);
            INST(53U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(52U);
        }
        BASIC_BLOCK(4U, 3U, 4U)
        {
            INST(18U, Opcode::Phi).s32().Inputs({{2U, 2U}, {4U, 49U}});
            INST(19U, Opcode::Phi).s32().Inputs({{2U, 2U}, {4U, 48U}});
            // Must be replaced with a1 not v7
            INST(31U, Opcode::LoadArray).ref().Inputs(0U, 2U);
            INST(36U, Opcode::LoadArray).s32().Inputs(31U, 2U);
            // Must be replaced with a1 not v7
            INST(41U, Opcode::LoadArray).ref().Inputs(0U, 2U);
            INST(46U, Opcode::LoadArray).s32().Inputs(41U, 2U);
            INST(47U, Opcode::Add).s32().Inputs(46U, 36U);
            INST(48U, Opcode::Add).s32().Inputs(47U, 19U);
            INST(49U, Opcode::Add).s32().Inputs(18U, 50U);
            INST(25U, Opcode::Compare).b().CC(CC_GE).Inputs(49U, 15U);
            INST(26U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(25U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(55U, Opcode::Phi).s32().Inputs({{4U, 48U}, {2U, 2U}});
            INST(51U, Opcode::Return).s32().Inputs(55U);
        }
    }
}

OUT_GRAPH(OverwrittenPhiCandidates, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        CONSTANT(2U, 0x0U).s64();
        CONSTANT(50U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(7U, Opcode::LoadArray).ref().Inputs(0U, 2U);
            INST(12U, Opcode::StoreArray).ref().Inputs(0U, 2U, 1U);
            INST(15U, Opcode::LenArray).s32().Inputs(7U);
            INST(52U, Opcode::Compare).b().CC(CC_GE).Inputs(2U, 15U);
            INST(53U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(52U);
        }
        BASIC_BLOCK(4U, 3U, 4U)
        {
            INST(18U, Opcode::Phi).s32().Inputs({{2U, 2U}, {4U, 49U}});
            INST(19U, Opcode::Phi).s32().Inputs({{2U, 2U}, {4U, 48U}});
            INST(36U, Opcode::LoadArray).s32().Inputs(1U, 2U);
            INST(46U, Opcode::LoadArray).s32().Inputs(1U, 2U);
            INST(47U, Opcode::Add).s32().Inputs(46U, 36U);
            INST(48U, Opcode::Add).s32().Inputs(47U, 19U);
            INST(49U, Opcode::Add).s32().Inputs(18U, 50U);
            INST(25U, Opcode::Compare).b().CC(CC_GE).Inputs(49U, 15U);
            INST(26U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(25U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(55U, Opcode::Phi).s32().Inputs({{4U, 48U}, {2U, 2U}});
            INST(51U, Opcode::Return).s32().Inputs(55U);
        }
    }
}

TEST_F(LSETest, OverwrittenPhiCandidates)
{
    src_graph::OverwrittenPhiCandidates::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::OverwrittenPhiCandidates::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/**
 * Do not use a deleted instruction as a valid phi candidate
 *    [entry]
 *       |
 *      [2]
 *       |
 *      [3]--\
 *       |   [7]<--\
 *      [6]<-/ \---/
 *       |
 *     [exit]
 */
SRC_GRAPH(DeletedPhiCandidate, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(9U, 0x0U).s64();
        CONSTANT(47U, 0x1U).s64();
        BASIC_BLOCK(2U, 6U, 7U)
        {
            INST(10U, Opcode::SaveState).Inputs(9U).SrcVregs({0U});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(10U).TypeId(0U);
            INST(1U, Opcode::LoadStatic).s32().Inputs(3U).TypeId(3059U);  // A valid phi candidate for 3059
            INST(97U, Opcode::SaveState);
            INST(4U, Opcode::NewArray).ref().Inputs(3U, 1U, 97U).TypeId(273U);
            INST(5U, Opcode::StoreStatic).ref().Inputs(3U, 4U).TypeId(3105U);
            INST(65U, Opcode::LoadStatic).s32().Inputs(3U).TypeId(3059U);  // Would be a deleted phi candidate
            INST(94U, Opcode::SaveState);
            INST(68U, Opcode::NewArray).ref().Inputs(3U, 65U, 94U).TypeId(273U);
            INST(69U, Opcode::StoreStatic).ref().Inputs(3U, 68U).TypeId(3105U);
            INST(89U, Opcode::LoadStatic).s32().Inputs(3U).TypeId(3059U);  // Would be a deleted phi candidate
            INST(90U, Opcode::Compare).b().CC(CC_GE).Inputs(9U, 89U);
            INST(91U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(90U);
        }
        BASIC_BLOCK(7U, 6U, 7U)
        {
            INST(72U, Opcode::Phi).s32().Inputs({{2U, 9U}, {7U, 87U}});
            INST(79U, Opcode::LoadStatic).ref().Inputs(3U).TypeId(3105U);
            INST(87U, Opcode::Add).s32().Inputs(72U, 47U);
            INST(76U, Opcode::LoadStatic).s32().Inputs(3U).TypeId(3059U);
            INST(77U, Opcode::Compare).b().CC(CC_GE).Inputs(87U, 76U);
            INST(78U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(77U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(88U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(DeletedPhiCandidate, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(9U, 0x0U).s64();
        CONSTANT(47U, 0x1U).s64();
        BASIC_BLOCK(2U, 6U, 7U)
        {
            INST(10U, Opcode::SaveState).Inputs(9U).SrcVregs({0U});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(10U).TypeId(0U);
            INST(1U, Opcode::LoadStatic).s32().Inputs(3U).TypeId(3059U);
            INST(97U, Opcode::SaveState);
            INST(4U, Opcode::NewArray).ref().Inputs(3U, 1U, 97U).TypeId(273U);
            INST(5U, Opcode::StoreStatic).ref().Inputs(3U, 4U).TypeId(3105U);
            INST(94U, Opcode::SaveState);
            INST(68U, Opcode::NewArray).ref().Inputs(3U, 1U, 94U).TypeId(273U);
            INST(69U, Opcode::StoreStatic).ref().Inputs(3U, 68U).TypeId(3105U);
            INST(90U, Opcode::Compare).b().CC(CC_GE).Inputs(9U, 1U);
            INST(91U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(90U);
        }
        BASIC_BLOCK(7U, 6U, 7U)
        {
            INST(72U, Opcode::Phi).s32().Inputs({{2U, 9U}, {7U, 87U}});
            INST(87U, Opcode::Add).s32().Inputs(72U, 47U);
            INST(77U, Opcode::Compare).b().CC(CC_GE).Inputs(87U, 1U);
            INST(78U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(77U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(88U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(LSETest, DeletedPhiCandidate)
{
    src_graph::DeletedPhiCandidate::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::DeletedPhiCandidate::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Stored value might be casted therefore we should cast if types are different
TEST_F(LSETest, PrimitiveTypeCasting)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).s64().Inputs(1U, 2U);
            INST(6U, Opcode::StoreObject).s32().Inputs(0U, 3U).TypeId(157U);
            INST(9U, Opcode::LoadObject).s32().Inputs(0U).TypeId(157U);
            INST(10U, Opcode::Return).s32().Inputs(9U);
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).s64().Inputs(1U, 2U);
            INST(6U, Opcode::StoreObject).s32().Inputs(0U, 3U).TypeId(157U);
            // Types of a stored value (v3) and loaded (v9) are different. Cast is needed.
            INST(11U, Opcode::Cast).s32().SrcType(DataType::INT64).Inputs(3U);
            INST(10U, Opcode::Return).s32().Inputs(11U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/**
 * Stored value might be casted therefore we should cast if types are different
 * To avoid inappropriate Cast, skip the elimination of some loadobj inst
 */
SRC_GRAPH(PrimitiveInt8TypeCasting, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();

        PARAMETER(3U, 3U).ref();
        PARAMETER(4U, 4U).s8();
        PARAMETER(5U, 5U).s8();

        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::Add).s64().Inputs(1U, 2U);
            INST(9U, Opcode::StoreObject).s32().Inputs(0U, 7U).TypeId(159U);
            INST(11U, Opcode::LoadObject).s32().Inputs(0U).TypeId(159U);

            INST(16U, Opcode::Add).s8().Inputs(4U, 5U);
            INST(17U, Opcode::StoreObject).s32().Inputs(3U, 16U).TypeId(163U);
            INST(18U, Opcode::LoadObject).s32().Inputs(3U).TypeId(163U);
            INST(23U, Opcode::Add).s32().Inputs(11U, 18U);
            INST(19U, Opcode::Return).s32().Inputs(23U);
        }
    }
}

OUT_GRAPH(PrimitiveInt8TypeCasting, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();

        PARAMETER(3U, 3U).ref();
        PARAMETER(4U, 4U).s8();
        PARAMETER(5U, 5U).s8();

        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::Add).s64().Inputs(1U, 2U);
            INST(9U, Opcode::StoreObject).s32().Inputs(0U, 7U).TypeId(159U);
            // Types of stored value (v4, INT8) and loaded (v9, INT32) are different and
            // legal for cast. We can use Cast to take place of LoadObject.
            INST(11U, Opcode::Cast).s32().SrcType(DataType::INT64).Inputs(7U);

            INST(16U, Opcode::Add).s8().Inputs(4U, 5U);
            INST(17U, Opcode::StoreObject).s32().Inputs(3U, 16U).TypeId(163U);
            // if eliminate the loadobj inst, inappropriate inst 'i32 Cast i8' will be created.
            INST(18U, Opcode::LoadObject).s32().Inputs(3U).TypeId(163U);
            INST(23U, Opcode::Add).s32().Inputs(11U, 18U);
            INST(19U, Opcode::Return).s32().Inputs(23U);
        }
    }
}

TEST_F(LSETest, PrimitiveInt8TypeCasting)
{
    auto graph = CreateEmptyBytecodeGraph();
    src_graph::PrimitiveInt8TypeCasting::CREATE(graph);
    auto graphLsed = CreateEmptyBytecodeGraph();
    out_graph::PrimitiveInt8TypeCasting::CREATE(graphLsed);
    ASSERT_TRUE(graph->RunPass<Lse>());
    GraphChecker(graph).Check();
    ASSERT_TRUE(GraphComparator().Compare(graph, graphLsed));
}

/// Overwritten load in loop
SRC_GRAPH(LoopWithOverwrite, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(7U, 0x0U).s64();
        CONSTANT(28U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::LoadObject).ref().Inputs(0U).TypeId(242U);
            INST(6U, Opcode::LenArray).s32().Inputs(3U);
            INST(30U, Opcode::Cmp).s32().Inputs(7U, 6U);
            INST(31U, Opcode::Compare).b().CC(CC_GE).Inputs(30U, 7U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        // For (v10 = 0, v10 < lenarr(v3), v10++)
        //     v3 = v3[v10]
        BASIC_BLOCK(4U, 3U, 4U)
        {
            INST(10U, Opcode::Phi).s32().Inputs({{2U, 7U}, {4U, 27U}});

            INST(20U, Opcode::LoadObject).ref().Inputs(0U).TypeId(242U);
            INST(25U, Opcode::LoadArray).ref().Inputs(20U, 10U);

            INST(27U, Opcode::Add).s32().Inputs(10U, 28U);

            INST(33U, Opcode::StoreObject).ref().Inputs(0U, 25U).TypeId(242U);

            INST(15U, Opcode::Cmp).s32().Inputs(27U, 6U);
            INST(16U, Opcode::Compare).b().CC(CC_GE).Inputs(15U, 7U);
            INST(17U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(16U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(29U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(LoopWithOverwrite, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(7U, 0x0U).s64();
        CONSTANT(28U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::LoadObject).ref().Inputs(0U).TypeId(242U);
            INST(6U, Opcode::LenArray).s32().Inputs(3U);
            INST(30U, Opcode::Cmp).s32().Inputs(7U, 6U);
            INST(31U, Opcode::Compare).b().CC(CC_GE).Inputs(30U, 7U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        // For (v10 = 0, v10 < lenarr(v3), v10++)
        //     v3 = v3[v10]
        BASIC_BLOCK(4U, 3U, 4U)
        {
            INST(10U, Opcode::Phi).s32().Inputs({{2U, 7U}, {4U, 27U}});
            INST(11U, Opcode::Phi).ref().Inputs({{2U, 3U}, {4U, 25U}});

            INST(25U, Opcode::LoadArray).ref().Inputs(11U, 10U);

            INST(27U, Opcode::Add).s32().Inputs(10U, 28U);

            INST(33U, Opcode::StoreObject).ref().Inputs(0U, 25U).TypeId(242U);

            INST(15U, Opcode::Cmp).s32().Inputs(27U, 6U);
            INST(16U, Opcode::Compare).b().CC(CC_GE).Inputs(15U, 7U);
            INST(17U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(16U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(29U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(LSETest, LoopWithOverwrite)
{
    src_graph::LoopWithOverwrite::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::LoopWithOverwrite::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Eliminate loads saved in SaveState and SafePoint.
TEST_F(LSETest, SavedLoadInSafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::LoadObject).ref().Inputs(0U).TypeId(100U);
            INST(6U, Opcode::StoreObject).ref().Inputs(0U, 3U).TypeId(122U);

            INST(7U, Opcode::SafePoint).Inputs(0U, 3U).SrcVregs({0U, 2U});

            // Eliminable because of v3 saved in SafePoint
            INST(8U, Opcode::LoadObject).ref().Inputs(0U).TypeId(100U);

            INST(12U, Opcode::Return).ref().Inputs(8U);
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::LoadObject).ref().Inputs(0U).TypeId(100U);
            INST(6U, Opcode::StoreObject).ref().Inputs(0U, 3U).TypeId(122U);

            INST(7U, Opcode::SafePoint).Inputs(0U, 3U).SrcVregs({0U, 2U});

            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// SafePoint may relocate collected references making them invalid
TEST_F(LSETest, LoopWithSafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, 8U)
        {
            INST(5U, Opcode::LoadObject).ref().Inputs(0U).TypeId(657U);
            INST(8U, Opcode::LenArray).s32().Inputs(5U);
        }
        BASIC_BLOCK(8U, 8U, 4U)
        {
            INST(16U, Opcode::SafePoint).Inputs(0U).SrcVregs({0U});
            INST(22U, Opcode::LoadObject).ref().Inputs(0U).TypeId(657U);
            INST(23U, Opcode::LenArray).s32().Inputs(22U);
            INST(17U, Opcode::Compare).b().CC(CC_GE).Inputs(23U, 8U);
            INST(18U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(17U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(19U, Opcode::ReturnVoid);
        }
    }

    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, 8U)
        {
            INST(5U, Opcode::LoadObject).ref().Inputs(0U).TypeId(657U);
            INST(8U, Opcode::LenArray).s32().Inputs(5U);
        }
        BASIC_BLOCK(8U, 8U, 4U)
        {
            INST(16U, Opcode::SafePoint).Inputs(0U, 5U).SrcVregs({0U, VirtualRegister::BRIDGE});
            INST(23U, Opcode::LenArray).s32().Inputs(5U);
            INST(17U, Opcode::Compare).b().CC(CC_GE).Inputs(23U, 8U);
            INST(18U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(17U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(19U, Opcode::ReturnVoid);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Call generally invalidates all heap.
TEST_F(LSETest, MemEscaping)
{
    std::vector<Graph *> equalGraphs = {GetGraph(), CreateEmptyGraph()};
    for (auto graph : equalGraphs) {
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).ref();
            PARAMETER(1U, 1U).s64();
            BASIC_BLOCK(2U, -1L)
            {
                INST(4U, Opcode::StoreObject).s64().Inputs(0U, 1U).TypeId(122U);
                INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 2U});
                INST(6U, Opcode::CallStatic).s64().Inputs({{DataType::REFERENCE, 0U}, {DataType::NO_TYPE, 5U}});
                INST(9U, Opcode::LoadObject).s64().Inputs(0U).TypeId(122U);
                INST(10U, Opcode::Return).s64().Inputs(9U);
            }
        }
    }
    ASSERT_FALSE(equalGraphs[0U]->RunPass<Lse>());
    GraphChecker(equalGraphs[0U]).Check();
    ASSERT_TRUE(GraphComparator().Compare(equalGraphs[0U], equalGraphs[1U]));
}

/**
 * Use a dominated access after elimination
 *     [entry]
 *        |
 *    /--[2]--\
 *   [4]      |
 *    \----->[3]------\
 *            |       |
 *        /->[7]--\   |
 *        \---|   |   |
 *           [5]<-----/
 *            |
 *          [exit]
 */
SRC_GRAPH(ReplaceByDominated, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x2U).s64();
        CONSTANT(2U, 0x0U).s64();
        CONSTANT(6U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(40U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(30U, Opcode::LoadAndInitClass).ref().Inputs(40U).TypeId(0U);
            INST(3U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 0U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(3U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(5U, Opcode::LoadStatic).ref().Inputs(30U).TypeId(83U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 6U, 1U, 5U).SrcVregs({5U, 4U, 3U, 2U, 1U, 0U});
            INST(8U, Opcode::NullCheck).ref().Inputs(5U, 7U);
            INST(11U, Opcode::StoreArray).s8().Inputs(5U, 1U, 6U);
        }
        BASIC_BLOCK(3U, 5U, 7U)
        {
            INST(13U, Opcode::Phi).s32().Inputs({{2U, 0U}, {4U, 1U}});
            INST(15U, Opcode::LoadStatic).ref().Inputs(30U).TypeId(83U);
            INST(20U, Opcode::LoadArray).s32().Inputs(15U, 13U);
            INST(21U, Opcode::Compare).b().CC(CC_EQ).Inputs(20U, 2U);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(21U);
        }
        BASIC_BLOCK(7U, 5U, 7U)
        {
            // Replace by dominated v15 not v5
            INST(33U, Opcode::LoadStatic).ref().Inputs(30U).TypeId(83U);
            INST(38U, Opcode::StoreArray).s32().Inputs(33U, 13U, 6U);
            INST(31U, Opcode::Compare).b().CC(CC_GT).Inputs(6U, 13U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(45U, Opcode::Phi).s32().Inputs({{3U, 20U}, {7U, 6U}});
            INST(46U, Opcode::Return).s32().Inputs(45U);
        }
    }
}

OUT_GRAPH(ReplaceByDominated, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x2U).s64();
        CONSTANT(2U, 0x0U).s64();
        CONSTANT(6U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(40U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(30U, Opcode::LoadAndInitClass).ref().Inputs(40U).TypeId(0U);
            INST(3U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 0U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(3U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(5U, Opcode::LoadStatic).ref().Inputs(30U).TypeId(83U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 6U, 1U, 5U).SrcVregs({5U, 4U, 3U, 2U, 1U, 0U});
            INST(8U, Opcode::NullCheck).ref().Inputs(5U, 7U);
            INST(11U, Opcode::StoreArray).s8().Inputs(5U, 1U, 6U);
        }
        BASIC_BLOCK(3U, 5U, 7U)
        {
            INST(13U, Opcode::Phi).s32().Inputs({{2U, 0U}, {4U, 1U}});
            INST(15U, Opcode::LoadStatic).ref().Inputs(30U).TypeId(83U);
            INST(20U, Opcode::LoadArray).s32().Inputs(15U, 13U);
            INST(21U, Opcode::Compare).b().CC(CC_EQ).Inputs(20U, 2U);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(21U);
        }
        BASIC_BLOCK(7U, 5U, 7U)
        {
            INST(38U, Opcode::StoreArray).s32().Inputs(15U, 13U, 6U);
            INST(31U, Opcode::Compare).b().CC(CC_GT).Inputs(6U, 13U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(45U, Opcode::Phi).s32().Inputs({{3U, 20U}, {7U, 6U}});
            INST(46U, Opcode::Return).s32().Inputs(45U);
        }
    }
}

TEST_F(LSETest, ReplaceByDominated)
{
    src_graph::ReplaceByDominated::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::ReplaceByDominated::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Normal loads and stores can be eliminated after volatile store
TEST_F(LSETest, ReorderableVolatileStore)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);
            INST(3U, Opcode::LoadObject).ref().Inputs(2U).TypeId(152U);
            INST(4U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::StoreObject).ref().Inputs(5U, 3U).TypeId(138U);
            INST(7U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(8U, Opcode::NullCheck).ref().Inputs(0U, 7U);
            INST(9U, Opcode::LoadObject).ref().Inputs(8U).TypeId(152U);
            INST(10U, Opcode::Return).ref().Inputs(9U);
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);
            INST(3U, Opcode::LoadObject).ref().Inputs(2U).TypeId(152U);
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, VirtualRegister::BRIDGE});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::StoreObject).ref().Inputs(5U, 3U).TypeId(138U);
            INST(10U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

TEST_F(LSETest, ReorderableVolatileStoreWithOutBridge)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);
            INST(3U, Opcode::LoadObject).s32().Inputs(2U).TypeId(152U);
            INST(4U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::StoreObject).s32().Inputs(5U, 3U).TypeId(138U);
            INST(7U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(8U, Opcode::NullCheck).ref().Inputs(0U, 7U);
            INST(9U, Opcode::LoadObject).s32().Inputs(8U).TypeId(152U);
            INST(10U, Opcode::Return).s32().Inputs(9U);
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);
            INST(3U, Opcode::LoadObject).s32().Inputs(2U).TypeId(152U);
            INST(4U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::StoreObject).s32().Inputs(5U, 3U).TypeId(138U);
            INST(10U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// v9 and v12 MAY_ALIAS each other. But after elimination of v11 they have NO_ALIAS.
SRC_GRAPH(PhiCandidatesWithUpdatedAA, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f32();
        CONSTANT(1U, 0x2U);
        CONSTANT(3U, 0x0U).f32();
        BASIC_BLOCK(2U, 3U)
        {
            INST(5U, Opcode::SaveState).Inputs(3U, 1U).SrcVregs({1U, 5U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 5U);

            INST(23U, Opcode::SaveState).Inputs(7U).SrcVregs({0U});
            INST(8U, Opcode::NewArray).ref().Inputs(6U, 1U, 23U).TypeId(20U);
            INST(9U, Opcode::StoreArray).ref().Inputs(8U, 1U, 7U);
            INST(10U, Opcode::LoadArray).ref().Inputs(8U, 1U);
            INST(11U, Opcode::StoreObject).f32().Inputs(10U, 0U).TypeId(2726U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(14U, Opcode::Phi).f32().Inputs({{2U, 3U}, {3U, 15U}});
            INST(12U, Opcode::LoadObject).ref().Inputs(10U).TypeId(2740U);
            INST(13U, Opcode::LoadArray).f32().Inputs(12U, 1U);
            INST(15U, Opcode::Add).f32().Inputs(14U, 13U);
            INST(16U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(true).Inputs(3U, 15U);
            INST(19U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0x0U).Inputs(16U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(20U, Opcode::Return).f32().Inputs(15U);
        }
    }
}

OUT_GRAPH(PhiCandidatesWithUpdatedAA, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).f32();
        CONSTANT(1U, 0x2U);
        CONSTANT(3U, 0x0U).f32();
        BASIC_BLOCK(2U, 3U)
        {
            INST(5U, Opcode::SaveState).Inputs(3U, 1U).SrcVregs({1U, 5U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 5U);

            INST(23U, Opcode::SaveState).Inputs(7U).SrcVregs({0U});
            INST(8U, Opcode::NewArray).ref().Inputs(6U, 1U, 23U).TypeId(20U);
            INST(9U, Opcode::StoreArray).ref().Inputs(8U, 1U, 7U);
            INST(11U, Opcode::StoreObject).f32().Inputs(7U, 0U).TypeId(2726U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(14U, Opcode::Phi).f32().Inputs({{2U, 3U}, {3U, 15U}});
            INST(12U, Opcode::LoadObject).ref().Inputs(7U).TypeId(2740U);
            INST(13U, Opcode::LoadArray).f32().Inputs(12U, 1U);
            INST(15U, Opcode::Add).f32().Inputs(14U, 13U);
            INST(16U, Opcode::Cmp).s32().SrcType(DataType::Type::FLOAT32).Fcmpg(true).Inputs(3U, 15U);
            INST(19U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0x0U).Inputs(16U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(20U, Opcode::Return).f32().Inputs(15U);
        }
    }
}

TEST_F(LSETest, PhiCandidatesWithUpdatedAA)
{
    src_graph::PhiCandidatesWithUpdatedAA::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::PhiCandidatesWithUpdatedAA::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/**
 * The test relies on LSE debug internal check. The check aimed to control
 * that none of eliminated instructions are replaced by other eliminated
 * instructions.
 *
 * Here v25 may be erroneously replaced by v14.
 */
TEST_F(LSETest, EliminationOrderMatters)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s64();
        PARAMETER(3U, 3U).s64();
        CONSTANT(4U, 0x0U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(9U, Opcode::StoreArray).s64().Inputs(1U, 4U, 2U);
            // V14 is eliminated due to v9
            INST(14U, Opcode::LoadArray).s64().Inputs(1U, 4U);
            // v19 MAY_ALIAS v9 and v14 therefore pops them from block_heap
            INST(19U, Opcode::StoreArray).s64().Inputs(1U, 3U, 3U);

            INST(22U, Opcode::StoreObject).s64().Inputs(0U, 14U).TypeId(382U);
            // v25 is eliminated due to v22 and should be replaced with v2 because v14 is eliminated too
            INST(25U, Opcode::LoadObject).s64().Inputs(0U).TypeId(382U);
            INST(29U, Opcode::Return).s64().Inputs(25U);
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s64();
        PARAMETER(3U, 3U).s64();
        CONSTANT(4U, 0x0U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(9U, Opcode::StoreArray).s64().Inputs(1U, 4U, 2U);
            INST(19U, Opcode::StoreArray).s64().Inputs(1U, 3U, 3U);

            INST(22U, Opcode::StoreObject).s64().Inputs(0U, 2U).TypeId(382U);
            INST(29U, Opcode::Return).s64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/**
 * Similar to the test above but chain of elimination is across loops.
 *
 * Here v19 may be erroneously replaced by v15.
 */
SRC_GRAPH(EliminationOrderMattersLoops, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(10U, 0x0U).s64();
        CONSTANT(11U, 0xffffffffffffffffU).s64();
        BASIC_BLOCK(5U, 6U)
        {
            INST(12U, Opcode::Cast).s16().SrcType(DataType::INT64).Inputs(11U);
            INST(13U, Opcode::LoadObject).ref().Inputs(0U).TypeId(947545U);
            INST(14U, Opcode::LenArray).s32().Inputs(13U);
        }
        BASIC_BLOCK(6U, 7U, 6U)
        {
            // v15 is eliminated due to v13
            INST(15U, Opcode::LoadObject).ref().Inputs(0U).TypeId(947545U);
            INST(16U, Opcode::StoreArray).s16().Inputs(15U, 14U, 12U);
            INST(17U, Opcode::Compare).b().CC(CC_EQ).Inputs(14U, 10U);
            INST(18U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(17U);
        }
        BASIC_BLOCK(7U, 8U, 7U)
        {
            // v19 is eliminated due to v15 (a valid heap value after the last iteration of previous loop)
            // and should be replaced with v13 because v15 is eliminated too
            INST(19U, Opcode::LoadObject).ref().Inputs(0U).TypeId(947545U);
            INST(20U, Opcode::StoreArray).s16().Inputs(19U, 14U, 12U);
            INST(21U, Opcode::Compare).b().CC(CC_NE).Inputs(14U, 10U);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(21U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(23U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(EliminationOrderMattersLoops, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(10U, 0x0U).s64();
        CONSTANT(11U, 0xffffffffffffffffU).s64();
        BASIC_BLOCK(5U, 6U)
        {
            INST(12U, Opcode::Cast).s16().SrcType(DataType::INT64).Inputs(11U);
            INST(13U, Opcode::LoadObject).ref().Inputs(0U).TypeId(947545U);
            INST(14U, Opcode::LenArray).s32().Inputs(13U);
        }
        BASIC_BLOCK(6U, 7U, 6U)
        {
            INST(16U, Opcode::StoreArray).s16().Inputs(13U, 14U, 12U);
            INST(17U, Opcode::Compare).b().CC(CC_EQ).Inputs(14U, 10U);
            INST(18U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(17U);
        }
        BASIC_BLOCK(7U, 8U, 7U)
        {
            INST(20U, Opcode::StoreArray).s16().Inputs(13U, 14U, 12U);
            INST(21U, Opcode::Compare).b().CC(CC_NE).Inputs(14U, 10U);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(21U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(23U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(LSETest, EliminationOrderMattersLoops)
{
    src_graph::EliminationOrderMattersLoops::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::EliminationOrderMattersLoops::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/*
 * We can eliminate over SafePoints if the reference is listed in arguments
 */
TEST_F(LSETest, EliminationWithSafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::LoadObject).ref().Inputs(0U).TypeId(100U);
            INST(6U, Opcode::StoreObject).ref().Inputs(0U, 3U).TypeId(122U);

            INST(7U, Opcode::SafePoint).Inputs(3U, 0U).SrcVregs({0U, 1U});
            INST(8U, Opcode::LoadObject).ref().Inputs(0U).TypeId(122U);
            INST(9U, Opcode::LoadObject).ref().Inputs(0U).TypeId(100U);
            INST(10U, Opcode::SaveState).Inputs(8U, 9U).SrcVregs({0U, 1U});
            INST(11U, Opcode::NullCheck).ref().Inputs(9U, 10U);
            INST(12U, Opcode::Return).ref().Inputs(8U);
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::LoadObject).ref().Inputs(0U).TypeId(100U);
            INST(6U, Opcode::StoreObject).ref().Inputs(0U, 3U).TypeId(122U);

            INST(7U, Opcode::SafePoint).Inputs(3U, 0U).SrcVregs({0U, 1U});
            INST(10U, Opcode::SaveState).Inputs(3U, 3U).SrcVregs({0U, 1U});
            INST(11U, Opcode::NullCheck).ref().Inputs(3U, 10U);
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/*
 * We should be able to eliminate over SafePoints if the reference listed in
 * arguments was eliminated
 */
TEST_F(LSETest, EliminatedWithSafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::StoreObject).ref().Inputs(0U, 1U).TypeId(122U);
            // v4 would be replaced with v1
            INST(4U, Opcode::LoadObject).ref().Inputs(0U).TypeId(122U);
            // v1 should be alive after replacements
            INST(5U, Opcode::SafePoint).Inputs(0U, 4U).SrcVregs({0U, 1U});
            // v6 would be replaced with v1
            INST(6U, Opcode::LoadObject).ref().Inputs(0U).TypeId(122U);
            INST(7U, Opcode::Return).ref().Inputs(6U);
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::StoreObject).ref().Inputs(0U, 1U).TypeId(122U);
            INST(5U, Opcode::SafePoint).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(7U, Opcode::Return).ref().Inputs(1U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

TEST_F(LSETest, EliminatedWithSafePointNeedBridge)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::LoadObject).ref().Inputs(0U).TypeId(122U);
            INST(5U, Opcode::SafePoint).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::LoadObject).ref().Inputs(0U).TypeId(122U);
            INST(7U, Opcode::Return).ref().Inputs(6U);
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::LoadObject).ref().Inputs(0U).TypeId(122U);
            INST(5U, Opcode::SafePoint).Inputs(0U, 4U).SrcVregs({0U, VirtualRegister::BRIDGE});
            INST(7U, Opcode::Return).ref().Inputs(4U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Not aliased array acceses, since array cannot overlap each other
SRC_GRAPH(SameArrayAccesses, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(5U, 0x1U).s64();
        CONSTANT(9U, 0x0U).s64();
        CONSTANT(32U, 0x2U).s64();
        CONSTANT(58U, 0x3U).s64();
        BASIC_BLOCK(2U, 1U)
        {
            INST(4U, Opcode::LoadObject).ref().TypeId(302U).Inputs(0U);
            INST(8U, Opcode::LoadObject).ref().TypeId(312U).Inputs(0U);

            INST(14U, Opcode::LoadArray).s64().Inputs(8U, 9U);
            INST(22U, Opcode::LoadArray).s64().Inputs(8U, 5U);
            INST(23U, Opcode::Or).s64().Inputs(14U, 22U);
            INST(28U, Opcode::StoreArray).s64().Inputs(4U, 5U, 23U);

            // Same to 14, in spite of possible aliasing of i4 and i8, they
            // have been accessed at different indices
            INST(40U, Opcode::LoadArray).s64().Inputs(8U, 9U);
            INST(48U, Opcode::LoadArray).s64().Inputs(8U, 32U);
            INST(49U, Opcode::Or).s64().Inputs(40U, 48U);
            INST(54U, Opcode::StoreArray).s64().Inputs(4U, 32U, 49U);

            // Same to 14 and 40, in spite of possible aliasing of i4 and i8,
            // they have been accessed at different indices
            INST(66U, Opcode::LoadArray).s64().Inputs(8U, 9U);
            INST(74U, Opcode::LoadArray).s64().Inputs(8U, 58U);
            INST(75U, Opcode::Or).s64().Inputs(66U, 74U);
            INST(80U, Opcode::StoreArray).s64().Inputs(4U, 58U, 75U);

            INST(81U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(SameArrayAccesses, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(5U, 0x1U).s64();
        CONSTANT(9U, 0x0U).s64();
        CONSTANT(32U, 0x2U).s64();
        CONSTANT(58U, 0x3U).s64();
        BASIC_BLOCK(2U, 1U)
        {
            INST(4U, Opcode::LoadObject).ref().TypeId(302U).Inputs(0U);
            INST(8U, Opcode::LoadObject).ref().TypeId(312U).Inputs(0U);

            INST(14U, Opcode::LoadArray).s64().Inputs(8U, 9U);
            INST(22U, Opcode::LoadArray).s64().Inputs(8U, 5U);
            INST(23U, Opcode::Or).s64().Inputs(14U, 22U);
            INST(28U, Opcode::StoreArray).s64().Inputs(4U, 5U, 23U);

            INST(48U, Opcode::LoadArray).s64().Inputs(8U, 32U);
            INST(49U, Opcode::Or).s64().Inputs(14U, 48U);
            INST(54U, Opcode::StoreArray).s64().Inputs(4U, 32U, 49U);

            INST(74U, Opcode::LoadArray).s64().Inputs(8U, 58U);
            INST(75U, Opcode::Or).s64().Inputs(14U, 74U);
            INST(80U, Opcode::StoreArray).s64().Inputs(4U, 58U, 75U);

            INST(81U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(LSETest, SameArrayAccesses)
{
    src_graph::SameArrayAccesses::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::SameArrayAccesses::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Eliminate in case of inlined virtual calls
SRC_GRAPH(OverInlinedVirtualCall, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).i32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::StoreObject).i32().Inputs(1U, 2U).TypeId(122U);
            INST(4U, Opcode::SaveState).Inputs(1U).SrcVregs({0U});
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(6U, Opcode::CallVirtual).v0id().InputsAutoType(1U, 2U, 4U).Inlined();
            INST(7U, Opcode::LoadObject).i32().Inputs(1U).TypeId(122U);
            INST(8U, Opcode::ReturnInlined).v0id().Inputs(4U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(9U, Opcode::CallVirtual).v0id().InputsAutoType(1U, 2U, 4U).Inlined();
            INST(10U, Opcode::LoadObject).i32().Inputs(1U).TypeId(122U);
            INST(11U, Opcode::ReturnInlined).v0id().Inputs(4U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(12U, Opcode::LoadObject).i32().Inputs(1U).TypeId(122U);
            INST(13U, Opcode::Return).i32().Inputs(12U);
        }
    }
}

OUT_GRAPH(OverInlinedVirtualCall, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).i32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::StoreObject).i32().Inputs(1U, 2U).TypeId(122U);
            INST(4U, Opcode::SaveState).Inputs(1U).SrcVregs({0U});
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(6U, Opcode::CallVirtual).v0id().InputsAutoType(1U, 2U, 4U).Inlined();
            INST(8U, Opcode::ReturnInlined).v0id().Inputs(4U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(9U, Opcode::CallVirtual).v0id().InputsAutoType(1U, 2U, 4U).Inlined();
            INST(11U, Opcode::ReturnInlined).v0id().Inputs(4U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(13U, Opcode::Return).i32().Inputs(2U);
        }
    }
}

TEST_F(LSETest, OverInlinedVirtualCall)
{
    src_graph::OverInlinedVirtualCall::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::OverInlinedVirtualCall::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Counter case with aliased store
TEST_F(LSETest, SameArrayAccessesWithOverwrite)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(5U, 0x1U).s64();
        CONSTANT(9U, 0x0U).s64();
        CONSTANT(32U, 0x2U).s64();
        CONSTANT(58U, 0x3U).s64();
        BASIC_BLOCK(2U, 1U)
        {
            INST(4U, Opcode::LoadObject).ref().TypeId(302U).Inputs(0U);
            INST(8U, Opcode::LoadObject).ref().TypeId(312U).Inputs(0U);

            INST(14U, Opcode::LoadArray).s64().Inputs(8U, 9U);
            INST(22U, Opcode::LoadArray).s64().Inputs(8U, 5U);
            INST(23U, Opcode::Or).s64().Inputs(14U, 22U);
            INST(28U, Opcode::StoreArray).s64().Inputs(4U, 9U, 23U);

            // Same to 14 but i4 and i8 may be aliased and v28 may overwrite
            // previous load
            INST(40U, Opcode::LoadArray).s64().Inputs(8U, 9U);
            INST(48U, Opcode::LoadArray).s64().Inputs(8U, 32U);
            INST(49U, Opcode::Or).s64().Inputs(40U, 48U);
            INST(54U, Opcode::StoreArray).s64().Inputs(4U, 32U, 49U);

            INST(81U, Opcode::ReturnVoid).v0id();
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Counter case with unknown index
TEST_F(LSETest, SameArrayAccessesWithUnknownIndices)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        CONSTANT(5U, 0x1U).s64();
        CONSTANT(32U, 0x2U).s64();
        CONSTANT(58U, 0x3U).s64();
        BASIC_BLOCK(2U, 1U)
        {
            INST(4U, Opcode::LoadObject).ref().TypeId(302U).Inputs(0U);
            INST(8U, Opcode::LoadObject).ref().TypeId(312U).Inputs(0U);

            INST(14U, Opcode::LoadArray).s64().Inputs(8U, 1U);
            INST(22U, Opcode::LoadArray).s64().Inputs(8U, 5U);
            INST(23U, Opcode::Or).s64().Inputs(14U, 22U);
            INST(28U, Opcode::StoreArray).s64().Inputs(4U, 5U, 23U);

            // Same to v14 but index is unknown, it may be equal to v5
            INST(40U, Opcode::LoadArray).s64().Inputs(8U, 1U);
            INST(48U, Opcode::LoadArray).s64().Inputs(8U, 32U);
            INST(49U, Opcode::Or).s64().Inputs(40U, 48U);
            INST(54U, Opcode::StoreArray).s64().Inputs(4U, 32U, 49U);

            INST(81U, Opcode::ReturnVoid).v0id();
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// A store does not dominate a load
TEST_F(LSETest, NoDominationHere)
{
    std::vector<Graph *> equalGraphs = {GetGraph(), CreateEmptyGraph()};
    for (auto graph : equalGraphs) {
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).ref();
            PARAMETER(1U, 1U).s64();
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(2U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 1U);
                INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
            }
            BASIC_BLOCK(4U, 3U)
            {
                INST(8U, Opcode::StoreArray).s64().Inputs(0U, 1U, 1U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(14U, Opcode::LoadArray).s64().Inputs(0U, 1U);
                INST(15U, Opcode::Return).s64().Inputs(14U);
            }
        }
    }
    ASSERT_FALSE(equalGraphs[0U]->RunPass<Lse>());
    GraphChecker(equalGraphs[0U]).Check();
    ASSERT_TRUE(GraphComparator().Compare(equalGraphs[0U], equalGraphs[1U]));
}

SRC_GRAPH(EliminateMonitoredStores, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(15U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::StoreObject).s64().Inputs(0U, 1U).TypeId(243U);
            INST(4U, Opcode::Monitor).v0id().Entry().Inputs(0U, 15U);
            INST(5U, Opcode::Add).s64().Inputs(1U, 2U);
            INST(6U, Opcode::StoreObject).s64().Inputs(0U, 5U).TypeId(243U);
            INST(7U, Opcode::Add).s64().Inputs(5U, 2U);
            INST(8U, Opcode::StoreObject).s64().Inputs(0U, 7U).TypeId(243U);
            INST(16U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(9U, Opcode::Monitor).v0id().Exit().Inputs(0U, 16U);
            INST(10U, Opcode::Add).s64().Inputs(7U, 2U);
            INST(11U, Opcode::StoreObject).s64().Inputs(0U, 10U).TypeId(243U);
            INST(12U, Opcode::Add).s64().Inputs(10U, 2U);
            INST(13U, Opcode::StoreObject).s64().Inputs(0U, 12U).TypeId(243U);
            INST(14U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(EliminateMonitoredStores, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(15U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::StoreObject).s64().Inputs(0U, 1U).TypeId(243U);
            INST(4U, Opcode::Monitor).v0id().Entry().Inputs(0U, 15U);
            INST(5U, Opcode::Add).s64().Inputs(1U, 2U);
            INST(7U, Opcode::Add).s64().Inputs(5U, 2U);
            INST(8U, Opcode::StoreObject).s64().Inputs(0U, 7U).TypeId(243U);
            INST(16U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(9U, Opcode::Monitor).v0id().Exit().Inputs(0U, 16U);
            INST(10U, Opcode::Add).s64().Inputs(7U, 2U);
            INST(12U, Opcode::Add).s64().Inputs(10U, 2U);
            INST(13U, Opcode::StoreObject).s64().Inputs(0U, 12U).TypeId(243U);
            INST(14U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(LSETest, EliminateMonitoredStores)
{
    src_graph::EliminateMonitoredStores::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::EliminateMonitoredStores::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

TEST_F(LSETest, EliminateMonitoredLoads)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(11U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(1U, Opcode::LoadObject).s64().Inputs(0U).TypeId(243U);
            INST(2U, Opcode::LoadObject).s64().Inputs(0U).TypeId(243U);
            INST(3U, Opcode::Monitor).v0id().Entry().Inputs(0U, 11U);
            INST(4U, Opcode::LoadObject).s64().Inputs(0U).TypeId(243U);
            INST(5U, Opcode::LoadObject).s64().Inputs(0U).TypeId(243U);
            INST(12U, Opcode::SaveState).Inputs(0U, 1U, 2U, 4U, 5U).SrcVregs({0U, 1U, 2U, 4U, 5U});
            INST(6U, Opcode::Monitor).v0id().Exit().Inputs(0U, 12U);
            INST(7U, Opcode::LoadObject).s64().Inputs(0U).TypeId(243U);
            INST(8U, Opcode::LoadObject).s64().Inputs(0U).TypeId(243U);
            INST(13U, Opcode::SaveState).Inputs(0U, 1U, 2U, 4U, 5U, 7U, 8U).SrcVregs({0U, 1U, 2U, 4U, 5U, 7U, 8U});
            INST(9U, Opcode::CallStatic).b().InputsAutoType(1U, 2U, 4U, 5U, 7U, 8U, 13U);
            INST(10U, Opcode::Return).b().Inputs(9U);
        }
    }
    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(11U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(1U, Opcode::LoadObject).s64().Inputs(0U).TypeId(243U);
            INST(3U, Opcode::Monitor).v0id().Entry().Inputs(0U, 11U);
            INST(4U, Opcode::LoadObject).s64().Inputs(0U).TypeId(243U);
            INST(12U, Opcode::SaveState).Inputs(0U, 1U, 1U, 4U, 4U).SrcVregs({0U, 1U, 2U, 4U, 5U});
            INST(6U, Opcode::Monitor).v0id().Exit().Inputs(0U, 12U);
            INST(13U, Opcode::SaveState).Inputs(0U, 1U, 1U, 4U, 4U, 4U, 4U).SrcVregs({0U, 1U, 2U, 4U, 5U, 7U, 8U});
            INST(9U, Opcode::CallStatic).b().InputsAutoType(1U, 1U, 4U, 4U, 4U, 4U, 13U);
            INST(10U, Opcode::Return).b().Inputs(9U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Stores that cannot be eliminated due to monitors
TEST_F(LSETest, NotEliminableMonitoredStores)
{
    std::vector<Graph *> equalGraphs = {GetGraph(), CreateEmptyGraph()};
    for (auto graph : equalGraphs) {
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).ref();
            PARAMETER(1U, 1U).s64();
            BASIC_BLOCK(2U, -1L)
            {
                INST(9U, Opcode::StoreObject).s64().Inputs(0U, 1U).TypeId(243U);
                INST(7U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
                INST(2U, Opcode::Monitor).v0id().Entry().Inputs(0U, 7U);
                INST(5U, Opcode::StoreObject).s64().Inputs(0U, 1U).TypeId(243U);
                INST(8U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
                INST(6U, Opcode::Monitor).v0id().Exit().Inputs(0U, 8U);
                INST(10U, Opcode::ReturnVoid).v0id();
            }
        }
    }
    ASSERT_TRUE(equalGraphs[0U]->RunPass<MonitorAnalysis>());
    ASSERT_FALSE(equalGraphs[0U]->RunPass<Lse>());
    GraphChecker(equalGraphs[0U]).Check();
    ASSERT_TRUE(GraphComparator().Compare(equalGraphs[0U], equalGraphs[1U]));
}

TEST_F(LSETest, NotEliminableMonitoredLoadStore)
{
    std::vector<Graph *> equalGraphs = {GetGraph(), CreateEmptyGraph()};
    for (auto graph : equalGraphs) {
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).ref();
            BASIC_BLOCK(2U, -1L)
            {
                INST(9U, Opcode::LoadObject).s64().Inputs(0U).TypeId(243U);
                INST(7U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
                INST(2U, Opcode::Monitor).v0id().Entry().Inputs(0U, 7U);
                INST(5U, Opcode::StoreObject).s64().Inputs(0U, 9U).TypeId(243U);
                INST(8U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
                INST(6U, Opcode::Monitor).v0id().Exit().Inputs(0U, 8U);
                INST(10U, Opcode::ReturnVoid).v0id();
            }
        }
    }
    ASSERT_TRUE(equalGraphs[0U]->RunPass<MonitorAnalysis>());
    ASSERT_FALSE(equalGraphs[0U]->RunPass<Lse>());
    GraphChecker(equalGraphs[0U]).Check();
    ASSERT_TRUE(GraphComparator().Compare(equalGraphs[0U], equalGraphs[1U]));
}

/// Inner loop overwrites outer loop reference. No elimination
// CC-OFFNXT(huge_method, G.FUN.01) graph creation
SRC_GRAPH(InnerOverwrite, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        CONSTANT(8U, 0x0U).s64();
        CONSTANT(53U, 0x1U).s64();
        BASIC_BLOCK(2U, 8U)
        {
            INST(4U, Opcode::LoadObject).ref().Inputs(0U).TypeId(194U);
            INST(7U, Opcode::LenArray).s32().Inputs(4U);
        }
        // For (v11 = 0, v11 < lenarr(v4), v11++)
        BASIC_BLOCK(8U, 3U, 4U)
        {
            INST(11U, Opcode::Phi).s32().Inputs({{2U, 8U}, {5U, 63U}});
            INST(12U, Opcode::Phi).s32().Inputs({{2U, 8U}, {5U, 62U}});
            INST(17U, Opcode::Cmp).s32().Inputs(11U, 7U);
            INST(18U, Opcode::Compare).b().CC(CC_GE).Inputs(17U, 8U);
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(18U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(22U, Opcode::LenArray).s32().Inputs(1U);
            INST(65U, Opcode::Cmp).s32().Inputs(8U, 22U);
            INST(66U, Opcode::Compare).b().CC(CC_GE).Inputs(65U, 8U);
            INST(67U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(66U);
        }
        //     For (v28 = 0, v28 < lenarr(v1), v28++)
        //         v4 = v28[v4[v11]]
        BASIC_BLOCK(6U, 5U, 6U)
        {
            INST(28U, Opcode::Phi).s32().Inputs({{4U, 8U}, {6U, 52U}});
            INST(38U, Opcode::LoadObject).ref().Inputs(0U).TypeId(194U);  // Cannot eliminate due to v51
            INST(43U, Opcode::LoadArray).s32().Inputs(38U, 11U);
            INST(48U, Opcode::LoadArray).ref().Inputs(1U, 43U);
            INST(51U, Opcode::StoreObject).ref().Inputs(0U, 48U).TypeId(194U);
            INST(52U, Opcode::Add).s32().Inputs(28U, 53U);
            INST(33U, Opcode::Cmp).s32().Inputs(52U, 22U);
            INST(34U, Opcode::Compare).b().CC(CC_GE).Inputs(33U, 8U);
            INST(35U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(34U);
        }
        //     v12 += v4[v11]
        BASIC_BLOCK(5U, 8U)
        {
            INST(56U, Opcode::LoadObject).ref().Inputs(0U).TypeId(194U);  // Cannot eliminate due to v51
            INST(61U, Opcode::LoadArray).s32().Inputs(56U, 11U);
            INST(62U, Opcode::Add).s32().Inputs(61U, 12U);
            INST(63U, Opcode::Add).s32().Inputs(11U, 53U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(64U, Opcode::Return).s32().Inputs(12U);
        }
    }
}

TEST_F(LSETest, InnerOverwrite)
{
    src_graph::InnerOverwrite::CREATE(GetGraph());
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>(false));
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Outer loop overwrites inner loop reference.
// CC-OFFNXT(huge_method, G.FUN.01) graph creation
SRC_GRAPH(OuterOverwrite, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        CONSTANT(8U, 0x0U).s64();
        CONSTANT(46U, 0x1U).s64();
        BASIC_BLOCK(2U, 8U)
        {
            INST(4U, Opcode::LoadObject).ref().Inputs(0U).TypeId(194U);
            INST(7U, Opcode::LenArray).s32().Inputs(4U);
        }
        // For (v11 = 0, v11 < lenarr(v4), v11++)
        BASIC_BLOCK(8U, 3U, 4U)
        {
            INST(11U, Opcode::Phi).s32().Inputs({{2U, 8U}, {5U, 55U}});
            INST(12U, Opcode::Phi).s32().Inputs({{2U, 8U}, {5U, 62U}});
            INST(17U, Opcode::Cmp).s32().Inputs(11U, 7U);
            INST(18U, Opcode::Compare).b().CC(CC_GE).Inputs(17U, 8U);
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(18U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(57U, Opcode::Cmp).s32().Inputs(8U, 7U);
            INST(58U, Opcode::Compare).b().CC(CC_GE).Inputs(57U, 8U);
            INST(59U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(58U);
        }
        //     For (v28 = 0, v28 < lenarr(v4), v28++)
        //         v12 += v4[v28]
        BASIC_BLOCK(6U, 5U, 6U)
        {
            INST(26U, Opcode::Phi).s32().Inputs({{4U, 12U}, {6U, 44U}});
            INST(28U, Opcode::Phi).s32().Inputs({{4U, 8U}, {6U, 45U}});
            INST(38U, Opcode::LoadObject).ref().Inputs(0U).TypeId(194U);
            INST(43U, Opcode::LoadArray).s32().Inputs(38U, 28U);
            INST(44U, Opcode::Add).s32().Inputs(43U, 26U);
            INST(45U, Opcode::Add).s32().Inputs(28U, 46U);
            INST(33U, Opcode::Cmp).s32().Inputs(45U, 7U);
            INST(34U, Opcode::Compare).b().CC(CC_GE).Inputs(33U, 8U);
            INST(35U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(34U);
        }
        //     v4 = v1[v11]
        BASIC_BLOCK(5U, 8U)
        {
            INST(62U, Opcode::Phi).s32().Inputs({{6U, 44U}, {4U, 12U}});
            INST(51U, Opcode::LoadArray).ref().Inputs(1U, 11U);
            INST(54U, Opcode::StoreObject).ref().Inputs(0U, 51U).TypeId(194U);
            INST(55U, Opcode::Add).s32().Inputs(11U, 46U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(56U, Opcode::Return).s32().Inputs(12U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
OUT_GRAPH(OuterOverwrite, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        CONSTANT(8U, 0x0U).s64();
        CONSTANT(46U, 0x1U).s64();
        BASIC_BLOCK(2U, 8U)
        {
            INST(4U, Opcode::LoadObject).ref().Inputs(0U).TypeId(194U);
            INST(7U, Opcode::LenArray).s32().Inputs(4U);
        }
        // For (v11 = 0, v11 < lenarr(v4), v11++)
        BASIC_BLOCK(8U, 3U, 4U)
        {
            INST(11U, Opcode::Phi).s32().Inputs({{2U, 8U}, {5U, 55U}});
            INST(12U, Opcode::Phi).s32().Inputs({{2U, 8U}, {5U, 62U}});
            INST(13U, Opcode::Phi).ref().Inputs({{2U, 4U}, {5U, 51U}});
            INST(17U, Opcode::Cmp).s32().Inputs(11U, 7U);
            INST(18U, Opcode::Compare).b().CC(CC_GE).Inputs(17U, 8U);
            INST(19U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(18U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(57U, Opcode::Cmp).s32().Inputs(8U, 7U);
            INST(58U, Opcode::Compare).b().CC(CC_GE).Inputs(57U, 8U);
            INST(59U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(58U);
        }
        //     For (v28 = 0, v28 < lenarr(v4), v28++)
        //         v12 += v4[v28]
        BASIC_BLOCK(6U, 5U, 6U)
        {
            INST(26U, Opcode::Phi).s32().Inputs({{4U, 12U}, {6U, 44U}});
            INST(28U, Opcode::Phi).s32().Inputs({{4U, 8U}, {6U, 45U}});
            INST(43U, Opcode::LoadArray).s32().Inputs(13U, 28U);
            INST(44U, Opcode::Add).s32().Inputs(43U, 26U);
            INST(45U, Opcode::Add).s32().Inputs(28U, 46U);
            INST(33U, Opcode::Cmp).s32().Inputs(45U, 7U);
            INST(34U, Opcode::Compare).b().CC(CC_GE).Inputs(33U, 8U);
            INST(35U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(34U);
        }
        //     v4 = v1[v11]
        BASIC_BLOCK(5U, 8U)
        {
            INST(62U, Opcode::Phi).s32().Inputs({{6U, 44U}, {4U, 12U}});
            INST(51U, Opcode::LoadArray).ref().Inputs(1U, 11U);
            INST(54U, Opcode::StoreObject).ref().Inputs(0U, 51U).TypeId(194U);
            INST(55U, Opcode::Add).s32().Inputs(11U, 46U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(56U, Opcode::Return).s32().Inputs(12U);
        }
    }
}

TEST_F(LSETest, OuterOverwrite)
{
    src_graph::OuterOverwrite::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::OuterOverwrite::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Invalidating phi candidates
TEST_F(LSETest, PhiCandOverCall)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(7U, 0x0U).s64();
        CONSTANT(28U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::LoadObject).ref().Inputs(0U).TypeId(130U);
            INST(3U, Opcode::LoadObject).ref().Inputs(2U).TypeId(242U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(30U, Opcode::Cmp).s32().Inputs(7U, 4U);
            INST(31U, Opcode::Compare).b().CC(CC_GE).Inputs(30U, 7U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        // For (v10 = 0, v10 < lenarr(v3), ++v10)
        //     v11 += v3[v10]
        BASIC_BLOCK(4U, 3U, 4U)
        {
            INST(10U, Opcode::Phi).s32().Inputs({{2U, 7U}, {4U, 27U}});
            INST(11U, Opcode::Phi).s32().Inputs({{2U, 7U}, {4U, 26U}});
            INST(40U, Opcode::SaveState).NoVregs();
            INST(18U, Opcode::CallStatic).v0id().InputsAutoType(2U, 40U);
            // Can't eliminated v19 because array may be overwritted in v4
            INST(19U, Opcode::LoadObject).ref().Inputs(0U).TypeId(130U);
            INST(20U, Opcode::LoadObject).ref().Inputs(19U).TypeId(242U);

            INST(25U, Opcode::LoadArray).s32().Inputs(20U, 10U);

            INST(26U, Opcode::Add).s32().Inputs(25U, 11U);
            INST(27U, Opcode::Add).s32().Inputs(10U, 28U);
            INST(16U, Opcode::Compare).b().CC(CC_GE).Inputs(27U, 4U);
            INST(17U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(16U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(35U, Opcode::Phi).s32().Inputs({{4U, 26U}, {2U, 7U}});
            INST(29U, Opcode::Return).s32().Inputs(35U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Not dominated volatile load
TEST_F(LSETest, NotDominatedVolatileLoad)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(7U, 0x0U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::LoadObject).s32().Inputs(0U).TypeId(152U);
            INST(6U, Opcode::Compare).b().CC(CC_EQ).Inputs(2U, 7U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(11U, Opcode::LoadObject).s32().Volatile().Inputs(0U).TypeId(138U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(12U, Opcode::Phi).s32().Inputs({{2U, 1U}, {4U, 11U}});
            INST(14U, Opcode::SaveState).Inputs(2U, 1U, 0U, 12U, 5U).SrcVregs({4U, 3U, 2U, 1U, 0U});
            INST(15U, Opcode::NullCheck).ref().Inputs(0U, 14U);
            // v16 is not eliminable due to volatile load v11
            INST(16U, Opcode::LoadObject).s32().Inputs(0U).TypeId(152U);
            INST(17U, Opcode::Add).s32().Inputs(16U, 12U);
            INST(18U, Opcode::Return).s32().Inputs(17U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// If we eliminate load over safepoint, check that it is correctly bridged
TEST_F(LSETest, LoadsWithSafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::LoadObject).ref().Inputs(0U).TypeId(100U);
            INST(6U, Opcode::StoreObject).ref().Inputs(0U, 3U).TypeId(122U);

            INST(7U, Opcode::SafePoint).Inputs(0U).SrcVregs({0U});
            // Eliminable because of v6 but v6 can be relocated by GC
            INST(8U, Opcode::LoadObject).ref().Inputs(0U).TypeId(122U);
            // Eliminable because of v3 but v3 can be relocated by GC
            INST(9U, Opcode::LoadObject).ref().Inputs(0U).TypeId(100U);
            INST(10U, Opcode::SaveState).Inputs(8U).SrcVregs({0U});
            INST(11U, Opcode::NullCheck).ref().Inputs(9U, 10U);
            INST(12U, Opcode::Return).ref().Inputs(8U);
        }
    }

    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::LoadObject).ref().Inputs(0U).TypeId(100U);
            INST(6U, Opcode::StoreObject).ref().Inputs(0U, 3U).TypeId(122U);

            INST(7U, Opcode::SafePoint).Inputs(0U, 3U).SrcVregs({0U, VirtualRegister::BRIDGE});
            INST(10U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(11U, Opcode::NullCheck).ref().Inputs(3U, 10U);
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Pool constants can be relocated as well and should be bridged.
TEST_F(LSETest, RelocatablePoolConstants)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::SaveState).NoVregs();
            INST(1U, Opcode::LoadAndInitClass).ref().Inputs(0U);

            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::LoadString).ref().Inputs(2U).TypeId(42U);
            INST(4U, Opcode::StoreStatic).ref().Inputs(1U, 3U);

            INST(7U, Opcode::SafePoint).NoVregs();

            INST(8U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::LoadString).ref().Inputs(8U).TypeId(42U);
            INST(10U, Opcode::StoreStatic).ref().Inputs(1U, 9U);

            INST(11U, Opcode::ReturnVoid);
        }
    }

    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::SaveState).NoVregs();
            INST(1U, Opcode::LoadAndInitClass).ref().Inputs(0U);

            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::LoadString).ref().Inputs(2U).TypeId(42U);
            INST(4U, Opcode::StoreStatic).ref().Inputs(1U, 3U);

            INST(7U, Opcode::SafePoint).Inputs(3U).SrcVregs({VirtualRegister::BRIDGE});

            INST(10U, Opcode::StoreStatic).ref().Inputs(1U, 3U);

            INST(11U, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// If we replace load in loop by value stored before loop, check that it's correctly bridged in any loop safepoints
TEST_F(LSETest, StoreAndLoadLoopWithSafepoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, 3U)
        {
            INST(6U, Opcode::StoreObject).ref().Inputs(0U, 1U).TypeId(122U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(7U, Opcode::LoadObject).ref().Inputs(0U).TypeId(122U);
            INST(8U, Opcode::SafePoint).Inputs(0U).SrcVregs({0U});
            INST(9U, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::Return).ref().Inputs(7U);
        }
    }

    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, 3U)
        {
            INST(6U, Opcode::StoreObject).ref().Inputs(0U, 1U).TypeId(122U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(8U, Opcode::SafePoint).Inputs(0U, 1U).SrcVregs({0U, VirtualRegister::BRIDGE});
            INST(9U, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0U).Inputs(1U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::Return).ref().Inputs(1U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// If we hoist load from loop, check that it's correctly bridged in any loop safepoints
TEST_F(LSETest, HoistFromLoopWithSafepoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, 3U) {}
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(7U, Opcode::LoadObject).ref().Inputs(0U).TypeId(122U);
            INST(8U, Opcode::SafePoint).Inputs(0U).SrcVregs({0U});
            INST(9U, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::Return).ref().Inputs(7U);
        }
    }

    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, 3U)
        {
            INST(7U, Opcode::LoadObject).ref().Inputs(0U).TypeId(122U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(8U, Opcode::SafePoint).Inputs(0U, 7U).SrcVregs({0U, VirtualRegister::BRIDGE});
            INST(9U, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::Return).ref().Inputs(7U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

TEST_F(LSETest, HoistFromLoopInRPO)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, 3U) {}
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(7U, Opcode::LoadObject).ref().Inputs(0U).TypeId(122U);
            INST(11U, Opcode::LoadObject).ref().Inputs(7U).TypeId(122U);
            INST(8U, Opcode::LoadObject).ref().Inputs(11U).TypeId(122U);
            INST(12U, Opcode::LoadObject).ref().Inputs(8U).TypeId(122U);
            INST(9U, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::Return).ref().Inputs(12U);
        }
    }

    Graph *graphLsed = CreateEmptyGraph();
    GRAPH(graphLsed)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, 3U)
        {
            INST(7U, Opcode::LoadObject).ref().Inputs(0U).TypeId(122U);
            INST(11U, Opcode::LoadObject).ref().Inputs(7U).TypeId(122U);
            INST(8U, Opcode::LoadObject).ref().Inputs(11U).TypeId(122U);
            INST(12U, Opcode::LoadObject).ref().Inputs(8U).TypeId(122U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(9U, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::Return).ref().Inputs(12U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Store that is not read, but overwritten on all paths is removed
SRC_GRAPH(RemoveShadowedStore, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(8U, 0x8U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(7U, Opcode::StoreArray).s32().Inputs(0U, 2U, 1U);
            INST(9U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 8U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(21U, Opcode::Add).s32().Inputs(1U, 1U);
            INST(17U, Opcode::StoreArray).s32().Inputs(0U, 2U, 21U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(22U, Opcode::Mul).s32().Inputs(1U, 1U);
            INST(18U, Opcode::StoreArray).s32().Inputs(0U, 2U, 22U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(23U, Opcode::Return).s32().Inputs(2U);
        }
    }
}

OUT_GRAPH(RemoveShadowedStore, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(8U, 0x8U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(9U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 8U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(21U, Opcode::Add).s32().Inputs(1U, 1U);
            INST(17U, Opcode::StoreArray).s32().Inputs(0U, 2U, 21U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(22U, Opcode::Mul).s32().Inputs(1U, 1U);
            INST(18U, Opcode::StoreArray).s32().Inputs(0U, 2U, 22U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(23U, Opcode::Return).s32().Inputs(2U);
        }
    }
}

TEST_F(LSETest, RemoveShadowedStore)
{
    src_graph::RemoveShadowedStore::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::RemoveShadowedStore::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Store that is not read, but not overwritten on all paths is not removed
TEST_F(LSETest, DontRemoveStoreIfPathWithoutShadowExists)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(8U, 0x8U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(7U, Opcode::StoreArray).s32().Inputs(0U, 2U, 1U);
            INST(9U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 8U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(21U, Opcode::Add).s32().Inputs(1U, 1U);
            INST(17U, Opcode::StoreArray).s32().Inputs(0U, 2U, 21U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(23U, Opcode::Return).s32().Inputs(2U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

SRC_GRAPH(DISABLED_ShadowInInnerLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(8U, 0x8U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(7U, Opcode::StoreArray).s32().Inputs(0U, 2U, 1U);
            INST(9U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 8U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(21U, Opcode::Add).s32().Inputs(1U, 1U);
            INST(17U, Opcode::StoreArray).s32().Inputs(0U, 2U, 21U);
        }
        BASIC_BLOCK(4U, 5U, 4U)
        {
            INST(6U, Opcode::Phi).s32().Inputs({{2U, 1U}, {4U, 22U}});
            INST(22U, Opcode::Add).s32().Inputs(6U, 1U);
            INST(18U, Opcode::StoreArray).s32().Inputs(0U, 2U, 22U);
            INST(11U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 8U);
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(23U, Opcode::Return).s32().Inputs(2U);
        }
    }
}

OUT_GRAPH(DISABLED_ShadowInInnerLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(8U, 0x8U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(9U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 8U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(21U, Opcode::Add).s32().Inputs(1U, 1U);
            INST(17U, Opcode::StoreArray).s32().Inputs(0U, 2U, 21U);
        }
        BASIC_BLOCK(4U, 5U, 4U)
        {
            INST(6U, Opcode::Phi).s32().Inputs({{2U, 1U}, {4U, 22U}});
            INST(22U, Opcode::Add).s32().Inputs(6U, 1U);
            INST(18U, Opcode::StoreArray).s32().Inputs(0U, 2U, 22U);
            INST(11U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 8U);
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(23U, Opcode::Return).s32().Inputs(2U);
        }
    }
}

TEST_F(LSETest, DISABLED_ShadowInInnerLoop)
{
    src_graph::DISABLED_ShadowInInnerLoop::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::DISABLED_ShadowInInnerLoop::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Stores are not removed from loops, unless they're shadowed in the same loop
SRC_GRAPH(ShadowedStoresInLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(8U, 0x8U).s64();
        BASIC_BLOCK(2U, 3U) {}
        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(7U, Opcode::Phi).s32().Inputs({{2U, 1U}, {3U, 21U}});
            INST(21U, Opcode::Add).s32().Inputs(7U, 1U);
            INST(17U, Opcode::StoreArray).s32().Inputs(0U, 2U, 21U);
            INST(9U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 8U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, 5U, 4U)
        {
            INST(27U, Opcode::Phi).s32().Inputs({{3U, 1U}, {4U, 21U}});
            INST(35U, Opcode::StoreArray).s32().Inputs(0U, 2U, 27U);
            INST(41U, Opcode::Mul).s32().Inputs(27U, 1U);
            INST(37U, Opcode::StoreArray).s32().Inputs(0U, 2U, 41U);
            INST(30U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(28U, Opcode::Phi).s32().Inputs({{4U, 1U}, {6U, 41U}});
            INST(36U, Opcode::StoreArray).s32().Inputs(0U, 2U, 28U);
        }
        BASIC_BLOCK(6U, 7U, 5U)
        {
            INST(42U, Opcode::Mul).s32().Inputs(27U, 1U);
            INST(38U, Opcode::StoreArray).s32().Inputs(0U, 2U, 42U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(23U, Opcode::Return).s32().Inputs(2U);
        }
    }
}

OUT_GRAPH(ShadowedStoresInLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(8U, 0x8U).s64();
        BASIC_BLOCK(2U, 3U) {}
        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(7U, Opcode::Phi).s32().Inputs({{2U, 1U}, {3U, 21U}});
            INST(21U, Opcode::Add).s32().Inputs(7U, 1U);
            INST(17U, Opcode::StoreArray).s32().Inputs(0U, 2U, 21U);
            INST(9U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 8U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, 5U, 4U)
        {
            INST(27U, Opcode::Phi).s32().Inputs({{3U, 1U}, {4U, 21U}});
            INST(41U, Opcode::Mul).s32().Inputs(27U, 1U);
            INST(37U, Opcode::StoreArray).s32().Inputs(0U, 2U, 41U);
            INST(30U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(28U, Opcode::Phi).s32().Inputs({{4U, 1U}, {6U, 41U}});
        }
        BASIC_BLOCK(6U, 7U, 5U)
        {
            INST(42U, Opcode::Mul).s32().Inputs(27U, 1U);
            INST(38U, Opcode::StoreArray).s32().Inputs(0U, 2U, 42U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(23U, Opcode::Return).s32().Inputs(2U);
        }
    }
}

TEST_F(LSETest, ShadowedStoresInLoop)
{
    src_graph::ShadowedStoresInLoop::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::ShadowedStoresInLoop::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Store that can be read is not removed
TEST_F(LSETest, DontRemoveShadowedStoreIfRead)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        PARAMETER(3U, 3U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::StoreArray).u32().Inputs(0U, 2U, 1U);
            INST(8U, Opcode::NullCheck).ref().Inputs(3U);
            INST(10U, Opcode::Add).s32().Inputs(1U, 1U);
            INST(11U, Opcode::StoreArray).u32().Inputs(0U, 2U, 10U);
            INST(13U, Opcode::ReturnVoid).v0id();
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Check that shadows store search correctly handles irreducible loops
TEST_F(LSETest, ShadowStoreWithIrreducibleLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        PARAMETER(3U, 3U).s32();
        PARAMETER(4U, 4U).ref();
        CONSTANT(5U, 0x2aU).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(7U, Opcode::StoreArray).s32().Inputs(4U, 2U, 1U);
            INST(8U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(4U, 5U, 7U)
        {
            INST(10U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0U).Inputs(1U);
        }
        BASIC_BLOCK(5U, 6U, 9U)
        {
            INST(20U, Opcode::Phi).s32().Inputs({{4U, 5U}, {7U, 11U}});
            INST(26U, Opcode::Mul).s32().Inputs(20U, 5U);
            INST(28U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(30U, Opcode::SaveState).Inputs(4U).SrcVregs({0U});
        }
        BASIC_BLOCK(7U, 5U, 9U)
        {
            INST(11U, Opcode::Phi).s32().Inputs({{4U, 5U}, {6U, 26U}});
            INST(19U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 9U)
        {
            INST(12U, Opcode::StoreArray).s32().Inputs(4U, 2U, 3U);
        }
        BASIC_BLOCK(9U, -1L)
        {
            INST(34U, Opcode::Phi).s32().Inputs({{3U, 1U}, {5U, 2U}, {7U, 3U}});
            INST(35U, Opcode::Return).s32().Inputs(34U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Do not eliminate loads over runtime calls due to GC relocations
TEST_F(LSETest, LoadsWithRuntimeCalls)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::LoadObject).ref().Inputs(0U).TypeId(100U);
            INST(6U, Opcode::StoreObject).ref().Inputs(0U, 3U).TypeId(122U);

            INST(13U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(14U, Opcode::LoadAndInitClass).ref().Inputs(13U);
            INST(15U, Opcode::StoreStatic).ref().Inputs(14U, 3U);

            // Eliminable because of v6 but v6 can be relocated by GC
            INST(8U, Opcode::LoadObject).ref().Inputs(0U).TypeId(122U);
            // Eliminable because of v3 but v3 can be relocated by GC
            INST(9U, Opcode::LoadObject).ref().Inputs(0U).TypeId(100U);
            INST(10U, Opcode::SaveState).Inputs(8U).SrcVregs({0U});
            INST(11U, Opcode::NullCheck).ref().Inputs(9U, 10U);
            INST(12U, Opcode::Return).ref().Inputs(8U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Do not eliminate through static constructors
TEST_F(LSETest, OverClassInitialization)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(7U, Opcode::StoreStatic).ref().Inputs(6U, 0U).TypeId(42U);

            INST(9U, Opcode::InitClass).Inputs(5U).TypeId(200U);

            INST(11U, Opcode::LoadStatic).ref().Inputs(6U).TypeId(42U);
            INST(21U, Opcode::Return).ref().Inputs(11U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Don't hoist from irreducible loop
TEST_F(LSETest, DontHoistWithIrreducibleLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        PARAMETER(3U, 3U).s32();
        PARAMETER(4U, 4U).ref();
        CONSTANT(5U, 0x2aU).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(7U, Opcode::StoreArray).s32().Inputs(4U, 2U, 1U);
            INST(8U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(4U, 5U, 7U)
        {
            INST(10U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0U).Inputs(1U);
        }
        BASIC_BLOCK(5U, 6U, 9U)
        {
            INST(20U, Opcode::Phi).s32().Inputs({{4U, 5U}, {7U, 11U}});
            INST(21U, Opcode::LoadArray).s32().Inputs(4U, 2U);
            INST(26U, Opcode::Mul).s32().Inputs(20U, 21U);
            INST(28U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(30U, Opcode::SaveState).Inputs(4U).SrcVregs({0U});
        }
        BASIC_BLOCK(7U, 5U, 9U)
        {
            INST(11U, Opcode::Phi).s32().Inputs({{4U, 5U}, {6U, 26U}});
            INST(19U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_EQ).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 9U)
        {
            INST(12U, Opcode::StoreArray).s32().Inputs(4U, 2U, 3U);
        }
        BASIC_BLOCK(9U, -1L)
        {
            INST(34U, Opcode::Phi).s32().Inputs({{3U, 1U}, {5U, 2U}, {7U, 3U}});
            INST(35U, Opcode::Return).s32().Inputs(34U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Don't hoist values not alive on backedge
TEST_F(LSETest, DontHoistIfNotAlive)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x2U).s64();
        CONSTANT(2U, 0x1U).s64();
        CONSTANT(6U, 0x10U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(40U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(30U, Opcode::LoadAndInitClass).ref().Inputs(40U).TypeId(0U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(45U, Opcode::Phi).s64().Inputs({{2U, 1U}, {6U, 35U}});
            INST(21U, Opcode::Compare).b().CC(CC_LT).Inputs(45U, 6U);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(21U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(25U, Opcode::Add).s64().Inputs(45U, 45U);
            INST(26U, Opcode::StoreStatic).s64().Inputs(30U, 25U).TypeId(83U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(16U, Opcode::LoadStatic).s64().Inputs(30U).TypeId(83U);
            INST(17U, Opcode::Mul).s64().Inputs(16U, 45U);
        }
        BASIC_BLOCK(6U, 3U, 7U)
        {
            INST(35U, Opcode::Phi).s64().Inputs({{4U, 25U}, {5U, 17U}});
            INST(31U, Opcode::Compare).b().CC(CC_GE).Inputs(35U, 2U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(46U, Opcode::Return).s32().Inputs(45U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Don't hoist values not alive on backedge
TEST_F(LSETest, DontHoistIfNotAliveTriangle)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x2U).s64();
        CONSTANT(2U, 0x1U).s64();
        CONSTANT(6U, 0x10U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(40U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(30U, Opcode::LoadAndInitClass).ref().Inputs(40U).TypeId(0U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(25U, Opcode::Phi).s64().Inputs({{2U, 1U}, {5U, 35U}});
            INST(21U, Opcode::Compare).b().CC(CC_LT).Inputs(25U, 6U);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(21U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(16U, Opcode::LoadStatic).s64().Inputs(30U).TypeId(83U);
            INST(17U, Opcode::Mul).s64().Inputs(16U, 25U);
        }
        BASIC_BLOCK(5U, 3U, 6U)
        {
            INST(35U, Opcode::Phi).s64().Inputs({{3U, 25U}, {4U, 17U}});
            INST(33U, Opcode::StoreStatic).s64().Inputs(30U, 35U).TypeId(83U);
            INST(34U, Opcode::Add).s64().Inputs(35U, 1U);
            INST(31U, Opcode::Compare).b().CC(CC_GE).Inputs(34U, 2U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(46U, Opcode::ReturnVoid).v0id();
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Double hoist
SRC_GRAPH(HoistInnerLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x2U).s64();
        CONSTANT(2U, 0x1U).s64();
        CONSTANT(6U, 0x10U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(40U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(30U, Opcode::LoadAndInitClass).ref().Inputs(40U).TypeId(0U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(45U, Opcode::Phi).s64().Inputs({{2U, 1U}, {4U, 25U}});
            INST(21U, Opcode::Compare).b().CC(CC_LT).Inputs(45U, 6U);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(21U);
        }
        BASIC_BLOCK(4U, 4U, 3U)
        {
            INST(35U, Opcode::Phi).s64().Inputs({{3U, 45U}, {4U, 25U}});
            INST(16U, Opcode::LoadStatic).s64().Inputs(30U).TypeId(83U);
            INST(25U, Opcode::Add).s64().Inputs(45U, 16U);
            INST(31U, Opcode::Compare).b().CC(CC_GE).Inputs(16U, 2U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(46U, Opcode::Return).s32().Inputs(45U);
        }
    }
}

OUT_GRAPH(HoistInnerLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x2U).s64();
        CONSTANT(2U, 0x1U).s64();
        CONSTANT(6U, 0x10U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(40U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(30U, Opcode::LoadAndInitClass).ref().Inputs(40U).TypeId(0U);
            INST(16U, Opcode::LoadStatic).s64().Inputs(30U).TypeId(83U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(45U, Opcode::Phi).s64().Inputs({{2U, 1U}, {4U, 25U}});
            INST(21U, Opcode::Compare).b().CC(CC_LT).Inputs(45U, 6U);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(21U);
        }
        BASIC_BLOCK(4U, 4U, 3U)
        {
            INST(35U, Opcode::Phi).s64().Inputs({{3U, 45U}, {4U, 25U}});
            INST(25U, Opcode::Add).s64().Inputs(45U, 16U);
            INST(31U, Opcode::Compare).b().CC(CC_GE).Inputs(16U, 2U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(46U, Opcode::Return).s32().Inputs(45U);
        }
    }
}

TEST_F(LSETest, HoistInnerLoop)
{
    src_graph::HoistInnerLoop::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::HoistInnerLoop::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Single hoist
SRC_GRAPH(HoistInnerLoop2, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x2U).s64();
        CONSTANT(2U, 0x1U).s64();
        CONSTANT(6U, 0x10U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(40U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(30U, Opcode::LoadAndInitClass).ref().Inputs(40U).TypeId(0U);
        }
        BASIC_BLOCK(3U, 4U, 6U)
        {
            INST(45U, Opcode::Phi).s64().Inputs({{2U, 1U}, {5U, 25U}});
            INST(21U, Opcode::Compare).b().CC(CC_LT).Inputs(45U, 6U);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(21U);
        }
        BASIC_BLOCK(4U, 4U, 5U)
        {
            INST(35U, Opcode::Phi).s64().Inputs({{3U, 45U}, {4U, 25U}});
            INST(16U, Opcode::LoadStatic).s64().Inputs(30U).TypeId(83U);
            INST(25U, Opcode::Add).s64().Inputs(45U, 16U);
            INST(31U, Opcode::Compare).b().CC(CC_GE).Inputs(16U, 2U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        BASIC_BLOCK(5U, 3U)
        {
            INST(17U, Opcode::StoreStatic).s64().Inputs(30U, 25U).TypeId(83U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(46U, Opcode::Return).s32().Inputs(45U);
        }
    }
}

OUT_GRAPH(HoistInnerLoop2, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x2U).s64();
        CONSTANT(2U, 0x1U).s64();
        CONSTANT(6U, 0x10U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(40U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(30U, Opcode::LoadAndInitClass).ref().Inputs(40U).TypeId(0U);
        }
        BASIC_BLOCK(3U, 4U, 6U)
        {
            INST(45U, Opcode::Phi).s64().Inputs({{2U, 1U}, {5U, 25U}});
            INST(21U, Opcode::Compare).b().CC(CC_LT).Inputs(45U, 6U);
            INST(16U, Opcode::LoadStatic).s64().Inputs(30U).TypeId(83U);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(21U);
        }
        BASIC_BLOCK(4U, 4U, 5U)
        {
            INST(35U, Opcode::Phi).s64().Inputs({{3U, 45U}, {4U, 25U}});
            INST(25U, Opcode::Add).s64().Inputs(45U, 16U);
            INST(31U, Opcode::Compare).b().CC(CC_GE).Inputs(16U, 2U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        BASIC_BLOCK(5U, 3U)
        {
            INST(17U, Opcode::StoreStatic).s64().Inputs(30U, 25U).TypeId(83U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(46U, Opcode::Return).s32().Inputs(45U);
        }
    }
}

TEST_F(LSETest, HoistInnerLoop2)
{
    src_graph::HoistInnerLoop2::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::HoistInnerLoop2::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Don't hoist due to inner store
TEST_F(LSETest, DontHoistOuterLoop)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x2U).s64();
        CONSTANT(2U, 0x1U).s64();
        CONSTANT(6U, 0x10U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(40U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(30U, Opcode::LoadAndInitClass).ref().Inputs(40U).TypeId(0U);
        }
        BASIC_BLOCK(3U, 4U, 6U)
        {
            INST(45U, Opcode::Phi).s64().Inputs({{2U, 1U}, {4U, 25U}});
            INST(16U, Opcode::LoadStatic).s64().Inputs(30U).TypeId(83U);
            INST(21U, Opcode::Compare).b().CC(CC_LT).Inputs(45U, 6U);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(21U);
        }
        BASIC_BLOCK(4U, 4U, 3U)
        {
            INST(35U, Opcode::Phi).s64().Inputs({{3U, 45U}, {4U, 25U}});
            INST(25U, Opcode::Add).s64().Inputs(45U, 16U);
            INST(17U, Opcode::StoreStatic).s64().Inputs(30U, 25U).TypeId(83U);
            INST(31U, Opcode::Compare).b().CC(CC_GE).Inputs(16U, 2U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(46U, Opcode::Return).s32().Inputs(45U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Don't hoist from OSR loop
TEST_F(LSETest, DontHoistFromOsrLoop)
{
    GetGraph()->SetMode(GraphMode::Osr());
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s32();
        BASIC_BLOCK(2U, 3U)
        {
            INST(6U, Opcode::StoreObject).ref().Inputs(0U, 1U).TypeId(122U);
            INST(11U, Opcode::StoreObject).s32().Inputs(0U, 2U).TypeId(123U);
            INST(12U, Opcode::LoadObject).s32().Inputs(0U).TypeId(124U);
            INST(13U, Opcode::LoadObject).s32().Inputs(0U).TypeId(125U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(7U, Opcode::SaveStateOsr).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(8U, Opcode::LoadObject).ref().Inputs(0U).TypeId(122U);        // store-load pair
            INST(14U, Opcode::StoreObject).s32().Inputs(0U, 2U).TypeId(123U);  // store-store pair
            INST(15U, Opcode::LoadObject).s32().Inputs(0U).TypeId(124U);       // load-load pair
            INST(9U, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(16U, Opcode::LoadObject).s32().Inputs(0U).TypeId(125U);  // load-load pair, before and after OSR loop
            INST(10U, Opcode::Return).s32().Inputs(16U);
        }
    }

    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

SRC_GRAPH(AliveLoadInBackedge, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x2U).s64();
        CONSTANT(2U, 0x0U).s64();
        CONSTANT(6U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(40U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(30U, Opcode::LoadAndInitClass).ref().Inputs(40U).TypeId(0U);
            INST(5U, Opcode::LoadStatic).s64().Inputs(30U).TypeId(83U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(15U, Opcode::LoadStatic).s64().Inputs(30U).TypeId(83U);
            INST(21U, Opcode::Compare).b().CC(CC_EQ).Inputs(15U, 2U);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(21U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(24U, Opcode::Add).s64().Inputs(15U, 1U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(23U, Opcode::Mul).s64().Inputs(15U, 15U);
            INST(7U, Opcode::StoreStatic).s64().Inputs(30U, 23U).TypeId(83U);
        }
        BASIC_BLOCK(6U, 7U, 3U)
        {
            INST(45U, Opcode::Phi).s64().Inputs({{4U, 24U}, {5U, 23U}});
            INST(16U, Opcode::LoadStatic).s64().Inputs(30U).TypeId(83U);
            INST(31U, Opcode::Compare).b().CC(CC_EQ).Inputs(16U, 6U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(46U, Opcode::Return).s32().Inputs(45U);
        }
    }
}

OUT_GRAPH(AliveLoadInBackedge, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x2U).s64();
        CONSTANT(2U, 0x0U).s64();
        CONSTANT(6U, 0x1U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(40U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(30U, Opcode::LoadAndInitClass).ref().Inputs(40U).TypeId(0U);
            INST(5U, Opcode::LoadStatic).s64().Inputs(30U).TypeId(83U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(15U, Opcode::Phi).s64().Inputs({{2U, 5U}, {6U, 16U}});
            INST(21U, Opcode::Compare).b().CC(CC_EQ).Inputs(15U, 2U);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(21U);
        }
        BASIC_BLOCK(4U, 6U)
        {
            INST(24U, Opcode::Add).s64().Inputs(15U, 1U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(23U, Opcode::Mul).s64().Inputs(15U, 15U);
            INST(7U, Opcode::StoreStatic).s64().Inputs(30U, 23U).TypeId(83U);
        }
        BASIC_BLOCK(6U, 7U, 3U)
        {
            INST(45U, Opcode::Phi).s64().Inputs({{4U, 24U}, {5U, 23U}});
            INST(16U, Opcode::LoadStatic).s64().Inputs(30U).TypeId(83U);
            INST(31U, Opcode::Compare).b().CC(CC_EQ).Inputs(16U, 6U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(46U, Opcode::Return).s32().Inputs(45U);
        }
    }
}

TEST_F(LSETest, AliveLoadInBackedge)
{
    src_graph::AliveLoadInBackedge::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::AliveLoadInBackedge::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

SRC_GRAPH(AliveLoadInBackedgeInnerLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x2U).s64();
        CONSTANT(2U, 0x1U).s64();
        CONSTANT(6U, 0x10U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(40U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(30U, Opcode::LoadAndInitClass).ref().Inputs(40U).TypeId(0U);
            INST(5U, Opcode::LoadStatic).s64().Inputs(30U).TypeId(83U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(15U, Opcode::LoadStatic).s64().Inputs(30U).TypeId(83U);
            INST(24U, Opcode::Add).s64().Inputs(15U, 1U);
            INST(7U, Opcode::StoreStatic).s64().Inputs(30U, 24U).TypeId(83U);
        }
        BASIC_BLOCK(4U, 4U, 5U)
        {
            INST(45U, Opcode::Phi).s64().Inputs({{3U, 1U}, {4U, 25U}});
            INST(25U, Opcode::Mul).s64().Inputs(45U, 45U);
            INST(31U, Opcode::Compare).b().CC(CC_EQ).Inputs(25U, 6U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        BASIC_BLOCK(5U, 3U, 6U)
        {
            INST(16U, Opcode::LoadStatic).s64().Inputs(30U).TypeId(83U);
            INST(27U, Opcode::Compare).b().CC(CC_EQ).Inputs(16U, 2U);
            INST(28U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(27U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(46U, Opcode::Return).s32().Inputs(45U);
        }
    }
}

OUT_GRAPH(AliveLoadInBackedgeInnerLoop, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x2U).s64();
        CONSTANT(2U, 0x1U).s64();
        CONSTANT(6U, 0x10U).s64();
        BASIC_BLOCK(2U, 3U)
        {
            INST(40U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(30U, Opcode::LoadAndInitClass).ref().Inputs(40U).TypeId(0U);
            INST(5U, Opcode::LoadStatic).s64().Inputs(30U).TypeId(83U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(55U, Opcode::Phi).s64().Inputs({{2U, 5U}, {5U, 24U}});
            INST(24U, Opcode::Add).s64().Inputs(55U, 1U);
            INST(7U, Opcode::StoreStatic).s64().Inputs(30U, 24U).TypeId(83U);
        }
        BASIC_BLOCK(4U, 4U, 5U)
        {
            INST(45U, Opcode::Phi).s64().Inputs({{3U, 1U}, {4U, 25U}});
            INST(25U, Opcode::Mul).s64().Inputs(45U, 45U);
            INST(31U, Opcode::Compare).b().CC(CC_EQ).Inputs(25U, 6U);
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(31U);
        }
        BASIC_BLOCK(5U, 3U, 6U)
        {
            INST(27U, Opcode::Compare).b().CC(CC_EQ).Inputs(24U, 2U);
            INST(28U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0x0U).Inputs(27U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(46U, Opcode::Return).s32().Inputs(45U);
        }
    }
}

TEST_F(LSETest, AliveLoadInBackedgeInnerLoop)
{
    src_graph::AliveLoadInBackedgeInnerLoop::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::AliveLoadInBackedgeInnerLoop::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Do not eliminate loads over runtime calls due to GC relocations
SRC_GRAPH(PhiOverSaveStates, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).ref();
        BASIC_BLOCK(2U, 3U)
        {
            INST(4U, Opcode::StoreObject).ref().Inputs(0U, 1U).TypeId(122U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(6U, Opcode::SafePoint).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::LoadObject).ref().Inputs(0U).TypeId(122U);
            INST(8U, Opcode::SaveState).Inputs(0U, 2U, 7U).SrcVregs({0U, 2U, 3U});

            INST(9U, Opcode::LoadObject).ref().Inputs(7U).TypeId(100U);

            INST(10U, Opcode::SaveState).Inputs(0U, 2U, 9U).SrcVregs({0U, 2U, 3U});
            INST(15U, Opcode::LoadObject).s64().Inputs(9U).TypeId(133U);
            INST(11U, Opcode::StoreObject).ref().Inputs(0U, 9U).TypeId(122U);
            INST(12U, Opcode::IfImm).SrcType(DataType::INT64).CC(CC_NE).Imm(0x0U).Inputs(15U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(23U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(PhiOverSaveStates, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).ref();
        BASIC_BLOCK(2U, 3U)
        {
            INST(4U, Opcode::StoreObject).ref().Inputs(0U, 1U).TypeId(122U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(25U, Opcode::Phi).ref().Inputs(1U, 9U);
            INST(6U, Opcode::SafePoint).Inputs(0U, 1U, 2U, 25U).SrcVregs({0U, 1U, 2U, VirtualRegister::BRIDGE});
            INST(8U, Opcode::SaveState).Inputs(0U, 2U, 25U).SrcVregs({0U, 2U, 3U});

            INST(9U, Opcode::LoadObject).ref().Inputs(25U).TypeId(100U);

            INST(10U, Opcode::SaveState).Inputs(0U, 2U, 9U).SrcVregs({0U, 2U, 3U});
            INST(15U, Opcode::LoadObject).s64().Inputs(9U).TypeId(133U);
            INST(11U, Opcode::StoreObject).ref().Inputs(0U, 9U).TypeId(122U);
            INST(12U, Opcode::IfImm).SrcType(DataType::INT64).CC(CC_NE).Imm(0x0U).Inputs(15U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(23U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(LSETest, PhiOverSaveStates)
{
    src_graph::PhiOverSaveStates::CREATE(GetGraph());

    Graph *graphLsed = CreateEmptyGraph();
    out_graph::PhiOverSaveStates::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Simple condition phi
SRC_GRAPH(SimpleConditionPhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(8U, 0x8U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::LoadArray).s32().Inputs(0U, 2U);
            INST(9U, Opcode::Compare).b().CC(CC_GT).Inputs(5U, 8U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(21U, Opcode::Add).s32().Inputs(5U, 1U);
            INST(7U, Opcode::StoreArray).s32().Inputs(0U, 2U, 21U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(22U, Opcode::LoadArray).s32().Inputs(0U, 2U);
            INST(23U, Opcode::Return).s32().Inputs(22U);
        }
    }
}

OUT_GRAPH(SimpleConditionPhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(8U, 0x8U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::LoadArray).s32().Inputs(0U, 2U);
            INST(9U, Opcode::Compare).b().CC(CC_GT).Inputs(5U, 8U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(21U, Opcode::Add).s32().Inputs(5U, 1U);
            INST(7U, Opcode::StoreArray).s32().Inputs(0U, 2U, 21U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(25U, Opcode::Phi).s32().Inputs({{2U, 5U}, {3U, 21U}});
            INST(23U, Opcode::Return).s32().Inputs(25U);
        }
    }
}

TEST_F(LSETest, SimpleConditionPhi)
{
    src_graph::SimpleConditionPhi::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::SimpleConditionPhi::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// Simple condition phi
SRC_GRAPH(SimpleConditionPhi2, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        PARAMETER(3U, 3U).s32();
        CONSTANT(8U, 0x8U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::LoadArray).s32().Inputs(0U, 2U);
            INST(9U, Opcode::Compare).b().CC(CC_GT).Inputs(5U, 8U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(21U, Opcode::Add).s32().Inputs(5U, 1U);
            INST(7U, Opcode::StoreArray).s32().Inputs(0U, 3U, 21U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(6U, Opcode::LoadArray).s32().Inputs(0U, 3U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(22U, Opcode::LoadArray).s32().Inputs(0U, 3U);
            INST(23U, Opcode::Return).s32().Inputs(22U);
        }
    }
}

OUT_GRAPH(SimpleConditionPhi2, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        PARAMETER(3U, 3U).s32();
        CONSTANT(8U, 0x8U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::LoadArray).s32().Inputs(0U, 2U);
            INST(9U, Opcode::Compare).b().CC(CC_GT).Inputs(5U, 8U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(21U, Opcode::Add).s32().Inputs(5U, 1U);
            INST(7U, Opcode::StoreArray).s32().Inputs(0U, 3U, 21U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(6U, Opcode::LoadArray).s32().Inputs(0U, 3U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(25U, Opcode::Phi).s32().Inputs({{3U, 21U}, {4U, 6U}});
            INST(23U, Opcode::Return).s32().Inputs(25U);
        }
    }
}

TEST_F(LSETest, SimpleConditionPhi2)
{
    src_graph::SimpleConditionPhi2::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::SimpleConditionPhi2::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

TEST_F(LSETest, SimpleConditionPhiMayAlias)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(8U, 0x8U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::LoadArray).s32().Inputs(0U, 2U);
            INST(9U, Opcode::Compare).b().CC(CC_GT).Inputs(5U, 8U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(21U, Opcode::Add).s32().Inputs(5U, 1U);
            INST(7U, Opcode::StoreArray).s32().Inputs(0U, 2U, 21U);
            INST(6U, Opcode::StoreArray).s32().Inputs(0U, 1U, 1U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(22U, Opcode::LoadArray).s32().Inputs(0U, 2U);
            INST(23U, Opcode::Return).s32().Inputs(22U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

/// Simple condition phi, check bridges
SRC_GRAPH(SimpleConditionPhiWithRefInputs, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).ref();
        PARAMETER(3U, 3U).ref();
        CONSTANT(4U, 0x4U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::StoreArray).ref().Inputs(1U, 0U, 2U);
            INST(9U, Opcode::Compare).b().CC(CC_GT).Inputs(0U, 4U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(7U, Opcode::StoreArray).ref().Inputs(1U, 0U, 3U);
            INST(8U, Opcode::SafePoint).Inputs(1U).SrcVregs({1U});
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(21U, Opcode::SafePoint).Inputs(1U).SrcVregs({1U});
            INST(22U, Opcode::LoadArray).ref().Inputs(1U, 0U);
            INST(23U, Opcode::SaveState).Inputs(22U).SrcVregs({22U});
            INST(24U, Opcode::CallStatic).v0id().InputsAutoType(22U, 23U);
            INST(25U, Opcode::Return).ref().Inputs(22U);
        }
    }
}

OUT_GRAPH(SimpleConditionPhiWithRefInputs, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).ref();
        PARAMETER(3U, 3U).ref();
        CONSTANT(4U, 0x4U).s64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::StoreArray).ref().Inputs(1U, 0U, 2U);
            INST(9U, Opcode::Compare).b().CC(CC_GT).Inputs(0U, 4U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(7U, Opcode::StoreArray).ref().Inputs(1U, 0U, 3U);
            INST(8U, Opcode::SafePoint).Inputs(1U, 3U).SrcVregs({1U, VirtualRegister::BRIDGE});
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(26U, Opcode::Phi).ref().Inputs({{2U, 2U}, {4U, 3U}});
            INST(21U, Opcode::SafePoint).Inputs(1U, 26U).SrcVregs({1U, VirtualRegister::BRIDGE});
            INST(23U, Opcode::SaveState).Inputs(26U).SrcVregs({22U});
            INST(24U, Opcode::CallStatic).v0id().InputsAutoType(26U, 23U);
            INST(25U, Opcode::Return).ref().Inputs(26U);
        }
    }
}

TEST_F(LSETest, SimpleConditionPhiWithRefInputs)
{
    src_graph::SimpleConditionPhiWithRefInputs::CREATE(GetGraph());
    Graph *graphLsed = CreateEmptyGraph();
    out_graph::SimpleConditionPhiWithRefInputs::CREATE(graphLsed);
    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

/// SaveStateDeoptimize does not prevent elimination and does not need bridges
SRC_GRAPH(EliminateOverSaveStateDeoptimize, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s32();
        PARAMETER(3U, 3U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::StoreObject).ref().Inputs(0U, 1U).TypeId(122U);
            INST(11U, Opcode::StoreObject).s32().Inputs(0U, 2U).TypeId(123U);
            INST(12U, Opcode::LoadObject).ref().Inputs(0U).TypeId(124U);

            INST(7U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(8U, Opcode::LoadObject).ref().Inputs(0U).TypeId(122U);        // store-load pair
            INST(14U, Opcode::StoreObject).s32().Inputs(0U, 3U).TypeId(123U);  // store-store pair
            INST(15U, Opcode::LoadObject).ref().Inputs(0U).TypeId(124U);       // load-load pair

            INST(16U, Opcode::SaveState).Inputs(12U, 8U, 15U).SrcVregs({0U, 1U, 2U});
            INST(17U, Opcode::CallStatic).ref().InputsAutoType(12U, 8U, 15U, 16U);

            INST(10U, Opcode::Return).ref().Inputs(17U);
        }
    }
}

OUT_GRAPH(EliminateOverSaveStateDeoptimize, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s32();
        PARAMETER(3U, 3U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::StoreObject).ref().Inputs(0U, 1U).TypeId(122U);
            INST(12U, Opcode::LoadObject).ref().Inputs(0U).TypeId(124U);

            INST(7U, Opcode::SaveStateDeoptimize)
                .Inputs(0U, 1U, 12U)
                .SrcVregs({0U, VirtualRegister::BRIDGE, VirtualRegister::BRIDGE});
            INST(14U, Opcode::StoreObject).s32().Inputs(0U, 3U).TypeId(123U);

            INST(16U, Opcode::SaveState).Inputs(12U, 1U, 12U).SrcVregs({0U, 1U, 2U});
            INST(17U, Opcode::CallStatic).ref().InputsAutoType(12U, 1U, 12U, 16U);

            INST(10U, Opcode::Return).ref().Inputs(17U);
        }
    }
}

TEST_F(LSETest, EliminateOverSaveStateDeoptimize)
{
    src_graph::EliminateOverSaveStateDeoptimize::CREATE(GetGraph());

    Graph *graphLsed = CreateEmptyGraph();
    out_graph::EliminateOverSaveStateDeoptimize::CREATE(graphLsed);

    ASSERT_TRUE(GetGraph()->RunPass<Lse>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphLsed));
}

// NOLINTEND(readability-magic-numbers)

}  //  namespace ark::compiler
