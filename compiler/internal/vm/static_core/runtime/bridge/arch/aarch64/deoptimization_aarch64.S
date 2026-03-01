/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "arch/asm_support.h"
#include "arch/aarch64/helpers_aarch64.S"
#include "shorty_values.h"

.extern MAKE_ASM_NAME(InvokeInterpreter)

.macro CFRAME_COPY_LR fpreg
    ldr lr, [\fpreg, #(-CFRAME_FLAGS_SLOT * 8)]
    tst lr, #CFRAME_KIND_OSR
    beq 1f
    ldr lr, [\fpreg, #(-(CFRAME_LOCALS_START_SLOT + 2) * 8)]
    str lr, [\fpreg, #(-CFRAME_LR_SLOT * 8)]
1:
.endm

.macro CFRAME_GET_LR fpreg
    ldr lr, [\fpreg, #(-CFRAME_FLAGS_SLOT * 8)]
    tst lr, #CFRAME_KIND_OSR
    bne 1f
    ldr lr, [\fpreg, #(-CFRAME_LR_SLOT * 8)]
    b 2f
1:
    ldr lr, [\fpreg, #(-(CFRAME_LOCALS_START_SLOT + 2) * 8)]
2:
.endm

.macro RESTORE_CALLEE_REGISTERS basereg
    ldp x27, x28, [\basereg, #-16]!
    ldp x25, x26, [\basereg, #-16]!
    ldp x23, x24, [\basereg, #-16]!
    ldp x21, x22, [\basereg, #-16]!
    ldp x19, x20, [\basereg, #-16]!
    ldp d14, d15, [\basereg, #-16]!
    ldp d12, d13, [\basereg, #-16]!
    ldp d10, d11, [\basereg, #-16]!
    ldp d8,  d9,  [\basereg, #-16]!
.endm

// ============================================================================

.global MAKE_ASM_NAME(DeoptimizeAfterCFrame)
TYPE_FUNCTION(DeoptimizeAfterCFrame)
MAKE_ASM_NAME(DeoptimizeAfterCFrame):
    CFI_STARTPROC

    // Parameters:
    // x0 - thread
    // x1 - pc of the entry
    // x2 - pointer to interpreter Frame
    // x3 - pointer to cframe origin
    // x4 - last restored interpreter Frame
    // x5 - pointer to a callee-saved registers buffer from StackWalker

    CFI_DEF_CFA(x3, (2 * 8))
    CFI_REL_OFFSET(lr, 8)
    CFI_REL_OFFSET(fp, 0)
    CFI_REL_OFFSET(x28, -((CFRAME_CALLEE_REGS_START_SLOT + 0) * 8))
    CFI_REL_OFFSET(x27, -((CFRAME_CALLEE_REGS_START_SLOT + 1) * 8))
    CFI_REL_OFFSET(x26, -((CFRAME_CALLEE_REGS_START_SLOT + 2) * 8))
    CFI_REL_OFFSET(x25, -((CFRAME_CALLEE_REGS_START_SLOT + 3) * 8))
    CFI_REL_OFFSET(x24, -((CFRAME_CALLEE_REGS_START_SLOT + 4) * 8))
    CFI_REL_OFFSET(x23, -((CFRAME_CALLEE_REGS_START_SLOT + 5) * 8))
    CFI_REL_OFFSET(x22, -((CFRAME_CALLEE_REGS_START_SLOT + 6) * 8))
    CFI_REL_OFFSET(x21, -((CFRAME_CALLEE_REGS_START_SLOT + 7) * 8))
    CFI_REL_OFFSET(x20, -((CFRAME_CALLEE_REGS_START_SLOT + 8) * 8))
    CFI_REL_OFFSET(x19, -((CFRAME_CALLEE_REGS_START_SLOT + 9) * 8))
    CFI_REL_OFFSET(d15, -((CFRAME_CALLEE_REGS_START_SLOT + 10) * 8))
    CFI_REL_OFFSET(d14, -((CFRAME_CALLEE_REGS_START_SLOT + 11) * 8))
    CFI_REL_OFFSET(d13, -((CFRAME_CALLEE_REGS_START_SLOT + 12) * 8))
    CFI_REL_OFFSET(d12, -((CFRAME_CALLEE_REGS_START_SLOT + 13) * 8))
    CFI_REL_OFFSET(d11, -((CFRAME_CALLEE_REGS_START_SLOT + 14) * 8))
    CFI_REL_OFFSET(d10, -((CFRAME_CALLEE_REGS_START_SLOT + 15) * 8))
    CFI_REL_OFFSET(d9, -((CFRAME_CALLEE_REGS_START_SLOT + 16) * 8))
    CFI_REL_OFFSET(d8, -((CFRAME_CALLEE_REGS_START_SLOT + 17) * 8))

    // Morph CFrame into C2I bridge frame
    // FROM         TO
    //  lr          lr
    //  fp <----    COMPILED_CODE_TO_INTERPRETER_BRIDGE
    //  method      fp <----
    sub sp, x3, #(((CFRAME_HEADER_SIZE - 2) * 8) + CALLEE_SAVED_SIZE)
    ldr x7, [x3]
    mov x8, #COMPILED_CODE_TO_INTERPRETER_BRIDGE
    stp x7, x8, [x3, #-8]!
    CFI_ADJUST_CFA_OFFSET(8)
    CFI_REL_OFFSET(fp, 0)

    mov fp, x3
    CFI_DEF_CFA_REGISTER(fp)

    // Set last IFrame's prev_frame to this C2I bridge frame
    str x3, [x4, #FRAME_PREV_FRAME_OFFSET]

    BOUNDARY_FRAME_SLOT = ((CFRAME_HEADER_SIZE - 3) + 1)

    // Copy callee saved registers from StackWalker into boundary frame.
    // Note, we do not preserve x5 (holding a pointer to a callee-saved
    // registers buffer from StackWalker) as it is not needed later on.
    sub x17, x3, #((BOUNDARY_FRAME_SLOT - 1) * 8)
    ldp x14, x15, [x5, #-16]!
    stp x14, x15, [x17, #-16]!
    CFI_REL_OFFSET(x28, -((BOUNDARY_FRAME_SLOT + 0) * 8))
    CFI_REL_OFFSET(x27, -((BOUNDARY_FRAME_SLOT + 1) * 8))
    ldp x14, x15, [x5, #-16]!
    stp x14, x15, [x17, #-16]!
    CFI_REL_OFFSET(x26, -((BOUNDARY_FRAME_SLOT + 2) * 8))
    CFI_REL_OFFSET(x25, -((BOUNDARY_FRAME_SLOT + 3) * 8))
    ldp x14, x15, [x5, #-16]!
    stp x14, x15, [x17, #-16]!
    CFI_REL_OFFSET(x24, -((BOUNDARY_FRAME_SLOT + 4) * 8))
    CFI_REL_OFFSET(x23, -((BOUNDARY_FRAME_SLOT + 5) * 8))
    ldp x14, x15, [x5, #-16]!
    stp x14, x15, [x17, #-16]!
    CFI_REL_OFFSET(x22, -((BOUNDARY_FRAME_SLOT + 6) * 8))
    CFI_REL_OFFSET(x21, -((BOUNDARY_FRAME_SLOT + 7) * 8))
    ldp x14, x15, [x5, #-16]!
    stp x14, x15, [x17, #-16]!
    CFI_REL_OFFSET(x20, -((BOUNDARY_FRAME_SLOT + 8) * 8))
    CFI_REL_OFFSET(x19, -((BOUNDARY_FRAME_SLOT + 9) * 8))
    ldp x14, x15, [x5, #-16]!
    stp x14, x15, [x17, #-16]!
    CFI_REL_OFFSET(d15, -((BOUNDARY_FRAME_SLOT + 10) * 8))
    CFI_REL_OFFSET(d14, -((BOUNDARY_FRAME_SLOT + 11) * 8))
    ldp x14, x15, [x5, #-16]!
    stp x14, x15, [x17, #-16]!
    CFI_REL_OFFSET(d13, -((BOUNDARY_FRAME_SLOT + 12) * 8))
    CFI_REL_OFFSET(d12, -((BOUNDARY_FRAME_SLOT + 13) * 8))
    ldp x14, x15, [x5, #-16]!
    stp x14, x15, [x17, #-16]!
    CFI_REL_OFFSET(d11, -((BOUNDARY_FRAME_SLOT + 14) * 8))
    CFI_REL_OFFSET(d10, -((BOUNDARY_FRAME_SLOT + 15) * 8))
    ldp x14, x15, [x5, #-16]!
    stp x14, x15, [x17, #-16]!
    CFI_REL_OFFSET(d9, -((BOUNDARY_FRAME_SLOT + 16) * 8))
    CFI_REL_OFFSET(d8, -((BOUNDARY_FRAME_SLOT + 17) * 8))

    ASSERT_REGS_CMP sp, x17, eq

    // Save used registers
    stp x0, x1, [sp, #-16]!
    stp x2, x3, [sp, #-16]!

    mov x3, x4
    // Arguments are already lie in the registers, because signature of DeoptimizeAfterCFrame is similar to InvokeInterpreter
    bl MAKE_ASM_NAME(InvokeInterpreter)

    // Restore used registers, but not x0 which holds result of the invoked method and should survive to the end
    // So we use r7 instead of r0 for restoring THRED_REG
    ldp x2, x3, [sp], #16
    ldp x7, x1, [sp], #16

    // Restore callee saved registers
    sub x3, x3, #((BOUNDARY_FRAME_SLOT - 1) * 8)
    RESTORE_CALLEE_REGISTERS x3
    CFI_RESTORE(x28)
    CFI_RESTORE(x27)
    CFI_RESTORE(x26)
    CFI_RESTORE(x25)
    CFI_RESTORE(x24)
    CFI_RESTORE(x23)
    CFI_RESTORE(x22)
    CFI_RESTORE(x21)
    CFI_RESTORE(x20)
    CFI_RESTORE(x19)
    CFI_RESTORE(d15)
    CFI_RESTORE(d14)
    CFI_RESTORE(d13)
    CFI_RESTORE(d12)
    CFI_RESTORE(d11)
    CFI_RESTORE(d10)
    CFI_RESTORE(d9)
    CFI_RESTORE(d8)

    # Restore stack pointer
    add sp, sp, #(((CFRAME_HEADER_SIZE - 2) * 8) + CALLEE_SAVED_SIZE)

    ldr fp, [sp, #-8]
    CFI_RESTORE(fp)
    CFI_DEF_CFA(sp, (2 * 8))
    ldr lr, [sp, #8]
    CFI_RESTORE(lr)
    add sp, sp, #16
    CFI_ADJUST_CFA_OFFSET(-(2 * 8))
    mov THREAD_REG, x7
    CFI_RESTORE(THREAD_REG)
    fmov d0, x0
    ret
    CFI_ENDPROC


.global MAKE_ASM_NAME(DeoptimizeAfterIFrame)
TYPE_FUNCTION(DeoptimizeAfterIFrame)
MAKE_ASM_NAME(DeoptimizeAfterIFrame):
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)
#ifdef PANDA_COMPILER_DEBUG_INFO
    stp fp, lr, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(2 * 8)
    CFI_REL_OFFSET(lr, 8)
#endif

    // Parameters:
    // x0 - thread
    // x1 - pc of the entry
    // x2 - pointer to interpreter Frame
    // x3 - pointer to cframe origin
    // x4 - last restored interpreter Frame
    // x5 - pointer to a callee-saved registers buffer from StackWalker

    CFRAME_COPY_LR x3

    CFI_DEF_CFA(x3, (2 * 8))
    CFI_REL_OFFSET(lr, 8)
    CFI_REL_OFFSET(fp, 0)
    CFI_REL_OFFSET(x28, -((CFRAME_CALLEE_REGS_START_SLOT + 0) * 8))
    CFI_REL_OFFSET(x27, -((CFRAME_CALLEE_REGS_START_SLOT + 1) * 8))
    CFI_REL_OFFSET(x26, -((CFRAME_CALLEE_REGS_START_SLOT + 2) * 8))
    CFI_REL_OFFSET(x25, -((CFRAME_CALLEE_REGS_START_SLOT + 3) * 8))
    CFI_REL_OFFSET(x24, -((CFRAME_CALLEE_REGS_START_SLOT + 4) * 8))
    CFI_REL_OFFSET(x23, -((CFRAME_CALLEE_REGS_START_SLOT + 5) * 8))
    CFI_REL_OFFSET(x22, -((CFRAME_CALLEE_REGS_START_SLOT + 6) * 8))
    CFI_REL_OFFSET(x21, -((CFRAME_CALLEE_REGS_START_SLOT + 7) * 8))
    CFI_REL_OFFSET(x20, -((CFRAME_CALLEE_REGS_START_SLOT + 8) * 8))
    CFI_REL_OFFSET(x19, -((CFRAME_CALLEE_REGS_START_SLOT + 9) * 8))
    CFI_REL_OFFSET(d15, -((CFRAME_CALLEE_REGS_START_SLOT + 10) * 8))
    CFI_REL_OFFSET(d14, -((CFRAME_CALLEE_REGS_START_SLOT + 11) * 8))
    CFI_REL_OFFSET(d13, -((CFRAME_CALLEE_REGS_START_SLOT + 12) * 8))
    CFI_REL_OFFSET(d12, -((CFRAME_CALLEE_REGS_START_SLOT + 13) * 8))
    CFI_REL_OFFSET(d11, -((CFRAME_CALLEE_REGS_START_SLOT + 14) * 8))
    CFI_REL_OFFSET(d10, -((CFRAME_CALLEE_REGS_START_SLOT + 15) * 8))
    CFI_REL_OFFSET(d9, -((CFRAME_CALLEE_REGS_START_SLOT + 16) * 8))
    CFI_REL_OFFSET(d8, -((CFRAME_CALLEE_REGS_START_SLOT + 17) * 8))

    // Restore fp by pointer to the I2C boundary frame
    mov fp, x3
    CFI_DEF_CFA_REGISTER(fp)

    // Restore return address, so after interperter finished, it returns to the I2C bridge

    // Restore callee saved registers
    RESTORE_CALLEE_REGISTERS x5
    CFI_RESTORE(x28)
    CFI_RESTORE(x27)
    CFI_RESTORE(x26)
    CFI_RESTORE(x25)
    CFI_RESTORE(x24)
    CFI_RESTORE(x23)
    CFI_RESTORE(x22)
    CFI_RESTORE(x21)
    CFI_RESTORE(x20)
    CFI_RESTORE(x19)
    CFI_RESTORE(d15)
    CFI_RESTORE(d14)
    CFI_RESTORE(d13)
    CFI_RESTORE(d12)
    CFI_RESTORE(d11)
    CFI_RESTORE(d10)
    CFI_RESTORE(d9)
    CFI_RESTORE(d8)

    // Restore stack pointer to the beginning of the cframe
    mov sp, fp

    mov x3, x4
    bl MAKE_ASM_NAME(InvokeInterpreter)
    // InvokeInterpreter returns int64 value, but result can be double, so we copy value to vector register
    fmov d0, x0

    ldp fp, lr, [sp], #16
    CFI_RESTORE(fp)
    CFI_RESTORE(lr)
    CFI_DEF_CFA(sp, 0)
    ret
    CFI_ENDPROC


.global MAKE_ASM_NAME(DropCompiledFrameAndReturn)
TYPE_FUNCTION(DropCompiledFrameAndReturn)
MAKE_ASM_NAME(DropCompiledFrameAndReturn):
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)
#ifdef PANDA_COMPILER_DEBUG_INFO
    stp fp, lr, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(2 * 8)
    CFI_REL_OFFSET(lr, 8)
#endif

    // x0 - pointer to cframe origin
    // x1 - pointer to a callee-saved registers buffer from StackWalker
    CFRAME_GET_LR x0

    CFI_DEF_CFA(x0, (2 * 8))
    CFI_RESTORE(lr)
    CFI_REL_OFFSET(fp, 0)
    CFI_REL_OFFSET(x28, -((CFRAME_CALLEE_REGS_START_SLOT + 0) * 8))
    CFI_REL_OFFSET(x27, -((CFRAME_CALLEE_REGS_START_SLOT + 1) * 8))
    CFI_REL_OFFSET(x26, -((CFRAME_CALLEE_REGS_START_SLOT + 2) * 8))
    CFI_REL_OFFSET(x25, -((CFRAME_CALLEE_REGS_START_SLOT + 3) * 8))
    CFI_REL_OFFSET(x24, -((CFRAME_CALLEE_REGS_START_SLOT + 4) * 8))
    CFI_REL_OFFSET(x23, -((CFRAME_CALLEE_REGS_START_SLOT + 5) * 8))
    CFI_REL_OFFSET(x22, -((CFRAME_CALLEE_REGS_START_SLOT + 6) * 8))
    CFI_REL_OFFSET(x21, -((CFRAME_CALLEE_REGS_START_SLOT + 7) * 8))
    CFI_REL_OFFSET(x20, -((CFRAME_CALLEE_REGS_START_SLOT + 8) * 8))
    CFI_REL_OFFSET(x19, -((CFRAME_CALLEE_REGS_START_SLOT + 9) * 8))
    CFI_REL_OFFSET(d15, -((CFRAME_CALLEE_REGS_START_SLOT + 10) * 8))
    CFI_REL_OFFSET(d14, -((CFRAME_CALLEE_REGS_START_SLOT + 11) * 8))
    CFI_REL_OFFSET(d13, -((CFRAME_CALLEE_REGS_START_SLOT + 12) * 8))
    CFI_REL_OFFSET(d12, -((CFRAME_CALLEE_REGS_START_SLOT + 13) * 8))
    CFI_REL_OFFSET(d11, -((CFRAME_CALLEE_REGS_START_SLOT + 14) * 8))
    CFI_REL_OFFSET(d10, -((CFRAME_CALLEE_REGS_START_SLOT + 15) * 8))
    CFI_REL_OFFSET(d9, -((CFRAME_CALLEE_REGS_START_SLOT + 16) * 8))
    CFI_REL_OFFSET(d8, -((CFRAME_CALLEE_REGS_START_SLOT + 17) * 8))

    mov fp, x0
    CFI_DEF_CFA_REGISTER(fp)

    // Restore callee saved registers
    RESTORE_CALLEE_REGISTERS x1
    CFI_RESTORE(x28)
    CFI_RESTORE(x27)
    CFI_RESTORE(x26)
    CFI_RESTORE(x25)
    CFI_RESTORE(x24)
    CFI_RESTORE(x23)
    CFI_RESTORE(x22)
    CFI_RESTORE(x21)
    CFI_RESTORE(x20)
    CFI_RESTORE(x19)
    CFI_RESTORE(d15)
    CFI_RESTORE(d14)
    CFI_RESTORE(d13)
    CFI_RESTORE(d12)
    CFI_RESTORE(d11)
    CFI_RESTORE(d10)
    CFI_RESTORE(d9)
    CFI_RESTORE(d8)

    // We need to clear return value, since it will be written to the IFrame's accumulator. Without this, it holds
    // garbage and StackWalker verification might fail.
    mov x0, xzr

    mov sp, fp
    ldr fp, [sp], #16
    CFI_RESTORE(fp)
    CFI_DEF_CFA(sp, 0)
    ret
    CFI_ENDPROC
