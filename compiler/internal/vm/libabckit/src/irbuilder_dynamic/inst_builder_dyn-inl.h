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

#ifndef LIBABCKIT_SRC_IR_BUILDER_DYNAMIC_INST_BUILDER_INL_H
#define LIBABCKIT_SRC_IR_BUILDER_DYNAMIC_INST_BUILDER_INL_H

#include "libabckit/src/irbuilder_dynamic/inst_builder_dyn.h"

namespace libabckit {

template <compiler::Opcode OPCODE>
void InstBuilder::BuildLoadFromPool(const BytecodeInst *bcInst)
{
    static_assert(OPCODE == compiler::Opcode::LoadString);
    BuildAbcKitLoadStringIntrinsic(bcInst);
}

inline void InstBuilder::BuildAbcKitLoadStringIntrinsic(const BytecodeInst *bcInst)
{
    ark::compiler::IntrinsicInst *inst =
        GetGraph()->CreateInstIntrinsic(ark::compiler::DataType::ANY, GetPc(bcInst->GetAddress()),
                                        ark::compiler::RuntimeInterface::IntrinsicId::INTRINSIC_ABCKIT_LOAD_STRING);
    uint32_t stringId0 = bcInst->GetId(0).AsIndex();
    stringId0 = GetRuntime()->ResolveMethodIndex(GetGraph()->GetMethod(), stringId0);
    inst->AddImm(GetGraph()->GetAllocator(), stringId0);
    auto method = GetGraph()->GetMethod();
    inst->SetMethod(method);
    AddInstruction(inst);
    UpdateDefinitionAcc(inst);
}

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_IR_BUILDER_DYNAMIC_INST_BUILDER_INL_H
