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
#include "libabckit/src/codegen/codegen_dynamic.h"
#include "libabckit/src/adapter_dynamic/runtime_adapter_dynamic.h"
#include "generated/generate_ecma.inl"
#include "static_core/runtime/include/coretypes/tagged_value.h"

namespace libabckit {

void DoLda(ark::compiler::Register reg, std::vector<InsWrapper> &result)
{
    if (reg != ark::compiler::GetAccReg()) {
        if (reg > ark::compiler::INVALID_REG) {
            ASSERT(ark::compiler::IsFrameSizeLarge());
            result.emplace_back(PandasmWrapper::Create_MOV_Wrapper(CodeGenDynamic::RESERVED_REG, reg));
            result.emplace_back(PandasmWrapper::Create_LDA_Wrapper(CodeGenDynamic::RESERVED_REG));
        } else {
            result.emplace_back(PandasmWrapper::Create_LDA_Wrapper(reg));
        }
    }
}

void DoSta(ark::compiler::Register reg, std::vector<InsWrapper> &result)
{
    if (reg != ark::compiler::GetAccReg()) {
        if (reg > ark::compiler::INVALID_REG) {
            ASSERT(ark::compiler::IsFrameSizeLarge());
            result.emplace_back(PandasmWrapper::Create_STA_Wrapper(CodeGenDynamic::RESERVED_REG));
            result.emplace_back(PandasmWrapper::Create_MOV_Wrapper(reg, CodeGenDynamic::RESERVED_REG));
        } else {
            result.emplace_back(PandasmWrapper::Create_STA_Wrapper(reg));
        }
    }
}

void CodeGenDynamic::AppendCatchBlock(uint32_t typeId, const ark::compiler::BasicBlock *tryBegin,
                                      const ark::compiler::BasicBlock *tryEnd,
                                      const ark::compiler::BasicBlock *catchBegin,
                                      const ark::compiler::BasicBlock *catchEnd)
{
    auto cb = FunctionWrapper::CatchBlockWrapper();
    if (typeId != 0) {
        cb.exceptionRecord = irInterface_->GetTypeIdByOffset(typeId);
    }
    cb.tryBeginLabel = LabelName(tryBegin->GetId());
    cb.tryEndLabel = "end_" + LabelName(tryEnd->GetId());
    cb.catchBeginLabel = LabelName(catchBegin->GetId());
    cb.catchEndLabel = catchEnd == nullptr ? cb.catchBeginLabel : "end_" + LabelName(catchEnd->GetId());
    catchBlocks_.emplace_back(cb);
}

void CodeGenDynamic::VisitTryBegin(const ark::compiler::BasicBlock *bb)
{
    ASSERT(bb->IsTryBegin());
    auto tryInst = GetTryBeginInst(bb);
    auto tryEnd = tryInst->GetTryEndBlock();
    ASSERT(tryEnd != nullptr && tryEnd->IsTryEnd());

    bb->EnumerateCatchHandlers([&, bb, tryEnd](BasicBlock *catchHandler, size_t typeId) {
        AppendCatchBlock(typeId, bb, tryEnd, catchHandler);
        return true;
    });
}

bool CodeGenDynamic::RunImpl()
{
    Reserve(function_->ins.size());
    if (!GetGraph()->GetTryBeginBlocks().empty()) {
        // Workaround for AOT and JIT
        result_.emplace_back(PandasmWrapper::Create_NOP_Wrapper());
    }
    for (auto *bb : GetGraph()->GetBlocksLinearOrder()) {
        EmitLabel(LabelName(bb->GetId()));
        if (bb->IsTryEnd() || bb->IsCatch()) {
            auto label = "end_" + LabelName(bb->GetId());
            EmitLabel(label);
        }
        for (const auto &inst : bb->AllInsts()) {
            auto start = GetResult().size();
            VisitInstruction(inst);
            if (!GetStatus()) {
                return false;
            }
            auto end = GetResult().size();
            ASSERT(end >= start);
            for (auto i = start; i < end; ++i) {
                AddLineNumber(inst, i);
                AddColumnNumber(inst, i);
            }
        }
        if (bb->NeedsJump()) {
            EmitJump(bb);
        }
    }
    if (!GetStatus()) {
        return false;
    }
    // Visit try-blocks in order they were declared
    for (auto *bb : GetGraph()->GetTryBeginBlocks()) {
        VisitTryBegin(bb);
    }
    function_->ins = std::move(GetResult());
    function_->catchBlocks = catchBlocks_;
    return true;
}

void CodeGenDynamic::EmitJump(const BasicBlock *bb)
{
    BasicBlock *sucBb = nullptr;
    ASSERT(bb != nullptr);

    if (bb->GetLastInst() == nullptr) {
        ASSERT(bb->IsEmpty());
        sucBb = bb->GetSuccsBlocks()[0];
        result_.push_back(PandasmWrapper::Create_JMP_Wrapper(LabelName(sucBb->GetId())));
        return;
    }

    ASSERT(bb->GetLastInst() != nullptr);
    switch (bb->GetLastInst()->GetOpcode()) {
        case Opcode::If:
        case Opcode::IfImm:
            ASSERT(bb->GetSuccsBlocks().size() == ark::compiler::MAX_SUCCS_NUM);
            sucBb = bb->GetFalseSuccessor();
            break;
        default:
            sucBb = bb->GetSuccsBlocks()[0];
            break;
    }
    result_.push_back(PandasmWrapper::Create_JMP_Wrapper(LabelName(sucBb->GetId())));
}

void CodeGenDynamic::AddLineNumber([[maybe_unused]] const Inst *inst, [[maybe_unused]] const size_t idx) {}

void CodeGenDynamic::AddColumnNumber([[maybe_unused]] const Inst *inst, [[maybe_unused]] const uint32_t idx) {}

void CodeGenDynamic::EncodeSpillFillData(const ark::compiler::SpillFillData &sf)
{
    if (sf.SrcType() != ark::compiler::LocationType::REGISTER ||
        sf.DstType() != ark::compiler::LocationType::REGISTER) {
        std::cerr << "EncodeSpillFillData with unknown move type, src_type: " << static_cast<int>(sf.SrcType())
                  << " dst_type: " << static_cast<int>(sf.DstType());
        success_ = false;
        UNREACHABLE();
        return;
    }
    ASSERT(sf.GetType() != ark::compiler::DataType::NO_TYPE);
    ASSERT(sf.SrcValue() != ark::compiler::GetInvalidReg() && sf.DstValue() != ark::compiler::GetInvalidReg());

    if (sf.SrcValue() == sf.DstValue()) {
        return;
    }

    result_.emplace_back(PandasmWrapper::Create_MOV_Wrapper(sf.DstValue(), sf.SrcValue()));
}

void CodeGenDynamic::VisitSpillFill(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<CodeGenDynamic *>(visitor);
    for (auto sf : inst->CastToSpillFill()->GetSpillFills()) {
        enc->EncodeSpillFillData(sf);
    }
}

template <typename UnaryPred>
bool HasUserPredicate(Inst *inst, UnaryPred p)
{
    bool found = false;
    for (auto const &u : inst->GetUsers()) {
        if (p(u.GetInst())) {
            found = true;
            break;
        }
    }
    return found;
}

void CodeGenDynamic::VisitConstant(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<CodeGenDynamic *>(visitor);
    auto type = inst->GetType();

    InsWrapper movi;
    movi.regs.emplace_back(inst->GetDstReg());
    switch (type) {
        case ark::compiler::DataType::INT64:
        case ark::compiler::DataType::UINT64:
            enc->result_.emplace_back(PandasmWrapper::Create_LDAI_Wrapper(inst->CastToConstant()->GetInt64Value()));
            DoSta(inst->GetDstReg(), enc->result_);
            break;
        case ark::compiler::DataType::FLOAT64:
            enc->result_.emplace_back(PandasmWrapper::Create_FLDAI_Wrapper(inst->CastToConstant()->GetDoubleValue()));
            DoSta(inst->GetDstReg(), enc->result_);
            break;
        case ark::compiler::DataType::INT32:
        case ark::compiler::DataType::UINT32:
            enc->result_.emplace_back(PandasmWrapper::Create_LDAI_Wrapper(inst->CastToConstant()->GetInt32Value()));
            DoSta(inst->GetDstReg(), enc->result_);
            break;
        default:
            UNREACHABLE();
            std::cerr << "VisitConstant with unknown type" << type;
            enc->success_ = false;
    }
}

void CodeGenDynamic::EncodeSta(ark::compiler::Register reg, ark::compiler::DataType::Type type)
{
    std::string opc;
    if (type != ark::compiler::DataType::ANY) {
        UNREACHABLE();
        std::cerr << "EncodeSta with unknown type" << type;
        success_ = false;
    }
    InsWrapper sta;
    sta.opcode = "STA";
    sta.regs.emplace_back(reg);

    result_.emplace_back(sta);
}

// NOLINTNEXTLINE(readability-function-size)
void CodeGenDynamic::VisitIf(GraphVisitor *v, Inst *instBase)
{
    auto inst = instBase->CastToIf();

    size_t notZeroConstIdx = 1;
    if (IsZeroConstant(inst->GetInput(0).GetInst())) {
        notZeroConstIdx = 1;
    } else if (IsZeroConstant(inst->GetInput(1).GetInst())) {
        notZeroConstIdx = 0;
    } else {
        UNREACHABLE();
    }

    auto enc = static_cast<CodeGenDynamic *>(v);
    DoLda(inst->GetSrcReg(notZeroConstIdx), enc->result_);
    auto label = LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());
    switch (inst->GetCc()) {
        case ark::compiler::CC_NE:
            enc->result_.emplace_back(PandasmWrapper::Create_JNEZ_Wrapper(label));
            return;
        case ark::compiler::CC_EQ:
            enc->result_.emplace_back(PandasmWrapper::Create_JEQZ_Wrapper(label));
            return;
        default:
            UNREACHABLE();
    }
}

