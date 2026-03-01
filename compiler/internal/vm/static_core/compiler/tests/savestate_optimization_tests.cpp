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

#include "libarkbase/macros.h"
#include "unit_test.h"
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/savestate_optimization.h"
#include "optimizer/optimizations/cleanup.h"
#include "optimizer/ir/runtime_interface.h"

namespace ark::compiler {
class SaveStateOptimizationTest : public CommonTest {
public:
    SaveStateOptimizationTest()
        : graph_(CreateGraphStartEndBlocks()),
          defaultCompilerSafePointsRequireRegMap_(g_options.IsCompilerSafePointsRequireRegMap())
    {
    }

    ~SaveStateOptimizationTest() override
    {
        g_options.SetCompilerSafePointsRequireRegMap(defaultCompilerSafePointsRequireRegMap_);
    }

    NO_COPY_SEMANTIC(SaveStateOptimizationTest);
    NO_MOVE_SEMANTIC(SaveStateOptimizationTest);

    Graph *GetGraph()
    {
        return graph_;
    }

private:
    Graph *graph_ {nullptr};
    bool defaultCompilerSafePointsRequireRegMap_;
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(SaveStateOptimizationTest, RemoveSafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        INST(2U, Opcode::SafePoint).Inputs(0U, 1U).SrcVregs({0U, 1U});
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<SaveStateOptimization>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Not applied, have runtime calls
TEST_F(SaveStateOptimizationTest, RemoveSafePoint1)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        INST(2U, Opcode::SafePoint).Inputs(0U, 1U).SrcVregs({0U, 1U});
        BASIC_BLOCK(2U, 1U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallStatic).v0id().InputsAutoType(20U);
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<SaveStateOptimization>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// Not applied, a lot of instructions
TEST_F(SaveStateOptimizationTest, RemoveSafePoint2)
{
    uint64_t n = g_options.GetCompilerSafepointEliminationLimit();
    auto block = GetGraph()->CreateEmptyBlock();
    GetGraph()->GetStartBlock()->AddSucc(block);
    block->AddSucc(GetGraph()->GetEndBlock());
    auto param1 = GetGraph()->CreateInstParameter(0U, DataType::INT32);
    auto param2 = GetGraph()->CreateInstParameter(1U, DataType::INT32);
    GetGraph()->GetStartBlock()->AppendInst(param1);
    GetGraph()->GetStartBlock()->AppendInst(param2);
    auto sp = GetGraph()->CreateInstSafePoint();
    sp->AppendInput(param1);
    sp->AppendInput(param2);
    sp->SetVirtualRegister(0U, VirtualRegister(0U, VRegInfo::VRegType::VREG));
    sp->SetVirtualRegister(1U, VirtualRegister(1U, VRegInfo::VRegType::VREG));
    GetGraph()->GetStartBlock()->AppendInst(sp);
    ArenaVector<Inst *> insts(GetGraph()->GetLocalAllocator()->Adapter());
    insts.push_back(param1);
    insts.push_back(param2);
    for (uint64_t i = 2; i <= n + 1U; i++) {
        auto inst = GetGraph()->CreateInstAdd(DataType::INT32, INVALID_PC, insts[i - 2L], insts[i - 1L]);
        block->AppendInst(inst);
        insts.push_back(inst);
    }
    auto ret = GetGraph()->CreateInstReturn(DataType::INT32, INVALID_PC, insts[n + 1U]);
    block->AppendInst(ret);
    GraphChecker(GetGraph()).Check();
    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<SaveStateOptimization>());
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

// Applied, have CallStatic.Inlined
TEST_F(SaveStateOptimizationTest, RemoveSafePoint3)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        INST(2U, Opcode::SafePoint).Inputs(0U, 1U).SrcVregs({0U, 1U});
        BASIC_BLOCK(2U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::CallStatic).v0id().InputsAutoType(8U).Inlined();
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(7U, Opcode::ReturnInlined).Inputs(8U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GetGraph()->RunPass<SaveStateOptimization>());
    GetGraph()->RunPass<Cleanup>();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).s32();
        PARAMETER(1U, 1U).s32();
        BASIC_BLOCK(2U, 1U)
        {
            INST(8U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(5U, Opcode::CallStatic).v0id().InputsAutoType(8U).Inlined();
            INST(3U, Opcode::Add).s32().Inputs(0U, 1U);
            INST(7U, Opcode::ReturnInlined).Inputs(8U);
            INST(4U, Opcode::Return).s32().Inputs(3U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Removes numeric inputs from SS without Deoptimize and doen't remove with Deoptimize
TEST_F(SaveStateOptimizationTest, RemovNumericInputs)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).b();
        PARAMETER(1U, 2U).s32();
        PARAMETER(2U, 3U).f64();
        PARAMETER(3U, 4U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::DeoptimizeIf).Inputs(0U, 4U);
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(6U, Opcode::CallStatic).u64().InputsAutoType(0U, 1U, 2U, 3U, 5U);
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }
    Graph *graphEt = CreateEmptyGraph();
    GRAPH(graphEt)
    {
        PARAMETER(0U, 1U).b();
        PARAMETER(1U, 2U).s32();
        PARAMETER(2U, 3U).f64();
        PARAMETER(3U, 4U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::DeoptimizeIf).Inputs(0U, 4U);
            INST(5U, Opcode::SaveState).Inputs(3U).SrcVregs({3U});
            INST(6U, Opcode::CallStatic).u64().InputsAutoType(0U, 1U, 2U, 3U, 5U);
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<SaveStateOptimization>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

// Removes object inputs from SS without Deoptimize
TEST_F(SaveStateOptimizationTest, RemovObjectInputs)
{
    // 1 don't removed because they used in SS with deoptimization
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).b();
        PARAMETER(1U, 2U).ref();
        PARAMETER(2U, 3U).ref();
        PARAMETER(3U, 4U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(8U, Opcode::DeoptimizeIf).Inputs(0U, 4U);
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(6U, Opcode::CallStatic).u64().InputsAutoType(0U, 5U);
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }
    Graph *graphEt = CreateEmptyGraph();
    GRAPH(graphEt)
    {
        PARAMETER(0U, 1U).b();
        PARAMETER(1U, 2U).ref();
        PARAMETER(2U, 3U).ref();
        PARAMETER(3U, 4U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(8U, Opcode::DeoptimizeIf).Inputs(0U, 4U);
            INST(5U, Opcode::SaveState).Inputs(1U).SrcVregs({1U});
            INST(6U, Opcode::CallStatic).u64().InputsAutoType(0U, 5U);
            INST(7U, Opcode::Return).u64().Inputs(6U);
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<SaveStateOptimization>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

// Removes object inputs from SP
TEST_F(SaveStateOptimizationTest, RemoveObjectInputsSafePoint)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).b();
        PARAMETER(1U, 2U).ref();
        PARAMETER(2U, 3U).ref();
        PARAMETER(3U, 4U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SafePoint).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graphEt = CreateEmptyGraph();
    GRAPH(graphEt)
    {
        PARAMETER(0U, 1U).b();
        PARAMETER(1U, 2U).ref();
        PARAMETER(2U, 3U).ref();
        PARAMETER(3U, 4U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SafePoint).Inputs(0U).SrcVregs({0U});
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }

    ASSERT_TRUE(GetGraph()->RunPass<SaveStateOptimization>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

// Doesn't remove object inputs from SP
TEST_F(SaveStateOptimizationTest, RemoveObjectInputsSafePointRequireRegMap)
{
    g_options.SetCompilerSafePointsRequireRegMap(true);

    GRAPH(GetGraph())
    {
        PARAMETER(0U, 1U).b();
        PARAMETER(1U, 2U).ref();
        PARAMETER(2U, 3U).ref();
        PARAMETER(3U, 4U).ref();
        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SafePoint).Inputs(0U, 1U, 2U, 3U).SrcVregs({0U, 1U, 2U, 3U});
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }
    Graph *graphEt = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();

    ASSERT_FALSE(GetGraph()->RunPass<SaveStateOptimization>());
    GraphChecker(GetGraph()).Check();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graphEt));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
