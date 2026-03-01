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

#define cfi_adjust_cfa_offset(off)      .cfi_adjust_cfa_offset off
#define cfi_rel_offset(reg, off)        .cfi_rel_offset reg, off
#define cfi_restore(reg)                .cfi_restore reg
#define cfi_def_cfa_register(reg)       .cfi_def_cfa_register reg

////////////////////////////////////////////////////////////////////////////////
// MCC_C2NStub simply forwards arguments passed by runtime, i.e., arguments for compiled method are passed
// according to C/C++ calling convention, which usually means efficiency.
////////////////////////////////////////////////////////////////////////////////

#define StubFrameContextSize          (4 * 30 - 4 * 2)
#define StubCalleeSaveAreaSize        (4 * 6)
#define FuncAddrAndCpStacksizeOffset  (4 * 4)
#define SafeStateOffset               (4 * 2)

// R means runtime, while C means compiled method. XX indicates the return type of this method.

// On execution of "bl MCC_C2NStub", the frame layout of stack(growing downwards) looks like:
// x0~x7: hold the first 8 arguments arg0~arg7 if existed
// x30: return address of "bl MCC_C2NStub"
// callee funcAddr and cpStackSize are saved on the stack
// all on-stack arguments are addressable by SP as the frame layout shows.
//                 | ...          |
//                 | lr           | lr for the caller of MCC_C2NStub
// caller fp  -->  | r11          |
//                 | ...          |
//                 | arg11        |
//                 | arg10        |
//                 | arg9         |
//                 | arg8         |
//                 | cpStackSize  |
// caller sp  -->  | funcAddr     |

// the frame layout of stack(growing downwards) after MCC_C2NStub frame is built looks like:
//                 | ...          |
//                 | lr           | lr for the caller of MCC_C2NStub
// caller fp  -->  | r11          |
//                 | ...          |
//                 | arg11        |
//                 | arg10        |
//                 | arg9         |
// caller sp  -->  | arg8         |
// callee saved    | d7(high)     | <== MCC_C2NStub frame starts from here
//                 | d7(low)      |
//                 | d6(high)     |
//                 | d6(low)      |
//                 | d5(high)     |
//                 | d5(low)      |
//                 | d4(high)     |
//                 | d4(low)      |
//                 | d3(high)     |
//                 | d3(low)      |
//                 | d2(high)     |
//                 | d2(low)      |
//                 | d1(high)     |
//                 | d1(low)      |
//                 | d0(high)     |
//                 | d0(low)      |
//                 | null         |
//                 | r10          |
//                 | r9           |
//                 | r8           |
//                 | r7           |
//                 | r6           |
//                 | r5           |
// callee saved    | r4           |
//                 | cpStackSize  |
//                 | calleeAddr   |
//                 | null         |
//                 | entersafe    |
//                 | lr           |
//   stub fp  -->  | r11 callerfp |
//                 | ...          | <== copy caller Stack start here
//                 | arg11        |
//                 | arg10        |
//                 | arg9         |
//   stub sp  -->  | arg8         | <== MCC_C2NStub frame ends at here

#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
    .global C2NStubFrameSize
C2NStubFrameSize:
    .quad StubFrameContextSize
#endif

    .text
    .align 2
    .global CODE_MCC_C2NStub
    .type CODE_MCC_C2NStub, %function
