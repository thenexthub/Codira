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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_LIVE_REGISTERS_H
#define COMPILER_OPTIMIZER_ANALYSIS_LIVE_REGISTERS_H

#include <optimizer/ir/inst.h>
#include <optimizer/ir/graph.h>
#include "libarkbase/utils/arena_containers.h"
#include "optimizer/pass.h"
#include "optimizer/analysis/liveness_analyzer.h"

namespace ark::compiler {

using LifeIntervalsIt = ArenaVector<LifeIntervals *>::iterator;
class LifeIntervalsTreeNode {
public:
    explicit LifeIntervalsTreeNode(LifeNumber minValue, LifeNumber maxValue, LifeIntervalsIt begin, LifeIntervalsIt end)
        : minValue_(minValue), maxValue_(maxValue), begin_(begin), end_(end)
    {
    }

    DEFAULT_MOVE_SEMANTIC(LifeIntervalsTreeNode);
    DEFAULT_COPY_SEMANTIC(LifeIntervalsTreeNode);
    ~LifeIntervalsTreeNode() = default;

    LifeNumber GetMidpoint() const
    {
        return (minValue_ + maxValue_) / 2U;
    }

    LifeNumber GetMinValue() const
    {
        return minValue_;
    }

    LifeNumber GetMaxValue() const
    {
        return maxValue_;
    }

    LifeIntervalsIt GetBegin() const
    {
        return begin_;
    }

    LifeIntervalsIt GetEnd() const
    {
        return end_;
    }

    LifeIntervalsTreeNode *GetLeft() const
    {
        return left_;
    }

    void SetLeft(LifeIntervalsTreeNode *left)
    {
        ASSERT(left_ == nullptr);
        left_ = left;
    }

    LifeIntervalsTreeNode *GetRight() const
    {
        return right_;
    }

    void SetRight(LifeIntervalsTreeNode *right)
    {
        ASSERT(right_ == nullptr);
        right_ = right;
    }

private:
    LifeNumber minValue_;
    LifeNumber maxValue_;
    LifeIntervalsIt begin_;
    LifeIntervalsIt end_;
    LifeIntervalsTreeNode *left_ {nullptr};
    LifeIntervalsTreeNode *right_ {nullptr};
};

// Simplified intervals tree implementation.
// Each LifeIntervalsTreeNode stores intervals covering the mid point associated with a node, these intervals
// sorted by the life range end in descending order. Every left child stores intervals ended before current node's
// mid point and every right child stores intervals started after current node's mid point.
class LifeIntervalsTree {
public:
    static LifeIntervalsTree *BuildIntervalsTree(Graph *graph)
    {
        ASSERT(graph->IsAnalysisValid<LivenessAnalyzer>());
        return LifeIntervalsTree::BuildIntervalsTree(graph->GetAnalysis<LivenessAnalyzer>().GetLifeIntervals(), graph);
    }

    static LifeIntervalsTree *BuildIntervalsTree(const ArenaVector<LifeIntervals *> &lifeIntervals, const Graph *graph);

    explicit LifeIntervalsTree(LifeIntervalsTreeNode *root) : root_(root) {};

    DEFAULT_MOVE_SEMANTIC(LifeIntervalsTree);
    DEFAULT_COPY_SEMANTIC(LifeIntervalsTree);
    ~LifeIntervalsTree() = default;

    template <bool LIVE_INPUTS = true, typename Func>
    void VisitIntervals(LifeNumber ln, Func func, const Inst *skipInst = nullptr) const
    {
        for (auto node = root_; node != nullptr; node = ln < node->GetMidpoint() ? node->GetLeft() : node->GetRight()) {
            if (ln < node->GetMinValue() || ln > node->GetMaxValue()) {
                // current node does not contain intervals covering target life number
                continue;
            }
            for (auto i = node->GetBegin(); i < node->GetEnd(); i++) {
                auto interval = *i;
                if (interval->GetInst() == skipInst) {
                    continue;
                }
                if (ln > interval->GetEnd()) {
                    // intervals are ordered by its end in descending order, so we can stop on first interval
                    // that ends before target life number
                    break;
                }
                if (interval->SplitCover<LIVE_INPUTS>(ln)) {
                    func(interval);
                }
            }
        }
    }

private:
    LifeIntervalsTreeNode *root_;
};

// Analysis collecting live intervals with assigned registers and
// allowing to visit those of them which are intersecting with specific instruction.
class LiveRegisters : public Analysis {
public:
    explicit LiveRegisters(Graph *graph) : Analysis(graph) {};

    NO_MOVE_SEMANTIC(LiveRegisters);
    NO_COPY_SEMANTIC(LiveRegisters);
    ~LiveRegisters() override = default;

    bool RunImpl() override
    {
        instLifeIntervalsTree_ = LifeIntervalsTree::BuildIntervalsTree(GetGraph());
        return true;
    }

    // Visit all live intervals with assigned registers intersecting with specified instruction
    // (excluding the interval for that instruction).
    template <bool LIVE_INPUTS = true, typename Func>
    void VisitIntervalsWithLiveRegisters(Inst *inst, Func func)
    {
        ASSERT(GetGraph()->IsAnalysisValid<LivenessAnalyzer>());

        if (instLifeIntervalsTree_ == nullptr) {
            return;
        }

        auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
        auto li = la.GetInstLifeIntervals(inst);
        instLifeIntervalsTree_->VisitIntervals<LIVE_INPUTS, Func>(li->GetBegin(), func, inst);
    }

    const char *GetPassName() const override
    {
        return "Live Registers";
    }

private:
    LifeIntervalsTree *instLifeIntervalsTree_ {nullptr};
};

}  // namespace ark::compiler
#endif  // COMPILER_OPTIMIZER_ANALYSIS_LIVE_REGISTERS_H
