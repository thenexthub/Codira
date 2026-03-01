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

#include "encode_visitor.h"
#include "optimizer/code_generator/operands.h"
#include "optimizer/ir/datatype.h"
#include "libarkbase/utils/regmask.h"
#include "intrinsic_string_flat_check.inl"

namespace ark::compiler {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define UNARY_OPERATION(opc)                                              \
    void EncodeVisitor::Visit##opc(GraphVisitor *visitor, Inst *inst)     \
    {                                                                     \
        EncodeVisitor *enc = static_cast<EncodeVisitor *>(visitor);       \
        auto [dst, src0] = enc->GetCodegen()->ConvertRegisters<1U>(inst); \
        enc->GetEncoder()->Encode##opc(dst, src0);                        \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_OPERATION(opc)                                                   \
    void EncodeVisitor::Visit##opc(GraphVisitor *visitor, Inst *inst)           \
    {                                                                           \
        EncodeVisitor *enc = static_cast<EncodeVisitor *>(visitor);             \
        auto [dst, src0, src1] = enc->GetCodegen()->ConvertRegisters<2U>(inst); \
        enc->GetEncoder()->Encode##opc(dst, src0, src1);                        \
    }

// CC-OFFNXT(G.PRE.06) solid logic
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_SHIFTED_REGISTER_OPERATION(opc)                                        \
    void EncodeVisitor::Visit##opc##SR(GraphVisitor *visitor, Inst *inst)             \
    {                                                                                 \
        EncodeVisitor *enc = static_cast<EncodeVisitor *>(visitor);                   \
        auto [dst, src0, src1] = enc->GetCodegen()->ConvertRegisters<2U>(inst);       \
        auto imm_shift_inst = static_cast<BinaryShiftedRegisterOperation *>(inst);    \
        auto dstSize = dst.GetSize() > 1 ? dst.GetSize() - 1 : 0;                     \
        auto imm_value = static_cast<uint32_t>(imm_shift_inst->GetImm()) & (dstSize); \
        auto shift = Shift(src1, imm_shift_inst->GetShiftType(), imm_value);          \
        enc->GetEncoder()->Encode##opc(dst, src0, shift);                             \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INST_DEF(OPCODE, MACRO) MACRO(OPCODE)

ENCODE_MATH_LIST(INST_DEF)
ENCODE_INST_WITH_SHIFTED_OPERAND(INST_DEF)

#undef UNARY_OPERATION
#undef BINARY_OPERATION
#undef BINARY_SHIFTED_REGISTER_OPERATION
#undef INST_DEF

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_IMM_OPERATION(opc)                                         \
    void EncodeVisitor::Visit##opc##I(GraphVisitor *visitor, Inst *inst)  \
    {                                                                     \
        auto binop = inst->CastTo##opc##I();                              \
        EncodeVisitor *enc = static_cast<EncodeVisitor *>(visitor);       \
        auto [dst, src0] = enc->GetCodegen()->ConvertRegisters<1U>(inst); \
        enc->GetEncoder()->Encode##opc(dst, src0, Imm(binop->GetImm()));  \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARRY_IMM_OPS(DEF) \
    DEF(Add)                 \
    DEF(Sub)                 \
    DEF(Shl)                 \
    DEF(AShr)                \
    DEF(And)                 \
    DEF(Or)                  \
    DEF(Xor)

BINARRY_IMM_OPS(BINARY_IMM_OPERATION)

#undef BINARRY_IMM_OPS
#undef BINARY_IMM_OPERATION

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_IMM_SIGN_OPERATION(opc)                                                       \
    void EncodeVisitor::Visit##opc##I(GraphVisitor *visitor, Inst *inst)                     \
    {                                                                                        \
        auto type = inst->GetType();                                                         \
        auto binop = inst->CastTo##opc##I();                                                 \
        EncodeVisitor *enc = static_cast<EncodeVisitor *>(visitor);                          \
        auto [dst, src0] = enc->GetCodegen()->ConvertRegisters<1U>(inst);                    \
        enc->GetEncoder()->Encode##opc(dst, src0, Imm(binop->GetImm()), IsTypeSigned(type)); \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_IMM_SIGN_OPS(DEF) \
    DEF(Div)                     \
    DEF(Mod)

BINARY_IMM_SIGN_OPS(BINARY_IMM_SIGN_OPERATION)

#undef BINARY_IMM_SIGN_OPS
#undef BINARY_IMM_SIGN_OPERATION

// CC-OFFNXT(G.PRE.06) solid logic
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BINARY_SIGN_UNSIGN_OPERATION(opc)                                        \
    void EncodeVisitor::Visit##opc(GraphVisitor *visitor, Inst *inst)            \
    {                                                                            \
        auto type = inst->GetType();                                             \
        EncodeVisitor *enc = static_cast<EncodeVisitor *>(visitor);              \
        auto [dst, src0, src1] = enc->GetCodegen()->ConvertRegisters<2U>(inst);  \
        auto arch = enc->GetCodegen()->GetArch();                                \
        if (!Codegen::InstEncodedWithLibCall(inst, arch)) {                      \
            enc->GetEncoder()->Encode##opc(dst, IsTypeSigned(type), src0, src1); \
            return; /* CC-OFF(G.PRE.05) function gen */                          \
        }                                                                        \
        ASSERT(arch == Arch::AARCH32);                                           \
        if (enc->cg_->GetGraph()->IsAotMode()) {                                 \
            enc->GetEncoder()->SetFalseResult();                                 \
            return; /* CC-OFF(G.PRE.05) function gen */                          \
        }                                                                        \
        auto [liveRegs, liveFpRegs] = enc->GetCodegen()->GetLiveRegisters(inst); \
        enc->GetEncoder()->SetRegister(&liveRegs, &liveFpRegs, dst, false);      \
        enc->GetCodegen()->SaveCallerRegisters(liveRegs, liveFpRegs, true);      \
        enc->GetEncoder()->Encode##opc(dst, IsTypeSigned(type), src0, src1);     \
        enc->GetCodegen()->LoadCallerRegisters(liveRegs, liveFpRegs, true);      \
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SIGN_UNSIGN_OPS(DEF) \
    DEF(Div)                 \
    DEF(Min)                 \
    DEF(Max)

SIGN_UNSIGN_OPS(BINARY_SIGN_UNSIGN_OPERATION)

#undef SIGN_UNSIGN_OPS
#undef BINARY_SIGN_UNSIGN_OPERATION

void EncodeVisitor::VisitMod(GraphVisitor *visitor, Inst *inst)
{
    auto type = inst->GetType();
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto [dst, src0, src1] = enc->GetCodegen()->ConvertRegisters<2U>(inst);
    auto arch = enc->GetCodegen()->GetArch();
    if (!Codegen::InstEncodedWithLibCall(inst, arch)) {
        enc->GetEncoder()->EncodeMod(dst, IsTypeSigned(type), src0, src1);
        return;
    }
    if (DataType::IsFloatType(type)) {
        RuntimeInterface::IntrinsicId entry =
            ((type == DataType::FLOAT32) ? RuntimeInterface::IntrinsicId::LIB_CALL_FMODF
                                         : RuntimeInterface::IntrinsicId::LIB_CALL_FMOD);
        auto [liveRegs, liveFpRegs] = enc->GetCodegen()->GetLiveRegisters(inst);
        enc->GetCodegen()->SaveCallerRegisters(liveRegs, liveFpRegs, true);

        enc->GetCodegen()->FillCallParams(src0, src1);
        enc->GetCodegen()->CallIntrinsic(inst, entry);

        auto retVal = enc->GetCodegen()->GetTarget().GetReturnFpReg();
        if (retVal.GetType().IsFloat()) {
            // ret_val is FLOAT64 for Arm64, AMD64 and ARM32 HardFP, but dst can be FLOAT32
            // so we convert ret_val to FLOAT32
            enc->GetEncoder()->EncodeMov(dst, Reg(retVal.GetId(), dst.GetType()));
        } else {
            // case for ARM32 SoftFP
            enc->GetEncoder()->EncodeMov(dst,
                                         Reg(retVal.GetId(), dst.GetSize() == WORD_SIZE ? INT32_TYPE : INT64_TYPE));
        }

        enc->GetEncoder()->SetRegister(&liveRegs, &liveFpRegs, dst, false);
        enc->GetCodegen()->LoadCallerRegisters(liveRegs, liveFpRegs, true);
        return;
    }
    ASSERT(arch == Arch::AARCH32);
    // Fix after supporting AOT mode for arm32
    if (enc->cg_->GetGraph()->IsAotMode()) {
        enc->GetEncoder()->SetFalseResult();
        return;
    }
    auto [liveRegs, liveFpRegs] = enc->GetCodegen()->GetLiveRegisters(inst);
    enc->GetEncoder()->SetRegister(&liveRegs, &liveFpRegs, dst, false);
    enc->GetCodegen()->SaveCallerRegisters(liveRegs, liveFpRegs, true);
    enc->GetEncoder()->EncodeMod(dst, IsTypeSigned(type), src0, src1);
    enc->GetCodegen()->LoadCallerRegisters(liveRegs, liveFpRegs, true);
}

void EncodeVisitor::VisitExtractBitfield(GraphVisitor *visitor, Inst *inst)
{
    auto bfx = static_cast<ExtractBitfieldInst *>(inst);
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto [dst, src0] = enc->GetCodegen()->ConvertRegisters<1U>(inst);
    enc->GetEncoder()->EncodeExtractBits(dst, src0, Imm(bfx->GetSourceBit()), Imm(bfx->GetWidth()),
                                         bfx->IsSignBitExtension());
}

void EncodeVisitor::VisitShrI(GraphVisitor *visitor, Inst *inst)
{
    auto binop = inst->CastToShrI();
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto [dst, src0] = enc->GetCodegen()->ConvertRegisters<1U>(inst);
    enc->GetEncoder()->EncodeShr(dst, src0, Imm(binop->GetImm()));
}

void EncodeVisitor::VisitMAdd(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto [dst, src0, src1, src2] = enc->GetCodegen()->ConvertRegisters<3U>(inst);
    enc->GetEncoder()->EncodeMAdd(dst, src0, src1, src2);
}

void EncodeVisitor::VisitMSub(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto [dst, src0, src1, src2] = enc->GetCodegen()->ConvertRegisters<3U>(inst);
    enc->GetEncoder()->EncodeMSub(dst, src0, src1, src2);
}

void EncodeVisitor::VisitMNeg(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto [dst, src0, src1] = enc->GetCodegen()->ConvertRegisters<2U>(inst);
    enc->GetEncoder()->EncodeMNeg(dst, src0, src1);
}

void EncodeVisitor::VisitOrNot(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto [dst, src0, src1] = enc->GetCodegen()->ConvertRegisters<2U>(inst);
    enc->GetEncoder()->EncodeOrNot(dst, src0, src1);
}

void EncodeVisitor::VisitAndNot(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto [dst, src0, src1] = enc->GetCodegen()->ConvertRegisters<2U>(inst);
    enc->GetEncoder()->EncodeAndNot(dst, src0, src1);
}

void EncodeVisitor::VisitXorNot(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto [dst, src0, src1] = enc->GetCodegen()->ConvertRegisters<2U>(inst);
    enc->GetEncoder()->EncodeXorNot(dst, src0, src1);
}

void EncodeVisitor::VisitNegSR(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto [dst, src] = enc->GetCodegen()->ConvertRegisters<1U>(inst);
    auto immShiftInst = static_cast<UnaryShiftedRegisterOperation *>(inst);
    enc->GetEncoder()->EncodeNeg(dst, Shift(src, immShiftInst->GetShiftType(), immShiftInst->GetImm()));
}

void EncodeVisitor::VisitCast(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto srcType = inst->GetInputType(0);
    auto dstType = inst->GetType();
    ASSERT(dstType != DataType::ANY);
    bool srcSigned = IsTypeSigned(srcType);
    bool dstSigned = IsTypeSigned(dstType);
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), srcType);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), dstType);
    if (dstType == DataType::BOOL) {
        enc->GetEncoder()->EncodeCastToBool(dst, src);
        return;
    }
    if (inst->CastToCast()->IsDynamicCast()) {
        enc->GetCodegen()->EncodeDynamicCast(inst, dst, dstSigned, src);
        return;
    }
    auto arch = enc->GetCodegen()->GetArch();
    if (!Codegen::InstEncodedWithLibCall(inst, arch)) {
        enc->GetEncoder()->EncodeCast(dst, dstSigned, src, srcSigned);
        return;
    }
    ASSERT(arch == Arch::AARCH32);
    // Fix after supporting AOT mode for arm32
    if (enc->cg_->GetGraph()->IsAotMode()) {
        enc->GetEncoder()->SetFalseResult();
        return;
    }
    auto [liveRegs, liveVregs] = enc->GetCodegen()->GetLiveRegisters(inst);
    enc->GetEncoder()->SetRegister(&liveRegs, &liveVregs, dst, false);
    enc->GetCodegen()->SaveCallerRegisters(liveRegs, liveVregs, true);
    enc->GetEncoder()->EncodeCast(dst, dstSigned, src, srcSigned);
    enc->GetCodegen()->LoadCallerRegisters(liveRegs, liveVregs, true);
}

void EncodeVisitor::VisitBitcast(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();
    auto srcType = inst->GetInputType(0);
    auto dstType = inst->GetType();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), dstType);
    auto src = codegen->ConvertRegister(inst->GetSrcReg(0), srcType);
    enc->GetEncoder()->EncodeMov(dst, src);
}

