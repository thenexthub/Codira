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

#include "../../src/mock/abckit_mock.h"
#include "../../src/mock/mock_values.h"

#include "../../include/libabckit/c/metadata_core.h"
#include "../../include/libabckit/c/isa/isa_static.h"

#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>

namespace libabckit::mock::isa_api_static {

// NOLINTBEGIN(readability-identifier-naming)

AbckitCoreClass *IgetClass(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_CORE_CLASS;
}

void IsetClass(AbckitInst *inst, AbckitCoreClass *klass)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(klass == DEFAULT_CORE_CLASS);
}

enum AbckitIsaApiStaticConditionCode IgetConditionCode(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_ENUM_ISA_STATIC_CONDITION_CODE;
}

void IsetConditionCode(AbckitInst *inst, enum AbckitIsaApiStaticConditionCode cc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(cc == DEFAULT_ENUM_ISA_STATIC_CONDITION_CODE);
}

AbckitIsaApiStaticOpcode IgetOpcode(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_ENUM_ISA_API_STATIC_OPCODE;
}

void IsetTargetType(AbckitInst *inst, AbckitTypeId t)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(t == DEFAULT_TYPE_ID);
}

enum AbckitTypeId IgetTargetType(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_ENUM_TYPE_ID;
}

AbckitInst *IcreateCmp(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateLoadString(AbckitGraph *graph, AbckitString *str)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(str == DEFAULT_STRING);
    return DEFAULT_INST;
}

