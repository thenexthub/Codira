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
#include "codegen.h"
#include "runtime/include/coretypes/tagged_value.h"

namespace ark::bytecodeopt {

void DoLdaObj(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    if (reg != compiler::GetAccReg()) {
        result.emplace_back(pandasm::Create_LDA_OBJ(reg));
    }
}

void DoLda(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    if (reg != compiler::GetAccReg()) {
        result.emplace_back(pandasm::Create_LDA(reg));
    }
}

void DoLda64(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    if (reg != compiler::GetAccReg()) {
        result.emplace_back(pandasm::Create_LDA_64(reg));
    }
}

void DoStaObj(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    if (reg != compiler::GetAccReg()) {
        result.emplace_back(pandasm::Create_STA_OBJ(reg));
    }
}

void DoSta(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    if (reg != compiler::GetAccReg()) {
        result.emplace_back(pandasm::Create_STA(reg));
    }
}

void DoSta64(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    if (reg != compiler::GetAccReg()) {
        result.emplace_back(pandasm::Create_STA_64(reg));
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
    if (reg != compiler::GetAccReg()) {
        result.emplace_back(pandasm::Create_LDA_DYN(reg));
    }
}

void DoStaDyn(compiler::Register reg, std::vector<pandasm::Ins> &result)
{
    if (reg != compiler::GetAccReg()) {
        result.emplace_back(pandasm::Create_STA_DYN(reg));
    }
}

void BytecodeGen::AppendCatchBlock(uint32_t typeId, const compiler::BasicBlock *tryBegin,
                                   const compiler::BasicBlock *tryEnd, const compiler::BasicBlock *catchBegin,
                                   const compiler::BasicBlock *catchEnd)
{
    auto cb = pandasm::Function::CatchBlock();
    if (typeId != 0U) {
        cb.exceptionRecord = irInterface_->GetTypeIdByOffset(typeId);
    }
    cb.tryBeginLabel = BytecodeGen::LabelName(tryBegin->GetId());
    cb.tryEndLabel = "end_" + BytecodeGen::LabelName(tryEnd->GetId());
    cb.catchBeginLabel = BytecodeGen::LabelName(catchBegin->GetId());
    cb.catchEndLabel = catchEnd == nullptr ? cb.catchBeginLabel : "end_" + BytecodeGen::LabelName(catchEnd->GetId());
    catchBlocks_.emplace_back(std::move(cb));
}

void BytecodeGen::VisitTryBegin(const compiler::BasicBlock *bb)
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

void BytecodeGen::AddLineAndColumnNumber(const compiler::Inst *inst, size_t idx)
{
    AddLineNumber(inst, idx);
    if (GetGraph()->IsDynamicMethod()) {
        AddColumnNumber(inst, idx);
    }
}

bool BytecodeGen::RunImpl()
{
    Reserve(function_->ins.size());
    for (auto *bb : GetGraph()->GetBlocksLinearOrder()) {
        EmitLabel(BytecodeGen::LabelName(bb->GetId()));
        if (bb->IsTryEnd()) {
            auto label = "end_" + BytecodeGen::LabelName(bb->GetId());
            EmitLabel(std::move(label));
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
    function_->catchBlocks = std::move(catchBlocks_);
    return true;
}

void BytecodeGen::EmitJump(const BasicBlock *bb)
{
    BasicBlock *sucBb = nullptr;
    ASSERT(bb != nullptr);

    if (bb->GetLastInst() == nullptr) {
        ASSERT(bb->IsEmpty());
        sucBb = bb->GetSuccsBlocks()[0U];
        result_.push_back(pandasm::Create_JMP(BytecodeGen::LabelName(sucBb->GetId())));
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
    result_.push_back(pandasm::Create_JMP(BytecodeGen::LabelName(sucBb->GetId())));
}

void BytecodeGen::AddLineNumber(const Inst *inst, const size_t idx)
{
    if (irInterface_ != nullptr && idx < result_.size()) {
        auto ln = irInterface_->GetLineNumberByPc(inst->GetPc());
        result_[idx].insDebug.SetLineNumber(ln);
    }
}

void BytecodeGen::AddColumnNumber(const Inst *inst, const uint32_t idx)
{
    if (irInterface_ != nullptr && idx < result_.size()) {
        auto cn = irInterface_->GetColumnNumberByPc(inst->GetPc());
        result_[idx].insDebug.SetColumnNumber(cn);
    }
}

void BytecodeGen::EncodeSpillFillData(const compiler::SpillFillData &sf)
{
    if (sf.SrcType() != compiler::LocationType::REGISTER || sf.DstType() != compiler::LocationType::REGISTER) {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "EncodeSpillFillData with unknown move type, src_type: "
                                       << static_cast<int>(sf.SrcType())
                                       << " dst_type: " << static_cast<int>(sf.DstType());
        success_ = false;
        UNREACHABLE();
        return;
    }
    ASSERT(sf.GetType() != compiler::DataType::NO_TYPE);
    ASSERT(sf.SrcValue() != compiler::GetInvalidReg() && sf.DstValue() != compiler::GetInvalidReg());

    if (sf.SrcValue() == sf.DstValue()) {
        return;
    }

    pandasm::Ins move {};
    if (GetGraph()->IsDynamicMethod()) {
        result_.emplace_back(pandasm::Create_MOV_DYN(sf.DstValue(), sf.SrcValue()));
        return;
    }
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

void BytecodeGen::VisitSpillFill(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<BytecodeGen *>(visitor);
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

static void VisitConstant32(BytecodeGen *enc, compiler::Inst *inst, std::vector<pandasm::Ins> &res)
{
    auto type = inst->GetType();
    ASSERT(compiler::DataType::Is32Bits(type, Arch::NONE));

    pandasm::Ins movi {};
    auto dstReg = inst->GetDstReg();
    movi.EmplaceReg(inst->GetDstReg());

    switch (type) {
        case compiler::DataType::BOOL:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT32:
        case compiler::DataType::UINT32:
            if (enc->GetGraph()->IsDynamicMethod()) {
                res.emplace_back(pandasm::Create_LDAI_DYN(inst->CastToConstant()->GetInt32Value()));
                DoStaDyn(inst->GetDstReg(), res);
            } else {
                if (dstReg == compiler::GetAccReg()) {
                    pandasm::Ins ldai = pandasm::Create_LDAI(inst->CastToConstant()->GetInt32Value());
                    res.emplace_back(std::move(ldai));
                } else {
                    movi = pandasm::Create_MOVI(dstReg, inst->CastToConstant()->GetInt32Value());
                    res.emplace_back(std::move(movi));
                }
            }
            break;
        case compiler::DataType::FLOAT32:
            if (enc->GetGraph()->IsDynamicMethod()) {
                res.emplace_back(pandasm::Create_FLDAI_DYN(inst->CastToConstant()->GetFloatValue()));
                DoStaDyn(inst->GetDstReg(), res);
            } else {
                if (dstReg == compiler::GetAccReg()) {
                    pandasm::Ins ldai = pandasm::Create_FLDAI(inst->CastToConstant()->GetFloatValue());
                    res.emplace_back(std::move(ldai));
                } else {
                    movi = pandasm::Create_FMOVI(dstReg, inst->CastToConstant()->GetFloatValue());
                    res.emplace_back(std::move(movi));
                }
            }
            break;
        default:
            UNREACHABLE();
    }
}

static void VisitConstant64(BytecodeGen *enc, compiler::Inst *inst, std::vector<pandasm::Ins> &res)
{
    auto type = inst->GetType();
    ASSERT(compiler::DataType::Is64Bits(type, Arch::NONE));

    pandasm::Ins movi {};
    auto dstReg = inst->GetDstReg();
    movi.EmplaceReg(inst->GetDstReg());

    switch (type) {
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64:
            if (enc->GetGraph()->IsDynamicMethod()) {
                res.emplace_back(pandasm::Create_LDAI_DYN(inst->CastToConstant()->GetInt64Value()));
                DoStaDyn(inst->GetDstReg(), res);
            } else {
                if (dstReg == compiler::GetAccReg()) {
                    pandasm::Ins ldai = pandasm::Create_LDAI_64(inst->CastToConstant()->GetInt64Value());
                    res.emplace_back(std::move(ldai));
                } else {
                    movi = pandasm::Create_MOVI_64(dstReg, inst->CastToConstant()->GetInt64Value());
                    res.emplace_back(std::move(movi));
                }
            }
            break;
        case compiler::DataType::FLOAT64:
            if (enc->GetGraph()->IsDynamicMethod()) {
                res.emplace_back(pandasm::Create_FLDAI_DYN(inst->CastToConstant()->GetDoubleValue()));
                DoStaDyn(inst->GetDstReg(), res);
            } else {
                if (dstReg == compiler::GetAccReg()) {
                    pandasm::Ins ldai = pandasm::Create_FLDAI_64(inst->CastToConstant()->GetDoubleValue());
                    res.emplace_back(std::move(ldai));
                } else {
                    movi = pandasm::Create_FMOVI_64(dstReg, inst->CastToConstant()->GetDoubleValue());
                    res.emplace_back(std::move(movi));
                }
            }
            break;
        default:
            UNREACHABLE();
    }
}

void BytecodeGen::VisitConstant(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<BytecodeGen *>(visitor);
    auto type = inst->GetType();

    /* Do not emit unused code for Const -> CastValueToAnyType chains */
    if (enc->GetGraph()->IsDynamicMethod()) {
        if (!HasUserPredicate(inst,
                              [](Inst const *i) { return i->GetOpcode() != compiler::Opcode::CastValueToAnyType; })) {
            return;
        }
    }

    switch (type) {
        case compiler::DataType::BOOL:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT32:
        case compiler::DataType::UINT32:
        case compiler::DataType::FLOAT32:
            VisitConstant32(enc, inst, enc->result_);
            break;
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64:
        case compiler::DataType::FLOAT64:
            VisitConstant64(enc, inst, enc->result_);
            break;
        case compiler::DataType::ANY: {
            UNREACHABLE();
        }
        default:
            UNREACHABLE();
            LOG(ERROR, BYTECODE_OPTIMIZER) << "VisitConstant with unknown type" << type;
            enc->success_ = false;
    }
}

void BytecodeGen::EncodeSta(compiler::Register reg, compiler::DataType::Type type)
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
            LOG(ERROR, BYTECODE_OPTIMIZER) << "EncodeSta with unknown type" << type;
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
           op == compiler::Opcode::InitObject);
    if (nargs > MAX_NUM_NON_RANGE_ARGS) {
        switch (op) {
            case compiler::Opcode::CallStatic:
                return pandasm::Opcode::CALL_RANGE;
            case compiler::Opcode::CallVirtual:
                return pandasm::Opcode::CALL_VIRT_RANGE;
            case compiler::Opcode::InitObject:
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
            case compiler::Opcode::InitObject:
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
        case compiler::Opcode::InitObject:
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
        case compiler::Opcode::InitObject:
            return inst->CastToInitObject();
        default:
            UNREACHABLE();
    }
}

void BytecodeGen::CallHandler(GraphVisitor *visitor, Inst *inst, std::string methodId, bool isDevirtual)
{
    auto op = inst->GetOpcode();
    ASSERT(op == compiler::Opcode::CallStatic || op == compiler::Opcode::CallVirtual ||
           op == compiler::Opcode::InitObject || op == compiler::Opcode::Intrinsic);
    if (op == compiler::Opcode::Intrinsic) {
        ASSERT(inst->IsCallOrIntrinsic());
        op = compiler::Opcode::CallStatic;
    }
    auto *enc = static_cast<BytecodeGen *>(visitor);
    auto sfCount = inst->GetInputsCount() - (inst->RequireState() ? 1U : 0U);
    size_t start = op == compiler::Opcode::InitObject ? 1U : 0U;  // exclude LoadAndInitClass
    auto nargs = sfCount - start;                                 // exclude LoadAndInitClass
    pandasm::Ins ins {};
    if (isDevirtual) {
        ins.SetIsDevirtual();
    }

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
                ins.EmplaceImm(static_cast<int64_t>(i));
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

void BytecodeGen::CallHandler(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<BytecodeGen *>(visitor);
    auto methodOffset = CastToCall(inst)->GetCallMethodId();
    bool isDevirtual = !enc->irInterface_->IsMethodStatic(methodOffset);
    CallHandler(visitor, inst, enc->irInterface_->GetMethodIdByOffset(methodOffset), isDevirtual);
}

void BytecodeGen::VisitCallStatic(GraphVisitor *visitor, Inst *inst)
{
    CallHandler(visitor, inst);
}

void BytecodeGen::VisitCallVirtual(GraphVisitor *visitor, Inst *inst)
{
    CallHandler(visitor, inst);
}

void BytecodeGen::VisitInitObject(GraphVisitor *visitor, Inst *inst)
{
    CallHandler(visitor, inst);
}

static void VisitIf32(BytecodeGen *enc, compiler::IfInst *inst, std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(Is32Bits(inst->GetInputType(0U), Arch::NONE));

    if (enc->GetGraph()->IsDynamicMethod()) {
        DoLdaDyn(inst->GetSrcReg(0U), res);
    } else {
        DoLda(inst->GetSrcReg(0U), res);
    }

    compiler::Register src = inst->GetSrcReg(1);
    std::string label = BytecodeGen::LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());

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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            success = false;
    }
}

static void VisitIf64Signed(BytecodeGen *enc, compiler::IfInst *inst, std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(Is64Bits(inst->GetInputType(0U), Arch::NONE));
    ASSERT(IsTypeSigned(inst->GetInputType(0U)));

    if (enc->GetGraph()->IsDynamicMethod()) {
        DoLdaDyn(inst->GetSrcReg(0U), res);
    } else {
        DoLda64(inst->GetSrcReg(0U), res);
    }

    res.emplace_back(pandasm::Create_CMP_64(inst->GetSrcReg(1U)));
    std::string label = BytecodeGen::LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());

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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            success = false;
    }
}

static void VisitIf64Unsigned(BytecodeGen *enc, compiler::IfInst *inst, std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(Is64Bits(inst->GetInputType(0U), Arch::NONE));
    ASSERT(!IsTypeSigned(inst->GetInputType(0U)));

    if (enc->GetGraph()->IsDynamicMethod()) {
        DoLdaDyn(inst->GetSrcReg(0U), res);
    } else {
        DoLda64(inst->GetSrcReg(0U), res);
    }

    res.emplace_back(pandasm::Create_UCMP_64(inst->GetSrcReg(1U)));
    std::string label = BytecodeGen::LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());

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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            success = false;
    }
}

static void VisitIfRef([[maybe_unused]] BytecodeGen *enc, compiler::IfInst *inst, std::vector<pandasm::Ins> &res,
                       bool &success)
{
    ASSERT(IsReference(inst->GetInputType(0U)));

    DoLdaObj(inst->GetSrcReg(0U), res);

    compiler::Register src = inst->GetSrcReg(1);
    std::string label = BytecodeGen::LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());

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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            success = false;
    }
}

