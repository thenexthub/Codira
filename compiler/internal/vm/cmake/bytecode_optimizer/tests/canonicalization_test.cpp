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

#include "canonicalization.h"
#include "common.h"

namespace panda::bytecodeopt::test {

TEST_F(CommonTest, CanonicalizationSwapCompareInputs)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s32();
        CONSTANT(1, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).s32().Inputs(0, 1);
            INST(3, Opcode::Compare).b().Inputs(0, 2);
            INST(4, Opcode::Return).b().Inputs(3);
        }
    }

    graph->RunPass<Canonicalization>();

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        CONSTANT(0, 0).s32();
        CONSTANT(1, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).s32().Inputs(0, 1);
            INST(3, Opcode::Compare).b().Inputs(2, 0);
            INST(4, Opcode::Return).b().Inputs(3);
        }
    }

    EXPECT_TRUE(GraphComparator().Compare(graph, expected));
}

}  // namespace panda::bytecodeopt::test
