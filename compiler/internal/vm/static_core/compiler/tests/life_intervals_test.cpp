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
#include "optimizer/analysis/liveness_analyzer.h"

namespace ark::compiler {
// NOLINTBEGIN(readability-magic-numbers)
class LifeIntervalsTest : public CommonTest {
public:
    LifeIntervalsTest() : graph_(CreateEmptyGraph()) {}

    LifeIntervals *Create(std::initializer_list<std::pair<LifeNumber, LifeNumber>> lns)
    {
        auto inst = graph_->CreateInstConstant(42U);
        auto li = GetAllocator()->New<LifeIntervals>(GetAllocator(), inst);
        for (auto range = std::rbegin(lns); range != std::rend(lns); range++) {
            li->AppendRange(range->first, range->second);
        }
        li->Finalize();
        return li;
    }

    void CheckSiblings(std::initializer_list<LifeIntervals *> intervals)
    {
        if (intervals.size() == 0U) {
            return;
        }
        for (auto it = std::begin(intervals); it != std::end(intervals); it++) {
            auto next = std::next(it);
            if (next == std::end(intervals)) {
                ASSERT_EQ((*it)->GetSibling(), nullptr);  // NOLINT
            } else {
                ASSERT_EQ((*it)->GetSibling(), *next);
            }
        }
    }

    void CheckRanges(LifeIntervals *interval, std::initializer_list<std::pair<LifeNumber, LifeNumber>> ranges)
    {
        ASSERT(interval != nullptr);
        auto liRanges = interval->GetRanges();
        ASSERT_EQ(liRanges.size(), ranges.size());

        auto actualIt = liRanges.rbegin();
        auto expectedIt = std::begin(ranges);
        while (actualIt != liRanges.rend() && expectedIt != std::end(ranges)) {
            auto actualRange = *(actualIt++);
            auto expectedLns = *(expectedIt++);
            ASSERT_EQ(actualRange.GetBegin(), expectedLns.first);
            ASSERT_EQ(actualRange.GetEnd(), expectedLns.second);
        }
    }

    void CheckSiblingSequence(LifeIntervals *interval, std::initializer_list<std::pair<LifeNumber, LifeNumber>> ranges)
    {
        auto expectedIt = std::begin(ranges);
        for (auto sibling = interval; sibling != nullptr; sibling = sibling->GetSibling()) {
            auto expected = *(expectedIt++);
            ASSERT_EQ(sibling->GetBegin(), expected.first);
            ASSERT_EQ(sibling->GetEnd(), expected.second);
        }
    }