// NOLINTNEXTLINE(readability-function-size)
void BytecodeGen::VisitIf(GraphVisitor *v, Inst *instBase)
{
    auto enc = static_cast<BytecodeGen *>(v);
    auto inst = instBase->CastToIf();
    switch (inst->GetInputType(0U)) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32: {
            VisitIf32(enc, inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::INT64: {
            VisitIf64Signed(enc, inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::UINT64: {
            VisitIf64Unsigned(enc, inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::REFERENCE: {
            VisitIfRef(enc, inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::ANY: {
            if (enc->GetGraph()->IsDynamicMethod()) {
#if defined(ENABLE_BYTECODE_OPT) && defined(PANDA_WITH_ECMASCRIPT) && defined(ARK_INTRINSIC_SET)
                IfEcma(v, inst);
                break;
#endif
            }
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            enc->success_ = false;
            break;
        }
        default:
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            enc->success_ = false;
    }
}

#if defined(ENABLE_BYTECODE_OPT) && defined(PANDA_WITH_ECMASCRIPT)
static std::optional<coretypes::TaggedValue> IsEcmaConstTemplate(Inst const *inst)
{
    if (inst->GetOpcode() != compiler::Opcode::CastValueToAnyType) {
        return {};
    }
    auto cvatInst = inst->CastToCastValueToAnyType();
    if (!cvatInst->GetInput(0U).GetInst()->IsConst()) {
        return {};
    }
    auto constInst = cvatInst->GetInput(0U).GetInst()->CastToConstant();

    switch (cvatInst->GetAnyType()) {
        case compiler::AnyBaseType::ECMASCRIPT_UNDEFINED_TYPE:
            return coretypes::TaggedValue(coretypes::TaggedValue::VALUE_UNDEFINED);
        case compiler::AnyBaseType::ECMASCRIPT_INT_TYPE:
            return coretypes::TaggedValue(static_cast<int32_t>(constInst->GetIntValue()));
        case compiler::AnyBaseType::ECMASCRIPT_DOUBLE_TYPE:
            return coretypes::TaggedValue(constInst->GetDoubleValue());
        case compiler::AnyBaseType::ECMASCRIPT_BOOLEAN_TYPE:
            return coretypes::TaggedValue(static_cast<bool>(constInst->GetInt64Value() != 0U));
        case compiler::AnyBaseType::ECMASCRIPT_NULL_TYPE:
            return coretypes::TaggedValue(coretypes::TaggedValue::VALUE_NULL);
        default:
            return {};
    }
}

#if defined(ARK_INTRINSIC_SET)
void BytecodeGen::IfEcma(GraphVisitor *v, compiler::IfInst *inst)
{
    auto enc = static_cast<BytecodeGen *>(v);

    compiler::Register reg = compiler::INVALID_REG_ID;
    coretypes::TaggedValue cmpVal;

    auto testLhs = IsEcmaConstTemplate(inst->GetInput(0U).GetInst());
    auto testRhs = IsEcmaConstTemplate(inst->GetInput(1U).GetInst());

    if (testLhs.has_value() && testLhs->IsBoolean()) {
        cmpVal = testLhs.value();
        reg = inst->GetSrcReg(1U);
    } else if (testRhs.has_value() && testRhs->IsBoolean()) {
        cmpVal = testRhs.value();
        reg = inst->GetSrcReg(0U);
    } else {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
        enc->success_ = false;
        return;
    }

    DoLdaDyn(reg, enc->result_);
    switch (inst->GetCc()) {
        case compiler::CC_EQ: {
            if (cmpVal.IsTrue()) {
                enc->result_.emplace_back(
                    pandasm::Create_ECMA_JTRUE(LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId())));
            } else {
                enc->result_.emplace_back(
                    pandasm::Create_ECMA_JFALSE(LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId())));
            }
            break;
        }
        case compiler::CC_NE: {
            if (cmpVal.IsTrue()) {
                enc->result_.emplace_back(pandasm::Create_ECMA_ISTRUE());
            } else {
                enc->result_.emplace_back(pandasm::Create_ECMA_ISFALSE());
            }
            enc->result_.emplace_back(
                pandasm::Create_ECMA_JFALSE(LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId())));
            break;
        }
        default:
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            enc->success_ = false;
            return;
    }
}
#endif
#endif

void BytecodeGen::VisitIfImm(GraphVisitor *v, Inst *instBase)
{
    auto inst = instBase->CastToIfImm();
    auto imm = inst->GetImm();
    if (imm == 0U) {
        IfImmZero(v, instBase);
        return;
    }
    IfImmNonZero(v, instBase);
}

static void IfImmZero32(BytecodeGen *enc, compiler::IfImmInst *inst, std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(Is32Bits(inst->GetInputType(0U), Arch::NONE));

    if (enc->GetGraph()->IsDynamicMethod()) {
        DoLdaDyn(inst->GetSrcReg(0U), res);
    } else {
        DoLda(inst->GetSrcReg(0U), res);
    }

    std::string label = BytecodeGen::LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());

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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            success = false;
    }
}

static void IfImmZeroRef(BytecodeGen *enc, compiler::IfImmInst *inst, std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(IsReference(inst->GetInputType(0U)));

    if (enc->GetGraph()->IsDynamicMethod()) {
        DoLdaDyn(inst->GetSrcReg(0U), res);
    } else {
        DoLdaObj(inst->GetSrcReg(0U), res);
    }

    std::string label = BytecodeGen::LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());

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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            success = false;
    }
}

