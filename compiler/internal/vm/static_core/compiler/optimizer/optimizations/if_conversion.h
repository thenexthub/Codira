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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_IF_CONVERSION_H
#define COMPILER_OPTIMIZER_ANALYSIS_IF_CONVERSION_H

#include "optimizer/ir/graph.h"
#include "libarkbase/utils/arena_containers.h"

namespace ark::compiler {
class BasicBlock;
class Graph;

class IfConversion : public Optimization {
public:
    explicit IfConversion(Graph *graph, uint32_t limit = 2) : Optimization(graph), limit_(limit)
    {
        auto encoder = graph->GetEncoder();
        ASSERT(encoder != nullptr);
        canEncodeFloatSelect_ = encoder->CanEncodeFloatSelect();
    }

    NO_MOVE_SEMANTIC(IfConversion);
    NO_COPY_SEMANTIC(IfConversion);
    ~IfConversion() override = default;

    bool RunImpl() override;

    bool IsEnable() const override
    {
        return g_options.IsCompilerIfConversion();
    }

    const char *GetPassName() const override
    {
        return "IfConversion";
    }

    void InvalidateAnalyses() override;

private:
    uint32_t limit_;
    bool canEncodeFloatSelect_ {false};
    bool TryTriangle(BasicBlock *bb);
    bool TryDiamond(BasicBlock *bb);
    static bool LoopInvariantPreventConversion(BasicBlock *bb);
    static bool IsConvertable(BasicBlock *bb, uint32_t *instCount);
    bool IsPhisAllowed(BasicBlock *bb, BasicBlock *pred1, BasicBlock *pred2, uint32_t *phiCount);
    static bool IsConditionChainPhi(Inst *phi);
    uint32_t GetIfcLimit(BasicBlock *bb);
    bool TryOptimizeSelectInsts();
    bool TryOptimizeSelectInst(Inst *selectInst);
    bool TryOptimizeSelectInstByMergingUnaryOperation(Inst *selectInst, Inst *v0, Inst *v1);
    bool TryOptimizeSelectInstWithIncrement(Inst *selectInst, Inst *v0, Inst *v1);
    bool TryOptimizeSelectInstWithBitwiseInversion(Inst *selectInst, Inst *v0, Inst *v1);
    bool TryOptimizeSelectInstWithArithmeticNegation(Inst *selectInst, Inst *v0, Inst *v1);
    // CC-OFFNXT(G.FUN.01-CPP) Internal helper function
    bool OptimizeSelectInstWithTransform(SelectTransformType transform, Inst *selectInst, Inst *v0, Inst *v1,
                                         bool inputsSwapped = false, bool secondInputIsIdentity = false);
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_IF_CONVERSION_H
