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

#include "check_resolver.h"
#include "common.h"

namespace ark::bytecodeopt::test {

// NOLINTBEGIN(readability-magic-numbers)

TEST_F(CommonTest, CheckResolverLenArray)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::NullCheck).ref().Inputs(1U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 0U, 2U);
            INST(6U, Opcode::LoadArray).s32().Inputs(3U, 5U);
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }

    EXPECT_TRUE(graph->RunPass<CheckResolver>());
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::bytecodeopt::test
