/*
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

#include "runtime/fibers/arch/asm_macros.h"
#include "runtime/fibers/arch/amd64/context_layout.h"

#define GPR_O(reg) FCTX_GPR_OFFSET_BYTES_ ## reg
#define FP_O(reg) FCTX_FP_OFFSET_BYTES_ ## reg
#include "runtime/fibers/arch/amd64/helpers.S"

/**
 * arguments:
 * rdi: uint8_t* ctx_memory_from
 * rsi: uint8_t* ctx_memory_to
 */
FUNCTION_START(SwitchContext)
    CFI_STARTPROC
    /*** save current context ***/
    STORE_CONTEXT ctx_baseaddr_reg=%rdi, tmp_reg=%rcx

    /*** restore next context ***/
    // fp: env
    leaq    FP_O(FPENV)(%rsi), %rcx
    // NB: unmasks FP exceptions
    fldenv  (%rcx)
    // fp: csr
    ldmxcsr FP_O(MXCSR)(%rsi)

    // gp: callee-saved
    CFI_DEF_CFA(%rsi, 0)
    CFI_OFFSET(%rbx, GPR_O(RBX))
    movq    GPR_O(RBX)(%rsi), %rbx
    CFI_OFFSET(%rbp, GPR_O(RBP))
    movq    GPR_O(RBP)(%rsi), %rbp
    CFI_OFFSET(%r12, GPR_O(R12))
    movq    GPR_O(R12)(%rsi), %r12
    CFI_OFFSET(%r13, GPR_O(R13))
    movq    GPR_O(R13)(%rsi), %r13
    CFI_OFFSET(%r14, GPR_O(R14))
    movq    GPR_O(R14)(%rsi), %r14
    CFI_OFFSET(%r15, GPR_O(R15))
    movq    GPR_O(R15)(%rsi), %r15
    // gp: the arg register
    CFI_OFFSET(%rdi, GPR_O(RDI))
    movq    GPR_O(RDI)(%rsi), %rdi
    // gp: stack pointer
    CFI_OFFSET(%rsp, GPR_O(RSP))
    movq    GPR_O(RSP)(%rsi), %rsp
    // push next_context->RIP to restore it on return from SwitchContext
    CFI_OFFSET(%rip, GPR_O(RIP))
    movq    GPR_O(RIP)(%rsi), %rcx
    pushq   %rcx

    CFI_ENDPROC
    CFI_STARTPROC
    xorq    %rax, %rax
    ret
    CFI_ENDPROC
FUNCTION_END(SwitchContext)

#undef GPR_O
#undef FP_O

// we don't need executable stack.
.section .note.GNU-stack,"",%progbits