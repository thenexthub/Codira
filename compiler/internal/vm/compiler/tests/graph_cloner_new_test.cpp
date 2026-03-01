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
#include <cstdint>
#include <gtest/gtest.h>
#include <string>
#include <unordered_set>
#include <vector>

#include "graph_comparator.h"
#include "graph_test.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/ir/inst.h"

using namespace testing::ext;

namespace panda::compiler {
class GraphClonerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};

    GraphTest graph_test_;

    static BasicBlock *GetLoopOuterBlock(BasicBlock *exit_block)
    {
        EXPECT_TRUE(exit_block->GetSuccsBlocks().size() == 2U);
        auto loop = exit_block->GetLoop();
        auto outer = exit_block->GetTrueSuccessor();
        if (outer->GetLoop() == loop) {
            outer = exit_block->GetFalseSuccessor();
        }
        EXPECT_TRUE(outer->GetLoop() != loop);
        return outer;
    }

    static bool CompareInstsOpcode(BasicBlock *bb1, BasicBlock *bb2)
    {
        EXPECT_TRUE(bb1 != nullptr);
        EXPECT_TRUE(bb2 != nullptr);
        auto insts1 = bb1->Insts();
        auto insts2 = bb2->Insts();
        return std::equal(insts1.begin(), insts1.end(), insts2.begin(), insts2.end(),
                          [](Inst *inst1, Inst *inst2) { return inst1->GetOpcode() == inst2->GetOpcode(); });
    }

    static bool CompareInstsOpcode(const std::vector<Inst*> &insts1, const InstIter &insts2)
    {
        return std::equal(insts1.begin(), insts1.end(), insts2.begin(), insts2.end(),
                          [](Inst *inst1, Inst *inst2) { return inst1->GetOpcode() == inst2->GetOpcode(); });
    }

    template<typename Callback>
    static void ForEachNonRootLoop(Graph *graph, Callback cb)
    {
        EXPECT_TRUE(graph != nullptr);
        if (!graph->HasLoop()) {
            return;
        }

        auto root_loop = graph->GetRootLoop();
        for (auto loop : root_loop->GetInnerLoops()) {
            VisitLoopRec(loop, cb);
        }
    }

    template<typename Callback>
    static void VisitLoopRec(Loop *loop, Callback cb)
    {
        EXPECT_TRUE(loop != nullptr);
        cb(loop);
        for (auto inner_loop : loop->GetInnerLoops()) {
            VisitLoopRec(inner_loop, cb);
        }
    }

    static Graph* CloneGraph(Graph *graph)
    {
        GraphCloner graph_cloner(graph, graph->GetAllocator(), graph->GetLocalAllocator());
        auto graph_clone = graph_cloner.CloneGraph();
        EXPECT_TRUE(graph_clone != nullptr);
        EXPECT_TRUE(graph_clone->RunPass<DominatorsTree>());
        EXPECT_TRUE(graph_clone->RunPass<LoopAnalyzer>());
        return graph_clone;
    }

    template <UnrollType type>
    static Loop* CloneFirstLoopAndUnroll(Graph *graph, size_t factor)
    {
        auto graph_clone = CloneGraph(graph);
        auto loop_clone = graph_clone->GetRootLoop()->GetInnerLoops()[0];
        GraphCloner loop_cloner(graph_clone, graph_clone->GetAllocator(), graph_clone->GetLocalAllocator());
        loop_cloner.UnrollLoopBody<type>(loop_clone, factor);
        return loop_clone;
    }
};