void EncodeVisitor::VisitPhi([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst) {}

void EncodeVisitor::VisitConstant(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    if (inst->GetDstReg() == INVALID_REG) {
        return;
    }
    if (inst->GetDstReg() == enc->cg_->GetGraph()->GetZeroReg()) {
        ASSERT(IsZeroConstant(inst));
        ASSERT(enc->GetRegfile()->GetZeroReg() != INVALID_REGISTER);
        return;
    }
    auto *constInst = inst->CastToConstant();
    auto type = inst->GetType();
    if (enc->cg_->GetGraph()->IsDynamicMethod() && type == DataType::INT64) {
        type = DataType::INT32;
    }
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
#ifndef NDEBUG
    switch (type) {
        case DataType::FLOAT32:
            enc->GetEncoder()->EncodeMov(dst, Imm(constInst->GetFloatValue()));
            break;
        case DataType::FLOAT64:
            enc->GetEncoder()->EncodeMov(dst, Imm(constInst->GetDoubleValue()));
            break;
        default:
            enc->GetEncoder()->EncodeMov(dst, Imm(constInst->GetRawValue()));
    }
#else
    enc->GetEncoder()->EncodeMov(dst, Imm(constInst->GetRawValue()));
#endif
}

void EncodeVisitor::VisitNullPtr(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    if (inst->GetDstReg() == enc->cg_->GetGraph()->GetZeroReg()) {
        ASSERT_PRINT(enc->GetRegfile()->GetZeroReg() != INVALID_REGISTER,
                     "NullPtr doesn't have correct destination register");
        return;
    }
    auto type = inst->GetType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    enc->GetEncoder()->EncodeMov(dst, Imm(0));
}

void EncodeVisitor::VisitLoadUniqueObject(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto type = inst->GetType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    auto runtime = enc->GetCodegen()->GetGraph()->GetRuntime();
    auto graph = enc->GetCodegen()->GetGraph();
    if (graph->IsJitOrOsrMode()) {
        enc->GetEncoder()->EncodeMov(dst, Imm(runtime->GetUniqueObject()));
    } else {
        auto ref = MemRef(enc->GetCodegen()->ThreadReg(), runtime->GetTlsUniqueObjectOffset(graph->GetArch()));
        enc->GetEncoder()->EncodeLdr(dst, false, ref);
    }
}

void EncodeVisitor::VisitCallIndirect(GraphVisitor *visitor, Inst *inst)
{
    static_cast<EncodeVisitor *>(visitor)->GetCodegen()->VisitCallIndirect(inst->CastToCallIndirect());
}

void EncodeVisitor::VisitCall(GraphVisitor *visitor, Inst *inst)
{
    static_cast<EncodeVisitor *>(visitor)->GetCodegen()->VisitCall(inst->CastToCall());
}

void EncodeVisitor::VisitCompare(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);

    auto type = inst->CastToCompare()->GetOperandsType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    auto cc = enc->GetCodegen()->ConvertCc(inst->CastToCompare()->GetCc());
    if (IsTestCc(cc)) {
        enc->GetEncoder()->EncodeCompareTest(dst, src0, src1, cc);
    } else {
        enc->GetEncoder()->EncodeCompare(dst, src0, src1, cc);
    }
}

void EncodeVisitor::VisitCmp(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto cmpInst = inst->CastToCmp();
    auto type = cmpInst->GetOperandsType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    Condition cc;
    if (DataType::IsFloatType(type)) {
        // check whether MI is fully correct here
        cc = cmpInst->IsFcmpg() ? (Condition::MI) : (Condition::LT);
    } else if (IsTypeSigned(type)) {
        cc = Condition::LT;
    } else {
        cc = Condition::LO;
    }
    enc->GetEncoder()->EncodeCmp(dst, src0, src1, cc);
}

void EncodeVisitor::VisitReturnVoid(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        enc->GetEncoder()->EncodeMemoryBarrier(memory_order::RELEASE);
    }
    enc->GetCodegen()->CreateReturn(inst);
}

void EncodeVisitor::VisitReturn(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), inst->GetType());
    enc->GetEncoder()->EncodeMov(enc->GetCodegen()->GetTarget().GetReturnReg(src.GetType()), src);
    enc->GetCodegen()->CreateReturn(inst);
}

void EncodeVisitor::VisitReturnI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();
    auto rzero = enc->GetRegfile()->GetZeroReg();
    auto immVal = inst->CastToReturnI()->GetImm();
    Imm imm = codegen->ConvertImmWithExtend(immVal, inst->GetType());
    auto returnReg = codegen->GetTarget().GetReturnReg(codegen->ConvertDataType(inst->GetType(), codegen->GetArch()));
    if (immVal == 0 && codegen->GetTarget().SupportZeroReg() && !DataType::IsFloatType(inst->GetType())) {
        enc->GetEncoder()->EncodeMov(returnReg, rzero);
    } else {
        enc->GetEncoder()->EncodeMov(returnReg, imm);
    }
    enc->GetCodegen()->CreateReturn(inst);
}

#if defined(EVENT_METHOD_EXIT_ENABLED) && EVENT_METHOD_EXIT_ENABLED != 0
static CallInst *GetCallInstFromReturnInlined(Inst *return_inlined)
{
    auto ss = return_inlined->GetSaveState();
    for (auto &user : ss->GetUsers()) {
        auto inst = user.GetInst();
        if (inst->IsCall() && static_cast<CallInst *>(inst)->IsInlined()) {
            return static_cast<CallInst *>(inst);
        }
    }
    return nullptr;
}
#endif

void EncodeVisitor::VisitReturnInlined(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        enc->GetEncoder()->EncodeMemoryBarrier(memory_order::RELEASE);
    }
#if defined(EVENT_METHOD_EXIT_ENABLED) && EVENT_METHOD_EXIT_ENABLED != 0
    if (!enc->cg_->GetGraph()->IsAotMode()) {
        auto callInst = GetCallInstFromReturnInlined(inst->CastToReturnInlined());
        ASSERT(callInst != nullptr);
        static_cast<EncodeVisitor *>(visitor)->GetCodegen()->InsertTrace(
            {Imm(static_cast<size_t>(TraceId::METHOD_EXIT)), Imm(reinterpret_cast<size_t>(callInst->GetCallMethod())),
             Imm(static_cast<size_t>(events::MethodExitKind::INLINED))});
    }
#endif
}

void EncodeVisitor::VisitLoadConstArray(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto method = inst->CastToLoadConstArray()->GetMethod();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto arrayType = inst->CastToLoadConstArray()->GetTypeId();
    auto typeIdImm = enc->GetCodegen()->GetTypeIdImm(inst, arrayType);
    enc->GetCodegen()->CallRuntimeWithMethod(inst, method, EntrypointId::RESOLVE_LITERAL_ARRAY, dst, typeIdImm);
}

void EncodeVisitor::VisitFillConstArray(GraphVisitor *visitor, Inst *inst)
{
    auto type = inst->GetType();
    ASSERT(type != DataType::REFERENCE);
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    auto arrayType = inst->CastToFillConstArray()->GetTypeId();
    auto arch = enc->cg_->GetGraph()->GetArch();
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto method = inst->CastToFillConstArray()->GetMethod();
    auto offset = runtime->GetArrayDataOffset(enc->GetCodegen()->GetArch());
    auto arraySize = inst->CastToFillConstArray()->GetImm() << DataType::ShiftByType(type, arch);
    auto arrayReg = enc->GetCodegen()->ConvertInstTmpReg(inst, DataType::GetIntTypeForReference(arch));
    encoder->EncodeAdd(arrayReg, src, Imm(offset));
    ASSERT(arraySize != 0);
    if (enc->cg_->GetGraph()->IsAotMode()) {
        auto arrOffset = runtime->GetOffsetToConstArrayData(method, arrayType);
        auto pfOffset = runtime->GetPandaFileOffset(arch);
        ScopedTmpReg methodReg(encoder);
        enc->GetCodegen()->LoadMethod(methodReg);
        // load pointer to panda file
        encoder->EncodeLdr(methodReg, false, MemRef(methodReg, pfOffset));
        // load pointer to binary file
        encoder->EncodeLdr(methodReg, false, MemRef(methodReg, runtime->GetBinaryFileBaseOffset(enc->GetArch())));
        // Get pointer to array data
        encoder->EncodeAdd(methodReg, methodReg, Imm(arrOffset));
        // call memcpy
        RuntimeInterface::IntrinsicId entry = RuntimeInterface::IntrinsicId::LIB_CALL_MEM_COPY;
        auto [liveRegs, liveFpRegs] = enc->GetCodegen()->GetLiveRegisters(inst);
        enc->GetCodegen()->SaveCallerRegisters(liveRegs, liveFpRegs, true);

        enc->GetCodegen()->FillCallParams(arrayReg, methodReg, TypedImm(arraySize));
        enc->GetCodegen()->CallIntrinsic(inst, entry);
        enc->GetCodegen()->LoadCallerRegisters(liveRegs, liveFpRegs, true);
    } else {
        auto data = runtime->GetPointerToConstArrayData(method, arrayType);
        // call memcpy
        RuntimeInterface::IntrinsicId entry = RuntimeInterface::IntrinsicId::LIB_CALL_MEM_COPY;
        auto [liveRegs, liveFpRegs] = enc->GetCodegen()->GetLiveRegisters(inst);
        enc->GetCodegen()->SaveCallerRegisters(liveRegs, liveFpRegs, true);

        enc->GetCodegen()->FillCallParams(arrayReg, TypedImm(data), TypedImm(arraySize));
        enc->GetCodegen()->CallIntrinsic(inst, entry);
        enc->GetCodegen()->LoadCallerRegisters(liveRegs, liveFpRegs, true);
    }
}

void EncodeVisitor::VisitNewArray(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    return enc->GetCodegen()->VisitNewArray(inst);
}

void EncodeVisitor::VisitParameter(GraphVisitor *visitor, Inst *inst)
{
    // Default register parameters pushed in ir_builder
    // In regalloc filled spill/fill parameters part.
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();
    auto paramInst = inst->CastToParameter();
    auto sf = paramInst->GetLocationData();
    if (sf.GetSrc() == sf.GetDst()) {
        return;
    }
    auto tmpSf = codegen->GetGraph()->CreateInstSpillFill();
    tmpSf->AddSpillFill(sf);
    SpillFillEncoder(codegen, tmpSf).EncodeSpillFill();
}

void EncodeVisitor::VisitStoreArray(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto array = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto index = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::INT32);
    constexpr int64_t IMM_2 = 2;
    auto storedValue = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_2), inst->GetType());
    auto offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch());
    auto scale = DataType::ShiftByType(inst->GetType(), enc->GetCodegen()->GetArch());
    auto memRef = [enc, inst, array, index, offset, scale]() {
        if (inst->CastToStoreArray()->GetNeedBarrier()) {
            auto tmpOffset =
                enc->GetCodegen()->ConvertInstTmpReg(inst, DataType::GetIntTypeForReference(enc->GetArch()));
            auto indexUpcasted = index.As(array.GetType());
            enc->GetEncoder()->EncodeShl(tmpOffset, indexUpcasted, Imm(scale));
            enc->GetEncoder()->EncodeAdd(tmpOffset, tmpOffset, Imm(offset));
            return MemRef(array, tmpOffset, 0);
        }
        auto tmp = enc->GetCodegen()->ConvertInstTmpReg(inst, DataType::REFERENCE);
        enc->GetEncoder()->EncodeAdd(tmp, array, Imm(offset));
        return MemRef(tmp, index, scale);
    };
    auto mem = memRef();
    if (inst->CastToStoreArray()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem, MakeMask(array.GetId(), storedValue.GetId()));
    }
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeStr(storedValue, mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
    if (inst->CastToStoreArray()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, mem, storedValue, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitSpillFill(GraphVisitor *visitor, Inst *inst)
{
    auto codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    SpillFillEncoder(codegen, inst).EncodeSpillFill();
}

void EncodeVisitor::VisitSaveState([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    // Nothing to do, SaveState is processed in its users.
}

void EncodeVisitor::VisitSaveStateDeoptimize([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    // Nothing to do, SaveStateDeoptimize is processed in its users.
}

void EncodeVisitor::VisitSaveStateOsr(GraphVisitor *visitor, Inst *inst)
{
    static_cast<EncodeVisitor *>(visitor)->GetCodegen()->CreateOsrEntry(inst->CastToSaveStateOsr());
}

void EncodeVisitor::VisitLoadArray(GraphVisitor *visitor, Inst *inst)
{
    auto instLoadArray = inst->CastToLoadArray();
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    ASSERT(instLoadArray->IsArray() || !runtime->IsCompressedStringsEnabled());

    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // array
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::INT32);      // index
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                   // load value
    auto offset = instLoadArray->IsArray() ? runtime->GetArrayDataOffset(enc->GetCodegen()->GetArch())
                                           : runtime->GetStringDataOffset(enc->GetArch());
    auto encoder = enc->GetEncoder();
    auto arch = encoder->GetArch();
    auto shift = DataType::ShiftByType(type, arch);
    ScopedTmpReg scopedTmp(encoder, Codegen::ConvertDataType(DataType::GetIntTypeForReference(arch), arch));
    auto tmp = scopedTmp.GetReg();
    encoder->EncodeAdd(tmp, src0, Imm(offset));
    auto mem = MemRef(tmp, src1, shift);
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    if (instLoadArray->GetNeedBarrier()) {
        enc->GetCodegen()->CreateReadViaBarrier(inst, mem, dst);
    } else {
        encoder->EncodeLdr(dst, IsTypeSigned(type), mem);
    }
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
}

void EncodeVisitor::VisitLoadCompressedStringChar(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    auto type = inst->GetType();
    auto array = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto index = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::INT32);
    auto length = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(2), DataType::INT32);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);  // load value
    auto offset = runtime->GetStringDataOffset(enc->GetArch());
    auto encoder = enc->GetEncoder();
    auto arch = encoder->GetArch();
    auto shift = DataType::ShiftByType(type, arch);
    auto indexUpcasted = index.As(array.GetType());
    ScopedTmpReg scopedTmp(encoder, Codegen::ConvertDataType(DataType::GetIntTypeForReference(arch), arch));
    auto tmp = scopedTmp.GetReg();
    ASSERT(encoder->CanEncodeCompressedStringCharAt());
    auto mask = runtime->GetStringCompressionMask();
    if (mask != 1) {
        UNREACHABLE();  // mask is hardcoded in JCL, but verify it just in case it's changed
    }
    enc->GetEncoder()->EncodeCompressedStringCharAt({dst, array, indexUpcasted, length, tmp, offset, shift});
}

void EncodeVisitor::VisitLenArray(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // array
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                   // len array
    auto lenArrayInst = inst->CastToLenArray();
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    int64_t offset = lenArrayInst->IsArray() ? runtime->GetArrayLengthOffset(enc->GetCodegen()->GetArch())
                                             : runtime->GetStringLengthOffset(enc->GetArch());
    auto mem = MemRef(src0, offset);
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeLdr(dst, IsTypeSigned(type), mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
}

void EncodeVisitor::VisitBuiltin([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    UNREACHABLE();
}

void EncodeVisitor::VisitNullCheck(GraphVisitor *visitor, Inst *inst)
{
    if (inst->CastToNullCheck()->IsImplicit()) {
        return;
    }
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->template CreateUnaryCheck<SlowPathImplicitNullCheck>(inst, EntrypointId::NULL_POINTER_EXCEPTION,
                                                                            DeoptimizeType::NULL_CHECK, Condition::EQ);
}

void EncodeVisitor::VisitBoundsCheck(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto lenType = inst->GetInputType(0);
    auto len = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), lenType);
    auto indexType = inst->GetInputType(1);
    auto index = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), indexType);
    [[maybe_unused]] constexpr int64_t IMM_2 = 2;
    ASSERT(inst->GetInput(IMM_2).GetInst()->GetOpcode() == Opcode::SaveState ||
           inst->GetInput(IMM_2).GetInst()->GetOpcode() == Opcode::SaveStateDeoptimize);
    EntrypointId entrypoint = inst->CastToBoundsCheck()->IsArray() ? EntrypointId::ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION
                                                                   : EntrypointId::STRING_INDEX_OUT_OF_BOUNDS_EXCEPTION;
    LabelHolder::LabelId label;
    if (inst->CanDeoptimize()) {
        DeoptimizeType type = DeoptimizeType::BOUNDS_CHECK_WITH_DEOPT;
        label = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, type)->GetLabel();
    } else {
        label = enc->GetCodegen()->CreateSlowPath<SlowPathCheck>(inst, entrypoint)->GetLabel();
    }
    enc->GetEncoder()->EncodeJump(label, index, len, Condition::HS);
}

