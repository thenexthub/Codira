/*
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

#include "unit_test.h"
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "optimizer/optimizations/regalloc/reg_alloc_graph_coloring.h"

namespace ark::compiler {
class RegAllocCommonTest : public GraphTest {
public:
    template <typename Checker>
    void RunRegAllocatorsAndCheck(Graph *graph, Checker checker) const
    {
        RunRegAllocatorsAndCheck(graph, graph->GetArchUsedRegs(), checker);
    }

    template <typename Checker>
    void RunRegAllocatorsAndCheck(Graph *graph, RegMask mask, Checker checker) const
    {
        if (graph->GetCallingConvention() == nullptr) {
            return;
        }
        auto graphLs = GraphCloner(graph, graph->GetAllocator(), graph->GetLocalAllocator()).CloneGraph();
        auto rals = RegAllocLinearScan(graphLs);
        rals.SetRegMask(mask);
        ASSERT_TRUE(rals.Run());
        checker(graphLs);

        // RegAllocGraphColoring is not supported for AARCH32
        if (GetGraph()->GetArch() == Arch::AARCH32) {
            return;
        }
        auto graphGc = GraphCloner(graph, graph->GetAllocator(), graph->GetLocalAllocator()).CloneGraph();
        auto ragc = RegAllocGraphColoring(graphGc);
        ragc.SetRegMask(mask);
        ASSERT_TRUE(ragc.Run());
        checker(graphGc);
    }

protected:
    template <DataType::Type REG_TYPE>
    void TestParametersLocations() const;
};

// NOLINTBEGIN(readability-magic-numbers)
SRC_GRAPH(TestParametersLocationsUINT64, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).u64();
        PARAMETER(2U, 2U).u64();
        PARAMETER(3U, 3U).u64();
        PARAMETER(4U, 4U).u64();
        PARAMETER(5U, 5U).u64();
        PARAMETER(6U, 6U).u64();
        PARAMETER(7U, 7U).u64();
        PARAMETER(8U, 8U).u64();
        PARAMETER(9U, 9U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(30U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(11U, Opcode::NullCheck).ref().Inputs(0U, 30U);
            INST(12U, Opcode::StoreObject).u64().Inputs(11U, 1U);
            INST(13U, Opcode::StoreObject).u64().Inputs(11U, 2U);
            INST(14U, Opcode::StoreObject).u64().Inputs(11U, 3U);
            INST(15U, Opcode::StoreObject).u64().Inputs(11U, 4U);
            INST(16U, Opcode::StoreObject).u64().Inputs(11U, 5U);
            INST(17U, Opcode::StoreObject).u64().Inputs(11U, 6U);
            INST(18U, Opcode::StoreObject).u64().Inputs(11U, 7U);
            INST(19U, Opcode::StoreObject).u64().Inputs(11U, 8U);
            INST(20U, Opcode::StoreObject).u64().Inputs(11U, 9U);
            INST(31U, Opcode::ReturnVoid).v0id();
        }
    }
}
SRC_GRAPH(TestParametersLocationsUINT32, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).u32();
        PARAMETER(2U, 2U).u32();
        PARAMETER(3U, 3U).u32();
        PARAMETER(4U, 4U).u32();
        PARAMETER(5U, 5U).u32();
        PARAMETER(6U, 6U).u32();
        PARAMETER(7U, 7U).u32();
        PARAMETER(8U, 8U).u32();
        PARAMETER(9U, 9U).u32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(30U, Opcode::SaveState).Inputs(0U).SrcVregs({0U});
            INST(11U, Opcode::NullCheck).ref().Inputs(0U, 30U);
            INST(12U, Opcode::StoreObject).u32().Inputs(11U, 1U);
            INST(13U, Opcode::StoreObject).u32().Inputs(11U, 2U);
            INST(14U, Opcode::StoreObject).u32().Inputs(11U, 3U);
            INST(15U, Opcode::StoreObject).u32().Inputs(11U, 4U);
            INST(16U, Opcode::StoreObject).u32().Inputs(11U, 5U);
            INST(17U, Opcode::StoreObject).u32().Inputs(11U, 6U);
            INST(18U, Opcode::StoreObject).u32().Inputs(11U, 7U);
            INST(19U, Opcode::StoreObject).u32().Inputs(11U, 8U);
            INST(20U, Opcode::StoreObject).u32().Inputs(11U, 9U);
            INST(31U, Opcode::ReturnVoid).v0id();
        }
    }
}

template <DataType::Type REG_TYPE>
void RegAllocCommonTest::TestParametersLocations() const
{
    auto graph = CreateEmptyGraph();
    if constexpr (DataType::UINT64 == REG_TYPE) {
        src_graph::TestParametersLocationsUINT64::CREATE(graph);
    } else {
        src_graph::TestParametersLocationsUINT32::CREATE(graph);
    }

    RunRegAllocatorsAndCheck(graph, [type = REG_TYPE](Graph *checkGraph) {
        auto arch = checkGraph->GetArch();
        unsigned slotInc = Is64Bits(type, arch) && !Is64BitsArch(arch) ? 2U : 1U;

        unsigned paramsOnRegisters = 0;
        if (Arch::AARCH64 == checkGraph->GetArch()) {
            /**
             * Test case for Arch::AARCH64:
             *
             * - Parameters [arg0 - arg6] are placed in the registers [r1-r7]
             * - All other Parameters are placed in stack slots [slot0 - ...]
             */
            paramsOnRegisters = 7;
        } else if (Arch::AARCH32 == checkGraph->GetArch()) {
            /**
             * Test case for Arch::AARCH32:
             * - ref-Parameter (arg0) is placed in the r1 register
             * - If arg1 is 64-bit Parameter, it is placed in the [r2-r3] registers
             * - If arg1, arg2 are 32-bit Parameters, they are placed in the [r2-r3] registers
             * - All other Parameters are placed in stack slots [slot0 - ...]
             */
            paramsOnRegisters = (type == DataType::UINT64) ? 2U : 3U;
        }

        std::map<Location, Inst *> assignedLocations;
        unsigned index = 0;
        unsigned argSlot = 0;
        for (auto paramInst : checkGraph->GetParameters()) {
            // Check intial locations
            auto srcLocation = paramInst->CastToParameter()->GetLocationData().GetSrc();
            if (index < paramsOnRegisters) {
                EXPECT_EQ(srcLocation.GetKind(), LocationType::REGISTER);
                EXPECT_EQ(srcLocation.GetValue(), index + 1U);
            } else {
                EXPECT_EQ(srcLocation.GetKind(), LocationType::STACK_PARAMETER);
                EXPECT_EQ(srcLocation.GetValue(), argSlot);
                argSlot += slotInc;
            }

            // Check that assigned locations do not overlap
            auto dstLocation = paramInst->CastToParameter()->GetLocationData().GetDst();
            EXPECT_EQ(assignedLocations.count(dstLocation), 0U);
            assignedLocations.insert({dstLocation, paramInst});

            index++;
        }
    });
}

