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

#ifndef LIBABCKIT_SRC_CODEGEN_COMMON_H
#define LIBABCKIT_SRC_CODEGEN_COMMON_H

#include "static_core/compiler/optimizer/ir/constants.h"
#include "static_core/compiler/optimizer/ir/inst.h"

namespace libabckit {
static constexpr ark::compiler::Register MIN_REGISTER_NUMBER = 0;
static constexpr ark::compiler::Register MAX_NUM_SHORT_CALL_ARGS = 2;
static constexpr ark::compiler::Register MAX_NUM_NON_RANGE_ARGS = 4;
static constexpr ark::compiler::Register MAX_NUM_INPUTS = MAX_NUM_NON_RANGE_ARGS;
static constexpr ark::compiler::Register NUM_COMPACTLY_ENCODED_REGS = 16;
static constexpr uint32_t MAX_BYTECODE_SIZE = 100000U;

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_CODEGEN_COMMON_H
