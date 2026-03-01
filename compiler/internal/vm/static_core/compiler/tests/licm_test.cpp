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
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/licm.h"

namespace ark::compiler {
class LicmTest : public GraphTest {
public:
    static constexpr uint32_t HOST_LIMIT = 8;

    void HoistResolverBeforeMovableObjectBuildInitialGraph();
    Graph *HoistResolverBeforeMovableObjectBuildExpectedGraph();

    void BuildGraphPreHeaderWithIf();
    void BuildGraphLicmResolver();
    void BuildGraphLicmResolverIfHeaderIsNotExit();
    void BuildGraphHoistLenArray();
    Graph *BuildGraphLoadImmediate(RuntimeInterface::ClassPtr class1);
    Graph *BuildGraphLoadObjFromConst(uintptr_t obj1);
    Graph *BuildGraphFunctionImmediate(uintptr_t fptr);

    Graph *BuildGraphLoadFromConstantPool();
};

// NOLINTBEGIN(readability-magic-numbers)
/*
 * Test Graph:
 *              [0]
 *               |
 *               v
 *        /-----[2]--------\
 *        |      ^         |
 *        |      |         |
 *        |     [6]        |
 *        |      ^         |
 *        |      |         |
 *        |     [5]------->|
 *        |      ^         |
 *        |      |         |
 *        |     [4]        |
 *        |      ^         |
 *        |      |         |
 *        \---->[3]        |
 *               |         |
 *               v         |
 *             [exit]<-----/
 */

TEST_F(LicmTest, LoopExits)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        BASIC_BLOCK(2U, 3U, 7U)
        {
            INST(2U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(3U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(2U);
        }
        BASIC_BLOCK(3U, 4U, 7U)
        {
            INST(4U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(4U);
        }
        BASIC_BLOCK(4U, 5U) {}
        BASIC_BLOCK(5U, 6U, 7U)
        {
            INST(6U, Opcode::Compare).b().Inputs(0U, 1U);
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(6U, 2U) {}
        BASIC_BLOCK(7U, -1L)
        {
            INST(8U, Opcode::ReturnVoid);
        }
    }

    auto licm = Licm(GetGraph(), HOST_LIMIT);
    licm.RunImpl();

    ASSERT_TRUE(licm.IsBlockLoopExit(&BB(3U)));
    ASSERT_TRUE(licm.IsBlockLoopExit(&BB(5U)));

    ASSERT_FALSE(licm.IsBlockLoopExit(&BB(0U)));
    ASSERT_FALSE(licm.IsBlockLoopExit(&BB(1U)));
    ASSERT_FALSE(licm.IsBlockLoopExit(&BB(2U)));
    ASSERT_FALSE(licm.IsBlockLoopExit(&BB(4U)));
    ASSERT_FALSE(licm.IsBlockLoopExit(&BB(6U)));
    ASSERT_FALSE(licm.IsBlockLoopExit(&BB(7U)));
}

/*
 * Test Graph:
 *              [0]
 *               |
 *               v
 *        /-----[2]<----\
 *        |      |      |
 *        |      v      |
 *        |     [3]-----/
 *        |
 *        \---->[4]
 *               |
 *               v
 *             [exit]
 */
TEST_F(LicmTest, OneLoop)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 1U);
        CONSTANT(1U, 10U);
        CONSTANT(2U, 20U);
        CONSTANT(12U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Phi).u64().Inputs({{0U, 0U}, {3U, 7U}});
            INST(4U, Opcode::Phi).u64().Inputs({{0U, 1U}, {3U, 8U}});
            INST(5U, Opcode::Compare).b().Inputs(4U, 0U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }

        BASIC_BLOCK(3U, 2U)
        {
            INST(7U, Opcode::Mul).u64().Inputs(3U, 4U);
            INST(13U, Opcode::Mul).u64().Inputs(12U, 0U);
            INST(8U, Opcode::Sub).u64().Inputs(4U, 13U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(10U, Opcode::Add).u64().Inputs(2U, 3U);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(15U, Opcode::CallStatic).u64().InputsAutoType(3U, 4U, 20U);
            INST(11U, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<Licm>(HOST_LIMIT);
    ASSERT_EQ(INS(13U).GetBasicBlock(), BB(3U).GetLoop()->GetPreHeader());
    ASSERT_EQ(INS(7U).GetBasicBlock(), &BB(3U));
    ASSERT_EQ(INS(8U).GetBasicBlock(), &BB(3U));

    GraphChecker(GetGraph()).Check();
}

/*
 * NOTE (a.popov) Improve Licm to support this test with updated DF: `INST(19, Opcode::Phi).u64().Inputs(1, 18)`
 *
 * Test Graph:
 *              [0]
 *               |
 *               v
 *              [2]<----------\
 *               |            |
 *               v            |
 *        /-----[3]<----\     |
 *        |      |      |     |
 *        |      v      |     |
 *        |     [4]-----/    [6]
 *        |                   |
 *        \---->[5]-----------/
 *               |
 *               v
 *             [exit]
 */
TEST_F(LicmTest, DISABLED_TwoLoops)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 10U).u64();
        PARAMETER(3U, 100U).u64();
        BASIC_BLOCK(2U, 3U) {}
        BASIC_BLOCK(3U, 4U, 5U)
        {
            INST(5U, Opcode::Phi).u64().Inputs(2U, 9U);
            INST(6U, Opcode::Phi).u64().Inputs(3U, 11U);
            INST(19U, Opcode::Phi).u64().Inputs(1U, 18U);
            INST(7U, Opcode::Compare).b().Inputs(5U, 6U);
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }
        BASIC_BLOCK(4U, 3U)
        {
            INST(9U, Opcode::Mul).u64().Inputs(5U, 1U);
            INST(10U, Opcode::Mul).u64().Inputs(1U, 2U);
            INST(18U, Opcode::Mul).u64().Inputs(10U, 10U);
            INST(11U, Opcode::Sub).u64().Inputs(6U, 1U);
        }

        BASIC_BLOCK(5U, 6U, 7U)
        {
            INST(13U, Opcode::Compare).b().Inputs(6U, 1U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(6U, 2U) {}
        BASIC_BLOCK(7U, -1L)
        {
            INST(16U, Opcode::Add).u64().Inputs(19U, 0U);
            INST(17U, Opcode::ReturnVoid);
        }
    }

    GetGraph()->RunPass<Licm>(HOST_LIMIT);
    ASSERT_EQ(INS(10U).GetBasicBlock(), BB(2U).GetLoop()->GetPreHeader());
    ASSERT_EQ(INS(18U).GetBasicBlock(), BB(2U).GetLoop()->GetPreHeader());
    ASSERT_EQ(INS(9U).GetBasicBlock(), &BB(4U));
    ASSERT_EQ(INS(11U).GetBasicBlock(), &BB(4U));
    GraphChecker(GetGraph()).Check();
}

TEST_F(LicmTest, EmptyPreHeaderWithPhi)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 4U) {}
        BASIC_BLOCK(4U, 5U)
        {
            INST(6U, Opcode::Phi).u64().Inputs(0U, 1U);
        }
        BASIC_BLOCK(5U, 5U, 6U)
        {
            INST(7U, Opcode::Phi).u64().Inputs(0U, 8U);
            INST(8U, Opcode::Add).u64().Inputs(7U, 1U);
            INST(9U, Opcode::Mul).u64().Inputs(6U, 6U);
            INST(10U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).CC(CC_LT).Inputs(8U, 9U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(6U, -1L)
        {
            INST(12U, Opcode::Return).u64().Inputs(8U);
        }
    }

    GetGraph()->RunPass<Licm>(HOST_LIMIT);
    ASSERT_EQ(INS(9U).GetBasicBlock(), BB(5U).GetLoop()->GetPreHeader());
}

void LicmTest::BuildGraphPreHeaderWithIf()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(5U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(7U, Opcode::Phi).u64().Inputs(0U, 8U);
            INST(8U, Opcode::Add).u64().Inputs(7U, 1U);
            INST(9U, Opcode::Mul).u64().Inputs(0U, 0U);
            INST(10U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).CC(CC_LT).Inputs(8U, 9U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(12U, Opcode::Phi).u64().Inputs(0U, 8U);
            INST(13U, Opcode::Return).u64().Inputs(12U);
        }
    }
    GetGraph()->RunPass<Licm>(HOST_LIMIT);
}

TEST_F(LicmTest, PreHeaderWithIf)
{
    BuildGraphPreHeaderWithIf();
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        CONSTANT(1U, 1U);

        BASIC_BLOCK(2U, 5U, 4U)
        {
            INST(5U, Opcode::IfImm).SrcType(DataType::UINT64).CC(CC_NE).Imm(0U).Inputs(0U);
        }
        BASIC_BLOCK(5U, 3U)
        {
            INST(9U, Opcode::Mul).u64().Inputs(0U, 0U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(7U, Opcode::Phi).u64().Inputs(0U, 8U);
            INST(8U, Opcode::Add).u64().Inputs(7U, 1U);
            INST(10U, Opcode::Compare).b().SrcType(DataType::Type::UINT64).CC(CC_LT).Inputs(8U, 9U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(12U, Opcode::Phi).u64().Inputs(0U, 8U);
            INST(13U, Opcode::Return).u64().Inputs(12U);
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void LicmTest::BuildGraphLicmResolver()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(20U, 0xaU);

        BASIC_BLOCK(5U, 2U)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(1U, Opcode::SaveStateDeoptimize).NoVregs();
            // We can safely hoist ResolveVirtual (INST[8]) into BLOCK[5] and link it to SaveState (INST[5])
            // as INST[6] produces a reference, which is never moved by GC (note that ResolveVirtual calls
            // runtime and may trigger GC).
        }
        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::ResolveVirtual).ptr().Inputs(4U, 7U);
            INST(9U, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8U}, {DataType::REFERENCE, 4U}, {DataType::NO_TYPE, 7U}});
            INST(10U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::NullCheck).ref().Inputs(0U, 10U);
            INST(12U, Opcode::LoadObject).s32().Inputs(11U).TypeId(122U);
            INST(13U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12U, 20U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(15U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(LicmTest, LicmResolver)
{
    BuildGraphLicmResolver();
    ASSERT_TRUE(GetGraph()->RunPass<Licm>(HOST_LIMIT));
    GetGraph()->RunPass<Cleanup>();

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(20U, 0xaU);

        BASIC_BLOCK(5U, 2U)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(8U, Opcode::ResolveVirtual).ptr().Inputs(4U, 5U);
            INST(1U, Opcode::SaveStateDeoptimize).NoVregs();
        }
        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8U}, {DataType::REFERENCE, 4U}, {DataType::NO_TYPE, 7U}});
            INST(10U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::NullCheck).ref().Inputs(0U, 10U);
            INST(12U, Opcode::LoadObject).s32().Inputs(11U).TypeId(122U);
            INST(13U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12U, 20U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(15U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

void LicmTest::BuildGraphLicmResolverIfHeaderIsNotExit()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(20U, 0xaU);

        BASIC_BLOCK(5U, 2U)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(1U, Opcode::SaveStateDeoptimize).NoVregs();
        }
        BASIC_BLOCK(2U, 3U)
        {
            INST(10U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::NullCheck).ref().Inputs(0U, 10U);
            INST(12U, Opcode::LoadObject).s32().Inputs(11U).TypeId(122U);
        }
        BASIC_BLOCK(3U, 2U, 4U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::ResolveVirtual).ptr().Inputs(4U, 7U);
            INST(9U, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8U}, {DataType::REFERENCE, 4U}, {DataType::NO_TYPE, 7U}});
            INST(13U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12U, 20U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(15U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(LicmTest, LicmResolverIfHeaderIsNotExit)
{
    BuildGraphLicmResolverIfHeaderIsNotExit();
    ASSERT_TRUE(GetGraph()->RunPass<Licm>(HOST_LIMIT));

    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(20U, 0xaU);

        BASIC_BLOCK(5U, 2U)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(8U, Opcode::ResolveVirtual).ptr().Inputs(4U, 5U);
            INST(1U, Opcode::SaveStateDeoptimize).NoVregs();
        }
        BASIC_BLOCK(2U, 3U)
        {
            INST(10U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::NullCheck).ref().Inputs(0U, 10U);
            INST(12U, Opcode::LoadObject).s32().Inputs(11U).TypeId(122U);
        }
        BASIC_BLOCK(3U, 2U, 4U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8U}, {DataType::REFERENCE, 4U}, {DataType::NO_TYPE, 7U}});
            INST(13U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12U, 20U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(15U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

TEST_F(LicmTest, DontLicmResolverIfHeaderIsExit)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(20U, 0xaU);

        BASIC_BLOCK(5U, 2U)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(1U, Opcode::SaveStateDeoptimize).NoVregs();
        }
        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(10U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::NullCheck).ref().Inputs(0U, 10U);
            INST(12U, Opcode::LoadObject).s32().Inputs(11U).TypeId(122U);
            INST(13U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12U, 20U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(3U, 2U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::ResolveVirtual).ptr().Inputs(4U, 7U);
            INST(9U, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8U}, {DataType::REFERENCE, 4U}, {DataType::NO_TYPE, 7U}});
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(15U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Licm>(HOST_LIMIT));
}

