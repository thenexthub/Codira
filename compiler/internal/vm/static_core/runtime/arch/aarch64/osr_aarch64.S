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

#include "arch/aarch64/helpers_aarch64.S"
#include "arch/asm_support.h"

#define REGS_COUNT 30
#define FP_REGS_COUNT 32
#define REGS_BUFFER_SIZE (REGS_COUNT * 8)
#define FP_REGS_BUFFER_SIZE (FP_REGS_COUNT * 8)

.extern MAKE_ASM_NAME(ConvertIFrameToCFrame)
.extern MAKE_ASM_NAME(PrepareOsrEntry)
.extern MAKE_ASM_NAME(SetResult)
.extern MAKE_ASM_NAME(GetCurrentManagedThread)

.macro OSR_ENTRY
    // Prepare buffer for scalar registers
    sub sp, sp, REGS_BUFFER_SIZE
    mov x4, sp
    // Prepare buffer for vector registers
    sub sp, sp, FP_REGS_BUFFER_SIZE
    mov x5, sp

    // x0 - pointer to iframe
    // x1 - bytecode offset
    // x2 - osr code pointer
    // x3 - pointer to cframe
    // x4 - buffer for scalar registers
    // x5 - buffer for vector registers
    bl MAKE_ASM_NAME(PrepareOsrEntry)
    mov x17, x0

    mov x16, sp
    stp x16, x17, [sp, #-16]!

    bl MAKE_ASM_NAME(GetCurrentManagedThread)
    mov THREAD_REG, x0

    ldp x16, x17, [sp], #16

    // Restore registers from buffers prepared by PrepareOsrEntry
    ldp d0, d1, [x16], #16
    ldp d2, d3, [x16], #16
    ldp d4, d5, [x16], #16
    ldp d6, d7, [x16], #16
    ldp d8, d9, [x16], #16
    ldp d10, d11, [x16], #16
    ldp d12, d13, [x16], #16
    ldp d14, d15, [x16], #16
    ldp d16, d17, [x16], #16
    ldp d18, d19, [x16], #16
    ldp d20, d21, [x16], #16
    ldp d22, d23, [x16], #16
    ldp d24, d25, [x16], #16
    ldp d26, d27, [x16], #16
    ldp d28, d29, [x16], #16
    ldp d30, d31, [x16], #16

    ldp x0, x1, [x16], #16
    ldp x2, x3, [x16], #16
    ldp x4, x5, [x16], #16
    ldp x6, x7, [x16], #16
    ldp x8, x9, [x16], #16
    ldp x10, x11, [x16], #16
    ldp x12, x13, [x16], #16
    ldp x14, x15, [x16], #16
    add x16, x16 ,16
    ldp x18, x19, [x16], #16
    ldp x20, x21, [x16], #16
    ldp x22, x23, [x16], #16
    ldp x24, x25, [x16], #16
    ldp x26, x27, [x16], #16
    // X28 is a THREAD_REG, so skip it
    add sp, sp, (REGS_BUFFER_SIZE + FP_REGS_BUFFER_SIZE)

    blr x17
.endm

.global MAKE_ASM_NAME(OsrEntryAfterCFrame)
TYPE_FUNCTION(OsrEntryAfterCFrame)
MAKE_ASM_NAME(OsrEntryAfterCFrame):
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)
#ifdef PANDA_COMPILER_DEBUG_INFO
    stp fp, lr, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(2 * 8)
    CFI_REL_OFFSET(lr, 8)
    CFI_REL_OFFSET(fp, 0)
