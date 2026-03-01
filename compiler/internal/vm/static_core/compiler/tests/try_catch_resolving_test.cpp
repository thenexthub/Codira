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
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/optimizations/try_catch_resolving.h"
#include "optimizer/optimizations/cleanup.h"

namespace ark::compiler {
class TryCatchResolvingTest : public AsmTest {  // NOLINT(fuchsia-multiple-inheritance)
public:
    TryCatchResolvingTest() : defaultCompilerNonOptimizing_(g_options.IsCompilerNonOptimizing())
    {
        g_options.SetCompilerNonOptimizing(false);
    }

    ~TryCatchResolvingTest() override
    {
        g_options.SetCompilerNonOptimizing(defaultCompilerNonOptimizing_);
    }

    NO_COPY_SEMANTIC(TryCatchResolvingTest);
    NO_MOVE_SEMANTIC(TryCatchResolvingTest);

private:
    bool defaultCompilerNonOptimizing_;
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(TryCatchResolvingTest, ThrowNewObject)
{
    auto source = R"(
.record E1 {}

.function u1 main() {
    newobj v0, E1
try_begin:
    throw v0
    ldai 2
try_end:
    return

catch_block1_begin:
    ldai 0
    return

catch_block2_begin:
    ldai 10
    return

.catchall try_begin, try_end, catch_block1_begin
.catch E1, try_begin, try_end, catch_block2_begin
}
    )";

    auto graph = GetGraph();
    ASSERT_TRUE(ParseToGraph<true>(source, "main", graph));
    graph->RunPass<TryCatchResolving>();

    auto expectedGraph = CreateGraphWithDefaultRuntime();
    GRAPH(expectedGraph)
    {
        BASIC_BLOCK(2U, 3U)
        {
            INST(5U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 5U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(8U, Opcode::SaveState).Inputs(7U).SrcVregs({0U});
            INST(9U, Opcode::Throw).Inputs(7U, 8U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

SRC_GRAPH(RemoveAllCatchHandlers, Graph *graph)
{
    GRAPH(graph)
    {
        BASIC_BLOCK(2U, -1L)
        {
            INST(7U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(8U, Opcode::LoadAndInitClass).ref().Inputs(7U);
            INST(9U, Opcode::NewObject).ref().Inputs(8U, 7U);
            INST(10U, Opcode::SaveState).Inputs(9U).SrcVregs({0U});
            INST(11U, Opcode::Throw).Inputs(9U, 10U);
        }
    }
}

TEST_F(TryCatchResolvingTest, RemoveAllCatchHandlers)
{
    auto source = R"(
    .record E0 {}
    .record E1 {}
    .record E2 {}
    .record E3 {}

    .function u1 main() {
    try_begin:
        newobj v0, E0
        throw v0
        ldai 3
    try_end:
        return

    catch_block1:
        ldai 0
        return

    catch_block2:
        ldai 1
        return

    catch_block3:
        ldai 2
        return

    .catch E1, try_begin, try_end, catch_block1
    .catch E2, try_begin, try_end, catch_block2
    .catch E3, try_begin, try_end, catch_block3
    }
    )";

    auto graph = GetGraph();
    ASSERT_TRUE(ParseToGraph<true>(source, "main", graph));
    graph->RunPass<TryCatchResolving>();

    auto expectedGraph = CreateGraphWithDefaultRuntime();
    src_graph::RemoveAllCatchHandlers::CREATE(expectedGraph);
    ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

SRC_GRAPH(EmptyTryCatches, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(5U, 2U).i32();
        CONSTANT(8U, 10U).i32();
        BASIC_BLOCK(7U, 2U) {}
        BASIC_BLOCK(2U, 4U, 9U)
        {
            INST(1U, Opcode::Try).CatchTypeIds({0U});
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(6U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(7U, Opcode::LoadType).ref().Inputs(6U).TypeId(4646690U);
        }
        BASIC_BLOCK(3U, 5U, 9U) {}
        BASIC_BLOCK(9U, 6U)
        {
            INST(2U, Opcode::CatchPhi).i32().ThrowableInsts({}).Inputs(5U);
            INST(4U, Opcode::CatchPhi).ref().ThrowableInsts({}).Inputs();
        }
        BASIC_BLOCK(5U, 8U) {}
        BASIC_BLOCK(6U, 8U) {}
        BASIC_BLOCK(8U, -1L)
        {
            INST(10U, Opcode::Phi).i32().Inputs(8U, 2U);
            INST(13U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(14U, Opcode::LoadAndInitClass).ref().Inputs(13U);
            INST(15U, Opcode::StoreStatic).s32().Inputs(14U, 10U);
            INST(16U, Opcode::ReturnVoid);
        }
    }
}

OUT_GRAPH(EmptyTryCatches, Graph *graph)
{
    GRAPH(graph)
    {
        CONSTANT(8U, 10U).i32();
        BASIC_BLOCK(8U, -1L)
        {
            INST(13U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(14U, Opcode::LoadAndInitClass).ref().Inputs(13U);
            INST(15U, Opcode::StoreStatic).s32().Inputs(14U, 8U);
            INST(16U, Opcode::ReturnVoid);
        }
    }
}

TEST_F(TryCatchResolvingTest, EmptyTryCatches)
{
    auto graph = CreateGraphWithDefaultRuntime();
    src_graph::EmptyTryCatches::CREATE(graph);
    BB(2U).SetTryBegin(true);
    BB(4U).SetTry(true);
    BB(3U).SetTryEnd(true);
    INS(1U).CastToTry()->SetTryEndBlock(&BB(3U));
    BB(9U).SetCatchBegin(true);
    BB(9U).SetCatch(true);
    BB(6U).SetCatch(true);
    GraphChecker(graph).Check();

    graph->RunPass<TryCatchResolving>();
    auto expectedGraph = CreateGraphWithDefaultRuntime();
    out_graph::EmptyTryCatches::CREATE(expectedGraph);
    GraphChecker(expectedGraph).Check();

    ASSERT_TRUE(GraphComparator().Compare(graph, expectedGraph));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
