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

#include "ir_static.h"

#include "datatype.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/statuses.h"
#include "libabckit/src/adapter_static/helpers_static.h"

#include "libabckit/c/metadata_core.h"
#include "libabckit/src/statuses_impl.h"
#include "libabckit/src/macros.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/helpers_common.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/wrappers/pandasm_wrapper.h"

#include "static_core/assembler/mangling.h"

#include "static_core/compiler/optimizer/ir/graph.h"
#include "static_core/compiler/optimizer/ir/basicblock.h"
#include "static_core/compiler/optimizer/ir/inst.h"
#include "static_core/compiler/optimizer/analysis/loop_analyzer.h"

#include <cstdarg>
#include <cstdint>
#include <limits>

namespace {
constexpr uint32_t IC_SLOT_VALUE = 0xF;
}  // namespace

namespace libabckit {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark;

AbckitInst *IcreateDynCallthis0Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_CALLTHIS0_IMM8_V8);
}

AbckitInst *IcreateDynCallthis1Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, input1, compiler::IntrinsicInst::IntrinsicId::DYN_CALLTHIS1_IMM8_V8_V8);
}

AbckitInst *IcreateDynCallarg0Static(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_CALLARG0_IMM8);
}

AbckitInst *IcreateDynCallarg1Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_CALLARG1_IMM8_V8);
}

AbckitInst *IcreateDynTryldglobalbynameStatic(AbckitGraph *graph, AbckitString *string)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, GetStringOffset(graph, string),
                         compiler::IntrinsicInst::IntrinsicId::DYN_TRYLDGLOBALBYNAME_IMM16_ID16);
}

AbckitInst *IcreateDynLdobjbynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, GetStringOffset(graph, string),
                         compiler::IntrinsicInst::IntrinsicId::DYN_LDOBJBYNAME_IMM8_ID16);
}

AbckitInst *IcreateDynNewobjrangeStatic(AbckitGraph *graph, size_t argCount, std::va_list args)
{
    LIBABCKIT_LOG_FUNC;
    auto argCountBitSize = GetBitLengthUnsigned(argCount);
    if (argCountBitSize > AbckitBitImmSize::BITSIZE_8) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, argCount, args, compiler::IntrinsicInst::IntrinsicId::DYN_NEWOBJRANGE_IMM16_IMM8_V8);
}

AbckitInst *IcreateDynDefinefuncStatic(AbckitGraph *graph, AbckitCoreFunction *function, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_8) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, GetMethodOffset(graph, function), imm0,
                         compiler::IntrinsicInst::IntrinsicId::DYN_DEFINEFUNC_IMM16_ID16_IMM8);
}

AbckitInst *IcreateDynNegStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_NEG_IMM8);
}

AbckitInst *IcreateDynAdd2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_ADD2_IMM8_V8);
}

AbckitInst *IcreateDynSub2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_SUB2_IMM8_V8);
}

AbckitInst *IcreateDynMul2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_MUL2_IMM8_V8);
}

AbckitInst *IcreateDynDiv2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_DIV2_IMM8_V8);
}

AbckitInst *IcreateDynMod2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_MOD2_IMM8_V8);
}

AbckitInst *IcreateDynExpStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_EXP_IMM8_V8);
}

AbckitInst *IcreateDynShl2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_SHL2_IMM8_V8);
}

AbckitInst *IcreateDynShr2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_SHR2_IMM8_V8);
}

AbckitInst *IcreateDynAshr2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_ASHR2_IMM8_V8);
}

AbckitInst *IcreateDynNotStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_NOT_IMM8);
}

AbckitInst *IcreateDynOr2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_OR2_IMM8_V8);
}

AbckitInst *IcreateDynXor2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_XOR2_IMM8_V8);
}

AbckitInst *IcreateDynAnd2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_AND2_IMM8_V8);
}

AbckitInst *IcreateDynIncStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_INC_IMM8);
}

AbckitInst *IcreateDynDecStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_DEC_IMM8);
}

AbckitInst *IcreateDynEqStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_EQ_IMM8_V8);
}

AbckitInst *IcreateDynNoteqStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_NOTEQ_IMM8_V8);
}

AbckitInst *IcreateDynLessStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_LESS_IMM8_V8);
}

AbckitInst *IcreateDynLesseqStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_LESSEQ_IMM8_V8);
}

AbckitInst *IcreateDynGreaterStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_GREATER_IMM8_V8);
}

AbckitInst *IcreateDynGreatereqStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_GREATEREQ_IMM8_V8);
}

AbckitInst *IcreateDynStrictnoteqStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_STRICTNOTEQ_IMM8_V8);
}

AbckitInst *IcreateDynStricteqStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_STRICTEQ_IMM8_V8);
}

AbckitInst *IcreateDynIstrueStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_ISTRUE);
}

AbckitInst *IcreateDynIsfalseStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_ISFALSE);
}

AbckitInst *IcreateDynTonumberStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_TONUMBER_IMM8);
}

AbckitInst *IcreateDynTonumericStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_TONUMERIC_IMM8);
}

AbckitInst *IcreateDynThrowStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_THROW_PREF_NONE);
}

AbckitInst *IcreateDynThrowNotexistsStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_THROW_NOTEXISTS_PREF_NONE);
}

AbckitInst *IcreateDynThrowPatternnoncoercibleStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_THROW_PATTERNNONCOERCIBLE_PREF_NONE);
}

AbckitInst *IcreateDynThrowDeletesuperpropertyStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_THROW_DELETESUPERPROPERTY_PREF_NONE);
}

AbckitInst *IcreateDynThrowConstassignmentStatic(AbckitGraph *graph, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, input0, compiler::IntrinsicInst::IntrinsicId::DYN_THROW_CONSTASSIGNMENT_PREF_V8);
}

AbckitInst *IcreateDynThrowIfnotobjectStatic(AbckitGraph *graph, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, input0, compiler::IntrinsicInst::IntrinsicId::DYN_THROW_IFNOTOBJECT_PREF_V8);
}

AbckitInst *IcreateDynThrowUndefinedifholeStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, input0, input1,
                         compiler::IntrinsicInst::IntrinsicId::DYN_THROW_UNDEFINEDIFHOLE_PREF_V8_V8);
}

AbckitInst *IcreateDynThrowIfsupernotcorrectcallStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    AbckitInst *inst {nullptr};
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize <= AbckitBitImmSize::BITSIZE_8) {
        inst = CreateDynInst(graph, acc, imm0,
                             compiler::IntrinsicInst::IntrinsicId::DYN_THROW_IFSUPERNOTCORRECTCALL_PREF_IMM8, false);
    } else if (imm0BitSize <= AbckitBitImmSize::BITSIZE_16) {
        inst = CreateDynInst(graph, acc, imm0,
                             compiler::IntrinsicInst::IntrinsicId::DYN_THROW_IFSUPERNOTCORRECTCALL_PREF_IMM16, false);
    } else {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
    }
    return inst;
}

AbckitInst *IcreateDynThrowUndefinedifholewithnameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, GetStringOffset(graph, string),
                         compiler::IntrinsicInst::IntrinsicId::DYN_THROW_UNDEFINEDIFHOLEWITHNAME_PREF_ID16, false);
}

AbckitInst *IcreateDynCallruntimeCreateprivatepropertyStatic(AbckitGraph *graph, uint64_t imm0,
                                                             AbckitLiteralArray *literalArray)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, imm0, GetLiteralArrayOffset(graph, literalArray),
                         compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_CREATEPRIVATEPROPERTY_PREF_IMM16_ID16,
                         false);
}

AbckitInst *IcreateDynDefineclasswithbufferStatic(AbckitGraph *graph, AbckitCoreFunction *function,
                                                  AbckitLiteralArray *literalArray, uint64_t imm0, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, GetMethodOffset(graph, function), GetLiteralArrayOffset(graph, literalArray), imm0,
                         input0,
                         compiler::IntrinsicInst::IntrinsicId::DYN_DEFINECLASSWITHBUFFER_IMM16_ID16_ID16_IMM16_V8);
    ;
}

AbckitInst *IcreateDynCallruntimeDefinesendableclassStatic(AbckitGraph *graph, AbckitCoreFunction *function,
                                                           AbckitLiteralArray *literalArray, uint64_t imm0,
                                                           AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(
        graph, GetMethodOffset(graph, function), GetLiteralArrayOffset(graph, literalArray), imm0, input0,
        compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_DEFINESENDABLECLASS_PREF_IMM16_ID16_ID16_IMM16_V8);
}

AbckitInst *IcreateDynLdnanStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_LDNAN);
}

AbckitInst *IcreateDynLdinfinityStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_LDINFINITY);
}

AbckitInst *IcreateDynLdundefinedStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_LDUNDEFINED);
}

AbckitInst *IcreateDynLdnullStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_LDNULL);
}

AbckitInst *IcreateDynLdsymbolStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_LDSYMBOL);
}