TEST_F(LicmTest, DontLicmResolverIfNoAppropriateSaveState)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(20U, 0xaU);

        BASIC_BLOCK(5U, 2U)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            // We must not hoist ResolveVirtual (INST[8]) into BLOCK[5] and link it to SaveState (INST[2]).
            // Otherwise a reference produced by INST[4] might be moved by GC as ResolveVirtual calls runtime.
            INST(1U, Opcode::SaveStateDeoptimize).NoVregs();
        }
        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::ResolveVirtual).ptr().Inputs(4U, 7U);
            INST(9U, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8U}, {DataType::REFERENCE, 4U}, {DataType::NO_TYPE, 7U}});
            INST(10U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::NullCheck).ref().Inputs(0U, 10U);
            INST(12U, Opcode::LoadObject).s32().Inputs(11U).TypeId(122U);
            INST(13U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12U, 20U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(15U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Licm>(HOST_LIMIT));
}

TEST_F(LicmTest, DontLicmResolverThroughTry)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(20U, 0xaU);

        BASIC_BLOCK(5U, 2U)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(1U, Opcode::SaveStateDeoptimize).NoVregs();
        }
        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::ResolveVirtual).ptr().Inputs(4U, 7U);
            INST(9U, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8U}, {DataType::REFERENCE, 4U}, {DataType::NO_TYPE, 7U}});
            INST(10U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::NullCheck).ref().Inputs(0U, 10U);
            INST(12U, Opcode::LoadObject).s32().Inputs(11U).TypeId(122U);
            INST(13U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12U, 20U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(15U, Opcode::ReturnVoid).v0id();
        }
    }
    BB(2U).SetTry(true);
    ASSERT_FALSE(GetGraph()->RunPass<Licm>(HOST_LIMIT));
}

