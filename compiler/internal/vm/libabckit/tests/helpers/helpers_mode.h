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

namespace libabckit::test::helpers_mode {

void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint32_t index), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, int64_t value), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t value), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm0, uint64_t imm1), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm, AbckitInst *inst), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm, AbckitLiteralArray *inst), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm, AbckitInst *inst0, AbckitInst *inst1),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, double value), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitString *str), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitString *str, uint64_t imm), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreExportDescriptor *e), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreImportDescriptor *i), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                        AbckitTypeId typeId),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitCoreExportDescriptor *e),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitIsaApiDynamicConditionCode cc),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, uint64_t imm0, uint64_t imm1),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, uint64_t imm0, AbckitInst *inst1),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, uint64_t imm0, uint64_t imm1,
                                        AbckitInst *inst1),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, AbckitCoreFunction *method,
                                        size_t argCount, ...),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, size_t argCount, ...), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm, ...),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, size_t argCount, ...), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreFunction *method, uint64_t imm), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreFunction *method, size_t argCount, ...),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreModule *m), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreClass *klass), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreClass *klass1, AbckitCoreClass *klass2),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreClass *klass, AbckitInst *inst), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitLiteralArray *litarr), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreFunction *m, AbckitLiteralArray *litarr,
                                        uint64_t val, AbckitInst *inst),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1,
                                        AbckitIsaApiStaticConditionCode),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitBasicBlock *catchBegin, size_t argCount, ...),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitString *input1), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitTypeId targetType),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0), bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, AbckitInst *input2),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, AbckitInst *input2,
                                        AbckitInst *input3),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, AbckitInst *input2,
                                        AbckitInst *input3, AbckitInst *input4),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitString *input1,
                                        AbckitInst *input2),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *acc, AbckitCoreFunction *method, uint64_t imm0),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *arrayRef, AbckitInst *idx, AbckitInst *value,
                                        AbckitTypeId valueTypeId),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inputObj, AbckitType *targetType),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inputObj, AbckitString *field,
                                        AbckitTypeId returnTypeId),
              bool isDynamic);
void TestMode(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inputObj, AbckitString *fieldId,
                                        AbckitInst *value, AbckitTypeId typeId),
              bool isDynamic);
}  // namespace libabckit::test::helpers_mode

#endif /*LIBABCKIT_TESTS_INVALID_HELPERS */
