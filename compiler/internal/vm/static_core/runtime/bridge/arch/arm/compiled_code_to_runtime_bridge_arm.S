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

.macro SAVE_CALLEE_GP_REGS base_reg, tmp_reg, offset
    add \tmp_reg, \base_reg, #(\offset - CALLEE_REG0_OFFSET)
    stm \tmp_reg, {r4-r10}
    CFI_REL_OFFSET(r10, (\offset - CALLEE_REG0_OFFSET + 4*6))
    CFI_REL_OFFSET(r9,  (\offset - CALLEE_REG0_OFFSET + 4*5))
    CFI_REL_OFFSET(r8,  (\offset - CALLEE_REG0_OFFSET + 4*4))
    CFI_REL_OFFSET(r7,  (\offset - CALLEE_REG0_OFFSET + 4*3))
    CFI_REL_OFFSET(r6,  (\offset - CALLEE_REG0_OFFSET + 4*2))
    CFI_REL_OFFSET(r5,  (\offset - CALLEE_REG0_OFFSET + 4*1))
    CFI_REL_OFFSET(r4,  (\offset - CALLEE_REG0_OFFSET + 4*0))
.endm

.macro RESTORE_CALLEE_GP_REGS base_reg, tmp_reg, offset
    add \tmp_reg, \base_reg, #(\offset - CALLEE_REG0_OFFSET)
    ldm \tmp_reg, {r4-r10}
    CFI_RESTORE(r10)
    CFI_RESTORE(r9)
    CFI_RESTORE(r8)
    CFI_RESTORE(r7)
    CFI_RESTORE(r6)
    CFI_RESTORE(r5)
    CFI_RESTORE(r4)
.endm

.macro SAVE_CALLEE_FP_REGS base_reg, tmp_reg, offset
    add \tmp_reg, \base_reg, #(\offset - CALLEE_VREG0_OFFSET)
    vstm \tmp_reg, {d8-d15}
    CFI_REL_OFFSET(d15, (\offset - CALLEE_VREG0_OFFSET + 4*14))
    CFI_REL_OFFSET(d14, (\offset - CALLEE_VREG0_OFFSET + 4*12))
    CFI_REL_OFFSET(d13, (\offset - CALLEE_VREG0_OFFSET + 4*10))
    CFI_REL_OFFSET(d12, (\offset - CALLEE_VREG0_OFFSET + 4*8))
    CFI_REL_OFFSET(d11, (\offset - CALLEE_VREG0_OFFSET + 4*6))
    CFI_REL_OFFSET(d10, (\offset - CALLEE_VREG0_OFFSET + 4*4))
    CFI_REL_OFFSET(d9,  (\offset - CALLEE_VREG0_OFFSET + 4*2))
    CFI_REL_OFFSET(d8,  (\offset - CALLEE_VREG0_OFFSET + 4*0))
.endm

.macro RESTORE_CALLEE_FP_REGS base_reg, tmp_reg, offset
    add \tmp_reg, \base_reg, #(\offset - CALLEE_VREG0_OFFSET)
    vldm \tmp_reg, {d8-d15}
    CFI_RESTORE(d15)
    CFI_RESTORE(d14)
    CFI_RESTORE(d13)
    CFI_RESTORE(d12)
    CFI_RESTORE(d11)
    CFI_RESTORE(d10)
    CFI_RESTORE(d9)
    CFI_RESTORE(d8)
.endm

.macro SAVE_CALLER_GP_REGS fp_reg, tmp_reg, paramsnum, ret_type
    add \tmp_reg, \fp_reg, #-CALLER_REG0_OFFSET+4*4
.ifeq \paramsnum
#ifdef PANDA_TARGET_ARM32_ABI_HARD
.ifnc \ret_type,INTEGER
#else
.ifc \ret_type,VOID
#endif
    stmdb \tmp_reg, {r0-r3}
.else
    stmdb \tmp_reg, {r2-r3}
.endif
.endif

