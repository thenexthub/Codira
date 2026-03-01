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

#ifndef LIBABCKIT_SRC_ADAPTER_STATIC_HELPERS_STATIC_H
#define LIBABCKIT_SRC_ADAPTER_STATIC_HELPERS_STATIC_H

#include "libabckit/c/abckit.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/isa/isa_static.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/adapter_static/metadata_inspect_static.h"
#include "libabckit/src/adapter_static/runtime_adapter_static.h"

#include "static_core/compiler/optimizer/ir/inst.h"
#include "libarkbase/mem/arena_allocator.h"
#include "static_core/assembler/annotation.h"

#include <string>
#include <tuple>

namespace libabckit {

struct CtxGInternal {
    ark::ArenaAllocator *allocator;
    ark::ArenaAllocator *localAllocator;
    const AbckitIrInterface *irInterface;
    AbckitRuntimeAdapterStatic *runtimeAdapter;
};

std::tuple<std::string, std::string> ClassGetNames(const std::string &fullName);
std::tuple<std::string, std::string> FuncGetNames(const std::string &fullSig);
std::string FuncNameCropModule(const std::string &fullSig);

void CheckInvalidOpcodes(ark::compiler::Graph *graph, bool isDynamic);
size_t GetIntrinicMaxInputsCount(AbckitInst *inst);
bool IsCallInst(AbckitInst *inst);

AbckitIsaApiStaticOpcode GetStaticOpcode(ark::compiler::Inst *inst);
AbckitIsaApiDynamicOpcode GetDynamicOpcode(ark::compiler::Inst *inst);
AbckitTypeId TypeToTypeId(ark::compiler::DataType::Type type);
ark::compiler::DataType::Type TypeIdToType(AbckitTypeId typeId);
ark::compiler::ConditionCode CcToCc(AbckitIsaApiDynamicConditionCode cc);
ark::compiler::ConditionCode CcToCc(AbckitIsaApiStaticConditionCode cc);
AbckitIsaApiDynamicConditionCode CcToDynamicCc(ark::compiler::ConditionCode cc);
AbckitIsaApiStaticConditionCode CcToStaticCc(ark::compiler::ConditionCode cc);
void SetLastError(AbckitStatus err);

std::string GetFuncName(AbckitCoreFunction *function);
std::string GetMangleFuncName(AbckitCoreFunction *function);

uint32_t GetClassOffset(AbckitGraph *graph, AbckitCoreClass *klass);
uint32_t GetMethodOffset(AbckitGraph *graph, AbckitCoreFunction *function);
uint32_t GetStringOffset(AbckitGraph *graph, AbckitString *string);
uint32_t GetFieldOffset(AbckitGraph *graph, AbckitString *string);
uint32_t GetLiteralArrayOffset(AbckitGraph *graph, AbckitLiteralArray *arr);
AbckitInst *CreateInstFromImpl(AbckitGraph *graph, ark::compiler::Inst *impl);
AbckitInst *FindOrCreateInstFromImpl(AbckitGraph *graph, ark::compiler::Inst *impl);

AbckitInst *CreateDynInst(AbckitGraph *graph, ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);

AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, ark::compiler::IntrinsicInst::IntrinsicId intrinsicId,
                          bool hasIC = true);
AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, ark::compiler::DataType::Type retType,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, AbckitInst *inputSize,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, ark::compiler::IntrinsicInst::IntrinsicId intrinsicId,
                          bool hasIC = true);

AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, uint64_t imm1,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, AbckitInst *input2,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0, uint64_t imm1,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0, uint64_t imm1,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                          AbckitInst *input2, ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, uint64_t imm0, uint64_t imm1, uint64_t imm2, AbckitInst *input0,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                          AbckitInst *input2, AbckitInst *input3, ark::compiler::IntrinsicInst::IntrinsicId intrinsicId,
                          bool hasIC = true);

// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0, std::va_list args,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
// CC-OFFNXT(G.FUN.01) helper function
AbckitInst *CreateDynInst(AbckitGraph *graph, AbckitInst *acc, size_t argCount, std::va_list args,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);
AbckitInst *CreateDynInst(AbckitGraph *graph, size_t argCount, std::va_list args,
                          ark::compiler::IntrinsicInst::IntrinsicId intrinsicId, bool hasIC = true);

// Forward declaration of functions which implemented in helpers_dynamic.cpp
panda::pandasm::Function *GetDynFunction(AbckitCoreFunction *function);
AbckitModulePayloadDyn *GetDynModulePayload(AbckitCoreModule *mod);
AbckitDynamicImportDescriptorPayload *GetDynImportDescriptorPayload(AbckitCoreImportDescriptor *id);
AbckitDynamicExportDescriptorPayload *GetDynExportDescriptorPayload(AbckitCoreExportDescriptor *ed);
void FixPreassignedRegisters(ark::compiler::Graph *graph);
bool AllocateRegisters(ark::compiler::Graph *graph, uint8_t reservedReg);

