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
#include "optimizer/optimizations/balance_expressions.h"

namespace ark::compiler {
class BalanceExpressionsTest : public GraphTest {
public:
    void AddMulParallelBuildGraph();
};

// NOLINTBEGIN(readability-magic-numbers)

void BalanceExpressionsTest::AddMulParallelBuildGraph()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        PARAMETER(3U, 3U).u64();
        PARAMETER(4U, 4U).u64();
        PARAMETER(5U, 5U).u64();
        PARAMETER(6U, 6U).u64();
        PARAMETER(7U, 7U).u64();

        /**
         * From:
         *  (((((((a + b) + c) + d) + e) + f) + g) + h)
         *  interlined with
         *  (((((((a * b) * c) * d) * e) * f) * g) * h)
         *
         *  (Critical path is 8)
         */
        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(9U, Opcode::Mul).u64().Inputs(0U, 1U);

            INST(10U, Opcode::Add).u64().Inputs(8U, 2U);
            INST(11U, Opcode::Mul).u64().Inputs(9U, 2U);

            INST(12U, Opcode::Add).u64().Inputs(10U, 3U);
            INST(13U, Opcode::Mul).u64().Inputs(11U, 3U);

            INST(14U, Opcode::Add).u64().Inputs(12U, 4U);
            INST(15U, Opcode::Mul).u64().Inputs(13U, 4U);

            INST(16U, Opcode::Add).u64().Inputs(14U, 5U);
            INST(17U, Opcode::Mul).u64().Inputs(15U, 5U);

            INST(18U, Opcode::Add).u64().Inputs(16U, 6U);
            INST(19U, Opcode::Mul).u64().Inputs(17U, 6U);

            INST(20U, Opcode::Add).u64().Inputs(18U, 7U);
            INST(21U, Opcode::Mul).u64().Inputs(19U, 7U);

            INST(22U, Opcode::Mul).u64().Inputs(21U, 20U);
            INST(23U, Opcode::Return).u64().Inputs(22U);
        }
    }
}

