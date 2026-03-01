/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "compiler/optimizer/code_generator/operands.h"
#include "compiler/optimizer/code_generator/codegen.h"
#include "compiler/optimizer/ir/analysis.h"
#include "runtime/include/coretypes/string.h"
#include "runtime/include/coretypes/array.h"
#include "libarkbase/utils/regmask.h"
#include "plugins/ets/compiler/compiler_constants_inl.h"
#include "plugins/ets/compiler/compiler_intrinsic_id_mapping_inl.h"

namespace ark::compiler {

class SbAppendArgs {
private:
    Reg dst_;
    Reg builder_;
    Reg value_;

public:
    SbAppendArgs() = delete;
    SbAppendArgs(Reg dst, Reg builder, Reg value) : dst_(dst), builder_(builder), value_(value)
    {
        ASSERT(dst_ != INVALID_REGISTER);
        ASSERT(builder_ != INVALID_REGISTER);
        ASSERT(value_ != INVALID_REGISTER);
    }
    Reg Dst() const
    {
        return dst_;
    }
    Reg Builder() const
    {
        return builder_;
    }
    Reg Value() const
    {
        return value_;
    }
    bool DstCanBeUsedAsTemp() const
    {
        return (dst_.GetId() != builder_.GetId() && dst_.GetId() != value_.GetId());
    }
    MemRef SbBufferAddr() const
    {
        return MemRef(builder_, RuntimeInterface::GetSbBufferOffset());
    }
    MemRef SbIndexAddr() const
    {
        return MemRef(builder_, RuntimeInterface::GetSbIndexOffset());
    }
    MemRef SbCompressAddr() const
    {
        return MemRef(builder_, RuntimeInterface::GetSbCompressOffset());
    }
    MemRef SbLengthAddr() const
    {
        return MemRef(builder_, RuntimeInterface::GetSbLengthOffset());
    }
};

void Codegen::CreateMathTrunc([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeTrunc(dst, src[0]);
}

void Codegen::CreateETSMathRound([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeRoundToPInfReturnFloat(dst, src[0]);
}

void Codegen::CreateArrayCopyTo(IntrinsicInst *inst, [[maybe_unused]] Reg dst, SRCREGS src)
{
    auto entrypointId = EntrypointId::INVALID;

    switch (inst->GetIntrinsicId()) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BOOL_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_BYTE_COPY_TO:
            entrypointId = EntrypointId::ARRAY_COPY_TO_1B;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_CHAR_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SHORT_COPY_TO:
            entrypointId = EntrypointId::ARRAY_COPY_TO_2B;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_INT_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_FLOAT_COPY_TO:
            entrypointId = EntrypointId::ARRAY_COPY_TO_4B;
            break;

        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_LONG_COPY_TO:
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_DOUBLE_COPY_TO:
            entrypointId = EntrypointId::ARRAY_COPY_TO_8B;
            break;

        default:
            UNREACHABLE();
            break;
    }

    ASSERT(entrypointId != EntrypointId::COUNT);

    auto srcObj = src[FIRST_OPERAND];
    auto dstObj = src[SECOND_OPERAND];
    auto dstStart = src[THIRD_OPERAND];
    auto srcStart = src[FOURTH_OPERAND];
    auto srcEnd = src[FIFTH_OPERAND];
    CallFastPath(inst, entrypointId, INVALID_REGISTER, RegMask::GetZeroMask(), srcObj, dstObj, dstStart, srcStart,
                 srcEnd);
}

void Codegen::CreateEscompatArrayIsPlatformArray(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    CallFastPath(inst, EntrypointId::ESCOMPAT_ARRAY_IS_PLATFORM_ARRAY_FAST, dst, {}, src[FIRST_OPERAND]);
}

static RuntimeInterface::EntrypointId GetArrayFastReverseEntrypointId(mem::BarrierType barrierType)
{
    using EntrypointId = RuntimeInterface::EntrypointId;
    switch (barrierType) {
        case mem::BarrierType::POST_CMC_WRITE_BARRIER:  // CMC GC
            return EntrypointId::ESCOMPAT_ARRAY_REVERSE_HYBRID;
        case mem::BarrierType::POST_INTERREGION_BARRIER:  // G1 GC
            return EntrypointId::ESCOMPAT_ARRAY_REVERSE_ASYNC;
        default:  // STW GC
            return EntrypointId::ESCOMPAT_ARRAY_REVERSE_SYNC;
    }
}

void Codegen::CreateEscompatArrayReverse(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);

    auto labelSlowPath = enc_->CreateLabel();
    auto len = src[SECOND_OPERAND];
    enc_->EncodeJump(labelSlowPath, len, Imm(MAGIC_NUM_FOR_SHORT_ARRAY_THRESHOLD), Condition::GT);
    auto labelEnd = enc_->CreateLabel();
    auto fastPathEntrypointId = GetArrayFastReverseEntrypointId(GetGraph()->GetRuntime()->GetPostType());
    // NOTE(srokashevich, #30178): remove fake arg(second src[FIRST_OPERAND]) after fix
    CallFastPath(inst, fastPathEntrypointId, dst, {}, src[FIRST_OPERAND], src[FIRST_OPERAND], src[SECOND_OPERAND]);
    enc_->EncodeJump(labelEnd);

    enc_->BindLabel(labelSlowPath);
    static constexpr auto SLOW_PATH_ENTRYPOINT_ID = EntrypointId::ESCOMPAT_ARRAY_REVERSE;
    CallRuntime(inst, SLOW_PATH_ENTRYPOINT_ID, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND]);
    enc_->BindLabel(labelEnd);
}

