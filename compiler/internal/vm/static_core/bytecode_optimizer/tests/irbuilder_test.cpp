/**
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

namespace ark::bytecodeopt::test {

TEST_F(IrBuilderTest, CallVirtShort)
{
    auto source = R"(
        .record A {}
        .record B <extends=A> {}

        .function i32 A.foo(A a0, i32 a1) <noimpl>
        .function i32 B.foo(B a0, i32 a1) {
            ldai 0
            return
        }

        .function u1 main() {
            movi v1, 1
            newobj v0, B
            call.virt.short A.foo, v0, v1
            return
        }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);

        BASIC_BLOCK(2U, -1L)
        {
            using namespace compiler::DataType;  // NOLINT(google-build-using-namespace)
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 5U}, {INT32, 0U}, {NO_TYPE, 4U}});
            INST(7U, Opcode::Return).b().Inputs(6U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, CallVirt)
{
    auto source = R"(
        .record A {}
        .record B <extends=A> {}

        .function i32 A.foo(A a0, i32 a1, i32 a2) <noimpl>
        .function i32 B.foo(B a0, i32 a1, i32 a2) {
            ldai 0
            return
        }

        .function u1 main() {
            movi v1, 1
            movi v2, 2
            newobj v0, B
            call.virt A.foo, v0, v1, v2
            return
        }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 2U);

        BASIC_BLOCK(2U, -1L)
        {
            using namespace compiler::DataType;  // NOLINT(google-build-using-namespace)
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::NullCheck).ref().Inputs(4U, 5U);
            INST(7U, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 6U}, {INT32, 0U}, {INT32, 1U}, {NO_TYPE, 5U}});
            INST(8U, Opcode::Return).b().Inputs(7U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(IrBuilderTest, CallVirtRange)
{
    auto source = R"(
        .record A {}
        .record B <extends=A> {}

        .function i32 A.foo(A a0, i32 a1, i32 a2) <noimpl>
        .function i32 B.foo(B a0, i32 a1, i32 a2) {
            ldai 0
            return
        }

        .function u1 main() {
            movi v1, 1
            movi v2, 2
            newobj v0, B
            call.virt.range A.foo, v0
            return
        }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 2U);

        BASIC_BLOCK(2U, -1L)
        {
            using namespace compiler::DataType;  // NOLINT(google-build-using-namespace)
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::NullCheck).ref().Inputs(4U, 5U);
            INST(7U, Opcode::CallVirtual).s32().Inputs({{REFERENCE, 6U}, {INT32, 0U}, {INT32, 1U}, {NO_TYPE, 5U}});
            INST(8U, Opcode::Return).b().Inputs(7U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

}  // namespace ark::bytecodeopt::test
