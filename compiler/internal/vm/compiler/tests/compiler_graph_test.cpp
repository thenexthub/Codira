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
#include <memory>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "bytecode_optimizer/reg_encoder.h"
#include "bytecode_optimizer/reg_acc_alloc.h"
#include "compiler_options.h"
#include "constants.h"
#include "compiler/optimizer/analysis/rpo.h"
#include "compiler/optimizer/ir_builder/phi_resolver.h"
#include "compiler/optimizer/optimizations/vn.h"
#include "compiler/optimizer/optimizations/lowering.h"
#include "compiler/optimizer/optimizations/move_constants.h"
#include "compiler/optimizer/pass_manager.h"
#include "graph.h"
#include "graph.cpp"
#include "graph_checker.h"
#include "graph_test.h"
#include "graph_visitor.h"
#include "inst.h"
#include "locations.h"
#include "mem/pool_manager.h"

using namespace testing::ext;

namespace panda::compiler {

class CompilerGraphTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    GraphTest graph_test_;
};

/**
 * @tc.name: graph_test_001
 * @tc.desc: Verify the MarkBlocksRec function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest, graph_test_001, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        auto start_block = graph->GetStartBlock();
        Marker mrk = 16;  // 16: random number
        MarkBlocksRec(mrk, start_block);
        EXPECT_TRUE(start_block->IsMarked(mrk));
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_002
 * @tc.desc: Verify the DisconnectBlockRec function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest, graph_test_002, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);

        BasicBlock *bb = nullptr;
        for (const auto &block : graph->GetVectorBlocks()) {
            if (block == nullptr) {
                continue;
            }
            bb = block;
        }
        graph->DisconnectBlockRec(bb, true, true);
        EXPECT_NE(bb->GetGraph(), nullptr);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_003
 * @tc.desc: Verify the GetParametersSlotsCount function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest, graph_test_003, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        EXPECT_EQ(graph->GetParametersSlotsCount(), 0);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_004
 * @tc.desc: Verify the GetBranchCounter function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest, graph_test_004, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func5";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        for (const auto &block : graph->GetVectorBlocks()) {
            if (block == nullptr) {
                continue;
            }
            for (auto inst : block->AllInsts()) {
                if (inst->GetOpcode() != Opcode::IfImm) {
                    continue;
                }
                status = true;
                EXPECT_EQ(graph->GetBranchCounter(block, true), 1);
                block->GetLastInst()->SetOpcode(Opcode::If);
                EXPECT_EQ(block->GetLastInst()->GetOpcode(), Opcode::If);
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_005
 * @tc.desc: Verify the RemoveUnreachableBlocks function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest,  graph_test_005, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "testTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        status = true;
        auto size = graph->GetBlocksRPO().size();
        graph->RemoveUnreachableBlocks();
        EXPECT_EQ(graph->GetBlocksRPO().size(), size);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_006
 * @tc.desc: Verify the FindOrAddConstant function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest,  graph_test_006, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func5";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        ConstantInst first_constant(Opcode::Constant, 0, false);
        first_constant.SetType(DataType::Type::ANY);
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (inst->GetOpcode() != Opcode::Constant) {
                    continue;
                }

                status = true;
                auto constant = inst->CastToConstant();
                EXPECT_TRUE(graph->FindOrAddConstant(constant)->HasType());
            }
        }
        EXPECT_NE(graph->FindOrAddConstant(&first_constant), nullptr);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_007
 * @tc.desc: Verify the DumpThrowableInsts function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest,  graph_test_007, TestSize.Level1)
{
    std::stringstream out;
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile,
        [&test_method_name, &out, &status](Graph* graph, std::string &method_name) {
            if (test_method_name != method_name) {
                return;
            }
            status = true;
            EXPECT_NE(graph, nullptr);
            graph->DumpThrowableInsts(&out);
            std::string str = "Throwable Inst   "
                            "17.void Intrinsic.trystglobalbyname v16, v18"
                            "                                                         bc: 0x0000001a\n"
                            "Catch handlers: BB 9\n";
            EXPECT_EQ(out.str(), str);
        });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_008
 * @tc.desc: Verify the GetMethodFullName function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest,  graph_test_011, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        std::string str = GetMethodFullName(graph, graph->GetMethod());
        EXPECT_EQ(str, "L_GLOBAL;::func_main_0");
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_012
 * @tc.desc: Verify the InitDefaultLocations function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest,  graph_test_012, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char* test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        graph->InitDefaultLocations();
        EXPECT_TRUE(graph->IsDefaultLocationsInit());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_013
 * @tc.desc: Verify the MakeStackParameter function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest,  graph_test_013, TestSize.Level1)
{
    Location locations(Location::Kind::STACK_PARAMETER, 2);  // 2: random number
    Location location1(Location::Kind::REGISTER, 1);
    Location location;
    locations.MakeRegister(2, DataType::Type::FLOAT32);  // 2: random number
    EXPECT_TRUE(location.MakeStackSlot(3).IsStack());  // 3: random number
    EXPECT_EQ(location1.MakeStackParameter(4).GetValue(), 4);  // 4: random number
}

/**
 * @tc.name: graph_test_014
 * @tc.desc: Verify the AddNewParameter function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest,  graph_test_014, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char* test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        uint16_t arg_number = 16;  // 16: random number
        auto inst = graph->AddNewParameter(arg_number);
        EXPECT_NE(inst, nullptr);
        EXPECT_TRUE(inst->IsParameter());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_015
 * @tc.desc: Verify the RemoveConstFromList function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest,  graph_test_015, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char* test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        graph->RemoveConstFromList(graph->GetFirstConstInst());
        EXPECT_NE(graph->GetFirstConstInst()->GetNextConst(), nullptr);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_016
 * @tc.desc: Verify the RemoveThrowableInst function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest,  graph_test_016, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (inst->GetOpcode() != Opcode::CatchPhi) {
                    continue;
                }
                status = true;
                auto catch_phi = inst->CastToCatchPhi();
                catch_phi->SetIsAcc();
                graph->AppendThrowableInst(catch_phi, bb);
                EXPECT_TRUE(graph->IsInstThrowable(catch_phi));
                graph->RemoveThrowableInst(catch_phi);
                EXPECT_FALSE(graph->IsInstThrowable(catch_phi));
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_017
 * @tc.desc: Verify the RestoreBlock function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest,  graph_test_017, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        BasicBlock *bb = nullptr;
        for (const auto &block : graph->GetVectorBlocks()) {
            if (block == nullptr) {
                continue;
            }
            bb = block;
        }
        graph->EraseBlock(bb);
        EXPECT_EQ(bb->GetGraph(), nullptr);
        graph->RestoreBlock(bb);
        EXPECT_NE(bb->GetGraph(), nullptr);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_018
 * @tc.desc: Verify the DynamicMethod function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest,  graph_test_018, TestSize.Level1)
{
    std::stringstream out;
    // Numbers are random numbers
    int32_t arg = 20;
    int32_t arg1 = 88;
    GraphMode mode(arg1);
    mode.Native(true);
    mode.Dump(out);
    EXPECT_EQ(out.str(), "Native, FastPath, Interpreter");
    GraphMode mode1(arg);
    mode1.FastPath(true);
    out.clear();
    out.str("");
    mode1.Dump(out);
    EXPECT_EQ(out.str(), "DynamicMethod, FastPath");
    GraphMode graph_mode(arg1);
    graph_mode.Boundary(true);
    out.clear();
    out.str("");
    graph_mode.Dump(out);
    EXPECT_EQ(out.str(), "Native, FastPath, Interpreter");
    GraphMode graph_mode1(arg1);
    graph_mode1.Interpreter(true);
    out.clear();
    out.str("");
    graph_mode1.Dump(out);
    EXPECT_EQ(out.str(), "Native, FastPath, Interpreter");
    GraphMode graph(arg1);
    graph.InterpreterEntry(true);
    out.clear();
    out.str("");
    graph.Dump(out);
    EXPECT_EQ(out.str(), "Native, FastPath, Interpreter");
    GraphMode graph1(arg1);
    graph1.Osr(true);
    out.clear();
    out.str("");
    graph1.Dump(out);
    EXPECT_EQ(out.str(), "Native, FastPath, Interpreter");
    GraphMode graph2(arg1);
    graph2.DynamicMethod(true);
    out.clear();
    out.str("");
    graph2.Dump(out);
    EXPECT_EQ(out.str(), "Native, FastPath, Interpreter");
    EXPECT_FALSE(graph1.SupportManagedCode());
}

/**
 * @tc.name: graph_test_019
 * @tc.desc: Verify the SupportManagedCode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest,  graph_test_019, TestSize.Level1)
{
    // Numbers are random numbers
    int32_t arg = 20;
    int32_t arg1 = 88;
    GraphMode mode(88);
    mode.Native(true);
    EXPECT_TRUE(!mode.SupportManagedCode());
    GraphMode mode1(arg);
    mode1.FastPath(true);
    EXPECT_TRUE(!mode1.SupportManagedCode());

    GraphMode graph_mode(arg1);
    graph_mode.Boundary(true);
    EXPECT_TRUE(!graph_mode.SupportManagedCode());
    GraphMode graph_mode1(arg1);
    graph_mode1.Interpreter(true);
    EXPECT_TRUE(!graph_mode1.SupportManagedCode());
    GraphMode graph(arg1);
    graph.InterpreterEntry(true);
    EXPECT_FALSE(graph.SupportManagedCode());
    GraphMode graph1(arg1);
    graph1.SetNative(false);
    graph1.SetFastPath(false);
    graph1.SetBoundary(false);
    graph1.SetInterpreter(false);
    graph1.SetInterpreterEntry(false);
    graph1.SetOsr(true);
    EXPECT_TRUE(graph1.SupportManagedCode());
}

/**
 * @tc.name: graph_test_020
 * @tc.desc: Verify the MarkLoopExits function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest,  graph_test_020, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        Marker mark = 4;
        EXPECT_NE(graph, nullptr);
        MarkLoopExits(graph, mark);
        EXPECT_FALSE(graph->GetEndBlock()->IsMarked(mark));
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_021
 * @tc.desc: Verify the RemovePredecessors function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest,  graph_test_021, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            graph->RemovePredecessors(bb, true);
            EXPECT_EQ(bb->GetPredsBlocks().size(), 0);
            graph->RemovePredecessors(bb, false);
            EXPECT_EQ(bb->GetPredsBlocks().size(), 0);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_022
 * @tc.desc: Verify the GetParametersSlotsCount function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest,  graph_test_022, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char* test_method_name = "func5";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        status = true;
        ArenaVector<bool> regs =
            ArenaVector<bool>(std::max(MAX_NUM_REGS, MAX_NUM_VREGS), false, graph->GetAllocator()->Adapter());
        graph->InitUsedRegs<DataType::FLOAT64>(&regs);
        graph->SetRegUsage(0, DataType::FLOAT64);
        EXPECT_NE(graph->GetUsedRegs<DataType::FLOAT64>(), nullptr);
        graph->SetExtSlotsStart(0);
        EXPECT_EQ(graph->GetExtSlotsStart(), 0);
        EXPECT_NE(graph->GetRootLoop(), nullptr);
        EXPECT_TRUE(graph->HasLoop());
        EXPECT_FALSE(graph->HasInfiniteLoop());
        EXPECT_EQ(graph->GetParametersSlotsCount(), 0);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_023
 * @tc.desc: Verify the Run function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest, graph_test_023, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char* test_method_name = "func5";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        PhiResolver phi_resolver(graph);
        phi_resolver.Run();
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: graph_test_024
 * @tc.desc: Verify the CreateRootsStackMask function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerGraphTest, graph_test_024, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char* test_method_name = "func5";
    bool status = false;
    bool status1 = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status, &status1](Graph* graph,
        std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);

        for (auto block : graph->GetBlocksRPO()) {
            if (block == nullptr) {
                continue;
            }
            for (auto inst : block->AllInstsSafe()) {
                if (inst->IsPhi()) {
                    status = true;
                    auto phiInst = inst->CastToPhi();
                    phiInst->SetPhiInputBbNum(0, 0);
                    EXPECT_FALSE(inst->IsBoolConst());
                    EXPECT_EQ(phiInst->GetPhiInputBbNum(0), 0);
                }
                if (inst->IsSaveState()) {
                    status1 = true;
                    auto save_inst = inst->CastToSaveState();
                    VirtualRegister reg(0, true);
                    save_inst->CreateRootsStackMask(graph->GetAllocator());
                    EXPECT_NE(save_inst->GetRootsStackMask(), nullptr);
                }
            }
        }
    });
    EXPECT_TRUE(status);
    EXPECT_TRUE(status1);
}
}  // namespace panda::compiler

