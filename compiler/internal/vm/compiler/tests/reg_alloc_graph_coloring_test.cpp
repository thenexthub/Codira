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
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/code_generator/registers_description.h"
#include "optimizer/optimizations/regalloc/reg_alloc_graph_coloring.h"
#include "optimizer/optimizations/regalloc/spill_fills_resolver.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/ir/graph_cloner.h"

namespace panda::compiler {
class RegAllocGraphColoringTest : public GraphTest {
public:
    SpillFillData GetParameterSpillFilll(Inst *param)
    {
        ASSERT(param->IsParameter());
        ASSERT(param->GetNext()->IsSpillFill());
        auto spill_fill = param->GetNext()->CastToSpillFill()->GetSpillFill(0);
        auto param_liveness = GetGraph()->GetAnalysis<LivenessAnalyzer>().GetInstLifeIntervals(param);
        EXPECT_EQ(param_liveness->GetReg(), spill_fill.SrcValue());
        EXPECT_EQ(param_liveness->GetSibling()->GetReg(), spill_fill.DstValue());
        return spill_fill;
    }
};

TEST_F(RegAllocGraphColoringTest, Simple)
{
    // Test is for bigger number of registers (spilling is not supported yet)
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        return;
    }

    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);
        CONSTANT(2, 10);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(2, 0);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }

        BASIC_BLOCK(3, 4)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 2);
        }

        BASIC_BLOCK(4, -1)
        {
            INST(6, Opcode::Phi).u64().Inputs(2, 5);
            INST(7, Opcode::Add).u64().Inputs(6, 1);
            INST(8, Opcode::Return).u64().Inputs(7);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocGraphColoring>();
    ASSERT_TRUE(result);
    GraphChecker(GetGraph()).Check();
    EXPECT_NE(INS(0).GetDstReg(), INS(6).GetDstReg());
    EXPECT_NE(INS(0).GetDstReg(), INS(1).GetDstReg());
    EXPECT_NE(INS(0).GetDstReg(), INS(2).GetDstReg());

    auto arch = GetGraph()->GetArch();
    size_t first_callee = arch != Arch::NONE ? GetFirstCalleeReg(arch, false) : 0;
    EXPECT_LT(INS(0).CastToParameter()->GetLocationData().DstValue(), first_callee);
    EXPECT_LT(INS(1).GetDstReg(), first_callee);
    EXPECT_LT(INS(2).GetDstReg(), first_callee);
    EXPECT_LT(INS(5).GetDstReg(), first_callee);
    EXPECT_LT(INS(6).GetDstReg(), first_callee);
    EXPECT_LT(INS(7).GetDstReg(), first_callee);
}

TEST_F(RegAllocGraphColoringTest, AffineComponent)
{
    // Test is for bigger number of registers (spilling is not supported yet)
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        return;
    }

    GRAPH(GetGraph())
    {
        PARAMETER(0, 0).u64();
        CONSTANT(1, 1);
        CONSTANT(2, 10);

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(2, 0);
            INST(4, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(3);
        }

        BASIC_BLOCK(3, 8)
        {
            INST(5, Opcode::Add).u64().Inputs(0, 2);
        }

        BASIC_BLOCK(4, 5, 6)
        {
            INST(7, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::Type::UINT64).Inputs(1, 0);
            INST(8, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0).Inputs(7);
        }

        BASIC_BLOCK(5, 7)
        {
            CONSTANT(9, 188);
        }

        BASIC_BLOCK(6, 7)
        {
            INST(6, Opcode::Add).u64().Inputs(0, 1);
        }

        BASIC_BLOCK(7, 8)
        {
            INST(10, Opcode::Phi).u64().Inputs(9, 6);
        }

        BASIC_BLOCK(8, -1)
        {
            INST(11, Opcode::Phi).u64().Inputs(5, 10);
            INST(12, Opcode::Add).u64().Inputs(11, 1);
            INST(13, Opcode::Add).u64().Inputs(12, 0);
            INST(14, Opcode::Return).u64().Inputs(13);
        }
    }
    auto result = GetGraph()->RunPass<RegAllocGraphColoring>();
    ASSERT_TRUE(result);
    GraphChecker(GetGraph()).Check();
    EXPECT_EQ(INS(0).GetDstReg(), INS(3).GetSrcReg(1));
    EXPECT_EQ(INS(0).GetDstReg(), INS(7).GetSrcReg(1));

    // Check affinity group
    EXPECT_EQ(INS(5).GetDstReg(), INS(6).GetDstReg());
    EXPECT_EQ(INS(5).GetDstReg(), INS(9).GetDstReg());
    EXPECT_EQ(INS(5).GetDstReg(), INS(10).GetDstReg());
    EXPECT_EQ(INS(5).GetDstReg(), INS(11).GetDstReg());
}
}  // namespace panda::compiler
