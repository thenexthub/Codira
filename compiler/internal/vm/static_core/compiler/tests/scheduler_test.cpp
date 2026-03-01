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

#include "optimizer/optimizations/scheduler.h"

namespace ark::compiler {
class SchedulerTest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(SchedulerTest, Basic)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 42U);
        CONSTANT(1U, 43U);
        CONSTANT(2U, 44U);
        CONSTANT(3U, 45U);
        CONSTANT(4U, 46U);
        CONSTANT(5U, 47U);
        CONSTANT(6U, 48U);
        CONSTANT(7U, 49U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(9U, Opcode::Add).u64().Inputs(2U, 3U);
            // should be moved down
            INST(10U, Opcode::Add).u64().Inputs(8U, 9U);

            INST(11U, Opcode::Add).u64().Inputs(4U, 5U);
            INST(12U, Opcode::Add).u64().Inputs(6U, 7U);
            INST(13U, Opcode::Add).u64().Inputs(11U, 12U);
            // Grand total
            INST(14U, Opcode::Add).u64().Inputs(10U, 13U);
            INST(15U, Opcode::Return).u64().Inputs(14U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Scheduler>());

    ASSERT_EQ(INS(8U).GetNext(), &INS(9U));
    ASSERT_NE(INS(9U).GetNext(), &INS(10U));

    EXPECT_TRUE((INS(11U).GetNext() == &INS(10U)) || (INS(12U).GetNext() == &INS(10U)));

    ASSERT_EQ(INS(13U).GetNext(), &INS(14U));
    ASSERT_EQ(INS(14U).GetNext(), &INS(15U));
}

