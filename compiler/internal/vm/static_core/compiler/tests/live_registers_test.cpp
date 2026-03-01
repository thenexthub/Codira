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
#include "optimizer/analysis/live_registers.h"
#include "optimizer/optimizations/regalloc/reg_alloc_linear_scan.h"

namespace ark::compiler {

// NOLINTBEGIN(readability-magic-numbers)
class LiveRegistersTest : public GraphTest {};

TEST_F(LiveRegistersTest, EmptyIntervals)
{
    auto intervals = ArenaVector<LifeIntervals *>(GetGraph()->GetAllocator()->Adapter());
    ASSERT_EQ(LifeIntervalsTree::BuildIntervalsTree(intervals, GetGraph()), nullptr);
}

TEST_F(LiveRegistersTest, IntervalsWithoutRegisters)
{
    auto alloc = GetGraph()->GetAllocator();
    auto intervals = ArenaVector<LifeIntervals *>(alloc->Adapter());
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(0U, 42U)));
    ASSERT_EQ(LifeIntervalsTree::BuildIntervalsTree(intervals, GetGraph()), nullptr);
}

TEST_F(LiveRegistersTest, IntervalsWithRegisters)
{
    auto alloc = GetGraph()->GetAllocator();
    auto intervals = ArenaVector<LifeIntervals *>(alloc->Adapter());
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(0U, 10U)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(0U, 2U)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(2U, 3U)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(5U, 6U)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(6U, 8U)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(8U, 10U)));

    Register reg {0U};
    for (auto &li : intervals) {
        li->SetType(DataType::UINT64);
        li->SetReg(reg++);
    }
    auto tree = LifeIntervalsTree::BuildIntervalsTree(intervals, GetGraph());
    ASSERT_NE(tree, nullptr);

    RegMask mask {};
    tree->VisitIntervals(5, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {0b1001U}, mask);

    mask.reset();
    tree->VisitIntervals(11, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {}, mask);

    mask.reset();
    tree->VisitIntervals(8, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {0b110001U}, mask);

    mask.reset();
    tree->VisitIntervals(4, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {0b1U}, mask);

    // Not-live splits at target life-position
    mask.reset();
    tree->VisitIntervals<false>(8, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {0b100001U}, mask);
}

TEST_F(LiveRegistersTest, IntervalsWithHole)
{
    auto alloc = GetGraph()->GetAllocator();
    auto intervals = ArenaVector<LifeIntervals *>(alloc->Adapter());
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(0U, 2U)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(8U, 10U)));

    Register reg {0U};
    for (auto &li : intervals) {
        li->SetType(DataType::UINT64);
        li->SetReg(reg++);
    }
    auto tree = LifeIntervalsTree::BuildIntervalsTree(intervals, GetGraph());
    ASSERT_NE(tree, nullptr);

    RegMask mask {};
    tree->VisitIntervals(5, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {}, mask);

    mask.reset();
    tree->VisitIntervals(0, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {0b1U}, mask);

    mask.reset();
    tree->VisitIntervals(9, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {0b10U}, mask);
}

TEST_F(LiveRegistersTest, IntervalsOutOfRange)
{
    auto alloc = GetGraph()->GetAllocator();
    auto intervals = ArenaVector<LifeIntervals *>(alloc->Adapter());
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(4U, 6U)));

    Register reg {0U};
    for (auto &li : intervals) {
        li->SetType(DataType::UINT64);
        li->SetReg(reg++);
    }
    auto tree = LifeIntervalsTree::BuildIntervalsTree(intervals, GetGraph());
    ASSERT_NE(tree, nullptr);

    size_t count = 0;
    tree->VisitIntervals(1U, [&count]([[maybe_unused]] const auto &li) { count++; });
    ASSERT_EQ(count, 0U);

    count = 0;
    tree->VisitIntervals(42U, [&count]([[maybe_unused]] const auto &li) { count++; });
    ASSERT_EQ(count, 0U);
}

TEST_F(LiveRegistersTest, MultipleBranches)
{
    auto alloc = GetGraph()->GetAllocator();
    auto intervals = ArenaVector<LifeIntervals *>(alloc->Adapter());
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(0U, 80U)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(0U, 39U)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(21U, 39U)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(21U, 29U)));
    intervals.push_back(alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(26U, 29U)));

    Register reg {0U};
    for (auto &li : intervals) {
        li->SetType(DataType::UINT64);
        li->SetReg(reg++);
    }
    auto tree = LifeIntervalsTree::BuildIntervalsTree(intervals, GetGraph());
    ASSERT_NE(tree, nullptr);

    RegMask mask {};
    tree->VisitIntervals(27, [&mask]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(mask.test(li->GetReg()), false);
        mask.set(li->GetReg());
    });
    ASSERT_EQ(RegMask {0b11111U}, mask);
}

