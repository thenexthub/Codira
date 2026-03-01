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
#include "optimizer/ir/analysis.h"

namespace ark::compiler {
class AnalysisTest : public GraphTest {};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(AnalysisTest, FixSSInBBFromBlockToOut)
{
    GRAPH(GetGraph())
    {
        // SS will be fixed ONLY in BB2
        BASIC_BLOCK(2U, 3U)
        {
            INST(1U, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            // This SS will fix
            INST(2U, Opcode::SaveState).NoVregs();
        }
        BASIC_BLOCK(3U, -1L)
        {
            // This SS will not fix
            INST(3U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::Intrinsic)
                .ref()
                .Inputs({{DataType::REFERENCE, 1U}})
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            // This SS will not fix
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::ReturnVoid).v0id();
        }
    }

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, 3U)
        {
            INST(1U, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(2U, Opcode::SaveState).Inputs(1U).SrcVregs({VirtualRegister::BRIDGE});
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::Intrinsic)
                .ref()
                .Inputs({{DataType::REFERENCE, 1U}})
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::ReturnVoid).v0id();
        }
    }
    SaveStateBridgesBuilder ssb;
    ssb.FixSaveStatesInBB(GetGraph()->GetVectorBlocks().at(2U));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(AnalysisTest, FixSSInBBFromOutToBlock)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, 3U)
        {
            INST(1U, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            // This SS will not fix
            INST(2U, Opcode::SaveState).NoVregs();
        }
        // SS will be fixed ONLY in BB3
        BASIC_BLOCK(3U, -1L)
        {
            // This SS will fix
            INST(3U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::Intrinsic)
                .v0id()
                .Inputs({{DataType::REFERENCE, 1U}})
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            // This SS will not fix
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::ReturnVoid).v0id();
        }
    }

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, 3U)
        {
            INST(1U, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(2U, Opcode::SaveState).NoVregs();
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(1U).SrcVregs({VirtualRegister::BRIDGE});
            INST(4U, Opcode::Intrinsic)
                .v0id()
                .Inputs({{DataType::REFERENCE, 1U}})
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::ReturnVoid).v0id();
        }
    }
    SaveStateBridgesBuilder ssb;
    ssb.FixSaveStatesInBB(GetGraph()->GetVectorBlocks().at(3U));
    ASSERT(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(AnalysisTest, FixSSInBBUnrealInput)
{
    builder_->EnableGraphChecker(false);
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            // After moving Intrinsic use before declare, so fix SaveState
            INST(1U, Opcode::SaveState).Inputs(2U).SrcVregs({10U});
            INST(2U, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(3U, Opcode::ReturnVoid).v0id();
        }
    }

    builder_->EnableGraphChecker(true);
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).NoVregs();
            INST(2U, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(3U, Opcode::ReturnVoid).v0id();
        }
    }
    SaveStateBridgesBuilder ssb;
    ssb.FixSaveStatesInBB(GetGraph()->GetVectorBlocks().at(2U));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(AnalysisTest, FixSSInBBInside)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            // This SS will fix
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::Intrinsic)
                .ref()
                .Inputs({{DataType::REFERENCE, 1U}})
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            // This SS will not fix
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(2U, Opcode::SaveState).Inputs(1U).SrcVregs({VirtualRegister::BRIDGE});
            INST(3U, Opcode::Intrinsic)
                .ref()
                .Inputs({{DataType::REFERENCE, 1U}})
                .ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    SaveStateBridgesBuilder ssb;
    ssb.FixSaveStatesInBB(GetGraph()->GetVectorBlocks().at(2U));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(AnalysisTest, FixBridgesInOptimizedGraph)
{
    GRAPH(GetGraph())
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::ReturnVoid).v0id();
        }
    }

    auto bridgeExample = CreateEmptyGraph();
    GRAPH(bridgeExample)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Intrinsic).ref().ClearFlag(compiler::inst_flags::REQUIRE_STATE);
            INST(2U, Opcode::SaveState).Inputs(1U).SrcVregs({VirtualRegister::BRIDGE});
            INST(3U, Opcode::ReturnVoid).v0id();
        }
    }

    Graph *graphBc = CreateEmptyBytecodeGraph();
    graphBc->SetRuntime(&runtime_);
    GRAPH(graphBc)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).ref();
            INST(3U, Opcode::ReturnVoid).v0id();
        }
    }

    Graph *example = CreateEmptyGraph();
    GRAPH(example)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).ref();
            INST(2U, Opcode::ReturnVoid).v0id();
        }
    }

    SaveStateBridgesBuilder ssb;

    const BasicBlock *bb = GetGraph()->GetVectorBlocks().at(2U);
    ssb.SearchAndCreateMissingObjInSaveState(GetGraph(), bb->GetFirstInst(), bb->GetLastInst());

    const BasicBlock *bbBc = graphBc->GetVectorBlocks().at(2U);
    ssb.SearchAndCreateMissingObjInSaveState(graphBc, bbBc->GetFirstInst(), bbBc->GetLastInst());

    ASSERT_TRUE(GraphComparator().Compare(graphBc, example));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), bridgeExample));
}

