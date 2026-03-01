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
#include "ir_impl.h"

namespace libabckit::test::helpers_wrong_imm {

void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm0), AbckitBitImmSize bitsize);
void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm0, uint64_t imm1),
                  AbckitBitImmSize bitsize);
void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, uint64_t imm0),
                  AbckitBitImmSize bitsize);
void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0,
                                            ...),
                  AbckitBitImmSize bitsize);
void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, uint64_t imm0, AbckitInst *inst1),
                  AbckitBitImmSize bitsize);
void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, uint64_t imm0, uint64_t imm1,
                                            AbckitInst *inst1),
                  AbckitBitImmSize bitsize);
void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, uint64_t imm0, uint64_t imm1),
                  AbckitBitImmSize bitsize);
void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm0, AbckitInst *inst0, AbckitInst *inst1),
                  AbckitBitImmSize bitsize);
void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreFunction *method, uint64_t imm),
                  AbckitBitImmSize bitsize);
void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *acc, AbckitCoreFunction *method,
                                            uint64_t imm0),
                  AbckitBitImmSize bitsize);
void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *input0, AbckitInst *input1, uint64_t imm0),
                  AbckitBitImmSize bitsize);
void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, uint64_t imm0, AbckitLiteralArray *litarr),
                  AbckitBitImmSize bitsize);
void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitCoreFunction *m, AbckitLiteralArray *litarr,
                                            uint64_t imm0, AbckitInst *inst),
                  AbckitBitImmSize bitsize);
void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, size_t argCount, ...), AbckitBitImmSize bitsize);
void TestWrongImm(AbckitInst *(*apiToCheck)(AbckitGraph *graph, AbckitInst *inst, size_t argCount, ...),
                  AbckitBitImmSize bitsize);

}  // namespace libabckit::test::helpers_wrong_imm

#endif /*LIBABCKIT_TESTS_INVALID_HELPERS */
