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
#include "optimizer/ir/graph_cloner.h"
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"
#include "optimizer/optimizations/regalloc/reg_alloc_graph_coloring.h"

namespace panda::compiler {
class RegAllocCommonTest : public GraphTest {
public:
    template <typename Checker>
    void RunRegAllocatorsAndCheck(Graph *graph, Checker checker) const
    {
    }

protected:
    template <DataType::Type reg_type>
    void TestParametersLocations() const;
};

template <DataType::Type reg_type>
void RegAllocCommonTest::TestParametersLocations() const
{
    auto graph = CreateEmptyGraph();
    if constexpr (DataType::UINT64 == reg_type) {
        GRAPH(graph)
        {
            PARAMETER(0, 0).ref();
            PARAMETER(1, 1).u64();
            PARAMETER(2, 2).u64();
            PARAMETER(3, 3).u64();
            PARAMETER(4, 4).u64();
            PARAMETER(5, 5).u64();
            PARAMETER(6, 6).u64();
            PARAMETER(7, 7).u64();
            PARAMETER(8, 8).u64();
            PARAMETER(9, 9).u64();

            BASIC_BLOCK(2, -1)
            {
                INST(30, Opcode::SaveState).Inputs(0).SrcVregs({0});
                INST(11, Opcode::NullCheck).ref().Inputs(0, 30);
                INST(12, Opcode::StoreObject).u64().Inputs(11, 1);
                INST(13, Opcode::StoreObject).u64().Inputs(11, 2);
                INST(14, Opcode::StoreObject).u64().Inputs(11, 3);
                INST(15, Opcode::StoreObject).u64().Inputs(11, 4);
                INST(16, Opcode::StoreObject).u64().Inputs(11, 5);
                INST(17, Opcode::StoreObject).u64().Inputs(11, 6);
                INST(18, Opcode::StoreObject).u64().Inputs(11, 7);
                INST(19, Opcode::StoreObject).u64().Inputs(11, 8);
                INST(20, Opcode::StoreObject).u64().Inputs(11, 9);
                INST(31, Opcode::ReturnVoid).v0id();
            }
        }
    } else {
        GRAPH(graph)
        {
            PARAMETER(0, 0).ref();
            PARAMETER(1, 1).u32();
            PARAMETER(2, 2).u32();
            PARAMETER(3, 3).u32();
            PARAMETER(4, 4).u32();
            PARAMETER(5, 5).u32();
            PARAMETER(6, 6).u32();
            PARAMETER(7, 7).u32();
            PARAMETER(8, 8).u32();
            PARAMETER(9, 9).u32();

            BASIC_BLOCK(2, -1)
            {
                INST(30, Opcode::SaveState).Inputs(0).SrcVregs({0});
                INST(11, Opcode::NullCheck).ref().Inputs(0, 30);
                INST(12, Opcode::StoreObject).u32().Inputs(11, 1);
                INST(13, Opcode::StoreObject).u32().Inputs(11, 2);
                INST(14, Opcode::StoreObject).u32().Inputs(11, 3);
                INST(15, Opcode::StoreObject).u32().Inputs(11, 4);
                INST(16, Opcode::StoreObject).u32().Inputs(11, 5);
                INST(17, Opcode::StoreObject).u32().Inputs(11, 6);
                INST(18, Opcode::StoreObject).u32().Inputs(11, 7);
                INST(19, Opcode::StoreObject).u32().Inputs(11, 8);
                INST(20, Opcode::StoreObject).u32().Inputs(11, 9);
                INST(31, Opcode::ReturnVoid).v0id();
            }
        }
    }

    RunRegAllocatorsAndCheck(graph, [type = reg_type](Graph *check_graph) {
        auto arch = check_graph->GetArch();
        unsigned slot_inc = Is64Bits(type, arch) && !Is64BitsArch(arch) ? 2U : 1U;

        unsigned params_on_registers = 0;
        if (Arch::AARCH64 == check_graph->GetArch()) {
            /**
             * Test case for Arch::AARCH64:
             *
             * - Parameters [arg0 - arg6] are placed in the registers [r1-r7]
             * - All other Parameters are placed in stack slots [slot0 - ...]
             */
            params_on_registers = 7;
        } else if (Arch::AARCH32 == check_graph->GetArch()) {
            /**
             * Test case for Arch::AARCH32:
             * - ref-Parameter (arg0) is placed in the r1 register
             * - If arg1 is 64-bit Parameter, it is placed in the [r2-r3] registers
             * - If arg1, arg2 are 32-bit Parameters, they are placed in the [r2-r3] registers
             * - All other Parameters are placed in stack slots [slot0 - ...]
             */
            params_on_registers = (type == DataType::UINT64) ? 2U : 3U;
        }

        std::map<Location, Inst *> assigned_locations;
        unsigned index = 0;
        unsigned arg_slot = 0;
        for (auto param_inst : check_graph->GetParameters()) {
            // Check intial locations
            auto src_location = param_inst->CastToParameter()->GetLocationData().GetSrc();
            if (index < params_on_registers) {
                EXPECT_EQ(src_location.GetKind(), LocationType::REGISTER);
                EXPECT_EQ(src_location.GetValue(), index + 1U);
            } else {
                EXPECT_EQ(src_location.GetKind(), LocationType::STACK_PARAMETER);
                EXPECT_EQ(src_location.GetValue(), arg_slot);
                arg_slot += slot_inc;
            }

            // Check that assigned locations do not overlap
            auto dst_location = param_inst->CastToParameter()->GetLocationData().GetDst();
            EXPECT_EQ(assigned_locations.count(dst_location), 0U);
            assigned_locations.insert({dst_location, param_inst});

            index++;
        }
    });
}

TEST_F(RegAllocCommonTest, ParametersLocation)
{
    TestParametersLocations<DataType::UINT64>();
    TestParametersLocations<DataType::UINT32>();
}

TEST_F(RegAllocCommonTest, LocationsNoSplits)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 1).s32();
        CONSTANT(1, 2).s32();
        CONSTANT(2, 3).s32();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::Add).u32().Inputs(0, 1);
            INST(4, Opcode::SaveState).Inputs().SrcVregs({});
            INST(5, Opcode::CallStatic).InputsAutoType(3, 2, 0, 1, 4).u32();
            INST(6, Opcode::Return).u32().Inputs(5);
        }
    }

    if (graph->GetArch() == Arch::AARCH32) {
        // Enable after full registers mask support the ARM32
        return;
    }

    // Check that there are no spill-fills in the graph
    RunRegAllocatorsAndCheck(graph, [](Graph *check_graph) {
        for (auto bb : check_graph->GetBlocksRPO()) {
            for (auto inst : bb->AllInsts()) {
                EXPECT_FALSE(inst->IsSpillFill());
                if (inst->NoDest()) {
                    return;
                }
                EXPECT_NE(inst->GetDstReg(), INVALID_REG);
            }
        }
    });
}

