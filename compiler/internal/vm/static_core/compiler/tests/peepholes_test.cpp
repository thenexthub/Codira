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

#include "libarkbase/utils/utils.h"
#include "libarkbase/macros.h"
#include "unit_test.h"
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/peepholes.h"
#include "optimizer/optimizations/branch_elimination.h"
#include "optimizer/optimizations/lowering.h"
#include "optimizer/optimizations/cleanup.h"

namespace ark::compiler {
class PeepholesTest : public CommonTest {
public:
    PeepholesTest()
        : graph_(CreateGraphStartEndBlocks()),
          defaultCompilerSafePointsRequireRegMap_(g_options.IsCompilerSafePointsRequireRegMap())
    {
    }

    ~PeepholesTest() override
    {
        g_options.SetCompilerSafePointsRequireRegMap(defaultCompilerSafePointsRequireRegMap_);
    }

    NO_COPY_SEMANTIC(PeepholesTest);
    NO_MOVE_SEMANTIC(PeepholesTest);

    Graph *GetGraph()
    {
        return graph_;
    }

    void CheckCompare(DataType::Type paramType, ConditionCode origCc, ConditionCode cc)
    {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            CONSTANT(0U, 0U);
            PARAMETER(1U, 0U);
            INS(1U).SetType(paramType);
            PARAMETER(2U, 1U);
            INS(2U).SetType(paramType);
            BASIC_BLOCK(2U, 1U)
            {
                INST(3U, Opcode::Cmp).s32().Inputs(1U, 2U);
                INST(4U, Opcode::Compare).b().CC(origCc).Inputs(3U, 0U);
                INST(5U, Opcode::Return).b().Inputs(4U);
            }
        }
        graph1->RunPass<Peepholes>();
        GraphChecker(graph1).Check();

        auto graph2 = CreateEmptyGraph();
        GRAPH(graph2)
        {
            CONSTANT(0U, 0U);
            PARAMETER(1U, 0U);
            INS(1U).SetType(paramType);
            PARAMETER(2U, 1U);
            INS(2U).SetType(paramType);
            BASIC_BLOCK(2U, 1U)
            {
                INST(3U, Opcode::Cmp).s32().Inputs(1U, 2U);
                INST(4U, Opcode::Compare).b().CC(cc).Inputs(1U, 2U);
                INST(5U, Opcode::Return).b().Inputs(4U);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }

    void CheckCompare(ConditionCode cc, int64_t cst, std::optional<uint64_t> expCst, bool expInv)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            CONSTANT(0U, cst);
            PARAMETER(1U, 0U).b();
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(3U, Opcode::Compare).b().CC(cc).Inputs(1U, 0U);
                INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(5U, Opcode::ReturnVoid);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(6U, Opcode::ReturnVoid);
            }
        }
        graph->RunPass<Peepholes>();
        GraphChecker(graph).Check();

        EXPECT_FALSE(INS(3U).HasUsers());
        if (expCst.has_value()) {
            auto inst = INS(4U).GetInput(0U).GetInst();
            EXPECT_EQ(inst->GetOpcode(), Opcode::Constant);
            EXPECT_EQ(inst->CastToConstant()->GetIntValue(), *expCst);
        } else {
            EXPECT_EQ(INS(4U).GetInput(0U).GetInst(), &INS(1U));
        }
        EXPECT_EQ(INS(4U).CastToIfImm()->GetImm(), 0U);
        EXPECT_EQ(INS(4U).CastToIfImm()->GetCc() == CC_EQ, expInv);
    }

    void CheckCast(DataType::Type srcType, DataType::Type tgtType, bool applied)
    {
        if (srcType == DataType::REFERENCE || tgtType == DataType::REFERENCE) {
            return;
        }
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(srcType);
            BASIC_BLOCK(2U, 1U)
            {
                INST(1U, Opcode::Cast).SrcType(srcType).Inputs(0U);
                INS(1U).SetType(tgtType);
                INST(2U, Opcode::Return).Inputs(1U);
                INS(2U).SetType(tgtType);
            }
        }
        auto graph2 = CreateEmptyGraph();
        if (applied) {
            GRAPH(graph2)
            {
                PARAMETER(0U, 0U);
                INS(0U).SetType(srcType);
                BASIC_BLOCK(2U, 1U)
                {
                    INST(1U, Opcode::Cast).SrcType(srcType).Inputs(0U);
                    INS(1U).SetType(tgtType);
                    INST(2U, Opcode::Return).Inputs(0U);
                    INS(2U).SetType(srcType);
                }
            }
        } else {
            GRAPH(graph2)
            {
                PARAMETER(0U, 0U);
                INS(0U).SetType(srcType);
                BASIC_BLOCK(2U, 1U)
                {
                    INST(1U, Opcode::Cast).SrcType(srcType).Inputs(0U);
                    INS(1U).SetType(tgtType);
                    INST(2U, Opcode::Return).Inputs(1U);
                    INS(2U).SetType(tgtType);
                }
            }
        }
        ASSERT_EQ(graph1->RunPass<Peepholes>(), applied);
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }

    Graph *BuildCheckCastResJoined(DataType::Type srcType, DataType::Type mdlType, DataType::Type tgtType)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(srcType);
            BASIC_BLOCK(2U, 1U)
            {
                INST(1U, Opcode::Cast).SrcType(srcType).Inputs(0U);
                INS(1U).SetType(mdlType);
                INST(2U, Opcode::Cast).SrcType(srcType).Inputs(0U);
                INS(2U).SetType(tgtType);
                INST(3U, Opcode::Return).Inputs(2U);
                INS(3U).SetType(tgtType);
            }
        }
        return graph;
    }

    Graph *BuildCheckCastResNotJoined(DataType::Type srcType, DataType::Type mdlType, DataType::Type tgtType)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(srcType);
            BASIC_BLOCK(2U, 1U)
            {
                INST(1U, Opcode::Cast).SrcType(srcType).Inputs(0U);
                INS(1U).SetType(mdlType);
                INST(2U, Opcode::Cast).SrcType(mdlType).Inputs(1U);
                INS(2U).SetType(tgtType);
                INST(3U, Opcode::Return).Inputs(0U);
                INS(3U).SetType(srcType);
            }
        }
        return graph;
    }

    void CheckCast(DataType::Type srcType, DataType::Type mdlType, DataType::Type tgtType, bool applied, bool joinCast)
    {
        if (srcType == DataType::REFERENCE || tgtType == DataType::REFERENCE || mdlType == DataType::REFERENCE) {
            return;
        }
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(srcType);
            BASIC_BLOCK(2U, 1U)
            {
                INST(1U, Opcode::Cast).SrcType(srcType).Inputs(0U);
                INS(1U).SetType(mdlType);
                INST(2U, Opcode::Cast).SrcType(mdlType).Inputs(1U);
                INS(2U).SetType(tgtType);
                INST(3U, Opcode::Return).Inputs(2U);
                INS(3U).SetType(tgtType);
            }
        }
        Graph *graph2;
        if (applied && joinCast) {
            graph2 = BuildCheckCastResJoined(srcType, mdlType, tgtType);
        } else if (applied) {
            graph2 = BuildCheckCastResNotJoined(srcType, mdlType, tgtType);
        } else {
            graph2 = GraphCloner(graph1, GetAllocator(), GetLocalAllocator()).CloneGraph();
        }
        ASSERT_EQ(graph1->RunPass<Peepholes>(), applied);
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }

    void CheckCast(int i, int j, int k)
    {
        if (i == j || j == k) {
            return;
        }
        auto srcType = static_cast<DataType::Type>(i);
        auto mdlType = static_cast<DataType::Type>(j);
        auto tgtType = static_cast<DataType::Type>(k);
        auto joinCast = DataType::GetTypeSize(srcType, GetArch()) > DataType::GetTypeSize(mdlType, GetArch()) &&
                        DataType::GetTypeSize(mdlType, GetArch()) > DataType::GetTypeSize(tgtType, GetArch());
        CheckCast(srcType, mdlType, tgtType,
                  (srcType == tgtType &&
                   DataType::GetTypeSize(mdlType, GetArch()) > DataType::GetTypeSize(tgtType, GetArch())) ||
                      joinCast,
                  joinCast);
    }

    void CheckCompareFoldIntoTest(uint64_t constant, ConditionCode cc, bool success, ConditionCode expectedCc = CC_EQ)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();
            PARAMETER(1U, 1U).u64();
            CONSTANT(2U, constant);

            BASIC_BLOCK(2U, -1L)
            {
                INST(3U, Opcode::And).u64().Inputs(0U, 1U);
                INST(4U, Opcode::Compare).b().CC(cc).Inputs(3U, 2U);
                INST(5U, Opcode::Return).b().Inputs(4U);
            }
        }

        ASSERT_EQ(graph->RunPass<Peepholes>(), success);
        if (!success) {
            return;
        }

        graph->RunPass<Cleanup>();

        auto expectedGraph = CreateEmptyGraph();
        GRAPH(expectedGraph)
        {
            PARAMETER(0U, 0U).u64();
            PARAMETER(1U, 1U).u64();

            BASIC_BLOCK(2U, -1L)
            {
                INST(4U, Opcode::Compare).b().CC(expectedCc).Inputs(0U, 1U);
                INST(5U, Opcode::Return).b().Inputs(4U);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
    }

    void CheckIfAndZeroFoldIntoIfTest(uint64_t constant, ConditionCode cc, bool success,
                                      ConditionCode expectedCc = CC_EQ)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();
            PARAMETER(1U, 1U).u64();
            CONSTANT(2U, constant);
            CONSTANT(3U, -1L);
            CONSTANT(4U, -2L);

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(5U, Opcode::And).u64().Inputs(0U, 1U);
                INST(6U, Opcode::If).SrcType(DataType::UINT64).CC(cc).Inputs(5U, 2U);
            }

            BASIC_BLOCK(3U, 4U) {}

            BASIC_BLOCK(4U, -1L)
            {
                INST(7U, Opcode::Phi).s64().Inputs(3U, 4U);
                INST(8U, Opcode::Return).s64().Inputs(7U);
            }
        }

        ASSERT_EQ(graph->RunPass<Peepholes>(), success);
        if (!success) {
            return;
        }

        graph->RunPass<Cleanup>();

        auto expectedGraph = CreateEmptyGraph();
        GRAPH(expectedGraph)
        {
            PARAMETER(0U, 0U).u64();
            PARAMETER(1U, 1U).u64();
            CONSTANT(3U, -1L);
            CONSTANT(4U, -2L);

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(6U, Opcode::If).SrcType(DataType::UINT64).CC(expectedCc).Inputs(0U, 1U);
            }

            BASIC_BLOCK(3U, 4U) {}

            BASIC_BLOCK(4U, -1L)
            {
                INST(7U, Opcode::Phi).s64().Inputs(3U, 4U);
                INST(8U, Opcode::Return).s64().Inputs(7U);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
    }

    void CheckCompareLenArrayWithZeroTest(int64_t constant, ConditionCode cc, std::optional<bool> expectedValue,
                                          bool swap = false)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).ref();
            CONSTANT(1U, constant);
            BASIC_BLOCK(2U, -1L)
            {
                INST(3U, Opcode::LenArray).s32().Inputs(0U);
                if (swap) {
                    INST(4U, Opcode::Compare).b().CC(cc).Inputs(1U, 3U);
                } else {
                    INST(4U, Opcode::Compare).b().CC(cc).Inputs(3U, 1U);
                }
                INST(5U, Opcode::Return).b().Inputs(4U);
            }
        }

        ASSERT_EQ(graph->RunPass<Peepholes>(), expectedValue.has_value());
        if (!expectedValue.has_value()) {
            return;
        }

        graph->RunPass<Cleanup>();

        auto expectedGraph = CreateEmptyGraph();
        GRAPH(expectedGraph)
        {
            CONSTANT(1U, *expectedValue);
            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, Opcode::Return).b().Inputs(1U);
            }
        }

        ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
    }

    void CastTest2Addition1MainLoop(int i, int j);

private:
    Graph *graph_ {nullptr};

private:
    bool defaultCompilerSafePointsRequireRegMap_;
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(PeepholesTest, TestAnd1)
{
    // case 1:
    // 0.i64 Const ...
    // 1.i64 AND v0, v5
    // ===>
    // 0.i64 Const 25
    // 1.i64 AND v5, v0
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::And).u64().Inputs(1U, 0U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), &INS(0U));
    ASSERT_EQ(INS(2U).GetInput(1U).GetInst(), &INS(1U));
    ASSERT_EQ(INS(1U).GetUsers().Front().GetIndex(), 1U);
    ASSERT_EQ(INS(0U).GetUsers().Front().GetIndex(), 0U);
}

TEST_F(PeepholesTest, TestAnd2)
{
    // case 2:
    // 0.i64 Const 0xFFF..FF
    // 1.i64 AND v5, v0
    // 2.i64 INS which use v1
    // ===>
    // 0.i64 Const 0xFFF..FF
    // 1.i64 AND v5, v0
    // 2.i64 INS which use v5
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        CONSTANT(1U, -1L);
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::And).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(0U));
}

TEST_F(PeepholesTest, TestAnd2Addition)
{
    // Case 1 + Case 2
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        CONSTANT(1U, -1L);
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::And).u64().Inputs(1U, 0U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(0U));
    ASSERT_EQ(INS(1U).GetUsers().Front().GetIndex(), 1U);
    ASSERT_EQ(INS(0U).GetUsers().Front().GetIndex(), 0U);
}

TEST_F(PeepholesTest, TestAnd3)
{
    // case 3:
    // 1.i64 AND v5, v5
    // 2.i64 INS which use v1
    // ===>
    // 1.i64 AND v5, v5
    // 2.i64 INS which use v5
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::And).u64().Inputs(0U, 0U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(0U));
}

TEST_F(PeepholesTest, TestAnd3Addition)
{
    // case 3:
    // but input's type not equal with and-inst's type
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::And).s16().Inputs(0U, 0U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(2U));
}

TEST_F(PeepholesTest, TestAnd4)
{
    // case 4: De Morgan rules
    // 2.i64 not v0 -> {4}
    // 3.i64 not v1 -> {4}
    // 4.i64 AND v2, v3
    // ===>
    // 5.i64 OR v0, v1
    // 6.i64 not v5
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).u64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Not).u64().Inputs(0U);
            INST(3U, Opcode::Not).u64().Inputs(1U);
            INST(4U, Opcode::And).u64().Inputs(2U, 3U);
            INST(5U, Opcode::Return).u64().Inputs(4U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto notInst = INS(5U).GetInput(0U).GetInst();
    ASSERT_EQ(notInst->GetOpcode(), Opcode::Not);
    auto orInst = notInst->GetInput(0U).GetInst();
    ASSERT_EQ(orInst->GetOpcode(), Opcode::Or);
    ASSERT_EQ(orInst->GetInput(0U).GetInst(), &INS(0U));
    ASSERT_EQ(orInst->GetInput(1U).GetInst(), &INS(1U));
}

TEST_F(PeepholesTest, TestAnd4Addition)
{
    // Case 4, but NOT-inst have more than 1 user
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).u64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Not).u64().Inputs(0U);
            INST(3U, Opcode::Not).u64().Inputs(1U);
            INST(4U, Opcode::And).u64().Inputs(2U, 3U);
            INST(5U, Opcode::Not).u64().Inputs(2U);
            INST(6U, Opcode::Not).u64().Inputs(3U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).u64().InputsAutoType(4U, 5U, 6U, 20U);
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto andInst = INS(7U).GetInput(0U).GetInst();
    ASSERT_EQ(andInst->GetOpcode(), Opcode::And);
}

TEST_F(PeepholesTest, TestOr1)
{
    // case 1:
    // 0.i64 Const ...
    // 1.i64 Or v0, v5
    // ===>
    // 0.i64 Const 25
    // 1.i64 Or v5, v0
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Or).u64().Inputs(1U, 0U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(2U).GetInput(0U).GetInst(), &INS(0U));
    ASSERT_EQ(INS(2U).GetInput(1U).GetInst(), &INS(1U));
    ASSERT_EQ(INS(1U).GetUsers().Front().GetIndex(), 1U);
    ASSERT_EQ(INS(0U).GetUsers().Front().GetIndex(), 0U);
}

TEST_F(PeepholesTest, TestOr2)
{
    // case 2:
    // 0.i64 Const 0x000..00
    // 1.i64 OR v5, v0
    // 2.i64 INS which use v1
    // ===>
    // 0.i64 Const 0x000..00
    // 1.i64 OR v5, v0
    // 2.i64 INS which use v5
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        CONSTANT(1U, 0U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Or).u64().Inputs(1U, 0U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(0U));
}

TEST_F(PeepholesTest, AddConstantTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 1U);
        CONSTANT(3U, 0U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Add).u64().Inputs(2U, 0U);
            INST(5U, Opcode::Add).u64().Inputs(3U, 1U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).u64().InputsAutoType(4U, 5U, 20U);
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_TRUE(CheckInputs(INS(4U), {0U, 2U}));
    ASSERT_TRUE(CheckInputs(INS(5U), {1U, 3U}));
    ASSERT_TRUE(CheckInputs(INS(6U), {4U, 1U, 20U}));

    ASSERT_TRUE(CheckUsers(INS(0U), {4U}));
    ASSERT_TRUE(CheckUsers(INS(2U), {4U}));
    ASSERT_TRUE(CheckUsers(INS(3U), {5U}));
    ASSERT_TRUE(INS(5U).GetUsers().Empty());
}

TEST_F(PeepholesTest, AddConstantTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 5U);
        CONSTANT(2U, 6U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Sub).u64().Inputs(0U, 1U);

            INST(5U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(6U, Opcode::Add).u64().Inputs(2U, 4U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).u64().InputsAutoType(5U, 6U, 20U);
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    auto const3 = INS(2U).GetNext();
    ASSERT_TRUE(const3 != nullptr);
    auto const4 = const3->GetNext();
    ASSERT_TRUE(const4 != nullptr && const4->GetNext() == nullptr);

    ASSERT_TRUE(INS(5U).GetInput(0U) == &INS(0U) && INS(5U).GetInput(1U) == const3);
    ASSERT_TRUE(INS(6U).GetInput(0U) == &INS(0U) && INS(6U).GetInput(1U) == const4);

    ASSERT_TRUE(CheckUsers(INS(0U), {3U, 4U, 5U, 6U}));
    ASSERT_TRUE(CheckUsers(INS(1U), {3U, 4U}));
    ASSERT_TRUE(CheckUsers(*const3, {5U}));
    ASSERT_TRUE(CheckUsers(*const4, {6U}));
    ASSERT_TRUE(INS(2U).GetUsers().Empty());
}

TEST_F(PeepholesTest, AddConstantTest3)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u32();
        CONSTANT(1U, 5U);
        CONSTANT(2U, 6U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).u16().Inputs(0U, 1U);
            INST(4U, Opcode::Sub).u16().Inputs(0U, 1U);

            INST(5U, Opcode::Add).u32().Inputs(2U, 3U);
            INST(6U, Opcode::Add).u32().Inputs(2U, 4U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).u64().InputsAutoType(5U, 6U, 20U);
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(2U).GetNext() == nullptr);

    ASSERT_TRUE(CheckInputs(INS(3U), {0U, 1U}));
    ASSERT_TRUE(CheckInputs(INS(4U), {0U, 1U}));
    ASSERT_TRUE(CheckInputs(INS(5U), {3U, 2U}));
    ASSERT_TRUE(CheckInputs(INS(6U), {4U, 2U}));

    ASSERT_TRUE(CheckUsers(INS(0U), {3U, 4U}));
    ASSERT_TRUE(CheckUsers(INS(1U), {3U, 4U}));
    ASSERT_TRUE(CheckUsers(INS(2U), {5U, 6U}));
    ASSERT_TRUE(CheckUsers(INS(3U), {5U}));
    ASSERT_TRUE(CheckUsers(INS(4U), {6U}));
}

