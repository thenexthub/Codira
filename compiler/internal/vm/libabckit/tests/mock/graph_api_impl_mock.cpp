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

#include <cstring>
#include <iostream>
#include <gtest/gtest.h>

namespace libabckit::mock::graph_api {

// NOLINTBEGIN(readability-identifier-naming)

enum AbckitIsaType GgetIsa(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_ENUM_ISA_TYPE;
}

AbckitFile *GgetFile(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_FILE;
}

AbckitBasicBlock *GgetStartBasicBlock(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_BB;
}

AbckitBasicBlock *GgetEndBasicBlock(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_BB;
}

uint32_t GgetNumberOfBasicBlocks(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_U32;
}

bool GvisitBlocksRpo(AbckitGraph *graph, void *data, bool (*cb)(AbckitBasicBlock *basicBlock, void *data))
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    cb(DEFAULT_BB, data);
    return DEFAULT_BOOL;
}

AbckitBasicBlock *GgetBasicBlock(AbckitGraph *graph, uint32_t id)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(id == DEFAULT_U32);
    return DEFAULT_BB;
}

AbckitInst *GgetParameter(AbckitGraph *graph, uint32_t index)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(index == DEFAULT_U32);
    return DEFAULT_INST;
}

uint32_t GgetNumberOfParameters(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_U32;
}

void GinsertTryCatch(AbckitBasicBlock *tryFirstBB, AbckitBasicBlock *tryLastBB, AbckitBasicBlock *catchBeginBB,
                     AbckitBasicBlock *catchEndBB)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(tryFirstBB == DEFAULT_BB);
    EXPECT_TRUE(tryLastBB == DEFAULT_BB);
    EXPECT_TRUE(catchBeginBB == DEFAULT_BB);
    EXPECT_TRUE(catchEndBB == DEFAULT_BB);
}

void Gdump(AbckitGraph *graph, int32_t fd)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(fd == DEFAULT_I32);
}

AbckitInst *GfindOrCreateConstantI32(AbckitGraph *graph, int32_t value)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(value == DEFAULT_I32);
    return DEFAULT_INST;
}

AbckitInst *GfindOrCreateConstantI64(AbckitGraph *graph, int64_t value)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(value == DEFAULT_I64);
    return DEFAULT_INST;
}

AbckitInst *GfindOrCreateConstantU64(AbckitGraph *graph, uint64_t value)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(value == DEFAULT_U64);
    return DEFAULT_INST;
}

AbckitInst *GfindOrCreateConstantF64(AbckitGraph *graph, double value)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    EXPECT_TRUE(value == DEFAULT_DOUBLE);
    return DEFAULT_INST;
}

void GrunPassRemoveUnreachableBlocks(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
}

AbckitBasicBlock *BBCreateEmpty(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(graph == DEFAULT_GRAPH);
    return DEFAULT_BB;
}

uint32_t BBGetId(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_U32;
}

AbckitGraph *BBGetGraph(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_GRAPH;
}

uint64_t BBGetPredBlockCount(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_I64;
}

AbckitBasicBlock *BBGetPredBlock(AbckitBasicBlock *basicBlock, uint32_t index)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    EXPECT_TRUE(index == DEFAULT_U32);
    return DEFAULT_BB;
}

bool BBVisitPredBlocks(AbckitBasicBlock *basicBlock, void *data,
                       bool (*cb)(AbckitBasicBlock *predBasicBlock, void *data))
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    cb(DEFAULT_BB, data);
    return DEFAULT_BB;
}

uint64_t BBGetSuccBlockCount(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_I64;
}

AbckitBasicBlock *BBGetSuccBlock(AbckitBasicBlock *basicBlock, uint32_t index)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    EXPECT_TRUE(index == DEFAULT_U32);
    return DEFAULT_BB;
}

void BBInsertSuccBlock(AbckitBasicBlock *basicBlock, AbckitBasicBlock *succBlock, uint32_t index)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    EXPECT_TRUE(succBlock == DEFAULT_BB);
    EXPECT_TRUE(index == DEFAULT_U32);
}

void BBAppendSuccBlock(AbckitBasicBlock *basicBlock, AbckitBasicBlock *succBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    EXPECT_TRUE(succBlock == DEFAULT_BB);
}

void BBdisconnectSuccBlock(AbckitBasicBlock *basicBlock, uint32_t index)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    EXPECT_TRUE(index == DEFAULT_U32);
}

bool BBVisitSuccBlocks(AbckitBasicBlock *basicBlock, void *data,
                       bool (*cb)(AbckitBasicBlock *succBasicBlock, void *data))
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    cb(DEFAULT_BB, data);
    return DEFAULT_BOOL;
}

AbckitBasicBlock *BBGetTrueBranch(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_BB;
}

