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
 
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <gtest/gtest.h>
#include <memory>

#include "basicblock.h"
#include "bytecode_optimizer/reg_encoder.h"
#include "bytecode_optimizer/reg_acc_alloc.h"
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

class CompilerInstTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    GraphTest graph_test_;
};

using CompilerInstDeathTest = CompilerInstTest;

class TestAllocationFailureAllocator : public ArenaAllocator {
public:
    explicit TestAllocationFailureAllocator(SpaceType space_type) : ArenaAllocator(space_type) {}
    ~TestAllocationFailureAllocator() {}

    void *Alloc(size_t size, Alignment align = DEFAULT_ARENA_ALIGNMENT) override
    {
        return nullptr;
    }
};

/**
 * @tc.name: compiler_inst_test_001
 * @tc.desc: Verify the GetInverseConditionCode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_001, TestSize.Level1)
{
    auto cc = ConditionCode::CC_EQ;
    auto data = ConditionCode::CC_NE;
    auto code = GetInverseConditionCode(data);
    EXPECT_EQ(code, cc);
    auto code1 = GetInverseConditionCode(cc);
    EXPECT_EQ(code1, data);
}

/**
 * @tc.name: compiler_inst_test_002
 * @tc.desc: Verify the AllocateImmediates function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_002, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        size_t size = 5;  // 5: random number
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (!inst->IsSaveState()) {
                    continue;
                }
                status = true;
                auto save_state_inst = inst->CastToSaveState();
                save_state_inst->AllocateImmediates(graph->GetAllocator(), size);
                EXPECT_EQ(save_state_inst->GetImmediatesCount(), size);
                EXPECT_TRUE(save_state_inst->IsSaveState());
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_inst_test_003
 * @tc.desc: Verify the SwapOperandsConditionCode function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_003, TestSize.Level1)
{
    IfInst if_inst(ConditionCode::CC_EQ);
    auto cc = if_inst.GetCc();
    IfInst if_Inst(ConditionCode::CC_NE);
    auto data = if_Inst.GetCc();
    auto code = SwapOperandsConditionCode(data);
    EXPECT_EQ(code, data);
    auto code1 = SwapOperandsConditionCode(cc);
    EXPECT_EQ(code1, cc);
}

/**
 * @tc.name: compiler_inst_test_004
 * @tc.desc: Verify the Clone function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_004, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func5";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (inst->GetOpcode() != Opcode::Constant) {
                    continue;
                }
                status = true;
                auto constInst = inst->Clone(graph);
                EXPECT_EQ(constInst->GetOpcode(), Opcode::Constant);
                inst->SetType(DataType::FLOAT32);
                constInst = inst->Clone(graph);
                EXPECT_EQ(constInst->GetType(), DataType::FLOAT32);
                inst->SetType(DataType::FLOAT64);
                constInst = inst->Clone(graph);
                EXPECT_EQ(constInst->GetType(), DataType::FLOAT64);
                inst->SetType(DataType::ANY);
                constInst = inst->Clone(graph);
                EXPECT_EQ(constInst->GetType(), DataType::ANY);
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_inst_test_005
 * @tc.desc: Verify the CastToParameter function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_005, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&](Graph* graph, std::string &method_name) {
        auto constInst = graph->GetFirstConstInst();
        auto same = constInst->InSameBlockOrDominate(constInst);
        EXPECT_TRUE(same);
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (inst->GetOpcode() != Opcode::Parameter) {
                    continue;
                }
                status = true;
                auto parameterInst = inst->CastToParameter();
                EXPECT_EQ(parameterInst->Clone(graph)->GetOpcode(), Opcode::Parameter);
            }
        }
    });
    EXPECT_TRUE(true);
}

/**
 * @tc.name: compiler_inst_test_006
 * @tc.desc: Verify the InSameBlockOrDominate function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_006, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "moduleTryCatch.abc";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&](Graph* graph, std::string &method_name) {
        const auto constInst = graph->GetFirstConstInst();
        auto same = constInst->InSameBlockOrDominate(constInst);
        EXPECT_TRUE(same);
        const auto startBlock = graph->GetStartBlock();
        const auto firstInst = startBlock->GetFirstInst();

        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (firstInst->GetBasicBlock() != inst->GetBasicBlock()) {
                    continue;
                }
                status = true;
                EXPECT_TRUE(firstInst->InSameBlockOrDominate(inst));
            }
        }
    });
    EXPECT_TRUE(true);
}

/**
 * @tc.name: compiler_inst_test_007
 * @tc.desc: Verify the GetImmediatesCount function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_007, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        uint64_t imm = 1;
        uint16_t vreg = 2;
        bool isAcc = true;
        EXPECT_NE(graph, nullptr);

        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (!inst->IsSaveState()) {
                    continue;
                }
                status = true;
                auto save_state_inst = inst->CastToSaveState();
                save_state_inst->AppendImmediate(imm, vreg, DataType::Type::INT64, isAcc);
                EXPECT_EQ(save_state_inst->GetImmediatesCount(), 1);
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_inst_test_008
 * @tc.desc: Verify the SetVnObject function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_008, TestSize.Level1)
{
    IfInst if_inst(ConditionCode::CC_EQ);
    VnObject vn_object;
    if_inst.SetVnObject(&vn_object);
    EXPECT_EQ(vn_object.GetSize(), 1);
}

/**
 * @tc.name: compiler_inst_test_009
 * @tc.desc: Verify the SetVnObject function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_009, TestSize.Level1)
{
    CompareInst compareInst(ConditionCode::CC_EQ);
    VnObject vn_object;
    compareInst.SetVnObject(&vn_object);
    EXPECT_EQ(vn_object.GetSize(), 1);
}

/**
 * @tc.name: compiler_inst_test_010
 * @tc.desc: Verify the SetVnObject function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_010, TestSize.Level1)
{
    IfImmInst if_imm_inst(ConditionCode::CC_EQ);
    VnObject vn_object;
    if_imm_inst.SetVnObject(&vn_object);
    EXPECT_EQ(vn_object.GetSize(), 1);
}

/**
 * @tc.name: compiler_inst_test_011
 * @tc.desc: Verify the IsPrecedingInSameBlock function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_011, TestSize.Level1)
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
                if (!inst->IsSaveState()) {
                    continue;
                }
                status = true;
                auto save_state_inst = inst->CastToSaveState();
                EXPECT_TRUE(save_state_inst->IsPrecedingInSameBlock(inst));
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_inst_test_012
 * @tc.desc: Verify the CastToCompare function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_012, TestSize.Level1)
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
                if (inst->GetOpcode() != Opcode::Compare) {
                    continue;
                }
                status = true;
                auto compareInst = inst->CastToCompare();
                EXPECT_EQ(compareInst->Clone(graph)->GetOpcode(), Opcode::Compare);
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_inst_test_013
 * @tc.desc: Verify the CastToSpillFill function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_013, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func4";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        EXPECT_TRUE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<bytecodeopt::RegAccAlloc>());
        EXPECT_FALSE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(RegAlloc(graph));
        EXPECT_FALSE(graph->RunPass<Cleanup>());
        EXPECT_TRUE(graph->RunPass<bytecodeopt::RegEncoder>());
        for (const auto &block : graph->GetVectorBlocks()) {
            for (auto inst : block->AllInsts()) {
                if (inst->GetOpcode() != Opcode::SpillFill) {
                    continue;
                }
                status = true;
                auto spillFill = inst->CastToSpillFill();
                EXPECT_EQ(spillFill->Clone(graph)->GetOpcode(), Opcode::SpillFill);
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_inst_test_014
 * @tc.desc: Verify the IsNativeCall function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_014, TestSize.Level1)
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
                if (!inst->IsIntrinsic()) {
                    continue;
                }
                status = true;
                auto intrinsic_inst = inst->CastToIntrinsic();
                EXPECT_NE(intrinsic_inst->Clone(graph), nullptr);
                EXPECT_FALSE(intrinsic_inst->IsNativeCall());
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_inst_test_015
 * @tc.desc: Verify the GetThrowableInsts function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_015, TestSize.Level1)
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
                auto catch_phi_inst =inst->CastToCatchPhi();
                catch_phi_inst->AppendThrowableInst(graph->GetFirstConstInst());

                auto catch_inst = catch_phi_inst->GetThrowableInsts();
                auto it = std::find(catch_inst->begin(), catch_inst->end(), graph->GetFirstConstInst());
                EXPECT_NE(it, catch_inst->end());

                catch_phi_inst->ReplaceThrowableInst(graph->GetFirstConstInst(), inst);
                auto it1 = std::find(catch_inst->begin(), catch_inst->end(), graph->GetFirstConstInst());
                EXPECT_EQ(it1, catch_inst->end());
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_inst_test_016
 * @tc.desc: Verify the Clone function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_016, TestSize.Level1)
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
                if (inst->GetOpcode() != Opcode::SaveState) {
                    continue;
                }
                status = true;
                auto save_state = inst->CastToSaveState();
                EXPECT_EQ(save_state->Clone(graph)->GetOpcode(), Opcode::SaveState);
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_inst_test_017
 * @tc.desc: Verify the FindOrCreateConstant function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_017, TestSize.Level1)
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
        int32_t int32_const[3] = {-5, 0, 5};
        int64_t int64_const[3] = {-5, 0, 5};

        auto start = graph->GetStartBlock();
        for (auto i = 0; i < 3; i++) {
            // add first int32 constant
            int32_t val = int32_const[i];
            auto const1 = graph->FindOrCreateConstant(val);
            EXPECT_EQ(const1->GetType(), DataType::INT32);
            EXPECT_EQ(const1->GetBasicBlock(), start);
            uint64_t val1 = int64_const[i];
            // add int64 constant, graph creates new constant
            auto const2 = graph->FindOrCreateConstant(val1);
            EXPECT_EQ(const2->GetType(), DataType::INT64);
            EXPECT_EQ(const2->GetBasicBlock(), start);
            EXPECT_EQ(const1->GetIntValue(), val1);
            // add second int32 constant, graph doesn't create new constant
            int32_t val2 = int32_const[i];
            auto const3 = graph->FindOrCreateConstant(val2);
            EXPECT_EQ(const3, const1);
            EXPECT_EQ(const1->GetInt32Value(), val2);
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_inst_test_018
 * @tc.desc: Verify the AppendCatchTypeId function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_018, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        uint32_t id = 7;
        uint32_t catchIndex = 5;
        for (auto bb : graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                if (!inst->IsTry()) {
                    continue;
                }
                status = true;
                TryInst *try_inst = inst->CastToTry();
                try_inst->AppendCatchTypeId(id, catchIndex);
                EXPECT_EQ(try_inst->Clone(graph)->GetOpcode(), Opcode::Try);
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_inst_test_019
 * @tc.desc: Verify the AppendCatchTypeId function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_019, TestSize.Level1)
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
                if (!inst->IsTry()) {
                    continue;
                }
                status = true;
                auto try_inst = inst->CastToTry();
                try_inst->AppendCatchTypeId(1, 1);
                auto catch_type_id = try_inst->GetCatchTypeIds();
                auto it1 = std::find(catch_type_id->begin(), catch_type_id->end(), 1);
                EXPECT_NE(it1, catch_type_id->end());
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_inst_test_020
 * @tc.desc: Verify the GetCommonType function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_020, TestSize.Level1)
{
    auto temp = DataType::Type::INT64;
    auto type = DataType::Type::UINT8;
    auto type1 = DataType::Type::UINT16;
    auto str = DataType::Type::REFERENCE;
    auto value = GetCommonType(type);
    EXPECT_EQ(value, temp);

    auto value1 = GetCommonType(type1);
    EXPECT_EQ(value1, temp);

    auto result = GetCommonType(str);
    EXPECT_EQ(result, str);
}

/**
 * @tc.name: compiler_inst_test_021
 * @tc.desc: Verify the IsInt32Bit function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_021, TestSize.Level1)
{
    auto type = DataType::Type::UINT8;
    auto type1 = DataType::Type::UINT16;
    auto str = DataType::Type::REFERENCE;
    auto value = IsInt32Bit(type);
    EXPECT_TRUE(value);

    auto value1 = IsInt32Bit(type1);
    EXPECT_TRUE(value1);

    auto result = IsInt32Bit(str);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: compiler_basicblock_test_022
 * @tc.desc: Verify the Is32Bits function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_022, TestSize.Level1)
{
    auto type = DataType::Type::UINT8;
    auto type1 = DataType::Type::UINT16;
    auto str = DataType::Type::REFERENCE;
    auto str1 = DataType::Type::POINTER;
    auto vi = DataType::Type::VOID;
    auto value1 = Is32Bits(type, Arch::NONE);
    EXPECT_TRUE(value1);

    auto value = Is32Bits(type1, Arch::NONE);
    EXPECT_TRUE(value);

    auto result = Is32Bits(str, Arch::AARCH32);
    EXPECT_TRUE(result);

    auto result1 = Is32Bits(str1, Arch::AARCH64);
    EXPECT_FALSE(result1);

    auto result2 = Is32Bits(vi, Arch::AARCH64);
    EXPECT_FALSE(result2);
}

/**
 * @tc.name: compiler_basicblock_test_023
 * @tc.desc: Verify the IsTypeCollectable function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_023, TestSize.Level1)
{
    auto type = DataType::Type::REFERENCE;
    auto value = IsTypeCollectable(type);
    EXPECT_TRUE(value);

    auto type1 = DataType::Type::UINT16;
    auto value1 = IsTypeCollectable(type1);
    EXPECT_FALSE(value1);
}

/**
 * @tc.name: compiler_basicblock_test_024
 * @tc.desc: Verify the GetIntTypeForReference function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_024, TestSize.Level1)
{
    auto type = DataType::Type::UINT32;
    auto value = DataType::GetIntTypeForReference(Arch::AARCH32);
    EXPECT_EQ(value, type);
}

/**
 * @tc.name: compiler_basicblock_test_025
 * @tc.desc: Verify the IsTypeSigned function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_025, TestSize.Level1)
{
    auto type = DataType::Type::REFERENCE;
    auto value = IsTypeSigned(type);
    EXPECT_FALSE(value);

    auto type1 = DataType::Type::INT8;
    auto value1 = IsTypeSigned(type1);
    EXPECT_TRUE(value1);
}

/**
 * @tc.name: compiler_basicblock_test_026
 * @tc.desc: Verify the GetPhiInputBb function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_026, TestSize.Level1)
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
            for (auto inst : bb->PhiInsts()) {
                status = true;
                auto phi = inst->CastToPhi();
                auto block = phi->GetPhiInputBb(0);
                EXPECT_NE(phi->GetPhiInput(block), nullptr);
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_027
 * @tc.desc: Verify the GetEdgeIfInputFalse function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_027, TestSize.Level1)
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
                if (inst->GetOpcode() != Opcode::IfImm) {
                    continue;
                }
                status = true;
                auto if_imm_inst = inst->CastToIfImm();
                EXPECT_NE(if_imm_inst->GetEdgeIfInputTrue(), nullptr);
                EXPECT_NE(if_imm_inst->GetEdgeIfInputFalse(), nullptr);
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_028
 * @tc.desc: Verify the GetSaveState function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_028, TestSize.Level1)
{
    SaveStateInst save_state1;
    save_state1.SetFlag(inst_flags::Flags::CAN_THROW);
    EXPECT_EQ(save_state1.GetSaveState(), nullptr);

    SaveStateInst save_state2;
    save_state2.SetFlag(inst_flags::Flags::REQUIRE_STATE);
    EXPECT_EQ(save_state2.GetSaveState(), nullptr);
}

/**
 * @tc.name: compiler_basicblock_test_029
 * @tc.desc: Verify the ReplaceInput function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_029, TestSize.Level1)
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
                status = true;
                if (inst->GetInputsCount() != 0) {
                    status = true;
                    auto inst1 = graph->CreateInstTry(DataType::ANY, 0);
                    inst->ReplaceInput(inst->GetInput(0).GetInst(), inst1);
                }
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_030
 * @tc.desc: Verify the RemoveInputs function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_030, TestSize.Level1)
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
                status = true;
                inst->RemoveInputs();
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_031
 * @tc.desc: Verify the IsInvalid function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_031, TestSize.Level1)
{
    IfInst if_inst(ConditionCode::CC_EQ);
    Location location(Location::Kind::REGISTER, 253);  // 253: random number
    if_inst.SetLikely();
    EXPECT_TRUE(if_inst.IsLikely());
    if_inst.InverseConditionCode();
    IfInst if_inst1(ConditionCode::CC_NE);
    if_inst1.SetUnlikely();
    EXPECT_TRUE(if_inst.IsUnlikely());
}

/**
 * @tc.name: compiler_basicblock_test_032
 * @tc.desc: Verify the AllocateInputTypes function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_032, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "styleTryCatch.abc";
    const char *test_method_name = "func_main_0";
    bool status = false;
    graph_test_.TestBuildGraphFromFile(pfile, [&test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        size_t size = 2;  // 2: random number
        EXPECT_NE(graph, nullptr);
        InputTypesMixin input_types_mixin;
        input_types_mixin.AllocateInputTypes(graph->GetAllocator(), size);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_033
 * @tc.desc: Verify the RequireRegMap function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_033, TestSize.Level1)
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
            if (!bb->IsTryBegin()) {
                continue;
            }
            for (auto inst : bb->AllInsts()) {
                if (!inst->IsTry()) {
                    continue;
                }
                status = true;
                auto try_inst = GetTryBeginInst(bb);
                EXPECT_NE(try_inst, nullptr);
                EXPECT_FALSE(try_inst->RequireRegMap());
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_basicblock_test_034
 * @tc.desc: Verify the CloneTypes function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_034, TestSize.Level1)
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
        InputTypesMixin input_types_mixin;
        InputTypesMixin input_types_mixin1;
        input_types_mixin1.AllocateInputTypes(graph->GetAllocator(), 0);
        EXPECT_NE(input_types_mixin1.GetInputTypes(), nullptr);
        input_types_mixin1.CloneTypes(graph->GetAllocator(), &input_types_mixin);
        EXPECT_EQ(input_types_mixin1.GetInputTypes()->size(), 0);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_inst_test_035
 * @tc.desc: Verify the ResolveOffsetByIndex function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_035, TestSize.Level1)
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
        auto method = graph->GetMethod();
        RuntimeInterface interface;
        EXPECT_EQ(interface.ResolveOffsetByIndex(method, 0), 0);
        EXPECT_EQ(interface.GetBinaryFileForMethod(method), nullptr);
        EXPECT_EQ(interface.GetMethodId(method), 0);
        EXPECT_EQ(interface.GetMethodTotalArgumentsCount(method), 0);
        EXPECT_EQ(interface.GetMethodArgumentsCount(method, 0), 0);
        EXPECT_EQ(interface.GetMethodArgumentsCount(method), 0);
        EXPECT_EQ(interface.GetMethodRegistersCount(method), 0);
        EXPECT_EQ(interface.GetClassNameFromMethod(method), "UnknownClass");
        EXPECT_EQ(interface.GetMethodName(method), "UnknownMethod");
        EXPECT_EQ(interface.GetMethodCode(method), nullptr);
        EXPECT_EQ(interface.GetMethodCodeSize(method), 0);
        EXPECT_EQ(interface.GetMethodSourceLanguage(method), SourceLanguage::PANDA_ASSEMBLY);
        EXPECT_EQ(interface.GetClass(method), nullptr);
        EXPECT_EQ(interface.GetMethodFullName(method, true), "UnknownMethod");
        EXPECT_EQ(interface.GetClassIdForMethod(method), 0);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: compiler_inst_test_036
 * @tc.desc: Verify the SetVirtualRegister function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CompilerInstTest, compiler_inst_test_036, TestSize.Level1)
{
    User user(false, 0, 1);
    user.GetVirtualRegister();
    VirtualRegister reg;
    user.SetVirtualRegister(reg);
    EXPECT_TRUE(user.GetVirtualRegister().IsAccumulator());
    EXPECT_TRUE(user.IsDynamic());
}

/**
 * @tc.name: compiler_inst_test_037
 * @tc.desc: Check nullptr when allocation fails.
 * @tc.type: FUNC
 * @tc.require: #I9TFIC
 */
HWTEST_F(CompilerInstDeathTest, compiler_inst_test_037, TestSize.Level1)
{
    TestAllocationFailureAllocator test_allocator {SpaceType::SPACE_TYPE_COMPILER};
    CHECK_NOT_NULL(&test_allocator);
    EXPECT_DEATH(CHECK_NOT_NULL(nullptr), "CHECK FAILED.*");
    EXPECT_DEATH(Inst::New<SpillFillInst>(&test_allocator, Opcode::SpillFill), "CHECK FAILED.*");
    EXPECT_DEATH(Inst::New<ConstantInst>(&test_allocator, Opcode::Constant), "CHECK FAILED.*");
    EXPECT_DEATH(Inst::New<SaveStateInst>(&test_allocator, Opcode::SaveState), "CHECK FAILED.*");
    EXPECT_DEATH(Inst::New<CompareInst>(&test_allocator, Opcode::Compare), "CHECK FAILED.*");
}

}  // namespace panda::compiler

