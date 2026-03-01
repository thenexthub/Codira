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

 #include "asm_defines.h"
 #include "arch/asm_support.h"

.macro SAVE_CALLEE_GP_REGS base_reg, offset
    stp x27, x28, [\base_reg, #(\offset - CALLEE_REG0_OFFSET + 8*8)]
    CFI_REL_OFFSET(x28, (\offset - CALLEE_REG0_OFFSET + 8*9))
    CFI_REL_OFFSET(x27, (\offset - CALLEE_REG0_OFFSET + 8*8))
    stp x25, x26, [\base_reg, #(\offset - CALLEE_REG0_OFFSET + 8*6)]
    CFI_REL_OFFSET(x26, (\offset - CALLEE_REG0_OFFSET + 8*7))
    CFI_REL_OFFSET(x25, (\offset - CALLEE_REG0_OFFSET + 8*6))
    stp x23, x24, [\base_reg, #(\offset - CALLEE_REG0_OFFSET + 8*4)]
    CFI_REL_OFFSET(x24, (\offset - CALLEE_REG0_OFFSET + 8*5))
    CFI_REL_OFFSET(x23, (\offset - CALLEE_REG0_OFFSET + 8*4))
    stp x21, x22, [\base_reg, #(\offset - CALLEE_REG0_OFFSET + 8*2)]
    CFI_REL_OFFSET(x22, (\offset - CALLEE_REG0_OFFSET + 8*3))
    CFI_REL_OFFSET(x21, (\offset - CALLEE_REG0_OFFSET + 8*2))
    stp x19, x20, [\base_reg, #(\offset - CALLEE_REG0_OFFSET)]
    CFI_REL_OFFSET(x20, (\offset - CALLEE_REG0_OFFSET + 8*1))
    CFI_REL_OFFSET(x19, (\offset - CALLEE_REG0_OFFSET + 8*0))
.endm

.macro RESTORE_CALLEE_GP_REGS base_reg, offset
    ldp x27, x28, [\base_reg, #(\offset - CALLEE_REG0_OFFSET + 8*8)]
    CFI_RESTORE(x28)
    CFI_RESTORE(x27)
    ldp x25, x26, [\base_reg, #(\offset - CALLEE_REG0_OFFSET + 8*6)]
    CFI_RESTORE(x26)
    CFI_RESTORE(x25)
    ldp x23, x24, [\base_reg, #(\offset - CALLEE_REG0_OFFSET + 8*4)]
    CFI_RESTORE(x24)
    CFI_RESTORE(x23)
    ldp x21, x22, [\base_reg, #(\offset - CALLEE_REG0_OFFSET + 8*2)]
    CFI_RESTORE(x22)
    CFI_RESTORE(x21)
    ldp x19, x20, [\base_reg, #(\offset - CALLEE_REG0_OFFSET)]
    CFI_RESTORE(x20)
    CFI_RESTORE(x19)
.endm

.macro SAVE_CALLEE_FP_REGS base_reg, offset
    stp d14, d15, [\base_reg, #(\offset - CALLEE_VREG0_OFFSET + 8*6)]
    CFI_REL_OFFSET(d15, (\offset - CALLEE_VREG0_OFFSET + 8*7))
    CFI_REL_OFFSET(d14, (\offset - CALLEE_VREG0_OFFSET + 8*6))
    stp d12, d13, [\base_reg, #(\offset - CALLEE_VREG0_OFFSET + 8*4)]
    CFI_REL_OFFSET(d13, (\offset - CALLEE_VREG0_OFFSET + 8*5))
    CFI_REL_OFFSET(d12, (\offset - CALLEE_VREG0_OFFSET + 8*4))
    stp d10, d11, [\base_reg, #(\offset - CALLEE_VREG0_OFFSET + 8*2)]
    CFI_REL_OFFSET(d11, (\offset - CALLEE_VREG0_OFFSET + 8*3))
    CFI_REL_OFFSET(d10, (\offset - CALLEE_VREG0_OFFSET + 8*2))
    stp d8, d9,   [\base_reg, #(\offset - CALLEE_VREG0_OFFSET)]
    CFI_REL_OFFSET(d9,  (\offset - CALLEE_VREG0_OFFSET + 8*1))
    CFI_REL_OFFSET(d8,  (\offset - CALLEE_VREG0_OFFSET + 8*0))
.endm

.macro RESTORE_CALLEE_FP_REGS base_reg, offset
    ldp d14, d15, [\base_reg, #(\offset - CALLEE_VREG0_OFFSET + 8*6)]
    CFI_RESTORE(d15)
    CFI_RESTORE(d14)
    ldp d12, d13, [\base_reg, #(\offset - CALLEE_VREG0_OFFSET + 8*4)]
    CFI_RESTORE(d13)
    CFI_RESTORE(d12)
    ldp d10, d11, [\base_reg, #(\offset - CALLEE_VREG0_OFFSET + 8*2)]
    CFI_RESTORE(d11)
    CFI_RESTORE(d10)
    ldp d8, d9,   [\base_reg, #(\offset - CALLEE_VREG0_OFFSET)]
    CFI_RESTORE(d9)
    CFI_RESTORE(d8)
.endm

.macro SAVE_CALLER_GP_REGS fp_reg, tmp_reg, paramsnum, mode, ret_type
.if \mode != RUNTIME_MODE_STUB
    // parameter holding registers are saved by the caller function
.ifeq \paramsnum
.ifnc \ret_type,INTEGER
    stp x0,  x1,  [\fp_reg, #-CALLER_REG0_OFFSET+8*0]
.else
    add \tmp_reg, \fp_reg, #-CALLER_REG0_OFFSET
    str x1,  [\tmp_reg, #8*1]
.endif
.else // paramsnum > 0
.if \mode == RUNTIME_MODE_ODD_SAVED
.ifeq \paramsnum-1
    add \tmp_reg, \fp_reg, #-CALLER_REG0_OFFSET
    str x1,  [\tmp_reg, #8*1]
.endif
.endif // mode == RUNTIME_MODE_ODD_SAVED
.endif
.ifle \paramsnum-2
    stp x2,  x3,  [\fp_reg, #-CALLER_REG0_OFFSET+8*2]
.else
.if \mode == RUNTIME_MODE_ODD_SAVED
.ifeq \paramsnum-3
    add \tmp_reg, \fp_reg, #-CALLER_REG0_OFFSET
    str x3,  [\tmp_reg, #8*3]
.endif
.endif // mode == RUNTIME_MODE_ODD_SAVED
.endif
.ifle \paramsnum-4
    stp x4,  x5,  [\fp_reg, #-CALLER_REG0_OFFSET+8*4]
.else
.if \mode == RUNTIME_MODE_ODD_SAVED
.ifeq \paramsnum-5
    add \tmp_reg, \fp_reg, #-CALLER_REG0_OFFSET
    str x5,  [\tmp_reg, #8*5]
.endif
.endif // mode == RUNTIME_MODE_ODD_SAVED
.endif
.endif // mode != RUNTIME_MODE_STUB

    // Save caller saved regs
    // first registers are stored by user of this macro or caller which uses them for arguments
    stp x6,  x7,  [\fp_reg, #-CALLER_REG0_OFFSET+8*6]
    stp x8,  x9,  [\fp_reg, #-CALLER_REG0_OFFSET+8*8]
    stp x10, x11, [\fp_reg, #-CALLER_REG0_OFFSET+8*10]
    stp x12, x13, [\fp_reg, #-CALLER_REG0_OFFSET+8*12]
    stp x14, x15, [\fp_reg, #-CALLER_REG0_OFFSET+8*14]
    stp x16, x17, [\fp_reg, #-CALLER_REG0_OFFSET+8*16]
    str x18, [\fp_reg, #-CALLER_REG0_OFFSET+8*18]
.endm

.macro RESTORE_CALLER_GP_REGS fp_reg, tmp_reg, ret_type
.ifnc \ret_type,INTEGER
    ldp x0,  x1,  [\fp_reg, #-CALLER_REG0_OFFSET+8*0]
.else
    add \tmp_reg, \fp_reg, #-CALLER_REG0_OFFSET
    ldr x1,  [\tmp_reg, #8*1]
.endif
    ldp x2,  x3,  [\fp_reg, #-CALLER_REG0_OFFSET+8*2]
    ldp x4,  x5,  [\fp_reg, #-CALLER_REG0_OFFSET+8*4]
    ldp x6,  x7,  [\fp_reg, #-CALLER_REG0_OFFSET+8*6]
    ldp x8,  x9,  [\fp_reg, #-CALLER_REG0_OFFSET+8*8]
    ldp x10, x11, [\fp_reg, #-CALLER_REG0_OFFSET+8*10]
    ldp x12, x13, [\fp_reg, #-CALLER_REG0_OFFSET+8*12]
    ldp x14, x15, [\fp_reg, #-CALLER_REG0_OFFSET+8*14]
    ldp x16, x17, [\fp_reg, #-CALLER_REG0_OFFSET+8*16]
    ldr x18, [\fp_reg, #-CALLER_REG0_OFFSET+8*18]
.endm

.macro SAVE_CALLER_FP_REGS fp_reg, tmp_reg, ret_type
    add \tmp_reg, \fp_reg, #-CALLER_VREG0_OFFSET
.ifnc \ret_type,FLOAT
    stp d0, d1, [\tmp_reg]
.else
    str d1, [\tmp_reg, #8*1]
.endif
    stp d2, d3, [\tmp_reg, #8*2]
    stp d4, d5, [\tmp_reg, #8*4]
    stp d6, d7, [\tmp_reg, #8*6]
    stp d16, d17, [\tmp_reg, #8*8]
    stp d18, d19, [\tmp_reg, #8*10]
    stp d20, d21, [\tmp_reg, #8*12]
    stp d22, d23, [\tmp_reg, #8*14]
    stp d24, d25, [\tmp_reg, #8*16]
    stp d26, d27, [\tmp_reg, #8*18]
    stp d28, d29, [\tmp_reg, #8*20]
    stp d30, d31, [\tmp_reg, #8*22]
.endm

.macro RESTORE_CALLER_FP_REGS fp_reg, tmp_reg, ret_type
    add \tmp_reg, \fp_reg, #-CALLER_VREG0_OFFSET
.ifnc \ret_type,FLOAT
    ldp d0, d1, [\tmp_reg]
.else
    ldr d1, [\tmp_reg, #8*1]
.endif
    ldp d2, d3, [\tmp_reg, #8*2]
    ldp d4, d5, [\tmp_reg, #8*4]
    ldp d6, d7, [\tmp_reg, #8*6]
    ldp d16, d17, [\tmp_reg, #8*8]
    ldp d18, d19, [\tmp_reg, #8*10]
    ldp d20, d21, [\tmp_reg, #8*12]
    ldp d22, d23, [\tmp_reg, #8*14]
    ldp d24, d25, [\tmp_reg, #8*16]
    ldp d26, d27, [\tmp_reg, #8*18]
    ldp d28, d29, [\tmp_reg, #8*20]
    ldp d30, d31, [\tmp_reg, #8*22]
.endm

.macro CHECK_NATIVE_EXCEPTION ret_type
    ldr lr, [THREAD_REG, #MANAGED_THREAD_EXCEPTION_OFFSET]
    cmp lr, 0

    beq 1f
    CFI_REMEMBER_STATE

.ifc \ret_type,INTEGER
    add lr, fp, #-CALLER_REG0_OFFSET
    ldr x0, [lr]
.endif
.ifc \ret_type,FLOAT
    add lr, fp, #-CALLER_VREG0_OFFSET
    ldr d0, [lr]
.endif

    ldr lr, [sp, #(BRIDGE_FRAME_SIZE - 8)]
    CFI_RESTORE(lr)

    add sp, sp, #BRIDGE_FRAME_SIZE
    CFI_ADJUST_CFA_OFFSET(-BRIDGE_FRAME_SIZE)

    b MAKE_ASM_NAME(ThrowNativeExceptionBridge)
    CFI_RESTORE_STATE

1:
    ldr lr, [sp, #(BRIDGE_FRAME_SIZE - 8)]
    CFI_RESTORE(lr)

    add sp, sp, #BRIDGE_FRAME_SIZE
    CFI_ADJUST_CFA_OFFSET(-BRIDGE_FRAME_SIZE)
.endm

.macro CALL_RUNTIME mode, entry, paramsnum, ret_type
    sub sp, sp, #BRIDGE_FRAME_SIZE
    CFI_ADJUST_CFA_OFFSET(BRIDGE_FRAME_SIZE)

    // BoundaryFrame setup
    str lr, [sp, #(BRIDGE_FRAME_SIZE - 8)]    // Bridge frame, slot 1 = npc = LR (the StackMap is just after the bridge call)
    CFI_REL_OFFSET(lr, (BRIDGE_FRAME_SIZE - 1 * 8))
    str lr, [THREAD_REG, #MANAGED_THREAD_NATIVE_PC_OFFSET]   // ManagedThread.npc update
    mov lr, #COMPILED_CODE_TO_INTERPRETER_BRIDGE
    str lr, [sp, #(BRIDGE_FRAME_SIZE - 16)] // Bridge frame, slot 2 = COMPILED_CODE_TO_INTERPRETER
    str fp, [sp, #(BRIDGE_FRAME_SIZE - 24)] // Bridge frame, slot 3 = parent frame pointer
    CFI_REL_OFFSET(fp, (BRIDGE_FRAME_SIZE - 3 * 8))
    add lr, sp, #(BRIDGE_FRAME_SIZE - 24)
    str lr, [THREAD_REG, #MANAGED_THREAD_FRAME_OFFSET]    // ManagedThread._frame = this boundary frame

    SAVE_CALLEE_GP_REGS sp, BRIDGE_FRAME_SIZE
    // StackWalker requires callee-saved FP registers to be saved unconditionally in the runtime bridge
    SAVE_CALLEE_FP_REGS sp, BRIDGE_FRAME_SIZE

.if \mode != RUNTIME_MODE_SLOW_PATH
    SAVE_CALLER_GP_REGS fp, lr, \paramsnum, \mode, \ret_type
.endif

    ldr x19, [fp, #(-CFRAME_FLAGS_SLOT * 8)]
    tbz x19, #CFRAME_HAS_FLOAT_REGS_FLAG_BIT, 2f

    SAVE_CALLER_FP_REGS fp, lr, \ret_type

2:
// The call
    bl \entry

    // Restore previous state
    str fp, [THREAD_REG, #MANAGED_THREAD_FRAME_OFFSET]

    tbz x19, #CFRAME_HAS_FLOAT_REGS_FLAG_BIT, 3f

    RESTORE_CALLEE_FP_REGS sp, BRIDGE_FRAME_SIZE
    RESTORE_CALLER_FP_REGS fp, lr, \ret_type

3:
    RESTORE_CALLEE_GP_REGS sp, BRIDGE_FRAME_SIZE
    RESTORE_CALLER_GP_REGS fp, lr, \ret_type

    CHECK_NATIVE_EXCEPTION \ret_type
.endm
