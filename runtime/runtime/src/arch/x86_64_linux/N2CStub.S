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
#define N2CStubFrameSize           (8 * 38)

// On execution of "bl MCC_N2CStub", the frame layout of stack(growing downwards) looks like:
// arg will be passed to %r9(sixth arg) from caller's stack after MCC_N2CStub is built.
//                 |  ...         |
//                 |  ra          | return address for the caller of MCC_N2CStub
// caller rbp  --> |  rbp         |
//                 |  ...         |
//                 | arg9         |
//                 | arg8         |
//                 | arg7         |
//                 | arg6         |
//                 | callee addr  |
// caller rsp  --> | cpStackSize  |

// the frame layout of stack(growing downwards) after MCC_N2CStub frame is built looks like:
//                 |  ...         |
//                 |  ra          | return address for the caller of MCC_N2CStub
// caller rbp  --> |  rbp         |
//                 |  ...         |
//                 | arg9         |
//                 | arg8         |
//                 | arg7         |
//                 | arg6         |
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
//                 | current rsp  | <==N2CStubFrameSize
//                 | ....         | <== copy caller Stack start here
//                 | arg9         |
//                 | arg8         |
//                 | arg7         |
//   stub rsp  --> | arg6         | <== MCC_N2CStub frame ends at here

    .text
    .align 2
    .global   CODE_MCC_N2CStub
    .type     CODE_MCC_N2CStub, @function
CODE_MCC_N2CStub:
    .cfi_startproc
    pushq   %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq    %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq    $N2CStubFrameSize, %rsp

    movq    %r15, -72(%rbp)
    movq    %r14, -80(%rbp)
    movq    %r13, -88(%rbp)
    movq    %r12, -96(%rbp)
    movq    %rbx, -104(%rbp)
    .cfi_rel_offset %rbx, -104
    .cfi_rel_offset %r12, -96
    .cfi_rel_offset %r13, -88
    .cfi_rel_offset %r14, -80
    .cfi_rel_offset %r15, -72

    // save arg0~arg5 to stack
    movq    %rdi,  -112(%rbp) // arg0 is function address
    movq    %rsi,  -120(%rbp)
    movq    %rdx,  -128(%rbp)
    movq    %rcx,  -136(%rbp)
    movq    %r8,   -144(%rbp)
    movq    %r9,   -152(%rbp)
    movq    %rax,  -160(%rbp)
    movapd  %xmm0, -176(%rbp)
    movapd  %xmm1, -192(%rbp)
    movapd  %xmm2, -208(%rbp)
    movapd  %xmm3, -224(%rbp)
    movapd  %xmm4, -240(%rbp)
    movapd  %xmm5, -256(%rbp)
    movapd  %xmm6, -272(%rbp)
    movapd  %xmm7, -288(%rbp)

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
    movq    %rbp, %rdi
    callq   MRT_SaveTopManagedContextToN2CStub

    // Store rsp before coping arg6, arg7...
    movq    %rsp, -296(%rbp)

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
    callq   MRT_GetThreadLocalData@PLT
    movq    %rax, %r15

    movq    $0, %rdi
    callq   MRT_SetStackGrow@PLT

    // Prepare arg0 to arg5.
    movq    -112(%rbp), %rdi
    movq    -120(%rbp), %rsi
    movq    -128(%rbp), %rdx
    movq    -136(%rbp), %rcx
    movq    -144(%rbp), %r8
    movq    -152(%rbp), %r9
    movq    -160(%rbp), %rax
    movapd  -176(%rbp), %xmm0
    movapd  -192(%rbp), %xmm1
    movapd  -208(%rbp), %xmm2
    movapd  -224(%rbp), %xmm3
    movapd  -240(%rbp), %xmm4
    movapd  -256(%rbp), %xmm5
    movapd  -272(%rbp), %xmm6
    movapd  -288(%rbp), %xmm7

    // Get callee function address from rbx.
    callq   *%r12
    .global unwindPCForN2CStub
unwindPCForN2CStub:

    // keep potential return value
    // rax is 1st return register, rdx is 2nd return register
    // xmm0-xmm1 are used to return floating point arguments
    movq    %rdx,  -112(%rbp)
    movq    %rax,  -120(%rbp)
    movapd  %xmm0, -144(%rbp)
    movapd  %xmm1, -160(%rbp)

    movq    $1, %rdi
    callq   MRT_SetStackGrow@PLT

    // restore topContext from n2cStub stot
    movq    %rbp, %rdi
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
    je     .L_none_enter
    movq    $0, %rdi
    callq   MRT_EnterSaferegion
.L_none_enter:

    // Restore the value of rsp before coping arg6, arg7...
    movq    -296(%rbp), %rsp

    // set potential return value
    movq    -112(%rbp), %rdx
    movq    -120(%rbp), %rax
    movapd  -144(%rbp), %xmm0
    movapd  -160(%rbp), %xmm1

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
    movq    -104(%rbp), %rbx

    popq    %rbp
    .cfi_def_cfa %rsp, 8
    retq
    .cfi_endproc
    .size CODE_MCC_N2CStub, .-CODE_MCC_N2CStub

    .text
    .align 2
    .global   ExecuteCodiraStub
    .type     ExecuteCodiraStub, @function
ExecuteCodiraStub:
    .cfi_startproc
    pushq   %r15
    .cfi_adjust_cfa_offset 8
    movq    %r8, %r15
    callq   *%rcx
    popq    %r15
    .cfi_adjust_cfa_offset -8
    retq
    .cfi_endproc
    .size ExecuteCodiraStub, .-ExecuteCodiraStub

    .text
    .align 2
    .global   InitCODELibraryStub
    .type     InitCODELibraryStub, @function