SRC_GRAPH(FixBridgesInGraphWithPhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, 3U) {}

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(10U, Opcode::Phi).Inputs({{2U, 0U}, {3U, 12U}}).ref();
            INST(11U, Opcode::SaveState).Inputs(10U).SrcVregs({1U});
            INST(12U, Opcode::Call)
                .Inputs({{DataType::REFERENCE, 10U}, {DataType::NO_TYPE, 11U}})
                .SetFlag(compiler::inst_flags::REQUIRE_STATE)
                .ref();
            INST(13U, Opcode::Compare).Inputs(1U, 12U).CC(ConditionCode::CC_NE).b();
            INST(14U, Opcode::IfImm).Inputs(13U).Imm(0U).CC(ConditionCode::CC_NE);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(20U, Opcode::SaveState).Inputs(10U).SrcVregs({2U});
            INST(21U, Opcode::Call)
                .Inputs({{DataType::REFERENCE, 10U}, {DataType::NO_TYPE, 20U}})
                .SetFlag(compiler::inst_flags::REQUIRE_STATE)
                .ref();
            INST(30U, Opcode::Return).Inputs(21U).ref();
        }
    }
}

OUT_GRAPH(FixBridgesInGraphWithPhi, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, 3U) {}

        BASIC_BLOCK(3U, 4U, 3U)
        {
            INST(10U, Opcode::Phi).Inputs({{2U, 0U}, {3U, 12U}}).ref();
            INST(11U, Opcode::SaveState).Inputs(10U, 1U).SrcVregs({1U, VirtualRegister::BRIDGE});
            INST(12U, Opcode::Call)
                .Inputs({{DataType::REFERENCE, 10U}, {DataType::NO_TYPE, 11U}})
                .SetFlag(compiler::inst_flags::REQUIRE_STATE)
                .ref();
            INST(13U, Opcode::Compare).Inputs(1U, 12U).CC(ConditionCode::CC_NE).b();
            INST(14U, Opcode::IfImm).Inputs(13U).Imm(0U).CC(ConditionCode::CC_NE);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(20U, Opcode::SaveState).Inputs(10U).SrcVregs({2U});
            INST(21U, Opcode::Call)
                .Inputs({{DataType::REFERENCE, 10U}, {DataType::NO_TYPE, 20U}})
                .SetFlag(compiler::inst_flags::REQUIRE_STATE)
                .ref();
            INST(30U, Opcode::Return).Inputs(21U).ref();
        }
    }
}

TEST_F(AnalysisTest, FixBridgesInGraphWithPhi)
{
    src_graph::FixBridgesInGraphWithPhi::CREATE(GetGraph());
    auto *param0 = GetGraph()->GetStartBlock()->GetFirstInst();
    auto *param1 = param0->GetNext();

    SaveStateBridgesBuilder ssb;
    ssb.FixInstUsageInSS(GetGraph(), param0);
    ssb.FixInstUsageInSS(GetGraph(), param1);

    Graph *graph = CreateEmptyGraph();
    out_graph::FixBridgesInGraphWithPhi::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
// NOLINTEND(readability-magic-numbers)
}  // namespace ark::compiler
