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
#include "libabckit/src/codegen/codegen_static.h"
#include "static_core/runtime/include/coretypes/tagged_value.h"
namespace ark {
#include <generated/abckit_intrinsics.inl>
}  // namespace ark

namespace libabckit {

uint8_t AccReadIndex(const compiler::Inst *inst)
{
    // Calls can have accumulator at any position, return false for them
    ASSERT(!inst->IsCall());

    if (inst->IsIntrinsic() && inst->IsAccRead()) {
        ASSERT(inst->GetInputsCount() >= 1U);
        return 0U;
    }

    return 0U;
}

// This method is used by bytecode optimizer's codegen.
bool CanConvertToIncI(const compiler::BinaryImmOperation *binop)
{
    ASSERT(binop->GetBasicBlock()->GetGraph()->IsRegAllocApplied());
    ASSERT(binop->GetOpcode() == compiler::Opcode::AddI || binop->GetOpcode() == compiler::Opcode::SubI);

    // IncI works on the same register.
    if (binop->GetSrcReg(0U) != binop->GetDstReg()) {
        return false;
    }

    // IncI cannot write accumulator.
    if (binop->GetSrcReg(0U) == compiler::GetAccReg()) {
        return false;
    }

    // IncI users cannot read from accumulator.
    // While Addi/SubI stores the output in accumulator, IncI works directly on registers.
    for (const auto &user : binop->GetUsers()) {
        const auto *uinst = user.GetInst();

        if (uinst->IsCall()) {
            continue;
        }

        const uint8_t index = AccReadIndex(uinst);
        if (uinst->GetInput(index).GetInst() == binop && uinst->GetSrcReg(index) == compiler::GetAccReg()) {
            return false;
        }
    }

    constexpr uint64_t BITMASK = 0xffffffff;
    // Define min and max values of i4 type.
    // NOLINTNEXTLINE(readability-identifier-naming)
    constexpr int32_t min = -8;
    // NOLINTNEXTLINE(readability-identifier-naming)
    constexpr int32_t max = 7;

    int32_t imm = binop->GetImm() & BITMASK;
    // Note: subi 3 is the same as inci v2, -3.
    if (binop->GetOpcode() == compiler::Opcode::SubI) {
        imm = -imm;
    }

    // IncI works only with 4 bits immediates.
    return imm >= min && imm <= max;
}

void DoLdaObj(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    if (reg != compiler::GetAccReg()) {
        if (reg > compiler::INVALID_REG) {
            ASSERT(compiler::IsFrameSizeLarge());
            result.emplace_back(pandasm::Create_MOV_OBJ(CodeGenStatic::RESERVED_REG, reg));
            result.emplace_back(pandasm::Create_LDA_OBJ(CodeGenStatic::RESERVED_REG));
        } else {
            result.emplace_back(pandasm::Create_LDA_OBJ(reg));
        }
    }
}

void DoLda(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    if (reg != compiler::GetAccReg()) {
        if (reg > compiler::INVALID_REG) {
            ASSERT(compiler::IsFrameSizeLarge());
            result.emplace_back(pandasm::Create_MOV(CodeGenStatic::RESERVED_REG, reg));
            result.emplace_back(pandasm::Create_LDA(CodeGenStatic::RESERVED_REG));
        } else {
            result.emplace_back(pandasm::Create_LDA(reg));
        }
    }
}

void DoLda64(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    if (reg != compiler::GetAccReg()) {
        if (reg > compiler::INVALID_REG) {
            ASSERT(compiler::IsFrameSizeLarge());
            result.emplace_back(pandasm::Create_MOV_64(CodeGenStatic::RESERVED_REG, reg));
            result.emplace_back(pandasm::Create_LDA_64(CodeGenStatic::RESERVED_REG));
        } else {
            result.emplace_back(pandasm::Create_LDA_64(reg));
        }
    }
}

void DoStaObj(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    if (reg != compiler::GetAccReg()) {
        if (reg > compiler::INVALID_REG) {
            ASSERT(compiler::IsFrameSizeLarge());
            result.emplace_back(pandasm::Create_STA_OBJ(CodeGenStatic::RESERVED_REG));
            result.emplace_back(pandasm::Create_MOV_OBJ(reg, CodeGenStatic::RESERVED_REG));
        } else {
            result.emplace_back(pandasm::Create_STA_OBJ(reg));
        }
    }
}

void DoSta(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    if (reg != compiler::GetAccReg()) {
        if (reg > compiler::INVALID_REG) {
            ASSERT(compiler::IsFrameSizeLarge());
            result.emplace_back(pandasm::Create_STA(CodeGenStatic::RESERVED_REG));
            result.emplace_back(pandasm::Create_MOV(reg, CodeGenStatic::RESERVED_REG));
        } else {
            result.emplace_back(pandasm::Create_STA(reg));
        }
    }
}

void DoSta64(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    if (reg != compiler::GetAccReg()) {
        if (reg > compiler::INVALID_REG) {
            ASSERT(compiler::IsFrameSizeLarge());
            result.emplace_back(pandasm::Create_STA_64(CodeGenStatic::RESERVED_REG));
            result.emplace_back(pandasm::Create_MOV_64(reg, CodeGenStatic::RESERVED_REG));
        } else {
            result.emplace_back(pandasm::Create_STA_64(reg));
        }
    }
}

void DoSta(compiler::DataType::Type type, compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    if (compiler::DataType::Is64Bits(type, Arch::NONE)) {
        DoSta64(reg, result);
    } else {
        DoSta(reg, result);
    }
}

void DoLdaDyn(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    ASSERT(reg <= compiler::INVALID_REG);
    if (reg != compiler::GetAccReg()) {
        result.emplace_back(pandasm::Create_LDA_DYN(reg));
    }
}

void DoStaDyn(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    ASSERT(reg <= compiler::INVALID_REG);
    if (reg != compiler::GetAccReg()) {
        result.emplace_back(pandasm::Create_STA_DYN(reg));
    }
}

void CodeGenStatic::AppendCatchBlock(uint32_t typeId, const compiler::BasicBlock *tryBegin,
                                     const compiler::BasicBlock *tryEnd, const compiler::BasicBlock *catchBegin,
                                     const compiler::BasicBlock *catchEnd)
{
    auto cb = pandasm::Function::CatchBlock();
    if (typeId != 0U) {
        cb.exceptionRecord = irInterface_->GetTypeIdByOffset(typeId);
    }
    cb.tryBeginLabel = CodeGenStatic::LabelName(tryBegin->GetId());
    cb.tryEndLabel = "end_" + CodeGenStatic::LabelName(tryEnd->GetId());
    cb.catchBeginLabel = CodeGenStatic::LabelName(catchBegin->GetId());
    cb.catchEndLabel = catchEnd == nullptr ? cb.catchBeginLabel : "end_" + CodeGenStatic::LabelName(catchEnd->GetId());
    catchBlocks_.emplace_back(cb);
}

void CodeGenStatic::VisitTryBegin(const compiler::BasicBlock *bb)
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

void CodeGenStatic::AddLineAndColumnNumber(const compiler::Inst *inst, size_t idx)
{
    AddLineNumber(inst, idx);
}

bool CodeGenStatic::RunImpl()
{
    Reserve(function_->ins.size());
    for (auto *bb : GetGraph()->GetBlocksLinearOrder()) {
        EmitLabel(CodeGenStatic::LabelName(bb->GetId()));
        // NOTE(ivagin) if (bb->IsTryEnd() || bb->IsCatchEnd()) {
        if (bb->IsTryEnd() || bb->IsCatch()) {
            auto label = "end_" + CodeGenStatic::LabelName(bb->GetId());
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
                AddLineAndColumnNumber(inst, i);
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

void CodeGenStatic::EmitJump(const BasicBlock *bb)
{
    BasicBlock *sucBb = nullptr;
    ASSERT(bb != nullptr);

    if (bb->GetLastInst() == nullptr) {
        ASSERT(bb->IsEmpty());
        sucBb = bb->GetSuccsBlocks()[0U];
        result_.push_back(pandasm::Create_JMP(CodeGenStatic::LabelName(sucBb->GetId())));
        return;
    }

    ASSERT(bb->GetLastInst() != nullptr);
    switch (bb->GetLastInst()->GetOpcode()) {
        case Opcode::If:
        case Opcode::IfImm:
            ASSERT(bb->GetSuccsBlocks().size() == compiler::MAX_SUCCS_NUM);
            sucBb = bb->GetFalseSuccessor();
            break;
        default:
            sucBb = bb->GetSuccsBlocks()[0U];
            break;
    }
    result_.push_back(pandasm::Create_JMP(CodeGenStatic::LabelName(sucBb->GetId())));
}

void CodeGenStatic::AddLineNumber([[maybe_unused]] const Inst *inst, [[maybe_unused]] const size_t idx) {}

void CodeGenStatic::AddColumnNumber([[maybe_unused]] const Inst *inst, [[maybe_unused]] const uint32_t idx) {}

void CodeGenStatic::EncodeSpillFillData(const compiler::SpillFillData &sf)
{
    if (sf.SrcType() != compiler::LocationType::REGISTER || sf.DstType() != compiler::LocationType::REGISTER) {
        std::cerr << "EncodeSpillFillData with unknown move type, src_type: " << static_cast<int>(sf.SrcType())
                  << " dst_type: " << static_cast<int>(sf.DstType()) << std::endl;
        success_ = false;
        UNREACHABLE();
        return;
    }
    ASSERT(sf.GetType() != compiler::DataType::NO_TYPE);
    ASSERT(sf.SrcValue() != compiler::GetInvalidReg() && sf.DstValue() != compiler::GetInvalidReg());

    if (sf.SrcValue() == sf.DstValue()) {
        return;
    }

    pandasm::Ins move;
    switch (sf.GetType()) {
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64:
        case compiler::DataType::FLOAT64:
            move = pandasm::Create_MOV_64(sf.DstValue(), sf.SrcValue());
            break;
        case compiler::DataType::REFERENCE:
            move = pandasm::Create_MOV_OBJ(sf.DstValue(), sf.SrcValue());
            break;
        default:
            move = pandasm::Create_MOV(sf.DstValue(), sf.SrcValue());
    }
    result_.emplace_back(std::move(move));
}

void CodeGenStatic::VisitSpillFill(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<CodeGenStatic *>(visitor);
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

static void VisitConstant32(compiler::Inst *inst, std::vector<pandasm::Ins> &res)
{
    auto type = inst->GetType();
    ASSERT(compiler::DataType::Is32Bits(type, Arch::NONE));

    auto const dstReg = inst->GetDstReg();

    switch (type) {
        case compiler::DataType::BOOL:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT32:
        case compiler::DataType::UINT32:
            if (dstReg == compiler::GetAccReg()) {
                res.emplace_back(pandasm::Create_LDAI(inst->CastToConstant()->GetInt32Value()));
            } else {
                res.emplace_back(pandasm::Create_MOVI(dstReg, inst->CastToConstant()->GetInt32Value()));
            }
            break;
        case compiler::DataType::FLOAT32:
            if (dstReg == compiler::GetAccReg()) {
                res.emplace_back(pandasm::Create_FLDAI(inst->CastToConstant()->GetFloatValue()));
            } else {
                res.emplace_back(pandasm::Create_FMOVI(dstReg, inst->CastToConstant()->GetFloatValue()));
            }
            break;
        default:
            UNREACHABLE();
    }
}

static void VisitConstant64(compiler::Inst *inst, std::vector<pandasm::Ins> &res)
{
    auto type = inst->GetType();
    ASSERT(compiler::DataType::Is64Bits(type, Arch::NONE));

    auto const dstReg = inst->GetDstReg();

    switch (type) {
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64:
            if (dstReg == compiler::GetAccReg()) {
                res.emplace_back(pandasm::Create_LDAI_64(inst->CastToConstant()->GetInt64Value()));
            } else {
                res.emplace_back(pandasm::Create_MOVI_64(dstReg, inst->CastToConstant()->GetInt64Value()));
            }
            break;
        case compiler::DataType::FLOAT64:
            if (dstReg == compiler::GetAccReg()) {
                res.emplace_back(pandasm::Create_FLDAI_64(inst->CastToConstant()->GetDoubleValue()));
            } else {
                res.emplace_back(pandasm::Create_FMOVI_64(dstReg, inst->CastToConstant()->GetDoubleValue()));
            }
            break;
        default:
            UNREACHABLE();
    }
}

void CodeGenStatic::VisitConstant(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<CodeGenStatic *>(visitor);
    auto type = inst->GetType();

    switch (type) {
        case compiler::DataType::BOOL:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT32:
        case compiler::DataType::UINT32:
        case compiler::DataType::FLOAT32:
            VisitConstant32(inst, enc->result_);
            break;
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64:
        case compiler::DataType::FLOAT64:
            VisitConstant64(inst, enc->result_);
            break;
        case compiler::DataType::ANY: {
            UNREACHABLE();
        }
        default:
            UNREACHABLE();
            std::cerr << "VisitConstant with unknown type" << type << std::endl;
            enc->success_ = false;
    }
}

void CodeGenStatic::EncodeSta(compiler::Register reg, compiler::DataType::Type type)
{
    pandasm::Opcode opc;
    switch (type) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32:
        case compiler::DataType::FLOAT32:
            opc = pandasm::Opcode::STA;
            break;
        case compiler::DataType::UINT64:
        case compiler::DataType::INT64:
        case compiler::DataType::FLOAT64:
            opc = pandasm::Opcode::STA_64;
            break;
        case compiler::DataType::ANY:
            opc = pandasm::Opcode::STA_DYN;
            break;
        case compiler::DataType::REFERENCE:
            opc = pandasm::Opcode::STA_OBJ;
            break;
        default:
            UNREACHABLE();
            std::cerr << "EncodeSta with unknown type" << type << std::endl;
            success_ = false;
    }
    pandasm::Ins sta {};
    sta.opcode = opc;
    sta.EmplaceReg(reg);

    result_.emplace_back(std::move(sta));
}

static pandasm::Opcode ChooseCallOpcode(compiler::Opcode op, size_t nargs)
{
    ASSERT(op == compiler::Opcode::CallStatic || op == compiler::Opcode::CallVirtual ||
           op == compiler::Opcode::Intrinsic);
    if (nargs > MAX_NUM_NON_RANGE_ARGS) {
        switch (op) {
            case compiler::Opcode::CallStatic:
                return pandasm::Opcode::CALL_RANGE;
            case compiler::Opcode::CallVirtual:
                return pandasm::Opcode::CALL_VIRT_RANGE;
            case compiler::Opcode::Intrinsic:
                return pandasm::Opcode::INITOBJ_RANGE;
            default:
                UNREACHABLE();
        }
    } else if (nargs > MAX_NUM_SHORT_CALL_ARGS) {
        switch (op) {
            case compiler::Opcode::CallStatic:
                return pandasm::Opcode::CALL;
            case compiler::Opcode::CallVirtual:
                return pandasm::Opcode::CALL_VIRT;
            case compiler::Opcode::Intrinsic:
                return pandasm::Opcode::INITOBJ;
            default:
                UNREACHABLE();
        }
    }
    switch (op) {
        case compiler::Opcode::CallStatic:
            return pandasm::Opcode::CALL_SHORT;
        case compiler::Opcode::CallVirtual:
            return pandasm::Opcode::CALL_VIRT_SHORT;
        case compiler::Opcode::Intrinsic:
            return pandasm::Opcode::INITOBJ_SHORT;
        default:
            UNREACHABLE();
    }
}

static pandasm::Opcode ChooseCallAccOpcode(pandasm::Opcode op)
{
    switch (op) {
        case pandasm::Opcode::CALL_SHORT:
            return pandasm::Opcode::CALL_ACC_SHORT;
        case pandasm::Opcode::CALL:
            return pandasm::Opcode::CALL_ACC;
        case pandasm::Opcode::CALL_VIRT_SHORT:
            return pandasm::Opcode::CALL_VIRT_ACC_SHORT;
        case pandasm::Opcode::CALL_VIRT:
            return pandasm::Opcode::CALL_VIRT_ACC;
        default:
            return pandasm::Opcode::INVALID;
    }
}

static ark::compiler::CallInst *CastToCall(ark::compiler::Inst *inst)
{
    switch (inst->GetOpcode()) {
        case compiler::Opcode::CallStatic:
            return inst->CastToCallStatic();
        case compiler::Opcode::CallVirtual:
            return inst->CastToCallVirtual();
        default:
            UNREACHABLE();
    }
}

void CodeGenStatic::CallHandler(GraphVisitor *visitor, Inst *inst, std::string methodId)
{
    auto op = inst->GetOpcode();
    ASSERT(op == compiler::Opcode::CallStatic || op == compiler::Opcode::CallVirtual ||
           op == compiler::Opcode::Intrinsic);
    auto *enc = static_cast<CodeGenStatic *>(visitor);
    auto sfCount = inst->GetInputsCount() - (inst->RequireState() ? 1U : 0U);
    size_t start = 0U;
    auto nargs = sfCount;
    pandasm::Ins ins;

    ins.opcode = ChooseCallOpcode(op, nargs);

    if (nargs > MAX_NUM_NON_RANGE_ARGS) {
#ifndef NDEBUG
        auto startReg = inst->GetSrcReg(start);
        ASSERT(startReg <= MAX_8_BIT_REG);
        for (auto i = start; i < sfCount; ++i) {
            auto reg = inst->GetSrcReg(i);
            ASSERT(reg - startReg == static_cast<int>(i - start));  // check 'range-ness' of registers
        }
#endif  // !NDEBUG
        ins.EmplaceReg(inst->GetSrcReg(start));
    } else {
        for (size_t i = start; i < sfCount; ++i) {
            auto reg = inst->GetSrcReg(i);
            ASSERT(reg < NUM_COMPACTLY_ENCODED_REGS || reg == compiler::GetAccReg());
            if (reg == compiler::GetAccReg()) {
                ASSERT(inst->IsCallOrIntrinsic());
                ins.EmplaceImm(i);
                ins.opcode = ChooseCallAccOpcode(ins.opcode);
            } else {
                ins.EmplaceReg(reg);
            }
        }
    }
    ins.EmplaceID(std::move(methodId));
    enc->result_.emplace_back(std::move(ins));
    if (inst->GetDstReg() != compiler::GetInvalidReg() && inst->GetDstReg() != compiler::GetAccReg()) {
        enc->EncodeSta(inst->GetDstReg(), inst->GetType());
    }
}

void CodeGenStatic::CallHandler(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<CodeGenStatic *>(visitor);
    uint32_t methodOffset = 0;
    if (inst->IsIntrinsic()) {
        ASSERT(IsAbcKitInitObject(inst->CastToIntrinsic()->GetIntrinsicId()));
        methodOffset = inst->CastToIntrinsic()->GetImm(0);
    } else {
        methodOffset = CastToCall(inst)->GetCallMethodId();
    }
    CallHandler(visitor, inst, enc->irInterface_->GetMethodIdByOffset(methodOffset));
}

void CodeGenStatic::VisitCallStatic(GraphVisitor *visitor, Inst *inst)
{
    CallHandler(visitor, inst);
}

void CodeGenStatic::VisitCallVirtual(GraphVisitor *visitor, Inst *inst)
{
    CallHandler(visitor, inst);
}

static void VisitIf32(compiler::IfInst *inst, std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(Is32Bits(inst->GetInputType(0U), Arch::NONE));

    DoLda(inst->GetSrcReg(0U), res);

    compiler::Register src = inst->GetSrcReg(1);
    std::string label = CodeGenStatic::LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());

    switch (inst->GetCc()) {
        case compiler::CC_EQ: {
            res.emplace_back(pandasm::Create_JEQ(src, label));
            break;
        }
        case compiler::CC_NE: {
            res.emplace_back(pandasm::Create_JNE(src, label));
            break;
        }
        case compiler::CC_LT: {
            res.emplace_back(pandasm::Create_JLT(src, label));
            break;
        }
        case compiler::CC_LE: {
            res.emplace_back(pandasm::Create_JLE(src, label));
            break;
        }
        case compiler::CC_GT: {
            res.emplace_back(pandasm::Create_JGT(src, label));
            break;
        }
        case compiler::CC_GE: {
            res.emplace_back(pandasm::Create_JGE(src, label));
            break;
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            success = false;
    }
}

static void VisitIf64Signed(compiler::IfInst *inst, std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(Is64Bits(inst->GetInputType(0U), Arch::NONE));
    ASSERT(IsTypeSigned(inst->GetInputType(0U)));

    DoLda64(inst->GetSrcReg(0U), res);

    res.emplace_back(pandasm::Create_CMP_64(inst->GetSrcReg(1U)));
    std::string label = CodeGenStatic::LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());

    switch (inst->GetCc()) {
        case compiler::CC_EQ: {
            res.emplace_back(pandasm::Create_JEQZ(label));
            break;
        }
        case compiler::CC_NE: {
            res.emplace_back(pandasm::Create_JNEZ(label));
            break;
        }
        case compiler::CC_LT: {
            res.emplace_back(pandasm::Create_JLTZ(label));
            break;
        }
        case compiler::CC_LE: {
            res.emplace_back(pandasm::Create_JLEZ(label));
            break;
        }
        case compiler::CC_GT: {
            res.emplace_back(pandasm::Create_JGTZ(label));
            break;
        }
        case compiler::CC_GE: {
            res.emplace_back(pandasm::Create_JGEZ(label));
            break;
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            success = false;
    }
}

static void VisitIf64Unsigned(compiler::IfInst *inst, std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(Is64Bits(inst->GetInputType(0U), Arch::NONE));
    ASSERT(!IsTypeSigned(inst->GetInputType(0U)));

    DoLda64(inst->GetSrcReg(0U), res);

    res.emplace_back(pandasm::Create_UCMP_64(inst->GetSrcReg(1U)));
    std::string label = CodeGenStatic::LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());

    switch (inst->GetCc()) {
        case compiler::CC_EQ: {
            res.emplace_back(pandasm::Create_JEQZ(label));
            break;
        }
        case compiler::CC_NE: {
            res.emplace_back(pandasm::Create_JNEZ(label));
            break;
        }
        case compiler::CC_LT: {
            res.emplace_back(pandasm::Create_JLTZ(label));
            break;
        }
        case compiler::CC_LE: {
            res.emplace_back(pandasm::Create_JLEZ(label));
            break;
        }
        case compiler::CC_GT: {
            res.emplace_back(pandasm::Create_JGTZ(label));
            break;
        }
        case compiler::CC_GE: {
            res.emplace_back(pandasm::Create_JGEZ(label));
            break;
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            success = false;
    }
}

static void VisitIfRef([[maybe_unused]] CodeGenStatic *enc, compiler::IfInst *inst, std::vector<pandasm::Ins> &res,
                       bool &success)
{
    ASSERT(IsReference(inst->GetInputType(0U)));

    DoLdaObj(inst->GetSrcReg(0U), res);

    compiler::Register src = inst->GetSrcReg(1);
    std::string label = CodeGenStatic::LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());

    switch (inst->GetCc()) {
        case compiler::CC_EQ: {
            res.emplace_back(pandasm::Create_JEQ_OBJ(src, label));
            break;
        }
        case compiler::CC_NE: {
            res.emplace_back(pandasm::Create_JNE_OBJ(src, label));
            break;
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            success = false;
    }
}

// NOLINTNEXTLINE(readability-function-size)
void CodeGenStatic::VisitIf(GraphVisitor *v, Inst *instBase)
{
    auto enc = static_cast<CodeGenStatic *>(v);
    auto inst = instBase->CastToIf();
    switch (inst->GetInputType(0U)) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32: {
            VisitIf32(inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::INT64: {
            VisitIf64Signed(inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::UINT64: {
            VisitIf64Unsigned(inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::REFERENCE: {
            VisitIfRef(enc, inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::ANY: {
            UNREACHABLE();
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            enc->success_ = false;
    }
}

void CodeGenStatic::VisitIfImm(GraphVisitor *v, Inst *instBase)
{
    auto inst = instBase->CastToIfImm();
    auto imm = inst->GetImm();
    if (imm == 0U) {
        IfImmZero(v, instBase);
        return;
    }
    IfImmNonZero(v, instBase);
}

static void IfImmZero32(compiler::IfImmInst *inst, std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(Is32Bits(inst->GetInputType(0U), Arch::NONE));

    DoLda(inst->GetSrcReg(0U), res);

    std::string label = CodeGenStatic::LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());

    switch (inst->GetCc()) {
        case compiler::CC_EQ: {
            res.emplace_back(pandasm::Create_JEQZ(label));
            break;
        }
        case compiler::CC_NE: {
            res.emplace_back(pandasm::Create_JNEZ(label));
            break;
        }
        case compiler::CC_LT: {
            res.emplace_back(pandasm::Create_JLTZ(label));
            break;
        }
        case compiler::CC_LE: {
            res.emplace_back(pandasm::Create_JLEZ(label));
            break;
        }
        case compiler::CC_GT: {
            res.emplace_back(pandasm::Create_JGTZ(label));
            break;
        }
        case compiler::CC_GE: {
            res.emplace_back(pandasm::Create_JGEZ(label));
            break;
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            success = false;
    }
}

static void IfImmZeroRef(compiler::IfImmInst *inst, std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(IsReference(inst->GetInputType(0U)));

    DoLdaObj(inst->GetSrcReg(0U), res);

    std::string label = CodeGenStatic::LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());

    switch (inst->GetCc()) {
        case compiler::CC_EQ: {
            res.emplace_back(pandasm::Create_JEQZ_OBJ(label));
            break;
        }
        case compiler::CC_NE: {
            res.emplace_back(pandasm::Create_JNEZ_OBJ(label));
            break;
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            success = false;
    }
}

// NOLINTNEXTLINE(readability-function-size)
void CodeGenStatic::IfImmZero(GraphVisitor *v, Inst *instBase)
{
    auto enc = static_cast<CodeGenStatic *>(v);
    auto inst = instBase->CastToIfImm();
    switch (inst->GetInputType(0U)) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32: {
            IfImmZero32(inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64: {
            IfImm64(v, instBase);
            break;
        }
        case compiler::DataType::REFERENCE: {
            IfImmZeroRef(inst, enc->result_, enc->success_);
            break;
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            enc->success_ = false;
    }
}

static void IfImmNonZero32(compiler::IfImmInst *inst, std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(Is32Bits(inst->GetInputType(0U), Arch::NONE));

    res.emplace_back(pandasm::Create_LDAI(inst->GetImm()));

    compiler::Register src = inst->GetSrcReg(0);
    std::string label = CodeGenStatic::LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());

    switch (inst->GetCc()) {
        case compiler::CC_EQ: {
            res.emplace_back(pandasm::Create_JEQ(src, label));
            break;
        }
        case compiler::CC_NE: {
            res.emplace_back(pandasm::Create_JNE(src, label));
            break;
        }
        case compiler::CC_LT: {
            res.emplace_back(pandasm::Create_JLT(src, label));
            break;
        }
        case compiler::CC_LE: {
            res.emplace_back(pandasm::Create_JLE(src, label));
            break;
        }
        case compiler::CC_GT: {
            res.emplace_back(pandasm::Create_JGT(src, label));
            break;
        }
        case compiler::CC_GE: {
            res.emplace_back(pandasm::Create_JGE(src, label));
            break;
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            success = false;
    }
}

// NOLINTNEXTLINE(readability-function-size)
void CodeGenStatic::IfImmNonZero(GraphVisitor *v, Inst *instBase)
{
    auto enc = static_cast<CodeGenStatic *>(v);
    auto inst = instBase->CastToIfImm();
    switch (inst->GetInputType(0U)) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32: {
            IfImmNonZero32(inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64: {
            IfImm64(v, instBase);
            break;
        }
        case compiler::DataType::REFERENCE: {
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            std::cerr << "VisitIfImm does not support non-zero imm of type reference, as no pandasm matches";
            enc->success_ = false;
            break;
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            enc->success_ = false;
    }
}

// NOLINTNEXTLINE(readability-function-size)
void CodeGenStatic::IfImm64(GraphVisitor *v, Inst *instBase)
{
    auto enc = static_cast<CodeGenStatic *>(v);
    auto inst = instBase->CastToIfImm();

    enc->result_.emplace_back(pandasm::Create_LDAI_64(inst->GetImm()));

    std::string label = LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());

    switch (inst->GetInputType(0U)) {
        case compiler::DataType::INT64: {
            enc->result_.emplace_back(pandasm::Create_CMP_64(inst->GetSrcReg(0U)));
            break;
        }
        case compiler::DataType::UINT64: {
            enc->result_.emplace_back(pandasm::Create_UCMP_64(inst->GetSrcReg(0U)));
            break;
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            enc->success_ = false;
            return;
    }

    switch (inst->GetCc()) {
        case compiler::CC_EQ: {
            enc->result_.emplace_back(pandasm::Create_JEQZ(label));
            break;
        }
        case compiler::CC_NE: {
            enc->result_.emplace_back(pandasm::Create_JNEZ(label));
            break;
        }
        case compiler::CC_LT: {
            enc->result_.emplace_back(pandasm::Create_JGTZ(label));
            break;
        }
        case compiler::CC_LE: {
            enc->result_.emplace_back(pandasm::Create_JGEZ(label));
            break;
        }
        case compiler::CC_GT: {
            enc->result_.emplace_back(pandasm::Create_JLTZ(label));
            break;
        }
        case compiler::CC_GE: {
            enc->result_.emplace_back(pandasm::Create_JLEZ(label));
            break;
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            enc->success_ = false;
            return;
    }
}

static void CastAccDownFromI32(compiler::DataType::Type type, std::vector<pandasm::Ins> &res)
{
    auto sizeBits = compiler::DataType::GetTypeSize(type, Arch::NONE);
    if (compiler::DataType::IsTypeSigned(type)) {
        auto shift = compiler::WORD_SIZE - sizeBits;
        res.emplace_back(pandasm::Create_SHLI(shift));
        res.emplace_back(pandasm::Create_ASHRI(shift));
    } else {
        auto andMask = (1U << sizeBits) - 1U;
        res.emplace_back(pandasm::Create_ANDI(andMask));
    }
}

static void VisitCastFromI32([[maybe_unused]] CodeGenStatic *enc, compiler::CastInst *inst,
                             std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(Is32Bits(inst->GetInputType(0U), Arch::NONE));

    if (inst->GetType() == compiler::DataType::UINT32) {
        return;
    }
    DoLda(inst->GetSrcReg(0U), res);

    switch (inst->GetType()) {
        case compiler::DataType::FLOAT32: {
            res.emplace_back(pandasm::Create_I32TOF32());
            break;
        }
        case compiler::DataType::FLOAT64: {
            res.emplace_back(pandasm::Create_I32TOF64());
            break;
        }
        case compiler::DataType::INT64: {
            res.emplace_back(pandasm::Create_I32TOI64());
            break;
        }
        case compiler::DataType::INT16:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT8: {
            CastAccDownFromI32(inst->GetType(), res);
            break;
        }
        case compiler::DataType::ANY: {
            UNREACHABLE();
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            success = false;
            return;
    }
    DoSta(inst->GetType(), inst->GetDstReg(), res);
}

static void VisitCastFromU32([[maybe_unused]] CodeGenStatic *enc, compiler::CastInst *inst,
                             std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(Is32Bits(inst->GetInputType(0U), Arch::NONE));

    if (inst->GetType() == compiler::DataType::INT32) {
        return;
    }
    DoLda(inst->GetSrcReg(0U), res);

    switch (inst->GetType()) {
        case compiler::DataType::FLOAT32: {
            res.emplace_back(pandasm::Create_U32TOF32());
            break;
        }
        case compiler::DataType::FLOAT64: {
            res.emplace_back(pandasm::Create_U32TOF64());
            break;
        }
        case compiler::DataType::INT64: {
            res.emplace_back(pandasm::Create_U32TOI64());
            break;
        }
        case compiler::DataType::INT16:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT8: {
            CastAccDownFromI32(inst->GetType(), res);
            break;
        }
        case compiler::DataType::ANY: {
            UNREACHABLE();
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            success = false;
            return;
    }
    DoSta(inst->GetType(), inst->GetDstReg(), res);
}

static void VisitCastFromI64([[maybe_unused]] CodeGenStatic *enc, compiler::CastInst *inst,
                             std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(Is64Bits(inst->GetInputType(0U), Arch::NONE));
    constexpr int64_t ANDI_32 = 0xffffffff;

    DoLda64(inst->GetSrcReg(0U), res);

    switch (inst->GetType()) {
        case compiler::DataType::INT32: {
            res.emplace_back(pandasm::Create_I64TOI32());
            break;
        }
        case compiler::DataType::UINT32: {
            res.emplace_back(pandasm::Create_ANDI(ANDI_32));
            break;
        }
        case compiler::DataType::FLOAT32: {
            res.emplace_back(pandasm::Create_I64TOF32());
            break;
        }
        case compiler::DataType::FLOAT64: {
            res.emplace_back(pandasm::Create_I64TOF64());
            break;
        }
        case compiler::DataType::INT16:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT8: {
            res.emplace_back(pandasm::Create_I64TOI32());
            CastAccDownFromI32(inst->GetType(), res);
            break;
        }
        case compiler::DataType::ANY: {
            UNREACHABLE();
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            success = false;
            return;
    }
    DoSta(inst->GetType(), inst->GetDstReg(), res);
}

static void VisitCastFromF32([[maybe_unused]] CodeGenStatic *enc, compiler::CastInst *inst,
                             std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(Is32Bits(inst->GetInputType(0U), Arch::NONE));
    ASSERT(IsFloatType(inst->GetInputType(0U)));

    constexpr int64_t ANDI_32 = 0xffffffff;

    DoLda(inst->GetSrcReg(0U), res);

    switch (inst->GetType()) {
        case compiler::DataType::FLOAT64: {
            res.emplace_back(pandasm::Create_F32TOF64());
            break;
        }
        case compiler::DataType::INT64: {
            res.emplace_back(pandasm::Create_F32TOI64());
            break;
        }
        case compiler::DataType::UINT64: {
            res.emplace_back(pandasm::Create_F32TOU64());
            break;
        }
        case compiler::DataType::INT32: {
            res.emplace_back(pandasm::Create_F32TOI32());
            break;
        }
        case compiler::DataType::UINT32: {
            res.emplace_back(pandasm::Create_F32TOU32());
            res.emplace_back(pandasm::Create_ANDI(ANDI_32));
            break;
        }
        case compiler::DataType::ANY: {
            UNREACHABLE();
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            success = false;
            return;
    }
    DoSta(inst->GetType(), inst->GetDstReg(), res);
}

static void VisitCastFromF64([[maybe_unused]] CodeGenStatic *enc, compiler::CastInst *inst,
                             std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(Is64Bits(inst->GetInputType(0U), Arch::NONE));
    ASSERT(IsFloatType(inst->GetInputType(0U)));

    constexpr int64_t ANDI_32 = 0xffffffff;

    DoLda64(inst->GetSrcReg(0U), res);

    switch (inst->GetType()) {
        case compiler::DataType::FLOAT32: {
            res.emplace_back(pandasm::Create_F64TOF32());
            break;
        }
        case compiler::DataType::INT64: {
            res.emplace_back(pandasm::Create_F64TOI64());
            break;
        }
        case compiler::DataType::INT32: {
            res.emplace_back(pandasm::Create_F64TOI32());
            break;
        }
        case compiler::DataType::UINT32: {
            res.emplace_back(pandasm::Create_F64TOI64());
            res.emplace_back(pandasm::Create_ANDI(ANDI_32));
            break;
        }
        case compiler::DataType::ANY: {
            UNREACHABLE();
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            success = false;
            return;
    }
    DoSta(inst->GetType(), inst->GetDstReg(), res);
}

// NOLINTNEXTLINE(readability-function-size)
void CodeGenStatic::VisitCast(GraphVisitor *v, Inst *instBase)
{
    auto enc = static_cast<CodeGenStatic *>(v);
    auto inst = instBase->CastToCast();
    switch (inst->GetInputType(0U)) {
        case compiler::DataType::INT32: {
            VisitCastFromI32(enc, inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::UINT32: {
            VisitCastFromU32(enc, inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::INT64: {
            VisitCastFromI64(enc, inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::FLOAT32: {
            VisitCastFromF32(enc, inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::FLOAT64: {
            VisitCastFromF64(enc, inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::REFERENCE: {
            switch (inst->GetType()) {
                case compiler::DataType::ANY: {
                    UNREACHABLE();
                }
                default:
                    std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
                    enc->success_ = false;
            }
            break;
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            enc->success_ = false;
    }
}

void CodeGenStatic::VisitReturn(GraphVisitor *v, Inst *instBase)
{
    pandasm::Ins ins;
    auto enc = static_cast<CodeGenStatic *>(v);
    auto inst = instBase->CastToReturn();
    switch (inst->GetType()) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32:
        case compiler::DataType::FLOAT32: {
            DoLda(inst->GetSrcReg(0U), enc->result_);
            enc->result_.emplace_back(pandasm::Create_RETURN());
            break;
        }
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64:
        case compiler::DataType::FLOAT64: {
            DoLda64(inst->GetSrcReg(0U), enc->result_);
            enc->result_.emplace_back(pandasm::Create_RETURN_64());
            break;
        }
        case compiler::DataType::REFERENCE: {
            DoLdaObj(inst->GetSrcReg(0U), enc->result_);
            enc->result_.emplace_back(pandasm::Create_RETURN_OBJ());
            break;
        }
        case compiler::DataType::VOID: {
            enc->result_.emplace_back(pandasm::Create_RETURN_VOID());
            break;
        }
        case compiler::DataType::ANY: {
            UNREACHABLE();
            break;
        }
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            enc->success_ = false;
    }
}

// NOLINTNEXTLINE(readability-function-size)
void CodeGenStatic::VisitStoreObjectIntrinsic(GraphVisitor *v, Inst *instBase)
{
    auto enc = static_cast<CodeGenStatic *>(v);
    const compiler::IntrinsicInst *inst = instBase->CastToIntrinsic();

    compiler::Register vd = inst->GetSrcReg(1U);
    compiler::Register vs = inst->GetSrcReg(0U);
    std::string id = enc->irInterface_->GetFieldIdByOffset(inst->GetImm(0));

    bool isAccType = (vs == compiler::GetAccReg());

    switch (inst->GetType()) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32:
        case compiler::DataType::FLOAT32:
            if (isAccType) {
                enc->result_.emplace_back(pandasm::Create_STOBJ(vd, id));
            } else {
                enc->result_.emplace_back(pandasm::Create_STOBJ_V(vs, vd, id));
            }
            break;
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64:
        case compiler::DataType::FLOAT64:
            if (isAccType) {
                enc->result_.emplace_back(pandasm::Create_STOBJ_64(vd, id));
            } else {
                enc->result_.emplace_back(pandasm::Create_STOBJ_V_64(vs, vd, id));
            }
            break;
        case compiler::DataType::REFERENCE:
            if (isAccType) {
                enc->result_.emplace_back(pandasm::Create_STOBJ_OBJ(vd, id));
            } else {
                enc->result_.emplace_back(pandasm::Create_STOBJ_V_OBJ(vs, vd, id));
            }
            break;
        default:
            std::cerr << "Wrong DataType for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            enc->success_ = false;
    }
}

void CodeGenStatic::VisitStoreStaticIntrinsic(GraphVisitor *v, Inst *instBase)
{
    auto enc = static_cast<CodeGenStatic *>(v);
    auto inst = instBase->CastToIntrinsic();

    compiler::Register vs = inst->GetSrcReg(0U);
    std::string id = enc->irInterface_->GetFieldIdByOffset(inst->GetImm(0));

    switch (inst->GetType()) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32:
        case compiler::DataType::FLOAT32:
            DoLda(vs, enc->result_);
            enc->result_.emplace_back(pandasm::Create_STSTATIC(id));
            break;
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64:
        case compiler::DataType::FLOAT64:
            DoLda64(vs, enc->result_);
            enc->result_.emplace_back(pandasm::Create_STSTATIC_64(id));
            break;
        case compiler::DataType::REFERENCE:
            DoLdaObj(vs, enc->result_);
            enc->result_.emplace_back(pandasm::Create_STSTATIC_OBJ(id));
            break;
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            enc->success_ = false;
    }
}

void CodeGenStatic::VisitLoadObjectIntrinsic(GraphVisitor *v, Inst *instBase)
{
    auto enc = static_cast<CodeGenStatic *>(v);
    auto inst = instBase->CastToIntrinsic();

    compiler::Register vs = inst->GetSrcReg(0U);
    compiler::Register vd = inst->GetDstReg();
    std::string id = enc->irInterface_->GetFieldIdByOffset(inst->GetImm(0));

    bool isAccType = (inst->GetDstReg() == compiler::GetAccReg());

    switch (inst->GetType()) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32:
        case compiler::DataType::FLOAT32:
            if (isAccType) {
                enc->result_.emplace_back(pandasm::Create_LDOBJ(vs, id));
            } else {
                enc->result_.emplace_back(pandasm::Create_LDOBJ_V(vd, vs, id));
            }
            break;
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64:
        case compiler::DataType::FLOAT64:
            if (isAccType) {
                enc->result_.emplace_back(pandasm::Create_LDOBJ_64(vs, id));
            } else {
                enc->result_.emplace_back(pandasm::Create_LDOBJ_V_64(vd, vs, id));
            }
            break;
        case compiler::DataType::REFERENCE:
            if (isAccType) {
                enc->result_.emplace_back(pandasm::Create_LDOBJ_OBJ(vs, id));
            } else {
                enc->result_.emplace_back(pandasm::Create_LDOBJ_V_OBJ(vd, vs, id));
            }
            break;
        default:
            std::cerr << "Wrong DataType for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            enc->success_ = false;
    }
}

void CodeGenStatic::VisitLoadStaticIntrinsic(GraphVisitor *v, Inst *instBase)
{
    auto enc = static_cast<CodeGenStatic *>(v);
    auto inst = instBase->CastToIntrinsic();

    compiler::Register vd = inst->GetDstReg();
    std::string id = enc->irInterface_->GetFieldIdByOffset(inst->GetImm(0));

    switch (inst->GetType()) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32:
        case compiler::DataType::FLOAT32:
            enc->result_.emplace_back(pandasm::Create_LDSTATIC(id));
            DoSta(vd, enc->result_);
            break;
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64:
        case compiler::DataType::FLOAT64:
            enc->result_.emplace_back(pandasm::Create_LDSTATIC_64(id));
            DoSta64(vd, enc->result_);
            break;
        case compiler::DataType::REFERENCE:
            enc->result_.emplace_back(pandasm::Create_LDSTATIC_OBJ(id));
            DoStaObj(vd, enc->result_);
            break;
        default:
            std::cerr << "CodeGen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed\n";
            enc->success_ = false;
    }
}

void CodeGenStatic::VisitLoadTypeIntrinsic(GraphVisitor *v, Inst *instBase)
{
    auto enc = static_cast<CodeGenStatic *>(v);
    auto inst = instBase->CastToIntrinsic();

    compiler::Register vd = inst->GetDstReg();
    std::string id = enc->irInterface_->GetTypeIdByOffset(inst->GetImm(0));

    enc->result_.emplace_back(pandasm::Create_LDA_TYPE(id));
    DoStaObj(vd, enc->result_);
}

void CodeGenStatic::VisitCatchPhi(GraphVisitor *v, Inst *inst)
{
    if (inst->CastToCatchPhi()->IsAcc()) {
        bool hasRealUsers = false;
        for (auto &user : inst->GetUsers()) {
            if (!user.GetInst()->IsSaveState()) {
                hasRealUsers = true;
                break;
            }
        }
        if (hasRealUsers) {
            auto enc = static_cast<CodeGenStatic *>(v);
            enc->result_.emplace_back(pandasm::Create_STA_OBJ(inst->GetDstReg()));
        }
    }
}

#include "generated/codegen_intrinsics_static.cpp"
#include "generated/insn_selection_static.cpp"
}  // namespace libabckit
