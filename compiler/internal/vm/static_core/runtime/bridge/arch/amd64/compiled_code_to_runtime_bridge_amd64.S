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
#include "arch/amd64/helpers_amd64.S"

.macro SAVE_CALLEE_GP_REGS base_reg, offset
    movq %r15, (\offset - CALLEE_REG0_OFFSET + 8*4)(%\base_reg)
    CFI_REL_OFFSET(r15, (\offset - CALLEE_REG0_OFFSET+8*4))
    movq %r14, (\offset - CALLEE_REG0_OFFSET + 8*3)(%\base_reg)
    CFI_REL_OFFSET(r14, (\offset - CALLEE_REG0_OFFSET+8*3))
    movq %r13, (\offset - CALLEE_REG0_OFFSET + 8*2)(%\base_reg)
    CFI_REL_OFFSET(r13, (\offset - CALLEE_REG0_OFFSET+8*2))
    movq %r12, (\offset - CALLEE_REG0_OFFSET + 8*1)(%\base_reg)
    CFI_REL_OFFSET(r12, (\offset - CALLEE_REG0_OFFSET+8*1))
    movq %rbx, (\offset - CALLEE_REG0_OFFSET + 8*0)(%\base_reg)
    CFI_REL_OFFSET(rbx, (\offset - CALLEE_REG0_OFFSET+8*0))
.endm

.macro RESTORE_CALLEE_GP_REGS base_reg, offset
    movq (\offset - CALLEE_REG0_OFFSET + 8*4)(%\base_reg), %r15
    CFI_RESTORE(r15)
    movq (\offset - CALLEE_REG0_OFFSET + 8*3)(%\base_reg), %r14
    CFI_RESTORE(r14)
    movq (\offset - CALLEE_REG0_OFFSET + 8*2)(%\base_reg), %r13
    CFI_RESTORE(r13)
    movq (\offset - CALLEE_REG0_OFFSET + 8*1)(%\base_reg), %r12
    CFI_RESTORE(r12)
    movq (\offset - CALLEE_REG0_OFFSET + 8*0)(%\base_reg), %rbx
    CFI_RESTORE(rbx)
.endm

.macro SAVE_CALLER_GP_REGS fp_reg, paramsnum, ret_type
.ifnc \ret_type,INTEGER
    movq %rax, (-CALLER_REG0_OFFSET + 0)(%\fp_reg)
.endif
.ifle \paramsnum-3
    movq %rcx, (-CALLER_REG0_OFFSET + 8)(%\fp_reg)
.endif
.ifle \paramsnum-2
    movq %rdx, (-CALLER_REG0_OFFSET + 16)(%\fp_reg)
.endif
    movq %r11, (-CALLER_REG0_OFFSET + 24)(%\fp_reg)
    movq %r10, (-CALLER_REG0_OFFSET + 32)(%\fp_reg)
.ifle \paramsnum-5
    movq %r9, (-CALLER_REG0_OFFSET + 40)(%\fp_reg)
.endif
.ifle \paramsnum-1
    movq %rsi, (-CALLER_REG0_OFFSET + 48)(%\fp_reg)
.endif
.ifeq \paramsnum
    movq %rdi, (-CALLER_REG0_OFFSET + 56)(%\fp_reg)
.endif
.ifle \paramsnum-4
    movq %r8, (-CALLER_REG0_OFFSET + 64)(%\fp_reg)
.endif
.endm

.macro RESTORE_CALLER_GP_REGS fp_reg, ret_type
.ifnc \ret_type,INTEGER
    movq (-CALLER_REG0_OFFSET + 0)(%\fp_reg), %rax
.endif
    movq (-CALLER_REG0_OFFSET + 8)(%\fp_reg), %rcx
    movq (-CALLER_REG0_OFFSET + 16)(%\fp_reg), %rdx
    movq (-CALLER_REG0_OFFSET + 24)(%\fp_reg), %r11
    movq (-CALLER_REG0_OFFSET + 32)(%\fp_reg), %r10
    movq (-CALLER_REG0_OFFSET + 40)(%\fp_reg), %r9
    movq (-CALLER_REG0_OFFSET + 48)(%\fp_reg), %rsi
    movq (-CALLER_REG0_OFFSET + 56)(%\fp_reg), %rdi
    movq (-CALLER_REG0_OFFSET + 64)(%\fp_reg), %r8
.endm

