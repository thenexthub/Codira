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

#define StackGrowStubFrame           (8 * 46)

// This signal handler stub relies on MRT_PrepareToHandleSignal to put continuation pc in x17.
// Continuation pc describes the control flow after signal handling completes.


    .def CODE_MCC_StackGrowStub;
    .scl 2;
    .type 32;
    .endef
    .global CODE_MCC_StackGrowStub
    .p2align    4, 0x90
CODE_MCC_StackGrowStub:
    .seh_proc CODE_MCC_StackGrowStub

    pushq   %rbp
    .seh_pushreg %rbp
    movq    %rsp, %rbp
    .seh_setframe %rbp, 0
    subq    $StackGrowStubFrame, %rsp
    .seh_stackalloc StackGrowStubFrame
    .seh_endprologue

    movq    %rax, -8(%rbp)
    movq    %rbx, -16(%rbp)
    movq    %rcx, -24(%rbp)
    movq    %rdx, -32(%rbp)
    movq    %rdi, -40(%rbp)
    movq    %rsi, -48(%rbp)
    movq    %r8, -56(%rbp)
    movq    %r9, -64(%rbp)
    movq    %r10, -72(%rbp)
    movq    %r11, -80(%rbp)
    movq    %r12, -88(%rbp)
    movq    %r13, -96(%rbp)
    movq    %r14, -104(%rbp)
    movq    %r15, -112(%rbp)

    movapd  %xmm0, -128(%rbp)
    movapd  %xmm1, -144(%rbp)
    movapd  %xmm2, -160(%rbp)
    movapd  %xmm3, -176(%rbp)
    movapd  %xmm4, -192(%rbp)
    movapd  %xmm5, -208(%rbp)
    movapd  %xmm6, -224(%rbp)
    movapd  %xmm7, -240(%rbp)
    movapd  %xmm8, -256(%rbp)
    movapd  %xmm9, -272(%rbp)
    movapd  %xmm10, -288(%rbp)
    movapd  %xmm11, -304(%rbp)
    movapd  %xmm12, -320(%rbp)
    movapd  %xmm13, -336(%rbp)
    movapd  %xmm14, -352(%rbp)
    movapd  %xmm15, -368(%rbp)

    movq   $1, %r8
    movq   %rbp, %rdx
    leaq   31(%rip), %rcx
    movq   %r15, %r9
    callq  MRT_UpdateUwContext

    movq   %rsp,    %rcx
    addq   $StackGrowStubFrame, %rcx
    movq   -8(%rbp), %rdx
    movq   8(%rbp),  %r8
    callq  MRT_StackGrow
    .global unwindPCForStackGrowStub
unwindPCForStackGrowStub:

    addq %rax, %rsp
    addq %rax, %rbp
    addq %rax, %gs:0x08

    movq   %rax, %rcx
    callq  MRT_FreeOldStack

    callq  MRT_GetThreadLocalData
    movq   %rax, %r15
    movq   %rax, %rcx
    callq  MRT_DeleteC2NContext

    addq    $StackGrowStubFrame, %rsp

    movq    -8(%rbp), %rax
    movq    -16(%rbp), %rbx
    movq    -24(%rbp), %rcx
    movq    -32(%rbp), %rdx
    movq    -40(%rbp), %rdi
    movq    -48(%rbp), %rsi
    movq    -56(%rbp), %r8
    movq    -64(%rbp), %r9
    movq    -72(%rbp), %r10
    movq    -80(%rbp), %r11
    movq    -88(%rbp), %r12
    movq    -96(%rbp), %r13
    movq    -104(%rbp), %r14
    movq    -112(%rbp), %rax

    movapd  -128(%rbp), %xmm0
    movapd  -144(%rbp), %xmm1
    movapd  -160(%rbp), %xmm2
    movapd  -176(%rbp), %xmm3
    movapd  -192(%rbp), %xmm4
    movapd  -208(%rbp), %xmm5
    movapd  -224(%rbp), %xmm6
    movapd  -240(%rbp), %xmm7
    movapd  -256(%rbp), %xmm8
    movapd  -272(%rbp), %xmm9
    movapd  -288(%rbp), %xmm10
    movapd  -304(%rbp), %xmm11
    movapd  -320(%rbp), %xmm12
    movapd  -336(%rbp), %xmm13
    movapd  -352(%rbp), %xmm14
    movapd  -368(%rbp), %xmm15

    popq    %rbp
    retq
    .seh_endproc
