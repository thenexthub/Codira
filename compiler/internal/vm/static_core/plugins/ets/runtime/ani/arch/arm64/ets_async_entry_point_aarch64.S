/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "arch/aarch64/shorty.S"
#include "shorty_values.h"

// Promise EtsAsyncCall(Method *method, ManagedThread *thread, const uint8_t *reg_args, const uint8_t *stack_args)
.extern MAKE_ASM_NAME(EtsAsyncCall)

// The entrypoint for an async method
.global MAKE_ASM_NAME(EtsAsyncEntryPoint)
TYPE_FUNCTION(EtsAsyncEntryPoint)
MAKE_ASM_NAME(EtsAsyncEntryPoint):
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    stp fp, lr, [sp, #-16]!
    CFI_ADJUST_CFA_OFFSET(2 * 8)
    CFI_REL_OFFSET(lr, 8)
    CFI_REL_OFFSET(fp, 0)
    mov fp, sp
    CFI_DEF_CFA_REGISTER(fp)
    sub sp, sp, #16
    str x0, [sp, #8]
    mov x16, #CFRAME_KIND_NATIVE
    str x16, [sp]

    // Skip locals
    sub sp, sp, #(CFRAME_LOCALS_COUNT * 8)

    // save all the callee saved registers to the stack
    // stack walker will read them during stack unwinding
    stp x27, x28, [sp, #-16]!
    CFI_REL_OFFSET(THREAD_REG, -((CFRAME_CALLEE_REGS_START_SLOT + 0) * 8))
    CFI_REL_OFFSET(x27, -((CFRAME_CALLEE_REGS_START_SLOT + 1) * 8))
    stp x25, x26, [sp, #-16]!
    CFI_REL_OFFSET(x26, -((CFRAME_CALLEE_REGS_START_SLOT + 2) * 8))
    CFI_REL_OFFSET(x25, -((CFRAME_CALLEE_REGS_START_SLOT + 3) * 8))
    stp x23, x24, [sp, #-16]!
    CFI_REL_OFFSET(x24, -((CFRAME_CALLEE_REGS_START_SLOT + 4) * 8))
    CFI_REL_OFFSET(x23, -((CFRAME_CALLEE_REGS_START_SLOT + 5) * 8))
    stp x21, x22, [sp, #-16]!
    CFI_REL_OFFSET(x22, -((CFRAME_CALLEE_REGS_START_SLOT + 6) * 8))
    CFI_REL_OFFSET(x21, -((CFRAME_CALLEE_REGS_START_SLOT + 7) * 8))
    stp x19, x20, [sp, #-16]!
    CFI_REL_OFFSET(x20, -((CFRAME_CALLEE_REGS_START_SLOT + 8) * 8))
    CFI_REL_OFFSET(x19, -((CFRAME_CALLEE_REGS_START_SLOT + 9) * 8))
    stp d14, d15, [sp, #-16]!
    CFI_REL_OFFSET(d15, -((CFRAME_CALLEE_REGS_START_SLOT + 10) * 8))
    CFI_REL_OFFSET(d14, -((CFRAME_CALLEE_REGS_START_SLOT + 11) * 8))
    stp d12, d13, [sp, #-16]!
    CFI_REL_OFFSET(d13, -((CFRAME_CALLEE_REGS_START_SLOT + 12) * 8))
    CFI_REL_OFFSET(d12, -((CFRAME_CALLEE_REGS_START_SLOT + 13) * 8))
    stp d10, d11, [sp, #-16]!
    CFI_REL_OFFSET(d11, -((CFRAME_CALLEE_REGS_START_SLOT + 14) * 8))
    CFI_REL_OFFSET(d10, -((CFRAME_CALLEE_REGS_START_SLOT + 15) * 8))
    stp d8, d9, [sp, #-16]!
    CFI_REL_OFFSET(d9, -((CFRAME_CALLEE_REGS_START_SLOT + 16) * 8))
    CFI_REL_OFFSET(d8, -((CFRAME_CALLEE_REGS_START_SLOT + 17) * 8))

    // save arguments to the stack
    stp d6, d7, [sp, #-16]!
    stp d4, d5, [sp, #-16]!
    stp d2, d3, [sp, #-16]!
    stp d0, d1, [sp, #-16]!
    stp x6, x7, [sp, #-16]!
    stp x4, x5, [sp, #-16]!
    stp x2, x3, [sp, #-16]!
    stp x0, x1, [sp, #-16]!
    mov x2, sp

    ldrb w20, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]

    str fp, [THREAD_REG, #MANAGED_THREAD_FRAME_OFFSET]
    mov w1, #1
    strb w1, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]

    add x3, fp, #16
    mov x1, THREAD_REG
    bl MAKE_ASM_NAME(EtsAsyncCall)

    strb w20, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]
    mov w10, w20

    // Restore callee registers, since GC may change its values while moving objects.
    // Signal handler of the sampling profiler use stack space below sp, 
    // so change it carefully only after registers restoration
    sub sp, fp, #((CFRAME_LOCALS_COUNT + 12) * 8)
    ldp x19, x20, [sp], #16
    CFI_RESTORE(x20)
    CFI_RESTORE(x19)
    ldp x21, x22, [sp], #16
    CFI_RESTORE(x22)
    CFI_RESTORE(x21)
    ldp x23, x24, [sp], #16
    CFI_RESTORE(x24)
    CFI_RESTORE(x23)
    ldp x25, x26, [sp], #16
    CFI_RESTORE(x26)
    CFI_RESTORE(x25)
    ldp x27, x28, [sp], #16
    CFI_RESTORE(THREAD_REG)
    CFI_RESTORE(x27)

    mov sp, fp

    ldp fp, lr, [sp], #16
    CFI_RESTORE(lr)
    CFI_RESTORE(fp)
    CFI_DEF_CFA(sp, 0)

    // check native exception
    ldr x11, [THREAD_REG, #MANAGED_THREAD_EXCEPTION_OFFSET]
    cmp x11, #0
    // check frame is compiled
    ccmp w10, #0, #4, ne
    // check prev frame is true CFRAME and not BYPASS
    ldr x9, [fp, #(SLOT_SIZE * COMP_METHOD_OFFSET)]
    ccmp x9, #BYPASS_BRIDGE, 4, ne
    beq .Lexit

    ldr x13, [fp, #(-CFRAME_FLAGS_SLOT * 8)]
    tbz x13, #CFRAME_HAS_FLOAT_REGS_FLAG_BIT, 2f

    add x12, fp, #-CALLER_VREG0_OFFSET
    ldp d0, d1, [x12]
    ldp d2, d3, [x12, #8*2]
    ldp d4, d5, [x12, #8*4]
    ldp d6, d7, [x12, #8*6]
    ldp d16, d17, [x12, #8*8]
    ldp d18, d19, [x12, #8*10]
    ldp d20, d21, [x12, #8*12]
    ldp d22, d23, [x12, #8*14]
    ldp d24, d25, [x12, #8*16]
    ldp d26, d27, [x12, #8*18]
    ldp d28, d29, [x12, #8*20]
    ldp d30, d31, [x12, #8*22]

2:
    ldp x0,  x1,  [fp, #-CALLER_REG0_OFFSET+8*0]
    ldp x2,  x3,  [fp, #-CALLER_REG0_OFFSET+8*2]
    ldp x4,  x5,  [fp, #-CALLER_REG0_OFFSET+8*4]
    ldp x6,  x7,  [fp, #-CALLER_REG0_OFFSET+8*6]
    ldp x8,  x9,  [fp, #-CALLER_REG0_OFFSET+8*8]
    ldp x10, x11, [fp, #-CALLER_REG0_OFFSET+8*10]
    ldp x12, x13, [fp, #-CALLER_REG0_OFFSET+8*12]
    ldp x14, x15, [fp, #-CALLER_REG0_OFFSET+8*14]
    ldp x16, x17, [fp, #-CALLER_REG0_OFFSET+8*16]
    ldr x18, [fp, #-CALLER_REG0_OFFSET+8*18]

    b MAKE_ASM_NAME(ThrowNativeExceptionBridge)

.Lexit:
    ret
    CFI_ENDPROC