AbckitInst *IcreateDynLdglobalStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_LDGLOBAL);
}

AbckitInst *IcreateDynLdtrueStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_LDTRUE);
}

AbckitInst *IcreateDynLdfalseStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_LDFALSE);
}

AbckitInst *IcreateDynLdholeStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_LDHOLE);
}

AbckitInst *IcreateDynLdfunctionStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_LDFUNCTION);
}

AbckitInst *IcreateDynLdnewtargetStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_LDNEWTARGET);
}

AbckitInst *IcreateDynLdthisStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_LDTHIS);
}

AbckitInst *IcreateDynTypeofStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_TYPEOF_IMM16);
}

AbckitInst *IcreateDynIsinStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_ISIN_IMM8_V8);
}

AbckitInst *IcreateDynInstanceofStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_INSTANCEOF_IMM8_V8);
}

AbckitInst *IcreateDynReturnStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_RETURN);
}

AbckitInst *IcreateDynReturnundefinedStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_RETURNUNDEFINED);
}

AbckitInst *IcreateDynStownbynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, GetStringOffset(graph, string),
                         compiler::IntrinsicInst::IntrinsicId::DYN_STOWNBYNAME_IMM16_ID16_V8);
}

AbckitInst *IcreateDynStownbyvalueStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, input1,
                         compiler::IntrinsicInst::IntrinsicId::DYN_STOWNBYVALUE_IMM16_V8_V8);
}

AbckitInst *IcreateDynStownbyindexStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, imm0,
                         compiler::IntrinsicInst::IntrinsicId::DYN_STOWNBYINDEX_IMM16_V8_IMM16);
}

AbckitInst *IcreateDynWideStownbyindexStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_32) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, imm0,
                         compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_STOWNBYINDEX_PREF_V8_IMM32, false);
}

AbckitInst *IcreateDynStownbyvaluewithnamesetStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                    AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, input1,
                         compiler::IntrinsicInst::IntrinsicId::DYN_STOWNBYVALUEWITHNAMESET_IMM16_V8_V8);
}

AbckitInst *IcreateDynStownbynamewithnamesetStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                                   AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, GetStringOffset(graph, string),
                         compiler::IntrinsicInst::IntrinsicId::DYN_STOWNBYNAMEWITHNAMESET_IMM16_ID16_V8);
}

AbckitInst *IcreateDynCreateemptyobjectStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_CREATEEMPTYOBJECT);
}

AbckitInst *IcreateDynStobjbyvalueStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, input1,
                         compiler::IntrinsicInst::IntrinsicId::DYN_STOBJBYVALUE_IMM16_V8_V8);
}

AbckitInst *IcreateDynStobjbyindexStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, acc, input0, imm0,
                         compiler::IntrinsicInst::IntrinsicId::DYN_STOBJBYINDEX_IMM16_V8_IMM16);
}

AbckitInst *IcreateDynWideStobjbyindexStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_32) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, imm0,
                         compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_STOBJBYINDEX_PREF_V8_IMM32, false);
}

AbckitInst *IcreateDynStobjbynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, GetStringOffset(graph, string),
                         compiler::IntrinsicInst::IntrinsicId::DYN_STOBJBYNAME_IMM16_ID16_V8);
}

AbckitInst *IcreateDynPoplexenvStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_POPLEXENV, false);
}

AbckitInst *IcreateDynGetunmappedargsStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_GETUNMAPPEDARGS, false);
}

AbckitInst *IcreateDynAsyncfunctionenterStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_ASYNCFUNCTIONENTER, false);
}

AbckitInst *IcreateDynDebuggerStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_DEBUGGER, false);
}

AbckitInst *IcreateDynCreateemptyarrayStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, compiler::IntrinsicInst::IntrinsicId::DYN_CREATEEMPTYARRAY_IMM16);
}

AbckitInst *IcreateDynGetpropiteratorStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_GETPROPITERATOR);
}

AbckitInst *IcreateDynGetiteratorStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_GETITERATOR_IMM16);
}

AbckitInst *IcreateDynGetasynciteratorStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_GETASYNCITERATOR_IMM8);
}

AbckitInst *IcreateDynCreategeneratorobjStatic(AbckitGraph *graph, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, input0, compiler::IntrinsicInst::IntrinsicId::DYN_CREATEGENERATOROBJ_V8);
}

AbckitInst *IcreateDynCreateasyncgeneratorobjStatic(AbckitGraph *graph, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, input0, compiler::IntrinsicInst::IntrinsicId::DYN_CREATEASYNCGENERATOROBJ_V8);
}

AbckitInst *IcreateDynCallruntimeNotifyconcurrentresultStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc,
                         compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_NOTIFYCONCURRENTRESULT_PREF_NONE);
}

AbckitInst *IcreateDynCallruntimeTopropertykeyStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_TOPROPERTYKEY_PREF_NONE);
}

AbckitInst *IcreateDynResumegeneratorStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_RESUMEGENERATOR);
}

AbckitInst *IcreateDynGetresumemodeStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_GETRESUMEMODE);
}

AbckitInst *IcreateDynGettemplateobjectStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_GETTEMPLATEOBJECT_IMM16);
}

AbckitInst *IcreateDynGetnextpropnameStatic(AbckitGraph *graph, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, input0, compiler::IntrinsicInst::IntrinsicId::DYN_GETNEXTPROPNAME_V8, false);
}

AbckitInst *IcreateDynLdthisbyvalueStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_LDTHISBYVALUE_IMM16);
}

AbckitInst *IcreateDynDynamicimportStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_DYNAMICIMPORT);
}

//* Binary instructions *//

AbckitInst *IcreateDynCreateiterresultobjStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, input0, input1, compiler::IntrinsicInst::IntrinsicId::DYN_CREATEITERRESULTOBJ_V8_V8);
}

AbckitInst *IcreateDynNewobjapplyStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_NEWOBJAPPLY_IMM16_V8);
}

AbckitInst *IcreateDynCallruntimeCallinitStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0,
                         compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_CALLINIT_PREF_IMM8_V8);
}

AbckitInst *IcreateDynSupercallspreadStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_SUPERCALLSPREAD_IMM8_V8);
}

AbckitInst *IcreateDynDelobjpropStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_DELOBJPROP_V8);
}

AbckitInst *IcreateDynSuspendgeneratorStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_SUSPENDGENERATOR_V8);
}

AbckitInst *IcreateDynAsyncfunctionawaituncaughtStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_ASYNCFUNCTIONAWAITUNCAUGHT_V8);
}

AbckitInst *IcreateDynCopydatapropertiesStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_COPYDATAPROPERTIES_V8);
}

AbckitInst *IcreateDynSetobjectwithprotoStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_SETOBJECTWITHPROTO_IMM16_V8);
}

AbckitInst *IcreateDynLdobjbyvalueStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_LDOBJBYVALUE_IMM16_V8);
}

AbckitInst *IcreateDynLdsuperbyvalueStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_LDSUPERBYVALUE_IMM16_V8);
}

AbckitInst *IcreateDynAsyncfunctionresolveStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_ASYNCFUNCTIONRESOLVE_V8);
}

AbckitInst *IcreateDynAsyncfunctionrejectStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_ASYNCFUNCTIONREJECT_V8);
}

AbckitInst *IcreateDynStthisbyvalueStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_STTHISBYVALUE_IMM16_V8);
}

AbckitInst *IcreateDynAsyncgeneratorrejectStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, compiler::IntrinsicInst::IntrinsicId::DYN_ASYNCGENERATORREJECT_V8);
}

//* Ternary instructions *//

AbckitInst *IcreateDynCallargs2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, input1, compiler::IntrinsicInst::IntrinsicId::DYN_CALLARGS2_IMM8_V8_V8);
}

AbckitInst *IcreateDynApplyStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, input1, compiler::IntrinsicInst::IntrinsicId::DYN_APPLY_IMM8_V8_V8);
}

AbckitInst *IcreateDynStarrayspreadStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, input1, compiler::IntrinsicInst::IntrinsicId::DYN_STARRAYSPREAD_V8_V8);
}

AbckitInst *IcreateDynStsuperbyvalueStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, input1,
                         compiler::IntrinsicInst::IntrinsicId::DYN_STSUPERBYVALUE_IMM16_V8_V8);
}

AbckitInst *IcreateDynAsyncgeneratorresolveStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                                  AbckitInst *input2)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, input0, input1, input2,
                         compiler::IntrinsicInst::IntrinsicId::DYN_ASYNCGENERATORRESOLVE_V8_V8_V8);
}

AbckitInst *IcreateDynCallruntimeDefinefieldbyvalueStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                          AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, input1,
                         compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_DEFINEFIELDBYVALUE_PREF_IMM8_V8_V8);
}

AbckitInst *IcreateDynDefinefieldbynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                              AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, GetStringOffset(graph, string),
                         compiler::IntrinsicInst::IntrinsicId::DYN_DEFINEFIELDBYNAME_IMM8_ID16_V8);
}

AbckitInst *IcreateDynDefinepropertybynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                                 AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, GetStringOffset(graph, string),
                         compiler::IntrinsicInst::IntrinsicId::DYN_DEFINEPROPERTYBYNAME_IMM8_ID16_V8);
}

AbckitInst *IcreateDynStsuperbynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, GetStringOffset(graph, string),
                         compiler::IntrinsicInst::IntrinsicId::DYN_STSUPERBYNAME_IMM16_ID16_V8);
}

AbckitInst *IcreateDynCreateobjectwithexcludedkeysStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                                         uint64_t imm0, std::va_list args)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_8) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, input0, input1, imm0, args,
                         compiler::IntrinsicInst::IntrinsicId::DYN_CREATEOBJECTWITHEXCLUDEDKEYS_IMM8_V8_V8, false);
}

AbckitInst *IcreateDynWideCreateobjectwithexcludedkeysStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                                             uint64_t imm0, std::va_list args)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, input0, input1, imm0, args,
                         compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_CREATEOBJECTWITHEXCLUDEDKEYS_PREF_IMM16_V8_V8,
                         false);
}

AbckitInst *IcreateDynCallruntimeDefinefieldbyindexStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                          AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_32) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, imm0,
                         compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_DEFINEFIELDBYINDEX_PREF_IMM8_IMM32_V8);
}

AbckitInst *IcreateDynCallthisrangeStatic(AbckitGraph *graph, AbckitInst *acc, size_t argCount, std::va_list args)
{
    LIBABCKIT_LOG_FUNC;
    auto argCountBitSize = GetBitLengthUnsigned(argCount);
    if (argCountBitSize > AbckitBitImmSize::BITSIZE_8) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    auto intrImpl = graph->impl->CreateInstIntrinsic(
        compiler::DataType::ANY, 0, compiler::IntrinsicInst::IntrinsicId::DYN_CALLTHISRANGE_IMM8_IMM8_V8);
    size_t argsCount {argCount + 2};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(acc->impl, compiler::DataType::ANY);
    for (size_t index = 0; index < argCount + 1; ++index) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        AbckitInst *input = va_arg(args, AbckitInst *);
        intrImpl->AppendInput(input->impl, compiler::DataType::ANY);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    intrImpl->AddImm(graph->impl->GetAllocator(), argCount);
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *IcreateDynWideCallthisrangeStatic(AbckitGraph *graph, AbckitInst *acc, size_t argCount, std::va_list args)
{
    LIBABCKIT_LOG_FUNC;
    auto argCountBitSize = GetBitLengthUnsigned(argCount);
    if (argCountBitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    auto intrImpl = graph->impl->CreateInstIntrinsic(
        compiler::DataType::ANY, 0, compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_CALLTHISRANGE_PREF_IMM16_V8);
    size_t argsCount {argCount + 2};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(acc->impl, compiler::DataType::ANY);
    for (size_t index = 0; index < argCount + 1; ++index) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        AbckitInst *input = va_arg(args, AbckitInst *);
        intrImpl->AppendInput(input->impl, compiler::DataType::ANY);
    }
    intrImpl->AddImm(graph->impl->GetAllocator(), argCount);
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *IcreateDynSupercallarrowrangeStatic(AbckitGraph *graph, AbckitInst *acc, size_t argCount, std::va_list args)
{
    LIBABCKIT_LOG_FUNC;
    auto argCountBitSize = GetBitLengthUnsigned(argCount);
    if (argCountBitSize > AbckitBitImmSize::BITSIZE_8) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, argCount, args,
                         compiler::IntrinsicInst::IntrinsicId::DYN_SUPERCALLARROWRANGE_IMM8_IMM8_V8);
}

AbckitInst *IcreateDynCallrangeStatic(AbckitGraph *graph, AbckitInst *acc, size_t argCount, std::va_list args)
{
    LIBABCKIT_LOG_FUNC;
    auto argCountBitSize = GetBitLengthUnsigned(argCount);
    if (argCountBitSize > AbckitBitImmSize::BITSIZE_8) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, argCount, args, compiler::IntrinsicInst::IntrinsicId::DYN_CALLRANGE_IMM8_IMM8_V8);
}

AbckitInst *IcreateDynWideCallrangeStatic(AbckitGraph *graph, AbckitInst *acc, size_t argCount, std::va_list args)
{
    LIBABCKIT_LOG_FUNC;
    auto argCountBitSize = GetBitLengthUnsigned(argCount);
    if (argCountBitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, argCount, args,
                         compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_CALLRANGE_PREF_IMM16_V8, false);
}

AbckitInst *IcreateDynWideSupercallarrowrangeStatic(AbckitGraph *graph, AbckitInst *acc, size_t argCount,
                                                    std::va_list args)
{
    LIBABCKIT_LOG_FUNC;
    auto argCountBitSize = GetBitLengthUnsigned(argCount);
    if (argCountBitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, argCount, args,
                         compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_SUPERCALLARROWRANGE_PREF_IMM16_V8, false);
}

AbckitInst *IcreateDynStprivatepropertyStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1,
                                              AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    auto imm1BitSize = GetBitLengthUnsigned(imm1);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16 || imm1BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, imm0, imm1,
                         compiler::IntrinsicInst::IntrinsicId::DYN_STPRIVATEPROPERTY_IMM8_IMM16_IMM16_V8);
}
AbckitInst *IcreateDynCallruntimeDefineprivatepropertyStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                             uint64_t imm1, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    auto imm1BitSize = GetBitLengthUnsigned(imm1);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16 || imm1BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(
        graph, acc, input0, imm0, imm1,
        compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_DEFINEPRIVATEPROPERTY_PREF_IMM8_IMM16_IMM16_V8);
}

AbckitInst *IcreateDynCreatearraywithbufferStatic(AbckitGraph *graph, AbckitLiteralArray *literalArray)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, GetLiteralArrayOffset(graph, literalArray),
                         compiler::IntrinsicInst::IntrinsicId::DYN_CREATEARRAYWITHBUFFER_IMM16_ID16);
}

AbckitInst *IcreateDynCreateobjectwithbufferStatic(AbckitGraph *graph, AbckitLiteralArray *literalArray)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, GetLiteralArrayOffset(graph, literalArray),
                         compiler::IntrinsicInst::IntrinsicId::DYN_CREATEOBJECTWITHBUFFER_IMM16_ID16);
}

AbckitInst *IcreateDynCallargs3Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                                      AbckitInst *input2)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, input1, input2,
                         compiler::IntrinsicInst::IntrinsicId::DYN_CALLARGS3_IMM8_V8_V8_V8);
}

AbckitInst *IcreateDynCallthis2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                                      AbckitInst *input2)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, input1, input2,
                         compiler::IntrinsicInst::IntrinsicId::DYN_CALLTHIS2_IMM8_V8_V8_V8);
}

// CC-OFFNXT(G.FUN.01) function args are necessary
AbckitInst *IcreateDynCallthis3Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                                      AbckitInst *input2, AbckitInst *input3)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, input1, input2, input3,
                         compiler::IntrinsicInst::IntrinsicId::DYN_CALLTHIS3_IMM8_V8_V8_V8_V8);
}

// CC-OFFNXT(G.FUN.01) function args are necessary
AbckitInst *IcreateDynDefinegettersetterbyvalueStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                      AbckitInst *input1, AbckitInst *input2, AbckitInst *input3)
{
    LIBABCKIT_LOG_FUNC;
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, acc, input0, input1, input2, input3,
                         compiler::IntrinsicInst::IntrinsicId::DYN_DEFINEGETTERSETTERBYVALUE_V8_V8_V8_V8);
}

AbckitInst *IcreateDynStlexvarStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    LIBABCKIT_LOG_FUNC;
    AbckitInst *inst {nullptr};
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    auto imm1BitSize = GetBitLengthUnsigned(imm1);
    if (imm0BitSize <= AbckitBitImmSize::BITSIZE_4 && imm1BitSize <= AbckitBitImmSize::BITSIZE_4) {
        inst =
            CreateDynInst(graph, acc, imm0, imm1, compiler::IntrinsicInst::IntrinsicId::DYN_STLEXVAR_IMM4_IMM4, false);
    } else if (imm0BitSize <= AbckitBitImmSize::BITSIZE_8 && imm1BitSize <= AbckitBitImmSize::BITSIZE_8) {
        inst =
            CreateDynInst(graph, acc, imm0, imm1, compiler::IntrinsicInst::IntrinsicId::DYN_STLEXVAR_IMM8_IMM8, false);
    } else {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
    }
    return inst;
}

AbckitInst *IcreateDynWideStlexvarStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    auto imm1BitSize = GetBitLengthUnsigned(imm1);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16 || imm1BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, acc, imm0, imm1,
                         compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_STLEXVAR_PREF_IMM16_IMM16, false);
}

