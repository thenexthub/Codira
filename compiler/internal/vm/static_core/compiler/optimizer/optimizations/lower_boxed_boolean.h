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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_LOWER_BOXED_BOOLEAN_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_LOWER_BOXED_BOOLEAN_H

#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/analysis.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/inst.h"
#include "optimizer/pass.h"
#include "optimizer/ir/graph_visitor.h"

namespace ark::compiler {

/*
 * This pass lowers boxed Boolean values into primitive boolean operations.
 * It targets LoadObject instructions that access the 'value' field of std.core.Boolean and replaces them
 * with primitive values (0 or 1) when the input is known at compile time.
 */

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class LowerBoxedBoolean : public Optimization, public GraphVisitor {
public:
    explicit LowerBoxedBoolean(Graph *graph)
        : Optimization(graph), instReplacements_(graph->GetLocalAllocator()->Adapter())
    {
    }

    NO_MOVE_SEMANTIC(LowerBoxedBoolean);
    NO_COPY_SEMANTIC(LowerBoxedBoolean);
    ~LowerBoxedBoolean() override = default;

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "LowerBoxedBoolean";
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    bool IsEnable() const override
    {
        return g_options.IsCompilerLowerBoxedBoolean();
    }

    static void VisitCompare(GraphVisitor *v, Inst *inst);
    static void VisitLoadObject(GraphVisitor *v, Inst *inst);

#include "optimizer/ir/visitor.inc"

private:
    static void ProcessInput(GraphVisitor *v, Inst *inst);
    static void ProcessLoadStatic(GraphVisitor *v, Inst *inst);
    static void ProcessPhi(GraphVisitor *v, Inst *inst);
    static bool IsValidLoadObjectInput(Inst *input);
    static bool IsCompareWithNullPtr(Inst *inst);
    static bool HasOnlyKnownUsers(Inst *inst);
    static bool ProcessSaveState(Inst *saveState, Inst *inst);
    static bool CheckSaveStateUsers(Inst *saveStateInst, Inst *inst);
    static bool IsNullCheckUsingInput(Inst *inst, Inst *input);
    static std::optional<uint32_t> GetBooleanFieldValue(Inst *inst);

    void SetInstReplacement(Inst *oldInst, Inst *newInst);
    Inst *GetReplacement(Inst *inst);
    bool HasReplacement(Inst *inst) const;
    void SetVisited(Inst *inst);
    bool IsVisited(Inst *inst) const;
    void SetApplied();

    bool isApplied_ {false};
    Marker visitedMarker_ {UNDEF_MARKER};
    ArenaUnorderedMap<Inst *, Inst *> instReplacements_;
};

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_LOWER_BOXED_BOOLEAN_H