void EncodeVisitor::VisitRefTypeCheck(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto arrayReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto refReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    [[maybe_unused]] constexpr int64_t IMM_2 = 2;
    ASSERT(inst->GetInput(IMM_2).GetInst()->GetOpcode() == Opcode::SaveState ||
           inst->GetInput(IMM_2).GetInst()->GetOpcode() == Opcode::SaveStateDeoptimize);
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    auto eid = inst->CanDeoptimize() ? RuntimeInterface::EntrypointId::CHECK_STORE_ARRAY_REFERENCE_DEOPTIMIZE
                                     : RuntimeInterface::EntrypointId::CHECK_STORE_ARRAY_REFERENCE;
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathRefCheck>(inst, eid);
    slowPath->SetRegs(arrayReg, refReg);
    slowPath->CreateBackLabel(encoder);
    // We don't check if stored object is nullptr
    encoder->EncodeJump(slowPath->GetBackLabel(), refReg, Condition::EQ);
    ScopedTmpReg tmpReg(encoder, Codegen::ConvertDataType(DataType::REFERENCE, enc->GetCodegen()->GetArch()));
    ScopedTmpReg tmpReg1(encoder, Codegen::ConvertDataType(DataType::REFERENCE, enc->GetCodegen()->GetArch()));
    // Get Class from array
    enc->GetCodegen()->LoadClassFromObject(tmpReg, arrayReg);
    // Get element type Class from array class
    encoder->EncodeLdr(tmpReg, false, MemRef(tmpReg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Get Class from stored object
    enc->GetCodegen()->LoadClassFromObject(tmpReg1, refReg);
    // If the object's and array element's types match we do not check further
    encoder->EncodeJump(slowPath->GetBackLabel(), tmpReg, tmpReg1, Condition::EQ);
    // If the array's element class is not Object (baseclass == null)
    // we call CheckStoreArrayReference, otherwise we fall through
    encoder->EncodeLdr(tmpReg1, false, MemRef(tmpReg, runtime->GetClassBaseOffset(enc->GetArch())));
    encoder->EncodeJump(slowPath->GetLabel(), tmpReg1, Condition::NE);
    slowPath->BindBackLabel(encoder);
}

void EncodeVisitor::VisitZeroCheck(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->template CreateUnaryCheck<SlowPathCheck>(inst, EntrypointId::ARITHMETIC_EXCEPTION,
                                                                DeoptimizeType::ZERO_CHECK, Condition::EQ);
}

void EncodeVisitor::VisitNegativeCheck(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->template CreateUnaryCheck<SlowPathCheck>(inst, EntrypointId::NEGATIVE_ARRAY_SIZE_EXCEPTION,
                                                                DeoptimizeType::NEGATIVE_CHECK, Condition::LT);
}

void EncodeVisitor::VisitNotPositiveCheck(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto srcType = inst->GetInputType(0);
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), srcType);
    ASSERT(inst->GetInput(1).GetInst()->GetOpcode() == Opcode::SaveState ||
           inst->GetInput(1).GetInst()->GetOpcode() == Opcode::SaveStateDeoptimize);
    ASSERT(inst->CanDeoptimize());
    DeoptimizeType type = DeoptimizeType::NEGATIVE_CHECK;
    auto label = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, type)->GetLabel();
    enc->GetEncoder()->EncodeJump(label, src, Condition::LE);
}

void EncodeVisitor::VisitDeoptimizeIf(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto srcType = inst->GetInput(0).GetInst()->GetType();
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), srcType);
    auto slowPath =
        enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, inst->CastToDeoptimizeIf()->GetDeoptimizeType());
    // create jump to slow path if src is true
    enc->GetEncoder()->EncodeJump(slowPath->GetLabel(), src, Condition::NE);
}

template <typename T>
void EncodeJump(Encoder *encoder, LabelHolder::LabelId label, Reg src0, T src1, Condition cc)
{
    if (IsTestCc(cc)) {
        encoder->EncodeJumpTest(label, src0, src1, cc);
    } else {
        encoder->EncodeJump(label, src0, src1, cc);
    }
}

void EncodeVisitor::VisitDeoptimizeCompare(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto deopt = inst->CastToDeoptimizeCompare();
    ASSERT(deopt->GetOperandsType() != DataType::NO_TYPE);
    auto src0 = enc->GetCodegen()->ConvertRegister(deopt->GetSrcReg(0), deopt->GetOperandsType());
    auto src1 = enc->GetCodegen()->ConvertRegister(deopt->GetSrcReg(1), deopt->GetOperandsType());
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(
        inst, inst->CastToDeoptimizeCompare()->GetDeoptimizeType());
    EncodeJump(enc->GetEncoder(), slowPath->GetLabel(), src0, src1, enc->GetCodegen()->ConvertCc(deopt->GetCc()));
}

void EncodeVisitor::VisitDeoptimizeCompareImm(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto deopt = inst->CastToDeoptimizeCompareImm();
    ASSERT(deopt->GetOperandsType() != DataType::NO_TYPE);
    auto cc = deopt->GetCc();
    auto src0 = enc->GetCodegen()->ConvertRegister(deopt->GetSrcReg(0), deopt->GetOperandsType());
    auto slowPathLabel =
        enc->GetCodegen()
            ->CreateSlowPath<SlowPathDeoptimize>(inst, inst->CastToDeoptimizeCompareImm()->GetDeoptimizeType())
            ->GetLabel();

    if (deopt->GetImm() == 0) {
        Arch arch = enc->GetCodegen()->GetArch();
        DataType::Type type = deopt->GetInput(0).GetInst()->GetType();
        ASSERT(!IsFloatType(type));
        if (IsTypeSigned(type) && (cc == ConditionCode::CC_LT || cc == ConditionCode::CC_GE)) {
            auto signBit = GetTypeSize(type, arch) - 1;
            if (cc == ConditionCode::CC_LT) {
                // x < 0
                encoder->EncodeBitTestAndBranch(slowPathLabel, src0, signBit, true);
                return;
            }
            if (cc == ConditionCode::CC_GE) {
                // x >= 0
                encoder->EncodeBitTestAndBranch(slowPathLabel, src0, signBit, false);
                return;
            }
        }
        if (enc->GetCodegen()->GetTarget().SupportZeroReg()) {
            auto zreg = enc->GetRegfile()->GetZeroReg();
            EncodeJump(encoder, slowPathLabel, src0, zreg, enc->GetCodegen()->ConvertCc(cc));
        } else {
            EncodeJump(encoder, slowPathLabel, src0, Imm(0), enc->GetCodegen()->ConvertCc(cc));
        }
        return;
    }
    EncodeJump(encoder, slowPathLabel, src0, Imm(deopt->GetImm()), enc->GetCodegen()->ConvertCc(cc));
}

void EncodeVisitor::VisitLoadString(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto method = inst->CastToLoadString()->GetMethod();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto stringType = inst->CastToLoadString()->GetTypeId();
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    ASSERT(inst->IsRuntimeCall());

    // Static constructor invoked only once, so there is no sense in replacing regular
    // ResolveString runtime call with optimized version that will only slow down constructor's execution.
    auto isCctor = graph->GetRuntime()->IsMethodStaticConstructor(method);
    if (graph->IsAotMode() && g_options.IsCompilerAotLoadStringPlt() && !isCctor) {
        auto aotData = graph->GetAotData();
        intptr_t slotOffset = aotData->GetStringSlotOffset(
            encoder->GetCursorOffset(), stringType, enc->GetCodegen()->GetAOTBinaryFileSnapshotIndexForInst(inst));
        ScopedTmpRegU64 addrReg(encoder);
        ScopedTmpRegU64 tmpDst(encoder);
        encoder->MakeLoadAotTableAddr(slotOffset, addrReg, tmpDst);

        auto slowPath =
            enc->GetCodegen()->CreateSlowPath<SlowPathResolveStringAot>(inst, EntrypointId::RESOLVE_STRING_AOT);
        slowPath->SetDstReg(dst);
        slowPath->SetAddrReg(addrReg);
        slowPath->SetStringId(stringType);
        slowPath->SetMethod(method);
        encoder->EncodeJump(slowPath->GetLabel(), tmpDst, Imm(RuntimeInterface::RESOLVE_STRING_AOT_COUNTER_LIMIT),
                            Condition::LT);
        encoder->EncodeMov(dst, Reg(tmpDst.GetReg().GetId(), dst.GetType()));
        slowPath->BindBackLabel(encoder);
        return;
    }
    if (!graph->IsAotMode()) {
        ObjectPointerType stringPtr = graph->GetRuntime()->GetNonMovableString(method, stringType);
        if (stringPtr != 0) {
            encoder->EncodeMov(dst, Imm(stringPtr));
            EVENT_JIT_USE_RESOLVED_STRING(graph->GetRuntime()->GetMethodName(method), stringType);
            return;
        }
    }
    auto typeIdImm = enc->GetCodegen()->GetTypeIdImm(inst, stringType);
    enc->GetCodegen()->CallRuntimeWithMethod(inst, method, EntrypointId::RESOLVE_STRING, dst, typeIdImm);
}

void EncodeVisitor::VisitLoadObject(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto loadObj = inst->CastToLoadObject();
    auto type = inst->GetType();
    auto typeInput = inst->GetInput(0).GetInst()->GetType();
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), typeInput);  // obj
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);        // load value
    auto graph = enc->cg_->GetGraph();
    auto field = loadObj->GetObjField();
    size_t offset = GetObjectOffset(graph, loadObj->GetObjectType(), field, loadObj->GetTypeId());
    auto mem = MemRef(src, offset);
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    if (loadObj->GetNeedBarrier()) {
        enc->GetCodegen()->CreateReadViaBarrier(inst, mem, dst, loadObj->GetVolatile());
    } else {
        if (loadObj->GetVolatile()) {
            enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), mem);
        } else {
            enc->GetEncoder()->EncodeLdr(dst, IsTypeSigned(type), mem);
        }
    }
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
}

void EncodeVisitor::VisitResolveObjectField(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto resolver = inst->CastToResolveObjectField();
    if (resolver->GetNeedBarrier()) {
        // Inserts barriers for GC
    }
    auto type = inst->GetType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    auto typeId = resolver->GetTypeId();
    auto method = resolver->GetMethod();
    if (graph->IsAotMode()) {
        auto typeIdImm = enc->GetCodegen()->GetTypeIdImm(inst, typeId);
        enc->GetCodegen()->CallRuntimeWithMethod(inst, method, EntrypointId::GET_FIELD_OFFSET, dst, typeIdImm);
    } else {
        auto skind = UnresolvedTypesInterface::SlotKind::FIELD;
        ASSERT(graph->GetRuntime()->GetUnresolvedTypes() != nullptr);
        auto fieldOffsetAddr = graph->GetRuntime()->GetUnresolvedTypes()->GetTableSlot(method, typeId, skind);
        ScopedTmpReg tmpReg(enc->GetEncoder());
        // load field offset and if it's 0 then call runtime EntrypointId::GET_FIELD_OFFSET
        enc->GetEncoder()->EncodeMov(tmpReg, Imm(fieldOffsetAddr));
        enc->GetEncoder()->EncodeLdr(tmpReg, false, MemRef(tmpReg));
        auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, EntrypointId::GET_FIELD_OFFSET);
        slowPath->SetUnresolvedType(method, typeId);
        slowPath->SetDstReg(tmpReg);
        slowPath->SetSlotAddr(fieldOffsetAddr);
        enc->GetEncoder()->EncodeJump(slowPath->GetLabel(), tmpReg, Condition::EQ);
        slowPath->BindBackLabel(enc->GetEncoder());
        enc->GetEncoder()->EncodeMov(dst, tmpReg);
    }
}

void EncodeVisitor::VisitLoadResolvedObjectField([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto load = inst->CastToLoadResolvedObjectField();
    if (load->GetNeedBarrier()) {
        // Inserts barriers for GC
    }
    auto type = inst->GetType();
    auto obj = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto ofs = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::UINT32);     // field offset
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                  // load value
    if (load->GetNeedBarrier()) {
        auto mem = MemRef(obj, ofs, 0);
        enc->GetCodegen()->CreateReadViaBarrier(inst, mem, dst, true);
    } else {
        ScopedTmpReg tmpReg(enc->GetEncoder());
        enc->GetEncoder()->EncodeAdd(tmpReg, obj, ofs);
        enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), MemRef(tmpReg));
    }
}

void EncodeVisitor::VisitLoad(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto loadByOffset = inst->CastToLoad();
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0U), DataType::POINTER);  // pointer
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1U), DataType::UINT32);   // offset
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                  // load value
    auto mem = MemRef(src0, src1, loadByOffset->GetScale());

    if (loadByOffset->GetNeedBarrier()) {
        enc->GetCodegen()->CreateReadViaBarrier(inst, mem, dst, loadByOffset->GetVolatile());
    } else if (loadByOffset->GetVolatile()) {
        enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), mem);
    } else {
        enc->GetEncoder()->EncodeLdr(dst, IsTypeSigned(type), mem);
    }
}

void EncodeVisitor::VisitLoadI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto loadByOffset = inst->CastToLoadI();
    auto type = inst->GetType();
    auto base = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0U), DataType::POINTER);  // pointer
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                  // load value
    auto mem = MemRef(base, loadByOffset->GetImm());

    if (loadByOffset->GetNeedBarrier()) {
        enc->GetCodegen()->CreateReadViaBarrier(inst, mem, dst, loadByOffset->GetVolatile());
    } else if (loadByOffset->GetVolatile()) {
        enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), MemRef(base, loadByOffset->GetImm()));
    } else {
        enc->GetEncoder()->EncodeLdr(dst, IsTypeSigned(type), MemRef(base, loadByOffset->GetImm()));
    }
}