    std::vector<std::function<LifeIntervals *()>> IsSameLocationFactories(Graph *graph);

private:
    Graph *graph_;
};

TEST_F(LifeIntervalsTest, SplitAtTheEnd)
{
    auto interval = Create({{0U, 4U}});
    auto split = interval->SplitAt(4U, GetAllocator());

    CheckSiblings({interval, split});
    CheckRanges(interval, {{0U, 4U}});
    CheckRanges(split, {});
}

TEST_F(LifeIntervalsTest, SplitBetweenRanges)
{
    auto interval = Create({{0U, 4U}, {8U, 10U}});
    auto split = interval->SplitAt(6U, GetAllocator());

    CheckSiblings({interval, split});
    CheckRanges(interval, {{0U, 4U}});
    CheckRanges(split, {{8U, 10U}});
}

TEST_F(LifeIntervalsTest, SplitRange)
{
    auto interval = Create({{0U, 10U}});
    auto split = interval->SplitAt(6U, GetAllocator());

    CheckSiblings({interval, split});
    CheckRanges(interval, {{0U, 6U}});
    CheckRanges(split, {{6U, 10U}});
}

TEST_F(LifeIntervalsTest, SplitIntervalWithMultipleRanges)
{
    auto interval = Create({{0U, 4U}, {6U, 10U}, {12U, 20U}});
    auto split = interval->SplitAt(8U, GetAllocator());

    CheckSiblings({interval, split});
    CheckRanges(interval, {{0U, 4U}, {6U, 8U}});
    CheckRanges(split, {{8U, 10U}, {12U, 20U}});
}

TEST_F(LifeIntervalsTest, RecursiveSplits)
{
    auto interval = Create({{0U, 100U}});
    auto split0 = interval->SplitAt(50U, GetAllocator());
    auto split1 = split0->SplitAt(75U, GetAllocator());
    auto split2 = interval->SplitAt(25U, GetAllocator());

    CheckSiblings({interval, split2, split0, split1});
    CheckRanges(interval, {{0U, 25U}});
    CheckRanges(split2, {{25U, 50U}});
    CheckRanges(split0, {{50U, 75U}});
    CheckRanges(split1, {{75U, 100U}});
}

TEST_F(LifeIntervalsTest, IntervalsCoverage)
{
    auto interval = Create({{0U, 20U}, {22U, 40U}, {42U, 100U}});
    auto split0 = interval->SplitAt(21U, GetAllocator());
    auto split1 = split0->SplitAt(50U, GetAllocator());

    EXPECT_TRUE(interval->SplitCover(18U));
    EXPECT_FALSE(split0->SplitCover(18U));
    EXPECT_FALSE(split1->SplitCover(18U));

    EXPECT_FALSE(interval->SplitCover(42U));
    EXPECT_TRUE(split0->SplitCover(42U));
    EXPECT_FALSE(split1->SplitCover(42U));
}

TEST_F(LifeIntervalsTest, FindSiblingAt)
{
    auto interval = Create({{0U, 20U}, {22U, 40U}, {42U, 100U}});
    auto split0 = interval->SplitAt(22U, GetAllocator());
    auto split1 = split0->SplitAt(50U, GetAllocator());

    EXPECT_EQ(interval->FindSiblingAt(0U), interval);
    EXPECT_EQ(interval->FindSiblingAt(20U), interval);
    EXPECT_EQ(interval->FindSiblingAt(39U), split0);
    EXPECT_EQ(interval->FindSiblingAt(51U), split1);
    EXPECT_EQ(interval->FindSiblingAt(21U), nullptr);
}

TEST_F(LifeIntervalsTest, Intersects)
{
    auto interval = Create({{6U, 10U}});

    EXPECT_TRUE(interval->Intersects(LiveRange(0U, 20U)));
    EXPECT_TRUE(interval->Intersects(LiveRange(6U, 10U)));
    EXPECT_TRUE(interval->Intersects(LiveRange(0U, 8U)));
    EXPECT_TRUE(interval->Intersects(LiveRange(8U, 20U)));
    EXPECT_TRUE(interval->Intersects(LiveRange(7U, 9U)));

    EXPECT_FALSE(interval->Intersects(LiveRange(0U, 4U)));
    EXPECT_FALSE(interval->Intersects(LiveRange(12U, 20U)));
}

std::vector<std::function<LifeIntervals *()>> LifeIntervalsTest::IsSameLocationFactories(Graph *graph)
{
    auto con0 = &INS(0U);
    auto con42 = &INS(1U);
    auto add = &INS(2U);
    return {[this, add]() {
                auto li = GetAllocator()->New<LifeIntervals>(GetAllocator(), add);
                li->SetReg(0U);
                return li;
            },
            [this, add]() {
                auto li = GetAllocator()->New<LifeIntervals>(GetAllocator(), add);
                li->SetReg(1U);
                return li;
            },
            [this, add]() {
                auto li = GetAllocator()->New<LifeIntervals>(GetAllocator(), add);
                li->SetLocation(Location::MakeStackSlot(1U));
                return li;
            },
            [this, add]() {
                auto li = GetAllocator()->New<LifeIntervals>(GetAllocator(), add);
                li->SetLocation(Location::MakeStackSlot(2U));
                return li;
            },
            [this, con42]() {
                auto li = GetAllocator()->New<LifeIntervals>(GetAllocator(), con42);
                li->SetLocation(Location::MakeConstant(0U));
                return li;
            },
            [this, con42]() {
                auto li = GetAllocator()->New<LifeIntervals>(GetAllocator(), con42);
                li->SetLocation(Location::MakeConstant(1U));
                return li;
            },
            [this, graph, con0]() {
                auto li = GetAllocator()->New<LifeIntervals>(GetAllocator(), con0);
                if (graph->GetZeroReg() == INVALID_REG) {
                    ASSERT(graph->GetArch() != Arch::AARCH64);
                    li->SetReg(31U);
                } else {
                    li->SetReg(graph->GetZeroReg());
                }
                return li;
            }};
}

TEST_F(LifeIntervalsTest, IsSameLocation)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0U, 0U);
        CONSTANT(1U, 42U);

        BASIC_BLOCK(2U, -1L)
        {
            INST(2U, Opcode::Add).u64().Inputs(0U, 1U);
            INST(3U, Opcode::Return).u64().Inputs(2U);
        }
    };

    auto factories = IsSameLocationFactories(graph);

    for (size_t l1Idx = 0; l1Idx < factories.size(); l1Idx++) {
        auto interval0 = factories[l1Idx]();
        for (size_t l2Idx = 0; l2Idx < factories.size(); l2Idx++) {
            auto interval1 = factories[l2Idx]();
            bool sameSettings = l1Idx == l2Idx;
            EXPECT_EQ(sameSettings, interval0->GetLocation() == interval1->GetLocation());
        }
    }
}