// Generates a call to StringBuilder.append() for values (EtsBool/Char/Bool/Short/Int/Long),
// which are translated to array of utf16 chars.
static inline void GenerateSbAppendCall(Codegen *cg, IntrinsicInst *inst, SbAppendArgs args,
                                        RuntimeInterface::EntrypointId entrypoint)
{
    auto *runtime = cg->GetGraph()->GetRuntime();
    if (cg->GetGraph()->IsAotMode()) {
        auto *enc = cg->GetEncoder();
        ScopedTmpReg klass(enc);
        enc->EncodeLdr(klass, false, MemRef(cg->ThreadReg(), runtime->GetArrayU16ClassPointerTlsOffset(cg->GetArch())));
        cg->CallFastPath(inst, entrypoint, args.Dst(), {}, args.Builder(), args.Value(), klass);
    } else {
        auto klass = TypedImm(reinterpret_cast<uintptr_t>(runtime->GetArrayU16Class(cg->GetGraph()->GetMethod())));
        cg->CallFastPath(inst, entrypoint, args.Dst(), {}, args.Builder(), args.Value(), klass);
    }
}

void Codegen::CreateStringBuilderAppendNumber(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto sb = src[FIRST_OPERAND];
    auto num = src[SECOND_OPERAND];
    auto type = ConvertDataType(DataType::INT64, GetArch());
    ScopedTmpReg tmp(GetEncoder(), type);

    if (num.GetType() != INT64_TYPE) {
        ASSERT(num.GetType() == INT32_TYPE || num.GetType() == INT16_TYPE || num.GetType() == INT8_TYPE);
        if (dst.GetId() != sb.GetId() && dst.GetId() != num.GetId()) {
            GetEncoder()->EncodeCast(dst.As(type), true, num, true);
            num = dst.As(type);
        } else {
            GetEncoder()->EncodeCast(tmp, true, num, true);
            num = tmp.GetReg();
        }
    }
    GenerateSbAppendCall(this, inst, SbAppendArgs(dst, sb, num), EntrypointId::STRING_BUILDER_APPEND_LONG);
}

void Codegen::CreateStringBuilderAppendChar(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto entrypoint = EntrypointId::STRING_BUILDER_APPEND_CHAR;
    SbAppendArgs args(dst, src[FIRST_OPERAND], src[SECOND_OPERAND]);
    GenerateSbAppendCall(this, inst, args, entrypoint);
}

void Codegen::CreateStringBuilderAppendBool(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    SbAppendArgs args(dst, src[FIRST_OPERAND], src[SECOND_OPERAND]);
    GenerateSbAppendCall(this, inst, args, EntrypointId::STRING_BUILDER_APPEND_BOOL);
}

static inline void EncodeSbAppendNullString(Codegen *cg, IntrinsicInst *inst, Reg dst, Reg builder)
{
    auto entrypoint = RuntimeInterface::EntrypointId::STRING_BUILDER_APPEND_NULL_STRING;
    cg->CallRuntime(inst, entrypoint, dst, {}, builder);
}

static inline void EncodeSbInsertStringIntoSlot(Codegen *cg, IntrinsicInst *inst, Reg slot, SbAppendArgs args)
{
    ASSERT(slot.IsValid());
    auto slotMemRef = MemRef(slot.As(Codegen::ConvertDataType(DataType::REFERENCE, cg->GetArch())));
    RegMask preserved(MakeMask(args.Builder().GetId(), args.Value().GetId(), slot.GetId()));
    cg->CreatePreWRB(inst, slotMemRef, preserved);
    cg->GetEncoder()->EncodeStr(args.Value(), slotMemRef);
    preserved.Reset(slot.GetId());
    cg->CreatePostWRB(inst, slotMemRef, args.Value(), INVALID_REGISTER, preserved);
}

static void EncodeSbAppendString(Codegen *cg, IntrinsicInst *inst, const SbAppendArgs &args,
                                 LabelHolder::LabelId labelReturn, LabelHolder::LabelId labelSlowPath)
{
    auto *enc = cg->GetEncoder();
    ScopedTmpReg tmp1(enc);
    ScopedTmpRegLazy tmp2(enc, false);
    auto reg0 = cg->ConvertInstTmpReg(inst, DataType::REFERENCE);
    auto reg1 = tmp1.GetReg().As(INT32_TYPE);
    auto reg2 = INVALID_REGISTER;
    if (args.DstCanBeUsedAsTemp() && args.Dst().GetId() != reg0.GetId()) {
        reg2 = args.Dst().As(INT32_TYPE);
    } else {
        tmp2.Acquire();
        reg2 = tmp2.GetReg().As(INT32_TYPE);
    }
    auto labelInsertStringIntoSlot = enc->CreateLabel();
    auto labelFastPathDone = enc->CreateLabel();
    auto labelIncIndex = enc->CreateLabel();
    // Jump to slowPath if buffer is full and needs to be reallocated
    if (cg->GetRuntime()->NeedsPreReadBarrier()) {
        cg->CreateReadViaBarrier(inst, args.SbBufferAddr(), reg0, false,
                                 MakeMask(args.SbBufferAddr().GetBase().GetId(), args.Value().GetId()));
    } else {
        enc->EncodeLdr(reg0, false, args.SbBufferAddr());
    }
    enc->EncodeLdr(reg1, false, MemRef(reg0, coretypes::Array::GetLengthOffset()));
    enc->EncodeLdr(reg2, false, args.SbIndexAddr());
    enc->EncodeJump(labelSlowPath, reg2, reg1, Condition::HS);
    // Compute an address of a free slot so as not to reload SbIndex again
    enc->EncodeShl(reg1, reg2, Imm(compiler::DataType::ShiftByType(compiler::DataType::REFERENCE, cg->GetArch())));
    enc->EncodeAdd(reg0, reg0, Imm(coretypes::Array::GetDataOffset()));
    enc->EncodeAdd(reg0, reg0, reg1);
    // Process string length and compression
    enc->EncodeLdr(reg1, false, MemRef(args.Value(), ark::coretypes::STRING_LENGTH_OFFSET));
    // Do nothing if length of string is equal to 0.
    // The least significant bit indicates COMPRESSED/UNCOMPRESSED,
    // thus if (packed length <= 1) then the actual length is equal to 0.
    enc->EncodeJump(labelFastPathDone, reg1, Imm(1), Condition::LS);
    // Skip setting 'compress' to false if the string is compressed.
    enc->EncodeJumpTest(labelIncIndex, reg1, Imm(1), Condition::TST_EQ);
    // Otherwise set 'compress' to false
    enc->EncodeSti(0, 1, args.SbCompressAddr());
    // Increment 'index' field
    enc->BindLabel(labelIncIndex);
    enc->EncodeAdd(reg2, reg2, Imm(1));
    enc->EncodeStr(reg2, args.SbIndexAddr());
    // Unpack length of string
    enc->EncodeShr(reg1, reg1, Imm(ark::coretypes::String::STRING_LENGTH_SHIFT));
    // Add length of string to the current length of StringBuilder
    enc->EncodeLdr(reg2, false, args.SbLengthAddr());
    enc->EncodeAdd(reg2, reg2, reg1);
    enc->EncodeStr(reg2, args.SbLengthAddr());
    // Insert the string into the slot:
    // - reg0 contains an address of the slot
    // - release temps for barriers
    enc->BindLabel(labelInsertStringIntoSlot);
    tmp1.Release();
    tmp2.Release();
    EncodeSbInsertStringIntoSlot(cg, inst, reg0, args);
    // Return the reference to StringBuilder
    enc->BindLabel(labelFastPathDone);
    enc->EncodeMov(args.Dst(), args.Builder());
    enc->EncodeJump(labelReturn);
}