void EncodeVisitor::VisitStoreI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto storeInst = inst->CastToStoreI();
    auto base = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0U), DataType::POINTER);
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1U), inst->GetType());
    auto mem = MemRef(base, storeInst->GetImm());
    if (inst->CastToStoreI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem, MakeMask(base.GetId(), src.GetId()));
    }
    if (storeInst->GetVolatile()) {
        enc->GetEncoder()->EncodeStrRelease(src, mem);
    } else {
        enc->GetEncoder()->EncodeStr(src, mem);
    }
    if (inst->CastToStoreI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, mem, src, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitStoreObject(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto storeObj = inst->CastToStoreObject();
    auto codegen = enc->GetCodegen();
    auto src0 = codegen->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto src1 = codegen->ConvertRegister(inst->GetSrcReg(1), inst->GetType());      // store value
    auto graph = enc->cg_->GetGraph();
    auto field = storeObj->GetObjField();
    size_t offset = GetObjectOffset(graph, storeObj->GetObjectType(), field, storeObj->GetTypeId());
    if (!enc->GetCodegen()->OffsetFitReferenceTypeSize(offset)) {
        // such code should not be executed
        enc->GetEncoder()->EncodeAbort();
        return;
    }
    auto mem = MemRef(src0, offset);
    auto encoder = enc->GetEncoder();
    if (inst->CastToStoreObject()->GetNeedBarrier()) {
        if (storeObj->GetObjectType() == ObjectType::MEM_DYN_CLASS) {
            codegen->CreatePreWRB<true>(inst, mem, MakeMask(src0.GetId(), src1.GetId()));
        } else {
            codegen->CreatePreWRB(inst, mem, MakeMask(src0.GetId(), src1.GetId()));
        }
    }
    auto prevOffset = encoder->GetCursorOffset();
    if (storeObj->GetVolatile()) {
        encoder->EncodeStrRelease(src1, mem);
    } else {
        encoder->EncodeStr(src1, mem);
    }
    codegen->TryInsertImplicitNullCheck(inst, prevOffset);
    if (inst->CastToStoreObject()->GetNeedBarrier()) {
        ScopedTmpRegLazy tmp(encoder);
        if (storeObj->GetObjectType() == ObjectType::MEM_DYN_CLASS) {
            tmp.AcquireIfInvalid();
            encoder->EncodeLdr(tmp, false, MemRef(src1, graph->GetRuntime()->GetManagedClassOffset(graph->GetArch())));
            src1 = tmp;
        }
        codegen->CreatePostWRB(inst, mem, src1, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitStoreResolvedObjectField(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto store = inst->CastToStoreResolvedObjectField();
    auto obj = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // object
    auto val = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), inst->GetType());      // store value
    auto ofs = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(2), DataType::UINT32);     // field offset
    auto mem = MemRef(obj, ofs, 0);
    if (store->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem);
    }
    // Unknown store, assume it can be volatile
    enc->GetEncoder()->EncodeStrRelease(val, mem);
    if (store->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, mem, val, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitStore(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto storeByOffset = inst->CastToStore();
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0U), DataType::POINTER);  // pointer
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1U), DataType::UINT32);   // offset
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(2U), type);               // store value
    auto mem = MemRef(src0, src1, storeByOffset->GetScale());
    if (inst->CastToStore()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem, MakeMask(src0.GetId(), src2.GetId()));
    }
    if (storeByOffset->GetVolatile()) {
        enc->GetEncoder()->EncodeStrRelease(src2, mem);
    } else {
        enc->GetEncoder()->EncodeStr(src2, mem);
    }
    if (inst->CastToStore()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, mem, src2, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitInitClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto classId = inst->CastToInitClass()->GetTypeId();
    auto encoder = enc->GetEncoder();
    ASSERT(inst->IsRuntimeCall());
    if (graph->IsAotMode()) {
        ScopedTmpReg tmpReg(encoder);
        ScopedTmpReg classReg(encoder);
        auto aotData = graph->GetAotData();
        intptr_t offset = aotData->GetClassSlotOffset(encoder->GetCursorOffset(), classId, true,
                                                      enc->GetCodegen()->GetAOTBinaryFileSnapshotIndexForInst(inst));
        encoder->MakeLoadAotTableAddr(offset, tmpReg, classReg);
        auto label = encoder->CreateLabel();
        encoder->EncodeJump(label, classReg, Condition::NE);
        // PLT Class Init Resolver has special calling convention:
        // First encoder temporary (tmp_reg) works as parameter and return value (which is unnecessary here)
        CHECK_EQ(tmpReg.GetReg().GetId(), encoder->GetTarget().GetTempRegsMask().GetMinRegister());
        enc->GetCodegen()->CreateJumpToClassResolverPltShared(inst, tmpReg.GetReg(), EntrypointId::CLASS_INIT_RESOLVER);
        encoder->BindLabel(label);
    } else {  // JIT mode
        auto klass = reinterpret_cast<uintptr_t>(inst->CastToInitClass()->GetClass());
        ASSERT(klass != 0);
        if (!runtime->IsClassInitialized(klass)) {
            auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::INITIALIZE_CLASS);
            auto stateOffset = runtime->GetClassStateOffset(enc->GetArch());
            int64_t initValue = runtime->GetClassInitializedValue();
            ScopedTmpReg tmpReg(encoder);
            encoder->EncodeMov(tmpReg, Imm(klass + stateOffset));
            auto tmpI8 = enc->GetCodegen()->ConvertRegister(tmpReg.GetReg().GetId(), DataType::INT8);
            encoder->EncodeLdr(tmpI8, false, MemRef(tmpReg));
            encoder->EncodeJump(slowPath->GetLabel(), tmpI8, Imm(initValue), Condition::NE);
            slowPath->BindBackLabel(encoder);
        }
    }
}

void EncodeVisitor::VisitLoadClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto loadClass = inst->CastToLoadClass();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto graph = enc->cg_->GetGraph();
    auto typeId = loadClass->GetTypeId();
    ASSERT(inst->IsRuntimeCall());
    if (graph->IsAotMode()) {
        auto methodClassId = graph->GetRuntime()->GetClassIdForMethod(graph->GetMethod());
        if (methodClassId == typeId) {
            auto dstPtr = dst.As(Codegen::ConvertDataType(DataType::POINTER, graph->GetArch()));
            enc->GetCodegen()->LoadMethod(dstPtr);
            auto mem = MemRef(dstPtr, graph->GetRuntime()->GetClassOffset(graph->GetArch()));
            encoder->EncodeLdr(dst.As(Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch())), false, mem);
            return;
        }
        ScopedTmpReg tmpReg(encoder);
        enc->GetCodegen()->CreateLoadClassFromPLT(inst, tmpReg, dst, typeId);
    } else {  // JIT mode
        auto klass = loadClass->GetClass();
        if (klass == nullptr) {
            FillLoadClassUnresolved(visitor, inst);
        } else {
            encoder->EncodeMov(dst, Imm(reinterpret_cast<uintptr_t>(klass)));
        }
    }
}

void EncodeVisitor::FillLoadClassUnresolved(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto loadClass = inst->CastToLoadClass();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto graph = enc->cg_->GetGraph();
    auto typeId = loadClass->GetTypeId();
    auto method = loadClass->GetMethod();
    auto utypes = graph->GetRuntime()->GetUnresolvedTypes();
    ASSERT(utypes != nullptr);
    auto klassAddr = utypes->GetTableSlot(method, typeId, UnresolvedTypesInterface::SlotKind::CLASS);
    Reg dstPtr(dst.GetId(), enc->GetCodegen()->GetPtrRegType());
    encoder->EncodeMov(dstPtr, Imm(klassAddr));
    encoder->EncodeLdr(dst, false, MemRef(dstPtr));
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, EntrypointId::RESOLVE_CLASS);
    slowPath->SetUnresolvedType(method, typeId);
    slowPath->SetDstReg(dst);
    slowPath->SetSlotAddr(klassAddr);
    encoder->EncodeJump(slowPath->GetLabel(), dst, Condition::EQ);
    slowPath->BindBackLabel(encoder);
}

void EncodeVisitor::VisitGetGlobalVarAddress(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto id = inst->CastToGetGlobalVarAddress()->GetTypeId();
    auto encoder = enc->GetEncoder();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());  // load value
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::ANY);
    auto entrypointId = runtime->GetGlobalVarEntrypointId();
    ASSERT(inst->IsRuntimeCall());
    if (graph->IsAotMode()) {
        ScopedTmpReg addr(encoder);
        auto aotData = graph->GetAotData();
        Reg constPool = src;
        ScopedTmpRegLazy tmpReg(encoder);
        if (dst.GetId() == src.GetId()) {
            tmpReg.AcquireIfInvalid();
            constPool = tmpReg.GetReg().As(src.GetType());
            encoder->EncodeMov(constPool, src);
        }
        intptr_t offset = aotData->GetCommonSlotOffset(encoder->GetCursorOffset(), id);
        encoder->MakeLoadAotTableAddr(offset, addr, dst);
        auto label = encoder->CreateLabel();
        encoder->EncodeJump(label, dst, Condition::NE);
        enc->GetCodegen()->CallRuntime(inst, entrypointId, dst, RegMask::GetZeroMask(), constPool, TypedImm(id));
        encoder->EncodeStr(dst, MemRef(addr));
        encoder->BindLabel(label);
    } else {
        auto address = inst->CastToGetGlobalVarAddress()->GetAddress();
        if (address == 0) {
            address = runtime->GetGlobalVarAddress(graph->GetMethod(), id);
        }
        if (address == 0) {
            enc->GetCodegen()->CallRuntime(inst, entrypointId, dst, RegMask::GetZeroMask(), src, TypedImm(id));
        } else {
            encoder->EncodeMov(dst, Imm(address));
        }
    }
}

void EncodeVisitor::VisitLoadRuntimeClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());  // load value
    size_t classAddrOffset = codegen->GetGraph()->GetRuntime()->GetTlsPromiseClassPointerOffset(codegen->GetArch());
    auto mem = MemRef(codegen->ThreadReg(), classAddrOffset);
    enc->GetEncoder()->EncodeLdr(dst, false, mem);
}

void EncodeVisitor::EncodeLoadAndInitClassInAot(EncodeVisitor *enc, Encoder *encoder, Inst *inst, uint32_t classId,
                                                Reg dst)
{
    auto graph = enc->cg_->GetGraph();
    ScopedTmpReg tmpReg(encoder);
    auto aotData = graph->GetAotData();
    intptr_t offset = aotData->GetClassSlotOffset(encoder->GetCursorOffset(), classId, true,
                                                  enc->GetCodegen()->GetAOTBinaryFileSnapshotIndexForInst(inst));
    encoder->MakeLoadAotTableAddr(offset, tmpReg, dst);
    auto label = encoder->CreateLabel();
    encoder->EncodeJump(label, dst, Condition::NE);
    // PLT Class Init Resolver has special calling convention:
    // First encoder temporary (tmp_reg) works as parameter and return value
    CHECK_EQ(tmpReg.GetReg().GetId(), encoder->GetTarget().GetTempRegsMask().GetMinRegister());
    enc->GetCodegen()->CreateJumpToClassResolverPltShared(inst, tmpReg.GetReg(), EntrypointId::CLASS_INIT_RESOLVER);
    encoder->EncodeMov(dst, tmpReg);
    encoder->BindLabel(label);
}

void EncodeVisitor::VisitLoadAndInitClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto classId = inst->CastToLoadAndInitClass()->GetTypeId();
    auto encoder = enc->GetEncoder();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());  // load value
    ASSERT(inst->IsRuntimeCall());
    if (graph->IsAotMode()) {
        auto methodClassId = runtime->GetClassIdForMethod(graph->GetMethod());
        if (methodClassId == classId) {
            auto dstPtr = dst.As(Codegen::ConvertDataType(DataType::POINTER, graph->GetArch()));
            enc->GetCodegen()->LoadMethod(dstPtr);
            auto mem = MemRef(dstPtr, graph->GetRuntime()->GetClassOffset(graph->GetArch()));
            encoder->EncodeLdr(dst.As(Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch())), false, mem);
            return;
        }
        EncodeLoadAndInitClassInAot(enc, encoder, inst, classId, dst);
    } else {  // JIT mode
        auto klass = reinterpret_cast<uintptr_t>(inst->CastToLoadAndInitClass()->GetClass());
        encoder->EncodeMov(dst, Imm(klass));
        if (runtime->IsClassInitialized(klass)) {
            return;
        }
        auto methodClass = runtime->GetClass(graph->GetMethod());
        if (methodClass == inst->CastToLoadAndInitClass()->GetClass()) {
            return;
        }
        auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::INITIALIZE_CLASS);
        auto stateOffset = runtime->GetClassStateOffset(enc->GetArch());
        int64_t initValue = runtime->GetClassInitializedValue();
        ScopedTmpReg stateReg(encoder, INT8_TYPE);
        encoder->EncodeLdr(stateReg, false, MemRef(dst, stateOffset));
        encoder->EncodeJump(slowPath->GetLabel(), stateReg, Imm(initValue), Condition::NE);
        slowPath->BindBackLabel(encoder);
    }
}

void EncodeVisitor::VisitUnresolvedLoadAndInitClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto classId = inst->CastToUnresolvedLoadAndInitClass()->GetTypeId();
    auto encoder = enc->GetEncoder();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());  // load value
    ASSERT(inst->IsRuntimeCall());
    if (graph->IsAotMode()) {
        EncodeLoadAndInitClassInAot(enc, encoder, inst, classId, dst);
    } else {  // JIT mode
        auto method = inst->CastToUnresolvedLoadAndInitClass()->GetMethod();
        auto utypes = graph->GetRuntime()->GetUnresolvedTypes();
        ASSERT(utypes != nullptr);
        auto klassAddr = utypes->GetTableSlot(method, classId, UnresolvedTypesInterface::SlotKind::CLASS);
        Reg dstPtr(dst.GetId(), enc->GetCodegen()->GetPtrRegType());
        encoder->EncodeMov(dstPtr, Imm(klassAddr));
        encoder->EncodeLdr(dst, false, MemRef(dstPtr));
        static constexpr auto ENTRYPOINT_ID = RuntimeInterface::EntrypointId::INITIALIZE_CLASS_BY_ID;
        auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, ENTRYPOINT_ID);
        slowPath->SetUnresolvedType(method, classId);
        slowPath->SetDstReg(dst);
        slowPath->SetSlotAddr(klassAddr);
        encoder->EncodeJump(slowPath->GetLabel(), dst, Condition::EQ);
        slowPath->BindBackLabel(encoder);
    }
}

void EncodeVisitor::VisitLoadStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto loadStatic = inst->CastToLoadStatic();

    auto type = inst->GetType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);                   // load value
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // class
    auto graph = enc->cg_->GetGraph();
    auto field = loadStatic->GetObjField();
    auto offset = graph->GetRuntime()->GetFieldOffset(field);
    auto mem = MemRef(src0, offset);
    if (loadStatic->GetNeedBarrier()) {
        enc->GetCodegen()->CreateReadViaBarrier(inst, mem, dst, loadStatic->GetVolatile());
    } else {
        if (loadStatic->GetVolatile()) {
            enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), mem);
        } else {
            enc->GetEncoder()->EncodeLdr(dst, IsTypeSigned(type), mem);
        }
    }
}

