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

#ifndef LIBABCKIT_SRC_ADAPTER_STATIC_IR_STATIC_H
#define LIBABCKIT_SRC_ADAPTER_STATIC_IR_STATIC_H

#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/c/isa/isa_dynamic.h"
#include "libabckit/c/isa/isa_static.h"

#include <cstdint>
#include <cstdarg>

namespace libabckit {

// ========================================
// Api for Graph manipulation
// ========================================

AbckitBasicBlock *GgetStartBasicBlockStatic(AbckitGraph *graph);
AbckitBasicBlock *GgetEndBasicBlockStatic(AbckitGraph *graph);
AbckitBasicBlock *GgetBasicBlockStatic(AbckitGraph *graph, uint32_t id);
uint32_t GgetNumberOfParametersStatic(AbckitGraph *graph);
AbckitInst *GgetParameterStatic(AbckitGraph *graph, uint32_t index);
void GinsertTryCatchStatic(AbckitBasicBlock *tryFirstBB, AbckitBasicBlock *tryLastBB, AbckitBasicBlock *catchBeginBB,
                           AbckitBasicBlock *catchEndBB);
uint32_t GgetNumberOfBasicBlocksStatic(AbckitGraph *graph);
void GdumpStatic(AbckitGraph *graph, int fd);
void GrunPassRemoveUnreachableBlocksStatic(AbckitGraph *graph);
void IdumpStatic(AbckitInst *inst, int fd);
bool GvisitBlocksRPOStatic(AbckitGraph *graph, void *data, bool (*cb)(AbckitBasicBlock *bb, void *data));

// ========================================
// Api for basic block manipulation
// ========================================

bool BBvisitSuccBlocksStatic(AbckitBasicBlock *curBasicBlock, void *data,
                             bool (*cb)(AbckitBasicBlock *succBasicBlock, void *data));
bool BBvisitPredBlocksStatic(AbckitBasicBlock *curBasicBlock, void *data,
                             bool (*cb)(AbckitBasicBlock *succBasicBlock, void *data));
bool BBvisitDominatedBlocksStatic(AbckitBasicBlock *basicBlock, void *data,
                                  bool (*cb)(AbckitBasicBlock *dominatedBasicBlock, void *data));
AbckitInst *BBgetFirstInstStatic(AbckitBasicBlock *basicBlock);
AbckitInst *BBgetLastInstStatic(AbckitBasicBlock *basicBlock);
AbckitGraph *BBgetGraphStatic(AbckitBasicBlock *basicBlock);
AbckitBasicBlock *BBgetTrueBranchStatic(AbckitBasicBlock *bb);
AbckitBasicBlock *BBgetFalseBranchStatic(AbckitBasicBlock *bb);
AbckitBasicBlock *BBcreateEmptyStatic(AbckitGraph *graph);
AbckitBasicBlock *BBgetSuccBlockStatic(AbckitBasicBlock *basicBlock, uint32_t index);
AbckitBasicBlock *BBgetPredBlockStatic(AbckitBasicBlock *basicBlock, uint32_t index);
AbckitBasicBlock *BBgetImmediateDominatorStatic(AbckitBasicBlock *basicBlock);
uint64_t BBgetSuccBlockCountStatic(AbckitBasicBlock *basicBlock);
uint64_t BBgetPredBlockCountStatic(AbckitBasicBlock *basicBlock);
uint32_t BBgetIdStatic(AbckitBasicBlock *basicBlock);
uint32_t BBgetNumberOfInstructionsStatic(AbckitBasicBlock *basicBlock);
AbckitBasicBlock *BBsplitBlockAfterInstructionStatic(AbckitBasicBlock *basicBlock, AbckitInst *inst, bool makeEdge);
void BBremoveAllInstsStatic(AbckitBasicBlock *basicBlock);
void BBaddInstFrontStatic(AbckitBasicBlock *basicBlock, AbckitInst *inst);
void BBaddInstBackStatic(AbckitBasicBlock *basicBlock, AbckitInst *inst);
void BBdumpStatic(AbckitBasicBlock *basicBlock, int fd);
void BBinsertSuccBlockStatic(AbckitBasicBlock *basicBlock, AbckitBasicBlock *succBlock, uint32_t index);
void BBappendSuccBlockStatic(AbckitBasicBlock *basicBlock, AbckitBasicBlock *succBlock);
void BBdisconnectSuccBlockStatic(AbckitBasicBlock *bb, size_t index);
bool BBisStartStatic(AbckitBasicBlock *basicBlock);
bool BBisEndStatic(AbckitBasicBlock *basicBlock);
bool BBisLoopHeadStatic(AbckitBasicBlock *basicBlock);
bool BBisLoopPreheadStatic(AbckitBasicBlock *basicBlock);
bool BBisTryBeginStatic(AbckitBasicBlock *basicBlock);
bool BBisTryStatic(AbckitBasicBlock *basicBlock);
bool BBisTryEndStatic(AbckitBasicBlock *basicBlock);
bool BBisCatchBeginStatic(AbckitBasicBlock *basicBlock);
bool BBisCatchStatic(AbckitBasicBlock *basicBlock);
bool BBcheckDominanceStatic(AbckitBasicBlock *basicBlock, AbckitBasicBlock *dominator);
AbckitInst *BBcreatePhiStatic(AbckitBasicBlock *bb, size_t argCount, std::va_list args);
AbckitInst *BBcreateCatchPhiStatic(AbckitBasicBlock *catchBegin, size_t argCount, std::va_list args);

// ========================================
// Api for instruction manipulation
// ========================================

AbckitInst *IcreateCallStaticStatic(AbckitGraph *graph, AbckitCoreFunction *inputFunction, size_t argCount,
                                    va_list argp);
AbckitInst *IcreateCallVirtualStatic(AbckitGraph *graph, AbckitInst *inputObj, AbckitCoreFunction *inputFunction,
                                     size_t argCount, va_list argp);
AbckitInst *IcreateLoadStringStatic(AbckitGraph *graph, AbckitString *str);
AbckitInst *GfindOrCreateConstantI32Static(AbckitGraph *graph, int32_t value);
AbckitInst *GfindOrCreateConstantI64Static(AbckitGraph *graph, int64_t value);
AbckitInst *GfindOrCreateConstantU64Static(AbckitGraph *graph, uint64_t value);
AbckitInst *GfindOrCreateConstantF64Static(AbckitGraph *graph, double value);

AbckitInst *IcreateCmpStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateNegStatic(AbckitGraph *graph, AbckitInst *input0);

AbckitInst *IcreateAddStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateSubStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateMulStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateDivStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateModStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateEqualsStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateStrictEqualsStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);

