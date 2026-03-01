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

#include "slow_path.h"
#include "codegen.h"

namespace ark::compiler {

void SlowPathBase::Generate(Codegen *codegen)
{
    ASSERT(!generated_);

#ifndef NDEBUG
    std::string opcodeStr(GetInst()->GetOpcodeStr());
    if (GetInst()->IsIntrinsic()) {
        opcodeStr += "." + GetIntrinsicName(static_cast<IntrinsicInst *>(GetInst())->GetIntrinsicId());
    }
#endif
    SCOPED_DISASM_STR(codegen,
                      std::string("SlowPath for inst ") + std::to_string(GetInst()->GetId()) + ". " + opcodeStr);
    Encoder *encoder = codegen->GetEncoder();
    ASSERT(encoder->IsValid());
    encoder->BindLabel(GetLabel());

    GenerateImpl(codegen);

    if (encoder->IsLabelValid(labelBack_)) {
        codegen->GetEncoder()->EncodeJump(GetBackLabel());
    }
#ifndef NDEBUG
    generated_ = true;
#endif
}

// ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION, STRING_INDEX_OUT_OF_BOUNDS_EXCEPTION
bool SlowPathEntrypoint::GenerateThrowOutOfBoundsException(Codegen *codegen)
{
    auto lenReg = codegen->ConvertRegister(GetInst()->GetSrcReg(0), GetInst()->GetInputType(0));
    if (GetInst()->GetOpcode() == Opcode::BoundsCheckI) {
        ScopedTmpReg indexReg(codegen->GetEncoder());
        codegen->GetEncoder()->EncodeMov(indexReg, Imm(GetInst()->CastToBoundsCheckI()->GetImm()));
        codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), indexReg, lenReg);
    } else {
        ASSERT(GetInst()->GetOpcode() == Opcode::BoundsCheck);
        auto indexReg = codegen->ConvertRegister(GetInst()->GetSrcReg(1), GetInst()->GetInputType(1));
        codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), indexReg, lenReg);
    }
    return true;
}

// INITIALIZE_CLASS
bool SlowPathEntrypoint::GenerateInitializeClass(Codegen *codegen)
{
    auto inst = GetInst();
    if (GetInst()->GetDstReg() != INVALID_REG) {
        ASSERT(inst->GetOpcode() == Opcode::LoadAndInitClass);
        Reg klassReg {codegen->ConvertRegister(GetInst()->GetDstReg(), DataType::REFERENCE)};
        RegMask preservedRegs;
        codegen->GetEncoder()->SetRegister(&preservedRegs, nullptr, klassReg);
        codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, preservedRegs, klassReg);
    } else {
        ASSERT(inst->GetOpcode() == Opcode::InitClass);
        ASSERT(!codegen->GetGraph()->IsAotMode());
        // check uintptr_t for cross:
        auto klass = reinterpret_cast<uintptr_t>(inst->CastToInitClass()->GetClass());
        codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), TypedImm(klass));
    }
    return true;
}

// IS_INSTANCE
bool SlowPathEntrypoint::GenerateIsInstance(Codegen *codegen)
{
    auto src = codegen->ConvertRegister(GetInst()->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto klass = codegen->ConvertRegister(GetInst()->GetSrcReg(1), DataType::REFERENCE);
    auto dst = codegen->ConvertRegister(GetInst()->GetDstReg(), GetInst()->GetType());
    codegen->CallRuntime(GetInst(), EntrypointId::IS_INSTANCE, dst, RegMask::GetZeroMask(), src, klass);
    return true;
}

// CHECK_CAST
bool SlowPathEntrypoint::GenerateCheckCast(Codegen *codegen)
{
    auto src = codegen->ConvertRegister(GetInst()->GetSrcReg(0), DataType::REFERENCE);  // obj
    auto klass = codegen->ConvertRegister(GetInst()->GetSrcReg(1), DataType::REFERENCE);
    codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), src, klass);
    return true;
}

// CREATE_OBJECT
bool SlowPathEntrypoint::GenerateCreateObject(Codegen *codegen)
{
    auto inst = GetInst();
    auto dst = codegen->ConvertRegister(inst->GetDstReg(), inst->GetType());
    auto src = codegen->ConvertRegister(inst->GetSrcReg(0), inst->GetInputType(0));

    codegen->CallRuntime(inst, EntrypointId::CREATE_OBJECT_BY_CLASS, dst, RegMask::GetZeroMask(), src);

    return true;
}

