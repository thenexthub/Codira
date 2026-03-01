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

#include "optimizer/ir/datatype.h"
#include "optimizer/ir/inst.h"
#include "unit_test.h"

#include "optimizer/optimizations/lower_boxed_boolean.h"
namespace ark::compiler {
class LowerBoxedBooleanTest : public AsmTest {
public:
    bool IsFieldBooleanFalse(FieldPtr field) const override
    {
        return GetFieldName(field) == "FALSE";
    }
    bool IsFieldBooleanTrue(FieldPtr field) const override
    {
        return GetFieldName(field) == "TRUE";
    }

    const Inst *GetCompareInst(Graph *graph, ConditionCode cc) const
    {
        for (const auto *bb : graph->GetBlocksRPO()) {
            for (const auto *inst : bb->Insts()) {
                if (inst->GetOpcode() == Opcode::Compare && inst->CastToCompare()->GetCc() == cc) {
                    return inst;
                }
            }
        }
        UNREACHABLE();
    }
};

// This test verifies that after the LowerBoxedBoolean pass, the `Compare NE ref, NullPtr` instruction is replaced with
// the constant 1.
TEST_F(LowerBoxedBooleanTest, CheckCompareNeSubstituion)
{
    auto source = R"(
    .record ref
    .record std.core.Boolean {
        ref FALSE <static>
        ref TRUE <static>
    }
    .function u1 foo(f64 a0) {
        mov.null v0
        fmovi.64 v1, 0x0
        lda.64 v1
        fcmpg.64 a0
        jgez jump_label_1
        ldstatic std.core.Boolean.TRUE
        sta.obj v0
        jmp jump_label_0
jump_label_1:
        ldstatic std.core.Boolean.FALSE
        sta.obj v0
jump_label_0:
        lda.obj v0
        jnez.obj jump_label_2
        ldai 0x0
        return
jump_label_2:
        ldai 0x1
        return
    }
    )";
    ParseToGraph(source, "foo");
    auto *graph = GetGraph();
    EXPECT_TRUE(graph->RunPass<LowerBoxedBoolean>());
    // After the pass, we expect that the Compare instruction is no longer used. In IfImm, we expect a constant inst.
    auto *compareInst = GetCompareInst(graph, ConditionCode::CC_NE);
    ASSERT_TRUE(compareInst->GetUsers().Empty());
    auto *ifImmInst = compareInst->GetNext();
    auto *constInst = ifImmInst->GetInput(0U).GetInst();
    ASSERT_EQ(constInst->GetType(), DataType::INT64);
    ASSERT_EQ(constInst->CastToConstant()->GetInt64Value(), 1);
}

// This test verifies that after the LowerBoxedBoolean pass, the `Compare EQ ref, NullPtr` instruction is replaced with
// the constant 0.
TEST_F(LowerBoxedBooleanTest, CheckCompareEqSubstituion)
{
    auto source = R"(
    .record ref
    .record std.core.Boolean {
        ref FALSE <static>
        ref TRUE <static>
    }
    .function u1 foo(f64 a0) {
        mov.null v0
        fmovi.64 v1, 0x0
        lda.64 v1
        fcmpg.64 a0
        jgez jump_label_1
        ldstatic std.core.Boolean.TRUE
        sta.obj v0
        jmp jump_label_0
jump_label_1:
        ldstatic std.core.Boolean.FALSE
        sta.obj v0
jump_label_0:
        lda.obj v0
        jeqz.obj jump_label_2
        ldai 0x0
        return
jump_label_2:
        ldai 0x1
        return
    }
    )";
    ParseToGraph(source, "foo");
    auto *graph = GetGraph();
    EXPECT_TRUE(graph->RunPass<LowerBoxedBoolean>());
    // After the pass, we expect that the Compare instruction is no longer used. In IfImm, we expect a constant inst.
    auto *compareInst = GetCompareInst(graph, ConditionCode::CC_EQ);
    ASSERT_TRUE(compareInst->GetUsers().Empty());
    auto *ifImmInst = compareInst->GetNext();
    auto *constInst = ifImmInst->GetInput(0U).GetInst();
    ASSERT_EQ(constInst->GetType(), DataType::INT64);
    ASSERT_EQ(constInst->CastToConstant()->GetInt64Value(), 0);
}

}  // namespace ark::compiler