AbckitInst *IcreateAddIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm);
AbckitInst *IcreateSubIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm);
AbckitInst *IcreateMulIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm);
AbckitInst *IcreateDivIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm);
AbckitInst *IcreateModIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm);

AbckitInst *IcreateShlStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateShrStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateAShrStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);

AbckitInst *IcreateShlIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm);
AbckitInst *IcreateShrIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm);
AbckitInst *IcreateAShrIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm);

AbckitInst *IcreateNotStatic(AbckitGraph *graph, AbckitInst *input0);
AbckitInst *IcreateOrStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateXorStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateAndStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);

AbckitInst *IcreateOrIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm);
AbckitInst *IcreateXorIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm);
AbckitInst *IcreateAndIStatic(AbckitGraph *graph, AbckitInst *input0, uint64_t imm);

AbckitInst *IcreateLoadArrayStatic(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx,
                                   AbckitTypeId returnTypeId);
AbckitInst *IcreateStoreArrayStatic(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx, AbckitInst *value,
                                    AbckitTypeId valueTypeId);
AbckitInst *IcreateStoreArrayWideStatic(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx, AbckitInst *value,
                                        AbckitTypeId valueTypeId);
AbckitInst *IcreateLenArrayStatic(AbckitGraph *graph, AbckitInst *arr);
AbckitInst *IcreateLoadConstArrayStatic(AbckitGraph *graph, AbckitLiteralArray *literalArray);

AbckitInst *IcreateCheckCastStatic(AbckitGraph *graph, AbckitInst *inputObj, AbckitType *targetType);
AbckitInst *IcreateIsInstanceStatic(AbckitGraph *graph, AbckitInst *inputObj, AbckitType *targetType);
AbckitInst *IcreateLoadNullValueStatic(AbckitGraph *graph);
AbckitInst *IcreateCastStatic(AbckitGraph *graph, AbckitInst *input0, AbckitTypeId targetTypeId);
AbckitInst *IcreateIsUndefinedStatic(AbckitGraph *graph, AbckitInst *inputObj);
AbckitInst *IcreateNullCheckStatic(AbckitGraph *graph, AbckitInst *inputObj);
AbckitInst *IcreateThrowStatic(AbckitGraph *graph, AbckitInst *input0);