/**
 * @tc.name: graph_cloner_test_001
 * @tc.desc: Verify the CloneGraph function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(GraphClonerTest, graph_cloner_test_001, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "graphTest.abc";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&status](Graph* graph, std::string &method_name) {
        GraphCloner cloner(graph, graph->GetAllocator(), graph->GetLocalAllocator());
        auto graph_clone = cloner.CloneGraph();
        EXPECT_TRUE(graph_clone != nullptr);
        EXPECT_TRUE(GraphComparator().Compare(graph, graph_clone));
        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_cloner_test_002
 * @tc.desc: Verify the CloneLoopHeader function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(GraphClonerTest, graph_cloner_test_002, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "graphTest.abc";
    std::unordered_set<std::string> test_method_names {
        "loop1",
        "loop2",
        "loop3",
        "loop4",
    };
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_names, &status](Graph* graph, std::string &method_name) {
        if (test_method_names.count(method_name) == 0) {
            return;
        }

        EXPECT_TRUE(graph->RunPass<LoopAnalyzer>());

        auto graph_clone = CloneGraph(graph);
        EXPECT_TRUE(GraphComparator().Compare(graph, graph_clone));
        ForEachNonRootLoop(graph_clone, [&graph_clone](Loop *loop) {
            auto blocks = graph_clone->GetVectorBlocks();
            auto header = blocks[loop->GetHeader()->GetId()];
            auto preheader = blocks[loop->GetPreHeader()->GetId()];
            auto outer_bb = GetLoopOuterBlock(header);

            GraphCloner header_cloner(graph_clone, graph_clone->GetAllocator(), graph_clone->GetLocalAllocator());
            auto header_clone = header_cloner.CloneLoopHeader(header, GetLoopOuterBlock(header), preheader);
            auto resolver_bb = GetLoopOuterBlock(header);

            EXPECT_TRUE(preheader->HasSucc(header_clone));
            EXPECT_TRUE(header_clone->HasSucc(header));
            EXPECT_TRUE(CompareInstsOpcode(header_clone, header));
            EXPECT_TRUE(resolver_bb->HasSucc(outer_bb));
        });

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_cloner_test_003
 * @tc.desc: Verify the IsLoopClonable function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(GraphClonerTest, graph_cloner_test_003, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "graphTest.abc";
    std::unordered_set<std::string> test_method_names {
        "loop1",
        "loop2",
        "loop3",
        "loop4",
    };
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_names, &status](Graph* graph, std::string &method_name) {
        if (test_method_names.count(method_name) == 0) {
            return;
        }

        EXPECT_TRUE(graph->RunPass<LoopAnalyzer>());

        GraphCloner cloner(graph, graph->GetAllocator(), graph->GetLocalAllocator());

        ForEachNonRootLoop(graph, [&cloner](Loop *loop) {
            if (!loop->GetInnerLoops().empty() || !loop->GetOuterLoop()->IsRoot() ||
                !IsLoopSingleBackEdgeExitPoint(loop)) {
                EXPECT_FALSE(cloner.IsLoopClonable(loop, UINT32_MAX));
            } else {
                EXPECT_TRUE(cloner.CloneLoopHeader(loop->GetHeader(), GetLoopOuterBlock(loop->GetHeader()),
                                                   loop->GetPreHeader()) != nullptr);

                EXPECT_TRUE(cloner.IsLoopClonable(loop, UINT32_MAX));
                EXPECT_FALSE(cloner.IsLoopClonable(loop, 1));
            }
        });

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_cloner_test_004
 * @tc.desc: Verify the CloneLoop function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(GraphClonerTest, graph_cloner_test_004, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "graphTest.abc";
    const char *test_method_name = "loop4";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_TRUE(graph->RunPass<LoopAnalyzer>());
        auto loop = graph->GetRootLoop()->GetInnerLoops()[0];
        EXPECT_TRUE(IsLoopSingleBackEdgeExitPoint(loop));

        GraphCloner header_cloner(graph, graph->GetAllocator(), graph->GetLocalAllocator());
        EXPECT_TRUE(header_cloner.CloneLoopHeader(loop->GetHeader(), GetLoopOuterBlock(loop->GetHeader()),
                                           loop->GetPreHeader()) != nullptr);
        EXPECT_TRUE(loop->GetPreHeader()->GetLastInst()->GetOpcode() == Opcode::IfImm);

        GraphCloner loop_cloner(graph, graph->GetAllocator(), graph->GetLocalAllocator());
        auto loop_clone = loop_cloner.CloneLoop(loop);
        EXPECT_TRUE(loop_clone != nullptr);
        EXPECT_TRUE(GetLoopOuterBlock(loop->GetHeader())->HasSucc(loop_clone->GetPreHeader()));
        EXPECT_TRUE(CompareInstsOpcode(loop->GetPreHeader(), loop_clone->GetPreHeader()));
        EXPECT_TRUE(CompareInstsOpcode(loop->GetHeader(), loop_clone->GetHeader()));

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_cloner_test_005
 * @tc.desc: Verify the UnrollLoopBody function with side exits.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(GraphClonerTest, graph_cloner_test_005, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "graphTest.abc";
    const char *test_method_name = "loop4";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_TRUE(graph->RunPass<LoopAnalyzer>());
        auto loop = graph->GetRootLoop()->GetInnerLoops()[0];
        EXPECT_TRUE(IsLoopSingleBackEdgeExitPoint(loop));
        auto loop_body = loop->GetHeader();
        auto outer_bb_id = GetLoopOuterBlock(loop->GetHeader())->GetId();

        auto loop_clone = CloneFirstLoopAndUnroll<UnrollType::UNROLL_WITH_SIDE_EXITS>(graph, 3);
        auto graph_clone = loop_clone->GetPreHeader()->GetGraph();

        auto resolver_bb = graph_clone->GetVectorBlocks()[outer_bb_id]->GetPredecessor(0);
        EXPECT_EQ(loop->GetPreHeader()->GetId(), loop_clone->GetPreHeader()->GetId());
        for (auto bb : loop_clone->GetBlocks()) {
            if (!bb->IsLoopHeader() && !bb->IsLoopPreHeader()) {
                EXPECT_TRUE(bb->HasSucc(resolver_bb));
                EXPECT_TRUE(CompareInstsOpcode(bb, loop_body));
            }
        }

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_cloner_test_006
 * @tc.desc: Verify the UnrollLoopBody function without side exits.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(GraphClonerTest, graph_cloner_test_006, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "graphTest.abc";
    const char *test_method_name = "loop4";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_TRUE(graph->RunPass<LoopAnalyzer>());
        auto loop = graph->GetRootLoop()->GetInnerLoops()[0];
        EXPECT_TRUE(IsLoopSingleBackEdgeExitPoint(loop));

        // Make preheader have IfImm inst, which is required for UnrollLoopBody without side exits
        GraphCloner header_cloner(graph, graph->GetAllocator(), graph->GetLocalAllocator());
        EXPECT_TRUE(header_cloner.CloneLoopHeader(loop->GetHeader(), GetLoopOuterBlock(loop->GetHeader()),
                                           loop->GetPreHeader()) != nullptr);
        EXPECT_TRUE(loop->GetPreHeader()->GetLastInst()->GetOpcode() == Opcode::IfImm);

        auto outer_bb_id = GetLoopOuterBlock(loop->GetHeader())->GetId();
        auto loop_body = loop->GetHeader();
        std::vector<Inst*> loop_body_insts;
        std::vector<Inst*> loop_cmp_if_insts;
        for (auto inst : loop_body->Insts()) {
            if (inst->GetOpcode() == Opcode::IfImm || inst->GetOpcode() == Opcode::Compare) {
                loop_cmp_if_insts.emplace_back(inst);
            } else {
                loop_body_insts.emplace_back(inst);
            }
        }

        auto loop_clone = CloneFirstLoopAndUnroll<UnrollType::UNROLL_WITHOUT_SIDE_EXITS>(graph, 3);
        auto graph_clone = loop_clone->GetPreHeader()->GetGraph();

        EXPECT_EQ(loop->GetPreHeader()->GetId(), loop_clone->GetPreHeader()->GetId());
        for (auto bb : loop_clone->GetBlocks()) {
            if (bb->IsLoopHeader() || bb->IsLoopPreHeader()) {
                continue;
            }
            auto insts = bb->Insts();
            if (bb->GetSuccsBlocks().size() == 1) {
                // Insts are expected to be loop body insts
                EXPECT_TRUE(CompareInstsOpcode(loop_body_insts, insts));
            } else {
                // Insts are expected to be the Compare and IfImm insts
                EXPECT_TRUE(bb->HasSucc(graph_clone->GetVectorBlocks()[outer_bb_id]));
                EXPECT_TRUE(CompareInstsOpcode(loop_cmp_if_insts, insts));
            }
        }

        status = true;
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_cloner_test_007
 * @tc.desc: Verify the UnrollLoopBody function with post increment.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(GraphClonerTest, graph_cloner_test_007, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "graphTest.abc";
    const char *test_method_name = "loop4";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_TRUE(graph->RunPass<LoopAnalyzer>());
        auto loop = graph->GetRootLoop()->GetInnerLoops()[0];
        EXPECT_TRUE(IsLoopSingleBackEdgeExitPoint(loop));
        auto loop_body = loop->GetHeader();

        auto loop_clone = CloneFirstLoopAndUnroll<UnrollType::UNROLL_POST_INCREMENT>(graph, 3);

        for (auto bb : loop_clone->GetBlocks()) {
            if (bb->IsLoopHeader() || bb->IsLoopPreHeader()) {
                continue;
            }
            if (bb->GetSuccessor(0)->GetLoop() == bb->GetLoop()) {
                EXPECT_TRUE(CompareInstsOpcode(bb, loop_body));
            } else {
                EXPECT_TRUE(bb->GetLastInst()->GetOpcode() == Opcode::Compare);
            }
        }

        status = true;
    });
    EXPECT_TRUE(status);
}
}  // namespace panda::compiler

