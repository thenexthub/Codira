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

#include "assembler/assembly-emitter.h"
#include "assembler/assembly-function.h"
#include "assembler/assembly-literals.h"
#include "assembler/assembly-parser.h"
#include "assembler/assembly-program.h"
#include "common.h"
#include "codegen.h"
#include "compiler/optimizer/optimizations/peepholes.h"
#include "compiler/optimizer/optimizations/cleanup.h"
#include "compiler/optimizer/optimizations/lowering.h"
#include "compiler/optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "optimize_bytecode.h"
#include "reg_acc_alloc.h"

namespace ark::bytecodeopt::test {

// NOLINTBEGIN(readability-magic-numbers)

TEST(ToolTest, BytecodeOptIrInterfaceCommon)
{
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.methods.insert({0U, std::string("method")});
    maps.fields.insert({0U, std::string("field")});
    maps.classes.insert({0U, std::string("class")});
    maps.strings.insert({0U, std::string("string")});
    maps.literalarrays.insert({0U, std::string("0")});

    pandasm::Program prog;
    prog.literalarrayTable.emplace("0", pandasm::LiteralArray());

    BytecodeOptIrInterface interface(&maps, &prog);

    EXPECT_EQ(interface.GetMethodIdByOffset(0U), std::string("method"));
    EXPECT_EQ(interface.GetStringIdByOffset(0U), std::string("string"));
    EXPECT_EQ(interface.GetLiteralArrayIdByOffset(0U), "0");
    EXPECT_EQ(interface.GetTypeIdByOffset(0U), std::string("class"));
    EXPECT_EQ(interface.GetFieldIdByOffset(0U), std::string("field"));
    interface.StoreLiteralArray("1", pandasm::LiteralArray());
    EXPECT_EQ(interface.GetLiteralArrayTableSize(), 2U);
}

TEST(ToolTest, BytecodeOptIrInterfacePcLineNumber)
{
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;

    pandasm::Program prog;
    prog.literalarrayTable.emplace("0", pandasm::LiteralArray());

    BytecodeOptIrInterface interface(&maps, &prog);

    auto map = interface.GetPcInsMap();
    EXPECT_EQ(interface.GetLineNumberByPc(0U), 0U);

    auto ins = pandasm::Ins();
    ins.insDebug.SetLineNumber(1);
    ASSERT_NE(map, nullptr);
    map->insert({0U, &ins});
    EXPECT_EQ(interface.GetLineNumberByPc(0U), 1U);
    interface.ClearPcInsMap();
    EXPECT_NE(interface.GetLineNumberByPc(0U), 1U);
}

TEST(ToolTest, BytecodeOptIrInterfaceLiteralArray)
{
    BytecodeOptIrInterface interface(nullptr, nullptr);
#ifdef NDEBUG
    EXPECT_EQ(interface.GetLiteralArrayIdByOffset(0U), std::nullopt);
#endif
}

TEST_F(CommonTest, CodeGenBinaryImms)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::AddI).s32().Inputs(0U).Imm(2U);
            INST(2U, Opcode::SubI).s32().Inputs(0U).Imm(2U);
            INST(3U, Opcode::MulI).s32().Inputs(0U).Imm(2U);
            INST(4U, Opcode::DivI).s32().Inputs(0U).Imm(2U);
            INST(5U, Opcode::ModI).s32().Inputs(0U).Imm(2U);
            INST(6U, Opcode::AndI).u32().Inputs(0U).Imm(2U);
            INST(7U, Opcode::OrI).s32().Inputs(0U).Imm(2U);
            INST(8U, Opcode::XorI).s32().Inputs(0U).Imm(2U);
            INST(10U, Opcode::Return).b().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST_F(CommonTest, CodeGenIfImm)
{
    auto graph = CreateEmptyGraph();

    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::Compare).b().CC(compiler::ConditionCode::CC_EQ).Inputs(0U, 1U);
            INST(3U, Opcode::IfImm)
                .SrcType(compiler::DataType::BOOL)
                .CC(compiler::ConditionCode::CC_NE)
                .Imm(0U)
                .Inputs(2U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(10U, Opcode::Return).s32().Inputs(0U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Return).s32().Inputs(0U);
        }
    }