AbckitBasicBlock *BBGetFalseBranch(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_BB;
}

AbckitBasicBlock *BBSplitBlockAfterInstruction(AbckitBasicBlock *basicBlock, AbckitInst *inst, bool makeEdge)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(makeEdge == DEFAULT_BOOL);
    return DEFAULT_BB;
}

void BBAddInstFront(AbckitBasicBlock *basicBlock, AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    EXPECT_TRUE(inst == DEFAULT_INST);
}

void BBAddInstBack(AbckitBasicBlock *basicBlock, AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    EXPECT_TRUE(inst == DEFAULT_INST);
}

void BBRemoveAllInsts(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
}

AbckitInst *BBGetFirstInst(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_INST;
}

AbckitInst *BBGetLastInst(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_INST;
}

uint32_t BBGetNumberOfInstructions(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_U32;
}

AbckitBasicBlock *BBGetImmediateDominator(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_BB;
}

bool BBCheckDominance(AbckitBasicBlock *basicBlock, AbckitBasicBlock *dominator)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    EXPECT_TRUE(dominator == DEFAULT_BB);
    return DEFAULT_BOOL;
}

bool BBVisitDominatedBlocks(AbckitBasicBlock *basicBlock, void *data,
                            bool (*cb)(AbckitBasicBlock *dominatedBasicBlock, void *data))
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    cb(DEFAULT_BB, data);
    return DEFAULT_BOOL;
}

bool BBIsStart(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_BOOL;
}

bool BBIsEnd(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_BOOL;
}

bool BBIsLoopHead(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_BOOL;
}

bool BBIsLoopPrehead(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_BOOL;
}

bool BBIsTryBegin(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_BOOL;
}

bool BBIsTry(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_BOOL;
}

bool BBIsTryEnd(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_BOOL;
}

bool BBIsCatchBegin(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_BOOL;
}

bool BBIsCatch(AbckitBasicBlock *basicBlock)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    return DEFAULT_BOOL;
}

void BBDump(AbckitBasicBlock *basicBlock, int32_t fd)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    EXPECT_TRUE(fd == DEFAULT_I32);
}

AbckitInst *BBCreatePhi(AbckitBasicBlock *basicBlock, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(basicBlock == DEFAULT_BB);
    EXPECT_TRUE(argCount == 2U);
    return DEFAULT_INST;
}

AbckitInst *BBCreateCatchPhi(AbckitBasicBlock *catchBegin, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(catchBegin == DEFAULT_BB);
    EXPECT_TRUE(argCount == 2U);
    return DEFAULT_INST;
}

void Iremove(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
}

uint32_t IgetId(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_U32;
}

AbckitInst *IgetNext(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_INST;
}

AbckitInst *IgetPrev(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_INST;
}

void IinsertAfter(AbckitInst *newInst, AbckitInst *ref)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(newInst == DEFAULT_INST);
    EXPECT_TRUE(ref == DEFAULT_INST);
}

void IinsertBefore(AbckitInst *newInst, AbckitInst *ref)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(newInst == DEFAULT_INST);
    EXPECT_TRUE(ref == DEFAULT_INST);
}

AbckitType *IgetType(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_TYPE;
}

AbckitBasicBlock *IgetBasicBlock(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_BB;
}

AbckitGraph *IgetGraph(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_GRAPH;
}

bool IcheckDominance(AbckitInst *inst, AbckitInst *dominator)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(dominator == DEFAULT_INST);
    return DEFAULT_BOOL;
}

bool IcheckIsCall(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_BOOL;
}

uint32_t IgetUserCount(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_U32;
}

bool IvisitUsers(AbckitInst *inst, void *data, bool (*cb)(AbckitInst *user, void *data))
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    cb(DEFAULT_INST, data);
    return DEFAULT_BOOL;
}

uint32_t IgetInputCount(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_U32;
}

AbckitInst *IgetInput(AbckitInst *inst, uint32_t index)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(index == DEFAULT_U32);
    return DEFAULT_INST;
}

bool IvisitInputs(AbckitInst *inst, void *data, bool (*cb)(AbckitInst *input, size_t inputIdx, void *data))
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    cb(DEFAULT_INST, DEFAULT_SIZE_T, data);
    return DEFAULT_BOOL;
}

void IsetInput(AbckitInst *inst, AbckitInst *input, uint32_t index)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(input == DEFAULT_INST);
    EXPECT_TRUE(index == DEFAULT_U32);
}

void IsetInputs(AbckitInst *inst, size_t argCount, ...)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(argCount == 2U);
}

void IappendInput(AbckitInst *inst, AbckitInst *input)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(input == DEFAULT_INST);
}

void Idump(AbckitInst *inst, int32_t fd)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(fd == DEFAULT_I32);
}