#endif

    // x0 - pointer to Frame
    // x1 - bytecode offset
    // x2 - pointer to compiled code
    // x3 - frame size

    // We're going to replace C2I frame by new cframe
    ldr x16, [x0, #FRAME_PREV_FRAME_OFFSET]
    add x18, x16, #24

    // We will return to the caller cframe skipping interpreter and C2I bridge
    ldr fp, [x16]
    ldr lr, [x16, #16]

    // Copy callee saved registers from boundary frame into cframe.
    // Destination - callee saved stack in the cframe
    sub x17, x18, #((CFRAME_HEADER_SIZE + CFRAME_LOCALS_COUNT) * 8 + CALLEE_SAVED_SIZE)
    // Source - callee saved stack in the bounadary frame
    sub x16, x18, #((CFRAME_HEADER_SIZE * 8) + CALLEE_SAVED_SIZE)

    ldp x14, x15, [x16], #16
    stp x14, x15, [x17], #16
    ldp x14, x15, [x16], #16
    stp x14, x15, [x17], #16
    ldp x14, x15, [x16], #16
    stp x14, x15, [x17], #16
    ldp x14, x15, [x16], #16
    stp x14, x15, [x17], #16
    ldp x14, x15, [x16], #16
    stp x14, x15, [x17], #16
    ldp x14, x15, [x16], #16
    stp x14, x15, [x17], #16
    ldp x14, x15, [x16], #16
    stp x14, x15, [x17], #16
    ldp x14, x15, [x16], #16
    stp x14, x15, [x17], #16
    ldp x14, x15, [x16], #16
    stp x14, x15, [x17], #16

    // Create CFrame
    stp fp, lr, [x18, #-16]
    ldr x16, [x0, #FRAME_METHOD_OFFSET]
    stp xzr, x16, [x18, #-32]

    mov x4, x3
    // Set x3 to created cframe
    sub x3, x18, #16
    mov fp, x3

    CFI_DEF_CFA(fp, (2 * 8))
    CFI_REL_OFFSET(lr, 8)
    CFI_REL_OFFSET(fp, 0)
    CFI_REL_OFFSET(THREAD_REG, -((CFRAME_CALLEE_REGS_START_SLOT + 0) * 8))
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

    // Adjust stack for the whole cframe
    sub sp, x18, x4

    OSR_ENTRY

    // We need to presrve lr/fp stored in the cframe, since GetCurrentManagedThread will modify stack memory.
    sub sp, sp, #16

    // Restore THREAD REG
    str x0, [sp, #-16]!
    bl MAKE_ASM_NAME(GetCurrentManagedThread)
    mov THREAD_REG, x0
    ldr x0, [sp], #16

    // Load lr/fp and restore sp
    ldp fp, lr, [sp], #16
    CFI_RESTORE(lr)
    CFI_RESTORE(fp)
    CFI_DEF_CFA(sp, 0)
    ret
    CFI_ENDPROC

.global MAKE_ASM_NAME(OsrEntryAfterIFrame)
TYPE_FUNCTION(OsrEntryAfterIFrame)
MAKE_ASM_NAME(OsrEntryAfterIFrame):
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    // x0 - pointer to Frame
    // x1 - bytecode offset
    // x2 - pointer to compiled code
    // x3 - frame size
    // x4 - stack params size

    // Save original lr and fp before cframe
    stp fp, lr, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)
    CFI_REL_OFFSET(lr, 8)
    CFI_REL_OFFSET(fp, 0)
    stp x0, x22, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)
    CFI_REL_OFFSET(x22, 8)

    // Create I2C bridge
    ldr x16, [x0, #FRAME_PREV_FRAME_OFFSET]
    stp xzr, lr, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)
    mov x17, #INTERPRETER_TO_COMPILED_CODE_BRIDGE
    stp x16, x17, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)

    // Save pointer to I2C bridge in temp register
    add x16, sp, #16

    // Allocate space for stack parameters
    sub sp, sp, x4
    // Save stack parameters size in callee register, so we'll can to deallocate space after call
    mov x22, x4

    // Create CFrame
    stp x16, lr, [sp, #-16]
    ldr x16, [x0, #FRAME_METHOD_OFFSET]
    // Save method and clear properties slot in CFrame header
    stp xzr, x16, [sp, #-32]

    // Push callee saved registers into cframe. They will be restored by epilogue of the compiled code.
    sub x16, sp, #((CFRAME_HEADER_SIZE + CFRAME_LOCALS_COUNT) * 8)
    PUSH_CALLEE_REGS x16

    mov x4, x3
    // Set x3 to created cframe
    sub x3, sp, #16
    mov fp, x3

    CFI_DEF_CFA_REGISTER(fp)
    CFI_ADJUST_CFA_OFFSET(16)
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

    // Adjust stack for the whole cframe
    sub sp, sp, x4

    OSR_ENTRY

    // Set frame kind in the TLS
    stp x0, xzr, [sp, #-16]!
    bl MAKE_ASM_NAME(GetCurrentManagedThread)
    strb wzr, [x0, #MANAGED_THREAD_FRAME_KIND_OFFSET]
    ldp x0, xzr, [sp], #16

    // Deallocate space for stack parameters
    add sp, sp, x22

    // Remove I2C bridge
    add sp, sp, #32

    // Restore interpreter frame and x22 register which held stack parameters size
    ldp x16, x22, [sp], #16
    CFI_RESTORE(x22)

    // Call SetOsrResult for writing the result to the interpreter frame's accumulator
    mov x1, x0
    mov x0, x16
    bl MAKE_ASM_NAME(SetOsrResult)

    // Restore original lr and fp
    ldp fp, lr, [sp], #16
    CFI_RESTORE(lr)
    CFI_RESTORE(fp)
    CFI_DEF_CFA(sp, 0)
    ret
    CFI_ENDPROC

.global MAKE_ASM_NAME(OsrEntryTopFrame)
TYPE_FUNCTION(OsrEntryTopFrame)
MAKE_ASM_NAME(OsrEntryTopFrame):
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    // x0 - pointer to Frame
    // x1 - bytecode offset
    // x2 - pointer to compiled code
    // x3 - frame size
    // x4 - stack params size

    // Save original lr and fp before cframe
    stp fp, lr, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)
    CFI_REL_OFFSET(lr, 8)
    CFI_REL_OFFSET(fp, 0)
    // Save pointer to IFrame, we'll need it later
    stp x0, x22, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(16)

    // Allocate space for stack parameters
    sub sp, sp, x4
    // Save stack parameters size in callee register, so we'll can to deallocate space after call
    mov x22, x4

    // Save lr and fp, fp is null because it is the top frame
    str lr, [sp, #-8]
    str xzr, [sp, #-16]

    // Save method
    ldr x16, [x0, #FRAME_METHOD_OFFSET]
    stp xzr, x16, [sp, #-32]

    // Push callee saved registers into cframe. They will be restored by epilogue of the compiled code.
    sub x4, sp, #((CFRAME_HEADER_SIZE + CFRAME_LOCALS_COUNT) * 8)
    PUSH_CALLEE_REGS x4

    mov x4, x3
    // Set x3 to created cframe
    sub x3, sp, #16
    mov fp, x3

    CFI_DEF_CFA_REGISTER(fp)
    CFI_ADJUST_CFA_OFFSET(16)
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

    // Adjust stack for the whole cframe
    sub sp, sp, x4

    OSR_ENTRY

    // Deallocate space for stack parameters
    add sp, sp, x22

    // Set frame kind in the TLS
    stp x0, xzr, [sp, #-16]!
    bl MAKE_ASM_NAME(GetCurrentManagedThread)
    strb wzr, [x0, #MANAGED_THREAD_FRAME_KIND_OFFSET]
    ldp x0, xzr, [sp], #16

    // Restore interpreter frame and x22 register, which held stack parameters size
    ldp x16, x22, [sp], #16

    // Call SetOsrResult to write result to the interpreter frame's accumulator
    mov x1, x0
    mov x0, x16
    bl MAKE_ASM_NAME(SetOsrResult)

    // Restore original lr and fp
    ldp fp, lr, [sp], #16
    CFI_RESTORE(lr)
    CFI_RESTORE(fp)
    CFI_DEF_CFA(sp, 0)
    ret
    CFI_ENDPROC