void Codegen::CreateStringBuilderAppendString(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    using StringLengthType = std::result_of<decltype (&coretypes::String::GetLength)(coretypes::String)>::type;
    static_assert(TypeInfo::GetScalarTypeBySize(sizeof(ark::ArraySizeT) * CHAR_BIT) == INT32_TYPE);
    static_assert(TypeInfo::GetScalarTypeBySize(sizeof(StringLengthType) * CHAR_BIT) == INT32_TYPE);
    ASSERT(GetArch() != Arch::AARCH32);
    ASSERT(IsCompressedStringsEnabled());

    auto *enc = GetEncoder();
    auto builder = src[FIRST_OPERAND];
    auto *strInst = inst->GetInput(1).GetInst();
    if (strInst->IsNullPtr()) {
        EncodeSbAppendNullString(this, inst, dst, builder);
        return;
    }
    auto labelReturn = enc->CreateLabel();
    auto labelSlowPath = enc->CreateLabel();
    auto str = src[SECOND_OPERAND];
    if (IsInstNotNull(strInst)) {
        EncodeSbAppendString(this, inst, SbAppendArgs(dst, builder, str), labelReturn, labelSlowPath);
    } else {
        auto labelStrNotNull = enc->CreateLabel();
        enc->EncodeJump(labelStrNotNull, str, Condition::NE);
        EncodeSbAppendNullString(this, inst, dst, builder);
        enc->EncodeJump(labelReturn);
        enc->BindLabel(labelStrNotNull);
        EncodeSbAppendString(this, inst, SbAppendArgs(dst, builder, str), labelReturn, labelSlowPath);
    }
    // Slow path
    static constexpr auto ENTRYPOINT_ID = RuntimeInterface::EntrypointId::STRING_BUILDER_APPEND_STRING;
    enc->BindLabel(labelSlowPath);
    CallRuntime(inst, ENTRYPOINT_ID, dst, {}, builder, str);
    // Return
    enc->BindLabel(labelReturn);
}

RuntimeInterface::EntrypointId GetStringBuilderAppendStringsEntrypointId(uint32_t numArgs, mem::BarrierType barrierType)
{
    using EntrypointId = RuntimeInterface::EntrypointId;
    switch (barrierType) {
        case mem::BarrierType::POST_INTERREGION_BARRIER: {  // G1 GC
            std::array<EntrypointId, 5U> entrypoints {
                EntrypointId::INVALID,  // numArgs = 0
                EntrypointId::INVALID,  // numArgs = 1
                EntrypointId::STRING_BUILDER_APPEND_STRING2_ASYNC,
                EntrypointId::STRING_BUILDER_APPEND_STRING3_ASYNC,
                EntrypointId::STRING_BUILDER_APPEND_STRING4_ASYNC,
            };
            return entrypoints[numArgs];
        }
        default: {  // STW GC
            std::array<EntrypointId, 5U> entrypoints {
                EntrypointId::INVALID,  // numArgs = 0
                EntrypointId::INVALID,  // numArgs = 1
                EntrypointId::STRING_BUILDER_APPEND_STRING2_SYNC,
                EntrypointId::STRING_BUILDER_APPEND_STRING3_SYNC,
                EntrypointId::STRING_BUILDER_APPEND_STRING4_SYNC,
            };
            return entrypoints[numArgs];
        }
    }
}