.macro SAVE_CALLER_FP_REGS fp_reg, ret_type
.ifnc \ret_type,FLOAT
    movsd %xmm0, (-CALLER_VREG0_OFFSET + 0)(%\fp_reg)
.endif
    movsd %xmm1, (-CALLER_VREG0_OFFSET + 8)(%\fp_reg)
    movsd %xmm2, (-CALLER_VREG0_OFFSET + 16)(%\fp_reg)
    movsd %xmm3, (-CALLER_VREG0_OFFSET + 24)(%\fp_reg)
    movsd %xmm4, (-CALLER_VREG0_OFFSET + 32)(%\fp_reg)
    movsd %xmm5, (-CALLER_VREG0_OFFSET + 40)(%\fp_reg)
    movsd %xmm6, (-CALLER_VREG0_OFFSET + 48)(%\fp_reg)
    movsd %xmm7, (-CALLER_VREG0_OFFSET + 56)(%\fp_reg)
    movsd %xmm8, (-CALLER_VREG0_OFFSET + 64)(%\fp_reg)
    movsd %xmm9, (-CALLER_VREG0_OFFSET + 72)(%\fp_reg)
    movsd %xmm10, (-CALLER_VREG0_OFFSET + 80)(%\fp_reg)
    movsd %xmm11, (-CALLER_VREG0_OFFSET + 88)(%\fp_reg)
    movsd %xmm12, (-CALLER_VREG0_OFFSET + 96)(%\fp_reg)
    movsd %xmm13, (-CALLER_VREG0_OFFSET + 104)(%\fp_reg)
    movsd %xmm14, (-CALLER_VREG0_OFFSET + 112)(%\fp_reg)
    movsd %xmm15, (-CALLER_VREG0_OFFSET + 120)(%\fp_reg)
.endm

.macro RESTORE_CALLER_FP_REGS fp_reg, ret_type
.ifnc \ret_type,FLOAT
    movsd (-CALLER_VREG0_OFFSET + 0)(%\fp_reg), %xmm0
.endif
    movsd (-CALLER_VREG0_OFFSET + 8)(%\fp_reg), %xmm1
    movsd (-CALLER_VREG0_OFFSET + 16)(%\fp_reg), %xmm2
    movsd (-CALLER_VREG0_OFFSET + 24)(%\fp_reg), %xmm3
    movsd (-CALLER_VREG0_OFFSET + 32)(%\fp_reg), %xmm4
    movsd (-CALLER_VREG0_OFFSET + 40)(%\fp_reg), %xmm5
    movsd (-CALLER_VREG0_OFFSET + 48)(%\fp_reg), %xmm6
    movsd (-CALLER_VREG0_OFFSET + 56)(%\fp_reg), %xmm7
    movsd (-CALLER_VREG0_OFFSET + 64)(%\fp_reg), %xmm8
    movsd (-CALLER_VREG0_OFFSET + 72)(%\fp_reg), %xmm9
    movsd (-CALLER_VREG0_OFFSET + 80)(%\fp_reg), %xmm10
    movsd (-CALLER_VREG0_OFFSET + 88)(%\fp_reg), %xmm11
    movsd (-CALLER_VREG0_OFFSET + 96)(%\fp_reg), %xmm12
    movsd (-CALLER_VREG0_OFFSET + 104)(%\fp_reg), %xmm13
    movsd (-CALLER_VREG0_OFFSET + 112)(%\fp_reg), %xmm14
    movsd (-CALLER_VREG0_OFFSET + 120)(%\fp_reg), %xmm15
.endm

.macro BRIDGE_SELECTOR name, notcompiled_entry, compiled_entry
.global \name
#ifdef PANDA_WITH_HIDDEN_SYMBOLS
.hidden \name
.hidden \notcompiled_entry
.hidden \compiled_entry
#else
.protected \name
.protected \notcompiled_entry
.protected \compiled_entry
#endif
TYPE_FUNCTION(\name)
\name:
    movb MANAGED_THREAD_FRAME_KIND_OFFSET(%THREAD_REG), %r10b
    testb %r10b, %r10b
    jnz \compiled_entry
    jmp \notcompiled_entry
.endm