// NOLINTNEXTLINE(readability-function-size)
void BytecodeGen::IfImmZero(GraphVisitor *v, Inst *instBase)
{
    auto enc = static_cast<BytecodeGen *>(v);
    auto inst = instBase->CastToIfImm();
    switch (inst->GetInputType(0U)) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32: {
            IfImmZero32(enc, inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64: {
            IfImm64(v, instBase);
            break;
        }
        case compiler::DataType::REFERENCE: {
            IfImmZeroRef(enc, inst, enc->result_, enc->success_);
            break;
        }
        default:
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            enc->success_ = false;
    }
}

static void IfImmNonZero32(BytecodeGen *enc, compiler::IfImmInst *inst, std::vector<pandasm::Ins> &res, bool &success)
{
    ASSERT(Is32Bits(inst->GetInputType(0U), Arch::NONE));

    if (enc->GetGraph()->IsDynamicMethod()) {
        res.emplace_back(pandasm::Create_LDAI_DYN(inst->GetImm()));
    } else {
        res.emplace_back(pandasm::Create_LDAI(inst->GetImm()));
    }

    compiler::Register src = inst->GetSrcReg(0);
    std::string label = BytecodeGen::LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());

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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            success = false;
    }
}

// NOLINTNEXTLINE(readability-function-size)
void BytecodeGen::IfImmNonZero(GraphVisitor *v, Inst *instBase)
{
    auto enc = static_cast<BytecodeGen *>(v);
    auto inst = instBase->CastToIfImm();
    switch (inst->GetInputType(0U)) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32: {
            IfImmNonZero32(enc, inst, enc->result_, enc->success_);
            break;
        }
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64: {
            IfImm64(v, instBase);
            break;
        }
        case compiler::DataType::REFERENCE: {
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "VisitIfImm does not support non-zero imm of type reference, as no pandasm matches";
            enc->success_ = false;
            break;
        }
        default:
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            enc->success_ = false;
    }
}

