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

.extern InvokeInterpreter

.global DeoptimizeAfterCFrame
.type DeoptimizeAfterCFrame, %function
DeoptimizeAfterCFrame:
    CFI_STARTPROC

    // Parameters:
    // r0 - thread
    // r1 - pc of the entry
    // r2 - pointer to interpreter Frame
    // r3 - pointer to cframe origin
    // r4(slot0) - last restored interpreter Frame

    // Morph CFrame into C2I bridge frame
    // FROM         TO
    //  lr          lr
    //  fp <----    COMPILED_CODE_TO_INTERPRETER_BRIDGE
    //  method      fp <----

    CFI_DEF_CFA(r3, (2 * 4))
    CFI_REL_OFFSET(lr, 4)
    CFI_REL_OFFSET(fp, 0)
    CFI_REL_OFFSET(THREAD_REG, -((CFRAME_CALLEE_REGS_START_SLOT + 0) * 4))
    CFI_REL_OFFSET(r9,  -((CFRAME_CALLEE_REGS_START_SLOT + 1) * 4))
    CFI_REL_OFFSET(r8,  -((CFRAME_CALLEE_REGS_START_SLOT + 2) * 4))
    CFI_REL_OFFSET(r7,  -((CFRAME_CALLEE_REGS_START_SLOT + 3) * 4))
    CFI_REL_OFFSET(r6,  -((CFRAME_CALLEE_REGS_START_SLOT + 4) * 4))
    CFI_REL_OFFSET(r5,  -((CFRAME_CALLEE_REGS_START_SLOT + 5) * 4))
    CFI_REL_OFFSET(r4,  -((CFRAME_CALLEE_REGS_START_SLOT + 6) * 4))
    ldr r4, [sp]

    sub sp, r3, #((CFRAME_CALLEE_REGS_START_SLOT * 4) + CALLEE_SAVED_SIZE)

    ldr r5, [r3], #-4
    CFI_ADJUST_CFA_OFFSET(4)
    mov r6, #COMPILED_CODE_TO_INTERPRETER_BRIDGE
    stm r3, {r5, r6}
    CFI_REL_OFFSET(fp, 0)

    mov fp, r3
    CFI_DEF_CFA_REGISTER(fp)

    // Set IFrame's prev_frame to this C2I bridge frame
    str r3, [r4, #FRAME_PREV_FRAME_OFFSET]

    BOUNDARY_FRAME_SLOT = ((CFRAME_HEADER_SIZE - 3) + 1)

    // Copy callee saved registers from cframe into boundary frame.
    sub r12, r3, #((CFRAME_CALLEE_REGS_START_SLOT - 1) * 4 - 4)
    sub r11, r3, #((BOUNDARY_FRAME_SLOT - 1) * 4)

    // Scalar
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(r10, -((BOUNDARY_FRAME_SLOT + 0) * 4)) 
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(r9, -((BOUNDARY_FRAME_SLOT + 1) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(r8, -((BOUNDARY_FRAME_SLOT + 2) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(r7, -((BOUNDARY_FRAME_SLOT + 3) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(r6, -((BOUNDARY_FRAME_SLOT + 4) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(r5, -((BOUNDARY_FRAME_SLOT + 5) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(r4, -((BOUNDARY_FRAME_SLOT + 6) * 4))
    // Vector (use 32-bit)
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(s31, -((BOUNDARY_FRAME_SLOT + 7) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(s30, -((BOUNDARY_FRAME_SLOT + 8) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(s29, -((BOUNDARY_FRAME_SLOT + 9) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(s28, -((BOUNDARY_FRAME_SLOT + 10) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(s27, -((BOUNDARY_FRAME_SLOT + 11) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(s26, -((BOUNDARY_FRAME_SLOT + 12) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(s25, -((BOUNDARY_FRAME_SLOT + 13) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(s24, -((BOUNDARY_FRAME_SLOT + 14) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(s23, -((BOUNDARY_FRAME_SLOT + 15) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(s22, -((BOUNDARY_FRAME_SLOT + 16) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(s21, -((BOUNDARY_FRAME_SLOT + 17) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(s20, -((BOUNDARY_FRAME_SLOT + 18) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(s19, -((BOUNDARY_FRAME_SLOT + 19) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(s18, -((BOUNDARY_FRAME_SLOT + 20) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(s17, -((BOUNDARY_FRAME_SLOT + 21) * 4))
    ldr r10, [r12, #-4]!
    str r10, [r11, #-4]!
    CFI_REL_OFFSET(s16, -((BOUNDARY_FRAME_SLOT + 22) * 4))
    // Save used registers
    str r3, [sp, #-4]!
    str r2, [sp, #-4]!
    str r1, [sp, #-4]!
    str r0, [sp, #-4]!
    mov r3, r4

    // Arguments are already lay in registers, because signature of DeoptimizeAfterCFrame is similar to InvokeInterpreter
    blx InvokeInterpreter
    // r0, r1 - return values

    // Restore used registers, but not r0, r1 which holds result of the invoked method and should be survive to the end
    // So we use r4 instead of r0 for restoring THREAD_REG
    ldr r4, [sp], #4
    add sp, sp, #4
    ldrd r2, r3, [sp], #8

    // Restore stack pointer so that it points to COMPILED_CODE_TO_INTERPRETER_BRIDGE
    sub sp, r3, #4

    // Restore callee saved registers
    sub r3, r3, #((BOUNDARY_FRAME_SLOT - 1) * 4)

    ldr r10, [r3, #-4]!
    ldrd r8, r9, [r3, #-8]!
    CFI_RESTORE(r9)
    CFI_RESTORE(r8)
    ldrd r6, r7, [r3, #-8]!
    CFI_RESTORE(r7)
    CFI_RESTORE(r6)
    // THREAD_REG is r10, but we can't remove `ldr r10, [r3, #-4]!` because we can change number of the THREAD_REG
    // !NOTE fix this code if THREAD_REG will be change to r4 or r5
    mov THREAD_REG, r4
    CFI_RESTORE(THREAD_REG)

    ldrd r4, r5, [r3, #-8]!
    CFI_RESTORE(r5)
    CFI_RESTORE(r4)

#ifndef PANDA_TARGET_ARM32_ABI_SOFT
    vldr d15, [r3, #-8]
    CFI_RESTORE(d15)
    vldr d14, [r3, #-16]
    CFI_RESTORE(d14)
    vldr d13, [r3, #-24]
    CFI_RESTORE(d13)
    vldr d12, [r3, #-32]
    CFI_RESTORE(d12)
    vldr d11, [r3, #-40]
    CFI_RESTORE(d11)
    vldr d10, [r3, #-48]
    CFI_RESTORE(d10)
    vldr d9, [r3, #-56]
    CFI_RESTORE(d9)
    vldr d8, [r3, #-64]
    CFI_RESTORE(d8)

    vmov.f64 d0, r0, r1
#endif

    ldr fp, [sp, #4]
    CFI_RESTORE(fp)
    CFI_DEF_CFA(sp, (4 * 4))
    ldr lr, [sp, #12]
    CFI_RESTORE(lr)
    add sp, sp, #16
    CFI_ADJUST_CFA_OFFSET(-(4 * 4))
    bx lr
    CFI_ENDPROC

.global DeoptimizeAfterIFrame
.type DeoptimizeAfterIFrame, %function
DeoptimizeAfterIFrame:
    CFI_STARTPROC

    // Parameters:
    // r0 - thread
    // r1 - pc of the entry
    // r2 - pointer to interpreter Frame
    // r3 - pointer to cframe origin
    // r4(slot0) - last restored interpreter Frame
    // load num_inlined_methods from stack

    CFI_DEF_CFA(r3, (2 * 4))
    CFI_REL_OFFSET(lr, 4)
    CFI_REL_OFFSET(fp, 0)
    CFI_REL_OFFSET(r10, -((CFRAME_CALLEE_REGS_START_SLOT + 0) * 4))
    CFI_REL_OFFSET(r9,  -((CFRAME_CALLEE_REGS_START_SLOT + 1) * 4))
    CFI_REL_OFFSET(r8,  -((CFRAME_CALLEE_REGS_START_SLOT + 2) * 4))
    CFI_REL_OFFSET(r7,  -((CFRAME_CALLEE_REGS_START_SLOT + 3) * 4))
    CFI_REL_OFFSET(r6,  -((CFRAME_CALLEE_REGS_START_SLOT + 4) * 4))
    CFI_REL_OFFSET(r5,  -((CFRAME_CALLEE_REGS_START_SLOT + 5) * 4))
    CFI_REL_OFFSET(r4,  -((CFRAME_CALLEE_REGS_START_SLOT + 6) * 4))

    ldr r4, [sp]

    // Restore fp by pointer to the I2C boundary frame
    // Restore return address, so after interperter finish its work it returns to the I2C bridge
    mov fp, r3
    CFI_DEF_CFA_REGISTER(fp)

    // Restore stack pointer to the beginning of the cframe
    mov sp, fp

    // save num_inlined_methods to new stack
    str r4, [sp, #-4]!

    // Restore callee saved registers
    sub r3, r3, #((CFRAME_CALLEE_REGS_START_SLOT * 4) - 4)
    ldr r10, [r3, #-4]!
    CFI_RESTORE(r10)
    ldrd r8, r9, [r3, #-8]!
    CFI_RESTORE(r9)
    CFI_RESTORE(r8)
    ldrd r6, r7, [r3, #-8]!
    CFI_RESTORE(r7)
    CFI_RESTORE(r6)
    ldrd r4, r5, [r3, #-8]!
    CFI_RESTORE(r5)
    CFI_RESTORE(r4)

#ifndef PANDA_TARGET_ARM32_ABI_SOFT
    vldr d15, [r3, #-8]
    CFI_RESTORE(d15)
    vldr d14, [r3, #-16]
    CFI_RESTORE(d14)
    vldr d13, [r3, #-24]
    CFI_RESTORE(d13)
    vldr d12, [r3, #-32]
    CFI_RESTORE(d12)
    vldr d11, [r3, #-40]
    CFI_RESTORE(d11)
    vldr d10, [r3, #-48]
    CFI_RESTORE(d10)
    vldr d9, [r3, #-56]
    CFI_RESTORE(d9)
    vldr d8, [r3, #-64]
    CFI_RESTORE(d8)
#endif

    // load num_inlined_methods from new stack
    ldr r3, [sp], #4

    bl InvokeInterpreter

#ifndef PANDA_TARGET_ARM32_ABI_SOFT
    // InvokeInterpreter returns int64 value, but result can be double, so we copy value to vector register
    vmov.f64 d0, r0, r1
#endif

    pop {fp, lr}
    CFI_RESTORE(lr)
    CFI_RESTORE(fp)
    CFI_DEF_CFA(sp, 0)
    bx lr
    CFI_ENDPROC

.global DropCompiledFrameAndReturn
.type DropCompiledFrameAndReturn, %function
DropCompiledFrameAndReturn:
    CFI_STARTPROC

    // r0 - pointer to cframe origin
    CFI_DEF_CFA(r0, (2 * 4))
    CFI_REL_OFFSET(lr, 4)
    CFI_REL_OFFSET(fp, 0)
    CFI_REL_OFFSET(r10, -((CFRAME_CALLEE_REGS_START_SLOT + 0) * 4))
    CFI_REL_OFFSET(r9,  -((CFRAME_CALLEE_REGS_START_SLOT + 1) * 4))
    CFI_REL_OFFSET(r8,  -((CFRAME_CALLEE_REGS_START_SLOT + 2) * 4))
    CFI_REL_OFFSET(r7,  -((CFRAME_CALLEE_REGS_START_SLOT + 3) * 4))
    CFI_REL_OFFSET(r6,  -((CFRAME_CALLEE_REGS_START_SLOT + 4) * 4))
    CFI_REL_OFFSET(r5,  -((CFRAME_CALLEE_REGS_START_SLOT + 5) * 4))
    CFI_REL_OFFSET(r4,  -((CFRAME_CALLEE_REGS_START_SLOT + 6) * 4))

    mov fp, r0
    CFI_DEF_CFA_REGISTER(fp)

    // Restore callee saved registers from dropped CFrame
    sub r0, r0, #((CFRAME_CALLEE_REGS_START_SLOT * 4) - 4)
    ldr r10, [r0, #-4]!
    CFI_RESTORE(r10)
    ldrd r8, r9, [r0, #-8]!
    CFI_RESTORE(r9)
    CFI_RESTORE(r8)
    ldrd r6, r7, [r0, #-8]!
    CFI_RESTORE(r7)
    CFI_RESTORE(r6)
    ldrd r4, r5, [r0, #-8]!
    CFI_RESTORE(r5)
    CFI_RESTORE(r4)

#ifndef PANDA_TARGET_ARM32_ABI_SOFT
    vldr d15, [r0, #-8]
    CFI_RESTORE(d15)
    vldr d14, [r0, #-16]
    CFI_RESTORE(d14)
    vldr d13, [r0, #-24]
    CFI_RESTORE(d13)
    vldr d12, [r0, #-32]
    CFI_RESTORE(d12)
    vldr d11, [r0, #-40]
    CFI_RESTORE(d11)
    vldr d10, [r0, #-48]
    CFI_RESTORE(d10)
    vldr d9, [r0, #-56]
    CFI_RESTORE(d9)
    vldr d8, [r0, #-64]
    CFI_RESTORE(d8)
#endif

    // We need to clear return value, since it will be written to the IFrame's accumulator. Without this, it holds
    // garbage and StackWalker verification might fail.
    mov r0, #0
    mov r1, #0

    mov sp, fp
    pop {fp, lr}
    CFI_RESTORE(lr)
    CFI_RESTORE(fp)
    CFI_DEF_CFA(sp, 0)
    bx lr
    CFI_ENDPROC