void EncodeVisitor::VisitResolveObjectFieldStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto resolver = inst->CastToResolveObjectFieldStatic();
    ASSERT(resolver->GetType() == DataType::REFERENCE);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto typeId = resolver->GetTypeId();
    auto method = resolver->GetMethod();
    EntrypointId entrypoint = EntrypointId::GET_UNKNOWN_STATIC_FIELD_MEMORY_ADDRESS;  // REFERENCE
    UnresolvedTypesInterface::SlotKind slotKind = UnresolvedTypesInterface::SlotKind::FIELD;
    if (graph->IsAotMode()) {
        auto typeIdImm = enc->GetCodegen()->GetTypeIdImm(inst, typeId);
        enc->GetCodegen()->CallRuntimeWithMethod(inst, method, entrypoint, dst, TypedImm(typeIdImm), TypedImm(0));
    } else {
        ScopedTmpReg tmpReg(enc->GetEncoder());
        ASSERT(graph->GetRuntime()->GetUnresolvedTypes() != nullptr);
        auto fieldAddr = graph->GetRuntime()->GetUnresolvedTypes()->GetTableSlot(method, typeId, slotKind);
        enc->GetEncoder()->EncodeMov(tmpReg, Imm(fieldAddr));
        enc->GetEncoder()->EncodeLdr(tmpReg, false, MemRef(tmpReg));
        auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathUnresolved>(inst, entrypoint);
        slowPath->SetUnresolvedType(method, typeId);
        slowPath->SetDstReg(tmpReg);
        slowPath->SetSlotAddr(fieldAddr);
        enc->GetEncoder()->EncodeJump(slowPath->GetLabel(), tmpReg, Condition::EQ);
        slowPath->BindBackLabel(enc->GetEncoder());
        enc->GetEncoder()->EncodeMov(dst, tmpReg);
    }
}

void EncodeVisitor::VisitLoadResolvedObjectFieldStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto load = inst->CastToLoadResolvedObjectFieldStatic();

    auto type = inst->GetType();
    auto fieldAddr = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    // Unknown load, assume it can be volatile
    if (load->GetNeedBarrier()) {
        enc->GetCodegen()->CreateReadViaBarrier(inst, MemRef(fieldAddr), dst, true);
    } else {
        enc->GetEncoder()->EncodeLdrAcquire(dst, IsTypeSigned(type), MemRef(fieldAddr));
    }
}

void EncodeVisitor::VisitStoreStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto storeStatic = inst->CastToStoreStatic();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // class
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), inst->GetType());      // store value
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto field = storeStatic->GetObjField();
    auto offset = runtime->GetFieldOffset(field);
    auto mem = MemRef(src0, offset);
    if (inst->CastToStoreStatic()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem, MakeMask(src1.GetId()));
    }
    if (storeStatic->GetVolatile()) {
        enc->GetEncoder()->EncodeStrRelease(src1, mem);
    } else {
        enc->GetEncoder()->EncodeStr(src1, mem);
    }
    if (!inst->CastToStoreStatic()->GetNeedBarrier()) {
        return;
    }
    enc->GetCodegen()->CreatePostWRB(inst, mem, src1, INVALID_REGISTER);
}

void EncodeVisitor::VisitLoadObjectDynamic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto *codegen = enc->GetCodegen();
    codegen->CreateLoadObjectDynamic(inst);
}

void EncodeVisitor::VisitStoreObjectDynamic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto *codegen = enc->GetCodegen();
    codegen->CreateStoreObjectDynamic(inst);
}

void EncodeVisitor::VisitUnresolvedStoreStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto storeStatic = inst->CastToUnresolvedStoreStatic();
    ASSERT(storeStatic->GetType() == DataType::REFERENCE);
    ASSERT(storeStatic->GetNeedBarrier());
    auto typeId = storeStatic->GetTypeId();
    auto value = enc->GetCodegen()->ConvertRegister(storeStatic->GetSrcReg(0), storeStatic->GetType());
    auto entrypoint = RuntimeInterface::EntrypointId::UNRESOLVED_STORE_STATIC_BARRIERED;
    auto method = storeStatic->GetMethod();
    ASSERT(method != nullptr);
    auto typeIdImm = enc->GetCodegen()->GetTypeIdImm(inst, typeId);
    enc->GetCodegen()->CallRuntimeWithMethod(storeStatic, method, entrypoint, Reg(), typeIdImm, value);
}

void EncodeVisitor::VisitStoreResolvedObjectFieldStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto store = inst->CastToStoreResolvedObjectFieldStatic();
    ASSERT(store->GetType() != DataType::REFERENCE);
    ASSERT(!store->GetNeedBarrier());
    auto val = enc->GetCodegen()->ConvertRegister(store->GetSrcReg(1), store->GetType());
    auto reg = enc->GetCodegen()->ConvertRegister(store->GetSrcReg(0), DataType::REFERENCE);
    // Non-barriered case. Unknown store, assume it can be volatile
    enc->GetEncoder()->EncodeStrRelease(val, MemRef(reg));
}

void EncodeVisitor::VisitNewObject(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    // NOTE(msherstennikov): use irtoced entrypoint once spill-fills will be supported for entrypoints mode.
    if (enc->cg_->GetArch() == Arch::AARCH32) {
        enc->GetCodegen()->CreateNewObjCallOld(inst->CastToNewObject());
    } else {
        enc->GetCodegen()->CreateNewObjCall(inst->CastToNewObject());
    }
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        enc->GetEncoder()->EncodeMemoryBarrier(memory_order::RELEASE);
    }
}

void EncodeVisitor::VisitUnresolvedLoadType(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto codegen = enc->GetCodegen();
    auto loadType = inst->CastToUnresolvedLoadType();
    if (loadType->GetNeedBarrier()) {
        // Consider inserting barriers for GC
    }
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto graph = enc->cg_->GetGraph();
    auto typeId = loadType->GetTypeId();
    auto runtime = graph->GetRuntime();
    auto method = loadType->GetMethod();
    if (graph->IsAotMode()) {
        ScopedTmpReg tmpReg(encoder);
        // Load pointer to klass from PLT
        codegen->CreateLoadClassFromPLT(inst, tmpReg, dst, typeId);
        // Finally load Object
        encoder->EncodeLdr(dst, false, MemRef(dst, runtime->GetManagedClassOffset(enc->GetArch())));
    } else {
        auto utypes = runtime->GetUnresolvedTypes();
        ASSERT(utypes != nullptr);
        auto clsAddr = utypes->GetTableSlot(method, typeId, UnresolvedTypesInterface::SlotKind::MANAGED_CLASS);
        Reg dstPtr(dst.GetId(), codegen->GetPtrRegType());
        encoder->EncodeMov(dstPtr, Imm(clsAddr));
        encoder->EncodeLdr(dst, false, MemRef(dstPtr));
        auto slowPath = codegen->CreateSlowPath<SlowPathUnresolved>(inst, EntrypointId::RESOLVE_CLASS_OBJECT);
        slowPath->SetUnresolvedType(method, typeId);
        slowPath->SetDstReg(dst);
        slowPath->SetSlotAddr(clsAddr);
        encoder->EncodeJump(slowPath->GetLabel(), dst, Condition::EQ);
        slowPath->BindBackLabel(encoder);
    }
}

void EncodeVisitor::VisitLoadType(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto loadType = inst->CastToLoadType();
    if (loadType->GetNeedBarrier()) {
        // Consider inserting barriers for GC
    }
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto graph = enc->cg_->GetGraph();
    auto typeId = loadType->GetTypeId();
    auto runtime = graph->GetRuntime();
    auto method = loadType->GetMethod();
    if (graph->IsAotMode()) {
        auto methodClassId = runtime->GetClassIdForMethod(graph->GetMethod());
        if (methodClassId == typeId) {
            auto dstPtr = dst.As(Codegen::ConvertDataType(DataType::POINTER, graph->GetArch()));
            enc->GetCodegen()->LoadMethod(dstPtr);
            auto mem = MemRef(dstPtr, graph->GetRuntime()->GetClassOffset(graph->GetArch()));
            encoder->EncodeLdr(dst.As(Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch())), false, mem);
        } else {
            ScopedTmpReg tmpReg(encoder);
            // Load pointer to klass from PLT
            enc->GetCodegen()->CreateLoadClassFromPLT(inst, tmpReg, dst, typeId);
        }
        // Finally load ManagedClass object
        encoder->EncodeLdr(dst, false, MemRef(dst, runtime->GetManagedClassOffset(enc->GetArch())));
    } else {  // JIT mode
        auto klass = reinterpret_cast<uintptr_t>(runtime->ResolveType(method, typeId));
        auto managedKlass = runtime->GetManagedType(klass);
        encoder->EncodeMov(dst, Imm(managedKlass));
    }
}

void EncodeVisitor::FillUnresolvedClass(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    auto classReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    auto eid = inst->CanDeoptimize() ? RuntimeInterface::EntrypointId::CHECK_CAST_DEOPTIMIZE
                                     : RuntimeInterface::EntrypointId::CHECK_CAST;
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, eid);
    encoder->EncodeJump(slowPath->GetLabel(), classReg, Condition::EQ);
    slowPath->CreateBackLabel(encoder);
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    encoder->EncodeJump(slowPath->GetBackLabel(), src, Condition::EQ);
    ScopedTmpReg tmpReg(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    enc->GetCodegen()->LoadClassFromObject(tmpReg, src);
    encoder->EncodeJump(slowPath->GetLabel(), tmpReg, classReg, Condition::NE);
    slowPath->BindBackLabel(encoder);
}

void EncodeVisitor::FillObjectClass(GraphVisitor *visitor, Reg tmpReg, LabelHolder::LabelId throwLabel)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto encoder = enc->GetEncoder();
    Reg typeReg(tmpReg.GetId(), INT8_TYPE);
    // Load type class
    encoder->EncodeLdr(typeReg, false, MemRef(tmpReg, runtime->GetClassTypeOffset(enc->GetArch())));
    // Jump to EH if type not reference
    encoder->EncodeJump(throwLabel, typeReg, Imm(runtime->GetReferenceTypeMask()), Condition::NE);
}

/* The CheckCast class should be a subclass of input class:
    ......................
    bool Class::IsSubClassOf(const Class *klass) const {
        const Class *current = this;
        do {
            if (current == klass) {
                return true;
            }
            current = current->GetBase();
        } while (current != nullptr);
        return false;
    }
*/

void EncodeVisitor::FillOtherClass(GraphVisitor *visitor, Inst *inst, Reg tmpReg, LabelHolder::LabelId throwLabel)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    auto loopLabel = encoder->CreateLabel();
    // First compare `current == klass` we make before switch
    encoder->BindLabel(loopLabel);
    // Load base klass
    encoder->EncodeLdr(tmpReg, false, MemRef(tmpReg, graph->GetRuntime()->GetClassBaseOffset(enc->GetArch())));
    encoder->EncodeJump(throwLabel, tmpReg, Condition::EQ);
    auto classReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    encoder->EncodeJump(loopLabel, tmpReg, classReg, Condition::NE);
}

void EncodeVisitor::FillArrayObjectClass(GraphVisitor *visitor, Reg tmpReg, LabelHolder::LabelId throwLabel)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto encoder = enc->GetEncoder();
    Reg typeReg(tmpReg.GetId(), INT8_TYPE);
    // Load Component class
    encoder->EncodeLdr(tmpReg, false, MemRef(tmpReg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Jump to EH if src is not array class
    encoder->EncodeJump(throwLabel, tmpReg, Condition::EQ);
    // Load type of the component class
    encoder->EncodeLdr(typeReg, false, MemRef(tmpReg, runtime->GetClassTypeOffset(enc->GetArch())));
    // Jump to EH if type not reference
    encoder->EncodeJump(throwLabel, typeReg, Imm(runtime->GetReferenceTypeMask()), Condition::NE);
}

void EncodeVisitor::FillArrayClass(GraphVisitor *visitor, Inst *inst, Reg tmpReg, LabelHolder::LabelId throwLabel)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto encoder = enc->GetEncoder();
    auto eid = inst->CanDeoptimize() ? RuntimeInterface::EntrypointId::CHECK_CAST_DEOPTIMIZE
                                     : RuntimeInterface::EntrypointId::CHECK_CAST;
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, eid);
    // Load Component type of Input
    encoder->EncodeLdr(tmpReg, false, MemRef(tmpReg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Check that src is array class
    encoder->EncodeJump(throwLabel, tmpReg, Condition::EQ);
    // Load Component type of the instance
    auto classReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    ScopedTmpReg tmpReg1(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    encoder->EncodeLdr(tmpReg1, false, MemRef(classReg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Compare component types
    encoder->EncodeJump(slowPath->GetLabel(), tmpReg, tmpReg1, Condition::NE);
    slowPath->BindBackLabel(encoder);
}

void EncodeVisitor::FillInterfaceClass(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto codegen = enc->GetCodegen();
    if (inst->CanDeoptimize() || codegen->GetArch() == Arch::AARCH32) {
        auto eid = inst->CanDeoptimize() ? RuntimeInterface::EntrypointId::CHECK_CAST_DEOPTIMIZE
                                         : RuntimeInterface::EntrypointId::CHECK_CAST;
        auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, eid);
        encoder->EncodeJump(slowPath->GetLabel());
        slowPath->BindBackLabel(encoder);
    } else {
        codegen->CreateCheckCastInterfaceCall(inst);
    }
}

// Note(#27132): implement FastPath
void EncodeVisitor::FillUnionClass(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto eid = inst->CanDeoptimize() ? RuntimeInterface::EntrypointId::CHECK_CAST_DEOPTIMIZE
                                     : RuntimeInterface::EntrypointId::CHECK_CAST;
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, eid);
    encoder->EncodeJump(slowPath->GetLabel());
    slowPath->BindBackLabel(encoder);
}

// CC-OFFNXT(huge_method) solid logic
void EncodeVisitor::FillCheckCast(GraphVisitor *visitor, Inst *inst, Reg src, LabelHolder::LabelId endLabel,
                                  compiler::ClassType klassType)
{
    if (klassType == ClassType::INTERFACE_CLASS) {
        FillInterfaceClass(visitor, inst);
        return;
    }
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();

    if (klassType == ClassType::UNION_CLASS) {
        FillUnionClass(visitor, inst);
        return;
    }

    // class_reg - CheckCast class
    // tmp_reg - input class
    auto classReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    ScopedTmpReg tmpReg(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    enc->GetCodegen()->LoadClassFromObject(tmpReg, src);
    // There is no exception if the classes are equal
    encoder->EncodeJump(endLabel, classReg, tmpReg, Condition::EQ);
    LabelHolder::LabelId throwLabel;
    if (inst->CanDeoptimize()) {
        throwLabel =
            enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, DeoptimizeType::CHECK_CAST)->GetLabel();
    } else {
        auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathCheckCast>(inst, EntrypointId::CLASS_CAST_EXCEPTION);
        slowPath->SetClassReg(classReg);
        throwLabel = slowPath->GetLabel();
    }
    switch (klassType) {
        // The input class should be not primitive type
        case ClassType::OBJECT_CLASS: {
            FillObjectClass(visitor, tmpReg, throwLabel);
            break;
        }
        case ClassType::OTHER_CLASS: {
            FillOtherClass(visitor, inst, tmpReg, throwLabel);
            break;
        }
        // The input class should be array class and component type should be not primitive type
        case ClassType::ARRAY_OBJECT_CLASS: {
            FillArrayObjectClass(visitor, tmpReg, throwLabel);
            break;
        }
        // Check that components types are equals, else call slow path
        case ClassType::ARRAY_CLASS: {
            FillArrayClass(visitor, inst, tmpReg, throwLabel);
            break;
        }
        case ClassType::FINAL_CLASS: {
            EVENT_CODEGEN_SIMPLIFICATION(events::CodegenSimplificationInst::CHECKCAST,
                                         events::CodegenSimplificationReason::FINAL_CLASS);
            encoder->EncodeJump(throwLabel);
            break;
        }
        default: {
            UNREACHABLE();
        }
    }
}

void EncodeVisitor::VisitCheckCast(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto method = inst->CastToCheckCast()->GetMethod();
    auto typeId = inst->CastToCheckCast()->GetTypeId();
    auto encoder = enc->GetEncoder();
    auto klassType = inst->CastToCheckCast()->GetClassType();
    if (klassType == ClassType::UNRESOLVED_CLASS) {
        FillUnresolvedClass(visitor, inst);
        return;
    }
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto endLabel = encoder->CreateLabel();
    if (inst->CastToCheckCast()->GetOmitNullCheck()) {
        EVENT_CODEGEN_SIMPLIFICATION(events::CodegenSimplificationInst::CHECKCAST,
                                     events::CodegenSimplificationReason::SKIP_NULLCHECK);
    } else {
        // Compare with nullptr
        encoder->EncodeJump(endLabel, src, Condition::EQ);
    }
    [[maybe_unused]] auto klass = enc->cg_->GetGraph()->GetRuntime()->GetClass(method, typeId);
    ASSERT(klass != nullptr);
    FillCheckCast(visitor, inst, src, endLabel, klassType);
    encoder->BindLabel(endLabel);
}

void EncodeVisitor::FillIsInstanceUnresolved(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto classReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::IS_INSTANCE);
    encoder->EncodeJump(slowPath->GetLabel(), classReg, Condition::EQ);
    slowPath->CreateBackLabel(encoder);
    auto endLabel = slowPath->GetBackLabel();
    // Compare with nullptr
    auto nextLabel = encoder->CreateLabel();
    encoder->EncodeJump(nextLabel, src, Condition::NE);
    encoder->EncodeMov(dst, Imm(0));
    encoder->EncodeJump(endLabel);
    encoder->BindLabel(nextLabel);
    // Get instance class
    ScopedTmpReg tmpReg(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    enc->GetCodegen()->LoadClassFromObject(tmpReg, src);
    // Sets true if the classes are equal
    encoder->EncodeJump(slowPath->GetLabel(), tmpReg, classReg, Condition::NE);
    encoder->EncodeMov(dst, Imm(1));
    slowPath->BindBackLabel(encoder);
}

void EncodeVisitor::FillIsInstanceCaseObject(GraphVisitor *visitor, Inst *inst, Reg tmpReg)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    // ClassType::OBJECT_CLASS
    Reg typeReg(tmpReg.GetId(), INT8_TYPE);
    // Load type class
    encoder->EncodeLdr(typeReg, false, MemRef(tmpReg, runtime->GetClassTypeOffset(enc->GetArch())));
    ScopedTmpReg typeMaskReg(encoder, INT8_TYPE);
    encoder->EncodeMov(typeMaskReg, Imm(runtime->GetReferenceTypeMask()));
    encoder->EncodeCompare(dst, typeMaskReg, typeReg, Condition::EQ);
}

/* Sets true if IsInstance class is a subclass of input class:
    ......................
    bool Class::IsSubClassOf(const Class *klass) const {
        const Class *current = this;
        do {
            if (current == klass) {
                return true;
            }
            current = current->GetBase();
        } while (current != nullptr);
        return false;
    }
*/

void EncodeVisitor::FillIsInstanceCaseOther(GraphVisitor *visitor, Inst *inst, Reg tmpReg,
                                            LabelHolder::LabelId endLabel)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto classReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    // ClassType::OTHER_CLASS
    auto loopLabel = encoder->CreateLabel();
    auto falseLabel = encoder->CreateLabel();
    // First compare `current == klass` we make before switch
    encoder->BindLabel(loopLabel);
    // Load base klass
    encoder->EncodeLdr(tmpReg, false, MemRef(tmpReg, runtime->GetClassBaseOffset(enc->GetArch())));
    encoder->EncodeJump(falseLabel, tmpReg, Condition::EQ);
    encoder->EncodeJump(loopLabel, tmpReg, classReg, Condition::NE);
    // Set true result and jump to exit
    encoder->EncodeMov(dst, Imm(1));
    encoder->EncodeJump(endLabel);
    // Set false result and jump to exit
    encoder->BindLabel(falseLabel);
    encoder->EncodeMov(dst, Imm(0));
}