TEST_F(BalanceExpressionsTest, AddMulParallel)
{
    // Check that independent expression are not mixed with each other and being considered sequentially:
    AddMulParallelBuildGraph();

    ASSERT_TRUE(GetGraph()->RunPass<BalanceExpressions>());
    ASSERT_TRUE(CheckUsers(INS(20U), {22U}));
    ASSERT_TRUE(CheckUsers(INS(22U), {23U}));

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        PARAMETER(3U, 3U).u64();
        PARAMETER(4U, 4U).u64();
        PARAMETER(5U, 5U).u64();
        PARAMETER(6U, 6U).u64();
        PARAMETER(7U, 7U).u64();

        /**
         * To:
         *  (((a + b) + (c + d)) + ((e + f) + (g + h)))
         *  followed by
         *  (((a * b) * (c * d)) * ((e * f) * (g * h)))
         *
         *  (Critical path is 4)
         */
        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(10U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(12U, Opcode::Add).u64().Inputs(8U, 10U);
            INST(14U, Opcode::Add).u64().Inputs(4U, 5U);
            INST(16U, Opcode::Add).u64().Inputs(6U, 7U);
            INST(18U, Opcode::Add).u64().Inputs(14U, 16U);
            INST(20U, Opcode::Add).u64().Inputs(12U, 18U);

            INST(9U, Opcode::Mul).u64().Inputs(0U, 1U);
            INST(11U, Opcode::Mul).u64().Inputs(2U, 3U);
            INST(13U, Opcode::Mul).u64().Inputs(9U, 11U);
            INST(15U, Opcode::Mul).u64().Inputs(4U, 5U);
            INST(17U, Opcode::Mul).u64().Inputs(6U, 7U);
            INST(19U, Opcode::Mul).u64().Inputs(15U, 17U);
            INST(21U, Opcode::Mul).u64().Inputs(13U, 19U);

            INST(22U, Opcode::Mul).u64().Inputs(21U, 20U);
            INST(23U, Opcode::Return).u64().Inputs(22U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(BalanceExpressionsTest, MultipleUsers)
{
    // Instruction with more than one user may create side effects, so it is needed to check that they are considered as
    // sources (thus would not be modified).
    // Also checks that the last operator of an expression has the same users as before its optimization:
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        PARAMETER(3U, 3U).u64();
        PARAMETER(4U, 4U).u64();
        PARAMETER(5U, 5U).u64();
        PARAMETER(6U, 6U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(9U, Opcode::Add).u64().Inputs(2U, 8U);

            // Has multiple users:
            INST(10U, Opcode::Add).u64().Inputs(3U, 9U);

            INST(11U, Opcode::Add).u64().Inputs(4U, 10U);
            INST(12U, Opcode::Add).u64().Inputs(5U, 6U);
            INST(13U, Opcode::Add).u64().Inputs(11U, 12U);
            INST(14U, Opcode::Add).u64().Inputs(10U, 13U);

            INST(15U, Opcode::Return).u64().Inputs(14U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<BalanceExpressions>());

    // The same users as we expect that the second expression would be unchanged
    ASSERT_TRUE(CheckUsers(INS(10U), {11U, 14U}));
    ASSERT_TRUE(CheckUsers(INS(14U), {15U}));

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        PARAMETER(3U, 3U).u64();
        PARAMETER(4U, 4U).u64();
        PARAMETER(5U, 5U).u64();
        PARAMETER(6U, 6U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::Add).u64().Inputs(3U, 2U);
            INST(9U, Opcode::Add).u64().Inputs(0U, 1U);

            // Has multiple users:
            INST(10U, Opcode::Add).u64().Inputs(8U, 9U);

            INST(11U, Opcode::Add).u64().Inputs(4U, 10U);
            INST(12U, Opcode::Add).u64().Inputs(5U, 6U);
            INST(13U, Opcode::Add).u64().Inputs(11U, 12U);
            INST(14U, Opcode::Add).u64().Inputs(10U, 13U);

            INST(15U, Opcode::Return).u64().Inputs(14U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(BalanceExpressionsTest, SameSource)
{
    // Check that expression with repeated sources are handled correctly:
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Add).u64().Inputs(0U, 0U);
            INST(2U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Add).u64().Inputs(0U, 2U);

            INST(4U, Opcode::Return).u64().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<BalanceExpressions>());
    ASSERT_TRUE(CheckUsers(INS(3U), {4U}));

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Add).u64().Inputs(0U, 0U);
            INST(2U, Opcode::Add).u64().Inputs(0U, 0U);
            INST(3U, Opcode::Add).u64().Inputs(1U, 2U);

            INST(4U, Opcode::Return).u64().Inputs(3U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(BalanceExpressionsTest, OddSources)
{
    // Check that expression with odd number of sources are handled correctly:
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        PARAMETER(3U, 3U).u64();
        PARAMETER(4U, 4U).u64();

        /**
         * From:
         *  (a + (e + ((c + d) + b)))
         *
         *  (Critical path is 4)
         */
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(6U, Opcode::Add).u64().Inputs(5U, 1U);
            INST(7U, Opcode::Add).u64().Inputs(4U, 6U);
            INST(8U, Opcode::Add).u64().Inputs(0U, 7U);

            INST(9U, Opcode::Return).u64().Inputs(8U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<BalanceExpressions>());
    ASSERT_TRUE(CheckUsers(INS(8U), {9U}));

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        PARAMETER(3U, 3U).u64();
        PARAMETER(4U, 4U).u64();

        /**
         * To:
         *  (((a + e) + (c + d)) + b)
         *
         *  (Critical path is 3)
         */
        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::Add).u64().Inputs(0U, 4U);
            INST(6U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(7U, Opcode::Add).u64().Inputs(5U, 6U);
            INST(8U, Opcode::Add).u64().Inputs(7U, 1U);

            INST(9U, Opcode::Return).u64().Inputs(8U);
        }
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