TEST_F(PeepholesTest, AddNegTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Neg).u64().Inputs(0U);
            INST(3U, Opcode::Neg).u64().Inputs(1U);

            INST(4U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(5U, Opcode::Return).u64().Inputs(4U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    auto newNeg = INS(4U).GetNext();

    ASSERT_TRUE(CheckUsers(INS(0U), {2U, 4U}));
    ASSERT_TRUE(CheckUsers(INS(1U), {3U, 4U}));
    ASSERT_TRUE(INS(2U).GetUsers().Empty());
    ASSERT_TRUE(INS(3U).GetUsers().Empty());

    ASSERT(CheckInputs(INS(4U), {0U, 1U}));

    auto user = INS(4U).GetUsers().begin();
    ASSERT_TRUE(user->GetInst() == newNeg);
    ASSERT_FALSE(++user != INS(4U).GetUsers().end());

    ASSERT_TRUE(newNeg->GetInput(0U) == &INS(4U));
    ASSERT_TRUE(CheckUsers(*newNeg, {5U}));

    ASSERT_TRUE(INS(5U).GetInput(0U) == newNeg);
}

TEST_F(PeepholesTest, AddNegTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Neg).u64().Inputs(0U);
            INST(3U, Opcode::Neg).u64().Inputs(1U);

            INST(4U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::CallStatic).u64().InputsAutoType(2U, 4U, 20U);
            INST(5U, Opcode::Return).u64().Inputs(6U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(CheckUsers(INS(0U), {2U}));
    ASSERT_TRUE(CheckUsers(INS(1U), {3U}));

    ASSERT_TRUE(CheckInputs(INS(2U), {0U}));
    ASSERT_TRUE(CheckUsers(INS(2U), {4U, 6U}));
    ASSERT_TRUE(CheckInputs(INS(3U), {1U}));
    ASSERT_TRUE(CheckUsers(INS(3U), {4U}));

    ASSERT_TRUE(CheckInputs(INS(4U), {2U, 3U}));
}

TEST_F(PeepholesTest, AddNegTest3)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        CONSTANT(8U, 1U);
        CONSTANT(9U, 2U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Neg).u64().Inputs(0U);
            INST(3U, Opcode::Neg).u64().Inputs(1U);

            INST(4U, Opcode::Add).u64().Inputs(2U, 8U);
            INST(5U, Opcode::Add).u64().Inputs(3U, 9U);

            INST(6U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(10U, Opcode::CallStatic).u64().InputsAutoType(4U, 5U, 6U, 20U);
            INST(7U, Opcode::Return).u64().Inputs(10U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_TRUE(CheckUsers(INS(0U), {2U}));
    ASSERT_TRUE(CheckUsers(INS(1U), {3U}));
    ASSERT_TRUE(CheckUsers(INS(2U), {4U, 6U}));
    ASSERT_TRUE(CheckUsers(INS(3U), {5U, 6U}));

    ASSERT(CheckInputs(INS(4U), {2U, 8U}));
    ASSERT(CheckInputs(INS(5U), {3U, 9U}));

    ASSERT_TRUE(CheckInputs(INS(6U), {2U, 3U}));
    ASSERT_TRUE(CheckUsers(INS(6U), {10U}));
    ASSERT_TRUE(INS(6U).GetNext()->GetNext() == &INS(10U));

    ASSERT_TRUE(CheckInputs(INS(7U), {10U}));
}

TEST_F(PeepholesTest, AddNegTest4)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Neg).u64().Inputs(0U);
            INST(3U, Opcode::Neg).u64().Inputs(1U);

            INST(4U, Opcode::Add).u64().Inputs(0U, 3U);
            INST(5U, Opcode::Add).u64().Inputs(2U, 1U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).u64().InputsAutoType(4U, 5U, 20U);
            INST(6U, Opcode::Return).u64().Inputs(7U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(CheckUsers(INS(0U), {2U, 4U, 5U}));
    ASSERT_TRUE(CheckUsers(INS(1U), {3U, 4U, 5U}));

    ASSERT_TRUE(INS(2U).GetUsers().Empty());
    ASSERT_TRUE(INS(3U).GetUsers().Empty());

    ASSERT_TRUE(CheckInputs(INS(4U), {0U, 1U}));
    ASSERT_TRUE(INS(4U).GetOpcode() == Opcode::Sub);

    ASSERT_TRUE(CheckInputs(INS(5U), {1U, 0U}));
    ASSERT_TRUE(INS(5U).GetOpcode() == Opcode::Sub);
}

TEST_F(PeepholesTest, AddNegConstOne)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::CallStatic).b().InputsAutoType(1U);
            INST(3U, Opcode::Neg).s32().Inputs(2U);
            INST(4U, Opcode::Add).s32().Inputs(3U, 0U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(6U, 0U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::CallStatic).b().InputsAutoType(1U);
            INST(7U, Opcode::Compare).s32().b().CC(ConditionCode::CC_EQ).SetType(DataType::BOOL).Inputs(2U, 6U);
            INST(5U, Opcode::Return).s32().Inputs(7U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, SameAddAndSubTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Sub).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Add).u64().Inputs(2U, 1U);
            INST(4U, Opcode::Add).u64().Inputs(1U, 2U);
            INST(5U, Opcode::Add).u64().Inputs(3U, 4U);

            INST(6U, Opcode::Add).u32().Inputs(1U, 2U);
            INST(7U, Opcode::Add).u32().Inputs(2U, 1U);
            INST(8U, Opcode::Add).u32().Inputs(6U, 7U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(10U, Opcode::CallStatic).u64().InputsAutoType(5U, 8U, 20U);
            INST(9U, Opcode::Return).u64().Inputs(10U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(3U).GetUsers().Empty());
    ASSERT_TRUE(INS(4U).GetUsers().Empty());
    ASSERT_TRUE(CheckInputs(INS(5U), {0U, 0U}));

    ASSERT_TRUE(CheckInputs(INS(6U), {1U, 2U}));
    ASSERT_TRUE(CheckInputs(INS(7U), {2U, 1U}));
    ASSERT_TRUE(CheckInputs(INS(8U), {6U, 7U}));
}

// (a - b) + (c + b) -> a + c
TEST_F(PeepholesTest, AddSubAndAddSameOperandTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Sub).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Add).u64().Inputs(2U, 1U);
            INST(5U, Opcode::Add).u64().Inputs(3U, 4U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5U).GetOpcode(), Opcode::Add);
    ASSERT_TRUE(CheckInputs(INS(5U), {0U, 2U}));
}

// (a - b) + (b + c) -> a + c
TEST_F(PeepholesTest, AddSubAndAddSameOperandTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Sub).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Add).u64().Inputs(1U, 2U);
            INST(5U, Opcode::Add).u64().Inputs(3U, 4U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5U).GetOpcode(), Opcode::Add);
    ASSERT_TRUE(CheckInputs(INS(5U), {0U, 2U}));
}

// (a + b) + (c - b) -> c + a
TEST_F(PeepholesTest, AddAddAndSubSameOperandTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Sub).u64().Inputs(2U, 1U);
            INST(4U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(5U, Opcode::Add).u64().Inputs(4U, 3U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5U).GetOpcode(), Opcode::Add);
    ASSERT_TRUE(CheckInputs(INS(5U), {2U, 0U}));
}

// (b + a) + (c - b) -> c + a
TEST_F(PeepholesTest, AddAddAndSubSameOperandTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Sub).u64().Inputs(2U, 1U);
            INST(4U, Opcode::Add).u64().Inputs(1U, 0U);
            INST(5U, Opcode::Add).u64().Inputs(4U, 3U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5U).GetOpcode(), Opcode::Add);
    ASSERT_TRUE(CheckInputs(INS(5U), {2U, 0U}));
}

// (a - b) + (b - c) -> a - c
TEST_F(PeepholesTest, AddSubAndSubSameOperandTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Sub).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Sub).u64().Inputs(1U, 2U);
            INST(5U, Opcode::Add).u64().Inputs(3U, 4U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5U).GetOpcode(), Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(5U), {0U, 2U}));
}

// (a - b) + (c - a) -> c - b
TEST_F(PeepholesTest, AddSubAndSubSameOperandTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Sub).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Sub).u64().Inputs(2U, 0U);
            INST(5U, Opcode::Add).u64().Inputs(3U, 4U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5U).GetOpcode(), Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(5U), {2U, 1U}));
}

// a + (0 - b) -> a - b
TEST_F(PeepholesTest, AddSubLeftZeroOperandTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        PARAMETER(2U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Sub).u64().Inputs(1U, 2U);
            INST(4U, Opcode::Add).u64().Inputs(0U, 3U);
            INST(5U, Opcode::Return).u64().Inputs(4U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(4U).GetOpcode(), Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(4U), {0U, 2U}));
}

// (0 - a) + b -> b - a
TEST_F(PeepholesTest, AddSubLeftZeroOperandTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        PARAMETER(2U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Sub).u64().Inputs(1U, 0U);
            INST(4U, Opcode::Add).u64().Inputs(3U, 2U);
            INST(5U, Opcode::Return).u64().Inputs(4U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(4U).GetOpcode(), Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(4U), {2U, 0U}));
}

// (x + a) - (x + b) -> a - b
TEST_F(PeepholesTest, SubAddSameOperandTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Add).u64().Inputs(0U, 2U);
            INST(5U, Opcode::Sub).u64().Inputs(3U, 4U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5U).GetOpcode(), Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(5U), {1U, 2U}));
}

// (a + x) - (x + b) -> a - b
TEST_F(PeepholesTest, SubAddSameOperandTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).u64().Inputs(1U, 0U);
            INST(4U, Opcode::Add).u64().Inputs(0U, 2U);
            INST(5U, Opcode::Sub).u64().Inputs(3U, 4U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5U).GetOpcode(), Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(5U), {1U, 2U}));
}

// (a + x) - (b + x) -> a - b
TEST_F(PeepholesTest, SubAddSameOperandTest3)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).u64().Inputs(1U, 0U);
            INST(4U, Opcode::Add).u64().Inputs(2U, 0U);
            INST(5U, Opcode::Sub).u64().Inputs(3U, 4U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5U).GetOpcode(), Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(5U), {1U, 2U}));
}

// (x + a) - (b + x) -> a - b
TEST_F(PeepholesTest, SubAddSameOperandTest4)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Add).u64().Inputs(2U, 0U);
            INST(5U, Opcode::Sub).u64().Inputs(3U, 4U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    ASSERT_EQ(INS(5U).GetOpcode(), Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(5U), {1U, 2U}));
}

TEST_F(PeepholesTest, SubZeroConstantTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(3U, 0U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Sub).u64().Inputs(0U, 3U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::CallStatic).s32().InputsAutoType(6U, 20U);
            INST(10U, Opcode::Return).s32().Inputs(9U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(6U).GetUsers().Empty());
    ASSERT_TRUE(INS(9U).GetInput(0U) == &INS(0U));
}

TEST_F(PeepholesTest, SubFromZeroConstantTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        CONSTANT(1U, 0U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Sub).s64().Inputs(1U, 0U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Neg).s64().Inputs(0U);
            INST(3U, Opcode::Return).s64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, SubFromZeroConstantFPTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).f64();
        CONSTANT(1U, 0.0);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Sub).f64().Inputs(1U, 0U);
            INST(3U, Opcode::Return).f64().Inputs(2U);
        }
    }

    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
}

TEST_F(PeepholesTest, SubConstantTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 5U);
        CONSTANT(2U, 6U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Sub).u64().Inputs(0U, 1U);

            INST(5U, Opcode::Sub).u64().Inputs(3U, 2U);
            INST(6U, Opcode::Sub).u64().Inputs(4U, 2U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).u64().InputsAutoType(5U, 6U, 20U);
            INST(7U, Opcode::Return).u64().Inputs(8U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    auto const3 = static_cast<ConstantInst *>(INS(2U).GetNext());
    ASSERT_TRUE(const3 != nullptr);
    auto const4 = static_cast<ConstantInst *>(const3->GetNext());
    ASSERT_TRUE(const4 != nullptr && const4->GetNext() == nullptr);
    ASSERT_TRUE(const3->IsEqualConst(1U));
    ASSERT_TRUE(const4->IsEqualConst(11U));

    ASSERT_TRUE(INS(5U).GetInput(0U) == &INS(0U) && INS(5U).GetInput(1U) == const3);
    ASSERT_TRUE(INS(6U).GetInput(0U) == &INS(0U) && INS(6U).GetInput(1U) == const4);

    ASSERT_TRUE(CheckUsers(INS(0U), {3U, 4U, 5U, 6U}));
    ASSERT_TRUE(CheckUsers(INS(1U), {3U, 4U}));
    ASSERT_TRUE(INS(2U).GetUsers().Empty());
    ASSERT_TRUE(CheckUsers(*const3, {5U}));
    ASSERT_TRUE(CheckUsers(*const4, {6U}));
}

TEST_F(PeepholesTest, SubConstantTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 5U);
        CONSTANT(2U, 6U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Sub).u64().Inputs(0U, 1U);

            INST(5U, Opcode::Sub).u32().Inputs(3U, 2U);
            INST(6U, Opcode::Sub).u32().Inputs(4U, 2U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).u64().InputsAutoType(5U, 6U, 20U);
            INST(7U, Opcode::Return).u64().Inputs(8U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(2U).GetNext() == nullptr);

    ASSERT_TRUE(CheckInputs(INS(3U), {0U, 1U}));
    ASSERT_TRUE(CheckInputs(INS(4U), {0U, 1U}));
    ASSERT_TRUE(CheckInputs(INS(5U), {3U, 2U}));
    ASSERT_TRUE(CheckInputs(INS(6U), {4U, 2U}));

    ASSERT_TRUE(CheckUsers(INS(0U), {3U, 4U}));
    ASSERT_TRUE(CheckUsers(INS(1U), {3U, 4U}));
    ASSERT_TRUE(CheckUsers(INS(2U), {5U, 6U}));
    ASSERT_TRUE(CheckUsers(INS(3U), {5U}));
    ASSERT_TRUE(CheckUsers(INS(4U), {6U}));
}

TEST_F(PeepholesTest, SubNegTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Neg).u64().Inputs(0U);
            INST(3U, Opcode::Neg).u64().Inputs(1U);

            INST(4U, Opcode::Sub).u64().Inputs(0U, 3U);
            INST(5U, Opcode::Sub).u64().Inputs(2U, 0U);
            INST(6U, Opcode::Sub).u64().Inputs(2U, 3U);
            INST(7U, Opcode::Sub).u64().Inputs(3U, 2U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).u64().InputsAutoType(4U, 5U, 6U, 7U, 20U);
            INST(9U, Opcode::Return).u64().Inputs(8U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(CheckInputs(INS(4U), {0U, 1U}));
    ASSERT_TRUE(INS(4U).GetOpcode() == Opcode::Add);
    ASSERT_TRUE(CheckInputs(INS(5U), {2U, 0U}));
    ASSERT_TRUE(INS(5U).GetOpcode() == Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(6U), {1U, 0U}));
    ASSERT_TRUE(INS(6U).GetOpcode() == Opcode::Sub);
    ASSERT_TRUE(CheckInputs(INS(7U), {0U, 1U}));
    ASSERT_TRUE(INS(7U).GetOpcode() == Opcode::Sub);
}

TEST_F(PeepholesTest, SameSubAndAddTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).u64().Inputs(0U, 1U);

            INST(4U, Opcode::Sub).u64().Inputs(2U, 0U);
            INST(5U, Opcode::Sub).u64().Inputs(2U, 1U);
            INST(6U, Opcode::Add).u64().Inputs(4U, 5U);
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());

    ASSERT_TRUE(INS(4U).GetUsers().Empty());
    ASSERT_TRUE(INS(5U).GetUsers().Empty());
    ASSERT_TRUE(CheckInputs(INS(6U), {1U, 0U}));
}

TEST_F(PeepholesTest, SameSubAndAddTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Sub).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Sub).u64().Inputs(1U, 0U);

            INST(4U, Opcode::Sub).u64().Inputs(0U, 2U);
            INST(5U, Opcode::Sub).u64().Inputs(3U, 1U);
            INST(6U, Opcode::Add).u64().Inputs(4U, 5U);
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(4U).GetUsers().Empty());
    ASSERT_TRUE(CheckUsers(INS(5U), {6U}));
    ASSERT_TRUE(CheckInputs(INS(6U), {1U, 5U}));
}

TEST_F(PeepholesTest, TestOr2Addition1)
{
    // Case 1 + Case 2
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        CONSTANT(1U, 0U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Or).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(0U));
    ASSERT_EQ(INS(1U).GetUsers().Front().GetIndex(), 1U);
    ASSERT_EQ(INS(0U).GetUsers().Front().GetIndex(), 0U);
}

TEST_F(PeepholesTest, TestOr3)
{
    // case 3:
    // 1.i64 OR v5, v5
    // 2.i64 INS which use v1
    // ===>
    // 1.i64 OR v5, v5
    // 2.i64 INS which use v5
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Or).u64().Inputs(0U, 0U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(0U));
}

TEST_F(PeepholesTest, TestOr4)
{
    // case 4: De Morgan rules
    // 2.i64 not v0 -> {4}
    // 3.i64 not v1 -> {4}
    // 4.i64 OR v2, v3
    // ===>
    // 5.i64 AND v0, v1
    // 6.i64 not v5
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).u64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Not).u64().Inputs(0U);
            INST(3U, Opcode::Not).u64().Inputs(1U);
            INST(4U, Opcode::Or).u64().Inputs(2U, 3U);
            INST(5U, Opcode::Return).u64().Inputs(4U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    auto notInst = INS(5U).GetInput(0U).GetInst();
    ASSERT_EQ(notInst->GetOpcode(), Opcode::Not);
    auto andInst = notInst->GetInput(0U).GetInst();
    ASSERT_EQ(andInst->GetOpcode(), Opcode::And);
    ASSERT_EQ(andInst->GetInput(0U).GetInst(), &INS(0U));
    ASSERT_EQ(andInst->GetInput(1U).GetInst(), &INS(1U));
}

TEST_F(PeepholesTest, TestOr4Addition1)
{
    // Case 4, but NOT-inst have more than 1 user
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).u64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Not).u64().Inputs(0U);
            INST(3U, Opcode::Not).u64().Inputs(1U);
            INST(4U, Opcode::Or).u64().Inputs(2U, 3U);
            INST(5U, Opcode::Not).u64().Inputs(2U);
            INST(6U, Opcode::Not).u64().Inputs(3U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).u64().InputsAutoType(4U, 5U, 6U, 20U);
            INST(7U, Opcode::Return).u64().Inputs(8U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto orInst = INS(8U).GetInput(0U).GetInst();
    ASSERT_EQ(orInst->GetOpcode(), Opcode::Or);
}

TEST_F(PeepholesTest, NegTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Neg).s64().Inputs(0U);
            INST(5U, Opcode::Neg).s64().Inputs(4U);
            INST(6U, Opcode::Add).s64().Inputs(1U, 5U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::CallStatic).u32().InputsAutoType(6U, 20U);
            INST(10U, Opcode::Return).u32().Inputs(11U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(CheckInputs(INS(5U), {4U}));
    ASSERT_TRUE(INS(5U).GetUsers().Empty());
    ASSERT_TRUE(CheckInputs(INS(6U), {1U, 0U}));
    ASSERT_EQ(INS(6U).GetOpcode(), Opcode::Add);
}

TEST_F(PeepholesTest, NegTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Sub).s64().Inputs(0U, 1U);
            INST(3U, Opcode::Neg).s64().Inputs(2U);
            INST(4U, Opcode::Return).s64().Inputs(3U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(3U).GetUsers().Empty());
    ASSERT_NE(INS(3U).GetNext(), &INS(4U));
    auto sub = INS(3U).GetNext();
    ASSERT_EQ(sub->GetOpcode(), Opcode::Sub);
    ASSERT_EQ(sub->GetType(), INS(3U).GetType());

    ASSERT_EQ(sub->GetInput(0U).GetInst(), &INS(1U));
    ASSERT_EQ(sub->GetInput(1U).GetInst(), &INS(0U));

    ASSERT_EQ(INS(4U).GetInput(0U).GetInst(), sub);
}

SRC_GRAPH(NegationCompare, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        CONSTANT(5U, 0x1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Compare).b().CC(CC_GT).Inputs(0U, 1U);
            INST(3U, Opcode::Neg).s32().Inputs(2U);
            INST(6U, Opcode::Add).s32().Inputs(3U, 5U);
            INST(4U, Opcode::Return).b().Inputs(6U);
        }
    }
}

OUT_GRAPH(NegationCompare, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LE).Inputs(0U, 1U);
            INST(4U, Opcode::Return).b().Inputs(2U);
        }
    }
}

SRC_GRAPH(NegationCompareOSR, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        CONSTANT(5U, 0x1U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_GT).Inputs(0U, 1U);
            INST(3U, Opcode::Neg).s32().Inputs(2U);
            INST(6U, Opcode::Add).s32().Inputs(3U, 5U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(4U, Opcode::Return).b().Inputs(6U);
        }
    }
}

OUT_GRAPH(NegationCompareOSR, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        CONSTANT(5U, 0x1U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::Compare).b().CC(CC_LE).Inputs(0U, 1U);
            INST(3U, Opcode::Neg).s32().Inputs(2U);
            INST(6U, Opcode::Add).s32().Inputs(3U, 5U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(4U, Opcode::Return).b().Inputs(2U);
        }
    }
}

TEST_F(PeepholesTest, NegationCompare)
{
    src_graph::NegationCompare::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    auto graph = CreateEmptyGraph();
    out_graph::NegationCompare::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));

    // Test with OSR
    auto defaultGraph = CreateEmptyGraph();
    src_graph::NegationCompareOSR::CREATE(defaultGraph);
    Graph *graphOsr =
        GraphCloner(defaultGraph, defaultGraph->GetAllocator(), defaultGraph->GetLocalAllocator()).CloneGraph();
    graphOsr->SetMode(GraphMode::Osr());

    Graph *graphClone = GraphCloner(graphOsr, graphOsr->GetAllocator(), graphOsr->GetLocalAllocator()).CloneGraph();

    ASSERT_FALSE(graphOsr->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(graphOsr, graphClone));

    auto optimizedGraph = CreateEmptyGraph();
    out_graph::NegationCompareOSR::CREATE(optimizedGraph);
    ASSERT_TRUE(defaultGraph->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(defaultGraph, optimizedGraph));
}

SRC_GRAPH(TransformNegationToCompare, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        CONSTANT(5U, 0x1U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::CallStatic).b().InputsAutoType(1U);
            INST(3U, Opcode::Neg).s32().Inputs(2U);
            INST(6U, Opcode::Add).s32().Inputs(3U, 5U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(4U, Opcode::Return).b().Inputs(6U);
        }
    }
}

OUT_GRAPH(TransformNegationToCompare, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        CONSTANT(5U, 0x1U);
        CONSTANT(7U, 0x0U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::CallStatic).b().InputsAutoType(1U);
            INST(3U, Opcode::Neg).s32().Inputs(2U);
            INST(6U, Opcode::Add).s32().Inputs(3U, 5U);
            INST(8U, Opcode::Compare).b().Inputs(2U, 7U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(4U, Opcode::Return).b().Inputs(8U);
        }
    }
}

TEST_F(PeepholesTest, TransformNegationToCompare)
{
    // Test with OSR
    auto defaultGraph = CreateEmptyGraph();
    src_graph::TransformNegationToCompare::CREATE(defaultGraph);
    Graph *graphOsr =
        GraphCloner(defaultGraph, defaultGraph->GetAllocator(), defaultGraph->GetLocalAllocator()).CloneGraph();
    graphOsr->SetMode(GraphMode::Osr());

    Graph *graphClone = GraphCloner(graphOsr, graphOsr->GetAllocator(), graphOsr->GetLocalAllocator()).CloneGraph();

    ASSERT_FALSE(graphOsr->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(graphOsr, graphClone));
    // Test without OSR
    auto optimizedGraph = CreateEmptyGraph();
    out_graph::TransformNegationToCompare::CREATE(optimizedGraph);
    ASSERT_TRUE(defaultGraph->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(defaultGraph, optimizedGraph));
}

TEST_F(PeepholesTest, NegCompareNotWork)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Compare).b().CC(CC_GT).Inputs(0U, 1U);
            INST(3U, Opcode::Neg).b().Inputs(2U);
            INST(5U, Opcode::Mul).b().Inputs(2U, 3U);
            INST(4U, Opcode::Return).b().Inputs(5U);
        }
    }
    Graph *graphClone =
        GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphClone));
}

SRC_GRAPH(CompareNegation, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        CONSTANT(8U, 0x1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Neg).i32().Inputs(0U);
            INST(9U, Opcode::Add).i32().Inputs(2U, 8U);
            INST(4U, Opcode::Compare).b().CC(ConditionCode::CC_EQ).SrcType(DataType::BOOL).Inputs(1U, 9U);
            INST(7U, Opcode::Return).b().Inputs(4U);
        }
    }
}

OUT_GRAPH(CompareNegation, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Compare).b().CC(ConditionCode::CC_NE).SrcType(DataType::BOOL).Inputs(1U, 0U);
            INST(7U, Opcode::Return).b().Inputs(4U);
        }
    }
}

