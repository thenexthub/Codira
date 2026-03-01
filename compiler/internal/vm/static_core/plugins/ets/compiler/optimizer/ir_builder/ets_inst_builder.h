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

#ifndef PLUGINS_ETS_COMPILER_OPTIMIZER_IR_BUILDER_ETS_INST_BUILDER_H
#define PLUGINS_ETS_COMPILER_OPTIMIZER_IR_BUILDER_ETS_INST_BUILDER_H

#ifdef PANDA_ETS_INTEROP_JS
#include "plugins/ets/compiler/optimizer/ir_builder/js_interop/js_interop_inst_builder.h"
#endif

template <bool IS_ABC_KIT = false>
void BuildLdObjByName(const BytecodeInstruction *bcInst, compiler::DataType::Type type);
template <bool IS_ABC_KIT = false>
IntrinsicInst *CreateStObjByNameIntrinsic(size_t pc, compiler::DataType::Type type);
template <bool IS_ABC_KIT = false>
void BuildStObjByName(const BytecodeInstruction *bcInst, compiler::DataType::Type type);
template <bool IS_RANGE>
void BuildCallByName(const BytecodeInstruction *bcInst);
virtual void BuildIsNullValue(const BytecodeInstruction *bcInst);
virtual void BuildNullcheck(const BytecodeInstruction *bcInst);
template <bool IS_STRICT = false>
void BuildEquals(const BytecodeInstruction *bcInst);
virtual void BuildTypeof(const BytecodeInstruction *bcInst);
virtual void BuildIstrue(const BytecodeInstruction *bcInst);

#endif  // PLUGINS_ETS_COMPILER_OPTIMIZER_IR_BUILDER_ETS_INST_BUILDER_H