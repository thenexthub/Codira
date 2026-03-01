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
#ifndef LIBABCKIT_TESTS_INVALID_HELPERS
#define LIBABCKIT_TESTS_INVALID_HELPERS

#include "libabckit/c/metadata_core.h"
#include "libabckit/c/ir_core.h"

#include "helpers/helpers.h"

namespace libabckit::test::helpers_wrong_ctx {

void TestWrongCtx(void (*apiToCheck)(AbckitCoreFunction *method, AbckitGraph *graph));
void TestWrongCtx(void (*apiToCheck)(AbckitBasicBlock *bb1, AbckitInst *inst));
void TestWrongCtx(bool (*apiToCheck)(AbckitBasicBlock *bb1, AbckitBasicBlock *bb2));
void TestWrongCtx(void (*apiToCheck)(AbckitBasicBlock *bb1, AbckitBasicBlock *bb2, bool val));
void TestWrongCtx(void (*apiToCheck)(AbckitBasicBlock *bb1, AbckitBasicBlock *bb2));
void TestWrongCtx(void (*apiToCheck)(AbckitBasicBlock *bb1, AbckitBasicBlock *bb2, uint32_t index));
void TestWrongCtx(bool (*apiToCheck)(AbckitInst *inst, AbckitInst *dominator));

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreFunction *method, size_t argCount, ...));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitTypeId targetType));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                            AbckitInst *input2));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                            AbckitInst *input2, AbckitInst *input3));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, size_t argCount, ...));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0,
                                            ...));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, uint64_t imm0, AbckitInst *inst1));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, uint64_t imm0, uint64_t imm1,
                                            AbckitInst *inst1));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreFunction *m, AbckitLiteralArray *litarr,
                                            uint64_t val, AbckitInst *inst));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, uint64_t imm0, uint64_t imm1));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                            AbckitInst *input2, AbckitInst *input3, AbckitInst *input4));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm, AbckitInst *inst0, AbckitInst *inst1));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitString *input1,
                                            AbckitInst *input2));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreFunction *method, uint64_t imm));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *acc, AbckitCoreFunction *method,
                                            uint64_t imm0));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreModule *m));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0,
                                            AbckitIsaApiDynamicConditionCode cc));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitString *input1));

void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx,
                                            AbckitInst *value, AbckitTypeId valueTypeId));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                            AbckitTypeId typeId));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitString *field,
                                            AbckitTypeId returnTypeId));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitCoreExportDescriptor *e));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreClass *klass));
void TestWrongCtx(void (*apiToCheck)(AbckitInst *inst, AbckitInst *next));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                            AbckitIsaApiStaticConditionCode));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreClass *klass, AbckitInst *input0));
void TestWrongCtx(void (*apiToCheck)(AbckitInst *input0, AbckitCoreFunction *method));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitType *type));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitCoreFunction *method,
                                            size_t argCount, ...));
void TestWrongCtx(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inputObj, AbckitString *fieldId,
                                            AbckitInst *value, AbckitTypeId typeId));

}  // namespace libabckit::test::helpers_wrong_ctx

#endif /*LIBABCKIT_TESTS_INVALID_HELPERS */
