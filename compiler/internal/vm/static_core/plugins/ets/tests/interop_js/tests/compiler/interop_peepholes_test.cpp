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
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/peepholes.h"

namespace ark::compiler {
class PeepholesTest : public GraphTest {
public:
    PeepholesTest() = default;

    void SetPropertyStringBuildInitialGraph();
    Graph *SetPropertyStringBuildExpectedGraph();
};

// NOLINTBEGIN(readability-magic-numbers)

TEST_F(PeepholesTest, GetPropertyDouble)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadString).Inputs(1U).ref();
            INST(3U, Opcode::SaveState).Inputs(0U, 2U).SrcVregs({0U, 1U});
            INST(4U, Opcode::Intrinsic)
                .ref()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_JS_VALUE)
                .InputsAutoType(0U, 2U, 3U);
            INST(6U, Opcode::Intrinsic)
                .f64()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_VALUE_DOUBLE)
                .InputsAutoType(4U, 3U);
            INST(7U, Opcode::Return).f64().Inputs(6U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadString).Inputs(1U).ref();
            INST(3U, Opcode::SaveState).Inputs(0U, 2U).SrcVregs({0U, 1U});
            INST(4U, Opcode::Intrinsic)
                .f64()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_DOUBLE)
                .InputsAutoType(0U, 2U, 3U);
            INST(7U, Opcode::Return).f64().Inputs(4U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(PeepholesTest, GetPropertyDoubleNotApplied)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadString).Inputs(1U).ref();
            INST(3U, Opcode::SaveState).Inputs(0U, 2U).SrcVregs({0U, 1U});
            INST(4U, Opcode::Intrinsic)
                .ref()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_PROPERTY_JS_VALUE)
                .InputsAutoType(0U, 2U, 3U);

            INST(5U, Opcode::CallStatic).s32().InputsAutoType(0U, 3U);

            INST(6U, Opcode::Intrinsic)
                .f64()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_GET_VALUE_DOUBLE)
                .InputsAutoType(4U, 3U);
            INST(7U, Opcode::Return).f64().Inputs(6U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Peepholes>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

void PeepholesTest::SetPropertyStringBuildInitialGraph()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::Intrinsic)
                .ref()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_NEW_JS_VALUE_STRING)
                .InputsAutoType(1U, 2U);
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            // Instructions with side effects do not prevent optimization because cast *to* JSValue never fails
            INST(5U, Opcode::CallStatic).s32().InputsAutoType(0U, 1U, 4U);
            INST(6U, Opcode::SaveState).Inputs(0U, 1U, 5U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::Intrinsic)
                .v0id()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY_JS_VALUE)
                .InputsAutoType(0U, 1U, 3U, 6U);
            INST(8U, Opcode::LoadString).ref().Inputs(6U);
            INST(9U, Opcode::Intrinsic)
                .v0id()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY_JS_VALUE)
                .InputsAutoType(0U, 8U, 3U, 6U);
            INST(10U, Opcode::Return).s32().Inputs(5U);
        }
    }
}

Graph *PeepholesTest::SetPropertyStringBuildExpectedGraph()
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(3U, Opcode::Intrinsic)
                .ref()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_NEW_JS_VALUE_STRING)
                .InputsAutoType(1U, 2U);
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            // Instructions with side effects do not prevent optimization because cast *to* JSValue never fails
            INST(5U, Opcode::CallStatic).s32().InputsAutoType(0U, 1U, 4U);
            INST(6U, Opcode::SaveState).Inputs(0U, 1U, 5U).SrcVregs({0U, 1U, 2U});
            INST(7U, Opcode::Intrinsic)
                .v0id()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY_STRING)
                .InputsAutoType(0U, 1U, 1U, 6U);
            INST(8U, Opcode::LoadString).ref().Inputs(6U);
            INST(9U, Opcode::Intrinsic)
                .v0id()
                .IntrinsicId(RuntimeInterface::IntrinsicId::INTRINSIC_JS_RUNTIME_SET_PROPERTY_STRING)
                .InputsAutoType(0U, 8U, 1U, 6U);
            INST(10U, Opcode::Return).s32().Inputs(5U);
        }
    }
    return graph;
}

TEST_F(PeepholesTest, SetPropertyString)
{
    SetPropertyStringBuildInitialGraph();
    ASSERT_TRUE(GetGraph()->RunPass<Peepholes>());
    auto expected = SetPropertyStringBuildExpectedGraph();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expected));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