CODE_MCC_C2NStub:
    .cfi_startproc
    // actual stack size is StubFrameContextSize + 8
    // 8 means the size has been pre allocated: calleeAddr and cpStackSize
    sub  sp, sp, #StubFrameContextSize

    // save calleeAddr and cpStackSize
    ldr  r12, [sp, #StubFrameContextSize]  // calleeAddr
    str  r12, [sp, #FuncAddrAndCpStacksizeOffset]
    ldr  r12, [sp, #StubFrameContextSize+4]  // cpStackSize
    str  r12, [sp, #FuncAddrAndCpStacksizeOffset+4]

    str  r11, [sp]
    str  lr, [sp, #4]
    cfi_adjust_cfa_offset (StubFrameContextSize)
    cfi_rel_offset (r11, 0)
    cfi_rel_offset (lr, 4)

    mov  r11, sp
    cfi_def_cfa_register (r11)

    // save all used callee-saved registers
    str  r4, [sp, #StubCalleeSaveAreaSize]
    str  r5, [sp, #StubCalleeSaveAreaSize+4]
    str  r6, [sp, #StubCalleeSaveAreaSize+8]
    str  r7, [sp, #StubCalleeSaveAreaSize+12]
    str  r8, [sp, #StubCalleeSaveAreaSize+16]
    str  r9, [sp, #StubCalleeSaveAreaSize+20]
    str  r10, [sp, #StubCalleeSaveAreaSize+24]
    cfi_rel_offset (r4, StubCalleeSaveAreaSize)
    cfi_rel_offset (r5, StubCalleeSaveAreaSize+4)
    cfi_rel_offset (r6, StubCalleeSaveAreaSize+8)
    cfi_rel_offset (r7, StubCalleeSaveAreaSize+12)
    cfi_rel_offset (r8, StubCalleeSaveAreaSize+16)
    cfi_rel_offset (r9, StubCalleeSaveAreaSize+20)
    cfi_rel_offset (r10, StubCalleeSaveAreaSize+24)

    // save double arg registers
    vstr  d0, [sp, #StubCalleeSaveAreaSize+32]
    vstr  d1, [sp, #StubCalleeSaveAreaSize+40]
    vstr  d2, [sp, #StubCalleeSaveAreaSize+48]
    vstr  d3, [sp, #StubCalleeSaveAreaSize+56]
    vstr  d4, [sp, #StubCalleeSaveAreaSize+64]
    vstr  d5, [sp, #StubCalleeSaveAreaSize+72]
    vstr  d6, [sp, #StubCalleeSaveAreaSize+80]
    vstr  d7, [sp, #StubCalleeSaveAreaSize+88]
    cfi_rel_offset (d0, StubCalleeSaveAreaSize+32)
    cfi_rel_offset (d1, StubCalleeSaveAreaSize+40)
    cfi_rel_offset (d2, StubCalleeSaveAreaSize+48)
    cfi_rel_offset (d3, StubCalleeSaveAreaSize+56)
    cfi_rel_offset (d4, StubCalleeSaveAreaSize+64)
    cfi_rel_offset (d5, StubCalleeSaveAreaSize+72)
    cfi_rel_offset (d6, StubCalleeSaveAreaSize+80)
    cfi_rel_offset (d7, StubCalleeSaveAreaSize+88)

    mov  r5, r0
    mov  r6, r1
    mov  r7, r2
    mov  r8, r3
    ldr  r4, [sp, #FuncAddrAndCpStacksizeOffset+4]  // cpStackSize

    bl CODE_CODEThreadStackGuardGet
    add  r0, r4, r0
    mov  r1, sp
    cmp  r1, r0
    bls  .L_stack_check_fail

    bl   MRT_GetThreadLocalData
    mov  r2, r0

    // r10 <- previous sp
    add  r10, sp, #StubFrameContextSize
    add  r10, r10, #8  // cpStakSize slot + calleeAddr slot

    // r4 <- previous sp + cpStackSize
    add  r4, r10, r4

    // save last managed context
    mov  r1, r11
    adr  r0, .L_current_pc
.L_current_pc:
    .global unwindPCForC2NStub
unwindPCForC2NStub:

    bl  MRT_SaveC2NContext
    mov  r0, #0
    bl  MRT_EnterSaferegion
    mov  r1, #0
    str  r0, [sp, #SafeStateOffset]
#ifdef __OHOS__
    bl  CODE_AddSingleModelC2NCount
#endif
    
    vldr  d0, [sp, #StubCalleeSaveAreaSize+32]
    vldr  d1, [sp, #StubCalleeSaveAreaSize+40]
    vldr  d2, [sp, #StubCalleeSaveAreaSize+48]
    vldr  d3, [sp, #StubCalleeSaveAreaSize+56]
    vldr  d4, [sp, #StubCalleeSaveAreaSize+64]
    vldr  d5, [sp, #StubCalleeSaveAreaSize+72]
    vldr  d6, [sp, #StubCalleeSaveAreaSize+80]
    vldr  d7, [sp, #StubCalleeSaveAreaSize+88]
    cfi_restore (d0)
    cfi_restore (d1)
    cfi_restore (d2)
    cfi_restore (d3)
    cfi_restore (d4)
    cfi_restore (d5)
    cfi_restore (d6)
    cfi_restore (d7)

    mov  r12, sp
    // copy arg8, arg9, arg10, ... (if existed)
.L_copy:
    cmp  r4, r10
    bls  .L_copy_end
    sub  r4, r4, #8
    ldr  r1, [r4]
    ldr  r2, [r4, #4]
    // SP is always 8 byte-aligned.
    sub  sp, sp, #8
    str  r1, [sp]
    str  r2, [sp, #4]
    b .L_copy
.L_copy_end:

    // prepare arguments for invoking target method
    mov  r0, r5
    mov  r1, r6
    mov  r2, r7
    mov  r3, r8
    ldr  r9, [r12, #FuncAddrAndCpStacksizeOffset]  // calleeAddr
    blx  r9

    // keep potential return value
    mov  r6, r0
    mov  r7, r1

    vstr  d0, [r11, #StubCalleeSaveAreaSize+32]
    vstr  d1, [r11, #StubCalleeSaveAreaSize+40]
    cfi_rel_offset(d0, StubCalleeSaveAreaSize+32)
    cfi_rel_offset(d1, StubCalleeSaveAreaSize+40)

    // restoring the SP Value. the stack which extended for invoking c method is useless now.
#ifdef __OHOS__
    bl   CODE_DecSingleModelC2NCount
#endif
    mov  sp, r11
    cfi_def_cfa_register (sp)

    ldr  r0, [r11, #SafeStateOffset]
    ldr  r1, [r11, #SafeStateOffset+4]
    cmp  r0, #0
    beq .L_none_leave
    bl  MRT_LeaveSaferegion
.L_none_leave:
    bl  MRT_GetThreadLocalData
    bl  MRT_DeleteC2NContext

    mov r0, #0
    bl  MRT_ThrowPendingException

#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
    .global unwindPCForC2NStubExceptionReturn
unwindPCForC2NStubExceptionReturn:
#endif

    /* set potential return value */
    mov  r0, r6
    mov  r1, r7

    vldr  d0, [sp, #StubCalleeSaveAreaSize+32]
    vldr  d1, [sp, #StubCalleeSaveAreaSize+40]
    cfi_restore (d0)
    cfi_restore (d1)

.L_stack_restore:
    // restore all used callee-saved registers
    ldr  r4, [sp, #StubCalleeSaveAreaSize]
    ldr  r5, [sp, #StubCalleeSaveAreaSize+4]
    ldr  r6, [sp, #StubCalleeSaveAreaSize+8]
    ldr  r7, [sp, #StubCalleeSaveAreaSize+12]
    ldr  r8, [sp, #StubCalleeSaveAreaSize+16]
    ldr  r9, [sp, #StubCalleeSaveAreaSize+20]
    ldr  r10, [sp, #StubCalleeSaveAreaSize+24]
    cfi_restore (r4)
    cfi_restore (r5)
    cfi_restore (r6)
    cfi_restore (r7)
    cfi_restore (r8)
    cfi_restore (r9)
    cfi_restore (r10)

    ldr  r11, [sp]
    ldr  lr, [sp, 4]
    add  sp, sp, #StubFrameContextSize
    cfi_adjust_cfa_offset (-StubFrameContextSize)
    cfi_restore (r11)
    cfi_restore (lr)
    // restore for calleeAddr and cpStackSize slot in stack
    add  sp, sp, #8
    cfi_adjust_cfa_offset (-8)
    bx lr

.L_stack_check_fail:
    bl  MCC_ThrowStackOverflowError
    b   .L_stack_restore
    .cfi_endproc
    .size CODE_MCC_C2NStub, .-CODE_MCC_C2NStub