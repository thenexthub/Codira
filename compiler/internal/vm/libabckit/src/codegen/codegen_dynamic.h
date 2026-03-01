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

#ifndef LIBABCKIT_SRC_CODEGEN_CODEGEN_DYNAMIC_H
#define LIBABCKIT_SRC_CODEGEN_CODEGEN_DYNAMIC_H

#include "static_core/compiler/optimizer/pass.h"
#include "static_core/compiler/optimizer/ir/basicblock.h"
#include "static_core/compiler/optimizer/ir/graph.h"
#include "static_core/compiler/optimizer/ir/graph_visitor.h"
#include "static_core/compiler/optimizer/ir/constants.h"
#include "static_core/compiler/optimizer/ir/inst.h"
#include "libabckit/src/codegen/common.h"
#include "libabckit/src/wrappers/pandasm_wrapper.h"
#include "libabckit/src/ir_impl.h"

namespace libabckit {

using ark::compiler::BasicBlock;
using ark::compiler::Inst;
using ark::compiler::Opcode;

void DoLda(ark::compiler::Register reg, std::vector<InsWrapper> &result);
void DoSta(ark::compiler::Register reg, std::vector<InsWrapper> &result);

// NOLINTNEXTLINE(fuchsia-multiple-inheritance)
class CodeGenDynamic : public ark::compiler::Optimization, public ark::compiler::GraphVisitor {
public:
    explicit CodeGenDynamic(ark::compiler::Graph *graph, FunctionWrapper *function,
                            const AbckitIrInterface *irInterface)
        : ark::compiler::Optimization(graph), function_(function), irInterface_(irInterface)
    {
    }

    constexpr static ark::compiler::Register RESERVED_REG = 0U;
    bool RunImpl() override;
    const char *GetPassName() const override
    {
        return "CodeGenDynamic";
    }
    std::vector<InsWrapper> GetEncodedInstructions() const
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

    const std::vector<InsWrapper> &GetResult() const
    {
        return result_;
    }

    std::vector<InsWrapper> &&GetResult()
    {
        return std::move(result_);
    }

    static std::string LabelName(uint32_t id)
    {
        return "label_" + std::to_string(id);
    }

    void EmitLabel(const std::string &label)
    {
        InsWrapper l;
        l.label = label;
        l.setLabel = true;
        result_.emplace_back(l);
    }

    void EmitJump(const BasicBlock *bb);

    void EncodeSpillFillData(const ark::compiler::SpillFillData &sf);
    void EncodeSta(ark::compiler::Register reg, ark::compiler::DataType::Type type);
    void AddLineNumber(const Inst *inst, const size_t idx);
    void AddColumnNumber(const Inst *inst, const uint32_t idx);

    const ark::ArenaVector<BasicBlock *> &GetBlocksToVisit() const override
    {
        return GetGraph()->GetBlocksRPO();
    }
    static void VisitSpillFill(GraphVisitor *visitor, Inst *inst);
    static void VisitConstant(GraphVisitor *visitor, Inst *inst);
    static void VisitCatchPhi(GraphVisitor *visitor, Inst *inst);

    static void VisitIf(GraphVisitor *v, Inst *instBase);
    static void VisitIfImm(GraphVisitor *v, Inst *instBase);
    static void IfImmZero(GraphVisitor *v, Inst *instBase);
    static void VisitIntrinsic(GraphVisitor *visitor, Inst *instBase);
    static void VisitLoadString(GraphVisitor *v, Inst *instBase);
    static void VisitLoadStringIntrinsic(GraphVisitor *v, Inst *instBase);
    static void VisitReturn(GraphVisitor *v, Inst *instBase);

    static void VisitEcma(GraphVisitor *v, Inst *instBase);

#include "generated/codegen_visitors_dyn.inc"

#include "generated/insn_selection_dynamic.h"

    void VisitDefault(Inst *inst) override
    {
        std::cerr << "Opcode " << ark::compiler::GetOpcodeString(inst->GetOpcode())
                  << " not yet implemented in codegen\n";
        success_ = false;
    }

#include "compiler/optimizer/ir/visitor.inc"

private:
    void AppendCatchBlock(uint32_t typeId, const ark::compiler::BasicBlock *tryBegin,
                          const ark::compiler::BasicBlock *tryEnd, const ark::compiler::BasicBlock *catchBegin,
                          const ark::compiler::BasicBlock *catchEnd = nullptr);
    void VisitTryBegin(const ark::compiler::BasicBlock *bb);

private:
    FunctionWrapper *function_;
    const AbckitIrInterface *irInterface_;

    std::vector<InsWrapper> res_;
    std::vector<FunctionWrapper::CatchBlockWrapper> catchBlocks_;

    bool success_ {true};
    std::vector<InsWrapper> result_;
};

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_CODEGEN_CODEGEN_DYNAMIC_H