TEST_F(SchedulerTest, LoadBarrier)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        CONSTANT(2U, 42U);
        CONSTANT(3U, 43U);
        CONSTANT(4U, 44U);
        CONSTANT(5U, 45U);
        CONSTANT(6U, 46U);
        CONSTANT(7U, 47U);
        CONSTANT(8U, 48U);
        CONSTANT(9U, 5U);  // len array

        BASIC_BLOCK(2U, -1L)
        {
            INST(10U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(11U, Opcode::Add).u64().Inputs(4U, 5U);
            // should be moved down
            INST(12U, Opcode::Add).u64().Inputs(10U, 11U);

            INST(13U, Opcode::Add).u64().Inputs(6U, 7U);

            INST(21U, Opcode::SafePoint).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(14U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(15U, Opcode::BoundsCheck).s32().Inputs(9U, 0U, 14U);
            // can't move up because of SafePoint
            INST(16U, Opcode::LoadArray).u64().Inputs(1U, 15U);

            INST(17U, Opcode::Add).u64().Inputs(8U, 16U);
            INST(18U, Opcode::Add).u64().Inputs(13U, 17U);

            INST(19U, Opcode::Add).u64().Inputs(12U, 18U);
            INST(20U, Opcode::Return).u64().Inputs(19U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Scheduler>());

    ASSERT_EQ(INS(11U).GetNext(), &INS(13U));
    ASSERT_EQ(INS(13U).GetNext(), &INS(12U));
    ASSERT_EQ(INS(12U).GetNext(), &INS(21U));
    ASSERT_EQ(INS(21U).GetNext(), &INS(14U));
    ASSERT_EQ(INS(15U).GetNext(), &INS(16U));
    ASSERT_EQ(INS(16U).GetNext(), &INS(17U));
}

TEST_F(SchedulerTest, Load)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        CONSTANT(2U, 42U);
        CONSTANT(3U, 43U);
        CONSTANT(4U, 44U);
        CONSTANT(5U, 45U);
        CONSTANT(6U, 46U);
        CONSTANT(7U, 47U);
        CONSTANT(8U, 48U);
        CONSTANT(9U, 5U);  // len array

        BASIC_BLOCK(2U, -1L)
        {
            INST(10U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(11U, Opcode::Add).u64().Inputs(4U, 5U);
            // should be moved down
            INST(12U, Opcode::Add).u64().Inputs(10U, 11U);

            INST(13U, Opcode::Add).u64().Inputs(6U, 7U);

            // all three should be moved up
            INST(14U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(15U, Opcode::BoundsCheck).s32().Inputs(9U, 0U, 14U);
            INST(16U, Opcode::LoadArray).u64().Inputs(1U, 15U);

            INST(17U, Opcode::Add).u64().Inputs(8U, 16U);
            INST(18U, Opcode::Add).u64().Inputs(13U, 17U);

            INST(19U, Opcode::Add).u64().Inputs(12U, 18U);
            INST(20U, Opcode::Return).u64().Inputs(19U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Scheduler>());

    ASSERT_EQ(INS(14U).GetNext(), &INS(10U));
    ASSERT_EQ(INS(10U).GetNext(), &INS(15U));
    ASSERT_EQ(INS(15U).GetNext(), &INS(11U));
    ASSERT_EQ(INS(11U).GetNext(), &INS(16U));
    ASSERT_EQ(INS(16U).GetNext(), &INS(13U));
    ASSERT_EQ(INS(13U).GetNext(), &INS(12U));
    ASSERT_EQ(INS(12U).GetNext(), &INS(17U));
    ASSERT_EQ(INS(17U).GetNext(), &INS(18U));
    ASSERT_EQ(INS(18U).GetNext(), &INS(19U));
    ASSERT_EQ(INS(19U).GetNext(), &INS(20U));
}

TEST_F(SchedulerTest, FixSSInBBAfterScheduler)
{
    GRAPH(GetGraph())
    {
        CONSTANT(1U, 1U).s32();
        CONSTANT(2U, 2U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U).TypeId(68U);
            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::LoadConstArray).ref().Inputs(6U);
            INST(8U, Opcode::Add).s32().Inputs(1U, 2U);
            // LoadArray will move on one position down
            INST(9U, Opcode::LoadArray).s32().Inputs(7U, 8U);
            INST(10U, Opcode::SaveState).NoVregs();

            INST(11U, Opcode::NOP);
            INST(12U, Opcode::Return).s32().Inputs(9U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Scheduler>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(1U, 1U).s32();
        CONSTANT(2U, 2U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U).TypeId(68U);
            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::LoadConstArray).ref().Inputs(6U);
            INST(8U, Opcode::Add).s32().Inputs(1U, 2U);

            INST(10U, Opcode::SaveState).Inputs(7U).SrcVregs({VirtualRegister::BRIDGE});
            INST(9U, Opcode::LoadArray).s32().Inputs(7U, 8U);

            INST(11U, Opcode::NOP);
            INST(12U, Opcode::Return).s32().Inputs(9U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(FixSSInBBAfterSchedulerOutToBlock, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(1U, 1U).s32();
        CONSTANT(2U, 2U).s32();

        BASIC_BLOCK(2U, 3U)
        {
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U).TypeId(68U);
            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::LoadConstArray).ref().Inputs(6U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(8U, Opcode::Add).s32().Inputs(1U, 2U);
            // LoadArray will move on one position down
            INST(9U, Opcode::LoadArray).s32().Inputs(7U, 8U);
            INST(10U, Opcode::SaveState).NoVregs();

            INST(11U, Opcode::NOP);
            INST(12U, Opcode::Return).s32().Inputs(9U);
        }
    }
}

OUT_GRAPH(FixSSInBBAfterSchedulerOutToBlock, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(1U, 1U).s32();
        CONSTANT(2U, 2U).s32();

        BASIC_BLOCK(2U, 3U)
        {
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U).TypeId(68U);
            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::LoadConstArray).ref().Inputs(6U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(8U, Opcode::Add).s32().Inputs(1U, 2U);

            INST(10U, Opcode::SaveState).Inputs(7U).SrcVregs({VirtualRegister::BRIDGE});
            INST(9U, Opcode::LoadArray).s32().Inputs(7U, 8U);

            INST(11U, Opcode::NOP);
            INST(12U, Opcode::Return).s32().Inputs(9U);
        }
    }
}

TEST_F(SchedulerTest, FixSSInBBAfterSchedulerOutToBlock)
{
    src_graph::FixSSInBBAfterSchedulerOutToBlock::CREATE(GetGraph());

    ASSERT_TRUE(GetGraph()->RunPass<Scheduler>());

    auto graph = CreateEmptyGraph();
    out_graph::FixSSInBBAfterSchedulerOutToBlock::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(SchedulerTest, LoadI)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::BoundsCheckI).s32().Inputs(0U, 2U).Imm(1U);
            INST(4U, Opcode::StoreArrayI).u64().Inputs(1U, 0U).Imm(1U);
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(6U, Opcode::BoundsCheckI).s32().Inputs(0U, 5U).Imm(0U);
            INST(7U, Opcode::LoadArrayI).u64().Inputs(1U).Imm(0U);
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
    }

    ASSERT_FALSE(GetGraph()->RunPass<Scheduler>());
}