#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    graph->RunPass<compiler::Lowering>();
    graph->RunPass<compiler::Cleanup>();

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        PARAMETER(0U, 0U).u32();
        PARAMETER(1U, 1U).u32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(12U, Opcode::If).CC(compiler::ConditionCode::CC_EQ).SrcType(compiler::DataType::UINT32).Inputs(1U, 0U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(10U, Opcode::Return).s32().Inputs(0U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(11U, Opcode::Return).s32().Inputs(0U);
        }
    }

    EXPECT_TRUE(GraphComparator().Compare(graph, expected));

    EXPECT_TRUE(expected->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(expected->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenIf)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u32();
            PARAMETER(1U, 1U).u32();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::If).CC(cond).SrcType(compiler::DataType::UINT32).Inputs(0U, 1U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).s32().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).s32().Inputs(0U);
            }
        }

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfDynamic)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u32();
            PARAMETER(1U, 1U).u32();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::If).CC(cond).SrcType(compiler::DataType::UINT32).Inputs(0U, 1U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).s32().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).s32().Inputs(0U);
            }
        }

        graph->SetDynamicMethod();

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfINT64)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).s64();
            PARAMETER(1U, 1U).s64();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::If).CC(cond).SrcType(compiler::DataType::INT64).Inputs(0U, 1U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).s64().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).s64().Inputs(0U);
            }
        }

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfINT64Dynamic)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).s64();
            PARAMETER(1U, 1U).s64();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::If).CC(cond).SrcType(compiler::DataType::INT64).Inputs(0U, 1U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).s64().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).s64().Inputs(0U);
            }
        }

        graph->SetDynamicMethod();

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfUINT64)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();
            PARAMETER(1U, 1U).u64();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::If).CC(cond).SrcType(compiler::DataType::UINT64).Inputs(0U, 1U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).u64().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).u64().Inputs(0U);
            }
        }

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfUINT64Dynamic)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();
            PARAMETER(1U, 1U).u64();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::If).CC(cond).SrcType(compiler::DataType::UINT64).Inputs(0U, 1U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).u64().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).u64().Inputs(0U);
            }
        }

        graph->SetDynamicMethod();

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfREFERENCE)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).ref();
            PARAMETER(1U, 1U).ref();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(2U, Opcode::If).SrcType(compiler::DataType::REFERENCE).CC(cond).Inputs(0U, 1U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(3U, Opcode::ReturnVoid).v0id();
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(4U, Opcode::ReturnVoid).v0id();
            }
        }

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmZero)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).s32();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::IfImm).CC(cond).Imm(0U).SrcType(compiler::DataType::INT32).Inputs(0U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).s32().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).s32().Inputs(0U);
            }
        }

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmZeroDynamic)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).s32();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::IfImm).CC(cond).Imm(0U).SrcType(compiler::DataType::INT32).Inputs(0U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).s32().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).s32().Inputs(0U);
            }
        }

        graph->SetDynamicMethod();
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmZeroINT64)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).s64();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::IfImm).CC(cond).Imm(0U).SrcType(compiler::DataType::INT64).Inputs(0U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).s64().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).s64().Inputs(0U);
            }
        }

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmZeroINT64Dynamic)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).s64();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::IfImm).CC(cond).Imm(0U).SrcType(compiler::DataType::INT64).Inputs(0U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).s64().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).s64().Inputs(0U);
            }
        }

        graph->SetDynamicMethod();
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmZeroUINT64)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::IfImm).CC(cond).Imm(0U).SrcType(compiler::DataType::UINT64).Inputs(0U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).u64().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).u64().Inputs(0U);
            }
        }

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmZeroUINT64Dynamic)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::IfImm).CC(cond).Imm(0U).SrcType(compiler::DataType::UINT64).Inputs(0U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).u64().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).u64().Inputs(0U);
            }
        }

        graph->SetDynamicMethod();
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmZeroREFERENCE)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).ref();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::IfImm).CC(cond).Imm(0U).SrcType(compiler::DataType::REFERENCE).Inputs(0U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).ref().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).ref().Inputs(0U);
            }
        }

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmZeroREFERENCEDynamic)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).ref();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::IfImm).CC(cond).Imm(0U).SrcType(compiler::DataType::REFERENCE).Inputs(0U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).ref().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).ref().Inputs(0U);
            }
        }

        graph->SetDynamicMethod();
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmNonZero)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).u64();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::IfImm).CC(cond).Imm(1U).SrcType(compiler::DataType::UINT64).Inputs(0U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).u64().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).u64().Inputs(0U);
            }
        }

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmNonZero32)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).s32();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::IfImm).CC(cond).Imm(1U).SrcType(compiler::DataType::INT32).Inputs(0U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).s32().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).s32().Inputs(0U);
            }
        }

