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
#include "optimizer/optimizations/regalloc/cleanup_empty_blocks.h"
#include "optimizer/optimizations/regalloc/reg_alloc_graph_coloring.h"
#include "optimizer/optimizations/cleanup.h"

namespace ark::compiler {
class CleanupEmptyBlocksTest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers)
SRC_GRAPH(RemoveEmptyBlockAfterRegAlloc, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i32();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 6U, 3U)
        {
            INST(3U, Opcode::AndI).i32().Inputs(0U).Imm(3U);
            INST(4U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            CONSTANT(5U, 100U);
            INST(6U, Opcode::Mod).i32().Inputs(0U, 5U);
            INST(7U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 6U, 5U)
        {
            CONSTANT(8U, 400U);
            INST(9U, Opcode::Mod).i32().Inputs(0U, 8U);
            INST(10U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(5U, 6U) {}
        BASIC_BLOCK(6U, -1L)
        {
            INST(11U, Opcode::Phi).b().Inputs({{2U, 1U}, {4U, 1U}, {5U, 2U}});
            INST(12U, Opcode::Return).b().Inputs(11U);
        }
    }
}

OUT_GRAPH(RemoveEmptyBlockAfterRegAlloc, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i32();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        BASIC_BLOCK(2U, 8U, 3U)
        {
            INST(3U, Opcode::AndI).i32().Inputs(0U).Imm(3U);
            INST(4U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 6U, 4U)
        {
            CONSTANT(5U, 100U);
            INST(6U, Opcode::Mod).i32().Inputs(0U, 5U);
            INST(7U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 7U, 6U)
        {
            CONSTANT(8U, 400U);
            INST(9U, Opcode::Mod).i32().Inputs(0U, 8U);
            INST(10U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(7U, 6U)
        {
            INST(13U, Opcode::SpillFill);
        }
        BASIC_BLOCK(8U, 6U)
        {
            INST(14U, Opcode::SpillFill);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(11U, Opcode::Phi).b().Inputs({{8U, 1U}, {7U, 1U}, {3U, 2U}, {4U, 2U}});
            INST(12U, Opcode::Return).b().Inputs(11U);
        }
    }
}

TEST_F(CleanupEmptyBlocksTest, RemoveEmptyBlockAfterRegAlloc)
{
    if (GetGraph()->GetArch() == Arch::AARCH32 || !BackendSupport(GetGraph()->GetArch())) {
        return;
    }

    // Empty basic block 5 cannot be removed because it is part of a special triangle
    // After RegAlloc inserts BB 7, BB 5 can be removed
    src_graph::RemoveEmptyBlockAfterRegAlloc::CREATE(GetGraph());

    ASSERT_FALSE(GetGraph()->RunPass<Cleanup>());
    ASSERT_TRUE(GetGraph()->RunPass<RegAllocGraphColoring>());
    ASSERT_TRUE(CleanupEmptyBlocks(GetGraph()));

    auto graph = CreateEmptyGraph();
    out_graph::RemoveEmptyBlockAfterRegAlloc::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
