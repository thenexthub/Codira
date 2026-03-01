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
#define C2NStubFrameSize           (8 * 30)

// On execution of "bl MCC_C2NStub", the frame layout of stack(growing downwards) looks like:
// Nativefunc address will be passed to %rax after MCC_C2NStub is built.
//                 |  ...         |
//                 |  ra          | return address for the caller of MCC_C2NStub
// caller rbp  --> |  rbp         |
//                 |  ...         |
//                 | arg9         |
//                 | arg8         |
//                 | arg7         |
//                 | arg6         |
//                 | callee addr  |
// caller rsp  --> | cpStackSize  |

// the frame layout of stack(growing downwards) after MCC_C2NStub frame is built looks like:
//                 |  ...         |
//                 |  ra          | return address for the caller of MCC_C2NStub
// caller rbp  --> |  rbp         |
//                 |  ...         |
//                 | arg9         |
//                 | arg8         |
//                 | arg7         |
//                 | arg6         |
//                 | callee addr  |
// caller rsp  --> | cpStackSize  |
//                 | ra           |
//   stub rbp  --> | caller rbp   | <== C2NStubStart
// callee register | %r15         |
//                 | %r14         |
//                 | %r13         |
//                 | %r12         |
//                 | %rbx         |
// arg0            | %rdi         |
// arg1            | %rsi         |
// arg2            | %rdx         |
// arg3            | %rcx         |
// arg4            | %r8          |
// arg5            | %r9          |
//                 | %rax         |
//                 | xmm0(high)   |
//                 | xmm0(low)    |
//                 | xmm1(high)   |
//                 | xmm1(low)    |
//                 | xmm2(high)   |
//                 | xmm2(low)    |
//                 | xmm3(high)   |
//                 | xmm3(low)    |
//                 | xmm4(high)   |
//                 | xmm4(low)    |
//                 | xmm5(high)   |
//                 | xmm5(low)    |
//                 | xmm6(high)   |
//                 | xmm6(low)    |
//                 | xmm7(high)   |
//                 | xmm7(low)    |
//                 | entersafe    | <== C2NStubFrameSize
//                 | ....         | <== copy caller Stack start here
//                 | arg9         |
//                 | arg8         |
//                 | arg7         |
//   stub rsp  --> | arg6         | <== MCC_C2NStub frame ends at here

    .text
    .align 2
    .global _CODE_MCC_C2NStub
_CODE_MCC_C2NStub:
    .cfi_startproc
    pushq   %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq    %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq    $C2NStubFrameSize, %rsp

    movq    %r15, -8(%rbp)
    movq    %r14, -16(%rbp)
    movq    %r13, -24(%rbp)
    movq    %r12, -32(%rbp)
    movq    %rbx, -40(%rbp)
    .cfi_rel_offset %rbx, -40
    .cfi_rel_offset %r12, -32
    .cfi_rel_offset %r13, -24
    .cfi_rel_offset %r14, -16
    .cfi_rel_offset %r15, -8

    // save arg0~arg5 to stack
    movq    %rdi,  -48(%rbp)
    movq    %rsi,  -56(%rbp)
    movq    %rdx,  -64(%rbp)
    movq    %rcx,  -72(%rbp)
    movq    %r8,   -80(%rbp)
    movq    %r9,   -88(%rbp)
    movq    %rax,  -96(%rbp)
    movapd  %xmm0, -112(%rbp)
    movapd  %xmm1, -128(%rbp)
    movapd  %xmm2, -144(%rbp)
    movapd  %xmm3, -160(%rbp)
    movapd  %xmm4, -176(%rbp)
    movapd  %xmm5, -192(%rbp)
    movapd  %xmm6, -208(%rbp)
    movapd  %xmm7, -224(%rbp)

    callq   _CODE_CODEThreadStackGuardGet
    // rbx = cpStackSize
    movq    16(%rbp), %rbx
    addq    %rbx, %rax
    cmpq    %rax, %rsp
    jbe     .L_stack_check_fail

    movq    %rbp,  %rsi
    // Get the current pc address through the following two assembly instructions.
    call    .L_current_pc
.L_current_pc:
     .global _unwindPCForC2NStub
_unwindPCForC2NStub:
    pop     %rdi
    mov     %r15, %rdx
    callq   _MRT_SaveC2NContext
    movq    $0, %rdi
    callq   _MRT_EnterSaferegion
    movq    %rax, -232(%rbp)

    // rbx = cpStackSize, r12 = calleeAddr
    movq    16(%rbp), %rbx
    movq    24(%rbp), %r12

    // Let the r14 register(current rbp) point to the starting point of the stack to be copied
    movq    %rbp, %r14
    addq    $32,  %r14
    addq    %rbx, %r14
    // Let the r13 register(current rbp) point to the ending point of the stack to be copied
    movq    %rbp, %r13
    addq    $32,  %r13

    // copy arg6, arg7, ...(if existed)
.L_copy:
    cmpq    %r13,    %r14
    jle     .L_copy_end
    subq    $8,      %r14
    movq    0(%r14), %r15
    pushq   %r15
    jmp     .L_copy
.L_copy_end:

    // Prepare arg0 to arg6.
    movq    -48(%rbp),  %rdi
    movq    -56(%rbp),  %rsi
    movq    -64(%rbp),  %rdx
    movq    -72(%rbp),  %rcx
    movq    -80(%rbp),  %r8
    movq    -88(%rbp),  %r9
    movq    -96(%rbp),  %rax
    movapd  -112(%rbp), %xmm0
    movapd  -128(%rbp), %xmm1
    movapd  -144(%rbp), %xmm2
    movapd  -160(%rbp), %xmm3
    movapd  -176(%rbp), %xmm4
    movapd  -192(%rbp), %xmm5
    movapd  -208(%rbp), %xmm6
    movapd  -224(%rbp), %xmm7

    callq   *%r12

    // keep potential return value
    // rax is 1st return register, rdx is 2nd return register
    // xmm0-xmm1 are used to return floating point arguments
    movq    %rdx,  -48(%rbp)
    movq    %rax,  -56(%rbp)
    movapd  %xmm0, -80(%rbp)
    movapd  %xmm1, -96(%rbp)

    movq   -232(%rbp), %rax
    cmpq    $0, %rax
    je     .L_none_leave
    callq   _MRT_LeaveSaferegion
    callq   _MRT_GetThreadLocalData
    movq    %rax, -8(%rbp)
.L_none_leave:
    movq    %rax, %rdi
    callq   _MRT_DeleteC2NContext

    mov     $0, %rdi
    callq   _MRT_ThrowPendingException

    // set potential return value
    movq    -48(%rbp), %rdx
    movq    -56(%rbp), %rax
    movapd  -80(%rbp), %xmm0
    movapd  -96(%rbp), %xmm1

    // Restore the value of rsp before coping arg6, arg7...
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
    movq    -40(%rbp), %rbx

    popq    %rbp
    .cfi_def_cfa %rsp, 8
    retq
.L_stack_check_fail:
    call    _MCC_ThrowStackOverflowError
    callq   _MRT_GetThreadLocalData
    movq    %rax, -8(%rbp)
    jmp     .L_stack_check
    .cfi_endproc