TEST_F(PeepholesTest, CompareNegation)
{
    src_graph::CompareNegation::CREATE(GetGraph());
    auto graph = CreateEmptyGraph();
    out_graph::CompareNegation::CREATE(graph);
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));

    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        CONSTANT(8U, 0x1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Neg).i32().Inputs(0U);
            INST(9U, Opcode::Add).i32().Inputs(2U, 8U);
            INST(4U, Opcode::Compare).b().CC(ConditionCode::CC_EQ).SrcType(DataType::BOOL).Inputs(1U, 9U);
            INST(11U, Opcode::Add).i32().Inputs(4U, 9U);
            INST(7U, Opcode::Return).b().Inputs(11U);
        }
    }
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        CONSTANT(8U, 0x1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Neg).i32().Inputs(0U);
            INST(9U, Opcode::Add).i32().Inputs(2U, 8U);
            INST(4U, Opcode::Compare).b().CC(ConditionCode::CC_NE).SrcType(DataType::BOOL).Inputs(1U, 0U);
            INST(11U, Opcode::Add).i32().Inputs(4U, 9U);
            INST(7U, Opcode::Return).b().Inputs(11U);
        }
    }
    GraphChecker(graph1).Check();
    ASSERT_TRUE(graph1->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

SRC_GRAPH(CompareNegationBoolOsr, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(1U, 1U).b();
        CONSTANT(3U, 0x1U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).b().InputsAutoType(6U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(2U, Opcode::Neg).s32().Inputs(7U);
            INST(8U, Opcode::Add).s32().Inputs(2U, 3U);
            INST(4U, Opcode::Compare).b().CC(ConditionCode::CC_EQ).SrcType(DataType::BOOL).Inputs(8U, 1U);
            INST(5U, Opcode::Return).b().Inputs(4U);
        }
    }
}

OUT_GRAPH(CompareNegationBoolOsr, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(1U, 1U).b();
        CONSTANT(3U, 0x1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).b().InputsAutoType(6U);

            INST(2U, Opcode::Neg).s32().Inputs(7U);
            INST(8U, Opcode::Add).s32().Inputs(2U, 3U);
            INST(4U, Opcode::Compare).b().CC(ConditionCode::CC_EQ).SrcType(DataType::BOOL).Inputs(8U, 1U);
            INST(5U, Opcode::Return).b().Inputs(4U);
        }
    }
}

TEST_F(PeepholesTest, CompareNegationBoolOsr)
{
    // Peephole don't work with that graph in OSR
    GetGraph()->SetMode(GraphMode::Osr());
    src_graph::CompareNegationBoolOsr::CREATE(GetGraph());
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());

    // Work with that graph
    auto optimizableGraph = CreateEmptyGraph();
    optimizableGraph->SetMode(GraphMode::Osr());
    out_graph::CompareNegationBoolOsr::CREATE(optimizableGraph);

    auto optimizableGraphAfter = CreateEmptyGraph();
    optimizableGraphAfter->SetMode(GraphMode::Osr());
    GRAPH(optimizableGraphAfter)
    {
        PARAMETER(1U, 1U).b();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).b().InputsAutoType(6U);

            INST(4U, Opcode::Compare).b().CC(ConditionCode::CC_NE).SrcType(DataType::BOOL).Inputs(7U, 1U);
            INST(5U, Opcode::Return).b().Inputs(4U);
        }
    }
    ASSERT_TRUE(optimizableGraph->RunPass<Peepholes>());
    ASSERT_TRUE(optimizableGraph->RunPass<Cleanup>());
    ASSERT_TRUE(GraphComparator().Compare(optimizableGraph, optimizableGraphAfter));
}

SRC_GRAPH(IfImmNegation, Graph *graph, ConditionCode code)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        CONSTANT(8U, 0x1U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Neg).b().Inputs(0U);
            INST(9U, Opcode::Add).i32().Inputs(2U, 8U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(1U).CC(code).Inputs(9U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Return).b().Inputs(0U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::Return).b().Inputs(0U);
        }
    }
}

OUT_GRAPH(IfImmNegation, Graph *graph, ConditionCode code)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        CONSTANT(8U, 0x1U);
        CONSTANT(10U, 0x0U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Neg).b().Inputs(0U);
            INST(9U, Opcode::Add).i32().Inputs(2U, 8U);
            INST(11U, Opcode::Compare).b().SrcType(DataType::BOOL).Inputs(0U, 10U);
            // That optimized by combination Compare + IfImm
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(1U).CC(GetInverseConditionCode(code)).Inputs(0U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Return).b().Inputs(0U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::Return).b().Inputs(0U);
        }
    }
}

TEST_F(PeepholesTest, IfImmNegation)
{
    // ConditionCode in IfImm with bool src maybe not only CC_EQ or CC_NE
    std::array<ConditionCode, 4U> codes {ConditionCode::CC_NE, ConditionCode::CC_EQ, ConditionCode::CC_GE,
                                         ConditionCode::CC_B};
    for (auto code : codes) {
        auto graph = CreateEmptyGraph();
        src_graph::IfImmNegation::CREATE(graph, code);

        Graph *finalGraph = nullptr;
        finalGraph = CreateEmptyGraph();
        out_graph::IfImmNegation::CREATE(finalGraph, code);
        ASSERT_TRUE(graph->RunPass<Peepholes>());
        ASSERT_TRUE(GraphComparator().Compare(graph, finalGraph));
    }
}

SRC_GRAPH(IfImmNegationOsr, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        CONSTANT(3U, 0x1U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).b().InputsAutoType(6U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(2U, Opcode::Neg).s32().Inputs(7U);
            INST(9U, Opcode::Add).s32().Inputs(2U, 3U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(1U).CC(ConditionCode::CC_EQ).Inputs(9U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(5U, Opcode::Return).b().Inputs(0U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(8U, Opcode::Return).b().Inputs(0U);
        }
    }
}

OUT_GRAPH(IfImmNegationOsr, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        CONSTANT(3U, 0x1U);
        BASIC_BLOCK(2U, 4U, 5U)
        {
            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).b().InputsAutoType(6U);

            INST(2U, Opcode::Neg).s32().Inputs(7U);
            INST(9U, Opcode::Add).s32().Inputs(2U, 3U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(1U).CC(ConditionCode::CC_EQ).Inputs(9U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(5U, Opcode::Return).b().Inputs(0U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(8U, Opcode::Return).b().Inputs(0U);
        }
    }
}

TEST_F(PeepholesTest, IfImmNegationOsr)
{
    // Peephole don't work with that graph in OSR
    GetGraph()->SetMode(GraphMode::Osr());
    src_graph::IfImmNegationOsr::CREATE(GetGraph());
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());

    // Work with that graph
    auto optimizableGraph = CreateEmptyGraph();
    optimizableGraph->SetMode(GraphMode::Osr());
    out_graph::IfImmNegationOsr::CREATE(optimizableGraph);

    auto optimizableGraphAfter = CreateEmptyGraph();
    optimizableGraphAfter->SetMode(GraphMode::Osr());
    GRAPH(optimizableGraphAfter)
    {
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        CONSTANT(3U, 0x1U);
        CONSTANT(10U, 0x0U);
        BASIC_BLOCK(2U, 4U, 5U)
        {
            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::CallStatic).b().InputsAutoType(6U);

            INST(2U, Opcode::Neg).s32().Inputs(7U);
            INST(9U, Opcode::Add).s32().Inputs(2U, 3U);
            INST(11U, Opcode::Compare).b().SrcType(DataType::BOOL).Inputs(7U, 10U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(1U).CC(ConditionCode::CC_NE).Inputs(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(5U, Opcode::Return).b().Inputs(0U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(8U, Opcode::Return).b().Inputs(0U);
        }
    }
    ASSERT_TRUE(optimizableGraph->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(optimizableGraph, optimizableGraphAfter));
}

// Checking the shift with zero constant
TEST_F(PeepholesTest, ShlZeroTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Shl).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Shl).u64().Inputs(0U, 2U);

            INST(5U, Opcode::Add).u64().Inputs(3U, 4U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(3U).GetUsers().Empty());

    ASSERT_TRUE(CheckInputs(INS(5U), {0U, 4U}));
    ASSERT_EQ(INS(5U).GetOpcode(), Opcode::Add);
}

// Checking repeated shifts for constants with the same types
TEST_F(PeepholesTest, ShlRepeatConstTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 5U);
        CONSTANT(2U, 6U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Shl).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Shl).u64().Inputs(3U, 2U);
            INST(5U, Opcode::Return).u64().Inputs(4U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_NE(INS(2U).GetNext(), nullptr);
    auto const3 = static_cast<ConstantInst *>(INS(2U).GetNext());
    ASSERT_TRUE(const3->IsEqualConst(11U));

    ASSERT_TRUE(INS(3U).GetUsers().Empty());
    ASSERT_EQ(INS(3U).GetOpcode(), Opcode::Shl);
    ASSERT_TRUE(CheckUsers(INS(4U), {5U}));
    ASSERT_EQ(INS(4U).GetOpcode(), Opcode::Shl);
    ASSERT_EQ(INS(4U).GetInput(0U).GetInst(), &INS(0U));
    ASSERT_EQ(INS(4U).GetInput(1U).GetInst(), const3);

    ASSERT_TRUE(CheckInputs(INS(5U), {4U}));
}

// Checking repeated shifts for constants with different types
TEST_F(PeepholesTest, ShlRepeatConstTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u32();
        CONSTANT(1U, 5U);
        CONSTANT(2U, 6U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Shl).u16().Inputs(0U, 1U);
            INST(4U, Opcode::Shl).u32().Inputs(3U, 2U);
            INST(5U, Opcode::Return).u32().Inputs(4U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_EQ(INS(2U).GetNext(), nullptr);

    ASSERT_TRUE(CheckInputs(INS(3U), {0U, 1U}));
    ASSERT_TRUE(CheckUsers(INS(3U), {4U}));
    ASSERT_EQ(INS(3U).GetOpcode(), Opcode::Shl);
    ASSERT_TRUE(CheckInputs(INS(4U), {3U, 2U}));
    ASSERT_TRUE(CheckUsers(INS(4U), {5U}));
    ASSERT_EQ(INS(4U).GetOpcode(), Opcode::Shl);

    ASSERT_TRUE(CheckInputs(INS(5U), {4U}));
}

// Checking the shift for a constant greater than the type size
SRC_GRAPH(ShlBigConstTest, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u32();
        PARAMETER(2U, 2U).u16();
        PARAMETER(3U, 3U).u8();
        CONSTANT(4U, 127U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::Shl).u64().Inputs(0U, 4U);
            INST(6U, Opcode::Shl).u32().Inputs(1U, 4U);
            INST(7U, Opcode::Shl).u16().Inputs(2U, 4U);
            INST(8U, Opcode::Shl).u8().Inputs(3U, 4U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(10U, Opcode::CallStatic).s64().InputsAutoType(5U, 6U, 7U, 8U, 20U);
            INST(9U, Opcode::Return).s64().Inputs(10U);
        }
    }
}

TEST_F(PeepholesTest, ShlBigConstTest)
{
    src_graph::ShlBigConstTest::CREATE(GetGraph());
    GetGraph()->RunPass<Peepholes>();

    ASSERT_NE(INS(4U).GetNext(), nullptr);
    auto const64 = static_cast<ConstantInst *>(INS(4U).GetNext());
    ASSERT_TRUE(const64->IsEqualConst(63U));

    ASSERT_NE(const64->GetNext(), nullptr);
    auto const32 = static_cast<ConstantInst *>(const64->GetNext());
    ASSERT_TRUE(const32->IsEqualConst(31U));

    ASSERT_NE(const32->GetNext(), nullptr);
    auto const16 = static_cast<ConstantInst *>(const32->GetNext());
    ASSERT_TRUE(const16->IsEqualConst(15U));

    ASSERT_NE(const16->GetNext(), nullptr);
    auto const8 = static_cast<ConstantInst *>(const16->GetNext());
    ASSERT_TRUE(const8->IsEqualConst(7U));

    ASSERT_EQ(INS(5U).GetInput(0U).GetInst(), &INS(0U));
    ASSERT_EQ(INS(5U).GetInput(1U).GetInst(), const64);
    ASSERT_EQ(INS(5U).GetOpcode(), Opcode::Shl);

    ASSERT_EQ(INS(6U).GetInput(0U).GetInst(), &INS(1U));
    ASSERT_EQ(INS(6U).GetInput(1U).GetInst(), const32);
    ASSERT_EQ(INS(6U).GetOpcode(), Opcode::Shl);

    ASSERT_EQ(INS(7U).GetInput(0U).GetInst(), &INS(2U));
    ASSERT_EQ(INS(7U).GetInput(1U).GetInst(), const16);
    ASSERT_EQ(INS(7U).GetOpcode(), Opcode::Shl);

    ASSERT_EQ(INS(8U).GetInput(0U).GetInst(), &INS(3U));
    ASSERT_EQ(INS(8U).GetInput(1U).GetInst(), const8);
    ASSERT_EQ(INS(8U).GetOpcode(), Opcode::Shl);
}

TEST_F(PeepholesTest, ShlPlusShrTest)
{
    // applied
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x18U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Shr).s32().Inputs(2U, 1U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(5U, 0xffU);

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::And).s32().Inputs(0U, 5U);
            INST(4U, Opcode::Return).s32().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, ShlPlusShrTest1)
{
    // not applied, different constants
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x18U);
        CONSTANT(5U, 0x10U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Shr).s32().Inputs(2U, 5U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x18U);
        CONSTANT(5U, 0x10U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Shr).s32().Inputs(2U, 5U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, ShlPlusShrTest2)
{
    // not applied, same inputs but not a constant
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Shr).s32().Inputs(2U, 1U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Shr).s32().Inputs(2U, 1U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Checking the shift with zero constant
TEST_F(PeepholesTest, ShrZeroTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Shr).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Shr).u64().Inputs(0U, 2U);

            INST(5U, Opcode::Add).u64().Inputs(3U, 4U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(3U).GetUsers().Empty());

    ASSERT_TRUE(CheckInputs(INS(5U), {0U, 4U}));
    ASSERT_EQ(INS(5U).GetOpcode(), Opcode::Add);
}

// Checking repeated shifts for constants with the same types
TEST_F(PeepholesTest, ShrRepeatConstTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 5U);
        CONSTANT(2U, 6U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Shr).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Shr).u64().Inputs(3U, 2U);
            INST(5U, Opcode::Return).u64().Inputs(4U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_NE(INS(2U).GetNext(), nullptr);
    auto const3 = static_cast<ConstantInst *>(INS(2U).GetNext());
    ASSERT_TRUE(const3->IsEqualConst(11U));

    ASSERT_TRUE(INS(3U).GetUsers().Empty());
    ASSERT_EQ(INS(3U).GetOpcode(), Opcode::Shr);
    ASSERT_TRUE(CheckUsers(INS(4U), {5U}));
    ASSERT_EQ(INS(4U).GetOpcode(), Opcode::Shr);
    ASSERT_EQ(INS(4U).GetInput(0U).GetInst(), &INS(0U));
    ASSERT_EQ(INS(4U).GetInput(1U).GetInst(), const3);

    ASSERT_TRUE(CheckInputs(INS(5U), {4U}));
}

// Checking repeated shifts for constants with different types
TEST_F(PeepholesTest, ShrRepeatConstTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u32();
        CONSTANT(1U, 5U);
        CONSTANT(2U, 6U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Shr).u16().Inputs(0U, 1U);
            INST(4U, Opcode::Shr).u32().Inputs(3U, 2U);
            INST(5U, Opcode::Return).u32().Inputs(4U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_EQ(INS(2U).GetNext(), nullptr);

    ASSERT_TRUE(CheckInputs(INS(3U), {0U, 1U}));
    ASSERT_TRUE(CheckUsers(INS(3U), {4U}));
    ASSERT_EQ(INS(3U).GetOpcode(), Opcode::Shr);
    ASSERT_TRUE(CheckInputs(INS(4U), {3U, 2U}));
    ASSERT_TRUE(CheckUsers(INS(4U), {5U}));
    ASSERT_EQ(INS(4U).GetOpcode(), Opcode::Shr);

    ASSERT_TRUE(CheckInputs(INS(5U), {4U}));
}

// Checking the shift for a constant greater than the type size
SRC_GRAPH(ShrBigConstTest, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u32();
        PARAMETER(2U, 2U).u16();
        PARAMETER(3U, 3U).u8();
        CONSTANT(4U, 127U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::Shr).u64().Inputs(0U, 4U);
            INST(6U, Opcode::Shr).u32().Inputs(1U, 4U);
            INST(7U, Opcode::Shr).u16().Inputs(2U, 4U);
            INST(8U, Opcode::Shr).u8().Inputs(3U, 4U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::CallStatic).u64().InputsAutoType(5U, 6U, 7U, 8U, 20U);
            INST(10U, Opcode::Return).u64().Inputs(9U);
        }
    }
}

TEST_F(PeepholesTest, ShrBigConstTest)
{
    src_graph::ShrBigConstTest::CREATE(GetGraph());
    GetGraph()->RunPass<Peepholes>();

    ASSERT_NE(INS(4U).GetNext(), nullptr);
    auto const64 = static_cast<ConstantInst *>(INS(4U).GetNext());
    ASSERT_TRUE(const64->IsEqualConst(63U));

    ASSERT_NE(const64->GetNext(), nullptr);
    auto const32 = static_cast<ConstantInst *>(const64->GetNext());
    ASSERT_TRUE(const32->IsEqualConst(31U));

    ASSERT_NE(const32->GetNext(), nullptr);
    auto const16 = static_cast<ConstantInst *>(const32->GetNext());
    ASSERT_TRUE(const16->IsEqualConst(15U));

    ASSERT_NE(const16->GetNext(), nullptr);
    auto const8 = static_cast<ConstantInst *>(const16->GetNext());
    ASSERT_TRUE(const8->IsEqualConst(7U));

    ASSERT_EQ(INS(5U).GetInput(0U).GetInst(), &INS(0U));
    ASSERT_EQ(INS(5U).GetInput(1U).GetInst(), const64);
    ASSERT_EQ(INS(5U).GetOpcode(), Opcode::Shr);

    ASSERT_EQ(INS(6U).GetInput(0U).GetInst(), &INS(1U));
    ASSERT_EQ(INS(6U).GetInput(1U).GetInst(), const32);
    ASSERT_EQ(INS(6U).GetOpcode(), Opcode::Shr);

    ASSERT_EQ(INS(7U).GetInput(0U).GetInst(), &INS(2U));
    ASSERT_EQ(INS(7U).GetInput(1U).GetInst(), const16);
    ASSERT_EQ(INS(7U).GetOpcode(), Opcode::Shr);

    ASSERT_EQ(INS(8U).GetInput(0U).GetInst(), &INS(3U));
    ASSERT_EQ(INS(8U).GetInput(1U).GetInst(), const8);
    ASSERT_EQ(INS(8U).GetOpcode(), Opcode::Shr);
}

// Checking the shift with zero constant
TEST_F(PeepholesTest, AShrZeroTest)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::AShr).u64().Inputs(0U, 1U);
            INST(4U, Opcode::AShr).u64().Inputs(0U, 2U);

            INST(5U, Opcode::Add).u64().Inputs(3U, 4U);
            INST(6U, Opcode::Return).u64().Inputs(5U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_TRUE(INS(3U).GetUsers().Empty());

    ASSERT_TRUE(CheckInputs(INS(5U), {0U, 4U}));
    ASSERT_EQ(INS(5U).GetOpcode(), Opcode::Add);
}

// Checking repeated shifts for constants with the same types
TEST_F(PeepholesTest, AShrRepeatConstTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 5U);
        CONSTANT(2U, 6U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::AShr).u64().Inputs(0U, 1U);
            INST(4U, Opcode::AShr).u64().Inputs(3U, 2U);
            INST(5U, Opcode::Return).u64().Inputs(4U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_NE(INS(2U).GetNext(), nullptr);
    auto const3 = static_cast<ConstantInst *>(INS(2U).GetNext());
    ASSERT_TRUE(const3->IsEqualConst(11U));

    ASSERT_TRUE(INS(3U).GetUsers().Empty());
    ASSERT_EQ(INS(3U).GetOpcode(), Opcode::AShr);
    ASSERT_TRUE(CheckUsers(INS(4U), {5U}));
    ASSERT_EQ(INS(4U).GetOpcode(), Opcode::AShr);
    ASSERT_EQ(INS(4U).GetInput(0U).GetInst(), &INS(0U));
    ASSERT_EQ(INS(4U).GetInput(1U).GetInst(), const3);

    ASSERT_TRUE(CheckInputs(INS(5U), {4U}));
}

// Checking repeated shifts for constants with different types
TEST_F(PeepholesTest, AShrRepeatConstTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u32();
        CONSTANT(1U, 5U);
        CONSTANT(2U, 6U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::AShr).u16().Inputs(0U, 1U);
            INST(4U, Opcode::AShr).u32().Inputs(3U, 2U);
            INST(5U, Opcode::Return).u32().Inputs(4U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_EQ(INS(2U).GetNext(), nullptr);

    ASSERT_TRUE(CheckInputs(INS(3U), {0U, 1U}));
    ASSERT_TRUE(CheckUsers(INS(3U), {4U}));
    ASSERT_EQ(INS(3U).GetOpcode(), Opcode::AShr);
    ASSERT_TRUE(CheckInputs(INS(4U), {3U, 2U}));
    ASSERT_TRUE(CheckUsers(INS(4U), {5U}));
    ASSERT_EQ(INS(4U).GetOpcode(), Opcode::AShr);

    ASSERT_TRUE(CheckInputs(INS(5U), {4U}));
}

// Checking the shift for a constant greater than the type size
SRC_GRAPH(AShrBigConstTest, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u32();
        PARAMETER(2U, 2U).u16();
        PARAMETER(3U, 3U).u8();
        CONSTANT(4U, 127U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::AShr).u64().Inputs(0U, 4U);
            INST(6U, Opcode::AShr).u32().Inputs(1U, 4U);
            INST(7U, Opcode::AShr).u16().Inputs(2U, 4U);
            INST(8U, Opcode::AShr).u8().Inputs(3U, 4U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(10U, Opcode::CallStatic).u64().InputsAutoType(5U, 6U, 7U, 8U, 20U);
            INST(9U, Opcode::Return).u64().Inputs(10U);
        }
    }
}

