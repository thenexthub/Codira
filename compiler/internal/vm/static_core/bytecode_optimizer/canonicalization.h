/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_BYTECODE_OPT_CANONICALIZATION_H
#define PANDA_BYTECODE_OPT_CANONICALIZATION_H

#include "bytecodeopt_options.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/pass.h"
#include "compiler/optimizer/ir/graph_visitor.h"

namespace ark::bytecodeopt {

using ark::compiler::BasicBlock;
using ark::compiler::Inst;
using ark::compiler::Opcode;

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class Canonicalization : public compiler::Optimization, public compiler::GraphVisitor {
public:
    explicit Canonicalization(compiler::Graph *graph) : compiler::Optimization(graph) {}
    ~Canonicalization() override = default;
    NO_COPY_SEMANTIC(Canonicalization);
    NO_MOVE_SEMANTIC(Canonicalization);

    bool RunImpl() override;
    const char *GetPassName() const override
    {
        return "Canonicalization";
    }
    bool IsEnable() const override
    {
        return g_options.IsCanonicalization();
    }

    bool GetStatus() const
    {
        return result_;
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    void VisitCommutative(Inst *inst);
    static void VisitCompare([[maybe_unused]] GraphVisitor *v, Inst *inst);

    static bool TrySwapReverseInput(Inst *inst);
    static bool TrySwapConstantInput(Inst *inst);
#include "optimizer/ir/visitor.inc"
private:
    bool result_ {false};
};
}  // namespace ark::bytecodeopt

#endif  // PANDA_BYTECODE_OPT_CANONICALIZATION_H