// NOLINTNEXTLINE(readability-function-size)
void BytecodeGen::IfImm64(GraphVisitor *v, Inst *instBase)
{
    auto enc = static_cast<BytecodeGen *>(v);
    auto inst = instBase->CastToIfImm();

    if (enc->GetGraph()->IsDynamicMethod()) {
        enc->result_.emplace_back(pandasm::Create_LDAI_DYN(inst->GetImm()));
    } else {
        enc->result_.emplace_back(pandasm::Create_LDAI_64(inst->GetImm()));
    }

    std::string label = LabelName(inst->GetBasicBlock()->GetTrueSuccessor()->GetId());

    if (inst->GetInputType(0) == compiler::DataType::INT64) {
        enc->result_.emplace_back(pandasm::Create_CMP_64(inst->GetSrcReg(0U)));
    } else if (inst->GetInputType(0) == compiler::DataType::UINT64) {
        enc->result_.emplace_back(pandasm::Create_UCMP_64(inst->GetSrcReg(0U)));
    } else {
        LOG(ERROR, BYTECODE_OPTIMIZER) << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
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

static void VisitCastFromI32([[maybe_unused]] BytecodeGen *enc, compiler::CastInst *inst,
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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            success = false;
            return;
    }
    DoSta(inst->GetType(), inst->GetDstReg(), res);
}

static void VisitCastFromU32([[maybe_unused]] BytecodeGen *enc, compiler::CastInst *inst,
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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            success = false;
            return;
    }
    DoSta(inst->GetType(), inst->GetDstReg(), res);
}

