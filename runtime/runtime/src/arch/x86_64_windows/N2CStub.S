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

#define N2CStubFrameSize           (8 * 48)

// On execution of "bl MCC_N2CStub", the frame layout of stack(growing downwards) looks like:
//                 |  ...         |
//                 |  ra          | return address for the caller of MCC_N2CStub
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

// the frame layout of stack(growing downwards) after MCC_N2CStub frame is built looks like:
//                 |  ...         |
//                 |  ra          | return address for the caller of MCC_N2CStub
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
//   stub rbp  --> | caller rbp   | <==N2CStubStart
//                 | topState     |
//                 | topContextFP |
//                 | topContextPC |
//                 | n2cstubslot  |
//                 | n2cstubslot  |
//                 | n2cstubslot  |
//                 | n2cstubslot  |
//                 | leavestate/argSize |
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
//                 | ....         | <== copy caller Stack start here
//                 | arg7         |
//                 | arg6         |
//                 | arg5         |
//                 | arg4         |
//                 | shadow space |
//                 | shadow space |
//                 | shadow space |
//   stub rsp  --> | shadow space | <== MCC_N2CStub frame ends at here

    .def CODE_MCC_N2CStub;
    .scl 2;
    .type 32;
    .endef
    .global CODE_MCC_N2CStub
    .p2align    4, 0x90
CODE_MCC_N2CStub:
    .seh_proc CODE_MCC_N2CStub
    pushq   %rbp
    .seh_pushreg %rbp
    movq    %rsp, %rbp
    .seh_setframe %rbp, 0
    subq    $N2CStubFrameSize, %rsp
    .seh_stackalloc N2CStubFrameSize
    .seh_endprologue

    movq    %r15,  -72(%rbp)
    movq    %r14,  -80(%rbp)
    movq    %r13,  -88(%rbp)
    movq    %r12,  -96(%rbp)
    movq    %rdi,  -104(%rbp)
    movq    %rsi,  -112(%rbp)
    movq    %rbx,  -120(%rbp)

    movq    %rcx,  -128(%rbp)
    movq    %rdx,  -136(%rbp)
    movq    %r8,   -144(%rbp)
    movq    %r9,   -152(%rbp)
    movq    %rax,  -160(%rbp)
    movapd  %xmm0, -176(%rbp)
    movapd  %xmm1, -192(%rbp)
    movapd  %xmm2, -208(%rbp)
    movapd  %xmm3, -224(%rbp)

    movapd  %xmm6, -240(%rbp)
    movapd  %xmm7, -256(%rbp)
    movapd  %xmm8, -272(%rbp)
    movapd  %xmm9, -288(%rbp)
    movapd  %xmm10, -304(%rbp)
    movapd  %xmm11, -320(%rbp)
    movapd  %xmm12, -336(%rbp)
    movapd  %xmm13, -352(%rbp)
    movapd  %xmm14, -368(%rbp)
    movapd  %xmm15, -384(%rbp)

    // store whether new CODEThread result in -60(%rbp)
    // and use it after calling Codira function.
    callq   MRT_TryNewAndRunCODEThread
    mov     %al,  -60(%rbp)

    // mutator mustn't be in safe region before setting context.
    // store whether leave saferegion result in -64(%rbp)
    // and use it after calling Codira function.
    callq   MRT_LeaveSaferegion
    mov     %al,  -64(%rbp)

    // need to save topContext to stub slot.
    movq    %rbp, %rcx
    callq   MRT_SaveTopManagedContextToN2CStub

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

    callq  MRT_GetThreadLocalData
    movq   %rax, %r15

    movq    $0, %rcx
    callq   MRT_SetStackGrow

    // Prepare arg0 to arg3.
    movq    -128(%rbp), %rcx
    movq    -136(%rbp), %rdx
    movq    -144(%rbp), %r8
    movq    -152(%rbp), %r9
    movq    -160(%rbp), %rax
    movapd  -176(%rbp), %xmm0
    movapd  -192(%rbp), %xmm1
    movapd  -208(%rbp), %xmm2
    movapd  -224(%rbp), %xmm3

    // Get callee function address from rbx.
    callq   *%r12
    .global unwindPCForN2CStub