AbckitInst *IcreateDynTestinStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    auto imm1BitSize = GetBitLengthUnsigned(imm1);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16 || imm1BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, acc, imm0, imm1, compiler::IntrinsicInst::IntrinsicId::DYN_TESTIN_IMM8_IMM16_IMM16);
}

AbckitInst *IcreateDynLdprivatepropertyStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    auto imm1BitSize = GetBitLengthUnsigned(imm1);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16 || imm1BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, acc, imm0, imm1,
                         compiler::IntrinsicInst::IntrinsicId::DYN_LDPRIVATEPROPERTY_IMM8_IMM16_IMM16);
}

AbckitInst *IcreateDynLdlexvarStatic(AbckitGraph *graph, uint64_t imm0, uint64_t imm1)
{
    LIBABCKIT_LOG_FUNC;
    AbckitInst *inst {nullptr};
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    auto imm1BitSize = GetBitLengthUnsigned(imm1);
    if (imm0BitSize <= AbckitBitImmSize::BITSIZE_4 && imm1BitSize <= AbckitBitImmSize::BITSIZE_4) {
        inst = CreateDynInst(graph, imm0, imm1, compiler::IntrinsicInst::IntrinsicId::DYN_LDLEXVAR_IMM4_IMM4, false);
    } else if (imm0BitSize <= AbckitBitImmSize::BITSIZE_8 && imm1BitSize <= AbckitBitImmSize::BITSIZE_8) {
        inst = CreateDynInst(graph, imm0, imm1, compiler::IntrinsicInst::IntrinsicId::DYN_LDLEXVAR_IMM8_IMM8, false);
    } else {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
    }
    return inst;
}
AbckitInst *IcreateDynWideLdlexvarStatic(AbckitGraph *graph, uint64_t imm0, uint64_t imm1)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    auto imm1BitSize = GetBitLengthUnsigned(imm1);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16 || imm1BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, imm0, imm1, compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_LDLEXVAR_PREF_IMM16_IMM16,
                         false);
}

AbckitInst *IcreateDynStthisbynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, GetStringOffset(graph, string),
                         compiler::IntrinsicInst::IntrinsicId::DYN_STTHISBYNAME_IMM16_ID16);
}

AbckitInst *IcreateDynTrystglobalbynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, GetStringOffset(graph, string),
                         compiler::IntrinsicInst::IntrinsicId::DYN_TRYSTGLOBALBYNAME_IMM16_ID16);
}

AbckitInst *IcreateDynStglobalvarStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, GetStringOffset(graph, string),
                         compiler::IntrinsicInst::IntrinsicId::DYN_STGLOBALVAR_IMM16_ID16);
}

AbckitInst *IcreateDynLdsuperbynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, GetStringOffset(graph, string),
                         compiler::IntrinsicInst::IntrinsicId::DYN_LDSUPERBYNAME_IMM16_ID16);
}

AbckitInst *IcreateDynWideStpatchvarStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, acc, imm0, compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_STPATCHVAR_PREF_IMM16, false);
}

AbckitInst *IcreateDynSetgeneratorstateStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_8) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, acc, imm0, compiler::IntrinsicInst::IntrinsicId::DYN_SETGENERATORSTATE_IMM8, false);
}

AbckitInst *IcreateDynLdobjbyindexStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, acc, imm0, compiler::IntrinsicInst::IntrinsicId::DYN_LDOBJBYINDEX_IMM16_IMM16);
    ;
}

AbckitInst *IcreateDynWideLdobjbyindexStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_32) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, acc, imm0, compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_LDOBJBYINDEX_PREF_IMM32,
                         false);
}

AbckitInst *IcreateDynSupercallthisrangeStatic(AbckitGraph *graph, size_t argCount, std::va_list args)
{
    LIBABCKIT_LOG_FUNC;
    auto argCountBitSize = GetBitLengthUnsigned(argCount);
    if (argCountBitSize > AbckitBitImmSize::BITSIZE_8) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, argCount, args,
                         compiler::IntrinsicInst::IntrinsicId::DYN_SUPERCALLTHISRANGE_IMM8_IMM8_V8);
}

AbckitInst *IcreateDynWideSupercallthisrangeStatic(AbckitGraph *graph, size_t argCount, std::va_list args)
{
    LIBABCKIT_LOG_FUNC;
    auto argCountBitSize = GetBitLengthUnsigned(argCount);
    if (argCountBitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, argCount, args,
                         compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_SUPERCALLTHISRANGE_PREF_IMM16_V8, false);
}

AbckitInst *IcreateDynWideNewobjrangeStatic(AbckitGraph *graph, size_t argCount, std::va_list args)
{
    LIBABCKIT_LOG_FUNC;
    auto argCountBitSize = GetBitLengthUnsigned(argCount);
    if (argCountBitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // CC-OFFNXT(WordsTool.190) sensitive word conflict
    return CreateDynInst(graph, argCount, args,
                         compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_NEWOBJRANGE_PREF_IMM16_V8, false);
}

AbckitInst *IcreateDynWideLdpatchvarStatic(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, imm0, compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_LDPATCHVAR_PREF_IMM16, false);
}

AbckitInst *IcreateDynNewlexenvStatic(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_8) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, imm0, compiler::IntrinsicInst::IntrinsicId::DYN_NEWLEXENV_IMM8, false);
}

AbckitInst *IcreateDynWideNewlexenvStatic(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, imm0, compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_NEWLEXENV_PREF_IMM16, false);
}

AbckitInst *IcreateDynNewlexenvwithnameStatic(AbckitGraph *graph, uint64_t imm0, AbckitLiteralArray *literalArray)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_8) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, imm0, GetLiteralArrayOffset(graph, literalArray),
                         compiler::IntrinsicInst::IntrinsicId::DYN_NEWLEXENVWITHNAME_IMM8_ID16, false);
}

AbckitInst *IcreateDynWideNewlexenvwithnameStatic(AbckitGraph *graph, uint64_t imm0, AbckitLiteralArray *literalArray)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, imm0, GetLiteralArrayOffset(graph, literalArray),
                         compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_NEWLEXENVWITHNAME_PREF_IMM16_ID16, false);
}

AbckitInst *IcreateDynCopyrestargsStatic(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_8) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, imm0, compiler::IntrinsicInst::IntrinsicId::DYN_COPYRESTARGS_IMM8, false);
}

AbckitInst *IcreateDynWideCopyrestargsStatic(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, imm0, compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_COPYRESTARGS_PREF_IMM16, false);
}

AbckitInst *IcreateDynCallruntimeLdsendableclassStatic(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, imm0, compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_LDSENDABLECLASS_PREF_IMM16,
                         false);
}

AbckitInst *IcreateDynCallruntimeLdsendableexternalmodulevarStatic(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_8) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, imm0,
                         compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_LDSENDABLEEXTERNALMODULEVAR_PREF_IMM8,
                         false);
}

AbckitInst *IcreateDynCallruntimeWideldsendableexternalmodulevarStatic(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(
        graph, imm0, compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_WIDELDSENDABLEEXTERNALMODULEVAR_PREF_IMM16,
        false);
}

AbckitInst *IcreateDynCallruntimeNewsendableenvStatic(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_8) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, imm0, compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_NEWSENDABLEENV_PREF_IMM8,
                         false);
}

AbckitInst *IcreateDynCallruntimeWidenewsendableenvStatic(AbckitGraph *graph, uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, imm0,
                         compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_WIDENEWSENDABLEENV_PREF_IMM16, false);
}

AbckitInst *IcreateDynCallruntimeStsendablevarStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    LIBABCKIT_LOG_FUNC;
    AbckitInst *inst {nullptr};
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    auto imm1BitSize = GetBitLengthUnsigned(imm1);
    if (imm0BitSize <= AbckitBitImmSize::BITSIZE_4 && imm1BitSize <= AbckitBitImmSize::BITSIZE_4) {
        inst = CreateDynInst(graph, acc, imm0, imm1,
                             compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_STSENDABLEVAR_PREF_IMM4_IMM4, false);
    } else if (imm0BitSize <= AbckitBitImmSize::BITSIZE_8 && imm1BitSize <= AbckitBitImmSize::BITSIZE_8) {
        inst = CreateDynInst(graph, acc, imm0, imm1,
                             compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_STSENDABLEVAR_PREF_IMM8_IMM8, false);
    } else {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
    }
    return inst;
}

AbckitInst *IcreateDynCallruntimeWidestsendablevarStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                         uint64_t imm1)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    auto imm1BitSize = GetBitLengthUnsigned(imm1);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16 || imm1BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, acc, imm0, imm1,
                         compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_WIDESTSENDABLEVAR_PREF_IMM16_IMM16,
                         false);
}