// Sets true if the Input class is array class and component type is not primitive type
void EncodeVisitor::FillIsInstanceCaseArrayObject(GraphVisitor *visitor, Inst *inst, Reg tmpReg,
                                                  LabelHolder::LabelId endLabel)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    // ClassType::ARRAY_OBJECT_CLASS
    Reg dstRef(dst.GetId(), Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    Reg typeReg(tmpReg.GetId(), INT8_TYPE);
    // Load Component class
    encoder->EncodeLdr(dstRef, false, MemRef(tmpReg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Check that src is array class
    encoder->EncodeJump(endLabel, dstRef, Condition::EQ);
    // Load type of the component class
    encoder->EncodeLdr(typeReg, false, MemRef(dstRef, runtime->GetClassTypeOffset(enc->GetArch())));
    ScopedTmpReg typeMaskReg(encoder, INT8_TYPE);
    encoder->EncodeMov(typeMaskReg, Imm(runtime->GetReferenceTypeMask()));
    encoder->EncodeCompare(dst, typeMaskReg, typeReg, Condition::EQ);
}

// Check that components types are equals, else call slow path
void EncodeVisitor::FillIsInstanceCaseArrayClass(GraphVisitor *visitor, Inst *inst, Reg tmpReg,
                                                 LabelHolder::LabelId endLabel)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto graph = enc->cg_->GetGraph();
    auto runtime = graph->GetRuntime();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    // ClassType::ARRAY_CLASS
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::IS_INSTANCE);
    auto nextLabel1 = encoder->CreateLabel();
    // Load Component type of Input
    encoder->EncodeLdr(tmpReg, false, MemRef(tmpReg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Check that src is array class
    encoder->EncodeJump(nextLabel1, tmpReg, Condition::NE);
    encoder->EncodeMov(dst, Imm(0));
    encoder->EncodeJump(endLabel);
    encoder->BindLabel(nextLabel1);
    // Load Component type of the instance
    auto classReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    ScopedTmpReg tmpReg1(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    encoder->EncodeLdr(tmpReg1, false, MemRef(classReg, runtime->GetClassComponentTypeOffset(enc->GetArch())));
    // Compare component types
    encoder->EncodeJump(slowPath->GetLabel(), tmpReg, tmpReg1, Condition::NE);
    encoder->EncodeMov(dst, Imm(1));
    slowPath->BindBackLabel(encoder);
}

void EncodeVisitor::FillIsInstanceCaseInterface(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    // ClassType::INTERFACE_CLASS
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::IS_INSTANCE);
    encoder->EncodeJump(slowPath->GetLabel());
    slowPath->BindBackLabel(encoder);
}

void EncodeVisitor::FillIsInstance(GraphVisitor *visitor, Inst *inst, Reg tmpReg, LabelHolder::LabelId endLabel)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    if (inst->CastToIsInstance()->GetOmitNullCheck()) {
        EVENT_CODEGEN_SIMPLIFICATION(events::CodegenSimplificationInst::ISINSTANCE,
                                     events::CodegenSimplificationReason::SKIP_NULLCHECK);
    } else {
        // Compare with nullptr
        auto nextLabel = encoder->CreateLabel();
        encoder->EncodeJump(nextLabel, src, Condition::NE);
        encoder->EncodeMov(dst, Imm(0));
        encoder->EncodeJump(endLabel);
        encoder->BindLabel(nextLabel);
    }
    auto classReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::REFERENCE);
    enc->GetCodegen()->LoadClassFromObject(tmpReg, src);
    // Sets true if the classes are equals
    if (inst->CastToIsInstance()->GetClassType() == ClassType::FINAL_CLASS) {
        encoder->EncodeCompare(dst, classReg, tmpReg, Condition::EQ);
    } else if (dst.GetId() != src.GetId() && dst.GetId() != classReg.GetId()) {
        encoder->EncodeCompare(dst, classReg, tmpReg, Condition::EQ);
        encoder->EncodeJump(endLabel, dst, Condition::NE);
    } else {
        auto nextLabel1 = encoder->CreateLabel();
        encoder->EncodeJump(nextLabel1, classReg, tmpReg, Condition::NE);
        encoder->EncodeMov(dst, Imm(1));
        encoder->EncodeJump(endLabel);
        encoder->BindLabel(nextLabel1);
    }
}

void EncodeVisitor::VisitIsInstance(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto graph = enc->cg_->GetGraph();
    auto encoder = enc->GetEncoder();
    auto klassType = inst->CastToIsInstance()->GetClassType();
    if (klassType == ClassType::UNRESOLVED_CLASS || klassType == ClassType::UNION_CLASS) {
        FillIsInstanceUnresolved(visitor, inst);
        return;
    }
    // tmp_reg - input class
    ScopedTmpReg tmpReg(encoder, Codegen::ConvertDataType(DataType::REFERENCE, graph->GetArch()));
    auto endLabel = encoder->CreateLabel();
    FillIsInstance(visitor, inst, tmpReg, endLabel);
    switch (klassType) {
        // Sets true if the Input class is not primitive type
        case ClassType::OBJECT_CLASS: {
            FillIsInstanceCaseObject(visitor, inst, tmpReg);
            break;
        }
        case ClassType::OTHER_CLASS: {
            FillIsInstanceCaseOther(visitor, inst, tmpReg, endLabel);
            break;
        }
        // Sets true if the Input class is array class and component type is not primitive type
        case ClassType::ARRAY_OBJECT_CLASS: {
            FillIsInstanceCaseArrayObject(visitor, inst, tmpReg, endLabel);
            break;
        }
        // Check that components types are equals, else call slow path
        case ClassType::ARRAY_CLASS: {
            FillIsInstanceCaseArrayClass(visitor, inst, tmpReg, endLabel);
            break;
        }
        case ClassType::INTERFACE_CLASS: {
            FillIsInstanceCaseInterface(visitor, inst);
            break;
        }
        case ClassType::FINAL_CLASS: {
            EVENT_CODEGEN_SIMPLIFICATION(events::CodegenSimplificationInst::ISINSTANCE,
                                         events::CodegenSimplificationReason::FINAL_CLASS);
            break;
        }
        default: {
            UNREACHABLE();
        }
    }
    encoder->BindLabel(endLabel);
}

void EncodeVisitor::VisitMonitor(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    if (enc->GetCodegen()->GetArch() == Arch::AARCH32) {
        enc->GetCodegen()->CreateMonitorCallOld(inst->CastToMonitor());
    } else {
        enc->GetCodegen()->CreateMonitorCall(inst->CastToMonitor());
    }
}

void EncodeVisitor::VisitIntrinsic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();
    auto intrinsic = inst->CastToIntrinsic();
    auto id = intrinsic->GetIntrinsicId();
    auto arch = codegen->GetGraph()->GetArch();
    auto runtime = codegen->GetGraph()->GetRuntime();
    if (codegen->GetGraph()->IsStringFlatCheckConstraint() && GetStringFlatCheckArgMask(id) > 0) {
        // Tree strings are enabled and StringFlatCheck is disabled but IRTOC implementation requires flat string, so
        // call CPP implementation
        codegen->CreateCallIntrinsic(intrinsic);
        return;
    }

    if (EncodesBuiltin(runtime, id, arch) || IsIrtocIntrinsic(id)) {
        codegen->CreateBuiltinIntrinsic(intrinsic);
        return;
    }
    codegen->CreateCallIntrinsic(intrinsic);
}

void EncodeVisitor::VisitBoundsCheckI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto lenReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), inst->GetInputType(0));

    ASSERT(inst->GetInput(1).GetInst()->GetOpcode() == Opcode::SaveState ||
           inst->GetInput(1).GetInst()->GetOpcode() == Opcode::SaveStateDeoptimize);
    EntrypointId entrypoint = inst->CastToBoundsCheckI()->IsArray()
                                  ? EntrypointId::ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION
                                  : EntrypointId::STRING_INDEX_OUT_OF_BOUNDS_EXCEPTION;
    LabelHolder::LabelId label;
    if (inst->CanDeoptimize()) {
        label = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, DeoptimizeType::BOUNDS_CHECK)->GetLabel();
    } else {
        label = enc->GetCodegen()->CreateSlowPath<SlowPathCheck>(inst, entrypoint)->GetLabel();
    }
    auto value = inst->CastToBoundsCheckI()->GetImm();
    if (enc->GetEncoder()->CanEncodeImmAddSubCmp(value, WORD_SIZE, false)) {
        enc->GetEncoder()->EncodeJump(label, lenReg, Imm(value), Condition::LS);
    } else {
        ScopedTmpReg tmp(enc->GetEncoder(), lenReg.GetType());
        enc->GetEncoder()->EncodeMov(tmp, Imm(value));
        enc->GetEncoder()->EncodeJump(label, lenReg, tmp, Condition::LS);
    }
}

void EncodeVisitor::VisitStoreArrayI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto arrayReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto type = inst->GetType();
    auto value = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    auto index = inst->CastToStoreArrayI()->GetImm();
    auto offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch()) +
                  (index << DataType::ShiftByType(type, enc->GetCodegen()->GetArch()));
    if (!enc->GetCodegen()->OffsetFitReferenceTypeSize(offset)) {
        // such code should not be executed
        enc->GetEncoder()->EncodeAbort();
        return;
    }
    auto mem = MemRef(arrayReg, offset);
    if (inst->CastToStoreArrayI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem, MakeMask(arrayReg.GetId(), value.GetId()));
    }
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeStr(value, mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
    if (inst->CastToStoreArrayI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, mem, value, INVALID_REGISTER);
    }
}

void EncodeVisitor::VisitLoadArrayI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto instLoadArrayI = inst->CastToLoadArrayI();
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    ASSERT(instLoadArrayI->IsArray() || !runtime->IsCompressedStringsEnabled());

    auto arrayReg = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0));
    uint32_t index = instLoadArrayI->GetImm();
    auto type = inst->GetType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);
    auto dataOffset = instLoadArrayI->IsArray() ? runtime->GetArrayDataOffset(enc->GetArch())
                                                : runtime->GetStringDataOffset(enc->GetArch());
    uint32_t shift = DataType::ShiftByType(type, enc->GetArch());
    auto offset = dataOffset + (index << shift);
    auto mem = MemRef(arrayReg, offset);
    auto encoder = enc->GetEncoder();
    auto arch = enc->GetArch();
    ScopedTmpReg scopedTmp(encoder, Codegen::ConvertDataType(DataType::GetIntTypeForReference(arch), arch));
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    if (instLoadArrayI->GetNeedBarrier()) {
        enc->GetCodegen()->CreateReadViaBarrier(inst, mem, dst, false);
    } else {
        encoder->EncodeLdr(dst, IsTypeSigned(type), mem);
    }
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
}