TEST_F(PeepholesTest, AShrBigConstTest)
{
    src_graph::AShrBigConstTest::CREATE(GetGraph());
    GetGraph()->RunPass<Peepholes>();

    ASSERT_NE(INS(4U).GetNext(), nullptr);
    auto const64 = static_cast<ConstantInst *>(INS(4U).GetNext());
    ASSERT_TRUE(const64->IsEqualConst(63U));

    ASSERT_NE(const64->GetNext(), nullptr);
    auto const32 = static_cast<ConstantInst *>(const64->GetNext());
    ASSERT_TRUE(const32->IsEqualConst(31U));

    ASSERT_NE(const32->GetNext(), nullptr);
    auto const16 = static_cast<ConstantInst *>(const32->GetNext());
    ASSERT_TRUE(const16->IsEqualConst(15U));

    ASSERT_NE(const16->GetNext(), nullptr);
    auto const8 = static_cast<ConstantInst *>(const16->GetNext());
    ASSERT_TRUE(const8->IsEqualConst(7U));

    ASSERT_EQ(INS(5U).GetInput(0U).GetInst(), &INS(0U));
    ASSERT_EQ(INS(5U).GetInput(1U).GetInst(), const64);
    ASSERT_EQ(INS(5U).GetOpcode(), Opcode::AShr);

    ASSERT_EQ(INS(6U).GetInput(0U).GetInst(), &INS(1U));
    ASSERT_EQ(INS(6U).GetInput(1U).GetInst(), const32);
    ASSERT_EQ(INS(6U).GetOpcode(), Opcode::AShr);

    ASSERT_EQ(INS(7U).GetInput(0U).GetInst(), &INS(2U));
    ASSERT_EQ(INS(7U).GetInput(1U).GetInst(), const16);
    ASSERT_EQ(INS(7U).GetOpcode(), Opcode::AShr);

    ASSERT_EQ(INS(8U).GetInput(0U).GetInst(), &INS(3U));
    ASSERT_EQ(INS(8U).GetInput(1U).GetInst(), const8);
    ASSERT_EQ(INS(8U).GetOpcode(), Opcode::AShr);
}

TEST_F(PeepholesTest, ShlPlusAShrTest1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x18U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s32().Inputs(0U, 1U);
            INST(3U, Opcode::AShr).s32().Inputs(2U, 1U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Cast).s8().SrcType(DataType::INT32).Inputs(0U);
            INST(4U, Opcode::Return).s32().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, ShlPlusAShrTest2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x10U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s32().Inputs(0U, 1U);
            INST(3U, Opcode::AShr).s32().Inputs(2U, 1U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Cast).s16().SrcType(DataType::INT32).Inputs(0U);
            INST(4U, Opcode::Return).s32().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, ShlPlusAShrTest3)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        CONSTANT(1U, 0x8U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s64().Inputs(0U, 1U);
            INST(3U, Opcode::AShr).s64().Inputs(2U, 1U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Cast).s32().SrcType(DataType::INT64).Inputs(0U);
            INST(4U, Opcode::Return).s32().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, ShlPlusAShrTest4)
{
    // not applied, different constants
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x18U);
        CONSTANT(5U, 0x10U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s32().Inputs(0U, 1U);
            INST(3U, Opcode::AShr).s32().Inputs(2U, 5U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x18U);
        CONSTANT(5U, 0x10U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s32().Inputs(0U, 1U);
            INST(3U, Opcode::AShr).s32().Inputs(2U, 5U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, ShlPlusAShrTest5)
{
    // not applied, same inputs but not a constant
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Shr).s32().Inputs(2U, 1U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Shr).s32().Inputs(2U, 1U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TestMulCase1)
{
    // case 1:
    // 0. Const 1
    // 1. MUL v5, v0 -> {v2, ...}
    // 2. INS v1
    // ===>
    // 0. Const 1
    // 1. MUL v5, v0
    // 2. INS v5
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(3U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Mul).u64().Inputs(0U, 3U);
            INST(12U, Opcode::Mul).u16().Inputs(0U, 3U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(21U, Opcode::CallStatic).u64().InputsAutoType(6U, 12U, 20U);
            INST(14U, Opcode::Return).u64().Inputs(21U);
        }
    }
    GetGraph()->RunPass<Peepholes>();

    ASSERT_EQ(INS(21U).GetInput(0U).GetInst(), &INS(0U));
    ASSERT_TRUE(INS(6U).GetUsers().Empty());

    ASSERT_EQ(INS(21U).GetInput(1U).GetInst(), &INS(0U));
    ASSERT_TRUE(INS(12U).GetUsers().Empty());
}

TEST_F(PeepholesTest, TestMulCase2)
{
    // case 2:
    // 0. Const -1
    // 1. MUL v5, v0
    // 2. INS v1
    // ===>
    // 0. Const -1
    // 1. MUL v5, v0
    // 3. NEG v5 -> {v2, ...}
    // 2. INS v3
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(3U, -1L);
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Mul).u64().Inputs(0U, 3U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(13U, Opcode::CallStatic).u64().InputsAutoType(6U, 20U);
            INST(12U, Opcode::Return).u64().Inputs(13U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Neg).u64().Inputs(0U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(13U, Opcode::CallStatic).u64().InputsAutoType(6U, 20U);
            INST(12U, Opcode::Return).u64().Inputs(13U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TestMulCase3)
{
    // case 3:
    // 0. Const 2
    // 1. MUL v5, v0
    // 2. INS v1
    // ===>
    // 0. Const -1
    // 1. MUL v5, v0
    // 3. ADD v5 , V5 -> {v2, ...}
    // 2. INS v3
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(3U, 2U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Mul).u64().Inputs(0U, 3U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(13U, Opcode::CallStatic).u64().InputsAutoType(6U, 20U);
            INST(12U, Opcode::Return).u64().Inputs(13U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Add).u64().Inputs(0U, 0U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(13U, Opcode::CallStatic).u64().InputsAutoType(6U, 20U);
            INST(12U, Opcode::Return).u64().Inputs(13U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(TestMulCase4, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).u16();
        PARAMETER(13U, 3U).f32();
        PARAMETER(14U, 4U).f64();
        CONSTANT(3U, 4U);
        CONSTANT(4U, 16U);
        CONSTANT(5U, 512U);
        CONSTANT(15U, 4.0F);
        CONSTANT(16U, 16.0_D);
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Mul).u64().Inputs(0U, 3U);
            INST(8U, Opcode::Mul).s64().Inputs(1U, 4U);
            INST(10U, Opcode::Mul).u16().Inputs(2U, 5U);
            INST(17U, Opcode::Mul).f32().Inputs(13U, 15U);
            INST(19U, Opcode::Mul).f64().Inputs(14U, 16U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(21U, Opcode::CallStatic).u64().InputsAutoType(6U, 8U, 10U, 17U, 19U, 20U);
            INST(22U, Opcode::Return).u64().Inputs(21U);
        }
    }
}

OUT_GRAPH(TestMulCase4, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).u16();
        PARAMETER(13U, 3U).f32();
        PARAMETER(14U, 4U).f64();
        CONSTANT(3U, 4U);
        CONSTANT(15U, 4.0F);
        CONSTANT(16U, 16.0_D);
        CONSTANT(23U, 2U);
        CONSTANT(24U, 9U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Shl).u64().Inputs(0U, 23U);
            INST(8U, Opcode::Shl).s64().Inputs(1U, 3U);
            INST(10U, Opcode::Shl).u16().Inputs(2U, 24U);
            INST(17U, Opcode::Mul).f32().Inputs(13U, 15U);
            INST(19U, Opcode::Mul).f64().Inputs(14U, 16U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(21U, Opcode::CallStatic).u64().InputsAutoType(6U, 8U, 10U, 17U, 19U, 20U);
            INST(22U, Opcode::Return).u64().Inputs(21U);
        }
    }
}

TEST_F(PeepholesTest, TestMulCase4)
{
    // case 4:
    // 0. Const 2^N
    // 1. MUL v5, v0
    // 2. INS v1
    // ===>
    // 0. Const -1
    // 1. MUL v5, v0
    // 3. SHL v5 , N -> {v2, ...}
    // 2. INS v3
    src_graph::TestMulCase4::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    out_graph::TestMulCase4::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TestDivCase1)
{
    // case 1:
    // 0. Const 1
    // 1. DIV v5, v0 -> {v2, ...}
    // 2. INS v1
    // ===>
    // 0. Const 1
    // 1. DIV v5, v0
    // 3. MOV v5 -> {v2, ...}
    // 2. INS v3
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(3U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Div).u64().Inputs(0U, 3U);
            INST(12U, Opcode::Div).u16().Inputs(0U, 3U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(21U, Opcode::CallStatic).u64().InputsAutoType(6U, 12U, 20U);
            INST(22U, Opcode::Return).u64().Inputs(21U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();

    ASSERT_EQ(INS(21U).GetInput(0U).GetInst(), &INS(0U));
    ASSERT_TRUE(INS(6U).GetUsers().Empty());

    ASSERT_EQ(INS(21U).GetInput(1U).GetInst(), &INS(0U));
    ASSERT_TRUE(INS(12U).GetUsers().Empty());
}

TEST_F(PeepholesTest, TestDivCase2)
{
    // case 2:
    // 0. Const -1
    // 1. DIV v5, v0 -> {v2, ...}
    // 2. INS v1
    // ===>
    // 0. Const -1
    // 1. DIV v5, v0
    // 3. NEG v5 -> {v2, ...}
    // 2. INS v3
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(3U, -1L);
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Div).u64().Inputs(0U, 3U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(21U, Opcode::CallStatic).u64().InputsAutoType(6U, 20U);
            INST(22U, Opcode::Return).u64().Inputs(21U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Neg).u64().Inputs(0U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(13U, Opcode::CallStatic).u64().InputsAutoType(6U, 20U);
            INST(12U, Opcode::Return).u64().Inputs(13U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TestDivCase3Unsigned)
{
    for (auto type : {DataType::UINT8, DataType::UINT16, DataType::UINT32, DataType::UINT64}) {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(type);
            CONSTANT(1U, 4U);
            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, Opcode::Div).Inputs(0U, 1U);
                INS(2U).SetType(type);
                INST(3U, Opcode::Return).Inputs(2U);
                INS(3U).SetType(type);
            }
        }
        ASSERT_TRUE(graph1->RunPass<Peepholes>());
        graph1->RunPass<Cleanup>();
        auto graph2 = CreateEmptyGraph();
        GRAPH(graph2)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(type);
            CONSTANT(1U, 2U);
            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, Opcode::Shr).Inputs(0U, 1U);
                INS(2U).SetType(type);
                INST(3U, Opcode::Return).Inputs(2U);
                INS(3U).SetType(type);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(PeepholesTest, TestDivCase3SignedPositive)
{
    for (auto type : {DataType::INT8, DataType::INT16, DataType::INT32, DataType::INT64}) {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(type);
            CONSTANT(1U, 4U);
            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, Opcode::Div).Inputs(0U, 1U);
                INS(2U).SetType(type);
                INST(3U, Opcode::Return).Inputs(2U);
                INS(3U).SetType(type);
            }
        }

        ASSERT_FALSE(graph1->RunPass<Peepholes>());
        graph1->RunPass<Cleanup>();

        auto graph2 = CreateEmptyGraph();
        GRAPH(graph2)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(type);
            CONSTANT(1U, 4U);
            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, Opcode::Div).Inputs(0U, 1U);
                INS(2U).SetType(type);
                INST(3U, Opcode::Return).Inputs(2U);
                INS(3U).SetType(type);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(PeepholesTest, TestDivCase3SignedNegative)
{
    for (auto type : {DataType::INT8, DataType::INT16, DataType::INT32, DataType::INT64}) {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(type);
            CONSTANT(1U, -16L);
            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, Opcode::Div).Inputs(0U, 1U);
                INS(2U).SetType(type);
                INST(3U, Opcode::Return).Inputs(2U);
                INS(3U).SetType(type);
            }
        }

        ASSERT_FALSE(graph1->RunPass<Peepholes>());
        graph1->RunPass<Cleanup>();

        auto graph2 = CreateEmptyGraph();
        GRAPH(graph2)
        {
            PARAMETER(0U, 0U);
            INS(0U).SetType(type);
            CONSTANT(1U, -16L);
            BASIC_BLOCK(2U, -1L)
            {
                INST(2U, Opcode::Div).Inputs(0U, 1U);
                INS(2U).SetType(type);
                INST(3U, Opcode::Return).Inputs(2U);
                INS(3U).SetType(type);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(PeepholesTest, TestLenArray1)
{
    // 1. .... ->{v2}
    // 2. NewArray v1 ->{v3,..}
    // 3. LenArray v2 ->{v4, v5...}
    // ===>
    // 1. .... ->{v2, v4, v5, ...}
    // 2. NewArray v1 ->{v3,..}
    // 3. LenArray v2
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(1U, Opcode::NewArray).ref().Inputs(44U, 0U);
            INST(2U, Opcode::LenArray).s32().Inputs(1U);
            INST(3U, Opcode::Return).s32().Inputs(2U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto cnst = INS(3U).GetInput(0U).GetInst();
    ASSERT_EQ(cnst->GetOpcode(), Opcode::Constant);
    ASSERT_TRUE(INS(2U).GetUsers().Empty());
}

TEST_F(PeepholesTest, TestLenArray2)
{
    // 1. .... ->{v2}
    // 2. NewArray v1 ->{v3,..}
    // 3. LenArray v2 ->{v4, v5...}
    // ===>
    // 1. .... ->{v2, v4, v5, ...}
    // 2. NewArray v1 ->{v3,..}
    // 3. LenArray v2
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(10U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(1U, Opcode::NegativeCheck).s32().Inputs(0U, 10U);
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(10U).TypeId(68U);
            INST(2U, Opcode::NewArray).ref().Inputs(44U, 1U);
            INST(3U, Opcode::NullCheck).ref().Inputs(2U, 10U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto cnst = INS(5U).GetInput(0U).GetInst();
    ASSERT_EQ(cnst->GetOpcode(), Opcode::Constant);
    ASSERT_TRUE(INS(4U).GetUsers().Empty());
}

SRC_GRAPH(TestPhi1, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::Sub).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::Sub).s32().Inputs(1U, 0U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(5U, 7U);
            INST(10U, Opcode::Phi).s32().Inputs(5U, 7U);
            INST(11U, Opcode::Add).s32().Inputs(9U, 0U);
            INST(12U, Opcode::Add).s32().Inputs(10U, 1U);
            INST(13U, Opcode::Add).s32().Inputs(11U, 12U);
            INST(14U, Opcode::Return).s32().Inputs(13U);
        }
    }
}

OUT_GRAPH(TestPhi1, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::Sub).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::Sub).s32().Inputs(1U, 0U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(5U, 7U);
            INST(10U, Opcode::Phi).s32().Inputs(5U, 7U);
            INST(11U, Opcode::Add).s32().Inputs(9U, 0U);
            INST(12U, Opcode::Add).s32().Inputs(9U, 1U);
            INST(13U, Opcode::Add).s32().Inputs(11U, 12U);
            INST(14U, Opcode::Return).s32().Inputs(13U);
        }
    }
}

TEST_F(PeepholesTest, TestPhi1)
{
    // Users isn't intersect
    // 2.type  Phi v0, v1 -> v4
    // 3.type  Phi v0, v1 -> v5
    // ===>
    // 2.type  Phi v0, v1 -> v4, v5
    // 3.type  Phi v0, v1
    src_graph::TestPhi1::CREATE(GetGraph());
    GetGraph()->RunPass<Peepholes>();
    auto graph = CreateEmptyGraph();
    out_graph::TestPhi1::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(PhiTestWithChecks, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::NullCheck).ref().Inputs(0U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::NullCheck).ref().Inputs(1U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(9U, Opcode::Phi).ref().Inputs(5U, 7U);
            INST(10U, Opcode::Phi).ref().Inputs(0U, 1U);
            INST(12U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(11U, Opcode::CallStatic).s32().InputsAutoType(9U, 10U, 12U);
            INST(14U, Opcode::Return).s32().Inputs(11U);
        }
    }
}

OUT_GRAPH(PhiTestWithChecks, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::NullCheck).ref().Inputs(0U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::NullCheck).ref().Inputs(1U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(9U, Opcode::Phi).ref().Inputs(5U, 7U);
            INST(10U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(11U, Opcode::CallStatic).s32().InputsAutoType(9U, 9U, 10U);
            INST(14U, Opcode::Return).s32().Inputs(11U);
        }
    }
}

TEST_F(PeepholesTest, PhiTestWithChecks)
{
    // Users isn't intersect
    // 2.type  Phi v0, v1 -> v4
    // 3.type  Phi v0, v1 -> v5
    // ===>
    // 2.type  Phi v0, v1 -> v4, v5
    // 3.type  Phi v0, v1
    src_graph::PhiTestWithChecks::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto graph = CreateEmptyGraph();
    out_graph::PhiTestWithChecks::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(TestPhi2, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::Sub).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::Sub).s32().Inputs(1U, 0U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(5U, 7U);
            INST(10U, Opcode::Phi).s32().Inputs(5U, 7U);
            INST(11U, Opcode::Add).s32().Inputs(9U, 0U);
            INST(12U, Opcode::Add).s32().Inputs(9U, 10U);
            INST(13U, Opcode::Add).s32().Inputs(10U, 1U);
            INST(14U, Opcode::Add).s32().Inputs(11U, 12U);
            INST(15U, Opcode::Add).s32().Inputs(13U, 14U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(16U, Opcode::CallStatic).v0id().InputsAutoType(9U, 10U, 15U, 20U);
            ;
            INST(17U, Opcode::Return).s32().Inputs(15U);
        }
    }
}

OUT_GRAPH(TestPhi2, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::Sub).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::Sub).s32().Inputs(1U, 0U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(5U, 7U);
            INST(10U, Opcode::Phi).s32().Inputs(5U, 7U);
            INST(11U, Opcode::Add).s32().Inputs(9U, 0U);
            INST(12U, Opcode::Add).s32().Inputs(9U, 9U);
            INST(13U, Opcode::Add).s32().Inputs(9U, 1U);
            INST(14U, Opcode::Add).s32().Inputs(11U, 12U);
            INST(15U, Opcode::Add).s32().Inputs(13U, 14U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(16U, Opcode::CallStatic).v0id().InputsAutoType(9U, 9U, 15U, 20U);
            INST(17U, Opcode::Return).s32().Inputs(15U);
        }
    }
}

TEST_F(PeepholesTest, TestPhi2)
{
    // Users is intersect
    // 2.type  Phi v0, v1 -> v4, v5
    // 3.type  Phi v0, v1 -> v5, v6
    // ===>
    // 2.type  Phi v0, v1 -> v4, v5
    // 3.type  Phi v0, v1
    src_graph::TestPhi2::CREATE(GetGraph());
    GetGraph()->RunPass<Peepholes>();
    auto graph = CreateEmptyGraph();
    out_graph::TestPhi2::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(TestPhi3, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::Sub).s32().Inputs(0U, 1U);  // Phi 11
            INST(6U, Opcode::Add).s32().Inputs(0U, 1U);  // Phi 12
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(8U, Opcode::Sub).s32().Inputs(1U, 0U);  // Phi 11
            INST(9U, Opcode::Add).s32().Inputs(0U, 1U);  // Phi 12
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(11U, Opcode::Phi).s32().Inputs(5U, 8U);
            INST(12U, Opcode::Phi).s32().Inputs(6U, 9U);
            INST(13U, Opcode::Add).s32().Inputs(11U, 12U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).v0id().InputsAutoType(11U, 12U, 13U, 20U);
            INST(15U, Opcode::Return).s32().Inputs(13U);
        }
    }
}

OUT_GRAPH(TestPhi3, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::Sub).s32().Inputs(0U, 1U);  // Phi 11
            INST(6U, Opcode::Add).s32().Inputs(0U, 1U);  // Phi 12
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(8U, Opcode::Sub).s32().Inputs(1U, 0U);  // Phi 11
            INST(9U, Opcode::Add).s32().Inputs(0U, 1U);  // Phi 12
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(11U, Opcode::Phi).s32().Inputs(5U, 8U);
            INST(12U, Opcode::Phi).s32().Inputs(6U, 9U);
            INST(13U, Opcode::Add).s32().Inputs(11U, 12U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).v0id().InputsAutoType(11U, 12U, 13U, 20U);
            INST(15U, Opcode::Return).s32().Inputs(13U);
        }
    }
}

TEST_F(PeepholesTest, TestPhi3)
{
    // Peephole rule isn't applied.
    // Same types, different inputs.
    src_graph::TestPhi3::CREATE(GetGraph());
    GetGraph()->RunPass<Peepholes>();
    auto graph = CreateEmptyGraph();
    out_graph::TestPhi3::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(TestPhi4, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::Sub).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(8U, Opcode::Sub).s32().Inputs(1U, 0U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(11U, Opcode::Phi).s16().Inputs(5U, 8U);
            INST(12U, Opcode::Phi).s32().Inputs(5U, 8U);
            INST(13U, Opcode::Add).s32().Inputs(11U, 12U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).v0id().InputsAutoType(11U, 12U, 13U, 20U);
            INST(15U, Opcode::Return).s32().Inputs(13U);
        }
    }
}

OUT_GRAPH(TestPhi4, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::Sub).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(8U, Opcode::Sub).s32().Inputs(1U, 0U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(11U, Opcode::Phi).s16().Inputs(5U, 8U);
            INST(12U, Opcode::Phi).s32().Inputs(5U, 8U);
            INST(13U, Opcode::Add).s32().Inputs(11U, 12U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(14U, Opcode::CallStatic).v0id().InputsAutoType(11U, 12U, 13U, 20U);
            INST(15U, Opcode::Return).s32().Inputs(13U);
        }
    }
}

TEST_F(PeepholesTest, TestPhi4)
{
    // Peephole rule isn't applied.
    // Different types, same inputs.
    src_graph::TestPhi4::CREATE(GetGraph());
    GetGraph()->RunPass<Peepholes>();
    auto graph = CreateEmptyGraph();
    out_graph::TestPhi4::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(TestPhi5, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(17U, 2U).f32();
        PARAMETER(18U, 3U).f32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::Sub).s32().Inputs(0U, 1U);    // Phi 11
            INST(6U, Opcode::Add).f32().Inputs(18U, 17U);  // Phi 12
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(8U, Opcode::Sub).s32().Inputs(1U, 0U);    // Phi 11
            INST(9U, Opcode::Add).f32().Inputs(18U, 17U);  // Phi 12
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(11U, Opcode::Phi).s32().Inputs(5U, 8U);
            INST(12U, Opcode::Phi).f32().Inputs(6U, 9U);
            INST(13U, Opcode::Cast).f32().SrcType(DataType::INT32).Inputs(11U);
            INST(14U, Opcode::Add).f32().Inputs(13U, 12U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(15U, Opcode::CallStatic).v0id().InputsAutoType(11U, 12U, 13U, 14U, 20U);
            INST(16U, Opcode::Return).f32().Inputs(14U);
        }
    }
}

