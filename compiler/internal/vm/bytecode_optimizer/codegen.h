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

#ifndef BYTECODE_OPTIMIZER_CODEGEN_H
#define BYTECODE_OPTIMIZER_CODEGEN_H

#include "assembler/annotation.h"
#include "assembler/assembly-function.h"
#include "ins_create_api.h"
#include "ir_interface.h"
#include "compiler/optimizer/pass.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/graph_visitor.h"
#include "utils/logger.h"

namespace panda::bytecodeopt {

using compiler::BasicBlock;
using compiler::Inst;
using compiler::Opcode;

void DoLda(compiler::Register reg, std::vector<pandasm::InsPtr> &result);
void DoSta(compiler::Register reg, std::vector<pandasm::InsPtr> &result);

class BytecodeGen : public compiler::Optimization, public compiler::GraphVisitor {
public:
    explicit BytecodeGen(compiler::Graph *graph, pandasm::Function *function,
        const BytecodeOptIrInterface *iface, pandasm::Program *prog)
        : compiler::Optimization(graph), function_(function), ir_interface_(iface), program_(prog)
    {
    }
    ~BytecodeGen() override = default;
    bool RunImpl() override;
    const char *GetPassName() const override
    {
        return "BytecodeGen";
    }
    const std::vector<pandasm::InsPtr> &GetEncodedInstructions() const noexcept
    {
        return res_;
    }

    void Reserve(size_t res_size = 0)
    {
        if (res_size > 0) {
            result_.reserve(res_size);
        }
    }

    bool GetStatus() const
    {
        return success_;
    }

    const std::vector<pandasm::InsPtr> &GetResult() const
    {
        return result_;
    }

    std::vector<pandasm::InsPtr> &&GetResult()
    {
        return std::move(result_);
    }

    static std::string LabelName(uint32_t id)
    {
        return "label_" + std::to_string(id);
    }

    void EmitLabel(const std::string label)
    {
        result_.emplace_back(new pandasm::LabelIns(label));
    }

    void EmitJump(const BasicBlock *bb);

    void EncodeSpillFillData(const compiler::SpillFillData &sf);
    void EncodeSta(compiler::Register reg, compiler::DataType::Type type);
    void AddLineNumber(const Inst *inst, const size_t idx);
    void AddColumnNumber(const Inst *inst, const uint32_t idx);

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }
    static void VisitSpillFill(GraphVisitor *visitor, Inst *inst);
    static void VisitConstant(GraphVisitor *visitor, Inst *inst);
    static void VisitCatchPhi(GraphVisitor *visitor, Inst *inst);

    static void VisitIf(GraphVisitor *v, Inst *inst_base);
    static void VisitIfImm(GraphVisitor *v, Inst *inst_base);
    static void IfImmZero(GraphVisitor *v, Inst *inst_base);
    static void VisitIntrinsic(GraphVisitor *visitor, Inst *inst_base);
    static void VisitLoadString(GraphVisitor *v, Inst *inst_base);
    static void VisitReturn(GraphVisitor *v, Inst *inst_base);

    static void VisitCastValueToAnyType(GraphVisitor *v, Inst *inst_base);

    static void VisitEcma(GraphVisitor *v, Inst *inst_base);
    static void IfEcma(GraphVisitor *v, compiler::IfInst *inst);

#include "generated/codegen_visitors.inc"

#include "generated/insn_selection.h"

    void VisitDefault(Inst *inst) override
    {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Opcode " << compiler::GetOpcodeString(inst->GetOpcode())
                                       << " not yet implemented in codegen";
        success_ = false;
    }

#include "compiler/optimizer/ir/visitor.inc"

private:
    void AppendCatchBlock(uint32_t type_id, const compiler::BasicBlock *try_begin, const compiler::BasicBlock *try_end,
                          const compiler::BasicBlock *catch_begin, const compiler::BasicBlock *catch_end = nullptr);
    void VisitTryBegin(const compiler::BasicBlock *bb);

private:
    pandasm::Function *function_;
    const BytecodeOptIrInterface *ir_interface_;
    pandasm::Program *program_;

    std::vector<pandasm::InsPtr> res_;
    std::vector<pandasm::Function::CatchBlock> catch_blocks_;

    bool success_ {true};
    std::vector<pandasm::InsPtr> result_;
};

}  // namespace panda::bytecodeopt

#endif  // BYTECODE_OPTIMIZER_CODEGEN_H