bool SlowPathEntrypoint::GenerateByEntry(Codegen *codegen)
{
    switch (GetEntrypoint()) {
        case EntrypointId::THROW_EXCEPTION: {
            auto src = codegen->ConvertRegister(GetInst()->GetSrcReg(0), DataType::Type::REFERENCE);
            codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), src);
            return true;
        }
        case EntrypointId::NULL_POINTER_EXCEPTION:
        case EntrypointId::ARITHMETIC_EXCEPTION:
        case EntrypointId::THROW_NATIVE_EXCEPTION:
            codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, {});
            return true;
        case EntrypointId::ARRAY_INDEX_OUT_OF_BOUNDS_EXCEPTION:
        case EntrypointId::STRING_INDEX_OUT_OF_BOUNDS_EXCEPTION:
            return GenerateThrowOutOfBoundsException(codegen);
        case EntrypointId::NEGATIVE_ARRAY_SIZE_EXCEPTION: {
            auto size = codegen->ConvertRegister(GetInst()->GetSrcReg(0), GetInst()->GetInputType(0));
            codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), size);
            return true;
        }
        case EntrypointId::INITIALIZE_CLASS:
            return GenerateInitializeClass(codegen);
        case EntrypointId::IS_INSTANCE:
            return GenerateIsInstance(codegen);
        case EntrypointId::CHECK_CAST:
        case EntrypointId::CHECK_CAST_DEOPTIMIZE:
            return GenerateCheckCast(codegen);
        case EntrypointId::CREATE_OBJECT_BY_CLASS:
            return GenerateCreateObject(codegen);
        case EntrypointId::SAFEPOINT:
            codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, {});
            return true;
        default:
            return false;
    }
}

void SlowPathEntrypoint::GenerateImpl(Codegen *codegen)
{
    if (!GenerateByEntry(codegen)) {
        switch (GetEntrypoint()) {
            case EntrypointId::GET_UNKNOWN_CALLEE_METHOD:
            case EntrypointId::RESOLVE_UNKNOWN_VIRTUAL_CALL:
            case EntrypointId::GET_FIELD_OFFSET:
            case EntrypointId::GET_UNKNOWN_STATIC_FIELD_MEMORY_ADDRESS:
            case EntrypointId::RESOLVE_CLASS_OBJECT:
            case EntrypointId::RESOLVE_CLASS:
            case EntrypointId::ABSTRACT_METHOD_ERROR:
            case EntrypointId::INITIALIZE_CLASS_BY_ID:
            case EntrypointId::CHECK_STORE_ARRAY_REFERENCE:
            case EntrypointId::RESOLVE_STRING_AOT:
            case EntrypointId::CLASS_CAST_EXCEPTION:
                break;
            default:
                LOG(FATAL, COMPILER) << "Unsupported entrypoint!";
                UNREACHABLE();
                break;
        }
    }
}

void SlowPathDeoptimize::GenerateImpl(Codegen *codegen)
{
    uintptr_t value =
        helpers::ToUnderlying(deoptimizeType_) | (GetInst()->GetId() << MinimumBitsToStore(DeoptimizeType::COUNT));
    codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), TypedImm(value));
}

void SlowPathIntrinsic::GenerateImpl(Codegen *codegen)
{
    codegen->CreateCallIntrinsic(GetInst()->CastToIntrinsic());
}

void SlowPathImplicitNullCheck::GenerateImpl(Codegen *codegen)
{
    ASSERT(!GetInst()->CastToNullCheck()->IsImplicit());
    SlowPathEntrypoint::GenerateImpl(codegen);
}

void SlowPathShared::GenerateImpl(Codegen *codegen)
{
    ASSERT(tmpReg_ != INVALID_REGISTER);
    [[maybe_unused]] ScopedTmpReg tmpReg(codegen->GetEncoder(), tmpReg_);
    ASSERT(tmpReg.GetReg().GetId() == tmpReg_.GetId());
    auto graph = codegen->GetGraph();
    ASSERT(graph->IsAotMode());
    auto aotData = graph->GetAotData();
    aotData->SetSharedSlowPathOffset(GetEntrypoint(), codegen->GetEncoder()->GetCursorOffset());
    ScopedTmpReg tmp1Reg(codegen->GetEncoder());
    codegen->GetEntrypoint(tmp1Reg, GetEntrypoint());
    codegen->GetEncoder()->EncodeJump(tmp1Reg);
}

void SlowPathResolveStringAot::GenerateImpl(Codegen *codegen)
{
    ScopedTmpRegU64 tmpAddrReg(codegen->GetEncoder());
    // Slot address was loaded into temporary register before we jumped into slow path, but it is already released
    // because temporary registers are scoped. Try to allocate a new one and check that it is the same register
    // as was allocated in codegen. If it is a different register then copy the slot address into it.
    if (tmpAddrReg.GetReg() != addrReg_) {
        codegen->GetEncoder()->EncodeMov(tmpAddrReg, addrReg_);
    }
    auto typeIdImm = codegen->GetTypeIdImm(GetInst(), stringId_);
    codegen->CallRuntimeWithMethod(GetInst(), method_, GetEntrypoint(), dstReg_, typeIdImm, tmpAddrReg);
}

