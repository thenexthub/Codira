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

#define C2NStubFrameSize           (8 * 42)

// On execution of "bl MCC_C2NStub", the frame layout of stack(growing downwards) looks like:
// Nativefunc address will be passed to %rax after MCC_C2NStub is built.
//                 |  ...         |
//                 |  ra          | return address for the caller of MCC_C2NStub
// caller rbp  --> |  rbp         |
//                 |  ...         |
//                 | arg7         |
//                 | arg6         |
//                 | arg5         |
//                 | arg4         |
//                 | shadow space |
//                 | shadow space |
//                 | shadow space |
//                 | shadow space |
//                 | callee addr  |
// caller rsp  --> | cpStackSize  |

// the frame layout of stack(growing downwards) after MCC_C2NStub frame is built looks like:
//                 |  ...         |
//                 |  ra          | return address for the caller of MCC_C2NStub
// caller rbp  --> |  rbp         |
//                 |  ...         |
//                 | arg7         |
//                 | arg6         |
//                 | arg5         |
//                 | arg4         |
//                 | shadow space |
//                 | shadow space |
//                 | shadow space |
//                 | shadow space |
//                 | callee addr  |
// caller rsp  --> | cpStackSize  |
//                 | ra           |
//   stub rbp  --> | caller rbp   | <== C2NStubStart
// callee register | %r15         |
//                 | %r14         |
//                 | %r13         |
//                 | %r12         |
//                 | %rdi         |
//                 | %rsi         |
//                 | %rbx         |
// arg0            | %rcx         |
// arg1            | %rdx         |
// arg2            | %r8          |
// arg3            | %r9          |
//                 | %rax         |
//                 | %xmm0(high)  |
//                 | %xmm0(low)   |
//                 | %xmm1(high)  |
//                 | %xmm1(low)   |
//                 | %xmm2(high)  |
//                 | %xmm2(low)   |
//                 | %xmm3(high)  |
//                 | %xmm3(low)   |
//                 | %xmm6(high)  |
//                 | %xmm6(low)   |
//                 | %xmm7(high)  |
//                 | %xmm7(low)   |
//                 | %xmm8(high)  |
//                 | %xmm8(low)   |
//                 | %xmm9(high)  |
//                 | %xmm9(low)   |
//                 | %xmm10(high) |
//                 | %xmm10(low)  |
//                 | %xmm11(high) |
//                 | %xmm11(low)  |
//                 | %xmm12(high) |
//                 | %xmm12(low)  |
//                 | %xmm13(high) |
//                 | %xmm13(low)  |
//                 | %xmm14(high) |
//                 | %xmm14(low)  |
//                 | %xmm15(high) |
//                 | %xmm15(low)  |
//                 | entersafe    | <== C2NStubFrameSize
//                 | ....         | <== copy caller Stack start here
//                 | arg7         |
//                 | arg6         |
//                 | arg5         |
//                 | arg4         |
//                 | shadow space |
//                 | shadow space |
//                 | shadow space |
//   stub rsp  --> | shadow space | <== MCC_C2NStub frame ends at here
    .def CODE_MCC_C2NStub
    .scl 2
    .type 32
    .endef
    .global CODE_MCC_C2NStub
    .p2align    4, 0x90
CODE_MCC_C2NStub:
    .seh_proc CODE_MCC_C2NStub
    pushq	%rbp
    .seh_pushreg %rbp
    movq    %rsp, %rbp
    .seh_setframe %rbp, 0
    subq    $C2NStubFrameSize, %rsp
    .seh_stackalloc C2NStubFrameSize
    .seh_endprologue

    // save callee-saved to stack
    movq    %r15, -8(%rbp)
    movq    %r14, -16(%rbp)
    movq    %r13, -24(%rbp)
    movq    %r12, -32(%rbp)
    movq    %rdi, -40(%rbp)
    movq    %rsi, -48(%rbp)
    movq    %rbx, -56(%rbp)

    // save args to stack
    movq    %rcx,  -64(%rbp)
    movq    %rdx,  -72(%rbp)
    movq    %r8,   -80(%rbp)
    movq    %r9,   -88(%rbp)
    movq    %rax,  -96(%rbp)

    movapd  %xmm0, -112(%rbp)
    movapd  %xmm1, -128(%rbp)
    movapd  %xmm2, -144(%rbp)
    movapd  %xmm3, -160(%rbp)

    // save callee-saved to stack
    movapd  %xmm6, -176(%rbp)
    movapd  %xmm7, -192(%rbp)
    movapd  %xmm8, -208(%rbp)
    movapd  %xmm9, -224(%rbp)
    movapd  %xmm10, -240(%rbp)
    movapd  %xmm11, -256(%rbp)
    movapd  %xmm12, -272(%rbp)
    movapd  %xmm13, -288(%rbp)
    movapd  %xmm14, -304(%rbp)
    movapd  %xmm15, -320(%rbp)

    callq   CODE_CODEThreadStackGuardGet
    // rbx = cpStackSize
    movq    16(%rbp), %rbx
    addq    %rbx, %rax
    cmpq    %rax, %rsp
    jbe     .L_stack_check_fail

    movq    %rbp, %rdx
    // Get the current pc address through the following two assembly instructions.
    call    .L_current_pc
