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

#include "common.h"
#include "const_array_resolver.h"

namespace panda::bytecodeopt::test {

TEST_F(CommonTest, ConstArrayResolverInt64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(1, 2).s64();
        CONSTANT(2, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(4, Opcode::LoadAndInitClass).ref().Inputs(20).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(4, 1, 20);
            INST(11, Opcode::StoreArray).u64().Inputs(3, 2, 2);
            INST(13, Opcode::StoreArray).u64().Inputs(3, 2, 2);
            INST(10, Opcode::Return).ref().Inputs(3);
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0, "i64[]");
    IrInterfaceTest interface(&program, &maps);
    options.SetConstArrayResolver(true);
    EXPECT_TRUE(graph->RunPass<ConstArrayResolver>(&interface));
    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        CONSTANT(1, 2).s64();
        CONSTANT(2, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(4, Opcode::LoadAndInitClass).ref().Inputs(20).TypeId(68);
            INST(22, Opcode::SaveState).NoVregs();
            INST(21, Opcode::LoadConstArray).ref().Inputs(22);
            INST(10, Opcode::Return).ref().Inputs(21);
        }
    }
    EXPECT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(CommonTest, ConstArrayResolverInt32)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(1, 2).s32();
        CONSTANT(2, 0).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(4, Opcode::LoadAndInitClass).ref().Inputs(20).TypeId(68);
            INST(3, Opcode::NewArray).ref().Inputs(4, 1, 20);
            INST(11, Opcode::StoreArray).u32().Inputs(3, 2, 2);
            INST(13, Opcode::StoreArray).u32().Inputs(3, 2, 2);
            INST(10, Opcode::Return).ref().Inputs(3);
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0, "i32[]");
    IrInterfaceTest interface(&program, &maps);
    options.SetConstArrayResolver(true);
    EXPECT_TRUE(graph->RunPass<ConstArrayResolver>(&interface));
}

TEST_F(CommonTest, ConstArrayResolverFloat32)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 3).s32();
        CONSTANT(4, 0).s32();
        CONSTANT(6, 100).f64();
        CONSTANT(10, 1).s32();
        CONSTANT(13, 200).f64();
        CONSTANT(16, 2).s32();
        CONSTANT(20, 300).f64();

        BASIC_BLOCK(2, -1)
        {
            using namespace compiler::DataType;

            INST(3, Opcode::SaveState).NoVregs();
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(3).TypeId(68);
            INST(5, Opcode::NewArray).ref().Inputs(44, 0, 3);
            INST(12, Opcode::Cast).f32().SrcType(FLOAT64).Inputs(6);
            INST(11, Opcode::StoreArray).f32().Inputs(5, 4, 12);
            INST(19, Opcode::Cast).f32().SrcType(FLOAT64).Inputs(13);
            INST(18, Opcode::StoreArray).f32().Inputs(5, 10, 19);
            INST(26, Opcode::Cast).f32().SrcType(FLOAT64).Inputs(20);
            INST(25, Opcode::StoreArray).f32().Inputs(5, 16, 26);
            INST(27, Opcode::Return).ref().Inputs(5);
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0, "f32[]");
    IrInterfaceTest interface(&program, &maps);
    options.SetConstArrayResolver(true);
    EXPECT_TRUE(graph->RunPass<ConstArrayResolver>(&interface));
}

TEST_F(CommonTest, ConstArrayResolverFloat64)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s32();
        CONSTANT(1, 1).s32();
        CONSTANT(2, 2).s32();
        CONSTANT(3, 100.0).f64();
        CONSTANT(4, 200.0).f64();

        BASIC_BLOCK(2, -1)
        {
            INST(20, Opcode::SaveState).NoVregs();
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(20).TypeId(68);
            INST(5, Opcode::NewArray).ref().Inputs(44, 2, 20);
            INST(11, Opcode::StoreArray).f64().Inputs(5, 0, 3);
            INST(13, Opcode::StoreArray).f64().Inputs(5, 1, 4);
            INST(10, Opcode::Return).ref().Inputs(5);
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0, "f64[]");
    IrInterfaceTest interface(&program, &maps);
    options.SetConstArrayResolver(true);
    EXPECT_TRUE(graph->RunPass<ConstArrayResolver>(&interface));
}

