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
#include "optimizer/optimizations/checks_elimination.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/ir/runtime_interface.h"
#include "optimizer/ir/graph_cloner.h"

namespace ark::compiler {
class ChecksEliminationTest : public CommonTest {
public:
    ChecksEliminationTest() : graph_(CreateGraphStartEndBlocks()) {}

    Graph *GetGraph()
    {
        return graph_;
    }

    struct RuntimeInterfaceNotStaticMethod : public compiler::RuntimeInterface {
        bool IsMethodStatic([[maybe_unused]] MethodPtr method) const override
        {
            return false;
        }
    };

    // NOLINTBEGIN(readability-magic-numbers)
    template <bool IS_APPLIED>
    void SimpleTest(int32_t index, int32_t arrayLen)
    {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            CONSTANT(0U, arrayLen);
            CONSTANT(1U, index);
            BASIC_BLOCK(2U, 1U)
            {
                INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
                INST(44U, Opcode::LoadAndInitClass).ref().Inputs(2U).TypeId(68U);
                INST(3U, Opcode::NewArray).ref().Inputs(44U, 0U, 2U);
                INST(4U, Opcode::BoundsCheck).s32().Inputs(0U, 1U, 2U);
                INST(5U, Opcode::LoadArray).s32().Inputs(3U, 4U);
                INST(6U, Opcode::Return).s32().Inputs(5U);
            }
        }
        graph1->RunPass<ChecksElimination>();
        auto graph2 = CreateEmptyGraph();
        if constexpr (IS_APPLIED) {
            GRAPH(graph2)
            {
                CONSTANT(0U, arrayLen);
                CONSTANT(1U, index);
                BASIC_BLOCK(2U, 1U)
                {
                    INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
                    INST(44U, Opcode::LoadAndInitClass).ref().Inputs(2U).TypeId(68U);
                    INST(3U, Opcode::NewArray).ref().Inputs(44U, 0U, 2U);
                    INST(4U, Opcode::NOP);
                    INST(5U, Opcode::LoadArray).s32().Inputs(3U, 1U);
                    INST(6U, Opcode::Return).s32().Inputs(5U);
                }
            }
        } else {
            GRAPH(graph2)
            {
                CONSTANT(0U, arrayLen);
                CONSTANT(1U, index);
                BASIC_BLOCK(2U, 1U)
                {
                    INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
                    INST(44U, Opcode::LoadAndInitClass).ref().Inputs(2U).TypeId(68U);
                    INST(3U, Opcode::NewArray).ref().Inputs(44U, 0U, 2U);
                    INST(4U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(2U);
                }
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }

    enum AppliedType { NOT_APPLIED, REMOVED, REPLACED };

    Graph *ArithmeticTestInput(int32_t index, int32_t arrayLen, Opcode opc, int32_t val)
    {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            CONSTANT(0U, arrayLen);
            CONSTANT(1U, index);
            CONSTANT(2U, val);
            BASIC_BLOCK(2U, 1U)
            {
                INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
                INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).TypeId(68U);
                INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
                INST(5U, opc).s32().Inputs(1U, 2U);
                INST(6U, Opcode::BoundsCheck).s32().Inputs(0U, 5U, 3U);
                INST(7U, Opcode::LoadArray).s32().Inputs(4U, 6U);
                INST(8U, Opcode::Return).s32().Inputs(7U);
            }
        }
        return graph;
    }

    Graph *ArithmeticTestOutput1(int32_t index, int32_t arrayLen, Opcode opc, int32_t val)
    {
        auto graph = CreateEmptyGraph();

        GRAPH(graph)
        {
            CONSTANT(0U, arrayLen);
            CONSTANT(1U, index);
            CONSTANT(2U, val);
            BASIC_BLOCK(2U, 1U)
            {
                INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
                INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).TypeId(68U);
                INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
                INST(5U, opc).s32().Inputs(1U, 2U);
                INST(6U, Opcode::NOP);
                INST(7U, Opcode::LoadArray).s32().Inputs(4U, 5U);
                INST(8U, Opcode::Return).s32().Inputs(7U);
            }
        }

        return graph;
    }

    Graph *ArithmeticTestOutput2(int32_t index, int32_t arrayLen, Opcode opc, int32_t val)
    {
        auto graph = CreateEmptyGraph();

        GRAPH(graph)
        {
            CONSTANT(0U, arrayLen);
            CONSTANT(1U, index);
            CONSTANT(2U, val);
            BASIC_BLOCK(2U, 1U)
            {
                INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
                INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).TypeId(68U);
                INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
                INST(5U, opc).s32().Inputs(1U, 2U);
                INST(6U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(3U);
            }
        }
        return graph;
    }

    template <AppliedType APPLIED_TYPE>
    void ArithmeticTest(int32_t index, int32_t arrayLen, Opcode opc, int32_t val)
    {
        auto graph1 = ArithmeticTestInput(index, arrayLen, opc, val);
        auto clone = GraphCloner(graph1, graph1->GetAllocator(), graph1->GetLocalAllocator()).CloneGraph();
        bool result = graph1->RunPass<ChecksElimination>();
        Graph *graph2 = nullptr;
        if constexpr (APPLIED_TYPE == AppliedType::REMOVED) {
            graph2 = ArithmeticTestOutput1(index, arrayLen, opc, val);
            ASSERT_TRUE(result);
        } else if constexpr (APPLIED_TYPE == AppliedType::REPLACED) {
            graph2 = ArithmeticTestOutput2(index, arrayLen, opc, val);
            ASSERT_TRUE(result);
        } else {
            ASSERT_FALSE(result);
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, (APPLIED_TYPE == AppliedType::NOT_APPLIED) ? clone : graph2));
    }

    Graph *ModTestInput(int32_t arrayLen, int32_t mod)
    {
        auto graph = CreateEmptyGraph();

        GRAPH(graph)
        {
            CONSTANT(0U, arrayLen);
            CONSTANT(1U, mod);
            CONSTANT(12U, 0U);
            PARAMETER(2U, 0U).s32();
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(10U, Opcode::Compare).b().CC(CC_GE).SrcType(DataType::Type::INT32).Inputs(2U, 12U);
                INST(11U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0U).Inputs(10U);
            }
            BASIC_BLOCK(3U, 1U)
            {
                INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
                INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).TypeId(68U);
                INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
                INST(5U, Opcode::Mod).s32().Inputs(2U, 1U);
                INST(6U, Opcode::BoundsCheck).s32().Inputs(0U, 5U, 3U);
                INST(7U, Opcode::LoadArray).s32().Inputs(4U, 6U);
                INST(8U, Opcode::Return).s32().Inputs(7U);
            }
            BASIC_BLOCK(4U, 1U)
            {
                INST(9U, Opcode::Return).s32().Inputs(1U);
            }
        }

        return graph;
    }

    Graph *ModTestOutput(int32_t arrayLen, int32_t mod)
    {
        auto graph = CreateEmptyGraph();

        GRAPH(graph)
        {
            CONSTANT(0U, arrayLen);
            CONSTANT(1U, mod);
            CONSTANT(12U, 0U);
            PARAMETER(2U, 0U).s32();
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(10U, Opcode::Compare).b().CC(CC_GE).SrcType(DataType::Type::INT32).Inputs(2U, 12U);
                INST(11U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0U).Inputs(10U);
            }
            BASIC_BLOCK(3U, 1U)
            {
                INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
                INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).TypeId(68U);
                INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
                INST(5U, Opcode::Mod).s32().Inputs(2U, 1U);
                INST(6U, Opcode::NOP);
                INST(7U, Opcode::LoadArray).s32().Inputs(4U, 5U);
                INST(8U, Opcode::Return).s32().Inputs(7U);
            }
            BASIC_BLOCK(4U, 1U)
            {
                INST(9U, Opcode::Return).s32().Inputs(1U);
            }
        }
        return graph;
    }

    template <bool IS_APPLIED>
    void ModTest(int32_t arrayLen, int32_t mod)
    {
        auto graph1 = ModTestInput(arrayLen, mod);

        Graph *graph2 = nullptr;
        if constexpr (IS_APPLIED) {
            graph2 = ModTestOutput(arrayLen, mod);
        } else {
            graph2 = GraphCloner(graph1, graph1->GetAllocator(), graph1->GetLocalAllocator()).CloneGraph();
        }
        graph1->RunPass<ChecksElimination>();
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }

    Graph *PhiTestInput(int32_t index, int32_t lenArray, int32_t mod)
    {
        auto graph = CreateEmptyGraph();

        GRAPH(graph)
        {
            PARAMETER(0U, 0U).b();
            PARAMETER(1U, 1U).s32();
            CONSTANT(2U, lenArray);  // len array
            CONSTANT(3U, index);     // index 2
            CONSTANT(12U, mod);
            CONSTANT(13U, 0U);
            BASIC_BLOCK(11U, 2U, 5U)
            {
                INST(41U, Opcode::Compare).b().CC(CC_GE).SrcType(DataType::Type::INT32).Inputs(1U, 13U);
                INST(42U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0U).Inputs(41U);
            }
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(43U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
                INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
                INST(4U, Opcode::NewArray).ref().Inputs(44U, 2U, 43U);
                INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
            }
            BASIC_BLOCK(3U, 4U)
            {
                INST(6U, Opcode::Mod).s32().Inputs(1U, 12U);
            }
            BASIC_BLOCK(4U, -1)
            {
                INST(7U, Opcode::Phi).s32().Inputs(3U, 6U);
                INST(8U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 4U).SrcVregs({0U, 1U, 2U, 3U, 4U});
                INST(9U, Opcode::BoundsCheck).s32().Inputs(2U, 7U, 8U);
                INST(10U, Opcode::LoadArray).s32().Inputs(4U, 9U);
                INST(11U, Opcode::Return).s32().Inputs(10U);
            }
            BASIC_BLOCK(5U, -1)
            {
                INST(19U, Opcode::Return).s32().Inputs(1U);
            }
        }

        return graph;
    }
    Graph *PhiTestOutput1(int32_t index, int32_t lenArray, int32_t mod)
    {
        auto graph = CreateEmptyGraph();

        GRAPH(graph)
        {
            PARAMETER(0U, 0U).b();
            PARAMETER(1U, 1U).s32();  // index 1
            CONSTANT(2U, lenArray);   // len array
            CONSTANT(3U, index);      // index 2
            CONSTANT(12U, mod);
            CONSTANT(13U, 0U);
            BASIC_BLOCK(11U, 2U, 5U)
            {
                INST(41U, Opcode::Compare).b().CC(CC_GE).SrcType(DataType::Type::INT32).Inputs(1U, 13U);
                INST(42U, Opcode::IfImm).SrcType(compiler::DataType::BOOL).CC(compiler::CC_NE).Imm(0U).Inputs(41U);
            }
            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(43U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
                INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
                INST(4U, Opcode::NewArray).ref().Inputs(44U, 2U, 43U);
                INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(0U);
            }
            BASIC_BLOCK(3U, 4U)
            {
                INST(6U, Opcode::Mod).s32().Inputs(1U, 12U);
            }
            BASIC_BLOCK(4U, -1)
            {
                INST(7U, Opcode::Phi).s32().Inputs(3U, 6U);
                INST(8U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 4U).SrcVregs({0U, 1U, 2U, 3U, 4U});
                INST(9U, Opcode::NOP);
                INST(10U, Opcode::LoadArray).s32().Inputs(4U, 7U);
                INST(11U, Opcode::Return).s32().Inputs(10U);
            }
            BASIC_BLOCK(5U, -1)
            {
                INST(19U, Opcode::Return).s32().Inputs(1U);
            }
        }

        return graph;
    }

    template <bool IS_APPLIED>
    void PhiTest(int32_t index, int32_t lenArray, int32_t mod)
    {
        auto graph1 = PhiTestInput(index, lenArray, mod);
        [[maybe_unused]] Graph *graph2 = nullptr;
        if constexpr (IS_APPLIED) {
            ASSERT_TRUE(graph1->RunPass<ChecksElimination>());
            graph2 = PhiTestOutput1(index, lenArray, mod);
        } else {
            ASSERT_FALSE(graph1->RunPass<ChecksElimination>());
            graph2 = PhiTestInput(index, lenArray, mod);
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }

    void BuildHoistRefTypeCheckGraph();
    void BuildHoistCheckCastGraph();
    Graph *BuildGraphLoopWithUnknowLowerUpperValueDown(ConditionCode cc);
    void BuildGraphNullCheckTest3();
    void BuildGraphNullCheckTest4();
    void BuildGraphHoistNegativeCheckTest();
    void BuildGraphHoistZeroCheckTest();
    void BuildGraphIfTestTrueBlock();
    void BuildGraphIfTestFalseBlock();
    void BuildGraphIfTestTrueBlock1();
    void BuildGraphIfTestTrueBlock2();
    void BuildGraphIfTestTrueBlock3();
    void BuildGraphIfTestFalseBlock1();
    void BuildGraphIfTestFalseBlock2();
    void BuildGraphIfTestFalseBlock3();
    void BuildGraphSimpleLoopTestInc();
    void BuildGraphSimpleLoopTestIncAfterPeeling();
    void BuildGraphSimpleLoopTestIncAfterPeeling1();
    void BuildGraphSimpleLoopTestDec();
    void BuildGraphLoopWithUnknowLowerUpperValue();
    void BuildGraphUpperOOB();
    void BuildGraphUpperWithDec();
    void BuildGraphUnknownUpperWithDec();
    void BuildGraphLoopWithUnknowLowerValue();
    void BuildGraphLoopWithUnknowUpperValueLE();
    void BuildGraphLoopWithUnknowUpperValueLT();
    void BuildGraphLoopSeveralBoundsChecks();
    void BuildGraphLoopSeveralIndexesBoundsChecks();
    void BuildGraphHeadExitLoop();
    void BuildGraphLoopTest1();
    void BuildGraphBatchLoopTest();
    void BuildGraphGroupedBoundsCheck();
    void BuildGraphGroupedBoundsCheckConstIndex();
    void BuildGraphRefTypeCheck();
    void BuildGraphRefTypeCheckFirstNullCheckEliminated();
    void BuildGraphBugWithNullCheck();
    void BuildGraphNullAndBoundsChecksNestedLoop();
    void BuildGraphLoopWithTwoPhi();
    void BuildGraphLoopWithBigStepGE();
    void BuildGraphLoopWithBigStepLE();
    void BuildGraphLoopWithBigStepLT();
    void BuildGraphLoopWithBoundsCheckUnderIfGE();
    void BuildGraphLoopWithBoundsCheckUnderIfLT();
    void BuildGraphCheckCastAfterIsInstance();
    void BuildGraphOverflowCheckOptimize();

private:
    Graph *graph_ {nullptr};
};

