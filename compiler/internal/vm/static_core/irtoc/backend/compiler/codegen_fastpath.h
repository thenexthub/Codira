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

#ifndef PANDA_CODEGEN_ENTRYPOINT_H
#define PANDA_CODEGEN_ENTRYPOINT_H

#include "compiler/optimizer/code_generator/codegen.h"

namespace ark::compiler {

/**
 * Code generation for fast paths
 *
 * Fast path code supports only native calls, that hasn't safepoints inside. Compiler has specific intrinsics to
 * save/restore registers before calling native functions. These intrinsics pushes caller saved registers on the stack
 * below the frame, hereby it change SP register. Due to this fact spill/fills aren't supported in the entrypoints mode.
 *
 * Compiled code(caller) saves only parameter registers.
 * Fast path entrypoint saves only used registers, and saves all caller registers if it goes to the slow path.
 * Slow path is invoked via code-to-runtime bridge, which forms boundary frame and saves callee registers.
 * So, before calling the slow path we restore all modified callee registers.
 *
 * To call C-implemented entrypoint (slow path) we need a separate bridge, because it shouldn't save caller registers
 * before calling slow path, since they are saved within irtoced entrypoint.
 */
class CodegenFastPath : public Codegen {
public:
    using Codegen::Codegen;

    explicit CodegenFastPath(Graph *graph) : Codegen(graph)
    {
        // We don't save LR register in a prologue, so we can't use LR as a temp register in the whole Fast Path code.
        GetEncoder()->EnableLrAsTempReg(false);
    }

    void GeneratePrologue() override;
    void GenerateEpilogue() override;

    void CreateFrameInfo() override;

    void EmitSimdIntrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src) override;
    void EmitReverseIntrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src) override;
    void EmitMarkWordIntrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src) override;
    void EmitDataMemoryBarrierFullIntrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src) override;
    void EmitWriteTlabStatsSafeIntrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src) override;
    void EmitExpandU8ToU16Intrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src) override;
    void EmitAtomicByteOrIntrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src) override;
    void EmitSaveOrRestoreRegsEpIntrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src) override;
    void EmitTailCallIntrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src) override;
    void EmitSlowPathEntryIntrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src) override;
    void EmitJsCastDoubleToCharIntrinsic(IntrinsicInst *inst, Reg dst, SRCREGS src) override;

private:
    RegMask GetCallerRegistersToRestore() const;
    void CreateTailCall(IntrinsicInst *inst, bool isFastpath);

private:
    RegMask savedRegisters_ {};
    VRegMask savedFpRegisters_ {};
};

}  // namespace ark::compiler

#endif  // PANDA_CODEGEN_ENTRYPOINT_H
