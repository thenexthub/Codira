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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_SPLIT_RESOLVER_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_SPLIT_RESOLVER_H

#include "compiler/optimizer/analysis/liveness_analyzer.h"
#include "compiler/optimizer/ir/graph.h"

namespace panda::compiler {
class SplitResolver {
public:
    explicit SplitResolver(Graph *graph) : SplitResolver(graph, &graph->GetAnalysis<LivenessAnalyzer>()) {}
    explicit SplitResolver(Graph *graph, LivenessAnalyzer *liveness) : graph_(graph), liveness_(liveness) {}

    DEFAULT_COPY_SEMANTIC(SplitResolver);
    DEFAULT_MOVE_SEMANTIC(SplitResolver);

    ~SplitResolver() = default;

    void Run();

private:
    void ConnectSiblings(LifeIntervals *interval);
    void ProcessBlock(BasicBlock *block);
    SpillFillInst *CreateSpillFillForSiblings(Inst *connect_at);
    SpillFillInst *CreateSpillFillForSplitMove(BasicBlock *source_block);
    void ConnectSpiltFromPredBlock(BasicBlock *src_bb, LifeIntervals *src_interval, BasicBlock *target_bb,
                                   LifeIntervals *target_split);

    template <SpillFillType expected_type>
    static bool Is(Inst *inst)
    {
        ASSERT(inst != nullptr);
        if (!inst->IsSpillFill()) {
            return false;
        }
        auto sf = inst->CastToSpillFill();
        ASSERT(sf->GetSpillFillType() != SpillFillType::UNKNOWN);
        return sf->GetSpillFillType() == expected_type;
    }

private:
    Graph *graph_;
    LivenessAnalyzer *liveness_;
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_SPLIT_RESOLVER_H
