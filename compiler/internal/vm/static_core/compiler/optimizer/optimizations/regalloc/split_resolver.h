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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_SPLIT_RESOLVER_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_SPLIT_RESOLVER_H

#include "compiler/optimizer/analysis/liveness_analyzer.h"
#include "compiler/optimizer/ir/graph.h"

namespace ark::compiler {
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
    SpillFillInst *CreateSpillFillForSiblings(Inst *connectAt);
    SpillFillInst *CreateSpillFillForSplitMove(BasicBlock *sourceBlock);
    void ConnectSplitFromPredBlock(BasicBlock *srcBb, LifeIntervals *srcInterval, BasicBlock *targetBb,
                                   LifeIntervals *targetSplit);

    template <SpillFillType EXPECTED_TYPE>
    static bool Is(Inst *inst)
    {
        ASSERT(inst != nullptr);
        if (!inst->IsSpillFill()) {
            return false;
        }
        auto sf = inst->CastToSpillFill();
        ASSERT(sf->GetSpillFillType() != SpillFillType::UNKNOWN);
        return sf->GetSpillFillType() == EXPECTED_TYPE;
    }

private:
    Graph *graph_;
    LivenessAnalyzer *liveness_;
};
}  // namespace ark::compiler
#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_SPLIT_RESOLVER_H