TEST_F(SchedulerTest, TrickyLoadI)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(6U, Opcode::BoundsCheckI).s32().Inputs(0U, 5U).Imm(0U);
            // Manually moved here
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::BoundsCheckI).s32().Inputs(0U, 2U).Imm(1U);
            INST(4U, Opcode::StoreArrayI).u64().Inputs(1U, 0U).Imm(1U);
            // But than all 3 may be moved below the load
            INST(7U, Opcode::LoadArrayI).u64().Inputs(1U).Imm(0U);
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Scheduler>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(6U, Opcode::BoundsCheckI).s32().Inputs(0U, 5U).Imm(0U);
            INST(7U, Opcode::LoadArrayI).u64().Inputs(1U).Imm(0U);

            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::BoundsCheckI).s32().Inputs(0U, 2U).Imm(1U);
            INST(4U, Opcode::StoreArrayI).u64().Inputs(1U, 0U).Imm(1U);

            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(SchedulerTest, MustAliasLoadI)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(6U, Opcode::BoundsCheckI).s32().Inputs(0U, 5U).Imm(42U);
            // Manually moved here
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::BoundsCheckI).s32().Inputs(0U, 2U).Imm(42U);
            INST(4U, Opcode::StoreArrayI).u64().Inputs(1U, 0U).Imm(42U);
            // But than all 3 may be moved below the load
            INST(7U, Opcode::LoadArrayI).u64().Inputs(1U).Imm(42U);
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
    }

    ASSERT_FALSE(GetGraph()->RunPass<Scheduler>());
}

SRC_GRAPH(LoadPair, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        CONSTANT(11U, 41U);
        CONSTANT(12U, 42U);
        CONSTANT(13U, 43U);
        CONSTANT(14U, 44U);
        CONSTANT(15U, 45U);
        CONSTANT(16U, 46U);
        CONSTANT(17U, 47U);
        CONSTANT(18U, 48U);
        CONSTANT(19U, 49U);
        CONSTANT(20U, 50U);
        CONSTANT(21U, 51U);
        CONSTANT(22U, 52U);
        CONSTANT(23U, 53U);
        CONSTANT(24U, 54U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(31U, Opcode::Add).u64().Inputs(11U, 12U);
            INST(32U, Opcode::Add).u64().Inputs(13U, 14U);
            INST(41U, Opcode::Add).u64().Inputs(31U, 32U);

            INST(33U, Opcode::Add).u64().Inputs(15U, 16U);
            INST(34U, Opcode::Add).u64().Inputs(17U, 18U);
            INST(42U, Opcode::Add).u64().Inputs(33U, 34U);

            INST(35U, Opcode::Add).u64().Inputs(19U, 20U);
            INST(36U, Opcode::Add).u64().Inputs(21U, 22U);
            INST(43U, Opcode::Add).u64().Inputs(35U, 36U);

            INST(92U, Opcode::LoadArrayPair).u64().Inputs(1U, 0U);
            INST(93U, Opcode::LoadPairPart).u64().Inputs(92U).Imm(0U);
            INST(94U, Opcode::LoadPairPart).u64().Inputs(92U).Imm(1U);

            INST(37U, Opcode::Add).u64().Inputs(23U, 93U);
            INST(38U, Opcode::Add).u64().Inputs(24U, 94U);
            INST(44U, Opcode::Add).u64().Inputs(37U, 38U);

            INST(51U, Opcode::Add).u64().Inputs(41U, 42U);
            INST(52U, Opcode::Add).u64().Inputs(43U, 44U);

            INST(61U, Opcode::Add).u64().Inputs(51U, 52U);
            INST(62U, Opcode::Return).u64().Inputs(61U);
        }
    }
}