#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenIfImmNonZero32Dynamic)
{
    using compiler::ConditionCode;
    std::vector<compiler::ConditionCode> conds {ConditionCode::CC_EQ, ConditionCode::CC_NE, ConditionCode::CC_LT,
                                                ConditionCode::CC_LE, ConditionCode::CC_GT, ConditionCode::CC_GE};

    for (auto cond : conds) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).s32();

            BASIC_BLOCK(2U, 3U, 4U)
            {
                INST(12U, Opcode::IfImm).CC(cond).Imm(1U).SrcType(compiler::DataType::INT32).Inputs(0U);
            }
            BASIC_BLOCK(3U, -1L)
            {
                INST(10U, Opcode::Return).s32().Inputs(0U);
            }
            BASIC_BLOCK(4U, -1L)
            {
                INST(11U, Opcode::Return).s32().Inputs(0U);
            }
        }

        graph->SetDynamicMethod();
#ifndef NDEBUG
        graph->SetLowLevelInstructionsEnabled();
#endif

        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
    }
}

TEST_F(CommonTest, CodegenConstantINT64Dynamic)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).s64().Inputs(0U);
        }
    }

    graph->SetDynamicMethod();

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenConstantFLOAT64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).f64().Inputs(0U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenConstantFLOAT64Dynamic)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).f64().Inputs(0U);
        }
    }

    graph->SetDynamicMethod();

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenConstantINT32Dynamic)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).s32().Inputs(0U);
        }
    }

    graph->SetDynamicMethod();

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt2F64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).f64().SrcType(compiler::DataType::INT32).Inputs(0U);
            INST(2U, Opcode::Return).f64().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt2Uint)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).u32().SrcType(compiler::DataType::INT32).Inputs(0U);
            INST(2U, Opcode::Return).u32().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt2Int16)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).s16().SrcType(compiler::DataType::INT32).Inputs(0U);
            INST(2U, Opcode::Return).s16().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt2Uint16)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).u16().SrcType(compiler::DataType::INT32).Inputs(0U);
            INST(2U, Opcode::Return).u16().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt2Int8)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).s8().SrcType(compiler::DataType::INT32).Inputs(0U);
            INST(2U, Opcode::Return).s8().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt2Uint8)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).u8().SrcType(compiler::DataType::INT32).Inputs(0U);
            INST(2U, Opcode::Return).u8().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt64ToInt32)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).s32().SrcType(compiler::DataType::INT64).Inputs(0U);
            INST(2U, Opcode::Return).s32().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt64ToUint32)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).u32().SrcType(compiler::DataType::INT64).Inputs(0U);
            INST(2U, Opcode::Return).u32().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt64ToF64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).f64().SrcType(compiler::DataType::INT64).Inputs(0U);
            INST(2U, Opcode::Return).f64().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt64ToInt16)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).s16().SrcType(compiler::DataType::INT64).Inputs(0U);
            INST(2U, Opcode::Return).s16().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt64ToUint16)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).u16().SrcType(compiler::DataType::INT64).Inputs(0U);
            INST(2U, Opcode::Return).u16().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt64ToInt8)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).s8().SrcType(compiler::DataType::INT64).Inputs(0U);
            INST(2U, Opcode::Return).s8().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastInt64ToUint8)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).u8().SrcType(compiler::DataType::INT64).Inputs(0U);
            INST(2U, Opcode::Return).u8().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastFloat64ToInt64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).s64().SrcType(compiler::DataType::FLOAT64).Inputs(0U);
            INST(2U, Opcode::Return).s64().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastFloat64ToInt32)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).s32().SrcType(compiler::DataType::FLOAT64).Inputs(0U);
            INST(2U, Opcode::Return).s32().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(CommonTest, CodegenCastFloat64ToUint32)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Cast).u32().SrcType(compiler::DataType::FLOAT64).Inputs(0U);
            INST(2U, Opcode::Return).u32().Inputs(1U);
        }
    }

    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, nullptr));
}

