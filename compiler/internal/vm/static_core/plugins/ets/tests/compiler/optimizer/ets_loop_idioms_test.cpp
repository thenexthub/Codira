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

#include "tests/unit_test.h"

#include "optimizer/ir/graph_checker.h"
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/optimizations/loop_idioms.h"

#include <gtest/gtest.h>

namespace ark::compiler {
class EtsLoopIdiomsTest : public GraphTest {
protected:
    void SetUp() override
    {
        if (GetGraph()->GetArch() == Arch::AARCH32) {
            GTEST_SKIP();
        }
    }

public:
    EtsLoopIdiomsTest()
    {
        GetGraph()->SetLanguage(SourceLanguage::ETS);
    }

    NO_COPY_SEMANTIC(EtsLoopIdiomsTest);
    NO_MOVE_SEMANTIC(EtsLoopIdiomsTest);

    static RuntimeInterface::ClassPtr NewArrayClass()
    {
        return reinterpret_cast<RuntimeInterface::ClassPtr>(FAKE_NEW_ARRAY_CLASS);
    }

    static RuntimeInterface::FieldPtr TargetField()
    {
        return reinterpret_cast<RuntimeInterface::FieldPtr>(FAKE_TARGET_FIELD);
    }

    static constexpr uintptr_t FAKE_NEW_ARRAY_CLASS = 0xdeadf;
    static constexpr uintptr_t FAKE_TARGET_FIELD = 0xbeaff;
    static constexpr uint32_t PREHEADER_BB_ID = 2U;
};

// NOLINTBEGIN(readability-magic-numbers)
SRC_GRAPH(NewArrayDstFastMemmove, Graph *graph, RuntimeInterface::ClassPtr newArrayClass)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(21U, 42U);