OUT_GRAPH(TestPhi5, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(17U, 2U).f32();
        PARAMETER(18U, 3U).f32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::Sub).s32().Inputs(0U, 1U);    // Phi 11
            INST(6U, Opcode::Add).f32().Inputs(18U, 17U);  // Phi 12
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(8U, Opcode::Sub).s32().Inputs(1U, 0U);    // Phi 11
            INST(9U, Opcode::Add).f32().Inputs(18U, 17U);  // Phi 12
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(11U, Opcode::Phi).s32().Inputs(5U, 8U);
            INST(12U, Opcode::Phi).f32().Inputs(6U, 9U);
            INST(13U, Opcode::Cast).f32().SrcType(DataType::INT32).Inputs(11U);
            INST(14U, Opcode::Add).f32().Inputs(13U, 12U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(15U, Opcode::CallStatic).v0id().InputsAutoType(11U, 12U, 13U, 14U, 20U);
            INST(16U, Opcode::Return).f32().Inputs(14U);
        }
    }
}

TEST_F(PeepholesTest, TestPhi5)
{
    // Peephole rule isn't applied.
    // Different types, different inputs.
    src_graph::TestPhi5::CREATE(GetGraph());
    GetGraph()->RunPass<Peepholes>();
    auto graph = CreateEmptyGraph();
    out_graph::TestPhi5::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, MultiplePeepholeTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 100U);
        CONSTANT(1U, -46L);
        CONSTANT(2U, 0U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().SrcType(DataType::Type::INT64).CC(CC_LT).Inputs(1U, 2U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(5U, Opcode::Sub).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(8U, Opcode::Add).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(11U, Opcode::Phi).s32().Inputs(5U, 8U);
            INST(16U, Opcode::Return).s32().Inputs(11U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GetGraph()->RunPass<BranchElimination>();
    GetGraph()->RunPass<Cleanup>();
    GetGraph()->RunPass<Peepholes>();
#ifndef NDEBUG
    GetGraph()->SetLowLevelInstructionsEnabled();
#endif
    GetGraph()->RunPass<Lowering>();
    GetGraph()->RunPass<Cleanup>();

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(5U, 1U)
        {
            INST(16U, Opcode::ReturnI).s32().Imm(146U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTest)
{
    // test case 2
    CheckCompare(DataType::UINT32, ConditionCode::CC_LT, ConditionCode::CC_B);
    CheckCompare(DataType::UINT32, ConditionCode::CC_LE, ConditionCode::CC_BE);
    CheckCompare(DataType::UINT32, ConditionCode::CC_GT, ConditionCode::CC_A);
    CheckCompare(DataType::UINT32, ConditionCode::CC_GE, ConditionCode::CC_AE);
    CheckCompare(DataType::UINT32, ConditionCode::CC_EQ, ConditionCode::CC_EQ);
    CheckCompare(DataType::UINT32, ConditionCode::CC_NE, ConditionCode::CC_NE);

    CheckCompare(DataType::INT32, ConditionCode::CC_LT, ConditionCode::CC_LT);
    CheckCompare(DataType::INT32, ConditionCode::CC_LE, ConditionCode::CC_LE);
    CheckCompare(DataType::INT32, ConditionCode::CC_GT, ConditionCode::CC_GT);
    CheckCompare(DataType::INT32, ConditionCode::CC_GE, ConditionCode::CC_GE);
    CheckCompare(DataType::INT32, ConditionCode::CC_EQ, ConditionCode::CC_EQ);
    CheckCompare(DataType::INT32, ConditionCode::CC_NE, ConditionCode::CC_NE);

    CheckCompare(DataType::INT32, ConditionCode::CC_B, ConditionCode::CC_B);
    CheckCompare(DataType::INT32, ConditionCode::CC_BE, ConditionCode::CC_BE);
    CheckCompare(DataType::INT32, ConditionCode::CC_A, ConditionCode::CC_A);
    CheckCompare(DataType::INT32, ConditionCode::CC_AE, ConditionCode::CC_AE);

    CheckCompare(DataType::UINT64, ConditionCode::CC_LT, ConditionCode::CC_B);
    CheckCompare(DataType::UINT64, ConditionCode::CC_LE, ConditionCode::CC_BE);
    CheckCompare(DataType::UINT64, ConditionCode::CC_GT, ConditionCode::CC_A);
    CheckCompare(DataType::UINT64, ConditionCode::CC_GE, ConditionCode::CC_AE);
    CheckCompare(DataType::UINT64, ConditionCode::CC_EQ, ConditionCode::CC_EQ);
    CheckCompare(DataType::UINT64, ConditionCode::CC_NE, ConditionCode::CC_NE);

    CheckCompare(DataType::INT64, ConditionCode::CC_LT, ConditionCode::CC_LT);
    CheckCompare(DataType::INT64, ConditionCode::CC_LE, ConditionCode::CC_LE);
    CheckCompare(DataType::INT64, ConditionCode::CC_GT, ConditionCode::CC_GT);
    CheckCompare(DataType::INT64, ConditionCode::CC_GE, ConditionCode::CC_GE);
    CheckCompare(DataType::INT64, ConditionCode::CC_EQ, ConditionCode::CC_EQ);
    CheckCompare(DataType::INT64, ConditionCode::CC_NE, ConditionCode::CC_NE);

    CheckCompare(DataType::INT64, ConditionCode::CC_B, ConditionCode::CC_B);
    CheckCompare(DataType::INT64, ConditionCode::CC_BE, ConditionCode::CC_BE);
    CheckCompare(DataType::INT64, ConditionCode::CC_A, ConditionCode::CC_A);
    CheckCompare(DataType::INT64, ConditionCode::CC_AE, ConditionCode::CC_AE);
}

TEST_F(PeepholesTest, CompareTest1)
{
    // applied case 2
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        PARAMETER(1U, 0U).u32();
        PARAMETER(2U, 1U).u32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Cmp).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(3U, 0U);
            INST(5U, Opcode::Return).b().Inputs(4U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        PARAMETER(1U, 0U).u32();
        PARAMETER(2U, 1U).u32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Cmp).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Compare).b().CC(CC_B).Inputs(1U, 2U);
            INST(5U, Opcode::Return).b().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTest1Addition0)
{
    // cmp with zero and inputs are signed, compare operands are in normal order (constant is the second operand)
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        PARAMETER(1U, 1U).u32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Cmp).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Compare).b().CC(CC_LT).Inputs(2U, 0U);
            INST(4U, Opcode::Return).b().Inputs(3U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        PARAMETER(1U, 1U).u32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::Cmp).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Compare).b().CC(CC_B).Inputs(0U, 1U);
            INST(4U, Opcode::Return).b().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTest1Addition1)
{
    // cmp inputs are unsigned, compare operands are in reverse order (constant is the first operand)
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        PARAMETER(1U, 0U).u32();
        PARAMETER(2U, 1U).u32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Cmp).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 3U);
            INST(5U, Opcode::Return).b().Inputs(4U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        PARAMETER(1U, 0U).u32();
        PARAMETER(2U, 1U).u32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Cmp).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Compare).b().CC(CC_A).Inputs(1U, 2U);
            INST(5U, Opcode::Return).b().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTest1Addition2)
{
    // cmp inputs are signed and compare operands are in normal order
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        PARAMETER(1U, 0U).s32();
        PARAMETER(2U, 1U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Cmp).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(3U, 0U);
            INST(5U, Opcode::Return).b().Inputs(4U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        PARAMETER(1U, 0U).s32();
        PARAMETER(2U, 1U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Cmp).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(1U, 2U);
            INST(5U, Opcode::Return).b().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTest1Addition3)
{
    // cmp inputs are signed and compare operands are in reverse order
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        PARAMETER(1U, 0U).s32();
        PARAMETER(2U, 1U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Cmp).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 3U);
            INST(5U, Opcode::Return).b().Inputs(4U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        PARAMETER(1U, 0U).s32();
        PARAMETER(2U, 1U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Cmp).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Compare).b().CC(CC_GT).Inputs(1U, 2U);
            INST(5U, Opcode::Return).b().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTest2)
{
    // not applied, Compare with non zero constant
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        PARAMETER(1U, 0U).s32();
        PARAMETER(2U, 1U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Cmp).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(3U, 0U);
            INST(5U, Opcode::Return).b().Inputs(4U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);
        PARAMETER(1U, 0U).s32();
        PARAMETER(2U, 1U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Cmp).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(3U, 0U);
            INST(5U, Opcode::Return).b().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTest3)
{
    // not applied, cmp have more than 1 users.
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        PARAMETER(1U, 0U).s32();
        PARAMETER(2U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Cmp).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(3U, 0U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 1U)
        {
            INST(6U, Opcode::Return).s32().Inputs(3U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(7U, Opcode::Return).s32().Inputs(3U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);
        PARAMETER(1U, 0U).s32();
        PARAMETER(2U, 1U).s32();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Cmp).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(3U, 0U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 1U)
        {
            INST(6U, Opcode::Return).s32().Inputs(3U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(7U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

static bool CompareBoolWithConst(ConditionCode cc, bool input, int cst)
{
    switch (cc) {
        case CC_EQ:
            return static_cast<int>(input) == cst;
        case CC_NE:
            return static_cast<int>(input) != cst;
        case CC_LT:
            return static_cast<int>(input) < cst;
        case CC_LE:
            return static_cast<int>(input) <= cst;
        case CC_GT:
            return static_cast<int>(input) > cst;
        case CC_GE:
            return static_cast<int>(input) >= cst;
        case CC_B:
            return static_cast<uint64_t>(input) < static_cast<uint64_t>(cst);
        case CC_BE:
            return static_cast<uint64_t>(input) <= static_cast<uint64_t>(cst);
        case CC_A:
            return static_cast<uint64_t>(input) > static_cast<uint64_t>(cst);
        case CC_AE:
            return static_cast<uint64_t>(input) >= static_cast<uint64_t>(cst);
        default:
            UNREACHABLE();
    }
}

TEST_F(PeepholesTest, CompareTest4)
{
    for (auto cc : {CC_EQ, CC_NE, CC_LT, CC_LE, CC_GT, CC_GE, CC_B, CC_BE, CC_A, CC_AE}) {
        for (auto cst : {-2L, -1L, 0L, 1L, 2L}) {
            auto inputTrue = CompareBoolWithConst(cc, true, cst);
            auto inputFalse = CompareBoolWithConst(cc, false, cst);
            if (inputTrue && inputFalse) {
                CheckCompare(cc, cst, {1U}, false);
            } else if (!inputTrue && !inputFalse) {
                CheckCompare(cc, cst, {0U}, false);
            } else if (inputTrue && !inputFalse) {
                CheckCompare(cc, cst, std::nullopt, false);
            } else {
                CheckCompare(cc, cst, std::nullopt, true);
            }
        }
    }
}

TEST_F(PeepholesTest, CompareTestCmpWithZero1)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 10U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Cast).f64().SrcType(DataType::INT32).Inputs(1U);
            INST(4U, Opcode::Cast).f64().SrcType(DataType::INT32).Inputs(2U);
            INST(5U, Opcode::Cmp).s32().Inputs(3U, 4U);
            INST(6U, Opcode::Compare).b().CC(CC_LT).Inputs(5U, 0U);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 10U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_LT).Inputs(1U, 2U);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTestCmpWithZero2)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 10.0F);
        PARAMETER(2U, 1U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Cast).f32().SrcType(DataType::INT32).Inputs(2U);
            INST(4U, Opcode::Cmp).s32().Inputs(3U, 1U);
            INST(5U, Opcode::Compare).b().CC(CC_LT).Inputs(4U, 0U);
            INST(6U, Opcode::Return).b().Inputs(5U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GetGraph()->RunPass<Cleanup>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(2U, 1U).s32();
        CONSTANT(7U, 10U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(5U, Opcode::Compare).b().CC(CC_LT).Inputs(2U, 7U);
            INST(6U, Opcode::Return).b().Inputs(5U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, CompareTestCmpMultipleUsers)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        PARAMETER(1U, 0U).s32();
        PARAMETER(2U, 1U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Cmp).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(3U, 0U);
            INST(5U, Opcode::Add).i32().Inputs(3U, 4U);
            INST(6U, Opcode::Return).b().Inputs(5U);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        PARAMETER(1U, 0U).s32();
        PARAMETER(2U, 1U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Cmp).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(1U, 2U);
            INST(5U, Opcode::Add).i32().Inputs(3U, 4U);
            INST(6U, Opcode::Return).b().Inputs(5U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// cast case 1
TEST_F(PeepholesTest, CastTest1)
{
    for (int i = 1; i < DataType::ANY; ++i) {
        for (int j = 1; j < DataType::ANY; ++j) {
            if ((i == DataType::FLOAT32 || i == DataType::FLOAT64) && j >= DataType::BOOL && j <= DataType::INT16) {
                continue;
            }
            CheckCast(static_cast<DataType::Type>(i), static_cast<DataType::Type>(j), i == j);
        }
    }
}

void PeepholesTest::CastTest2Addition1MainLoop(int i, int j)
{
    for (int k = 1; k < DataType::LAST - 5L; ++k) {
        if ((i == DataType::FLOAT32 || i == DataType::FLOAT64) && j >= DataType::BOOL && j <= DataType::INT16) {
            continue;
        }
        CheckCast(i, j, k);
    }
}

// cast case 2
TEST_F(PeepholesTest, CastTest2Addition1)
{
    for (int i = 1; i < DataType::LAST - 5L; ++i) {
        for (int j = 1; j < DataType::LAST - 5L; ++j) {
            CastTest2Addition1MainLoop(i, j);
        }
    }
}
// cast case 2
TEST_F(PeepholesTest, CastTest2Addition2)
{
    for (int i = DataType::LAST - 5L; i < DataType::ANY; ++i) {
        for (int j = DataType::LAST - 5L; j < DataType::ANY; ++j) {
            for (int k = DataType::LAST - 5L; k < DataType::ANY; ++k) {
                CheckCast(i, j, k);
            }
        }
    }
}

// cast case 1, several casts
TEST_F(PeepholesTest, CastTest3)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Cast).s32().SrcType(DataType::INT32).Inputs(0U);
            INST(2U, Opcode::Cast).s32().SrcType(DataType::INT32).Inputs(1U);
            INST(3U, Opcode::Cast).s32().SrcType(DataType::INT32).Inputs(2U);
            INST(4U, Opcode::Cast).s32().SrcType(DataType::INT32).Inputs(3U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Cast).s32().SrcType(DataType::INT32).Inputs(0U);
            INST(2U, Opcode::Cast).s32().SrcType(DataType::INT32).Inputs(0U);
            INST(3U, Opcode::Cast).s32().SrcType(DataType::INT32).Inputs(0U);
            INST(4U, Opcode::Cast).s32().SrcType(DataType::INT32).Inputs(0U);
            INST(5U, Opcode::Return).s32().Inputs(0U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Casts from float32/64 to int8/16 don't support.
// cast case 2, several casts
TEST_F(PeepholesTest, DISABLED_CastTest4)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s16();
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Cast).f32().SrcType(DataType::INT16).Inputs(0U);
            INST(2U, Opcode::Cast).s16().SrcType(DataType::FLOAT32).Inputs(1U);
            INST(3U, Opcode::Cast).f64().SrcType(DataType::INT16).Inputs(2U);
            INST(4U, Opcode::Cast).s16().SrcType(DataType::FLOAT64).Inputs(3U);
            INST(5U, Opcode::Return).s16().Inputs(4U);
        }
    }
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s16();
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::Cast).f32().SrcType(DataType::INT16).Inputs(0U);
            INST(2U, Opcode::Cast).s16().SrcType(DataType::FLOAT32).Inputs(1U);
            INST(3U, Opcode::Cast).f64().SrcType(DataType::INT16).Inputs(0U);
            INST(4U, Opcode::Cast).s16().SrcType(DataType::FLOAT64).Inputs(3U);
            INST(5U, Opcode::Return).s16().Inputs(0U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// cast case 3
TEST_F(PeepholesTest, CastTest5)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x18U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Shr).s32().Inputs(2U, 1U);
            INST(4U, Opcode::Shl).s32().Inputs(3U, 1U);
            INST(5U, Opcode::AShr).s32().Inputs(4U, 1U);
            INST(6U, Opcode::Return).s32().Inputs(5U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x18U);
        CONSTANT(7U, 0xffU);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Shl).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Shr).s32().Inputs(2U, 1U);
            INST(8U, Opcode::And).s32().Inputs(0U, 7U);

            INST(4U, Opcode::Shl).s32().Inputs(8U, 1U);
            INST(5U, Opcode::AShr).s32().Inputs(4U, 1U);
            INST(9U, Opcode::Cast).s8().SrcType(DataType::INT32).Inputs(0U);

            INST(6U, Opcode::Return).s32().Inputs(9U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TrySwapInputsTest)
{
    for (Opcode opc : {Opcode::Add, Opcode::And, Opcode::Or, Opcode::Xor, Opcode::Min, Opcode::Max, Opcode::Mul}) {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            PARAMETER(0U, 0U).s32();
            CONSTANT(1U, 23U);
            BASIC_BLOCK(2U, 1U)
            {
                INST(2U, opc).s32().Inputs(1U, 0U);
                INST(3U, Opcode::Return).s32().Inputs(2U);
            }
        }
        ASSERT_TRUE(graph1->RunPass<Peepholes>());
        auto graph2 = CreateEmptyGraph();
        GRAPH(graph2)
        {
            PARAMETER(0U, 0U).s32();
            CONSTANT(1U, 23U);
            BASIC_BLOCK(2U, 1U)
            {
                INST(2U, opc).s32().Inputs(0U, 1U);
                INST(3U, Opcode::Return).s32().Inputs(2U);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(PeepholesTest, ReplaceXorWithNot)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, -1L);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Xor).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Xor).s32().Inputs(1U, 0U);
            INST(4U, Opcode::Add).s32().Inputs(2U, 3U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Not).s32().Inputs(0U);
            INST(2U, Opcode::Not).s32().Inputs(0U);
            INST(3U, Opcode::Add).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, XorWithZero)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Xor).s32().Inputs(0U, 1U);
            INST(3U, Opcode::Xor).s32().Inputs(1U, 0U);
            INST(4U, Opcode::Add).s32().Inputs(2U, 3U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).s32().Inputs(0U, 0U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(CleanupTrigger, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);
        CONSTANT(3U, 3U);
        CONSTANT(4U, 4U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Add).u64().Inputs(1U, 4U);
            INST(6U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(7U, Opcode::Compare).b().Inputs(0U, 3U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(9U, Opcode::Phi).u64().Inputs({{2U, 6U}, {3U, 5U}});
            INST(10U, Opcode::Return).u64().Inputs(9U);
        }
    }
}

OUT_GRAPH(CleanupTrigger, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);
        CONSTANT(3U, 3U);
        CONSTANT(4U, 4U);
        CONSTANT(11U, 5U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Add).u64().Inputs(1U, 4U);
            INST(6U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(7U, Opcode::Compare).b().Inputs(0U, 3U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(9U, Opcode::Phi).u64().Inputs({{2U, 11U}, {3U, 11U}});
            INST(10U, Opcode::Return).u64().Inputs(9U);
        }
    }
}

TEST_F(PeepholesTest, CleanupTrigger)
{
    src_graph::CleanupTrigger::CREATE(GetGraph());

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());

    auto graph = CreateEmptyGraph();
    out_graph::CleanupTrigger::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, TestAbsUnsigned)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Abs).u64().Inputs(0U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(3U).GetInput(0U).GetInst(), &INS(0U));
}

TEST_F(PeepholesTest, SafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).s32();
        PARAMETER(2U, 3U).f64();
        PARAMETER(3U, 4U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SafePoint).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(6U, Opcode::CallStatic).u64().InputsAutoType(0U, 1U, 2U, 3U, 5U);
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }
    Graph *graphEt = CreateEmptyGraph();
    GRAPH(graphEt)
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).s32();
        PARAMETER(2U, 3U).f64();
        PARAMETER(3U, 4U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SafePoint).Inputs(3U).SrcVregs({3U});
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(6U, Opcode::CallStatic).u64().InputsAutoType(0U, 1U, 2U, 3U, 5U);
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

TEST_F(PeepholesTest, SafePointWithRegMap)
{
    g_options.SetCompilerSafePointsRequireRegMap(true);

    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).u64();
        PARAMETER(1U, 2U).s32();
        PARAMETER(2U, 3U).f64();
        PARAMETER(3U, 4U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SafePoint).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(6U, Opcode::CallStatic).u64().InputsAutoType(0U, 1U, 2U, 3U, 5U);
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }
    Graph *graphEt = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

SRC_GRAPH(ShlShlAddAdd, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i64();
        CONSTANT(2U, 16U);
        CONSTANT(3U, 17U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Shl).i64().Inputs(0U, 2U);
            INST(5U, Opcode::Shl).i64().Inputs(0U, 3U);
            INST(6U, Opcode::Add).i64().Inputs(4U, 5U);
            INST(7U, Opcode::Add).i64().Inputs(1U, 6U);
            INST(8U, Opcode::Return).i64().Inputs(7U);
        }
    }
}

OUT_GRAPH(ShlShlAddAdd, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i64();
        CONSTANT(2U, 16U);
        CONSTANT(3U, 17U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Shl).i64().Inputs(0U, 2U);
            INST(5U, Opcode::Shl).i64().Inputs(0U, 3U);
            INST(6U, Opcode::Add).i64().Inputs(1U, 4U);
            INST(7U, Opcode::Add).i64().Inputs(6U, 5U);
            INST(8U, Opcode::Return).i64().Inputs(7U);
        }
    }
}

SRC_GRAPH(ShlShlAddAdd1, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i64();
        CONSTANT(2U, 16U);
        CONSTANT(3U, 17U);
        BASIC_BLOCK(2U, -1L)
        {
            // shl4 !HasSingleUser()
            INST(4U, Opcode::Shl).i64().Inputs(0U, 2U);
            INST(5U, Opcode::Shl).i64().Inputs(0U, 3U);
            INST(6U, Opcode::Add).i64().Inputs(4U, 5U);
            INST(7U, Opcode::Add).i64().Inputs(1U, 6U);
            INST(8U, Opcode::Add).i64().Inputs(4U, 7U);
            INST(9U, Opcode::Return).i64().Inputs(8U);
        }
    }
}

