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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_REG_ALLOC_GRAPH_COLORING_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_REG_ALLOC_GRAPH_COLORING_H

#include "reg_alloc_base.h"
#include "compiler_logger.h"
#include "interference_graph.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "reg_map.h"
#include "utils/arena_containers.h"
#include "utils/small_vector.h"

namespace panda::compiler {
using InstructionsRanges = ArenaDeque<LifeIntervals *>;

class RegAllocGraphColoring : public RegAllocBase {
    struct WorkingRanges {
        explicit WorkingRanges(ArenaAllocator *allocator)
            : regular(allocator->Adapter()), physical(allocator->Adapter())
        {
        }

        InstructionsRanges regular;   // NOLINT(misc-non-private-member-variables-in-classes)
        InstructionsRanges physical;  // NOLINT(misc-non-private-member-variables-in-classes)
    };

public:
    explicit RegAllocGraphColoring(Graph *graph);
    RegAllocGraphColoring(Graph *graph, size_t regs_count);

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

protected:
    bool Allocate() override;

private:
    void InitWorkingRanges(WorkingRanges *general_ranges, WorkingRanges *fp_ranges);
    void BuildIG(InterferenceGraph *ig, WorkingRanges *ranges);
    IndexVector PrecolorIG(InterferenceGraph *ig, const RegisterMap &map);
    void BuildBias(InterferenceGraph *ig, const IndexVector &affinity_nodes);
    void AddAffinityEdges(InterferenceGraph *ig, ColorNode *node, IndexVector *affinity_nodes);
    void AddAffinityEdgeToSibling(InterferenceGraph *ig, ColorNode *node, IndexVector *affinity_nodes);
    Register AllocateRegisters(InterferenceGraph *ig, WorkingRanges *ranges, const RegisterMap &map);
    void Remap(const InterferenceGraph &ig, const RegisterMap &map);
    void InitMap(RegisterMap *map, bool is_vector);
    void Presplit(WorkingRanges *ranges);
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_REG_ALLOC_GRAPH_COLORING_H