InitCODELibraryStub:
    .cfi_startproc
    pushq   %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq    %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movq    %rdi, %r11
    pushq   %r11
    pushq   $0
    callq   CODE_MCC_N2CStub
    popq    %rbp
    .cfi_def_cfa %rsp, 8
    retq
    .cfi_endproc
    .size InitCODELibraryStub, .-InitCODELibraryStub

    .text
    .align 2
    .global   ResolveCycleRefStub
    .type     ResolveCycleRefStub, @function
ResolveCycleRefStub:
    .cfi_startproc
    pushq   %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq    %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movq    %rdi, %r11
    pushq   %r11
    pushq   $0
    movq    %rsi, %rdi
    movq    %rdx, %rsi
    callq   CODE_MCC_N2CStub
    popq    %rbp
    .cfi_def_cfa %rsp, 8
    retq
    .cfi_endproc
    .size ResolveCycleRefStub, .-ResolveCycleRefStub

#ifdef __OHOS__


    .text
    .align 2
    .global   CODE_MRT_ARKTS_CreateEngineStub
    .type     CODE_MRT_ARKTS_CreateEngineStub, @function
CODE_MRT_ARKTS_CreateEngineStub:
    .cfi_startproc
    pushq   %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq    %rsp, %rbp
    .cfi_def_cfa_register %rbp
    movq    %rsp, %r12
    callq   IsForeignThread
    cmpq    $0, %rax
    jne     .L_no_need_switch
    callq   GetNativeSPForUIThread
    movq    %rax, %rsp
.L_no_need_switch:
    callq   CODE_MRT_ARKTS_CreateEngine
    movq    %r12, %rsp
    popq    %rbp
    .cfi_def_cfa %rsp, 8
    retq
    .cfi_endproc
    .size CODE_MRT_ARKTS_CreateEngineStub, .-CODE_MRT_ARKTS_CreateEngineStub
#endif

    .text
    .align 2
    .global   ApplyCodiraMethodStub
    .type     ApplyCodiraMethodStub, @function
    .global   ApplyCodiraMethodStubFloat32
    .type     ApplyCodiraMethodStubFloat32, @function
    .global   ApplyCodiraMethodStubFloat64
    .type     ApplyCodiraMethodStubFloat64, @function
ApplyCodiraMethodStub:
ApplyCodiraMethodStubFloat32:
ApplyCodiraMethodStubFloat64:
    .cfi_startproc
    pushq   %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq    %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq    $N2CStubFrameSize, %rsp

    movq    %r15, -72(%rbp)
    movq    %r14, -80(%rbp)
    movq    %r13, -88(%rbp)
    movq    %r12, -96(%rbp)
    movq    %rbx, -104(%rbp)
    .cfi_rel_offset %rbx, -104
    .cfi_rel_offset %r12, -96
    .cfi_rel_offset %r13, -88
    .cfi_rel_offset %r14, -80
    .cfi_rel_offset %r15, -72

    movq    %rdi, %r10 // save args
    movq    %rsi, %r12 // stackSize
    //movq    %rdx, %r11 // slot
    movq    %rdx, %r13 // save func
    movq    %rcx,  %r14 // theadData

    // kRegArgsSize * 8 = 112
    movq    %r10, %rdi
    addq    $112,  %rdi

    movq    %r10, %rsi
    addq    $112,  %rsi
    addq    %r12,  %rsi

    // copy arg6, arg7, ...(if existed)
.L_copy_args:
    cmpq    %rdi,    %rsi
    jle     .L_copy_args_end
    subq    $8,      %rsi
    movq    (%rsi),  %rdx
    pushq   %rdx
    jmp     .L_copy_args
.L_copy_args_end:

    // prepare arg0-arg5
    movq    (%r10),   %rdi // arg0 in callee
    movq    8(%r10),  %rsi // arg1
    movq    16(%r10), %rdx // arg2
    movq    24(%r10), %rcx // arg3
    movq    32(%r10), %r8  // arg4
    movq    40(%r10), %r9  // arg5

    // prepare xmm0-xmm7
    addq    $48,      %r10
    movsd   (%r10),   %xmm0
    movsd   8(%r10),  %xmm1
    movsd   16(%r10), %xmm2
    movsd   24(%r10), %xmm3
    movsd   32(%r10), %xmm4
    movsd   40(%r10), %xmm5
    movsd   48(%r10), %xmm6
    movsd   56(%r10), %xmm7

    // call method
    movq    %r14, %r15
    callq   *%r13

    // keep potential return value
    // rax is 1st return register, rdx is 2nd return register
    // xmm0-xmm1 are used to return floating point arguments
    movq    %rdx,  -112(%rbp)
    movq    %rax,  -120(%rbp)
    movsd   %xmm0, -144(%rbp)
    movsd   %xmm1, -160(%rbp)

    // set potential return value
    movq    -112(%rbp), %rdx
    movq    -120(%rbp), %rax
    movsd   -144(%rbp), %xmm0
    movsd   -160(%rbp), %xmm1

    addq    %r12, %rsp
    addq    $N2CStubFrameSize, %rsp

    movq    -72(%rbp),  %r15
    movq    -80(%rbp),  %r14
    movq    -88(%rbp),  %r13
    movq    -96(%rbp),  %r12
    movq    -104(%rbp), %rbx

    popq   %rbp
    .cfi_adjust_cfa_offset -8
    retq
    .cfi_endproc
    .size ApplyCodiraMethodStub, .-ApplyCodiraMethodStub
