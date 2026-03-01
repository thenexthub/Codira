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

#include "bytecodeopt_peepholes.h"
#include "call_devirtualization.h"
#include "common.h"
#include "compiler/optimizer/optimizations/cleanup.h"

namespace ark::bytecodeopt::test {

// NOLINTBEGIN(readability-magic-numbers)

TEST_F(IrBuilderTest, PeepholesTryBlockInstBetween)
{
    auto source = R"(
    .record E {}
    .record R {
        u1 field
    }

    .function void R.ctor(R a0) <ctor> {
        newobj v0, E
        throw v0
    }

    .function u8 main() {
    try_begin:
        movi v1, 0x1
        newobj v0, R
        movi v1, 0x2
        call.short R.ctor, v0
    try_end:
        ldai 0x0
        return
    catch_all:
        lda v1
        return
    .catchall try_begin, try_end, catch_all
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    EXPECT_FALSE(GetGraph()->RunPass<BytecodeOptPeepholes>());
}

TEST_F(IrBuilderTest, PeepholesTryBlockNoInstBetween)
{
    auto source = R"(
    .record E {}
    .record R {
        u1 field
    }

    .function void R.ctor(R a0) <ctor> {
        newobj v0, E
        throw v0
    }

    .function u8 main() {
    try_begin:
        movi v1, 0x1
        newobj v0, R
        call.short R.ctor, v0
    try_end:
        ldai 0x0
        return
    catch_all:
        lda v1
        return
    .catchall try_begin, try_end, catch_all
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));

    EXPECT_TRUE(GetGraph()->RunPass<BytecodeOptPeepholes>());
}

TEST_F(IrBuilderTest, VirtualCallToDirectCallSimple)
{
    auto source = std::string(R"(
        .record A {}
        .record B <extends=A> {}

        .function i32 A.foo(A a0) <noimpl>
        .function i32 B.foo(B a0) {
            ldai 10
            return
        }

        .function i32 main() {
            newobj v10, B
            call.virt B.foo, v10

            ldai 0
            return
        }
        )");

    ASSERT_TRUE(ParseToGraph(source, "main"));
    EXPECT_TRUE(GetGraph()->RunPass<CallDevirtualization>());
    const int expStaticCallsCount = 2;
    CheckVirtualCallsNoExist(expStaticCallsCount);
}

TEST_F(IrBuilderTest, VirtualCallToDirectCallAdvanced)
{
    auto source = std::string(R"(
        .record A {}
        .record B <extends=A> {}

        .function i32 A.foo(A a0) <noimpl>
        .function i32 B.foo(B a0) {
            ldai 10
            return
        }

        .function i32 main() {
            newobj v10, B
            call.virt A.foo, v10

            ldai 0
            return
        }
        )");

    ASSERT_TRUE(ParseToGraph(source, "main"));
    EXPECT_FALSE(GetGraph()->RunPass<CallDevirtualization>());
}

TEST_F(IrBuilderTest, VirtualCallToDirectCallArray)
{
    auto source = std::string(R"(
        .record std.core.Array <external>
        .record std.core.Double <external>
        .record std.core.Object <external>

        .function i32 main() {
            initobj.short std.core.Array._ctor_:(std.core.Array)
            sta.obj v0

            newobj v1, std.core.Double
            fldai.64 0x3ff0000000000000
            call.acc.short std.core.Double._ctor_:(std.core.Double, f64), v1, 0x1

            call.virt.short std.core.Array.pushOne:(std.core.Array, std.core.Object), v0, v1

            ldai 0
            return
        }

        .function void std.core.Array._ctor_(std.core.Array a0) <ctor, external>

        .function f64 std.core.Array.pushOne(std.core.Array a0, std.core.Object a1) <external, access.function=public>

        .function void std.core.Double._ctor_(std.core.Double a0, f64 a1) <ctor, external, access.function=public>
        )");

    ASSERT_TRUE(ParseToGraph(source, "main"));
    EXPECT_TRUE(GetGraph()->RunPass<CallDevirtualization>());
    const int expStaticCallsCount = 7;
    CheckVirtualCallsNoExist(expStaticCallsCount);
}

TEST_F(IrBuilderTest, VirtualCallOnStaticFunction)
{
    auto source = std::string(R"(
        .record A <access.record=public> {}
        .record std.core.Object <external>

        .function f64 A.func() <static, access.function=public> {
            fldai.64 0x3ff0000000000000
            return.64
        }

        .function void main() <static, access.function=public> {
            initobj.short A._ctor_:(A)
            call.virt.acc.short A.func:(A), v0, 0x0
            return.void
        }

        .function void A._ctor_(A a0) <ctor, access.function=public> {
            call.short std.core.Object._ctor_:(std.core.Object), a0
            return.void
        }

        .function f64 A.func(A a0) <access.function=public> {
            fldai.64 0x4000000000000000
            return.64
        }

        .function void std.core.Object._ctor_(std.core.Object a0) <ctor, external, access.function=public>
        )");

    ASSERT_TRUE(ParseToGraph(source, "main"));
    EXPECT_TRUE(GetGraph()->RunPass<CallDevirtualization>());
    const int expStaticCallsCount = 3;
    CheckVirtualCallsNoExist(expStaticCallsCount);
}

// NOTE(aromanov): enable
TEST_F(CommonTest, DISABLED_NoNullCheck)
{
    RuntimeInterfaceMock runtime(0U);
    auto graph = CreateEmptyGraph();
    graph->SetRuntime(&runtime);
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;
        CONSTANT(6U, 0U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::SaveState).NoVregs();
            INST(1U, Opcode::LoadAndInitClass).ref().Inputs(0U).TypeId(68U);
            INST(2U, Opcode::NewObject).ref().Inputs(1U, 0U).TypeId(68U);
            INST(3U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 2U}, {NO_TYPE, 3U}});
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }

    EXPECT_TRUE(graph->RunPass<BytecodeOptPeepholes>());
    EXPECT_TRUE(graph->RunPass<compiler::Cleanup>());

    auto after = CreateEmptyGraph();
    GRAPH(after)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;
        CONSTANT(6U, 0U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::SaveState).NoVregs();
            INST(1U, Opcode::LoadAndInitClass).ref().Inputs(0U).TypeId(68U);
            INST(3U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::InitObject).ref().Inputs({{REFERENCE, 1U}, {NO_TYPE, 3U}});
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }

    EXPECT_TRUE(GraphComparator().Compare(graph, after));
}

