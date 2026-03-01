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

#include "../include/libabckit/c/isa/isa_dynamic.h"

#include <cstring>
#include <gtest/gtest.h>

namespace libabckit::mock::isa_api_dynamic {

// NOLINTBEGIN(readability-identifier-naming)

AbckitCoreModule *IgetModule(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_CORE_MODULE;
}

void IsetModule(AbckitInst *inst, AbckitCoreModule *md)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(md == DEFAULT_CORE_MODULE);
}

enum AbckitIsaApiDynamicConditionCode IgetConditionCode(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_ENUM_ISA_DYNAMIC_CONDITION_TYPE;
}

void IsetConditionCode(AbckitInst *inst, enum AbckitIsaApiDynamicConditionCode cc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(cc == DEFAULT_ENUM_ISA_DYNAMIC_CONDITION_TYPE);
}

enum AbckitIsaApiDynamicOpcode IgetOpcode(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_ENUM_ISA_API_DYNAMIC_OPCODE;
}

AbckitCoreImportDescriptor *IgetImportDescriptor(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_CORE_IMPORT_DESCRIPTOR;
}

void IsetImportDescriptor(AbckitInst *inst, AbckitCoreImportDescriptor *id)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(id == DEFAULT_CORE_IMPORT_DESCRIPTOR);
}

AbckitCoreExportDescriptor *IgetExportDescriptor(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_CORE_EXPORT_DESCRIPTOR;
}

void IsetExportDescriptor(AbckitInst *inst, AbckitCoreExportDescriptor *ed)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(ed == DEFAULT_CORE_EXPORT_DESCRIPTOR);
}