void SlowPathRefCheck::GenerateImpl(Codegen *codegen)
{
    ASSERT(arrayReg_ != INVALID_REGISTER);
    ASSERT(refReg_ != INVALID_REGISTER);
    codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), arrayReg_, refReg_);
}

void SlowPathAbstract::GenerateImpl(Codegen *codegen)
{
    SCOPED_DISASM_STR(codegen, std::string("SlowPath for Abstract method ") + std::to_string(GetInst()->GetId()));
    ASSERT(methodReg_ != INVALID_REGISTER);
    ScopedTmpReg methodReg(codegen->GetEncoder(), methodReg_);
    ASSERT(methodReg.GetReg().GetId() == methodReg_.GetId());
    codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), methodReg.GetReg());
}

void SlowPathCheckCast::GenerateImpl(Codegen *codegen)
{
    SCOPED_DISASM_STR(codegen, std::string("SlowPath for CheckCast exception") + std::to_string(GetInst()->GetId()));
    auto inst = GetInst();
    auto src = codegen->ConvertRegister(inst->GetSrcReg(0), inst->GetInputType(0));

    codegen->CallRuntime(GetInst(), GetEntrypoint(), INVALID_REGISTER, RegMask::GetZeroMask(), classReg_, src);
}

void SlowPathUnresolved::GenerateImpl(Codegen *codegen)
{
    SlowPathEntrypoint::GenerateImpl(codegen);

    ASSERT(method_ != nullptr);
    ASSERT(typeId_ != 0);
    ASSERT(slotAddr_ != 0);
    auto typeIdImm = codegen->GetTypeIdImm(GetInst(), typeId_);
    auto arch = codegen->GetGraph()->GetArch();
    // On 32-bit architecture slot address requires additional down_cast,
    // similar to `method` address processing in `CallRuntimeWithMethod`
    auto slotImm = Is64BitsArch(arch) ? TypedImm(slotAddr_) : TypedImm(down_cast<uint32_t>(slotAddr_));

    ScopedTmpReg valueReg(codegen->GetEncoder());
    if (GetInst()->GetOpcode() == Opcode::ResolveVirtual || GetInst()->GetOpcode() == Opcode::ResolveByName) {
        codegen->CallRuntimeWithMethod(GetInst(), method_, GetEntrypoint(), valueReg, argReg_, typeIdImm, slotImm);
    } else if (GetEntrypoint() == EntrypointId::GET_UNKNOWN_CALLEE_METHOD ||
               GetEntrypoint() == EntrypointId::GET_UNKNOWN_STATIC_FIELD_MEMORY_ADDRESS) {
        codegen->CallRuntimeWithMethod(GetInst(), method_, GetEntrypoint(), valueReg, typeIdImm, slotImm);
    } else {
        codegen->CallRuntimeWithMethod(GetInst(), method_, GetEntrypoint(), valueReg, typeIdImm);

        ScopedTmpReg addrReg(codegen->GetEncoder());
        codegen->GetEncoder()->EncodeMov(addrReg, Imm(slotAddr_));
        codegen->GetEncoder()->EncodeStr(valueReg, MemRef(addrReg));
    }

    if (dstReg_.IsValid()) {
        codegen->GetEncoder()->EncodeMov(dstReg_, valueReg);
    }
}

void SlowPathJsCastDoubleToInt32::GenerateImpl(Codegen *codegen)
{
    ASSERT(dstReg_.IsValid());
    ASSERT(srcReg_.IsValid());

    auto enc {codegen->GetEncoder()};
    if (codegen->GetGraph()->GetMode().SupportManagedCode()) {
        ScopedTmpRegU64 tmp(enc);
        enc->EncodeMov(tmp, srcReg_);
        codegen->CallRuntime(GetInst(), EntrypointId::JS_CAST_DOUBLE_TO_INT32, dstReg_, RegMask::GetZeroMask(), tmp);
        return;
    }

    auto [live_regs, live_vregs] {codegen->GetLiveRegisters<true>(GetInst())};
    live_regs.Reset(dstReg_.GetId());

    codegen->SaveCallerRegisters(live_regs, live_vregs, true);
    codegen->FillCallParams(srcReg_);
    codegen->EmitCallRuntimeCode(nullptr, EntrypointId::JS_CAST_DOUBLE_TO_INT32_NO_BRIDGE);

    auto retReg {codegen->GetTarget().GetReturnReg(dstReg_.GetType())};
    if (dstReg_.GetId() != retReg.GetId()) {
        enc->EncodeMov(dstReg_, retReg);
    }
    codegen->LoadCallerRegisters(live_regs, live_vregs, true);
}

void SlowPathStringHashCode::GenerateImpl(Codegen *codegen)
{
    ASSERT(dstReg_.IsValid());
    ASSERT(srcReg_.IsValid());
    codegen->CallFastPath(GetInst(), GetEntrypoint(), dstReg_, RegMask::GetZeroMask(), srcReg_);
}

}  // namespace ark::compiler
