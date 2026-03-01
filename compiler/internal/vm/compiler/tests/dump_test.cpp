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

#include "bytecode_optimizer/reg_acc_alloc.h"
#include "bytecode_optimizer/reg_encoder.h"
#include "dump.cpp"
#include "graph.h"
#include "graph_test.h"
#include "inst.h"
#include "locations.h"
#include "optimizer/optimizations/vn.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/regalloc/reg_alloc_resolver.h"
#include "optimizer/optimizations/regalloc/reg_alloc.h"
#include "mem/pool_manager.h"

using namespace testing::ext;

namespace panda::compiler {
class DumpTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    GraphTest graph_test_;
};

/**
 * @tc.name: dump_test_001
 * @tc.desc: Verify the InstId function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_001, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    compiler::options.SetCompilerDumpCompact(true);
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (inst->GetOpcode() != Opcode::SaveState) {
                    auto value1 = InstId(inst, graph->GetAllocator());
                    EXPECT_FALSE(value1.empty());
                }

                status = true;
                auto value = InstId(inst, graph->GetAllocator());
                EXPECT_FALSE(value.empty());
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_002
 * @tc.desc: Verify the PcToString function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_002, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        uint32_t id = 2;
        std::string test_data = "prop: start, bc: 0x00000000\n";
        std::stringstream data;
        BlockProps(graph->GetStartBlock(), &data);
        EXPECT_EQ(data.str(), test_data);
        data.clear();
        data.str("");
        test_data = "prop: end, bc: 0x00000025\n";
        BlockProps(graph->GetEndBlock(), &data);
        EXPECT_EQ(data.str(), test_data);
        Loop loop(graph->GetAllocator(), graph->GetEndBlock(), 1);
        Loop loop1(graph->GetAllocator(), graph->GetEndBlock(), id);
        Loop loop2(graph->GetAllocator(), graph->GetStartBlock(), 0);
        Loop loop3(graph->GetAllocator(), graph->GetStartBlock(), 1);
        loop1.SetIsIrreducible(true);
        graph->GetEndBlock()->SetLoop(&loop);
        loop3.SetAsRoot();
        data.clear();
        data.str("");
        test_data = "prop: end, head, loop 1, bc: 0x00000025\n";
        BlockProps(graph->GetEndBlock(), &data);
        EXPECT_EQ(data.str(), test_data);
        data.clear();
        data.str("");
        test_data = "prop: loop 0, try_begin (id 0), bc: 0x00000015\n"
                    "prop: loop (irreducible) 2, try (id 0), bc: 0x00000015\n"
                    "prop: try_end (id 0), bc: 0x0000001e\n"
                    "prop: catch_begin, catch, bc: 0x00000020\n"
                    "prop: catch_begin, catch, bc: 0x00000020\n"
                    "prop: catch, bc: 0x00000020\n";
        for (auto block : graph->GetBlocksRPO()) {
            if (block->IsTry()) {
                block->SetLoop(&loop1);
                BlockProps(block, &data);
            }
            if (block->IsTryBegin()) {
                block->SetLoop(&loop2);
                BlockProps(block, &data);
            }
            if (block->IsTryEnd()) {
                block->SetLoop(&loop3);
                BlockProps(block, &data);
            }
            if (block->IsCatch()) {
                block->SetLoop(nullptr);
                BlockProps(block, &data);
            }
            if (block->IsCatchBegin() || block->IsCatchEnd()) {
                BlockProps(block, &data);
            }
        }
        auto value1 = PcToString(graph->GetEndBlock()->GetGuestPc(), graph->GetLocalAllocator());
        std::string str = value1.data();
        EXPECT_EQ(str, "bc: 0x00000025");
        EXPECT_EQ(data.str(), test_data);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_003
 * @tc.desc: Verify the BBId function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_003, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        auto value = BBId(graph->GetStartBlock(), graph->GetAllocator());
        std::string str = value.data();
        EXPECT_EQ(str, "7");
        BasicBlock *block = nullptr;
        str = BBId(block, graph->GetAllocator()).data();
        EXPECT_EQ(str, "null");
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_004
 * @tc.desc: Verify the DumpInputs function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_004, TestSize.Level1)
{
    std::stringstream out;
    uint64_t value = 1;
    ConstantInst constant(Opcode::Constant, value, false);
    constant.DumpInputs(&out);
    EXPECT_EQ(out.str(), "0x1");
    constant.GetInt64Value();
    EXPECT_TRUE(constant.IsBoolConst());

    float float_value = 1.0;
    ConstantInst first_constant(Opcode::Constant, float_value, false);
    out.clear();
    out.str("");
    EXPECT_TRUE(first_constant.DumpInputs(&out));
    EXPECT_EQ(out.str(), "1");

    double double_value = 1.0;
    ConstantInst constant_inst(Opcode::Constant, double_value, false);
    out.clear();
    out.str("");
    EXPECT_TRUE(constant_inst.DumpInputs(&out));
    EXPECT_EQ(out.str(), "1");
    EXPECT_FALSE(constant_inst.IsBoolConst());

    uint32_t integer_value = 1;
    ConstantInst constant_inst1(Opcode::Constant, integer_value, true);
    out.clear();
    out.str("");
    EXPECT_TRUE(constant_inst1.DumpInputs(&out));
    EXPECT_EQ(out.str(), "0x1");
    constant_inst1.GetInt32Value();
    EXPECT_FALSE(constant_inst1.IsBoolConst());

    uint16_t num = 0;
    ConstantInst second_constant(Opcode::Constant, num, false);
    EXPECT_TRUE(second_constant.DumpInputs(&out));
    EXPECT_TRUE(second_constant.IsBoolConst());
    ConstantInst third_constant(Opcode::Constant, num, false);
    auto type = DataType::Type::ANY;
    third_constant.SetType(type);
    out.clear();
    out.str("");
    EXPECT_TRUE(third_constant.DumpInputs(&out));
    EXPECT_EQ(out.str(), "0x0");
    EXPECT_TRUE(third_constant.DumpInputs(&out));
}

/**
 * @tc.name: dump_test_005
 * @tc.desc: Verify the PrintIfValidLocation function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_005, TestSize.Level1)
{
    std::stringstream out;
    uintptr_t value = 255;  // 255: random number
    uintptr_t num = 2;  // 2: random number
    Location location(Location::Kind::REGISTER, value);
    PrintIfValidLocation(location, Arch::AARCH64, &out, false);
    EXPECT_EQ(out.str(), "");

    Location locations_info(Location::Kind::FP_REGISTER, num);
    out.clear();
    out.str("");
    PrintIfValidLocation(locations_info, Arch::AARCH64, &out, true);
    EXPECT_EQ(out.str(), "()");
    auto unEqual = location.operator==(locations_info);
    EXPECT_FALSE(unEqual);

    Location locations(Location::Kind::STACK_ARGUMENT);
    EXPECT_FALSE(locations.IsInvalid());
    out.clear();
    out.str("");
    PrintIfValidLocation(locations, Arch::AARCH64, &out, false);
    EXPECT_EQ(out.str(), " ");
    EXPECT_FALSE(locations.IsUnallocatedRegister());

    out.clear();
    out.str("");
    Location invalid_location(Location::Kind::INVALID);
    PrintIfValidLocation(invalid_location, Arch::AARCH64, &out, false);
    EXPECT_EQ(out.str(), "");
}

/**
 * @tc.name: dump_test_006
 * @tc.desc: Verify the GetCondCodeToString function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_006, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        IfImmInst imm_inst(ConditionCode::CC_EQ);
        auto value = GetCondCodeToString(imm_inst.GetCc(), graph->GetAllocator());
        std::string str = value.data();
        EXPECT_EQ(str, "EQ");
        IfImmInst inst(ConditionCode::CC_NE);
        auto value1 = GetCondCodeToString(inst.GetCc(), graph->GetAllocator());
        str = value1.data();
        EXPECT_EQ(str, "NE");

        IfImmInst ifImm_inst(ConditionCode::CC_LT);
        auto result = GetCondCodeToString(ifImm_inst.GetCc(), graph->GetAllocator());
        str = result.data();
        EXPECT_EQ(str, "LT");
        IfImmInst ifImm(ConditionCode::CC_LE);
        auto num = GetCondCodeToString(ifImm.GetCc(), graph->GetAllocator());
        str = num.data();
        EXPECT_EQ(str, "LE");

        IfImmInst ifInst(ConditionCode::CC_GE);
        auto number = GetCondCodeToString(ifInst.GetCc(), graph->GetAllocator());
        str = number.data();
        EXPECT_EQ(str, "GE");
        IfImmInst imm_inst1(ConditionCode::CC_GT);
        auto info = GetCondCodeToString(imm_inst1.GetCc(), graph->GetAllocator());
        str = info.data();
        EXPECT_EQ(str, "GT");

        IfImmInst ifImm_inst1(ConditionCode::CC_B);
        auto testData = GetCondCodeToString(ifImm_inst1.GetCc(), graph->GetAllocator());
        str = testData.data();
        EXPECT_EQ(str, "B");
        IfImmInst first_imm_inst(ConditionCode::CC_BE);
        auto code = GetCondCodeToString(first_imm_inst.GetCc(), graph->GetAllocator());
        str = code.data();
        EXPECT_EQ(str, "BE");

        IfImmInst first_inst(ConditionCode::CC_A);
        auto number1 = GetCondCodeToString(first_inst.GetCc(), graph->GetAllocator());
        str = number1.data();
        EXPECT_EQ(str, "A");
        IfImmInst second_inst(ConditionCode::CC_AE);
        auto number2 = GetCondCodeToString(second_inst.GetCc(), graph->GetAllocator());
        str = number2.data();
        EXPECT_EQ(str, "AE");

        IfImmInst first_imm_inst1(ConditionCode::CC_TST_EQ);
        auto datum = GetCondCodeToString(first_imm_inst1.GetCc(), graph->GetAllocator());
        str = datum.data();
        EXPECT_EQ(str, "TST_EQ");

        IfImmInst second_imm_inst(ConditionCode::CC_TST_NE);
        auto datum1 = GetCondCodeToString(second_imm_inst.GetCc(), graph->GetAllocator());
        str = datum1.data();
        EXPECT_EQ(str, "TST_NE");

        IfImmInst if_inst2(ConditionCode::CC_FIRST);
        auto data = GetCondCodeToString(if_inst2.GetCc(), graph->GetAllocator());
        str = data.data();
        EXPECT_EQ(str, "EQ");
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_007
 * @tc.desc: Verify the DumpOpcode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_007, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        std::string test_data = "                  Parameter";
        std::stringstream out;
        auto inst = graph->GetStartBlock()->GetFirstInst();
        inst->DumpOpcode(&out);
        EXPECT_EQ(out.str(), test_data);
        EXPECT_FALSE(inst->IsSaveState());
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_008
 * @tc.desc: Verify the DumpInputs function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_008, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        std::string test_data = "arg 0arg 1arg 2";
        std::stringstream out;
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (inst->GetOpcode() != Opcode::Parameter) {
                    continue;
                }
                status = true;
                auto parameterInst = inst->CastToParameter();
                EXPECT_TRUE(parameterInst->DumpInputs(&out));
            }
        }
        EXPECT_EQ(out.str(), test_data);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_009
 * @tc.desc: Verify the DumpInputs function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_009, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func4";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        graph->RunPass<Cleanup>();
        graph->RunPass<bytecodeopt::RegAccAlloc>();
        graph->RunPass<Cleanup>();
        EXPECT_TRUE(RegAlloc(graph));
        graph->RunPass<Cleanup>();
        EXPECT_TRUE(graph->RunPass<bytecodeopt::RegEncoder>());
        std::string test_data = " ->  [u64],  ->  [u64],  ->  [u64],  ->  [u64],  ->  [u64]";
        std::stringstream out;
        for (auto block : graph->GetVectorBlocks()) {
            if (block == nullptr) {
                continue;
            }
            for (auto inst : block->AllInsts()) {
                if (inst->GetOpcode() != Opcode::SpillFill) {
                    continue;
                }
                status = true;
                auto spillFill = inst->CastToSpillFill();
                EXPECT_TRUE(spillFill->DumpInputs(&out));
            }
        }
        EXPECT_EQ(out.str(), test_data);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_010
 * @tc.desc: Verify the DumpTypedOpcode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_010, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        std::string test_data = "                SaveState 1";
        std::stringstream out;
        uint32_t typeId = 1;
        auto inst = graph->CreateInst(Opcode::SaveState);
        DumpTypedOpcode(&out, inst->GetOpcode(), typeId, graph->GetLocalAllocator());
        EXPECT_TRUE(inst->IsSaveState());
        EXPECT_EQ(out.str(), test_data);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_011
 * @tc.desc: Verify the DumpOpcode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_011, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        std::stringstream data;
        std::string test_data = "             Compare EQ any             Compare EQ any             Compare EQ any"
                                "             Compare EQ any             Compare EQ any";
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                auto value = inst->IsZeroRegInst();
                EXPECT_FALSE(value);
                if (inst->GetOpcode() != Opcode::Compare) {
                    continue;
                }
                status = true;
                auto compare = inst->CastToCompare();
                compare->DumpOpcode(&data);
            }
        }
        EXPECT_EQ(data.str(), test_data);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_012
 * @tc.desc: Verify the DumpOpcode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_012, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "foo";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        std::stringstream data;
        std::string test_data = "                 IfImm NE b";
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (inst->GetOpcode() != Opcode::IfImm) {
                    continue;
                }
                status = true;
                auto ifImmInst = inst->CastToIfImm();
                ifImmInst->DumpOpcode(&data);
                EXPECT_TRUE(inst->IsAccRead());
            }
        }
        EXPECT_EQ(data.str(), test_data);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_013
 * @tc.desc: Verify the DumpInputs function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_013, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "foo";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        std::stringstream data;
        std::string test_data = "v10, 0x0";
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (inst->GetOpcode() != Opcode::IfImm) {
                    continue;
                }
                status = true;
                auto ifImmInst = inst->CastToIfImm();
                EXPECT_TRUE(ifImmInst->DumpInputs(&data));
            }
        }
        EXPECT_EQ(data.str(), test_data);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_014
 * @tc.desc: Verify the DumpOpcode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_014, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        std::stringstream data;
        std::string test_data = "       Intrinsic.definefunc"
                                "       Intrinsic.stglobalvar        Intrinsic.definefunc"
                                "       Intrinsic.stglobalvar Intrinsic.trystglobalbyname Intrinsic.tryldglobalbyname"
                                "   Intrinsic.returnundefined ";
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (inst->GetOpcode() != Opcode::Intrinsic) {
                    continue;
                }
                status = true;
                auto intrinsic = inst->CastToIntrinsic();
                intrinsic->DumpOpcode(&data);
            }
        }
        EXPECT_EQ(data.str(), test_data);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_015
 * @tc.desc: Verify the DumpOpcode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_015, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        LoadFromPool loadFromPool;
        std::stringstream out;
        std::string test_data = "               LoadString 1";
        uint32_t id = 1;
        uint32_t guestPc = 5;  // 5: random number
        BasicBlock block(graph, guestPc);
        loadFromPool.SetOpcode(Opcode::LoadString);
        EXPECT_EQ(loadFromPool.GetOpcode(), Opcode::LoadString);
        loadFromPool.SetBasicBlock(&block);
        EXPECT_NE(loadFromPool.GetBasicBlock(), nullptr);
        loadFromPool.SetTypeId(id);
        EXPECT_EQ(loadFromPool.GetTypeId(), 1);
        loadFromPool.DumpOpcode(&out);
        auto result = block.IsCatch();
        EXPECT_FALSE(result);
        EXPECT_EQ(out.str(), test_data);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_016
 * @tc.desc: Verify the DumpTypedFieldOpcode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_016, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "foo";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        std::stringstream out;
        std::string test_data = "          Phi 1 field_name ";
        ArenaString fieldName("field_name", graph->GetAllocator()->Adapter());
        uint32_t typeId = 1;
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->PhiInsts()) {
                DumpTypedFieldOpcode(&out, inst->GetOpcode(), typeId, fieldName, graph->GetAllocator());
                EXPECT_TRUE(inst->IsPhi());
            }
        }
        EXPECT_EQ(out.str(), test_data);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_017
 * @tc.desc: Verify the DumpOpcode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_017, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        std::stringstream out;
        std::string test_data = "CastValueToAnyType UNDEFINED_TYPE ";
        uint32_t guestPc = 5;
        BasicBlock block(graph, guestPc);
        CastValueToAnyTypeInst castValueToAnyType;
        castValueToAnyType.SetOpcode(Opcode::CastValueToAnyType);
        castValueToAnyType.SetBasicBlock(&block);
        castValueToAnyType.SetAnyType(AnyBaseType::UNDEFINED_TYPE);
        castValueToAnyType.DumpOpcode(&out);
        EXPECT_EQ(out.str(), test_data);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_018
 * @tc.desc: Verify the Dump function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_018, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        std::stringstream out;
        std::string test_data = "Method: L_GLOBAL;::func_main_0\n\n"
                                "BB 7\n"
                                "prop: start, bc: 0x00000000\n"
                                "    5.any  Parameter                  "
                                "arg 0                                                           \n ->  [u64]\n"
                                "    6.any  Parameter                  "
                                "arg 1                                                           \n ->  [u64]\n"
                                "    7.any  Parameter                  "
                                "arg 2                                                           \n ->  [u64]\n"
                                "   16.i32  Constant                   0x1 -> "
                                "(v17)                                                    \n"
                                "succs: [bb 0]\n\n"
                                "BB 0  preds: [bb 7]\n"
                                "prop: bc: 0x00000000\n"
                                "    8.any  Intrinsic.definefunc        ss9 -> (v10)"
                                "                                                    \n"
                                "   10.void Intrinsic.stglobalvar       v8, ss11"
                                "                                                        \n"
                                "   12.any  Intrinsic.definefunc        ss13 -> (v14)"
                                "                                                   \n"
                                "   14.void Intrinsic.stglobalvar       v12, ss15"
                                "                                                       \n"
                                "succs: [bb 2]\n\n"
                                "BB 2  preds: [bb 0]\n"
                                "prop: try_begin (id 0), bc: 0x00000015\n"
                                "    0.     Try                                   "
                                "                                                     \n"
                                "succs: [bb 4, bb 9]\n\n"
                                "BB 4  preds: [bb 2]\n"
                                "prop: try (id 0), bc: 0x00000015\n"
                                "   17.void Intrinsic.trystglobalbyname v16, ss18"
                                "                                                       \n"
                                "succs: [bb 3]\n\n"
                                "BB 3  preds: [bb 4]\n"
                                "prop: try_end (id 0), bc: 0x0000001e\n"
                                "succs: [bb 5, bb 9]\n\n"
                                "BB 9  preds: [bb 2, bb 3]\n"
                                "prop: catch_begin, catch, bc: 0x00000020\n"
                                "succs: [bb 6]\n\n"
                                "BB 6  preds: [bb 9]\n"
                                "prop: catch, bc: 0x00000020\n"
                                "   19.any  Intrinsic.tryldglobalbyname ss20                        "
                                "                                    \n"
                                "succs: [bb 1]\n\n"
                                "BB 5  preds: [bb 3]\n"
                                "prop: bc: 0x0000001e\n"
                                "succs: [bb 1]\n\n"
                                "BB 1  preds: [bb 5, bb 6]\n"
                                "prop: bc: 0x00000024\n"
                                "   25.void Intrinsic.returnundefined   ss26"
                                "                                                            \n"
                                "succs: [bb 8]\n\n"
                                "BB 8  preds: [bb 1]\n"
                                "prop: end, bc: 0x00000025\n\n";
        graph->Dump(&out);
        EXPECT_TRUE(graph->HasEndBlock());
        EXPECT_EQ(out.str(), test_data);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_019
 * @tc.desc: Verify the Dump function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_019, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);
        std::stringstream out;
        std::string test_data = "BB 7\n"
                                "prop: start, bc: 0x00000000\n"
                                "    5.any  Parameter                  arg 0"
                                "                                                           \n ->  [u64]\n"
                                "    6.any  Parameter                  arg 1"
                                "                                                           \n ->  [u64]\n"
                                "    7.any  Parameter                  arg 2"
                                "                                                           \n ->  [u64]\n"
                                "   16.i32  Constant                   0x1 -> (v17)"
                                "                                                    \n"
                                "succs: [bb 0]\n"
                                "BB 0  preds: [bb 7]\n"
                                "prop: bc: 0x00000000\n"
                                "    8.any  Intrinsic.definefunc        ss9 -> (v10)"
                                "                                                    \n"
                                "   10.void Intrinsic.stglobalvar       v8, ss11"
                                "                                                        \n"
                                "   12.any  Intrinsic.definefunc        ss13 -> (v14)"
                                "                                                   \n"
                                "   14.void Intrinsic.stglobalvar       v12, ss15"
                                "                                                       \n"
                                "succs: [bb 2]\n"
                                "BB 2  preds: [bb 0]\n"
                                "prop: try_begin (id 0), bc: 0x00000015\n"
                                "    0.     Try                                                       "
                                "                                 \n"
                                "succs: [bb 4, bb 9]\n"
                                "BB 4  preds: [bb 2]\n"
                                "prop: try (id 0), bc: 0x00000015\n"
                                "   17.void Intrinsic.trystglobalbyname v16, ss18"
                                "                                                       \n"
                                "succs: [bb 3]\n"
                                "BB 3  preds: [bb 4]\n"
                                "prop: try_end (id 0), bc: 0x0000001e\n"
                                "succs: [bb 5, bb 9]\n"
                                "BB 9  preds: [bb 2, bb 3]\n"
                                "prop: catch_begin, catch, bc: 0x00000020\n"
                                "succs: [bb 6]\n"
                                "BB 6  preds: [bb 9]\n"
                                "prop: catch, bc: 0x00000020\n"
                                "   19.any  Intrinsic.tryldglobalbyname ss20"
                                "                                                            \n"
                                "succs: [bb 1]\n"
                                "BB 5  preds: [bb 3]\n"
                                "prop: bc: 0x0000001e\n"
                                "succs: [bb 1]\n"
                                "BB 1  preds: [bb 5, bb 6]\n"
                                "prop: bc: 0x00000024\n"
                                "   25.void Intrinsic.returnundefined   ss26"
                                "                                                            \n"
                                "succs: [bb 8]\n"
                                "BB 8  preds: [bb 1]\n"
                                "prop: end, bc: 0x00000025\n";
        for (auto bb : graph->GetBlocksRPO()) {
            bb->Dump(&out);
        }
        EXPECT_EQ(out.str(), test_data);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_020
 * @tc.desc: Verify the AppendImmediate function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_020, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "regallocTest.abc";
    const char *test_method_name = "func5";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        uint64_t immediate = 1;
        uint16_t vreg = 2;  // 2: random number
        bool isAcc = true;
        std::stringstream out;
        std::string test_data = "0x1(acc)0x1(acc)0x1(acc)0x1(acc)0x1(acc)0x1(acc)0x1(acc)0x1(acc)0x1(acc)0x1(acc)0x1"
                                "(acc)0x1(acc)0x1(acc)0x1(acc)";

        for (auto bb : graph->GetVectorBlocks()) {
            for (auto inst : bb->AllInsts()) {
                if (inst->GetOpcode() != Opcode::SaveState) {
                    continue;
                }
                status = true;
                auto saveState = inst->CastToSaveState();
                saveState->AppendImmediate(immediate, vreg, DataType::Type::INT64, isAcc);
                EXPECT_TRUE(saveState->DumpInputs(&out));
            }
        }
        EXPECT_EQ(out.str(), test_data);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: dump_test_021
 * @tc.desc: Verify the DumpInputs function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(DumpTest, dump_test_021, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    const char *test_method_name = "foo";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        std::stringstream out;
        std::string test_data = "v4(bb0), v5(bb2)";
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->PhiInsts()) {
                auto phi = inst->CastToPhi();
                status = true;
                EXPECT_TRUE(phi->DumpInputs(&out));
            }
        }

        EXPECT_EQ(out.str(), test_data);
    });
    EXPECT_TRUE(status);
}
}  // namespace panda::compiler

