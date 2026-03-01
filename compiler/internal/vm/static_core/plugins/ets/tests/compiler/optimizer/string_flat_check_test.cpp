/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "compiler/tests/unit_test.h"
#include "optimizer/optimizations/string_flat_check.h"

namespace ark::compiler {
class StringFlatCheckTest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(StringFlatCheckTest, NotRequired)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::SaveState).Inputs(0U).SrcVregs({2U});
            INST(9U, Opcode::NullCheck).Inputs(0U, 8U).ref();
            INST(10U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_IS_EMPTY)
                .Inputs({{DataType::REFERENCE, 9U}})
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE)
                .ref();
            INST(11U, Opcode::Return).Inputs(10U).ref();
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<StringFlatCheck>());
}

TEST_F(StringFlatCheckTest, NotRequiredForLoadString)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        PARAMETER(2U, 2U).i32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(8U, Opcode::SaveState).Inputs(0U).SrcVregs({2U});
            INST(9U, Opcode::LoadString).Inputs(8U).ref();
            INST(10U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 9U}, {DataType::INT32, 1U}, {DataType::NO_TYPE, 8U}})
                .ref();
            INST(11U, Opcode::Return).Inputs(10U).ref();
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<StringFlatCheck>());
}

SRC_GRAPH(SingleUser, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        PARAMETER(2U, 2U).i32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(40U, Opcode::SaveState).Inputs(0U).SrcVregs({2U});
            INST(41U, Opcode::CallStatic).Inlined().Inputs({{DataType::NO_TYPE, 40U}}).v0id();
            INST(8U, Opcode::SaveState).Caller(41U).Inputs(0U).SrcVregs({2U});
            INST(9U, Opcode::NullCheck).Inputs(0U, 8U).ref();
            INST(10U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 9U}, {DataType::INT32, 1U}, {DataType::NO_TYPE, 8U}})
                .ref();
            INST(42U, Opcode::ReturnInlined).Inputs(40U);
            INST(11U, Opcode::Return).Inputs(10U).ref();
        }
    }
}

OUT_GRAPH(SingleUser, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        PARAMETER(2U, 2U).i32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(40U, Opcode::SaveState).Inputs(0U).SrcVregs({2U});
            INST(41U, Opcode::CallStatic).Inlined().Inputs({{DataType::NO_TYPE, 40U}}).v0id();
            INST(8U, Opcode::SaveState).Caller(41U).Inputs(0U).SrcVregs({2U});
            INST(9U, Opcode::NullCheck).Inputs(0U, 8U).ref();
            INST(45U, Opcode::SaveState).Caller(41U).Inputs(0U).SrcVregs({2U});
            INST(43U, Opcode::StringFlatCheck).Inputs(9U, 45U).ref();
            INST(44U, Opcode::SaveState).Caller(41U).Inputs(43U).SrcVregs({2U});
            INST(10U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 43U}, {DataType::INT32, 1U}, {DataType::NO_TYPE, 44U}})
                .ref();
            INST(42U, Opcode::ReturnInlined).Inputs(40U);
            INST(11U, Opcode::Return).Inputs(10U).ref();
        }
    }
}