TEST_F(AsmTest, CodegenLoadString)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).NoVregs();

            INST(2U, Opcode::LoadString).ref().TypeId(235U).Inputs(1U);

            INST(3U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::NullCheck).ref().Inputs(2U, 3U);
            INST(5U, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 4U}, {NO_TYPE, 3U}});
            INST(6U, Opcode::Return).s32().Inputs(5U);
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));

    graph->SetDynamicMethod();
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenStoreObject64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(7U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            // NOLINTNEXTLINE(google-build-using-namespace)
            using namespace compiler::DataType;
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 5U}, {NO_TYPE, 4U}});
            INST(8U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::NullCheck).ref().Inputs(3U, 8U);

            INST(10U, Opcode::StoreObject).s64().Inputs(9U, 7U);

            INST(11U, Opcode::ReturnVoid).v0id();
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenStoreObjectReference)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(7U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            // NOLINTNEXTLINE(google-build-using-namespace)
            using namespace compiler::DataType;
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 5U}, {NO_TYPE, 4U}});
            INST(8U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::NullCheck).ref().Inputs(3U, 8U);

            INST(10U, Opcode::StoreObject).ref().Inputs(9U, 5U);

            INST(11U, Opcode::ReturnVoid).v0id();
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenLoadObjectInt)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);
            INST(3U, Opcode::LoadObject).s32().Inputs(2U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenLoadObjectInt64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);
            INST(3U, Opcode::LoadObject).s64().Inputs(2U);
            INST(4U, Opcode::Return).s64().Inputs(3U);
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenLoadObjectReference)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::NullCheck).ref().Inputs(0U, 1U);
            INST(3U, Opcode::LoadObject).ref().Inputs(2U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenInitObject)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            // NOLINTNEXTLINE(google-build-using-namespace)
            using namespace compiler::DataType;

            INST(0U, Opcode::SaveState).NoVregs();
            INST(1U, Opcode::LoadAndInitClass).ref().Inputs(0U).TypeId(68U);
            INST(3U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::InitObject).ref().Inputs({{REFERENCE, 1U}, {NO_TYPE, 3U}});
            INST(7U, Opcode::ReturnVoid).v0id();
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenEncodeSta64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            // NOLINTNEXTLINE(google-build-using-namespace)
            using namespace compiler::DataType;

            INST(1U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::CallVirtual).f64().Inputs({{REFERENCE, 0U}, {NO_TYPE, 1U}});
            INST(6U, Opcode::LoadObject).s64().Inputs(0U);
            INST(7U, Opcode::Cast).f64().SrcType(INT64).Inputs(6U);
            CONSTANT(15U, 3.6e+06).f64();
            INST(9U, Opcode::Div).f64().Inputs(7U, 15U);
            INST(10U, Opcode::Add).f64().Inputs(9U, 3U);
            CONSTANT(16U, 24U).f64();
            INST(12U, Opcode::SaveState).NoVregs();
            INST(13U, Opcode::CallStatic).f64().Inputs({{FLOAT64, 10U}, {FLOAT64, 16U}, {NO_TYPE, 12U}});
            INST(14U, Opcode::ReturnVoid).v0id();
        }
    }

    RuntimeInterfaceMock runtime(1U);
    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<RegEncoder>());
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenBlockNeedsJump)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(4U, 4U).ref();
        PARAMETER(7U, 7U).s32();
        PARAMETER(13U, 13U).ref();
        PARAMETER(19U, 19U).ref();
        PARAMETER(35U, 35U).ref();
        PARAMETER(43U, 43U).s32();

        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(10U, Opcode::IfImm).SrcType(INT32).CC(compiler::CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, 5U, 7U)
        {
            INST(39U, Opcode::If).SrcType(REFERENCE).CC(compiler::CC_EQ).Inputs(13U, 4U);
        }
        BASIC_BLOCK(3U, 7U, 8U)
        {
            INST(40U, Opcode::If).SrcType(REFERENCE).CC(compiler::CC_NE).Inputs(19U, 4U);
        }
        BASIC_BLOCK(8U, 5U, 7U)
        {
            INST(41U, Opcode::If).SrcType(INT32).CC(compiler::CC_EQ).Inputs(7U, 43U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(28U, Opcode::ReturnVoid).v0id();
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(37U, Opcode::SaveState).NoVregs();
            INST(38U, Opcode::Throw).Inputs(35U, 37U);
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(CommonTest, CodegenNotProduceUnsupportedCast)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(1U, 1U).i16();
        CONSTANT(3U, 0xFFFFU);

        BASIC_BLOCK(2U, -1L)
        {
            INST(11U, Opcode::And).u16().Inputs(1U, 3U);
            INST(17U, Opcode::Cast).i16().SrcType(compiler::DataType::INT32).Inputs(11U);

            INST(24U, Opcode::ReturnVoid).v0id();
        }
    }
    IrInterfaceTest interface(nullptr);
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_FALSE(graph->RunPass<compiler::Peepholes>());
}