TEST_F(LicmTest, DontLicmResolverThroughInitClass)
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(20U, 0xaU);

        BASIC_BLOCK(5U, 2U)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(1U, Opcode::SaveStateDeoptimize).NoVregs();
        }
        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(100U, Opcode::SaveState).NoVregs();
            INST(101U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(100U).TypeId(0U);
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::ResolveVirtual).ptr().Inputs(4U, 7U);
            INST(9U, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8U}, {DataType::REFERENCE, 4U}, {DataType::NO_TYPE, 7U}});
            INST(10U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::NullCheck).ref().Inputs(0U, 10U);
            INST(12U, Opcode::LoadObject).s32().Inputs(11U).TypeId(122U);
            INST(13U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12U, 20U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(15U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_FALSE(GetGraph()->RunPass<Licm>(HOST_LIMIT));
}

void LicmTest::HoistResolverBeforeMovableObjectBuildInitialGraph()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(20U, 0xaU);

        BASIC_BLOCK(5U, 2U)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(5U).TypeId(0U);
            INST(16U, Opcode::NewObject).ref().Inputs(6U, 5U);
            INST(17U, Opcode::LoadObject).ref().Inputs(16U).TypeId(1U);
            INST(1U, Opcode::SaveStateDeoptimize).NoVregs();
            // We can safely hoist ResolveVirtual (INST[8]) into BLOCK[5] before INST[6] and link it to SaveState
            // (INST[5])
        }
        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::ResolveVirtual).ptr().Inputs(4U, 7U);
            INST(9U, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8U},
                         {DataType::REFERENCE, 4U},
                         {DataType::REFERENCE, 17U},
                         {DataType::NO_TYPE, 7U}});
            INST(10U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::NullCheck).ref().Inputs(0U, 10U);
            INST(12U, Opcode::LoadObject).s32().Inputs(11U).TypeId(122U);
            INST(13U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12U, 20U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(15U, Opcode::ReturnVoid).v0id();
        }
    }
}