TEST_F(StringFlatCheckTest, SingleUser)
{
    src_graph::SingleUser::CREATE(GetGraph());
    INS(8U).CastToSaveState()->SetInliningDepth(1U);
    ASSERT_TRUE(GetGraph()->RunPass<StringFlatCheck>());

    auto *graph = CreateEmptyGraph();
    out_graph::SingleUser::CREATE(graph);
    INS(8U).CastToSaveState()->SetInliningDepth(1U);
    INS(44U).CastToSaveState()->SetInliningDepth(1U);
    INS(45U).CastToSaveState()->SetInliningDepth(1U);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// CC-OFFNXT(huge_method, G.FUD.05) graph creation
SRC_GRAPH(MultipleUsers, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        PARAMETER(2U, 2U).i32();
        PARAMETER(3U, 3U).i32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(40U, Opcode::SaveState).Inputs(0U).SrcVregs({2U});
            INST(41U, Opcode::Call)
                .Inputs({{DataType::NO_TYPE, 40U}})
                .SetFlag(compiler::inst_flags::REQUIRE_STATE)
                .v0id();
            INST(5U, Opcode::Compare).Inputs(1U, 2U).CC(ConditionCode::CC_GE).b();
            INST(6U, Opcode::IfImm).Inputs(5U).Imm(0U).CC(ConditionCode::CC_NE);
        }

        BASIC_BLOCK(3U, 5U, 6U)
        {
            INST(7U, Opcode::Compare).Inputs(1U, 3U).CC(ConditionCode::CC_GE).b();
            INST(8U, Opcode::IfImm).Inputs(7U).Imm(0U).CC(ConditionCode::CC_NE);
        }

        BASIC_BLOCK(4U, 7U, 8U)
        {
            INST(28U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_LENGTH)
                .Inputs({{DataType::REFERENCE, 0U}})
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE)
                .i32();
            INST(9U, Opcode::Compare).Inputs(28U, 3U).CC(ConditionCode::CC_GE).b();
            INST(10U, Opcode::IfImm).Inputs(9U).Imm(0U).CC(ConditionCode::CC_NE);
        }

        BASIC_BLOCK(5U, 9U)
        {
            INST(20U, Opcode::SaveState).Inputs(0U).SrcVregs({2U});
            INST(21U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 0U}, {DataType::INT32, 1U}, {DataType::NO_TYPE, 20U}})
                .ref();
        }

        BASIC_BLOCK(6U, 9U)
        {
            INST(22U, Opcode::SaveState).Inputs(0U).SrcVregs({2U});
            INST(23U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 0U}, {DataType::INT32, 1U}, {DataType::NO_TYPE, 22U}})
                .ref();
        }

        BASIC_BLOCK(7U, 9U)
        {
            INST(24U, Opcode::SaveState).Inputs(0U).SrcVregs({2U});
            INST(25U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 0U}, {DataType::INT32, 2U}, {DataType::NO_TYPE, 24U}})
                .ref();
        }

        BASIC_BLOCK(8U, 9U)
        {
            INST(26U, Opcode::SaveState).Inputs(0U).SrcVregs({2U});
            INST(27U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 0U}, {DataType::INT32, 1U}, {DataType::NO_TYPE, 26U}})
                .ref();
        }

        BASIC_BLOCK(9U, -1L)
        {
            INST(30U, Opcode::Phi).Inputs(21U, 23U, 25U, 27U).ref();
            INST(31U, Opcode::Return).Inputs(30U).ref();
        }
    }
}

// CC-OFFNXT(huge_method, G.FUD.05) graph creation
OUT_GRAPH(MultipleUsers, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        PARAMETER(2U, 2U).i32();
        PARAMETER(3U, 3U).i32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(46U, Opcode::SaveState).Inputs(0U).SrcVregs({2U});
            INST(44U, Opcode::StringFlatCheck).Inputs(0U, 46U).ref();
            INST(40U, Opcode::SaveState).Inputs(44U).SrcVregs({2U});
            INST(41U, Opcode::Call)
                .Inputs({{DataType::NO_TYPE, 40U}})
                .SetFlag(compiler::inst_flags::REQUIRE_STATE)
                .v0id();
            INST(5U, Opcode::Compare).Inputs(1U, 2U).CC(ConditionCode::CC_GE).b();
            INST(6U, Opcode::IfImm).Inputs(5U).Imm(0U).CC(ConditionCode::CC_NE);
        }

        BASIC_BLOCK(3U, 5U, 6U)
        {
            INST(7U, Opcode::Compare).Inputs(1U, 3U).CC(ConditionCode::CC_GE).b();
            INST(8U, Opcode::IfImm).Inputs(7U).Imm(0U).CC(ConditionCode::CC_NE);
        }

        BASIC_BLOCK(4U, 7U, 8U)
        {
            INST(28U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_LENGTH)
                .Inputs({{DataType::REFERENCE, 44U}})
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE)
                .i32();
            INST(9U, Opcode::Compare).Inputs(28U, 3U).CC(ConditionCode::CC_GE).b();
            INST(10U, Opcode::IfImm).Inputs(9U).Imm(0U).CC(ConditionCode::CC_NE);
        }

        BASIC_BLOCK(5U, 9U)
        {
            INST(20U, Opcode::SaveState).Inputs(44U).SrcVregs({2U});
            INST(21U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 44U}, {DataType::INT32, 1U}, {DataType::NO_TYPE, 20U}})
                .ref();
        }

        BASIC_BLOCK(6U, 9U)
        {
            INST(22U, Opcode::SaveState).Inputs(44U).SrcVregs({2U});
            INST(23U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 44U}, {DataType::INT32, 1U}, {DataType::NO_TYPE, 22U}})
                .ref();
        }

        BASIC_BLOCK(7U, 9U)
        {
            INST(24U, Opcode::SaveState).Inputs(44U).SrcVregs({2U});
            INST(25U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 44U}, {DataType::INT32, 2U}, {DataType::NO_TYPE, 24U}})
                .ref();
        }

        BASIC_BLOCK(8U, 9U)
        {
            INST(43U, Opcode::SaveState).Inputs(44U).SrcVregs({2U});
            INST(42U, Opcode::StringFlatCheck).Inputs(44U, 43U).ref();
            INST(26U, Opcode::SaveState).Inputs(42U).SrcVregs({2U});
            INST(27U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 42U}, {DataType::INT32, 1U}, {DataType::NO_TYPE, 26U}})
                .ref();
        }

        BASIC_BLOCK(9U, -1L)
        {
            INST(30U, Opcode::Phi).Inputs(21U, 23U, 25U, 27U).ref();
            INST(31U, Opcode::Return).Inputs(30U).ref();
        }
    }
}

