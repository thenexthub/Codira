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
#include "optimizer/optimizations/escape.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/analysis/liveness_analyzer.h"

namespace ark::compiler {

inline const RuntimeInterface::FieldPtr OBJ_FIELD = reinterpret_cast<void *>(0xDEADBEEFU);
inline const RuntimeInterface::FieldPtr INT64_FIELD = reinterpret_cast<void *>(0xDEADFEEDU);
inline const RuntimeInterface::FieldPtr UINT64_FIELD = reinterpret_cast<void *>(0xDEAD64BU);
inline const RuntimeInterface::FieldPtr INT32_FIELD = reinterpret_cast<void *>(0xDEAD5320U);
inline const RuntimeInterface::FieldPtr UINT8_FIELD = reinterpret_cast<void *>(0xB00B00UL);
inline const RuntimeInterface::FieldPtr INT8_FIELD = reinterpret_cast<void *>(0xB00B01UL);
inline const RuntimeInterface::ClassPtr INT32_CLASS = reinterpret_cast<void *>(0xDEAD5320U);
inline const RuntimeInterface::IdType CUSTOM_CLASS_ID = 0xDEAD1337U;
inline const RuntimeInterface::ClassPtr CUSTOM_CLASS = reinterpret_cast<void *>(0x00133700U);

class EscapeAnalysisTest : public GraphTest {
public:
    EscapeAnalysisTest()
    {
        RegisterFieldType(OBJ_FIELD, DataType::REFERENCE);
        RegisterFieldType(INT64_FIELD, DataType::INT64);
        RegisterFieldType(UINT64_FIELD, DataType::UINT64);
        RegisterFieldType(INT32_FIELD, DataType::INT32);
        RegisterFieldType(UINT8_FIELD, DataType::UINT8);
        RegisterFieldType(INT8_FIELD, DataType::INT8);

        RegisterArrayComponentType(INT32_CLASS, DataType::INT32);

        RegisterClass(CUSTOM_CLASS_ID, CUSTOM_CLASS);
    }