void EncodeVisitor::VisitLoadCompressedStringCharI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto runtime = enc->cg_->GetGraph()->GetRuntime();
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0));                   // array
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::INT32);  // length
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), type);               // load value
    auto offset = runtime->GetStringDataOffset(enc->GetArch());
    auto encoder = enc->GetEncoder();
    auto arch = encoder->GetArch();
    uint8_t shift = DataType::ShiftByType(type, arch);
    uint32_t index = inst->CastToLoadCompressedStringCharI()->GetImm();
    ASSERT(encoder->CanEncodeCompressedStringCharAt());
    auto mask = runtime->GetStringCompressionMask();
    if (mask != 1) {
        UNREACHABLE();  // mask is hardcoded in JCL, but verify it just in case it's changed
    }
    enc->GetEncoder()->EncodeCompressedStringCharAtI({dst, src0, src1, offset, index, shift});
}

void EncodeVisitor::VisitMultiArray(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();
    auto arrayInst = inst->CastToMultiArray();
    codegen->CreateMultiArrayCall(arrayInst);
    if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
        enc->GetEncoder()->EncodeMemoryBarrier(memory_order::RELEASE);
    }
}

void EncodeVisitor::VisitInitEmptyString(GraphVisitor *visitor, Inst *inst)
{
    auto codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    codegen->CallRuntime(inst, EntrypointId::CREATE_EMPTY_STRING, dst, RegMask::GetZeroMask());
}

void EncodeVisitor::VisitInitString(GraphVisitor *visitor, Inst *inst)
{
    auto cg = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    auto initStr = inst->CastToInitString();
    auto dst = cg->ConvertRegister(initStr->GetDstReg(), initStr->GetType());
    auto ctorArg = cg->ConvertRegister(initStr->GetSrcReg(0), initStr->GetInputType(0));
    if (cg->GetArch() == Arch::AARCH32) {
        auto entryId =
            initStr->IsFromString() ? EntrypointId::CREATE_STRING_FROM_STRING : EntrypointId::CREATE_STRING_FROM_CHARS;
        cg->CallRuntime(initStr, entryId, dst, RegMask::GetZeroMask(), ctorArg);
        return;
    }
    if (initStr->IsFromString()) {
        auto entryId = cg->GetRuntime()->IsCompressedStringsEnabled()
                           ? compiler::RuntimeInterface::EntrypointId::CREATE_STRING_FROM_STRING_TLAB_COMPRESSED
                           : compiler::RuntimeInterface::EntrypointId::CREATE_STRING_FROM_STRING_TLAB;
        cg->CallFastPath(initStr, entryId, dst, RegMask::GetZeroMask(), ctorArg);
    } else {
        auto enc = cg->GetEncoder();
        auto runtime = cg->GetGraph()->GetRuntime();
        auto mem = MemRef(ctorArg, static_cast<int64_t>(runtime->GetArrayLengthOffset(cg->GetArch())));
        ScopedTmpReg lengthReg(enc);
        enc->EncodeLdr(lengthReg, IsTypeSigned(initStr->GetType()), mem);
        cg->CreateStringFromCharArrayTlab(initStr, dst,
                                          Codegen::SRCREGS {cg->GetRegfile()->GetZeroReg(), lengthReg, ctorArg});
    }
}

void EncodeVisitor::VisitResolveStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->EmitResolveStatic(inst->CastToResolveStatic());
}

void EncodeVisitor::VisitCallResolvedStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->EmitCallResolvedStatic(inst->CastToCallResolvedStatic());
}

void EncodeVisitor::VisitCallStatic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->EmitCallStatic(inst->CastToCallStatic());
}

void EncodeVisitor::VisitCallVirtual(GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->EmitCallVirtual(inst->CastToCallVirtual());
}

void EncodeVisitor::VisitResolveVirtual(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->EmitResolveVirtual(inst->CastToResolveVirtual());
}

void EncodeVisitor::VisitCallResolvedVirtual(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->EmitCallResolvedVirtual(inst->CastToCallResolvedVirtual());
}

void EncodeVisitor::VisitCallDynamic(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->EmitCallDynamic(inst->CastToCallDynamic());
}

void EncodeVisitor::VisitCallNative(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->EmitCallNative(inst->CastToCallNative());
}

void EncodeVisitor::VisitWrapObjectNative(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->WrapObjectNative(inst->CastToWrapObjectNative());
}

void EncodeVisitor::VisitLoadConstantPool(GraphVisitor *visitor, Inst *inst)
{
    if (TryLoadConstantPoolGen(inst, static_cast<EncodeVisitor *>(visitor))) {
        return;
    }
    UNREACHABLE();
}

void EncodeVisitor::VisitLoadLexicalEnv(GraphVisitor *visitor, Inst *inst)
{
    if (TryLoadLexicalEnvGen(inst, static_cast<EncodeVisitor *>(visitor))) {
        return;
    }
    UNREACHABLE();
}

void EncodeVisitor::VisitLoadFromConstantPool([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    UNREACHABLE();
}

void EncodeVisitor::VisitSafePoint(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();
    auto graph = codegen->GetGraph();
    auto encoder = enc->GetEncoder();
    int64_t flagAddrOffset = graph->GetRuntime()->GetFlagAddrOffset(codegen->GetArch());
    ScopedTmpRegU16 tmp(encoder);
    // TMP <= Flag
    auto mem = MemRef(codegen->ThreadReg(), flagAddrOffset);
    encoder->EncodeLdr(tmp, false, mem);
    // check value and jump to call GC
    auto slowPath = codegen->CreateSlowPath<SlowPathEntrypoint>(inst, EntrypointId::SAFEPOINT);
    encoder->EncodeJump(slowPath->GetLabel(), tmp, Condition::NE);
    slowPath->BindBackLabel(encoder);
}

template <typename SelectInstType, typename = std::enable_if_t<!std::is_same_v<SelectInstType, Inst>>>
static auto GetFourthOperandOfSelectInst(EncodeVisitor *enc, SelectInstType *inst)
{
    constexpr bool FOURTH_OPERAND_IS_IMM = std::is_base_of_v<ImmediateMixin, SelectInstType>;
    if constexpr (FOURTH_OPERAND_IS_IMM) {
        return Imm(inst->GetImm());
    } else {
        auto cmpType = inst->GetOperandsType();
        return enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(3), cmpType);  // 3 : Index of 4th operand
    }
}

template <typename SelectInstType, typename = std::enable_if_t<!std::is_same_v<SelectInstType, Inst>>>
static auto DecomposeSelectInputs(EncodeVisitor *enc, SelectInstType *inst)
{
    constexpr int32_t IMM_2 = 2;
    constexpr bool FOURTH_OPERAND_IS_IMM = std::is_base_of_v<ImmediateMixin, SelectInstType>;

    auto [dst, src0, src1] = enc->GetCodegen()->ConvertRegisters<2U>(inst);
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_2), inst->GetOperandsType());
    auto src3OrImm = GetFourthOperandOfSelectInst(enc, inst);
    auto cc = enc->GetCodegen()->ConvertCc(inst->GetCc());
    if constexpr (std::is_same_v<SelectInstType, SelectTransformInst> ||
                  std::is_same_v<SelectInstType, SelectImmTransformInst>) {
        using ResultType =
            std::conditional_t<FOURTH_OPERAND_IS_IMM, Encoder::ArgsSelectImmTransform, Encoder::ArgsSelectTransform>;
        auto transform = inst->GetSelectTransformType();
        return ResultType {dst, src0, src1, src2, src3OrImm, cc, transform};
    } else {
        using ResultType = std::conditional_t<FOURTH_OPERAND_IS_IMM, Encoder::ArgsSelectImm, Encoder::ArgsSelect>;
        return ResultType {dst, src0, src1, src2, src3OrImm, cc};
    }
}

void EncodeVisitor::VisitSelect(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encArgs = DecomposeSelectInputs(static_cast<EncodeVisitor *>(visitor), inst->CastToSelect());
    if (IsTestCc(encArgs.cc)) {
        enc->GetEncoder()->EncodeSelectTest(encArgs);
    } else {
        enc->GetEncoder()->EncodeSelect(encArgs);
    }
}

void EncodeVisitor::VisitSelectImm(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encArgs = DecomposeSelectInputs(enc, inst->CastToSelectImm());
    if (IsTestCc(encArgs.cc)) {
        enc->GetEncoder()->EncodeSelectTest(encArgs);
    } else {
        enc->GetEncoder()->EncodeSelect(encArgs);
    }
}

void EncodeVisitor::VisitSelectTransform(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encArgs = DecomposeSelectInputs(enc, inst->CastToSelectTransform());
    if (IsTestCc(encArgs.cc)) {
        enc->GetEncoder()->EncodeSelectTestTransform(encArgs);
    } else {
        enc->GetEncoder()->EncodeSelectTransform(encArgs);
    }
}

void EncodeVisitor::VisitSelectImmTransform(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encArgs = DecomposeSelectInputs(enc, inst->CastToSelectImmTransform());
    if (IsTestCc(encArgs.cc)) {
        enc->GetEncoder()->EncodeSelectTestTransform(encArgs);
    } else {
        enc->GetEncoder()->EncodeSelectTransform(encArgs);
    }
}

void EncodeVisitor::VisitIf(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto bb = inst->GetBasicBlock();
    auto label = bb->GetTrueSuccessor()->GetId();
    auto type = inst->CastToIf()->GetOperandsType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    auto cc = enc->GetCodegen()->ConvertCc(inst->CastToIf()->GetCc());
    if (IsTestCc(cc)) {
        enc->GetEncoder()->EncodeJumpTest(label, src0, src1, cc);
    } else {
        enc->GetEncoder()->EncodeJump(label, src0, src1, cc);
    }
}

void EncodeVisitor::VisitIfImm(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto bb = inst->GetBasicBlock();
    auto label = bb->GetTrueSuccessor()->GetId();
    auto type = inst->CastToIfImm()->GetOperandsType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto imm = Imm(inst->CastToIfImm()->GetImm());
    auto cc = enc->GetCodegen()->ConvertCc(inst->CastToIfImm()->GetCc());
    if (IsTestCc(cc)) {
        enc->GetEncoder()->EncodeJumpTest(label, src0, imm, cc);
    } else {
        enc->GetEncoder()->EncodeJump(label, src0, imm, cc);
    }
}

void EncodeVisitor::VisitAddOverflow(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto bb = inst->GetBasicBlock();
    auto label = bb->GetTrueSuccessor()->GetId();
    auto type = inst->CastToAddOverflow()->GetOperandsType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    auto cc = enc->GetCodegen()->ConvertCcOverflow(inst->CastToAddOverflow()->GetCc());
    enc->GetEncoder()->EncodeAddOverflow(label, dst, src0, src1, cc);
}

void EncodeVisitor::VisitAddOverflowCheck(GraphVisitor *visitor, Inst *inst)
{
    ASSERT(DataType::IsTypeNumeric(inst->GetInput(0).GetInst()->GetType()));
    ASSERT(DataType::IsTypeNumeric(inst->GetInput(1).GetInst()->GetType()));
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, DeoptimizeType::OVERFLOW_TYPE);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src0 = Reg(inst->GetSrcReg(0), INT32_TYPE);
    auto src1 = Reg(inst->GetSrcReg(1), INT32_TYPE);
    enc->GetEncoder()->EncodeAddOverflow(slowPath->GetLabel(), dst, src0, src1, Condition::VS);
}

void EncodeVisitor::VisitSubOverflow(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto bb = inst->GetBasicBlock();
    auto label = bb->GetTrueSuccessor()->GetId();
    auto type = inst->CastToSubOverflow()->GetOperandsType();
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), type);
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);
    auto cc = enc->GetCodegen()->ConvertCcOverflow(inst->CastToSubOverflow()->GetCc());
    enc->GetEncoder()->EncodeSubOverflow(label, dst, src0, src1, cc);
}

void EncodeVisitor::VisitSubOverflowCheck(GraphVisitor *visitor, Inst *inst)
{
    ASSERT(DataType::IsTypeNumeric(inst->GetInput(0).GetInst()->GetType()));
    ASSERT(DataType::IsTypeNumeric(inst->GetInput(1).GetInst()->GetType()));
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, DeoptimizeType::OVERFLOW_TYPE);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src0 = Reg(inst->GetSrcReg(0), INT32_TYPE);
    auto src1 = Reg(inst->GetSrcReg(1), INT32_TYPE);
    enc->GetEncoder()->EncodeSubOverflow(slowPath->GetLabel(), dst, src0, src1, Condition::VS);
}

void EncodeVisitor::VisitNegOverflowAndZeroCheck(GraphVisitor *visitor, Inst *inst)
{
    ASSERT(DataType::IsTypeNumeric(inst->GetInput(0).GetInst()->GetType()));
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto slowPath = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, DeoptimizeType::OVERFLOW_TYPE);
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src = Reg(inst->GetSrcReg(0), INT32_TYPE);
    enc->GetEncoder()->EncodeNegOverflowAndZero(slowPath->GetLabel(), dst, src);
}

void EncodeVisitor::VisitLoadArrayPair(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto *loadArr = inst->CastToLoadArrayPair();
    auto type = inst->GetType();
    auto array = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto index = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::INT32);
    auto dst0 = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(0), type);  // first value
    auto dst1 = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(1), type);  // second value
    uint64_t indexImmValue = inst->CastToLoadArrayPair()->GetImm();
    auto offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch()) +
                  (indexImmValue << DataType::ShiftByType(type, enc->GetCodegen()->GetArch()));
    int32_t scale = DataType::ShiftByType(type, enc->GetCodegen()->GetArch());
    auto indexUpcasted = index.As(array.GetType());
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();

    if (loadArr->GetNeedBarrier()) {
        auto mem = MemRef(array, indexUpcasted, scale, offset);
        enc->GetCodegen()->CreateReadPairViaBarrier(inst, mem, dst0, dst1);
    } else {
        ScopedTmpReg tmp(enc->GetEncoder());
        enc->GetEncoder()->EncodeAdd(tmp, array, Shift(indexUpcasted, scale));
        enc->GetEncoder()->EncodeLdp(dst0, dst1, IsTypeSigned(type), MemRef(tmp, offset));
    }
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
}

void EncodeVisitor::VisitLoadObjectPair(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto loadObj = inst->CastToLoadObjectPair();
    auto type = inst->GetType();
    auto typeInput = inst->GetInput(0).GetInst()->GetType();
    auto src = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), typeInput);  // obj
    auto dst0 = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(0), type);
    auto dst1 = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(1), type);
    auto graph = enc->cg_->GetGraph();
    auto field0 = loadObj->GetObjField0();
    size_t offset0 = GetObjectOffset(graph, loadObj->GetObjectType(), field0, loadObj->GetTypeId0());
    auto mem = MemRef(src, offset0);
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();

    if (loadObj->GetNeedBarrier()) {
        enc->GetCodegen()->CreateReadPairViaBarrier(inst, mem, dst0, dst1);
    } else {
        enc->GetEncoder()->EncodeLdp(dst0, dst1, IsTypeSigned(type), mem);
    }
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
}