TEST_F(ChecksEliminationTest, NullCheckTest)
{
    // Check Elimination for NullCheck is applied.
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 10U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::LoadArray).s32().Inputs(3U, 1U);
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(6U, Opcode::NullCheck).ref().Inputs(0U, 5U);
            INST(7U, Opcode::StoreArray).s32().Inputs(6U, 1U, 4U);
            INST(8U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 10U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::LoadArray).s32().Inputs(3U, 1U);
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(6U, Opcode::NOP);
            INST(7U, Opcode::StoreArray).s32().Inputs(3U, 1U, 4U);
            INST(8U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NullCheckTest1)
{
    // Check Elimination for NullCheck isn't applied. Different inputs in NullCheck insts.
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        CONSTANT(2U, 10U);

        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(5U, Opcode::LoadArray).s32().Inputs(4U, 2U);
            INST(6U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::NullCheck).ref().Inputs(1U, 6U);
            INST(8U, Opcode::StoreArray).s32().Inputs(7U, 2U, 5U);
            INST(9U, Opcode::Return).s32().Inputs(5U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(ChecksEliminationTest, NullCheckTest2)
{
    // Check Elimination for NullCheck isn't applied. NullCheck(5) isn't dominator of NullCheck(8).
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 10U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(1U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::LoadArray).s32().Inputs(5U, 1U);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(8U, Opcode::NullCheck).ref().Inputs(0U, 7U);
            INST(9U, Opcode::LoadArray).s32().Inputs(8U, 1U);
        }

        BASIC_BLOCK(5U, 1U)
        {
            INST(10U, Opcode::Phi).s32().Inputs(6U, 9U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

void ChecksEliminationTest::BuildGraphNullCheckTest3()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(2U, 10U);
        PARAMETER(3U, 0U).ref();
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(16U, Opcode::LenArray).s32().Inputs(3U);
            INST(20U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);  // 0 < 10
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(15U, Opcode::NullCheck).ref().Inputs(3U, 7U);
            INST(8U, Opcode::BoundsCheck).s32().Inputs(16U, 4U, 7U);
            INST(9U, Opcode::StoreArray).s32().Inputs(15U, 8U, 0U);    // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);               // i++
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 2U);  // i < 10
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
}

TEST_F(ChecksEliminationTest, NullCheckTest3)
{
    BuildGraphNullCheckTest3();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(2U, 10U);
        PARAMETER(3U, 0U).ref();
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(16U, Opcode::LenArray).s32().Inputs(3U);
            INST(20U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(19U, Opcode::NullCheck).ref().Inputs(3U, 20U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(23U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(16U, 2U);  // len_array < 10
            INST(24U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(23U, 20U);
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);  // 0 < 10
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::NOP);
            INST(9U, Opcode::StoreArray).s32().Inputs(19U, 4U, 0U);    // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);               // i++
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 2U);  // i < 10
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void ChecksEliminationTest::BuildGraphNullCheckTest4()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(2U, 10U);
        CONSTANT(3U, nullptr);
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(20U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);  // 0 < 10
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(15U, Opcode::NullCheck).ref().Inputs(3U, 7U);
            INST(9U, Opcode::StoreArray).s32().Inputs(15U, 4U, 0U);    // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);               // i++
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 2U);  // i < 10
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
}

TEST_F(ChecksEliminationTest, NullCheckTest4)
{
    BuildGraphNullCheckTest4();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(2U, 10U);
        CONSTANT(3U, nullptr);
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(20U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);  // 0 < 10
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 1U)
        {
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(15U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(7U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NegativeCheckTest)
{
    // Check Elimination for NegativeCheck is applied.
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NegativeCheck).s32().Inputs(0U, 1U);
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 0U, 1U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 10U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NOP);
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 0U, 1U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NegativeCheckTestZero)
{
    // Check Elimination for NegativeCheck is applied.
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NegativeCheck).s32().Inputs(0U, 1U);
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U, 1U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NOP);
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 0U, 1U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NegativeCheckTest1)
{
    // Check Elimination for NegativeCheck is applied. Negative constant.
    // Check replaced by deoptimize
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -5L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NegativeCheck).s32().Inputs(0U, 1U);
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U, 1U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, -5L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::NEGATIVE_CHECK).Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NegativeCheckTest3)
{
    // Check Elimination for NegativeCheck is applied. Positiv value.
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -5L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(5U, Opcode::Neg).s32().Inputs(0U);
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NegativeCheck).s32().Inputs(5U, 1U);
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U, 1U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, -5L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(5U, Opcode::Neg).s32().Inputs(0U);
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NOP);
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 5U, 1U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NegativeCheckTest4)
{
    // Check Elimination for NegativeCheck is applied. Negative value.
    // Check replaced by deoptimize
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 5U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(5U, Opcode::Neg).s32().Inputs(0U);
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NegativeCheck).s32().Inputs(5U, 1U);
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U, 1U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 5U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(5U, Opcode::Neg).s32().Inputs(0U);
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::NEGATIVE_CHECK).Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void ChecksEliminationTest::BuildGraphHoistNegativeCheckTest()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(2U, 10U);
        PARAMETER(3U, 0U).s32();
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(20U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);  // 0 < 10
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::NegativeCheck).s32().Inputs(3U, 7U);
            INST(9U, Opcode::LoadAndInitClass).ref().Inputs(7U).TypeId(68U);
            INST(11U, Opcode::NewArray).ref().Inputs(9U, 8U, 7U);
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);               // i++
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 2U);  // i < 10
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(ChecksEliminationTest, HoistNegativeCheckTest)
{
    BuildGraphHoistNegativeCheckTest();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(2U, 10U);
        PARAMETER(3U, 0U).s32();
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(20U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(22U, Opcode::NegativeCheck).s32().Inputs(3U, 20U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);  // 0 < 10
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(9U, Opcode::LoadAndInitClass).ref().Inputs(7U).TypeId(68U);
            INST(11U, Opcode::NewArray).ref().Inputs(9U, 22U, 7U);
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);               // i++
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 2U);  // i < 10
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, ZeroCheckTest)
{
    // Check Elimination for ZeroCheck is applied.
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 12U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::ZeroCheck).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Div).s32().Inputs(0U, 3U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 12U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::NOP);
            INST(4U, Opcode::Div).s32().Inputs(0U, 1U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, ZeroCheckTestBigConst)
{
    // Check Elimination for ZeroCheck is applied.
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 0x8000000000000000U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::ZeroCheck).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Div).s32().Inputs(0U, 3U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 0x8000000000000000U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::NOP);
            INST(4U, Opcode::Div).s32().Inputs(0U, 1U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, ZeroCheckTest1)
{
    // Check Elimination for ZeroCheck isn't applied. Zero constant.
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::ZeroCheck).s32().Inputs(1U, 2U);
            INST(4U, Opcode::Div).s32().Inputs(0U, 3U);
            INST(5U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 10U);
        CONSTANT(1U, 0U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::ZERO_CHECK).Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, ZeroCheckTest2)
{
    // Check Elimination for ZeroCheck is applied. Positiv value.
    GRAPH(GetGraph())
    {
        CONSTANT(0U, -5L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(5U, Opcode::Abs).s32().Inputs(0U);
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NegativeCheck).s32().Inputs(5U, 1U);
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U, 1U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, -5L);
        BASIC_BLOCK(2U, 1U)
        {
            INST(5U, Opcode::Abs).s32().Inputs(0U);
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NOP);
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 5U, 1U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NegativeZeroCheckTest)
{
    // Test with NegativeCheck and ZeroCheck.
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::NegativeCheck).s32().Inputs(0U, 2U);
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(2U).TypeId(68U);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 3U, 2U);
            INST(5U, Opcode::ZeroCheck).s32().Inputs(0U, 2U);
            INST(6U, Opcode::Div).s32().Inputs(1U, 5U);
            INST(7U, Opcode::StoreArray).s32().Inputs(4U, 0U, 6U);
            INST(8U, Opcode::Return).ref().Inputs(4U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 2U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::NOP);
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(2U).TypeId(68U);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 2U);
            INST(5U, Opcode::NOP);
            INST(6U, Opcode::Div).s32().Inputs(1U, 0U);
            INST(7U, Opcode::StoreArray).s32().Inputs(4U, 0U, 6U);
            INST(8U, Opcode::Return).ref().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void ChecksEliminationTest::BuildGraphHoistZeroCheckTest()
{
    {
        GRAPH(GetGraph())
        {
            CONSTANT(0U, 0U);  // initial
            CONSTANT(1U, 1U);  // increment
            CONSTANT(2U, 10U);
            PARAMETER(3U, 0U).s32();
            BASIC_BLOCK(2U, 3U, 5U)
            {
                INST(20U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
                INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);  // 0 < 10
                INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
            }
            BASIC_BLOCK(3U, 3U, 5U)
            {
                INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);   // i
                INST(16U, Opcode::Phi).s32().Inputs(0U, 15U);  // s
                INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
                INST(8U, Opcode::ZeroCheck).s32().Inputs(3U, 7U);
                INST(17U, Opcode::Div).s32().Inputs(4U, 8U);
                INST(15U, Opcode::Add).s32().Inputs(16U, 17U);             // s += i / x
                INST(10U, Opcode::Add).s32().Inputs(4U, 1U);               // i++
                INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 2U);  // i < 10
                INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
            }
            BASIC_BLOCK(5U, 1U)
            {
                INST(18U, Opcode::Phi).s32().Inputs(0U, 16U);
                INST(12U, Opcode::Return).s32().Inputs(18U);
            }
        }
    }
}
TEST_F(ChecksEliminationTest, HoistZeroCheckTest)
{
    BuildGraphHoistZeroCheckTest();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(2U, 10U);
        PARAMETER(3U, 0U).s32();
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(20U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(22U, Opcode::ZeroCheck).s32().Inputs(3U, 20U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);  // 0 < 10
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);   // i
            INST(16U, Opcode::Phi).s32().Inputs(0U, 15U);  // s
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(17U, Opcode::Div).s32().Inputs(4U, 22U);
            INST(15U, Opcode::Add).s32().Inputs(16U, 17U);             // s += i / x
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);               // i++
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 2U);  // i < 10
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(18U, Opcode::Phi).s32().Inputs(0U, 16U);
            INST(12U, Opcode::Return).s32().Inputs(18U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, SimpleBoundsCheckTest)
{
    SimpleTest<true>(2U, 10U);
    SimpleTest<true>(9U, 10U);
    SimpleTest<true>(0U, 10U);
    SimpleTest<false>(10U, 10U);
    SimpleTest<false>(12U, 10U);
    SimpleTest<false>(-1L, 10U);
}

TEST_F(ChecksEliminationTest, SimpleBoundsCheckTest1)
{
    // not applied, we know nothing about len array
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();  // len array
        CONSTANT(1U, 5U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(2U).TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 0U, 2U);
            INST(4U, Opcode::BoundsCheck).s32().Inputs(0U, 1U, 2U);
            INST(5U, Opcode::LoadArray).s32().Inputs(3U, 4U);
            INST(6U, Opcode::Return).s32().Inputs(5U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(ChecksEliminationTest, SimpleBoundsCheckTest2)
{
    // not applied, we know nothing about index
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 10U);
        PARAMETER(1U, 0U).s32();  // index
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(2U).TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 0U, 2U);
            INST(4U, Opcode::BoundsCheck).s32().Inputs(0U, 1U, 2U);
            INST(5U, Opcode::LoadArray).s32().Inputs(3U, 4U);
            INST(6U, Opcode::Return).s32().Inputs(5U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(ChecksEliminationTest, ArithmeticTest)
{
    ArithmeticTest<AppliedType::REMOVED>(-1L, 20U, Opcode::Add, 1U);
    ArithmeticTest<AppliedType::REMOVED>(-1L, 20U, Opcode::Add, 5U);
    ArithmeticTest<AppliedType::REMOVED>(-1L, 20U, Opcode::Add, 20U);
    ArithmeticTest<AppliedType::REMOVED>(-100L, 20U, Opcode::Add, 115U);
    ArithmeticTest<AppliedType::REPLACED>(-1L, 20U, Opcode::Add, 0U);
    ArithmeticTest<AppliedType::REPLACED>(0U, 20U, Opcode::Add, 20U);
    ArithmeticTest<AppliedType::REPLACED>(-100L, 20U, Opcode::Add, 5U);
    ArithmeticTest<AppliedType::REPLACED>(-100L, 20U, Opcode::Add, 125U);
    // overflow
    ArithmeticTest<AppliedType::NOT_APPLIED>(INT32_MAX, INT32_MAX, Opcode::Add, 1U);
    ArithmeticTest<AppliedType::NOT_APPLIED>(INT32_MIN, INT32_MAX, Opcode::Add, -1L);
    ArithmeticTest<AppliedType::NOT_APPLIED>(1U, INT32_MAX, Opcode::Add, INT32_MAX);
    ArithmeticTest<AppliedType::NOT_APPLIED>(-1L, INT32_MAX, Opcode::Add, INT32_MIN);

    ArithmeticTest<AppliedType::REMOVED>(20U, 20U, Opcode::Sub, 1U);
    ArithmeticTest<AppliedType::REMOVED>(20U, 20U, Opcode::Sub, 5U);
    ArithmeticTest<AppliedType::REMOVED>(20U, 20U, Opcode::Sub, 19U);
    ArithmeticTest<AppliedType::REPLACED>(25U, 20U, Opcode::Sub, 3U);
    ArithmeticTest<AppliedType::REPLACED>(20U, 20U, Opcode::Sub, 60U);
    // overflow
    ArithmeticTest<AppliedType::NOT_APPLIED>(INT32_MIN, INT32_MAX, Opcode::Sub, 1U);
    ArithmeticTest<AppliedType::NOT_APPLIED>(INT32_MAX, INT32_MAX, Opcode::Sub, -1L);
    ArithmeticTest<AppliedType::NOT_APPLIED>(1U, INT32_MAX, Opcode::Sub, INT32_MIN);
    ArithmeticTest<AppliedType::REPLACED>(-1L, INT32_MAX, Opcode::Sub, INT32_MAX);

    ArithmeticTest<AppliedType::REMOVED>(1U, 20U, Opcode::Mod, 20U);
    ArithmeticTest<AppliedType::REMOVED>(101U, 20U, Opcode::Mod, 5U);
    ArithmeticTest<AppliedType::REMOVED>(205U, 20U, Opcode::Mod, 5U);
    ArithmeticTest<AppliedType::REMOVED>(16U, 5U, Opcode::Mod, 3U);
}

TEST_F(ChecksEliminationTest, ArithmeticTestMul)
{
    ArithmeticTest<AppliedType::REMOVED>(5U, 20U, Opcode::Mul, 2U);
    ArithmeticTest<AppliedType::REMOVED>(-5L, 20U, Opcode::Mul, -2L);
    ArithmeticTest<AppliedType::REMOVED>(3U, 20U, Opcode::Mul, 5U);
    ArithmeticTest<AppliedType::REPLACED>(5U, 20U, Opcode::Mul, -2L);
    ArithmeticTest<AppliedType::REPLACED>(-2L, 20U, Opcode::Mul, 5U);
    // Zero
    ArithmeticTest<AppliedType::REMOVED>(INT32_MAX, 20U, Opcode::Mul, 0U);
    ArithmeticTest<AppliedType::REMOVED>(0U, 20U, Opcode::Mul, INT32_MAX);
    ArithmeticTest<AppliedType::REMOVED>(INT32_MIN, 20U, Opcode::Mul, 0U);
    ArithmeticTest<AppliedType::REMOVED>(0U, 20U, Opcode::Mul, INT32_MIN);
    // overflow
    ArithmeticTest<AppliedType::NOT_APPLIED>(INT32_MAX, 20U, Opcode::Mul, 2U);
    ArithmeticTest<AppliedType::NOT_APPLIED>(INT32_MIN, 20U, Opcode::Mul, -2L);
    ArithmeticTest<AppliedType::NOT_APPLIED>(-2L, 20U, Opcode::Mul, INT32_MIN);
    ArithmeticTest<AppliedType::NOT_APPLIED>(2U, 20U, Opcode::Mul, INT32_MAX);
    ArithmeticTest<AppliedType::NOT_APPLIED>(INT32_MAX, 20U, Opcode::Mul, -2L);
    ArithmeticTest<AppliedType::NOT_APPLIED>(INT32_MIN, 20U, Opcode::Mul, 2U);
    ArithmeticTest<AppliedType::NOT_APPLIED>(2U, 20U, Opcode::Mul, INT32_MIN);
    ArithmeticTest<AppliedType::NOT_APPLIED>(-2L, 20U, Opcode::Mul, INT32_MAX);
}

TEST_F(ChecksEliminationTest, ModTest)
{
    ModTest<true>(10U, 10U);
    ModTest<true>(10U, 5U);
    ModTest<false>(10U, 11U);
    ModTest<false>(10U, 20U);
}

void ChecksEliminationTest::BuildGraphIfTestTrueBlock()
{
    // we can norrow bounds range for true block
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        CONSTANT(2U, 10U);  // len array
        CONSTANT(3U, 0U);
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_GE).Inputs(0U, 3U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(9U, Opcode::BoundsCheck).s32().Inputs(2U, 0U, 8U);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 9U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
}

TEST_F(ChecksEliminationTest, IfTestTrueBlock)
{
    BuildGraphIfTestTrueBlock();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        CONSTANT(2U, 10U);
        CONSTANT(3U, 0U);
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_GE).Inputs(0U, 3U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(9U, Opcode::NOP);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 0U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void ChecksEliminationTest::BuildGraphIfTestFalseBlock()
{
    // we can norrow bounds range for false block
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        CONSTANT(2U, 10U);  // len array
        CONSTANT(3U, 0U);
        BASIC_BLOCK(2U, 5U, 3U)
        {
            INST(4U, Opcode::Compare).b().CC(CC_GE).Inputs(0U, 2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 3U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(9U, Opcode::BoundsCheck).s32().Inputs(2U, 0U, 8U);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 9U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
}

TEST_F(ChecksEliminationTest, IfTestFalseBlock)
{
    BuildGraphIfTestFalseBlock();
    GetGraph()->RunPass<ChecksElimination>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        CONSTANT(2U, 10U);
        CONSTANT(3U, 0U);
        BASIC_BLOCK(2U, 5U, 3U)
        {
            INST(4U, Opcode::Compare).b().CC(CC_GE).Inputs(0U, 2U);  // index >= len array
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 3U);  // index < 0
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(9U, Opcode::NOP);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 0U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void ChecksEliminationTest::BuildGraphIfTestTrueBlock1()
{
    // we can norrow bounds range for true block
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        CONSTANT(3U, 0U);
        BASIC_BLOCK(6U, 3U, 5U)
        {
            INST(14U, Opcode::LenArray).s32().Inputs(1U);
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 14U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_GE).Inputs(0U, 3U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 3U});
            INST(9U, Opcode::BoundsCheck).s32().Inputs(14U, 0U, 8U);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 9U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
}

TEST_F(ChecksEliminationTest, IfTestTrueBlock1)
{
    BuildGraphIfTestTrueBlock1();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        CONSTANT(3U, 0U);
        BASIC_BLOCK(6U, 3U, 5U)
        {
            INST(14U, Opcode::LenArray).s32().Inputs(1U);
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 14U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_GE).Inputs(0U, 3U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 3U});
            INST(9U, Opcode::NOP);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 0U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void ChecksEliminationTest::BuildGraphIfTestTrueBlock2()
{
    // we can norrow bounds range for true block
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        CONSTANT(3U, 0U);
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(14U, Opcode::LenArray).s32().Inputs(1U);
            INST(4U, Opcode::Compare).b().CC(CC_GE).Inputs(0U, 3U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 14U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 3U});
            INST(9U, Opcode::BoundsCheck).s32().Inputs(14U, 0U, 8U);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 9U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
}

TEST_F(ChecksEliminationTest, IfTestTrueBlock2)
{
    BuildGraphIfTestTrueBlock2();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        CONSTANT(3U, 0U);
        BASIC_BLOCK(6U, 3U, 5U)
        {
            INST(14U, Opcode::LenArray).s32().Inputs(1U);
            INST(4U, Opcode::Compare).b().CC(CC_GE).Inputs(0U, 3U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 14U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 3U});
            INST(9U, Opcode::NOP);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 0U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void ChecksEliminationTest::BuildGraphIfTestTrueBlock3()
{
    // we can norrow bounds range for true block
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        CONSTANT(3U, 0U);
        BASIC_BLOCK(6U, 3U, 5U)
        {
            INST(14U, Opcode::LenArray).s32().Inputs(1U);
            INST(4U, Opcode::Compare).b().CC(CC_GT).Inputs(14U, 0U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_GE).Inputs(0U, 3U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 3U});
            INST(9U, Opcode::BoundsCheck).s32().Inputs(14U, 0U, 8U);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 9U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
}

TEST_F(ChecksEliminationTest, IfTestTrueBlock3)
{
    BuildGraphIfTestTrueBlock3();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        CONSTANT(3U, 0U);
        BASIC_BLOCK(6U, 3U, 5U)
        {
            INST(14U, Opcode::LenArray).s32().Inputs(1U);
            INST(4U, Opcode::Compare).b().CC(CC_GT).Inputs(14U, 0U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_GE).Inputs(0U, 3U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 3U});
            INST(9U, Opcode::NOP);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 0U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void ChecksEliminationTest::BuildGraphIfTestFalseBlock1()
{
    // we can norrow bounds range for false block
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        CONSTANT(3U, 0U);
        BASIC_BLOCK(2U, 5U, 3U)
        {
            INST(14U, Opcode::LenArray).s32().Inputs(1U);
            INST(4U, Opcode::Compare).b().CC(CC_GE).Inputs(0U, 14U);  // index >= len array
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 3U);  // index < 0
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 3U});
            INST(9U, Opcode::BoundsCheck).s32().Inputs(14U, 0U, 8U);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 9U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
}

TEST_F(ChecksEliminationTest, IfTestFalseBlock1)
{
    BuildGraphIfTestFalseBlock1();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        CONSTANT(3U, 0U);
        BASIC_BLOCK(2U, 5U, 3U)
        {
            INST(14U, Opcode::LenArray).s32().Inputs(1U);
            INST(4U, Opcode::Compare).b().CC(CC_GE).Inputs(0U, 14U);  // index >= len array
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 3U);  // index < 0
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 3U});
            INST(9U, Opcode::NOP);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 0U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void ChecksEliminationTest::BuildGraphIfTestFalseBlock2()
{
    // we can norrow bounds range for false block
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        CONSTANT(3U, 0U);
        BASIC_BLOCK(2U, 5U, 3U)
        {
            INST(14U, Opcode::LenArray).s32().Inputs(1U);
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 3U);  // index < 0
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_GE).Inputs(0U, 14U);  // index >= len array
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 3U});
            INST(9U, Opcode::BoundsCheck).s32().Inputs(14U, 0U, 8U);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 9U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
}

TEST_F(ChecksEliminationTest, IfTestFalseBlock2)
{
    BuildGraphIfTestFalseBlock2();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        CONSTANT(3U, 0U);
        BASIC_BLOCK(2U, 5U, 3U)
        {
            INST(14U, Opcode::LenArray).s32().Inputs(1U);
            INST(4U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 3U);  // index < 0
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_GE).Inputs(0U, 14U);  // index >= len array
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 3U});
            INST(9U, Opcode::NOP);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 0U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void ChecksEliminationTest::BuildGraphIfTestFalseBlock3()
{
    // we can norrow bounds range for false block
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        CONSTANT(3U, 0U);
        BASIC_BLOCK(2U, 5U, 3U)
        {
            INST(14U, Opcode::LenArray).s32().Inputs(1U);
            INST(4U, Opcode::Compare).b().CC(CC_LE).Inputs(14U, 0U);  // len array < index
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 3U);  // index < 0
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 3U});
            INST(9U, Opcode::BoundsCheck).s32().Inputs(14U, 0U, 8U);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 9U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
}