AbckitLiteral *FindOrCreateLiteralBoolStaticImpl(AbckitFile *file, bool value);
AbckitLiteral *FindOrCreateLiteralU8StaticImpl(AbckitFile *file, uint8_t value);
AbckitLiteral *FindOrCreateLiteralU16StaticImpl(AbckitFile *file, uint16_t value);
AbckitLiteral *FindOrCreateLiteralMethodAffiliateStaticImpl(AbckitFile *file, uint16_t value);
AbckitLiteral *FindOrCreateLiteralU32StaticImpl(AbckitFile *file, uint32_t value);
AbckitLiteral *FindOrCreateLiteralU64StaticImpl(AbckitFile *file, uint64_t value);
AbckitLiteral *FindOrCreateLiteralFloatStaticImpl(AbckitFile *file, float value);
AbckitLiteral *FindOrCreateLiteralDoubleStaticImpl(AbckitFile *file, double value);
AbckitLiteral *FindOrCreateLiteralStringStaticImpl(AbckitFile *file, const std::string &value);
AbckitLiteral *FindOrCreateLiteralMethodStaticImpl(AbckitFile *file, const std::string &value);

AbckitValue *FindOrCreateValueU1StaticImpl(AbckitFile *file, bool value);
AbckitValue *FindOrCreateValueIntStaticImpl(AbckitFile *file, int value);
AbckitValue *FindOrCreateValueDoubleStaticImpl(AbckitFile *file, double value);
AbckitValue *FindOrCreateValueStringStaticImpl(AbckitFile *file, const std::string &value);
AbckitValue *FindOrCreateValueMethodStaticImpl(AbckitFile *file, const std::string &value, bool isStatic);
AbckitValue *FindOrCreateRecordValueStaticImpl(AbckitFile *file, const ark::pandasm::Type &value);
AbckitValue *FindOrCreateLiteralArrayValueStaticImpl(AbckitFile *file, const std::string &value);
AbckitValue *FindOrCreateArrayValueStaticImpl(AbckitFile *file, const ark::pandasm::ArrayValue &value);
AbckitValue *FindOrCreateValueStatic(AbckitFile *file, const ark::pandasm::Value &value);

void GraphInvalidateAnalyses(ark::compiler::Graph *graph);
bool GraphHasUnreachableBlocks(ark::compiler::Graph *graph);
bool GraphDominatorsTreeAnalysisIsValid(ark::compiler::Graph *graph);
AbckitTypeId ArkPandasmTypeToAbckitTypeId(const ark::pandasm::Type &type);
ark::pandasm::Record *GetStaticImplRecord(
    const std::variant<AbckitCoreModule *, AbckitCoreNamespace *, AbckitCoreClass *, AbckitCoreInterface *,
                       AbckitCoreEnum *, AbckitCoreAnnotationInterface *> &coreObject);

std::string TypeToNameStatic(AbckitType *type);

constexpr AbckitBitImmSize GetBitLengthUnsigned(uint64_t imm)
{
    // NOLINTNEXTLINE(readability-identifier-naming)
    constexpr uint8_t bit4Max = 0xF;

    if (imm <= bit4Max) {
        return AbckitBitImmSize::BITSIZE_4;
    }
    if (imm <= std::numeric_limits<uint8_t>::max()) {
        return AbckitBitImmSize::BITSIZE_8;
    }
    if (imm <= std::numeric_limits<uint16_t>::max()) {
        return AbckitBitImmSize::BITSIZE_16;
    }
    if (imm <= std::numeric_limits<uint32_t>::max()) {
        return AbckitBitImmSize::BITSIZE_32;
    }
    return AbckitBitImmSize::BITSIZE_64;
}

constexpr AbckitBitImmSize GetBinaryImmOperationSize(ark::compiler::Opcode opcode)
{
    switch (opcode) {
        case ark::compiler::Opcode::AddI:
        case ark::compiler::Opcode::SubI:
        case ark::compiler::Opcode::MulI:
        case ark::compiler::Opcode::DivI:
        case ark::compiler::Opcode::ModI:
        case ark::compiler::Opcode::ShlI:
        case ark::compiler::Opcode::ShrI:
        case ark::compiler::Opcode::AShrI:
            return AbckitBitImmSize::BITSIZE_8;

        case ark::compiler::Opcode::AndI:
        case ark::compiler::Opcode::OrI:
        case ark::compiler::Opcode::XorI:
            return AbckitBitImmSize::BITSIZE_32;
        default:
            LIBABCKIT_LOG(DEBUG) << "Not BinaryImmOperation" << '\n';
    }
    return AbckitBitImmSize::BITSIZE_0;
}

}  // namespace libabckit

#endif
