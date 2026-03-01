/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "unit_test.h"

namespace ark::compiler {
class CallInputTypesTest : public AsmTest {  // NOLINT(fuchsia-multiple-inheritance)
public:
    const auto *GetCallInstruction(Graph *graph) const
    {
        for (const auto *bb : graph->GetBlocksRPO()) {
            for (const auto *inst : bb->Insts()) {
                if (inst->IsCall()) {
                    return inst;
                }
            }
        }
        UNREACHABLE();
    }
};

// Checks the build of the static call instruction
TEST_F(CallInputTypesTest, CallStatic)
{
    auto source = R"(
    .function i64 main() {
        movi.64 v0, 1
        movi.64 v1, 2
        call foo, v0, v1
        return
    }
    .function i64 foo(i32 a0, f64 a1) {
        ldai 0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto callInst = GetCallInstruction(GetGraph());
    ASSERT_EQ(callInst->GetInputType(0U), DataType::INT32);
    ASSERT_EQ(callInst->GetInputType(1U), DataType::FLOAT64);
    ASSERT_EQ(callInst->GetInputType(2U), DataType::NO_TYPE);  // SaveState instruction
}

// Checks the build of a call of a function without arguments
TEST_F(CallInputTypesTest, NoArgs)
{
    auto source = R"(
    .function i64 main() {
        movi.64 v0, 1
        movi.64 v1, 2
        call foo
        return
    }
    .function i64 foo() {
        ldai 0
        return
    }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto callInst = GetCallInstruction(GetGraph());
    ASSERT_EQ(callInst->GetInputType(0U), DataType::NO_TYPE);  // SaveState instruction
}

TEST_F(CallInputTypesTest, CallVirt)
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
            newobj v0, B
            movi.64 v1, 1
            call.virt A.foo, v0, v1
            return
        }
    )";
    ASSERT_TRUE(ParseToGraph(source, "main"));
    auto callInst = GetCallInstruction(GetGraph());
    ASSERT_EQ(callInst->GetInputType(0U), DataType::REFERENCE);
    ASSERT_EQ(callInst->GetInputType(1U), DataType::INT32);
    ASSERT_EQ(callInst->GetInputType(2U), DataType::NO_TYPE);  // SaveState instruction
}

}  // namespace ark::compiler