static void VisitCastFromI64([[maybe_unused]] BytecodeGen *enc, compiler::CastInst *inst,
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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            success = false;
            return;
    }
    DoSta(inst->GetType(), inst->GetDstReg(), res);
}

static void VisitCastFromF32([[maybe_unused]] BytecodeGen *enc, compiler::CastInst *inst,
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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            success = false;
            return;
    }
    DoSta(inst->GetType(), inst->GetDstReg(), res);
}

static void VisitCastFromF64([[maybe_unused]] BytecodeGen *enc, compiler::CastInst *inst,
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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            success = false;
            return;
    }
    DoSta(inst->GetType(), inst->GetDstReg(), res);
}

// NOLINTNEXTLINE(readability-function-size)
void BytecodeGen::VisitCast(GraphVisitor *v, Inst *instBase)
{
    auto enc = static_cast<BytecodeGen *>(v);
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
                    LOG(ERROR, BYTECODE_OPTIMIZER)
                        << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
                    enc->success_ = false;
            }
            break;
        }
        default:
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            enc->success_ = false;
    }
}

void BytecodeGen::VisitLoadString(GraphVisitor *v, Inst *instBase)
{
    pandasm::Ins ins;
    auto enc = static_cast<BytecodeGen *>(v);
    auto inst = instBase->CastToLoadString();

    /* Do not emit unused code for Str -> CastValueToAnyType chains */
    if (enc->GetGraph()->IsDynamicMethod()) {
        if (!HasUserPredicate(inst,
                              [](Inst const *i) { return i->GetOpcode() != compiler::Opcode::CastValueToAnyType; })) {
            return;
        }
    }

    enc->result_.emplace_back(pandasm::Create_LDA_STR(enc->irInterface_->GetStringIdByOffset(inst->GetTypeId())));
    if (inst->GetDstReg() != compiler::GetAccReg()) {
        if (enc->GetGraph()->IsDynamicMethod()) {
            enc->result_.emplace_back(pandasm::Create_STA_DYN(inst->GetDstReg()));
        } else {
            enc->result_.emplace_back(pandasm::Create_STA_OBJ(inst->GetDstReg()));
        }
    }
}