void CodeGenDynamic::VisitIfImm(GraphVisitor *v, Inst *instBase)
{
    auto inst = instBase->CastToIfImm();
    auto imm = inst->GetImm();
    if (imm == 0) {
        IfImmZero(v, instBase);
        return;
    }
}

void CodeGenDynamic::IfImmZero(GraphVisitor *v, Inst *instBase)
{
    auto enc = static_cast<CodeGenDynamic *>(v);
    auto inst = instBase->CastToIfImm();
    DoLda(inst->GetSrcReg(0), enc->result_);
    auto label = LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());
    switch (inst->GetCc()) {
        case ark::compiler::CC_EQ:
            enc->result_.emplace_back(PandasmWrapper::Create_JEQZ_Wrapper(label));
            return;
        case ark::compiler::CC_NE:
            enc->result_.emplace_back(PandasmWrapper::Create_JNEZ_Wrapper(label));
            return;
        default:
            UNREACHABLE();
    }
}

void CodeGenDynamic::VisitLoadString(GraphVisitor * /*v*/, Inst * /*instBase*/)
{
    UNREACHABLE();
}

void CodeGenDynamic::VisitLoadStringIntrinsic(GraphVisitor *v, Inst *instBase)
{
    InsWrapper ins;
    auto enc = static_cast<CodeGenDynamic *>(v);
    auto inst = instBase->CastToIntrinsic();
    enc->result_.emplace_back(
        PandasmWrapper::Create_LDA_STR_Wrapper(enc->irInterface_->GetStringIdByOffset(inst->GetImm(0))));
    DoSta(inst->GetDstReg(), enc->result_);
}

