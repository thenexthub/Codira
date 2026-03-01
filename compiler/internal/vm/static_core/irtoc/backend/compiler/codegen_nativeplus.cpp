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

#include "codegen_nativeplus.h"
#include "optimizer/ir/inst.h"
#include "relocations.h"

namespace ark::compiler {
void CodegenNativePlus::EmitTailCallIntrinsic([[maybe_unused]] IntrinsicInst *inst, [[maybe_unused]] Reg dst,
                                              [[maybe_unused]] SRCREGS src)
{
    CHECK_LE(inst->GetImms().size(), 2U);
    /* Once we reach the slow path, we can release all temp registers, since slow path terminates execution */
    auto tempsMask = GetTarget().GetTempRegsMask();
    for (size_t reg = tempsMask.GetMinRegister(); reg <= tempsMask.GetMaxRegister(); reg++) {
        if (tempsMask.Test(reg)) {
            GetEncoder()->ReleaseScratchRegister(Reg(reg, INT32_TYPE));
        }
    }
    ScopedTmpReg tmp(GetEncoder());
    auto offset = inst->GetImms()[0];
    GetCallingConvention()->GenerateEpilogueHead(*GetFrameInfo(), []() {});
    GetEntrypoint(tmp, offset);
    GetEncoder()->EncodeJump(tmp);
}
}  // namespace ark::compiler