AbckitInst *IcreateDynCallruntimeLdsendablevarStatic(AbckitGraph *graph, uint64_t imm0, uint64_t imm1)
{
    LIBABCKIT_LOG_FUNC;
    AbckitInst *inst {nullptr};
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    auto imm1BitSize = GetBitLengthUnsigned(imm1);
    if (imm0BitSize <= AbckitBitImmSize::BITSIZE_4 && imm1BitSize <= AbckitBitImmSize::BITSIZE_4) {
        inst = CreateDynInst(graph, imm0, imm1,
                             compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_LDSENDABLEVAR_PREF_IMM4_IMM4, false);
    } else if (imm0BitSize <= AbckitBitImmSize::BITSIZE_8 && imm1BitSize <= AbckitBitImmSize::BITSIZE_8) {
        inst = CreateDynInst(graph, imm0, imm1,
                             compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_LDSENDABLEVAR_PREF_IMM8_IMM8, false);
    } else {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
    }
    return inst;
}

AbckitInst *IcreateDynCallruntimeWideldsendablevarStatic(AbckitGraph *graph, uint64_t imm0, uint64_t imm1)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    auto imm1BitSize = GetBitLengthUnsigned(imm1);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_16 || imm1BitSize > AbckitBitImmSize::BITSIZE_16) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    return CreateDynInst(graph, imm0, imm1,
                         compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_WIDELDSENDABLEVAR_PREF_IMM16_IMM16,
                         false);
}

AbckitInst *IcreateDynCallruntimeIstrueStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_ISTRUE_PREF_IMM8);
}

AbckitInst *IcreateDynCallruntimeIsfalseStatic(AbckitGraph *graph, AbckitInst *acc)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, acc, compiler::IntrinsicInst::IntrinsicId::DYN_CALLRUNTIME_ISFALSE_PREF_IMM8);
}

AbckitInst *IcreateDynLdglobalvarStatic(AbckitGraph *graph, AbckitString *string)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, GetStringOffset(graph, string),
                         compiler::IntrinsicInst::IntrinsicId::DYN_LDGLOBALVAR_IMM16_ID16);
}

AbckitInst *IcreateDynLdbigintStatic(AbckitGraph *graph, AbckitString *string)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, GetStringOffset(graph, string), compiler::IntrinsicInst::IntrinsicId::DYN_LDBIGINT_ID16,
                         false);
}

AbckitInst *IcreateDynLdthisbynameStatic(AbckitGraph *graph, AbckitString *string)
{
    LIBABCKIT_LOG_FUNC;
    return CreateDynInst(graph, GetStringOffset(graph, string),
                         compiler::IntrinsicInst::IntrinsicId::DYN_LDTHISBYNAME_IMM16_ID16);
}

AbckitInst *IcreateDynIfStatic(AbckitGraph *graph, AbckitInst *input, AbckitIsaApiDynamicConditionCode cc)
{
    LIBABCKIT_LOG_FUNC;

    if (!((cc == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_NE) || (cc == ABCKIT_ISA_API_DYNAMIC_CONDITION_CODE_CC_EQ))) {
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        LIBABCKIT_LOG(DEBUG) << "IcreateDynIf works only with CC_NE and CC_EQ condidion codes\n";
        return nullptr;
    }

    auto constZeroImpl = graph->impl->FindOrCreateConstant(0L);
    auto *constZeroI = CreateInstFromImpl(graph, constZeroImpl);
    return IcreateIfDynamicStatic(graph, input, constZeroI, cc);
}

AbckitInst *IcreateDynGetmodulenamespaceStatic(AbckitGraph *graph, AbckitCoreModule *md)
{
    LIBABCKIT_LOG_FUNC;

    uint64_t imm = GetModuleIndex(graph, md);
    if (statuses::GetLastError() != ABCKIT_STATUS_NO_ERROR) {
        return nullptr;
    }
    return CreateDynInst(graph, imm, compiler::IntrinsicInst::IntrinsicId::DYN_GETMODULENAMESPACE_IMM8, false);
}

AbckitInst *IcreateDynWideGetmodulenamespaceStatic(AbckitGraph *graph, AbckitCoreModule *md)
{
    LIBABCKIT_LOG_FUNC;

    uint64_t imm = GetModuleIndex(graph, md);
    if (statuses::GetLastError() != ABCKIT_STATUS_NO_ERROR) {
        return nullptr;
    }
    return CreateDynInst(graph, imm, compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_GETMODULENAMESPACE_PREF_IMM16,
                         false);
}

AbckitInst *IcreateDynLdexternalmodulevarStatic(AbckitGraph *graph, AbckitCoreImportDescriptor *id)
{
    LIBABCKIT_LOG_FUNC;

    uint32_t imm = GetImportDescriptorIdxDynamic(graph, id);
    if (statuses::GetLastError() != ABCKIT_STATUS_NO_ERROR) {
        return nullptr;
    }
    return CreateDynInst(graph, imm, compiler::IntrinsicInst::IntrinsicId::DYN_LDEXTERNALMODULEVAR_IMM8, false);
}

AbckitInst *IcreateDynWideLdexternalmodulevarStatic(AbckitGraph *graph, AbckitCoreImportDescriptor *id)
{
    LIBABCKIT_LOG_FUNC;

    uint32_t imm = GetImportDescriptorIdxDynamic(graph, id);
    if (statuses::GetLastError() != ABCKIT_STATUS_NO_ERROR) {
        return nullptr;
    }
    return CreateDynInst(graph, imm, compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_LDEXTERNALMODULEVAR_PREF_IMM16,
                         false);
}

AbckitInst *IcreateDynLdlocalmodulevarStatic(AbckitGraph *graph, AbckitCoreExportDescriptor *ed)
{
    LIBABCKIT_LOG_FUNC;

    uint32_t imm = GetExportDescriptorIdxDynamic(graph, ed);
    if (statuses::GetLastError() != ABCKIT_STATUS_NO_ERROR) {
        return nullptr;
    }
    return CreateDynInst(graph, imm, compiler::IntrinsicInst::IntrinsicId::DYN_LDLOCALMODULEVAR_IMM8, false);
}

AbckitInst *IcreateDynWideLdlocalmodulevarStatic(AbckitGraph *graph, AbckitCoreExportDescriptor *ed)
{
    LIBABCKIT_LOG_FUNC;

    uint32_t imm = GetExportDescriptorIdxDynamic(graph, ed);
    if (statuses::GetLastError() != ABCKIT_STATUS_NO_ERROR) {
        return nullptr;
    }
    return CreateDynInst(graph, imm, compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_LDLOCALMODULEVAR_PREF_IMM16, false);
}

AbckitInst *IcreateDynStmodulevarStatic(AbckitGraph *graph, AbckitInst *acc, AbckitCoreExportDescriptor *ed)
{
    LIBABCKIT_LOG_FUNC;

    uint32_t imm = GetExportDescriptorIdxDynamic(graph, ed);
    if (statuses::GetLastError() != ABCKIT_STATUS_NO_ERROR) {
        return nullptr;
    }
    return CreateDynInst(graph, acc, imm, compiler::IntrinsicInst::IntrinsicId::DYN_STMODULEVAR_IMM8, false);
}

AbckitInst *IcreateDynWideStmodulevarStatic(AbckitGraph *graph, AbckitInst *acc, AbckitCoreExportDescriptor *ed)
{
    LIBABCKIT_LOG_FUNC;

    uint32_t imm = GetExportDescriptorIdxDynamic(graph, ed);
    if (statuses::GetLastError() != ABCKIT_STATUS_NO_ERROR) {
        return nullptr;
    }
    return CreateDynInst(graph, acc, imm, compiler::IntrinsicInst::IntrinsicId::DYN_WIDE_STMODULEVAR_PREF_IMM16, false);
}