.ifeq \paramsnum-1
#ifdef PANDA_TARGET_ARM32_ABI_HARD
.ifnc \ret_type,INTEGER
#else
.ifc \ret_type,VOID
#endif
    stmdb \tmp_reg, {r1-r3}
.else
    stmdb \tmp_reg, {r2-r3}
.endif
.endif

.ifeq \paramsnum-2
    stmdb \tmp_reg, {r2-r3}
.endif

.ifeq \paramsnum-3
    str r3, [\tmp_reg, #-4]
.endif
.endm

.macro RESTORE_CALLER_GP_REGS fp_reg, tmp_reg, ret_type
    add \tmp_reg, \fp_reg, #-CALLER_REG0_OFFSET+4*4
#ifdef PANDA_TARGET_ARM32_ABI_HARD
.ifnc \ret_type,INTEGER
#else
.ifc \ret_type,VOID
#endif
    ldmdb \tmp_reg, {r0-r3}
.else
    // r0-r1 is 64-bit return value
    ldmdb \tmp_reg, {r2-r3}
.endif
.endm

.macro SAVE_CALLER_FP_REGS fp_reg, tmp_reg, ret_type
    add \tmp_reg, \fp_reg, #-CALLER_VREG0_OFFSET+8*8
#ifdef PANDA_TARGET_ARM32_ABI_HARD
.ifnc \ret_type,FLOAT
    vstmdb \tmp_reg!, {d0-d7}
.else
    vstmdb \tmp_reg!, {d1-d7}
.endif
#else
    vstmdb \tmp_reg!, {d0-d7}
#endif
.endm

.macro RESTORE_CALLER_FP_REGS fp_reg, tmp_reg, ret_type
    add \tmp_reg, \fp_reg, #-CALLER_VREG0_OFFSET+8*8
#ifdef PANDA_TARGET_ARM32_ABI_HARD
.ifnc \ret_type,FLOAT
    vldmdb \tmp_reg!, {d0-d7}
.else
    vldmdb \tmp_reg!, {d1-d7}
.endif
#else
    vldmdb \tmp_reg!, {d0-d7}
#endif
.endm

.macro CHECK_NATIVE_EXCEPTION ret_type
    ldr lr, [THREAD_REG, #MANAGED_THREAD_EXCEPTION_OFFSET]
    cmp lr, #0

    beq 3f
    CFI_REMEMBER_STATE

#ifdef PANDA_TARGET_ARM32_ABI_HARD
.ifc \ret_type,INTEGER
#else
.ifnc \ret_type,VOID
#endif
    add lr, fp, #-CALLER_REG0_OFFSET
    ldm lr, {r0-r1}
.endif
#ifdef PANDA_TARGET_ARM32_ABI_HARD
.ifc \ret_type,FLOAT
    add lr, fp, #-CALLER_VREG0_OFFSET
    vldr d0, [lr]
.endif
#endif

    ldr lr, [sp, #(BRIDGE_FRAME_SIZE - 4)]
    CFI_RESTORE(lr)

    add sp, sp, #BRIDGE_FRAME_SIZE
    CFI_ADJUST_CFA_OFFSET(-BRIDGE_FRAME_SIZE)

    b ThrowNativeExceptionBridge
    CFI_RESTORE_STATE

3:
    ldr lr, [sp, #(BRIDGE_FRAME_SIZE - 4)]
    CFI_RESTORE(lr)

    add sp, sp, #BRIDGE_FRAME_SIZE
    CFI_ADJUST_CFA_OFFSET(-BRIDGE_FRAME_SIZE)
.endm

.macro BRIDGE_SELECTOR name, notcompiled_entry, compiled_entry
.global \name
.type \name, %function
\name:
    ldrb r12, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]
    tst r12, r12
    bne \compiled_entry
    b \notcompiled_entry
.endm


.macro RUNTIME_CALL_CHECKER name, entry
.global \name
.type \name, %function
\name:
      CFI_STARTPROC
      CFI_DEF_CFA(sp, 0)

      ldr r12, [THREAD_REG, #MANAGED_THREAD_RUNTIME_CALL_ENABLED_OFFSET]

      sub sp, sp, #8
      CFI_ADJUST_CFA_OFFSET(8)

      stm sp, {r12, lr}
      CFI_REL_OFFSET(lr, -4)

      mov r12, #0
      str r12, [THREAD_REG, #MANAGED_THREAD_RUNTIME_CALL_ENABLED_OFFSET]

      bl \entry

      ldm sp, {r12, lr}
      CFI_RESTORE(lr)

      add sp, sp, #8
      CFI_ADJUST_CFA_OFFSET(-8)

      str r12, [THREAD_REG, #MANAGED_THREAD_RUNTIME_CALL_ENABLED_OFFSET]

      bx lr
      CFI_ENDPROC
.endm

.macro CALL_RUNTIME mode, entry, paramsnum, ret_type
      sub sp, sp, #BRIDGE_FRAME_SIZE
      CFI_ADJUST_CFA_OFFSET(BRIDGE_FRAME_SIZE)

      // Setup BoundaryFrame
      str lr, [sp, #(BRIDGE_FRAME_SIZE - 1 * 4)]     // Bridge frame, slot 1 = npc = LR (the StackMap stays just after the bridge call)
      CFI_REL_OFFSET(lr, (BRIDGE_FRAME_SIZE - 1 * 4))
      str lr, [THREAD_REG, #MANAGED_THREAD_NATIVE_PC_OFFSET] // ManagedThread.npc update
      mov lr, #COMPILED_CODE_TO_INTERPRETER_BRIDGE
      str lr, [sp, #(BRIDGE_FRAME_SIZE - 2 * 4)]     // Bridge frame, slot 2 = COMPILED_CODE_TO_INTERPRETER_BRIDGE flag
      str fp, [sp, #(BRIDGE_FRAME_SIZE - 3 * 4)]    // Bridge frame, slot 3 = parent frame pointer
      CFI_REL_OFFSET(fp, (BRIDGE_FRAME_SIZE - 3 * 4))
      add lr, sp, #(BRIDGE_FRAME_SIZE - 3 * 4)
      str lr, [THREAD_REG, #MANAGED_THREAD_FRAME_OFFSET]    // ManagedThread._frame = this boundary frame

      SAVE_CALLEE_GP_REGS sp, lr, BRIDGE_FRAME_SIZE
#ifndef PANDA_TARGET_ARM32_ABI_SOFT
      // StackWalker requires callee-saved FP registers to be saved unconditionally in the runtime bridge
      SAVE_CALLEE_FP_REGS sp, lr, BRIDGE_FRAME_SIZE
#endif

.if \mode != RUNTIME_MODE_SLOW_PATH
      SAVE_CALLER_GP_REGS fp, lr, \paramsnum, \ret_type
.endif

#ifndef PANDA_TARGET_ARM32_ABI_SOFT
      ldr r4, [fp, #(-CFRAME_FLAGS_SLOT * 4)]
      tst r4, #CFRAME_HAS_FLOAT_REGS_FLAG_MASK
      beq 1f

      SAVE_CALLER_FP_REGS fp, lr, \ret_type
1:
#endif

      bl \entry

      // State restore
      str fp, [THREAD_REG, #MANAGED_THREAD_FRAME_OFFSET]

#ifndef PANDA_TARGET_ARM32_ABI_SOFT
      tst r4, #CFRAME_HAS_FLOAT_REGS_FLAG_MASK
      beq 2f

      RESTORE_CALLEE_FP_REGS sp, lr, BRIDGE_FRAME_SIZE
      RESTORE_CALLER_FP_REGS fp, lr, \ret_type
2:
#endif

      RESTORE_CALLEE_GP_REGS sp, lr, BRIDGE_FRAME_SIZE
      RESTORE_CALLER_GP_REGS fp, lr, \ret_type

      CHECK_NATIVE_EXCEPTION \ret_type
.endm

.macro ENTRYPOINT name, entry, paramsnum, ret_type
.global \name
.type \name, %function
\name:
      CFI_STARTPROC
      CFI_DEF_CFA(sp, 0)

      CALL_RUNTIME RUNTIME_MODE_DEFAULT, \entry, \paramsnum, \ret_type
      bx lr
      CFI_ENDPROC
.endm

// Unused for 32-bit arm
.macro ENTRYPOINT_ODD_SAVED name, entry, paramsnum, ret_type
      ENTRYPOINT \name, \entry, \paramsnum, \ret_type
.endm

.macro ENTRYPOINT_SLOW_PATH name, entry, paramsnum, ret_type
.global \name
.type \name, %function
\name:
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    CALL_RUNTIME RUNTIME_MODE_SLOW_PATH, \entry, \paramsnum, \ret_type
    bx lr
    CFI_ENDPROC
.endm

.macro MethodEntrypointStub name, entry, notcompiled
.global \name
.type \name, %function
\name:
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)

    // If the caller is not a compiled method, we need to call \entry
    // and return back after its execution
    ldrb r1, [THREAD_REG, #MANAGED_THREAD_FRAME_KIND_OFFSET]
    tst r1, r1
    beq .L\notcompiled
    CFI_REMEMBER_STATE
  
    str lr, [sp, #-4]
    CFI_REL_OFFSET(lr, -4)
    str lr, [THREAD_REG, #MANAGED_THREAD_NATIVE_PC_OFFSET]
    mov lr, #COMPILED_CODE_TO_INTERPRETER_BRIDGE
    str lr, [sp, #-8]
    str fp, [sp, #-12]
    CFI_REL_OFFSET(fp, -12)
    sub lr, sp, #12
    str lr, [THREAD_REG, #MANAGED_THREAD_FRAME_OFFSET]
    sub sp, sp, #16
    CFI_ADJUST_CFA_OFFSET(16) 

    push {r4 - r10}
    CFI_ADJUST_CFA_OFFSET(28)
    CFI_REL_OFFSET(r10, (6 * 4))
    CFI_REL_OFFSET(r9,  (5 * 4))
    CFI_REL_OFFSET(r8,  (4 * 4))
    CFI_REL_OFFSET(r7,  (3 * 4))
    CFI_REL_OFFSET(r6,  (2 * 4))
    CFI_REL_OFFSET(r5,  (1 * 4))
    CFI_REL_OFFSET(r4,  (0 * 4))

#ifndef PANDA_TARGET_ARM32_ABI_SOFT
    vpush {d8 - d15}
    CFI_ADJUST_CFA_OFFSET(64)
    CFI_REL_OFFSET(d15, (14 * 4))
    CFI_REL_OFFSET(d14, (12 * 4))
    CFI_REL_OFFSET(d13, (10 * 4))
    CFI_REL_OFFSET(d12, (8 * 4))
    CFI_REL_OFFSET(d11, (6 * 4))
    CFI_REL_OFFSET(d10, (4 * 4))
    CFI_REL_OFFSET(d9,  (2 * 4))
    CFI_REL_OFFSET(d8,  (0 * 4))
#endif

    // align to 8
    sub sp, sp, #4

    bl \entry
    // we're not going to return back here

.L\notcompiled:
    CFI_RESTORE_STATE
    CFI_DEF_CFA(sp, 0)
    str lr, [sp, #-4]
    CFI_REL_OFFSET(lr, -(1 * 4))
    sub sp, sp, #8
    CFI_ADJUST_CFA_OFFSET(2 * 4)
    bl \entry
    add sp, sp, #8
    CFI_ADJUST_CFA_OFFSET(-(2 * 4))
    ldr lr, [sp, #-4]
    CFI_RESTORE(lr)
    bx lr
    CFI_ENDPROC
.endm

#include "entrypoints_gen.S"
#include "entrypoints_bridge_asm_macro.inl"

MethodEntrypointStub AbstractMethodStub AbstractMethodErrorEntrypoint ame_not_compiled

MethodEntrypointStub DefaultConflictMethodStub IncompatibleClassChangeErrorForMethodConflictEntrypoint icce_not_compiled