AbckitInst *IcreateReturnStatic(AbckitGraph *graph, AbckitInst *input0);
AbckitInst *IcreateReturnVoidStatic(AbckitGraph *graph);
AbckitInst *GcreateNullPtrStatic(AbckitGraph *graph);

int32_t IgetConstantValueI32Static(AbckitInst *inst);
int64_t IgetConstantValueI64Static(AbckitInst *inst);
uint64_t IgetConstantValueU64Static(AbckitInst *inst);
double IgetConstantValueF64Static(AbckitInst *inst);
AbckitLiteralArray *IgetLiteralArrayStatic(AbckitInst *inst);
void IsetLiteralArrayStatic(AbckitInst *inst, AbckitLiteralArray *la);
AbckitString *IgetStringStatic(AbckitInst *inst);
void IsetStringStatic(AbckitInst *inst, AbckitString *str);

AbckitInst *IcreateDynCallthis0Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynCallthis1Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateDynCallarg0Static(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynCallarg1Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynTryldglobalbynameStatic(AbckitGraph *graph, AbckitString *string);
AbckitInst *IcreateDynLdobjbynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);
AbckitInst *IcreateDynNewobjrangeStatic(AbckitGraph *graph, size_t argCount, std::va_list args);

AbckitInst *IcreateDynDefinefuncStatic(AbckitGraph *graph, AbckitCoreFunction *function, uint64_t imm0);

AbckitInst *IcreateDynNegStatic(AbckitGraph *graph, AbckitInst *acc);

AbckitInst *IcreateDynAdd2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynSub2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynMul2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynDiv2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynMod2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynExpStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

AbckitInst *IcreateDynShl2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynShr2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynAshr2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

AbckitInst *IcreateDynNotStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynOr2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynXor2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynAnd2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

AbckitInst *IcreateDynIncStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynDecStatic(AbckitGraph *graph, AbckitInst *acc);

AbckitInst *IcreateDynEqStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynNoteqStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynLessStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynLesseqStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynGreaterStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynGreatereqStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynStrictnoteqStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynStricteqStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

AbckitInst *IcreateDynIstrueStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynIsfalseStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynTonumberStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynTonumericStatic(AbckitGraph *graph, AbckitInst *acc);

AbckitInst *IcreateDynThrowStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynThrowNotexistsStatic(AbckitGraph *graph);
AbckitInst *IcreateDynThrowPatternnoncoercibleStatic(AbckitGraph *graph);
AbckitInst *IcreateDynThrowDeletesuperpropertyStatic(AbckitGraph *graph);
AbckitInst *IcreateDynThrowConstassignmentStatic(AbckitGraph *graph, AbckitInst *input0);
AbckitInst *IcreateDynThrowIfnotobjectStatic(AbckitGraph *graph, AbckitInst *input0);
AbckitInst *IcreateDynThrowUndefinedifholeStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateDynThrowIfsupernotcorrectcallStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0);
AbckitInst *IcreateDynThrowUndefinedifholewithnameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);

AbckitInst *IcreateDynLdnanStatic(AbckitGraph *graph);
AbckitInst *IcreateDynLdinfinityStatic(AbckitGraph *graph);
AbckitInst *IcreateDynLdundefinedStatic(AbckitGraph *graph);
AbckitInst *IcreateDynLdnullStatic(AbckitGraph *graph);
AbckitInst *IcreateDynLdsymbolStatic(AbckitGraph *graph);
AbckitInst *IcreateDynLdglobalStatic(AbckitGraph *graph);
AbckitInst *IcreateDynLdtrueStatic(AbckitGraph *graph);
AbckitInst *IcreateDynLdfalseStatic(AbckitGraph *graph);
AbckitInst *IcreateDynLdholeStatic(AbckitGraph *graph);
AbckitInst *IcreateDynLdfunctionStatic(AbckitGraph *graph);
AbckitInst *IcreateDynLdnewtargetStatic(AbckitGraph *graph);
AbckitInst *IcreateDynLdthisStatic(AbckitGraph *graph);