TEST_F(LifeIntervalsTest, LastUsageBeforeEmpty)
{
    auto interval = Create({{10U, 100U}});

    ASSERT_EQ(INVALID_LIFE_NUMBER, interval->GetLastUsageBefore(100U));
}

TEST_F(LifeIntervalsTest, LastUsageBefore)
{
    LifeIntervals interval(GetAllocator());
    interval.AppendRange({10U, 100U});
    interval.AddUsePosition(12U);
    interval.AddUsePosition(50U);
    interval.AddUsePosition(60U);
    interval.Finalize();

    ASSERT_EQ(INVALID_LIFE_NUMBER, interval.GetLastUsageBefore(12U));
    ASSERT_EQ(12U, interval.GetLastUsageBefore(13U));
    ASSERT_EQ(60U, interval.GetLastUsageBefore(100U));
}

TEST_F(LifeIntervalsTest, SplitAroundUsesUnsplittable)
{
    // Unsplittable intervals
    {
        LifeIntervals li(GetAllocator());
        li.AppendRange({10U, 11U});
        li.AddUsePosition(10U);
        li.Finalize();
        li.SplitAroundUses(GetAllocator());
        EXPECT_EQ(li.GetSibling(), nullptr);
    }

    {
        LifeIntervals li(GetAllocator());
        li.AppendRange({10U, 11U});
        li.AddUsePosition(11U);
        li.Finalize();
        li.SplitAroundUses(GetAllocator());
        EXPECT_EQ(li.GetSibling(), nullptr);
    }

    {
        LifeIntervals li(GetAllocator());
        li.AppendRange({10U, 12U});
        li.AddUsePosition(11U);
        li.Finalize();
        li.SplitAroundUses(GetAllocator());
        EXPECT_EQ(li.GetSibling(), nullptr);
    }
}

TEST_F(LifeIntervalsTest, SplitAroundUsesAfterBegin)
{
    // Split after begin
    {
        LifeIntervals li(GetAllocator());
        li.AppendRange({10U, 12U});
        li.AddUsePosition(10U);
        li.Finalize();
        li.SplitAroundUses(GetAllocator());
        CheckSiblingSequence(&li, {{10U, 11U}, {11U, 12U}});
    }

    {
        LifeIntervals li(GetAllocator());
        li.AppendRange({10U, 100U});
        li.AddUsePosition(10U);
        li.Finalize();
        li.SplitAroundUses(GetAllocator());
        CheckSiblingSequence(&li, {{10U, 11U}, {11U, 100U}});
    }

    {
        LifeIntervals li(GetAllocator());
        li.AppendRange({10U, 100U});
        li.AddUsePosition(11U);
        li.Finalize();
        li.SplitAroundUses(GetAllocator());
        CheckSiblingSequence(&li, {{10U, 12U}, {12U, 100U}});
    }
}

