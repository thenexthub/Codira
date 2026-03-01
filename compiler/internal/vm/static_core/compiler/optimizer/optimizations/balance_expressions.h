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

#ifndef COMPILER_OPTIMIZER_BALANCE_EXPRESSIONS_H
#define COMPILER_OPTIMIZER_BALANCE_EXPRESSIONS_H

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"
#include "libarkbase/utils/arena_containers.h"

namespace ark::compiler {
class BalanceExpressions : public Optimization {
public:
    // For current realization, any binary commutative and associative operator suits;

    static constexpr size_t DEFAULT_GRAPH_DEPTH = 3;

    explicit BalanceExpressions(Graph *graph)
        : Optimization(graph),
          sources_(graph->GetLocalAllocator()->Adapter()),
          operators_(graph->GetLocalAllocator()->Adapter())
    {
        sources_.reserve(1U << DEFAULT_GRAPH_DEPTH);
        operators_.reserve(1U << DEFAULT_GRAPH_DEPTH);
    }

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "BalanceExpressions";
    }

    bool IsEnable() const override
    {
        return g_options.IsCompilerBalanceExpressions();
    }

    void Dump(std::ostream *out) const;

    void InvalidateAnalyses() override;

private:
    void ProcessBB(BasicBlock *bb);
    bool SuitableInst(Inst *inst);

    Inst *ProccesExpressionChain(Inst *lastOperator);
    void AnalyzeInputsRec(Inst *inst);
    void TryExtendChainRec(Inst *inst);

    bool NeedsOptimization();
    Inst *OptimizeExpression(Inst *instAfterExpr);
    template <bool IS_FIRST_CALL>
    Inst *AllocateSourcesRec(size_t firstIdx, size_t lastIdx);
    Inst *GetOperand(size_t firstIdx, size_t lastIdx);
    Inst *GenerateElementalOperator(Inst *lhs, Inst *rhs);
    void Reset();
    void Dump();

    void SetOpcode(Opcode opc)
    {
        opcode_ = opc;
    }
    Opcode GetOpcode() const
    {
        return opcode_;
    }
    void SetBB(BasicBlock *bb)
    {
        bb_ = bb;
    }
    BasicBlock *GetBB() const
    {
        return bb_;
    }
    void SetIsApplied(bool value)
    {
        isApplied_ = value;
    }

private:
    Opcode opcode_ {Opcode::INVALID};
    Marker processedInstMrk_ = UNDEF_MARKER;

    // i.e. critical path
    size_t exprCurDepth_ {0};
    size_t exprMaxDepth_ {0};

    BasicBlock *bb_ {nullptr};

    // Expression's terms:
    InstVector sources_;

    // Expression's operators:
    InstVector operators_;
    size_t operatorsAllocIdx_ = 0;

    bool isApplied_ {false};
};

inline std::ostream &operator<<(std::ostream &os, const BalanceExpressions &expr)
{
    expr.Dump(&os);
    return os;
}
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_BALANCE_EXPRESSIONS_H