Graph *LicmTest::HoistResolverBeforeMovableObjectBuildExpectedGraph()
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(20U, 0xaU);

        BASIC_BLOCK(5U, 2U)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(2U).TypeId(0U);
            INST(4U, Opcode::NewObject).ref().Inputs(3U, 2U);
            INST(5U, Opcode::SaveState).NoVregs();
            INST(6U, Opcode::UnresolvedLoadAndInitClass).ref().Inputs(5U).TypeId(0U);
            INST(8U, Opcode::ResolveVirtual).ptr().Inputs(4U, 5U);
            INST(16U, Opcode::NewObject).ref().Inputs(6U, 5U);
            INST(17U, Opcode::LoadObject).ref().Inputs(16U).TypeId(1U);
            INST(1U, Opcode::SaveStateDeoptimize).NoVregs();
            // We can safely hoist ResolveVirtual (INST[8]) into BLOCK[5] before INST[6] and link it to SaveState
            // (INST[5])
        }
        BASIC_BLOCK(2U, 2U, 3U)
        {
            INST(7U, Opcode::SaveState).NoVregs();
            INST(9U, Opcode::CallResolvedVirtual)
                .v0id()
                .Inputs({{DataType::POINTER, 8U},
                         {DataType::REFERENCE, 4U},
                         {DataType::REFERENCE, 17U},
                         {DataType::NO_TYPE, 7U}});
            INST(10U, Opcode::SaveState).NoVregs();
            INST(11U, Opcode::NullCheck).ref().Inputs(0U, 10U);
            INST(12U, Opcode::LoadObject).s32().Inputs(11U).TypeId(122U);
            INST(13U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(12U, 20U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(15U, Opcode::ReturnVoid).v0id();
        }
    }
    return graph;
}