TEST_F(StringFlatCheckTest, MultipleUsers)
{
    src_graph::MultipleUsers::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<StringFlatCheck>());

    auto *graph = CreateEmptyGraph();
    out_graph::MultipleUsers::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// CC-OFFNXT(huge_method, G.FUD.05) graph creation
SRC_GRAPH(InsertAfterOsrEntry, Graph *graph)
{
    graph->SetMode(GraphMode::Osr());
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).i32();

        BASIC_BLOCK(2U, 6U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({2U, 4U});
            INST(4U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 0U}, {DataType::INT32, 2U}, {DataType::NO_TYPE, 3U}})
                .ref();
        }

        BASIC_BLOCK(6U, 3U, 4U)
        {
            INST(40U, Opcode::Compare).Inputs(0U, 1U).CC(ConditionCode::CC_NE).b();
            INST(41U, Opcode::IfImm).Inputs(40U).Imm(0U).CC(ConditionCode::CC_NE);
        }

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(10U, Opcode::Phi).Inputs({{6U, 0U}, {5U, 16U}}).ref();
            INST(5U, Opcode::SaveStateOsr).Inputs(0U, 10U, 1U, 4U).SrcVregs({2U, 3U, 4U, 5U});
            INST(11U, Opcode::SaveState).Inputs(0U, 10U, 1U, 4U).SrcVregs({2U, 3U, 4U, 5U});
            INST(12U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 0U}, {DataType::INT32, 2U}, {DataType::NO_TYPE, 11U}})
                .ref();
            INST(13U, Opcode::Compare).Inputs(1U, 12U).CC(ConditionCode::CC_NE).b();
            INST(14U, Opcode::IfImm).Inputs(13U).Imm(0U).CC(ConditionCode::CC_NE);
        }

        BASIC_BLOCK(5U, 3U, 5U)
        {
            INST(19U, Opcode::SaveStateOsr).Inputs(0U, 10U, 1U, 4U).SrcVregs({2U, 3U, 4U, 5U});
            INST(15U, Opcode::SaveState).Inputs(0U, 10U, 1U, 4U).SrcVregs({2U, 3U, 4U, 5U});
            INST(16U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 0U}, {DataType::INT32, 2U}, {DataType::NO_TYPE, 15U}})
                .ref();
            INST(17U, Opcode::Compare).Inputs(1U, 16U).CC(ConditionCode::CC_NE).b();
            INST(18U, Opcode::IfImm).Inputs(17U).Imm(0U).CC(ConditionCode::CC_NE);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(20U, Opcode::SaveState).Inputs(0U).SrcVregs({2U});
            INST(21U, Opcode::Call)
                .Inputs({{DataType::REFERENCE, 0U}, {DataType::NO_TYPE, 20U}})
                .SetFlag(compiler::inst_flags::REQUIRE_STATE)
                .ref();
            INST(30U, Opcode::Return).Inputs(21U).ref();
        }
    }
}