TEST_F(ChecksEliminationTest, IfTestFalseBlock3)
{
    BuildGraphIfTestFalseBlock3();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).ref();
        CONSTANT(3U, 0U);
        BASIC_BLOCK(2U, 5U, 3U)
        {
            INST(14U, Opcode::LenArray).s32().Inputs(1U);
            INST(4U, Opcode::Compare).b().CC(CC_LE).Inputs(14U, 0U);  // len array < index
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(6U, Opcode::Compare).b().CC(CC_LT).Inputs(0U, 3U);  // index < 0
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 3U});
            INST(9U, Opcode::NOP);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 0U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, IfTest1)
{
    // not applied if without compare
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        PARAMETER(13U, 2U).b();
        CONSTANT(2U, 10U);  // len array
        CONSTANT(3U, 0U);
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(9U, Opcode::BoundsCheck).s32().Inputs(2U, 0U, 8U);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 9U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(ChecksEliminationTest, IfTest2)
{
    // not applied, compare intputs not int32
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();  // index
        PARAMETER(1U, 1U).ref();
        PARAMETER(13U, 2U).s64();
        PARAMETER(14U, 3U).s64();
        CONSTANT(2U, 10U);  // len array
        CONSTANT(3U, 0U);
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(15U, Opcode::Compare).b().CC(CC_GE).Inputs(13U, 14U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(15U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(16U, Opcode::Compare).b().CC(CC_GE).Inputs(13U, 14U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(16U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(9U, Opcode::BoundsCheck).s32().Inputs(2U, 0U, 8U);
            INST(10U, Opcode::LoadArray).s32().Inputs(1U, 9U);
            INST(11U, Opcode::Return).s32().Inputs(10U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).s32().Inputs(3U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(ChecksEliminationTest, PhiTest)
{
    PhiTest<true>(5U, 10U, 10U);
    PhiTest<true>(9U, 10U, 5U);
    PhiTest<false>(10U, 10U, 10U);
    PhiTest<false>(-1L, 10U, 10U);
    PhiTest<false>(5U, 10U, 11U);
}

void ChecksEliminationTest::BuildGraphSimpleLoopTestInc()
{
    // new_array(len_array)
    // For(int i = 0, i < len_array, i++) begin
    //  boundscheck(len_array, i) - can remove
    //  a[i] = 0
    // end
    // return new_array
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);   // initial
        CONSTANT(1U, 1U);   // increment
        CONSTANT(2U, 10U);  // len_array
        BASIC_BLOCK(2U, 3U)
        {
            INST(43U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U, 43U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(5U, Opcode::Compare).CC(CC_LT).b().Inputs(4U, 2U);  // i < len_array
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::BoundsCheck).s32().Inputs(2U, 4U, 7U);
            INST(9U, Opcode::StoreArray).s32().Inputs(3U, 8U, 0U);  // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);            // i++
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
}

TEST_F(ChecksEliminationTest, SimpleLoopTestInc)
{
    BuildGraphSimpleLoopTestInc();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);   // initial
        CONSTANT(1U, 1U);   // increment
        CONSTANT(2U, 10U);  // len_array
        BASIC_BLOCK(2U, 3U)
        {
            INST(43U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U, 43U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(5U, Opcode::Compare).CC(CC_LT).b().Inputs(4U, 2U);  // i < len_array
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::NOP);
            INST(9U, Opcode::StoreArray).s32().Inputs(3U, 4U, 0U);  // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);            // i++
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void ChecksEliminationTest::BuildGraphSimpleLoopTestIncAfterPeeling()
{
    // new_array(len_array)
    // For(int i = 0, i < len_array, i++) begin
    //  boundscheck(len_array, i) - can remove
    //  a[i] = 0
    // end
    // return new_array
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);   // initial
        CONSTANT(1U, 1U);   // increment
        CONSTANT(2U, 10U);  // len_array
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(43U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U, 43U);
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);  // 0 < len_array
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::BoundsCheck).s32().Inputs(2U, 4U, 7U);
            INST(9U, Opcode::StoreArray).s32().Inputs(3U, 8U, 0U);     // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);               // i++
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 2U);  // i < len_array
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
}

TEST_F(ChecksEliminationTest, SimpleLoopTestIncAfterPeeling)
{
    BuildGraphSimpleLoopTestIncAfterPeeling();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);   // initial
        CONSTANT(1U, 1U);   // increment
        CONSTANT(2U, 10U);  // len_array
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(43U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U, 43U);
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);  // 0 < len_array
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::NOP);
            INST(9U, Opcode::StoreArray).s32().Inputs(3U, 4U, 0U);     // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);               // i++
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 2U);  // i < len_array
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void ChecksEliminationTest::BuildGraphSimpleLoopTestIncAfterPeeling1()
{
    // new_array(len_array)
    // For(int i = 0, i < len_array, i++) begin
    //  boundscheck(len_array, i) - can remove
    //  a[i] = 0
    // end
    // return new_array
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);         // initial
        CONSTANT(1U, 1U);         // increment
        PARAMETER(3U, 0U).ref();  // array
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(16U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 16U);  // 0 < len_array
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 3U});
            INST(8U, Opcode::BoundsCheck).s32().Inputs(16U, 4U, 7U);
            INST(9U, Opcode::StoreArray).s32().Inputs(3U, 8U, 0U);      // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);                // i++
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 16U);  // i < len_array
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
}

TEST_F(ChecksEliminationTest, SimpleLoopTestIncAfterPeeling1)
{
    BuildGraphSimpleLoopTestIncAfterPeeling1();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);         // initial
        CONSTANT(1U, 1U);         // increment
        PARAMETER(3U, 0U).ref();  // array
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(16U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 16U);  // 0 < len_array
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 3U).SrcVregs({0U, 1U, 3U});
            INST(8U, Opcode::NOP);
            INST(9U, Opcode::StoreArray).s32().Inputs(3U, 4U, 0U);      // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);                // i++
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 16U);  // i < len_array
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void ChecksEliminationTest::BuildGraphSimpleLoopTestDec()
{
    // new_array(len_array)
    // For(int i = len_array-1, i >= 0, i--) begin
    //  boundscheck(len_array, i) - can remove
    //  a[i] = 0
    // end
    // return new_array
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);   // increment
        CONSTANT(2U, 10U);  // initial and len_array
        BASIC_BLOCK(2U, 3U)
        {
            INST(43U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U, 43U);
            INST(13U, Opcode::Sub).s32().Inputs(2U, 1U);  // len_array - 1
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(13U, 10U);
            INST(5U, Opcode::Compare).CC(CC_GE).b().Inputs(4U, 0U);  // i >= 0
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::BoundsCheck).s32().Inputs(2U, 4U, 7U);
            INST(9U, Opcode::StoreArray).s32().Inputs(3U, 8U, 0U);  // a[i] = 0
            INST(10U, Opcode::Sub).s32().Inputs(4U, 1U);            // i--
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
}

TEST_F(ChecksEliminationTest, SimpleLoopTestDec)
{
    BuildGraphSimpleLoopTestDec();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);   // increment
        CONSTANT(2U, 10U);  // initial and len_array
        BASIC_BLOCK(2U, 3U)
        {
            INST(43U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U, 43U);
            INST(13U, Opcode::Sub).s32().Inputs(2U, 1U);  // len_array - 1
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(13U, 10U);
            INST(5U, Opcode::Compare).CC(CC_GE).b().Inputs(4U, 0U);  // i >= 0
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::NOP);
            INST(9U, Opcode::StoreArray).s32().Inputs(3U, 4U, 0U);  // a[i] = 0
            INST(10U, Opcode::Sub).s32().Inputs(4U, 1U);            // i--
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Return).ref().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void ChecksEliminationTest::BuildGraphLoopWithUnknowLowerUpperValue()
{
    // applied
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);         // increment
        PARAMETER(1U, 0U).s32();  // lower
        PARAMETER(2U, 1U).s32();  // upper
        PARAMETER(3U, 2U).ref();  // array
        BASIC_BLOCK(7U, 3U, 6U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::LenArray).s32().Inputs(5U);
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(7U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(1U, 2U);  // lower < upper
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(1U, 13U);
            INST(10U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(11U, Opcode::BoundsCheck).s32().Inputs(6U, 9U, 10U);
            INST(12U, Opcode::StoreArray).s32().Inputs(3U, 11U, 0U);                  // a[i] = 0
            INST(13U, Opcode::Add).s32().Inputs(9U, 0U);                              // i++
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(13U, 2U);  // i < upper
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(16U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(17U, Opcode::Return).s32().Inputs(16U);
        }
    }
}