void Codegen::CreateStringBuilderAppendStrings(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto builder = src[FIRST_OPERAND];
    auto str0 = src[SECOND_OPERAND];
    auto str1 = src[THIRD_OPERAND];
    switch (inst->GetIntrinsicId()) {
        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING2: {
            auto entrypoint = GetStringBuilderAppendStringsEntrypointId(2U, GetGraph()->GetRuntime()->GetPostType());
            CallFastPath(inst, entrypoint, dst, {}, builder, str0, str1);
            break;
        }

        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING3: {
            auto str2 = src[FOURTH_OPERAND];
            auto entrypoint = GetStringBuilderAppendStringsEntrypointId(3U, GetGraph()->GetRuntime()->GetPostType());
            CallFastPath(inst, entrypoint, dst, {}, builder, str0, str1, str2);
            break;
        }

        case RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_SB_APPEND_STRING4: {
            auto str2 = src[FOURTH_OPERAND];
            auto str3 = src[FIFTH_OPERAND];
            auto entrypoint = GetStringBuilderAppendStringsEntrypointId(4U, GetGraph()->GetRuntime()->GetPostType());
            CallFastPath(inst, entrypoint, dst, {}, builder, str0, str1, str2, str3);
            break;
        }

        default:
            UNREACHABLE();
            break;
    }
}