// CC-OFFNXT(huge_method, G.FUD.05) graph creation
OUT_GRAPH(InsertAfterOsrEntry, Graph *graph)
{
    graph->SetMode(GraphMode::Osr());
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        PARAMETER(2U, 2U).i32();

        BASIC_BLOCK(2U, 6U)
        {
            INST(43U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({2U, 4U});
            INST(42U, Opcode::StringFlatCheck).Inputs(0U, 43U).ref();
            INST(3U, Opcode::SaveState).Inputs(42U, 1U, 0U).SrcVregs({2U, 4U, VirtualRegister::BRIDGE});
            INST(4U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 42U}, {DataType::INT32, 2U}, {DataType::NO_TYPE, 3U}})
                .ref();
        }

        BASIC_BLOCK(6U, 3U, 4U)
        {
            INST(40U, Opcode::Compare).Inputs(0U, 1U).CC(ConditionCode::CC_NE).b();
            INST(41U, Opcode::IfImm).Inputs(40U).Imm(0U).CC(ConditionCode::CC_NE);
        }

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(10U, Opcode::Phi).Inputs({{6U, 0U}, {5U, 16U}}).ref();
            INST(5U, Opcode::SaveStateOsr).Inputs(0U, 10U, 1U, 4U).SrcVregs({2U, 3U, 4U, 5U});
            INST(45U, Opcode::SaveState).Inputs(0U, 10U, 1U, 4U).SrcVregs({2U, 3U, 4U, 5U});
            INST(44U, Opcode::StringFlatCheck).Inputs(0U, 45U).ref();
            INST(11U, Opcode::SaveState)
                .Inputs(44U, 10U, 1U, 4U, 0U)
                .SrcVregs({2U, 3U, 4U, 5U, VirtualRegister::BRIDGE});
            INST(12U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 44U}, {DataType::INT32, 2U}, {DataType::NO_TYPE, 11U}})
                .ref();
            INST(13U, Opcode::Compare).Inputs(1U, 12U).CC(ConditionCode::CC_NE).b();
            INST(14U, Opcode::IfImm).Inputs(13U).Imm(0U).CC(ConditionCode::CC_NE);
        }

        BASIC_BLOCK(5U, 3U, 5U)
        {
            INST(19U, Opcode::SaveStateOsr).Inputs(0U, 10U, 1U, 4U).SrcVregs({2U, 3U, 4U, 5U});
            INST(47U, Opcode::SaveState).Inputs(0U, 10U, 1U, 4U).SrcVregs({2U, 3U, 4U, 5U});
            INST(46U, Opcode::StringFlatCheck).Inputs(0U, 47U).ref();
            INST(15U, Opcode::SaveState)
                .Inputs(46U, 10U, 1U, 4U, 0U)
                .SrcVregs({2U, 3U, 4U, 5U, VirtualRegister::BRIDGE});
            INST(16U, Opcode::Intrinsic)
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_REPEAT)
                .Inputs({{DataType::REFERENCE, 46U}, {DataType::INT32, 2U}, {DataType::NO_TYPE, 15U}})
                .ref();
            INST(17U, Opcode::Compare).Inputs(1U, 16U).CC(ConditionCode::CC_NE).b();
            INST(18U, Opcode::IfImm).Inputs(17U).Imm(0U).CC(ConditionCode::CC_NE);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(20U, Opcode::SaveState).Inputs(0U).SrcVregs({2U});
            INST(21U, Opcode::Call)
                .Inputs({{DataType::REFERENCE, 0U}, {DataType::NO_TYPE, 20U}})
                .SetFlag(compiler::inst_flags::REQUIRE_STATE)
                .ref();
            INST(30U, Opcode::Return).Inputs(21U).ref();
        }
    }
}

TEST_F(StringFlatCheckTest, InsertAfterOsrEntry)
{
    src_graph::InsertAfterOsrEntry::CREATE(GetGraph());
    ASSERT_TRUE(GetGraph()->RunPass<StringFlatCheck>());

    Graph *graph = CreateEmptyGraph();
    out_graph::InsertAfterOsrEntry::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
// NOLINTEND(readability-magic-numbers)
}  // namespace ark::compiler
