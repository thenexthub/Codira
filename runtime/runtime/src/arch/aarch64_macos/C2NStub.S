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

#define StubFrameContextSize          (8 * 32)
#define StubCalleeSaveAreaSize        (8 * 4)
#define FuncAddrAndCpStacksizeOffset  (8 * 2)
#define SafeStateOffset               (8 * 2)

// R means runtime, while C means compiled method. XX indicates the return type of this method.

// On execution of "bl MCC_C2NStub", the frame layout of stack(growing downwards) looks like:
// x0~x7: hold the first 8 arguments arg0~arg7 if existed
// x30: return address of "bl MCC_C2NStub"
// callee funcAddr and cpStackSize are saved on the stack
// all on-stack arguments are addressable by SP as the frame layout shows.
//                 | ...          |
//                 | x30          | lr for the caller of MCC_C2NStub
// caller fp  -->  | x29          |
//                 | ...          |
//                 | arg11        |
//                 | arg10        |
//                 | arg9         |
//                 | arg8         |
//                 | cpStackSize  |
// caller sp  -->  | funcAddr     |

// the frame layout of stack(growing downwards) after MCC_C2NStub frame is built looks like:
//                 | ...          |
//                 | x30          | lr for the caller of MCC_C2NStub
// caller fp  -->  | x29          |
//                 | ...          |
//                 | arg11        |
//                 | arg10        |
//                 | arg9         |
// caller sp  -->  | arg8         |
// callee saved    | q7(high)     | <== MCC_C2NStub frame starts from here
//                 | q7(low)      |
//                 | q6(high)     |
//                 | q6(low)      |
//                 | q5(high)     |
//                 | q5(low)      |
//                 | q4(high)     |
//                 | q4(low)      |
//                 | q3(high)     |
//                 | q3(low)      |
//                 | q2(high)     |
//                 | q2(low)      |
//                 | q1(high)     |
//                 | q1(low)      |
//                 | q0(high)     |
//                 | q0(low)      |
//                 | x9           |
//                 | x8           |
//                 | x28          |
//                 | x27          |
//                 | x26          |
//                 | x25          |
//                 | x24          |
//                 | x23          |
//                 | x22          |
//                 | x21          |
//                 | x20          |
// callee saved    | x19          |
//                 | null         |
//                 | entersafe    |
//                 | x30          |
//   stub fp  -->  | x29 callerfp |
//                 | ...          | <== copy caller Stack start here
//                 | arg11        |
//                 | arg10        |
//                 | arg9         |
//   stub sp  -->  | arg8         | <== MCC_C2NStub frame ends at here

    .text
    .align 2
    .global _CODE_MCC_C2NStub