TEST_F(LicmTest, HoistResolverBeforeMovableObject)
{
    HoistResolverBeforeMovableObjectBuildInitialGraph();

    ASSERT_TRUE(GetGraph()->RunPass<Licm>(HOST_LIMIT));
    GetGraph()->RunPass<Cleanup>();

    auto expected = HoistResolverBeforeMovableObjectBuildExpectedGraph();
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), expected));
}

TEST_F(LicmTest, DontHoistLenArray)
{
    // If ChecksElimination removed NullCheck based on BoundsAnalysis, we can hoist LenArray
    // only if the array cannot be null in the loop preheader
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);
        CONSTANT(2U, nullptr).ref();

        BASIC_BLOCK(2U, 3U)
        {
            INST(7U, Opcode::Compare).b().SrcType(DataType::REFERENCE).CC(CC_NE).Inputs(0U, 2U);
            INST(3U, Opcode::SaveStateDeoptimize).NoVregs();
        }

        BASIC_BLOCK(3U, 4U, 6U)
        {
            INST(4U, Opcode::SaveState).NoVregs();
            INST(5U, Opcode::CallStatic).b().InputsAutoType(4U);
            INST(6U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(5U);
        }

        BASIC_BLOCK(4U, 3U, 5U)
        {
            INST(8U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(7U);
        }

        BASIC_BLOCK(5U, 3U)
        {
            INST(9U, Opcode::SaveState).NoVregs();
            INST(10U, Opcode::LenArray).s32().Inputs(0U);
            INST(11U, Opcode::BoundsCheck).s32().Inputs(10U, 1U, 9U);
            INST(12U, Opcode::StoreArray).s32().Inputs(0U, 11U, 1U);
        }

        BASIC_BLOCK(6U, -1L)
        {
            INST(13U, Opcode::ReturnVoid).v0id();
        }
    }

    auto clone = GraphCloner(GetGraph(), GetGraph()->GetAllocator(), GetGraph()->GetLocalAllocator()).CloneGraph();
    ASSERT_FALSE(GetGraph()->RunPass<Licm>(HOST_LIMIT));
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), clone));
}