AbckitInst *IcreateDynTypeofStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynIsinStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynInstanceofStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynReturnStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynReturnundefinedStatic(AbckitGraph *graph);

AbckitInst *IcreateDynStownbynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0);
AbckitInst *IcreateDynStownbyvalueStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateDynStownbyindexStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0);
AbckitInst *IcreateDynWideStownbyindexStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0);
AbckitInst *IcreateDynStownbynamewithnamesetStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                                   AbckitInst *input0);
AbckitInst *IcreateDynStownbyvaluewithnamesetStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                    AbckitInst *input1);
AbckitInst *IcreateDynStobjbyvalueStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateDynStobjbyindexStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0);
AbckitInst *IcreateDynWideStobjbyindexStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, uint64_t imm0);
AbckitInst *IcreateDynStobjbynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string, AbckitInst *input0);

AbckitInst *IcreateDynCreateemptyobjectStatic(AbckitGraph *graph);

AbckitInst *IcreateDynPoplexenvStatic(AbckitGraph *graph);
AbckitInst *IcreateDynGetunmappedargsStatic(AbckitGraph *graph);
AbckitInst *IcreateDynAsyncfunctionenterStatic(AbckitGraph *graph);
AbckitInst *IcreateDynDebuggerStatic(AbckitGraph *graph);
AbckitInst *IcreateDynGetpropiteratorStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynGetiteratorStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynGetasynciteratorStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynLdprivatepropertyStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1);
AbckitInst *IcreateDynStprivatepropertyStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1,
                                              AbckitInst *input0);
AbckitInst *IcreateDynTestinStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1);
AbckitInst *IcreateDynDefinefieldbynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                              AbckitInst *input0);
AbckitInst *IcreateDynDefinepropertybynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                                 AbckitInst *input0);
AbckitInst *IcreateDynCreateemptyarrayStatic(AbckitGraph *graph);
AbckitInst *IcreateDynCreategeneratorobjStatic(AbckitGraph *graph, AbckitInst *input0);
AbckitInst *IcreateDynCreateiterresultobjStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateDynCreateobjectwithexcludedkeysStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                                         uint64_t imm0, std::va_list args);
AbckitInst *IcreateDynWideCreateobjectwithexcludedkeysStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                                             uint64_t imm0, std::va_list args);

AbckitInst *IcreateDynDefineclasswithbufferStatic(AbckitGraph *graph, AbckitCoreFunction *function,
                                                  AbckitLiteralArray *literalArray, uint64_t imm0, AbckitInst *input0);
AbckitInst *IcreateDynNewobjapplyStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynWideNewobjrangeStatic(AbckitGraph *graph, size_t argCount, std::va_list args);
AbckitInst *IcreateDynNewlexenvStatic(AbckitGraph *graph, uint64_t imm0);
AbckitInst *IcreateDynWideNewlexenvStatic(AbckitGraph *graph, uint64_t imm0);
AbckitInst *IcreateDynNewlexenvwithnameStatic(AbckitGraph *graph, uint64_t imm0, AbckitLiteralArray *literalArray);
AbckitInst *IcreateDynWideNewlexenvwithnameStatic(AbckitGraph *graph, uint64_t imm0, AbckitLiteralArray *literalArray);

AbckitInst *IcreateDynCreateasyncgeneratorobjStatic(AbckitGraph *graph, AbckitInst *input0);
AbckitInst *IcreateDynAsyncgeneratorresolveStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                                  AbckitInst *input2);
AbckitInst *IcreateDynCallruntimeNotifyconcurrentresultStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynCallruntimeDefinefieldbyvalueStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                          AbckitInst *input1);
AbckitInst *IcreateDynCallruntimeDefinefieldbyindexStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                          AbckitInst *input0);
AbckitInst *IcreateDynCallruntimeTopropertykeyStatic(AbckitGraph *graph, AbckitInst *acc);

AbckitInst *IcreateDynCallruntimeDefineprivatepropertyStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                             uint64_t imm1, AbckitInst *input0);
AbckitInst *IcreateDynCallruntimeCallinitStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);

AbckitInst *IcreateDynCallruntimeDefinesendableclassStatic(AbckitGraph *graph, AbckitCoreFunction *function,
                                                           AbckitLiteralArray *literalArray, uint64_t imm0,
                                                           AbckitInst *input0);