    bool Run() const
    {
        EscapeAnalysis ea {GetGraph()};
        ea.RelaxClassRestrictions();
        auto res = ea.Run();
        if (res) {
            GetGraph()->RunPass<Cleanup>();
        }
        return res;
    }
};

TEST_F(EscapeAnalysisTest, NewEmptyUnusedObject)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState);
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState);
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(4U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, NewEmptyUnusedArray)
{
    GRAPH(GetGraph())
    {
        CONSTANT(5U, 5U);
        // NOLINTBEGIN(readability-magic-numbers)
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState);
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U).Class(INT32_CLASS);
            INST(3U, Opcode::NewArray).ref().Inputs(2U, 5U, 1U);
            INST(4U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState);
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U).Class(INT32_CLASS);
            INST(4U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(NewUnusedObjectAndControlFlow, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 42U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U) {}

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Phi).Inputs(0U, 1U).s32();
            INST(7U, Opcode::Return).Inputs(6U).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(NewUnusedObjectAndControlFlow, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 42U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U) {}

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Phi).Inputs(0U, 1U).s32();
            INST(7U, Opcode::Return).Inputs(6U).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, NewUnusedObjectAndControlFlow)
{
    src_graph::NewUnusedObjectAndControlFlow::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::NewUnusedObjectAndControlFlow::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(NewUnusedArrayAndControlFlow, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 42U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U).Class(INT32_CLASS);
            INST(4U, Opcode::NewArray).ref().Inputs(3U, 1U, 2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U) {}

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Phi).Inputs(0U, 1U).s32();
            INST(7U, Opcode::Return).Inputs(6U).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(NewUnusedArrayAndControlFlow, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 42U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U).Class(INT32_CLASS);
            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U) {}

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Phi).Inputs(0U, 1U).s32();
            INST(7U, Opcode::Return).Inputs(6U).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, NewUnusedArrayAndControlFlow)
{
    src_graph::NewUnusedArrayAndControlFlow::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::NewUnusedArrayAndControlFlow::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(NewUnusedObjectsFromDifferentBranches, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 42U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);  // virt
            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(6U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(7U, Opcode::NewObject).ref().Inputs(3U, 6U);  // virt
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::Phi).Inputs(0U, 1U).s32();
            INST(9U, Opcode::Return).Inputs(8U).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(NewUnusedObjectsFromDifferentBranches, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 42U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U) {}

        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::Phi).Inputs(0U, 1U).s32();
            INST(9U, Opcode::Return).Inputs(8U).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, NewUnusedObjectsFromDifferentBranches)
{
    src_graph::NewUnusedObjectsFromDifferentBranches::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::NewUnusedObjectsFromDifferentBranches::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(NewUnusedArrayFromDifferentBranches, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 42U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U).Class(INT32_CLASS);
            INST(4U, Opcode::NewArray).ref().Inputs(3U, 1U, 2U);  // virt
            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(6U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(7U, Opcode::NewArray).ref().Inputs(3U, 1U, 6U);  // virt
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::Phi).Inputs(0U, 1U).s32();
            INST(9U, Opcode::Return).Inputs(8U).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(NewUnusedArrayFromDifferentBranches, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 42U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U).Class(INT32_CLASS);
            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U) {}

        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::Phi).Inputs(0U, 1U).s32();
            INST(9U, Opcode::Return).Inputs(8U).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, NewUnusedArrayFromDifferentBranches)
{
    src_graph::NewUnusedArrayFromDifferentBranches::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::NewUnusedArrayFromDifferentBranches::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(MergeDifferentMaterializedStatesForSameObject, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);

        BASIC_BLOCK(2U, 3U, 7U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);  // virt
            INST(5U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_NE).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(6U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(1U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(4U, 6U)
        {
            INST(7U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(4U, 7U);
        }

        BASIC_BLOCK(5U, 6U)
        {
            INST(9U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(10U, Opcode::CallStatic).v0id().InputsAutoType(4U, 9U);
        }

        BASIC_BLOCK(6U, 7U)
        {
            INST(11U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(12U, Opcode::NullCheck).ref().Inputs(4U, 11U);
            INST(13U, Opcode::LoadObject).u64().Inputs(12U);
        }

        BASIC_BLOCK(7U, -1L)
        {
            INST(14U, Opcode::Phi).Inputs(1U, 13U).u64();
            INST(15U, Opcode::Return).Inputs(14U).u64();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(MergeDifferentMaterializedStatesForSameObject, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);

        BASIC_BLOCK(2U, 3U, 7U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_NE).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(16U, Opcode::SaveState);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 16U);
            INST(6U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(1U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(4U, 6U)
        {
            INST(7U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(4U, 7U);
        }

        BASIC_BLOCK(5U, 6U)
        {
            INST(9U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(10U, Opcode::CallStatic).v0id().InputsAutoType(4U, 9U);
        }

        BASIC_BLOCK(6U, 7U)
        {
            INST(11U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(12U, Opcode::NullCheck).ref().Inputs(4U, 11U);
            INST(13U, Opcode::LoadObject).u64().Inputs(12U);
        }

        BASIC_BLOCK(7U, -1L)
        {
            INST(14U, Opcode::Phi).Inputs(1U, 13U).u64();
            INST(15U, Opcode::Return).Inputs(14U).u64();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, MergeDifferentMaterializedStatesForSameObject)
{
    src_graph::MergeDifferentMaterializedStatesForSameObject::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::MergeDifferentMaterializedStatesForSameObject::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(MergeDifferentMaterializedStatesForSameArray, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(42U, 42U);

        BASIC_BLOCK(2U, 3U, 7U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U).Class(INT32_CLASS);
            INST(4U, Opcode::NewArray).ref().Inputs(3U, 42U, 2U);  // virt
            INST(5U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_NE).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(6U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(1U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(4U, 6U)
        {
            INST(7U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(4U, 7U);
        }

        BASIC_BLOCK(5U, 6U)
        {
            INST(9U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(10U, Opcode::CallStatic).v0id().InputsAutoType(4U, 9U);
        }

        BASIC_BLOCK(6U, 7U)
        {
            INST(11U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(12U, Opcode::NullCheck).ref().Inputs(4U, 11U);
            INST(13U, Opcode::LoadArray).u64().Inputs(12U, 1U);
        }

        BASIC_BLOCK(7U, -1L)
        {
            INST(14U, Opcode::Phi).Inputs(1U, 13U).u64();
            INST(15U, Opcode::Return).Inputs(14U).u64();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(MergeDifferentMaterializedStatesForSameArray, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 0U);
        CONSTANT(42U, 42U);

        BASIC_BLOCK(2U, 3U, 7U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U).Class(INT32_CLASS);
            INST(5U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_NE).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(16U, Opcode::SaveState);
            INST(4U, Opcode::NewArray).ref().Inputs(3U, 42U, 16U);
            INST(6U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(1U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(4U, 6U)
        {
            INST(7U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(4U, 7U);
        }

        BASIC_BLOCK(5U, 6U)
        {
            INST(9U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(10U, Opcode::CallStatic).v0id().InputsAutoType(4U, 9U);
        }

        BASIC_BLOCK(6U, 7U)
        {
            INST(11U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(12U, Opcode::NullCheck).ref().Inputs(4U, 11U);
            INST(13U, Opcode::LoadArray).u64().Inputs(12U, 1U);
        }

        BASIC_BLOCK(7U, -1L)
        {
            INST(14U, Opcode::Phi).Inputs(1U, 13U).u64();
            INST(15U, Opcode::Return).Inputs(14U).u64();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, MergeDifferentMaterializedStatesForSameArray)
{
    src_graph::MergeDifferentMaterializedStatesForSameArray::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::MergeDifferentMaterializedStatesForSameArray::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, ObjectEscapement)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState);
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

TEST_F(EscapeAnalysisTest, ArrayEscapement)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(0U, 5U);
        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState);
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U).Class(INT32_CLASS);
            INST(3U, Opcode::NewArray).ref().Inputs(2U, 0U, 1U);
            INST(4U, Opcode::Return).ref().Inputs(3U);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

SRC_GRAPH(ObjectPartialEscapement, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 42U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(10U, Opcode::NullCheck).ref().Inputs(4U, 2U);
            INST(11U, Opcode::StoreObject).s32().Inputs(10U, 1U).ObjField(INT32_FIELD);
            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(8U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(9U, Opcode::CallStatic).v0id().InputsAutoType(4U, 8U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Phi).Inputs(0U, 1U).s32();
            INST(7U, Opcode::Return).Inputs(6U).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(ObjectPartialEscapement, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 42U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(10U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(11U, Opcode::NewObject).ref().Inputs(3U, 10U);
            INST(12U, Opcode::StoreObject).s32().Inputs(11U, 1U).ObjField(INT32_FIELD);
            INST(8U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 11U);
            INST(9U, Opcode::CallStatic).v0id().InputsAutoType(11U, 8U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Phi).Inputs(0U, 1U).s32();
            INST(7U, Opcode::Return).Inputs(6U).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, ObjectPartialEscapement)
{
    src_graph::ObjectPartialEscapement::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::ObjectPartialEscapement::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, RepeatedLoadStores)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();

        BASIC_BLOCK(2U, -1)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(10U, Opcode::NullCheck).ref().Inputs(4U, 2U);
            INST(11U, Opcode::StoreObject).ref().Inputs(10U, 5U).ObjField(OBJ_FIELD);
            INST(12U, Opcode::LoadObject).ref().Inputs(10U).ObjField(OBJ_FIELD);
            INST(13U, Opcode::StoreObject).ref().Inputs(10U, 12U).ObjField(OBJ_FIELD);
            INST(14U, Opcode::LoadObject).ref().Inputs(10U).ObjField(OBJ_FIELD);
            INST(15U, Opcode::SaveState).SrcVregs({0U, 2U}).Inputs(0U, 14U);
            INST(16U, Opcode::Deoptimize).Inputs(15U);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();

        BASIC_BLOCK(2U, -1)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(17U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(5U, Opcode::NewObject).ref().Inputs(3U, 17U);
            INST(15U, Opcode::SaveState).SrcVregs({0U, 2U}).Inputs(0U, 5U);
            INST(16U, Opcode::Deoptimize).Inputs(15U);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(ArrayPartialEscapement, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 42U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U).Class(INT32_CLASS);
            INST(4U, Opcode::NewArray).ref().Inputs(3U, 0U, 2U);
            INST(10U, Opcode::NullCheck).ref().Inputs(4U, 2U);
            INST(11U, Opcode::StoreArray).s32().Inputs(10U, 1U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(8U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(9U, Opcode::CallStatic).v0id().InputsAutoType(4U, 8U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Phi).Inputs(0U, 1U).s32();
            INST(7U, Opcode::Return).Inputs(6U).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(ArrayPartialEscapement, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 42U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U).Class(INT32_CLASS);

            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(8U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(4U, Opcode::NewArray).ref().Inputs(3U, 0U, 8U);
            INST(11U, Opcode::StoreArray).s32().Inputs(4U, 1U, 1U);
            INST(12U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(9U, Opcode::CallStatic).v0id().InputsAutoType(4U, 12U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Phi).Inputs(0U, 1U).s32();
            INST(7U, Opcode::Return).Inputs(6U).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, ArrayPartialEscapement)
{
    src_graph::ArrayPartialEscapement::CREATE(GetGraph());

    ASSERT_TRUE(Run());
    auto graph = CreateEmptyGraph();
    out_graph::ArrayPartialEscapement::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, ArrayPartialEscapementNonConstantIndex)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 42U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U).Class(INT32_CLASS);
            INST(4U, Opcode::NewArray).ref().Inputs(3U, 1U, 2U);
            INST(10U, Opcode::NullCheck).ref().Inputs(4U, 2U);
            INST(11U, Opcode::StoreArray).s32().Inputs(10U, 0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(8U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(9U, Opcode::CallStatic).v0id().InputsAutoType(4U, 8U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Phi).Inputs(0U, 1U).s32();
            INST(7U, Opcode::Return).Inputs(6U).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

TEST_F(EscapeAnalysisTest, ObjectEscapementThroughPhi)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 42U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(4U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U);
            INST(6U, Opcode::NewObject).ref().Inputs(5U, 4U);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(7U, Opcode::SaveState).SrcVregs({0U}).Inputs(0U);
            INST(8U, Opcode::LoadAndInitClass).ref().Inputs(7U);
            INST(9U, Opcode::NewObject).ref().Inputs(8U, 7U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(10U, Opcode::Phi).Inputs(6U, 9U).ref();
            INST(11U, Opcode::Return).Inputs(10U).ref();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

TEST_F(EscapeAnalysisTest, ReturnField)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(6U, Opcode::StoreObject).u64().Inputs(5U, 0U).ObjField(UINT64_FIELD);
            INST(7U, Opcode::LoadObject).u64().Inputs(5U).ObjField(UINT64_FIELD);
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(8U, Opcode::Return).u64().Inputs(0U);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(ReturnSumOf2Elements, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(3U, 0U).u64();
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).Inputs(3U).SrcVregs({3U});
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U).Class(INT32_CLASS);
            INST(6U, Opcode::NewArray).ref().Inputs(5U, 2U, 4U);

            INST(7U, Opcode::SaveState).Inputs(3U, 6U).SrcVregs({0U, 1U});
            INST(8U, Opcode::NullCheck).ref().Inputs(6U, 7U);
            INST(9U, Opcode::BoundsCheck).s32().Inputs(0U, 2U, 7U);
            INST(10U, Opcode::StoreArray).u64().Inputs(8U, 9U, 3U);

            INST(11U, Opcode::SaveState).Inputs(3U, 6U).SrcVregs({0U, 1U});
            INST(12U, Opcode::NullCheck).ref().Inputs(6U, 11U);
            INST(13U, Opcode::BoundsCheck).s32().Inputs(1U, 2U, 11U);
            INST(14U, Opcode::StoreArray).u64().Inputs(12U, 13U, 3U);

            INST(15U, Opcode::SaveState).Inputs(3U, 6U).SrcVregs({0U, 1U});
            INST(16U, Opcode::NullCheck).ref().Inputs(6U, 15U);
            INST(17U, Opcode::BoundsCheck).s32().Inputs(0U, 2U, 15U);
            INST(18U, Opcode::LoadArray).u64().Inputs(16U, 17U);

            INST(19U, Opcode::SaveState).Inputs(3U, 6U).SrcVregs({0U, 1U});
            INST(20U, Opcode::NullCheck).ref().Inputs(6U, 19U);
            INST(21U, Opcode::BoundsCheck).s32().Inputs(1U, 2U, 19U);
            INST(22U, Opcode::LoadArray).u64().Inputs(20U, 21U);

            INST(23U, Opcode::Add).u64().Inputs(18U, 22U);
            INST(24U, Opcode::Return).u64().Inputs(23U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(ReturnSumOf2Elements, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(3U, 0U).u64();
        CONSTANT(0U, 0U);
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(4U, Opcode::SaveState).Inputs(3U).SrcVregs({3U});
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U);

            INST(7U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(9U, Opcode::BoundsCheck).s32().Inputs(0U, 2U, 7U);

            INST(11U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(13U, Opcode::BoundsCheck).s32().Inputs(1U, 2U, 11U);

            INST(15U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(17U, Opcode::BoundsCheck).s32().Inputs(0U, 2U, 15U);

            INST(19U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(21U, Opcode::BoundsCheck).s32().Inputs(1U, 2U, 19U);

            INST(23U, Opcode::Add).u64().Inputs(3U, 3U);
            INST(24U, Opcode::Return).u64().Inputs(23U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, ReturnSumOf2Elements)
{
    src_graph::ReturnSumOf2Elements::CREATE(GetGraph());

    ASSERT_TRUE(Run());
    auto graph = CreateEmptyGraph();
    out_graph::ReturnSumOf2Elements::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, EscapeThroughAlias)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);  // should escape
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(6U, Opcode::NewObject).ref().Inputs(2U, 4U);  // should escape

            INST(7U, Opcode::SaveState).Inputs(0U, 3U, 6U).SrcVregs({0U, 1U, 2U});

            INST(8U, Opcode::NullCheck).ref().Inputs(3U, 7U);  // should escape
            INST(9U, Opcode::NullCheck).ref().Inputs(6U, 7U);  // should escape

            INST(10U, Opcode::CallStatic).v0id().InputsAutoType(3U, 9U, 7U);
            INST(11U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

TEST_F(EscapeAnalysisTest, VirtualObjectsChain)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);  // should not escape
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(6U, Opcode::NewObject).ref().Inputs(2U, 4U);  // should not escape

            INST(7U, Opcode::SaveState).Inputs(0U, 3U, 6U).SrcVregs({0U, 1U, 2U});

            INST(8U, Opcode::NullCheck).ref().Inputs(3U, 7U);

            INST(9U, Opcode::StoreObject).ref().Inputs(8U, 6U).ObjField(OBJ_FIELD);
            INST(11U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(11U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, VirtualArraysChain)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(42U, 42U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U).Class(INT32_CLASS);
            INST(3U, Opcode::NewArray).ref().Inputs(2U, 42U, 1U);  // should not escape
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(6U, Opcode::NewArray).ref().Inputs(2U, 42U, 4U);  // should not escape

            INST(7U, Opcode::SaveState).Inputs(0U, 3U, 6U).SrcVregs({0U, 1U, 2U});

            INST(8U, Opcode::NullCheck).ref().Inputs(3U, 7U);

            INST(9U, Opcode::StoreArray).ref().Inputs(8U, 42U, 6U);
            INST(11U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U).Class(INT32_CLASS);
            INST(11U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, VirtualArraysChainMaterialize)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(42U, 42U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U).Class(INT32_CLASS);
            INST(3U, Opcode::NewArray).ref().Inputs(2U, 42U, 1U);  // should not escape
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(6U, Opcode::NewArray).ref().Inputs(2U, 42U, 4U);  // should not escape

            INST(7U, Opcode::SaveState).Inputs(0U, 3U, 6U).SrcVregs({0U, 1U, 2U});

            INST(8U, Opcode::NullCheck).ref().Inputs(3U, 7U);

            INST(9U, Opcode::StoreArray).ref().Inputs(8U, 0U, 6U);
            INST(11U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

SRC_GRAPH(VirtualObjectsChainDereference, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);  // should not escape
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(6U, Opcode::NewObject).ref().Inputs(2U, 4U);  // should not escape

            INST(7U, Opcode::SaveState).Inputs(0U, 3U, 6U).SrcVregs({0U, 1U, 2U});

            INST(8U, Opcode::NullCheck).ref().Inputs(3U, 7U);

            INST(9U, Opcode::StoreObject).ref().Inputs(8U, 6U).ObjField(OBJ_FIELD);
            INST(10U, Opcode::LoadObject).ref().Inputs(8U).ObjField(OBJ_FIELD);

            INST(11U, Opcode::SaveState).Inputs(0U, 3U, 10U).SrcVregs({0U, 1U, 2U});
            INST(12U, Opcode::NullCheck).ref().Inputs(10U, 11U);
            INST(13U, Opcode::LoadObject).u64().Inputs(12U).ObjField(UINT64_FIELD);
            INST(14U, Opcode::Return).u64().Inputs(13U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(VirtualObjectsChainDereference, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(15U, 0U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(16U, Opcode::Cast).u64().SrcType(DataType::INT64).Inputs(15U);
            // That cast of constant will be optimized in constant folding in Peepholes
            INST(14U, Opcode::Return).u64().Inputs(16U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, VirtualObjectsChainDereference)
{
    src_graph::VirtualObjectsChainDereference::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::VirtualObjectsChainDereference::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, VirtualLoadEscape)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);  // should not escape
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(6U, Opcode::NewObject).ref().Inputs(2U, 4U);  // escape through return after load

            INST(7U, Opcode::SaveState).Inputs(0U, 3U, 6U).SrcVregs({0U, 1U, 2U});

            INST(8U, Opcode::NullCheck).ref().Inputs(3U, 7U);

            INST(9U, Opcode::StoreObject).ref().Inputs(8U, 6U).ObjField(OBJ_FIELD);
            INST(10U, Opcode::LoadObject).ref().Inputs(8U).ObjField(OBJ_FIELD);
            INST(14U, Opcode::Return).ref().Inputs(10U);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(4U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::NewObject).ref().Inputs(2U, 4U);
            INST(14U, Opcode::Return).ref().Inputs(6U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(EscapeThroughAliasOnCfgMerge, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        PARAMETER(9U, 1U).ref();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U, 9U).SrcVregs({0U, 1U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::SaveState).Inputs(0U, 9U, 3U).SrcVregs({0U, 1U, 2U});
            INST(6U, Opcode::NewObject).ref().Inputs(2U, 4U);

            INST(7U, Opcode::SaveState).Inputs(0U, 9U, 3U, 6U).SrcVregs({0U, 1U, 2U, 3U});
            INST(8U, Opcode::NullCheck).ref().Inputs(3U, 7U);
            INST(100U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_EQ).Inputs(0U).Imm(0U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(10U, Opcode::StoreObject).ref().Inputs(8U, 9U).ObjField(OBJ_FIELD);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(11U, Opcode::SaveState).Inputs(0U, 3U, 6U).SrcVregs({0U, 1U, 2U});
            INST(12U, Opcode::NullCheck).ref().Inputs(6U, 11U);
            INST(13U, Opcode::StoreObject).ref().Inputs(8U, 12U).ObjField(OBJ_FIELD);
        }

        BASIC_BLOCK(5U, 1U)
        {
            INST(16U, Opcode::SaveState).Inputs(3U).SrcVregs({1U});
            INST(14U, Opcode::NullCheck).ref().Inputs(3U, 16U);
            INST(15U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(EscapeThroughAliasOnCfgMerge, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        PARAMETER(9U, 1U).ref();

        BASIC_BLOCK(2U, 5U, 4U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U, 9U).SrcVregs({0U, 1U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(100U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_EQ).Inputs(0U).Imm(0U);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(13U, Opcode::SaveState);
            INST(14U, Opcode::NewObject).ref().Inputs(2U, 13U);
        }

        BASIC_BLOCK(5U, 1U)
        {
            INST(15U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, EscapeThroughAliasOnCfgMerge)
{
    src_graph::EscapeThroughAliasOnCfgMerge::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::EscapeThroughAliasOnCfgMerge::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, EscapeInsideLoop)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
        }

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(5U, Opcode::Phi).u64().Inputs(0U, 7U);
            INST(6U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_NE).Inputs(5U);
        }

        BASIC_BLOCK(4U, 3U)
        {
            INST(7U, Opcode::Sub).u64().Inputs(5U, 1U);
            INST(8U, Opcode::SaveState).Inputs(7U, 4U).SrcVregs({7U, 4U});
            INST(9U, Opcode::CallStatic).v0id().InputsAutoType(4U, 7U, 8U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(10U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

SRC_GRAPH(PartialEscapeInsideLoop, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U, 6U)
        {
            INST(2U, Opcode::Phi).u64().Inputs(0U, 11U);
            INST(3U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_NE).Inputs(2U);
        }

        BASIC_BLOCK(3U, 4U, 7U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U);
            INST(6U, Opcode::NewObject).ref().Inputs(5U, 4U);
            INST(7U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_NE).Inputs(2U);
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(8U, Opcode::Sub).u64().Inputs(2U, 1U);
            INST(9U, Opcode::SaveState).Inputs(6U).SrcVregs({1U});
            INST(10U, Opcode::CallStatic).v0id().InputsAutoType(6U, 9U);
        }

        BASIC_BLOCK(7U, 5U) {}

        BASIC_BLOCK(5U, 2U)
        {
            INST(11U, Opcode::Phi).u64().Inputs(8U, 0U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(12U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(PartialEscapeInsideLoop, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U) {}

        BASIC_BLOCK(3U, 4U, 6U)
        {
            INST(2U, Opcode::Phi).u64().Inputs(0U, 0U, 8U);
            INST(3U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_NE).Inputs(2U);
        }

        BASIC_BLOCK(4U, 5U, 3U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(5U, Opcode::LoadAndInitClass).ref().Inputs(4U);
            INST(7U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_NE).Inputs(2U);
        }

        BASIC_BLOCK(5U, 3U)
        {
            INST(8U, Opcode::Sub).u64().Inputs(2U, 1U);
            INST(13U, Opcode::SaveState);
            INST(14U, Opcode::NewObject).ref().Inputs(5U, 13U);
            INST(9U, Opcode::SaveState).Inputs(14U).SrcVregs({1U});
            INST(10U, Opcode::CallStatic).v0id().InputsAutoType(14U, 9U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(12U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, PartialEscapeInsideLoop)
{
    src_graph::PartialEscapeInsideLoop::CREATE(GetGraph());

    GetGraph()->RunPass<LivenessAnalyzer>();

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::PartialEscapeInsideLoop::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(MergeFieldInsideLoop, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(1U, 1U);
        CONSTANT(2U, 100U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::SaveState);
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U);
            INST(5U, Opcode::NewObject).ref().Inputs(4U, 3U);
        }

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(6U, Opcode::Phi).s32().Inputs(2U, 11U);
            INST(7U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_GE).Inputs(6U);
        }

        BASIC_BLOCK(4U, 3U)
        {
            INST(8U, Opcode::SaveState).Inputs(5U).SrcVregs({0U});
            INST(9U, Opcode::NullCheck).ref().Inputs(5U, 8U);
            INST(10U, Opcode::StoreObject).s32().Inputs(9U, 6U).ObjField(INT32_FIELD);
            INST(11U, Opcode::Sub).s32().Inputs(6U, 1U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(12U, Opcode::LoadObject).s32().Inputs(5U).ObjField(INT32_FIELD);
            INST(13U, Opcode::Return).s32().Inputs(12U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(MergeFieldInsideLoop, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(1U, 1U);
        CONSTANT(2U, 100U);
        CONSTANT(5U, 0U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::SaveState);
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U);
        }

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(6U, Opcode::Phi).s32().Inputs(2U, 11U);
            INST(8U, Opcode::Phi).s32().Inputs(5U, 6U);
            INST(7U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_GE).Inputs(6U);
        }

        BASIC_BLOCK(4U, 3U)
        {
            INST(11U, Opcode::Sub).s32().Inputs(6U, 1U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(12U, Opcode::Return).s32().Inputs(8U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, MergeFieldInsideLoop)
{
    src_graph::MergeFieldInsideLoop::CREATE(GetGraph());
    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::MergeFieldInsideLoop::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, MergeObjectsInLoopHeader)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(1U, 1U);
        CONSTANT(2U, 100U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::SaveState);
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U);
            INST(5U, Opcode::NewObject).ref().Inputs(4U, 3U);
        }

        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(6U, Opcode::Phi).ref().Inputs(5U, 10U);
            INST(12U, Opcode::Phi).s32().Inputs(2U, 11U);
            INST(7U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_GE).Inputs(12U);
        }

        BASIC_BLOCK(4U, 3U)
        {
            INST(8U, Opcode::SaveState).Inputs(5U, 6U).SrcVregs({0U, 1U});
            INST(9U, Opcode::LoadAndInitClass).ref().Inputs(8U);
            INST(10U, Opcode::NewObject).ref().Inputs(9U, 8U);
            INST(11U, Opcode::Sub).s32().Inputs(12U, 1U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(13U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

TEST_F(EscapeAnalysisTest, LoadDefaultValue)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(7U, Opcode::LoadObject).u64().Inputs(5U).ObjField(UINT64_FIELD);
            INST(8U, Opcode::Return).u64().Inputs(7U);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(9U, 0U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            // That cast of constant will be optimized in constant folding in Peepholes
            INST(10U, Opcode::Cast).u64().SrcType(DataType::INT64).Inputs(9U);
            INST(8U, Opcode::Return).u64().Inputs(10U);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(VirtualLoadAfterMaterialization, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);  // escape through return
            INST(6U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_EQ).Inputs(0U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U);
            INST(7U, Opcode::LoadObject).u64().Inputs(5U).ObjField(UINT64_FIELD);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::Phi).u64().Inputs(0U, 7U);
            INST(10U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(11U, Opcode::CallStatic).v0id().InputsAutoType(8U, 10U);
            INST(9U, Opcode::Return).ref().Inputs(3U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(VirtualLoadAfterMaterialization, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(12U, 0U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_EQ).Inputs(0U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            // Cast of constant will be optimize in constant folding in Peepholes
            INST(15U, Opcode::Cast).u64().SrcType(DataType::INT64).Inputs(12U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::Phi).u64().Inputs(0U, 15U);
            INST(10U, Opcode::SaveState);
            INST(11U, Opcode::CallStatic).v0id().InputsAutoType(8U, 10U);
            INST(13U, Opcode::SaveState);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 13U);
            INST(9U, Opcode::Return).ref().Inputs(3U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, VirtualLoadAfterMaterialization)
{
    src_graph::VirtualLoadAfterMaterialization::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::VirtualLoadAfterMaterialization::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(MergeVirtualFields, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U);
            INST(5U, Opcode::NewObject).ref().Inputs(4U, 3U);
            INST(6U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_EQ).Inputs(0U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(7U, Opcode::SaveState).Inputs(0U, 5U).SrcVregs({0U, 1U});
            INST(8U, Opcode::NullCheck).ref().Inputs(5U, 7U);
            INST(9U, Opcode::StoreObject).i64().Inputs(8U, 1U).ObjField(INT64_FIELD);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(10U, Opcode::SaveState).Inputs(0U, 5U).SrcVregs({0U, 1U});
            INST(11U, Opcode::NullCheck).ref().Inputs(5U, 10U);
            INST(12U, Opcode::StoreObject).i64().Inputs(11U, 2U).ObjField(INT64_FIELD);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(13U, Opcode::SaveState).Inputs(0U, 5U).SrcVregs({0U, 1U});
            INST(14U, Opcode::NullCheck).ref().Inputs(5U, 13U);
            INST(15U, Opcode::LoadObject).i64().Inputs(14U).ObjField(INT64_FIELD);
            INST(16U, Opcode::Return).i64().Inputs(15U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(MergeVirtualFields, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);
        CONSTANT(2U, 2U);

        BASIC_BLOCK(2U, 5U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U);
            INST(6U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_EQ).Inputs(0U);
        }
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, -1L)
        {
            INST(17U, Opcode::Phi).i64().Inputs(1U, 2U);
            INST(16U, Opcode::Return).i64().Inputs(17U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, MergeVirtualFields)
{
    src_graph::MergeVirtualFields::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::MergeVirtualFields::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(MergeVirtualObjFields, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U);
            INST(5U, Opcode::NewObject).ref().Inputs(4U, 3U);
            INST(17U, Opcode::NewObject).ref().Inputs(4U, 3U);
            INST(18U, Opcode::NewObject).ref().Inputs(4U, 3U);
            INST(6U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_EQ).Inputs(0U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(7U, Opcode::SaveState).Inputs(0U, 5U, 17U).SrcVregs({0U, 1U, 2U});
            INST(8U, Opcode::NullCheck).ref().Inputs(5U, 7U);
            INST(9U, Opcode::StoreObject).ref().Inputs(8U, 17U).ObjField(OBJ_FIELD);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(10U, Opcode::SaveState).Inputs(0U, 5U, 18U).SrcVregs({0U, 1U, 3U});
            INST(11U, Opcode::NullCheck).ref().Inputs(5U, 10U);
            INST(12U, Opcode::StoreObject).ref().Inputs(11U, 18U).ObjField(OBJ_FIELD);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(13U, Opcode::SaveState).Inputs(0U, 5U).SrcVregs({0U, 1U});
            INST(14U, Opcode::NullCheck).ref().Inputs(5U, 13U);
            INST(15U, Opcode::LoadObject).ref().Inputs(14U).ObjField(OBJ_FIELD);
            INST(16U, Opcode::Return).ref().Inputs(15U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(MergeVirtualObjFields, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U);
            INST(6U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_EQ).Inputs(0U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(21U, Opcode::SaveState);
            INST(17U, Opcode::NewObject).ref().Inputs(4U, 21U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(20U, Opcode::SaveState);
            INST(18U, Opcode::NewObject).ref().Inputs(4U, 20U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(19U, Opcode::Phi).ref().Inputs(17U, 18U);
            INST(16U, Opcode::Return).ref().Inputs(19U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, MergeVirtualObjFields)
{
    src_graph::MergeVirtualObjFields::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::MergeVirtualObjFields::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(MultipleIterationsOverCfg, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::NullPtr).ref();
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U);
            INST(5U, Opcode::NewObject).ref().Inputs(4U, 3U);
            INST(6U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_EQ).Inputs(0U);
        }
        BASIC_BLOCK(3U, 5U, 6U)
        {
            INST(7U, Opcode::SaveState).Inputs(0U, 5U).SrcVregs({0U, 1U});
            INST(8U, Opcode::NullCheck).ref().Inputs(5U, 7U);
            INST(9U, Opcode::StoreObject).u64().Inputs(8U, 1U).ObjField(UINT64_FIELD);
            INST(11U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(1U).CC(CC_EQ).Inputs(0U);
        }
        BASIC_BLOCK(4U, 5U) {}

        BASIC_BLOCK(5U, 7U) {}

        BASIC_BLOCK(7U, -1L)
        {
            INST(16U, Opcode::Return).ref().Inputs(5U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(17U, Opcode::Return).ref().Inputs(2U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(MultipleIterationsOverCfg, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);
        CONSTANT(22U, 0U);

        BASIC_BLOCK(2U, 3U, 7U)
        {
            INST(2U, Opcode::NullPtr).ref();
            INST(3U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U);
            INST(6U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 7U, 6U)
        {
            INST(11U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(1U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(7U, -1L)
        {
            INST(18U, Opcode::Phi).u64().Inputs(22U, 1U);
            INST(19U, Opcode::SaveState);
            INST(5U, Opcode::NewObject).ref().Inputs(4U, 19U);
            INST(20U, Opcode::StoreObject).u64().Inputs(5U, 18U);
            INST(16U, Opcode::Return).ref().Inputs(5U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(17U, Opcode::Return).ref().Inputs(2U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, MultipleIterationsOverCfg)
{
    src_graph::MultipleIterationsOverCfg::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::MultipleIterationsOverCfg::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(CarryNestedState, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).Inputs(0U, 4U).SrcVregs({0U, 1U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 5U);
            INST(8U, Opcode::SaveState).Inputs(0U, 4U, 7U).SrcVregs({0U, 1U, 2U});
            INST(9U, Opcode::NullCheck).ref().Inputs(7U, 8U);
            INST(10U, Opcode::StoreObject).ref().Inputs(9U, 0U).ObjField(OBJ_FIELD);
            INST(11U, Opcode::NullCheck).ref().Inputs(4U, 8U);
            INST(12U, Opcode::StoreObject).ref().Inputs(11U, 7U).ObjField(OBJ_FIELD);
            INST(13U, Opcode::IfImm).SrcType(DataType::REFERENCE).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(14U, Opcode::SaveState).Inputs(4U).SrcVregs({1U});
            INST(15U, Opcode::CallStatic).v0id().InputsAutoType(14U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(16U, Opcode::SaveState).Inputs(4U).SrcVregs({1U});
            INST(17U, Opcode::NullCheck).ref().Inputs(4U, 16U);
            INST(18U, Opcode::LoadObject).ref().Inputs(17U).ObjField(OBJ_FIELD);
            INST(19U, Opcode::SaveState).Inputs(18U).SrcVregs({0U});
            INST(20U, Opcode::NullCheck).ref().Inputs(18U, 19U);
            INST(21U, Opcode::LoadObject).ref().Inputs(20U).ObjField(OBJ_FIELD);
            INST(22U, Opcode::Return).ref().Inputs(21U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(CarryNestedState, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(13U, Opcode::IfImm).SrcType(DataType::REFERENCE).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(14U, Opcode::SaveState).Inputs(0U).SrcVregs({VirtualRegister::BRIDGE});
            INST(15U, Opcode::CallStatic).v0id().InputsAutoType(14U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(22U, Opcode::Return).ref().Inputs(0U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, CarryNestedState)
{
    src_graph::CarryNestedState::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::CarryNestedState::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(RemoveVirtualObjectFromSaveStates, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::SaveState);
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);

            INST(4U, Opcode::IfImm).SrcType(DataType::UINT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::ReturnVoid).v0id();
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(3U, 6U).Inlined();
            INST(8U, Opcode::SaveState).Inputs(3U).SrcVregs({0U}).Caller(7U);
            INST(9U, Opcode::CallStatic).v0id().InputsAutoType(3U, 8U).Inlined();
            INST(10U, Opcode::SaveState).Inputs(3U).SrcVregs({0U}).Caller(9U);
            INST(11U, Opcode::CallStatic).v0id().InputsAutoType(3U, 10U);
            INST(12U, Opcode::ReturnInlined).Inputs(8U);
            INST(13U, Opcode::ReturnInlined).Inputs(6U);
            INST(14U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(15U, Opcode::Throw).Inputs(3U, 14U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(RemoveVirtualObjectFromSaveStates, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u32();
        INST(18U, Opcode::NullPtr).ref();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::SaveState);
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(4U, Opcode::IfImm).SrcType(DataType::UINT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(5U, Opcode::ReturnVoid).v0id();
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::SaveState);
            INST(7U, Opcode::CallStatic).v0id().InputsAutoType(18U, 6U).Inlined();
            INST(8U, Opcode::SaveState).Caller(7U);
            INST(9U, Opcode::CallStatic).v0id().InputsAutoType(18U, 8U).Inlined();
            INST(41U, Opcode::SaveState).Caller(9U);
            INST(42U, Opcode::NewObject).ref().Inputs(2U, 41U);
            INST(10U, Opcode::SaveState).Caller(9U).Inputs(42U).SrcVregs({0U});
            INST(11U, Opcode::CallStatic).v0id().InputsAutoType(42U, 10U);
            INST(12U, Opcode::ReturnInlined).Inputs(8U);
            INST(13U, Opcode::ReturnInlined).Inputs(6U);
            INST(14U, Opcode::SaveState).Inputs(42U).SrcVregs({0U});
            INST(15U, Opcode::Throw).Inputs(42U, 14U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, RemoveVirtualObjectFromSaveStates)
{
    src_graph::RemoveVirtualObjectFromSaveStates::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::RemoveVirtualObjectFromSaveStates::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(CorrectlyHandleSingleBlockLoops, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
        }

        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(5U, Opcode::Phi).u64().Inputs(0U, 6U);
            INST(6U, Opcode::Sub).u64().Inputs(5U, 1U);
            INST(7U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_NE).Inputs(6U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(9U, Opcode::SaveState).Inputs(4U).SrcVregs({0U});
            INST(10U, Opcode::Throw).Inputs(4U, 9U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(CorrectlyHandleSingleBlockLoops, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
        }

        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(5U, Opcode::Phi).u64().Inputs(0U, 6U);
            INST(6U, Opcode::Sub).u64().Inputs(5U, 1U);
            INST(7U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_NE).Inputs(6U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(11U, Opcode::SaveState);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 11U);
            INST(9U, Opcode::SaveState).Inputs(4U).SrcVregs({0U});
            INST(10U, Opcode::Throw).Inputs(4U, 9U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, CorrectlyHandleSingleBlockLoops)
{
    src_graph::CorrectlyHandleSingleBlockLoops::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::CorrectlyHandleSingleBlockLoops::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, MaterializeObjOnPhiInsideLoop)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
        }

        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(5U, Opcode::Phi).u64().Inputs(0U, 6U);
            INST(11U, Opcode::Phi).ref().Inputs(4U, 14U);
            INST(6U, Opcode::Sub).u64().Inputs(5U, 1U);
            INST(12U, Opcode::SaveState).Inputs(4U, 11U).SrcVregs({0U, 1U});
            INST(13U, Opcode::LoadAndInitClass).ref().Inputs(12U);
            INST(14U, Opcode::NewObject).ref().Inputs(13U, 12U);
            INST(7U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_NE).Inputs(6U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(15U, Opcode::Return).ref().Inputs(11U);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

SRC_GRAPH(NotMergingMaterializationPaths, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(10U, Opcode::SaveState);
            INST(11U, Opcode::LoadAndInitClass).ref().Inputs(10U);
            INST(12U, Opcode::NewObject).ref().Inputs(11U, 10U);
            INST(13U, Opcode::IfImm).SrcType(DataType::INT64).Imm(0U).CC(CC_LE).Inputs(0U);
        }

        BASIC_BLOCK(3U, 5U, 6U)
        {
            INST(14U, Opcode::IfImm).SrcType(DataType::INT64).Imm(-1L).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(15U, Opcode::SaveState).Inputs(12U).SrcVregs({0U});
            INST(16U, Opcode::Throw).Inputs(12U, 15U);
        }

        BASIC_BLOCK(4U, 7U, 6U)
        {
            INST(17U, Opcode::IfImm).SrcType(DataType::INT64).Imm(1U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(7U, -1L)
        {
            INST(18U, Opcode::SaveState).Inputs(12U).SrcVregs({0U});
            INST(19U, Opcode::Throw).Inputs(12U, 18U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(20U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(NotMergingMaterializationPaths, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s64();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(10U, Opcode::SaveState);
            INST(11U, Opcode::LoadAndInitClass).ref().Inputs(10U);
            INST(13U, Opcode::IfImm).SrcType(DataType::INT64).Imm(0U).CC(CC_LE).Inputs(0U);
        }

        BASIC_BLOCK(3U, 5U, 6U)
        {
            INST(14U, Opcode::IfImm).SrcType(DataType::INT64).Imm(-1L).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(21U, Opcode::SaveState);
            INST(12U, Opcode::NewObject).ref().Inputs(11U, 21U);
            INST(15U, Opcode::SaveState).Inputs(12U).SrcVregs({0U});
            INST(16U, Opcode::Throw).Inputs(12U, 15U);
        }

        BASIC_BLOCK(4U, 7U, 6U)
        {
            INST(17U, Opcode::IfImm).SrcType(DataType::INT64).Imm(1U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(7U, -1L)
        {
            INST(22U, Opcode::SaveState);
            INST(23U, Opcode::NewObject).ref().Inputs(11U, 22U);
            INST(18U, Opcode::SaveState).Inputs(23U).SrcVregs({0U});
            INST(19U, Opcode::Throw).Inputs(23U, 18U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(20U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, NotMergingMaterializationPaths)
{
    src_graph::NotMergingMaterializationPaths::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::NotMergingMaterializationPaths::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(MaterializeCrossReferencingObjects, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();

        // obj.4 field0 = obj.7
        // obj.7 field0 = obj.4
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);

            INST(5U, Opcode::SaveState).Inputs(0U, 4U).SrcVregs({0U, 1U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 5U);

            INST(8U, Opcode::SaveState).Inputs(0U, 4U, 7U).SrcVregs({0U, 1U, 2U});
            INST(9U, Opcode::NullCheck).ref().Inputs(7U, 8U);
            INST(10U, Opcode::StoreObject).ref().Inputs(9U, 4U).ObjField(OBJ_FIELD);
            INST(11U, Opcode::NullCheck).ref().Inputs(4U, 8U);
            INST(12U, Opcode::StoreObject).ref().Inputs(11U, 7U).ObjField(OBJ_FIELD);
            INST(13U, Opcode::IfImm).SrcType(DataType::REFERENCE).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(14U, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({1U, 2U});
            INST(15U, Opcode::CallStatic).v0id().InputsAutoType(4U, 7U, 14U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(22U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(MaterializeCrossReferencingObjects, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(13U, Opcode::IfImm).SrcType(DataType::REFERENCE).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(16U, Opcode::SaveState);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 16U);
            INST(17U, Opcode::SaveState).Inputs(7U).SrcVregs({2U});
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 17U);
            INST(10U, Opcode::StoreObject).ref().Inputs(4U, 7U).ObjField(OBJ_FIELD);
            INST(12U, Opcode::StoreObject).ref().Inputs(7U, 4U).ObjField(OBJ_FIELD);
            INST(14U, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({1U, 2U});
            INST(15U, Opcode::CallStatic).v0id().InputsAutoType(4U, 7U, 14U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(22U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, MaterializeCrossReferencingObjects)
{
    src_graph::MaterializeCrossReferencingObjects::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::MaterializeCrossReferencingObjects::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(MaterializeCrossReferencingObjectsAtNewSaveState, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();

        // obj.7 field0 = obj.4
        // obj.4 field0 = obj.7
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);

            INST(5U, Opcode::SaveState).Inputs(0U, 4U).SrcVregs({0U, 1U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 5U);

            INST(8U, Opcode::SaveState).Inputs(0U, 4U, 7U).SrcVregs({0U, 1U, 2U});
            INST(9U, Opcode::NullCheck).ref().Inputs(7U, 8U);
            INST(10U, Opcode::StoreObject).ref().Inputs(9U, 4U).ObjField(OBJ_FIELD);
            INST(11U, Opcode::NullCheck).ref().Inputs(4U, 8U);
            INST(12U, Opcode::StoreObject).ref().Inputs(11U, 7U).ObjField(OBJ_FIELD);
            INST(13U, Opcode::IfImm).SrcType(DataType::REFERENCE).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(14U, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({1U, 2U});
            INST(15U, Opcode::CallStatic).v0id().InputsAutoType(14U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(22U, Opcode::Return).ref().Inputs(7U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(MaterializeCrossReferencingObjectsAtNewSaveState, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(5U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(13U, Opcode::IfImm).SrcType(DataType::REFERENCE).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(14U, Opcode::SaveState);
            INST(15U, Opcode::CallStatic).v0id().InputsAutoType(14U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(16U, Opcode::SaveState);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 16U);
            INST(17U, Opcode::SaveState).Inputs(7U).SrcVregs({VirtualRegister::BRIDGE});
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 17U);
            INST(10U, Opcode::StoreObject).ref().Inputs(4U, 7U).ObjField(OBJ_FIELD);
            INST(12U, Opcode::StoreObject).ref().Inputs(7U, 4U).ObjField(OBJ_FIELD);
            INST(22U, Opcode::Return).ref().Inputs(7U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, MaterializeCrossReferencingObjectsAtNewSaveState)
{
    src_graph::MaterializeCrossReferencingObjectsAtNewSaveState::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::MaterializeCrossReferencingObjectsAtNewSaveState::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, HandleCompareObjects)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(4U, Opcode::Compare).b().Inputs(0U, 3U).CC(CC_EQ).SrcType(DataType::REFERENCE);
            INST(5U, Opcode::Return).b().Inputs(4U);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();
        CONSTANT(6U, 0U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(5U, Opcode::Return).b().Inputs(6U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, MaterializeSaveStateInputsEscapingInSaveStateUser)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U, 5U)
        {
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            // Inst 4 escapes only in block 3, but it should be
            // materialized before corresponding save state.
            INST(5U, Opcode::SaveState).Inputs(4U).SrcVregs({0U});
            INST(6U, Opcode::CallStatic).u64().InputsAutoType(0U, 5U).Inlined();
            INST(7U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_EQ).Inputs(0U).Imm(0U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(8U, Opcode::CallStatic).v0id().InputsAutoType(4U, 5U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(10U, Opcode::Phi).u64().Inputs(0U, 1U);
            INST(11U, Opcode::ReturnInlined).u64().Inputs(5U);
            INST(12U, Opcode::Return).u64().Inputs(10U);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_FALSE(Run());
}

TEST_F(EscapeAnalysisTest, EliminateStoreToNarrowIntField)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u32();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::StoreObject).u8().Inputs(4U, 0U).ObjField(UINT8_FIELD);
            INST(6U, Opcode::LoadObject).u8().Inputs(4U).ObjField(UINT8_FIELD);
            INST(7U, Opcode::Add).u32().Inputs(6U, 1U);
            INST(8U, Opcode::Return).u32().Inputs(7U);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u32();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::Cast).u8().SrcType(DataType::UINT32).Inputs(0U);
            INST(7U, Opcode::Add).u32().Inputs(4U, 1U);
            INST(8U, Opcode::Return).u32().Inputs(7U);
        }
        // NOLINTEND(readability-magic-numbers)
    }

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(EscapeAnalysisTest, InverseMaterializationOrder)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).b();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).Inputs(4U).SrcVregs({0U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 5U);
            INST(8U, Opcode::StoreObject).ref().Inputs(7U, 4U).ObjField(OBJ_FIELD);
            INST(9U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, -1L)
        {
            INST(10U, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(11U, Opcode::LoadAndInitClass).ref().Inputs(10U);
            INST(12U, Opcode::NewObject).ref().Inputs(11U, 10U);
            INST(13U, Opcode::StoreObject).ref().Inputs(12U, 7U).ObjField(OBJ_FIELD);
            INST(14U, Opcode::SaveState).Inputs(4U, 7U, 12U).SrcVregs({0U, 1U, 2U});
            INST(15U, Opcode::NullCheck).ref().Inputs(4U, 14U);
            INST(16U, Opcode::CallStatic).v0id().InputsAutoType(15U, 12U, 14U);
            INST(17U, Opcode::ReturnVoid).v0id();
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(18U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }

    // Don't check transformed graph as the test intended to only reproduce a crash.
    ASSERT_TRUE(Run());
}

TEST_F(EscapeAnalysisTest, MaterializeObjectDuringItsFieldsMerge)
{
    GRAPH(GetGraph())
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).b();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::StoreObject).u64().Inputs(4U, 1U).ObjField(UINT64_FIELD);
            INST(6U, Opcode::StoreObject).s64().Inputs(4U, 1U).ObjField(INT64_FIELD);
            INST(7U, Opcode::StoreObject).s32().Inputs(4U, 1U).ObjField(INT32_FIELD);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(9U, Opcode::SaveState).Inputs(4U).SrcVregs({1U});
            INST(10U, Opcode::LoadAndInitClass).ref().Inputs(9U);
            INST(11U, Opcode::NewObject).ref().Inputs(10U, 9U);
            INST(12U, Opcode::StoreObject).ref().Inputs(11U, 4U).ObjField(OBJ_FIELD);
            INST(13U, Opcode::StoreObject).ref().Inputs(4U, 11U).ObjField(OBJ_FIELD);
        }

        BASIC_BLOCK(4U, -1L)
        {
            // We need to merge fields of the object (4), but during the merge we'll need to materialize
            // the object (11) as values of corresponding field diverged and require a phi.
            // Materialization of the object (11) will require materialization of the object (4) as it is
            // stored in 11's field. As a result materialization of the object (4) will be requested
            // during its fields merge.
            INST(14U, Opcode::LoadObject).ref().Inputs(4U).ObjField(OBJ_FIELD);
            INST(15U, Opcode::Return).ref().Inputs(14U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
    ASSERT_FALSE(Run());
}

SRC_GRAPH(MaterializeBeforeReferencedObjectMaterialization, Graph *graph, uint32_t callInstId, uint32_t ss1Id,
          uint32_t ss2Id)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).Inputs(4U).SrcVregs({0U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 5U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(1U).Imm(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(9U, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(callInstId, Opcode::CallStatic).v0id().InputsAutoType(9U).Inlined();
            INST(ss1Id, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(12U, Opcode::CallStatic).v0id().InputsAutoType(7U, 11U);
            INST(ss2Id, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            // Due to deopt inst all its save state inputs should be materialized before the first inlined call.
            // So both inst 4 and 7 should be materialized before inst 10, but before we hit deopt inst the inst 7
            // will be materialized at save state 11 (due to call static 12).
            INST(14U, Opcode::DeoptimizeIf).Inputs(0U, 13U);
            INST(15U, Opcode::ReturnInlined).v0id().Inputs(9U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(16U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(MaterializeBeforeReferencedObjectMaterialization, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(5U, Opcode::SaveState);
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(1U).Imm(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(17U, Opcode::SaveState);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 17U);
            INST(18U, Opcode::SaveState).Inputs(7U).SrcVregs({1U});
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 18U);

            INST(9U, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(10U, Opcode::CallStatic).v0id().InputsAutoType(9U).Inlined();
            INST(11U, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(12U, Opcode::CallStatic).v0id().InputsAutoType(7U, 11U);
            INST(13U, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(14U, Opcode::DeoptimizeIf).Inputs(0U, 13U);
            INST(15U, Opcode::ReturnInlined).v0id().Inputs(9U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(16U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, MaterializeBeforeReferencedObjectMaterialization)
{
    static constexpr uint32_t CALL_INST_ID = 10;
    static constexpr uint32_t SS1_ID = 11;
    static constexpr uint32_t SS2_ID = 13;
    src_graph::MaterializeBeforeReferencedObjectMaterialization::CREATE(GetGraph(), CALL_INST_ID, SS1_ID, SS2_ID);

    INS(SS1_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST_ID)));
    INS(SS2_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST_ID)));

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::MaterializeBeforeReferencedObjectMaterialization::CREATE(graph);
    INS(SS1_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST_ID)));
    INS(SS2_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST_ID)));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
SRC_GRAPH(FillSaveStateBetweenSaveState, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 42U).s32();
        PARAMETER(13U, 2U).ref();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U, 2U}).Inputs(0U, 13U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(10U, Opcode::NullCheck).ref().Inputs(4U, 2U);
            INST(11U, Opcode::StoreObject).s32().Inputs(10U, 1U).ObjField(INT32_FIELD);
            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            // Branch with BasicBlock 3 don't have 13(vr2) in SaveState
            INST(8U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 4U);
            INST(9U, Opcode::CallStatic).v0id().InputsAutoType(4U, 8U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Phi).Inputs(0U, 1U).s32();
            INST(14U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 13U);
            INST(7U, Opcode::Return).Inputs(6U).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(FillSaveStateBetweenSaveState, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).s32();
        CONSTANT(1U, 42U).s32();
        PARAMETER(13U, 2U).ref();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U, 2U}).Inputs(0U, 13U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::INT32).Imm(0U).CC(CC_EQ).Inputs(0U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(15U, Opcode::SaveState).SrcVregs({0U, VirtualRegister::BRIDGE}).Inputs(0U, 13U);
            INST(16U, Opcode::NewObject).ref().Inputs(3U, 15U);
            INST(17U, Opcode::StoreObject).s32().Inputs(16U, 1U).ObjField(INT32_FIELD);
            INST(8U, Opcode::SaveState).SrcVregs({0U, 1U, VirtualRegister::BRIDGE}).Inputs(0U, 16U, 13U);
            INST(9U, Opcode::CallStatic).v0id().InputsAutoType(16U, 8U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Phi).Inputs(0U, 1U).s32();
            INST(7U, Opcode::Return).Inputs(6U).s32();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, FillSaveStateBetweenSaveState)
{
    src_graph::FillSaveStateBetweenSaveState::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::FillSaveStateBetweenSaveState::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

/* if the original input for a newly created phi was a
 * load then we have to ensure the current phi's actual
 * input type matches the type of phi itself */
SRC_GRAPH(FixPhiInputTypes, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s8();
        CONSTANT(20U, 64U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 1U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::StoreObject).s8().Inputs(4U, 1U).ObjField(INT8_FIELD);
            INST(6U, Opcode::IfImm).SrcType(DataType::INT8).Imm(0U).CC(CC_EQ).Inputs(1U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(7U, Opcode::LoadObject).s8().Inputs(4U).ObjField(INT8_FIELD);
            INST(8U, Opcode::Add).s32().Inputs(7U, 20U);
            INST(9U, Opcode::StoreObject).s8().Inputs(4U, 8U).ObjField(INT8_FIELD);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::LoadObject).s8().Inputs(4U).ObjField(INT8_FIELD);
            INST(11U, Opcode::Return).Inputs(10U).s8();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(FixPhiInputTypes, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).s8();
        CONSTANT(20U, 64U).s32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState).SrcVregs({0U, 1U}).Inputs(0U, 1U);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::INT8).Imm(0U).CC(CC_EQ).Inputs(1U);
        }

        BASIC_BLOCK(3U, 4U)
        {
            INST(8U, Opcode::Add).s32().Inputs(1U, 20U);
            INST(9U, Opcode::Cast).s8().SrcType(DataType::INT32).Inputs(8U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::Phi).Inputs(1U, 9U).s8();
            INST(11U, Opcode::Return).Inputs(10U).s8();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, FixPhiInputTypes)
{
    src_graph::FixPhiInputTypes::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::FixPhiInputTypes::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(CorrectlyHandleSingleUsedRefInLoops, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        PARAMETER(18U, 1U).u64();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(15U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(16U, Opcode::LoadAndInitClass).ref().Inputs(15U);
            INST(17U, Opcode::NewObject).ref().Inputs(16U, 15U);
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
        }

        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(5U, Opcode::Phi).u64().Inputs(0U, 6U);

            INST(8U, Opcode::SaveState).SrcVregs({4U}).Inputs(4U);
            INST(12U, Opcode::Load).ref().Inputs(4U, 1U);
            INST(11U, Opcode::CallStatic).v0id().InputsAutoType(12U, 8U);

            // Check that Load 12 is not filled in SaveState 13
            INST(13U, Opcode::SaveState).SrcVregs({4U}).Inputs(4U);
            INST(19U, Opcode::CallStatic).u64().InputsAutoType(18U, 13U);
            INST(6U, Opcode::Sub).u64().Inputs(5U, 19U);
            INST(7U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_NE).Inputs(6U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(9U, Opcode::SaveState).Inputs(4U).SrcVregs({0U});
            INST(10U, Opcode::Throw).Inputs(4U, 9U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(CorrectlyHandleSingleUsedRefInLoops, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        PARAMETER(18U, 1U).u64();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(15U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(16U, Opcode::LoadAndInitClass).ref().Inputs(15U);
            INST(2U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
        }

        BASIC_BLOCK(3U, 3U, 5U)
        {
            INST(5U, Opcode::Phi).u64().Inputs(0U, 6U);

            INST(8U, Opcode::SaveState).SrcVregs({4U}).Inputs(4U);
            INST(12U, Opcode::Load).ref().Inputs(4U, 1U);
            INST(11U, Opcode::CallStatic).v0id().InputsAutoType(12U, 8U);

            // Check that Load 12 is not filled in SaveState 13
            INST(13U, Opcode::SaveState).SrcVregs({4U}).Inputs(4U);
            INST(19U, Opcode::CallStatic).u64().InputsAutoType(18U, 13U);
            INST(6U, Opcode::Sub).u64().Inputs(5U, 19U);
            INST(7U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_NE).Inputs(6U);
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(9U, Opcode::SaveState).Inputs(4U).SrcVregs({0U});
            INST(10U, Opcode::Throw).Inputs(4U, 9U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, CorrectlyHandleSingleUsedRefInLoops)
{
    src_graph::CorrectlyHandleSingleUsedRefInLoops::CREATE(GetGraph());
    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::CorrectlyHandleSingleUsedRefInLoops::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

SRC_GRAPH(SplitDeoptimizationP1, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(0U, 5);
        CONSTANT(1U, 4);
        CONSTANT(2U, 0);
        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(INT32_CLASS);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
            INST(10U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({4U});
            INST(11U, Opcode::NullCheck).ref().Inputs(4U, 10U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(5U, Opcode::BoundsCheck).b().Inputs(0U, 1U, 3U);
            INST(6U, Opcode::Compare).b().Inputs(5U, 2U).SrcType(DataType::BOOL).CC(ConditionCode::CC_EQ);
            INST(59U, Opcode::SaveState);
            INST(7U, Opcode::DeoptimizeIf).Inputs(6U, 59U);
            INST(8U, Opcode::LoadArray).s32().Inputs(4U, 5U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(SplitDeoptimizationP1, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(0U, 5);
        CONSTANT(1U, 4);
        CONSTANT(2U, 0);
        INST(53U, Opcode::NullPtr).ref();
        BASIC_BLOCK(3U, 10U, 11U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(INT32_CLASS);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
            INST(10U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({4U}).ClearFlag(inst_flags::NO_DCE);
            INST(54U, Opcode::Compare).b().Inputs(4U, 53U).SrcType(DataType::REFERENCE).CC(ConditionCode::CC_EQ);
            INST(55U, Opcode::IfImm).b().SrcType(DataType::BOOL).Imm(0U).CC(CC_EQ).Inputs(54U);
        }
        BASIC_BLOCK(11U, -1L)
        {
            INST(51U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({4U});
            INST(52U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(51U);
        }
        BASIC_BLOCK(10U, 8U, 9U)
        {
            INST(56U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(5U, Opcode::BoundsCheck).b().Inputs(0U, 1U, 56U);
            INST(6U, Opcode::Compare).b().Inputs(5U, 2U).SrcType(DataType::BOOL).CC(ConditionCode::CC_EQ);
            INST(47U, Opcode::SaveState).ClearFlag(inst_flags::NO_DCE);
            INST(50U, Opcode::IfImm).b().SrcType(DataType::BOOL).Imm(0U).CC(CC_EQ).Inputs(6U);
        }
        BASIC_BLOCK(9U, -1L)
        {
            INST(48U, Opcode::SaveState);
            INST(49U, Opcode::Deoptimize).Inputs(48U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(8U, Opcode::LoadArray).s32().Inputs(4U, 5U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(SplitDeoptimizationP2, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(0U, 5);
        CONSTANT(1U, 4);
        CONSTANT(2U, 0);
        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(INT32_CLASS);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
            INST(57U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({4U});
            INST(58U, Opcode::NullCheck).ref().Inputs(4U, 57U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(56U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(5U, Opcode::BoundsCheck).b().Inputs(0U, 1U, 56U);
            INST(6U, Opcode::Compare).b().Inputs(5U, 2U).SrcType(DataType::BOOL).CC(ConditionCode::CC_EQ);
            INST(59U, Opcode::SaveState);
            INST(60U, Opcode::DeoptimizeIf).Inputs(6U, 59U);
            INST(8U, Opcode::LoadArray).s32().Inputs(4U, 5U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

SRC_GRAPH(SplitDeoptimizationP3, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(0U, 5);
        CONSTANT(1U, 4);
        CONSTANT(2U, 0);
        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(INT32_CLASS);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
            INST(10U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({4U});
            INST(11U, Opcode::NullCheck).ref().Inputs(4U, 10U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(7U, Opcode::NullCheck).ref().Inputs(11U, 10U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(5U, Opcode::BoundsCheck).b().Inputs(0U, 1U, 3U);
            INST(8U, Opcode::LoadArray).s32().Inputs(4U, 5U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(SplitDeoptimizationP3, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(0U, 5);
        CONSTANT(1U, 4);
        CONSTANT(2U, 0);
        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(INT32_CLASS);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
            INST(55U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({4U});
            INST(56U, Opcode::NullCheck).ref().Inputs(4U, 55U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(57U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({4U});
            INST(58U, Opcode::NullCheck).ref().Inputs(56U, 57U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(50U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(5U, Opcode::BoundsCheck).b().Inputs(0U, 1U, 50U);
            INST(8U, Opcode::LoadArray).s32().Inputs(4U, 5U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

SRC_GRAPH(SplitDeoptimizationP4, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(0U, 5);
        CONSTANT(1U, 4);
        CONSTANT(2U, 0);
        BASIC_BLOCK(3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(INT32_CLASS);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
            INST(10U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({4U});
            INST(11U, Opcode::NullCheck).ref().Inputs(4U, 10U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(5U, Opcode::BoundsCheck).b().Inputs(0U, 1U, 3U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(6U, Opcode::Compare).b().Inputs(5U, 2U).SrcType(DataType::BOOL).CC(ConditionCode::CC_EQ);
            INST(7U, Opcode::DeoptimizeIf).Inputs(6U);
            INST(8U, Opcode::LoadArray).s32().Inputs(4U, 5U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

SRC_GRAPH(SplitDeoptimizationP5, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(3U, Opcode::NewObject).ref().Inputs(2U, 1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_EQ).Inputs(0U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(4U, Opcode::SaveState).Inputs(0U, 3U).SrcVregs({0U, 1U});
            INST(5U, Opcode::NullCheck).ref().Inputs(3U, 4U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(7U, Opcode::LoadObject).u64().Inputs(5U).ObjField(UINT64_FIELD);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::Phi).u64().Inputs(0U, 7U);
            INST(10U, Opcode::SaveState).Inputs(3U).SrcVregs({0U});
            INST(11U, Opcode::CallStatic).v0id().InputsAutoType(8U, 10U);
            INST(9U, Opcode::Return).ref().Inputs(3U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(SplitDeoptimizationP5, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).u64();
        CONSTANT(17U, 0U);
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(1U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(2U, Opcode::LoadAndInitClass).ref().Inputs(1U);
            INST(6U, Opcode::IfImm).SrcType(DataType::UINT64).Imm(0U).CC(CC_EQ).Inputs(0U);
        }
        BASIC_BLOCK(3U, 6U, 7U)
        {
            INST(16U, Opcode::IfImm).b().SrcType(DataType::BOOL).Imm(0U).CC(CC_EQ).Inputs(17U);
        }
        BASIC_BLOCK(7U, -1L)
        {
            INST(20U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(21U, Opcode::NewObject).ref().Inputs(2U, 20U);
            INST(12U, Opcode::SaveState).Inputs(0U, 21U).SrcVregs({0U, 1U});
            INST(13U, Opcode::Deoptimize).DeoptimizeType(DeoptimizeType::NULL_CHECK).Inputs(12U);
        }
        BASIC_BLOCK(6U, 4U)
        {
            INST(22U, Opcode::Cast).u64().SrcType(DataType::INT64).Inputs(17U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::Phi).u64().Inputs(0U, 22U);
            INST(10U, Opcode::SaveState);
            INST(11U, Opcode::CallStatic).v0id().InputsAutoType(8U, 10U);
            INST(18U, Opcode::SaveState);
            INST(19U, Opcode::NewObject).ref().Inputs(2U, 18U);
            INST(9U, Opcode::Return).ref().Inputs(19U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

SRC_GRAPH(SplitDeoptimizationP6, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(0U, 5);
        CONSTANT(1U, 4);
        CONSTANT(2U, 0);
        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(INT32_CLASS);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
            INST(10U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({4U});
            INST(11U, Opcode::NullCheck).ref().Inputs(4U, 10U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(5U, Opcode::BoundsCheck).b().Inputs(0U, 1U, 3U);
            INST(12U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({4U});
            INST(13U, Opcode::NullCheck).ref().Inputs(4U, 12U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(8U, Opcode::LoadArray).s32().Inputs(13U, 5U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(SplitDeoptimizationP6, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(0U, 5);
        CONSTANT(1U, 4);
        CONSTANT(2U, 0);
        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(INT32_CLASS);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
            INST(55U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({4U});
            INST(56U, Opcode::NullCheck).ref().Inputs(4U, 55U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(54U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(5U, Opcode::BoundsCheck).b().Inputs(0U, 1U, 54U);
            INST(57U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({4U});
            INST(58U, Opcode::NullCheck).ref().Inputs(4U, 57U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(8U, Opcode::LoadArray).s32().Inputs(58U, 5U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

SRC_GRAPH(SplitDeoptimizationP7, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(0U, 5);
        CONSTANT(1U, 4);
        CONSTANT(2U, 0);
        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(INT32_CLASS);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
            INST(10U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({4U});
            INST(11U, Opcode::NullCheck).ref().Inputs(4U, 10U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(5U, Opcode::BoundsCheck).b().Inputs(0U, 1U, 3U);
            INST(20U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
            INST(12U, Opcode::SaveStateDeoptimize).Inputs(20U).SrcVregs({20U});
            INST(13U, Opcode::NullCheck).ref().Inputs(20U, 12U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(8U, Opcode::LoadArray).s32().Inputs(13U, 5U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(SplitDeoptimizationP7, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(0U, 5);
        CONSTANT(1U, 4);
        CONSTANT(2U, 0);
        BASIC_BLOCK(3U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(INT32_CLASS);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
            INST(55U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({4U});
            INST(56U, Opcode::NullCheck).ref().Inputs(4U, 55U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(54U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(5U, Opcode::BoundsCheck).b().Inputs(0U, 1U, 54U);
            INST(20U, Opcode::NewArray).ref().Inputs(44U, 0U, 54U);
            INST(57U, Opcode::SaveStateDeoptimize).Inputs(20U).SrcVregs({20U});
            INST(58U, Opcode::NullCheck).ref().Inputs(20U, 57U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(8U, Opcode::LoadArray).s32().Inputs(58U, 5U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

SRC_GRAPH(SplitDeoptimizationP8, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        CONSTANT(0U, 5);
        CONSTANT(1U, 4);
        CONSTANT(2U, 0);
        BASIC_BLOCK(3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(44U, Opcode::LoadAndInitClass).ref().Inputs(3U).Class(INT32_CLASS);
            INST(4U, Opcode::NewArray).ref().Inputs(44U, 0U, 3U);
            INST(10U, Opcode::SaveStateDeoptimize).Inputs(4U).SrcVregs({4U});
            INST(11U, Opcode::NullCheck).ref().Inputs(4U, 10U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(5U, Opcode::BoundsCheck).b().Inputs(0U, 1U, 3U);
            INST(6U, Opcode::Compare).b().Inputs(5U, 2U).SrcType(DataType::BOOL).CC(ConditionCode::CC_EQ);
            INST(59U, Opcode::SaveState);
            INST(7U, Opcode::DeoptimizeIf).Inputs(6U, 59U);
        }
        BASIC_BLOCK(5U, -1L)
        {
            INST(8U, Opcode::LoadArray).s32().Inputs(4U, 5U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

SRC_GRAPH(SplitDeoptimizationP9, Graph *graph, uint32_t callInstId, uint32_t ss1Id, uint32_t ss2Id)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).Inputs(4U).SrcVregs({0U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 5U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(1U).Imm(0U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(9U, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(callInstId, Opcode::CallStatic).v0id().InputsAutoType(9U).Inlined();
            INST(ss1Id, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(12U, Opcode::CallStatic).v0id().InputsAutoType(7U, 11U);
            INST(ss2Id, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(14U, Opcode::DeoptimizeIf).Inputs(0U, 13U);
            INST(15U, Opcode::ReturnInlined).v0id().Inputs(9U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(16U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

SRC_GRAPH(SplitDeoptimizationP10, Graph *graph, uint32_t callInstId, uint32_t ss1Id, uint32_t ss2Id)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).Inputs(4U).SrcVregs({0U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 5U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(1U).Imm(0U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(9U, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(callInstId, Opcode::CallStatic).v0id().InputsAutoType(9U).Inlined();
            INST(ss1Id, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(ss2Id, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(12U, Opcode::CallStatic).v0id().InputsAutoType(7U, ss1Id);
            INST(14U, Opcode::DeoptimizeIf).Inputs(0U, ss2Id);
            INST(15U, Opcode::ReturnInlined).v0id().Inputs(9U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(16U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(SplitDeoptimizationP10, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(5U, Opcode::SaveState);
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(1U).Imm(0U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(17U, Opcode::SaveState);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 17U);
            INST(18U, Opcode::SaveState).Inputs(7U).SrcVregs({1U});
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 18U);

            INST(9U, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(10U, Opcode::CallStatic).v0id().InputsAutoType(9U).Inlined();
            INST(11U, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(12U, Opcode::CallStatic).v0id().InputsAutoType(7U, 11U);
            INST(13U, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(14U, Opcode::DeoptimizeIf).Inputs(0U, 13U);
            INST(15U, Opcode::ReturnInlined).v0id().Inputs(9U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(16U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

SRC_GRAPH(SplitDeoptimizationP11, Graph *graph, uint32_t callInst1Id, uint32_t callInst2Id, uint32_t ssId)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).Inputs(4U).SrcVregs({0U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 5U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(1U).Imm(0U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(9U, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(callInst2Id, Opcode::CallStatic).v0id().InputsAutoType(9U).Inlined();
            INST(ssId, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(callInst1Id, Opcode::CallStatic).v0id().InputsAutoType(ssId).Inlined();
            INST(15U, Opcode::ReturnInlined).v0id().Inputs(ssId);
            INST(14U, Opcode::DeoptimizeIf).Inputs(0U, ssId);
            INST(16U, Opcode::ReturnInlined).v0id().Inputs(9U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(17U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

OUT_GRAPH(SplitDeoptimizationP11, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(5U, Opcode::SaveState);
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(1U).Imm(0U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(17U, Opcode::SaveState);
            INST(22U, Opcode::NewObject).ref().Inputs(6U, 17U);
            INST(18U, Opcode::SaveState).Inputs(22U).SrcVregs({1U});
            INST(20U, Opcode::NewObject).ref().Inputs(3U, 18U);

            INST(9U, Opcode::SaveState).Inputs(20U, 22U).SrcVregs({0U, 1U});
            INST(19U, Opcode::CallStatic).v0id().InputsAutoType(9U).Inlined();
            INST(11U, Opcode::SaveState).Inputs(20U, 22U).SrcVregs({0U, 1U});
            INST(10U, Opcode::CallStatic).v0id().InputsAutoType(11U).Inlined();
            INST(15U, Opcode::ReturnInlined).v0id().Inputs(11U);
            INST(14U, Opcode::DeoptimizeIf).Inputs(0U, 11U);
            INST(16U, Opcode::ReturnInlined).v0id().Inputs(9U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(21U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

SRC_GRAPH(SplitDeoptimizationN1, Graph *graph, uint32_t callInst1Id, uint32_t callInst2Id, uint32_t ss1Id,
          uint32_t ss2Id)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).Inputs(4U).SrcVregs({0U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 5U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(1U).Imm(0U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(9U, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(callInst1Id, Opcode::CallStatic).v0id().InputsAutoType(9U).Inlined();
            INST(ss1Id, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(callInst2Id, Opcode::CallStatic).v0id().InputsAutoType(ss1Id).Inlined();
            INST(ss2Id, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(12U, Opcode::CallStatic).v0id().InputsAutoType(7U, ss2Id);
            INST(14U, Opcode::DeoptimizeIf).Inputs(0U, ss1Id);
            INST(15U, Opcode::ReturnInlined).v0id().Inputs(ss1Id);
            INST(16U, Opcode::ReturnInlined).v0id().Inputs(9U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(17U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

SRC_GRAPH(SplitDeoptimizationN2, Graph *graph, uint32_t callInstId, uint32_t ssId)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(2U, Opcode::SaveState);
            INST(3U, Opcode::LoadAndInitClass).ref().Inputs(2U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).Inputs(4U).SrcVregs({0U});
            INST(6U, Opcode::LoadAndInitClass).ref().Inputs(5U);
            INST(7U, Opcode::NewObject).ref().Inputs(6U, 5U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(1U).Imm(0U);
        }
        BASIC_BLOCK(3U, 4U)
        {
            INST(9U, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(callInstId, Opcode::CallStatic).v0id().InputsAutoType(9U).Inlined();
            INST(ssId, Opcode::SaveState).Inputs(4U, 7U).SrcVregs({0U, 1U});
            INST(15U, Opcode::ReturnInlined).v0id().Inputs(9U);
            INST(14U, Opcode::DeoptimizeIf).Inputs(0U, ssId);
            INST(16U, Opcode::SaveState).SrcVregs({4U, 7U}).Inputs(0U, 1U);
            INST(17U, Opcode::NullCheck).ref().Inputs(7U, 16U);
            INST(18U, Opcode::LoadObject).u64().Inputs(17U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(13U, Opcode::ReturnVoid).v0id();
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

// Positive Test 1 - Just decompose
TEST_F(EscapeAnalysisTest, SplitDeoptimizationP1)
{
    src_graph::SplitDeoptimizationP1::CREATE(GetGraph());
    EscapeAnalysis::DecomposeAnalysis da {GetGraph()};

    auto graph = CreateEmptyGraph();
    out_graph::SplitDeoptimizationP1::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Positive Test 2 - Multiple deoptimization insts without DFG dependency in one basic block
TEST_F(EscapeAnalysisTest, SplitDeoptimizationP2)
{
    src_graph::SplitDeoptimizationP1::CREATE(GetGraph());
    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::SplitDeoptimizationP2::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Positive Test 3 - Multiple deoptimization insts with DFG dependency in one basic block
TEST_F(EscapeAnalysisTest, SplitDeoptimizationP3)
{
    src_graph::SplitDeoptimizationP3::CREATE(GetGraph());
    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::SplitDeoptimizationP3::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Positive Test 4 - Multiple deoptimization insts in different basic blocks
TEST_F(EscapeAnalysisTest, SplitDeoptimizationP4)
{
    src_graph::SplitDeoptimizationP4::CREATE(GetGraph());
    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::SplitDeoptimizationP2::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Positive Test 5 - Materialization occurs
TEST_F(EscapeAnalysisTest, SplitDeoptimizationP5)
{
    src_graph::SplitDeoptimizationP5::CREATE(GetGraph());
    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::SplitDeoptimizationP5::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Positive Test 6 - Multiple NullCheck insts without direct DFG dependency in one basic block
TEST_F(EscapeAnalysisTest, SplitDeoptimizationP6)
{
    src_graph::SplitDeoptimizationP6::CREATE(GetGraph());
    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::SplitDeoptimizationP6::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Positive Test 7 - Multiple NullCheck insts without any DFG dependency in one basic block
TEST_F(EscapeAnalysisTest, SplitDeoptimizationP7)
{
    src_graph::SplitDeoptimizationP7::CREATE(GetGraph());
    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::SplitDeoptimizationP7::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Positive Test 8 - Deoptimization inst is the last inst in one basic block
TEST_F(EscapeAnalysisTest, SplitDeoptimizationP8)
{
    src_graph::SplitDeoptimizationP8::CREATE(GetGraph());
    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    src_graph::SplitDeoptimizationP1::CREATE(graph);
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Positive Test 9 - Deoptimization's savestate inst is in a matched inlined func
TEST_F(EscapeAnalysisTest, SplitDeoptimizationP9)
{
    static constexpr uint32_t CALL_INST_ID = 10;
    static constexpr uint32_t SS1_ID = 11;
    static constexpr uint32_t SS2_ID = 13;
    src_graph::SplitDeoptimizationP9::CREATE(GetGraph(), CALL_INST_ID, SS1_ID, SS2_ID);
    INS(SS1_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST_ID)));
    INS(SS2_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST_ID)));

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::MaterializeBeforeReferencedObjectMaterialization::CREATE(graph);
    INS(SS1_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST_ID)));
    INS(SS2_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST_ID)));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Positive Test 10 - Just no inlined call inst is between deopt inst and its savestate inst
TEST_F(EscapeAnalysisTest, SplitDeoptimizationP10)
{
    static constexpr uint32_t CALL_INST_ID = 10;
    static constexpr uint32_t SS1_ID = 11;
    static constexpr uint32_t SS2_ID = 13;
    src_graph::SplitDeoptimizationP10::CREATE(GetGraph(), CALL_INST_ID, SS1_ID, SS2_ID);
    INS(SS1_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST_ID)));
    INS(SS2_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST_ID)));

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::SplitDeoptimizationP10::CREATE(graph);
    INS(SS1_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST_ID)));
    INS(SS2_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST_ID)));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Positive Test 11 - Call.Inlined and ReturnInlined is match between deopt inst and its savestate inst
TEST_F(EscapeAnalysisTest, SplitDeoptimizationP11)
{
    static constexpr uint32_t CALL_INST1_ID = 10;
    static constexpr uint32_t CALL_INST2_ID = 18;
    static constexpr uint32_t SS_ID = 11;
    src_graph::SplitDeoptimizationP11::CREATE(GetGraph(), CALL_INST1_ID, CALL_INST2_ID, SS_ID);
    INS(SS_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST2_ID)));

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::SplitDeoptimizationP11::CREATE(graph);
    INS(SS_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST2_ID)));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Negative Test 1 - Call.Inlined is mismatch between deopt inst and its savestate inst
TEST_F(EscapeAnalysisTest, SplitDeoptimizationN1)
{
    static constexpr uint32_t CALL_INST1_ID = 10;
    static constexpr uint32_t CALL_INST2_ID = 18;
    static constexpr uint32_t SS1_ID = 11;
    static constexpr uint32_t SS2_ID = 13;
    src_graph::SplitDeoptimizationN1::CREATE(GetGraph(), CALL_INST1_ID, CALL_INST2_ID, SS1_ID, SS2_ID);
    INS(SS1_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST1_ID)));
    INS(SS2_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST2_ID)));

    EscapeAnalysis::DecomposeAnalysis da {GetGraph()};

    auto graph = CreateEmptyGraph();
    src_graph::SplitDeoptimizationN1::CREATE(graph, CALL_INST1_ID, CALL_INST2_ID, SS1_ID, SS2_ID);
    INS(SS1_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST1_ID)));
    INS(SS2_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST2_ID)));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// Negative Test 2 - ReturnInlined is mismatch between deopt inst and its savestate inst
TEST_F(EscapeAnalysisTest, SplitDeoptimizationN2)
{
    static constexpr uint32_t CALL_INST_ID = 10;
    static constexpr uint32_t SS_ID = 11;
    src_graph::SplitDeoptimizationN2::CREATE(GetGraph(), CALL_INST_ID, SS_ID);
    INS(SS_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST_ID)));

    EscapeAnalysis::DecomposeAnalysis da {GetGraph()};

    auto graph = CreateEmptyGraph();
    src_graph::SplitDeoptimizationN2::CREATE(graph, CALL_INST_ID, SS_ID);
    INS(SS_ID).CastToSaveState()->SetCallerInst(static_cast<CallInst *>(&INS(CALL_INST_ID)));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

// CC-OFFNXT(G.FUD.05) long test representation of IR
SRC_GRAPH(PhiAndPhiInputMaterialization, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();

        CONSTANT(2U, 1U).i32();
        CONSTANT(3U, 2U).i32();
        CONSTANT(4U, 3U).i32();

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(20U, Opcode::SaveState);
            INST(21U, Opcode::LoadAndInitClass).ref().Inputs(20U).Class(CUSTOM_CLASS);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(0U).Imm(0U);
        }
        BASIC_BLOCK(3U, 5U)
        {
            INST(30U, Opcode::SaveState);
            INST(31U, Opcode::NewObject).ref().Inputs(21U, 30U).TypeId(CUSTOM_CLASS_ID);
            INST(32U, Opcode::StoreObject).s32().Inputs(31U, 2U).ObjField(INT32_FIELD);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(40U, Opcode::SaveState);
            INST(41U, Opcode::NewObject).ref().Inputs(21U, 40U).TypeId(CUSTOM_CLASS_ID);
            INST(42U, Opcode::StoreObject).s32().Inputs(41U, 3U).ObjField(INT32_FIELD);
        }
        BASIC_BLOCK(5U, 6U, 7U)
        {
            INST(50U, Opcode::Phi).ref().Inputs(31U, 41U, 53U);
            INST(51U, Opcode::Phi).s32().Inputs(2U, 2U, 77U);
            INST(52U, Opcode::SaveState).Inputs(50U, 51U).SrcVregs({0U, 1U});
            INST(53U, Opcode::NewObject).ref().Inputs(21U, 52U).TypeId(CUSTOM_CLASS_ID);
            INST(54U, Opcode::StoreObject).s32().Inputs(53U, 4U).ObjField(INT32_FIELD);
            INST(55U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(1U).Imm(0U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(61U, Opcode::SaveStateDeoptimize).Inputs(50U, 51U, 53U).SrcVregs({0U, 1U, 2U});
            INST(62U, Opcode::Deoptimize).Inputs(61U);
        }
        BASIC_BLOCK(7U, 5U, 8U)
        {
            INST(71U, Opcode::SaveState).Inputs(50U, 51U, 53U).SrcVregs({0U, 1U, 2U});
            INST(72U, Opcode::CallStatic).v0id().InputsAutoType(50U, 53U, 71U).Inlined();
            INST(73U, Opcode::LoadObject).s32().Inputs(50U).ObjField(INT32_FIELD);
            INST(74U, Opcode::LoadObject).s32().Inputs(53U).ObjField(INT32_FIELD);
            INST(75U, Opcode::Add).s32().Inputs(73U, 74U);
            INST(76U, Opcode::ReturnInlined).v0id().Inputs(71U);
            INST(77U, Opcode::Add).s32().Inputs(51U, 75U);
            INST(78U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_GT).Inputs(77U).Imm(30U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(81U, Opcode::Return).i32().Inputs(77U);
        }
        // NOLINTEND(readability-magic-numbers)
    }
}

// CC-OFFNXT(G.FUD.05) long test representation of IR
OUT_GRAPH(PhiAndPhiInputMaterialization, Graph *graph)
{
    GRAPH(graph)
    {
        // NOLINTBEGIN(readability-magic-numbers)
        PARAMETER(0U, 0U).b();
        PARAMETER(1U, 1U).b();

        CONSTANT(2U, 1U).i32();
        CONSTANT(3U, 2U).i32();
        CONSTANT(4U, 3U).i32();
        NULLPTR(115U);

        BASIC_BLOCK(2U, 14U, 4U)
        {
            INST(20U, Opcode::SaveState);
            INST(21U, Opcode::LoadAndInitClass).ref().Inputs(20U).Class(CUSTOM_CLASS);
            INST(22U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(0U).Imm(0U);
        }
        BASIC_BLOCK(4U, 14U) {}
        BASIC_BLOCK(14U, 5U)
        {
            // preheader
            INST(105, Opcode::Phi).i32().Inputs(2U, 3U);
            INST(109U, Opcode::SaveState);
            INST(110U, Opcode::NewObject).Inputs(21U, 109U).ref();
            INST(111U, Opcode::StoreObject).s32().Inputs(110U, 105U).ObjField(INT32_FIELD);
        }
        BASIC_BLOCK(5U, 6U, 7U)
        {
            // First input dominates phi, materialized
            INST(50U, Opcode::Phi).ref().Inputs(110U, 113U);
            INST(51U, Opcode::Phi).s32().Inputs(2U, 77U);
            INST(55U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(1U).Imm(0U);
        }
        BASIC_BLOCK(7U, 5U, 8U)
        {
            INST(71U, Opcode::SaveState).Inputs(50U, 51U).SrcVregs({0U, 1U});
            INST(72U, Opcode::CallStatic).v0id().InputsAutoType(50U, 115U, 71U).Inlined();
            INST(73U, Opcode::LoadObject).s32().Inputs(50U).ObjField(INT32_FIELD);
            INST(75U, Opcode::Add).s32().Inputs(73U, 4U);
            INST(76U, Opcode::ReturnInlined).v0id().Inputs(71U);
            INST(77U, Opcode::Add).s32().Inputs(51U, 75U);
            INST(112U, Opcode::SaveState).Inputs();
            INST(113U, Opcode::NewObject).Inputs(21U, 112U).ref();
            INST(114U, Opcode::StoreObject).s32().Inputs(113U, 4U).ObjField(INT32_FIELD);
            INST(78U, Opcode::IfImm).SrcType(DataType::INT32).CC(CC_GT).Inputs(77U).Imm(30U);
        }
        BASIC_BLOCK(8U, -1L)
        {
            INST(81U, Opcode::Return).i32().Inputs(77U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(106U, Opcode::SaveState).Inputs(50U).SrcVregs({VirtualRegister::BRIDGE});
            INST(107U, Opcode::NewObject).Inputs(21U, 106U).ref();
            INST(108U, Opcode::StoreObject).s32().Inputs(107U, 4U).ObjField(INT32_FIELD);
            // Do not crash when phi (v50) input (new v107) is also input to savestate
            INST(61U, Opcode::SaveStateDeoptimize).Inputs(50U, 51U, 107U).SrcVregs({0U, 1U, 2U});
            INST(62U, Opcode::Deoptimize).Inputs(61U);
        }

        // NOLINTEND(readability-magic-numbers)
    }
}

TEST_F(EscapeAnalysisTest, PhiAndPhiInputMaterialization)
{
    src_graph::PhiAndPhiInputMaterialization::CREATE(GetGraph());

    ASSERT_TRUE(Run());

    auto graph = CreateEmptyGraph();
    out_graph::PhiAndPhiInputMaterialization::CREATE(graph);

    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}
}  // namespace ark::compiler
