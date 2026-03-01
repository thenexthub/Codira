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

#include <algorithm>
#include <gtest/gtest.h>
#include <utility>

#include "graph_test.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/constants.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/locations.h"
#include "optimizer/optimizations/regalloc/split_resolver.h"
#include "reg_acc_alloc.h"

using namespace testing::ext;

namespace panda::compiler {
class SplitResolverTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    GraphTest graph_test_;

    template<typename Predicate>
    static Inst* FindInstInBlock(BasicBlock *bb, Predicate f)
    {
        ASSERT(bb != nullptr);
        auto it = std::find_if(bb->AllInsts().begin(), bb->AllInsts().end(), f);
        if (it != bb->AllInsts().end()) {
            return *it;
        }
        return nullptr;
    }

    template<typename Predicate>
    static Inst* FindInstInGraph(Graph *graph, Predicate f)
    {
        ASSERT(graph != nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            auto it = std::find_if(bb->AllInsts().begin(), bb->AllInsts().end(), f);
            if (it != bb->AllInsts().end()) {
                return *it;
            }
        }
        return nullptr;
    }

    template <typename Predicate>
    static std::pair<Inst *, LifeIntervals *> FindInstWithInterval(const LivenessAnalyzer &la, BasicBlock *bb,
                                                                   Predicate f)
    {
        auto inst = FindInstInBlock(bb, f);
        ASSERT(inst != nullptr);
        auto interval = la.GetInstLifeIntervals(inst);
        ASSERT(interval != nullptr);
        return {inst, interval};
    }

