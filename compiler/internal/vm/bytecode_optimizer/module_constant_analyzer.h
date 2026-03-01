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

#ifndef BYTECODE_OPTIMIZER_MODULE_CONSTANT_ANALYZER_H
#define BYTECODE_OPTIMIZER_MODULE_CONSTANT_ANALYZER_H

#include <cstring>
#include "bytecode_optimizer/ir_interface.h"
#include "compiler/optimizer/pass.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/graph_visitor.h"
#include "libpandabase/utils/arena_containers.h"

namespace panda::bytecodeopt {

using compiler::BasicBlock;
using compiler::Inst;
using compiler::Opcode;
using compiler::RuntimeInterface;

class ConstantValue;

using ModuleConstantAnalysisResult = std::unordered_map<uint32_t, ConstantValue *>;

/*
 * ModuleConstantAnalyzer is used to scan the IR of the given function, and
 * analysis the initial value of all module constants (represented in
 * BytecodeAnalysisResult for now, extracts from bytecode later after new
 * stconstmodulevar instruction is enabled)
 */
class ModuleConstantAnalyzer : public compiler::Analysis, public compiler::GraphVisitor {
public:
    explicit ModuleConstantAnalyzer(compiler::Graph *graph,
                                    const std::unordered_set<uint32_t> &const_local_export_slots,
                                    ModuleConstantAnalysisResult &analysis_result,
                                    const BytecodeOptIrInterface *iface);

    NO_MOVE_SEMANTIC(ModuleConstantAnalyzer);
    NO_COPY_SEMANTIC(ModuleConstantAnalyzer);
    ~ModuleConstantAnalyzer() override = default;

    const char *GetPassName() const override
    {
        return "ModuleConstantAnalysis";
    }

    bool RunImpl() override;

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }

protected:
    static void VisitIntrinsic(GraphVisitor *v, Inst *inst);

#include "compiler/optimizer/ir/visitor.inc"

private:
    ConstantValue *GetInstConstValue(Inst *inst);
    ConstantValue *GetConstantInstConstValue(compiler::ConstantInst *inst);
    ConstantValue *GetIntrinsicInstConstValue(compiler::IntrinsicInst *inst);
    ConstantValue *GetLoadStringInstConstValue(compiler::LoadFromPool *inst);
    bool IsConstModuleVar(uint32_t slot);
    void RecordModuleConstValue(uint32_t slot, ConstantValue *value);

    const std::unordered_set<uint32_t> &const_local_export_slots_;
    ModuleConstantAnalysisResult &analysis_result_;
    const BytecodeOptIrInterface *ir_interface_;
};

}  // namespace panda::bytecodeopt

#endif  // BYTECODE_OPTIMIZER_MODULE_CONSTANT_ANALYZER_H