TEST_F(LifeIntervalsTest, SplitAroundUsesBeforeEnd)
{
    // Split before end
    {
        LifeIntervals li(GetAllocator());
        li.AppendRange({10U, 12U});
        li.AddUsePosition(12U);
        li.Finalize();
        li.SplitAroundUses(GetAllocator());
        CheckSiblingSequence(&li, {{10U, 11U}, {11U, 12U}});
    }

    {
        LifeIntervals li(GetAllocator());
        li.AppendRange({10U, 100U});
        li.AddUsePosition(100U);
        li.Finalize();
        li.SplitAroundUses(GetAllocator());
        CheckSiblingSequence(&li, {{10U, 99U}, {99U, 100U}});
    }

    {
        LifeIntervals li(GetAllocator());
        li.AppendRange({10U, 100U});
        li.AddUsePosition(99U);
        li.Finalize();
        li.SplitAroundUses(GetAllocator());
        CheckSiblingSequence(&li, {{10U, 98U}, {98U, 100U}});
    }
}

TEST_F(LifeIntervalsTest, SplitAroundUsesBeginAndEnd)
{
    // Split after begin and before end
    {
        LifeIntervals li(GetAllocator());
        li.AppendRange({10U, 100U});
        li.AddUsePosition(10U);
        li.AddUsePosition(100U);
        li.Finalize();
        li.SplitAroundUses(GetAllocator());
        CheckSiblingSequence(&li, {{10U, 11U}, {11U, 99U}, {99U, 100U}});
    }

    {
        LifeIntervals li(GetAllocator());
        li.AppendRange({10U, 100U});
        li.AddUsePosition(11U);
        li.AddUsePosition(99U);
        li.Finalize();
        li.SplitAroundUses(GetAllocator());
        CheckSiblingSequence(&li, {{10U, 12U}, {12U, 98U}, {98U, 100U}});
    }
}

TEST_F(LifeIntervalsTest, SplitAroundUsesMultisplits)
{
    // Multisplits
    {
        LifeIntervals li(GetAllocator());
        li.AppendRange({10U, 16U});
        li.AddUsePosition(10U);
        li.AddUsePosition(12U);
        li.AddUsePosition(14U);
        li.AddUsePosition(16U);
        li.Finalize();
        li.SplitAroundUses(GetAllocator());
        CheckSiblingSequence(&li, {{10U, 11U}, {11U, 13U}, {13U, 15U}, {15U, 16U}});
    }

    {
        LifeIntervals li(GetAllocator());
        li.AppendRange({10U, 16U});
        li.AddUsePosition(11U);
        li.AddUsePosition(13U);
        li.AddUsePosition(15U);
        li.Finalize();
        li.SplitAroundUses(GetAllocator());
        CheckSiblingSequence(&li, {{10U, 12U}, {12U, 14U}, {14U, 16U}});
    }

    {
        LifeIntervals li(GetAllocator());
        li.AppendRange({10U, 100U});
        li.AddUsePosition(30U);
        li.AddUsePosition(50U);
        li.AddUsePosition(70U);
        li.Finalize();
        li.SplitAroundUses(GetAllocator());
        CheckSiblingSequence(&li,
                             {{10U, 29U}, {29U, 31U}, {31U, 49U}, {49U, 51U}, {51U, 69U}, {69U, 71U}, {71U, 100U}});
    }
}
// NOLINTEND(readability-magic-numbers)
}  // namespace ark::compiler