AbckitInst *IcreateDynDefinemethodStatic(AbckitGraph *graph, AbckitInst *acc, AbckitCoreFunction *function,
                                         uint64_t imm0)
{
    LIBABCKIT_LOG_FUNC;
    auto imm0BitSize = GetBitLengthUnsigned(imm0);
    if (imm0BitSize > AbckitBitImmSize::BITSIZE_8) {
        LIBABCKIT_LOG(DEBUG) << "Immediate type overflow\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    auto methodOffset = GetMethodOffset(graph, function);
    return CreateDynInst(graph, acc, methodOffset, imm0,
                         compiler::IntrinsicInst::IntrinsicId::DYN_DEFINEMETHOD_IMM16_ID16_IMM8, true);
}

AbckitInst *IcreateNewArrayStatic(AbckitGraph *graph, AbckitCoreClass *inputClass, AbckitInst *inputSize)
{
    LIBABCKIT_LOG_FUNC;

    auto typeId = GetClassOffset(graph, inputClass);
    if (statuses::GetLastError() != ABCKIT_STATUS_NO_ERROR) {
        return nullptr;
    }
    return CreateDynInst(graph, typeId, inputSize, compiler::IntrinsicInst::IntrinsicId::INTRINSIC_ABCKIT_NEW_ARRAY,
                         false);
}

AbckitInst *IcreateNewObjectStatic(AbckitGraph *graph, AbckitCoreClass *inputClass)
{
    LIBABCKIT_LOG_FUNC;

    auto typeId = GetClassOffset(graph, inputClass);
    if (statuses::GetLastError() != ABCKIT_STATUS_NO_ERROR) {
        return nullptr;
    }
    return CreateDynInst(graph, typeId, compiler::DataType::REFERENCE,
                         compiler::IntrinsicInst::IntrinsicId::INTRINSIC_ABCKIT_NEW_OBJECT, false);
}

constexpr size_t MAX_NUM_SHORT_CALL_ARGS = 2;
constexpr size_t MAX_NUM_NON_RANGE_ARGS = 4;

AbckitInst *IcreateInitObjectStatic(AbckitGraph *graph, AbckitCoreFunction *inputFunction, size_t argCount,
                                    va_list argp)
{
    LIBABCKIT_LOG_FUNC;

    auto func = inputFunction->GetArkTSImpl()->GetStaticImpl();
    auto constrName = pandasm::MangleFunctionName(func->name, func->params, func->returnType);
    uint64_t methodId;
    bool found = false;
    for (const auto &[offset, name] : graph->irInterface->methods) {
        if (name == constrName) {
            methodId = offset;
            found = true;
            break;
        }
    }

    if (!found) {
        LIBABCKIT_LOG(DEBUG) << "Can not find method id for '" << constrName << "'\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    AbckitInst *inst {nullptr};
    if (argCount > MAX_NUM_NON_RANGE_ARGS) {
        inst = CreateDynInst(graph, methodId, ark::compiler::DataType::REFERENCE,
                             compiler::IntrinsicInst::IntrinsicId::INTRINSIC_ABCKIT_INIT_OBJECT_RANGE, false);
    } else if (argCount > MAX_NUM_SHORT_CALL_ARGS) {
        inst = CreateDynInst(graph, methodId, ark::compiler::DataType::REFERENCE,
                             compiler::IntrinsicInst::IntrinsicId::INTRINSIC_ABCKIT_INIT_OBJECT, false);
    } else {
        inst = CreateDynInst(graph, methodId, ark::compiler::DataType::REFERENCE,
                             compiler::IntrinsicInst::IntrinsicId::INTRINSIC_ABCKIT_INIT_OBJECT_SHORT, false);
    }

    inst->impl->CastToIntrinsic()->AllocateInputTypes(graph->impl->GetAllocator(), argCount);
    for (size_t i = 0; i < argCount; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        auto *arg = va_arg(argp, AbckitInst *);
        inst->impl->CastToIntrinsic()->AppendInput(arg->impl, arg->impl->GetType());
    }
    va_end(argp);

    return inst;
}

AbckitInst *IcreateStoreObjectStatic(AbckitGraph *graph, AbckitInst *inputObj, AbckitString *fieldId, AbckitInst *value,
                                     AbckitTypeId typeId)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_INTERNAL_ERROR(graph->impl, nullptr);
    LIBABCKIT_INTERNAL_ERROR(inputObj->impl, nullptr);
    LIBABCKIT_INTERNAL_ERROR(value->impl, nullptr);

    compiler::IntrinsicInst *intrImpl {nullptr};
    auto type = TypeIdToType(typeId);
    switch (type) {
        case compiler::DataType::BOOL:
        case compiler::DataType::UINT8:
        case compiler::DataType::INT8:
        case compiler::DataType::UINT16:
        case compiler::DataType::INT16:
        case compiler::DataType::UINT32:
        case compiler::DataType::INT32:
        case compiler::DataType::FLOAT32:
            intrImpl = graph->impl->CreateInstIntrinsic(
                type, 0, compiler::IntrinsicInst::IntrinsicId::INTRINSIC_ABCKIT_STORE_OBJECT);
            break;
        case compiler::DataType::INT64:
        case compiler::DataType::UINT64:
        case compiler::DataType::FLOAT64:
            intrImpl = graph->impl->CreateInstIntrinsic(
                type, 0, compiler::IntrinsicInst::IntrinsicId::INTRINSIC_ABCKIT_STORE_OBJECT_WIDE);
            break;
        case compiler::DataType::REFERENCE:
            intrImpl = graph->impl->CreateInstIntrinsic(
                type, 0, compiler::IntrinsicInst::IntrinsicId::INTRINSIC_ABCKIT_STORE_OBJECT_OBJECT);
            break;
        default:
            LIBABCKIT_UNREACHABLE;
    }

    size_t argsCount {2U};
    intrImpl->AddImm(graph->impl->GetAllocator(), GetFieldOffset(graph, fieldId));
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(value->impl, type);
    intrImpl->AppendInput(inputObj->impl, ark::compiler::DataType::REFERENCE);
    intrImpl->SetMethod(graph->impl->GetMethod());

    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *IcreateIfStaticStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                  AbckitIsaApiStaticConditionCode cc)
{
    LIBABCKIT_LOG_FUNC;
    auto instImpl = graph->impl->CreateInstIf(compiler::DataType::NO_TYPE, 0, input0->impl, input1->impl,
                                              input0->impl->GetType(), CcToCc(cc), graph->impl->GetMethod());
    return CreateInstFromImpl(graph, instImpl);
}

AbckitInst *IcreateIfDynamicStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                   AbckitIsaApiDynamicConditionCode cc)
{
    LIBABCKIT_LOG_FUNC;
    auto instImpl = graph->impl->CreateInstIf(compiler::DataType::NO_TYPE, 0, input0->impl, input1->impl,
                                              input0->impl->GetType(), CcToCc(cc), graph->impl->GetMethod());
    return CreateInstFromImpl(graph, instImpl);
}

AbckitInst *IcreateTryStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    auto instImpl = graph->impl->CreateInstTry();
    return CreateInstFromImpl(graph, instImpl);
}

AbckitInst *IcreateLoadArrayStatic(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx, AbckitTypeId returnTypeId)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_LOG(DEBUG) << "IcreateLoadArrayStatic" << '\n';

    if (returnTypeId == ABCKIT_TYPE_ID_INVALID || IsDynamic(graph->function->owningModule->target) ||
        arrayRef->impl->GetType() != compiler::DataType::REFERENCE ||
        (idx->impl->GetType() != compiler::DataType::INT32 && idx->impl->GetType() != compiler::DataType::INT64)) {
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    // ARRAY VERIFICATION!!!

    auto intrinsicId = compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_ARRAY;
    auto dataType = TypeIdToType(returnTypeId);
    auto loadArrayImpl = graph->impl->CreateInstIntrinsic(dataType, 0, intrinsicId);
    size_t argsCount {2U};

    loadArrayImpl->ReserveInputs(argsCount);
    loadArrayImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    loadArrayImpl->AppendInput(idx->impl, compiler::DataType::INT32);
    loadArrayImpl->AppendInput(arrayRef->impl, compiler::DataType::REFERENCE);

    return CreateInstFromImpl(graph, loadArrayImpl);
}