void EncodeVisitor::VisitLoadArrayPairI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto *loadArr = inst->CastToLoadArrayPairI();
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // array
    auto dst0 = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(0), type);                 // first value
    auto dst1 = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(1), type);                 // second value
    uint64_t index = loadArr->GetImm();
    auto offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch()) +
                  (index << DataType::ShiftByType(type, enc->GetCodegen()->GetArch()));
    auto mem = MemRef(src0, offset);
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();

    if (loadArr->GetNeedBarrier()) {
        enc->GetCodegen()->CreateReadPairViaBarrier(inst, mem, dst0, dst1);
    } else {
        enc->GetEncoder()->EncodeLdp(dst0, dst1, IsTypeSigned(type), mem);
    }
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
}

/**
 * It is a pseudo instruction that is needed to separate multiple outputs
 * from a single instruction in SSA such as LoadArrayPair and LoadArrayPairI.
 */
void EncodeVisitor::VisitLoadPairPart([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    // No codegeneration is required.
}

void EncodeVisitor::VisitStoreArrayPair(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto type = inst->GetType();
    auto array = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);
    auto index = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), DataType::INT32);
    constexpr auto IMM_2 = 2U;
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_2), type);  // first value
    constexpr auto IMM_3 = 3U;
    auto src3 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_3), type);  // second value
    uint64_t indexImmValue = inst->CastToStoreArrayPair()->GetImm();
    auto offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch()) +
                  (indexImmValue << DataType::ShiftByType(type, enc->GetCodegen()->GetArch()));
    auto scale = DataType::ShiftByType(type, enc->GetCodegen()->GetArch());
    auto tmp = enc->GetCodegen()->ConvertInstTmpReg(inst, DataType::REFERENCE);
    auto indexUpcasted = index.As(array.GetType());
    enc->GetEncoder()->EncodeAdd(tmp, array, Shift(indexUpcasted, scale));
    auto mem = MemRef(tmp, offset);
    if (inst->CastToStoreArrayPair()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem, MakeMask(array.GetId(), src2.GetId(), src3.GetId()), true);
    }
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeStp(src2, src3, mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
    if (inst->CastToStoreArrayPair()->GetNeedBarrier()) {
        auto tmpOffset = enc->GetCodegen()->ConvertInstTmpReg(inst, DataType::GetIntTypeForReference(enc->GetArch()));
        enc->GetEncoder()->EncodeShl(tmpOffset, indexUpcasted, Imm(scale));
        enc->GetEncoder()->EncodeAdd(tmpOffset, tmpOffset, Imm(offset));
        auto mem1 = MemRef(array, tmpOffset, 0);
        enc->GetCodegen()->CreatePostWRB(inst, mem1, src2, src3);
    }
}

void EncodeVisitor::VisitStoreObjectPair(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto encoder = enc->GetEncoder();
    auto storeObj = inst->CastToStoreObjectPair();
    ASSERT(storeObj->GetObjectType() == ObjectType::MEM_OBJECT || storeObj->GetObjectType() == ObjectType::MEM_STATIC);
    auto codegen = enc->GetCodegen();
    auto type = inst->GetType();
    auto src0 = codegen->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto src1 = codegen->ConvertRegister(inst->GetSrcReg(1), type);                 // store value 1
    auto src2 = codegen->ConvertRegister(inst->GetSrcReg(2), type);                 // store value 2
    auto graph = enc->cg_->GetGraph();
    auto field0 = storeObj->GetObjField0();
    auto field1 = storeObj->GetObjField1();
    size_t offset0 = GetObjectOffset(graph, storeObj->GetObjectType(), field0, storeObj->GetTypeId0());
    size_t offset1 = GetObjectOffset(graph, storeObj->GetObjectType(), field1, storeObj->GetTypeId1());
    if (!codegen->OffsetFitReferenceTypeSize(offset1)) {
        // such code should not be executed
        encoder->EncodeAbort();
    }
    auto mem = MemRef(src0, offset0);
    if (storeObj->GetNeedBarrier()) {
        codegen->CreatePreWRB(inst, mem, MakeMask(src1.GetId(), src2.GetId()));
    }
    auto prevOffset = encoder->GetCursorOffset();
    encoder->EncodeStp(src1, src2, mem);
    codegen->TryInsertImplicitNullCheck(inst, prevOffset);
    if (storeObj->GetNeedBarrier()) {
        codegen->CreatePostWRB(inst, mem, src1, src2);
    }
}

void EncodeVisitor::VisitStoreArrayPairI(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    auto type = inst->GetType();
    auto src0 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(0), DataType::REFERENCE);  // array
    auto src1 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(1), type);                 // first value
    constexpr int32_t IMM_2 = 2;
    auto src2 = enc->GetCodegen()->ConvertRegister(inst->GetSrcReg(IMM_2), type);  // second value
    auto index = inst->CastToStoreArrayPairI()->GetImm();
    auto offset = enc->cg_->GetGraph()->GetRuntime()->GetArrayDataOffset(enc->GetCodegen()->GetArch()) +
                  (index << DataType::ShiftByType(type, enc->GetCodegen()->GetArch()));
    if (!enc->GetCodegen()->OffsetFitReferenceTypeSize(offset)) {
        // such code should not be executed
        enc->GetEncoder()->EncodeAbort();
        return;
    }
    auto mem = MemRef(src0, offset);
    if (inst->CastToStoreArrayPairI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePreWRB(inst, mem, MakeMask(src0.GetId(), src1.GetId(), src2.GetId()), true);
    }
    auto prevOffset = enc->GetEncoder()->GetCursorOffset();
    enc->GetEncoder()->EncodeStp(src1, src2, mem);
    enc->GetCodegen()->TryInsertImplicitNullCheck(inst, prevOffset);
    if (inst->CastToStoreArrayPairI()->GetNeedBarrier()) {
        enc->GetCodegen()->CreatePostWRB(inst, mem, src1, src2);
    }
}

void EncodeVisitor::VisitNOP([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
#ifndef NDEBUG
    COMPILER_LOG(DEBUG, CODEGEN) << "The NOP wasn't removed before " << *inst;
#endif
}

void EncodeVisitor::VisitThrow(GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    auto codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    SlowPathCheck slowPath(codegen->GetEncoder()->CreateLabel(), inst, EntrypointId::THROW_EXCEPTION);
    slowPath.Generate(codegen);
}

void EncodeVisitor::VisitDeoptimize(GraphVisitor *visitor, Inst *inst)
{
    auto codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    ASSERT(inst->GetSaveState() != nullptr);
    SlowPathDeoptimize slowPath(codegen->GetEncoder()->CreateLabel(), inst,
                                inst->CastToDeoptimize()->GetDeoptimizeType());
    slowPath.Generate(codegen);
}

void EncodeVisitor::VisitIsMustDeoptimize(GraphVisitor *visitor, Inst *inst)
{
    auto *codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    auto *enc = static_cast<EncodeVisitor *>(visitor)->GetEncoder();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto offset = CFrameFlags::GetOffsetFromSpInBytes(codegen->GetFrameLayout());
    enc->EncodeLdr(dst, false, MemRef(codegen->SpReg(), offset));
    enc->EncodeAnd(dst, dst, Imm(1));
}

void EncodeVisitor::VisitGetInstanceClass(GraphVisitor *visitor, Inst *inst)
{
    auto *codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    auto [dst, objReg] = codegen->ConvertRegisters<1U>(inst);
    ASSERT(objReg.IsValid());
    codegen->LoadClassFromObject(dst, objReg);
}

void EncodeVisitor::VisitLoadImmediate(GraphVisitor *visitor, Inst *inst)
{
    auto codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto loadImm = inst->CastToLoadImmediate();
    if (loadImm->GetObjectType() == LoadImmediateInst::ObjectType::PANDA_FILE_OFFSET) {
        auto runtime = codegen->GetGraph()->GetRuntime();
        auto pfOffset = runtime->GetPandaFileOffset(codegen->GetGraph()->GetArch());
        codegen->LoadMethod(dst);
        // load pointer to panda file
        codegen->GetEncoder()->EncodeLdr(dst, false, MemRef(dst, pfOffset));
        codegen->GetEncoder()->EncodeLdr(dst, false, MemRef(dst, cross_values::GetFileBaseOffset(codegen->GetArch())));
        // add pointer to pf with offset to str
        codegen->GetEncoder()->EncodeAdd(dst, dst, Imm(loadImm->GetPandaFileOffset()));
    } else if (loadImm->GetObjectType() == LoadImmediateInst::ObjectType::TLS_OFFSET) {
        codegen->GetEncoder()->EncodeLdr(dst, false, MemRef(codegen->ThreadReg(), loadImm->GetTlsOffset()));
    } else {
        codegen->GetEncoder()->EncodeMov(dst, Imm(reinterpret_cast<uintptr_t>(loadImm->GetObject())));
    }
}

void EncodeVisitor::VisitFunctionImmediate(GraphVisitor *visitor, Inst *inst)
{
    auto *codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    codegen->GetEncoder()->EncodeMov(dst, Imm(inst->CastToFunctionImmediate()->GetFunctionPtr()));
    codegen->GetEncoder()->EncodeLdr(dst, false, MemRef(dst, 0U));
}

void EncodeVisitor::VisitLoadObjFromConst(GraphVisitor *visitor, Inst *inst)
{
    auto *codegen = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    Reg dstPointer(dst.As(TypeInfo::FromDataType(DataType::POINTER, codegen->GetArch())));
    codegen->GetEncoder()->EncodeMov(dstPointer, Imm(inst->CastToLoadObjFromConst()->GetObjPtr()));
    codegen->GetEncoder()->EncodeLdr(dst, false, MemRef(dstPointer, 0U));
}

void EncodeVisitor::VisitRegDef([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    // No codegeneration is required.
}

void EncodeVisitor::VisitLiveIn([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    // No codegeneration is required.
}

void EncodeVisitor::VisitLiveOut([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto codegen = enc->GetCodegen();
    codegen->AddLiveOut(inst->GetBasicBlock(), inst->GetDstReg());
    auto dstReg = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    if (codegen->GetTarget().GetTempRegsMask().Test(dstReg.GetId()) &&
        enc->GetEncoder()->IsScratchRegisterReleased(dstReg)) {
        enc->GetEncoder()->AcquireScratchRegister(dstReg);
    }
    if (inst->GetLocation(0) != inst->GetDstLocation()) {
        auto *src = inst->GetInput(0).GetInst();
        enc->GetEncoder()->EncodeMov(dstReg, codegen->ConvertRegister(inst->GetSrcReg(0), src->GetType()));
    }
}

void EncodeVisitor::VisitCompareAnyType(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    const auto *cati = inst->CastToCompareAnyType();
    if (cati->GetInputType(0) != DataType::Type::ANY) {
        enc->GetEncoder()->EncodeAbort();
        UNREACHABLE();
        return;
    }
    if (TryCompareAnyTypePluginGen(cati, enc)) {
        return;
    }
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), inst->GetType());
    if (cati->GetAnyType() == AnyBaseType::UNDEFINED_TYPE) {
        enc->GetEncoder()->EncodeMov(dst, Imm(true));
    } else {
        enc->GetEncoder()->EncodeMov(dst, Imm(false));
    }
}

void EncodeVisitor::VisitGetAnyTypeName(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    const auto *cati = inst->CastToGetAnyTypeName();
    if (TryGetAnyTypeNamePluginGen(cati, enc)) {
        return;
    }
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), DataType::Type::BOOL);
    enc->GetEncoder()->EncodeMov(dst, Imm(0));
}

void EncodeVisitor::VisitCastAnyTypeValue(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    const auto *cati = inst->CastToCastAnyTypeValue();
    if (cati->GetInputType(0) != DataType::Type::ANY) {
        enc->GetEncoder()->EncodeAbort();
        UNREACHABLE();
        return;
    }
    if (TryCastAnyTypeValuePluginGen(cati, enc)) {
        return;
    }
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), DataType::Type::BOOL);
    enc->GetEncoder()->EncodeMov(dst, Imm(0));
}

void EncodeVisitor::VisitCastValueToAnyType(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    const auto *cvai = inst->CastToCastValueToAnyType();
    if (TryCastValueToAnyTypePluginGen(cvai, enc)) {
        return;
    }
    auto dst = enc->GetCodegen()->ConvertRegister(inst->GetDstReg(), DataType::Type::BOOL);
    enc->GetEncoder()->EncodeMov(dst, Imm(0));
}

void EncodeVisitor::VisitAnyTypeCheck(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto *checkInst = inst->CastToAnyTypeCheck();
    if (checkInst->GetInputType(0) != DataType::Type::ANY) {
        enc->GetEncoder()->EncodeAbort();
        UNREACHABLE();
    }
    // Empty check
    if (checkInst->GetAnyType() == AnyBaseType::UNDEFINED_TYPE) {
        return;
    }
    if (TryAnyTypeCheckPluginGen(checkInst, enc)) {
        return;
    }
    UNREACHABLE();
}

void EncodeVisitor::VisitHclassCheck(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    auto *checkInst = inst->CastToHclassCheck();
    if (checkInst->GetInputType(0) != DataType::Type::REFERENCE) {
        enc->GetEncoder()->EncodeAbort();
        UNREACHABLE();
    }
    if (TryHclassCheckPluginGen(checkInst, enc)) {
        return;
    }
    UNREACHABLE();
}

void EncodeVisitor::VisitObjByIndexCheck(GraphVisitor *visitor, Inst *inst)
{
    auto enc = static_cast<EncodeVisitor *>(visitor);
    const auto *checkInst = inst->CastToObjByIndexCheck();
    if (checkInst->GetInputType(0) != DataType::Type::ANY) {
        enc->GetEncoder()->EncodeAbort();
        UNREACHABLE();
    }
    auto id = enc->GetCodegen()->CreateSlowPath<SlowPathDeoptimize>(inst, DeoptimizeType::ANY_TYPE_CHECK)->GetLabel();
    if (TryObjByIndexCheckPluginGen(checkInst, enc, id)) {
        return;
    }
    UNREACHABLE();
}

void EncodeVisitor::VisitResolveByName(GraphVisitor *visitor, Inst *inst)
{
    auto *enc = static_cast<EncodeVisitor *>(visitor);
    enc->GetCodegen()->ResolveCallByName(inst->CastToResolveByName());
}

void EncodeVisitor::VisitStringFlatCheck(GraphVisitor *visitor, Inst *inst)
{
    auto *cg = static_cast<EncodeVisitor *>(visitor)->GetCodegen();
    auto dst = cg->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src = cg->ConvertRegister(inst->GetSrcReg(0), inst->GetInputType(0));
    if (cg->GetArch() == Arch::AARCH32) {
        cg->CallRuntime(inst, EntrypointId::STRING_FLAT_CHECK_ARM32, dst, RegMask::GetZeroMask(), src);
    } else {
        cg->CallFastPath(inst, EntrypointId::STRING_FLAT_CHECK, dst, {}, src);
    }
}
}  // namespace ark::compiler
