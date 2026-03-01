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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_REG_ALLOC_GRAPH_COLORING_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_REG_ALLOC_GRAPH_COLORING_H

#include "reg_alloc_base.h"
#include "compiler_logger.h"
#include "interference_graph.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/code_generator/registers_description.h"
#include "optimizer/optimizations/regalloc/working_ranges.h"
#include "reg_map.h"
#include "libarkbase/utils/arena_containers.h"
#include "libarkbase/utils/small_vector.h"

namespace ark::compiler {
class RegAllocGraphColoring : public RegAllocBase {
public:
    explicit RegAllocGraphColoring(Graph *graph);
    PANDA_PUBLIC_API RegAllocGraphColoring(Graph *graph, size_t regsCount);
    PANDA_PUBLIC_API RegAllocGraphColoring(Graph *graph, LocationMask mask);

    const char *GetPassName() const override
    {
        return "RegAllocGraphColoring";
    }

    bool AbortIfFailed() const override
    {
        return true;
    }

    static const size_t DEFAULT_VECTOR_SIZE = 64;
    using IndexVector = SmallVector<unsigned, DEFAULT_VECTOR_SIZE>;
    using IndexVectorPair = std::pair<IndexVector, IndexVector>;

protected:
    bool Allocate() override;

private:
    void InitWorkingRanges(WorkingRanges *generalRanges, WorkingRanges *fpRanges);
    void FillPhysicalNodes(InterferenceGraph *ig, WorkingRanges *ranges, ArenaVector<ColorNode *> &physicalNodes);
    void BuildIG(InterferenceGraph *ig, WorkingRanges *ranges, bool rematConstants = false);
    IndexVector PrecolorIG(InterferenceGraph *ig);
    IndexVector PrecolorIG(InterferenceGraph *ig, const RegisterMap &map);
    void BuildBias(InterferenceGraph *ig, const IndexVector &affinityNodes);
    void WalkNodes(IndexVectorPair &&vectors, NodeVector &nodes, ColorNode node, InterferenceGraph *ig,
                   const IndexVector &affinityNodes);
    void AddAffinityEdgesToPhi(InterferenceGraph *ig, const ColorNode &node, IndexVector *affinityNodes);
    void AddAffinityEdgesToSiblings(InterferenceGraph *ig, const ColorNode &node, IndexVector *affinityNodes);
    void AddAffinityEdgesToPhysicalNodes(InterferenceGraph *ig, IndexVector *affinityNodes);
    void AddAffinityEdge(InterferenceGraph *ig, IndexVector *affinityNodes, const ColorNode &node, LifeIntervals *li);
    bool AllocateRegisters(InterferenceGraph *ig, WorkingRanges *ranges, WorkingRanges *stackRanges,
                           const RegisterMap &map);
    bool AllocateSlots(InterferenceGraph *ig, WorkingRanges *stackRanges);
    void Remap(const InterferenceGraph &ig, const RegisterMap &map);
    bool MapSlots(const InterferenceGraph &ig);
    void InitMap(RegisterMap *map, bool isVector);
    void Presplit(WorkingRanges *ranges);
    void SparseIG(InterferenceGraph *ig, unsigned regsCount, WorkingRanges *ranges, WorkingRanges *stackRanges);
    void SpillInterval(LifeIntervals *interval, WorkingRanges *ranges, WorkingRanges *stackRanges);
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_REG_ALLOC_GRAPH_COLORING_H