AbckitCoreFunction *IgetFunction(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_CORE_FUNCTION;
}

void IsetFunction(AbckitInst *inst, AbckitCoreFunction *function)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(function == DEFAULT_CORE_FUNCTION);
}

uint64_t IgetImmediate(AbckitInst *inst, size_t index)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(index == DEFAULT_SIZE_T);
    return DEFAULT_I64;
}

void IsetImmediate(AbckitInst *inst, size_t index, uint64_t imm)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(index == DEFAULT_SIZE_T);
    EXPECT_TRUE(imm == DEFAULT_U64);
}

AbckitBitImmSize IgetImmediateSize(AbckitInst *inst, size_t index)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(index == DEFAULT_SIZE_T);
    return DEFAULT_ENUM_BITSIZE;
}

uint64_t IgetImmediateCount(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_I64;
}

AbckitLiteralArray *IgetLiteralArray(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_LITERAL_ARRAY;
}

void IsetLiteralArray(AbckitInst *inst, AbckitLiteralArray *la)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(la == DEFAULT_LITERAL_ARRAY);
}

AbckitString *IgetString(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_STRING;
}

void IsetString(AbckitInst *inst, AbckitString *s)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    EXPECT_TRUE(s == DEFAULT_STRING);
}

int32_t IgetConstantValueI32(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_I32;
}

int64_t IgetConstantValueI64(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_I64;
}

uint64_t IgetConstantValueU64(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_I64;
}

double IgetConstantValueF64(AbckitInst *inst)
{
    g_calledFuncs.push(__func__);

    EXPECT_TRUE(inst == DEFAULT_INST);
    return DEFAULT_DOUBLE;
}

AbckitGraphApi g_graphApiImpl = {
    GgetIsa,
    GgetFile,

    // ========================================
    // Api for Graph manipulation
    // ========================================

    GgetStartBasicBlock,
    GgetEndBasicBlock,
    GgetNumberOfBasicBlocks,
    GvisitBlocksRpo,
    GgetBasicBlock,
    GgetParameter,
    GgetNumberOfParameters,
    GinsertTryCatch,
    Gdump,
    GfindOrCreateConstantI32,
    GfindOrCreateConstantI64,
    GfindOrCreateConstantU64,
    GfindOrCreateConstantF64,
    GrunPassRemoveUnreachableBlocks,

    // ========================================
    // Api for basic block manipulation
    // ========================================

    BBCreateEmpty,
    BBGetId,
    BBGetGraph,
    BBGetPredBlockCount,
    BBGetPredBlock,
    BBVisitPredBlocks,
    BBGetSuccBlockCount,
    BBGetSuccBlock,
    BBInsertSuccBlock,
    BBAppendSuccBlock,
    BBdisconnectSuccBlock,
    BBVisitSuccBlocks,
    BBGetTrueBranch,
    BBGetFalseBranch,
    BBSplitBlockAfterInstruction,
    BBAddInstFront,
    BBAddInstBack,
    BBRemoveAllInsts,
    BBGetFirstInst,
    BBGetLastInst,
    BBGetNumberOfInstructions,
    BBGetImmediateDominator,
    BBCheckDominance,
    BBVisitDominatedBlocks,
    BBIsStart,
    BBIsEnd,
    BBIsLoopHead,
    BBIsLoopPrehead,
    BBIsTryBegin,
    BBIsTry,
    BBIsTryEnd,
    BBIsCatchBegin,
    BBIsCatch,
    BBDump,
    BBCreatePhi,
    BBCreateCatchPhi,

    // ========================================
    // Api for instruction manipulation
    // ========================================

    Iremove,
    IgetId,
    IgetNext,
    IgetPrev,
    IinsertAfter,
    IinsertBefore,
    IgetType,
    IgetBasicBlock,
    IgetGraph,
    IcheckDominance,
    IcheckIsCall,
    IgetUserCount,
    IvisitUsers,
    IgetInputCount,
    IgetInput,
    IvisitInputs,
    IsetInput,
    IsetInputs,
    IappendInput,
    Idump,
    IgetFunction,
    IsetFunction,
    IgetImmediate,
    IsetImmediate,
    IgetImmediateSize,
    IgetImmediateCount,
    IgetLiteralArray,
    IsetLiteralArray,
    IgetString,
    IsetString,
    IgetConstantValueI32,
    IgetConstantValueI64,
    IgetConstantValueU64,
    IgetConstantValueF64,
};

// NOLINTEND(readability-identifier-naming)

}  // namespace libabckit::mock::graph_api

AbckitGraphApi const *AbckitGetMockGraphApiImpl([[maybe_unused]] AbckitApiVersion version)
{
    return &libabckit::mock::graph_api::g_graphApiImpl;
}