.macro RUNTIME_CALL_CHECKER name, entry
.global \name
#ifdef PANDA_WITH_HIDDEN_SYMBOLS
.hidden \name
.hidden \entry
#else
.protected \name
.protected \entry
#endif
TYPE_FUNCTION(\name)
\name:
    CFI_STARTPROC
    CFI_DEF_CFA(rsp, 8)

    pushq %r12
    CFI_ADJUST_CFA_OFFSET(8)
    CFI_REL_OFFSET(r12, 0)

    movq MANAGED_THREAD_RUNTIME_CALL_ENABLED_OFFSET(%THREAD_REG), %r12

    movq $0, MANAGED_THREAD_RUNTIME_CALL_ENABLED_OFFSET(%THREAD_REG)

    call \entry

    movq %r12, MANAGED_THREAD_RUNTIME_CALL_ENABLED_OFFSET(%THREAD_REG)
    popq %r12
    CFI_ADJUST_CFA_OFFSET(-8)
    CFI_RESTORE(r12)

    // return to the caller
    retq
    CFI_ENDPROC
.endm

.macro CALL_RUNTIME mode, entry, paramsnum, ret_type
    subq $(BRIDGE_FRAME_SIZE - 8), %rsp
    CFI_ADJUST_CFA_OFFSET((BRIDGE_FRAME_SIZE - 8))

    SAVE_CALLEE_GP_REGS rsp, BRIDGE_FRAME_SIZE

    // Bridge frame:
    //   [1] native_pc = retaddr
    //   [2] parent frame pointer
    //   [3] COMPILED_CODE_TO_INTERPRETER_BRIDGE flag

    // Bridge frame, slot 1 = npc = retaddr (StackMap stays just after the bridge call)
    mov (BRIDGE_FRAME_SIZE - 1 * 8)(%rsp), %r14
    // ManagedThread.npc update
    mov %r14, MANAGED_THREAD_NATIVE_PC_OFFSET(%THREAD_REG)

    // Bridge frame, slot 2 = COMPILED_CODE_TO_INTERPRETER_BRIDGE flag
    movq $COMPILED_CODE_TO_INTERPRETER_BRIDGE, (BRIDGE_FRAME_SIZE - 2 * 8)(%rsp)
    // Bridge frame, slot 3 = parent frame pointer
    mov %rbp, (BRIDGE_FRAME_SIZE - 3 * 8)(%rsp)
    CFI_REL_OFFSET(rbp, (BRIDGE_FRAME_SIZE - 3 * 8))

    leaq (BRIDGE_FRAME_SIZE - 3 * 8)(%rsp), %r13
    // ManagedThread._frame = this boundary frame
    mov %r13, MANAGED_THREAD_FRAME_OFFSET(%THREAD_REG)

.if \mode != RUNTIME_MODE_SLOW_PATH
    SAVE_CALLER_GP_REGS rbp, \paramsnum, \ret_type
.endif

    movq (-CFRAME_FLAGS_SLOT * 8)(%rbp), %r12
    testq $CFRAME_HAS_FLOAT_REGS_FLAG_MASK, %r12
    jz 1f

    SAVE_CALLER_FP_REGS rbp, \ret_type

1:
    // call to BoundaryFrame bridge
    call \entry

    // ManagedThread._frame = parent frame pointer
    movq %rbp, MANAGED_THREAD_FRAME_OFFSET(%THREAD_REG)

    RESTORE_CALLER_GP_REGS rbp, \ret_type

    testq $CFRAME_HAS_FLOAT_REGS_FLAG_MASK, %r12
    jz 2f

    RESTORE_CALLER_FP_REGS rbp, \ret_type

2:
    RESTORE_CALLEE_GP_REGS rsp, BRIDGE_FRAME_SIZE

    cmpq $0, MANAGED_THREAD_EXCEPTION_OFFSET(%THREAD_REG)
    jz 3f
    CFI_REMEMBER_STATE

.ifc \ret_type,INTEGER
    movq (-CALLER_REG0_OFFSET + 0)(%rbp), %rax
.endif
.ifc \ret_type,FLOAT
    movsd (-CALLER_VREG0_OFFSET + 0)(%rbp), %xmm0
.endif
    addq $(BRIDGE_FRAME_SIZE - 8), %rsp
    CFI_ADJUST_CFA_OFFSET(-(BRIDGE_FRAME_SIZE - 8))
    jmp ThrowNativeExceptionBridge
    CFI_RESTORE_STATE

3:
    addq $(BRIDGE_FRAME_SIZE - 8), %rsp
    CFI_ADJUST_CFA_OFFSET(-(BRIDGE_FRAME_SIZE - 8))
    // return to the caller
.endm