SRC_GRAPH(ShlShlAddAdd2, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i64();
        CONSTANT(2U, 16U);
        CONSTANT(3U, 17U);
        BASIC_BLOCK(2U, -1L)
        {
            // shl5 !HasSingleUser()
            INST(4U, Opcode::Shl).i64().Inputs(0U, 2U);
            INST(5U, Opcode::Shl).i64().Inputs(0U, 3U);
            INST(6U, Opcode::Add).i64().Inputs(4U, 5U);
            INST(7U, Opcode::Add).i64().Inputs(1U, 6U);
            INST(8U, Opcode::Add).i64().Inputs(5U, 7U);
            INST(9U, Opcode::Return).i64().Inputs(8U);
        }
    }
}

SRC_GRAPH(ShlShlAddAdd3, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i64();
        CONSTANT(2U, 16U);
        CONSTANT(3U, 17U);
        BASIC_BLOCK(2U, -1L)
        {
            // add6 !HasSingleUser()
            INST(4U, Opcode::Shl).i64().Inputs(0U, 2U);
            INST(5U, Opcode::Shl).i64().Inputs(0U, 3U);
            INST(6U, Opcode::Add).i64().Inputs(4U, 5U);
            INST(7U, Opcode::Add).i64().Inputs(1U, 6U);
            INST(8U, Opcode::Add).i64().Inputs(0U, 6U);
            INST(9U, Opcode::Return).i64().Inputs(8U);
        }
    }
}

SRC_GRAPH(ShlShlAddAdd4, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i64();
        CONSTANT(2U, 16U);
        CONSTANT(3U, 17U);
        BASIC_BLOCK(2U, -1L)
        {
            // shl4 and shl5 have different input(0).
            INST(4U, Opcode::Shl).i64().Inputs(0U, 2U);
            INST(5U, Opcode::Shl).i64().Inputs(1U, 3U);
            INST(6U, Opcode::Add).i64().Inputs(4U, 5U);
            INST(7U, Opcode::Add).i64().Inputs(1U, 6U);
            INST(8U, Opcode::Return).i64().Inputs(7U);
        }
    }
}

SRC_GRAPH(ShlShlAddAdd5, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i64();
        PARAMETER(2U, 2U).i64();
        CONSTANT(3U, 17U);
        BASIC_BLOCK(2U, -1L)
        {
            // shl4->GetInput(1) is not constant.
            INST(4U, Opcode::Shl).i64().Inputs(0U, 2U);
            INST(5U, Opcode::Shl).i64().Inputs(1U, 3U);
            INST(6U, Opcode::Add).i64().Inputs(4U, 5U);
            INST(7U, Opcode::Add).i64().Inputs(1U, 6U);
            INST(8U, Opcode::Return).i64().Inputs(7U);
        }
    }
}

SRC_GRAPH(ShlShlAddAdd6, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i64();
        CONSTANT(2U, 16U);
        PARAMETER(3U, 2U).i64();
        BASIC_BLOCK(2U, -1L)
        {
            // shl5->GetInput(1) is not constant.
            INST(4U, Opcode::Shl).i64().Inputs(0U, 2U);
            INST(5U, Opcode::Shl).i64().Inputs(1U, 3U);
            INST(6U, Opcode::Add).i64().Inputs(4U, 5U);
            INST(7U, Opcode::Add).i64().Inputs(1U, 6U);
            INST(8U, Opcode::Return).i64().Inputs(7U);
        }
    }
}

SRC_GRAPH(ShlShlAddAdd7, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i64();
        CONSTANT(2U, 16U);
        CONSTANT(3U, 17U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(20U, Opcode::Shl).i64().Inputs(0U, 2U);
            INST(21U, Opcode::Shl).i64().Inputs(0U, 3U);
            INST(22U, Opcode::Add).i64().Inputs(20U, 21U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(30U, Opcode::Add).i64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            // add40: input(0) does not dominate input(1)
            INST(40U, Opcode::Add).i64().Inputs(30U, 22U);
            INST(41U, Opcode::Return).i64().Inputs(40U);
        }
    }
}

TEST_F(PeepholesTest, ShlShlAddAdd)
{
    src_graph::ShlShlAddAdd::CREATE(GetGraph());
    Graph *graphPeepholed = CreateEmptyGraph();
    out_graph::ShlShlAddAdd::CREATE(graphPeepholed);

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphPeepholed));

    // Not optimizable cases
    Graph *graphNotOptimizable = CreateEmptyGraph();
    src_graph::ShlShlAddAdd1::CREATE(graphNotOptimizable);
    ASSERT_FALSE(graphNotOptimizable->RunPass<Peepholes>());

    graphNotOptimizable = CreateEmptyGraph();
    src_graph::ShlShlAddAdd2::CREATE(graphNotOptimizable);
    ASSERT_FALSE(graphNotOptimizable->RunPass<Peepholes>());

    graphNotOptimizable = CreateEmptyGraph();
    src_graph::ShlShlAddAdd3::CREATE(graphNotOptimizable);
    ASSERT_FALSE(graphNotOptimizable->RunPass<Peepholes>());

    graphNotOptimizable = CreateEmptyGraph();
    src_graph::ShlShlAddAdd4::CREATE(graphNotOptimizable);
    ASSERT_FALSE(graphNotOptimizable->RunPass<Peepholes>());

    graphNotOptimizable = CreateEmptyGraph();
    src_graph::ShlShlAddAdd5::CREATE(graphNotOptimizable);
    ASSERT_FALSE(graphNotOptimizable->RunPass<Peepholes>());

    graphNotOptimizable = CreateEmptyGraph();
    src_graph::ShlShlAddAdd6::CREATE(graphNotOptimizable);
    ASSERT_FALSE(graphNotOptimizable->RunPass<Peepholes>());

    graphNotOptimizable = CreateEmptyGraph();
    src_graph::ShlShlAddAdd7::CREATE(graphNotOptimizable);
    ASSERT_FALSE(graphNotOptimizable->RunPass<Peepholes>());
}

TEST_F(PeepholesTest, ShlShlAddSub)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i64();
        CONSTANT(2U, 16U);
        CONSTANT(3U, 17U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Shl).i64().Inputs(0U, 2U);
            INST(5U, Opcode::Shl).i64().Inputs(0U, 3U);
            INST(6U, Opcode::Add).i64().Inputs(4U, 5U);
            INST(7U, Opcode::Sub).i64().Inputs(1U, 6U);
            INST(8U, Opcode::Return).i64().Inputs(7U);
        }
    }
    Graph *graphPeepholed = CreateEmptyGraph();
    GRAPH(graphPeepholed)
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i64();
        CONSTANT(2U, 16U);
        CONSTANT(3U, 17U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Shl).i64().Inputs(0U, 2U);
            INST(5U, Opcode::Shl).i64().Inputs(0U, 3U);
            INST(6U, Opcode::Sub).i64().Inputs(1U, 4U);
            INST(7U, Opcode::Sub).i64().Inputs(6U, 5U);
            INST(8U, Opcode::Return).i64().Inputs(7U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphPeepholed));
}

TEST_F(PeepholesTest, ShlShlSubAdd)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i64();
        CONSTANT(2U, 16U);
        CONSTANT(3U, 17U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Shl).i64().Inputs(0U, 2U);
            INST(5U, Opcode::Shl).i64().Inputs(0U, 3U);
            INST(6U, Opcode::Sub).i64().Inputs(4U, 5U);
            INST(7U, Opcode::Add).i64().Inputs(1U, 6U);
            INST(8U, Opcode::Return).i64().Inputs(7U);
        }
    }
    Graph *graphPeepholed = CreateEmptyGraph();
    GRAPH(graphPeepholed)
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i64();
        CONSTANT(2U, 16U);
        CONSTANT(3U, 17U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Shl).i64().Inputs(0U, 2U);
            INST(5U, Opcode::Shl).i64().Inputs(0U, 3U);
            INST(6U, Opcode::Add).i64().Inputs(1U, 4U);
            INST(7U, Opcode::Sub).i64().Inputs(6U, 5U);
            INST(8U, Opcode::Return).i64().Inputs(7U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphPeepholed));
}

TEST_F(PeepholesTest, ShlShlSubSub)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i64();
        CONSTANT(2U, 16U);
        CONSTANT(3U, 17U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Shl).i64().Inputs(0U, 2U);
            INST(5U, Opcode::Shl).i64().Inputs(0U, 3U);
            INST(6U, Opcode::Sub).i64().Inputs(4U, 5U);
            INST(7U, Opcode::Sub).i64().Inputs(1U, 6U);
            INST(8U, Opcode::Return).i64().Inputs(7U);
        }
    }
    Graph *graphPeepholed = CreateEmptyGraph();
    GRAPH(graphPeepholed)
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i64();
        CONSTANT(2U, 16U);
        CONSTANT(3U, 17U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Shl).i64().Inputs(0U, 2U);
            INST(5U, Opcode::Shl).i64().Inputs(0U, 3U);
            INST(6U, Opcode::Sub).i64().Inputs(1U, 4U);
            INST(7U, Opcode::Add).i64().Inputs(6U, 5U);
            INST(8U, Opcode::Return).i64().Inputs(7U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphPeepholed));
}

TEST_F(PeepholesTest, CompareAndWithZero)
{
    CheckCompareFoldIntoTest(0U, CC_EQ, true, CC_TST_EQ);
    CheckCompareFoldIntoTest(0U, CC_NE, true, CC_TST_NE);

    CheckCompareFoldIntoTest(1U, CC_EQ, false);
    CheckCompareFoldIntoTest(2U, CC_NE, false);

    // check comparision with zero for all CCs except CC_EQ and CC_NE
    for (int cc = CC_LT; cc <= CC_LAST; ++cc) {
        CheckCompareFoldIntoTest(0U, static_cast<ConditionCode>(cc), false);
    }
}

TEST_F(PeepholesTest, IfAndComparedWithZero)
{
    CheckIfAndZeroFoldIntoIfTest(0U, CC_EQ, true, CC_TST_EQ);
    CheckIfAndZeroFoldIntoIfTest(0U, CC_NE, true, CC_TST_NE);

    CheckIfAndZeroFoldIntoIfTest(1U, CC_EQ, false);
    CheckIfAndZeroFoldIntoIfTest(2U, CC_NE, false);

    // check comparision with zero for all CCs except CC_EQ and CC_NE
    for (int cc = CC_LT; cc <= CC_LAST; ++cc) {
        CheckCompareFoldIntoTest(0U, static_cast<ConditionCode>(cc), false);
    }
}

TEST_F(PeepholesTest, CompareLenArrayWithZero)
{
    CheckCompareLenArrayWithZeroTest(0U, CC_GE, true);
    CheckCompareLenArrayWithZeroTest(0U, CC_LT, false);
    CheckCompareLenArrayWithZeroTest(0U, CC_LE, std::nullopt);
    CheckCompareLenArrayWithZeroTest(1U, CC_GE, std::nullopt);
    CheckCompareLenArrayWithZeroTest(1U, CC_LT, std::nullopt);
}

TEST_F(PeepholesTest, CompareLenArrayWithZeroSwapped)
{
    CheckCompareLenArrayWithZeroTest(0U, CC_LE, true, true);
    CheckCompareLenArrayWithZeroTest(0U, CC_GT, false, true);
    CheckCompareLenArrayWithZeroTest(0U, CC_GE, std::nullopt, true);
    CheckCompareLenArrayWithZeroTest(1U, CC_LE, std::nullopt, true);
    CheckCompareLenArrayWithZeroTest(1U, CC_GT, std::nullopt, true);
}

TEST_F(PeepholesTest, TestEqualInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Compare).b().CC(CC_TST_EQ).Inputs(0U, 0U);
            INST(2U, Opcode::Compare).b().CC(CC_TST_NE).Inputs(0U, 0U);
            INST(3U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::CallStatic).b().InputsAutoType(1U, 2U, 3U);
            INST(5U, Opcode::ReturnVoid);
        }
    }
    Graph *graphPeepholed = CreateEmptyGraph();
    GRAPH(graphPeepholed)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Compare).b().CC(CC_EQ).Inputs(0U, 1U);
            INST(3U, Opcode::Compare).b().CC(CC_NE).Inputs(0U, 1U);
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallStatic).b().InputsAutoType(2U, 3U, 4U);
            INST(6U, Opcode::ReturnVoid);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphPeepholed));
}

