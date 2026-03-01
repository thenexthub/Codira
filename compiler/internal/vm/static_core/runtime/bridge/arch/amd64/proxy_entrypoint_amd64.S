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

#include "arch/asm_support.h"
#include "arch/amd64/helpers_amd64.S"
#include "arch/amd64/shorty.S"
#include "shorty_values.h"

#define C2I_FP_OFFSET 2

.macro PROXY_ENTRYPOINT name, entry
.global \name
#ifdef PANDA_WITH_HIDDEN_SYMBOLS
.hidden \name
.hidden \entry
#else
.protected \name
.protected \entry
#endif
.type \name, %function
\name:
    CFI_STARTPROC
    CFI_DEF_CFA(rsp, 8)

    // %rsp % 16 == 8
    pushq $COMPILED_CODE_TO_INTERPRETER_BRIDGE
    CFI_ADJUST_CFA_OFFSET(8)

    pushq %rbp
    CFI_ADJUST_CFA_OFFSET(8)
    CFI_REL_OFFSET(rbp, 0)
    movq %rsp, %rbp // frame pointer
    CFI_DEF_CFA_REGISTER(rbp)

    subq $8, %rsp

    // save all the callee saved registers to the stack
    // stack walker will read them during stack unwinding
    pushq %r15
    CFI_REL_OFFSET(r15, -(2 * 8))
    pushq %r14
    CFI_REL_OFFSET(r14, -(3 * 8))
    pushq %r13
    CFI_REL_OFFSET(r13, -(4 * 8))
    pushq %r12
    CFI_REL_OFFSET(r12, -(5 * 8))
    pushq %rbx
    CFI_REL_OFFSET(rbx, -(6 * 8))

    subq $8, %rsp
    // %rsp % 16 == 0

    // If caller is the compiled method, we need to set C2I boundary frame in the TLS
    movb MANAGED_THREAD_FRAME_KIND_OFFSET(%THREAD_REG), %r12b
    testb %r12b, %r12b
    jz 0f

    movq %rbp, MANAGED_THREAD_FRAME_OFFSET(%THREAD_REG)

0:
    // save arguments to the stack
    PUSH_FP_REGS

    PUSH_GENERAL_REGS
    // %rsp % 16 == 0

    // Since PUSH_GENERAL_REGS has been called, then rsi will points to array of pushed arguments.
    // rsi is the second argument of calling function.
    movq %rsp, %rsi
    // rbp + 24 points to the stack arguments that has been pushed to the stack by i2c bridge before calling proxy bridge.
    // rdx is the third argument of calling function.
    leaq 24(%rbp), %rdx
    callq \entry

    popq %rdi

    // handle the result
    movq METHOD_SHORTY_OFFSET(%rdi), %rcx
    movl (%rcx), %ecx
    andl $0xF, %ecx
    cmpl $SHORTY_VOID, %ecx
    jne 1f

    // void method
    jmp 3f

1:  subl $SHORTY_FIRST_FLOAT, %ecx
    cmpl $(SHORTY_NUM_FLOAT_TYPES - 1), %ecx
    jbe 2f

    // the return value is integer
    jmp 3f

2:
    // it is a float or double
    movq %rax, %xmm0

3:
    movb %r12b, %r10b

    // Restore callee registers, since GC may change its values while moving objects.
    movq -((CFRAME_HEADER_SIZE - C2I_FP_OFFSET + 0) * 8)(%rbp), %r15
    CFI_RESTORE(r15)
    movq -((CFRAME_HEADER_SIZE - C2I_FP_OFFSET + 1) * 8)(%rbp), %r14
    CFI_RESTORE(r14)
    movq -((CFRAME_HEADER_SIZE - C2I_FP_OFFSET + 2) * 8)(%rbp), %r13
    CFI_RESTORE(r13)
    movq -((CFRAME_HEADER_SIZE - C2I_FP_OFFSET + 3) * 8)(%rbp), %r12
    CFI_RESTORE(r12)
    movq -((CFRAME_HEADER_SIZE - C2I_FP_OFFSET + 4) * 8)(%rbp), %rbx
    CFI_RESTORE(rbx)

    movq %rbp, %rsp
    popq %rbp
    CFI_RESTORE(rbp)
    CFI_DEF_CFA(rsp, (2 * 8))
    addq $8, %rsp
    CFI_ADJUST_CFA_OFFSET(-(1 * 8))

    // check native exception
    movq MANAGED_THREAD_EXCEPTION_OFFSET(%THREAD_REG), %r11
    testq %r11, %r11
    jz 4f

    // check frame is compiled
    testb %r10b, %r10b
    jz 4f

    // check prev frame is true CFRAME and not BYPASS
    cmpq $BYPASS_BRIDGE, (SLOT_SIZE * COMP_METHOD_OFFSET)(%rbp)
    je 4f

    movq (-CALLER_REG0_OFFSET + 0)(%rbp), %rax
    movq (-CALLER_REG0_OFFSET + 8)(%rbp), %rcx
    movq (-CALLER_REG0_OFFSET + 16)(%rbp), %rdx
    movq (-CALLER_REG0_OFFSET + 24)(%rbp), %r11
    movq (-CALLER_REG0_OFFSET + 32)(%rbp), %r10
    movq (-CALLER_REG0_OFFSET + 40)(%rbp), %r9
    movq (-CALLER_REG0_OFFSET + 48)(%rbp), %rsi
    movq (-CALLER_REG0_OFFSET + 56)(%rbp), %rdi
    movq (-CALLER_REG0_OFFSET + 64)(%rbp), %r8

    testq $CFRAME_HAS_FLOAT_REGS_FLAG_MASK, (-CFRAME_FLAGS_SLOT * SLOT_SIZE)(%rbp)
    jz 1f

    movsd (-CALLER_VREG0_OFFSET + 0)(%rbp), %xmm0
    movsd (-CALLER_VREG0_OFFSET + 8)(%rbp), %xmm1
    movsd (-CALLER_VREG0_OFFSET + 16)(%rbp), %xmm2
    movsd (-CALLER_VREG0_OFFSET + 24)(%rbp), %xmm3
    movsd (-CALLER_VREG0_OFFSET + 32)(%rbp), %xmm4
    movsd (-CALLER_VREG0_OFFSET + 40)(%rbp), %xmm5
    movsd (-CALLER_VREG0_OFFSET + 48)(%rbp), %xmm6
    movsd (-CALLER_VREG0_OFFSET + 56)(%rbp), %xmm7
    movsd (-CALLER_VREG0_OFFSET + 64)(%rbp), %xmm8
    movsd (-CALLER_VREG0_OFFSET + 72)(%rbp), %xmm9
    movsd (-CALLER_VREG0_OFFSET + 80)(%rbp), %xmm10
    movsd (-CALLER_VREG0_OFFSET + 88)(%rbp), %xmm11
    movsd (-CALLER_VREG0_OFFSET + 96)(%rbp), %xmm12
    movsd (-CALLER_VREG0_OFFSET + 104)(%rbp), %xmm13
    movsd (-CALLER_VREG0_OFFSET + 112)(%rbp), %xmm14
    movsd (-CALLER_VREG0_OFFSET + 120)(%rbp), %xmm15

1:
    jmp ThrowNativeExceptionBridge

4:
    retq
    CFI_ENDPROC
.endm