TEST_F(ChecksEliminationTest, LoopWithUnknowLowerUpperValue)
{
    BuildGraphLoopWithUnknowLowerUpperValue();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 1U);         // increment
        PARAMETER(1U, 0U).s32();  // lower
        PARAMETER(2U, 1U).s32();  // upper
        PARAMETER(3U, 2U).ref();  // array
        CONSTANT(22U, 0U);
        BASIC_BLOCK(7U, 3U, 6U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::LenArray).s32().Inputs(5U);
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(20U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(1U, 22U);  // (lower) < 0
            INST(21U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(20U, 30U);
            INST(18U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(6U, 2U);  // len_array < (upper)
            INST(19U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(18U, 30U);
            INST(7U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(1U, 2U);  // lower < X
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(1U, 13U);
            INST(10U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(11U, Opcode::NOP);
            INST(12U, Opcode::StoreArray).s32().Inputs(3U, 9U, 0U);                   // a[i] = 0
            INST(13U, Opcode::Add).s32().Inputs(9U, 0U);                              // i++
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(13U, 2U);  // i < upper
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(16U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(17U, Opcode::Return).s32().Inputs(16U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

Graph *ChecksEliminationTest::BuildGraphLoopWithUnknowLowerUpperValueDown(ConditionCode cc)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);         // increment
        PARAMETER(1U, 0U).s32();  // lower
        PARAMETER(2U, 1U).s32();  // upper
        PARAMETER(3U, 2U).ref();  // array
        BASIC_BLOCK(7U, 3U, 6U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::LenArray).s32().Inputs(5U);
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(7U, Opcode::Compare).CC(cc).b().Inputs(2U, 1U);  // upper >(>=) lower
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(2U, 13U);
            INST(10U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(11U, Opcode::BoundsCheck).s32().Inputs(6U, 9U, 10U);
            INST(12U, Opcode::StoreArray).s32().Inputs(3U, 11U, 0U);  // a[i] = 0
            INST(13U, Opcode::Sub).s32().Inputs(9U, 0U);              // i--
            INST(14U, Opcode::Compare).CC(cc).b().Inputs(13U, 1U);    // i >(>=) lower
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(16U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(17U, Opcode::Return).s32().Inputs(16U);
        }
    }
    return graph;
}

TEST_F(ChecksEliminationTest, LoopWithUnknowLowerUpperValueDown)
{
    for (auto cc : {CC_GT, CC_GE}) {
        // applied
        auto *graph = BuildGraphLoopWithUnknowLowerUpperValueDown(cc);
        EXPECT_TRUE(graph->RunPass<ChecksElimination>());
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            CONSTANT(0U, 1U);         // increment
            PARAMETER(1U, 0U).s32();  // lower
            PARAMETER(2U, 1U).s32();  // upper
            PARAMETER(3U, 2U).ref();  // array
            CONSTANT(22U, cc == CC_GT ? static_cast<uint64_t>(-1L) : 0U);
            BASIC_BLOCK(7U, 3U, 6U)
            {
                INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
                INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
                INST(6U, Opcode::LenArray).s32().Inputs(5U);
                INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
                INST(20U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(1U, 22U);  // (lower) < -1 (0)
                INST(21U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(20U, 30U);
                INST(18U, Opcode::Compare).CC(ConditionCode::CC_LE).b().Inputs(6U, 2U);  // len_array <= (upper)
                INST(19U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(18U, 30U);
                INST(7U, Opcode::Compare).CC(cc).b().Inputs(2U, 1U);  // upper >(>=) lower
                INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
            }
            BASIC_BLOCK(3U, 3U, 6U)
            {
                INST(9U, Opcode::Phi).s32().Inputs(2U, 13U);
                INST(10U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
                INST(11U, Opcode::NOP);
                INST(12U, Opcode::StoreArray).s32().Inputs(3U, 9U, 0U);  // a[i] = 0
                INST(13U, Opcode::Sub).s32().Inputs(9U, 0U);             // i--
                INST(14U, Opcode::Compare).CC(cc).b().Inputs(13U, 1U);   // i >(>=) lower
                INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
            }
            BASIC_BLOCK(6U, 1U)
            {
                INST(16U, Opcode::Phi).s32().Inputs(0U, 13U);
                INST(17U, Opcode::Return).s32().Inputs(16U);
            }
        }
        EXPECT_TRUE(GraphComparator().Compare(graph, graph1));
    }
}

void ChecksEliminationTest::BuildGraphUpperOOB()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // array
        CONSTANT(3U, 0U);
        CONSTANT(25U, 1U);
        BASIC_BLOCK(4U, 3U, 2U)
        {
            INST(2U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(27U, Opcode::SaveState).Inputs(3U, 0U).SrcVregs({0U, 3U});
            INST(28U, Opcode::NullCheck).ref().Inputs(0U, 27U);
            INST(29U, Opcode::LenArray).s32().Inputs(28U);
            INST(30U, Opcode::Compare).Inputs(29U, 3U).CC(CC_LT).b();
            INST(31U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(30U);
        }
        BASIC_BLOCK(2U, 3U, 2U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(3U, 24U);
            INST(15U, Opcode::SaveState).Inputs(4U, 0U, 4U).SrcVregs({0U, 3U, 4U});
            INST(16U, Opcode::NullCheck).ref().Inputs(0U, 15U);
            INST(18U, Opcode::BoundsCheck).s32().Inputs(29U, 4U, 15U);
            INST(19U, Opcode::StoreArray).s32().Inputs(16U, 18U, 3U);
            INST(24U, Opcode::Add).s32().Inputs(4U, 25U);
            INST(10U, Opcode::Compare).b().CC(CC_LT).Inputs(29U, 24U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(26U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(ChecksEliminationTest, UpperOOB)
{
    BuildGraphUpperOOB();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).ref();  // array
        CONSTANT(3U, 0U);
        CONSTANT(25U, 1U);
        CONSTANT(35U, 0x7ffffffeU);
        BASIC_BLOCK(4U, 3U, 2U)
        {
            INST(2U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(27U, Opcode::SaveState).Inputs(3U, 0U).SrcVregs({0U, 3U});
            INST(28U, Opcode::NullCheck).ref().Inputs(0U, 27U);
            INST(29U, Opcode::LenArray).s32().Inputs(28U);
            INST(33U, Opcode::Compare).Inputs(35U, 29U).CC(CC_LT).b();
            INST(34U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(33U, 27U);
            INST(36U, Opcode::Compare).Inputs(29U, 29U).CC(CC_LE).b();
            INST(37U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(36U, 27U);
            INST(30U, Opcode::Compare).Inputs(29U, 3U).CC(CC_LT).b();
            INST(31U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(30U);
        }
        BASIC_BLOCK(2U, 3U, 2U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(3U, 24U);
            INST(15U, Opcode::SaveState).Inputs(4U, 0U, 4U).SrcVregs({0U, 3U, 4U});
            INST(16U, Opcode::NOP);
            INST(18U, Opcode::NOP);
            INST(19U, Opcode::StoreArray).s32().Inputs(28U, 4U, 3U);
            INST(24U, Opcode::Add).s32().Inputs(4U, 25U);
            INST(10U, Opcode::Compare).b().CC(CC_LT).Inputs(29U, 24U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(3U, 1U)
        {
            INST(26U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

void ChecksEliminationTest::BuildGraphUpperWithDec()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // array
        CONSTANT(3U, 0U);         // lower
        CONSTANT(25U, 1U);        // increment
        BASIC_BLOCK(4U, 3U, 2U)
        {
            INST(2U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(27U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(28U, Opcode::NullCheck).ref().Inputs(0U, 27U);
            INST(29U, Opcode::LenArray).s32().Inputs(28U);
            INST(30U, Opcode::Sub).s32().Inputs(29U, 25U);             // upper = len_array - 1
            INST(31U, Opcode::Compare).Inputs(30U, 3U).CC(CC_LE).b();  // lower < upper
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(31U);
        }
        BASIC_BLOCK(2U, 3U, 2U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(3U, 5U);
            INST(5U, Opcode::Add).s32().Inputs(4U, 25U);
            INST(15U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(16U, Opcode::NullCheck).ref().Inputs(0U, 15U);
            INST(18U, Opcode::BoundsCheck).s32().Inputs(29U, 5U, 15U);
            INST(19U, Opcode::LoadArray).s32().Inputs(16U, 18U);
            INST(20U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(21U, Opcode::NullCheck).ref().Inputs(0U, 20U);
            INST(22U, Opcode::BoundsCheck).s32().Inputs(29U, 4U, 20U);
            INST(23U, Opcode::StoreArray).s32().Inputs(21U, 22U, 19U);  // a[i] = a[i + 1]
            INST(10U, Opcode::Compare).b().CC(CC_LE).Inputs(30U, 5U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(26U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(ChecksEliminationTest, UpperWithDec)
{
    BuildGraphUpperWithDec();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).ref();  // array
        CONSTANT(3U, 0U);         // lower
        CONSTANT(25U, 1U);        // increment
        BASIC_BLOCK(4U, 3U, 2U)
        {
            INST(2U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(27U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(28U, Opcode::NullCheck).ref().Inputs(0U, 27U);
            INST(29U, Opcode::LenArray).s32().Inputs(28U);
            INST(30U, Opcode::Sub).s32().Inputs(29U, 25U);             // upper = len_array - 1
            INST(31U, Opcode::Compare).Inputs(30U, 3U).CC(CC_LE).b();  // lower < upper
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(31U);
        }
        BASIC_BLOCK(2U, 3U, 2U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(3U, 5U);
            INST(5U, Opcode::Add).s32().Inputs(4U, 25U);
            INST(15U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(16U, Opcode::NOP);
            INST(18U, Opcode::NOP);
            INST(19U, Opcode::LoadArray).s32().Inputs(28U, 5U);
            INST(20U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(21U, Opcode::NOP);
            INST(22U, Opcode::NOP);
            INST(23U, Opcode::StoreArray).s32().Inputs(28U, 4U, 19U);  // a[i] = a[i + 1]
            INST(10U, Opcode::Compare).b().CC(CC_LE).Inputs(30U, 5U);  // i < upper
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(26U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

void ChecksEliminationTest::BuildGraphUnknownUpperWithDec()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // array
        PARAMETER(1U, 1U).s32();  // X
        CONSTANT(3U, 0U);         // lower
        CONSTANT(6U, 3U);
        CONSTANT(25U, 1U);  // increment
        BASIC_BLOCK(4U, 3U, 2U)
        {
            INST(2U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(27U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(28U, Opcode::NullCheck).ref().Inputs(0U, 27U);
            INST(29U, Opcode::LenArray).s32().Inputs(28U);
            INST(30U, Opcode::Sub).s32().Inputs(1U, 6U);               // upper = X - 3
            INST(31U, Opcode::Compare).Inputs(30U, 3U).CC(CC_LE).b();  // lower < upper
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(31U);
        }
        BASIC_BLOCK(2U, 3U, 2U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(3U, 5U);
            INST(5U, Opcode::Add).s32().Inputs(4U, 25U);
            INST(15U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(16U, Opcode::NullCheck).ref().Inputs(0U, 15U);
            INST(18U, Opcode::BoundsCheck).s32().Inputs(29U, 5U, 15U);
            INST(19U, Opcode::LoadArray).s32().Inputs(16U, 18U);
            INST(20U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(21U, Opcode::NullCheck).ref().Inputs(0U, 20U);
            INST(22U, Opcode::BoundsCheck).s32().Inputs(29U, 4U, 20U);
            INST(23U, Opcode::StoreArray).s32().Inputs(21U, 22U, 19U);  // a[i] = a[i + 1]
            INST(10U, Opcode::Compare).b().CC(CC_LE).Inputs(30U, 5U);   // i + 1 < upper
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(26U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(ChecksEliminationTest, UnknownUpperWithDec)
{
    BuildGraphUnknownUpperWithDec();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).ref();  // array
        PARAMETER(1U, 1U).s32();  // X
        CONSTANT(3U, 0U);         // lower
        CONSTANT(6U, 3U);
        CONSTANT(25U, 1U);  // increment
        BASIC_BLOCK(4U, 3U, 2U)
        {
            INST(2U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(27U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(37U, Opcode::NullCheck).ref().Inputs(0U, 27U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(35U, Opcode::LenArray).s32().Inputs(37U);
            INST(39U, Opcode::NOP);
            INST(29U, Opcode::LenArray).s32().Inputs(37U);
            INST(30U, Opcode::Sub).s32().Inputs(1U, 6U);  // upper = X - 3
            INST(41U, Opcode::Sub).s32().Inputs(35U, 25U);
            // Deoptimize if len_array - 1 < X - 3
            INST(42U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(41U, 30U);
            INST(43U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(42U, 27U);
            INST(31U, Opcode::Compare).Inputs(30U, 3U).CC(CC_LE).b();  // lower < upper
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(31U);
        }
        BASIC_BLOCK(2U, 3U, 2U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(3U, 5U);
            INST(5U, Opcode::Add).s32().Inputs(4U, 25U);
            INST(15U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(16U, Opcode::NOP);
            INST(18U, Opcode::NOP);
            INST(19U, Opcode::LoadArray).s32().Inputs(37U, 5U);
            INST(20U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(21U, Opcode::NOP);
            INST(22U, Opcode::NOP);
            INST(23U, Opcode::StoreArray).s32().Inputs(37U, 4U, 19U);  // a[i] = a[i + 1]
            INST(10U, Opcode::Compare).b().CC(CC_LE).Inputs(30U, 5U);  // i + 1 < upper
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(26U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, LoopWithoutPreHeaderCompare)
{
    // not applied
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);         // increment
        PARAMETER(1U, 0U).s32();  // lower
        PARAMETER(2U, 1U).s32();  // upper
        PARAMETER(3U, 2U).ref();  // array
        BASIC_BLOCK(7U, 3U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::LenArray).s32().Inputs(5U);
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(1U, 13U);
            INST(10U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(11U, Opcode::BoundsCheck).s32().Inputs(6U, 9U, 10U);
            INST(12U, Opcode::StoreArray).s32().Inputs(3U, 11U, 0U);  // a[i] = 0
        }
        BASIC_BLOCK(4U, 3U, 6U)
        {
            INST(13U, Opcode::Add).s32().Inputs(9U, 0U);                              // i++
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(13U, 2U);  // i < upper
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(17U, Opcode::Return).s32().Inputs(13U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

void ChecksEliminationTest::BuildGraphLoopWithUnknowLowerValue()
{
    // applied
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);         // increment
        PARAMETER(1U, 0U).s32();  // lower
        PARAMETER(3U, 2U).ref();  // array
        BASIC_BLOCK(7U, 3U, 6U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::LenArray).s32().Inputs(5U);
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(7U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(1U, 6U);  // lower < len_array
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(1U, 13U);
            INST(10U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(11U, Opcode::BoundsCheck).s32().Inputs(6U, 9U, 10U);
            INST(12U, Opcode::StoreArray).s32().Inputs(3U, 11U, 0U);                  // a[i] = 0
            INST(13U, Opcode::Add).s32().Inputs(9U, 0U);                              // i++
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(13U, 6U);  // i < len_array
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(16U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(17U, Opcode::Return).s32().Inputs(16U);
        }
    }
}

TEST_F(ChecksEliminationTest, LoopWithUnknowLowerValue)
{
    BuildGraphLoopWithUnknowLowerValue();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 1U);         // increment
        PARAMETER(1U, 0U).s32();  // lower
        PARAMETER(3U, 2U).ref();  // array
        CONSTANT(22U, 0U);
        BASIC_BLOCK(7U, 3U, 6U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::LenArray).s32().Inputs(5U);
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(20U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(1U, 22U);  // lower < 0
            INST(21U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(20U, 30U);
            INST(7U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(1U, 6U);  // lower < len_aray
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(1U, 13U);
            INST(10U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(11U, Opcode::NOP);
            INST(12U, Opcode::StoreArray).s32().Inputs(3U, 9U, 0U);                   // a[i] = 0
            INST(13U, Opcode::Add).s32().Inputs(9U, 0U);                              // i++
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(13U, 6U);  // i < upper
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(16U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(17U, Opcode::Return).s32().Inputs(16U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

void ChecksEliminationTest::BuildGraphLoopWithUnknowUpperValueLE()
{
    // applied
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);         // initial
        CONSTANT(1U, 1U);         // increment
        PARAMETER(2U, 0U).s32();  // X
        PARAMETER(3U, 1U).ref();  // array
        BASIC_BLOCK(7U, 3U, 6U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::LenArray).s32().Inputs(5U);
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(7U, Opcode::Compare).CC(ConditionCode::CC_LE).b().Inputs(0U, 2U);  // i <= X
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(10U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(11U, Opcode::BoundsCheck).s32().Inputs(6U, 9U, 10U);
            INST(12U, Opcode::StoreArray).s32().Inputs(3U, 11U, 0U);                  // a[i] = 0
            INST(13U, Opcode::Add).s32().Inputs(9U, 1U);                              // i++
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LE).b().Inputs(13U, 2U);  // i <= X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(16U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(17U, Opcode::Return).s32().Inputs(16U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
}

TEST_F(ChecksEliminationTest, LoopWithUnknowUpperValueLE)
{
    BuildGraphLoopWithUnknowUpperValueLE();
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 0U);            // initial
        CONSTANT(1U, 1U);            // increment
        PARAMETER(2U, 0U).s32();     // X
        PARAMETER(3U, 1U).ref();     // array
        CONSTANT(31U, 0x7ffffffeU);  // max
        BASIC_BLOCK(7U, 3U, 6U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::LenArray).s32().Inputs(5U);
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(18U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(31U, 2U);  // INT_MAX - 1 < X - infinite loop
            INST(19U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(18U, 30U);
            INST(32U, Opcode::Compare).CC(ConditionCode::CC_LE).b().Inputs(6U, 2U);  // len_array < X
            INST(33U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(32U, 30U);
            INST(7U, Opcode::Compare).CC(ConditionCode::CC_LE).b().Inputs(0U, 2U);  // i <= X
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(10U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(11U, Opcode::NOP);
            INST(12U, Opcode::StoreArray).s32().Inputs(3U, 9U, 0U);                   // a[i] = 0
            INST(13U, Opcode::Add).s32().Inputs(9U, 1U);                              // i++
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LE).b().Inputs(13U, 2U);  // i <= X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(16U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(17U, Opcode::Return).s32().Inputs(16U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

void ChecksEliminationTest::BuildGraphLoopWithUnknowUpperValueLT()
{
    // applied
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);         // initial
        CONSTANT(1U, 1U);         // increment
        PARAMETER(2U, 0U).s32();  // X
        PARAMETER(3U, 1U).ref();  // array
        BASIC_BLOCK(7U, 3U, 6U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::LenArray).s32().Inputs(5U);
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(7U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0U, 2U);  // i < X
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(10U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(11U, Opcode::BoundsCheck).s32().Inputs(6U, 9U, 10U);
            INST(12U, Opcode::StoreArray).s32().Inputs(3U, 11U, 0U);                  // a[i] = 0
            INST(13U, Opcode::Add).s32().Inputs(9U, 1U);                              // i++
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(13U, 2U);  // i < X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(16U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(17U, Opcode::Return).s32().Inputs(16U);
        }
    }
}

TEST_F(ChecksEliminationTest, LoopWithUnknowUpperValueLT)
{
    BuildGraphLoopWithUnknowUpperValueLT();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());

    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 0U);         // initial
        CONSTANT(1U, 1U);         // increment
        PARAMETER(2U, 0U).s32();  // X
        PARAMETER(3U, 1U).ref();  // array
        BASIC_BLOCK(7U, 3U, 6U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::LenArray).s32().Inputs(5U);
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(18U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(6U, 2U);
            INST(19U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(18U, 30U);
            INST(7U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0U, 2U);  // i < X
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(10U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(11U, Opcode::NOP);
            INST(12U, Opcode::StoreArray).s32().Inputs(3U, 9U, 0U);                   // a[i] = 0
            INST(13U, Opcode::Add).s32().Inputs(9U, 1U);                              // i++
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(13U, 2U);  // i < X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(16U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(17U, Opcode::Return).s32().Inputs(16U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

void ChecksEliminationTest::BuildGraphLoopSeveralBoundsChecks()
{
    // applied
    // array coping
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);         // initial
        CONSTANT(1U, 1U);         // increment
        PARAMETER(2U, 0U).ref();  // array1
        PARAMETER(3U, 1U).ref();  // array2
        PARAMETER(4U, 2U).s32();  // X

        BASIC_BLOCK(7U, 3U, 6U)
        {
            // check array 1
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(6U, Opcode::NullCheck).ref().Inputs(2U, 5U);
            INST(7U, Opcode::LenArray).s32().Inputs(6U);
            // check array 2
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(9U, Opcode::NullCheck).ref().Inputs(3U, 8U);
            INST(10U, Opcode::LenArray).s32().Inputs(9U);

            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});

            INST(11U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0U, 4U);  // 0 < X
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(13U, Opcode::Phi).s32().Inputs(0U, 20U);  // i
            INST(14U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(15U, Opcode::BoundsCheck).s32().Inputs(7U, 13U, 14U);
            INST(16U, Opcode::LoadArray).s32().Inputs(2U, 15U);
            INST(17U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(18U, Opcode::BoundsCheck).s32().Inputs(10U, 13U, 17U);
            INST(19U, Opcode::StoreArray).s32().Inputs(3U, 18U, 16U);                 // array2[i] = array1[i]
            INST(20U, Opcode::Add).s32().Inputs(13U, 1U);                             // i++
            INST(21U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(20U, 4U);  // i < X
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(21U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(23U, Opcode::Phi).s32().Inputs(0U, 20U);
            INST(24U, Opcode::Return).s32().Inputs(23U);
        }
    }
}

TEST_F(ChecksEliminationTest, LoopSeveralBoundsChecks)
{
    BuildGraphLoopSeveralBoundsChecks();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 0U);         // initial
        CONSTANT(1U, 1U);         // increment
        PARAMETER(2U, 0U).ref();  // array1
        PARAMETER(3U, 1U).ref();  // array2
        PARAMETER(4U, 2U).s32();  // X

        BASIC_BLOCK(7U, 3U, 6U)
        {
            // check array 1
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(6U, Opcode::NullCheck).ref().Inputs(2U, 5U);
            INST(7U, Opcode::LenArray).s32().Inputs(6U);
            // check array 2
            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(9U, Opcode::NullCheck).ref().Inputs(3U, 8U);
            INST(10U, Opcode::LenArray).s32().Inputs(9U);

            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(27U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(7U, 4U);  // len_array1 < X
            INST(28U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(27U, 30U);
            INST(25U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(10U, 4U);  // len_array2 < X
            INST(26U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(25U, 30U);

            INST(11U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(0U, 4U);  // 0 < X
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(13U, Opcode::Phi).s32().Inputs(0U, 20U);  // i
            INST(14U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(15U, Opcode::NOP);
            INST(16U, Opcode::LoadArray).s32().Inputs(2U, 13U);
            INST(17U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(18U, Opcode::NOP);
            INST(19U, Opcode::StoreArray).s32().Inputs(3U, 13U, 16U);                 // array2[i] = array1[i]
            INST(20U, Opcode::Add).s32().Inputs(13U, 1U);                             // i++
            INST(21U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(20U, 4U);  // i < X
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(21U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(23U, Opcode::Phi).s32().Inputs(0U, 20U);
            INST(24U, Opcode::Return).s32().Inputs(23U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

void ChecksEliminationTest::BuildGraphLoopSeveralIndexesBoundsChecks()
{
    // applied
    // array coping
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);         // initial
        CONSTANT(1U, 1U);         // increment
        PARAMETER(2U, 0U).ref();  // array
        PARAMETER(3U, 1U).s32();  // Y
        PARAMETER(4U, 2U).s32();  // X

        BASIC_BLOCK(7U, 3U, 6U)
        {
            // check array
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::NullCheck).ref().Inputs(2U, 5U);
            INST(7U, Opcode::LenArray).s32().Inputs(6U);

            INST(36U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});

            INST(11U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(3U, 4U);  // Y < X
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(13U, Opcode::Phi).s32().Inputs(3U, 20U);  // i

            INST(14U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(15U, Opcode::BoundsCheck).s32().Inputs(7U, 13U, 14U);
            INST(16U, Opcode::LoadArray).s32().Inputs(2U, 15U);  // array[i]

            INST(26U, Opcode::Sub).s32().Inputs(13U, 1U);  // i - 1
            INST(27U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(28U, Opcode::BoundsCheck).s32().Inputs(7U, 26U, 27U);
            INST(29U, Opcode::LoadArray).s32().Inputs(2U, 28U);  // array[i-1]

            INST(30U, Opcode::Add).s32().Inputs(13U, 1U);  // i + 1
            INST(31U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(32U, Opcode::BoundsCheck).s32().Inputs(7U, 30U, 31U);
            INST(33U, Opcode::LoadArray).s32().Inputs(2U, 32U);  // array[i+1]

            INST(34U, Opcode::Add).s32().Inputs(16U, 29U);  // array[i-1] + array[i]
            INST(35U, Opcode::Add).s32().Inputs(34U, 33U);  // array[i-1] + array[i] + array[i+1]

            INST(17U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(18U, Opcode::BoundsCheck).s32().Inputs(7U, 13U, 17U);
            INST(19U, Opcode::StoreArray).s32().Inputs(2U, 18U, 35U);  // array[i] = array[i-1] + array[i] + array[i+1]

            INST(20U, Opcode::Add).s32().Inputs(13U, 1U);                             // i++
            INST(21U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(20U, 4U);  // i < X
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(21U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(23U, Opcode::Phi).s32().Inputs(0U, 20U);
            INST(24U, Opcode::Return).s32().Inputs(23U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
TEST_F(ChecksEliminationTest, LoopSeveralIndexesBoundsChecks)
{
    BuildGraphLoopSeveralIndexesBoundsChecks();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 0U);         // initial
        CONSTANT(1U, 1U);         // increment
        PARAMETER(2U, 0U).ref();  // array
        PARAMETER(3U, 1U).s32();  // Y
        PARAMETER(4U, 2U).s32();  // X
        CONSTANT(43U, -1L);
        BASIC_BLOCK(7U, 3U, 6U)
        {
            // check array
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::NullCheck).ref().Inputs(2U, 5U);
            INST(7U, Opcode::LenArray).s32().Inputs(6U);

            INST(36U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(37U, Opcode::Add).s32().Inputs(3U, 43U);
            INST(38U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(37U, 0U);  // Y-1 < 0
            INST(39U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(38U, 36U);

            INST(40U, Opcode::Sub).s32().Inputs(7U, 1U);
            INST(41U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(40U, 4U);  // len_array - 1 < X
            INST(42U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(41U, 36U);

            INST(11U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(3U, 4U);  // Y < X
            INST(12U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(11U);
        }
        BASIC_BLOCK(3U, 3U, 6U)
        {
            INST(13U, Opcode::Phi).s32().Inputs(3U, 20U);  // i

            INST(14U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(15U, Opcode::NOP);
            INST(16U, Opcode::LoadArray).s32().Inputs(2U, 13U);  // array[i]

            INST(26U, Opcode::Sub).s32().Inputs(13U, 1U);  // i - 1
            INST(27U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(28U, Opcode::NOP);
            INST(29U, Opcode::LoadArray).s32().Inputs(2U, 26U);  // array[i-1]

            INST(30U, Opcode::Add).s32().Inputs(13U, 1U);  // i + 1
            INST(31U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(32U, Opcode::NOP);
            INST(33U, Opcode::LoadArray).s32().Inputs(2U, 30U);  // array[i+1]

            INST(34U, Opcode::Add).s32().Inputs(16U, 29U);  // array[i-1] + array[i]
            INST(35U, Opcode::Add).s32().Inputs(34U, 33U);  // array[i-1] + array[i] + array[i+1]

            INST(17U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(18U, Opcode::NOP);
            INST(19U, Opcode::StoreArray).s32().Inputs(2U, 13U, 35U);  // array[i] = array[i-1] + array[i] + array[i+1]

            INST(20U, Opcode::Add).s32().Inputs(13U, 1U);                             // i++
            INST(21U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(20U, 4U);  // i < X
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(21U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(23U, Opcode::Phi).s32().Inputs(0U, 20U);
            INST(24U, Opcode::Return).s32().Inputs(23U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

void ChecksEliminationTest::BuildGraphHeadExitLoop()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        PARAMETER(2U, 0U).ref();
        PARAMETER(3U, 1U).s32();  // X
        BASIC_BLOCK(2U, 3U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(5U, Opcode::NullCheck).ref().Inputs(2U, 4U);
            INST(6U, Opcode::LenArray).s32().Inputs(5U);
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(7U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(8U, Opcode::Compare).CC(CC_LT).b().Inputs(7U, 3U);  // i < X
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(10U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(11U, Opcode::BoundsCheck).s32().Inputs(6U, 7U, 10U);
            INST(12U, Opcode::StoreArray).s32().Inputs(5U, 11U, 0U);  // a[i] = 0
            INST(13U, Opcode::Add).s32().Inputs(7U, 1U);              // i++
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(14U, Opcode::Return).ref().Inputs(5U);
        }
    }
}

TEST_F(ChecksEliminationTest, HeadExitLoop)
{
    BuildGraphHeadExitLoop();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        PARAMETER(2U, 0U).ref();
        PARAMETER(3U, 1U).s32();
        BASIC_BLOCK(2U, 3U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(5U, Opcode::NullCheck).ref().Inputs(2U, 4U);
            INST(6U, Opcode::LenArray).s32().Inputs(5U);
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(15U, Opcode::Compare).b().CC(ConditionCode::CC_LT).Inputs(6U, 3U);  // len_array < X
            INST(16U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(15U, 30U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(7U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(8U, Opcode::Compare).CC(CC_LT).b().Inputs(7U, 3U);  // i < X
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(10U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(11U, Opcode::NOP);
            INST(12U, Opcode::StoreArray).s32().Inputs(5U, 7U, 0U);  // a[i] = 0
            INST(13U, Opcode::Add).s32().Inputs(7U, 1U);             // i++
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(14U, Opcode::Return).ref().Inputs(5U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, BoundsCheckInHeader)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        PARAMETER(2U, 0U).ref();
        PARAMETER(3U, 1U).s32();
        BASIC_BLOCK(2U, 3U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(5U, Opcode::NullCheck).ref().Inputs(2U, 4U);
            INST(6U, Opcode::LenArray).s32().Inputs(5U);
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(7U, Opcode::Phi).s32().Inputs(0U, 13U);
            INST(10U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(11U, Opcode::BoundsCheck).s32().Inputs(6U, 7U, 10U);
            INST(12U, Opcode::StoreArray).s32().Inputs(5U, 11U, 0U);  // a[i] = 0
            INST(8U, Opcode::Compare).CC(CC_LT).b().Inputs(7U, 3U);   // i < X
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(13U, Opcode::Add).s32().Inputs(7U, 1U);  // i++
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(14U, Opcode::Return).ref().Inputs(5U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

void ChecksEliminationTest::BuildGraphLoopTest1()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);          // initial
        CONSTANT(1U, 1U);          // increment
        PARAMETER(13U, 0U).ref();  // Array
        PARAMETER(27U, 1U).s32();  // X
        BASIC_BLOCK(2U, 6U, 3U)
        {
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_GE).b().Inputs(0U, 27U);  // i < X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(3U, 6U, 3U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 13U, 27U).SrcVregs({0U, 1U, 2U, 3U});
            INST(16U, Opcode::NullCheck).ref().Inputs(13U, 7U);
            INST(17U, Opcode::LenArray).s32().Inputs(16U);
            INST(8U, Opcode::BoundsCheck).s32().Inputs(17U, 4U, 7U);
            INST(9U, Opcode::StoreArray).s32().Inputs(16U, 8U, 0U);                   // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);                              // i++
            INST(5U, Opcode::Compare).CC(ConditionCode::CC_GE).b().Inputs(10U, 27U);  // i < X
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(26U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(12U, Opcode::Return).s32().Inputs(26U);
        }
    }
}

TEST_F(ChecksEliminationTest, LoopTest1)
{
    BuildGraphLoopTest1();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        PARAMETER(3U, 0U).ref();
        PARAMETER(2U, 1U).s32();
        BASIC_BLOCK(2U, 5U, 3U)
        {
            INST(20U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(33U, Opcode::NullCheck).ref().Inputs(3U, 20U).SetFlag(inst_flags::CAN_DEOPTIMIZE);  // array != nullptr
            INST(22U, Opcode::LenArray).s32().Inputs(33U);
            INST(23U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(22U, 2U);  // len_array < X
            INST(24U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(23U, 20U);
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_GE).b().Inputs(0U, 2U);  // 0 < X
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 5U, 3U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 3U, 2U).SrcVregs({0U, 1U, 2U, 3U});
            INST(15U, Opcode::NOP);
            INST(16U, Opcode::LenArray).s32().Inputs(33U);
            INST(8U, Opcode::NOP);
            INST(9U, Opcode::StoreArray).s32().Inputs(33U, 4U, 0U);    // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);               // i++
            INST(13U, Opcode::Compare).CC(CC_GE).b().Inputs(10U, 2U);  // i < X
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(26U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(12U, Opcode::Return).s32().Inputs(26U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

void ChecksEliminationTest::BuildGraphBatchLoopTest()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);
        CONSTANT(3U, 3U);         // increment
        PARAMETER(4U, 0U).ref();  // Array
        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 4U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(6U, Opcode::NullCheck).ref().Inputs(4U, 5U);
            INST(7U, Opcode::LenArray).s32().Inputs(6U);
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({0U});
            INST(25U, Opcode::Compare).CC(CC_GE).b().Inputs(0U, 7U);
            INST(26U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(25U);
        }
        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(8U, Opcode::Phi).s32().Inputs(0U, 20U);
            INST(9U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 4U, 7U).SrcVregs({0U, 1U, 2U, 3U, 4U, 7U});
            INST(10U, Opcode::NullCheck).ref().Inputs(4U, 9U);

            INST(12U, Opcode::Add).s32().Inputs(8U, 1U);
            INST(13U, Opcode::BoundsCheck).s32().Inputs(7U, 12U, 9U);
            INST(14U, Opcode::LoadArray).s32().Inputs(10U, 13U);

            INST(15U, Opcode::BoundsCheck).s32().Inputs(7U, 8U, 9U);
            INST(16U, Opcode::StoreArray).s32().Inputs(10U, 15U, 14U);  // a[i] = a[i + 1]

            INST(17U, Opcode::Add).s32().Inputs(8U, 2U);
            INST(18U, Opcode::BoundsCheck).s32().Inputs(7U, 17U, 9U);
            INST(19U, Opcode::StoreArray).s32().Inputs(10U, 18U, 14U);  // a[i + 2] = a[i + 1]

            INST(20U, Opcode::Add).s32().Inputs(8U, 3U);               // i += 3
            INST(21U, Opcode::Compare).CC(CC_GE).b().Inputs(20U, 7U);  // i < X
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(21U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(23U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(ChecksEliminationTest, BatchLoopTest)
{
    BuildGraphBatchLoopTest();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);
        CONSTANT(3U, 3U);         // increment
        PARAMETER(4U, 0U).ref();  // Array
        CONSTANT(36U, 0x7ffffffdU);
        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 4U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(6U, Opcode::NullCheck).ref().Inputs(4U, 5U);
            INST(7U, Opcode::LenArray).s32().Inputs(6U);
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({0U});
            INST(31U, Opcode::Compare).CC(CC_LT).b().Inputs(36U, 7U);
            INST(32U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(31U, 30U);
            INST(33U, Opcode::Mod).s32().Inputs(7U, 3U);
            INST(34U, Opcode::Compare).CC(CC_NE).b().Inputs(33U, 0U);
            INST(35U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(34U, 30U);
            INST(25U, Opcode::Compare).CC(CC_GE).b().Inputs(0U, 7U);
            INST(26U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(25U);
        }
        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(8U, Opcode::Phi).s32().Inputs(0U, 20U);
            INST(9U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 4U, 7U).SrcVregs({0U, 1U, 2U, 3U, 4U, 7U});
            INST(10U, Opcode::NOP);

            INST(12U, Opcode::Add).s32().Inputs(8U, 1U);
            INST(13U, Opcode::NOP);
            INST(14U, Opcode::LoadArray).s32().Inputs(6U, 12U);

            INST(15U, Opcode::NOP);
            INST(16U, Opcode::StoreArray).s32().Inputs(6U, 8U, 14U);  // a[i] = a[i + 1]

            INST(17U, Opcode::Add).s32().Inputs(8U, 2U);
            INST(18U, Opcode::NOP);
            INST(19U, Opcode::StoreArray).s32().Inputs(6U, 17U, 14U);  // a[i + 2] = a[i + 1]

            INST(20U, Opcode::Add).s32().Inputs(8U, 3U);               // i += 3
            INST(21U, Opcode::Compare).CC(CC_GE).b().Inputs(20U, 7U);  // i < X
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(21U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(23U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, NewlyAlllocatedCheck)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0x1U).s64();
        CONSTANT(1U, 0x0U).s64();
        CONSTANT(5U, 0x2aU).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(43U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 43U).TypeId(79U);
            INST(6U, Opcode::SaveState).Inputs(5U, 1U, 4U).SrcVregs({2U, 1U, 0U});
            INST(7U, Opcode::NullCheck).ref().Inputs(4U, 6U);
            INST(10U, Opcode::StoreArray).s32().Inputs(7U, 1U, 5U);
            INST(11U, Opcode::Return).ref().Inputs(4U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 0x1U).s64();
        CONSTANT(1U, 0x0U).s64();
        CONSTANT(5U, 0x2aU).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(43U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 43U).TypeId(79U);
            INST(6U, Opcode::SaveState).Inputs(5U, 1U, 4U).SrcVregs({2U, 1U, 0U});
            INST(7U, Opcode::NOP);
            INST(10U, Opcode::StoreArray).s32().Inputs(4U, 1U, 5U);
            INST(11U, Opcode::Return).ref().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, DominatedBoundsCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(1U, 1U, 0U).SrcVregs({2U, 1U, 0U});
            INST(4U, Opcode::LenArray).s32().Inputs(0U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 1U, 2U);
            INST(6U, Opcode::LoadArray).s64().Inputs(0U, 5U);
            INST(7U, Opcode::SaveState).Inputs(6U, 1U, 0U).SrcVregs({2U, 1U, 0U});
            INST(10U, Opcode::BoundsCheck).s32().Inputs(4U, 1U, 7U);
            INST(11U, Opcode::StoreArray).s64().Inputs(0U, 10U, 6U);
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(1U, 1U, 0U).SrcVregs({2U, 1U, 0U});
            INST(4U, Opcode::LenArray).s32().Inputs(0U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 1U, 2U);
            INST(6U, Opcode::LoadArray).s64().Inputs(0U, 5U);
            INST(7U, Opcode::SaveState).Inputs(6U, 1U, 0U).SrcVregs({2U, 1U, 0U});
            INST(10U, Opcode::NOP);
            INST(11U, Opcode::StoreArray).s64().Inputs(0U, 5U, 6U);
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

void ChecksEliminationTest::BuildGraphGroupedBoundsCheck()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // a
        PARAMETER(1U, 1U).s32();  // x
        CONSTANT(2U, 1U);
        CONSTANT(5U, -2L);
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::LenArray).s32().Inputs(0U);
            // a[x] = 1
            INST(7U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(8U, Opcode::BoundsCheck).s32().Inputs(6U, 1U, 7U);
            INST(9U, Opcode::StoreArray).s64().Inputs(0U, 8U, 2U);
            // a[x-1] = 1
            INST(10U, Opcode::Sub).s32().Inputs(1U, 2U);
            INST(11U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(12U, Opcode::BoundsCheck).s32().Inputs(6U, 10U, 11U);
            INST(13U, Opcode::StoreArray).s64().Inputs(0U, 12U, 2U);
            // a[x+1] = 1
            INST(14U, Opcode::Add).s32().Inputs(1U, 2U);
            INST(15U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(16U, Opcode::BoundsCheck).s32().Inputs(6U, 14U, 15U);
            INST(17U, Opcode::StoreArray).s64().Inputs(0U, 16U, 2U);
            // a[x+(-2)] = 1
            INST(18U, Opcode::Add).s32().Inputs(1U, 5U);
            INST(19U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(20U, Opcode::BoundsCheck).s32().Inputs(6U, 18U, 19U);
            INST(21U, Opcode::StoreArray).s64().Inputs(0U, 20U, 2U);
            // a[x-(-2)] = 1
            INST(22U, Opcode::Sub).s32().Inputs(1U, 5U);
            INST(23U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(24U, Opcode::BoundsCheck).s32().Inputs(6U, 22U, 23U);
            INST(25U, Opcode::StoreArray).s64().Inputs(0U, 24U, 2U);
            INST(26U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(ChecksEliminationTest, GroupedBoundsCheck)
{
    BuildGraphGroupedBoundsCheck();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).ref();  // a
        PARAMETER(1U, 1U).s32();  // x
        CONSTANT(2U, 1U);
        CONSTANT(5U, -2L);
        CONSTANT(3U, 2U);
        CONSTANT(4U, 0U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::LenArray).s32().Inputs(0U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});

            INST(30U, Opcode::Add).s32().Inputs(1U, 5U);
            INST(31U, Opcode::Compare).b().CC(CC_LT).SrcType(DataType::INT32).Inputs(30U, 4U);
            INST(32U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(31U, 7U);
            INST(27U, Opcode::Add).s32().Inputs(1U, 3U);
            INST(28U, Opcode::Compare).b().CC(CC_GE).SrcType(DataType::INT32).Inputs(27U, 6U);
            INST(29U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(28U, 7U);

            // a[x] = 1
            INST(8U, Opcode::NOP);
            INST(9U, Opcode::StoreArray).s64().Inputs(0U, 1U, 2U);
            // a[x-1] = 1
            INST(10U, Opcode::Sub).s32().Inputs(1U, 2U);
            INST(11U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(12U, Opcode::NOP);
            INST(13U, Opcode::StoreArray).s64().Inputs(0U, 10U, 2U);
            // a[x+1] = 1
            INST(14U, Opcode::Add).s32().Inputs(1U, 2U);
            INST(15U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(16U, Opcode::NOP);
            INST(17U, Opcode::StoreArray).s64().Inputs(0U, 14U, 2U);
            // a[x+(-2)] = 1
            INST(18U, Opcode::Add).s32().Inputs(1U, 5U);
            INST(19U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(20U, Opcode::NOP);
            INST(21U, Opcode::StoreArray).s64().Inputs(0U, 18U, 2U);
            // a[x-(-2)] = 1
            INST(22U, Opcode::Sub).s32().Inputs(1U, 5U);
            INST(23U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(24U, Opcode::NOP);
            INST(25U, Opcode::StoreArray).s64().Inputs(0U, 22U, 2U);
            INST(26U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

void ChecksEliminationTest::BuildGraphGroupedBoundsCheckConstIndex()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // a
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);
        CONSTANT(4U, 2U);
        CONSTANT(5U, 3U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::LenArray).s32().Inputs(0U);
            INST(7U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});

            // a[0] = 1
            INST(8U, Opcode::BoundsCheck).s32().Inputs(6U, 2U, 7U);
            INST(9U, Opcode::StoreArray).s64().Inputs(0U, 8U, 2U);
            // a[1] = 1
            INST(11U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(12U, Opcode::BoundsCheck).s32().Inputs(6U, 3U, 11U);
            INST(13U, Opcode::StoreArray).s64().Inputs(0U, 12U, 2U);
            // a[2] = 1
            INST(15U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(16U, Opcode::BoundsCheck).s32().Inputs(6U, 4U, 15U);
            INST(17U, Opcode::StoreArray).s64().Inputs(0U, 16U, 2U);
            // a[3] = 1
            INST(19U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(20U, Opcode::BoundsCheck).s32().Inputs(6U, 5U, 19U);
            INST(21U, Opcode::StoreArray).s64().Inputs(0U, 20U, 2U);
            INST(26U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(ChecksEliminationTest, GroupedBoundsCheckConstIndex)
{
    BuildGraphGroupedBoundsCheckConstIndex();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).ref();  // a
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);
        CONSTANT(4U, 2U);
        CONSTANT(5U, 3U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(6U, Opcode::LenArray).s32().Inputs(0U);
            INST(7U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});

            INST(28U, Opcode::Compare).b().CC(CC_GE).SrcType(DataType::INT32).Inputs(5U, 6U);
            INST(29U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(28U, 7U);

            // a[0] = 1
            INST(8U, Opcode::NOP);
            INST(9U, Opcode::StoreArray).s64().Inputs(0U, 2U, 2U);
            // a[1] = 1
            INST(11U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(12U, Opcode::NOP);
            INST(13U, Opcode::StoreArray).s64().Inputs(0U, 3U, 2U);
            // a[2] = 1
            INST(15U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(16U, Opcode::NOP);
            INST(17U, Opcode::StoreArray).s64().Inputs(0U, 4U, 2U);
            // a[3] = 1
            INST(19U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(20U, Opcode::NOP);
            INST(21U, Opcode::StoreArray).s64().Inputs(0U, 5U, 2U);
            INST(26U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, ConsecutiveNullChecks)
{
    builder_->EnableGraphChecker(false);
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, 1U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(1U, Opcode::NullCheck).ref().Inputs(0U, 5U);
            INST(2U, Opcode::NullCheck).ref().Inputs(1U, 5U);
            INST(3U, Opcode::NullCheck).ref().Inputs(2U, 5U);
            INST(6U, Opcode::StoreObject).ref().Inputs(3U, 0U).TypeId(1U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }
    builder_->EnableGraphChecker(true);
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, 1U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(1U, Opcode::NullCheck).ref().Inputs(0U, 5U);
            INST(2U, Opcode::NOP);
            INST(3U, Opcode::NOP);
            INST(6U, Opcode::StoreObject).ref().Inputs(1U, 0U).TypeId(1U);
            INST(4U, Opcode::Return).ref().Inputs(1U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NullCheckInCallVirt)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, 1U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 5U);
            INST(3U, Opcode::NullCheck).ref().Inputs(1U, 5U);
            INST(4U, Opcode::CallVirtual)
                .s32()
                .Inputs({{DataType::REFERENCE, 2U}, {DataType::REFERENCE, 3U}, {DataType::NO_TYPE, 5U}});
            INST(6U, Opcode::Return).s32().Inputs(4U);
        }
    }
    // Doesn't remove nullchecks if the method static
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));

    // Removes nullcheck from first parameter for not static method
    RuntimeInterfaceNotStaticMethod runtime;
    GetGraph()->SetRuntime(&runtime);
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, 1U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(2U, Opcode::NOP);
            INST(3U, Opcode::NullCheck).ref().Inputs(1U, 5U);
            INST(4U, Opcode::CallVirtual)
                .s32()
                .Inputs({{DataType::REFERENCE, 0U}, {DataType::REFERENCE, 3U}, {DataType::NO_TYPE, 5U}});
            INST(6U, Opcode::Return).s32().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, DominatedChecksWithDifferentTypes)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::ZeroCheck).s32().Inputs(0U, 2U);
            INST(4U, Opcode::Div).s32().Inputs(1U, 3U);
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(6U, Opcode::ZeroCheck).s64().Inputs(0U, 5U);
            INST(7U, Opcode::Mod).s64().Inputs(1U, 6U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::CallStatic).s32().InputsAutoType(4U, 7U, 20U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
    }
    auto initial = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), initial));
}

void ChecksEliminationTest::BuildGraphRefTypeCheck()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s32();
        PARAMETER(3U, 3U).s32();
        PARAMETER(4U, 4U).s32();
        CONSTANT(5U, nullptr);
        BASIC_BLOCK(2U, 1U)
        {
            INST(10U, Opcode::SaveState).Inputs(0U, 1U, 2U, 2U, 5U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(11U, Opcode::NullCheck).ref().Inputs(1U, 10U);
            INST(12U, Opcode::LenArray).s32().Inputs(11U);
            INST(13U, Opcode::BoundsCheck).s32().Inputs(12U, 2U, 10U);
            INST(14U, Opcode::RefTypeCheck).ref().Inputs(11U, 0U, 10U);
            INST(15U, Opcode::StoreArray).ref().Inputs(11U, 13U, 14U);

            INST(20U, Opcode::SaveState).Inputs(0U, 1U, 2U, 2U, 5U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(21U, Opcode::NullCheck).ref().Inputs(1U, 20U);
            INST(22U, Opcode::LenArray).s32().Inputs(21U);
            INST(23U, Opcode::BoundsCheck).s32().Inputs(22U, 3U, 20U);
            INST(24U, Opcode::RefTypeCheck).ref().Inputs(21U, 0U, 20U);
            INST(25U, Opcode::StoreArray).ref().Inputs(21U, 23U, 24U);

            INST(30U, Opcode::SaveState).Inputs(0U, 1U, 2U, 2U, 5U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(31U, Opcode::NullCheck).ref().Inputs(1U, 30U);
            INST(32U, Opcode::LenArray).s32().Inputs(31U);
            INST(33U, Opcode::BoundsCheck).s32().Inputs(32U, 4U, 30U);
            INST(34U, Opcode::RefTypeCheck).ref().Inputs(31U, 5U, 30U);
            INST(35U, Opcode::StoreArray).ref().Inputs(31U, 33U, 34U);

            INST(6U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(ChecksEliminationTest, RefTypeCheck)
{
    BuildGraphRefTypeCheck();
    // `24, Opcode::RefTypeCheck` is removed because `14, Opcode::RefTypeCheck` checks equal array and eqaul
    // Reference `34, Opcode::RefTypeCheck` is removed because store value id NullPtr
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s32();
        PARAMETER(3U, 3U).s32();
        PARAMETER(4U, 4U).s32();
        CONSTANT(5U, nullptr);
        BASIC_BLOCK(2U, 1U)
        {
            INST(10U, Opcode::SaveState).Inputs(0U, 1U, 2U, 2U, 5U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(11U, Opcode::NullCheck).ref().Inputs(1U, 10U);
            INST(12U, Opcode::LenArray).s32().Inputs(11U);
            INST(13U, Opcode::BoundsCheck).s32().Inputs(12U, 2U, 10U);
            INST(14U, Opcode::RefTypeCheck).ref().Inputs(11U, 0U, 10U);
            INST(15U, Opcode::StoreArray).ref().Inputs(11U, 13U, 14U);

            INST(20U, Opcode::SaveState).Inputs(0U, 1U, 2U, 2U, 5U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(21U, Opcode::NOP);
            INST(22U, Opcode::LenArray).s32().Inputs(11U);
            INST(23U, Opcode::BoundsCheck).s32().Inputs(22U, 3U, 20U);
            INST(24U, Opcode::NOP);
            INST(25U, Opcode::StoreArray).ref().Inputs(11U, 23U, 14U);

            INST(30U, Opcode::SaveState).Inputs(0U, 1U, 2U, 2U, 5U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(31U, Opcode::NOP);
            INST(32U, Opcode::LenArray).s32().Inputs(11U);
            INST(33U, Opcode::BoundsCheck).s32().Inputs(32U, 4U, 30U);
            INST(34U, Opcode::NOP);
            INST(35U, Opcode::StoreArray).ref().Inputs(11U, 33U, 5U);

            INST(6U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void ChecksEliminationTest::BuildGraphRefTypeCheckFirstNullCheckEliminated()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 2U).s32();
        PARAMETER(2U, 3U).s32();
        CONSTANT(3U, 10U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(5U, Opcode::NewArray).ref().Inputs(0U, 3U, 4U);
            INST(6U, Opcode::RefTypeCheck).ref().Inputs(5U, 0U, 4U);
            INST(7U, Opcode::StoreArray).ref().Inputs(5U, 1U, 6U);

            INST(14U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(15U, Opcode::NullCheck).ref().Inputs(5U, 14U);
            INST(16U, Opcode::RefTypeCheck).ref().Inputs(15U, 0U, 14U);
            INST(17U, Opcode::StoreArray).ref().Inputs(5U, 2U, 16U);

            INST(18U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(ChecksEliminationTest, RefTypeCheckFirstNullCheckEliminated)
{
    BuildGraphRefTypeCheckFirstNullCheckEliminated();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 2U).s32();
        PARAMETER(2U, 3U).s32();
        CONSTANT(3U, 10U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(5U, Opcode::NewArray).ref().Inputs(0U, 3U, 4U);
            INST(6U, Opcode::RefTypeCheck).ref().Inputs(5U, 0U, 4U);
            INST(7U, Opcode::StoreArray).ref().Inputs(5U, 1U, 6U);

            INST(14U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(15U, Opcode::NOP);
            INST(16U, Opcode::NOP);
            INST(17U, Opcode::StoreArray).ref().Inputs(5U, 2U, 6U);

            INST(18U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, RefTypeCheckEqualInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 2U).s32();
        PARAMETER(2U, 3U).s32();
        CONSTANT(3U, 10U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(5U, Opcode::NewArray).ref().Inputs(0U, 3U, 4U);
            INST(6U, Opcode::RefTypeCheck).ref().Inputs(5U, 0U, 4U);
            INST(7U, Opcode::StoreArray).ref().Inputs(5U, 1U, 6U);

            INST(14U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(16U, Opcode::RefTypeCheck).ref().Inputs(5U, 5U, 14U);
            INST(17U, Opcode::StoreArray).ref().Inputs(5U, 2U, 16U);

            INST(18U, Opcode::ReturnVoid).v0id();
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(clone, GetGraph()));
}

void ChecksEliminationTest::BuildHoistRefTypeCheckGraph()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(2U, 10U);
        PARAMETER(28U, 0U).ref();
        PARAMETER(29U, 1U).ref();
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(20U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U, 28U, 29U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);  // 0 < 10
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);  // i

            INST(30U, Opcode::SaveState).Inputs(28U, 29U).SrcVregs({0U, 1U});
            INST(31U, Opcode::NullCheck).ref().Inputs(29U, 30U);
            INST(32U, Opcode::LenArray).s32().Inputs(31U);
            INST(34U, Opcode::RefTypeCheck).ref().Inputs(31U, 28U, 30U);
            INST(35U, Opcode::StoreArray).ref().Inputs(31U, 4U, 34U);

            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);               // i++
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 2U);  // i < 10
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(ChecksEliminationTest, HoistRefTypeCheckTest)
{
    BuildHoistRefTypeCheckGraph();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(2U, 10U);
        PARAMETER(28U, 0U).ref();
        PARAMETER(29U, 1U).ref();
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(20U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U, 28U, 29U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(31U, Opcode::NullCheck).ref().Inputs(29U, 20U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(34U, Opcode::RefTypeCheck).ref().Inputs(31U, 28U, 20U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);  // 0 < 10
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);  // i

            INST(30U, Opcode::SaveState).Inputs(28U, 29U).SrcVregs({0U, 1U});
            INST(32U, Opcode::LenArray).s32().Inputs(31U);
            INST(35U, Opcode::StoreArray).ref().Inputs(31U, 4U, 34U);

            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);               // i++
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 2U);  // i < 10
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NotLenArrayInput)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(4U, 1U);
        CONSTANT(7U, 10U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(1U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(3U, Opcode::Phi).s32().Inputs(0U, 5U);
            INST(6U, Opcode::Compare).b().CC(CC_GE).Inputs(3U, 7U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(5U, 3U)
        {
            INST(9U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(10U, Opcode::NegativeCheck).s32().Inputs(3U, 9U);
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(9U).TypeId(68U);
            INST(11U, Opcode::NewArray).ref().Inputs(44U, 10U, 9U);
            INST(12U, Opcode::BoundsCheck).s32().Inputs(3U, 0U, 9U);
            INST(13U, Opcode::StoreArray).s32().Inputs(11U, 12U, 0U);
            INST(5U, Opcode::Add).s32().Inputs(3U, 4U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(2U, Opcode::ReturnVoid).v0id();
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(clone, GetGraph()));
}

void ChecksEliminationTest::BuildGraphBugWithNullCheck()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);          // initial
        CONSTANT(1U, 1U);          // increment
        PARAMETER(27U, 1U).s32();  // X
        BASIC_BLOCK(2U, 6U, 3U)
        {
            INST(43U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(13U, Opcode::NewArray).ref().Inputs(44U, 27U, 43U);
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_GE).b().Inputs(0U, 27U);  // i < X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(3U, 6U, 3U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 13U, 27U).SrcVregs({0U, 1U, 2U, 3U});
            INST(16U, Opcode::NullCheck).ref().Inputs(13U, 7U);
            INST(17U, Opcode::LenArray).s32().Inputs(16U);
            INST(8U, Opcode::BoundsCheck).s32().Inputs(17U, 4U, 7U);
            INST(9U, Opcode::StoreArray).s32().Inputs(16U, 8U, 0U);                   // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);                              // i++
            INST(5U, Opcode::Compare).CC(ConditionCode::CC_GE).b().Inputs(10U, 27U);  // i < X
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(26U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(12U, Opcode::Return).s32().Inputs(26U);
        }
    }
}

TEST_F(ChecksEliminationTest, BugWithNullCheck)
{
    BuildGraphBugWithNullCheck();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        PARAMETER(2U, 1U).s32();
        BASIC_BLOCK(2U, 5U, 3U)
        {
            INST(43U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 2U, 43U);
            INST(20U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(34U, Opcode::NOP);
            INST(22U, Opcode::LenArray).s32().Inputs(3U);
            INST(23U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(22U, 2U);  // len_array < X
            INST(24U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(23U, 20U);
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_GE).b().Inputs(0U, 2U);  // 0 < X
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 5U, 3U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 3U, 2U).SrcVregs({0U, 1U, 2U, 3U});
            INST(15U, Opcode::NOP);
            INST(16U, Opcode::LenArray).s32().Inputs(3U);
            INST(8U, Opcode::NOP);
            INST(9U, Opcode::StoreArray).s32().Inputs(3U, 4U, 0U);     // a[i] = 0
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);               // i++
            INST(13U, Opcode::Compare).CC(CC_GE).b().Inputs(10U, 2U);  // i < X
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(26U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(12U, Opcode::Return).s32().Inputs(26U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
void ChecksEliminationTest::BuildGraphNullAndBoundsChecksNestedLoop()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(3U, 0U);
        CONSTANT(9U, 4U);
        CONSTANT(32U, 1U);

        // fill 2D array
        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
        }
        BASIC_BLOCK(3U, 7U, 8U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(3U, 33U);
            INST(4U, Opcode::Compare).CC(CC_LE).b().Inputs(9U, 5U);
            INST(11U, Opcode::IfImm).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(8U, 4U)
        {
            INST(38U, Opcode::SaveStateDeoptimize).Inputs(0U, 3U, 5U).SrcVregs({0U, 1U, 2U});
        }
        BASIC_BLOCK(4U, 6U, 5U)
        {
            INST(15U, Opcode::Phi).s32().Inputs(3U, 31U);
            INST(19U, Opcode::Compare).CC(CC_LE).b().Inputs(9U, 15U);
            INST(20U, Opcode::IfImm).CC(CC_NE).Imm(0U).Inputs(19U);
        }
        BASIC_BLOCK(5U, 4U)
        {
            INST(21U, Opcode::SaveState).Inputs(0U, 3U, 5U, 15U).SrcVregs({0U, 1U, 2U, 3U});
            INST(22U, Opcode::NullCheck).ref().Inputs(0U, 21U);
            INST(23U, Opcode::LenArray).s32().Inputs(22U);
            INST(24U, Opcode::BoundsCheck).s32().Inputs(23U, 5U, 21U);
            INST(25U, Opcode::LoadArray).ref().Inputs(22U, 24U);

            INST(26U, Opcode::SaveState).Inputs(0U, 3U, 5U, 15U, 25U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(27U, Opcode::NullCheck).ref().Inputs(25U, 26U);
            INST(28U, Opcode::LenArray).s32().Inputs(27U);
            INST(29U, Opcode::BoundsCheck).s32().Inputs(28U, 15U, 26U);
            INST(30U, Opcode::StoreArray).s32().Inputs(27U, 29U, 5U);  // a[i][j] = i
            INST(31U, Opcode::Add).s32().Inputs(15U, 32U);
        }
        BASIC_BLOCK(6U, 3U)
        {
            INST(33U, Opcode::Add).s32().Inputs(5U, 32U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(34U, Opcode::ReturnVoid).v0id();
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
TEST_F(ChecksEliminationTest, NullAndBoundsChecksNestedLoop)
{
    BuildGraphNullAndBoundsChecksNestedLoop();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(3U, 0U);
        CONSTANT(9U, 4U);
        CONSTANT(32U, 1U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(35U, Opcode::NullCheck).ref().Inputs(0U, 2U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(39U, Opcode::LenArray).s32().Inputs(35U);
            INST(44U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_LT).Inputs(39U, 9U);
            INST(45U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(44U, 2U);
        }
        BASIC_BLOCK(3U, 7U, 8U)
        {
            INST(5U, Opcode::Phi).s32().Inputs(3U, 33U);
            INST(4U, Opcode::Compare).CC(CC_LE).b().Inputs(9U, 5U);
            INST(11U, Opcode::IfImm).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(8U, 4U)
        {
            INST(38U, Opcode::SaveStateDeoptimize).Inputs(0U, 3U, 5U).SrcVregs({0U, 1U, 2U});
            // we could put DeoptimizeIf NULL_CHECK here, but this is suboptimal
        }
        BASIC_BLOCK(4U, 6U, 5U)
        {
            INST(15U, Opcode::Phi).s32().Inputs(3U, 31U);
            INST(19U, Opcode::Compare).CC(CC_LE).b().Inputs(9U, 15U);
            INST(20U, Opcode::IfImm).CC(CC_NE).Imm(0U).Inputs(19U);
        }
        BASIC_BLOCK(5U, 4U)
        {
            INST(21U, Opcode::SaveState).Inputs(0U, 3U, 5U, 15U).SrcVregs({0U, 1U, 2U, 3U});
            INST(22U, Opcode::NOP);
            INST(23U, Opcode::LenArray).s32().Inputs(35U);
            INST(24U, Opcode::NOP);
            INST(25U, Opcode::LoadArray).ref().Inputs(35U, 5U);
            INST(26U, Opcode::SaveState).Inputs(0U, 3U, 5U, 15U, 25U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(27U, Opcode::NullCheck).ref().Inputs(25U, 26U);
            INST(28U, Opcode::LenArray).s32().Inputs(27U);
            INST(29U, Opcode::BoundsCheck).s32().Inputs(28U, 15U, 26U);
            INST(30U, Opcode::StoreArray).s32().Inputs(27U, 29U, 5U);  // a[i][j] = i
            INST(31U, Opcode::Add).s32().Inputs(15U, 32U);
        }
        BASIC_BLOCK(6U, 3U)
        {
            INST(33U, Opcode::Add).s32().Inputs(5U, 32U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(34U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void ChecksEliminationTest::BuildGraphLoopWithTwoPhi()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(3U, 0U);
        CONSTANT(4U, 1U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(5U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U, 3U, 4U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(6U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 4U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(7U, Opcode::NullCheck).ref().Inputs(0U, 6U);
            INST(8U, Opcode::LoadObject).ref().Inputs(7U);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(1U, 17U);
            INST(18U, Opcode::Phi).s32().Inputs(3U, 19U);
            INST(10U, Opcode::Compare).b().CC(CC_GE).Inputs(9U, 2U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(5U, 3U)
        {
            INST(12U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 4U, 8U).SrcVregs({0U, 1U, 2U, 3U, 4U, 5U});
            INST(13U, Opcode::NullCheck).ref().Inputs(8U, 12U);
            INST(14U, Opcode::LenArray).s32().Inputs(13U);
            INST(15U, Opcode::BoundsCheck).s32().Inputs(14U, 9U, 12U);
            INST(16U, Opcode::LoadArray).s32().Inputs(13U, 15U);
            INST(17U, Opcode::Add).s32().Inputs(9U, 4U);
            INST(19U, Opcode::Add).s32().Inputs(18U, 16U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(20U, Opcode::Return).s32().Inputs(18U);
        }
    }
}

TEST_F(ChecksEliminationTest, LoopWithTwoPhi)
{
    BuildGraphLoopWithTwoPhi();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        PARAMETER(2U, 2U).s32();
        CONSTANT(3U, 0U);
        CONSTANT(4U, 1U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(5U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U, 3U, 4U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(6U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 4U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(7U, Opcode::NullCheck).ref().Inputs(0U, 6U);
            INST(8U, Opcode::LoadObject).ref().Inputs(7U);
            INST(22U, Opcode::NullCheck).ref().Inputs(8U, 6U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
        }
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(9U, Opcode::Phi).s32().Inputs(1U, 17U);
            INST(18U, Opcode::Phi).s32().Inputs(3U, 19U);
            INST(10U, Opcode::Compare).b().CC(CC_GE).Inputs(9U, 2U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(5U, 3U)
        {
            INST(12U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 4U, 8U).SrcVregs({0U, 1U, 2U, 3U, 4U, 5U});
            INST(14U, Opcode::LenArray).s32().Inputs(22U);
            INST(15U, Opcode::BoundsCheck).s32().Inputs(14U, 9U, 12U);
            INST(16U, Opcode::LoadArray).s32().Inputs(22U, 15U);
            INST(17U, Opcode::Add).s32().Inputs(9U, 4U);
            INST(19U, Opcode::Add).s32().Inputs(18U, 16U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(20U, Opcode::Return).s32().Inputs(18U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

void ChecksEliminationTest::BuildGraphLoopWithBigStepGE()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 4U);  // increment
        CONSTANT(2U, 3U);
        PARAMETER(13U, 0U).ref();  // Array
        PARAMETER(27U, 1U).s32();  // X
        BASIC_BLOCK(2U, 3U)
        {
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(27U, 10U);
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(4U, 0U);  // i >= 0
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 13U, 27U).SrcVregs({0U, 1U, 2U, 3U});
            INST(16U, Opcode::NullCheck).ref().Inputs(13U, 7U);
            INST(17U, Opcode::LenArray).s32().Inputs(16U);
            INST(8U, Opcode::BoundsCheck).s32().Inputs(17U, 4U, 7U);
            INST(9U, Opcode::StoreArray).s32().Inputs(16U, 8U, 4U);  // a[i] = i
            INST(10U, Opcode::Sub).s32().Inputs(4U, 1U);             // i -= 4
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(ChecksEliminationTest, LoopWithBigStepGE)
{
    BuildGraphLoopWithBigStepGE();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 4U);  // increment
        CONSTANT(2U, 3U);
        PARAMETER(13U, 0U).ref();  // Array
        PARAMETER(27U, 1U).s32();  // X
        BASIC_BLOCK(2U, 3U)
        {
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(33U, Opcode::NullCheck).ref().Inputs(13U, 30U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(31U, Opcode::LenArray).s32().Inputs(33U);
            INST(36U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LE).b().Inputs(31U, 27U);
            // DeoptimizeIf len_array <= X
            INST(37U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(36U, 30U);
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(27U, 10U);
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_LT).b().Inputs(4U, 0U);  // i >= 0
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 13U, 27U).SrcVregs({0U, 1U, 2U, 3U});
            INST(16U, Opcode::NOP);
            INST(17U, Opcode::LenArray).s32().Inputs(33U);
            INST(8U, Opcode::NOP);
            INST(9U, Opcode::StoreArray).s32().Inputs(33U, 4U, 4U);  // a[i] = i
            INST(10U, Opcode::Sub).s32().Inputs(4U, 1U);             // i -= 4
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

void ChecksEliminationTest::BuildGraphLoopWithBigStepLE()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 2U);  // initial
        CONSTANT(1U, 8U);  // increment
        CONSTANT(2U, 3U);
        PARAMETER(13U, 0U).ref();  // Array
        PARAMETER(27U, 1U).s32();  // X
        BASIC_BLOCK(2U, 6U, 3U)
        {
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_GT).b().Inputs(0U, 27U);  // i <= X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(3U, 6U, 3U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 13U, 27U).SrcVregs({0U, 1U, 2U, 3U});
            INST(16U, Opcode::NullCheck).ref().Inputs(13U, 7U);
            INST(17U, Opcode::LenArray).s32().Inputs(16U);
            INST(8U, Opcode::BoundsCheck).s32().Inputs(17U, 4U, 7U);
            INST(9U, Opcode::LoadArray).s32().Inputs(16U, 8U);  // load a[i]
            INST(18U, Opcode::Add).s32().Inputs(4U, 2U);
            INST(19U, Opcode::BoundsCheck).s32().Inputs(17U, 18U, 7U);
            INST(20U, Opcode::StoreArray).s32().Inputs(16U, 19U, 9U);                 // a[i + 3] = a[i]
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);                              // i += 8
            INST(5U, Opcode::Compare).CC(ConditionCode::CC_GT).b().Inputs(10U, 27U);  // i <= X
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(26U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(12U, Opcode::Return).s32().Inputs(26U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
TEST_F(ChecksEliminationTest, LoopWithBigStepLE)
{
    BuildGraphLoopWithBigStepLE();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 2U);  // initial
        CONSTANT(1U, 8U);  // increment
        CONSTANT(2U, 3U);
        PARAMETER(13U, 0U).ref();  // Array
        PARAMETER(27U, 1U).s32();  // X
        CONSTANT(42U, 0x7ffffff7U);

        BASIC_BLOCK(2U, 6U, 3U)
        {
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(44U, Opcode::Compare).Inputs(42U, 27U).CC(CC_LT).b();
            INST(45U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(44U, 30U);
            INST(33U, Opcode::NullCheck).ref().Inputs(13U, 30U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(31U, Opcode::LenArray).s32().Inputs(33U);
            INST(36U, Opcode::Sub).s32().Inputs(27U, 0U);
            INST(37U, Opcode::Mod).s32().Inputs(36U, 1U);
            INST(38U, Opcode::Sub).s32().Inputs(27U, 37U);
            INST(39U, Opcode::Sub).s32().Inputs(31U, 2U);
            INST(40U, Opcode::Compare).b().CC(CC_LE).Inputs(39U, 38U);
            // DeoptimizeIf len - 3 <= X - (X - 2) % 8
            INST(41U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(40U, 30U);
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_GT).b().Inputs(0U, 27U);  // i <= X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(3U, 6U, 3U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 13U, 27U).SrcVregs({0U, 1U, 2U, 3U});
            INST(16U, Opcode::NOP);
            INST(17U, Opcode::LenArray).s32().Inputs(33U);
            INST(8U, Opcode::NOP);
            INST(9U, Opcode::LoadArray).s32().Inputs(33U, 4U);  // load a[i]
            INST(18U, Opcode::Add).s32().Inputs(4U, 2U);
            INST(19U, Opcode::NOP);
            INST(20U, Opcode::StoreArray).s32().Inputs(33U, 18U, 9U);                 // a[i + 3] = a[i]
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);                              // i += 8
            INST(5U, Opcode::Compare).CC(ConditionCode::CC_GT).b().Inputs(10U, 27U);  // i <= X
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(26U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(12U, Opcode::Return).s32().Inputs(26U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

void ChecksEliminationTest::BuildGraphLoopWithBigStepLT()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 2U);  // initial
        CONSTANT(1U, 8U);  // increment
        CONSTANT(2U, 3U);
        PARAMETER(13U, 0U).ref();  // Array
        PARAMETER(27U, 1U).s32();  // X
        BASIC_BLOCK(2U, 6U, 3U)
        {
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_GE).b().Inputs(0U, 27U);  // i < X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(3U, 6U, 3U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 13U, 27U).SrcVregs({0U, 1U, 2U, 3U});
            INST(16U, Opcode::NullCheck).ref().Inputs(13U, 7U);
            INST(17U, Opcode::LenArray).s32().Inputs(16U);
            INST(8U, Opcode::BoundsCheck).s32().Inputs(17U, 4U, 7U);
            INST(9U, Opcode::LoadArray).s32().Inputs(16U, 8U);  // load a[i]
            INST(18U, Opcode::Add).s32().Inputs(4U, 2U);
            INST(19U, Opcode::BoundsCheck).s32().Inputs(17U, 18U, 7U);
            INST(20U, Opcode::StoreArray).s32().Inputs(16U, 19U, 9U);                 // a[i + 3] = a[i]
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);                              // i += 8
            INST(5U, Opcode::Compare).CC(ConditionCode::CC_GE).b().Inputs(10U, 27U);  // i < X
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(26U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(12U, Opcode::Return).s32().Inputs(26U);
        }
    }
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
TEST_F(ChecksEliminationTest, LoopWithBigStepLT)
{
    BuildGraphLoopWithBigStepLT();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 2U);  // initial
        CONSTANT(1U, 8U);  // increment
        CONSTANT(2U, 3U);
        PARAMETER(13U, 0U).ref();  // Array
        PARAMETER(27U, 1U).s32();  // X
        CONSTANT(43U, 1U);
        CONSTANT(42U, 0x7ffffff8U);

        BASIC_BLOCK(2U, 6U, 3U)
        {
            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(44U, Opcode::Compare).Inputs(42U, 27U).CC(CC_LT).b();
            INST(45U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(44U, 30U);
            INST(33U, Opcode::NullCheck).ref().Inputs(13U, 30U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(31U, Opcode::LenArray).s32().Inputs(33U);
            INST(35U, Opcode::Add).s32().Inputs(0U, 43U);
            INST(36U, Opcode::Sub).s32().Inputs(27U, 35U);
            INST(37U, Opcode::Mod).s32().Inputs(36U, 1U);
            INST(38U, Opcode::Sub).s32().Inputs(27U, 37U);
            INST(39U, Opcode::Sub).s32().Inputs(31U, 2U);
            INST(40U, Opcode::Compare).b().CC(CC_LT).Inputs(39U, 38U);
            // DeoptimizeIf len - 3 < X - (X - (2 + 1)) % 8
            INST(41U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(40U, 30U);
            INST(14U, Opcode::Compare).CC(ConditionCode::CC_GE).b().Inputs(0U, 27U);  // i < X
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(3U, 6U, 3U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 13U, 27U).SrcVregs({0U, 1U, 2U, 3U});
            INST(16U, Opcode::NOP);
            INST(17U, Opcode::LenArray).s32().Inputs(33U);
            INST(8U, Opcode::NOP);
            INST(9U, Opcode::LoadArray).s32().Inputs(33U, 4U);  // load a[i]
            INST(18U, Opcode::Add).s32().Inputs(4U, 2U);
            INST(19U, Opcode::NOP);
            INST(20U, Opcode::StoreArray).s32().Inputs(33U, 18U, 9U);                 // a[i + 3] = a[i]
            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);                              // i += 8
            INST(5U, Opcode::Compare).CC(ConditionCode::CC_GE).b().Inputs(10U, 27U);  // i < X
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(26U, Opcode::Phi).s32().Inputs(0U, 10U);
            INST(12U, Opcode::Return).s32().Inputs(26U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
void ChecksEliminationTest::BuildGraphLoopWithBoundsCheckUnderIfGE()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(3U, 3U);
        PARAMETER(2U, 0U).ref();  // array
        PARAMETER(4U, 1U).s32();  // X

        BASIC_BLOCK(2U, 3U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::NullCheck).ref().Inputs(2U, 5U);
            INST(7U, Opcode::LenArray).s32().Inputs(6U);

            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
        }

        BASIC_BLOCK(3U, 4U, 8U)
        {
            INST(8U, Opcode::Phi).s32().Inputs(0U, 24U);             // i
            INST(25U, Opcode::Phi).s32().Inputs(0U, 23U);            // sum
            INST(9U, Opcode::Compare).CC(CC_LT).b().Inputs(8U, 4U);  // i < X
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(14U, Opcode::Compare).CC(CC_LE).b().Inputs(3U, 8U);  // 3 <= i
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(5U, 7U)
        {
            INST(16U, Opcode::Sub).s32().Inputs(8U, 3U);
            INST(27U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(17U, Opcode::BoundsCheck).s32().Inputs(7U, 16U, 27U);
            INST(18U, Opcode::LoadArray).s32().Inputs(6U, 17U);  // a[i - 3]
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(19U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(20U, Opcode::BoundsCheck).s32().Inputs(7U, 8U, 19U);
            INST(21U, Opcode::LoadArray).s32().Inputs(6U, 20U);  // a[i]
        }
        BASIC_BLOCK(7U, 3U)
        {
            INST(22U, Opcode::Phi).s32().Inputs(18U, 21U);
            INST(23U, Opcode::Add).s32().Inputs(25U, 22U);
            INST(24U, Opcode::Add).s32().Inputs(8U, 1U);
        }
        BASIC_BLOCK(8U, 1U)
        {
            INST(26U, Opcode::Return).s32().Inputs(25U);
        }
    }
}

// Lower bound is correct in each branch based on BoundsAnalysis, build deoptimize only for upper bound
// CC-OFFNXT(huge_method, G.FUN.01) graph creation
TEST_F(ChecksEliminationTest, LoopWithBoundsCheckUnderIfGE)
{
    BuildGraphLoopWithBoundsCheckUnderIfGE();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(3U, 3U);
        PARAMETER(2U, 0U).ref();  // array
        PARAMETER(4U, 1U).s32();  // X

        BASIC_BLOCK(2U, 3U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::NullCheck).ref().Inputs(2U, 5U);
            INST(7U, Opcode::LenArray).s32().Inputs(6U);

            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(31U, Opcode::Compare).b().CC(CC_LT).Inputs(7U, 4U);
            INST(32U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(31U, 30U);
        }

        BASIC_BLOCK(3U, 4U, 8U)
        {
            INST(8U, Opcode::Phi).s32().Inputs(0U, 24U);             // i
            INST(25U, Opcode::Phi).s32().Inputs(0U, 23U);            // sum
            INST(9U, Opcode::Compare).CC(CC_LT).b().Inputs(8U, 4U);  // i < X
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(14U, Opcode::Compare).CC(CC_LE).b().Inputs(3U, 8U);  // 3 <= i
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(5U, 7U)
        {
            INST(16U, Opcode::Sub).s32().Inputs(8U, 3U);
            INST(27U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(17U, Opcode::NOP);
            INST(18U, Opcode::LoadArray).s32().Inputs(6U, 16U);  // a[i - 3]
        }
        BASIC_BLOCK(6U, 7U)
        {
            INST(19U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(20U, Opcode::NOP);
            INST(21U, Opcode::LoadArray).s32().Inputs(6U, 8U);  // a[i]
        }
        BASIC_BLOCK(7U, 3U)
        {
            INST(22U, Opcode::Phi).s32().Inputs(18U, 21U);
            INST(23U, Opcode::Add).s32().Inputs(25U, 22U);
            INST(24U, Opcode::Add).s32().Inputs(8U, 1U);
        }
        BASIC_BLOCK(8U, 1U)
        {
            INST(26U, Opcode::Return).s32().Inputs(25U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

void ChecksEliminationTest::BuildGraphLoopWithBoundsCheckUnderIfLT()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(3U, 3U);
        PARAMETER(2U, 0U).ref();  // array
        PARAMETER(4U, 1U).s32();  // X

        BASIC_BLOCK(2U, 3U, 8U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::NullCheck).ref().Inputs(2U, 5U);
            INST(7U, Opcode::LenArray).s32().Inputs(6U);

            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(31U, Opcode::Compare).CC(CC_LT).b().Inputs(4U, 7U);  // X < len_array
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(31U);
        }

        BASIC_BLOCK(3U, 4U, 8U)
        {
            INST(8U, Opcode::Phi).s32().Inputs(4U, 24U);             // i = X
            INST(25U, Opcode::Phi).s32().Inputs(0U, 23U);            // sum
            INST(9U, Opcode::Compare).CC(CC_LT).b().Inputs(8U, 7U);  // i < len_array
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(13U, Opcode::Add).s32().Inputs(8U, 3U);
            INST(14U, Opcode::Compare).CC(CC_LT).b().Inputs(13U, 7U);  // i + 3 < len_array
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(20U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(17U, Opcode::BoundsCheck).s32().Inputs(7U, 13U, 20U);
            INST(18U, Opcode::LoadArray).s32().Inputs(6U, 17U);
            INST(19U, Opcode::Mul).s32().Inputs(8U, 18U);  // i * a[i + 3]
        }
        BASIC_BLOCK(6U, 3U)
        {
            INST(22U, Opcode::Phi).s32().Inputs(8U, 19U);
            INST(23U, Opcode::Add).s32().Inputs(25U, 22U);
            INST(24U, Opcode::Add).s32().Inputs(8U, 1U);
        }
        BASIC_BLOCK(8U, 1U)
        {
            INST(26U, Opcode::Phi).s32().Inputs(0U, 25U);
            INST(27U, Opcode::Return).s32().Inputs(26U);
        }
    }
}

// Upper bound is correct in each branch based on BoundsAnalysis, build deoptimize only for lower bound
// CC-OFFNXT(huge_method, G.FUN.01) graph creation
TEST_F(ChecksEliminationTest, LoopWithBoundsCheckUnderIfLT)
{
    BuildGraphLoopWithBoundsCheckUnderIfLT();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(3U, 3U);
        PARAMETER(2U, 0U).ref();  // array
        PARAMETER(4U, 1U).s32();  // X

        BASIC_BLOCK(2U, 3U, 8U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::NullCheck).ref().Inputs(2U, 5U);
            INST(7U, Opcode::LenArray).s32().Inputs(6U);

            INST(30U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(33U, Opcode::Add).s32().Inputs(4U, 3U);
            INST(34U, Opcode::Compare).b().CC(CC_LT).Inputs(33U, 0U);  // X + 3 < 0
            INST(35U, Opcode::DeoptimizeIf).DeoptimizeType(DeoptimizeType::BOUNDS_CHECK).Inputs(34U, 30U);
            INST(31U, Opcode::Compare).CC(CC_LT).b().Inputs(4U, 7U);  // X < len_array
            INST(32U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(31U);
        }
        BASIC_BLOCK(3U, 4U, 8U)
        {
            INST(8U, Opcode::Phi).s32().Inputs(4U, 24U);             // i = X
            INST(25U, Opcode::Phi).s32().Inputs(0U, 23U);            // sum
            INST(9U, Opcode::Compare).CC(CC_LT).b().Inputs(8U, 7U);  // i < len_array
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(4U, 5U, 6U)
        {
            INST(13U, Opcode::Add).s32().Inputs(8U, 3U);
            INST(14U, Opcode::Compare).CC(CC_LT).b().Inputs(13U, 7U);  // i + 3 < len_array
            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(14U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(20U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(17U, Opcode::NOP);
            INST(18U, Opcode::LoadArray).s32().Inputs(6U, 13U);
            INST(19U, Opcode::Mul).s32().Inputs(8U, 18U);  // i * a[i + 3]
        }
        BASIC_BLOCK(6U, 3U)
        {
            INST(22U, Opcode::Phi).s32().Inputs(8U, 19U);
            INST(23U, Opcode::Add).s32().Inputs(25U, 22U);
            INST(24U, Opcode::Add).s32().Inputs(8U, 1U);
        }
        BASIC_BLOCK(8U, 1U)
        {
            INST(26U, Opcode::Phi).s32().Inputs(0U, 25U);
            INST(27U, Opcode::Return).s32().Inputs(26U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph1));
}

TEST_F(ChecksEliminationTest, DeoptTest)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, nullptr);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::NullCheck).ref().Inputs(1U, 2U);
            INST(4U, Opcode::ZeroCheck).s32().Inputs(0U, 2U);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, nullptr);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(2U);
        }
    }
}

TEST_F(ChecksEliminationTest, CheckCastEqualInputs)
{
    // Check Elimination for CheckCast is applied.
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadClass).ref().Inputs(1U).TypeId(1U);
            INST(3U, Opcode::CheckCast).TypeId(1U).Inputs(0U, 2U, 1U);
            INST(4U, Opcode::CheckCast).TypeId(1U).Inputs(0U, 2U, 1U);
            INST(5U, Opcode::Return).ref().Inputs(0U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadClass).ref().Inputs(1U).TypeId(1U);
            INST(3U, Opcode::CheckCast).TypeId(1U).Inputs(0U, 2U, 1U);
            INST(4U, Opcode::NOP);
            INST(5U, Opcode::Return).ref().Inputs(0U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, CheckCastDifferentInputs)
{
    // Check Elimination for CheckCast is not applied.
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadClass).ref().Inputs(8U).TypeId(1U);
            INST(3U, Opcode::LoadClass).ref().Inputs(8U).TypeId(2U);
            INST(4U, Opcode::CheckCast).TypeId(1U).Inputs(0U, 2U, 8U);
            INST(5U, Opcode::CheckCast).TypeId(2U).Inputs(0U, 3U, 8U);
            INST(6U, Opcode::CheckCast).TypeId(2U).Inputs(1U, 3U, 8U);
            INST(7U, Opcode::Return).ref().Inputs(0U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

void ChecksEliminationTest::BuildGraphCheckCastAfterIsInstance()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(9U, nullptr);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadClass).ref().Inputs(1U).TypeId(1U);
            INST(3U, Opcode::IsInstance).b().Inputs(0U, 2U).TypeId(1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(4U, 1U)
        {
            INST(5U, Opcode::CheckCast).TypeId(1U).Inputs(0U, 2U, 1U);
            INST(6U, Opcode::LoadClass).ref().Inputs(1U).TypeId(2U);
            INST(7U, Opcode::CheckCast).TypeId(2U).Inputs(0U, 2U, 1U);
            INST(8U, Opcode::Return).ref().Inputs(0U);
        }

        BASIC_BLOCK(3U, 1U)
        {
            INST(10U, Opcode::Return).ref().Inputs(9U);
        }
    }
}

TEST_F(ChecksEliminationTest, CheckCastAfterIsInstance)
{
    // CheckCast after successful IsInstance can be removed.
    BuildGraphCheckCastAfterIsInstance();

    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(INS(7U).CastToCheckCast()->GetOmitNullCheck());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(9U, nullptr);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadClass).ref().Inputs(1U).TypeId(1U);
            INST(3U, Opcode::IsInstance).b().Inputs(0U, 2U).TypeId(1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(4U, 1U)
        {
            INST(5U, Opcode::NOP);
            INST(6U, Opcode::LoadClass).ref().Inputs(1U).TypeId(2U);
            INST(7U, Opcode::CheckCast).TypeId(2U).Inputs(0U, 2U, 1U);
            INST(8U, Opcode::Return).ref().Inputs(0U);
        }

        BASIC_BLOCK(3U, 1U)
        {
            INST(10U, Opcode::Return).ref().Inputs(9U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, CheckCastAfterIsInstanceTriangleCase)
{
    // CheckCast cannot be removed because dominating IsInstance can be false.
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(9U, nullptr);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadClass).ref().Inputs(1U).TypeId(1U);
            INST(3U, Opcode::IsInstance).b().Inputs(0U, 2U).TypeId(1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(3U, 4U) {}

        BASIC_BLOCK(4U, 1U)
        {
            INST(5U, Opcode::Phi).ref().Inputs({{2U, 9U}, {3U, 0U}});
            INST(6U, Opcode::CheckCast).TypeId(1U).Inputs(0U, 2U, 1U);
            INST(7U, Opcode::Return).ref().Inputs(5U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(ChecksEliminationTest, CheckCastAfterIsInstanceDiamondCase)
{
    // CheckCast cannot be removed because dominating IsInstance can be false.
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(9U, nullptr);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadClass).ref().Inputs(1U).TypeId(1U);
            INST(3U, Opcode::IsInstance).b().Inputs(0U, 2U).TypeId(1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(3U, 5U) {}

        BASIC_BLOCK(4U, 5U) {}

        BASIC_BLOCK(5U, 1U)
        {
            INST(5U, Opcode::Phi).ref().Inputs({{3U, 9U}, {4U, 0U}});
            INST(6U, Opcode::CheckCast).TypeId(1U).Inputs(0U, 2U, 1U);
            INST(7U, Opcode::Return).ref().Inputs(5U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

void ChecksEliminationTest::BuildHoistCheckCastGraph()
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(2U, 10U);
        PARAMETER(3U, 0U).ref();
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(21U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(22U, Opcode::LoadClass).ref().Inputs(21U).TypeId(1U);
            INST(20U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);  // 0 < 10
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);  // i
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::CheckCast).Inputs(3U, 22U, 7U).TypeId(1U);
            INST(9U, Opcode::LoadObject).ref().Inputs(3U);
            INST(23U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 9U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(11U, Opcode::CheckCast).Inputs(9U, 22U, 23U).TypeId(1U);

            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);               // i++
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 2U);  // i < 10
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(ChecksEliminationTest, HoistCheckCastTest)
{
    BuildHoistCheckCastGraph();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);  // initial
        CONSTANT(1U, 1U);  // increment
        CONSTANT(2U, 10U);
        PARAMETER(3U, 0U).ref();
        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(21U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(22U, Opcode::LoadClass).ref().Inputs(21U).TypeId(1U);
            INST(20U, Opcode::SaveStateDeoptimize).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::CheckCast).Inputs(3U, 22U, 20U).TypeId(1U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(5U, Opcode::Compare).SrcType(DataType::INT32).CC(CC_LT).b().Inputs(0U, 2U);  // 0 < 10
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(4U, Opcode::Phi).s32().Inputs(0U, 10U);  // i
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(9U, Opcode::LoadObject).ref().Inputs(3U);
            INST(23U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 9U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(11U, Opcode::CheckCast).Inputs(9U, 22U, 23U).TypeId(1U);

            INST(10U, Opcode::Add).s32().Inputs(4U, 1U);               // i++
            INST(13U, Opcode::Compare).CC(CC_LT).b().Inputs(10U, 2U);  // i < 10
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, NullCheckAfterIsInstance)
{
    // NullCheck after successful IsInstance can be removed.
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(9U, nullptr);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadClass).ref().Inputs(1U).TypeId(1U);
            INST(3U, Opcode::IsInstance).b().Inputs(0U, 2U).TypeId(1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(4U, 1U)
        {
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 1U);
            INST(6U, Opcode::Return).ref().Inputs(5U);
        }

        BASIC_BLOCK(3U, 1U)
        {
            INST(10U, Opcode::Return).ref().Inputs(9U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(9U, nullptr);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadClass).ref().Inputs(1U).TypeId(1U);
            INST(3U, Opcode::IsInstance).b().Inputs(0U, 2U).TypeId(1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Imm(0U).Inputs(3U);
        }

        BASIC_BLOCK(4U, 1U)
        {
            INST(5U, Opcode::NOP);
            INST(6U, Opcode::Return).ref().Inputs(0U);
        }

        BASIC_BLOCK(3U, 1U)
        {
            INST(10U, Opcode::Return).ref().Inputs(9U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(ChecksEliminationTest, OmitNullCheck)
{
    // Check Elimination for NullCheck is applied.
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 10U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::LoadArray).s32().Inputs(3U, 1U);
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(6U, Opcode::LoadClass).ref().Inputs(5U);
            INST(7U, Opcode::CheckCast).TypeId(1U).Inputs(0U, 6U, 5U);
            INST(8U, Opcode::LoadClass).ref().Inputs(5U);
            INST(9U, Opcode::IsInstance).TypeId(2U).b().Inputs(0U, 8U, 5U);
            INST(10U, Opcode::Return).b().Inputs(9U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
    ASSERT_TRUE(INS(7U).CastToCheckCast()->GetOmitNullCheck());
    ASSERT_TRUE(INS(9U).CastToIsInstance()->GetOmitNullCheck());
}

TEST_F(ChecksEliminationTest, DoNotOmitNullCheck)
{
    // NullCheck inside CheckCast and IsInstance cannot be omitted. NullCheck doesn't dominate them.
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 10U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().SrcType(DataType::Type::INT64).Inputs(1U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::LoadArray).s32().Inputs(5U, 1U);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(8U, Opcode::LoadClass).TypeId(1U).ref().Inputs(7U);
            INST(9U, Opcode::CheckCast).TypeId(1U).Inputs(0U, 8U, 7U);
            INST(10U, Opcode::LoadClass).TypeId(2U).ref().Inputs(7U);
            INST(11U, Opcode::IsInstance).TypeId(2U).b().Inputs(0U, 10U, 7U);
        }

        BASIC_BLOCK(5U, 1U)
        {
            INST(12U, Opcode::Phi).s32().Inputs(6U, 1U);
            INST(13U, Opcode::Return).s32().Inputs(12U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
    ASSERT_FALSE(INS(9U).CastToCheckCast()->GetOmitNullCheck());
    ASSERT_FALSE(INS(11U).CastToIsInstance()->GetOmitNullCheck());
}

// NOTE(schernykh): It's possible to remove boundschecks from this test, but BoundsAnalysis must be upgrade for
// it.
TEST_F(ChecksEliminationTest, OptimizeBoundsCheckElimination)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 0U);
        CONSTANT(14U, 1U);
        PARAMETER(1U, 0U).s32();
        PARAMETER(2U, 1U).s32();
        BASIC_BLOCK(2U, 6U, 3U)
        {
            INST(43U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 1U, 43U);
            INST(4U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LT).Inputs(2U, 0U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(3U, 6U, 4U)
        {
            INST(6U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(2U, 1U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, 6U, 5U)
        {
            INST(8U, Opcode::Add).s32().Inputs(2U, 14U);
            INST(9U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_GE).Inputs(8U, 1U);
            INST(10U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(9U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(11U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(12U, Opcode::BoundsCheck).s32().Inputs(1U, 8U, 11U);
            INST(13U, Opcode::StoreArray).s32().Inputs(3U, 8U, 0U);
        }
        BASIC_BLOCK(6U, 1U)
        {
            INST(15U, Opcode::ReturnVoid).v0id();
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

TEST_F(ChecksEliminationTest, BoundsCheckEqualInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(1U, 2U).s32();
        PARAMETER(2U, 3U).s32();
        CONSTANT(3U, 10U);
        BASIC_BLOCK(2U, 1U)
        {
            INST(4U, Opcode::SaveState).Inputs(1U, 2U).SrcVregs({1U, 2U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs().TypeId(68U);
            INST(5U, Opcode::NewArray).ref().Inputs(44U, 3U, 4U);
            INST(6U, Opcode::BoundsCheck).s32().Inputs(3U, 1U, 4U);
            INST(7U, Opcode::StoreArray).s32().Inputs(5U, 6U, 3U);

            INST(14U, Opcode::SaveState).Inputs(1U, 2U).SrcVregs({1U, 2U});
            INST(15U, Opcode::NewArray).ref().Inputs(44U, 2U, 14U);
            INST(16U, Opcode::BoundsCheck).s32().Inputs(2U, 3U, 14U);
            INST(17U, Opcode::StoreArray).s32().Inputs(15U, 16U, 3U);

            INST(18U, Opcode::ReturnVoid).v0id();
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GraphComparator().Compare(clone, GetGraph()));
}

// dominated checks
TEST_F(ChecksEliminationTest, AddSubOverflowCheckDom)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(3U, Opcode::AddOverflowCheck).s32().Inputs(0U, 1U, 2U);  // main

            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(5U, Opcode::AddOverflowCheck).s32().Inputs(0U, 1U, 4U);  // redundant

            INST(6U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(7U, Opcode::AddOverflowCheck).s32().Inputs(1U, 0U, 6U);  // redundant, swapped inputs

            INST(8U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(9U, Opcode::SubOverflowCheck).s32().Inputs(0U, 1U, 8U);  // main

            INST(10U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(11U, Opcode::SubOverflowCheck).s32().Inputs(0U, 1U, 10U);  // redundant

            INST(12U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(13U, Opcode::SubOverflowCheck).s32().Inputs(1U, 0U, 12U);  // not redundant, swapped inputs

            INST(14U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    ASSERT_TRUE(GetGraph()->RunPass<Cleanup>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(3U, Opcode::AddOverflowCheck).s32().Inputs(0U, 1U, 2U);  // main

            INST(8U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(9U, Opcode::SubOverflowCheck).s32().Inputs(0U, 1U, 8U);  // main

            INST(12U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(13U, Opcode::SubOverflowCheck).s32().Inputs(1U, 0U, 12U);  // not redundant, swapped inputs

            INST(14U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, GetGraph()));
}

void ChecksEliminationTest::BuildGraphOverflowCheckOptimize()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        CONSTANT(6U, 6U);
        CONSTANT(1000U, 0U);
        CONSTANT(13U, INT32_MAX);
        CONSTANT(14U, INT32_MIN);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(3U, Opcode::AddOverflowCheck).s32().Inputs(0U, 1U, 2U);  // maybe overflow

            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(5U, Opcode::SubOverflowCheck).s32().Inputs(0U, 1U, 4U);  // maybe overflow

            INST(20U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(30U, Opcode::AddOverflowCheck).s32().Inputs(0U, 6U, 20U);  // maybe overflow

            INST(40U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(50U, Opcode::SubOverflowCheck).s32().Inputs(0U, 6U, 40U);  // maybe overflow

            INST(7U, Opcode::Div).s32().Inputs(0U, 6U);
            INST(8U, Opcode::Div).s32().Inputs(1U, 6U);

            INST(9U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(10U, Opcode::AddOverflowCheck).s32().Inputs(7U, 8U, 9U);  // can't overflow

            INST(11U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(12U, Opcode::SubOverflowCheck).s32().Inputs(7U, 8U, 11U);  // can't overflow

            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1000U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(16U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(17U, Opcode::AddOverflowCheck).s32().Inputs(13U, 6U, 16U);  // must overflow
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(18U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(19U, Opcode::SubOverflowCheck).s32().Inputs(14U, 6U, 18U);  // must overflow
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(100U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(ChecksEliminationTest, OverflowCheckOptimize)
{
    BuildGraphOverflowCheckOptimize();
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        CONSTANT(6U, 6U);
        CONSTANT(1000U, 0U);
        CONSTANT(13U, INT32_MAX);
        CONSTANT(14U, INT32_MIN);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(3U, Opcode::AddOverflowCheck).s32().Inputs(0U, 1U, 2U);  // maybe overflow

            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(5U, Opcode::SubOverflowCheck).s32().Inputs(0U, 1U, 4U);  // maybe overflow

            INST(20U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(30U, Opcode::AddOverflowCheck).s32().Inputs(0U, 6U, 20U);  // maybe overflow

            INST(40U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(50U, Opcode::SubOverflowCheck).s32().Inputs(0U, 6U, 40U);  // maybe overflow

            INST(7U, Opcode::Div).s32().Inputs(0U, 6U);
            INST(8U, Opcode::Div).s32().Inputs(1U, 6U);

            INST(9U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(10U, Opcode::Add).s32().Inputs(7U, 8U);

            INST(11U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(12U, Opcode::Sub).s32().Inputs(7U, 8U);

            INST(15U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(1000U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(16U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(17U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::OVERFLOW_TYPE).Inputs(16U);  // must overflow
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(18U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({1U, 2U});
            INST(19U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::OVERFLOW_TYPE).Inputs(18U);  // must overflow
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, GetGraph()));
}

TEST_F(ChecksEliminationTest, LoopWithOverflowCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        CONSTANT(2U, 0U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::AddOverflowCheck).s32().Inputs(0U, 1U, 4U);
            INST(6U, Opcode::SubOverflowCheck).s32().Inputs(0U, 1U, 4U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).Inputs(2U).Imm(0U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        CONSTANT(2U, 0U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::SubOverflowCheck).s32().Inputs(0U, 1U, 3U);
            INST(6U, Opcode::AddOverflowCheck).s32().Inputs(0U, 1U, 3U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).Inputs(2U).Imm(0U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(8U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, GetGraph()));
}

TEST_F(ChecksEliminationTest, LoopWithAddOverflowCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        CONSTANT(10U, 10U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Phi).s32().Inputs(1U, 4U);
            INST(6U, Opcode::Compare).b().Inputs(2U, 10U).CC(CC_LT);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).Inputs(6U).Imm(0U).CC(CC_NE);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 10U).SrcVregs({0U, 1U, 10U});
            INST(4U, Opcode::AddOverflowCheck).s32().Inputs(2U, 1U, 3U);  // can't be overflow
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(7U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        CONSTANT(10U, 10U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Phi).s32().Inputs(1U, 4U);
            INST(6U, Opcode::Compare).b().Inputs(2U, 10U).CC(CC_LT);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).Inputs(6U).Imm(0U).CC(CC_NE);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 10U).SrcVregs({0U, 1U, 10U});
            INST(4U, Opcode::Add).s32().Inputs(2U, 1U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(7U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, GetGraph()));
}

TEST_F(ChecksEliminationTest, LoopWithSubOverflowCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        CONSTANT(10U, 10U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Phi).s32().Inputs(10U, 4U);
            INST(6U, Opcode::Compare).b().Inputs(2U, 1U).CC(CC_GT);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).Inputs(6U).Imm(0U).CC(CC_NE);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 10U).SrcVregs({0U, 1U, 10U});
            INST(4U, Opcode::SubOverflowCheck).s32().Inputs(2U, 1U, 3U);  // can't be overflow
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(7U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 1U);
        CONSTANT(10U, 10U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Phi).s32().Inputs(10U, 4U);
            INST(6U, Opcode::Compare).b().Inputs(2U, 1U).CC(CC_GT);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).Inputs(6U).Imm(0U).CC(CC_NE);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 10U).SrcVregs({0U, 1U, 10U});
            INST(4U, Opcode::Sub).s32().Inputs(2U, 1U);
        }
        BASIC_BLOCK(4U, 1U)
        {
            INST(7U, Opcode::Return).s32().Inputs(2U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, GetGraph()));
}

TEST_F(ChecksEliminationTest, AndWithAddOverFlowCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        CONSTANT(2U, 0x3U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::And).s32().Inputs(0U, 2U);
            INST(4U, Opcode::And).s32().Inputs(1U, 2U);
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(6U, Opcode::AddOverflowCheck).s32().Inputs(3U, 4U, 5U);
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<ChecksElimination>());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        PARAMETER(1U, 1U).s64();
        CONSTANT(2U, 0x3U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::And).s32().Inputs(0U, 2U);
            INST(4U, Opcode::And).s32().Inputs(1U, 2U);
            INST(5U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(6U, Opcode::Add).s32().Inputs(3U, 4U);
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, GetGraph()));
}

// Must Deoptimize
TEST_F(ChecksEliminationTest, NegOverflowAndZeroCheck1)
{
    for (auto cnst : {0, INT32_MIN}) {
        auto graph1 = CreateEmptyGraph();
        GRAPH(graph1)
        {
            CONSTANT(0U, cnst);
            BASIC_BLOCK(2U, -1L)
            {
                INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
                INST(6U, Opcode::NegOverflowAndZeroCheck).s32().Inputs(0U, 5U);
                INST(7U, Opcode::Return).s32().Inputs(6U);
            }
        }
        ASSERT_TRUE(graph1->RunPass<ChecksElimination>());
        auto graph2 = CreateEmptyGraph();
        GRAPH(graph2)
        {
            CONSTANT(0U, cnst);
            BASIC_BLOCK(2U, -1L)
            {
                INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
                INST(6U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::OVERFLOW_TYPE).Inputs(5U);
            }
        }
        ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
    }
}

// Remove dominated check
TEST_F(ChecksEliminationTest, NegOverflowAndZeroCheck2)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).u32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::NegOverflowAndZeroCheck).s32().Inputs(0U, 3U);
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::NegOverflowAndZeroCheck).s32().Inputs(0U, 5U);
            INST(7U, Opcode::CallStatic).s32().InputsAutoType(4U, 6U, 3U);
            INST(8U, Opcode::Return).s32().Inputs(7U);
        }
    }
    ASSERT_TRUE(graph1->RunPass<ChecksElimination>());
    ASSERT_TRUE(graph1->RunPass<Cleanup>());
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        PARAMETER(0U, 0U).u32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::NegOverflowAndZeroCheck).s32().Inputs(0U, 3U);
            INST(7U, Opcode::CallStatic).s32().InputsAutoType(4U, 4U, 3U);
            INST(8U, Opcode::Return).s32().Inputs(7U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

// Replace by Neg
TEST_F(ChecksEliminationTest, NegOverflowAndZeroCheck3)
{
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(0U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::NegOverflowAndZeroCheck).s32().Inputs(0U, 5U);
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }
    ASSERT_TRUE(graph1->RunPass<ChecksElimination>());
    auto graph2 = CreateEmptyGraph();
    GRAPH(graph2)
    {
        CONSTANT(0U, 1U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::Neg).s32().Inputs(0U);
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph1, graph2));
}

TEST_F(ChecksEliminationTest, OsrMode)
{
    auto osrGraph = CreateOsrGraph();
    GRAPH(osrGraph)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(42U, 1U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(15U, Opcode::SaveState).Inputs(42U).SrcVregs({42U});
            INST(16U, Opcode::NullCheck).ref().Inputs(42U, 15U);
            INST(17U, Opcode::LenArray).s32().Inputs(16U);
        }
        BASIC_BLOCK(3U, 5U, 4U)
        {
            INST(3U, Opcode::Phi).s32().Inputs(0U, 7U);
            INST(4U, Opcode::SaveStateOsr).Inputs(3U, 42U).SrcVregs({1U, 42U});
            INST(5U, Opcode::Compare).b().SrcType(DataType::Type::INT32).CC(CC_LE).Inputs(3U, 1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(7U, Opcode::Sub).s32().Inputs(3U, 2U);
            INST(8U, Opcode::SaveState).Inputs(42U).SrcVregs({42U});
            INST(9U, Opcode::NullCheck).ref().Inputs(42U, 8U);
            INST(10U, Opcode::LenArray).s32().Inputs(9U);
        }
        BASIC_BLOCK(5U, 6U)
        {
            INST(11U, Opcode::SaveState).Inputs(42U).SrcVregs({42U});
            INST(12U, Opcode::NullCheck).ref().Inputs(42U, 11U);
            INST(13U, Opcode::LenArray).s32().Inputs(12U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(14U, Opcode::Return).u32().Inputs(3U);
        }
    }
    ASSERT_FALSE(osrGraph->RunPass<ChecksElimination>());
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
