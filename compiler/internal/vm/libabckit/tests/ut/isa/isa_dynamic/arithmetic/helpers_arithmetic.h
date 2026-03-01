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

#ifndef LIBABCKIT_TESTS_IR_CREATE_INSTR_HELPER_ARITHMETIC_H
#define LIBABCKIT_TESTS_IR_CREATE_INSTR_HELPER_ARITHMETIC_H

#include "helpers/helpers.h"

namespace libabckit::test::helpers::arithmetic {

void TransformIrBinInstrValid(AbckitGraph *graph,
                              AbckitInst *(*binaryInstToCheck)(AbckitGraph *graph, AbckitInst *input0,
                                                               AbckitInst *input1),
                              uint32_t lhsId = 1, uint32_t rhsId = 2);
void TransformIrBinInstrWithImmValid(AbckitGraph *graph,
                                     AbckitInst *(*binaryInstToCheck)(AbckitGraph *graph, AbckitInst *input0,
                                                                      uint64_t imm));
void TransformIrUnaryInstValid(AbckitGraph *graph,
                               AbckitInst *(*unaryInstToCheck)(AbckitGraph *graph, AbckitInst *input0),
                               uint32_t operandId = 1);
std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> CreateBBSchemaForBinary(AbckitIsaApiStaticOpcode opcode);
std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> CreateBBSchemaForDynBinary(AbckitIsaApiDynamicOpcode opcode);
std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> CreateBBSchemaForBinaryWithImm(
    AbckitIsaApiStaticOpcode opcode);
std::vector<helpers::BBSchema<AbckitIsaApiStaticOpcode>> CreateBBSchemaForUnary(AbckitIsaApiStaticOpcode opcode);
std::vector<helpers::BBSchema<AbckitIsaApiDynamicOpcode>> CreateBBSchemaForDynUnary(AbckitIsaApiDynamicOpcode opcode);

}  // namespace libabckit::test::helpers::arithmetic
// NOLINTEND(readability-magic-numbers)::helpers::arithmetic

#endif  // LIBABCKIT_TESTS_IR_CREATE_INSTR_HELPER_ARITHMETIC_H