TEST_F(CommonTest, CodegenFloat32Constant)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0.0).f32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).f32().Inputs(0U);
        }
    }

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(AsmTest, CatchPhi)
{
    auto source = R"(
    .record E1 {}
    .record A {}

    .function i64 main(i64 a0) {
    try_begin:
        movi.64 v0, 100
        lda.64 v0
        div2.64 a0
        sta.64 v0
        div2.64 a0
    try_end:
        jmp exit

    catch_block1_begin:
        lda.64 v0
        return

    catch_block2_begin:
        lda.64 v0
        return

    exit:
        return

    .catch E1, try_begin, try_end, catch_block1_begin
    .catchall try_begin, try_end, catch_block2_begin
    }
    )";

    ark::pandasm::Parser parser;
    auto res = parser.Parse(source);
    ASSERT_TRUE(parser.ShowError().err == pandasm::Error::ErrorType::ERR_NONE);
    auto &prog = res.Value();
    ParseToGraph(&prog, "main");

    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(GetGraph()->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(GetGraph()->RunPass<BytecodeGen>(&function, &interface));
}

TEST_F(AsmTest, Float32Regression1)
{
    auto source = R"(
    .function f32 foo() <static> {
        fldai 0x0
        return
    }

    .function f32 bar() <static> {
        call.short foo
        sta v0
        fldai 0x3f000000
        fadd2 v0
        return
    }
    )";
    ark::pandasm::Parser parser;
    auto res = parser.Parse(source);
    EXPECT_TRUE(res);
    auto &prog = res.Value();
    ParseToGraph(&prog, "bar");
    IrInterfaceTest interface(nullptr);
    EXPECT_TRUE(GetGraph()->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(GetGraph()->RunPass<BytecodeGen>(&function, &interface));
    EXPECT_TRUE(std::none_of(function.ins.begin(), function.ins.end(),
                             [](const auto &inst) { return inst.opcode == pandasm::Opcode::STA_64; }));
    EXPECT_TRUE(std::any_of(function.ins.begin(), function.ins.end(),
                            [](const auto &inst) { return inst.opcode == pandasm::Opcode::STA; }));
}

