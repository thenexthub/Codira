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
#include "arch/aarch64/shorty.S"
#include "arch/aarch64/helpers_aarch64.S"
#include "shorty_values.h"

.macro PROXY_ENTRYPOINT name, entry, skip_c2i_bridge
.global \name
#ifdef PANDA_TARGET_MACOS
.private_extern \name
.private_extern \entry
#else
#ifdef PANDA_WITH_HIDDEN_SYMBOLS
.hidden \name
.hidden \entry
#else
.protected \name
.protected \entry
#endif
.type \name, %function
#endif
\name:
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    sub sp, sp, #32
    CFI_ADJUST_CFA_OFFSET(4 * 8)
    str lr, [sp, #24]
    CFI_REL_OFFSET(lr, 24)
    mov lr, #COMPILED_CODE_TO_INTERPRETER_BRIDGE
    stp fp, lr, [sp, #8]
    CFI_REL_OFFSET(fp, 8)
    add fp, sp, #8
    CFI_DEF_CFA(fp, (3 * 8))

    // save all the callee saved registers to the stack
    // stack walker will read them during stack unwinding
    PUSH_CALLEE_REGS sp
    CFI_REL_OFFSET(x28, -(2 * 8))
    CFI_REL_OFFSET(x27, -(3 * 8))
    CFI_REL_OFFSET(x26, -(4 * 8))
    CFI_REL_OFFSET(x25, -(5 * 8))
    CFI_REL_OFFSET(x24, -(6 * 8))
    CFI_REL_OFFSET(x23, -(7 * 8))
    CFI_REL_OFFSET(x22, -(8 * 8))
    CFI_REL_OFFSET(x21, -(9 * 8))
    CFI_REL_OFFSET(x20, -(10 * 8))
    CFI_REL_OFFSET(x19, -(11 * 8))
    CFI_REL_OFFSET(d15, -(12 * 8))
    CFI_REL_OFFSET(d14, -(13 * 8))
    CFI_REL_OFFSET(d13, -(14 * 8))
    CFI_REL_OFFSET(d12, -(15 * 8))
    CFI_REL_OFFSET(d11, -(16 * 8))
    CFI_REL_OFFSET(d10, -(17 * 8))
    CFI_REL_OFFSET(d9, -(18 * 8))
    CFI_REL_OFFSET(d8, -(19 * 8))

    // If caller is the compiled method, we need to set C2I boundary frame in the TLS
    ldrb w20, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]
    cmp w20, #0
    beq 0f

    str fp, [THREAD_REG, #MANAGED_THREAD_FRAME_OFFSET]

0:

    // save arguments to the stack
    stp d6, d7, [sp, #-16]!
    stp d4, d5, [sp, #-16]!
    stp d2, d3, [sp, #-16]!
    stp d0, d1, [sp, #-16]!
    stp x6, x7, [sp, #-16]!
    stp x4, x5, [sp, #-16]!
    stp x2, x3, [sp, #-16]!
    stp x0, x1, [sp, #-16]!

    mov x1, sp
    add x2, fp, #24

    bl \entry

    ldr x1, [sp]

    // handle the result
    ldr x1, [x1, #METHOD_SHORTY_OFFSET]
    ldr w1, [x1]
    and w1, w1, #0xF

    sub w1, w1, #SHORTY_FIRST_FLOAT
    cmp w1, #(SHORTY_NUM_FLOAT_TYPES - 1)
    bls 1f
    b 2f

1:  fmov d0, x0

2:
    mov w10, w20

    // Restore callee registers, since GC may change its values while moving objects.
    sub x16, fp, #8
    ldp x27, x28, [x16, #-16]!
    CFI_RESTORE(x28)
    CFI_RESTORE(x27)
    ldp x25, x26, [x16, #-16]!
    CFI_RESTORE(x26)
    CFI_RESTORE(x25)
    ldp x23, x24, [x16, #-16]!
    CFI_RESTORE(x24)
    CFI_RESTORE(x23)
    ldp x21, x22, [x16, #-16]!
    CFI_RESTORE(x22)
    CFI_RESTORE(x21)
    ldp x19, x20, [x16, #-16]!
    CFI_RESTORE(x20)
    CFI_RESTORE(x19)

    sub sp, fp, #8
    ldr fp, [sp, #8]
    CFI_RESTORE(fp)
    CFI_DEF_CFA(sp, (4 * 8))
    ldr lr, [sp, #24]
    CFI_RESTORE(lr)
    add sp, sp, #32
    CFI_ADJUST_CFA_OFFSET(-(4 * 8))

    // check native exception
    ldr x11, [THREAD_REG, #MANAGED_THREAD_EXCEPTION_OFFSET]
    cmp x11, #0
    // check frame is compiled
    ccmp w10, #0, #4, ne
    // check prev frame is true CFRAME and not BYPASS
    ldr x9, [fp, #(SLOT_SIZE * COMP_METHOD_OFFSET)]
    ccmp x9, #BYPASS_BRIDGE, 4, ne
    beq 3f

    ldr x13, [fp, #(-CFRAME_FLAGS_SLOT * 8)]
    tbz x13, #CFRAME_HAS_FLOAT_REGS_FLAG_BIT, 4f

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

4:
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

3:
    ret
    CFI_ENDPROC
.endm