TEST_F(RegAllocCommonTest, ParametersLocation)
{
    TestParametersLocations<DataType::UINT64>();
    TestParametersLocations<DataType::UINT32>();
}

static void LocationsNoSplitsCheckBB(BasicBlock *bb)
{
    for (auto inst : bb->AllInsts()) {
        EXPECT_FALSE(inst->IsSpillFill());
        if (inst->NoDest()) {
            ASSERT(inst->IsSaveState() || inst->GetOpcode() == Opcode::Return);
        } else {
            EXPECT_NE(inst->GetDstReg(), INVALID_REG);
        }
    }
}

TEST_F(RegAllocCommonTest, LocationsNoSplits)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).s32();
        CONSTANT(1U, 2U).s32();
        CONSTANT(2U, 3U).s32();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::Add).u32().Inputs(0U, 1U);
            INST(4U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(5U, Opcode::CallStatic).InputsAutoType(3U, 2U, 0U, 1U, 4U).u32();
            INST(6U, Opcode::Return).u32().Inputs(5U);
        }
    }

    if (graph->GetArch() == Arch::AARCH32) {
        // Enable after full registers mask support the ARM32
        return;
    }

    // Check that there are no spill-fills in the graph
    RunRegAllocatorsAndCheck(graph, [](Graph *checkGraph) {
        for (auto bb : checkGraph->GetBlocksRPO()) {
            LocationsNoSplitsCheckBB(bb);
        }
    });
}

