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

#include <gtest/gtest.h>

#include "graph_test.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/constants.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/spill_fill_data.h"
#include "optimizer/optimizations/regalloc/spill_fills_resolver.h"
#include "tests/graph_comparator.h"
#include "utils/arch.h"
#include "utils/arena_containers.h"

using namespace testing::ext;

namespace panda::compiler {
class SpillFillsResolverTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    static Graph* CreateGraphWithStart(Arch arch = Arch::NONE)
    {
        auto allocator = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        auto local_allocator = new ArenaAllocator(SpaceType::SPACE_TYPE_COMPILER);
        auto graph = allocator->New<Graph>(allocator, local_allocator, arch);
        graph->CreateStartBlock();
        return graph;
    }

    static void InitUsedRegs(Graph *graph, size_t count)
    {
        ASSERT(graph != nullptr);
        ArenaVector<bool> used_regs(count, false, graph->GetAllocator()->Adapter());
        graph->InitUsedRegs<DataType::INT64>(&used_regs);
    }
};

/**
 * @tc.name: spill_fills_resolver_test_001
 * @tc.desc: Verify resolve move overwrite.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SpillFillsResolverTest, spill_fills_resolver_test_001, TestSize.Level1)
{
    auto graph = CreateGraphWithStart();
    auto sf_inst = graph->CreateInstSpillFill();
    sf_inst->AddMove(0, 1, DataType::INT32);
    sf_inst->AddMove(1, 2, DataType::INT32);

    auto expect_sf_inst = graph->CreateInstSpillFill();
    expect_sf_inst->AddMove(1, 2, DataType::INT32);
    expect_sf_inst->AddMove(0, 1, DataType::INT32);

    SpillFillsResolver resolver(graph);
    resolver.VisitInstruction(sf_inst);

    EXPECT_TRUE(GraphComparator().Compare(sf_inst, expect_sf_inst));
}

/**
 * @tc.name: spill_fills_resolver_test_002
 * @tc.desc: Verify resolve stack overwrite.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SpillFillsResolverTest, spill_fills_resolver_test_002, TestSize.Level1)
{
    auto graph = CreateGraphWithStart();
    graph->SetStackSlotsCount(3);
    auto sf_inst = graph->CreateInstSpillFill();
    sf_inst->AddMemCopy(0, 1, DataType::FLOAT64);
    sf_inst->AddMemCopy(1, 2, DataType::FLOAT64);

    auto expect_sf_inst = graph->CreateInstSpillFill();
    expect_sf_inst->AddMemCopy(1, 2, DataType::FLOAT64);
    expect_sf_inst->AddMemCopy(0, 1, DataType::FLOAT64);

    SpillFillsResolver resolver(graph, INVALID_REG, 0, 3);
    resolver.ResolveIfRequired(sf_inst);

    EXPECT_TRUE(GraphComparator().Compare(sf_inst, expect_sf_inst));
}

/**
 * @tc.name: spill_fills_resolver_test_003
 * @tc.desc: Verify resolve cyclic move overwrite.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SpillFillsResolverTest, spill_fills_resolver_test_003, TestSize.Level1)
{
    auto graph = CreateGraphWithStart();
    auto sf_inst = graph->CreateInstSpillFill();
    sf_inst->AddMove(3, 1, DataType::FLOAT64);
    sf_inst->AddMove(2, 3, DataType::FLOAT64);
    sf_inst->AddMove(1, 2, DataType::FLOAT64);

    auto expect_sf_inst = graph->CreateInstSpillFill();
    expect_sf_inst->AddMove(1, 0, DataType::FLOAT64);
    expect_sf_inst->AddMove(3, 1, DataType::FLOAT64);
    expect_sf_inst->AddMove(2, 3, DataType::FLOAT64);
    expect_sf_inst->AddMove(0, 2, DataType::FLOAT64);

    SpillFillsResolver resolver(graph, INVALID_REG, 0, 5);
    resolver.VisitInstruction(sf_inst);

    EXPECT_TRUE(GraphComparator().Compare(sf_inst, expect_sf_inst));
}

/**
 * @tc.name: spill_fills_resolver_test_004
 * @tc.desc: Verify resolve cyclic move overwrite with preassigned resolver register.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SpillFillsResolverTest, spill_fills_resolver_test_004, TestSize.Level1)
{
    auto graph = CreateGraphWithStart();
    InitUsedRegs(graph, 6);
    auto sf_inst = graph->CreateInstSpillFill();
    sf_inst->AddMove(3, 1, DataType::INT32);
    sf_inst->AddMove(2, 3, DataType::INT32);
    sf_inst->AddMove(1, 2, DataType::INT32);

    auto expect_sf_inst = graph->CreateInstSpillFill();
    expect_sf_inst->AddMove(1, 5, DataType::INT32);
    expect_sf_inst->AddMove(3, 1, DataType::INT32);
    expect_sf_inst->AddMove(2, 3, DataType::INT32);
    expect_sf_inst->AddMove(5, 2, DataType::INT32);

    SpillFillsResolver resolver(graph, 5, 6, 0);
    resolver.VisitInstruction(sf_inst);

    EXPECT_TRUE(GraphComparator().Compare(sf_inst, expect_sf_inst));
}

/**
 * @tc.name: spill_fills_resolver_test_005
 * @tc.desc: Verify resolve cyclic move overwrite on AARCH32.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SpillFillsResolverTest, spill_fills_resolver_test_005, TestSize.Level1)
{
    auto graph = CreateGraphWithStart(Arch::AARCH32);
    graph->SetStackSlotsCount(1);
    auto sf_inst = graph->CreateInstSpillFill();
    sf_inst->AddMove(3, 1, DataType::FLOAT64);
    sf_inst->AddMove(2, 3, DataType::FLOAT64);
    sf_inst->AddMove(1, 2, DataType::FLOAT64);

    auto expect_sf_inst = graph->CreateInstSpillFill();
    expect_sf_inst->AddSpill(1, 0, DataType::FLOAT64);
    expect_sf_inst->AddMove(3, 1, DataType::FLOAT64);
    expect_sf_inst->AddMove(2, 3, DataType::FLOAT64);
    expect_sf_inst->AddFill(0, 2, DataType::FLOAT64);

    SpillFillsResolver resolver(graph, INVALID_REG, 0, 5);
    resolver.VisitInstruction(sf_inst);

    EXPECT_TRUE(GraphComparator().Compare(sf_inst, expect_sf_inst));
}
}  // namespace panda::compiler

