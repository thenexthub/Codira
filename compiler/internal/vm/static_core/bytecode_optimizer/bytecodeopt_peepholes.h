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

#ifndef PANDA_BYTECODE_OPTIMIZER_BYTECODEOPT_PEEPHOLES_H
#define PANDA_BYTECODE_OPTIMIZER_BYTECODEOPT_PEEPHOLES_H

#include "bytecodeopt_options.h"
#include "compiler/optimizer/pass.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/graph_visitor.h"
#include "compiler/optimizer/ir/inst.h"
#include "libarkbase/utils/arena_containers.h"
#include "runtime_adapter.h"

/*
 * BytecodeOptPeepholes
 *
 * BytecodeOptPeepholes includes now only transformation of NewObject and related instructions into
 * InitObject
 */

namespace ark::bytecodeopt {

using compiler::BasicBlock;
using compiler::CallInst;
using compiler::Inst;
using compiler::Opcode;

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class BytecodeOptPeepholes : public compiler::Optimization, public compiler::GraphVisitor {
public:
    explicit BytecodeOptPeepholes(compiler::Graph *graph) : compiler::Optimization(graph) {}
    ~BytecodeOptPeepholes() override = default;
    NO_COPY_SEMANTIC(BytecodeOptPeepholes);
    NO_MOVE_SEMANTIC(BytecodeOptPeepholes);

    bool RunImpl() override;

    const char *GetPassName() const override
    {
        return "BytecodeOptPeepholes";
    }

    bool IsEnable() const override
    {
        return g_options.IsBytecodeOptPeepholes();
    }

    /* This visitor replaces combination of NewObject, SaveState,
     * NullCheck and CallStatic with InitObject. It is used in order to replace newobj, sta and call
     * in some ark bytecode with one instruction initobj.
     */
    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

    static void VisitNewObject(GraphVisitor *v, Inst *inst);

public:
    bool IsApplied()
    {
        return isApplied_;
    }

#include "compiler/optimizer/ir/visitor.inc"

private:
    void SetIsApplied()
    {
        isApplied_ = true;
    }

    bool isApplied_ {false};
};

}  // namespace ark::bytecodeopt

#endif  // PANDA_BYTECODE_OPTIMIZER_BYTECODEOPT_PEEPHOLES_H