.L_current_pc:
     .global unwindPCForC2NStub
unwindPCForC2NStub:
    pop     %rcx
    movq    %r15, %r8
    callq   MRT_SaveC2NContext
    movq    $0, %rcx
    callq   MRT_EnterSaferegion
    movq    %rax, -328(%rbp)

    // rbx = cpStackSize, r12 = calleeAddr
    movq    16(%rbp), %rbx
    movq    24(%rbp), %r12

    // Let the r14 register(current rbp) point to the starting point of the stack to be copied
    movq    %rbp, %r14
    addq    $32,  %r14
    addq    %rbx, %r14
    // Let the r13 register(current rbp) point to the ending point of the stack to be copied
    movq    %rbp, %r13
    addq    $64,  %r13

    // copy arg4, arg5, ...(if existed)
.L_copy:
    cmpq    %r13,    %r14
    jle     .L_copy_end
    subq    $8,      %r14
    movq    0(%r14), %r15
    pushq   %r15
    .seh_pushreg %r15
    jmp     .L_copy
.L_copy_end:

    // shadow space, here just for aligning with Windows stack frame, so the content pushed here can be anything.
    // push 0 data will losing CFI info, thus we choose r15 as pushed content.
    pushq   %r15
    .seh_pushreg %r15
    pushq   %r15
    .seh_pushreg %r15
    pushq   %r15
    .seh_pushreg %r15
    pushq   %r15
    .seh_pushreg %r15

    // Prepare arg0 to arg3
    movq    -64(%rbp),  %rcx
    movq    -72(%rbp),  %rdx
    movq    -80(%rbp),  %r8
    movq    -88(%rbp),  %r9
    movq    -96(%rbp),  %rax
    movapd  -112(%rbp), %xmm0
    movapd  -128(%rbp), %xmm1
    movapd  -144(%rbp), %xmm2
    movapd  -160(%rbp), %xmm3

    callq   *%r12

    // keep potential return value
    // rax is 1st return register, no 2nd return register in Windows
    // xmm0 are used to return floating point arguments
    movq    %rax,  -64(%rbp)
    movapd  %xmm0, -80(%rbp)

    movq    -328(%rbp), %rax
    cmpq    $0, %rax
    je      .L_none_leave
    callq   MRT_LeaveSaferegion
    callq   MRT_GetThreadLocalData
    movq    %rax, -8(%rbp)
.L_none_leave:
    movq    %rax, %rcx
    callq   MRT_DeleteC2NContext

    mov     $0, %rcx
    callq   MRT_ThrowPendingException
    .global unwindPCForTPE
unwindPCForTPE:

    // set potential return value
    movq    -64(%rbp), %rax
    movapd  -80(%rbp), %xmm0

    // Restore the value of rsp before coping arg4, arg5...
    addq    %rbx, %rsp

    // delete callee&cpStackSize, Restore the caller stack.
.L_stack_check:
    movq    0(%rbp),   %r12
    movq    %r12,      16(%rbp)
    movq    8(%rbp),   %r12
    movq    %r12,      24(%rbp)
    addq    $16, %rsp

    addq    $C2NStubFrameSize, %rsp

    movq    -8(%rbp),  %r15
    movq    -16(%rbp), %r14
    movq    -24(%rbp), %r13
    movq    -32(%rbp), %r12
    movq    -40(%rbp), %rdi
    movq    -48(%rbp), %rsi
    movq    -56(%rbp), %rbx

    movapd  -176(%rbp), %xmm6
    movapd  -192(%rbp), %xmm7
    movapd  -208(%rbp), %xmm8
    movapd  -224(%rbp), %xmm9
    movapd  -240(%rbp), %xmm10
    movapd  -256(%rbp), %xmm11
    movapd  -272(%rbp), %xmm12
    movapd  -288(%rbp), %xmm13
    movapd  -304(%rbp), %xmm14
    movapd  -320(%rbp), %xmm15

    popq    %rbp
    retq
.L_stack_check_fail:
    call    MCC_ThrowStackOverflowError
    callq   MRT_GetThreadLocalData
    movq    %rax, -8(%rbp)
    jmp     .L_stack_check
    .seh_endproc
