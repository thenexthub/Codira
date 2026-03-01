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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_SAVESTATEOPTIMIZATION_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_SAVESTATEOPTIMIZATION_H

#include "compiler_logger.h"
#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/graph_visitor.h"

namespace ark::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class SaveStateOptimization : public Optimization, public GraphVisitor {
    using Optimization::Optimization;

public:
    explicit SaveStateOptimization(Graph *graph) : Optimization(graph) {}

    NO_MOVE_SEMANTIC(SaveStateOptimization);
    NO_COPY_SEMANTIC(SaveStateOptimization);
    ~SaveStateOptimization() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "SaveStateOptimization";
    }

    bool IsEnable() const override
    {
        return g_options.IsCompilerSaveStateElimination();
    }

    void RemoveSafePoints();

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    bool IsApplied() const
    {
        return isApplied_;
    }

    void SetApplied()
    {
        isApplied_ = true;
    }

    bool HaveCalls()
    {
        return haveCalls_;
    }

#include <savestate_optimization_call_visitors.inl>
    static void VisitSaveState(GraphVisitor *v, Inst *inst);
    static void VisitSaveStateDeoptimize(GraphVisitor *v, Inst *inst);
    void VisitDefault(Inst *inst) override;

#include "optimizer/ir/visitor.inc"

private:
    void SetHaveCalls()
    {
        haveCalls_ = true;
    }

    bool TryToRemoveRedundantSaveState(Inst *inst);
    bool RequireRegMap(Inst *inst);

private:
    bool isApplied_ {false};
    bool haveCalls_ {false};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_SAVESTATEOPTIMIZATION_H