TEST_F(RegAllocCommonTest, ImplicitNullCheckStackMap)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).ref();

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).NoVregs();
            INST(4U, Opcode::LoadAndInitClass).ref().Inputs(3U);
            INST(5U, Opcode::NewObject).ref().Inputs(4U, 3U);
            INST(6U, Opcode::SaveState).Inputs(0U, 1U).SrcVregs({0U, 1U});
            INST(7U, Opcode::NullCheck).ref().Inputs(5U, 6U);
            INST(8U, Opcode::LoadObject).s32().Inputs(7U);
            INST(9U, Opcode::Return).s32().Inputs(8U);
        }
    }
    INS(7U).CastToNullCheck()->SetImplicit(true);

    RunRegAllocatorsAndCheck(graph, [](Graph *checkGraph) {
        auto bb = checkGraph->GetStartBlock()->GetSuccessor(0U);
        // Find null_check
        Inst *nullCheck = nullptr;
        for (auto inst : bb->AllInsts()) {
            if (inst->IsNullCheck()) {
                nullCheck = inst;
                break;
            }
        }
        ASSERT(nullCheck != nullptr);
        auto saveState = nullCheck->GetSaveState();
        // Check that save_state's inputs are added to the roots
        auto roots = saveState->GetRootsRegsMask();
        for (auto input : saveState->GetInputs()) {
            auto reg = input.GetInst()->GetDstReg();
            EXPECT_NE(reg, INVALID_REG);
            EXPECT_TRUE(roots.test(reg));
        }
    });
}

TEST_F(RegAllocCommonTest, DynMethodNargsParamReserve)
{
    auto graph = GetGraph();
    graph->SetDynamicMethod();
#ifndef NDEBUG
    graph->SetDynUnitTestFlag();
#endif
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).any();
        PARAMETER(1U, 1U).any();

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::SaveState).NoVregs();
            INST(3U, Opcode::Intrinsic)
                .any()
                .Inputs({{DataType::ANY, 1U}, {DataType::ANY, 0U}, {DataType::NO_TYPE, 2U}});
            INST(4U, Opcode::Return).any().Inputs(3U);
        }
    }

    RunRegAllocatorsAndCheck(graph, [](Graph *checkGraph) {
        auto reg = Target(checkGraph->GetArch()).GetParamRegId(1U);

        for (auto inst : checkGraph->GetStartBlock()->Insts()) {
            if (!inst->IsSpillFill()) {
                continue;
            }
            auto sfs = inst->CastToSpillFill()->GetSpillFills();
            auto it = std::find_if(sfs.cbegin(), sfs.cend(), [reg](auto sf) {
                return sf.DstValue() == reg && sf.DstType() == LocationType::REGISTER;
            });
            ASSERT_EQ(it, sfs.cend());
        }
    });
}

TEST_F(RegAllocCommonTest, TempRegisters)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 1U).s32().DstReg(10U);
        CONSTANT(1U, 2U).s32().DstReg(11U);
        CONSTANT(2U, 3U).s32().DstReg(12U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(3U, Opcode::SaveState).Inputs().SrcVregs({});
            INST(4U, Opcode::CallStatic).InputsAutoType(0U, 1U, 2U, 3U).v0id().SetFlag(inst_flags::REQUIRE_TMP);
            INST(5U, Opcode::ReturnVoid).v0id();
        }
    }

    RunRegAllocatorsAndCheck(graph, RegMask {0xFFFFFFE1}, [](Graph *checkGraph) {
        auto bb = checkGraph->GetStartBlock()->GetSuccessor(0U);
        Inst *callInst = nullptr;
        for (auto inst : bb->Insts()) {
            if (inst->IsCall()) {
                callInst = inst;
                break;
            }
        }
        ASSERT_TRUE(callInst != nullptr);
        auto tmpLocation = callInst->GetTmpLocation();
        EXPECT_TRUE(tmpLocation.IsFixedRegister());

        // Check that temp register doesn't overlay input registers
        for (size_t i = 0; i < callInst->GetInputsCount(); i++) {
            EXPECT_NE(tmpLocation, callInst->GetLocation(i));
        }
    });
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