    template <typename Predicate>
    static std::pair<Inst *, LifeIntervals *> FindInstWithInterval(const LivenessAnalyzer &la, Graph *graph,
                                                                   Predicate f)
    {
        auto inst = FindInstInGraph(graph, f);
        ASSERT(inst != nullptr);
        auto interval = la.GetInstLifeIntervals(inst);
        ASSERT(interval != nullptr);
        return {inst, interval};
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
 * @tc.name: split_resolver_test_001
 * @tc.desc: Verify the ConnectSiblings function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SplitResolverTest, split_resolver_test_001, TestSize.Level1)
{
    const Register REG_PARAM_INIT = 0;
    const StackSlot SLOT_AT_ADD = 0;
    const Register REG_AT_MUL = 1;

    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "split1";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        graph->RunPass<LivenessAnalyzer>();
        auto &la = graph->GetAnalysis<LivenessAnalyzer>();
        
        auto start_bb = graph->GetStartBlock();
        EXPECT_TRUE(start_bb != nullptr);
        auto bb = start_bb->GetSuccessor(0);
        EXPECT_TRUE(bb != nullptr);

        auto [param_inst, param_interval] =
            FindInstWithInterval(la, start_bb, [](Inst *inst) { return inst->IsParameter() && inst->HasUsers(); });
        auto [add_inst, add_interval] = FindInstWithInterval(la, bb, [](Inst *inst) {
            return IsIntrinsic(inst, IntrinsicInst::IntrinsicId::ADD2_IMM8_V8);
        });
        auto [mul_inst, mul_interval] = FindInstWithInterval(la, bb, [](Inst *inst) {
            return IsIntrinsic(inst, IntrinsicInst::IntrinsicId::MUL2_IMM8_V8);
        });
        param_interval->SetReg(REG_PARAM_INIT);

        // Split param_interval at add inst and mul inst
        auto split = param_interval->SplitAt(add_interval->GetBegin() - 1, graph->GetAllocator());
        split->SetLocation(Location::MakeStackSlot(SLOT_AT_ADD));
        split = split->SplitAt(mul_interval->GetBegin() - 1, graph->GetAllocator());
        split->SetReg(REG_AT_MUL);

        InitUsedRegs(graph, 256);
        SplitResolver(graph, &la).Run();

        auto sf_data1 = add_inst->GetPrev()->CastToSpillFill()->GetSpillFill(0);
        auto sf_data2 = mul_inst->GetPrev()->CastToSpillFill()->GetSpillFill(0);
        EXPECT_EQ(sf_data1.GetSrc(), Location::MakeRegister(REG_PARAM_INIT));
        EXPECT_EQ(sf_data1.GetDst(), Location::MakeStackSlot(SLOT_AT_ADD));
        EXPECT_EQ(sf_data2.GetSrc(), Location::MakeStackSlot(SLOT_AT_ADD));
        EXPECT_EQ(sf_data2.GetDst(), Location::MakeRegister(REG_AT_MUL));

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: split_resolver_test_002
 * @tc.desc: Verify the ConnectSiblings function with existing spillfill insts.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SplitResolverTest, split_resolver_test_002, TestSize.Level1)
{
    const Register REG_PARAM1_INIT = 0;
    const Register REG_PARAM2_INIT = 0;
    const StackSlot SLOT_PARAM1_AT_CALL = 0;
    const Register REG_PARAM2_AT_CALL = 2;

    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "split2";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        // Insert a spillfill inst before add inst to test that
        // the ConnectSiblings function can skip this spillfill inst
        auto bb = graph->GetStartBlock()->GetSuccessor(0);
        auto call_inst = FindInstInBlock(
            bb, [](Inst *inst) { return IsIntrinsic(inst, IntrinsicInst::IntrinsicId::CALLARGS2_IMM8_V8_V8); });
        auto input_fill_inst = graph->CreateInstSpillFill();
        input_fill_inst->SetSpillFillType(SpillFillType::INPUT_FILL);
        call_inst->InsertBefore(input_fill_inst);

        graph->RunPass<LivenessAnalyzer>();
        auto &la = graph->GetAnalysis<LivenessAnalyzer>();

        auto [param1_inst, param1_interval] = FindInstWithInterval(
            la, graph->GetStartBlock(), [](Inst *inst) { return inst->IsParameter() && inst->HasUsers(); });
        auto param2_inst = param1_inst->GetNext();
        auto param2_interval = la.GetInstLifeIntervals(param2_inst);
        auto call_interval = la.GetInstLifeIntervals(call_inst);
        param1_interval->SetReg(REG_PARAM1_INIT);
        param2_interval->SetReg(REG_PARAM2_INIT);
        
        // Split at call inst
        auto split1 = param1_interval->SplitAt(call_interval->GetBegin() - 1, graph->GetAllocator());
        split1->SetLocation(Location::MakeStackSlot(SLOT_PARAM1_AT_CALL));
        auto split2 = param2_interval->SplitAt(call_interval->GetBegin() - 1, graph->GetAllocator());
        split2->SetReg(REG_PARAM2_AT_CALL);

        InitUsedRegs(graph, 256);
        SplitResolver(graph, &la).Run();

        EXPECT_EQ(call_inst->GetPrev(), input_fill_inst);
        auto sf_data = input_fill_inst->GetPrev()->CastToSpillFill()->GetSpillFills();
        EXPECT_EQ(sf_data[0].GetSrc(), Location::MakeRegister(REG_PARAM1_INIT));
        EXPECT_EQ(sf_data[0].GetDst(), Location::MakeStackSlot(SLOT_PARAM1_AT_CALL));
        EXPECT_EQ(sf_data[1].GetSrc(), Location::MakeRegister(REG_PARAM2_INIT));
        EXPECT_EQ(sf_data[1].GetDst(), Location::MakeRegister(REG_PARAM2_AT_CALL));

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: split_resolver_test_003
 * @tc.desc: Verify the ConnectSpiltFromPredBlock function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(SplitResolverTest, split_resolver_test_003, TestSize.Level1)
{
    const Register REG_PARAM_INIT = 0;
    const StackSlot SLOT_AT_MUL = 1;

    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "split3";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        graph->RunPass<LivenessAnalyzer>();
        auto &la = graph->GetAnalysis<LivenessAnalyzer>();

        auto [param_inst, param_interval] = FindInstWithInterval(
            la, graph->GetStartBlock(), [](Inst *inst) { return inst->IsParameter() && inst->HasUsers(); });
        auto [mul_inst, mul_interval] = FindInstWithInterval(la, graph, [](Inst *inst) {
            return IsIntrinsic(inst, IntrinsicInst::IntrinsicId::MUL2_IMM8_V8);
        });
        param_interval->SetReg(REG_PARAM_INIT);

        // Split in `if` branch
        auto split = param_interval->SplitAt(mul_interval->GetBegin() - 1, graph->GetAllocator());
        split->SetLocation(Location::MakeStackSlot(SLOT_AT_MUL));

        InitUsedRegs(graph, 256);
        SplitResolver(graph, &la).Run();

        auto sf_inst1 = mul_inst->GetPrev()->CastToSpillFill();
        EXPECT_EQ(sf_inst1->GetSpillFillType(), SpillFillType::CONNECT_SPLIT_SIBLINGS);
        auto sf_data1 = sf_inst1->CastToSpillFill()->GetSpillFill(0);
        EXPECT_EQ(sf_data1.GetSrc(), Location::MakeRegister(REG_PARAM_INIT));
        EXPECT_EQ(sf_data1.GetDst(), Location::MakeStackSlot(SLOT_AT_MUL));

        // This spillfill inst should be created at the end of `else` branch
        auto sf_inst2 = FindInstInGraph(graph, [](Inst *inst) {
            return inst->IsSpillFill() && inst->CastToSpillFill()->GetSpillFillType() == SpillFillType::SPLIT_MOVE;
        });
        EXPECT_TRUE(sf_inst2 != nullptr);
        auto sf_data2 = sf_inst2->CastToSpillFill()->GetSpillFill(0);
        EXPECT_EQ(sf_data2.GetSrc(), Location::MakeRegister(REG_PARAM_INIT));
        EXPECT_EQ(sf_data2.GetDst(), Location::MakeStackSlot(SLOT_AT_MUL));

        status = true;
    });
    EXPECT_TRUE(status);
}
}  // namespace panda::compiler