.macro ENTRYPOINT name, entry, paramsnum, ret_type
.global \name
#ifdef PANDA_WITH_HIDDEN_SYMBOLS
.hidden \name
.hidden \entry
#else
.protected \name
.protected \entry
#endif
TYPE_FUNCTION(\name)
\name:
    CFI_STARTPROC
    CFI_DEF_CFA(rsp, 8)

    CALL_RUNTIME RUNTIME_MODE_DEFAULT, \entry, \paramsnum, \ret_type
    retq
    CFI_ENDPROC
.endm

// Unused for x86_64
.macro ENTRYPOINT_ODD_SAVED name, entry, paramsnum, ret_type
      ENTRYPOINT \name, \entry, \paramsnum, \ret_type
.endm

.macro ENTRYPOINT_SLOW_PATH name, entry, paramsnum, ret_type
.global \name
#ifdef PANDA_WITH_HIDDEN_SYMBOLS
.hidden \name
.hidden \entry
#else
.protected \name
.protected \entry
#endif
TYPE_FUNCTION(\name)
\name:
    CFI_STARTPROC
    CFI_DEF_CFA(rsp, 8)

    CALL_RUNTIME RUNTIME_MODE_SLOW_PATH, \entry, \paramsnum, \ret_type
    retq
    CFI_ENDPROC
.endm

.macro MethodEntrypointStub name, entry, notcompiled
.global \name
#ifdef PANDA_WITH_HIDDEN_SYMBOLS
.hidden \name
.hidden \entry
#else
.protected \name
.protected \entry
#endif
TYPE_FUNCTION(\name)
\name:
    CFI_STARTPROC
    CFI_DEF_CFA(rsp, 8)

    // If the caller is not a compiled method, we need to call \entry
    // and return back after its execution
    movb MANAGED_THREAD_FRAME_KIND_OFFSET(%THREAD_REG), %r9b
    testb %r9b, %r9b
    jz .L\notcompiled
    CFI_REMEMBER_STATE

    movq (%rsp), %rax
    movq  %rax, MANAGED_THREAD_NATIVE_PC_OFFSET(%THREAD_REG)
    movq  %rbp,  -0x10(%rsp)
    CFI_REL_OFFSET(rbp, -(2 * 8))
    movq $COMPILED_CODE_TO_INTERPRETER_BRIDGE, -0x8(%rsp)
    lea -0x10(%rsp), %rax
    movq %rax, MANAGED_THREAD_FRAME_OFFSET(%THREAD_REG)

    movq %r15, -0x20(%rsp)
    CFI_REL_OFFSET(r15, -(4 * 8))
    movq %r14, -0x28(%rsp)
    CFI_REL_OFFSET(r14, -(5 * 8))
    movq %r13, -0x30(%rsp)
    CFI_REL_OFFSET(r13, -(6 * 8))
    movq %r12, -0x38(%rsp)
    CFI_REL_OFFSET(r12, -(7 * 8))
    movq %rbx, -0x40(%rsp)
    CFI_REL_OFFSET(rbx, -(8 * 8))

    // ------------- header
    // %rsp        : ret addr
    // %rsp -0x08  : $COMPILED_CODE_TO_INTERPRETER_BRIDGE
    // %rsp -0x10  : frame pointer
    // %rsp -0x18  : UNUSED
    // ------------- callee-saved regs
    // %rsp -0x20  : %r15
    // %rsp -0x28  : %r14
    // %rsp -0x30  : %r13
    // %rsp -0x38  : %r12
    // %rsp -0x40  : %rbx
    // %rsp -0x48  : empty slot for alignment
    subq $0x48, %rsp
    CFI_ADJUST_CFA_OFFSET(9 * 8)

    call \entry
    // we're not going to return back here

.L\notcompiled:
    CFI_RESTORE_STATE
    CFI_DEF_CFA(rsp, 8)
    subq $0x8, %rsp
    CFI_ADJUST_CFA_OFFSET(8)
    call \entry
    addq $0x8, %rsp
    CFI_ADJUST_CFA_OFFSET(-8)
    ret
    CFI_ENDPROC
.endm

#include "entrypoints_gen.S"
#include "entrypoints_bridge_asm_macro.inl"

MethodEntrypointStub AbstractMethodStub AbstractMethodErrorEntrypoint ame_not_compiled

MethodEntrypointStub DefaultConflictMethodStub IncompatibleClassChangeErrorForMethodConflictEntrypoint icce_not_compiled