_CODE_MCC_C2NStub:

    // x10 = cpStackSize, x9 = calleeAddr
    ldp  x9, x10, [sp]
    add  sp, sp, #FuncAddrAndCpStacksizeOffset

    .cfi_startproc
    stp  x29, x30, [sp, #-StubFrameContextSize]!
    cfi_adjust_cfa_offset (StubFrameContextSize)
    cfi_rel_offset (x29, 0)
    cfi_rel_offset (x30, 8)

    mov  x29, sp
    cfi_def_cfa_register (x29)

    // save all used callee-saved registers.
    stp  x19, x20, [sp, #StubCalleeSaveAreaSize]
    cfi_rel_offset (x19, StubCalleeSaveAreaSize)
    cfi_rel_offset (x20, StubCalleeSaveAreaSize+8)

    stp  x21, x22, [sp, #StubCalleeSaveAreaSize+0x10]
    cfi_rel_offset (x21, StubCalleeSaveAreaSize+0x10)
    cfi_rel_offset (x22, StubCalleeSaveAreaSize+0x18)

    stp  x23, x24, [sp, #StubCalleeSaveAreaSize+0x20]
    cfi_rel_offset (x23, StubCalleeSaveAreaSize+0x20)
    cfi_rel_offset (x24, StubCalleeSaveAreaSize+0x28)

    stp  x25, x26, [sp, #StubCalleeSaveAreaSize+0x30]
    cfi_rel_offset (x25, StubCalleeSaveAreaSize+0x30)
    cfi_rel_offset (x26, StubCalleeSaveAreaSize+0x38)

    stp  x27, x28, [sp, #StubCalleeSaveAreaSize+0x40]
    cfi_rel_offset (x27, StubCalleeSaveAreaSize+0x40)
    cfi_rel_offset (x28, StubCalleeSaveAreaSize+0x48)

    stp  x8, x9, [sp, #StubCalleeSaveAreaSize+0x50]
    cfi_rel_offset (x8, StubCalleeSaveAreaSize+0x50)
    cfi_rel_offset (x9, StubCalleeSaveAreaSize+0x58)

    stp  q0, q1, [sp, #StubCalleeSaveAreaSize+0x60]
    cfi_rel_offset (q0, StubCalleeSaveAreaSize+0x60)
    cfi_rel_offset (q1, StubCalleeSaveAreaSize+0x70)

    stp  q2, q3, [sp, #StubCalleeSaveAreaSize+0x80]
    cfi_rel_offset (q2, StubCalleeSaveAreaSize+0x80)
    cfi_rel_offset (q3, StubCalleeSaveAreaSize+0x90)

    stp  q4, q5, [sp, #StubCalleeSaveAreaSize+0xa0]
    cfi_rel_offset (q4, StubCalleeSaveAreaSize+0xa0)
    cfi_rel_offset (q5, StubCalleeSaveAreaSize+0xb0)

    stp  q6, q7, [sp, #StubCalleeSaveAreaSize+0xc0]
    cfi_rel_offset (q6, StubCalleeSaveAreaSize+0xc0)
    cfi_rel_offset (q7, StubCalleeSaveAreaSize+0xd0)

    mov  x20, x0
    mov  x21, x1
    mov  x22, x2
    mov  x23, x3
    mov  x24, x4
    mov  x25, x5
    mov  x26, x6
    mov  x27, x7
    mov  x19, x10

    bl _CODE_CODEThreadStackGuardGet
    // x19 = cpStackSize
    add x0, x19, x0
    mov x1, sp
    cmp x1, x0
    ble .L_stack_check_fail

    mov  x2, x28

    // x28 <- previous sp
    add  x28, sp, #StubFrameContextSize

    // x19 <- previous sp + cpStackSize
    add  x19, x28, x19

    // save last managed context
    mov  x1, x29
    adr  x0, #4
    .global _unwindPCForC2NStub
_unwindPCForC2NStub:

    bl   _MRT_SaveC2NContext
    mov  x0, #0
    bl   _MRT_EnterSaferegion
    mov  x1, #0
    stp  x0, x1, [sp, #SafeStateOffset]


    ldp  x8, x9, [sp, #StubCalleeSaveAreaSize+0x50]
    cfi_restore (x8)
    cfi_restore (x9)

    ldp  q0, q1, [sp, #StubCalleeSaveAreaSize+0x60]
    cfi_restore (q0)
    cfi_restore (q1)

    ldp  q2, q3, [sp, #StubCalleeSaveAreaSize+0x80]
    cfi_restore (q2)
    cfi_restore (q3)

    ldp  q4, q5, [sp, #StubCalleeSaveAreaSize+0xa0]
    cfi_restore (q4)
    cfi_restore (q5)

    ldp  q6, q7, [sp, #StubCalleeSaveAreaSize+0xc0]
    cfi_restore (q6)
    cfi_restore (q7)

    // copy arg8, arg9, arg10, ... (if existed)
.L_copy:
    cmp x19, x28
    ble .L_copy_end
    ldp x1, x2, [x19, #-16]!
    // SP is always 16 byte-aligned.
    stp x1, x2, [sp,  #-16]!
    b .L_copy
.L_copy_end:

    // prepare arguments for invoking target method
    mov  x0, x20
    mov  x1, x21
    mov  x2, x22
    mov  x3, x23
    mov  x4, x24
    mov  x5, x25
    mov  x6, x26
    mov  x7, x27
    blr  x9

    /* keep potential return value */
    mov  x21, x0
    mov  x22, x1
    mov  x23, x2
    mov  x24, x3
    mov  x25, x8

    // restoring the SP Value. the stack which extended for invoking c method is useless now.
    mov  sp,  x29
    cfi_def_cfa_register (sp)

    stp  d0, d1, [sp, #StubCalleeSaveAreaSize+0x60]
    cfi_rel_offset (d0, StubCalleeSaveAreaSize+0x60)
    cfi_rel_offset (d1, StubCalleeSaveAreaSize+0x68)

    stp  d2, d3, [sp, #StubCalleeSaveAreaSize+0x70]
    cfi_rel_offset (d2, StubCalleeSaveAreaSize+0x70)
    cfi_rel_offset (d3, StubCalleeSaveAreaSize+0x78)

    ldp x0, x1, [x29, #SafeStateOffset]
    cmp x0, #0
    beq .L_none_leave
    bl  _MRT_LeaveSaferegion
.L_none_leave:
    bl  _MRT_GetThreadLocalData
    str x0, [sp, #StubCalleeSaveAreaSize+0x48]
    bl  _MRT_DeleteC2NContext

    mov x0, #0
    bl  _MRT_ThrowPendingException

    /* set potential return value */
    mov  x0, x21
    mov  x1, x22
    mov  x2, x23
    mov  x3, x24
    mov  x8, x25

    ldp  d0, d1, [sp, #StubCalleeSaveAreaSize+0x60]
    cfi_restore (d0)
    cfi_restore (d1)

    ldp  d2, d3, [sp, #StubCalleeSaveAreaSize+0x70]
    cfi_restore (d2)
    cfi_restore (d3)

.L_stack_restore:
    // restore all used callee-saved registers.
    ldp  x19, x20, [sp, #StubCalleeSaveAreaSize]
    cfi_restore (x19)
    cfi_restore (x20)
    ldp  x21, x22, [sp, #StubCalleeSaveAreaSize+0x10]
    cfi_restore (x21)
    cfi_restore (x22)
    ldp  x23, x24, [sp, #StubCalleeSaveAreaSize+0x20]
    cfi_restore (x23)
    cfi_restore (x24)
    ldp  x25, x26, [sp, #StubCalleeSaveAreaSize+0x30]
    cfi_restore (x25)
    cfi_restore (x26)
    ldp  x27, x28, [sp, #StubCalleeSaveAreaSize+0x40]
    cfi_restore (x27)
    cfi_restore (x28)

    ldp  x29, x30, [sp], #StubFrameContextSize
    cfi_adjust_cfa_offset (-StubFrameContextSize)
    cfi_restore (x29)
    cfi_restore (x30)
    ret
.L_stack_check_fail:
    bl  _MCC_ThrowStackOverflowError
    bl  _MRT_GetThreadLocalData
    str x0, [sp, #StubCalleeSaveAreaSize+0x48]
    b   .L_stack_restore
    .cfi_endproc
