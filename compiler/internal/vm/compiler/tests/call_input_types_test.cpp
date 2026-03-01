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

#include "unit_test.h"

namespace panda::compiler {
class CallInputTypesTest : public AsmTest {
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
    auto call_inst = GetCallInstruction(GetGraph());
    ASSERT_EQ(call_inst->GetInputType(0), DataType::INT32);
    ASSERT_EQ(call_inst->GetInputType(1), DataType::FLOAT64);
    ASSERT_EQ(call_inst->GetInputType(2), DataType::NO_TYPE);  // SaveState instruction
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
    auto call_inst = GetCallInstruction(GetGraph());
    ASSERT_EQ(call_inst->GetInputType(0), DataType::NO_TYPE);  // SaveState instruction
}
}  // namespace panda::compiler