SRC_GRAPH(AndWithCast, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0xFFFFFFFFU);
        CONSTANT(3U, 0xFFFFU);
        CONSTANT(4U, 0xFFU);
        CONSTANT(5U, 0xF7U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(10U, Opcode::And).i32().Inputs(1U, 2U);
            INST(11U, Opcode::And).i32().Inputs(1U, 3U);
            INST(12U, Opcode::And).i32().Inputs(1U, 4U);
            INST(13U, Opcode::And).i32().Inputs(1U, 5U);

            INST(22U, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(10U);
            INST(23U, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(11U);
            INST(24U, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(12U);
            INST(25U, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(13U);

            INST(26U, Opcode::Cast).u16().SrcType(DataType::INT32).Inputs(10U);
            INST(27U, Opcode::Cast).u16().SrcType(DataType::INT32).Inputs(11U);
            INST(28U, Opcode::Cast).u16().SrcType(DataType::INT32).Inputs(12U);
            INST(29U, Opcode::Cast).u16().SrcType(DataType::INT32).Inputs(13U);

            INST(30U, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(10U);
            INST(31U, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(11U);
            INST(32U, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(12U);
            INST(33U, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(13U);

            INST(34U, Opcode::Cast).u8().SrcType(DataType::INT32).Inputs(10U);
            INST(35U, Opcode::Cast).u8().SrcType(DataType::INT32).Inputs(11U);
            INST(36U, Opcode::Cast).u8().SrcType(DataType::INT32).Inputs(12U);
            INST(37U, Opcode::Cast).u8().SrcType(DataType::INT32).Inputs(13U);

            INST(38U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(AndWithCast, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i64();
        PARAMETER(1U, 1U).i32();
        CONSTANT(2U, 0xFFFFFFFFU);
        CONSTANT(3U, 0xFFFFU);
        CONSTANT(4U, 0xFFU);
        CONSTANT(5U, 0xF7U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(10U, Opcode::And).i32().Inputs(1U, 2U);
            INST(11U, Opcode::And).i32().Inputs(1U, 3U);
            INST(12U, Opcode::And).i32().Inputs(1U, 4U);
            INST(13U, Opcode::And).i32().Inputs(1U, 5U);

            INST(22U, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(1U);
            INST(23U, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(1U);
            INST(24U, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(12U);
            INST(25U, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(13U);

            INST(26U, Opcode::Cast).u16().SrcType(DataType::INT32).Inputs(1U);
            INST(27U, Opcode::Cast).u16().SrcType(DataType::INT32).Inputs(1U);
            INST(28U, Opcode::Cast).u16().SrcType(DataType::INT32).Inputs(12U);
            INST(29U, Opcode::Cast).u16().SrcType(DataType::INT32).Inputs(13U);

            INST(30U, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(1U);
            INST(31U, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(1U);
            INST(32U, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(1U);
            INST(33U, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(13U);

            INST(34U, Opcode::Cast).u8().SrcType(DataType::INT32).Inputs(1U);
            INST(35U, Opcode::Cast).u8().SrcType(DataType::INT32).Inputs(1U);
            INST(36U, Opcode::Cast).u8().SrcType(DataType::INT32).Inputs(1U);
            INST(37U, Opcode::Cast).u8().SrcType(DataType::INT32).Inputs(13U);

            INST(38U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(PeepholesTest, AndWithCast)
{
    auto graph = CreateEmptyGraph();
    src_graph::AndWithCast::CREATE(graph);

    auto graphExpected = CreateEmptyGraph();
    out_graph::AndWithCast::CREATE(graphExpected);
    ASSERT_TRUE(graph->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
}

SRC_GRAPH(AndWithStore, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ptr();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 0xFFFFFFFFU);
        CONSTANT(3U, 0xFFFFU);
        CONSTANT(4U, 0xFFU);
        CONSTANT(5U, 0xF7U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::And).i32().Inputs(1U, 2U);
            INST(7U, Opcode::And).i32().Inputs(1U, 3U);
            INST(8U, Opcode::And).i32().Inputs(1U, 4U);
            INST(9U, Opcode::And).i32().Inputs(1U, 5U);

            INST(10U, Opcode::Store).i32().Inputs(0U, 1U, 6U);
            INST(11U, Opcode::Store).i32().Inputs(0U, 1U, 7U);
            INST(12U, Opcode::Store).i32().Inputs(0U, 1U, 8U);
            INST(13U, Opcode::Store).i32().Inputs(0U, 1U, 9U);

            INST(14U, Opcode::Store).u32().Inputs(0U, 1U, 6U);
            INST(15U, Opcode::Store).u32().Inputs(0U, 1U, 7U);
            INST(16U, Opcode::Store).u32().Inputs(0U, 1U, 8U);
            INST(17U, Opcode::Store).u32().Inputs(0U, 1U, 9U);

            INST(18U, Opcode::Store).i16().Inputs(0U, 1U, 6U);
            INST(19U, Opcode::Store).i16().Inputs(0U, 1U, 7U);
            INST(20U, Opcode::Store).i16().Inputs(0U, 1U, 8U);
            INST(21U, Opcode::Store).i16().Inputs(0U, 1U, 9U);

            INST(22U, Opcode::Store).u16().Inputs(0U, 1U, 6U);
            INST(23U, Opcode::Store).u16().Inputs(0U, 1U, 7U);
            INST(24U, Opcode::Store).u16().Inputs(0U, 1U, 8U);
            INST(25U, Opcode::Store).u16().Inputs(0U, 1U, 9U);

            INST(26U, Opcode::Store).i8().Inputs(0U, 1U, 6U);
            INST(27U, Opcode::Store).i8().Inputs(0U, 1U, 7U);
            INST(28U, Opcode::Store).i8().Inputs(0U, 1U, 8U);
            INST(29U, Opcode::Store).i8().Inputs(0U, 1U, 9U);

            INST(30U, Opcode::Store).u8().Inputs(0U, 1U, 6U);
            INST(31U, Opcode::Store).u8().Inputs(0U, 1U, 7U);
            INST(32U, Opcode::Store).u8().Inputs(0U, 1U, 8U);
            INST(33U, Opcode::Store).u8().Inputs(0U, 1U, 9U);

            INST(34U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(AndWithStore, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ptr();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 0xFFFFFFFFU);
        CONSTANT(3U, 0xFFFFU);
        CONSTANT(4U, 0xFFU);
        CONSTANT(5U, 0xF7U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::And).i32().Inputs(1U, 2U);
            INST(7U, Opcode::And).i32().Inputs(1U, 3U);
            INST(8U, Opcode::And).i32().Inputs(1U, 4U);
            INST(9U, Opcode::And).i32().Inputs(1U, 5U);

            INST(10U, Opcode::Store).i32().Inputs(0U, 1U, 1U);
            INST(11U, Opcode::Store).i32().Inputs(0U, 1U, 7U);
            INST(12U, Opcode::Store).i32().Inputs(0U, 1U, 8U);
            INST(13U, Opcode::Store).i32().Inputs(0U, 1U, 9U);

            INST(14U, Opcode::Store).u32().Inputs(0U, 1U, 1U);
            INST(15U, Opcode::Store).u32().Inputs(0U, 1U, 7U);
            INST(16U, Opcode::Store).u32().Inputs(0U, 1U, 8U);
            INST(17U, Opcode::Store).u32().Inputs(0U, 1U, 9U);

            INST(18U, Opcode::Store).i16().Inputs(0U, 1U, 1U);
            INST(19U, Opcode::Store).i16().Inputs(0U, 1U, 1U);
            INST(20U, Opcode::Store).i16().Inputs(0U, 1U, 8U);
            INST(21U, Opcode::Store).i16().Inputs(0U, 1U, 9U);

            INST(22U, Opcode::Store).u16().Inputs(0U, 1U, 1U);
            INST(23U, Opcode::Store).u16().Inputs(0U, 1U, 1U);
            INST(24U, Opcode::Store).u16().Inputs(0U, 1U, 8U);
            INST(25U, Opcode::Store).u16().Inputs(0U, 1U, 9U);

            INST(26U, Opcode::Store).i8().Inputs(0U, 1U, 1U);
            INST(27U, Opcode::Store).i8().Inputs(0U, 1U, 1U);
            INST(28U, Opcode::Store).i8().Inputs(0U, 1U, 1U);
            INST(29U, Opcode::Store).i8().Inputs(0U, 1U, 9U);

            INST(30U, Opcode::Store).u8().Inputs(0U, 1U, 1U);
            INST(31U, Opcode::Store).u8().Inputs(0U, 1U, 1U);
            INST(32U, Opcode::Store).u8().Inputs(0U, 1U, 1U);
            INST(33U, Opcode::Store).u8().Inputs(0U, 1U, 9U);

            INST(34U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(PeepholesTest, AndWithStore)
{
    auto graph = CreateEmptyGraph();
    src_graph::AndWithStore::CREATE(graph);
    auto graphExpected = CreateEmptyGraph();
    out_graph::AndWithStore::CREATE(graphExpected);
    ASSERT_TRUE(graph->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
}

SRC_GRAPH(CastWithStore, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ptr();
        PARAMETER(1U, 1U).i64();
        PARAMETER(2U, 2U).i32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Cast).i32().SrcType(DataType::INT64).Inputs(1U);
            INST(7U, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(2U);
            INST(8U, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(2U);

            INST(9U, Opcode::Store).i32().Inputs(0U, 1U, 6U);
            INST(10U, Opcode::Store).i32().Inputs(0U, 1U, 7U);
            INST(11U, Opcode::Store).i32().Inputs(0U, 1U, 8U);

            INST(12U, Opcode::Store).u32().Inputs(0U, 1U, 6U);
            INST(13U, Opcode::Store).u32().Inputs(0U, 1U, 7U);
            INST(14U, Opcode::Store).u32().Inputs(0U, 1U, 8U);

            INST(15U, Opcode::Store).i16().Inputs(0U, 1U, 6U);
            INST(16U, Opcode::Store).i16().Inputs(0U, 1U, 7U);
            INST(17U, Opcode::Store).i16().Inputs(0U, 1U, 8U);

            INST(18U, Opcode::Store).u16().Inputs(0U, 1U, 6U);
            INST(19U, Opcode::Store).u16().Inputs(0U, 1U, 7U);
            INST(20U, Opcode::Store).u16().Inputs(0U, 1U, 8U);

            INST(21U, Opcode::Store).i8().Inputs(0U, 1U, 6U);
            INST(22U, Opcode::Store).i8().Inputs(0U, 1U, 7U);
            INST(23U, Opcode::Store).i8().Inputs(0U, 1U, 8U);

            INST(24U, Opcode::Store).u8().Inputs(0U, 1U, 6U);
            INST(25U, Opcode::Store).u8().Inputs(0U, 1U, 7U);
            INST(26U, Opcode::Store).u8().Inputs(0U, 1U, 8U);

            INST(27U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(CastWithStore, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ptr();
        PARAMETER(1U, 1U).i64();
        PARAMETER(2U, 2U).i32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Cast).i32().SrcType(DataType::INT64).Inputs(1U);
            INST(7U, Opcode::Cast).i16().SrcType(DataType::INT32).Inputs(2U);
            INST(8U, Opcode::Cast).i8().SrcType(DataType::INT32).Inputs(2U);

            INST(9U, Opcode::Store).i32().Inputs(0U, 1U, 1U);
            INST(10U, Opcode::Store).i32().Inputs(0U, 1U, 7U);
            INST(11U, Opcode::Store).i32().Inputs(0U, 1U, 8U);

            INST(12U, Opcode::Store).u32().Inputs(0U, 1U, 1U);
            INST(13U, Opcode::Store).u32().Inputs(0U, 1U, 7U);
            INST(14U, Opcode::Store).u32().Inputs(0U, 1U, 8U);

            INST(15U, Opcode::Store).i16().Inputs(0U, 1U, 1U);
            INST(16U, Opcode::Store).i16().Inputs(0U, 1U, 2U);
            INST(17U, Opcode::Store).i16().Inputs(0U, 1U, 8U);

            INST(18U, Opcode::Store).u16().Inputs(0U, 1U, 1U);
            INST(19U, Opcode::Store).u16().Inputs(0U, 1U, 2U);
            INST(20U, Opcode::Store).u16().Inputs(0U, 1U, 8U);

            INST(21U, Opcode::Store).i8().Inputs(0U, 1U, 1U);
            INST(22U, Opcode::Store).i8().Inputs(0U, 1U, 2U);
            INST(23U, Opcode::Store).i8().Inputs(0U, 1U, 2U);

            INST(24U, Opcode::Store).u8().Inputs(0U, 1U, 1U);
            INST(25U, Opcode::Store).u8().Inputs(0U, 1U, 2U);
            INST(26U, Opcode::Store).u8().Inputs(0U, 1U, 2U);

            INST(27U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(PeepholesTest, CastWithStore)
{
    auto graph = CreateEmptyGraph();
    src_graph::CastWithStore::CREATE(graph);
    auto graphExpected = CreateEmptyGraph();
    out_graph::CastWithStore::CREATE(graphExpected);
    ASSERT_TRUE(graph->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
}

TEST_F(PeepholesTest, CastWithStoreSignMismatch)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ptr();
        PARAMETER(1U, 1U).i64();
        PARAMETER(2U, 2U).u8();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Cast).i32().SrcType(DataType::INT8).Inputs(2U);
            INST(4U, Opcode::Store).i32().Inputs(0U, 1U, 3U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
#ifndef NDEBUG
    ASSERT_DEATH(graph->RunPass<Peepholes>(), "");
#else
    ASSERT_FALSE(graph->RunPass<Peepholes>());
#endif
}

TEST_F(PeepholesTest, CastWithStoreTypeMismatch)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ptr();
        PARAMETER(1U, 1U).i64();
        PARAMETER(2U, 2U).i16();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Cast).i32().SrcType(DataType::INT8).Inputs(2U);
            INST(4U, Opcode::Store).i32().Inputs(0U, 1U, 3U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
#ifndef NDEBUG
    ASSERT_DEATH(graph->RunPass<Peepholes>(), "");
#else
    ASSERT_FALSE(graph->RunPass<Peepholes>());
#endif
}

TEST_F(PeepholesTest, CastI64ToU16)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i64();
        CONSTANT(2U, 0xFFFFU);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Cast).i32().SrcType(DataType::INT64).Inputs(0U);
            INST(4U, Opcode::And).i32().Inputs(3U, 2U);
            INST(5U, Opcode::Return).i32().Inputs(4U);
        }
    }

    auto graphExpected = CreateEmptyGraph();
    GRAPH(graphExpected)
    {
        PARAMETER(0U, 0U).i64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Cast).u16().SrcType(DataType::INT64).Inputs(0U);
            INST(5U, Opcode::Return).i32().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<Peepholes>());
    graph->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
}

TEST_F(PeepholesTest, CastI8ToU16)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i8();
        CONSTANT(2U, 0xFFFFU);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Cast).i32().SrcType(DataType::INT8).Inputs(0U);
            INST(4U, Opcode::And).i32().Inputs(3U, 2U);
            INST(5U, Opcode::Return).i32().Inputs(4U);
        }
    }

    auto graphExpected = CreateEmptyGraph();
    GRAPH(graphExpected)
    {
        PARAMETER(0U, 0U).i8();

        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::Cast).u16().SrcType(DataType::INT8).Inputs(0U);
            INST(5U, Opcode::Return).i32().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph->RunPass<Peepholes>());
    graph->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
}

TEST_F(PeepholesTest, CastI64ToB)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).i32().SrcType(DataType::INT64).Inputs(0U);
            INST(2U, Opcode::Cast).b().SrcType(DataType::INT32).Inputs(1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }

    auto graphExpected = CreateEmptyGraph();
    GRAPH(graphExpected)
    {
        PARAMETER(0U, 0U).i64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Cast).b().SrcType(DataType::INT64).Inputs(0U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }
    ASSERT_TRUE(graph->RunPass<Peepholes>());
    graph->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph, graphExpected));
}

TEST_F(PeepholesTest, CastI64ToBSignMismatch)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).i32().SrcType(DataType::INT64).Inputs(0U);
            INST(2U, Opcode::Cast).b().SrcType(DataType::UINT32).Inputs(1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }

#ifndef NDEBUG
    ASSERT_DEATH(graph->RunPass<Peepholes>(), "");
#else
    ASSERT_FALSE(graph->RunPass<Peepholes>());
#endif
}

TEST_F(PeepholesTest, CastI64ToBTypeMismatch)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).i32().SrcType(DataType::INT64).Inputs(0U);
            INST(2U, Opcode::Cast).b().SrcType(DataType::INT16).Inputs(1U);
            INST(3U, Opcode::Return).b().Inputs(2U);
        }
    }

#ifndef NDEBUG
    ASSERT_DEATH(graph->RunPass<Peepholes>(), "");
#else
    ASSERT_FALSE(graph->RunPass<Peepholes>());
#endif
}

TEST_F(PeepholesTest, ConstCombineAdd)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).i64();
        CONSTANT(1U, 3U);
        CONSTANT(2U, 7U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Add).s64().Inputs(0U, 1U);
            INST(4U, Opcode::Add).s64().Inputs(3U, 2U);
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0U, 0U).i64();
        CONSTANT(6U, 10U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(4U, Opcode::Add).s64().Inputs(0U, 6U);
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }
    ASSERT_TRUE(graph1->RunPass<Peepholes>());
    graph1->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(PeepholesTest, ConstCombineAddSub)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).i64();
        CONSTANT(1U, 3U);
        CONSTANT(2U, 7U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Sub).s64().Inputs(0U, 1U);
            INST(4U, Opcode::Add).s64().Inputs(3U, 2U);
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0U, 0U).i64();
        CONSTANT(6U, 4U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(4U, Opcode::Add).s64().Inputs(0U, 6U);
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }
    ASSERT_TRUE(graph1->RunPass<Peepholes>());
    graph1->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(PeepholesTest, ConstCombineSubAdd)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).i64();
        CONSTANT(1U, 3U);
        CONSTANT(2U, 7U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Add).s64().Inputs(0U, 1U);
            INST(4U, Opcode::Sub).s64().Inputs(3U, 2U);
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0U, 0U).i64();
        CONSTANT(6U, 4U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(4U, Opcode::Sub).s64().Inputs(0U, 6U);
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }
    ASSERT_TRUE(graph1->RunPass<Peepholes>());
    graph1->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(PeepholesTest, ConstCombineShift)
{
    for (auto opcode : {Opcode::Shr, Opcode::Shl, Opcode::AShr}) {
        for (auto shift : {7U, 71U}) {
            auto graph1 = CreateEmptyGraph();
            GRAPH(graph1)
            {
                PARAMETER(0U, 0U).i64();
                CONSTANT(1U, 3U);
                CONSTANT(2U, shift);
                BASIC_BLOCK(2U, 1U)
                {
                    INST(3U, opcode).s64().Inputs(0U, 1U);
                    INST(4U, opcode).s64().Inputs(3U, 2U);
                    INST(5U, Opcode::Return).s64().Inputs(4U);
                }
            }
            auto graph2 = CreateEmptyGraph();
            GRAPH(graph2)
            {
                PARAMETER(0U, 0U).i64();
                CONSTANT(6U, 10U);
                BASIC_BLOCK(2U, 1U)
                {
                    INST(4U, opcode).s64().Inputs(0U, 6U);
                    INST(5U, Opcode::Return).s64().Inputs(4U);
                }
            }
            ASSERT_TRUE(graph1->RunPass<Peepholes>());
            graph1->RunPass<Cleanup>();
            ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
        }
    }
}

TEST_F(PeepholesTest, ConstCombineShiftAlwaysZero)
{
    for (auto opcode : {Opcode::Shr, Opcode::Shl}) {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            PARAMETER(0U, 0U).i64();
            CONSTANT(1U, 31U);
            CONSTANT(2U, 33U);
            BASIC_BLOCK(2U, 1U)
            {
                INST(3U, opcode).s64().Inputs(0U, 1U);
                INST(4U, opcode).s64().Inputs(3U, 2U);
                INST(5U, Opcode::Return).s64().Inputs(4U);
            }
        }
        auto graph2 = CreateEmptyGraph();
        GRAPH(graph2)
        {
            CONSTANT(6U, 0U);
            BASIC_BLOCK(2U, 1U)
            {
                INST(5U, Opcode::Return).s64().Inputs(6U);
            }
        }
        ASSERT_TRUE(graph1->RunPass<Peepholes>());
        graph1->RunPass<Cleanup>();
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

TEST_F(PeepholesTest, ConstCombineAShrMax)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).i64();
        CONSTANT(1U, 31U);
        CONSTANT(2U, 33U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::AShr).s64().Inputs(0U, 1U);
            INST(4U, Opcode::AShr).s64().Inputs(3U, 2U);
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0U, 0U).i64();
        CONSTANT(6U, 63U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(4U, Opcode::AShr).s64().Inputs(0U, 6U);
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }
    ASSERT_TRUE(graph1->RunPass<Peepholes>());
    graph1->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(PeepholesTest, ConstCombineMul)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).i64();
        CONSTANT(1U, 3U);
        CONSTANT(2U, 7U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Mul).s64().Inputs(0U, 1U);
            INST(4U, Opcode::Mul).s64().Inputs(3U, 2U);
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0U, 0U).i64();
        CONSTANT(6U, 21U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(4U, Opcode::Mul).s64().Inputs(0U, 6U);
            INST(5U, Opcode::Return).s64().Inputs(4U);
        }
    }
    ASSERT_TRUE(graph1->RunPass<Peepholes>());
    graph1->RunPass<Cleanup>();
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

SRC_GRAPH(ConditionalAssignment, Graph *graph, bool inverse)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        CONSTANT(2U, 0x0U).s64();
        CONSTANT(4U, 0x1U).s32();
        CONSTANT(5U, 0x0U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::Compare).b().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 2U);
            INST(3U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(inverse ? CC_NE : CC_EQ).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(4U, 3U) {}

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Phi).b().Inputs(5U, 4U);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
}

OUT_GRAPH(ConditionalAssignment, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();

        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::Return).b().Inputs(0U);
        }
    }
}

OUT_GRAPH(ConditionalAssignmentInverse, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();

        BASIC_BLOCK(2U, -1L)
        {
            INST(10U, Opcode::XorI).b().Inputs(0U).Imm(1U);
            INST(7U, Opcode::Return).b().Inputs(10U);
        }
    }
}

TEST_F(PeepholesTest, ConditionalAssignment)
{
    for (auto inverse : {true, false}) {
        auto graph = CreateEmptyBytecodeGraph();
        src_graph::ConditionalAssignment::CREATE(graph, inverse);

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<Peepholes>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<Lowering>());
        graph->RunPass<Cleanup>();

        auto expected = CreateEmptyBytecodeGraph();
        if (inverse) {
            out_graph::ConditionalAssignmentInverse::CREATE(expected);
        } else {
            out_graph::ConditionalAssignment::CREATE(expected);
        }
        EXPECT_TRUE(GraphComparator().Compare(graph, expected));
    }
}

SRC_GRAPH(ConditionalAssignmentPreserveIf, Graph *graph, bool inverse)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(4U, 1U);
        CONSTANT(5U, 0U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_GT).Inputs(0U, 4U);
            INST(3U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(inverse ? CC_NE : CC_EQ).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(4U, 3U)
        {
            INST(8U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::CallStatic).u64().InputsAutoType(8U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Phi).b().Inputs(5U, 4U);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
}

OUT_GRAPH(ConditionalAssignmentPreserveIf, Graph *expected, bool inverse)
{
    GRAPH(expected)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(4U, 1U);
        if (inverse) {
            CONSTANT(5U, 0U);
        }

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_GT).Inputs(0U, 4U);
            INST(3U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(inverse ? CC_NE : CC_EQ).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(4U, 3U)
        {
            INST(8U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::CallStatic).u64().InputsAutoType(8U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            if (inverse) {
                INST(11U, Opcode::Compare).b().CC(ConditionCode::CC_EQ).SetType(DataType::BOOL).Inputs(1U, 5U);
                INST(7U, Opcode::Return).b().Inputs(11U);
            } else {
                INST(7U, Opcode::Return).b().Inputs(1U);
            }
        }
    }
}

TEST_F(PeepholesTest, ConditionalAssignmentPreserveIf)
{
    for (auto inverse : {true, false}) {
        auto graph = CreateEmptyGraph();
        src_graph::ConditionalAssignmentPreserveIf::CREATE(graph, inverse);

        ASSERT_TRUE(graph->RunPass<Peepholes>());
        ASSERT_TRUE(graph->RunPass<Cleanup>());

        auto expected = CreateEmptyGraph();
        out_graph::ConditionalAssignmentPreserveIf::CREATE(expected, inverse);

        EXPECT_TRUE(GraphComparator().Compare(graph, expected));
    }
}

TEST_F(PeepholesTest, ConditionalAssignmentBytecodeCompare)
{
    // In case of bytecode optimizer we can not generate Compare instruction,
    // the optimization isn't applied
    auto graph = CreateEmptyBytecodeGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0x0U).s32();
        CONSTANT(2U, 0x1U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GT).Inputs(0U, 2U);
            INST(4U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(4U, 3U) {}

        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::Phi).b().Inputs(1U, 2U);
            INST(6U, Opcode::Return).b().Inputs(5U);
        }
    }

    ASSERT_FALSE(graph->RunPass<Peepholes>());
}

TEST_F(PeepholesTest, ConditionalAssignmentWrongConstants)
{
    // Constants are the same or not equal to 0 and 1
    for (auto constant : {0U, 2U}) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).s32();
            CONSTANT(1U, 0U).s32();
            CONSTANT(2U, constant).s32();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(3U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GT).Inputs(0U, 2U);
                INST(4U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
            }

            BASIC_BLOCK(4U, 3U) {}

            BASIC_BLOCK(3U, -1L)
            {
                INST(5U, Opcode::Phi).b().Inputs(1U, 2U);
                INST(6U, Opcode::Return).b().Inputs(5U);
            }
        }

        EXPECT_FALSE(graph->RunPass<Peepholes>());
    }
}

SRC_GRAPH(ConditionalAssignmentsTwoBranches, Graph *graph, bool inverse)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).b();
        CONSTANT(2U, 0x0U).s64();
        CONSTANT(4U, 0x1U).s32();
        CONSTANT(5U, 0x0U).s32();

        BASIC_BLOCK(2U, 4U, 5U)
        {
            INST(1U, Opcode::Compare).b().SrcType(DataType::BOOL).CC(CC_NE).Inputs(0U, 2U);
            INST(3U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(inverse ? CC_NE : CC_EQ).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(4U, 6U) {}
        BASIC_BLOCK(6U, 3U) {}

        BASIC_BLOCK(5U, 3U) {}

        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::Phi).b().Inputs(4U, 5U);
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
}

TEST_F(PeepholesTest, ConditionalAssignmentsTwoBranches)
{
    for (auto inverse : {true, false}) {
        auto graph = CreateEmptyBytecodeGraph();
        src_graph::ConditionalAssignmentsTwoBranches::CREATE(graph, inverse);

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<Peepholes>());
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<Lowering>());
        graph->RunPass<Cleanup>();

        auto expected = CreateEmptyBytecodeGraph();
        if (!inverse) {
            GRAPH(expected)
            {
                PARAMETER(0U, 0U).b();

                BASIC_BLOCK(2U, -1L)
                {
                    INST(10U, Opcode::XorI).b().Inputs(0U).Imm(1U);
                    INST(7U, Opcode::Return).b().Inputs(10U);
                }
            }
        } else {
            GRAPH(expected)
            {
                PARAMETER(0U, 0U).b();

                BASIC_BLOCK(2U, -1L)
                {
                    INST(7U, Opcode::Return).b().Inputs(0U);
                }
            }
        }
        EXPECT_TRUE(GraphComparator().Compare(graph, expected));
    }
}

/* We cannot replace Phi because its value may come from both branches of If
 *   [entry]
 *      |
 *      v
 *     [2]--->[4]
 *      |      |
 *      v      v
 *     [3]--->[5]
 *      |      |
 *      v      |
 *     [6]<----/
 *      |
 *      v
 *    [exit]
 */
TEST_F(PeepholesTest, ConditionalAssignmentMixedBranches)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(4U, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_NE).Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }

        BASIC_BLOCK(3U, 5U, 6U)
        {
            INST(6U, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_NE).Inputs(0U, 2U);
            INST(7U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1U);
        }

        BASIC_BLOCK(4U, 5U) {}

        BASIC_BLOCK(5U, 6U) {}

        BASIC_BLOCK(6U, -1L)
        {
            INST(8U, Opcode::Phi).b().Inputs(1U, 2U);
            INST(9U, Opcode::Return).b().Inputs(8U);
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
}

/* We can go to true branch, to block 5, to block 4, and to block 5 again
 * So we need to check that block 3 has one predecessor
 *    [entry]
 *       |
 *       v F
 *      [2]-->[3]<--\
 *     T |     |    |
 *       |     v    |
 *       \--->[4]---/
 *             |
 *             v
 *            [5]
 *             |
 *             v
 *           [exit]
 */
TEST_F(PeepholesTest, ConditionalAssignmentLoopTriangle)
{
    for (auto inverse : {true, false}) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();
            PARAMETER(1U, 1U).u64();
            CONSTANT(2U, 0U);
            CONSTANT(3U, 1U);

            BASIC_BLOCK(2U, 4U, 3U)
            {
                INST(4U, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_NE).Inputs(0U, 2U);
                INST(5U, Opcode::IfImm)
                    .SrcType(compiler::DataType::BOOL)
                    .CC(inverse ? CC_EQ : CC_NE)
                    .Imm(0U)
                    .Inputs(4U);
            }

            BASIC_BLOCK(3U, 4U) {}

            BASIC_BLOCK(4U, 5U, 3U)
            {
                INST(6U, Opcode::Phi).b().Inputs(2U, 3U);
                INST(7U, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_LT).Inputs(1U, 3U);
                INST(8U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
            }

            BASIC_BLOCK(5U, -1L)
            {
                INST(9U, Opcode::Return).b().Inputs(6U);
            }
        }
        EXPECT_FALSE(graph->RunPass<Peepholes>());
    }
}

/* We can go to true branch, to block 5, to block 4, and to block 5 again
 * So we need to check that block 3 has one predecessor
 *
 *       [entry]
 *          |
 *          v F
 *         [2]-->[3]<--\
 *        T |     |    |
 *          v     v    |
 *         [4]-->[5]---/
 *                |
 *                v
 *               [6]
 *                |
 *                v
 *              [exit]
 */
TEST_F(PeepholesTest, ConditionalAssignmentLoopDiamond)
{
    for (auto inverse : {true, false}) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();
            PARAMETER(1U, 1U).u64();
            CONSTANT(2U, 0U);
            CONSTANT(3U, 1U);

            BASIC_BLOCK(2U, 4U, 3U)
            {
                INST(4U, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_NE).Inputs(0U, 2U);
                INST(5U, Opcode::IfImm)
                    .SrcType(compiler::DataType::BOOL)
                    .CC(inverse ? CC_EQ : CC_NE)
                    .Imm(0U)
                    .Inputs(4U);
            }

            BASIC_BLOCK(3U, 5U) {}

            BASIC_BLOCK(4U, 5U)
            {
                INST(6U, Opcode::SaveState).NoVregs();
                INST(7U, Opcode::CallStatic).u64().InputsAutoType(0U, 6U);
            }

            BASIC_BLOCK(5U, 6U, 3U)
            {
                INST(8U, Opcode::Phi).b().Inputs(2U, 3U);
                INST(9U, Opcode::Compare).b().SrcType(DataType::UINT64).CC(CC_LT).Inputs(1U, 3U);
                INST(10U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
            }

            BASIC_BLOCK(6U, -1L)
            {
                INST(11U, Opcode::Return).b().Inputs(8U);
            }
        }
        EXPECT_FALSE(graph->RunPass<Peepholes>());
    }
}

/*
 *    [entry]
 *       |
 *       v
 *      [2]------\
 *       |       |
 *       |       v
 *       |  /---[3]---\
 *       |  |         |
 *       |  v         v
 *       | [4]       [5]
 *       |  |         |
 *       |  \-->[6]<--/
 *       |       |
 *       v       |
 *      [7]<-----/
 *       |
 *       v
 *     [exit]
 */
