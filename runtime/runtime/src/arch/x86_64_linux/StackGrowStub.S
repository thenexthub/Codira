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

#define cfi_rel_offset(reg, off)	.cfi_rel_offset reg, off
#define cfi_restore(reg)		.cfi_restore reg
#define cfi_def_cfa_register(reg)	.cfi_def_cfa_register reg

#define StackGrowStubSize           (0x264)

// This signal handler stub relies on MRT_PrepareToHandleSignal to put continuation pc in x17.
// Continuation pc describes the control flow after signal handling completes.

    .text
    .align 2
    .global CODE_MCC_StackGrowStub
    .type CODE_MCC_StackGrowStub, %function
CODE_MCC_StackGrowStub:
    .cfi_startproc
    pushq   %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq    %rsp, %rbp
    .cfi_def_cfa_register %rbp
    pushq  %rax
    pushq  %rbx
    pushq  %rcx
    pushq  %rdx
    pushq  %rdi
    pushq  %rsi
    pushq  %rsp
    pushq  %r8
    pushq  %r9
    pushq  %r10
    pushq  %r11
    pushq  %r12
    pushq  %r13
    pushq  %r14
    pushq  %r15
    cfi_rel_offset (%rax, -8)
    cfi_rel_offset (%rbx, -16)
    cfi_rel_offset (%rcx, -24)
    cfi_rel_offset (%rdx, -32)
    cfi_rel_offset (%rdi, -40)
    cfi_rel_offset (%rsi, -48)
    cfi_rel_offset (%rsp, -56)
    cfi_rel_offset (%r8, -64)
    cfi_rel_offset (%r9, -72)
    cfi_rel_offset (%r10, -80)
    cfi_rel_offset (%r11, -88)
    cfi_rel_offset (%r12, -96)
    cfi_rel_offset (%r13, -104)
    cfi_rel_offset (%r14, -112)
    cfi_rel_offset (%r15, -120)

    // Need to ensure 16-bit alignment
    subq   $264, %rsp

    movapd %xmm0, 0(%rsp)
    movapd %xmm1, 16(%rsp)
    movapd %xmm2, 32(%rsp)
    movapd %xmm3, 48(%rsp)
    movapd %xmm4, 64(%rsp)
    movapd %xmm5, 80(%rsp)
    movapd %xmm6, 96(%rsp)
    movapd %xmm7, 112(%rsp)
    movapd %xmm8, 128(%rsp)
    movapd %xmm9, 144(%rsp)
    movapd %xmm10, 160(%rsp)
    movapd %xmm11, 176(%rsp)
    movapd %xmm12, 192(%rsp)
    movapd %xmm13, 208(%rsp)
    movapd %xmm14, 224(%rsp)
    movapd %xmm15, 240(%rsp)

    movq   %rbp,     %rsi
    movq   $1,       %rdx
    movq   %r15,     %rcx
    leaq   21(%rip), %rdi // rdi is addr of unwindPCForStackGrowStub
    callq  MRT_UpdateUwContext@PLT

    // The input rbp after stack expansion is used to determine 
    // whether the current stack expansion meets requirements.
    movq   %rbp,     %rdi
    movq   -8(%rbp), %rsi
    movq   8(%rbp),%rdx
    callq  MRT_StackGrow@PLT
    .global unwindPCForStackGrowStub
unwindPCForStackGrowStub:

    addq %rax, %rsp
    addq %rax, %rbp

    movq   %rax, %rdi
    callq  MRT_FreeOldStack@PLT

    callq  MRT_GetThreadLocalData@PLT
    movq   %rax, %r15
    movq   %rax, %rdi
    callq  MRT_DeleteC2NContext@PLT

    // restore callee registers
    movapd 0(%rsp), %xmm0
    movapd 16(%rsp), %xmm1
    movapd 32(%rsp), %xmm2
    movapd 48(%rsp), %xmm3
    movapd 64(%rsp), %xmm4
    movapd 80(%rsp), %xmm5
    movapd 96(%rsp), %xmm6
    movapd 112(%rsp), %xmm7
    movapd 128(%rsp), %xmm8
    movapd 144(%rsp), %xmm9
    movapd 160(%rsp), %xmm10
    movapd 176(%rsp), %xmm11
    movapd 192(%rsp), %xmm12
    movapd 208(%rsp), %xmm13
    movapd 224(%rsp), %xmm14
    movapd 240(%rsp), %xmm15

    // Need to ensure 16-bit alignment
    addq  $264, %rsp

    popq  %rax
    popq  %r14
    popq  %r13
    popq  %r12
    popq  %r11
    popq  %r10
    popq  %r9
    popq  %r8
    popq  %rsi
    popq  %rsi
    popq  %rdi
    popq  %rdx
    popq  %rcx
    popq  %rbx
    popq  %rax
    popq  %rbp
    cfi_restore (%rax)
    cfi_restore (%r14)
    cfi_restore (%r13)
    cfi_restore (%r12)
    cfi_restore (%r11)
    cfi_restore (%r10)
    cfi_restore (%r9)
    cfi_restore (%r8)
    cfi_restore (%rsi)
    cfi_restore (%rsi)
    cfi_restore (%rdi)
    cfi_restore (%rdx)
    cfi_restore (%rcx)
    cfi_restore (%rbx)
    cfi_restore (%rax)
    cfi_restore (%rbp)
    .cfi_def_cfa %rsp, 8

    retq

    .cfi_endproc
    .size CODE_MCC_StackGrowStub, .-CODE_MCC_StackGrowStub