TEST_F(AsmTest, Float32Regression2)
{
    auto source = R"(
    .record Test {
    }

    .function f32 foo(Test a0, f32 a1) {
        fmovi v0, 0x0
        lda a1
        fcmpg v0
        jgez jump_label_0
        mov a1, v0
    jump_label_0:
        lda a1
        return
    }
    )";
    ark::pandasm::Parser parser;
    auto res = parser.Parse(source);
    EXPECT_TRUE(res);
    auto &prog = res.Value();
    ParseToGraph(&prog, "foo");
    IrInterfaceTest interface(nullptr);
    auto *graph = GetGraph();
#ifndef NDEBUG
    graph->SetLowLevelInstructionsEnabled();
#endif
    graph->RunPass<compiler::Lowering>();
    graph->RunPass<compiler::Cleanup>();
    EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
    auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
    EXPECT_TRUE(GetGraph()->RunPass<BytecodeGen>(&function, &interface));
    EXPECT_TRUE(std::none_of(function.ins.begin(), function.ins.end(),
                             [](const auto &inst) { return inst.opcode == pandasm::Opcode::MOV_64; }));
}

TEST_F(CommonTest, CodegenLdai)
{
    using compiler::DataType::Type;
    std::vector<Type> types {Type::INT32, Type::INT64, Type::FLOAT32, Type::FLOAT64};
    std::vector<pandasm::Opcode> opcodes {pandasm::Opcode::LDAI, pandasm::Opcode::LDAI_64, pandasm::Opcode::FLDAI,
                                          pandasm::Opcode::FLDAI_64};

    for (size_t i = 0; i < types.size(); i++) {
        auto graph = CreateEmptyGraph();
        GRAPH(graph)
        {
            PARAMETER(0U, 0U).s32();
            BASIC_BLOCK(2U, -1L)
            {
                CONSTANT(1U, 159U).s32();
                INST(2U, Opcode::Add).s32().Inputs(0U, 1U);
                INST(3U, Opcode::Return).s32().Inputs(2U);
            }
        }
        INS(0U).SetType(types[i]);
        INS(1U).SetType(types[i]);
        INS(2U).SetType(types[i]);
        INS(3U).SetType(types[i]);
        graph->RunPass<bytecodeopt::RegAccAlloc>();
        EXPECT_TRUE(graph->RunPass<compiler::RegAllocLinearScan>(compiler::EmptyRegMask()));
        auto function = pandasm::Function(std::string(), ark::panda_file::SourceLang::PANDA_ASSEMBLY);
        IrInterfaceTest interface(nullptr);
        EXPECT_TRUE(graph->RunPass<BytecodeGen>(&function, &interface));
        EXPECT_TRUE(FuncHasInst(&function, opcodes[i]));
    }
}

TEST(TotalTest, OptimizeBytecode)
{
    auto source = R"(
    .function i8 main() {
        ldai 1
        return
    }
    )";
    ark::pandasm::Parser parser;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    const char fileName[] = "opt_bc.bin";
    auto res = parser.Parse(source, fileName);
    ASSERT_TRUE(parser.ShowError().err == pandasm::Error::ErrorType::ERR_NONE);
    auto &prog = res.Value();
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    pandasm::AsmEmitter::Emit(fileName, prog, nullptr, &maps);
    EXPECT_TRUE(OptimizeBytecode(&prog, &maps, fileName));
    EXPECT_TRUE(OptimizeBytecode(&prog, &maps, fileName, true));

    g_options.SetOptLevel(0U);
    EXPECT_FALSE(OptimizeBytecode(&prog, &maps, fileName));
    g_options.SetOptLevel(1U);
    EXPECT_TRUE(OptimizeBytecode(&prog, &maps, fileName));
#ifndef NDEBUG
    g_options.SetOptLevel(-1L);
    EXPECT_DEATH_IF_SUPPORTED(OptimizeBytecode(&prog, &maps, fileName), "");
#endif
    g_options.SetOptLevel(2U);

    g_options.SetMethodRegex(std::string());
    EXPECT_FALSE(OptimizeBytecode(&prog, &maps, fileName));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::bytecodeopt::test