AbckitInst *IcreateDynCallruntimeLdsendableclassStatic(AbckitGraph *graph, uint64_t imm0);
AbckitInst *IcreateDynCallruntimeLdsendableexternalmodulevarStatic(AbckitGraph *graph, uint64_t imm0);
AbckitInst *IcreateDynCallruntimeWideldsendableexternalmodulevarStatic(AbckitGraph *graph, uint64_t imm0);
AbckitInst *IcreateDynCallruntimeNewsendableenvStatic(AbckitGraph *graph, uint64_t imm0);
AbckitInst *IcreateDynCallruntimeWidenewsendableenvStatic(AbckitGraph *graph, uint64_t imm0);
AbckitInst *IcreateDynCallruntimeStsendablevarStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1);
AbckitInst *IcreateDynCallruntimeWidestsendablevarStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0,
                                                         uint64_t imm1);
AbckitInst *IcreateDynCallruntimeLdsendablevarStatic(AbckitGraph *graph, uint64_t imm0, uint64_t imm1);
AbckitInst *IcreateDynCallruntimeWideldsendablevarStatic(AbckitGraph *graph, uint64_t imm0, uint64_t imm1);
AbckitInst *IcreateDynCallruntimeIstrueStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynCallruntimeIsfalseStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynCallruntimeCreateprivatepropertyStatic(AbckitGraph *graph, uint64_t imm0,
                                                             AbckitLiteralArray *literalArray);
AbckitInst *IcreateDynCallargs2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateDynCallargs3Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                                      AbckitInst *input2);
AbckitInst *IcreateDynCallrangeStatic(AbckitGraph *graph, AbckitInst *acc, size_t argCount, std::va_list args);
AbckitInst *IcreateDynWideCallrangeStatic(AbckitGraph *graph, AbckitInst *acc, size_t argCount, std::va_list args);
AbckitInst *IcreateDynSupercallspreadStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynApplyStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateDynCallthis2Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                                      AbckitInst *input2);
// CC-OFFNXT(G.FUN.01) This is function from public API
AbckitInst *IcreateDynCallthis3Static(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1,
                                      AbckitInst *input2, AbckitInst *input3);
AbckitInst *IcreateDynCallthisrangeStatic(AbckitGraph *graph, AbckitInst *acc, size_t argCount, std::va_list args);
AbckitInst *IcreateDynWideCallthisrangeStatic(AbckitGraph *graph, AbckitInst *acc, size_t argCount, std::va_list args);
AbckitInst *IcreateDynSupercallthisrangeStatic(AbckitGraph *graph, size_t argCount, std::va_list args);
AbckitInst *IcreateDynWideSupercallthisrangeStatic(AbckitGraph *graph, size_t argCount, std::va_list args);
AbckitInst *IcreateDynSupercallarrowrangeStatic(AbckitGraph *graph, AbckitInst *acc, size_t argCount,
                                                std::va_list args);
AbckitInst *IcreateDynWideSupercallarrowrangeStatic(AbckitGraph *graph, AbckitInst *acc, size_t argCount,
                                                    std::va_list args);
// CC-OFFNXT(G.FUN.01) This is function from public API
AbckitInst *IcreateDynDefinegettersetterbyvalueStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0,
                                                      AbckitInst *input1, AbckitInst *input2, AbckitInst *input3);

AbckitInst *IcreateDynResumegeneratorStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynGetresumemodeStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynGettemplateobjectStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynGetnextpropnameStatic(AbckitGraph *graph, AbckitInst *input0);
AbckitInst *IcreateDynDelobjpropStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynSuspendgeneratorStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynAsyncfunctionawaituncaughtStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynCopydatapropertiesStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynStarrayspreadStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateDynSetobjectwithprotoStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynLdobjbyvalueStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynLdsuperbyvalueStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynStsuperbyvalueStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0, AbckitInst *input1);
AbckitInst *IcreateDynLdobjbyindexStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0);
AbckitInst *IcreateDynWideLdobjbyindexStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0);
AbckitInst *IcreateDynAsyncfunctionresolveStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynAsyncfunctionrejectStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynCopyrestargsStatic(AbckitGraph *graph, uint64_t imm0);
AbckitInst *IcreateDynWideCopyrestargsStatic(AbckitGraph *graph, uint64_t imm0);
AbckitInst *IcreateDynLdlexvarStatic(AbckitGraph *graph, uint64_t imm0, uint64_t imm1);
AbckitInst *IcreateDynWideLdlexvarStatic(AbckitGraph *graph, uint64_t imm0, uint64_t imm1);
AbckitInst *IcreateDynStlexvarStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1);
AbckitInst *IcreateDynWideStlexvarStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0, uint64_t imm1);

AbckitInst *IcreateDynTrystglobalbynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);
AbckitInst *IcreateDynLdglobalvarStatic(AbckitGraph *graph, AbckitString *string);
AbckitInst *IcreateDynStglobalvarStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);
AbckitInst *IcreateDynLdsuperbynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);
AbckitInst *IcreateDynStsuperbynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string,
                                          AbckitInst *input0);

AbckitInst *IcreateDynLdbigintStatic(AbckitGraph *graph, AbckitString *string);
AbckitInst *IcreateDynLdthisbynameStatic(AbckitGraph *graph, AbckitString *string);
AbckitInst *IcreateDynStthisbynameStatic(AbckitGraph *graph, AbckitInst *acc, AbckitString *string);
AbckitInst *IcreateDynLdthisbyvalueStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynStthisbyvalueStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynWideLdpatchvarStatic(AbckitGraph *graph, uint64_t imm0);
AbckitInst *IcreateDynWideStpatchvarStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0);
AbckitInst *IcreateDynDynamicimportStatic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynAsyncgeneratorrejectStatic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynSetgeneratorstateStatic(AbckitGraph *graph, AbckitInst *acc, uint64_t imm0);

AbckitInst *IcreateDynGetmodulenamespaceStatic(AbckitGraph *graph, AbckitCoreModule *md);
AbckitInst *IcreateDynWideGetmodulenamespaceStatic(AbckitGraph *graph, AbckitCoreModule *md);
AbckitInst *IcreateDynLdexternalmodulevarStatic(AbckitGraph *graph, AbckitCoreImportDescriptor *id);
AbckitInst *IcreateDynWideLdexternalmodulevarStatic(AbckitGraph *graph, AbckitCoreImportDescriptor *id);
AbckitInst *IcreateDynLdlocalmodulevarStatic(AbckitGraph *graph, AbckitCoreExportDescriptor *ed);
AbckitInst *IcreateDynWideLdlocalmodulevarStatic(AbckitGraph *graph, AbckitCoreExportDescriptor *ed);
AbckitInst *IcreateDynStmodulevarStatic(AbckitGraph *graph, AbckitInst *acc, AbckitCoreExportDescriptor *ed);
AbckitInst *IcreateDynWideStmodulevarStatic(AbckitGraph *graph, AbckitInst *acc, AbckitCoreExportDescriptor *ed);
AbckitInst *IcreateDynCreatearraywithbufferStatic(AbckitGraph *graph, AbckitLiteralArray *literalArray);
AbckitInst *IcreateDynCreateobjectwithbufferStatic(AbckitGraph *graph, AbckitLiteralArray *literalArray);

AbckitInst *IcreateDynDefinemethodStatic(AbckitGraph *graph, AbckitInst *acc, AbckitCoreFunction *function,
                                         uint64_t imm0);

void IremoveStatic(AbckitInst *inst);
uint32_t IgetIdStatic(AbckitInst *inst);
AbckitIsaApiStaticOpcode IgetOpcodeStaticStatic(AbckitInst *inst);
AbckitIsaApiDynamicOpcode IgetOpcodeDynamicStatic(AbckitInst *inst);

AbckitInst *IgetNextStatic(AbckitInst *instnext);
AbckitInst *IgetPrevStatic(AbckitInst *instprev);
void IinsertAfterStatic(AbckitInst *inst, AbckitInst *next);
void IinsertBeforeStatic(AbckitInst *inst, AbckitInst *prev);
bool IcheckDominanceStatic(AbckitInst *inst, AbckitInst *dominator);
bool IvisitUsersStatic(AbckitInst *inst, void *data, bool (*cb)(AbckitInst *user, void *data));
bool IvisitInputsStatic(AbckitInst *inst, void *data, bool (*cb)(AbckitInst *input, size_t inputIdx, void *data));
uint32_t IgetUserCountStatic(AbckitInst *inst);
void IsetImmediateStatic(AbckitInst *inst, size_t idx, uint64_t imm);
AbckitBitImmSize IgetImmediateSizeStatic(AbckitInst *inst, size_t idx);