unwindPCForN2CStub:

    // keep potential return value
    // rax is 1st return register
    // xmm0 was used to return floating point argument
    movq    %rax,  -128(%rbp)
    movapd  %xmm0, -144(%rbp)

    movq    $1, %rcx
    callq   MRT_SetStackGrow

    // restore topContext from n2cStub stot
    movq    %rbp, %rcx
    callq   MRT_RestoreTopManagedContextFromN2CStub
    mov     -60(%rbp), %al
    cmp     $0, %al
    je      .L_no_need_end
    callq   MRT_EndCODEThread
    cmpq    $1, %rax
    je      .L_none_enter
.L_no_need_end:
    mov     -64(%rbp), %al
    cmp     $0, %al
    je      .L_none_enter
    movq    $0, %rcx
    callq   MRT_EnterSaferegion
.L_none_enter:

    // set potential return value
    movq    -128(%rbp), %rax
    movapd  -144(%rbp), %xmm0

    // Restore the value of rsp before coping arg4, arg5...
    addq    %rbx, %rsp
    addq    $N2CStubFrameSize, %rsp

    // delete callee&cpStackSize, Restore the caller stack.
    movq    0(%rbp),   %r12
    movq    %r12,      16(%rbp)
    movq    8(%rbp),   %r12
    movq    %r12,      24(%rbp)
    addq    $16, %rsp

    movq    -72(%rbp),  %r15
    movq    -80(%rbp),  %r14
    movq    -88(%rbp),  %r13
    movq    -96(%rbp),  %r12
    movq    -104(%rbp), %rdi
    movq    -112(%rbp), %rsi
    movq    -120(%rbp), %rbx

    movapd  -240(%rbp), %xmm6
    movapd  -256(%rbp), %xmm7
    movapd  -272(%rbp), %xmm8
    movapd  -288(%rbp), %xmm9
    movapd  -304(%rbp), %xmm10
    movapd  -320(%rbp), %xmm11
    movapd  -336(%rbp), %xmm12
    movapd  -352(%rbp), %xmm13
    movapd  -368(%rbp), %xmm14
    movapd  -384(%rbp), %xmm15

    popq    %rbp
    retq
    .seh_endproc

    .text
    .def ExecuteCodiraStub
    .scl 2
    .type 32
    .endef
    .global ExecuteCodiraStub
    .p2align 4, 0x90
ExecuteCodiraStub:
.seh_proc ExecuteCodiraStub
    pushq   %rbp
    .seh_pushreg %rbp
    movq    %rsp, %rbp
    .seh_endprologue

    // we push a extra value for aligning.
    pushq   %r15
    .seh_pushreg %r15
    pushq   %r15
    .seh_pushreg %r15

    // save caller arg to stack
    pushq   %rcx
    pushq   %rdx
    pushq   %r8
    pushq   %r9

    callq   MRT_GetThreadLocalData
    movq    %rax, %r15

    // restore caller arg
    popq    %r9
    popq    %r8
    popq    %rdx
    popq    %rcx

    callq   *%r9

    popq    %r15
    popq    %r15

    popq    %rbp
    retq
.seh_endproc

    .def ApplyCodiraMethodStub
    .scl 2
    .type 32
    .endef
    .global ApplyCodiraMethodStub
    .global ApplyCodiraMethodStubFloat32
    .global ApplyCodiraMethodStubFloat64
    .p2align 4, 0x90
