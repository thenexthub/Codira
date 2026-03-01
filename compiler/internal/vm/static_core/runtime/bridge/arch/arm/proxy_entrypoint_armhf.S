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
#include "arch/arm/shorty.S"
#include "shorty_values.h"

.macro PROXY_ENTRYPOINT name, entry
.global \name
.type \name, %function
\name:
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)
    sub sp, sp, #16
    CFI_ADJUST_CFA_OFFSET(16)
    str lr, [sp, #12]
    CFI_REL_OFFSET(lr, 12)
    mov ip, #COMPILED_CODE_TO_INTERPRETER_BRIDGE
    str ip, [sp, #8]
    str fp, [sp, #4]
    CFI_REL_OFFSET(fp, 4)
    add fp, sp, #4
    CFI_DEF_CFA_REGISTER(fp)
    CFI_ADJUST_CFA_OFFSET(-4)

    // save all the callee saved registers to the stack
    // stack walker will read them during stack unwinding
    push {r4 - r10}
    CFI_REL_OFFSET(r10, -(2 * 4))
    CFI_REL_OFFSET(r9,  -(3 * 4))
    CFI_REL_OFFSET(r8,  -(4 * 4))
    CFI_REL_OFFSET(r7,  -(5 * 4))
    CFI_REL_OFFSET(r6,  -(6 * 4))
    CFI_REL_OFFSET(r5,  -(7 * 4))
    CFI_REL_OFFSET(r4,  -(8 * 4))
    vpush {d8 - d15}
    CFI_REL_OFFSET(d15, -(10 * 4))
    CFI_REL_OFFSET(d14, -(12 * 4))
    CFI_REL_OFFSET(d13, -(14 * 4))
    CFI_REL_OFFSET(d12, -(16 * 4))
    CFI_REL_OFFSET(d11, -(18 * 4))
    CFI_REL_OFFSET(d10, -(20 * 4))
    CFI_REL_OFFSET(d9,  -(22 * 4))
    CFI_REL_OFFSET(d8,  -(24 * 4))

    // align to 8
    sub sp, sp, #4

    // save arguments to the stack
    vpush {d0 - d7}
    push {r0 - r3}

    ldrb r7, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]

    mov r1, sp
    add r2, fp, #12
    blx \entry

    pop {r2}

    // handle the result
    ldr r2, [r2, #METHOD_SHORTY_OFFSET]
    ldr r2, [r2]
    and r2, r2, #0xF

    sub r2, r2, #SHORTY_FIRST_FLOAT
    cmp r2, #(SHORTY_NUM_FLOAT_TYPES - 1)
    bls 1f
    b 2f

    // it is a float value. Move it to d0
1:  vmov d0, r0, r1

2:
    // Restore callee registers, since GC may change its values while moving objects.
    sub sp, fp, #4

    mov r2, r7

    ldmdb sp, {r4 - r10}
    CFI_RESTORE(r10)
    CFI_RESTORE(r9)
    CFI_RESTORE(r8)
    CFI_RESTORE(r7)
    CFI_RESTORE(r6)
    CFI_RESTORE(r5)
    CFI_RESTORE(r4)

    sub sp, fp, #4
    CFI_DEF_CFA(sp, 16)
    ldr fp, [sp, #4]
    CFI_RESTORE(fp)
    add sp, sp, #12 // c2i
    CFI_ADJUST_CFA_OFFSET(-12)

    // check native exception
    ldr r3, [THREAD_REG, #MANAGED_THREAD_EXCEPTION_OFFSET]
    cmp r3, #0
    beq 3f

    // check frame is compiled
    cmp r2, #0
    beq 3f

    // check prev frame is true CFRAME and not BYPASS
    ldr r3, [fp, #(SLOT_SIZE * COMP_METHOD_OFFSET)]
    cmp r3, #BYPASS_BRIDGE
    beq 3f
    CFI_REMEMBER_STATE

    add lr, fp, #-CALLER_REG0_OFFSET
    ldm lr, {r0-r3}

    ldr lr, [fp, #(-CFRAME_FLAGS_SLOT * 4)]
    tst lr, #CFRAME_HAS_FLOAT_REGS_FLAG_MASK
    beq 4f

    add lr, fp, #-CALLER_VREG0_OFFSET
    vldm lr, {d0-d7}

4:
    pop {lr}
    CFI_ADJUST_CFA_OFFSET(-4)
    CFI_RESTORE(lr)

    b ThrowNativeExceptionBridge
    CFI_RESTORE_STATE

3:
    pop {lr}
    CFI_ADJUST_CFA_OFFSET(-4)
    CFI_RESTORE(lr)
    bx lr
    CFI_ENDPROC
.endm