        BASIC_BLOCK(EtsLoopIdiomsTest::PREHEADER_BB_ID, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(newArrayClass);
            INST(5U, Opcode::NewObject).ref().Inputs(4U, 3U);
            INST(6U, Opcode::NewArray).ref().Inputs(4U, 21U, 3U);
            INST(7U, Opcode::StoreObject).ref().Inputs(5U, 6U);
            INST(8U, Opcode::NullCheck).ref().Inputs(6U, 3U);
            INST(9U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(10U, Opcode::LenArray).i32().Inputs(9U);
            INST(11U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(12U, Opcode::Compare).b().Inputs(2U, 10U).CC(CC_LT).SrcType(DataType::INT32);
            INST(13U, Opcode::IfImm).Inputs(12U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(14U, Opcode::Phi).i32().Inputs(1U, 17U);
            INST(15U, Opcode::LoadArray).ref().Inputs(9U, 14U);
            INST(16U, Opcode::StoreArray).ref().Inputs(8U, 14U, 15U);
            INST(17U, Opcode::Add).i32().Inputs(14U, 2U);
            INST(18U, Opcode::Compare).b().Inputs(21U, 17U).CC(CC_LE).SrcType(DataType::INT32);
            INST(19U, Opcode::IfImm).Inputs(18U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(20U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(NewArrayDstFastMemmove, Graph *graph, RuntimeInterface::ClassPtr newArrayClass)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(21U, 42U);

        BASIC_BLOCK(EtsLoopIdiomsTest::PREHEADER_BB_ID, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(newArrayClass);
            INST(5U, Opcode::NewObject).ref().Inputs(4U, 3U);
            INST(6U, Opcode::NewArray).ref().Inputs(4U, 21U, 3U);
            INST(7U, Opcode::StoreObject).ref().Inputs(5U, 6U);
            INST(8U, Opcode::NullCheck).ref().Inputs(6U, 3U);
            INST(9U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(10U, Opcode::LenArray).i32().Inputs(9U);
            INST(11U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(12U, Opcode::Compare).b().Inputs(2U, 10U).CC(CC_LT).SrcType(DataType::INT32);
            INST(13U, Opcode::IfImm).Inputs(12U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(23U, Opcode::Add).i32().Inputs(1U, 1U);
            INST(24U, Opcode::Add).i32().Inputs(21U, 1U);
            INST(25U, Opcode::Sub).i32().Inputs(1U, 1U);
            INST(26U, Opcode::Add).i32().Inputs(23U, 25U);
            INST(27U, Opcode::SaveState)
                .Inputs(0U, 9U, 8U)
                .SrcVregs({0U, VirtualRegister::BRIDGE, VirtualRegister::BRIDGE});
            std::initializer_list<std::pair<int, int>> inputs = {{DataType::REFERENCE, 9U}, {DataType::REFERENCE, 8U},
                                                                 {DataType::INT32, 26U},    {DataType::INT32, 23U},
                                                                 {DataType::INT32, 24U},    {DataType::NO_TYPE, 27U}};
            INST(22U, Opcode::Intrinsic)
                .v0id()
                .Inputs(inputs)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_MEMMOVE_UNCHECKED_REF)
                .ClearFlag(compiler::inst_flags::CAN_THROW);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(20U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(EtsLoopIdiomsTest, NewArrayDstFastMemmove)
{
    src_graph::NewArrayDstFastMemmove::CREATE(GetGraph(), NewArrayClass());
    GraphChecker(GetGraph()).Check();
    ASSERT_NE(BB(PREHEADER_BB_ID).FindSaveStateDeoptimize(), nullptr);
    ASSERT_TRUE(GetGraph()->RunPass<LoopIdioms>());
    EXPECT_FALSE(GetGraph()->RunPass<Cleanup>());
    GraphChecker(GetGraph()).Check();
    auto *expectedGraph = CreateEmptyGraph();
    out_graph::NewArrayDstFastMemmove::CREATE(expectedGraph, NewArrayClass());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

SRC_GRAPH(NoNewArrayDstMemmove, Graph *graph, RuntimeInterface::ClassPtr newArrayClass)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(18U, 42U);

        BASIC_BLOCK(EtsLoopIdiomsTest::PREHEADER_BB_ID, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(newArrayClass);
            INST(5U, Opcode::NewArray).ref().Inputs(4U, 18U, 3U);
            INST(6U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(7U, Opcode::LenArray).i32().Inputs(6U);
            INST(8U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(9U, Opcode::Compare).b().Inputs(2U, 7U).CC(CC_LT).SrcType(DataType::INT32);
            INST(10U, Opcode::IfImm).Inputs(9U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(11U, Opcode::Phi).i32().Inputs(1U, 14U);
            INST(12U, Opcode::LoadArray).ref().Inputs(5U, 11U);
            INST(13U, Opcode::StoreArray).ref().Inputs(6U, 11U, 12U);
            INST(14U, Opcode::Add).i32().Inputs(11U, 2U);
            INST(15U, Opcode::Compare).b().Inputs(18U, 14U).CC(CC_LE).SrcType(DataType::INT32);
            INST(16U, Opcode::IfImm).Inputs(15U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(17U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(NoNewArrayDstMemmove, Graph *graph, RuntimeInterface::ClassPtr newArrayClass)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(18U, 42U);

        BASIC_BLOCK(EtsLoopIdiomsTest::PREHEADER_BB_ID, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(newArrayClass);
            INST(5U, Opcode::NewArray).ref().Inputs(4U, 18U, 3U);
            INST(6U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(7U, Opcode::LenArray).i32().Inputs(6U);
            INST(8U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(9U, Opcode::Compare).b().Inputs(2U, 7U).CC(CC_LT).SrcType(DataType::INT32);
            INST(10U, Opcode::IfImm).Inputs(9U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(21U, Opcode::Add).i32().Inputs(1U, 1U);
            INST(22U, Opcode::Add).i32().Inputs(18U, 1U);
            INST(23U, Opcode::Sub).i32().Inputs(1U, 1U);
            INST(24U, Opcode::Add).i32().Inputs(21U, 23U);
            INST(25U, Opcode::SaveState)
                .Inputs(0U, 5U, 6U)
                .SrcVregs({0U, VirtualRegister::BRIDGE, VirtualRegister::BRIDGE});
            std::initializer_list<std::pair<int, int>> inputs = {{DataType::REFERENCE, 5U}, {DataType::REFERENCE, 6U},
                                                                 {DataType::INT32, 24U},    {DataType::INT32, 21U},
                                                                 {DataType::INT32, 22U},    {DataType::NO_TYPE, 25U}};
            INST(20U, Opcode::Intrinsic)
                .v0id()
                .Inputs(inputs)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_MEMMOVE_UNCHECKED_REF)
                .ClearFlag(compiler::inst_flags::CAN_THROW);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(17U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(EtsLoopIdiomsTest, NoNewArrayDstMemmove)
{
    // When the StoreArray's instruction data flow input is not a NewArray instruction, ETS' extension for
    // the LoopIdioms pass substitutes the CompilerMemmoveUnchecked_Ref intrinsic instead of the more commonly
    // used CompilerFastMemmoveUnchecked_Ref. Because the es2panda frontend adds RefTypeCheck instructions
    // between the corresponding LoadArray and StoreArray, we cannot test this case other than with a unit test.
    src_graph::NoNewArrayDstMemmove::CREATE(GetGraph(), NewArrayClass());
    ASSERT_NE(BB(PREHEADER_BB_ID).FindSaveStateDeoptimize(), nullptr);
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GetGraph()->RunPass<LoopIdioms>());
    EXPECT_FALSE(GetGraph()->RunPass<Cleanup>());
    GraphChecker(GetGraph()).Check();
    auto *expectedGraph = CreateEmptyGraph();
    out_graph::NoNewArrayDstMemmove::CREATE(expectedGraph, NewArrayClass());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

SRC_GRAPH(SrcLoadObjectDominatesDst, Graph *graph, RuntimeInterface::ClassPtr newArrayClass,
          RuntimeInterface::FieldPtr storedField)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(20U, 42U);

        BASIC_BLOCK(EtsLoopIdiomsTest::PREHEADER_BB_ID, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(newArrayClass);
            INST(5U, Opcode::LoadObject).ref().ObjField(storedField).Inputs(0U);
            INST(6U, Opcode::NewArray).ref().Inputs(4U, 20U, 3U);
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(6U, 3U);
            INST(8U, Opcode::NullCheck).ref().Inputs(5U, 3U);
            INST(9U, Opcode::LenArray).i32().Inputs(8U);
            INST(10U, Opcode::SaveStateDeoptimize).Inputs(0U, 5U).SrcVregs({0U, 1U});
            INST(11U, Opcode::Compare).b().Inputs(2U, 9U).CC(CC_LT).SrcType(DataType::INT32);
            INST(12U, Opcode::IfImm).Inputs(11U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(13U, Opcode::Phi).i32().Inputs(1U, 16U);
            INST(14U, Opcode::LoadArray).ref().Inputs(5U, 13U);
            INST(15U, Opcode::StoreArray).ref().Inputs(6U, 13U, 14U);
            INST(16U, Opcode::Add).i32().Inputs(13U, 2U);
            INST(17U, Opcode::Compare).b().Inputs(20U, 16U).CC(CC_LE).SrcType(DataType::INT32);
            INST(18U, Opcode::IfImm).Inputs(17U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(19U, Opcode::ReturnVoid).v0id();
        }
    }
}

OUT_GRAPH(SrcLoadObjectDominatesDst, Graph *graph, RuntimeInterface::ClassPtr newArrayClass,
          RuntimeInterface::FieldPtr storedField)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(20U, 42U);

        BASIC_BLOCK(EtsLoopIdiomsTest::PREHEADER_BB_ID, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(newArrayClass);
            INST(5U, Opcode::LoadObject).ref().ObjField(storedField).Inputs(0U);
            INST(6U, Opcode::NewArray).ref().Inputs(4U, 20U, 3U);
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(6U, 3U);
            INST(8U, Opcode::NullCheck).ref().Inputs(5U, 3U);
            INST(9U, Opcode::LenArray).i32().Inputs(8U);
            INST(10U, Opcode::SaveStateDeoptimize).Inputs(0U, 5U).SrcVregs({0U, 1U});
            INST(11U, Opcode::Compare).b().Inputs(2U, 9U).CC(CC_LT).SrcType(DataType::INT32);
            INST(12U, Opcode::IfImm).Inputs(11U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(22U, Opcode::Add).i32().Inputs(1U, 1U);
            INST(23U, Opcode::Add).i32().Inputs(20U, 1U);
            INST(24U, Opcode::Sub).i32().Inputs(1U, 1U);
            INST(25U, Opcode::Add).i32().Inputs(22U, 24U);
            INST(26U, Opcode::SaveState)
                .Inputs(0U, 5U, 5U, 6U)
                .SrcVregs({0U, 1U, VirtualRegister::BRIDGE, VirtualRegister::BRIDGE});
            std::initializer_list<std::pair<int, int>> inputs = {{DataType::REFERENCE, 5U}, {DataType::REFERENCE, 6U},
                                                                 {DataType::INT32, 25U},    {DataType::INT32, 22U},
                                                                 {DataType::INT32, 23U},    {DataType::NO_TYPE, 26U}};
            INST(21U, Opcode::Intrinsic)
                .v0id()
                .Inputs(inputs)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_MEMMOVE_UNCHECKED_REF)
                .ClearFlag(compiler::inst_flags::CAN_THROW);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(19U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(EtsLoopIdiomsTest, SrcLoadObjectDominatesDst)
{
    src_graph::SrcLoadObjectDominatesDst::CREATE(GetGraph(), NewArrayClass(), TargetField());
    ASSERT_TRUE(GetGraph()->RunPass<LoopIdioms>());
    GraphChecker(GetGraph()).Check();

    auto *expectedGraph = CreateEmptyGraph();
    out_graph::SrcLoadObjectDominatesDst::CREATE(expectedGraph, NewArrayClass(), TargetField());
    EXPECT_TRUE(GraphComparator().Compare(GetGraph(), expectedGraph));
}

SRC_GRAPH(RefTypeCheckNoOpt, Graph *graph, RuntimeInterface::ClassPtr newArrayClass)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(21U, 42U);

        BASIC_BLOCK(EtsLoopIdiomsTest::PREHEADER_BB_ID, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(newArrayClass);
            INST(5U, Opcode::NewObject).ref().Inputs(4U, 3U);
            INST(6U, Opcode::NewArray).ref().Inputs(4U, 21U, 3U);
            INST(7U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(8U, Opcode::LenArray).i32().Inputs(7U);
            INST(9U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(10U, Opcode::Compare).b().Inputs(2U, 8U).CC(CC_LT).SrcType(DataType::INT32);
            INST(11U, Opcode::IfImm).Inputs(10U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(12U, Opcode::Phi).i32().Inputs(1U, 17U);
            INST(13U, Opcode::LoadArray).ref().Inputs(6U, 12U);
            INST(14U, Opcode::SaveState).Inputs(12U, 13U, 7U).SrcVregs({0U, 1U, 2U});
            INST(15U, Opcode::RefTypeCheck).ref().Inputs(6U, 13U, 14U);
            INST(16U, Opcode::StoreArray).ref().Inputs(7U, 12U, 15U);
            INST(17U, Opcode::Add).i32().Inputs(12U, 2U);
            INST(18U, Opcode::Compare).b().Inputs(21U, 17U).CC(CC_LE).SrcType(DataType::INT32);
            INST(19U, Opcode::IfImm).Inputs(18U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(20U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(EtsLoopIdiomsTest, RefTypeCheckNoOpt)
{
    src_graph::RefTypeCheckNoOpt::CREATE(GetGraph(), NewArrayClass());
    ASSERT_NE(BB(PREHEADER_BB_ID).FindSaveStateDeoptimize(), nullptr);
    Graph *clone = CreateEmptyGraph();
    src_graph::RefTypeCheckNoOpt::CREATE(clone, NewArrayClass());
    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

SRC_GRAPH(TrueAliasNoOpt, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(16U, 42U);

        BASIC_BLOCK(EtsLoopIdiomsTest::PREHEADER_BB_ID, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(5U, Opcode::LenArray).i32().Inputs(4U);
            INST(6U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(7U, Opcode::Compare).b().Inputs(2U, 5U).CC(CC_LT).SrcType(DataType::INT32);
            INST(8U, Opcode::IfImm).Inputs(7U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(9U, Opcode::Phi).i32().Inputs(1U, 12U);
            INST(10U, Opcode::LoadArray).ref().Inputs(4U, 9U);
            INST(11U, Opcode::StoreArray).ref().Inputs(4U, 9U, 10U);
            INST(12U, Opcode::Add).i32().Inputs(9U, 2U);
            INST(13U, Opcode::Compare).b().Inputs(16U, 12U).CC(CC_LE).SrcType(DataType::INT32);
            INST(14U, Opcode::IfImm).Inputs(13U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(15U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(EtsLoopIdiomsTest, TrueAliasNoOpt)
{
    src_graph::TrueAliasNoOpt::CREATE(GetGraph());
    ASSERT_NE(BB(PREHEADER_BB_ID).FindSaveStateDeoptimize(), nullptr);
    Graph *clone = CreateEmptyGraph();
    src_graph::TrueAliasNoOpt::CREATE(clone);
    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

SRC_GRAPH(MayAliasNoOpt, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        CONSTANT(2U, 0U);
        CONSTANT(3U, 1U);
        CONSTANT(18U, 42U);

        BASIC_BLOCK(EtsLoopIdiomsTest::PREHEADER_BB_ID, 3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(5U, Opcode::NullCheck).ref().Inputs(0U, 4U);
            INST(6U, Opcode::NullCheck).ref().Inputs(1U, 4U);
            INST(7U, Opcode::LenArray).i32().Inputs(5U);
            INST(8U, Opcode::SaveStateDeoptimize).Inputs(0U).SrcVregs({0U});
            INST(9U, Opcode::Compare).b().Inputs(3U, 7U).CC(CC_LT).SrcType(DataType::INT32);
            INST(10U, Opcode::IfImm).Inputs(9U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(11U, Opcode::Phi).i32().Inputs(2U, 14U);
            INST(12U, Opcode::LoadArray).ref().Inputs(5U, 11U);
            INST(13U, Opcode::StoreArray).ref().Inputs(6U, 11U, 12U);
            INST(14U, Opcode::Add).i32().Inputs(11U, 3U);
            INST(15U, Opcode::Compare).b().Inputs(18U, 14U).CC(CC_LE).SrcType(DataType::INT32);
            INST(16U, Opcode::IfImm).Inputs(15U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(17U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(EtsLoopIdiomsTest, MayAliasNoOpt)
{
    src_graph::MayAliasNoOpt::CREATE(GetGraph());
    ASSERT_NE(BB(PREHEADER_BB_ID).FindSaveStateDeoptimize(), nullptr);
    Graph *clone = CreateEmptyGraph();
    src_graph::MayAliasNoOpt::CREATE(clone);
    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

SRC_GRAPH(NoSaveStateDeoptimizeInPreheader, Graph *graph, RuntimeInterface::ClassPtr newArrayClass)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(20U, 42U);

        BASIC_BLOCK(EtsLoopIdiomsTest::PREHEADER_BB_ID, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(newArrayClass);
            INST(5U, Opcode::NewObject).ref().Inputs(4U, 3U);
            INST(6U, Opcode::NewArray).ref().Inputs(4U, 20U, 3U);
            INST(7U, Opcode::StoreObject).ref().Inputs(5U, 6U);
            INST(8U, Opcode::NullCheck).ref().Inputs(6U, 3U);
            INST(9U, Opcode::NullCheck).ref().Inputs(0U, 3U);
            INST(10U, Opcode::LenArray).i32().Inputs(9U);
            INST(11U, Opcode::Compare).b().Inputs(2U, 10U).CC(CC_LT).SrcType(DataType::INT32);
            INST(12U, Opcode::IfImm).Inputs(11U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(13U, Opcode::Phi).i32().Inputs(1U, 16U);
            INST(14U, Opcode::LoadArray).ref().Inputs(9U, 13U);
            INST(15U, Opcode::StoreArray).ref().Inputs(8U, 13U, 14U);
            INST(16U, Opcode::Add).i32().Inputs(13U, 2U);
            INST(17U, Opcode::Compare).b().Inputs(20U, 16U).CC(CC_LE).SrcType(DataType::INT32);
            INST(18U, Opcode::IfImm).Inputs(17U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(19U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(EtsLoopIdiomsTest, NoSaveStateDeoptimizeInPreheader)
{
    src_graph::NoSaveStateDeoptimizeInPreheader::CREATE(GetGraph(), NewArrayClass());
    ASSERT_EQ(BB(PREHEADER_BB_ID).FindSaveStateDeoptimize(), nullptr);
    Graph *clone = CreateEmptyGraph();
    src_graph::NoSaveStateDeoptimizeInPreheader::CREATE(clone, NewArrayClass());
    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

SRC_GRAPH(RewriteSrcWithDst, Graph *graph, RuntimeInterface::ClassPtr newArrayClass,
          RuntimeInterface::FieldPtr storedField)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, 1U);
        CONSTANT(21U, 42U);

        BASIC_BLOCK(EtsLoopIdiomsTest::PREHEADER_BB_ID, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(newArrayClass);
            INST(5U, Opcode::NewArray).ref().Inputs(4U, 21U, 3U);
            INST(6U, Opcode::StoreObject).ref().ObjField(storedField).Inputs(0U, 5U);
            INST(7U, Opcode::LoadObject).ref().ObjField(storedField).Inputs(0U);
            INST(8U, Opcode::NullCheck).ref().Inputs(5U, 3U);
            INST(9U, Opcode::NullCheck).ref().Inputs(7U, 3U);
            INST(10U, Opcode::LenArray).i32().Inputs(9U);
            INST(11U, Opcode::SaveStateDeoptimize).Inputs(0U, 7U).SrcVregs({0U, 1U});
            INST(12U, Opcode::Compare).b().Inputs(2U, 10U).CC(CC_LT).SrcType(DataType::INT32);
            INST(13U, Opcode::IfImm).Inputs(12U).Imm(0U).CC(CC_EQ).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(14U, Opcode::Phi).i32().Inputs(1U, 17U);
            INST(15U, Opcode::LoadArray).ref().Inputs(9U, 14U);
            INST(16U, Opcode::StoreArray).ref().Inputs(8U, 14U, 15U);
            INST(17U, Opcode::Add).i32().Inputs(14U, 2U);
            INST(18U, Opcode::Compare).b().Inputs(21U, 17U).CC(CC_LE).SrcType(DataType::INT32);
            INST(19U, Opcode::IfImm).Inputs(18U).Imm(0U).CC(CC_NE).SrcType(DataType::BOOL);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(20U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(EtsLoopIdiomsTest, RewriteSrcWithDst)
{
    src_graph::RewriteSrcWithDst::CREATE(GetGraph(), NewArrayClass(), TargetField());
    ASSERT_NE(BB(PREHEADER_BB_ID).FindSaveStateDeoptimize(), nullptr);
    Graph *clone = CreateEmptyGraph();
    src_graph::RewriteSrcWithDst::CREATE(clone, NewArrayClass(), TargetField());
    ASSERT_FALSE(GetGraph()->RunPass<LoopIdioms>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// NOLINTEND(readability-magic-numbers)
}  // namespace ark::compiler
