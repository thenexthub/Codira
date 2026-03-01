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

#ifndef BYTECODE_OPTIMIZER_REG_ACC_ALLOC_H
#define BYTECODE_OPTIMIZER_REG_ACC_ALLOC_H

#include "optimizer/ir/graph.h"
#include "optimizer/pass.h"
#include "compiler_options.h"

namespace panda::bytecodeopt {

class RegAccAlloc : public compiler::Optimization {
    using Optimization::Optimization;

public:
    explicit RegAccAlloc(compiler::Graph *graph) : compiler::Optimization(graph), acc_marker_(graph->NewMarker()) {};

    ~RegAccAlloc() override = default;

    bool IsEnable() const override
    {
        return compiler::options.IsCompilerRegAccAlloc();
    }

    const char *GetPassName() const override
    {
        return "RegAccAlloc";
    }

    bool RunImpl() override;

private:
    bool IsPhiOptimizable(compiler::Inst *phi) const;
    bool IsAccRead(compiler::Inst *inst) const;
    bool IsAccWrite(compiler::Inst *inst) const;

    bool CanUserReadAcc(compiler::Inst *inst, compiler::Inst *user) const;
    bool IsPhiAccReady(compiler::Inst *phi) const;
    void SetNeedLda(compiler::Inst *inst, bool need);

    void InitializeSourceRegisters();
    void MarkAccForPhiInstructions();
    void MarkAccForInstructions(compiler::BasicBlock *block);
    void UpdateInstructionsAfterMark(compiler::BasicBlock *block);

    compiler::Marker acc_marker_ {0};
};

}  // namespace panda::bytecodeopt

#endif  // BYTECODE_OPTIMIZER_REG_ACC_ALLOC_H
