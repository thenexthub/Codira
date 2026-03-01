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
#include "arch/arm/asm_constants.h"
#include "arch/arm/shorty.S"
#include "shorty_values.h"

// Promise EtsAsyncCall(Method *method, ManagedThread *thread, const uint8_t *reg_args, const uint8_t *stack_args)
.extern EtsAsyncCall

// The entrypoint for an async method
.global EtsAsyncEntryPoint
.type EtsAsyncEntryPoint, %function
EtsAsyncEntryPoint:
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    push {fp, lr}
    CFI_ADJUST_CFA_OFFSET(8)
    CFI_REL_OFFSET(lr, 4)
    CFI_REL_OFFSET(fp, 0)

    mov fp, sp
    CFI_DEF_CFA_REGISTER(fp)
    sub sp, sp, #8
    str r0, [sp, #4]
    mov lr, #CFRAME_KIND_NATIVE
    str lr, [sp]

    // Skip locals
    sub sp, sp, #(CFRAME_LOCALS_COUNT * 4)

    // save all the callee saved registers to the stack
    // stack walker will read them during stack unwinding
    vpush {d8 - d15}
    CFI_REL_OFFSET(d15, -((CFRAME_LOCALS_COUNT + 2 + 2) * 4))
    CFI_REL_OFFSET(d14, -((CFRAME_LOCALS_COUNT + 2 + 4) * 4))
    CFI_REL_OFFSET(d13, -((CFRAME_LOCALS_COUNT + 2 + 6) * 4))
    CFI_REL_OFFSET(d12, -((CFRAME_LOCALS_COUNT + 2 + 8) * 4))
    CFI_REL_OFFSET(d11, -((CFRAME_LOCALS_COUNT + 2 + 10) * 4))
    CFI_REL_OFFSET(d10, -((CFRAME_LOCALS_COUNT + 2 + 12) * 4))
    CFI_REL_OFFSET(d9,  -((CFRAME_LOCALS_COUNT + 2 + 14) * 4))
    CFI_REL_OFFSET(d8,  -((CFRAME_LOCALS_COUNT + 2 + 16) * 4))
    push {r4 - r10}
    CFI_REL_OFFSET(r10, -((CFRAME_LOCALS_COUNT + 18 + 1) * 4)) // Thread pointer
    CFI_REL_OFFSET(r9,  -((CFRAME_LOCALS_COUNT + 18 + 2) * 4)) 
    CFI_REL_OFFSET(r8,  -((CFRAME_LOCALS_COUNT + 18 + 3) * 4))
    CFI_REL_OFFSET(r7,  -((CFRAME_LOCALS_COUNT + 18 + 4) * 4))
    CFI_REL_OFFSET(r6,  -((CFRAME_LOCALS_COUNT + 18 + 5) * 4))
    CFI_REL_OFFSET(r5,  -((CFRAME_LOCALS_COUNT + 18 + 6) * 4))
    CFI_REL_OFFSET(r4,  -((CFRAME_LOCALS_COUNT + 18 + 7) * 4))

    // align to 8
    sub sp, sp, #4

    // save arguments to the stack
    vpush {d0 - d7}
    push {r0 - r3}
    mov r2, sp

    str fp, [THREAD_REG, #MANAGED_THREAD_FRAME_OFFSET]
    ldrb r7, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]
    mov r1, #1
    strb r1, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]

    add r3, fp, #8
    mov r1, THREAD_REG
    bl EtsAsyncCall

    strb r7, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]
    mov r2, r7

    // Restore callee registers, since GC may change its values while moving objects.
    sub r1, fp, #(CFRAME_ARM_HARD_CALLEE_REGS_OFFSET - 4)

    ldm r1, {r4 - r10}
    CFI_RESTORE(r10)
    CFI_RESTORE(r9)
    CFI_RESTORE(r8)
    CFI_RESTORE(r7)
    CFI_RESTORE(r6)
    CFI_RESTORE(r5)
    CFI_RESTORE(r4)
    mov sp, fp
    CFI_DEF_CFA(sp, 8)
    pop {fp}
    CFI_ADJUST_CFA_OFFSET(-4)
    CFI_RESTORE(fp)
    CFI_REMEMBER_STATE

    // check native exception
    ldr r3, [THREAD_REG, #MANAGED_THREAD_EXCEPTION_OFFSET]
    cmp r3, #0
    beq .Lexit

    // check frame is compiled
    cmp r2, #0
    beq .Lexit

    // check prev frame is true CFRAME and not BYPASS
    ldr r3, [fp, #(SLOT_SIZE * COMP_METHOD_OFFSET)]
    cmp r3, #BYPASS_BRIDGE
    beq .Lexit

    add lr, fp, #-CALLER_REG0_OFFSET
    ldm lr, {r0-r3}

    ldr lr, [fp, #(-CFRAME_FLAGS_SLOT * 4)]
    tst lr, #CFRAME_HAS_FLOAT_REGS_FLAG_MASK
    beq 1f

    add lr, fp, #-CALLER_VREG0_OFFSET
    vldm lr, {d0-d7}

1:
    pop {lr}
    CFI_ADJUST_CFA_OFFSET(-4)
    CFI_RESTORE(lr)

    b ThrowNativeExceptionBridge
    CFI_RESTORE_STATE

.Lexit:
    pop {lr}
    CFI_ADJUST_CFA_OFFSET(-4)
    CFI_RESTORE(lr)
    bx lr
    CFI_ENDPROC