TEST_F(LiveRegistersTest, LiveRegisterForGraph)
{
    GRAPH(GetGraph())
    {
        CONSTANT(0U, 42U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(1U, Opcode::Return).u64().Inputs(0U);
        }
    }

    auto result = GetGraph()->RunPass<RegAllocLinearScan>();
    ASSERT_TRUE(result);
    ASSERT_TRUE(GetGraph()->RunPass<LiveRegisters>());

    auto con = &INS(0U);

    auto &lr = GetGraph()->GetAnalysis<LiveRegisters>();
    lr.VisitIntervalsWithLiveRegisters(con, []([[maybe_unused]] const auto &li) { UNREACHABLE(); });

    size_t count = 0;
    lr.VisitIntervalsWithLiveRegisters(&INS(1), [&con, &count]([[maybe_unused]] const auto &li) {
        ASSERT_EQ(li->GetInst(), con);
        count++;
    });
    ASSERT_EQ(count, 1U);
}

TEST_F(LiveRegistersTest, LiveSplits)
{
    auto alloc = GetGraph()->GetAllocator();
    auto intervals = ArenaVector<LifeIntervals *>(alloc->Adapter());
    auto interval = alloc->New<LifeIntervals>(alloc, GetGraph()->CreateInstAdd(), LiveRange(0U, 30U));
    interval->Finalize();
    intervals.push_back(interval);

    interval->SetType(DataType::Type::UINT64);
    interval->SetReg(0U);

    auto split0 = interval->SplitAt(9U, alloc);
    split0->SetLocation(Location::MakeStackSlot(0U));
    auto split1 = split0->SplitAt(19U, alloc);
    split1->SetReg(1U);

    auto tree = LifeIntervalsTree::BuildIntervalsTree(intervals, GetGraph());
    ASSERT_NE(tree, nullptr);

    std::vector<std::pair<LifeNumber, RegMask::ValueType>> ln2mask = {
        {7U, 0b1U}, {9U, 0b1U}, {11U, 0b0U}, {19U, 0b10U}, {30U, 0b10U}};

    for (auto [ln, expected_mask] : ln2mask) {
        RegMask mask {};
        tree->VisitIntervals(ln, [&mask]([[maybe_unused]] const auto &li) {
            ASSERT_EQ(mask.test(li->GetReg()), false);
            mask.set(li->GetReg());
        });
        EXPECT_EQ(RegMask {expected_mask}, mask) << "Wrong mask @ life number " << ln;
    }
}

TEST_F(LiveRegistersTest, IntervalWithLifetimeHole)
{
    auto graph = GetGraph();
    if (graph->GetCallingConvention() == nullptr) {
        GTEST_SKIP();
    }

    GRAPH(graph)
    {
        PARAMETER(0U, 0U).u64();
        PARAMETER(1U, 1U).u64();
        CONSTANT(2U, 0U);

        BASIC_BLOCK(2U, 3U, 4U)
        {
            INST(3U, Opcode::Mul).Inputs(0U, 1U).u64();
            INST(4U, Opcode::Compare).b().Inputs(3U, 2U);
            INST(5U, Opcode::IfImm).SrcType(DataType::BOOL).CC(CC_EQ).Inputs(4U).Imm(0U);
        }

        BASIC_BLOCK(3U, 5U)
        {
            INST(6U, Opcode::SaveState).Inputs(0U, 1U, 2U).SrcVregs({0U, 1U, 2U});
            // interval for inst 3 will have a lifetime hole at this call
            INST(7U, Opcode::CallStatic).InputsAutoType(6U).v0id();
        }

        BASIC_BLOCK(4U, 5U)
        {
            INST(8U, Opcode::Mul).Inputs(3U, 3U).u64();
            INST(9U, Opcode::SaveState).Inputs(0U, 1U, 2U, 8U).SrcVregs({0U, 1U, 2U, 8U});
            INST(10U, Opcode::CallStatic).InputsAutoType(9U).v0id();
        }

        BASIC_BLOCK(5U, -1L)
        {
            INST(11U, Opcode::Return).Inputs(0U).u64();
        }
    }

    auto result = graph->RunPass<RegAllocLinearScan>();
    ASSERT_TRUE(result);
    ASSERT_TRUE(graph->RunPass<LiveRegisters>());

    auto &lr = graph->GetAnalysis<LiveRegisters>();
    lr.VisitIntervalsWithLiveRegisters<false>(&INS(7), [graph](const auto &li) {
        auto rd = graph->GetRegisters();
        auto callerMask =
            DataType::IsFloatType(li->GetType()) ? rd->GetCallerSavedVRegMask() : rd->GetCallerSavedRegMask();
        ASSERT_FALSE(callerMask.Test(li->GetReg())) << "There should be no live caller-saved registers at call, but "
                                                       "a register for following instruction is alive: "
                                                    << *(li->GetInst());
    });
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::compiler