TEST_F(RegAllocCommonTest, ImplicitNullCheckStackMap)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        PARAMETER(0, 0).ref();
        PARAMETER(1, 1).ref();

        BASIC_BLOCK(2, -1)
        {
            INST(3, Opcode::SaveState).NoVregs();
            INST(4, Opcode::LoadAndInitClass).ref().Inputs(3);
            INST(5, Opcode::NewObject).ref().Inputs(4, 3);
            INST(6, Opcode::SaveState).Inputs(0, 1).SrcVregs({0, 1});
            INST(7, Opcode::NullCheck).ref().Inputs(5, 6);
            INST(8, Opcode::LoadObject).s32().Inputs(7);
            INST(9, Opcode::Return).s32().Inputs(8);
        }
    }
    INS(7).CastToNullCheck()->SetImplicit(true);

    RunRegAllocatorsAndCheck(graph, [](Graph *check_graph) {
        auto bb = check_graph->GetStartBlock()->GetSuccessor(0);
        // Find null_check
        Inst *null_check = nullptr;
        for (auto inst : bb->AllInsts()) {
            if (inst->IsNullCheck()) {
                null_check = inst;
                break;
            }
        }
        ASSERT(null_check != nullptr);
        auto save_state = null_check->GetSaveState();
        // Check that save_state's inputs are added to the roots
        auto roots = save_state->GetRootsRegsMask();
        for (auto input : save_state->GetInputs()) {
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

    GRAPH(graph)
    {
        PARAMETER(0, 0).any();
        PARAMETER(1, 1).any();

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::SaveState).NoVregs();
            INST(3, Opcode::Intrinsic).any().Inputs({{DataType::ANY, 1}, {DataType::ANY, 0}, {DataType::NO_TYPE, 2}});
            INST(4, Opcode::Return).any().Inputs(3);
        }
    }

    RunRegAllocatorsAndCheck(graph, [](Graph *check_graph) {
        check_graph->Dump(&std::cout);
        auto reg = Target(check_graph->GetArch()).GetParamRegId(1);

        for (auto inst : check_graph->GetStartBlock()->Insts()) {
            if (inst->IsSpillFill()) {
                auto sfs = inst->CastToSpillFill()->GetSpillFills();
                auto it = std::find_if(sfs.cbegin(), sfs.cend(), [reg](auto sf) {
                    return sf.DstValue() == reg && sf.DstType() == LocationType::REGISTER;
                });
                ASSERT_EQ(it, sfs.cend());
            }
        }
    });
}
}  // namespace panda::compiler