static AbckitInst *IcreateStoreArrayBody(AbckitInst *arrayRef, AbckitInst *idx, AbckitInst *value,
                                         AbckitTypeId valueTypeId, bool isWide)
{
    auto *graph = arrayRef->graph;
    if (valueTypeId == ABCKIT_TYPE_ID_INVALID || IsDynamic(graph->function->owningModule->target) ||
        arrayRef->impl->GetType() != compiler::DataType::REFERENCE) {
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    // ARRAY VERIFICATION!!! :()
    auto dataType = TypeIdToType(valueTypeId);

    auto intrinsicId = isWide ? compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_STORE_ARRAY_WIDE
                              : compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_STORE_ARRAY;
    auto storeArrayImpl = graph->impl->CreateInstIntrinsic(dataType, 0, intrinsicId);
    size_t argsCount {3U};

    storeArrayImpl->ReserveInputs(argsCount);
    storeArrayImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    storeArrayImpl->AppendInput(value->impl, dataType);
    storeArrayImpl->AppendInput(arrayRef->impl, compiler::DataType::REFERENCE);
    storeArrayImpl->AppendInput(idx->impl, compiler::DataType::INT32);

    return CreateInstFromImpl(graph, storeArrayImpl);
}

AbckitInst *IcreateStoreArrayStatic([[maybe_unused]] AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx,
                                    AbckitInst *value, AbckitTypeId valueTypeId)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_LOG(DEBUG) << "IcreateStoreArrayStatic" << '\n';

    if (idx->impl->GetType() != compiler::DataType::INT32) {
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    return IcreateStoreArrayBody(arrayRef, idx, value, valueTypeId, false);
}

AbckitInst *IcreateStoreArrayWideStatic([[maybe_unused]] AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx,
                                        AbckitInst *value, AbckitTypeId valueTypeId)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_LOG(DEBUG) << "IcreateStoreArrayWideStatic" << '\n';

    if (idx->impl->GetType() != compiler::DataType::INT32 && idx->impl->GetType() != compiler::DataType::INT64) {
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    return IcreateStoreArrayBody(arrayRef, idx, value, valueTypeId, true);
}

AbckitInst *IcreateLoadObjectStatic(AbckitGraph *graph, AbckitInst *inputObj, AbckitString *field,
                                    AbckitTypeId returnTypeId)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_LOG(DEBUG) << "IcreateLoadObjectStatic" << '\n';

    if (returnTypeId == ABCKIT_TYPE_ID_INVALID || IsDynamic(graph->function->owningModule->target) ||
        inputObj->impl->GetType() != compiler::DataType::REFERENCE) {
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    auto dataType = TypeIdToType(returnTypeId);
    ark::compiler::IntrinsicInst *loadObjectImpl {nullptr};
    switch (dataType) {
        case ark::compiler::DataType::BOOL:
        case ark::compiler::DataType::UINT8:
        case ark::compiler::DataType::INT8:
        case ark::compiler::DataType::UINT16:
        case ark::compiler::DataType::INT16:
        case ark::compiler::DataType::UINT32:
        case ark::compiler::DataType::INT32:
        case ark::compiler::DataType::FLOAT32: {
            loadObjectImpl = graph->impl->CreateInstIntrinsic(
                dataType, 0, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_OBJECT);
            break;
        }
        case ark::compiler::DataType::INT64:
        case ark::compiler::DataType::UINT64:
        case ark::compiler::DataType::FLOAT64: {
            loadObjectImpl = graph->impl->CreateInstIntrinsic(
                dataType, 0, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_OBJECT_WIDE);
            break;
        }
        case ark::compiler::DataType::REFERENCE: {
            loadObjectImpl = graph->impl->CreateInstIntrinsic(
                dataType, 0, ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_OBJECT_OBJECT);
            break;
        }
        default:
            UNREACHABLE();
    }

    size_t argsCount {1U};
    loadObjectImpl->ReserveInputs(argsCount);
    loadObjectImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    loadObjectImpl->AppendInput(inputObj->impl, compiler::DataType::REFERENCE);
    loadObjectImpl->AddImm(graph->impl->GetAllocator(), GetFieldOffset(graph, field));

    return CreateInstFromImpl(graph, loadObjectImpl);
}

AbckitInst *IcreateLenArrayStatic(AbckitGraph *graph, AbckitInst *arr)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_LOG(DEBUG) << "IcreateLenArrayStatic" << '\n';

    if (IsDynamic(graph->function->owningModule->target) || arr->impl->GetType() != compiler::DataType::REFERENCE) {
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    auto lenArrayImpl = graph->impl->CreateInstLenArray(compiler::DataType::INT32, 0, arr->impl);
    return CreateInstFromImpl(graph, lenArrayImpl);
}

AbckitInst *IcreateLoadConstArrayStatic(AbckitGraph *graph, AbckitLiteralArray *literalArray)
{
    LIBABCKIT_LOG_FUNC;

    return CreateDynInst(graph, GetLiteralArrayOffset(graph, literalArray), compiler::DataType::REFERENCE,
                         compiler::IntrinsicInst::IntrinsicId::INTRINSIC_ABCKIT_LOAD_CONST_ARRAY, false);
}

AbckitInst *IcreateCheckCastStatic(AbckitGraph *graph, AbckitInst *inputObj, AbckitType *targetType)
{
    LIBABCKIT_LOG_FUNC;

    if (targetType->id != AbckitTypeId::ABCKIT_TYPE_ID_REFERENCE || targetType->GetClass() == nullptr) {
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::REFERENCE, 0,
                                                     compiler::IntrinsicInst::IntrinsicId::INTRINSIC_ABCKIT_CHECK_CAST);
    size_t argsCount {1U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(inputObj->impl, inputObj->impl->GetType());
    intrImpl->AddImm(graph->impl->GetAllocator(), GetClassOffset(graph, targetType->GetClass()));

    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *IcreateIsInstanceStatic(AbckitGraph *graph, AbckitInst *inputObj, AbckitType *targetType)
{
    LIBABCKIT_LOG_FUNC;

    if (targetType->id != AbckitTypeId::ABCKIT_TYPE_ID_REFERENCE || targetType->GetClass() == nullptr) {
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }
    auto intrImpl = graph->impl->CreateInstIntrinsic(
        compiler::DataType::INT32, 0, compiler::IntrinsicInst::IntrinsicId::INTRINSIC_ABCKIT_IS_INSTANCE);
    size_t argsCount {1U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(inputObj->impl, inputObj->impl->GetType());
    intrImpl->AddImm(graph->impl->GetAllocator(), GetClassOffset(graph, targetType->GetClass()));
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *IcreateLoadNullValueStatic(AbckitGraph *graph)
{
    auto instImpl = graph->impl->CreateInstLoadUniqueObject(compiler::DataType::REFERENCE);
    return CreateInstFromImpl(graph, instImpl);
}

AbckitInst *IcreateCastStatic(AbckitGraph *graph, AbckitInst *input0, AbckitTypeId targetTypeId)
{
    if (targetTypeId <= ABCKIT_TYPE_ID_INVALID || targetTypeId > ABCKIT_TYPE_ID_REFERENCE) {
        LIBABCKIT_LOG(DEBUG) << "Bad cast destination type\n";
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    auto instImpl = graph->impl->CreateInstCast(TypeIdToType(targetTypeId), 0, input0->impl, input0->impl->GetType());
    return CreateInstFromImpl(graph, instImpl);
}

AbckitInst *IcreateIsUndefinedStatic(AbckitGraph *graph, AbckitInst *inputObj)
{
    LIBABCKIT_LOG_FUNC;
    auto intrImpl = graph->impl->CreateInstIntrinsic(
        compiler::DataType::BOOL, 0, compiler::IntrinsicInst::IntrinsicId::INTRINSIC_ABCKIT_IS_NULL_VALUE);
    size_t argsCount {1U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(inputObj->impl, inputObj->impl->GetType());
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *IcreateNullCheckStatic(AbckitGraph *graph, AbckitInst *inputObj)
{
    LIBABCKIT_LOG_FUNC;
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::VOID, 0,
                                                     compiler::IntrinsicInst::IntrinsicId::INTRINSIC_ABCKIT_NULL_CHECK);
    size_t argsCount {1U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(inputObj->impl, compiler::DataType::REFERENCE);
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *IcreateReturnStatic(AbckitGraph *graph, AbckitInst *input0)
{
    auto instImpl = graph->impl->CreateInstReturn(input0->impl->GetType(), compiler::INVALID_PC, input0->impl);
    return CreateInstFromImpl(graph, instImpl);
}

AbckitInst *IcreateReturnVoidStatic(AbckitGraph *graph)
{
    auto instImpl = graph->impl->CreateInstReturnVoid();
    return CreateInstFromImpl(graph, instImpl);
}

AbckitInst *IcreateThrowStatic(AbckitGraph *graph, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::REFERENCE, 0,
                                                     compiler::IntrinsicInst::IntrinsicId::INTRINSIC_ABCKIT_THROW);
    size_t argsCount {1U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, compiler::DataType::REFERENCE);
    intrImpl->AddImm(graph->impl->GetAllocator(), IC_SLOT_VALUE);
    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *IcreateSubStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;

    auto subImpl = graph->impl->CreateInstSub(input0->impl->GetType(), 0, input0->impl, input1->impl);
    return CreateInstFromImpl(graph, subImpl);
}

AbckitInst *IcreateMulStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;

    auto mulImpl = graph->impl->CreateInstMul(input0->impl->GetType(), 0, input0->impl, input1->impl);
    return CreateInstFromImpl(graph, mulImpl);
}

AbckitInst *IcreateDivStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;

    auto divImpl = graph->impl->CreateInstDiv(input0->impl->GetType(), 0, input0->impl, input1->impl);
    return CreateInstFromImpl(graph, divImpl);
}

AbckitInst *IcreateModStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;

    auto modImpl = graph->impl->CreateInstMod(input0->impl->GetType(), 0, input0->impl, input1->impl);
    return CreateInstFromImpl(graph, modImpl);
}

AbckitInst *IcreateEqualsStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;

    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::BOOL, compiler::INVALID_PC,
                                                     compiler::IntrinsicInst::IntrinsicId::INTRINSIC_ABCKIT_EQUALS);
    size_t argsCount {2U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, input0->impl->GetType());
    intrImpl->AppendInput(input1->impl, input1->impl->GetType());

    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *IcreateStrictEqualsStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;

    auto intrImpl = graph->impl->CreateInstIntrinsic(compiler::DataType::BOOL, compiler::INVALID_PC,
                                                     compiler::IntrinsicInst::IntrinsicId::INTRINSIC_ABCKIT_EQUALS);
    size_t argsCount {2U};
    intrImpl->ReserveInputs(argsCount);
    intrImpl->AllocateInputTypes(graph->impl->GetAllocator(), argsCount);
    intrImpl->AppendInput(input0->impl, input0->impl->GetType());
    intrImpl->AppendInput(input1->impl, input1->impl->GetType());

    return CreateInstFromImpl(graph, intrImpl);
}

AbckitInst *IcreateAddIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_LOG_FUNC;
    auto addIImpl = graph->impl->CreateInstAddI(compiler::DataType::INT32, 0, input0->impl, imm);
    return CreateInstFromImpl(graph, addIImpl);
}

AbckitInst *IcreateSubIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_LOG_FUNC;
    auto subIImpl = graph->impl->CreateInstSubI(compiler::DataType::INT32, 0, input0->impl, imm);
    return CreateInstFromImpl(graph, subIImpl);
}

AbckitInst *IcreateMulIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_LOG_FUNC;
    auto mulIImpl = graph->impl->CreateInstMulI(compiler::DataType::INT32, 0, input0->impl, imm);
    return CreateInstFromImpl(graph, mulIImpl);
}

