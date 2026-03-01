/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_BYTECODE_OPTIMIZER_CALL_DEVIRTUALIZATION_H
#define PANDA_BYTECODE_OPTIMIZER_CALL_DEVIRTUALIZATION_H

#include "bytecodeopt_options.h"
#include "compiler/optimizer/pass.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/graph_visitor.h"
#include "compiler/optimizer/ir/inst.h"
#include "compiler/optimizer/ir/analysis.h"
#include "runtime_adapter.h"

/*
 * CallDevirtualization
 *
 * CallDevirtualization includes now only replacement of CallVirtual with CallStatic
 */

namespace ark::bytecodeopt {

using compiler::BasicBlock;
using compiler::CallInst;
using compiler::Inst;
using compiler::Opcode;

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class CallDevirtualization : public compiler::Optimization, public compiler::GraphVisitor {
public:
    explicit CallDevirtualization(compiler::Graph *graph) : compiler::Optimization(graph) {}
    ~CallDevirtualization() override = default;
    NO_COPY_SEMANTIC(CallDevirtualization);
    NO_MOVE_SEMANTIC(CallDevirtualization);

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "CallDevirtualization";
    }

    bool IsEnable() const override
    {
        return g_options.IsBytecodeOptCallDevirtualize();
    }

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

public:
    bool IsApplied()
    {
        return isApplied_;
    }

    static void VisitCallVirtual([[maybe_unused]] GraphVisitor *v, Inst *inst);

#include "compiler/optimizer/ir/visitor.inc"

private:
    void SetIsApplied()
    {
        isApplied_ = true;
    }

    bool isApplied_ {false};

    RuntimeInterface::ClassPtr GetClassPtr(Inst *inst, size_t inputNum);
    bool TryDevirtualize(CallInst *callVirtual);
};

}  // namespace ark::bytecodeopt

#endif  // PANDA_BYTECODE_OPTIMIZER_CALL_DEVIRTUALIZATION_H
