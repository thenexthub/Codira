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

#ifndef LIBABCKIT_SRC_CODEGEN_CODEGEN_STATIC_H
#define LIBABCKIT_SRC_CODEGEN_CODEGEN_STATIC_H

#include "static_core/assembler/assembly-function.h"
#include "static_core/assembler/assembly-ins.h"
#include "static_core/compiler/optimizer/pass.h"
#include "static_core/compiler/optimizer/ir/basicblock.h"
#include "static_core/compiler/optimizer/ir/graph.h"
#include "static_core/compiler/optimizer/ir/graph_visitor.h"
#include "libarkbase/utils/logger.h"

#include "ins_create_api.h"
#include "libabckit/src/ir_impl.h"

namespace libabckit {

// CC-OFFNXT(WordsTool.95) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark;

using compiler::BasicBlock;
using compiler::Inst;
using compiler::Opcode;

constexpr compiler::Register MIN_REGISTER_NUMBER = 0;
constexpr compiler::Register MAX_NUM_SHORT_CALL_ARGS = 2;
constexpr compiler::Register MAX_NUM_NON_RANGE_ARGS = 4;
constexpr compiler::Register MAX_NUM_INPUTS = MAX_NUM_NON_RANGE_ARGS;
constexpr ark::compiler::Register NUM_COMPACTLY_ENCODED_REGS = 16;
[[maybe_unused]] constexpr compiler::Register MAX_8_BIT_REG = 255 - 1U;  // exclude INVALID_REG

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
class CodeGenStatic : public compiler::Optimization, public compiler::GraphVisitor {
public:
    explicit CodeGenStatic(compiler::Graph *graph, pandasm::Function *function, const AbckitIrInterface *iface)
        : compiler::Optimization(graph), function_(function), irInterface_(iface)
    {
    }
    ~CodeGenStatic() override = default;
    NO_COPY_SEMANTIC(CodeGenStatic);
    NO_MOVE_SEMANTIC(CodeGenStatic);

    constexpr static compiler::Register RESERVED_REG = 0U;
    bool RunImpl() override;
    const char *GetPassName() const override
    {
        return "CodeGenStatic";
    }
    std::vector<pandasm::Ins> const &GetEncodedInstructions() const noexcept
    {
        return res_;
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

    static std::string LabelName(uint32_t id)
    {
        return "label_" + std::to_string(id);
    }

    static compiler::Register GetMaxReg()
    {
        return compiler::GetInvalidReg() - 1;
    }

    void EmitLabel(const std::string &label)
    {
        pandasm::Ins l {};
        l.SetLabel(label);
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
    static void VisitInitObject(GraphVisitor * /*visitor*/, Inst * /*inst*/)
    {
        UNREACHABLE();
    }
    static void VisitCatchPhi(GraphVisitor *visitor, Inst *inst);

    static void VisitIntrinsic(GraphVisitor *visitor, Inst *inst);

    static void VisitIf(GraphVisitor *v, Inst *instBase);
    static void VisitIfImm(GraphVisitor *v, Inst *instBase);
    static void VisitCast(GraphVisitor *v, Inst *instBase);
    static void IfImmZero(GraphVisitor *v, Inst *instBase);
    static void IfImmNonZero(GraphVisitor *v, Inst *instBase);
    static void IfImm64(GraphVisitor *v, Inst *instBase);
    static void CallHandler(GraphVisitor *visitor, Inst *inst, std::string methodId);
    static void CallHandler(GraphVisitor *visitor, Inst *inst);
    static void VisitStoreObject(GraphVisitor * /*v*/, Inst * /*instBase*/)
    {
        UNREACHABLE();
    }
    static void VisitStoreStatic(GraphVisitor * /*v*/, Inst * /*instBase*/)
    {
        UNREACHABLE();
    }
    static void VisitLoadObject(GraphVisitor * /*v*/, Inst * /*instBase*/)
    {
        UNREACHABLE();
    }
    static void VisitLoadStatic(GraphVisitor * /*v*/, Inst * /*instBase*/)
    {
        UNREACHABLE();
    }
    static void VisitLoadString(GraphVisitor * /*v*/, Inst * /*instBase*/)
    {
        UNREACHABLE();
    }
    static void VisitReturn(GraphVisitor *v, Inst *instBase);

#include "generated/codegen_visitors_static.inc"
#include "generated/insn_selection_static.h"

    void VisitDefault(Inst *inst) override
    {
        std::cerr << "Opcode " << compiler::GetOpcodeString(inst->GetOpcode()) << " not yet implemented in CodeGen";
        success_ = false;
    }

#include "compiler/optimizer/ir/visitor.inc"

private:
    void AppendCatchBlock(uint32_t typeId, const compiler::BasicBlock *tryBegin, const compiler::BasicBlock *tryEnd,
                          const compiler::BasicBlock *catchBegin, const compiler::BasicBlock *catchEnd = nullptr);
    void VisitTryBegin(const compiler::BasicBlock *bb);

private:
    pandasm::Function *function_;
    const AbckitIrInterface *irInterface_;

    std::vector<pandasm::Ins> res_;
    std::vector<pandasm::Function::CatchBlock> catchBlocks_;

    bool success_ {true};
    std::vector<pandasm::Ins> result_;
};

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_CODEGEN_CODEGEN_STATIC_H