void CodeGenDynamic::VisitReturn(GraphVisitor *v, Inst *instBase)
{
    InsWrapper ins;
    auto enc = static_cast<CodeGenDynamic *>(v);
    auto inst = instBase->CastToReturn();
    switch (inst->GetType()) {
        case ark::compiler::DataType::ANY: {
#if defined(PANDA_WITH_ECMASCRIPT)
            auto test_arg = IsEcmaConstTemplate(inst->GetInput(0).GetInst());
            if (test_arg.has_value() && test_arg->IsUndefined()) {
                enc->result_.emplace_back(PandasmWrapper::Create_RETURNUNDEFINED_Wrapper());
                break;
            }
#endif
            DoLda(inst->GetSrcReg(0), enc->result_);
            enc->result_.emplace_back(PandasmWrapper::Create_RETURN_Wrapper());
            break;
        }
        default:
            std::cerr << "Codegen for " << ark::compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            enc->success_ = false;
    }
}

// NOLINTNEXTLINE(readability-function-size)
void CodeGenDynamic::VisitIntrinsic(GraphVisitor *visitor, Inst *instBase)
{
    ASSERT(instBase->IsIntrinsic());
    if (instBase->CastToIntrinsic()->GetIntrinsicId() ==
        ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_STRING) {
        VisitLoadStringIntrinsic(visitor, instBase);
        return;
    }
    VisitEcma(visitor, instBase);
}

void CodeGenDynamic::VisitCatchPhi(GraphVisitor *visitor, Inst *inst)
{
    // The Acc register stores the exception object.
    // Create an STA instruction if the exception is used later in virtual registers.
    if (inst->CastToCatchPhi()->IsAcc()) {
        bool hasRealUsers = false;
        for (auto &user : inst->GetUsers()) {
            if (!user.GetInst()->IsSaveState()) {
                hasRealUsers = true;
                break;
            }
        }
        if (hasRealUsers) {
            auto enc = static_cast<CodeGenDynamic *>(visitor);
            DoSta(inst->GetDstReg(), enc->result_);
        }
    }
}

#include "generated/insn_selection_dynamic.cpp"
}  // namespace libabckit