ApplyCodiraMethodStub:
ApplyCodiraMethodStubFloat32:
ApplyCodiraMethodStubFloat64:
.seh_proc ApplyCodiraMethodStub
    pushq   %rbp
    .seh_pushreg %rbp
    movq    %rsp, %rbp
    .seh_setframe %rbp, 0
    subq    $N2CStubFrameSize, %rsp
    .seh_stackalloc N2CStubFrameSize
    .seh_endprologue

    movq    %r15,  -72(%rbp)
    movq    %r14,  -80(%rbp)
    movq    %r13,  -88(%rbp)
    movq    %r12,  -96(%rbp)
    movq    %rdi,  -104(%rbp)
    movq    %rsi,  -112(%rbp)
    movq    %rbx,  -120(%rbp)

    movq    %rcx,  -128(%rbp)
    movq    %rdx,  -136(%rbp)
    movq    %r8,   -144(%rbp)
    movq    %r9,   -152(%rbp)
    movq    %rax,  -160(%rbp)
    movq    %xmm0, -176(%rbp)
    movq    %xmm1, -192(%rbp)
    movq    %xmm2, -208(%rbp)
    movq    %xmm3, -224(%rbp)

    movapd  %xmm6, -240(%rbp)
    movapd  %xmm7, -256(%rbp)
    movapd  %xmm8, -272(%rbp)
    movapd  %xmm9, -288(%rbp)
    movapd  %xmm10, -304(%rbp)
    movapd  %xmm11, -320(%rbp)
    movapd  %xmm12, -336(%rbp)
    movapd  %xmm13, -352(%rbp)
    movapd  %xmm14, -368(%rbp)
    movapd  %xmm15, -384(%rbp)

    movq    -128(%rbp), %rcx // value
    movq    -136(%rbp), %rdx // stackSize
    movq    -144(%rbp), %r14  // entry pointer
    movq    -152(%rbp), %r15  // threadData

    // kRegArgsSize * 8 = 64
    movq    %rcx, %r8
    addq    $64,  %r8

    movq    %rcx, %r9
    addq    $64,  %r9
    addq    %rdx,  %r9

    // copy arg4, arg5, ...(if existed)
.L_copy_args:
    cmpq    %r8,   %r9
    jle     .L_copy_end_args
    subq    $8,    %r9
    movq    (%r9), %r12
    pushq   %r12
    .seh_pushreg %r12
    jmp     .L_copy_args
.L_copy_end_args:

    movq %rcx, %r13
    // prepare arg0-arg3
    movq (%r13),   %rcx
    movq 8(%r13),  %rdx
    movq 16(%r13), %r8
    movq 24(%r13), %r9


    // prepare xmm0-xmm3
    addq    $32, %r13
    movsd   (%r13),   %xmm0
    movsd   8(%r13),  %xmm1
    movsd   16(%r13), %xmm2
    movsd   24(%r13), %xmm3

    // shadow space, here just for aligning with Windows stack frame, so the content pushed here can be anything.
    // push 0 data will losing CFI info, thus we choose r12 as pushed content.
    pushq   %r12
    .seh_pushreg %r12
    pushq   %r12
    .seh_pushreg %r12
    pushq   %r12
    .seh_pushreg %r12
    pushq   %r12
    .seh_pushreg %r12

    callq   *%r14

    popq    %r12
    popq    %r12
    popq    %r12
    popq    %r12

    // keep potential return value
    // rax is 1st return register
    // xmm0 was used to return floating point argument
    movq    %rax,  -128(%rbp)
    movapd  %xmm0, -176(%rbp)

    // set potential return value
    movq    -128(%rbp), %rax
    movapd  -176(%rbp), %xmm0

    // Restore the value of rsp before coping arg4, arg5...
    addq    -136(%rbp), %rsp
    addq    $N2CStubFrameSize, %rsp

    movq    -72(%rbp),  %r15
    movq    -80(%rbp),  %r14
    movq    -88(%rbp),  %r13
    movq    -96(%rbp),  %r12
    movq    -104(%rbp), %rdi
    movq    -112(%rbp), %rsi
    movq    -120(%rbp), %rbx

    movapd  -240(%rbp), %xmm6
    movapd  -256(%rbp), %xmm7
    movapd  -272(%rbp), %xmm8
    movapd  -288(%rbp), %xmm9
    movapd  -304(%rbp), %xmm10
    movapd  -320(%rbp), %xmm11
    movapd  -336(%rbp), %xmm12
    movapd  -352(%rbp), %xmm13
    movapd  -368(%rbp), %xmm14
    movapd  -384(%rbp), %xmm15

    popq    %rbp
    retq
.seh_endproc