void BytecodeGen::VisitReturnAny(GraphVisitor *v, [[maybe_unused]] Inst *instBase)
{
    auto enc = static_cast<BytecodeGen *>(v);
    [[maybe_unused]] auto inst = instBase->CastToReturn();
#if defined(ENABLE_BYTECODE_OPT) && defined(PANDA_WITH_ECMASCRIPT)
    auto testArg = IsEcmaConstTemplate(inst->GetInput(0U).GetInst());
    if (testArg.has_value() && testArg->IsUndefined()) {
        enc->result_.emplace_back(pandasm::Create_ECMA_RETURNUNDEFINED());
        return;
    }
#endif
#ifdef ARK_INTRINSIC_SET
    DoLdaDyn(inst->GetSrcReg(0U), enc->result_);
    enc->result_.emplace_back(pandasm::Create_ECMA_RETURN_DYN());
#else
    // Do not support DataType::ANY in this case
    LOG(ERROR, BYTECODE_OPTIMIZER) << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
    enc->success_ = false;
#endif  // ARK_INTRINSIC_SET
}

void BytecodeGen::VisitReturn(GraphVisitor *v, Inst *instBase)
{
    pandasm::Ins ins;
    auto enc = static_cast<BytecodeGen *>(v);
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
            BytecodeGen::VisitReturnAny(v, instBase);
            break;
        }
        default:
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            enc->success_ = false;
    }
}