OUT_GRAPH(LoadPair, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        CONSTANT(11U, 41U);
        CONSTANT(12U, 42U);
        CONSTANT(13U, 43U);
        CONSTANT(14U, 44U);
        CONSTANT(15U, 45U);
        CONSTANT(16U, 46U);
        CONSTANT(17U, 47U);
        CONSTANT(18U, 48U);
        CONSTANT(19U, 49U);
        CONSTANT(20U, 50U);
        CONSTANT(21U, 51U);
        CONSTANT(22U, 52U);
        CONSTANT(23U, 53U);
        CONSTANT(24U, 54U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(92U, Opcode::LoadArrayPair).u64().Inputs(1U, 0U);
            INST(93U, Opcode::LoadPairPart).u64().Inputs(92U).Imm(0U);
            INST(94U, Opcode::LoadPairPart).u64().Inputs(92U).Imm(1U);

            INST(31U, Opcode::Add).u64().Inputs(11U, 12U);
            INST(32U, Opcode::Add).u64().Inputs(13U, 14U);
            INST(33U, Opcode::Add).u64().Inputs(15U, 16U);
            INST(34U, Opcode::Add).u64().Inputs(17U, 18U);
            INST(35U, Opcode::Add).u64().Inputs(19U, 20U);
            INST(36U, Opcode::Add).u64().Inputs(21U, 22U);
            INST(37U, Opcode::Add).u64().Inputs(23U, 93U);
            INST(38U, Opcode::Add).u64().Inputs(24U, 94U);

            INST(41U, Opcode::Add).u64().Inputs(31U, 32U);
            INST(42U, Opcode::Add).u64().Inputs(33U, 34U);
            INST(43U, Opcode::Add).u64().Inputs(35U, 36U);
            INST(44U, Opcode::Add).u64().Inputs(37U, 38U);

            INST(51U, Opcode::Add).u64().Inputs(41U, 42U);
            INST(52U, Opcode::Add).u64().Inputs(43U, 44U);

            INST(61U, Opcode::Add).u64().Inputs(51U, 52U);
            INST(62U, Opcode::Return).u64().Inputs(61U);
        }
    }
}

TEST_F(SchedulerTest, LoadPair)
{
    src_graph::LoadPair::CREATE(GetGraph());

    ASSERT_TRUE(GetGraph()->RunPass<Scheduler>());

    auto graph = CreateEmptyGraph();
    out_graph::LoadPair::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(SchedulerTest, NonVolatileLoadObject)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        PARAMETER(3U, 3U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Add).s32().Inputs(1U, 2U);
            INST(5U, Opcode::Add).s32().Inputs(1U, 3U);
            INST(6U, Opcode::Add).s32().Inputs(2U, 3U);
            INST(7U, Opcode::Add).s32().Inputs(4U, 5U);
            INST(8U, Opcode::Add).s32().Inputs(6U, 7U);
            INST(9U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(10U, Opcode::NullCheck).ref().Inputs(0U, 9U);
            INST(11U, Opcode::LoadObject).s32().Inputs(10U).TypeId(152U);
            INST(12U, Opcode::Add).s32().Inputs(8U, 11U);
            INST(13U, Opcode::Return).s32().Inputs(12U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Scheduler>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        PARAMETER(3U, 3U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(9U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::Add).s32().Inputs(1U, 2U);
            INST(10U, Opcode::NullCheck).ref().Inputs(0U, 9U);
            INST(5U, Opcode::Add).s32().Inputs(1U, 3U);
            INST(11U, Opcode::LoadObject).s32().Inputs(10U).TypeId(152U);
            INST(6U, Opcode::Add).s32().Inputs(2U, 3U);
            INST(7U, Opcode::Add).s32().Inputs(4U, 5U);
            INST(8U, Opcode::Add).s32().Inputs(6U, 7U);
            INST(12U, Opcode::Add).s32().Inputs(8U, 11U);
            INST(13U, Opcode::Return).s32().Inputs(12U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(SchedulerTest, VolatileLoadObject)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        PARAMETER(3U, 3U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Add).s32().Inputs(1U, 2U);
            INST(5U, Opcode::Add).s32().Inputs(1U, 3U);
            INST(6U, Opcode::Add).s32().Inputs(2U, 3U);
            INST(7U, Opcode::Add).s32().Inputs(4U, 5U);
            INST(8U, Opcode::Add).s32().Inputs(6U, 7U);
            INST(9U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(10U, Opcode::NullCheck).ref().Inputs(0U, 9U);
            INST(11U, Opcode::LoadObject).s32().Inputs(10U).TypeId(152U).Volatile();
            INST(12U, Opcode::Add).s32().Inputs(8U, 11U);
            INST(13U, Opcode::Return).s32().Inputs(12U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Scheduler>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        PARAMETER(3U, 3U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Add).s32().Inputs(1U, 2U);
            INST(5U, Opcode::Add).s32().Inputs(1U, 3U);
            INST(9U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::Add).s32().Inputs(2U, 3U);
            INST(7U, Opcode::Add).s32().Inputs(4U, 5U);
            INST(10U, Opcode::NullCheck).ref().Inputs(0U, 9U);
            INST(8U, Opcode::Add).s32().Inputs(6U, 7U);
            INST(11U, Opcode::LoadObject).s32().Inputs(10U).TypeId(152U).Volatile();
            INST(12U, Opcode::Add).s32().Inputs(8U, 11U);
            INST(13U, Opcode::Return).s32().Inputs(12U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