AbckitInst *IcreateReturn(AbckitGraph *graph, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateIf(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                      enum AbckitIsaApiStaticConditionCode cc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    EXPECT_TRUE(cc == DEFAULT_ENUM_ISA_STATIC_CONDITION_CODE);
    return DEFAULT_INST;
}

AbckitInst *IcreateNeg(AbckitGraph *graph, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateNot(AbckitGraph *graph, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateAdd(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateSub(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateMul(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateDiv(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateMod(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateShl(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateShr(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateAShr(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateAnd(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateOr(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateXor(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCast(AbckitGraph *graph, AbckitInst *input0, AbckitTypeId targetType)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(targetType == DEFAULT_TYPE_ID);
    return DEFAULT_INST;
}

AbckitInst *GcreateNullPtr(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateNewArray(AbckitGraph *graph, AbckitCoreClass *inputClass, AbckitInst *inputSize)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(inputClass == DEFAULT_CORE_CLASS);
    EXPECT_TRUE(inputSize == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateNewObject(AbckitGraph *graph, AbckitCoreClass *inputClass)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(inputClass == DEFAULT_CORE_CLASS);
    return DEFAULT_INST;
}

AbckitInst *IcreateInitObject(AbckitGraph *graph, AbckitCoreFunction *function, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    EXPECT_TRUE(argCount == DEFAULT_SIZE_T);
    return DEFAULT_INST;
}

AbckitInst *IcreateLoadArray(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx, AbckitTypeId returnTypeId)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(arrayRef == DEFAULT_INST);
    EXPECT_TRUE(idx == DEFAULT_INST);
    EXPECT_TRUE(returnTypeId == DEFAULT_TYPE_ID);
    return DEFAULT_INST;
}

AbckitInst *IcreateStoreArray(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx, AbckitInst *value,
                              AbckitTypeId returnTypeId)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(arrayRef == DEFAULT_INST);
    EXPECT_TRUE(idx == DEFAULT_INST);
    EXPECT_TRUE(value == DEFAULT_INST);
    EXPECT_TRUE(returnTypeId == DEFAULT_TYPE_ID);
    return DEFAULT_INST;
}

AbckitInst *IcreateLoadObject(AbckitGraph *graph, AbckitInst *inputObj, AbckitString *field, AbckitTypeId returnTypeId)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(inputObj == DEFAULT_INST);
    EXPECT_TRUE(field == DEFAULT_STRING);
    EXPECT_TRUE(returnTypeId == DEFAULT_TYPE_ID);
    return DEFAULT_INST;
}

AbckitInst *IcreateStoreObject(AbckitGraph *graph, AbckitInst *inputObj, AbckitString *fieldId, AbckitInst *value,
                               AbckitTypeId typeId)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(inputObj == DEFAULT_INST);
    EXPECT_TRUE(fieldId == DEFAULT_STRING);
    EXPECT_TRUE(value == DEFAULT_INST);
    EXPECT_TRUE(typeId == DEFAULT_ENUM_TYPE_ID);
    return DEFAULT_INST;
}

AbckitInst *IcreateStoreArrayWide(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx, AbckitInst *value,
                                  AbckitTypeId returnTypeId)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(arrayRef == DEFAULT_INST);
    EXPECT_TRUE(idx == DEFAULT_INST);
    EXPECT_TRUE(value == DEFAULT_INST);
    EXPECT_TRUE(returnTypeId == DEFAULT_TYPE_ID);
    return DEFAULT_INST;
}

AbckitInst *IcreateLenArray(AbckitGraph *graph, AbckitInst *arr)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(arr == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateLoadConstArray(AbckitGraph *graph, AbckitLiteralArray *literalArray)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(literalArray == DEFAULT_LITERAL_ARRAY);
    return DEFAULT_INST;
}

AbckitInst *IcreateCheckCast(AbckitGraph *graph, AbckitInst *inputObj, AbckitType *targetType)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(inputObj == DEFAULT_INST);
    EXPECT_TRUE(targetType == DEFAULT_TYPE);
    return DEFAULT_INST;
}

AbckitInst *IcreateIsInstance(AbckitGraph *graph, AbckitInst *inputObj, AbckitType *targetType)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(inputObj == DEFAULT_INST);
    EXPECT_TRUE(targetType == DEFAULT_TYPE);
    return DEFAULT_INST;
}

AbckitInst *IcreateLoadNullValue(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateReturnVoid(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateEquals(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallStatic(AbckitGraph *graph, AbckitCoreFunction *function, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    EXPECT_TRUE(argCount == DEFAULT_SIZE_T);
    return DEFAULT_INST;
}

AbckitInst *IcreateStrictEquals(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallVirtual(AbckitGraph *graph, AbckitInst *inputObj, AbckitCoreFunction *function, size_t argCount,
                               ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(inputObj == DEFAULT_INST);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    EXPECT_TRUE(argCount == DEFAULT_SIZE_T);
    return DEFAULT_INST;
}

AbckitInst *IcreateAddI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateSubI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateMulI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateDivI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateModI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateShlI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateShrI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateAShrI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateAndI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateOrI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateXorI(AbckitGraph *graph, AbckitInst *input0, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateThrow(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateIsUndefined(AbckitGraph *graph, AbckitInst *inputObj)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(inputObj == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateNullCheck(AbckitGraph *graph, AbckitInst *inputObj)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(inputObj == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateLoadObject(AbckitGraph *graph, AbckitInst *inputObj, AbckitCoreClassField *field)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(inputObj == DEFAULT_INST);
    EXPECT_TRUE(field == DEFAULT_CORE_CLASS_FIELD);
    return DEFAULT_INST;
}

AbckitIsaApiStatic g_isaApiStaticImpl = {
    IgetClass,
    IsetClass,
    IgetConditionCode,
    IsetConditionCode,
    IgetOpcode,
    IsetTargetType,
    IgetTargetType,
    IcreateCmp,
    IcreateLoadString,
    IcreateReturn,
    IcreateIf,
    IcreateNeg,
    IcreateNot,
    IcreateAdd,
    IcreateSub,
    IcreateMul,
    IcreateDiv,
    IcreateMod,
    IcreateShl,
    IcreateShr,
    IcreateAShr,
    IcreateAnd,
    IcreateOr,
    IcreateXor,
    IcreateCast,
    GcreateNullPtr,
    IcreateNewArray,
    IcreateNewObject,
    IcreateInitObject,
    IcreateLoadObject,
    IcreateStoreObject,
    IcreateLoadArray,
    IcreateStoreArray,
    IcreateStoreArrayWide,
    IcreateLenArray,
    IcreateLoadConstArray,
    IcreateCheckCast,
    IcreateIsInstance,
    IcreateLoadNullValue,
    IcreateReturnVoid,
    IcreateEquals,
    IcreateStrictEquals,
    IcreateCallStatic,
    IcreateCallVirtual,
    IcreateAddI,
    IcreateSubI,
    IcreateMulI,
    IcreateDivI,
    IcreateModI,
    IcreateShlI,
    IcreateShrI,
    IcreateAShrI,
    IcreateAndI,
    IcreateOrI,
    IcreateXorI,
    IcreateThrow,
    IcreateIsUndefined,
    IcreateNullCheck,
};

// NOLINTEND(readability-identifier-naming)

}  // namespace libabckit::mock::isa_api_static

AbckitIsaApiStatic const *AbckitGetMockIsaApiStaticImpl([[maybe_unused]] AbckitApiVersion version)
{
    return &libabckit::mock::isa_api_static::g_isaApiStaticImpl;
}