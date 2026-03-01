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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_LOWERING_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_LOWERING_H

#include "compiler_logger.h"
#include "compiler_options.h"
#include "optimizer/ir/graph.h"
#include "optimizer/ir/graph_visitor.h"

namespace panda::compiler {
// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Lowering : public Optimization, public GraphVisitor {
    using Optimization::Optimization;

public:
    explicit Lowering(Graph *graph) : Optimization(graph) {}

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return options.IsCompilerLowering();
    }

    const char *GetPassName() const override
    {
        return "Lowering";
    }

    void InvalidateAnalyses() override;
    static constexpr uint8_t HALF_SIZE = 32;
    static constexpr uint8_t WORD_SIZE = 64;

private:
    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }
    static void VisitIfImm([[maybe_unused]] GraphVisitor *v, Inst *inst);

#include "optimizer/ir/visitor.inc"

    static bool ConstantFitsCompareImm(Inst *cst, uint32_t size, ConditionCode cc);
    // We'd like to swap only to make second operand immediate
    static bool BetterToSwapCompareInputs(Inst *cmp);
    // Optimize order of input arguments for decreasing using accumulator (Bytecodeoptimizer only).
    static void OptimizeIfInput(compiler::Inst *if_inst);
    static void LowerIf(IfImmInst *inst);
    static void InPlaceLowerIfImm(IfImmInst *inst, Inst *input, Inst *cst, ConditionCode cc);
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_LOWERING_H
