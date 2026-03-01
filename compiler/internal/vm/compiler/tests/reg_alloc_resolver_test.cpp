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
#include <utility>
#include <vector>

#include "graph_test.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/ir/constants.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/locations.h"
#include "optimizer/optimizations/regalloc/reg_alloc_resolver.h"
#include "reg_acc_alloc.h"
#include "utils/arena_containers.h"

using namespace testing::ext;

namespace panda::compiler {
class RegAllocResolverTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    GraphTest graph_test_;

    template<typename Callback>
    static void ForEachInst(Graph *graph, Callback cb)
    {
        ASSERT(graph != nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                cb(inst);
            }
        }
    }

    static bool IsIntrinsic(Inst *inst, IntrinsicInst::IntrinsicId id)
    {
        return inst->IsIntrinsic() && inst->CastToIntrinsic()->GetIntrinsicId() == id;
    }

    static void InitUsedRegs(Graph *graph, size_t count)
    {
        ASSERT(graph != nullptr);
        ArenaVector<bool> used_regs(count, false, graph->GetAllocator()->Adapter());
        graph->InitUsedRegs<DataType::INT64>(&used_regs);
    }
};

/**
 * @tc.name: reg_alloc_resolver_test_001
 * @tc.desc: Verify the AddMoveToFixedLocation function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocResolverTest, reg_alloc_resolver_test_001, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func4";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        graph->RunPass<LivenessAnalyzer>();
        auto intervals = graph->GetAnalysis<LivenessAnalyzer>().GetLifeIntervals();

        graph->RunPass<bytecodeopt::RegAccAlloc>();

        // Allocate registers
        Register preassign_count = 0;
        for (auto interval : intervals) {
            interval->SetPreassignedReg(preassign_count++);
        }
        
        // Make some insts require a fixed input register, then spillfill insts are expected to be created.
        Register fixed_input_reg = preassign_count;
        std::vector<std::pair<Inst*, Register>> pairs;
        ForEachInst(graph, [&pairs, &fixed_input_reg](Inst *inst) {
            if (IsIntrinsic(inst, IntrinsicInst::IntrinsicId::ADD2_IMM8_V8) ||
                IsIntrinsic(inst, IntrinsicInst::IntrinsicId::SUB2_IMM8_V8)) {
                EXPECT_GE(inst->GetInputsCount(), 2);

                inst->SetLocation(0, Location::MakeRegister(fixed_input_reg));
                inst->SetLocation(1, Location::MakeRegister(fixed_input_reg + 1));
                pairs.emplace_back(inst, fixed_input_reg);
                fixed_input_reg += 2;
            }
        });
        EXPECT_FALSE(pairs.empty());

        // Run resolver
        InitUsedRegs(graph, 256);
        RegAllocResolver(graph).Resolve();

        for (auto [inst, input_reg] : pairs) {
            EXPECT_EQ(inst->GetSrcReg(0), input_reg);
            EXPECT_EQ(inst->GetSrcReg(1), input_reg + 1);

            auto sf_inst = inst->GetPrev();
            EXPECT_TRUE(sf_inst != nullptr);
            EXPECT_TRUE(sf_inst->IsSpillFill());
            EXPECT_EQ(sf_inst->CastToSpillFill()->GetSpillFillType(), SpillFillType::INPUT_FILL);
            
            auto sf_data1 = sf_inst->CastToSpillFill()->GetSpillFill(0);
            EXPECT_EQ(sf_data1.GetSrc(), inst->GetInput(0).GetInst()->GetDstLocation());
            EXPECT_EQ(sf_data1.GetDst(), inst->GetLocation(0));

            auto sf_data2 = sf_inst->CastToSpillFill()->GetSpillFill(1);
            EXPECT_EQ(sf_data2.GetSrc(), inst->GetInput(1).GetInst()->GetDstLocation());
            EXPECT_EQ(sf_data2.GetDst(), inst->GetLocation(1));
        }

        status = true;
    });
    EXPECT_TRUE(status);
}
}  // namespace panda::compiler

