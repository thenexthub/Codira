/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "common.h"
#include "const_array_resolver.h"

namespace ark::bytecodeopt::test {

// NOLINTBEGIN(readability-magic-numbers)

TEST_F(CommonTest, ConstArrayResolverInt64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(1U, 2U).s64();
        CONSTANT(2U, 0U).s64();
        CONSTANT(3U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(20U).TypeId(68U);
            INST(5U, Opcode::NewArray).ref().Inputs(4U, 1U, 20U);
            INST(11U, Opcode::StoreArray).u64().Inputs(5U, 2U, 2U);
            INST(13U, Opcode::StoreArray).u64().Inputs(5U, 3U, 2U);
            INST(10U, Opcode::Return).ref().Inputs(5U);
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0U, "i64[]");
    IrInterfaceTest interface(&program, &maps);
    g_options.SetConstArrayResolver(true);
    EXPECT_TRUE(graph->RunPass<ConstArrayResolver>(&interface));
    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        CONSTANT(1U, 2U).s64();
        CONSTANT(2U, 0U).s64();
        CONSTANT(3U, 1U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(20U).TypeId(68U);
            INST(22U, Opcode::SaveState).NoVregs();
            INST(21U, Opcode::LoadConstArray).ref().Inputs(22U);
            INST(10U, Opcode::Return).ref().Inputs(21U);
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(CommonTest, ConstArrayResolverInt32)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(1U, 2U).s32();
        CONSTANT(2U, 0U).s32();
        CONSTANT(3U, 1U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(20U).TypeId(68U);
            INST(5U, Opcode::NewArray).ref().Inputs(4U, 1U, 20U);
            INST(11U, Opcode::StoreArray).u32().Inputs(5U, 2U, 2U);
            INST(13U, Opcode::StoreArray).u32().Inputs(5U, 3U, 2U);
            INST(10U, Opcode::Return).ref().Inputs(5U);
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0U, "i32[]");
    IrInterfaceTest interface(&program, &maps);
    g_options.SetConstArrayResolver(true);
    EXPECT_TRUE(graph->RunPass<ConstArrayResolver>(&interface));
}

TEST_F(CommonTest, ConstArrayResolverOrderInt32)
{
    auto graph = CreateEmptyGraph();
    // in the zero and first position type and length of the array are put
    size_t offset = 2;
    GRAPH(graph)
    {
        CONSTANT(0U, 3U).s32();
        CONSTANT(1U, 0U).s32();
        CONSTANT(2U, 1U).s32();
        CONSTANT(3U, 2U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(20U).TypeId(68U);
            INST(5U, Opcode::NewArray).ref().Inputs(4U, 0U, 20U);
            INST(15U, Opcode::StoreArray).u32().Inputs(5U, 2U, 1U);
            INST(11U, Opcode::StoreArray).u32().Inputs(5U, 1U, 2U);
            INST(13U, Opcode::StoreArray).u32().Inputs(5U, 3U, 3U);
            INST(10U, Opcode::Return).ref().Inputs(5U);
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0U, "i32[]");
    IrInterfaceTest interface(&program, &maps);
    g_options.SetConstArrayResolver(true);
    EXPECT_TRUE(graph->RunPass<ConstArrayResolver>(&interface));

    auto litArr = program.literalarrayTable["0"];
    EXPECT_TRUE(std::get<uint32_t>(litArr.literals[0U + offset].value) == 1U);
    EXPECT_TRUE(std::get<uint32_t>(litArr.literals[1U + offset].value) == 0U);
    EXPECT_TRUE(std::get<uint32_t>(litArr.literals[2U + offset].value) == 2U);
}

TEST_F(CommonTest, ConstArrayResolverFloat32)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 3U).s32();
        CONSTANT(4U, 0U).s32();
        CONSTANT(6U, 100U).f64();
        CONSTANT(10U, 1U).s32();
        CONSTANT(13U, 200U).f64();
        CONSTANT(16U, 2U).s32();
        CONSTANT(20U, 300U).f64();

        BASIC_BLOCK(2U, -1L)
        {
            // NOLINTNEXTLINE(google-build-using-namespace)
            using namespace compiler::DataType;

            INST(3U, Opcode::SaveState).NoVregs();
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).TypeId(68U);
            INST(5U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
            INST(12U, Opcode::Cast).f32().SrcType(FLOAT64).Inputs(6U);
            INST(11U, Opcode::StoreArray).f32().Inputs(5U, 4U, 12U);
            INST(19U, Opcode::Cast).f32().SrcType(FLOAT64).Inputs(13U);
            INST(18U, Opcode::StoreArray).f32().Inputs(5U, 10U, 19U);
            INST(26U, Opcode::Cast).f32().SrcType(FLOAT64).Inputs(20U);
            INST(25U, Opcode::StoreArray).f32().Inputs(5U, 16U, 26U);
            INST(27U, Opcode::Return).ref().Inputs(5U);
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0U, "f32[]");
    IrInterfaceTest interface(&program, &maps);
    g_options.SetConstArrayResolver(true);
    EXPECT_TRUE(graph->RunPass<ConstArrayResolver>(&interface));
}

TEST_F(CommonTest, ConstArrayResolverFloat64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s32();
        CONSTANT(1U, 1U).s32();
        CONSTANT(2U, 2U).s32();
        CONSTANT(3U, 100.0_D).f64();
        CONSTANT(4U, 200.0_D).f64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(20U).TypeId(68U);
            INST(5U, Opcode::NewArray).ref().Inputs(44U, 2U, 20U);
            INST(11U, Opcode::StoreArray).f64().Inputs(5U, 0U, 3U);
            INST(13U, Opcode::StoreArray).f64().Inputs(5U, 1U, 4U);
            INST(10U, Opcode::Return).ref().Inputs(5U);
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0U, "f64[]");
    IrInterfaceTest interface(&program, &maps);
    g_options.SetConstArrayResolver(true);
    EXPECT_TRUE(graph->RunPass<ConstArrayResolver>(&interface));
}

