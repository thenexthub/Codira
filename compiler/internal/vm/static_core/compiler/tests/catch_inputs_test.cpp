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
#include "optimizer/analysis/catch_inputs.h"

namespace ark::compiler {
class CatchInputsTest : public GraphTest {};

TEST_F(CatchInputsTest, MarkCatchInputs)
{
    static constexpr int ESCAPED_INST_ID = 6;
    // NOLINTBEGIN(readability-magic-numbers)
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).i32();
        PARAMETER(1U, 1U).i32();

        BASIC_BLOCK(2U, 3U, 4U)  // try_begin
        {
            INST(3U, Opcode::Try).CatchTypeIds({0U});
        }

        BASIC_BLOCK(3U, 7U)  // try
        {
            INST(4U, Opcode::Mul).i32().Inputs(0U, 0U);
            INST(5U, Opcode::Mul).i32().Inputs(1U, 1U);
            INST(ESCAPED_INST_ID, Opcode::Add).i32().Inputs(4U, 5U);
            INST(7U, Opcode::Mul).i32().Inputs(4U, 5U);
        }

        BASIC_BLOCK(7U, 5U, 4U)  // try_end
        {
        }

        BASIC_BLOCK(4U, 6U)  // catch_begin
        {
            INST(8U, Opcode::CatchPhi).i32().Inputs(ESCAPED_INST_ID);
        }

        BASIC_BLOCK(6U, -1L)  // catch
        {
            INST(9U, Opcode::Mul).i32().Inputs(8U, 8U);
            INST(10U, Opcode::Return).i32().Inputs(9U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(11U, Opcode::Return).i32().Inputs(7U);
        }
    }

    BB(2U).SetTryBegin(true);
    BB(3U).SetTry(true);
    BB(7U).SetTryEnd(true);
    BB(4U).SetCatchBegin(true);
    BB(6U).SetCatch(true);
    // NOLINTEND(readability-magic-numbers)

    GetGraph()->RunPass<CatchInputs>();

    for (auto bb : GetGraph()->GetVectorBlocks()) {
        if (bb == nullptr) {
            continue;
        }
        for (auto inst : bb->AllInstsSafe()) {
            // Only inst 6 should have a flag
            ASSERT_EQ(inst->GetFlag(inst_flags::Flags::CATCH_INPUT), inst->GetId() == ESCAPED_INST_ID);
        }
    }
}

TEST_F(CatchInputsTest, HandlePhi)
{
    static constexpr int PARAM_ID = 0;
    static constexpr int PHI_ID = 2;
    static constexpr int LOAD_ID = 4;
    // NOLINTBEGIN(readability-magic-numbers)
    GRAPH(GetGraph())
    {
        PARAMETER(PARAM_ID, 0U).ref();

        BASIC_BLOCK(2U, 3U, 8U)
        {
            INST(PHI_ID, Opcode::Phi).ref().Inputs(PARAM_ID, PHI_ID, LOAD_ID);
            INST(3U, Opcode::IfImm).SrcType(DataType::REFERENCE).Inputs(PHI_ID).Imm(0U).CC(CC_NE);
        }

        BASIC_BLOCK(8U, 2U, 7U)
        {
            INST(7U, Opcode::IfImm).SrcType(DataType::REFERENCE).Inputs(PHI_ID).Imm(0U).CC(CC_NE);
        }

        BASIC_BLOCK(3U, 4U) {}

        BASIC_BLOCK(4U, 5U)
        {
            INST(LOAD_ID, Opcode::LoadObject).ref().Inputs(PHI_ID);
        }

        BASIC_BLOCK(5U, 2U, 6U) {}

        BASIC_BLOCK(6U, 7U)
        {
            INST(5U, Opcode::CatchPhi).ref().Inputs(PHI_ID);
        }

        BASIC_BLOCK(7U, -1L)
        {
            INST(6U, Opcode::ReturnVoid).v0id();
        }
    }

    BB(3U).SetTryBegin(true);
    BB(4U).SetTry(true);
    BB(5U).SetTryEnd(true);
    BB(6U).SetCatchBegin(true);
    BB(6U).SetCatch(true);
    // NOLINTEND(readability-magic-numbers)

    GetGraph()->RunPass<CatchInputs>();

    ASSERT_TRUE(INS(PARAM_ID).GetFlag(inst_flags::Flags::CATCH_INPUT));
    ASSERT_TRUE(INS(PHI_ID).GetFlag(inst_flags::Flags::CATCH_INPUT));
    ASSERT_TRUE(INS(LOAD_ID).GetFlag(inst_flags::Flags::CATCH_INPUT));
}
}  // namespace ark::compiler