void Codegen::CreateStringConcat([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto allStrings = GetGraph()->GetRuntime()->IsUseAllStrings();
    auto str1 = src[FIRST_OPERAND];
    auto str2 = src[SECOND_OPERAND];

    if (inst->GetIntrinsicId() == RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT2) {
        auto ep = allStrings ? EntrypointId::STRING_CONCAT2_TLAB_ALL_STRINGS : EntrypointId::STRING_CONCAT2_TLAB;
        CallFastPath(inst, ep, dst, {}, str1, str2);
        return;
    }
    if (inst->GetIntrinsicId() == RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT3) {
        auto str3 = src[THIRD_OPERAND];
        CallFastPath(inst, EntrypointId::STRING_CONCAT3_TLAB, dst, {}, str1, str2, str3);
        return;
    }
    if (inst->GetIntrinsicId() == RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_CONCAT4) {
        auto str3 = src[THIRD_OPERAND];
        auto str4 = src[FOURTH_OPERAND];
        CallFastPath(inst, EntrypointId::STRING_CONCAT4_TLAB, dst, {}, str1, str2, str3, str4);
        return;
    }
    UNREACHABLE();
}

void Codegen::CreateStringBuilderToString(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    ASSERT(IsCompressedStringsEnabled());

    auto *enc = GetEncoder();
    auto entrypoint = EntrypointId::STRING_BUILDER_TO_STRING;
    auto sb = src[FIRST_OPERAND];
    if (GetGraph()->IsAotMode()) {
        ScopedTmpReg klass(enc);
        enc->EncodeLdr(klass, false, MemRef(ThreadReg(), GetRuntime()->GetStringClassPointerTlsOffset(GetArch())));
        CallFastPath(inst, entrypoint, dst, {}, sb, klass);
    } else {
        auto klass =
            TypedImm(reinterpret_cast<uintptr_t>(GetRuntime()->GetLineStringClass(GetGraph()->GetMethod(), nullptr)));
        CallFastPath(inst, entrypoint, dst, {}, sb, klass);
    }
}

void Codegen::CreateDoubleToStringDecimal(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    ASSERT(inst->GetInputsCount() == 4U && inst->RequireState());
    auto cache = src[FIRST_OPERAND];
    auto numAsInt = src[SECOND_OPERAND];
    auto unused = src[THIRD_OPERAND];
    auto entrypoint = EntrypointId::DOUBLE_TO_STRING_DECIMAL;
    CallFastPath(inst, entrypoint, dst, {}, cache, numAsInt, unused);
}

void Codegen::CreateFloatIsInteger([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeIsInteger(dst, src[0]);
}

void Codegen::CreateFloatIsSafeInteger([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    GetEncoder()->EncodeIsSafeInteger(dst, src[0]);
}

/* See utf::IsWhiteSpaceChar() for the details */
void Codegen::CreateCharIsWhiteSpace([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    auto entrypoint = RuntimeInterface::EntrypointId::CHAR_IS_WHITE_SPACE;
    auto ch = src[FIRST_OPERAND];
    CallFastPath(inst, entrypoint, dst, {}, ch.As(INT16_TYPE));
}

void Codegen::CreateStringTrimLeft(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto str = src[FIRST_OPERAND];
    auto unused = TypedImm(0);
    // llvm backend needs unused args to call 3-args slow_path from 1-arg fast_path.
    CallFastPath(inst, RuntimeInterface::EntrypointId::STRING_TRIM_LEFT, dst, {}, str, unused, unused);
}

void Codegen::CreateStringTrimRight(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto str = src[FIRST_OPERAND];
    auto unused = TypedImm(0);
    // llvm backend needs unused args to call 3-args slow_path from 1-arg fast_path.
    CallFastPath(inst, RuntimeInterface::EntrypointId::STRING_TRIM_RIGHT, dst, {}, str, unused, unused);
}

void Codegen::CreateStringTrim(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto str = src[FIRST_OPERAND];
    auto unused = TypedImm(0);
    // llvm backend needs unused args to call 3-args slow_path from 1-arg fast_path.
    CallFastPath(inst, RuntimeInterface::EntrypointId::STRING_TRIM, dst, {}, str, unused, unused);
}

void Codegen::CreateStringStartsWith(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto str = src[FIRST_OPERAND];
    auto pfx = src[SECOND_OPERAND];
    auto idx = src[THIRD_OPERAND];
    CallFastPath(inst, RuntimeInterface::EntrypointId::STRING_STARTS_WITH, dst, {}, str, pfx, idx);
}

void Codegen::CreateStringEndsWith(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto str = src[FIRST_OPERAND];
    auto sfx = src[SECOND_OPERAND];
    auto idx = src[THIRD_OPERAND];
    CallFastPath(inst, RuntimeInterface::EntrypointId::STRING_ENDS_WITH, dst, {}, str, sfx, idx);
}

void Codegen::CreateStringGetBytesTlab([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto entrypointId = EntrypointId::STRING_GET_BYTES_TLAB;
    auto runtime = GetGraph()->GetRuntime();
    if (GetGraph()->IsAotMode()) {
        ScopedTmpReg klassReg(GetEncoder());
        GetEncoder()->EncodeLdr(klassReg, false,
                                MemRef(ThreadReg(), runtime->GetArrayU8ClassPointerTlsOffset(GetArch())));
        CallFastPath(inst, entrypointId, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                     klassReg);
    } else {
        auto klassImm = TypedImm(reinterpret_cast<uintptr_t>(runtime->GetArrayU8Class(GetGraph()->GetMethod())));
        CallFastPath(inst, entrypointId, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                     klassImm);
    }
}

void Codegen::CreateStringIndexOf(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto str = src[FIRST_OPERAND];
    auto ch = src[SECOND_OPERAND];
    CallFastPath(inst, RuntimeInterface::EntrypointId::STRING_INDEX_OF, dst, {}, str, ch, GetRegfile()->GetZeroReg());
}

void Codegen::CreateStringIndexOfAfter(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    ASSERT(IntrinsicNeedsParamLocations(inst->GetIntrinsicId()));
    auto *enc = GetEncoder();
    ScopedTmpReg tmpReg(enc);
    auto str = src[FIRST_OPERAND];
    auto ch = src[SECOND_OPERAND];
    auto startIndex = src[THIRD_OPERAND];
    auto tmp = tmpReg.GetReg().As(startIndex.GetType());
    auto zreg = GetRegfile()->GetZeroReg();
    // If 'startIndex' is negative then make it zero.
    // We must not overwrite 'startIndex' register so use a temp register instead.
    enc->EncodeSelect({tmp, startIndex, zreg, startIndex, Imm(0), Condition::GE});
    RegMask preserved(MakeMask(startIndex.GetId()));
    CallFastPath(inst, RuntimeInterface::EntrypointId::STRING_INDEX_OF_AFTER, dst, {preserved}, str, ch, tmp);
    // Irtoc implementation of INDEX_OF_AFTER calls the corresponding INDEX_OF_ variant.
    // That is done for the optimization purpose.
    // So here we must add 'startIndex' to the 'dst' (if it's not equal to '-1')
    auto charNotFoundLabel = enc->CreateLabel();
    enc->EncodeJump(charNotFoundLabel, dst, Imm(-1), Condition::EQ);
    enc->EncodeSelect({tmp, startIndex, zreg, startIndex, Imm(0), Condition::GE});
    enc->EncodeAdd(dst, dst, tmp);
    enc->BindLabel(charNotFoundLabel);
}

void Codegen::CreateStringLastIndexOf(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    constexpr uint32_t LENGTH_START_BIT = common::BaseString::LengthBits::START_BIT;
    constexpr uint32_t LENGTH_BITS_NUM = common::BaseString::STRING_LENGTH_BITS_NUM;
    constexpr auto LENGTH_ZERO = (((uint32_t)0xFFFFFFFF) << LENGTH_BITS_NUM) >> LENGTH_BITS_NUM;
    auto *enc = GetEncoder();
    auto str = src[FIRST_OPERAND];
    auto ch = src[SECOND_OPERAND];
    auto startIndex = src[THIRD_OPERAND];
    auto notFoundLabel = enc->CreateLabel();
    auto callLabel = enc->CreateLabel();
    auto retLabel = enc->CreateLabel();
    // Negative startIndex implies no match, i.e. return -1
    enc->EncodeJump(notFoundLabel, startIndex, Imm(0), Condition::LT);
    // Empty string implies no match, i.e. return -1
    ScopedTmpRegLazy tmpLazyReg(enc);
    auto tmp = INVALID_REGISTER;
    if (dst.GetId() != str.GetId() && dst.GetId() != ch.GetId() && dst.GetId() != startIndex.GetId()) {
        tmp = dst.As(INT32_TYPE);
    } else {
        tmpLazyReg.Acquire();
        tmp = tmpLazyReg.GetReg().As(INT32_TYPE);
    }
    enc->EncodeLdr(tmp, false, MemRef(str, ark::coretypes::STRING_LENGTH_OFFSET));
    enc->EncodeJump(notFoundLabel, tmp, Imm(LENGTH_ZERO), Condition::LS);
    // Make startIndex = str.length - 1 if startIndex >= str.length
    enc->EncodeShr(tmp, tmp, Imm(LENGTH_START_BIT));             // unpack length
    enc->EncodeJump(callLabel, startIndex, tmp, Condition::LO);  // unsigned less
    enc->EncodeSub(startIndex, tmp, Imm(1));
    enc->BindLabel(callLabel);
    CallFastPath(inst, RuntimeInterface::EntrypointId::STRING_LAST_INDEX_OF, dst, {}, str, ch, startIndex);
    enc->EncodeJump(retLabel);
    enc->BindLabel(notFoundLabel);
    enc->EncodeMov(dst, Imm(-1));
    enc->BindLabel(retLabel);
}

void Codegen::CreateStringFromCharCode(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto intrId = inst->GetIntrinsicId();
    ASSERT(intrId == RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_FROM_CHAR_CODE ||
           intrId == RuntimeInterface::IntrinsicId::INTRINSIC_COMPILER_ETS_STRING_FROM_CHAR_CODE_SINGLE_NO_CACHE);
    ASSERT(inst->GetInputsCount() == 2U && inst->RequireState());
    auto eid = intrId == RuntimeInterface::IntrinsicId::INTRINSIC_STD_CORE_STRING_FROM_CHAR_CODE
                   ? EntrypointId::CREATE_STRING_FROM_CHAR_CODE_TLAB
                   : EntrypointId::CREATE_STRING_FROM_CHAR_CODE_SINGLE_NO_CACHE_TLAB;
    auto array = src[FIRST_OPERAND];
    if (GetGraph()->IsAotMode()) {
        ScopedTmpReg klassReg(GetEncoder());
        GetEncoder()->EncodeLdr(
            klassReg, false,
            MemRef(ThreadReg(), static_cast<ssize_t>(GetRuntime()->GetStringClassPointerTlsOffset(GetArch()))));
        CallFastPath(inst, eid, dst, {}, array, klassReg);
    } else {
        auto klassImm =
            TypedImm(reinterpret_cast<uintptr_t>(GetRuntime()->GetLineStringClass(GetGraph()->GetMethod(), nullptr)));
        CallFastPath(inst, eid, dst, {}, array, klassImm);
    }
}

void Codegen::CreateStringFromCharCodeSingle(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(inst->GetInputsCount() == 3U && inst->RequireState());
    ASSERT(GetGraph()->GetRuntime()->IsStringCachesUsed());
    auto cache = src[FIRST_OPERAND];
    auto number = src[SECOND_OPERAND];
    constexpr auto eid = EntrypointId::CREATE_STRING_FROM_CHAR_CODE_SINGLE_TLAB;

    if (GetGraph()->IsAotMode()) {
        ScopedTmpReg klassReg(GetEncoder());
        GetEncoder()->EncodeLdr(
            klassReg, false,
            MemRef(ThreadReg(), static_cast<ssize_t>(GetRuntime()->GetStringClassPointerTlsOffset(GetArch()))));
        CallFastPath(inst, eid, dst, {}, cache, number, klassReg);
    } else {
        auto klassImm =
            TypedImm(reinterpret_cast<uintptr_t>(GetRuntime()->GetLineStringClass(GetGraph()->GetMethod(), nullptr)));
        CallFastPath(inst, eid, dst, {}, cache, number, klassImm);
    }
}

void Codegen::CreateStringRepeat([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(IsCompressedStringsEnabled());
    auto entrypointId = EntrypointId::STRING_REPEAT;
    CallFastPath(inst, entrypointId, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND]);
}

void Codegen::CreateStringCharAt(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetRuntime()->IsCompressedStringsEnabled());
    CallFastPath(inst, EntrypointId::STRING_CHAR_AT, dst, {}, src[0], src[1U]);
}

static RuntimeInterface::EntrypointId SubStringFromStringEntrypoint(RuntimeInterface *runtime)
{
    if (runtime->IsUseAllStrings()) {
        return runtime->IsCompressedStringsEnabled()
                   ? RuntimeInterface::EntrypointId::SUB_STRING_FROM_STRING_TLAB_ALL_STRINGS_COMPRESSED
                   : RuntimeInterface::EntrypointId::SUB_STRING_FROM_STRING_TLAB_ALL_STRINGS;
    }

    return runtime->IsCompressedStringsEnabled()
               ? RuntimeInterface::EntrypointId::SUB_STRING_FROM_STRING_TLAB_COMPRESSED
               : RuntimeInterface::EntrypointId::SUB_STRING_FROM_STRING_TLAB;
}

void Codegen::CreateStringSubstringTlab([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    auto entrypointId = SubStringFromStringEntrypoint(GetRuntime());
    CallFastPath(inst, entrypointId, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND]);
}

void Codegen::CreateInt8ArrayFillInternal(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    auto entrypoint = EntrypointId::INT8_ARRAY_FILL_INTERNAL;
    CallFastPath(inst, entrypoint, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                 src[FOURTH_OPERAND]);
}

void Codegen::CreateInt16ArrayFillInternal(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    auto entrypoint = EntrypointId::INT16_ARRAY_FILL_INTERNAL;
    CallFastPath(inst, entrypoint, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                 src[FOURTH_OPERAND]);
}

void Codegen::CreateInt32ArrayFillInternal(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    auto entrypoint = EntrypointId::INT32_ARRAY_FILL_INTERNAL;
    CallFastPath(inst, entrypoint, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                 src[FOURTH_OPERAND]);
}

void Codegen::CreateBigInt64ArrayFillInternal(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    auto entrypoint = EntrypointId::BIG_INT64_ARRAY_FILL_INTERNAL;
    CallFastPath(inst, entrypoint, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                 src[FOURTH_OPERAND]);
}

void Codegen::CreateFloat32ArrayFillInternal(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    auto valType = inst->GetInputType(SECOND_OPERAND);
    ASSERT(valType == DataType::FLOAT32);
    auto entrypoint = EntrypointId::FLOAT32_ARRAY_FILL_INTERNAL;
    ScopedTmpReg tmp(GetEncoder(), ConvertDataType(DataType::INT32, GetArch()));
    auto val = ConvertRegister(inst->GetSrcReg(SECOND_OPERAND), valType);
    GetEncoder()->EncodeMov(tmp, val);
    CallFastPath(inst, entrypoint, dst, {}, src[FIRST_OPERAND], tmp, src[THIRD_OPERAND], src[FOURTH_OPERAND]);
}

void Codegen::CreateFloat64ArrayFillInternal(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    auto valType = inst->GetInputType(SECOND_OPERAND);
    ASSERT(valType == DataType::FLOAT64);
    auto entrypoint = EntrypointId::FLOAT64_ARRAY_FILL_INTERNAL;
    ScopedTmpReg tmp(GetEncoder(), ConvertDataType(DataType::INT64, GetArch()));
    auto val = ConvertRegister(inst->GetSrcReg(SECOND_OPERAND), valType);
    GetEncoder()->EncodeMov(tmp, val);
    CallFastPath(inst, entrypoint, dst, {}, src[FIRST_OPERAND], tmp, src[THIRD_OPERAND], src[FOURTH_OPERAND]);
}

void Codegen::CreateUInt8ClampedArrayFillInternal(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    auto entrypoint = EntrypointId::U_INT8_CLAMPED_ARRAY_FILL_INTERNAL;
    CallFastPath(inst, entrypoint, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                 src[FOURTH_OPERAND]);
}

void Codegen::CreateUInt8ArrayFillInternal(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    auto entrypoint = EntrypointId::U_INT8_ARRAY_FILL_INTERNAL;
    CallFastPath(inst, entrypoint, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                 src[FOURTH_OPERAND]);
}

void Codegen::CreateUInt16ArrayFillInternal(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    auto entrypoint = EntrypointId::U_INT16_ARRAY_FILL_INTERNAL;
    CallFastPath(inst, entrypoint, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                 src[FOURTH_OPERAND]);
}

void Codegen::CreateUInt32ArrayFillInternal(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    auto entrypoint = EntrypointId::U_INT32_ARRAY_FILL_INTERNAL;
    CallFastPath(inst, entrypoint, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                 src[FOURTH_OPERAND]);
}

void Codegen::CreateBigUInt64ArrayFillInternal(IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    auto entrypoint = EntrypointId::BIG_U_INT64_ARRAY_FILL_INTERNAL;
    CallFastPath(inst, entrypoint, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                 src[FOURTH_OPERAND]);
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CODEGEN_TYPED_ARRAY_SET_VALUES_FROM_ARRAY(defName, defType)                                   \
    /* CC-OFFNXT(G.PRE.02) name part */                                                               \
    void Codegen::Create##defName##ArraySetValuesFromArray(IntrinsicInst *inst, Reg dst, SRCREGS src) \
    {                                                                                                 \
        ASSERT(inst->GetInputsCount() == 3U);                                                         \
        auto eid = EntrypointId::defType##_ARRAY_SET_VALUES_FROM_ARRAY;                               \
        /* CC-OFFNXT(G.PRE.05) function gen */                                                        \
        CallFastPath(inst, eid, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND]);                    \
    }

CODEGEN_TYPED_ARRAY_SET_VALUES_FROM_ARRAY(Int8, INT8)
CODEGEN_TYPED_ARRAY_SET_VALUES_FROM_ARRAY(Int16, INT16)
CODEGEN_TYPED_ARRAY_SET_VALUES_FROM_ARRAY(Int32, INT32)
CODEGEN_TYPED_ARRAY_SET_VALUES_FROM_ARRAY(BigInt64, BIG_INT64)
CODEGEN_TYPED_ARRAY_SET_VALUES_FROM_ARRAY(Float32, FLOAT32)
CODEGEN_TYPED_ARRAY_SET_VALUES_FROM_ARRAY(Float64, FLOAT64)
CODEGEN_TYPED_ARRAY_SET_VALUES_FROM_ARRAY(Uint8, UINT8)
CODEGEN_TYPED_ARRAY_SET_VALUES_FROM_ARRAY(Uint8Clamped, UINT8_CLAMPED)
CODEGEN_TYPED_ARRAY_SET_VALUES_FROM_ARRAY(Uint16, UINT16)
CODEGEN_TYPED_ARRAY_SET_VALUES_FROM_ARRAY(Uint32, UINT32)
CODEGEN_TYPED_ARRAY_SET_VALUES_FROM_ARRAY(BigUint64, BIG_UINT64)

#undef CODEGEN_TYPED_ARRAY_SET_VALUES_FROM_ARRAY

void Codegen::CreateGetHashCodeByValue([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    auto entrypointId = EntrypointId::GET_HASH_CODE_BY_VALUE_FAST_PATH;
    CallFastPath(inst, entrypointId, dst, {}, src[FIRST_OPERAND]);
}

void Codegen::CreateSameValueZero([[maybe_unused]] IntrinsicInst *inst, Reg dst, SRCREGS src)
{
    ASSERT(GetArch() != Arch::AARCH32);
    auto entrypointId = EntrypointId::SAME_VALUE_ZERO_FAST_PATH;
    CallFastPath(inst, entrypointId, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND]);
}

static RuntimeInterface::EntrypointId GetArrayFastCopyToRefEntrypointId(mem::BarrierType barrierType)
{
    using EntrypointId = RuntimeInterface::EntrypointId;
    switch (barrierType) {
        case mem::BarrierType::POST_CMC_WRITE_BARRIER:  // CMC GC
            return EntrypointId::ARRAY_FAST_COPY_TO_REF_HYBRID;
        case mem::BarrierType::POST_INTERREGION_BARRIER:  // G1 GC
            return EntrypointId::ARRAY_FAST_COPY_TO_REF_ASYNC;
        default:  // STW GC
            return EntrypointId::ARRAY_FAST_COPY_TO_REF_SYNC;
    }
}

void Codegen::CreateArrayFastCopyToRef(IntrinsicInst *inst, [[maybe_unused]] Reg dst, SRCREGS src)
{
    ASSERT(IntrinsicNeedsParamLocations(inst->GetIntrinsicId()));

    auto srcObj = src[FIRST_OPERAND];
    auto dstObj = src[SECOND_OPERAND];
    auto dstStart = src[THIRD_OPERAND];
    auto srcStart = src[FOURTH_OPERAND];
    auto srcEnd = src[FIFTH_OPERAND];

    ScopedTmpReg tmpReg(enc_);
    auto tmp = tmpReg.GetReg().As(dstStart.GetType());
    enc_->EncodeSub(tmp, srcEnd, srcStart);
    // Check whether the source range less or equal to what fits in a single GC card table. If not, go to the
    // slow path.
    static constexpr size_t IN_ONE_CARD_TABLE_PAGE_ITERATION_THRESHOLD =
        mem::CardTable::GetCardSize() / sizeof(ObjectPointerType);
    auto labelSlowPath = enc_->CreateLabel();
    auto labelEnd = enc_->CreateLabel();
    enc_->EncodeJump(labelSlowPath, tmp, Imm(IN_ONE_CARD_TABLE_PAGE_ITERATION_THRESHOLD), Condition::GT);
    auto fastPathEntrypointId = GetArrayFastCopyToRefEntrypointId(GetGraph()->GetRuntime()->GetPostType());
    CallFastPath(inst, fastPathEntrypointId, INVALID_REGISTER, {}, srcObj, dstObj, dstStart, srcStart, srcEnd);
    enc_->EncodeJump(labelEnd);

    enc_->BindLabel(labelSlowPath);
    static constexpr auto SLOW_PATH_ENTRYPOINT_ID = EntrypointId::ARRAY_FAST_COPY_TO_REF_ENTRYPOINT;
    CallRuntime(inst, SLOW_PATH_ENTRYPOINT_ID, INVALID_REGISTER, {}, srcObj, dstObj, dstStart, srcStart, srcEnd);
    enc_->BindLabel(labelEnd);
}

void Codegen::CreateEtsStringEquals(IntrinsicInst *inst, [[maybe_unused]] Reg dst, SRCREGS src)
{
    ASSERT(inst->GetInputsCount() == 3U && inst->RequireState());
    CallFastPath(inst, EntrypointId::ETS_STRING_EQUALS, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND]);
}

void Codegen::CreateStringToCase(IntrinsicInst *inst, RuntimeInterface::EntrypointId fastPathEntrypointId,
                                 RuntimeInterface::EntrypointId slowPathEntrypointId, Reg dst, Reg srcStr)
{
    auto labelSlowPath = enc_->CreateLabel();
    auto labelEnd = enc_->CreateLabel();

    const auto etsBoolType = INT8_TYPE;
    auto [live_regs, live_vregs] {GetLiveRegisters<false>(inst)};
    auto ret {GetTarget().GetReturnReg(etsBoolType)};
    live_regs.reset(ret.GetId());
    live_regs.set(srcStr.GetId());

    SaveCallerRegisters(live_regs, live_vregs, true);
    // The intrinsic has no parameters, so a call to FillCallParams() is not needed.
    EmitCallRuntimeCode(nullptr, EntrypointId::ETS_DEFAULT_LOCALE_ALLOWS_FAST_LATIN_CASE_CONVERSION);

    {
        ScopedTmpRegLazy tmpReg(enc_, false);
        if (srcStr.GetId() == ret.GetId()) {
            tmpReg.Acquire();
            auto tmp = tmpReg.GetReg().As(ret.GetType());
            enc_->EncodeMov(tmp, ret);
            ret = tmp;
        }
        LoadCallerRegisters(live_regs, live_vregs, true);
        enc_->EncodeJump(labelSlowPath, ret, EQ);
    }

    CallFastPath(inst, fastPathEntrypointId, dst, {}, srcStr);
    enc_->EncodeJump(labelEnd);

    enc_->BindLabel(labelSlowPath);
    CallRuntime(inst, slowPathEntrypointId, dst, {}, srcStr);
    enc_->BindLabel(labelEnd);
}

void Codegen::CreateStringToLowerCase(IntrinsicInst *inst, [[maybe_unused]] Reg dst, SRCREGS src)
{
    ASSERT(inst->GetInputsCount() == 2U && inst->RequireState());
    auto srcStr = src[FIRST_OPERAND];
    CreateStringToCase(inst, EntrypointId::STRING_TO_LOWER_CASE_TLAB, EntrypointId::STRING_TO_LOWER_CASE_ENTRYPOINT,
                       dst, srcStr);
}

void Codegen::CreateStringToUpperCase(IntrinsicInst *inst, [[maybe_unused]] Reg dst, SRCREGS src)
{
    ASSERT(inst->GetInputsCount() == 2U && inst->RequireState());
    auto srcStr = src[FIRST_OPERAND];
    CreateStringToCase(inst, EntrypointId::STRING_TO_UPPER_CASE_TLAB, EntrypointId::STRING_TO_UPPER_CASE_ENTRYPOINT,
                       dst, srcStr);
}

void Codegen::CreateStringToLocaleLowerCase(IntrinsicInst *inst, [[maybe_unused]] Reg dst, SRCREGS src)
{
    ASSERT(inst->GetInputsCount() == 3U && inst->RequireState());
    CallFastPath(inst, EntrypointId::STRING_TO_LOCALE_LOWER_CASE_TLAB, dst, {}, src[FIRST_OPERAND],
                 src[SECOND_OPERAND]);
}

void Codegen::CreateStringToLocaleUpperCase(IntrinsicInst *inst, [[maybe_unused]] Reg dst, SRCREGS src)
{
    ASSERT(inst->GetInputsCount() == 3U && inst->RequireState());
    CallFastPath(inst, EntrypointId::STRING_TO_LOCALE_UPPER_CASE_TLAB, dst, {}, src[FIRST_OPERAND],
                 src[SECOND_OPERAND]);
}

void Codegen::CreateArrayCopyWithin(IntrinsicInst *inst, [[maybe_unused]] Reg dst, SRCREGS src)
{
    auto entrypoint = GetEntrypointByIntrinsicId(inst->GetIntrinsicId());
    CallFastPath(inst, entrypoint, dst, {}, src[FIRST_OPERAND], src[SECOND_OPERAND], src[THIRD_OPERAND],
                 src[FOURTH_OPERAND]);
}

}  // namespace ark::compiler
