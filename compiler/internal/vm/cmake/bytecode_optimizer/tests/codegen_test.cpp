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

#include "assembler/assembly-parser.h"
#include "codegen.h"
#include "compiler/optimizer/optimizations/cleanup.h"
#include "compiler/optimizer/optimizations/lowering.h"
#include "compiler/optimizer/optimizations/move_constants.h"
#include "compiler/optimizer/optimizations/regalloc/reg_alloc.h"
#include "compiler/optimizer/optimizations/vn.h"
#include "reg_acc_alloc.h"
#include "reg_encoder.h"
#include "graph_test.h"
#include "mem/pool_manager.h"

using namespace testing::ext;

namespace panda::bytecodeopt {
using namespace compiler;
using namespace pandasm;
using namespace panda;
class CodegenTest : public testing::Test {
public:
    CodegenTest()
    {
        options.SetCompilerUseSafepoint(false);
    }
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}

    GraphTest graph_test_;
};

std::unique_ptr<const panda_file::File> ParseAndEmit(const std::string &source)
{
    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    EXPECT_EQ(parser.ShowError().err, Error::ErrorType::ERR_NONE);
    auto &program = res.Value();
    return AsmEmitter::Emit(program);
}

/**
 * @tc.name: codegen_test_001
 * @tc.desc: Verify the DoLda function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CodegenTest, codegen_test_001, TestSize.Level1)
{
    Register reg = 1;  // 1: It's a random number
    std::vector<InsPtr> result;
    DoLda(reg, result);
    EXPECT_EQ(result[0]->GetReg(0), reg);
    EXPECT_EQ(result[0]->opcode, panda::pandasm::Opcode::LDA);
}

/**
 * @tc.name: codegen_test_002
 * @tc.desc: Verify the DoSta function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CodegenTest, codegen_test_002, TestSize.Level1)
{
    Register reg = 1;  // 1: It's a random number
    std::vector<InsPtr> result;
    DoSta(reg, result);
    EXPECT_EQ(result[0]->GetReg(0), reg);
    EXPECT_EQ(result[0]->opcode, panda::pandasm::Opcode::STA);
}

/**
 * @tc.name: codegen_test_003
 * @tc.desc: Verify the EmitJump function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CodegenTest, codegen_test_003, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "codegenTryCatch.abc";
    const char *test_method_name = "func1";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            EXPECT_NE(bb, nullptr);
            if (bb->IsTryBegin()) {
                status = true;
                Function *function = nullptr;
                BytecodeOptIrInterface *interface = nullptr;
                Program *prog = nullptr;
                BytecodeGen bc_gen(graph, function, interface, prog);
                bc_gen.EmitJump(bb->GetSuccessor(1));
                EXPECT_FALSE(bc_gen.GetResult().empty());
                EXPECT_EQ(bc_gen.GetResult().back()->GetId(0), "label_6");
                bc_gen.EmitJump(bb->GetSuccessor(0));
                EXPECT_EQ(bc_gen.GetResult().back()->GetId(0), "label_3");
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: codegen_test_004
 * @tc.desc: Verify the EmitJump function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CodegenTest, codegen_test_004, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "codegenTryCatch.abc";
    const char *test_method_name = "func2";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            EXPECT_NE(bb, nullptr);
            for (auto inst : bb->AllInsts()) {
                if (inst->GetOpcode() == Opcode::IfImm) {
                    status = true;
                    Function *function = nullptr;
                    BytecodeOptIrInterface *interface = nullptr;
                    Program *prog = nullptr;
                    BytecodeGen bc_gen(graph, function, interface, prog);
                    bc_gen.EmitJump(bb);
                    EXPECT_FALSE(bc_gen.GetResult().empty());
                    EXPECT_EQ(bc_gen.GetResult().back()->opcode, panda::pandasm::Opcode::JMP);
                }
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: codegen_test_005
 * @tc.desc: Verify the VisitConstant function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CodegenTest, codegen_test_005, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "codegenTryCatch.abc";
    const char *test_method_name = "func5";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
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
                Function *function = nullptr;
                BytecodeOptIrInterface *interface = nullptr;
                Program *prog = nullptr;

                BytecodeGen graph_visitor(graph, function, interface, prog);

                ConstantInst *const_inst = inst->CastToConstant();
                const_inst->SetType(DataType::Type::INT64);
                EXPECT_EQ(const_inst->GetType(), DataType::Type::INT64);
                Register reg = INVALID_REG;
                const_inst->SetDstReg(reg);
                EXPECT_EQ(const_inst->GetDstReg(), reg);
                BytecodeGen::VisitConstant(&graph_visitor, const_inst);
                EXPECT_FALSE(graph_visitor.GetResult().empty());
                EXPECT_EQ(graph_visitor.GetResult().back()->opcode, panda::pandasm::Opcode::STA);

                ConstantInst *const_inst1 = inst->CastToConstant();
                const_inst1->SetType(DataType::Type::FLOAT64);
                EXPECT_EQ(const_inst->GetType(), DataType::Type::FLOAT64);
                const_inst1->SetDstReg(reg);
                EXPECT_EQ(const_inst->GetDstReg(), reg);
                BytecodeGen::VisitConstant(&graph_visitor, const_inst1);
                EXPECT_FALSE(graph_visitor.GetResult().empty());
                EXPECT_EQ(graph_visitor.GetResult().back()->opcode, panda::pandasm::Opcode::STA);

                ConstantInst *const_inst2 = inst->CastToConstant();
                const_inst2->SetType(DataType::Type::INT32);
                EXPECT_EQ(const_inst->GetType(), DataType::Type::INT32);
                const_inst2->SetDstReg(reg);
                EXPECT_EQ(const_inst->GetDstReg(), reg);
                BytecodeGen::VisitConstant(&graph_visitor, const_inst2);
                EXPECT_FALSE(graph_visitor.GetResult().empty());
                EXPECT_EQ(graph_visitor.GetResult().back()->opcode, panda::pandasm::Opcode::STA);
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: codegen_test_006
 * @tc.desc: Verify the EncodeSta function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CodegenTest, codegen_test_006, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "codegenTryCatch.abc";
    const char *test_method_name = "func1";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            EXPECT_NE(bb, nullptr);
            if (bb->IsTryBegin()) {
                status = true;
                Register reg = 1;  // 1: It's a random number
                Function *function = nullptr;
                BytecodeOptIrInterface *interface = nullptr;
                Program *prog = nullptr;
                BytecodeGen bc_gen(graph, function, interface, prog);
                bc_gen.EncodeSta(reg, DataType::Type::ANY);
                EXPECT_FALSE(bc_gen.GetResult().empty());
                EXPECT_EQ(bc_gen.GetResult().back()->opcode, panda::pandasm::Opcode::STA);
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: codegen_test_007
 * @tc.desc: Verify the VisitLoadString function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CodegenTest, codegen_test_007, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "codegenTryCatch.abc";
    const char *test_method_name = "func6";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetVectorBlocks()) {
            EXPECT_NE(bb, nullptr);
            for (auto inst1 : bb->AllInsts()) {
                if (inst1->GetOpcode() != Opcode::LoadString) {
                    continue;
                }
                status = true;
                Function *function = nullptr;
                Program *prog = nullptr;
                AsmEmitter::PandaFileToPandaAsmMaps maps;
                uint32_t id = 2;  // 2: It's a random number
                maps.strings.emplace(id, "i32[]");
                BytecodeOptIrInterface interface(&maps, prog);

                BytecodeGen graph_visitor(graph, function, &interface, prog);
                auto inst = inst1->CastToLoadString();
                inst->SetTypeId(2);

                unsigned index = 5; // 5: It's a random number
                unsigned size = 6; // 6: It's a random number
                User user(true, index, size);
                inst->AddUser(&user);

                Register reg1 = ACC_REG_ID;
                inst->SetDstReg(reg1);
                BytecodeGen::VisitLoadString(&graph_visitor, inst);
                EXPECT_FALSE(graph_visitor.GetResult().empty());
                EXPECT_EQ(graph_visitor.GetResult().back()->opcode, panda::pandasm::Opcode::LDA_STR);

                Register reg2 = INVALID_REG;
                inst->SetDstReg(reg2);
                BytecodeGen::VisitLoadString(&graph_visitor, inst);
                EXPECT_FALSE(graph_visitor.GetResult().empty());
                EXPECT_EQ(graph_visitor.GetResult().back()->opcode, panda::pandasm::Opcode::STA);
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: codegen_test_008
 * @tc.desc: Verify the VisitDefault function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CodegenTest, codegen_test_008, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "codegenTryCatch.abc";
    const char *test_method_name = "func1";
    bool status = false;
        
    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }

        EXPECT_NE(graph, nullptr);
        for (auto bb : graph->GetBlocksRPO()) {
            EXPECT_NE(bb, nullptr);
            if (bb->IsTryBegin()) {
                status = true;
                Function *function = nullptr;
                BytecodeOptIrInterface *interface = nullptr;
                Program *prog = nullptr;
                BytecodeGen bc_gen(graph, function, interface, prog);
                bc_gen.VisitDefault(bb->GetFirstInst());
                EXPECT_FALSE(bc_gen.GetStatus());
            }
        }
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: codegen_test_009
 * @tc.desc: Verify the GetLiteralArrayByOffset function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CodegenTest, codegen_test_009, TestSize.Level1)
{
    AsmEmitter::PandaFileToPandaAsmMaps maps;
    uint32_t id = 2;  // 2: It's a random number
    maps.literalarrays.emplace(id, "i32[]");
    maps.strings.emplace(id, "i33[]");
    Program *prog = nullptr;
    BytecodeOptIrInterface interface(&maps, prog);

    uint32_t offset = 2;  // 2: It's a random number
    EXPECT_EQ(interface.GetLiteralArrayByOffset(offset), "i32[]");
}

/**
 * @tc.name: codegen_test_010
 * @tc.desc: Verify the GetTypeIdByOffset function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CodegenTest, codegen_test_010, TestSize.Level1)
{
    AsmEmitter::PandaFileToPandaAsmMaps maps;
    uint32_t id = 2;  // 2: It's a random number
    maps.classes.emplace(id, "i32[]");
    Program *prog = nullptr;
    BytecodeOptIrInterface interface(&maps, prog);

    uint32_t offset = 2;  // 2: It's a random number
    EXPECT_EQ(interface.GetTypeIdByOffset(offset), "i32[]");
}

/**
 * @tc.name: codegen_test_011
 * @tc.desc: Verify the GetFieldIdByOffset function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CodegenTest, codegen_test_011, TestSize.Level1)
{
    AsmEmitter::PandaFileToPandaAsmMaps maps;
    uint32_t id = 2;  // 2: It's a random number
    maps.fields.emplace(id, "i32[]");
    Program *prog = nullptr;
    BytecodeOptIrInterface interface(&maps, prog);

    uint32_t offset = 2;  // 2: It's a random number
    interface.GetFieldIdByOffset(offset);
    EXPECT_EQ(interface.GetFieldIdByOffset(offset), "i32[]");
    EXPECT_TRUE(interface.IsMapsSet());
}


/**
 * @tc.name: codegen_test_012
 * @tc.desc: Verify the GetMethodArgumentsCount function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CodegenTest, codegen_test_012, TestSize.Level1)
{
    auto source = std::string(R"(
        .function u1 foo() {
            sta v4
            lda v4
            return
        }
    )");
    std::unique_ptr<const panda_file::File> pfile = ParseAndEmit(source);
    BytecodeOptimizerRuntimeAdapter runtime_adapter(*pfile.get());

    RuntimeInterface::MethodPtr caller = nullptr;
    auto class_id = pfile->GetClassId(utf::CStringAsMutf8("L_GLOBAL;"));
    EXPECT_TRUE(class_id.IsValid());
    panda_file::ClassDataAccessor cda(*pfile.get(), class_id);

    cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
        BytecodeOptimizerRuntimeAdapter::MethodId id = mda.GetMethodId().GetOffset();
        EXPECT_EQ(runtime_adapter.GetMethodArgumentsCount(caller, id), 0);
    });
}

/**
 * @tc.name: codegen_test_013
 * @tc.desc: Verify the GetMethodFullName function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CodegenTest, codegen_test_013, TestSize.Level1)
{
    auto source = std::string(R"(
        .function u1 foo() {
            sta v4
            lda v4
            return
        }
    )");
    std::unique_ptr<const panda_file::File> pfile = ParseAndEmit(source);
    BytecodeOptimizerRuntimeAdapter runtime_adapter(*pfile.get());

    auto class_id = pfile->GetClassId(utf::CStringAsMutf8("L_GLOBAL;"));
    EXPECT_TRUE(class_id.IsValid());
    panda_file::ClassDataAccessor cda(*pfile.get(), class_id);

    cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
        BytecodeOptimizerRuntimeAdapter::MethodId id = mda.GetMethodId().GetOffset();
        RuntimeInterface::MethodPtr method;
        method=(void*)(long)id;
        EXPECT_EQ(runtime_adapter.GetMethodFullName(method, false), "L_GLOBAL;::foo");
    });
}

/**
 * @tc.name: codegen_test_014
 * @tc.desc: Verify the GetBlocksToVisit function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CodegenTest, codegen_test_014, TestSize.Level1)
{
    std::string pfile = GRAPH_TEST_ABC_DIR "codegenTryCatch.abc";
    const char *test_method_name = "func1";
    bool status = false;

    graph_test_.TestBuildGraphFromFile(pfile, [test_method_name, &status](Graph* graph, std::string &method_name) {
        if (test_method_name != method_name) {
            return;
        }
        status = true;
        EXPECT_NE(graph, nullptr);

        Function *function = nullptr;
        BytecodeOptIrInterface *interface = nullptr;
        Program *prog = nullptr;
        size_t size = 10;  // 10: It's block size
        BytecodeGen bc_gen(graph, function, interface, prog);
        EXPECT_EQ(bc_gen.GetBlocksToVisit().size(), size);
    });
    EXPECT_TRUE(status);
}

/**
 * @tc.name: codegen_test_015
 * @tc.desc: Verify the RunImpl function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(CodegenTest, codegen_test_015, TestSize.Level1)
{
    const auto source = R"(
        .function any func_main_0(any a0, any a1, any a2) <static> {
            mov v0, a0
            mov v1, a1
            mov v2, a2
        try_begin_label_0:
            ldai 0x1
            trystglobalbyname 0x0, "a"
        try_end_label_0:
            jmp handler_end_label_0_0
        handler_begin_label_0_0:
            sta v4
            ldai 0x2
            trystglobalbyname 0x1, "a"
        handler_end_label_0_0:
            ldundefined
            returnundefined

        .catchall try_begin_label_0, try_end_label_0, handler_begin_label_0_0, handler_end_label_0_0
        }
    )";

    panda::pandasm::Parser parser;
    auto res = parser.Parse(source);
    auto &prog = res.Value();

    auto &function = prog.function_table.at("func_main_0:(any,any,any)");

    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps {};
    auto ir_interface = panda::bytecodeopt::BytecodeOptIrInterface(&maps, &prog);
    bool status = false;

    graph_test_.TestBuildGraphFromFunc(prog, "func_main_0:(any,any,any)", maps, ir_interface, [&function,
        &ir_interface, &prog, &status](Graph* graph) {

        EXPECT_NE(graph, nullptr);

        for (auto bb : graph->GetBlocksRPO()) {
            EXPECT_NE(bb, nullptr);
            if (bb->IsTryBegin()) {
                status = true;
                graph->AppendTryBeginBlock(bb);
            }
        }
        EXPECT_TRUE(graph->RunPass<compiler::Cleanup>());
        EXPECT_FALSE(graph->RunPass<compiler::Cleanup>());
        EXPECT_FALSE(graph->RunPass<panda::compiler::ValNum>());
        EXPECT_TRUE(graph->RunPass<panda::compiler::Lowering>());
        EXPECT_TRUE(graph->RunPass<panda::compiler::MoveConstants>());
        EXPECT_FALSE(graph->RunPass<compiler::Cleanup>());
        EXPECT_TRUE(graph->RunPass<RegAccAlloc>());
        EXPECT_FALSE(graph->RunPass<compiler::Cleanup>());
        EXPECT_TRUE(RegAlloc(graph));
        EXPECT_FALSE(graph->RunPass<compiler::Cleanup>());
        EXPECT_TRUE(graph->RunPass<RegEncoder>());

        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &ir_interface, &prog));
    });
    EXPECT_TRUE(status);
}
}  // namespace panda::bytecodeopt