// NOTE(aromanov): enable
TEST_F(CommonTest, DISABLED_NotRelatedNullCheck)
{
    RuntimeInterfaceMock runtime(1U);
    auto graph = CreateEmptyGraph();
    graph->SetRuntime(&runtime);
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;
        PARAMETER(10U, 0U).ref();
        CONSTANT(6U, 0U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::SaveState).NoVregs();
            INST(1U, Opcode::LoadAndInitClass).ref().Inputs(0U);
            INST(2U, Opcode::NewObject).ref().Inputs(1U, 0U);
            INST(3U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::NullCheck).ref().Inputs(10U, 3U);
            INST(5U, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 2U}, {NO_TYPE, 3U}});
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }

    EXPECT_FALSE(graph->RunPass<BytecodeOptPeepholes>());
}

TEST_F(CommonTest, CallStaticOtherBasicBlock)
{
    RuntimeInterfaceMock runtime(1U);
    auto graph = CreateEmptyGraph();
    graph->SetRuntime(&runtime);
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;
        PARAMETER(10U, 0U).ref();
        CONSTANT(6U, 0U);
        BASIC_BLOCK(2U, 3U)
        {
            INST(0U, Opcode::SaveState).NoVregs();
            INST(1U, Opcode::LoadAndInitClass).ref().Inputs(0U);
            INST(2U, Opcode::NewObject).ref().Inputs(1U, 0U);
            INST(3U, Opcode::SaveState).NoVregs();
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 2U}, {NO_TYPE, 3U}});
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }

    EXPECT_FALSE(graph->RunPass<BytecodeOptPeepholes>());
}

// NOTE(aromanov): enable
TEST_F(CommonTest, DISABLED_NoSaveStateNullCheckAfterNewObject)
{
    RuntimeInterfaceMock runtime(0U);
    auto graph = CreateEmptyGraph();
    graph->SetRuntime(&runtime);
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::SaveState).NoVregs();
            INST(1U, Opcode::LoadAndInitClass).ref().Inputs(0U);
            INST(2U, Opcode::NewObject).ref().Inputs(1U, 0U);
            CONSTANT(3U, 0U).s32();
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 2U}, {NO_TYPE, 4U}});
            INST(6U, Opcode::ReturnVoid).v0id();
        }
    }

    EXPECT_FALSE(graph->RunPass<BytecodeOptPeepholes>());
}

TEST_F(CommonTest, CallConstructorOtherClass)
{
    RuntimeInterfaceMock runtime(1U, false);
    auto graph = CreateEmptyGraph();
    graph->SetRuntime(&runtime);
    GRAPH(graph)
    {
        // NOLINTNEXTLINE(google-build-using-namespace)
        using namespace compiler::DataType;
        PARAMETER(10U, 0U).ref();
        CONSTANT(6U, 0U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(0U, Opcode::SaveState).NoVregs();
            INST(1U, Opcode::LoadAndInitClass).ref().Inputs(0U);
            INST(2U, Opcode::NewObject).ref().Inputs(1U, 0U);
            INST(3U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallStatic).v0id().Inputs({{REFERENCE, 2U}, {NO_TYPE, 3U}});
            INST(7U, Opcode::Return).s32().Inputs(6U);
        }
    }

    EXPECT_FALSE(graph->RunPass<BytecodeOptPeepholes>());
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::bytecodeopt::test
