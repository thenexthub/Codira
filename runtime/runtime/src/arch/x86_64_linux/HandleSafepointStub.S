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

// stub need to be 16-byte aligned, so we should use (8 * even numbers)
#define HandleSafepointStubFrameSize           (264)

#define cfi_adjust_cfa_offset(off)	.cfi_adjust_cfa_offset off
#define cfi_rel_offset(reg, off)	.cfi_rel_offset reg, off
#define cfi_restore(reg)		.cfi_restore reg
#define cfi_def_cfa_register(reg)	.cfi_def_cfa_register reg

    .text
    .align 2
    .global CODE_MCC_HandleSafepoint
    .type CODE_MCC_HandleSafepoint, %function
CODE_MCC_HandleSafepoint:
    .cfi_startproc

    // save callee registers
    pushq  %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq   %rsp, %rbp
    .cfi_def_cfa_register %rbp

    pushq  %rax
    cfi_rel_offset (%rax, -8)
    pushq  %rbx
    cfi_rel_offset (%rbx, -16)
    pushq  %rcx
    cfi_rel_offset (%rcx, -24)
    pushq  %rdx
    cfi_rel_offset (%rdx, -32)
    pushq  %rdi
    cfi_rel_offset (%rdi, -40)
    pushq  %rsi
    cfi_rel_offset (%rsi, -48)
    pushq  %rsp
    cfi_rel_offset (%rsp, -56)
    pushq  %r8
    cfi_rel_offset (%r8, -64)
    pushq  %r9
    cfi_rel_offset (%r9, -72)
    pushq  %r10
    cfi_rel_offset (%r10, -80)
    pushq  %r11
    cfi_rel_offset (%r11, -88)
    pushq  %r12
    cfi_rel_offset (%r12, -96)
    pushq  %r13
    cfi_rel_offset (%r13, -104)
    pushq  %r14
    cfi_rel_offset (%r14, -112)
    pushq  %r15    
    cfi_rel_offset (%r15, -120)

    // Need to ensure 16-bit alignment
    subq   $HandleSafepointStubFrameSize, %rsp

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
    movq   %r15,     %rcx
    movq   $3,       %rdx
    leaq   13(%rip), %rdi // rdi is addr of unwindPCForSafepointHandlerStub
    callq  MRT_UpdateUwContext@PLT

    // call safepoint handler
    movq   %r15,     %rdi
    callq  HandleSafepoint@PLT
    .global unwindPCForSafepointHandlerStub
unwindPCForSafepointHandlerStub:
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
    addq  $HandleSafepointStubFrameSize, %rsp

    // do not popq %r15
    popq  %rax
    cfi_restore (%rax)
    popq  %r14
    cfi_restore (%r14)
    popq  %r13
    cfi_restore (%r13)
    popq  %r12
    cfi_restore (%r12)
    popq  %r11
    cfi_restore (%r11)
    popq  %r10
    cfi_restore (%r10)
    popq  %r9
    cfi_restore (%r9)
    popq  %r8
    cfi_restore (%r8)
    popq  %rsp
    cfi_restore (%rsp)
    popq  %rsi
    cfi_restore (%rsi)
    popq  %rdi
    cfi_restore (%rdi)
    popq  %rdx
    cfi_restore (%rdx)
    popq  %rcx
    cfi_restore (%rcx)
    popq  %rbx
    cfi_restore (%rbx)
    popq  %rax
    cfi_restore (%rax)
    popq  %rbp
    cfi_restore (%rbp)
    .cfi_def_cfa %rsp, 8

    retq
    .cfi_endproc
    .size CODE_MCC_HandleSafepoint, .-CODE_MCC_HandleSafepoint