TEST_F(PeepholesTest, ConditionalAssignmentNestedIfs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);

        BASIC_BLOCK(2U, 7U, 3U)
        {
            INST(3U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_LT).Inputs(0U, 1U);
            INST(4U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(5U, Opcode::Compare).b().SrcType(DataType::INT64).CC(CC_GT).Inputs(0U, 2U);
            INST(6U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }

        BASIC_BLOCK(4U, 6U)
        {
            INST(7U, Opcode::Add).s64().Inputs(0U, 2U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(8U, Opcode::Sub).s64().Inputs(0U, 2U);
        }

        BASIC_BLOCK(6U, 7U)
        {
            INST(9U, Opcode::Phi).s64().Inputs(7U, 8U);
        }

        BASIC_BLOCK(7U, -1L)
        {
            INST(10U, Opcode::Phi).b().Inputs(2U, 1U);
            INST(11U, Opcode::Phi).s64().Inputs(0U, 9U);
            INST(12U, Opcode::SaveState).NoVregs();
            INST(13U, Opcode::CallStatic).u64().InputsAutoType(10U, 11U, 12U);
            INST(14U, Opcode::Return).u64().Inputs(13U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());

    ASSERT_EQ(INS(13U).GetInput(0U).GetInst(), &INS(3U));
}

TEST_F(PeepholesTest, OverflowChecksOptimize)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        PARAMETER(1U, 0U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::AddOverflowCheck).s32().Inputs(1U, 0U, 2U);
            INST(4U, Opcode::AddOverflowCheck).s32().Inputs(0U, 1U, 2U);
            INST(5U, Opcode::SubOverflowCheck).s32().Inputs(1U, 0U, 2U);
            INST(6U, Opcode::SubOverflowCheck).s32().Inputs(0U, 1U, 2U);  // can't optimize (0 - x)
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(3U, 4U, 5U, 6U, 2U);
            INST(8U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        PARAMETER(1U, 0U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(6U, Opcode::SubOverflowCheck).s32().Inputs(0U, 1U, 2U);  // can't optimize (0 - x)
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(1U, 1U, 1U, 6U, 2U);
            INST(8U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, GetGraph()));
}

/* We check, that in Peephole if new inputs (which is not Constant) and inst in different BB in OSR mode,
 *  optimization will not apply.
 *
 *  What check:
 *  0.u64  Param
 *  1.u64  Param
 *  14.u64 Param
 *  ...
 *  BB 2
 *  15.u64 Add v1, v0
 *  16.u64 Add v0, v14
 *  ...
 *  BB 3
 *  17.u64 Sub v15, v16   <<== Inputs will NOT change
 */
TEST_F(PeepholesTest, SubAddAddFromOtherBBInOsrMode)
{
    auto graphOsr = CreateEmptyGraph();
    graphOsr->SetMode(GraphMode::Osr());

    GRAPH(graphOsr)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(14U, 2U).u64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(15U, Opcode::Add).u64().Inputs(1U, 0U);
            INST(16U, Opcode::Add).u64().Inputs(0U, 14U);

            INST(2U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Compare).b().Inputs(0U, 2U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 6U}});
            INST(13U, Opcode::SaveStateOsr).Inputs(0U, 1U, 5U, 15U, 16U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(6U, Opcode::Sub).u64().Inputs(5U, 1U);
            // This INST 17 will not change!
            INST(17U, Opcode::Sub).u64().Inputs(15U, 16U);
            INST(7U, Opcode::Compare).b().Inputs(6U, 1U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(9U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 6U}});
            INST(10U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(11U, Opcode::Add).u64().Inputs(9U, 10U);
            INST(12U, Opcode::Return).u64().Inputs(11U);
        }
    }

    for (auto bb : graphOsr->GetBlocksRPO()) {
        if (bb->IsLoopHeader()) {
            bb->SetOsrEntry(true);
        }
    }
    auto cloneOsr = GraphCloner(graphOsr, graphOsr->GetAllocator(), graphOsr->GetLocalAllocator()).CloneGraph();
    cloneOsr->RunPass<Peepholes>();
    ASSERT_TRUE(GraphComparator().Compare(graphOsr, cloneOsr));
}

/* We check, that if in Peephole new inputs and inst in different BB in OSR mode for Constants,
 *  optimization will apply.
 *
 *  What check:
 *  14.    Const -> v15
 *  19.    Const -> v16
 *  ...
 *  BB 2
 *  15.u64 Add v14, v1 -> v17
 *  16.u64 Add v1, v19 -> v17
 *  ...
 *  BB 3
 *  17.u64 Sub v15, v16   <<== Inputs will change
 *  ============>
 *  14.    Const -> v15, v17
 *  19.    Const -> v16, v17
 *  ...
 *  BB 2
 *  15.u64 Add v14, v1
 *  16.u64 Add v1, v19
 *  ...
 *  BB 3
 *  17.u64 Sub v14, v19   <<== Inputs changed
 */
TEST_F(PeepholesTest, SubAddAddSameBBInOsrMode)
{
    GetGraph()->SetMode(GraphMode::Osr());
    // (const0 + param) - (param + const1) = const0 - const1
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();

        CONSTANT(14U, 0U);
        CONSTANT(19U, 2U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Compare).b().Inputs(0U, 2U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(5U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 6U}});
            INST(13U, Opcode::SaveStateOsr).Inputs(0U, 1U, 5U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::Sub).u64().Inputs(5U, 1U);

            INST(15U, Opcode::Add).u64().Inputs(14U, 1U);
            INST(16U, Opcode::Add).u64().Inputs(1U, 19U);
            // INST 17 will change
            INST(17U, Opcode::Sub).u64().Inputs(15U, 16U);
            INST(7U, Opcode::Compare).b().Inputs(6U, 1U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(9U, Opcode::Phi).u64().Inputs({{2U, 0U}, {3U, 6U}});
            INST(10U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(11U, Opcode::Add).u64().Inputs(9U, 10U);
            INST(12U, Opcode::Return).u64().Inputs(11U);
        }
    }

    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (bb->IsLoopHeader()) {
            bb->SetOsrEntry(true);
        }
    }
    GetGraph()->RunPass<Peepholes>();
    GraphChecker(GetGraph()).Check();
    ASSERT_EQ(INS(17U).GetInput(0U).GetInst(), &INS(14U));
    ASSERT_EQ(INS(17U).GetInput(1U).GetInst(), &INS(19U));
}

TEST_F(PeepholesTest, GetInstanceClassTest)
{
    auto klass = reinterpret_cast<RuntimeInterface::ClassPtr>(2U);
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::SaveState).NoVregs();
            INST(1U, Opcode::LoadAndInitClass).ref().Inputs(0U).TypeId(1U).Class(klass);
            INST(2U, Opcode::NewObject).ref().Inputs(1U, 0U).TypeId(1U);
            INST(3U, Opcode::GetInstanceClass).ref().Inputs(2U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }
    GetGraph()->RunPass<ObjectTypePropagation>();  // set analysis valid
    INS(2U).SetObjectTypeInfo(ObjectTypeInfo(klass, true));
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::SaveState).NoVregs();
            INST(1U, Opcode::LoadAndInitClass).ref().Inputs(0U).TypeId(1U).Class(klass);
            INST(2U, Opcode::NewObject).ref().Inputs(1U, 0U).TypeId(1U);
            INST(3U, Opcode::LoadImmediate).ref().Class(klass);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, GetGraph()));
}

// Replace by Neg
TEST_F(PeepholesTest, NegOverflowAndZeroCheckShl)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::NegOverflowAndZeroCheck).s32().Inputs(0U, 5U);
            INST(7U, Opcode::Shl).s32().Inputs(6U, 1U);
            INST(8U, Opcode::Return).s32().Inputs(7U);
        }
    }
    ASSERT_TRUE(graph1->RunPass<Peepholes>());
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::Neg).s32().Inputs(0U);
            INST(7U, Opcode::Shl).s32().Inputs(6U, 1U);
            INST(8U, Opcode::Return).s32().Inputs(7U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(PeepholesTest, NegOverflowAndZeroCheckShr)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::NegOverflowAndZeroCheck).s32().Inputs(0U, 5U);
            INST(7U, Opcode::Shr).s32().Inputs(6U, 1U);
            INST(8U, Opcode::Return).s32().Inputs(7U);
        }
    }
    ASSERT_TRUE(graph1->RunPass<Peepholes>());
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::Neg).s32().Inputs(0U);
            INST(7U, Opcode::Shr).s32().Inputs(6U, 1U);
            INST(8U, Opcode::Return).s32().Inputs(7U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(PeepholesTest, NegOverflowAndZeroCheckAShr)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::NegOverflowAndZeroCheck).s32().Inputs(0U, 5U);
            INST(7U, Opcode::AShr).s32().Inputs(6U, 1U);
            INST(8U, Opcode::Return).s32().Inputs(7U);
        }
    }
    ASSERT_TRUE(graph1->RunPass<Peepholes>());
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::Neg).s32().Inputs(0U);
            INST(7U, Opcode::AShr).s32().Inputs(6U, 1U);
            INST(8U, Opcode::Return).s32().Inputs(7U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(PeepholesTest, NegOverflowAndZeroCheckAnd)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::NegOverflowAndZeroCheck).s32().Inputs(0U, 5U);
            INST(7U, Opcode::And).u32().Inputs(6U, 1U);
            INST(8U, Opcode::Return).u32().Inputs(7U);
        }
    }
    ASSERT_TRUE(graph1->RunPass<Peepholes>());
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::Neg).s32().Inputs(0U);
            INST(7U, Opcode::And).u32().Inputs(6U, 1U);
            INST(8U, Opcode::Return).u32().Inputs(7U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(PeepholesTest, NegOverflowAndZeroCheckOr)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::NegOverflowAndZeroCheck).s32().Inputs(0U, 5U);
            INST(7U, Opcode::Or).u32().Inputs(6U, 1U);
            INST(8U, Opcode::Return).u32().Inputs(7U);
        }
    }
    ASSERT_TRUE(graph1->RunPass<Peepholes>());
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::Neg).s32().Inputs(0U);
            INST(7U, Opcode::Or).u32().Inputs(6U, 1U);
            INST(8U, Opcode::Return).u32().Inputs(7U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(PeepholesTest, NegOverflowAndZeroCheckXor)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::NegOverflowAndZeroCheck).s32().Inputs(0U, 5U);
            INST(7U, Opcode::Xor).u32().Inputs(6U, 1U);
            INST(8U, Opcode::Return).u32().Inputs(7U);
        }
    }
    ASSERT_TRUE(graph1->RunPass<Peepholes>());
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::Neg).s32().Inputs(0U);
            INST(7U, Opcode::Xor).u32().Inputs(6U, 1U);
            INST(8U, Opcode::Return).u32().Inputs(7U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(PeepholesTest, NegOverflowAndZeroCheckNotApplied)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::NegOverflowAndZeroCheck).s32().Inputs(0U, 5U);
            INST(7U, Opcode::Add).s32().Inputs(6U, 1U);
            INST(8U, Opcode::Return).s32().Inputs(7U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

SRC_GRAPH(OverflowCheckAsBitwiseInput, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(3U, 0U);
        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::AddOverflowCheck).s32().Inputs(0U, 1U, 4U);

            INST(6U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 5U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(7U, Opcode::SubOverflowCheck).s32().Inputs(5U, 2U, 6U);

            INST(8U, Opcode::Compare).b().CC(CC_GE).SrcType(DataType::Type::INT32).Inputs(2U, 0U);
            INST(9U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0U).Inputs(8U);
        }

        BASIC_BLOCK(4U, 3U)
        {
            INST(10U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 5U, 7U).SrcVregs({0U, 1U, 2U, 3U, 4U, 5U});
            INST(11U, Opcode::NegOverflowAndZeroCheck).s32().Inputs(2U, 10U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(12U, Opcode::Phi).s32().Inputs({{2U, 7U}, {4U, 11U}});
            INST(13U, Opcode::Or).s32().Inputs(12U, 3U);
            INST(14U, Opcode::Return).s32().Inputs(13U);
        }
    }
}

OUT_GRAPH(OverflowCheckAsBitwiseInput, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(3U, 0U);
        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::Add).s32().Inputs(0U, 1U);

            INST(6U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 5U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(7U, Opcode::Sub).s32().Inputs(5U, 2U);

            INST(8U, Opcode::Compare).b().CC(CC_GE).SrcType(DataType::Type::INT32).Inputs(2U, 0U);
            INST(9U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0U).Inputs(8U);
        }

        BASIC_BLOCK(4U, 3U)
        {
            INST(10U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 5U, 7U).SrcVregs({0U, 1U, 2U, 3U, 4U, 5U});
            INST(11U, Opcode::Neg).s32().Inputs(2U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(12U, Opcode::Phi).s32().Inputs({{2U, 7U}, {4U, 11U}});
            INST(13U, Opcode::Or).s32().Inputs(12U, 3U);
            INST(14U, Opcode::Return).s32().Inputs(12U);
        }
    }
}

TEST_F(PeepholesTest, OverflowCheckAsBitwiseInput)
{
    src_graph::OverflowCheckAsBitwiseInput::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    auto graph = CreateEmptyGraph();
    out_graph::OverflowCheckAsBitwiseInput::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, OverflowCheckAsBitwiseAndSSInput)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 255U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::AddOverflowCheck).s32().Inputs(0U, 1U, 2U);

            INST(6U, Opcode::And).s32().Inputs(3U, 1U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 3U, 6U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::AddOverflowCheck).s32().Inputs(6U, 1U, 7U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(PeepholesTest, ReplacingXorWithCompare)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::CallStatic).b().InputsAutoType(1U);
            INST(3U, Opcode::Xor).s32().Inputs(0U, 2U);
            INST(9U, Opcode::Return).s32().Inputs(3U);
        }
    }

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(10U, 0U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::CallStatic).b().InputsAutoType(1U);
            INST(11U, Opcode::Compare).b().CC(ConditionCode::CC_EQ).Inputs(2U, 10U);
            INST(9U, Opcode::Return).s32().Inputs(11U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());

    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, MultiArrayWithLenArrayFirstLayer)
{
    auto class1 = reinterpret_cast<RuntimeInterface::ClassPtr>(1U);
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0x3U);
        CONSTANT(1U, 0x2U);
        CONSTANT(7U, 0x1U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::LoadImmediate).ref().Class(class1);
            INST(4U, Opcode::MultiArray)
                .ref()
                .InputsAutoType(3U, 0U, 1U, 2U);  // Will be create [ [ [][] ], [ [][] ], [ [][] ] ]
            INST(5U, Opcode::LenArray).Inputs(4U).s32();
            INST(10U, Opcode::Return).s32().Inputs(5U);
        }
    }

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0x3U);
        CONSTANT(1U, 0x2U);
        CONSTANT(7U, 0x1U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::LoadImmediate).ref().Class(class1);
            INST(4U, Opcode::MultiArray)
                .ref()
                .InputsAutoType(3U, 0U, 1U, 2U);  // Will be create [ [ [][] ], [ [][] ], [ [][] ] ]
            INST(5U, Opcode::LenArray).Inputs(4U).s32();
            INST(10U, Opcode::Return).s32().Inputs(0U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(MultiArrayWithLenArraySecondLayer, Graph *graph)
{
    auto class1 = reinterpret_cast<RuntimeInterface::ClassPtr>(1U);
    GRAPH(graph)
    {
        PARAMETER(0U, 0x3U).s32();
        CONSTANT(1U, 0x2U);
        CONSTANT(7U, 0x1U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::LoadImmediate).ref().Class(class1);
            INST(4U, Opcode::MultiArray)
                .ref()
                .InputsAutoType(3U, 0U, 1U, 2U);  // Will be create [ [ [][] ], [ [][] ], [ [][] ] ]
            INST(5U, Opcode::LenArray).Inputs(4U).s32();
            INST(11U, Opcode::LoadArray).ref().Inputs(4U, 7U);
            INST(12U, Opcode::NullCheck).ref().Inputs(11U, 2U);
            INST(13U, Opcode::LenArray).Inputs(12U).s32();
            INST(14U, Opcode::Add).Inputs(5U, 13U).s32();
            INST(10U, Opcode::Return).s32().Inputs(14U);
        }
    }
}

OUT_GRAPH(MultiArrayWithLenArraySecondLayer, Graph *graph)
{
    auto class1 = reinterpret_cast<RuntimeInterface::ClassPtr>(1U);
    GRAPH(graph)
    {
        PARAMETER(0U, 0x3U).s32();
        CONSTANT(1U, 0x2U);
        CONSTANT(7U, 0x1U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::LoadImmediate).ref().Class(class1);
            INST(4U, Opcode::MultiArray)
                .ref()
                .InputsAutoType(3U, 0U, 1U, 2U);  // Will be create [ [ [][] ], [ [][] ], [ [][] ] ]
            INST(5U, Opcode::LenArray).Inputs(4U).s32();
            INST(11U, Opcode::LoadArray).ref().Inputs(4U, 7U);
            INST(12U, Opcode::NullCheck).ref().Inputs(11U, 2U);
            INST(13U, Opcode::LenArray).Inputs(12U).s32();
            INST(14U, Opcode::Add).Inputs(0U, 1U).s32();
            INST(10U, Opcode::Return).s32().Inputs(14U);
        }
    }
}

TEST_F(PeepholesTest, MultiArrayWithLenArraySecondLayer)
{
    src_graph::MultiArrayWithLenArraySecondLayer::CREATE(GetGraph());

    auto graph = CreateEmptyGraph();
    out_graph::MultiArrayWithLenArraySecondLayer::CREATE(graph);
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(MultiArrayWithLenArrayOfString, Graph *graph)
{
    auto class1 = reinterpret_cast<RuntimeInterface::ClassPtr>(1U);
    GRAPH(graph)
    {
        PARAMETER(0U, 0x1U).s32();
        CONSTANT(1U, 0x3U);
        PARAMETER(2U, 0x2U).s32();  // Will think, that value is 0x2

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState);
            INST(4U, Opcode::LoadImmediate).ref().Class(class1);
            INST(5U, Opcode::MultiArray)
                .ref()
                .InputsAutoType(4U, 1U, 2U,
                                3U);  // Will be create [ [ String, String ], [ String, String ], [ String, String ] ]
            INST(6U, Opcode::LenArray).Inputs(5U).s32();

            INST(7U, Opcode::LoadArray).ref().Inputs(5U, 0U);  // Loaded: [ String, String ]
            INST(8U, Opcode::NullCheck).ref().Inputs(7U, 3U);
            INST(9U, Opcode::LenArray).Inputs(8U).s32();

            INST(10U, Opcode::LoadArray).ref().Inputs(8U, 0U);  // Loaded: String
            INST(11U, Opcode::NullCheck).ref().Inputs(10U, 3U);
            INST(12U, Opcode::LenArray).Inputs(11U).s32();  // Length of String

            INST(13U, Opcode::Add).Inputs(9U, 6U).s32();
            INST(14U, Opcode::Add).Inputs(13U, 12U).s32();
            INST(15U, Opcode::Return).s32().Inputs(14U);
        }
    }
}

OUT_GRAPH(MultiArrayWithLenArrayOfString, Graph *graph)
{
    auto class1 = reinterpret_cast<RuntimeInterface::ClassPtr>(1U);
    GRAPH(graph)
    {
        PARAMETER(0U, 0x1U).s32();
        CONSTANT(1U, 0x3U);
        PARAMETER(2U, 0x2U).s32();  // Will think, that value is 0x2

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState);
            INST(4U, Opcode::LoadImmediate).ref().Class(class1);
            INST(5U, Opcode::MultiArray)
                .ref()
                .InputsAutoType(4U, 1U, 2U,
                                3U);  // Will be create [ [ String, String ], [ String, String ], [ String, String ] ]
            INST(6U, Opcode::LenArray).Inputs(5U).s32();

            INST(7U, Opcode::LoadArray).ref().Inputs(5U, 0U);  // Loaded: [ String, String ]
            INST(8U, Opcode::NullCheck).ref().Inputs(7U, 3U);
            INST(9U, Opcode::LenArray).Inputs(8U).s32();

            INST(10U, Opcode::LoadArray).ref().Inputs(8U, 0U);  // Loaded: String
            INST(11U, Opcode::NullCheck).ref().Inputs(10U, 3U);
            INST(12U, Opcode::LenArray).Inputs(11U).s32();  // Length of String

            INST(13U, Opcode::Add).Inputs(2U, 1U).s32();
            INST(14U, Opcode::Add).Inputs(13U, 12U).s32();
            INST(15U, Opcode::Return).s32().Inputs(14U);
        }
    }
}

TEST_F(PeepholesTest, MultiArrayWithLenArrayOfString)
{
    src_graph::MultiArrayWithLenArrayOfString::CREATE(GetGraph());

    auto graph = CreateEmptyGraph();
    out_graph::MultiArrayWithLenArrayOfString::CREATE(graph);

    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, RemoveDoubleXor)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).i32();  // arg 0
        CONSTANT(1U, 1U).i64();   // Constant i64 0x1
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Xor).i32().Inputs(0U, 1U);
            INST(3U, Opcode::Xor).i32().Inputs(2U, 1U);
            INST(4U, Opcode::Return).b().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).i32();  // arg 0
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::Return).b().Inputs(0U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, RemoveDoubleXorNeg1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).i32();  // arg 0
        PARAMETER(1U, 1U).i32();  // arg 1 (non-const input)
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Xor).i32().Inputs(0U, 1U);
            INST(3U, Opcode::Xor).i32().Inputs(2U, 1U);
            INST(4U, Opcode::Return).b().Inputs(3U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(PeepholesTest, RemoveDoubleXorNeg2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).i32();  // arg 0
        CONSTANT(1U, 1U).i64();   // Constant i64 0x1
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).i32().Inputs(0U, 1U);  // Non-Xor input of the second Xor
            INST(3U, Opcode::Xor).i32().Inputs(2U, 1U);
            INST(4U, Opcode::Return).b().Inputs(3U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(PeepholesTest, RemoveDoubleXorNeg3)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).i32();  // arg 0
        CONSTANT(1U, 1U).i64();   // Constant i64 0x1
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Xor).i32().Inputs(0U, 1U);
            INST(3U, Opcode::Xor).i32().Inputs(2U, 1U);
            INST(4U, Opcode::Add).i32().Inputs(2U, 0U);  // Extra user of first Xor
            INST(5U, Opcode::Return).b().Inputs(3U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(PeepholesTest, RemoveDoubleXorNeg4)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).i32();  // arg 0
        CONSTANT(1U, 1U).i64();   // Constant i64 0x1
        CONSTANT(2U, 2U).i64();   // Another constant i64 0x2
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Xor).i32().Inputs(0U, 1U);
            INST(4U, Opcode::Xor).i32().Inputs(3U, 2U);
            INST(5U, Opcode::Return).b().Inputs(4U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