AbckitInst *IcreateLoadString(AbckitGraph *graph, AbckitString *str)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(str == DEFAULT_STRING);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdnan(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdinfinity(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdundefined(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdnull(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdsymbol(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdglobal(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdtrue(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdfalse(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdhole(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdnewtarget(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdthis(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreatePoplexenv(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateGetunmappedargs(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateAsyncfunctionenter(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdfunction(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateDebugger(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateGetpropiterator(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateGetiterator(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateGetasynciterator(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdprivateproperty(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateStprivateproperty(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1,
                                     AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateTestin(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateDefinefieldbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateDefinepropertybyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCreateemptyobject(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateCreateemptyarray(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateCreategeneratorobj(AbckitGraph *graph, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCreateiterresultobj(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCreateobjectwithexcludedkeys(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                                uint64_t imm0, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideCreateobjectwithexcludedkeys(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                                    uint64_t imm0, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateCreatearraywithbuffer(AbckitGraph *graph, AbckitLiteralArray *literalArray)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(literalArray == DEFAULT_LITERAL_ARRAY);
    return DEFAULT_INST;
}

AbckitInst *IcreateCreateobjectwithbuffer(AbckitGraph *graph, AbckitLiteralArray *literalArray)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(literalArray == DEFAULT_LITERAL_ARRAY);
    return DEFAULT_INST;
}

AbckitInst *IcreateNewobjapply(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateNewobjrange(AbckitGraph *graph, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(argCount == 2U);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideNewobjrange(AbckitGraph *graph, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(argCount == 2U);
    return DEFAULT_INST;
}

AbckitInst *IcreateNewlexenv(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideNewlexenv(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateNewlexenvwithname(AbckitGraph *graph, uint64_t imm0, AbckitLiteralArray *literalArray)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(literalArray == DEFAULT_LITERAL_ARRAY);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideNewlexenvwithname(AbckitGraph *graph, uint64_t imm0, AbckitLiteralArray *literalArray)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(literalArray == DEFAULT_LITERAL_ARRAY);
    return DEFAULT_INST;
}

AbckitInst *IcreateCreateasyncgeneratorobj(AbckitGraph *graph, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateAsyncgeneratorresolve(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, AbckitInst *input2)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    EXPECT_TRUE(input2 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateAdd2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateSub2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateMul2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateDiv2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateMod2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateEq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateNoteq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateLess(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateLesseq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateGreater(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateGreatereq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateShl2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateShr2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateAshr2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateAnd2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateOr2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateXor2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateExp(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateTypeof(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateTonumber(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateTonumeric(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateNeg(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateNot(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateInc(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateDec(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateIstrue(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateIsfalse(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateIsin(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateInstanceof(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateStrictnoteq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateStricteq(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeNotifyconcurrentresult(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeDefinefieldbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                 AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeDefinefieldbyindex(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeTopropertykey(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeCreateprivateproperty(AbckitGraph *graph, uint64_t imm0, AbckitLiteralArray *literalArray)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(literalArray == DEFAULT_LITERAL_ARRAY);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeDefineprivateproperty(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1,
                                                    AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeCallinit(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeDefinesendableclass(AbckitGraph *graph, AbckitCoreFunction *function,
                                                  AbckitLiteralArray *literalArray, uint64_t imm0, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    EXPECT_TRUE(literalArray == DEFAULT_LITERAL_ARRAY);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeLdsendableclass(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeLdsendableexternalmodulevar(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeWideldsendableexternalmodulevar(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeNewsendableenv(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeWidenewsendableenv(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeStsendablevar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeWidestsendablevar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeLdsendablevar(AbckitGraph *graph, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeWideldsendablevar(AbckitGraph *graph, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeIstrue(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallruntimeIsfalse(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateThrow(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateThrowNotexists(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateThrowPatternnoncoercible(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateThrowDeletesuperproperty(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateThrowConstassignment(AbckitGraph *graph, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateThrowIfnotobject(AbckitGraph *graph, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateThrowUndefinedifhole(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateThrowIfsupernotcorrectcall(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateThrowUndefinedifholewithname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallarg0(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallarg1(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallargs2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallargs3(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                             AbckitInst *input2)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    EXPECT_TRUE(input2 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(argCount == 1U);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideCallrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(argCount == 1U);
    return DEFAULT_INST;
}

AbckitInst *IcreateSupercallspread(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateApply(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallthis0(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallthis1(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallthis2(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                             AbckitInst *input2)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    EXPECT_TRUE(input2 == DEFAULT_INST);
    return DEFAULT_INST;
}

// CC-OFFNXT(G.FUN.01) function args are necessary
AbckitInst *IcreateCallthis3(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                             AbckitInst *input2, AbckitInst *input3)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    EXPECT_TRUE(input2 == DEFAULT_INST);
    EXPECT_TRUE(input3 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCallthisrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(argCount == 1U);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideCallthisrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(argCount == 1U);
    return DEFAULT_INST;
}

AbckitInst *IcreateSupercallthisrange(AbckitGraph *graph, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(argCount == 1U);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideSupercallthisrange(AbckitGraph *graph, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(argCount == 1U);
    return DEFAULT_INST;
}

AbckitInst *IcreateSupercallarrowrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(argCount == 1U);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideSupercallarrowrange(AbckitGraph *graph, AbckitInst *acc, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(argCount == 1U);
    return DEFAULT_INST;
}

// CC-OFFNXT(G.FUN.01) function args are necessary
AbckitInst *IcreateDefinegettersetterbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                             AbckitInst *input1, AbckitInst *input2, AbckitInst *input3)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    EXPECT_TRUE(input2 == DEFAULT_INST);
    EXPECT_TRUE(input3 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateDefinefunc(AbckitGraph *graph, AbckitCoreFunction *function, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateDefinemethod(AbckitGraph *graph, AbckitInst *acc, AbckitCoreFunction *function, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateDefineclasswithbuffer(AbckitGraph *graph, AbckitCoreFunction *function,
                                         AbckitLiteralArray *literalArray, uint64_t imm0, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
    EXPECT_TRUE(literalArray == DEFAULT_LITERAL_ARRAY);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateResumegenerator(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateGetresumemode(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateGettemplateobject(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateGetnextpropname(AbckitGraph *graph, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateDelobjprop(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateSuspendgenerator(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateAsyncfunctionawaituncaught(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCopydataproperties(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateStarrayspread(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateSetobjectwithproto(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdobjbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateStobjbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateStownbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdsuperbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateStsuperbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdobjbyindex(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideLdobjbyindex(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateStobjbyindex(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideStobjbyindex(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateStownbyindex(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideStownbyindex(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateAsyncfunctionresolve(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateAsyncfunctionreject(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateCopyrestargs(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideCopyrestargs(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdlexvar(AbckitGraph *graph, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideLdlexvar(AbckitGraph *graph, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateStlexvar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideStlexvar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    EXPECT_TRUE(imm1 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateGetmodulenamespace(AbckitGraph *graph, AbckitCoreModule *md)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(md == DEFAULT_CORE_MODULE);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideGetmodulenamespace(AbckitGraph *graph, AbckitCoreModule *md)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(md == DEFAULT_CORE_MODULE);
    return DEFAULT_INST;
}

AbckitInst *IcreateStmodulevar(AbckitGraph *graph, AbckitInst *acc, AbckitCoreExportDescriptor *ed)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(ed == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideStmodulevar(AbckitGraph *graph, AbckitInst *acc, AbckitCoreExportDescriptor *ed)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(ed == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_INST;
}

AbckitInst *IcreateTryldglobalbyname(AbckitGraph *graph, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

AbckitInst *IcreateTrystglobalbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdglobalvar(AbckitGraph *graph, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

AbckitInst *IcreateStglobalvar(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdobjbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

AbckitInst *IcreateStobjbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateStownbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdsuperbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

AbckitInst *IcreateStsuperbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdlocalmodulevar(AbckitGraph *graph, AbckitCoreExportDescriptor *ed)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(ed == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideLdlocalmodulevar(AbckitGraph *graph, AbckitCoreExportDescriptor *ed)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(ed == DEFAULT_CORE_EXPORT_DESCRIPTOR);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdexternalmodulevar(AbckitGraph *graph, AbckitCoreImportDescriptor *id)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(id == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideLdexternalmodulevar(AbckitGraph *graph, AbckitCoreImportDescriptor *id)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(id == DEFAULT_CORE_IMPORT_DESCRIPTOR);
    return DEFAULT_INST;
}

AbckitInst *IcreateStownbyvaluewithnameset(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    EXPECT_TRUE(input1 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateStownbynamewithnameset(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdbigint(AbckitGraph *graph, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdthisbyname(AbckitGraph *graph, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

AbckitInst *IcreateStthisbyname(AbckitGraph *graph, AbckitInst *acc, AbckitString *string)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(string == DEFAULT_STRING);
    return DEFAULT_INST;
}

AbckitInst *IcreateLdthisbyvalue(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateStthisbyvalue(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideLdpatchvar(AbckitGraph *graph, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateWideStpatchvar(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateDynamicimport(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateAsyncgeneratorreject(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(input0 == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateSetgeneratorstate(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    EXPECT_TRUE(imm0 == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *IcreateReturn(AbckitGraph *graph, AbckitInst *acc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(acc == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IcreateReturnundefined(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_INST;
}

AbckitInst *IcreateIf(AbckitGraph *graph, AbckitInst *input, enum AbckitIsaApiDynamicConditionCode cc)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(input == DEFAULT_INST);
    EXPECT_TRUE(cc == DEFAULT_ENUM_ISA_DYNAMIC_CONDITION_TYPE);
    return DEFAULT_INST;
}

AbckitIsaApiDynamic g_isaApiDynamicImpl = {
    IgetModule,
    IsetModule,
    IgetConditionCode,
    IsetConditionCode,
    IgetOpcode,
    IgetImportDescriptor,
    IsetImportDescriptor,
    IgetExportDescriptor,
    IsetExportDescriptor,
    IcreateLoadString,
    IcreateLdnan,
    IcreateLdinfinity,
    IcreateLdundefined,
    IcreateLdnull,
    IcreateLdsymbol,
    IcreateLdglobal,
    IcreateLdtrue,
    IcreateLdfalse,
    IcreateLdhole,
    IcreateLdnewtarget,
    IcreateLdthis,
    IcreatePoplexenv,
    IcreateGetunmappedargs,
    IcreateAsyncfunctionenter,
    IcreateLdfunction,
    IcreateDebugger,
    IcreateGetpropiterator,
    IcreateGetiterator,
    IcreateGetasynciterator,
    IcreateLdprivateproperty,
    IcreateStprivateproperty,
    IcreateTestin,
    IcreateDefinefieldbyname,
    IcreateDefinepropertybyname,
    IcreateCreateemptyobject,
    IcreateCreateemptyarray,
    IcreateCreategeneratorobj,
    IcreateCreateiterresultobj,
    IcreateCreateobjectwithexcludedkeys,
    IcreateWideCreateobjectwithexcludedkeys,
    IcreateCreatearraywithbuffer,
    IcreateCreateobjectwithbuffer,
    IcreateNewobjapply,
    IcreateNewobjrange,
    IcreateWideNewobjrange,
    IcreateNewlexenv,
    IcreateWideNewlexenv,
    IcreateNewlexenvwithname,
    IcreateWideNewlexenvwithname,
    IcreateCreateasyncgeneratorobj,
    IcreateAsyncgeneratorresolve,
    IcreateAdd2,
    IcreateSub2,
    IcreateMul2,
    IcreateDiv2,
    IcreateMod2,
    IcreateEq,
    IcreateNoteq,
    IcreateLess,
    IcreateLesseq,
    IcreateGreater,
    IcreateGreatereq,
    IcreateShl2,
    IcreateShr2,
    IcreateAshr2,
    IcreateAnd2,
    IcreateOr2,
    IcreateXor2,
    IcreateExp,
    IcreateTypeof,
    IcreateTonumber,
    IcreateTonumeric,
    IcreateNeg,
    IcreateNot,
    IcreateInc,
    IcreateDec,
    IcreateIstrue,
    IcreateIsfalse,
    IcreateIsin,
    IcreateInstanceof,
    IcreateStrictnoteq,
    IcreateStricteq,
    IcreateCallruntimeNotifyconcurrentresult,
    IcreateCallruntimeDefinefieldbyvalue,
    IcreateCallruntimeDefinefieldbyindex,
    IcreateCallruntimeTopropertykey,
    IcreateCallruntimeCreateprivateproperty,
    IcreateCallruntimeDefineprivateproperty,
    IcreateCallruntimeCallinit,
    IcreateCallruntimeDefinesendableclass,
    IcreateCallruntimeLdsendableclass,
    IcreateCallruntimeLdsendableexternalmodulevar,
    IcreateCallruntimeWideldsendableexternalmodulevar,
    IcreateCallruntimeNewsendableenv,
    IcreateCallruntimeWidenewsendableenv,
    IcreateCallruntimeStsendablevar,
    IcreateCallruntimeWidestsendablevar,
    IcreateCallruntimeLdsendablevar,
    IcreateCallruntimeWideldsendablevar,
    IcreateCallruntimeIstrue,
    IcreateCallruntimeIsfalse,
    IcreateThrow,
    IcreateThrowNotexists,
    IcreateThrowPatternnoncoercible,
    IcreateThrowDeletesuperproperty,
    IcreateThrowConstassignment,
    IcreateThrowIfnotobject,
    IcreateThrowUndefinedifhole,
    IcreateThrowIfsupernotcorrectcall,
    IcreateThrowUndefinedifholewithname,
    IcreateCallarg0,
    IcreateCallarg1,
    IcreateCallargs2,
    IcreateCallargs3,
    IcreateCallrange,
    IcreateWideCallrange,
    IcreateSupercallspread,
    IcreateApply,
    IcreateCallthis0,
    IcreateCallthis1,
    IcreateCallthis2,
    IcreateCallthis3,
    IcreateCallthisrange,
    IcreateWideCallthisrange,
    IcreateSupercallthisrange,
    IcreateWideSupercallthisrange,
    IcreateSupercallarrowrange,
    IcreateWideSupercallarrowrange,
    IcreateDefinegettersetterbyvalue,
    IcreateDefinefunc,
    IcreateDefinemethod,
    IcreateDefineclasswithbuffer,
    IcreateResumegenerator,
    IcreateGetresumemode,
    IcreateGettemplateobject,
    IcreateGetnextpropname,
    IcreateDelobjprop,
    IcreateSuspendgenerator,
    IcreateAsyncfunctionawaituncaught,
    IcreateCopydataproperties,
    IcreateStarrayspread,
    IcreateSetobjectwithproto,
    IcreateLdobjbyvalue,
    IcreateStobjbyvalue,
    IcreateStownbyvalue,
    IcreateLdsuperbyvalue,
    IcreateStsuperbyvalue,
    IcreateLdobjbyindex,
    IcreateWideLdobjbyindex,
    IcreateStobjbyindex,
    IcreateWideStobjbyindex,
    IcreateStownbyindex,
    IcreateWideStownbyindex,
    IcreateAsyncfunctionresolve,
    IcreateAsyncfunctionreject,
    IcreateCopyrestargs,
    IcreateWideCopyrestargs,
    IcreateLdlexvar,
    IcreateWideLdlexvar,
    IcreateStlexvar,
    IcreateWideStlexvar,
    IcreateGetmodulenamespace,
    IcreateWideGetmodulenamespace,
    IcreateStmodulevar,
    IcreateWideStmodulevar,
    IcreateTryldglobalbyname,
    IcreateTrystglobalbyname,
    IcreateLdglobalvar,
    IcreateStglobalvar,
    IcreateLdobjbyname,
    IcreateStobjbyname,
    IcreateStownbyname,
    IcreateLdsuperbyname,
    IcreateStsuperbyname,
    IcreateLdlocalmodulevar,
    IcreateWideLdlocalmodulevar,
    IcreateLdexternalmodulevar,
    IcreateWideLdexternalmodulevar,
    IcreateStownbyvaluewithnameset,
    IcreateStownbynamewithnameset,
    IcreateLdbigint,
    IcreateLdthisbyname,
    IcreateStthisbyname,
    IcreateLdthisbyvalue,
    IcreateStthisbyvalue,
    IcreateWideLdpatchvar,
    IcreateWideStpatchvar,
    IcreateDynamicimport,
    IcreateAsyncgeneratorreject,
    IcreateSetgeneratorstate,
    IcreateReturn,
    IcreateReturnundefined,
    IcreateIf,
};

// NOLINTEND(readability-identifier-naming)

}  // namespace libabckit::mock::isa_api_dynamic

AbckitIsaApiDynamic const *AbckitGetMockIsaApiDynamicImpl([[maybe_unused]] AbckitApiVersion version)
{
    return &libabckit::mock::isa_api_dynamic::g_isaApiDynamicImpl;
}