TEST_F(CommonTest, ConstArrayResolverByteAccess)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s32();
        CONSTANT(1U, 1U).s32();
        CONSTANT(2U, 2U).s32();
        CONSTANT(3U, 3U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).NoVregs();
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(4U).TypeId(68U);
            INST(5U, Opcode::NewArray).ref().Inputs(44U, 3U, 4U);
            INST(6U, Opcode::StoreArray).s8().Inputs(5U, 0U, 0U);
            INST(7U, Opcode::StoreArray).s8().Inputs(5U, 1U, 1U);
            INST(8U, Opcode::StoreArray).s8().Inputs(5U, 2U, 2U);
            INST(9U, Opcode::Return).ref().Inputs(5U);
            ;
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0U, "i8[]");
    IrInterfaceTest interface(&program, &maps);
    g_options.SetConstArrayResolver(true);
    EXPECT_TRUE(graph->RunPass<ConstArrayResolver>(&interface));

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        CONSTANT(0U, 0U).s32();
        CONSTANT(1U, 1U).s32();
        CONSTANT(2U, 2U).s32();
        CONSTANT(3U, 3U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).NoVregs();
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(4U).TypeId(68U);
            INST(11U, Opcode::SaveState).NoVregs();
            INST(10U, Opcode::LoadConstArray).ref().Inputs(11U);
            INST(9U, Opcode::Return).ref().Inputs(10U);
        }
    }

    EXPECT_TRUE(GraphComparator().Compare(graph, expected));
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) solid logic
TEST_F(CommonTest, ConstArrayResolverStringAccess)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U).s32();
        CONSTANT(1U, 1U).s32();
        CONSTANT(2U, 2U).s32();
        CONSTANT(3U, 3U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).NoVregs();
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(4U).TypeId(68U);
            INST(5U, Opcode::NewArray).ref().Inputs(44U, 3U, 4U);

            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::LoadString).ref().Inputs(6U);
            INST(8U, Opcode::StoreArray).ref().Inputs(5U, 0U, 7U);

            INST(9U, Opcode::SaveState).NoVregs();
            INST(10U, Opcode::LoadString).ref().Inputs(9U);
            INST(11U, Opcode::StoreArray).ref().Inputs(5U, 1U, 10U);

            INST(12U, Opcode::SaveState).NoVregs();
            INST(13U, Opcode::LoadString).ref().Inputs(12U);
            INST(24U, Opcode::StoreArray).ref().Inputs(5U, 2U, 13U);

            INST(15U, Opcode::Return).ref().Inputs(5U);
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0U, "panda/String[]");
    IrInterfaceTest interface(&program, &maps);
    g_options.SetConstArrayResolver(true);
    EXPECT_TRUE(graph->RunPass<ConstArrayResolver>(&interface));

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        CONSTANT(0U, 0U).s32();
        CONSTANT(1U, 1U).s32();
        CONSTANT(2U, 2U).s32();
        CONSTANT(3U, 3U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).NoVregs();
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(4U).TypeId(68U);
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::LoadConstArray).ref().Inputs(5U);

            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::LoadString).ref().Inputs(7U);
            INST(9U, Opcode::SaveState).NoVregs();
            INST(10U, Opcode::LoadString).ref().Inputs(9U);
            INST(11U, Opcode::SaveState).NoVregs();
            INST(12U, Opcode::LoadString).ref().Inputs(11U);

            INST(13U, Opcode::Return).ref().Inputs(6U);
        }
    }

    EXPECT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(CommonTest, ConstArrayResolverParameterAsArray)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U).s64();
        CONSTANT(2U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::LoadArray).u64().Inputs(0U, 1U);
            INST(4U, Opcode::Return).u64().Inputs(3U);
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0U, "i64[]");
    IrInterfaceTest interface(&program, &maps);
    g_options.SetConstArrayResolver(true);
    EXPECT_FALSE(graph->RunPass<ConstArrayResolver>(&interface));
}

TEST_F(CommonTest, ConstArrayResolverDifferentBlocks)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 0U).s64();
        CONSTANT(2U, 0U).s64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SaveState);
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).TypeId(68U);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 1U, 3U);
            INST(5U, Opcode::IfImm).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(6U, Opcode::StoreArray).u64().Inputs(4U, 1U, 2U);
            INST(7U, Opcode::Return).ref().Inputs(4U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::ReturnVoid).v0id();
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0U, "i64[]");
    IrInterfaceTest interface(&program, &maps);
    g_options.SetConstArrayResolver(true);
    EXPECT_FALSE(graph->RunPass<ConstArrayResolver>(&interface));
}

TEST_F(CommonTest, ConstArrayResolverArraySizeNotConstant)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s64();
        CONSTANT(4U, 0U).s64();
        CONSTANT(5U, 0U).s64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState);
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(1U).TypeId(68U);
            INST(2U, Opcode::NewArray).ref().Inputs(44U, 0U, 1U);
            INST(3U, Opcode::StoreArray).u64().Inputs(2U, 4U, 5U);
            INST(6U, Opcode::ReturnVoid).v0id();
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0U, "i64[]");
    IrInterfaceTest interface(&program, &maps);
    g_options.SetConstArrayResolver(true);
    EXPECT_FALSE(graph->RunPass<ConstArrayResolver>(&interface));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::bytecodeopt::test