void LicmTest::BuildGraphHoistLenArray()
{
    GRAPH(GetGraph())
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::SaveStateDeoptimize).NoVregs();
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
        }

        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(6U, Opcode::SaveState).NoVregs();
            INST(7U, Opcode::LenArray).s32().Inputs(4U);
            INST(8U, Opcode::BoundsCheck).s32().Inputs(7U, 1U, 6U);
            INST(9U, Opcode::StoreArray).s32().Inputs(0U, 8U, 1U);
            INST(10U, Opcode::CallStatic).b().InputsAutoType(6U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
}

TEST_F(LicmTest, HoistLenArray)
{
    BuildGraphHoistLenArray();
    ASSERT_TRUE(GetGraph()->RunPass<Licm>(HOST_LIMIT));
    auto graph = CreateEmptyGraph();

    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        CONSTANT(1U, 0U);

        BASIC_BLOCK(2U, 3U)
        {
            INST(3U, Opcode::SaveStateDeoptimize).NoVregs();
            INST(4U, Opcode::NullCheck).ref().Inputs(0U, 3U).SetFlag(inst_flags::CAN_DEOPTIMIZE);
            INST(7U, Opcode::LenArray).s32().Inputs(4U);
        }

        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(6U, Opcode::SaveState).NoVregs();
            INST(8U, Opcode::BoundsCheck).s32().Inputs(7U, 1U, 6U);
            INST(9U, Opcode::StoreArray).s32().Inputs(0U, 8U, 1U);
            INST(10U, Opcode::CallStatic).b().InputsAutoType(6U);
            INST(11U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(10U);
        }

        BASIC_BLOCK(4U, -1L)
        {
            INST(12U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(GetGraph(), graph));
}

Graph *LicmTest::BuildGraphLoadImmediate(RuntimeInterface::ClassPtr class1)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(40U, 0xaU);
        CONSTANT(41U, 0xbU);
        BASIC_BLOCK(2U, 5U) {}
        BASIC_BLOCK(5U, 4U, 3U)
        {
            INST(13U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(40U, 41U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(2U, Opcode::LoadImmediate).ref().Class(class1);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(15U, Opcode::CallStatic).b().InputsAutoType(2U, 20U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(17U, Opcode::ReturnVoid);
        }
    }
    return graph;
}

TEST_F(LicmTest, LoadImmediate)
{
    auto class1 = reinterpret_cast<RuntimeInterface::ClassPtr>(1U);
    auto graph = BuildGraphLoadImmediate(class1);
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(40U, 0xaU);
        CONSTANT(41U, 0xbU);
        BASIC_BLOCK(2U, 5U)
        {
            INST(13U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(40U, 41U);
            INST(2U, Opcode::LoadImmediate).ref().Class(class1);
        }
        BASIC_BLOCK(5U, 4U, 3U)
        {
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(20U, Opcode::SaveState).NoVregs();
            INST(15U, Opcode::CallStatic).b().InputsAutoType(2U, 20U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(17U, Opcode::ReturnVoid);
        }
    }
    GraphChecker(graph).Check();
    ASSERT_TRUE(graph->RunPass<Licm>(HOST_LIMIT));
    ASSERT_TRUE(GraphComparator().Compare(graph, graph1));
}

Graph *LicmTest::BuildGraphLoadObjFromConst(uintptr_t obj1)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(40U, 0xaU);
        CONSTANT(41U, 0xbU);
        BASIC_BLOCK(2U, 5U) {}
        BASIC_BLOCK(5U, 4U, 3U)
        {
            INST(13U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(40U, 41U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(2U, Opcode::LoadObjFromConst).ref().SetPtr(obj1);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(15U, Opcode::CallStatic).b().InputsAutoType(2U, 20U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(17U, Opcode::ReturnVoid);
        }
    }
    return graph;
}

TEST_F(LicmTest, LoadObjFromConst)
{
    auto obj1 = static_cast<uintptr_t>(1U);
    auto graph = BuildGraphLoadObjFromConst(obj1);
    auto graph1 = CreateEmptyGraph();
    GRAPH(graph1)
    {
        CONSTANT(40U, 0xaU);
        CONSTANT(41U, 0xbU);
        BASIC_BLOCK(2U, 5U)
        {
            INST(13U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(40U, 41U);
            INST(2U, Opcode::LoadObjFromConst).ref().SetPtr(obj1);
        }
        BASIC_BLOCK(5U, 4U, 3U)
        {
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(20U, Opcode::SaveState).Inputs(2U).SrcVregs({VirtualRegister::BRIDGE});
            INST(15U, Opcode::CallStatic).b().InputsAutoType(2U, 20U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(17U, Opcode::ReturnVoid);
        }
    }
    GraphChecker(graph).Check();
    ASSERT_TRUE(graph->RunPass<Licm>(HOST_LIMIT));
    ASSERT_TRUE(GraphComparator().Compare(graph, graph1));
}

Graph *LicmTest::BuildGraphFunctionImmediate(uintptr_t fptr)
{
    auto graph = CreateEmptyGraph();
    graph->SetDynamicMethod();
#ifndef NDEBUG
    graph->SetDynUnitTestFlag();
#endif
    GRAPH(graph)
    {
        CONSTANT(40U, 0xaU);
        CONSTANT(41U, 0xbU);
        BASIC_BLOCK(2U, 5U) {}
        BASIC_BLOCK(5U, 4U, 3U)
        {
            INST(13U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(40U, 41U);
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(2U, Opcode::FunctionImmediate).any().SetPtr(fptr);
            INST(20U, Opcode::SaveState).NoVregs();
            INST(15U, Opcode::CallStatic).b().InputsAutoType(2U, 20U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(17U, Opcode::ReturnVoid);
        }
    }
    return graph;
}

TEST_F(LicmTest, FunctionImmediate)
{
    auto fptr = static_cast<uintptr_t>(1U);
    auto graph = BuildGraphFunctionImmediate(fptr);
    auto graph1 = CreateEmptyGraph();
    graph1->SetDynamicMethod();
#ifndef NDEBUG
    graph1->SetDynUnitTestFlag();
#endif
    GRAPH(graph1)
    {
        CONSTANT(40U, 0xaU);
        CONSTANT(41U, 0xbU);
        BASIC_BLOCK(2U, 5U)
        {
            INST(13U, Opcode::Compare).b().SrcType(DataType::INT32).CC(CC_GE).Inputs(40U, 41U);
            INST(2U, Opcode::FunctionImmediate).any().SetPtr(fptr);
        }
        BASIC_BLOCK(5U, 4U, 3U)
        {
            INST(14U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(13U);
        }
        BASIC_BLOCK(4U, 5U)
        {
            INST(20U, Opcode::SaveState).Inputs(2U).SrcVregs({VirtualRegister::BRIDGE});
            INST(15U, Opcode::CallStatic).b().InputsAutoType(2U, 20U);
        }
        BASIC_BLOCK(3U, -1L)
        {
            INST(17U, Opcode::ReturnVoid);
        }
    }
    GraphChecker(graph).Check();
    ASSERT_TRUE(graph->RunPass<Licm>(HOST_LIMIT));
    ASSERT_TRUE(GraphComparator().Compare(graph, graph1));
}

Graph *LicmTest::BuildGraphLoadFromConstantPool()
{
    auto graph = CreateEmptyGraph();
    graph->SetDynamicMethod();
#ifndef NDEBUG
    graph->SetDynUnitTestFlag();
#endif
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).any();
        PARAMETER(1U, 1U).any();
        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::LoadConstantPool).any().Inputs(0U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            INST(9U, Opcode::Intrinsic).any().Inputs({{DataType::ANY, 1U}, {DataType::NO_TYPE, 3U}});
            INST(4U, Opcode::LoadFromConstantPool).any().Inputs(2U).TypeId(1U);
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U, 4U, 9U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(6U, Opcode::Intrinsic).b().Inputs({{DataType::ANY, 4U}, {DataType::ANY, 9U}, {DataType::NO_TYPE, 5U}});
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::ReturnVoid).v0id();
        }
    }
    return graph;
}

TEST_F(LicmTest, LoadFromConstantPool)
{
    auto graph = BuildGraphLoadFromConstantPool();
    ASSERT_TRUE(graph->RunPass<Licm>(HOST_LIMIT));

    auto graph1 = CreateEmptyGraph();
    graph1->SetDynamicMethod();
#ifndef NDEBUG
    graph1->SetDynUnitTestFlag();
#endif
    GRAPH(graph1)
    {
        PARAMETER(0U, 0U).any();
        PARAMETER(1U, 1U).any();
        BASIC_BLOCK(2U, 3U)
        {
            INST(2U, Opcode::LoadConstantPool).any().Inputs(0U);
            INST(4U, Opcode::LoadFromConstantPool).any().Inputs(2U).TypeId(1U);
        }
        BASIC_BLOCK(3U, 3U, 4U)
        {
            INST(3U, Opcode::SaveState).Inputs(0U, 1U, 2U, 4U).SrcVregs({0U, 1U, 2U, VirtualRegister::BRIDGE});
            INST(9U, Opcode::Intrinsic).any().Inputs({{DataType::ANY, 1U}, {DataType::NO_TYPE, 3U}});
            INST(5U, Opcode::SaveState).Inputs(0U, 1U, 2U, 4U, 9U).SrcVregs({0U, 1U, 2U, 3U, 4U});
            INST(6U, Opcode::Intrinsic).b().Inputs({{DataType::ANY, 4U}, {DataType::ANY, 9U}, {DataType::NO_TYPE, 5U}});
            INST(7U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_NE).Imm(0U).Inputs(6U);
        }
        BASIC_BLOCK(4U, -1L)
        {
            INST(8U, Opcode::ReturnVoid).v0id();
        }
    }
    ASSERT_TRUE(GraphComparator().Compare(graph, graph1));
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