#if defined(ENABLE_BYTECODE_OPT) && defined(PANDA_WITH_ECMASCRIPT)
static auto IntToLdBool(int64_t v)
{
    return v != 0 ? pandasm::Create_ECMA_LDTRUE() : pandasm::Create_ECMA_LDFALSE();
}
#endif

void BytecodeGen::VisitCastValueToAnyType(GraphVisitor *v, [[maybe_unused]] Inst *instBase)
{
    auto enc = static_cast<BytecodeGen *>(v);

#if defined(ENABLE_BYTECODE_OPT) && defined(PANDA_WITH_ECMASCRIPT)
    auto cvat = instBase->CastToCastValueToAnyType();
    switch (cvat->GetAnyType()) {
        case compiler::AnyBaseType::ECMASCRIPT_NULL_TYPE:
            enc->result_.emplace_back(pandasm::Create_ECMA_LDNULL());
            break;
        case compiler::AnyBaseType::ECMASCRIPT_UNDEFINED_TYPE:
            if (!HasUserPredicate(cvat,
                                  [](Inst const *inst) { return inst->GetOpcode() != compiler::Opcode::Return; })) {
                return;
            }
            enc->result_.emplace_back(pandasm::Create_ECMA_LDUNDEFINED());
            break;
        case compiler::AnyBaseType::ECMASCRIPT_INT_TYPE:
            ASSERT(cvat->GetInput(0U).GetInst()->IsConst());
            enc->result_.emplace_back(
                pandasm::Create_LDAI_DYN(cvat->GetInput(0U).GetInst()->CastToConstant()->GetIntValue()));
            break;
        case compiler::AnyBaseType::ECMASCRIPT_DOUBLE_TYPE:
            ASSERT(cvat->GetInput(0U).GetInst()->IsConst());
            enc->result_.emplace_back(
                pandasm::Create_FLDAI_DYN(cvat->GetInput(0U).GetInst()->CastToConstant()->GetDoubleValue()));
            break;
        case compiler::AnyBaseType::ECMASCRIPT_BOOLEAN_TYPE:
            ASSERT(cvat->GetInput(0U).GetInst()->IsBoolConst());
            if (!HasUserPredicate(cvat, [](Inst const *inst) { return inst->GetOpcode() != compiler::Opcode::If; })) {
                return;
            }
            enc->result_.emplace_back(IntToLdBool((cvat->GetInput(0U).GetInst()->CastToConstant()->GetInt64Value())));
            break;
        case compiler::AnyBaseType::ECMASCRIPT_STRING_TYPE:
            enc->result_.emplace_back(pandasm::Create_LDA_STR(
                enc->irInterface_->GetStringIdByOffset(cvat->GetInput(0U).GetInst()->CastToLoadString()->GetTypeId())));
            break;
        case compiler::AnyBaseType::ECMASCRIPT_BIGINT_TYPE:
            enc->result_.emplace_back(pandasm::Create_ECMA_LDBIGINT(
                enc->irInterface_->GetStringIdByOffset(cvat->GetInput(0U).GetInst()->CastToLoadString()->GetTypeId())));
            break;
        default:
            UNREACHABLE();
    }
    DoStaDyn(cvat->GetDstReg(), enc->result_);
#else
    LOG(ERROR, BYTECODE_OPTIMIZER) << "Codegen for " << compiler::GetOpcodeString(instBase->GetOpcode()) << " failed";
    enc->success_ = false;
#endif
}

