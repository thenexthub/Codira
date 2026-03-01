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

#ifndef PANDA_BYTECODE_OPT_CODEGEN_H
#define PANDA_BYTECODE_OPT_CODEGEN_H

#include "assembler/assembly-function.h"
#include "assembler/assembly-ins.h"
#include "ins_create_api.h"
#include "ir_interface.h"
#include "compiler/optimizer/pass.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/graph_visitor.h"
#include "libarkbase/utils/logger.h"
#include "common.h"

namespace ark::bytecodeopt {

using compiler::BasicBlock;
using compiler::Inst;
using compiler::Opcode;

void DoLdaObj(compiler::Register reg, std::vector<pandasm::Ins> &result);
void DoLda(compiler::Register reg, std::vector<pandasm::Ins> &result);
void DoLda64(compiler::Register reg, std::vector<pandasm::Ins> &result);
void DoStaObj(compiler::Register reg, std::vector<pandasm::Ins> &result);
void DoSta(compiler::Register reg, std::vector<pandasm::Ins> &result);
void DoSta64(compiler::Register reg, std::vector<pandasm::Ins> &result);
void DoSta(compiler::DataType::Type type, compiler::Register reg, std::vector<pandasm::Ins> &result);
void DoLdaDyn(compiler::Register reg, std::vector<pandasm::Ins> &result);
void DoStaDyn(compiler::Register reg, std::vector<pandasm::Ins> &result);

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class BytecodeGen : public compiler::Optimization, public compiler::GraphVisitor {
public:
    explicit BytecodeGen(compiler::Graph *graph, pandasm::Function *function, const BytecodeOptIrInterface *iface)
        : compiler::Optimization(graph), function_(function), irInterface_(iface)
    {
    }
    ~BytecodeGen() override = default;
    NO_COPY_SEMANTIC(BytecodeGen);
    NO_MOVE_SEMANTIC(BytecodeGen);

    bool RunImpl() override;
    const char *GetPassName() const override
    {
        return "BytecodeGen";
    }

    void Reserve(size_t resSize = 0)
    {
        if (resSize > 0) {
            result_.reserve(resSize);
        }
    }

    bool GetStatus() const
    {
        return success_;
    }

    const std::vector<pandasm::Ins> &GetResult() const
    {
        return result_;
    }

    std::vector<pandasm::Ins> &&GetResult()
    {
        return std::move(result_);
    }

    static compiler::Register GetMaxReg()
    {
        return compiler::GetInvalidReg() - 1;
    }

    static std::string LabelName(uint32_t id)
    {
        return "label_" + std::to_string(id);
    }

    void EmitLabel(std::string &&label)
    {
        pandasm::Ins l {};
        l.SetLabel(std::move(label));
        result_.emplace_back(std::move(l));
    }

    void EmitJump(const BasicBlock *bb);

    void EncodeSpillFillData(const compiler::SpillFillData &sf);
    void EncodeSta(compiler::Register reg, compiler::DataType::Type type);
    void AddLineNumber(const Inst *inst, size_t idx);
    void AddColumnNumber(const Inst *inst, uint32_t idx);
    void AddLineAndColumnNumber(const Inst *inst, size_t idx);

    const ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }
    static void VisitSpillFill(GraphVisitor *v, Inst *inst);
    static void VisitConstant(GraphVisitor *v, Inst *inst);
    static void VisitCallStatic(GraphVisitor *visitor, Inst *inst);
    static void VisitCallVirtual(GraphVisitor *visitor, Inst *inst);
    static void VisitInitObject(GraphVisitor *visitor, Inst *inst);
    static void VisitCatchPhi(GraphVisitor *visitor, Inst *inst);

    static void VisitIf(GraphVisitor *v, Inst *instBase);
    static void VisitIfImm(GraphVisitor *v, Inst *instBase);
    static void VisitCast(GraphVisitor *v, Inst *instBase);
    static void IfImmZero(GraphVisitor *v, Inst *instBase);
    static void IfImmNonZero(GraphVisitor *v, Inst *instBase);
    static void IfImm64(GraphVisitor *v, Inst *instBase);
    static void VisitIntrinsic(GraphVisitor *v, Inst *instBase);
    static void CallHandler(GraphVisitor *visitor, Inst *inst, std::string methodId, bool isDevirtual = false);
    static void CallHandler(GraphVisitor *visitor, Inst *inst);
    static void VisitStoreObject(GraphVisitor *v, Inst *instBase);
    static void VisitStoreStatic(GraphVisitor *v, Inst *instBase);
    static void VisitLoadObject(GraphVisitor *v, Inst *instBase);
    static void VisitLoadStatic(GraphVisitor *v, Inst *instBase);
    static void VisitLoadString(GraphVisitor *v, Inst *instBase);
    static void VisitReturn(GraphVisitor *v, Inst *instBase);
    static void VisitReturnAny(GraphVisitor *v, Inst *instBase);

    static void VisitCastValueToAnyType(GraphVisitor *v, Inst *instBase);

    static void VisitEcma(GraphVisitor *v, Inst *instBase);
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
    void AppendCatchBlock(uint32_t typeId, const compiler::BasicBlock *tryBegin, const compiler::BasicBlock *tryEnd,
                          const compiler::BasicBlock *catchBegin, const compiler::BasicBlock *catchEnd = nullptr);
    void VisitTryBegin(const compiler::BasicBlock *bb);

private:
    pandasm::Function *function_;
    const BytecodeOptIrInterface *irInterface_;

    std::vector<pandasm::Ins> res_;
    std::vector<pandasm::Function::CatchBlock> catchBlocks_;

    bool success_ {true};
    std::vector<pandasm::Ins> result_;
};

}  // namespace ark::bytecodeopt

#endif  // PANDA_BYTECODE_OPT_CODEGEN_H
