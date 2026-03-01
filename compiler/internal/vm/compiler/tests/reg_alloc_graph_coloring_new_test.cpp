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
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "graph_test.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/ir/constants.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/locations.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "optimizer/optimizations/regalloc/reg_alloc_graph_coloring.h"
#include "optimizer/optimizations/regalloc/reg_alloc_resolver.h"
#include "optimizer/optimizations/regalloc/reg_alloc_stat.h"
#include "reg_acc_alloc.h"

using namespace testing::ext;

namespace panda::compiler {
class RegAllocGraphColoringTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    GraphTest graph_test_;
};

/**
 * @tc.name: reg_alloc_graph_coloring_test_001
 * @tc.desc: Verify regalloc with more than 255 registers.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocGraphColoringTest, reg_alloc_graph_coloring_test_001, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "manyRegisters";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

        // Currently allocating more than 255 registers is not supported
        EXPECT_FALSE(RegAlloc(graph));

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: reg_alloc_graph_coloring_test_002
 * @tc.desc: Verify regalloc with physical reg.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocGraphColoringTest, reg_alloc_graph_coloring_test_002, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "allocPhysical";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        graph->RunPass<LivenessAnalyzer>();
        auto intervals = graph->GetAnalysis<LivenessAnalyzer>().GetLifeIntervals();

        Register physical_reg_count = 0;
        std::vector<std::pair<Inst*, Register>> pairs;
        for (auto interval : intervals) {
            auto inst = interval->GetInst();
            if (inst->IsConst()) {
                interval->SetPhysicalReg(physical_reg_count, inst->GetType());
                pairs.emplace_back(inst, physical_reg_count);
                physical_reg_count++;
            }
        }
        EXPECT_GT(physical_reg_count, 0);

        EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());
        EXPECT_TRUE(RegAlloc(graph));

        for (auto [inst, expect_reg] : pairs) {
            EXPECT_EQ(inst->GetDstReg(), expect_reg);
        }

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: reg_alloc_graph_coloring_test_003
 * @tc.desc: Verify regalloc with physical fp reg.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocGraphColoringTest, reg_alloc_graph_coloring_test_003, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "allocFp";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        graph->RunPass<LivenessAnalyzer>();
        auto intervals = graph->GetAnalysis<LivenessAnalyzer>().GetLifeIntervals();

        Register fp_reg_count = 0;
        std::vector<std::pair<Inst*, Register>> pairs;
        for (auto interval : intervals) {
            auto inst = interval->GetInst();
            if (inst->IsConst() && inst->GetType() == DataType::FLOAT64) {
                interval->SetPhysicalReg(fp_reg_count, inst->GetType());
                pairs.emplace_back(inst, fp_reg_count);
                fp_reg_count++;
            }
        }
        EXPECT_GT(fp_reg_count, 0);

        EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());

        graph->RunPass<Cleanup>();
        RegAllocResolver(graph).ResolveCatchPhis();

        RegAllocGraphColoring regalloc(graph);
        regalloc.GetRegMask().Resize(VIRTUAL_FRAME_SIZE);
        regalloc.GetVRegMask().Resize(fp_reg_count);
        EXPECT_TRUE(regalloc.RunImpl());

        for (auto [inst, expect_reg] : pairs) {
            EXPECT_EQ(inst->GetDstReg(), expect_reg);
        }

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: reg_alloc_graph_coloring_test_004
 * @tc.desc: Verify the regalloc on Arch::AARCH32.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocGraphColoringTest, reg_alloc_graph_coloring_test_004, TestSize.Level1)
{
    std::string pfile_name = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "#*#func4";
    bool status = false;
    auto pfile = panda_file::OpenPandaFile(pfile_name);
    for (uint32_t id : pfile->GetClasses()) {
        panda_file::File::EntityId record_id {id};

        if (pfile->IsExternal(record_id)) {
            continue;
        }

        panda_file::ClassDataAccessor cda {*pfile, record_id};
        cda.EnumerateMethods([&pfile, test_method_name, &status](panda_file::MethodDataAccessor &mda) {
            if (mda.IsExternal()) {
                return;
            }
            ArenaAllocator allocator {SpaceType::SPACE_TYPE_COMPILER};
            ArenaAllocator local_allocator {SpaceType::SPACE_TYPE_COMPILER, nullptr, true};

            auto method_ptr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(
                mda.GetMethodId().GetOffset());
            panda::BytecodeOptimizerRuntimeAdapter adapter(mda.GetPandaFile());
            auto *graph = allocator.New<Graph>(&allocator, &local_allocator, Arch::AARCH32, method_ptr, &adapter,
                                               false, nullptr, true, true);
            graph->RunPass<panda::compiler::IrBuilder>();

            auto method_name = std::string(utf::Mutf8AsCString(pfile->GetStringData(mda.GetNameId()).data));
            if (test_method_name != method_name) {
                return;
            }
            
            EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());
            EXPECT_TRUE(RegAlloc(graph));
            status = true;
        });
    }
    EXPECT_TRUE(status);
}

/**
 * @tc.name: reg_alloc_graph_coloring_test_005
 * @tc.desc: Verify regalloc with preassigned registers.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocGraphColoringTest, reg_alloc_graph_coloring_test_005, TestSize.Level1)
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

        Register preassign_count = 0;
        std::vector<std::pair<Inst*, Register>> pairs;
        for (auto interval : intervals) {
            auto inst = interval->GetInst();
            auto inputs = inst->GetInputs();
            // Preassign registers for insts which have a parameter input
            auto it = std::find_if(inputs.begin(), inputs.end(), [](Input input) {
                return input.GetInst()->IsParameter();
            });
            if (it != inputs.end()) {
                interval->SetPreassignedReg(preassign_count);
                pairs.emplace_back(inst, preassign_count);
                preassign_count++;
            }
        }
        EXPECT_GT(preassign_count, 0);

        EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());
        EXPECT_TRUE(RegAlloc(graph));

        for (auto [inst, expect_reg] : pairs) {
            EXPECT_EQ(inst->GetDstReg(), expect_reg);
        }

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: reg_alloc_graph_coloring_test_006
 * @tc.desc: Verify regalloc with fixed locations.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocGraphColoringTest, reg_alloc_graph_coloring_test_006, TestSize.Level1)
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

        Register fixed_count = 0;
        std::vector<std::pair<Inst*, Register>> pairs;
        for (auto interval : intervals) {
            auto inst = interval->GetInst();
            auto inputs = inst->GetInputs();
            // Set fixed locations for insts which have a parameter input
            auto it = std::find_if(inputs.begin(), inputs.end(), [](Input input) {
                return input.GetInst()->IsParameter();
            });
            if (it != inputs.end()) {
                interval->SetLocation(Location::MakeRegister(fixed_count));
                pairs.emplace_back(inst, fixed_count);
                fixed_count++;
            }
        }
        EXPECT_GT(fixed_count, 0);

        EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());
        EXPECT_TRUE(RegAlloc(graph));

        for (auto [inst, expect_reg] : pairs) {
            EXPECT_EQ(inst->GetDstReg(), expect_reg);
        }

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: reg_alloc_graph_coloring_test_007
 * @tc.desc: Verity the Presplit function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocGraphColoringTest, reg_alloc_graph_coloring_test_007, TestSize.Level1)
{
    const Register FIX_REG = 1;

    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "presplit";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        graph->RunPass<LivenessAnalyzer>();
        auto intervals = graph->GetAnalysis<LivenessAnalyzer>().GetLifeIntervals();

        std::vector<LifeIntervals*> split_intervals;
        for (auto interval : intervals) {
            if (!interval->GetInst()->IsIntrinsic()) {
                continue;
            }

            // Set same fix register for add insts
            auto inst = interval->GetInst()->CastToIntrinsic();
            if (inst->CastToIntrinsic()->GetIntrinsicId() == IntrinsicInst::IntrinsicId::ADD2_IMM8_V8) {
                interval->SetLocation(Location::MakeRegister(FIX_REG));
                split_intervals.emplace_back(interval);
            }
        }

        EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());
        EXPECT_TRUE(RegAlloc(graph));

        for (auto interval : split_intervals) {
            if (interval->GetSibling()) {
                EXPECT_TRUE(interval->GetSibling()->IsSplitSibling());
            }
        }

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: reg_alloc_graph_coloring_test_008
 * @tc.desc: Verify regalloc with try-catch blocks.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocGraphColoringTest, reg_alloc_graph_coloring_test_008, TestSize.Level1)
{
    std::string pfile1 = GRAPH_TEST_ABC_DIR "regallocTryTest.abc";

    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile1, [&status](Graph* graph, std::string &method_name) {
        EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());
        EXPECT_TRUE(RegAlloc(graph));
        status = true;
    });
    EXPECT_TRUE(status);

    std::string pfile2 = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func5";
    status = false;
    graph_test_.TestBuildGraphFromFile(pfile2, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());
        EXPECT_TRUE(RegAlloc(graph));
        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: reg_alloc_graph_coloring_test_009
 * @tc.desc: Verify RegAllocStat.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocGraphColoringTest, reg_alloc_graph_coloring_test_009, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "stat";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        graph->RunPass<LivenessAnalyzer>();
        auto intervals = graph->GetAnalysis<LivenessAnalyzer>().GetLifeIntervals();

        Register reg_count = 0;
        for (auto interval : intervals) {
            if (interval->IsPhysical() || interval->NoDest()) {
                continue;
            }
            interval->SetLocation(Location::MakeRegister(reg_count++));
        }

        RegAllocStat st(intervals);
        EXPECT_EQ(st.GetRegCount(), reg_count);
        EXPECT_EQ(st.GetVRegCount(), 0);
        EXPECT_EQ(st.GetSlotCount(), 0);
        EXPECT_EQ(st.GetVSlotCount(), 0);

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: reg_alloc_graph_coloring_test_010
 * @tc.desc: Verify RegAllocStat with vregs.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocGraphColoringTest, reg_alloc_graph_coloring_test_010, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "stat";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        graph->RunPass<LivenessAnalyzer>();
        auto intervals = graph->GetAnalysis<LivenessAnalyzer>().GetLifeIntervals();

        Register fp_reg_count = 0;
        for (auto interval : intervals) {
            if (interval->IsPhysical() || interval->NoDest()) {
                continue;
            }
            interval->SetLocation(Location::MakeFpRegister(fp_reg_count++));
        }

        RegAllocStat st(intervals);
        EXPECT_EQ(st.GetRegCount(), 0);
        EXPECT_EQ(st.GetVRegCount(), fp_reg_count);
        EXPECT_EQ(st.GetSlotCount(), 0);
        EXPECT_EQ(st.GetVSlotCount(), 0);

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: reg_alloc_graph_coloring_test_011
 * @tc.desc: Verify RegAllocStat with constants.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RegAllocGraphColoringTest, reg_alloc_graph_coloring_test_011, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "stat";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        graph->RunPass<LivenessAnalyzer>();
        auto intervals = graph->GetAnalysis<LivenessAnalyzer>().GetLifeIntervals();

        for (auto interval : intervals) {
            interval->SetLocation(Location::MakeConstant(0));
        }

        RegAllocStat st(intervals);
        EXPECT_EQ(st.GetRegCount(), 0);
        EXPECT_EQ(st.GetVRegCount(), 0);
        EXPECT_EQ(st.GetSlotCount(), 0);
        EXPECT_EQ(st.GetVSlotCount(), 0);

        status = true;
    });
    EXPECT_TRUE(status);
}
}  // namespace panda::compiler

