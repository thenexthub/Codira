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

#include <cstring>
#include <gtest/gtest.h>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "basicblock.h"
#include "graph.h"
#include "graph_test.h"
#include "inst.h"
#include "mem/pool_manager.h"
#include "optimizer/analysis/loop_analyzer.h"

using namespace testing::ext;

namespace panda::compiler {
class CompilerBasicBlockTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    GraphTest graph_test_;
};

/**
 * @tc.name: compiler_basicblock_test_001
 * @tc.desc: Verify the BlocksPathDfsSearch function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerBasicBlockTest, compiler_basicblock_test_001, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    graph_test_.TestBuildGraphFromFile(pfile, [&](Graph* graph, std::string &method_name) {
        Marker mrk = 4;  // 4: It's a random number
        auto start_block = graph->GetStartBlock();
        auto end_block = graph->GetEndBlock();
        auto value = BlocksPathDfsSearch(mrk, start_block, start_block, end_block);
        EXPECT_TRUE(value);
        for (auto bb : graph->GetBlocksRPO()) {
            if (!bb->IsCatchBegin()) {
                continue;
            }

            EXPECT_TRUE(BlocksPathDfsSearch(mrk, start_block, end_block, bb->GetSuccessor(0)));
            EXPECT_FALSE(BlocksPathDfsSearch(mrk, start_block, bb->GetSuccessor(0), end_block));
            EXPECT_FALSE(BlocksPathDfsSearch(mrk, end_block, start_block, bb->GetSuccessor(0)));
        }
    });
}

/**
 * @tc.name: compiler_basicblock_test_002
 * @tc.desc: Verify the InsertBlockBefore function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerBasicBlockTest, compiler_basicblock_test_002, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        for (auto bb : graph->GetBlocksRPO()) {
            if (!bb->IsTry()) {
                continue;
            }
            graph->GetEndBlock()->InsertBlockBefore(bb->GetPredecessor(0));
            graph->GetStartBlock()->InsertBlockBefore(bb->GetPredecessor(0));
        }

        EXPECT_TRUE(graph->GetStartBlock()->IsDominate(graph->GetStartBlock()));
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_003
 * @tc.desc: Verify the Clone function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerBasicBlockTest, compiler_basicblock_test_003, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        for (auto bb : graph->GetBlocksRPO()) {
            if (bb->IsStartBlock() || bb->IsEndBlock() || bb->IsTryEnd()) {
                status = true;
                EXPECT_FALSE(bb->Clone(graph)->IsTry());
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_004
 * @tc.desc: Verify the CreateImmediateDominator function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerBasicBlockTest, compiler_basicblock_test_004, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        status = true;
        EXPECT_TRUE(graph->GetStartBlock()->CreateImmediateDominator()->IsEmpty());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_005
 * @tc.desc: Verify the IsDominate function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerBasicBlockTest, compiler_basicblock_test_005, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        auto start_block = graph->GetStartBlock();
        auto end_block = graph->GetEndBlock();

        EXPECT_TRUE(start_block->IsDominate(end_block));
        EXPECT_FALSE(end_block->IsDominate(start_block));
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_006
 * @tc.desc: Verify the EraseInst function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerBasicBlockTest, compiler_basicblock_test_006, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    options.SetCompilerUseSafepoint(false);
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->PhiInsts()) {
                status = true;
                bb->EraseInst(inst, true);
                EXPECT_EQ(inst->GetNext(), nullptr);
                break;
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_007
 * @tc.desc: Verify the RemoveInst function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerBasicBlockTest, compiler_basicblock_test_007, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph,
        std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        auto start_block = graph->GetStartBlock();
        start_block->RemoveInst(graph->GetFirstConstInst());
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                status = true;
                bb->RemoveInst(inst);
                EXPECT_EQ(inst->GetNext(), nullptr);
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_008
 * @tc.desc: Verify the IsInverted function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerBasicBlockTest, compiler_basicblock_test_008, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "testTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        auto bb = graph->GetStartBlock();
        bb->InstsSafe();
        EXPECT_EQ(bb->GetPredsBlocks().size(), 0);
        bb->PhiInstsSafeReverse();
        EXPECT_FALSE(bb->IsInverted());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_009
 * @tc.desc: Verify the ReplaceInst function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerBasicBlockTest, compiler_basicblock_test_009, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph,
        std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto phi : bb->PhiInsts()) {
                status = true;
                auto phi1 = graph->CreateInstPhi(DataType::ANY, 0U);
                bb->ReplaceInst(phi, phi1);
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_010
 * @tc.desc: Verify the InsertRangeBefore function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerBasicBlockTest, compiler_basicblock_test_010, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func2";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        status = true;
        auto block = graph->GetBlocksRPO()[0];
        block->InsertRangeBefore(block->GetFirstInst(), block->GetLastInst(), graph->GetFirstConstInst());
        EXPECT_EQ(block->GetLastInst()->GetNext(), graph->GetFirstConstInst());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_011
 * @tc.desc: Verify the InvalidateLoopIfIrreducible function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerBasicBlockTest, compiler_basicblock_test_011, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    bool status1 = false;
    bool status2 = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status, &status1, &status2](Graph* graph,
        std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        uint32_t id = 2;
        for (auto block : graph->GetBlocksRPO()) {
            if (block->IsTry()) {
                status = true;
                Loop loop(graph->GetAllocator(), block, id);
                loop.SetIsIrreducible(true);
                EXPECT_TRUE(loop.IsIrreducible());
                block->SetLoop(&loop);
                EXPECT_NE(block->GetLoop(), nullptr);
                block->InvalidateLoopIfIrreducible();
            }
            if (block->IsTryBegin()) {
                status1 = true;
                Loop loop1(graph->GetAllocator(), block, id);
                block->SetLoop(&loop1);
                EXPECT_NE(block->GetLoop(), nullptr);
                block->InvalidateLoopIfIrreducible();
            }
            if (block->IsTryEnd()) {
                status2 = true;
                Loop loop2(graph->GetAllocator(), graph->GetEndBlock(), id);
                loop2.SetIsIrreducible(true);
                EXPECT_TRUE(loop2.IsIrreducible());
                block->SetLoop(&loop2);
                EXPECT_NE(block->GetLoop(), nullptr);
            }
        }
    });
    EXPECT_TRUE(status);
    EXPECT_TRUE(status1);
    EXPECT_TRUE(status2);
}

/**
 * @tc.name: compiler_basicblock_test_012
 * @tc.desc: Verify the InsertAfter function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerBasicBlockTest, compiler_basicblock_test_012, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func2";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;

        for (auto block : graph->GetBlocksRPO()) {
            auto inst = graph->CreateInstSaveState(DataType::ANY, 0);
            graph->GetFirstConstInst()->SetBasicBlock(block);
            EXPECT_EQ(graph->GetFirstConstInst()->GetBasicBlock(), block);
            block->InsertAfter(inst, graph->GetFirstConstInst());
            EXPECT_EQ(graph->GetFirstConstInst()->GetNext(), inst);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_013
 * @tc.desc: Verify the AppendRangeInst function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerBasicBlockTest, compiler_basicblock_test_013, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        status = true;
        auto start_block = graph->GetStartBlock();
        auto end_block = graph->GetEndBlock();

        graph->GetFirstConstInst()->SetPrev(nullptr);
        EXPECT_EQ(graph->GetFirstConstInst()->GetPrev(), nullptr);
        start_block->AppendRangeInst(graph->GetFirstConstInst(), start_block->GetLastInst());
        EXPECT_EQ(graph->GetFirstConstInst()->GetPrev(), start_block->GetLastInst());
        start_block->AddSucc(end_block);
        EXPECT_NE(start_block->InsertNewBlockToSuccEdge(end_block), nullptr);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_014
 * @tc.desc: Verify the SplitBlockAfterInstruction function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerBasicBlockTest, compiler_basicblock_test_014, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    bool status1 = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status, &status1](Graph* graph,
        std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            if (bb->IsTry() && bb->IsLoopValid()) {
                auto inst = bb->GetLastInst();
                EXPECT_NE(bb->SplitBlockAfterInstruction(inst, false), nullptr);
                EXPECT_NE(bb->SplitBlockAfterInstruction(inst, true), nullptr);
                status = true;
                break;
            }
        }

        for (auto bb : graph->GetVectorBlocks()) {
            for (auto inst : bb->AllInsts()) {
                if (inst->IsPhi() || bb->IsStartBlock() || bb->IsEndBlock()) {
                    continue;
                }
                status1 = true;
                EXPECT_NE(bb->SplitBlockAfterInstruction(inst, true), nullptr);
            }
        }
    });
    EXPECT_TRUE(status);
    EXPECT_TRUE(status1);
}

/**
 * @tc.name: compiler_basicblock_test_015
 * @tc.desc: Verify the IsIfBlock function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerBasicBlockTest, compiler_basicblock_test_015, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    bool status1 = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status, &status1](Graph* graph,
        std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        EXPECT_NE(graph, nullptr);
        auto end_block = graph->GetEndBlock();
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (inst->GetOpcode() == Opcode::IfImm) {
                    status = true;
                    EXPECT_TRUE(bb->IsIfBlock());
                    bb->GetLastInst()->SetOpcode(Opcode::If);
                    EXPECT_TRUE(bb->IsIfBlock());
                }
                if (inst->IsTry()) {
                    status1 = true;
                    EXPECT_FALSE(bb->IsIfBlock());
                }

                EXPECT_FALSE(end_block->IsIfBlock());
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_016
 * @tc.desc: Verify the CopyTryCatchProps function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerBasicBlockTest, compiler_basicblock_test_016, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "testTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        auto bb = graph->GetStartBlock();
        status = true;
        bb->SetTry(true);
        bb->CopyTryCatchProps(bb);
        EXPECT_EQ(bb->GetTryId(), INVALID_ID);
        EXPECT_FALSE(bb->IsOsrEntry());
    });
    EXPECT_TRUE(status);
}
}  // namespace panda::compiler

