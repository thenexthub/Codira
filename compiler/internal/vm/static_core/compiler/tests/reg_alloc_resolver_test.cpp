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

#include <sstream>
#include "unit_test.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/optimizations/regalloc/reg_alloc_resolver.h"
#include "optimizer/optimizations/regalloc/reg_type.h"
namespace ark::compiler {
static bool operator==(const SpillFillData &lhs, const SpillFillData &rhs)
{
    return std::forward_as_tuple(lhs.SrcType(), lhs.DstType(), lhs.GetType(), lhs.SrcValue(), lhs.DstValue()) ==
           std::forward_as_tuple(rhs.SrcType(), rhs.DstType(), rhs.GetType(), rhs.SrcValue(), rhs.DstValue());
}

class RegAllocResolverTest : public GraphTest {
protected:
    void InitUsedRegs(Graph *graph)
    {
        ArenaVector<bool> regs =
            ArenaVector<bool>(std::max(MAX_NUM_REGS, MAX_NUM_VREGS), false, GetAllocator()->Adapter());
        graph->InitUsedRegs<DataType::INT64>(&regs);
        graph->InitUsedRegs<DataType::FLOAT64>(&regs);
        graph->SetStackSlotsCount(MAX_NUM_STACK_SLOTS);
    }

    void ResolveFixedInputsRunLiveness();
};

SRC_GRAPH(RegAllocResolverTest, Graph *graph)
{
    GRAPH(graph)
    {
        PARAMETER(0U, 0U).ref();
        PARAMETER(1U, 1U).i32();
        PARAMETER(2U, 2U).u64();

        BASIC_BLOCK(2U, -1L)
        {
            INST(5U, Opcode::AddI).u64().Inputs(2U).Imm(1U);
            INST(3U, Opcode::StoreArray).u64().Inputs(0U, 1U, 2U);
            INST(4U, Opcode::Return).u64().Inputs(5U);
        }
    }
}

void RegAllocResolverTest::ResolveFixedInputsRunLiveness()
{
    Target target(GetGraph()->GetArch());
    auto storeInst = &INS(3U);
    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    ASSERT_TRUE(la.Run());

    auto param0 = &INS(0U);
    auto param1 = &INS(1U);
    auto param2 = &INS(2U);

    auto param0Interval = la.GetInstLifeIntervals(param0);
    auto param1Interval = la.GetInstLifeIntervals(param1);
    auto param2Interval = la.GetInstLifeIntervals(param2);
    auto storeInterval = la.GetInstLifeIntervals(storeInst);
    auto addInterval = la.GetInstLifeIntervals(&INS(5U));

    param0Interval->SetLocation(Location::MakeRegister(target.GetParamRegId(0U)));
    param1Interval->SetLocation(Location::MakeRegister(target.GetParamRegId(1U)));
    param2Interval->SetLocation(Location::MakeRegister(target.GetParamRegId(2U)));
    addInterval->SetLocation(Location::MakeRegister(target.GetParamRegId(3U)));

    param0Interval->SplitAt(storeInterval->GetBegin() - 1U, GetGraph()->GetAllocator())
        ->SetLocation(Location::MakeStackSlot(0U));
    param1Interval->SplitAt(addInterval->GetBegin() - 1U, GetGraph()->GetAllocator())
        ->SetLocation(Location::MakeRegister(target.GetParamRegId(2U)));
    auto p2Split1 = param2Interval->SplitAt(addInterval->GetBegin() - 1U, GetGraph()->GetAllocator());
    p2Split1->SetLocation(Location::MakeRegister(6U));
    auto p2Split2 = p2Split1->SplitAt(storeInterval->GetBegin() - 1U, GetGraph()->GetAllocator());
    p2Split2->SetLocation(Location::MakeRegister(target.GetParamRegId(0U)));
    la.GetInstLifeIntervals(&INS(4U))->SetLocation(Location::MakeRegister(target.GetReturnRegId()));
    la.GetTmpRegInterval(storeInst)->SetLocation(Location::MakeRegister(8U));
}

// NOLINTBEGIN(readability-magic-numbers)
// CC-OFFNXT(huge_depth) false positive
TEST_F(RegAllocResolverTest, ResolveFixedInputs)
{
    src_graph::RegAllocResolverTest::CREATE(GetGraph());

    /*
     * Manually split intervals and allocate registers as follows:
     * - set fixed registers for v3's parameters: r0, r2 and r1 respectively;
     * - move v1 to r2 before v5;
     * - spill v0 from r0 to stack before v3;
     * - move v2 to some reg before v5 and then to r0 before v3.
     *
     * RegAllocResolver may choose r0-split for v0, because it ends right before v3,
     * but it's incorrect because r0 will be overridden by v2 (and SplitResolver will
     * create separate SpillFill for it).
     * This, correct moves for v3 params are:
     * - load v0 from stack slot into r0;
     * - move v2 from r0 into r1;
     */
    Target target(GetGraph()->GetArch());
    auto storeInst = &INS(3U);
    storeInst->SetLocation(0U, Location::MakeRegister(target.GetParamRegId(0U)));
    storeInst->SetLocation(1U, Location::MakeRegister(target.GetParamRegId(2U)));
    storeInst->SetLocation(2U, Location::MakeRegister(target.GetParamRegId(1U)));

    ResolveFixedInputsRunLiveness();
    InitUsedRegs(GetGraph());
    RegAllocResolver resolver(GetGraph());
    resolver.Resolve();

    auto prev = storeInst->GetPrev();
    ASSERT_TRUE(prev != nullptr && prev->GetOpcode() == Opcode::SpillFill);
    auto sf = prev->CastToSpillFill();
    auto sfData = sf->GetSpillFills();
    ASSERT_EQ(sfData.size(), 2U);

    std::vector<SpillFillData> expectedSf {
        SpillFillData {LocationType::REGISTER, LocationType::REGISTER, 0U, 1U, DataType::UINT64},
        SpillFillData {LocationType::STACK, LocationType::REGISTER, 0U, 0U,
                       ConvertRegType(GetGraph(), DataType::REFERENCE)}};

    for (auto &expSf : expectedSf) {
        bool found = false;
        for (auto &effSf : sfData) {
            if (effSf == expSf) {
                found = true;
                break;
            }
        }
        if (found) {
            continue;
        }
        std::stringstream f;
        for (auto &esf : sfData) {
            f << sf_data::ToString(esf, GetGraph()->GetArch()) << ", ";
        }
        ASSERT_TRUE(found) << "Spill fill " << sf_data::ToString(expSf, GetGraph()->GetArch()) << "  not found among "
                           << f.str();
    }
}
// NOLINTEND(readability-magic-numbers)
}  // namespace ark::compiler
