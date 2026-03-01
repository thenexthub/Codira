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
#include "optimizer/analysis/liveness_analyzer.h"

namespace panda::compiler {
class LifeIntervalsTest : public CommonTest {
public:
    LifeIntervalsTest() : graph_(CreateEmptyGraph()) {}

    LifeIntervals *Create(std::initializer_list<std::pair<LifeNumber, LifeNumber>> lns)
    {
        auto inst = graph_->CreateInstConstant(42);
        auto li = GetAllocator()->New<LifeIntervals>(GetAllocator(), inst);
        for (auto range = std::rbegin(lns); range != std::rend(lns); range++) {
            li->AppendRange(range->first, range->second);
        }
        return li;
    }

    void CheckSiblings(std::initializer_list<LifeIntervals *> intervals)
    {
        if (intervals.size() == 0) {
            return;
        }
        for (auto it = std::begin(intervals); it != std::end(intervals); it++) {
            auto next = std::next(it);
            if (next == std::end(intervals)) {
                ASSERT_EQ((*it)->GetSibling(), nullptr);
            } else {
                ASSERT_EQ((*it)->GetSibling(), *next);
            }
        }
    }

    void CheckRanges(LifeIntervals *interval, std::initializer_list<std::pair<LifeNumber, LifeNumber>> ranges)
    {
        ASSERT(interval != nullptr);
        auto li_ranges = interval->GetRanges();
        ASSERT_EQ(li_ranges.size(), ranges.size());

        auto actual_it = li_ranges.begin();
        auto expected_it = std::begin(ranges);
        while (actual_it != li_ranges.end() && expected_it != std::end(ranges)) {
            auto actual_range = *(actual_it++);
            auto expected_lns = *(expected_it++);
            ASSERT_EQ(actual_range.GetBegin(), expected_lns.first);
            ASSERT_EQ(actual_range.GetEnd(), expected_lns.second);
        }
    }

private:
    Graph *graph_;
};

TEST_F(LifeIntervalsTest, SplitAtTheEnd)
{
    auto interval = Create({{0, 4}});
    auto split = interval->SplitAt(4, GetAllocator());

    CheckSiblings({interval, split});
    CheckRanges(interval, {{0, 4}});
    CheckRanges(split, {});
}

TEST_F(LifeIntervalsTest, SplitBetweenRanges)
{
    auto interval = Create({{0, 4}, {8, 10}});
    auto split = interval->SplitAt(6, GetAllocator());

    CheckSiblings({interval, split});
    CheckRanges(interval, {{0, 4}});
    CheckRanges(split, {{8, 10}});
}

TEST_F(LifeIntervalsTest, SplitRange)
{
    auto interval = Create({{0, 10}});
    auto split = interval->SplitAt(6, GetAllocator());

    CheckSiblings({interval, split});
    CheckRanges(interval, {{0, 6}});
    CheckRanges(split, {{6, 10}});
}

TEST_F(LifeIntervalsTest, SplitIntervalWithMultipleRanges)
{
    auto interval = Create({{0, 4}, {6, 10}, {12, 20}});
    auto split = interval->SplitAt(8, GetAllocator());

    CheckSiblings({interval, split});
    CheckRanges(interval, {{0, 4}, {6, 8}});
    CheckRanges(split, {{8, 10}, {12, 20}});
}

TEST_F(LifeIntervalsTest, RecursiveSplits)
{
    auto interval = Create({{0, 100}});
    auto split0 = interval->SplitAt(50, GetAllocator());
    auto split1 = split0->SplitAt(75, GetAllocator());
    auto split2 = interval->SplitAt(25, GetAllocator());

    CheckSiblings({interval, split2, split0, split1});
    CheckRanges(interval, {{0, 25}});
    CheckRanges(split2, {{25, 50}});
    CheckRanges(split0, {{50, 75}});
    CheckRanges(split1, {{75, 100}});
}

TEST_F(LifeIntervalsTest, IntervalsCoverage)
{
    auto interval = Create({{0, 20}, {22, 40}, {42, 100}});
    auto split0 = interval->SplitAt(21, GetAllocator());
    auto split1 = split0->SplitAt(50, GetAllocator());

    EXPECT_TRUE(interval->SplitCover(18));
    EXPECT_FALSE(split0->SplitCover(18));
    EXPECT_FALSE(split1->SplitCover(18));

    EXPECT_FALSE(interval->SplitCover(42));
    EXPECT_TRUE(split0->SplitCover(42));
    EXPECT_FALSE(split1->SplitCover(42));
}

TEST_F(LifeIntervalsTest, FindSiblingAt)
{
    auto interval = Create({{0, 20}, {22, 40}, {42, 100}});
    auto split0 = interval->SplitAt(22, GetAllocator());
    auto split1 = split0->SplitAt(50, GetAllocator());

    EXPECT_EQ(interval->FindSiblingAt(0), interval);
    EXPECT_EQ(interval->FindSiblingAt(20), interval);
    EXPECT_EQ(interval->FindSiblingAt(39), split0);
    EXPECT_EQ(interval->FindSiblingAt(51), split1);
    EXPECT_EQ(interval->FindSiblingAt(21), nullptr);
}

TEST_F(LifeIntervalsTest, Intersects)
{
    auto interval = Create({{6, 10}});

    EXPECT_TRUE(interval->Intersects(LiveRange(0, 20)));
    EXPECT_TRUE(interval->Intersects(LiveRange(6, 10)));
    EXPECT_TRUE(interval->Intersects(LiveRange(0, 8)));
    EXPECT_TRUE(interval->Intersects(LiveRange(8, 20)));
    EXPECT_TRUE(interval->Intersects(LiveRange(7, 9)));

    EXPECT_FALSE(interval->Intersects(LiveRange(0, 4)));
    EXPECT_FALSE(interval->Intersects(LiveRange(12, 20)));
}

TEST_F(LifeIntervalsTest, IsSameLocation)
{
    auto graph = CreateEmptyGraph();
    GRAPH(graph)
    {
        CONSTANT(0, 0);
        CONSTANT(1, 42);

        BASIC_BLOCK(2, -1)
        {
            INST(2, Opcode::Add).u64().Inputs(0, 1);
            INST(3, Opcode::Return).u64().Inputs(2);
        }
    };

    auto con0 = &INS(0);
    auto con42 = &INS(1);
    auto add = &INS(2);

    std::vector<std::function<LifeIntervals *()>> factories = {
        [&]() {
            auto li = GetAllocator()->New<LifeIntervals>(GetAllocator(), add);
            li->SetReg(0);
            return li;
        },
        [&]() {
            auto li = GetAllocator()->New<LifeIntervals>(GetAllocator(), add);
            li->SetReg(1);
            return li;
        },
        [&]() {
            auto li = GetAllocator()->New<LifeIntervals>(GetAllocator(), add);
            li->SetLocation(Location::MakeStackSlot(1));
            return li;
        },
        [&]() {
            auto li = GetAllocator()->New<LifeIntervals>(GetAllocator(), add);
            li->SetLocation(Location::MakeStackSlot(2));
            return li;
        },
        [&]() {
            auto li = GetAllocator()->New<LifeIntervals>(GetAllocator(), con42);
            li->SetLocation(Location::MakeConstant(0));
            return li;
        },
        [&]() {
            auto li = GetAllocator()->New<LifeIntervals>(GetAllocator(), con42);
            li->SetLocation(Location::MakeConstant(1));
            return li;
        },
        [&]() {
            auto li = GetAllocator()->New<LifeIntervals>(GetAllocator(), con0);
            if (graph->GetZeroReg() == INVALID_REG) {
                ASSERT(graph->GetArch() != Arch::AARCH64);
                li->SetReg(31);
            } else {
                li->SetReg(graph->GetZeroReg());
            }
            return li;
        }};

    for (size_t l1_idx = 0; l1_idx < factories.size(); l1_idx++) {
        auto interval0 = factories[l1_idx]();
        for (size_t l2_idx = 0; l2_idx < factories.size(); l2_idx++) {
            auto interval1 = factories[l2_idx]();
            bool same_settings = l1_idx == l2_idx;
            EXPECT_EQ(same_settings, interval0->GetLocation() == interval1->GetLocation());
        }
    }
}

TEST_F(LifeIntervalsTest, LastUsageBefore)
{
    auto interval = Create({{10, 100}});

    ASSERT_EQ(INVALID_LIFE_NUMBER, interval->GetLastUsageBefore(100));

    interval->AddUsePosition(12);
    interval->AddUsePosition(50);
    interval->AddUsePosition(60);

    ASSERT_EQ(INVALID_LIFE_NUMBER, interval->GetLastUsageBefore(12));
    ASSERT_EQ(12, interval->GetLastUsageBefore(13));
    ASSERT_EQ(60, interval->GetLastUsageBefore(100));
}

}  // namespace panda::compiler
