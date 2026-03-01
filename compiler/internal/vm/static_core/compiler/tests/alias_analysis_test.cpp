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
#include "optimizer/optimizations/vn.h"
#include "optimizer/analysis/alias_analysis.h"

namespace ark::compiler {
class AliasAnalysisTest : public GraphTest {
public:
    void BuildGraphCompleteLoadArray();
    void BuildGraphDynamicMethods(Graph *graph);
};

// NOLINTBEGIN(readability-magic-numbers)

/**
 *    foo (int *arr, int a1, int a2, int a3)
 *        int tmp;
 *        if (a3 < 0)
 *            tmp = arr[a1];
 *        else
 *            tmp = arr[a2];
 *        return tmp + arr[a1] + arr[a2];
 *
 * arr[a1] must alias the second arr[a1] and may alias arr[a2]
 */
TEST_F(AliasAnalysisTest, SimpleLoad)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        PARAMETER(3U, 3U).s64();
        CONSTANT(4U, 0U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Compare).b().Inputs(3U, 4U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(11U, Opcode::LoadArray).s32().Inputs(0U, 1U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(17U, Opcode::LoadArray).s32().Inputs(0U, 2U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(18U, Opcode::Phi).s32().Inputs({{4U, 11U}, {3U, 17U}});

            INST(23U, Opcode::LoadArray).s32().Inputs(0U, 1U);

            INST(28U, Opcode::LoadArray).s32().Inputs(0U, 2U);

            INST(29U, Opcode::Add).s32().Inputs(28U, 23U);
            INST(30U, Opcode::Add).s32().Inputs(29U, 18U);
            INST(31U, Opcode::Return).s32().Inputs(30U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckRefAlias(&INS(0U), &INS(0U)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(11U), &INS(23U)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(17U), &INS(28U)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(11U), &INS(28U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(17U), &INS(23U)), AliasType::MAY_ALIAS);
}

/**
 *    foo (int *arr, int a1, int a2, int a3)
 *        int tmp;
 *        if (a3 < 0)
 *            (NullCheck(arr) for arr[a1]);
 *            (BoundsCheck for a1)
 *            tmp = arr[a1];
 *        else
 *            (NullCheck(arr) for arr[a2]);
 *            (BoundsCheck for a2)
 *            tmp = arr[a2];
 *        (NullCheck(arr) for arr[a1]);
 *        (BoundsCheck for a1)
 *        (NullCheck(arr) for arr[a2]);
 *        (BoundsCheck for a2)
 *        return tmp + arr[a1] + arr[a2];
 *
 * arr[a1] must alias the second arr[a1] and may alias arr[a2]
 */
void AliasAnalysisTest::BuildGraphCompleteLoadArray()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        PARAMETER(3U, 3U).s64();
        CONSTANT(4U, 0U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::Compare).b().Inputs(3U, 4U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({2U, 3U, 4U});
            INST(8U, Opcode::NullCheck).ref().Inputs(0U, 7U);
            INST(9U, Opcode::LenArray).s32().Inputs(8U);
            INST(10U, Opcode::BoundsCheck).s32().Inputs(9U, 1U, 7U);
            INST(11U, Opcode::LoadArray).s32().Inputs(8U, 10U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(13U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({2U, 3U, 4U});
            INST(14U, Opcode::NullCheck).ref().Inputs(0U, 13U);
            INST(15U, Opcode::LenArray).s32().Inputs(14U);
            INST(16U, Opcode::BoundsCheck).s32().Inputs(15U, 2U, 13U);
            INST(17U, Opcode::LoadArray).s32().Inputs(14U, 16U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(18U, Opcode::Phi).s32().Inputs({{4U, 11U}, {3U, 17U}});

            INST(19U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({2U, 3U, 4U});
            INST(20U, Opcode::NullCheck).ref().Inputs(0U, 19U);
            INST(21U, Opcode::LenArray).s32().Inputs(20U);
            INST(22U, Opcode::BoundsCheck).s32().Inputs(21U, 1U, 19U);
            INST(23U, Opcode::LoadArray).s32().Inputs(20U, 22U);

            INST(24U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({2U, 3U, 4U});
            INST(25U, Opcode::NullCheck).ref().Inputs(0U, 24U);
            INST(26U, Opcode::LenArray).s32().Inputs(25U);
            INST(27U, Opcode::BoundsCheck).s32().Inputs(26U, 2U, 24U);
            INST(28U, Opcode::LoadArray).s32().Inputs(25U, 27U);

            INST(29U, Opcode::Add).s32().Inputs(28U, 23U);
            INST(30U, Opcode::Add).s32().Inputs(29U, 18U);
            INST(31U, Opcode::Return).s32().Inputs(30U);
        }
    }
}

TEST_F(AliasAnalysisTest, CompleteLoadArray)
{
    BuildGraphCompleteLoadArray();
    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(11U), &INS(23U)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(17U), &INS(28U)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(11U), &INS(28U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(17U), &INS(23U)), AliasType::MAY_ALIAS);
}

/**
 *    foo (int *arr, int a1)
 *        (NullCheck(arr) for arr[a1]);
 *        (BoundsCheck for a1)
 *        int tmp = arr[a1];
 *        if (tmp == 0)
 *            (NullCheck(arr) for arr[a1]);
 *            (BoundsCheck for a1)
 *            arr[a1] = tmp;
 *        return tmp;
 *
 * arr[a1] must alias the second arr[a1] and may alias arr[a2]
 */
TEST_F(AliasAnalysisTest, LoadStoreAlias)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        CONSTANT(8U, 0U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U, 1U).SrcVregs({0U, 1U, 2U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::LenArray).s32().Inputs(3U);
            INST(5U, Opcode::BoundsCheck).s32().Inputs(4U, 1U, 2U);
            INST(6U, Opcode::LoadArray).u64().Inputs(3U, 5U);
            INST(7U, Opcode::Compare).b().CC(CC_EQ).Inputs(6U, 8U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::Return).s64().Inputs(6U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(11U, Opcode::SaveState).Inputs(0U, 1U, 6U).SrcVregs({0U, 1U, 2U});
            INST(12U, Opcode::NullCheck).ref().Inputs(0U, 11U);
            INST(13U, Opcode::LenArray).s32().Inputs(12U);
            INST(14U, Opcode::BoundsCheck).s32().Inputs(13U, 1U, 11U);
            INST(15U, Opcode::StoreArray).u64().Inputs(12U, 14U, 6U);
            INST(16U, Opcode::Return).s64().Inputs(6U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(6U), &INS(15U)), AliasType::MUST_ALIAS);
}

/**
 *    foo (int *arr1, int *arr2, int a2, int a3)
 *       arr1[a2] = a3
 *       arr2[a3] = a2
 *
 *    All parameters by default may be aliased by each other
 */
TEST_F(AliasAnalysisTest, DifferentObjects)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s64();
        PARAMETER(3U, 3U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 2U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::LenArray).s32().Inputs(5U);
            INST(7U, Opcode::BoundsCheck).s32().Inputs(6U, 3U, 4U);
            INST(8U, Opcode::StoreArray).u64().Inputs(5U, 7U, 2U);

            INST(9U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 3U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(10U, Opcode::NullCheck).ref().Inputs(1U, 9U);
            INST(11U, Opcode::LenArray).s32().Inputs(10U);
            INST(12U, Opcode::BoundsCheck).s32().Inputs(11U, 2U, 9U);
            INST(13U, Opcode::StoreArray).u64().Inputs(10U, 12U, 3U);

            INST(14U, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckRefAlias(&INS(0U), &INS(1U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(8U), &INS(13U)), AliasType::MAY_ALIAS);
}

/**
 *    foo (int *arr, a1, a2)
 *        arr[a1] = a2;
 *        int *tmp = arr;
 *        return tmp[a1];
 *
 * tmp must alias arr
 */
TEST_F(AliasAnalysisTest, ArrayToArray)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U, 2U).SrcVregs({1U, 2U, 3U, 4U});
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(5U, Opcode::LenArray).s32().Inputs(4U);
            INST(6U, Opcode::BoundsCheck).s32().Inputs(5U, 1U, 3U);
            INST(7U, Opcode::StoreArray).u64().Inputs(4U, 6U, 2U);

            INST(9U, Opcode::SaveState).Inputs(0U, 0U, 1U, 2U, 2U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(10U, Opcode::NullCheck).ref().Inputs(0U, 9U);
            INST(11U, Opcode::LenArray).s32().Inputs(10U);
            INST(12U, Opcode::BoundsCheck).s32().Inputs(11U, 1U, 9U);
            INST(13U, Opcode::LoadArray).s64().Inputs(10U, 12U);
            INST(14U, Opcode::Return).s64().Inputs(13U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(7U), &INS(13U)), AliasType::MUST_ALIAS);
}

/**
 *    foo (int *arr0, int *arr1, a2)
 *        int *tmp;
 *        if (a2 == 0)
 *            tmp = arr0;
 *        else
 *            tmp = arr1;
 *        tmp[a2] = 0;
 *        return 0;
 *
 * tmp may alias arr0 and arr1
 */
TEST_F(AliasAnalysisTest, PhiRef)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s64();
        CONSTANT(4U, 0U).s64();
        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(2U, 4U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, -1L)
        {
            INST(7U, Opcode::Phi).ref().Inputs({{3U, 0U}, {2U, 1U}});
            INST(8U, Opcode::StoreArray).u64().Inputs(7U, 2U, 4U);

            INST(9U, Opcode::Return).s64().Inputs(4U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckRefAlias(&INS(7U), &INS(0U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckRefAlias(&INS(7U), &INS(1U)), AliasType::MAY_ALIAS);
}

/**
 *    foo (int *arr0)
 *        int *x = new int[10];
 *        int tmp = arr0[0];
 *        x[0] = tmp;
 *        return x;
 *
 * arr0 and x are not aliasing
 */
TEST_F(AliasAnalysisTest, NewWithArg)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 10U).s64();
        CONSTANT(2U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(13U, Opcode::SaveState).Inputs(1U, 2U, 0U, 2U).SrcVregs({0U, 1U, 2U, 3U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(13U).TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 1U, 13U);
            INST(4U, Opcode::SaveState).Inputs(1U, 2U, 3U, 0U, 2U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::LoadArray).u64().Inputs(5U, 2U);

            INST(7U, Opcode::SaveState).Inputs(1U, 2U, 3U, 0U, 6U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(8U, Opcode::NullCheck).ref().Inputs(3U, 7U);
            INST(9U, Opcode::StoreArray).u64().Inputs(8U, 2U, 6U);
            INST(10U, Opcode::Return).ref().Inputs(3U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckRefAlias(&INS(3U), &INS(0U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(6U), &INS(9U)), AliasType::NO_ALIAS);
}

/**
 *    foo (int *arr0)
 *        int *x = new int[10];
 *        x[0] = 0;
 *        bar (x);               <- escaping call
 *        x[0] = 0;
 *
 *        int tmp = arr0[0];
 *        x[0] = tmp;
 *        return x;
 *
 * arr0 and x may be aliased because x is escaping because of calling bar function.
 */
TEST_F(AliasAnalysisTest, EscapingNewWithArg)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 10U).s64();
        CONSTANT(2U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(20U, Opcode::SaveState).Inputs(1U, 2U, 0U, 2U).SrcVregs({0U, 1U, 2U, 3U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(20U).TypeId(68U);
            INST(3U, Opcode::NewArray).ref().Inputs(44U, 1U, 20U);
            INST(11U, Opcode::StoreArray).u64().Inputs(3U, 2U, 2U);
            INST(12U, Opcode::CallStatic).ref().InputsAutoType(3U, 20U);
            INST(13U, Opcode::StoreArray).u64().Inputs(3U, 2U, 2U);
            INST(4U, Opcode::SaveState).Inputs(1U, 2U, 3U, 0U, 2U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::LoadArray).u64().Inputs(5U, 2U);

            INST(7U, Opcode::SaveState).Inputs(1U, 2U, 3U, 0U, 6U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(8U, Opcode::NullCheck).ref().Inputs(3U, 7U);
            INST(9U, Opcode::StoreArray).u64().Inputs(8U, 2U, 6U);
            INST(10U, Opcode::Return).ref().Inputs(3U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckRefAlias(&INS(3U), &INS(0U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(6U), &INS(9U)), AliasType::MAY_ALIAS);
}

/**
 *    foo (int **arr0)
 *        int *x = new int[10]
 *        arr0[0] = x;        <-- potential escaping
 *        int *y = arr0[10];
 *        x[0] = y[0];
 *
 *    The test aims to track escaping through a sequence of checks.
 */
TEST_F(AliasAnalysisTest, EscapingWithChecks)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 10U).s64();
        CONSTANT(2U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(20U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(20U).TypeId(68U);
            INST(9U, Opcode::NewArray).ref().Inputs(44U, 1U, 20U);
            INST(10U, Opcode::SaveState).Inputs(0U, 1U, 9U).SrcVregs({0U, 1U, 9U});

            // At first glance it is an aritificial sequence of checks but it may
            // happen if several inlinings occurs with multidimentional array operations
            INST(11U, Opcode::NullCheck).ref().Inputs(9U, 10U);
            INST(12U, Opcode::RefTypeCheck).ref().Inputs(11U, 0U, 10U);
            INST(15U, Opcode::StoreArrayI).ref().Imm(0U).Inputs(0U, 12U);
            INST(16U, Opcode::LoadArray).ref().Inputs(0U, 1U);

            INST(17U, Opcode::LoadArrayI).u32().Imm(0U).Inputs(16U);
            INST(18U, Opcode::StoreArrayI).u32().Imm(0U).Inputs(9U, 17U);
            INST(19U, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(17U), &INS(18U)), AliasType::NO_ALIAS);
}

/**
 *    foo (int *arr0, int a1)
 *        int tmp;
 *        if (a1 == 0)
 *            tmp = arr0[a1 + 10];
 *        else
 *            tmp = 10;
 *            arr0[a1 + 10] = tmp;
 *        return tmp;
 *
 * both arr0[a1 + 10] must alias each other
 */
TEST_F(AliasAnalysisTest, SimpleHeuristic)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        CONSTANT(2U, 10U).s64();
        CONSTANT(4U, 0U).s64();

        BASIC_BLOCK(2U, 4U, 3U)
        {
            INST(3U, Opcode::Compare).b().CC(CC_EQ).Inputs(1U, 4U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(3U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(21U, Opcode::AddI).s32().Imm(10ULL).Inputs(1U);
            INST(7U, Opcode::SaveState).Inputs(21U, 0U, 1U, 2U).SrcVregs({5U, 3U, 4U, 0U});
            INST(8U, Opcode::NullCheck).ref().Inputs(0U, 7U);
            INST(9U, Opcode::LenArray).s32().Inputs(8U);
            INST(10U, Opcode::BoundsCheck).s32().Inputs(9U, 21U, 7U);
            INST(11U, Opcode::LoadArray).s64().Inputs(8U, 10U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(22U, Opcode::AddI).s32().Imm(10ULL).Inputs(1U);
            INST(14U, Opcode::SaveState).Inputs(1U, 22U, 0U, 1U, 2U).SrcVregs({4U, 2U, 3U, 0U, 5U});
            INST(15U, Opcode::NullCheck).ref().Inputs(0U, 14U);
            INST(16U, Opcode::LenArray).s32().Inputs(15U);
            INST(17U, Opcode::BoundsCheck).s32().Inputs(16U, 22U, 14U);
            INST(18U, Opcode::StoreArray).u64().Inputs(15U, 17U, 2U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(19U, Opcode::Phi).s64().Inputs({{3U, 11U}, {4U, 2U}});
            INST(20U, Opcode::Return).s64().Inputs(19U);
        }
    }

    GetGraph()->RunPass<ValNum>();
    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(11U), &INS(18U)), AliasType::MUST_ALIAS);
}

/**
 *    foo (int32_t *arr0, int64_t *arr1, int64_t a2, int32_t a3)
 *        arr0[a2] = a2;
 *        arr1[a2] = arr0[a3];
 *        return arr1[a3];
 *
 * arr0 cannot alias arr1 due to different types
 */
TEST_F(AliasAnalysisTest, TypeComparison)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();  // i32[]
        PARAMETER(1U, 1U).ref();  // i64[]
        PARAMETER(2U, 2U).s64();
        PARAMETER(3U, 3U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 2U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::LenArray).s32().Inputs(5U);
            INST(7U, Opcode::BoundsCheck).s32().Inputs(6U, 2U, 4U);
            INST(8U, Opcode::StoreArray).u16().Inputs(5U, 7U, 2U);

            INST(9U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 3U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(12U, Opcode::BoundsCheck).s32().Inputs(6U, 3U, 9U);
            INST(13U, Opcode::LoadArray).u16().Inputs(5U, 12U);

            INST(14U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 13U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(15U, Opcode::NullCheck).ref().Inputs(1U, 14U);
            INST(16U, Opcode::LenArray).s32().Inputs(15U);
            INST(17U, Opcode::BoundsCheck).s32().Inputs(16U, 2U, 14U);
            INST(18U, Opcode::StoreArray).u32().Inputs(15U, 17U, 13U);

            INST(19U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U, 3U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(22U, Opcode::BoundsCheck).s32().Inputs(16U, 3U, 19U);
            INST(23U, Opcode::LoadArray).u32().Inputs(15U, 22U);
            INST(25U, Opcode::Return).s32().Inputs(23U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(8U), &INS(13U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(18U), &INS(23U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(8U), &INS(18U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(8U), &INS(23U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(13U), &INS(18U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(13U), &INS(23U)), AliasType::NO_ALIAS);
}

/**
 *    .function i32 foo(i32[] a0, R1[] a1, R1[] a2) {
 *        movi v0, 0
 *        lda v0
 *        ldarr.obj a1
 *        starr.obj a2, v0
 *
 *        lda v0
 *        ldarr a0
 *        return
 *    }
 *
 * Arrays of primitive types cannot alias arrays of ref type.
 */
TEST_F(AliasAnalysisTest, TypeComparison2)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).ref();
        CONSTANT(3U, 0x0U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::LoadArray).ref().Inputs(1U, 3U);
            INST(13U, Opcode::StoreArray).ref().Inputs(2U, 3U, 8U);
            INST(18U, Opcode::LoadArray).s32().Inputs(0U, 3U);
            INST(19U, Opcode::Return).s32().Inputs(18U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(8U), &INS(13U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(8U), &INS(18U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(13U), &INS(18U)), AliasType::NO_ALIAS);
}

/**
 *    foo(int *arr0, int *arr1, int a2)
 *        arr0[0] = a2;
 *        arr0[1] = a2;
 *        arr1[0] = a2;
 *        arr0[a2] = arr0[1];
 *
 * arr0[0] and arr0[1] do not alias each other
 * arr0[0] and arr1[0] may alias each other
 * arr0[a2] and arr0[0] may alias each other
 */
TEST_F(AliasAnalysisTest, LoadStoreImm)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(2U, 2U, 0U, 1U).SrcVregs({5U, 4U, 2U, 3U});
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(5U, Opcode::LenArray).s32().Inputs(4U);
            INST(6U, Opcode::BoundsCheckI).s32().Imm(0U).Inputs(5U, 3U);
            INST(7U, Opcode::StoreArrayI).u64().Imm(0U).Inputs(4U, 2U);

            INST(8U, Opcode::SaveState).Inputs(2U, 2U, 0U, 1U).SrcVregs({5U, 4U, 2U, 3U});
            INST(9U, Opcode::BoundsCheckI).s32().Imm(1U).Inputs(5U, 8U);
            INST(10U, Opcode::StoreArrayI).u64().Imm(1U).Inputs(4U, 2U);

            INST(11U, Opcode::SaveState).Inputs(2U, 2U, 0U, 1U).SrcVregs({5U, 4U, 2U, 3U});
            INST(12U, Opcode::NullCheck).ref().Inputs(1U, 11U);
            INST(13U, Opcode::LenArray).s32().Inputs(12U);
            INST(14U, Opcode::BoundsCheckI).s32().Imm(0U).Inputs(13U, 11U);
            INST(15U, Opcode::StoreArrayI).u64().Imm(0U).Inputs(12U, 2U);

            INST(16U, Opcode::SaveState).Inputs(2U, 1U, 0U).SrcVregs({4U, 3U, 2U});
            INST(17U, Opcode::BoundsCheckI).s32().Imm(1U).Inputs(5U, 16U);
            INST(18U, Opcode::LoadArrayI).u64().Imm(1U).Inputs(4U);

            INST(19U, Opcode::SaveState).Inputs(18U, 2U, 0U, 1U).SrcVregs({5U, 4U, 2U, 3U});
            INST(20U, Opcode::BoundsCheck).s32().Inputs(5U, 2U, 19U);
            INST(21U, Opcode::StoreArray).u64().Inputs(4U, 20U, 18U);

            INST(22U, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(7U), &INS(10U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(7U), &INS(18U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(7U), &INS(15U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(7U), &INS(21U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(15U), &INS(18U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(10U), &INS(18U)), AliasType::MUST_ALIAS);
}

/**
 *    .record R1 {}
 *    .record R3 {}
 *    .record R4 {
 *        R1     sf_r1 <static>   # type_id = 349
 *        R1     sf_r2 <static>   # type_id = 363
 *        R3     sf_r3 <static>   # type_id = 377
 *    }
 *    .function void foo() {
 *        ldstatic R4.sf_r1
 *        sta.obj v0
 *        ldstatic R4.sf_r3
 *        sta.obj v1
 *        lda.obj v0
 *        ststatic R4.sf_r3
 *        lda.obj v1
 *        ststatic R4.sf_r2
 *        return.void
 *    }
 *
 * Static acceses MUST_ALIAS themselves and has NO_ALIAS with anything else
 */
TEST_F(AliasAnalysisTest, StaticFields)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(5U, 0x2aU).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(9U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(9U).TypeId(0U);
            INST(7U, Opcode::LoadAndInitClass).ref().Inputs(9U).TypeId(0U);
            INST(8U, Opcode::LoadAndInitClass).ref().Inputs(9U).TypeId(0U);
            INST(1U, Opcode::LoadStatic).ref().Inputs(6U).TypeId(349U);
            INST(2U, Opcode::LoadStatic).ref().Inputs(7U).TypeId(377U);
            INST(3U, Opcode::StoreStatic).ref().Inputs(7U, 1U).TypeId(377U);
            INST(4U, Opcode::StoreStatic).ref().Inputs(8U, 2U).TypeId(363U);
            INST(25U, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    /* Global variable must alias itself */
    ASSERT_EQ(alias.CheckInstAlias(&INS(2U), &INS(3U)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(1U), &INS(2U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(1U), &INS(4U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(2U), &INS(4U)), AliasType::NO_ALIAS);
}

/*
 *    .record R1 {}
 *    .record R7 {
 *        R9 ref<static> # type_id = 211
 *    }
 *    .record R8 {
 *        R9 ref         $ type_id = 268
 *    }
 *    .record R9 {
 *        R1 ref         # type_id = 242
 *    }
 *    .function R1 foo(R8 a0, R9 a1) {
 *        ldstatic R7.ref
 *        sta.obj v1
 *        ldobj v1, R9.ref
 *        sta.obj v0
 *
 *        ldobj a0, R8.ref
 *        sta.obj v1
 *        ldobj v1, R9.ref
 *        stobj a1, R9.ref
 *
 *        lda.obj v0
 *        return.obj
 *    }
 *
 * R7.ref MAY_ALIAS R9.ref from argument
 */
TEST_F(AliasAnalysisTest, StaticFieldObject)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(16U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({2U, 3U});
            INST(9U, Opcode::LoadAndInitClass).ref().Inputs(16U).TypeId(0U);
            INST(2U, Opcode::LoadStatic).Inputs(9U).ref().TypeId(211U);
            INST(5U, Opcode::LoadObject).ref().Inputs(2U).TypeId(242U);
            INST(8U, Opcode::LoadObject).ref().Inputs(0U).TypeId(268U);
            INST(11U, Opcode::LoadObject).ref().Inputs(8U).TypeId(242U);
            INST(14U, Opcode::StoreObject).ref().Inputs(1U, 11U).TypeId(242U);
            INST(15U, Opcode::Return).ref().Inputs(5U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(14U), &INS(11U)), AliasType::MAY_ALIAS);
    /* Static field can be asigned outside */
    ASSERT_EQ(alias.CheckInstAlias(&INS(14U), &INS(5U)), AliasType::MAY_ALIAS);
}

/**
 *    .record R1 {}
 *    .record R2 {
 *        R1 if_r1 # type_id = 338
 *        R1 if_r2 # type_id = 352
 *    }
 *    .function void foo(R2 a0, R1 a1, R1 a2) {
 *        ldobj a0, R2.if_r1
 *        stobj a0, R2.if_r2
 *        lda.obj a1
 *        stobj a0, R2.if_r1
 *        return.void
 *    }
 *
 * Generally, if an object is not created in current scope it may alias any
 * other object. However, different fields even of the same object do not alias
 * each other.
 */
TEST_F(AliasAnalysisTest, ObjectFields)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::LoadObject).ref().Inputs(3U).TypeId(338U);

            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 4U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::NullCheck).ref().Inputs(0U, 5U);
            INST(7U, Opcode::StoreObject).ref().Inputs(6U, 4U).TypeId(352U);

            INST(8U, Opcode::SaveState).Inputs(0U, 1U, 4U).SrcVregs({0U, 1U, 2U});
            INST(9U, Opcode::NullCheck).ref().Inputs(0U, 8U);
            INST(10U, Opcode::StoreObject).ref().Inputs(9U, 1U).TypeId(338U);

            INST(11U, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(4U), &INS(7U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(7U), &INS(10U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(4U), &INS(10U)), AliasType::MUST_ALIAS);
}

/**
 *    .record R3 {
 *        R1 if_ref # type_id = 302
 *    }
 *    .record R4 {
 *        R3 if_r3  # type_id = 366
 *    }
 *    .function R3 init16(R4 a0, R1 a1) {
 *        ldobj a0, R4.if_r3
 *        sta.obj v1
 *        lda.obj a1
 *        stobj v1, R3.if_ref
 *
 *        newobj v2, R4
 *        newobj v3, R3
 *        lda.obj v3
 *        stobj v2, R4.if_r3
 *        return.obj
 *    }
 *
 * Similar to ObjectFields test but the same fields has NO_ALIAS due to
 * creation of the object in the current scope
 */
TEST_F(AliasAnalysisTest, MoreObjectFields)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({3U, 4U});
            INST(3U, Opcode::NullCheck).ref().Inputs(0U, 2U);
            INST(4U, Opcode::LoadObject).ref().Inputs(3U).TypeId(366U);

            INST(5U, Opcode::SaveState).Inputs(4U, 1U).SrcVregs({1U, 5U});
            INST(6U, Opcode::NullCheck).ref().Inputs(4U, 5U);
            INST(7U, Opcode::StoreObject).ref().Inputs(6U, 1U).TypeId(302U);

            INST(14U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(8U, Opcode::NewObject).ref().Inputs(14U, 5U);
            INST(9U, Opcode::NewObject).ref().Inputs(14U, 5U);
            INST(10U, Opcode::SaveState).Inputs(4U, 8U, 9U).SrcVregs({1U, 2U, 5U});
            INST(11U, Opcode::NullCheck).ref().Inputs(8U, 10U);
            INST(12U, Opcode::StoreObject).ref().Inputs(11U, 9U).TypeId(366U);
            INST(13U, Opcode::Return).ref().Inputs(9U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(4U), &INS(12U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(4U), &INS(7U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(7U), &INS(12U)), AliasType::NO_ALIAS);
}

/*
    struct {
        void *o1
        void *o2
        void *o3
    } st

    foo(uintptr *arr, int a1)
        st.o1 = "my_str"
        st.o2 = "your_str"
        st.o3 = "my_str"

 Values from pool MUST_ALIAS themselves
*/
TEST_F(AliasAnalysisTest, PoolAlias)
{
    uint32_t myStrId = 0;
    uint32_t yourStrId = 1;
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(12U, Opcode::SaveState).NoVregs();
            INST(0U, Opcode::LoadString).ref().Inputs(12U).TypeId(myStrId);
            INST(9U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(9U);
            INST(1U, Opcode::StoreStatic).ref().Inputs(6U, 0U);

            INST(13U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadString).ref().Inputs(13U).TypeId(yourStrId);
            INST(10U, Opcode::SaveState).Inputs(2U).SrcVregs({0U});
            INST(7U, Opcode::LoadAndInitClass).ref().Inputs(10U);
            INST(3U, Opcode::StoreStatic).ref().Inputs(7U, 2U);

            INST(14U, Opcode::SaveState).Inputs(2U).SrcVregs({0U});
            INST(4U, Opcode::LoadString).ref().Inputs(14U).TypeId(myStrId);
            INST(11U, Opcode::SaveState).Inputs(4U).SrcVregs({0U});
            INST(8U, Opcode::LoadAndInitClass).ref().Inputs(11U);
            INST(5U, Opcode::StoreStatic).ref().Inputs(8U, 4U);

            INST(21U, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(0U), &INS(0U)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(0U), &INS(2U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(0U), &INS(4U)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(2U), &INS(4U)), AliasType::NO_ALIAS);
}

/*
 *    .function R1 foo(R1[][] a0, i64 a1) {
 *        lda a1
 *        ldarr.obj a0
 *        sta.obj v0
 *        lda a1
 *        ldarr.obj v0
 *        return.obj
 *    }
 *
 */
TEST_F(AliasAnalysisTest, NestedArrays)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::LoadArray).ref().Inputs(0U, 2U);
            INST(12U, Opcode::LoadArray).ref().Inputs(7U, 2U);
            INST(13U, Opcode::Return).ref().Inputs(12U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(7U), &INS(7U)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(7U), &INS(12U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(12U), &INS(12U)), AliasType::MUST_ALIAS);
}

/// Nested arrays with immediate indices.
TEST_F(AliasAnalysisTest, NestedArraysImm)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).s64();
        CONSTANT(5U, 0x2aU).s64();
        CONSTANT(6U, 0x3aU).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::LoadArray).ref().Inputs(0U, 2U);
            INST(12U, Opcode::LoadArrayI).ref().Inputs(7U).Imm(5U);
            INST(13U, Opcode::LoadArrayI).ref().Inputs(7U).Imm(6U);
            INST(14U, Opcode::SaveState).Inputs(0U, 12U, 13U).SrcVregs({0U, 1U, 2U});
            INST(15U, Opcode::NullCheck).ref().Inputs(12U, 14U);
            INST(16U, Opcode::NullCheck).ref().Inputs(13U, 14U);
            INST(17U, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckRefAlias(&INS(12U), &INS(13U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(7U), &INS(12U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(7U), &INS(13U)), AliasType::MAY_ALIAS);
}

/*
 *    .function void foo (i64[] a0, i64 a1, i64 a2) {
 *        call.short GetArray
 *        sta.obj v0
 *        lda a2
 *        starr.64 v0, a1
 *        starr.64 a0, a1
 *        return.void
 *    }
 *
 * References obtained from calls are like parameters
 */
TEST_F(AliasAnalysisTest, StaticCall)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        PARAMETER(2U, 2U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 2U).SrcVregs({1U, 3U});
            INST(4U, Opcode::CallStatic).ref().Inputs({{DataType::NO_TYPE, 3U}});
            INST(9U, Opcode::StoreArray).u64().Inputs(4U, 1U, 2U);
            INST(14U, Opcode::StoreArray).u64().Inputs(0U, 1U, 2U);
            INST(15U, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(9U), &INS(14U)), AliasType::MAY_ALIAS);
}

/*
 *    .record R1 {
 *        ref f
 *    }
 *    .function void init3(R1[] a0, R1 a1) {
 *        movi v0, 23
 *        movi v2, 39
 *        lda v0
 *        ldarr.obj a0
 *        sta.obj v1
 *        lda v2
 *        ldarr.obj a0
 *        sta.obj v3
 *        ldobj.64 a1, R1.f
 *        stobj.64 v1, R1.f
 *        stobj.64 v3, R1.f
 *        return.void
 *    }
 *
 * Reference came from Load with immediate index
 */
TEST_F(AliasAnalysisTest, ImmediateRefLoad)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(16U, Opcode::LoadArrayI).ref().Inputs(0U).Imm(0x17U);
            INST(17U, Opcode::LoadArrayI).ref().Inputs(0U).Imm(0x27U);
            INST(10U, Opcode::LoadObject).ref().Inputs(1U).TypeId(1U);
            INST(13U, Opcode::StoreObject).ref().Inputs(16U, 10U).TypeId(1U);
            INST(14U, Opcode::StoreObject).ref().Inputs(17U, 10U).TypeId(1U);
            INST(15U, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(10U), &INS(13U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckRefAlias(&INS(16U), &INS(17U)), AliasType::MAY_ALIAS);
}

/* Null pointer dereferences. */
TEST_F(AliasAnalysisTest, NullPtrLoads)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, nullptr).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::LoadArrayI).u64().Inputs(0U).Imm(0U);
            INST(2U, Opcode::LoadArrayI).u64().Inputs(0U).Imm(0U);
            INST(3U, Opcode::Add).u64().Inputs(1U, 2U);
            INST(4U, Opcode::Return).u64().Inputs(3U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(1U), &INS(2U)), AliasType::MUST_ALIAS);
}

/* NullCheck in Phi inputs. */
TEST_F(AliasAnalysisTest, NullCheckPhiInput)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        CONSTANT(7U, nullptr).ref();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(8U, Opcode::Compare).b().Inputs(1U, 7U);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(8U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 7U).SrcVregs({0U, 1U, 2U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 5U);
            INST(3U, Opcode::NullCheck).ref().Inputs(1U, 5U);
            INST(4U, Opcode::CallVirtual)
                .v0id()
                .Inputs({{DataType::REFERENCE, 2U}, {DataType::REFERENCE, 3U}, {DataType::NO_TYPE, 5U}});
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::Phi).ref().Inputs({{3U, 3U}, {2U, 7U}});
            INST(11U, Opcode::LoadArrayI).u64().Inputs(10U).Imm(0U);
            INST(12U, Opcode::LoadArrayI).u64().Inputs(0U).Imm(0U);
            INST(13U, Opcode::Add).u64().Inputs(11U, 12U);
            INST(14U, Opcode::Return).u64().Inputs(13U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(11U), &INS(12U)), AliasType::MAY_ALIAS);
}

/// Select instruction is very similar to Phi.
TEST_F(AliasAnalysisTest, Select)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        CONSTANT(2U, 10U).s64();
        CONSTANT(3U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(13U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(13U).TypeId(68U);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 2U, 13U);
            INST(5U, Opcode::SelectImm)
                .ref()
                .SrcType(DataType::BOOL)
                .CC(CC_NE)
                .Imm(3U)
                .Inputs(0U, 1U, 3U)
                .SetFlag(compiler::inst_flags::NO_CSE)
                .SetFlag(compiler::inst_flags::NO_HOIST);
            INST(6U, Opcode::SelectImm)
                .ref()
                .SrcType(DataType::BOOL)
                .CC(CC_NE)
                .Imm(3U)
                .Inputs(5U, 4U, 3U)
                .SetFlag(compiler::inst_flags::NO_CSE)
                .SetFlag(compiler::inst_flags::NO_HOIST);
            INST(10U, Opcode::Return).ref().Inputs(6U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckRefAlias(&INS(4U), &INS(5U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckRefAlias(&INS(4U), &INS(6U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckRefAlias(&INS(0U), &INS(5U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckRefAlias(&INS(1U), &INS(5U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckRefAlias(&INS(0U), &INS(6U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckRefAlias(&INS(1U), &INS(6U)), AliasType::MAY_ALIAS);
}

TEST_F(AliasAnalysisTest, SomeKindsLoadArray)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();

        CONSTANT(2U, 0U).s64();
        CONSTANT(3U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(12U, Opcode::LoadArrayPair).s32().Inputs(0U, 1U);
            INST(14U, Opcode::LoadPairPart).s32().Inputs(12U).Imm(0x0U);
            INST(13U, Opcode::LoadPairPart).s32().Inputs(12U).Imm(0x1U);

            INST(15U, Opcode::LoadArrayI).s32().Imm(0U).Inputs(0U);
            INST(16U, Opcode::LoadArray).s32().Inputs(0U, 2U);

            INST(19U, Opcode::LoadArrayI).s32().Imm(1U).Inputs(0U);
            INST(20U, Opcode::LoadArray).s32().Inputs(0U, 3U);

            INST(17U, Opcode::LoadArray).s32().Inputs(0U, 1U);
            INST(18U, Opcode::Return).s32().Inputs(17U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(14U), &INS(15U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(15U), &INS(16U)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(19U), &INS(20U)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(15U), &INS(20U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(16U), &INS(19U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(15U), &INS(17U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(20U), &INS(17U)), AliasType::MAY_ALIAS);
}

TEST_F(AliasAnalysisTest, LoadPairObject)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s64();
        BASIC_BLOCK(2U, -1L)
        {
            INST(12U, Opcode::LoadArrayPair).ref().Inputs(0U, 1U);
            INST(14U, Opcode::LoadPairPart).ref().Inputs(12U).Imm(0x0U);
            INST(13U, Opcode::LoadPairPart).ref().Inputs(12U).Imm(0x1U);
            INST(15U, Opcode::LoadObject).s32().Inputs(13U).TypeId(42U);
            INST(16U, Opcode::LoadObject).s32().Inputs(14U).TypeId(42U);
            INST(17U, Opcode::Add).s32().Inputs(15U, 16U);
            INST(18U, Opcode::Return).s32().Inputs(17U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(15U), &INS(16U)), AliasType::MAY_ALIAS);
}

TEST_F(AliasAnalysisTest, CatchPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Try).CatchTypeIds({0xE1U});
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(5U, Opcode::LoadObject).ref().Inputs(4U).TypeId(122U);
            INST(6U, Opcode::SaveState).Inputs(0U, 5U).SrcVregs({0U, 5U});
            INST(7U, Opcode::NullCheck).ref().Inputs(5U, 6U);
            INST(9U, Opcode::Return).ref().Inputs(7U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::CatchPhi).ref().Inputs(0U, 5U);
            INST(11U, Opcode::Return).ref().Inputs(10U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckRefAlias(&INS(10U), &INS(0U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckRefAlias(&INS(10U), &INS(5U)), AliasType::MAY_ALIAS);
}

/// Test the assert that NullCheck is bypassed through RefTypeCheck
TEST_F(AliasAnalysisTest, RefTypeCheck)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, 1U)
        {
            INST(10U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(11U, Opcode::NullCheck).ref().Inputs(1U, 10U);
            INST(14U, Opcode::RefTypeCheck).ref().Inputs(0U, 11U, 10U);
            INST(15U, Opcode::StoreArrayI).ref().Imm(0U).Inputs(0U, 14U);

            INST(6U, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(15U), &INS(15U)), AliasType::MUST_ALIAS);
}

/// The field access is determined by field reference (if present) rather than TypeId.
TEST_F(AliasAnalysisTest, InheritedFields)
{
    RuntimeInterface::FieldPtr field = reinterpret_cast<void *>(0xDEADBEEFU);  // NOLINT(modernize-use-auto)
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, 1U)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::LoadStatic).ref().Inputs(2U).TypeId(377U).ObjField(field);
            INST(4U, Opcode::StoreStatic).ref().Inputs(2U, 3U).TypeId(378U).ObjField(field);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(3U), &INS(4U)), AliasType::MUST_ALIAS);
}

TEST_F(AliasAnalysisTest, PairedAccessesAliasing)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(10U, Opcode::LoadArrayPairI).ref().Imm(0x0U).Inputs(0U);
            INST(11U, Opcode::LoadPairPart).ref().Imm(0x0U).Inputs(10U);
            INST(12U, Opcode::LoadPairPart).ref().Imm(0x1U).Inputs(10U);
            INST(14U, Opcode::LoadObject).ref().TypeId(3005U).Inputs(12U);
            INST(17U, Opcode::StoreArrayI).s32().Imm(0x0U).Inputs(14U, 1U);
            INST(18U, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(17U), &INS(17U)), AliasType::MUST_ALIAS);
}

/// Volatile load does not quarantee that the same value would be loaded next time.
TEST_F(AliasAnalysisTest, VolatileAliasing)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, 1U)
        {
            INST(11U, Opcode::LoadObject).ref().TypeId(3005U).Volatile().Inputs(0U);
            INST(12U, Opcode::LoadObject).ref().TypeId(3005U).Volatile().Inputs(0U);
            INST(13U, Opcode::LoadObject).s32().TypeId(4005U).Inputs(11U);
            INST(14U, Opcode::LoadObject).s32().TypeId(4005U).Inputs(12U);
            INST(15U, Opcode::Add).s32().Inputs(13U, 14U);

            INST(16U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(17U, Opcode::LoadAndInitClass).ref().Inputs(16U).TypeId(0U);
            INST(18U, Opcode::LoadStatic).ref().TypeId(5005U).Volatile().Inputs(17U);
            INST(19U, Opcode::LoadStatic).ref().TypeId(5005U).Volatile().Inputs(17U);
            INST(20U, Opcode::LoadObject).s32().TypeId(4005U).Inputs(18U);
            INST(21U, Opcode::LoadObject).s32().TypeId(4005U).Inputs(19U);
            INST(22U, Opcode::Add).s32().Inputs(20U, 21U);
            INST(23U, Opcode::Add).s32().Inputs(15U, 22U);
            INST(24U, Opcode::Return).s32().Inputs(23U);
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(13U), &INS(14U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(20U), &INS(21U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(13U), &INS(21U)), AliasType::MAY_ALIAS);
}

/// LoadObject with ObjectType == MEM_STATIC is treatet the same as LoadStatic

TEST_F(AliasAnalysisTest, LoadObjectStatic)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(10U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(7U, Opcode::LoadAndInitClass).ref().Inputs(10U).TypeId(0U);
            INST(8U, Opcode::LoadAndInitClass).ref().Inputs(10U).TypeId(0U);
            INST(9U, Opcode::LoadAndInitClass).ref().Inputs(10U).TypeId(0U);
            INST(1U, Opcode::LoadStatic).ref().Inputs(7U).TypeId(349U);
            INST(2U, Opcode::LoadObject).ref().Inputs(7U).TypeId(349U).ObjectType(ObjectType::MEM_STATIC);
            INST(3U, Opcode::LoadObject).ref().Inputs(8U).TypeId(377U).ObjectType(ObjectType::MEM_STATIC);
            INST(4U, Opcode::StoreStatic).ref().Inputs(8U, 1U).TypeId(377U);
            INST(5U, Opcode::StoreObject).ref().Inputs(8U, 1U).TypeId(377U).ObjectType(ObjectType::MEM_STATIC);
            INST(6U, Opcode::StoreObject).ref().Inputs(9U, 2U).TypeId(363U).ObjectType(ObjectType::MEM_STATIC);
            INST(25U, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(1U), &INS(2U)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(4U), &INS(5U)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(3U), &INS(4U)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(2U), &INS(3U)), AliasType::NO_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(5U), &INS(6U)), AliasType::NO_ALIAS);
}

// CC-OFFNXT(huge_method, G.FUN.01) graph creation
void AliasAnalysisTest::BuildGraphDynamicMethods(Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).any();
        PARAMETER(1U, 1U).any();
        PARAMETER(2U, 2U).any();
        PARAMETER(3U, 3U).any();
        CONSTANT(4U, 2U).any();
        CONSTANT(5U, 0x2aU).any();
        BASIC_BLOCK(2U, -1L)
        {
            INST(10U, Opcode::LoadObjectDynamic)
                .any()
                .Inputs(0U, 1U, 0U)
                .SetAccessType(DynObjectAccessType::BY_INDEX)
                .SetAccessMode(DynObjectAccessMode::ARRAY);
            INST(11U, Opcode::StoreObjectDynamic)
                .any()
                .Inputs(0U, 1U, 5U, 0U)
                .SetAccessType(DynObjectAccessType::BY_INDEX)
                .SetAccessMode(DynObjectAccessMode::ARRAY);
            INST(12U, Opcode::StoreObjectDynamic)
                .any()
                .Inputs(0U, 4U, 10U, 0U)
                .SetAccessType(DynObjectAccessType::BY_INDEX)
                .SetAccessMode(DynObjectAccessMode::ARRAY);
            INST(13U, Opcode::StoreObjectDynamic)
                .any()
                .Inputs(0U, 5U, 10U, 0U)
                .SetAccessType(DynObjectAccessType::BY_INDEX)
                .SetAccessMode(DynObjectAccessMode::ARRAY);

            INST(14U, Opcode::LoadObjectDynamic)
                .any()
                .Inputs(0U, 1U, 0U)
                .SetAccessType(DynObjectAccessType::BY_INDEX)
                .SetAccessMode(DynObjectAccessMode::DICTIONARY);

            INST(15U, Opcode::LoadObjectDynamic)
                .any()
                .Inputs(2U, 3U, 0U)
                .SetAccessType(DynObjectAccessType::BY_INDEX)
                .SetAccessMode(DynObjectAccessMode::DICTIONARY);
            INST(16U, Opcode::StoreObjectDynamic)
                .any()
                .Inputs(2U, 3U, 14U, 0U)
                .SetAccessType(DynObjectAccessType::BY_NAME)
                .SetAccessMode(DynObjectAccessMode::DICTIONARY);
            INST(17U, Opcode::LoadObjectDynamic)
                .any()
                .Inputs(2U, 4U, 0U)
                .SetAccessType(DynObjectAccessType::BY_NAME)
                .SetAccessMode(DynObjectAccessMode::DICTIONARY);
            INST(18U, Opcode::StoreObjectDynamic)
                .any()
                .Inputs(2U, 5U, 16U, 0U)
                .SetAccessType(DynObjectAccessType::BY_INDEX)
                .SetAccessMode(DynObjectAccessMode::DICTIONARY);

            INST(25U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(AliasAnalysisTest, DynamicMethods)
{
    auto graph = CreateDynEmptyGraph();
    BuildGraphDynamicMethods(graph);

    graph->RunPass<AliasAnalysis>();
    EXPECT_TRUE(graph->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(graph).Check();

    auto &alias = graph->GetAnalysis<AliasAnalysis>();
    ASSERT_EQ(alias.CheckInstAlias(&INS(10U), &INS(11U)), AliasType::MUST_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(10U), &INS(12U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(12U), &INS(13U)), AliasType::NO_ALIAS);

    ASSERT_EQ(alias.CheckInstAlias(&INS(10U), &INS(14U)), AliasType::NO_ALIAS);

    ASSERT_EQ(alias.CheckInstAlias(&INS(14U), &INS(15U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(15U), &INS(16U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(15U), &INS(18U)), AliasType::MAY_ALIAS);
    ASSERT_EQ(alias.CheckInstAlias(&INS(16U), &INS(17U)), AliasType::MAY_ALIAS);
}

TEST_F(AliasAnalysisTest, LocalObjects)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(0U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(5U, Opcode::SaveState).Inputs(0U, 3U, 4U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::CallStatic).v0id().InputsAutoType(3U, 4U, 5U);
            INST(25U, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    EXPECT_FALSE(alias.GetPointerInfo(Pointer::CreateObject(&INS(0U))).IsLocalCreated());
    EXPECT_FALSE(alias.GetPointerInfo(Pointer::CreateObject(&INS(0U))).IsLocal());

    EXPECT_TRUE(alias.GetPointerInfo(Pointer::CreateObject(&INS(3U))).IsLocalCreated());
    EXPECT_FALSE(alias.GetPointerInfo(Pointer::CreateObject(&INS(3U))).IsLocal());

    EXPECT_EQ(alias.CheckRefAlias(&INS(3U), &INS(4U)), AliasType::NO_ALIAS);
    EXPECT_EQ(alias.CheckRefAlias(&INS(0U), &INS(4U)), AliasType::MAY_ALIAS);  // can be NO_ALIAS but ok
}

TEST_F(AliasAnalysisTest, ComplexEscape)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(0U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(7U, Opcode::StoreObject).ref().Inputs(3U, 4U).TypeId(0U);
            INST(8U, Opcode::IfImm).SrcType(DataType::REFERENCE).CC(CC_NE).Imm(0).Inputs(0U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, -1)
        {
            INST(9U, Opcode::Phi).ref().Inputs(3U, 0U);
            INST(10U, Opcode::LoadObject).ref().Inputs(9U).TypeId(0U);
            INST(5U, Opcode::SaveState).Inputs(0U, 3U, 4U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::CallStatic).ref().InputsAutoType(10U, 5U);
            INST(25U, Opcode::ReturnVoid).v0id();
        }
    }

    GetGraph()->RunPass<AliasAnalysis>();
    EXPECT_TRUE(GetGraph()->IsAnalysisValid<AliasAnalysis>());
    GraphChecker(GetGraph()).Check();

    auto &alias = GetGraph()->GetAnalysis<AliasAnalysis>();
    EXPECT_TRUE(alias.GetPointerInfo(Pointer::CreateObject(&INS(4U))).IsLocalCreated());
    // inst 4 escapes: 4 -> 7 -> 9p -> 10 -> 6
    EXPECT_FALSE(alias.GetPointerInfo(Pointer::CreateObject(&INS(4U))).IsLocal());
    EXPECT_FALSE(alias.GetPointerInfo(Pointer::CreateObject(&INS(3U))).IsLocal());  // can be true but ok

    EXPECT_EQ(alias.CheckRefAlias(&INS(4U), &INS(6U)), AliasType::MAY_ALIAS);
    EXPECT_EQ(alias.CheckRefAlias(&INS(3U), &INS(6U)), AliasType::MAY_ALIAS);  // can be NO_ALIAS but ok
    EXPECT_EQ(alias.CheckRefAlias(&INS(0U), &INS(6U)), AliasType::MAY_ALIAS);
}

// NOLINTEND(readability-magic-numbers)

}  //  namespace ark::compiler