TEST_F(CommonTest, ConstArrayResolverByteAccess)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s32();
        CONSTANT(1, 1).s32();
        CONSTANT(2, 2).s32();
        CONSTANT(3, 3).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).NoVregs();
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(4).TypeId(68);
            INST(5, Opcode::NewArray).ref().Inputs(44, 3, 4);
            INST(6, Opcode::StoreArray).s8().Inputs(5, 0, 0);
            INST(7, Opcode::StoreArray).s8().Inputs(5, 1, 1);
            INST(8, Opcode::StoreArray).s8().Inputs(5, 2, 2);
            INST(9, Opcode::Return).ref().Inputs(5);
            ;
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0, "i8[]");
    IrInterfaceTest interface(&program, &maps);
    options.SetConstArrayResolver(true);
    EXPECT_TRUE(graph->RunPass<ConstArrayResolver>(&interface));

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        CONSTANT(0, 0).s32();
        CONSTANT(1, 1).s32();
        CONSTANT(2, 2).s32();
        CONSTANT(3, 3).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).NoVregs();
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(4).TypeId(68);
            INST(11, Opcode::SaveState).NoVregs();
            INST(10, Opcode::LoadConstArray).ref().Inputs(11);
            INST(9, Opcode::Return).ref().Inputs(10);
        }
    }

    EXPECT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(CommonTest, ConstArrayResolverStringAccess)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0).s32();
        CONSTANT(1, 1).s32();
        CONSTANT(2, 2).s32();
        CONSTANT(3, 3).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).NoVregs();
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(4).TypeId(68);
            INST(5, Opcode::NewArray).ref().Inputs(44, 3, 4);

            INST(6, Opcode::SaveState).NoVregs();
            INST(7, Opcode::LoadString).ref().Inputs(6);
            INST(8, Opcode::StoreArray).ref().Inputs(5, 0, 7);

            INST(9, Opcode::SaveState).NoVregs();
            INST(10, Opcode::LoadString).ref().Inputs(9);
            INST(11, Opcode::StoreArray).ref().Inputs(5, 1, 10);

            INST(12, Opcode::SaveState).NoVregs();
            INST(13, Opcode::LoadString).ref().Inputs(12);
            INST(24, Opcode::StoreArray).ref().Inputs(5, 2, 13);

            INST(15, Opcode::Return).ref().Inputs(5);
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0, "panda/String[]");
    IrInterfaceTest interface(&program, &maps);
    options.SetConstArrayResolver(true);
    EXPECT_TRUE(graph->RunPass<ConstArrayResolver>(&interface));

    auto expected = CreateEmptyGraph();
    GRAPH(expected)
    {
        CONSTANT(0, 0).s32();
        CONSTANT(1, 1).s32();
        CONSTANT(2, 2).s32();
        CONSTANT(3, 3).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(4, Opcode::SaveState).NoVregs();
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(4).TypeId(68);
            INST(5, Opcode::SaveState).NoVregs();
            INST(6, Opcode::LoadConstArray).ref().Inputs(5);

            INST(7, Opcode::SaveState).NoVregs();
            INST(8, Opcode::LoadString).ref().Inputs(7);
            INST(9, Opcode::SaveState).NoVregs();
            INST(10, Opcode::LoadString).ref().Inputs(9);
            INST(11, Opcode::SaveState).NoVregs();
            INST(12, Opcode::LoadString).ref().Inputs(11);

            INST(13, Opcode::Return).ref().Inputs(6);
        }
    }

    EXPECT_TRUE(GraphComparator().Compare(graph, expected));
}

TEST_F(CommonTest, ConstArrayResolverParameterAsArray)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        CONSTANT(1, 0).s64();
        CONSTANT(2, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::LoadArray).u64().Inputs(0, 1);
            INST(4, Opcode::Return).u64().Inputs(3);
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0, "i64[]");
    IrInterfaceTest interface(&program, &maps);
    options.SetConstArrayResolver(true);
    EXPECT_FALSE(graph->RunPass<ConstArrayResolver>(&interface));
}

TEST_F(CommonTest, ConstArrayResolverDifferentBlocks)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s32();
        CONSTANT(1, 0).s64();
        CONSTANT(2, 0).s64();

        BASIC_BLOCK(2, 3, 4)
        {
            INST(3, Opcode::SaveState);
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(3).TypeId(68);
            INST(4, Opcode::NewArray).ref().Inputs(44, 1, 3);
            INST(5, Opcode::IfImm).SrcType(compiler::DataType::INT32).CC(compiler::CC_EQ).Imm(0).Inputs(0);
        }
        BASIC_BLOCK(3, -1)
        {
            INST(6, Opcode::StoreArray).u64().Inputs(4, 1, 2);
            INST(7, Opcode::Return).ref().Inputs(4);
        }
        BASIC_BLOCK(4, -1)
        {
            INST(8, Opcode::ReturnVoid).v0id();
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0, "i64[]");
    IrInterfaceTest interface(&program, &maps);
    options.SetConstArrayResolver(true);
    EXPECT_FALSE(graph->RunPass<ConstArrayResolver>(&interface));
}

TEST_F(CommonTest, ConstArrayResolverArraySizeNotConstant)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).s64();
        CONSTANT(4, 0).s64();
        CONSTANT(5, 0).s64();

        BASIC_BLOCK(2, -1)
        {
            INST(1, Opcode::SaveState);
            INST(44, Opcode::LoadAndInitClass).ref().Inputs(1).TypeId(68);
            INST(2, Opcode::NewArray).ref().Inputs(44, 0, 1);
            INST(3, Opcode::StoreArray).u64().Inputs(2, 4, 5);
            INST(6, Opcode::ReturnVoid).v0id();
        }
    }

    pandasm::Program program;
    pandasm::AsmEmitter::PandaFileToPandaAsmMaps maps;
    maps.classes.emplace(0, "i64[]");
    IrInterfaceTest interface(&program, &maps);
    options.SetConstArrayResolver(true);
    EXPECT_FALSE(graph->RunPass<ConstArrayResolver>(&interface));
}

}  // namespace panda::bytecodeopt::test