AbckitInst *IcreateDivIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_LOG_FUNC;
    auto divIImpl = graph->impl->CreateInstDivI(compiler::DataType::INT32, 0, input0->impl, imm);
    return CreateInstFromImpl(graph, divIImpl);
}

AbckitInst *IcreateModIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_LOG_FUNC;
    auto modIImpl = graph->impl->CreateInstModI(compiler::DataType::INT32, 0, input0->impl, imm);
    return CreateInstFromImpl(graph, modIImpl);
}

AbckitInst *IcreateShlStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    auto shlImpl = graph->impl->CreateInstShl(compiler::DataType::INT64, 0, input0->impl, input1->impl);
    return CreateInstFromImpl(graph, shlImpl);
}

AbckitInst *IcreateShrStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    auto shrImpl = graph->impl->CreateInstShr(compiler::DataType::INT64, 0, input0->impl, input1->impl);
    return CreateInstFromImpl(graph, shrImpl);
}

AbckitInst *IcreateAShrStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    auto aShrImpl = graph->impl->CreateInstAShr(compiler::DataType::INT64, 0, input0->impl, input1->impl);
    return CreateInstFromImpl(graph, aShrImpl);
}

AbckitInst *IcreateShlIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_LOG_FUNC;
    auto shlIImpl = graph->impl->CreateInstShlI(compiler::DataType::INT32, 0, input0->impl, imm);
    return CreateInstFromImpl(graph, shlIImpl);
}

AbckitInst *IcreateShrIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_LOG_FUNC;
    auto shrIImpl = graph->impl->CreateInstShrI(compiler::DataType::INT32, 0, input0->impl, imm);
    return CreateInstFromImpl(graph, shrIImpl);
}

AbckitInst *IcreateAShrIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_LOG_FUNC;
    auto aShrIImpl = graph->impl->CreateInstAShrI(compiler::DataType::INT32, 0, input0->impl, imm);
    return CreateInstFromImpl(graph, aShrIImpl);
}

AbckitInst *IcreateNotStatic(AbckitGraph *graph, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;
    auto notImpl = graph->impl->CreateInstNot(compiler::DataType::INT64, 0, input0->impl);
    return CreateInstFromImpl(graph, notImpl);
}

AbckitInst *IcreateOrStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    auto orImpl = graph->impl->CreateInstOr(compiler::DataType::INT64, 0, input0->impl, input1->impl);
    return CreateInstFromImpl(graph, orImpl);
}

AbckitInst *IcreateXorStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    auto xorImpl = graph->impl->CreateInstXor(compiler::DataType::INT64, 0, input0->impl, input1->impl);
    return CreateInstFromImpl(graph, xorImpl);
}

AbckitInst *IcreateAndStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;
    auto andImpl = graph->impl->CreateInstAnd(compiler::DataType::INT64, 0, input0->impl, input1->impl);
    return CreateInstFromImpl(graph, andImpl);
}

AbckitInst *IcreateOrIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_LOG_FUNC;
    auto orIImpl = graph->impl->CreateInstOrI(compiler::DataType::INT32, 0, input0->impl, imm);
    return CreateInstFromImpl(graph, orIImpl);
}

AbckitInst *IcreateXorIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_LOG_FUNC;
    auto xorIImpl = graph->impl->CreateInstXorI(compiler::DataType::INT32, 0, input0->impl, imm);
    return CreateInstFromImpl(graph, xorIImpl);
}

AbckitInst *IcreateAndIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    LIBABCKIT_LOG_FUNC;
    auto andIImpl = graph->impl->CreateInstAndI(compiler::DataType::INT32, 0, input0->impl, imm);
    return CreateInstFromImpl(graph, andIImpl);
}

AbckitInst *IcreateNegStatic(AbckitGraph *graph, AbckitInst *input0)
{
    LIBABCKIT_LOG_FUNC;

    auto negImpl = graph->impl->CreateInstNeg(input0->impl->GetType(), 0, input0->impl);
    return CreateInstFromImpl(graph, negImpl);
}

AbckitInst *IcreateAddStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;

    auto addImpl = graph->impl->CreateInstAdd(input0->impl->GetType(), 0, input0->impl, input1->impl);
    return CreateInstFromImpl(graph, addImpl);
}

AbckitInst *IcreateCallStaticStatic(AbckitGraph *graph, AbckitCoreFunction *inputFunction, size_t argCount,
                                    va_list argp)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_LOG(DEBUG) << "argCount: " << argCount << "\n";
    auto *func = inputFunction->GetArkTSImpl()->GetStaticImpl();
    size_t paramCount = func->GetParamsNum();
    auto methodOffset = GetMethodOffset(graph, inputFunction);
    auto methodPtr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(methodOffset);

    auto callImpl = graph->impl->CreateInstCallStatic(graph->impl->GetRuntime()->GetMethodReturnType(methodPtr), 0,
                                                      methodOffset, methodPtr);
    callImpl->ClearFlag(compiler::inst_flags::REQUIRE_STATE);
    callImpl->AllocateInputTypes(graph->impl->GetAllocator(), paramCount);
    for (size_t i = 0; i < argCount; i++) {
        LIBABCKIT_LOG(DEBUG) << "append arg " << i << '\n';
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        auto *inst = va_arg(argp, AbckitInst *);
        callImpl->AppendInput(inst->impl, inst->impl->GetType());
    }
    va_end(argp);

    return CreateInstFromImpl(graph, callImpl);
}

AbckitInst *IcreateCallVirtualStatic(AbckitGraph *graph, AbckitInst *inputObj, AbckitCoreFunction *inputFunction,
                                     size_t argCount, va_list argp)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_LOG(DEBUG) << "argCount: " << argCount << "\n";
    auto *func = inputFunction->GetArkTSImpl()->GetStaticImpl();
    size_t paramCount = func->GetParamsNum();
    auto methodOffset = GetMethodOffset(graph, inputFunction);
    auto methodPtr = reinterpret_cast<compiler::RuntimeInterface::MethodPtr>(methodOffset);

    auto callImpl = graph->impl->CreateInstCallVirtual(graph->impl->GetRuntime()->GetMethodReturnType(methodPtr), 0,
                                                       methodOffset, methodPtr);
    callImpl->ClearFlag(compiler::inst_flags::REQUIRE_STATE);
    callImpl->AllocateInputTypes(graph->impl->GetAllocator(), paramCount + 1);
    callImpl->AppendInput(inputObj->impl, compiler::DataType::REFERENCE);
    for (size_t i = 0; i < argCount; i++) {
        LIBABCKIT_LOG(DEBUG) << "append arg " << i << '\n';
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        auto *inst = va_arg(argp, AbckitInst *);
        callImpl->AppendInput(inst->impl, inst->impl->GetType());
    }
    va_end(argp);

    return CreateInstFromImpl(graph, callImpl);
}

AbckitInst *IcreateLoadStringStatic(AbckitGraph *graph, AbckitString *str)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_LOG(DEBUG) << GetStringOffset(graph, str) << '\n';
    auto intrinsicId = compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_STRING;
    auto dataType =
        IsDynamic(graph->function->owningModule->target) ? compiler::DataType::ANY : compiler::DataType::REFERENCE;
    auto loadStringImpl = graph->impl->CreateInstIntrinsic(dataType, 0, intrinsicId);
    auto *loadString = CreateInstFromImpl(graph, loadStringImpl);
    loadStringImpl->AddImm(graph->impl->GetAllocator(), GetStringOffset(graph, str));
    return loadString;
}

AbckitInst *IcreateCmpStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    LIBABCKIT_LOG_FUNC;

    if (input0->impl->GetType() != input1->impl->GetType()) {
        SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    auto cmpImpl =
        graph->impl->CreateInstCmp(compiler::DataType::INT32, 0, input0->impl, input1->impl, input0->impl->GetType());
    return CreateInstFromImpl(graph, cmpImpl);
}

}  // namespace libabckit
