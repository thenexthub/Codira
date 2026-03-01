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

#include <algorithm>
#include "liveness_analyzer.h"
#include "live_registers.h"

namespace ark::compiler {

namespace {
struct Split {
    Split(LifeIntervalsIt pBegin, LifeIntervalsIt pEnd, LifeNumber pMin, LifeNumber pMax,
          LifeIntervalsTreeNode *pParent)
        : begin(pBegin), end(pEnd), min(pMin), max(pMax), parent(pParent)
    {
        ASSERT(pBegin < pEnd);
        ASSERT(pMin <= pMax);
    }
    LifeIntervalsIt begin;          // NOLINT(misc-non-private-member-variables-in-classes)
    LifeIntervalsIt end;            // NOLINT(misc-non-private-member-variables-in-classes)
    LifeNumber min;                 // NOLINT(misc-non-private-member-variables-in-classes)
    LifeNumber max;                 // NOLINT(misc-non-private-member-variables-in-classes)
    LifeIntervalsTreeNode *parent;  // NOLINT(misc-non-private-member-variables-in-classes)
};

// copy intervals with assigned registers and compute min and max life numbers covered by all these intervals
std::pair<LifeNumber, LifeNumber> CopyIntervals(const ArenaVector<LifeIntervals *> &source,
                                                ArenaVector<LifeIntervals *> *destination)
{
    ASSERT(destination != nullptr);
    LifeNumber minLn = std::numeric_limits<LifeNumber>::max();
    LifeNumber maxLn = 0;
    for (auto &interval : source) {
        for (auto split = interval; !interval->IsPhysical() && split != nullptr; split = split->GetSibling()) {
            if (split->HasReg()) {
                minLn = std::min(minLn, split->GetBegin());
                maxLn = std::max(maxLn, split->GetEnd());
                destination->push_back(split);
            }
        }
    }
    return std::make_pair(minLn, maxLn);
}

LifeIntervalsIt PartitionLeftSplit(const LifeIntervalsIt &left, const LifeIntervalsIt &right, LifeNumber midpoint,
                                   LifeNumber *minLn, LifeNumber *maxLn)
{
    LifeNumber leftMinLn = std::numeric_limits<LifeNumber>::max();
    LifeNumber leftMaxLn = 0;
    auto result = std::partition(left, right, [&midpoint, &leftMinLn, &leftMaxLn](const auto &em) {
        if (em->GetEnd() < midpoint) {
            leftMinLn = std::min(leftMinLn, em->GetBegin());
            leftMaxLn = std::max(leftMaxLn, em->GetEnd());
            return true;
        }
        return false;
    });
    *minLn = leftMinLn;
    *maxLn = leftMaxLn;
    return result;
}

LifeIntervalsIt PartitionRightSplit(const LifeIntervalsIt &left, const LifeIntervalsIt &right, LifeNumber midpoint,
                                    LifeNumber *minLn, LifeNumber *maxLn)
{
    LifeNumber rightMinLn = std::numeric_limits<LifeNumber>::max();
    LifeNumber rightMaxLn = 0;
    auto result = std::partition(left, right, [&midpoint, &rightMinLn, &rightMaxLn](const auto &em) {
        if (em->GetBegin() > midpoint) {
            rightMinLn = std::min(rightMinLn, em->GetBegin());
            rightMaxLn = std::max(rightMaxLn, em->GetEnd());
            return false;
        }
        return true;
    });
    *minLn = rightMinLn;
    *maxLn = rightMaxLn;
    return result;
}
}  // namespace

LifeIntervalsTree *LifeIntervalsTree::BuildIntervalsTree(const ArenaVector<LifeIntervals *> &lifeIntervals,
                                                         const Graph *graph)
{
    auto alloc = graph->GetAllocator();
    auto lalloc = graph->GetLocalAllocator();
    auto intervals = alloc->New<ArenaVector<LifeIntervals *>>(alloc->Adapter());
    ArenaQueue<const Split *> queue(lalloc->Adapter());

    auto lnRange = CopyIntervals(lifeIntervals, intervals);
    if (intervals->empty()) {
        return nullptr;
    }
    queue.push(lalloc->New<Split>(intervals->begin(), intervals->end(), lnRange.first, lnRange.second, nullptr));

    LifeIntervalsTreeNode *root {nullptr};

    // Split each interval into three parts:
    // 1) intervals covering mid point;
    // 2) intervals ended before mid point;
    // 3) intervals started after mid point.
    // Allocate tree node for (1), recursively process (2) and (3).
    while (!queue.empty()) {
        auto split = queue.front();
        queue.pop();
        if (split->end - split->begin <= 0) {
            continue;
        }

        auto midpoint = split->min + (split->max - split->min) / 2U;

        LifeNumber leftMinLn;
        LifeNumber leftMaxLn;
        auto leftMidpoint = PartitionLeftSplit(split->begin, split->end, midpoint, &leftMinLn, &leftMaxLn);

        LifeNumber rightMinLn;
        LifeNumber rightMaxLn;
        auto rightMidpoint = PartitionRightSplit(leftMidpoint, split->end, midpoint, &rightMinLn, &rightMaxLn);

        std::sort(leftMidpoint, rightMidpoint,
                  [](LifeIntervals *l, LifeIntervals *r) { return l->GetEnd() > r->GetEnd(); });

        auto node = alloc->New<LifeIntervalsTreeNode>(split->min, split->max, leftMidpoint, rightMidpoint);
        if (split->parent == nullptr) {
            root = node;
        } else if (split->parent->GetMidpoint() > midpoint) {
            split->parent->SetLeft(node);
        } else {
            split->parent->SetRight(node);
        }
        if (split->begin < leftMidpoint) {
            queue.push(lalloc->New<Split>(split->begin, leftMidpoint, leftMinLn, leftMaxLn, node));
        }
        if (rightMidpoint < split->end) {
            queue.push(lalloc->New<Split>(rightMidpoint, split->end, rightMinLn, rightMaxLn, node));
        }
    }
    return alloc->New<LifeIntervalsTree>(root);
}

}  // namespace ark::compiler
