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

#include "canonicalization.h"
#include "common.h"

namespace ark::bytecodeopt::test {

// NOLINTBEGIN(readability-magic-numbers)

TEST_F(CommonTest, CanonicalizationSwapCompareInputs)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s32();
        CONSTANT(1U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Compare).b().Inputs(0U, 2U);
            INST(4U, Opcode::Return).b().Inputs(3U);
        }
    }

    graph->RunPass<Canonicalization>();

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        CONSTANT(0U, 0U).s32();
        CONSTANT(1U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Compare).b().Inputs(2U, 0U);
            INST(4U, Opcode::Return).b().Inputs(3U);
        }
    }

    EXPECT_TRUE(GraphComparator().Compare(graph, expected));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::bytecodeopt::test
