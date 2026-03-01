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
#include "optimizer/optimizations/memory_barriers.h"

namespace ark::compiler {
class MemoryBarrierTest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers)
SRC_GRAPH(Test1, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(0U, 0x2aU).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(2U, Opcode::NewArray).ref().Inputs(4U, 0U, 1U);
            INST(3U, Opcode::SaveState).Inputs(0U, 2U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NewObject).ref().Inputs(4U, 3U);
            INST(6U, Opcode::SaveState).Inputs(0U, 2U, 5U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::CallStatic).v0id().Inputs({{DataType::NO_TYPE, 6U}});
            INST(8U, Opcode::SaveState).Inputs(0U, 2U, 5U).SrcVregs({0U, 1U, 2U});
            INST(9U, Opcode::LoadAndInitClass).ref().Inputs(8U);
            INST(10U, Opcode::NewObject).ref().Inputs(9U, 8U);
            INST(11U, Opcode::SaveState).Inputs(0U, 2U, 5U, 10U).SrcVregs({0U, 1U, 2U, 3U});
            INST(12U, Opcode::CallVirtual)
                .s64()
                .Inputs({{DataType::REFERENCE, 2U}, {DataType::REFERENCE, 5U}, {DataType::NO_TYPE, 6U}});
            INST(13U, Opcode::Return).ref().Inputs(10U);
        }
    }
}

TEST_F(MemoryBarrierTest, Test1)
{
    src_graph::Test1::CREATE(GetGraph());
    ASSERT_EQ(INS(0U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(1U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(2U).GetFlag(inst_flags::MEM_BARRIER), true);
    ASSERT_EQ(INS(3U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(4U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(5U).GetFlag(inst_flags::MEM_BARRIER), true);
    ASSERT_EQ(INS(6U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(7U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(8U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(9U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(10U).GetFlag(inst_flags::MEM_BARRIER), true);
    ASSERT_EQ(INS(11U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(12U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(13U).GetFlag(inst_flags::MEM_BARRIER), false);

    ASSERT_TRUE(GetGraph()->RunPass<OptimizeMemoryBarriers>());
    GraphChecker(GetGraph()).Check();

    ASSERT_EQ(INS(0U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(1U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(2U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(3U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(4U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(5U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(6U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(7U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(8U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(9U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(10U).GetFlag(inst_flags::MEM_BARRIER), true);
    ASSERT_EQ(INS(11U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(12U).GetFlag(inst_flags::MEM_BARRIER), false);
    ASSERT_EQ(INS(13U).GetFlag(inst_flags::MEM_BARRIER), false);
}
// NOLINTEND(readability-magic-numbers)

}  //  namespace ark::compiler