// NOLINTNEXTLINE(readability-function-size)
void BytecodeGen::VisitStoreObject(GraphVisitor *v, Inst *instBase)
{
    if (TryPluginStoreObjectVisitor(v, instBase)) {
        return;
    }

    auto enc = static_cast<BytecodeGen *>(v);
    const compiler::StoreObjectInst *inst = instBase->CastToStoreObject();

    compiler::Register vd = inst->GetSrcReg(0U);
    compiler::Register vs = inst->GetSrcReg(1U);
    std::string id = enc->irInterface_->GetFieldIdByOffset(inst->GetTypeId());

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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Wrong DataType for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            enc->success_ = false;
    }
}

void BytecodeGen::VisitStoreStatic(GraphVisitor *v, Inst *instBase)
{
    if (TryPluginStoreStaticVisitor(v, instBase)) {
        return;
    }

    auto enc = static_cast<BytecodeGen *>(v);
    auto inst = instBase->CastToStoreStatic();

    compiler::Register vs = inst->GetSrcReg(1U);
    std::string id = enc->irInterface_->GetFieldIdByOffset(inst->GetTypeId());

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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            enc->success_ = false;
    }
}

static bool IsAccLoadObject(const compiler::LoadObjectInst *inst)
{
    return inst->GetDstReg() == compiler::GetAccReg();
}

void BytecodeGen::VisitLoadObject(GraphVisitor *v, Inst *instBase)
{
    if (TryPluginLoadObjectVisitor(v, instBase)) {
        return;
    }

    auto enc = static_cast<BytecodeGen *>(v);
    auto inst = instBase->CastToLoadObject();

    compiler::Register vs = inst->GetSrcReg(0U);
    compiler::Register vd = inst->GetDstReg();
    std::string id = enc->irInterface_->GetFieldIdByOffset(inst->GetTypeId());

    bool isAccType = IsAccLoadObject(inst);

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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Wrong DataType for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            enc->success_ = false;
    }
}

void BytecodeGen::VisitLoadStatic(GraphVisitor *v, Inst *instBase)
{
    if (TryPluginLoadStaticVisitor(v, instBase)) {
        return;
    }

    auto enc = static_cast<BytecodeGen *>(v);
    auto inst = instBase->CastToLoadStatic();

    compiler::Register vd = inst->GetDstReg();
    std::string id = enc->irInterface_->GetFieldIdByOffset(inst->GetTypeId());

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
            LOG(ERROR, BYTECODE_OPTIMIZER)
                << "Codegen for " << compiler::GetOpcodeString(inst->GetOpcode()) << " failed";
            enc->success_ = false;
    }
}

void BytecodeGen::VisitCatchPhi(GraphVisitor *v, Inst *inst)
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
            auto enc = static_cast<BytecodeGen *>(v);
            enc->result_.emplace_back(pandasm::Create_STA_OBJ(inst->GetDstReg()));
        }
    }
}

#include "generated/codegen_intrinsics.cpp"
#include "generated/insn_selection.cpp"
}  // namespace ark::bytecodeopt