uint64_t IgetInputCountStatic(AbckitInst *inst);
AbckitInst *IgetInputStatic(AbckitInst *inst, size_t index);
void IsetInputStatic(AbckitInst *inst, AbckitInst *input, int32_t index);
void IsetInputsStatic(AbckitInst *inst, size_t argCount, std::va_list args);
void IappendInputStatic(AbckitInst *inst, AbckitInst *input);
AbckitType *IgetTypeStatic(AbckitInst *inst);
AbckitTypeId IgetTargetTypeStatic(AbckitInst *inst);
void IsetTargetTypeStatic(AbckitInst *inst, AbckitTypeId t);
AbckitCoreFunction *IgetFunctionStatic(AbckitInst *inst);
void IsetFunctionStatic(AbckitInst *inst, AbckitCoreFunction *function);
void IsetClassStatic(AbckitInst *inst, AbckitCoreClass *klass);
AbckitCoreClass *IgetClassStatic(AbckitInst *inst);
uint64_t IgetImmediateStatic(AbckitInst *inst, size_t idx);
uint64_t IgetImmediateCountStatic(AbckitInst *inst);
AbckitInst *IcreateIfStaticStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                  AbckitIsaApiStaticConditionCode cc);
AbckitInst *IcreateIfDynamicStatic(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                   AbckitIsaApiDynamicConditionCode cc);
AbckitInst *IcreateTryStatic(AbckitGraph *graph);
AbckitBasicBlock *IgetBasicBlockStatic(AbckitInst *inst);
AbckitGraph *IgetGraphStatic(AbckitInst *inst);
AbckitCoreModule *IgetModuleStatic(AbckitInst *inst);
uint32_t GetImportDescriptorIdxDynamic(AbckitGraph *graph, AbckitCoreImportDescriptor *id);
uint32_t GetExportDescriptorIdxDynamic(AbckitGraph *graph, AbckitCoreExportDescriptor *ed);
uint64_t GetModuleIndex(AbckitGraph *graph, AbckitCoreModule *md);
void IsetModuleStatic(AbckitInst *inst, AbckitCoreModule *md);
AbckitCoreImportDescriptor *IgetImportDescriptorStatic(AbckitInst *inst);
void IsetImportDescriptorStatic(AbckitInst *inst, AbckitCoreImportDescriptor *id);
AbckitCoreExportDescriptor *IgetExportDescriptorStatic(AbckitInst *inst);
void IsetExportDescriptorStatic(AbckitInst *inst, AbckitCoreExportDescriptor *ed);
AbckitIsaApiStaticConditionCode IgetConditionCodeStaticStatic(AbckitInst *inst);
AbckitIsaApiDynamicConditionCode IgetConditionCodeDynamicStatic(AbckitInst *inst);
void IsetConditionCodeStaticStatic(AbckitInst *inst, AbckitIsaApiStaticConditionCode cc);
void IsetConditionCodeDynamicStatic(AbckitInst *inst, AbckitIsaApiDynamicConditionCode cc);

AbckitInst *IcreateDynCallarg0Dynamic(AbckitGraph *graph, AbckitInst *acc);
AbckitInst *IcreateDynCallarg1Dynamic(AbckitGraph *graph, AbckitInst *acc, AbckitInst *input0);
AbckitInst *IcreateDynIfStatic(AbckitGraph *graph, AbckitInst *input, AbckitIsaApiDynamicConditionCode cc);

AbckitInst *IcreateNewArrayStatic(AbckitGraph *graph, AbckitCoreClass *inputClass, AbckitInst *inputSize);

AbckitInst *IcreateNewObjectStatic(AbckitGraph *graph, AbckitCoreClass *inputClass);
AbckitInst *IcreateInitObjectStatic(AbckitGraph *graph, AbckitCoreFunction *inputFunction, size_t argCount,
                                    va_list argp);
AbckitInst *IcreateLoadObjectStatic(AbckitGraph *graph, AbckitInst *inputObj, AbckitString *field,
                                    AbckitTypeId returnTypeId);
AbckitInst *IcreateStoreObjectStatic(AbckitGraph *graph, AbckitInst *inputObj, AbckitString *fieldId, AbckitInst *value,
                                     AbckitTypeId typeId);
bool IcheckIsCallStatic(AbckitInst *inst);
}  // namespace libabckit

#endif
