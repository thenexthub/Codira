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

#include "compiler/optimizer/ir/basicblock.h"
#include "graph.h"
#include "graph_test.h"
#include "mem/pool_manager.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/analysis/rpo.h"

using namespace testing::ext;

namespace panda::compiler {
class LivenessAnalyzerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    GraphTest graph_test_;
};

/**
 * @tc.name: liveness_analyzer_test_001
 * @tc.desc: Verify the DumpLifeIntervals function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LivenessAnalyzerTest, liveness_analyzer_test_001, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        LivenessAnalyzer liveness_analyzer(graph);
        std::stringstream out;
        liveness_analyzer.DumpLifeIntervals(out);
        std::string str = "\nRegisters intervals\n-\n\n"
                          "Vector registers intervals\n-\n\n"
                          "Stack slots intervals\n-\n";
        EXPECT_EQ(out.str(), str);
        EXPECT_EQ(0, liveness_analyzer.GetBlocksCount());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: liveness_analyzer_test_002
 * @tc.desc: Verify the DumpLifeIntervals function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LivenessAnalyzerTest, liveness_analyzer_test_002, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);

        graph->RunPass<LivenessAnalyzer>();
        auto liveness_analyzer = &graph->GetAnalysis<LivenessAnalyzer>();

        Location location(Location::Kind::FP_REGISTER, 0);
        for (auto a : liveness_analyzer->GetLifeIntervals()) {
            a->SetLocation(location);
            EXPECT_EQ(a->GetLocation().GetValue(), 0);
            a->SetType(DataType::FLOAT32);
            EXPECT_EQ(a->GetType(), DataType::FLOAT32);
        }

        std::stringstream out;
        liveness_analyzer->DumpLifeIntervals(out);
        std::string str = "BB 7\t[0:10)\n"
                          "v5\t[2:4)@ ; \n"
                          "v6\t[4:6)@ ; \n"
                          "v7\t[6:8)@ ; \n"
                          "v8\t[8:20)@ ; \n"
                          "BB 0\t[10:12)\n"
                          "BB 2\t[12:16)\n"
                          "v0\t[14:16)@ ; \n"
                          "BB 4\t[16:22)\n"
                          "v10\t[18:20)@ ; \n"
                          "v9\t[20:22)@ ; \n"
                          "BB 3\t[22:24)\n"
                          "BB 9\t[26:28)\n"
                          "BB 6\t[28:34)\n"
                          "v12\t[30:32)@ ; \n"
                          "v11\t[32:34)@ ; \n"
                          "BB 5\t[24:26)\n"
                          "BB 1\t[34:40)\n"
                          "v18\t[36:38)@ ; \n"
                          "v17\t[38:40)@ ; \n"
                          "BB 8\t[40:42)\n\n"
                          "Registers intervals\n-\n\n"
                          "Vector registers intervals\n"
                          "vr0: [2:4); [4:6); [6:8); [8:20); [14:16); [18:20); [20:22); [30:32); [32:34); [36:38); "
                          "[38:40)\n\n"
                          "Stack slots intervals\n-\n";
        EXPECT_EQ(out.str(), str);
        EXPECT_TRUE(location.IsFpRegister());

        Location location1(Location::Kind::REGISTER, 0);
        for (auto a : liveness_analyzer->GetLifeIntervals()) {
            a->SetLocation(location1);
        }
        out.clear();
        out.str("");
        liveness_analyzer->DumpLifeIntervals(out);
        str = "BB 7\t[0:10)\n"
              "v5\t[2:4)@ ; \n"
              "v6\t[4:6)@ ; \n"
              "v7\t[6:8)@ ; \n"
              "v8\t[8:20)@ ; \n"
              "BB 0\t[10:12)\n"
              "BB 2\t[12:16)\n"
              "v0\t[14:16)@ ; \n"
              "BB 4\t[16:22)\n"
              "v10\t[18:20)@ ; \n"
              "v9\t[20:22)@ ; \n"
              "BB 3\t[22:24)\n"
              "BB 9\t[26:28)\n"
              "BB 6\t[28:34)\n"
              "v12\t[30:32)@ ; \n"
              "v11\t[32:34)@ ; \n"
              "BB 5\t[24:26)\n"
              "BB 1\t[34:40)\n"
              "v18\t[36:38)@ ; \n"
              "v17\t[38:40)@ ; \n"
              "BB 8\t[40:42)\n\n"
              "Registers intervals\n"
              "r0: [2:4); [4:6); [6:8); [8:20); [14:16); [18:20); [20:22); [30:32); [32:34); [36:38); "
              "[38:40)\n\n"
              "Vector registers intervals\n-\n\n"
              "Stack slots intervals\n-\n";
        EXPECT_EQ(out.str(), str);
        EXPECT_TRUE(location1.IsRegister());

        Location location2(Location::Kind::STACK, 0);
        for (auto a : liveness_analyzer->GetLifeIntervals()) {
            a->SetLocation(location2);
        }
        out.clear();
        out.str("");
        liveness_analyzer->DumpLifeIntervals(out);
        str = "BB 7\t[0:10)\n"
              "v5\t[2:4)@ ; \n"
              "v6\t[4:6)@ ; \n"
              "v7\t[6:8)@ ; \n"
              "v8\t[8:20)@ ; \n"
              "BB 0\t[10:12)\n"
              "BB 2\t[12:16)\n"
              "v0\t[14:16)@ ; \n"
              "BB 4\t[16:22)\n"
              "v10\t[18:20)@ ; \n"
              "v9\t[20:22)@ ; \n"
              "BB 3\t[22:24)\n"
              "BB 9\t[26:28)\n"
              "BB 6\t[28:34)\n"
              "v12\t[30:32)@ ; \n"
              "v11\t[32:34)@ ; \n"
              "BB 5\t[24:26)\n"
              "BB 1\t[34:40)\n"
              "v18\t[36:38)@ ; \n"
              "v17\t[38:40)@ ; \n"
              "BB 8\t[40:42)\n\n"
              "Registers intervals\n-\n\n"
              "Vector registers intervals\n-\n\n"
              "Stack slots intervals\n"
              "s0: [2:4); [4:6); [6:8); [8:20); [14:16); [18:20); [20:22); [30:32); [32:34); [36:38); "
              "[38:40)\n";
        EXPECT_EQ(out.str(), str);
        EXPECT_TRUE(location2.IsStack());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: liveness_analyzer_test_003
 * @tc.desc: Verify the IsCallBlockingRegisters function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LivenessAnalyzerTest, liveness_analyzer_test_003, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        LivenessAnalyzer liveness_analyzer(graph);
        for (auto bb : graph->GetBlocksRPO()) {
            EXPECT_NE(bb, nullptr);
            if (!bb->IsTry()) {
                continue;
            }
            EXPECT_FALSE(liveness_analyzer.IsCallBlockingRegisters(bb->GetFirstInst()));

            bb->GetFirstInst()->SetFlag(compiler::inst_flags::CALL);
            EXPECT_TRUE(liveness_analyzer.IsCallBlockingRegisters(bb->GetFirstInst()));
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: liveness_analyzer_test_004
 * @tc.desc: Verify the AppendRange function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LivenessAnalyzerTest, liveness_analyzer_test_004, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        auto bb = graph->GetStartBlock();
        LifeNumber end = 2;  // 2: It's a random number
        LiveRange live_range(0, end);
        LifeIntervals life_intervals(graph->GetAllocator(), bb->GetFirstInst(), live_range);

        life_intervals.SplitAt(1, graph->GetAllocator());
        EXPECT_NE(life_intervals.SplitAt(1, graph->GetAllocator()), nullptr);
        life_intervals.AppendRange(live_range);
        auto intervals = life_intervals.GetRanges();
        auto it = find(intervals.begin(), intervals.end(), live_range);
        EXPECT_NE(it, intervals.end());
        LifeNumber begin = 3;  // 3: It's a random number
        LiveRange live_range1(begin, end + begin);
        LifeIntervals life_intervals1(graph->GetAllocator(), bb->GetFirstInst(), live_range1);
        EXPECT_NE(life_intervals1.SplitAt(4, graph->GetAllocator()), nullptr);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: liveness_analyzer_test_005
 * @tc.desc: Verify the SplitAt function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LivenessAnalyzerTest, liveness_analyzer_test_005, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        auto bb = graph->GetStartBlock();
        LifeNumber end = 2;  // 2: It's a random number
        LiveRange live_range(0, end);
        LifeIntervals life_intervals(graph->GetAllocator(), bb->GetFirstInst(), live_range);

        EXPECT_NE(life_intervals.SplitAt(1, graph->GetAllocator()), nullptr);
        life_intervals.AppendRange(live_range);
        auto intervals = life_intervals.GetRanges();
        auto it = find(intervals.begin(), intervals.end(), live_range);
        EXPECT_NE(it, intervals.end());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: liveness_analyzer_test_006
 * @tc.desc: Verify the MergeSibling function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LivenessAnalyzerTest, liveness_analyzer_test_006, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        auto bb = graph->GetStartBlock();
        LifeNumber begin = 2;  // 2: It's a random number
        LiveRange live_range(begin, begin + 1);
        LifeIntervals life_intervals(graph->GetAllocator(), bb->GetFirstInst(), live_range);

        LifeNumber ln = 3;  // 3: It's a random number
        life_intervals.AddUsePosition(ln);
        EXPECT_NE(life_intervals.SplitAt(ln, graph->GetAllocator()), nullptr);
        life_intervals.MergeSibling();
        EXPECT_EQ(life_intervals.GetSibling(), nullptr);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: liveness_analyzer_test_007
 * @tc.desc: Verify the FindSiblingAt function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LivenessAnalyzerTest, liveness_analyzer_test_007, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        auto bb = graph->GetStartBlock();
        LifeNumber end = 2;  // 2: It's a random number
        LiveRange live_range(1, end);
        LifeIntervals life_intervals(graph->GetAllocator(), bb->GetFirstInst(), live_range);

        EXPECT_NE(life_intervals.FindSiblingAt(1), nullptr);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: liveness_analyzer_test_008
 * @tc.desc: Verify the FindSiblingAt function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LivenessAnalyzerTest, liveness_analyzer_test_008, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        auto bb = graph->GetStartBlock();
        LifeNumber begin = 2;  // 2: It's a random number
        LiveRange live_range(begin, 1);
        LifeIntervals life_intervals(graph->GetAllocator(), bb->GetFirstInst(), live_range);

        EXPECT_EQ(life_intervals.FindSiblingAt(1), nullptr);
        LiveRange live_range1(begin, 1);
        EXPECT_TRUE(life_intervals.Intersects(live_range1));
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: liveness_analyzer_test_009
 * @tc.desc: Verify the GetFirstIntersectionWith function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LivenessAnalyzerTest, liveness_analyzer_test_009, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        auto bb = graph->GetStartBlock();
        LifeNumber begin = 2;  // 2: It's a random number
        LiveRange live_range(begin, 1);
        LifeIntervals life_intervals(graph->GetAllocator(), bb->GetFirstInst(), live_range);

        LifeIntervals other(graph->GetAllocator());
        EXPECT_EQ(life_intervals.GetFirstIntersectionWith(&other, 1), INVALID_VN);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: liveness_analyzer_test_010
 * @tc.desc: Verify the GetFirstIntersectionWith function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LivenessAnalyzerTest, liveness_analyzer_test_010, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        auto bb = graph->GetStartBlock();
        LifeNumber begin = 7;  // 7: It's a random number
        LifeNumber end = 5;  // 5: It's a random number
        LiveRange live_range(begin, end + 2);
        LifeIntervals life_intervals(graph->GetAllocator(), bb->GetFirstInst(), live_range);

        LifeIntervals other(graph->GetAllocator());

        LifeNumber begin1 = 31;  // 31: It's a random number
        LiveRange live_range1(begin1, begin1 + 1);

        other.AppendRange(live_range1);
        auto intervals = life_intervals.GetRanges();
        auto it = find(intervals.begin(), intervals.end(), live_range);
        EXPECT_NE(it, intervals.end());
        EXPECT_EQ(life_intervals.GetFirstIntersectionWith(&other, 0), INVALID_LIFE_NUMBER);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: liveness_analyzer_test_011
 * @tc.desc: Verify the SplitCover function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LivenessAnalyzerTest, liveness_analyzer_test_011, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        auto bb = graph->GetStartBlock();
        LifeNumber first_end = 3;  // 3: It's a random number
        LiveRange firstlive_range(0, first_end);
        LifeIntervals firstlife_intervals(graph->GetAllocator(), bb->GetFirstInst(), firstlive_range);

        LifeNumber first_position = 2;  // 2: It's a random number
        EXPECT_TRUE(firstlife_intervals.SplitCover(first_position));

        LifeNumber secondBegin = 2;  // 2: It's a random number
        LiveRange secondlive_range(secondBegin, 1);
        LifeIntervals second_life_intervals(graph->GetAllocator(), bb->GetFirstInst(), secondlive_range);

        EXPECT_FALSE(second_life_intervals.SplitCover(1));
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: liveness_analyzer_test_012
 * @tc.desc: Verify the SetPhysicalReg function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LivenessAnalyzerTest, liveness_analyzer_test_012, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        auto bb = graph->GetStartBlock();
        LifeNumber begin = 2;  // 2: It's a random number
        LiveRange live_range(begin, 1);
        LifeIntervals life_intervals(graph->GetAllocator(), bb->GetFirstInst(), live_range);

        life_intervals.SetPhysicalReg(1, DataType::Type::INT64);
        EXPECT_TRUE(life_intervals.HasReg());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: liveness_analyzer_test_013
 * @tc.desc: Verify the ClearLocation function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LivenessAnalyzerTest, liveness_analyzer_test_013, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        auto bb = graph->GetStartBlock();
        LifeNumber begin = 2;  // 2: It's a random number
        LiveRange live_range(begin, 1);
        LifeIntervals life_intervals(graph->GetAllocator(), bb->GetFirstInst(), live_range);
        life_intervals.ClearLocation();
        EXPECT_EQ(life_intervals.GetLocation().GetKind(), Location::Kind::INVALID);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: liveness_analyzer_test_014
 * @tc.desc: Verify the Contains function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LivenessAnalyzerTest, liveness_analyzer_test_014, TestSize.Level1)
{
    LifeNumber begin = 2;  // 2: It's a random number
    LiveRange live_range(begin, 1);
    EXPECT_TRUE(live_range.Contains(live_range));
    LifeNumber number = 3;  // 3: It's a random number
    EXPECT_FALSE(live_range.Contains(number));
}

/**
 * @tc.name: liveness_analyzer_test_015
 * @tc.desc: Verify the Cleanup function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(LivenessAnalyzerTest, liveness_analyzer_test_015, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "dominatorsTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);

        EXPECT_TRUE(graph->RunPass<LivenessAnalyzer>());
        auto liveness_analyzer = &graph->GetAnalysis<LivenessAnalyzer>();
        liveness_analyzer->Cleanup();
    });
    EXPECT_TRUE(status);
}
}  // namespace panda::compiler
