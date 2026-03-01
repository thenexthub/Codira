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
// MCC_N2CStub simply forwards arguments passed by runtime, i.e., arguments for compiled method are passed
// according to C/C++ calling convention, which usually means efficiency.
////////////////////////////////////////////////////////////////////////////////

#define StubFrameContextSize          (4 * 38 - 4 * 2)
#define StubCalleeSaveAreaSize        (4 * 14)
#define FuncAddrAndCpStacksizeOffset  (4 * 12)
#define SafeStateOffset               (4 * 8)
#define ThreadLocalDataOffset         (4 * 10)

// R means runtime, while C means compiled method. XX indicates the return type of this method.

// On execution of "bl MCC_N2CStub", the frame layout of stack(growing downwards) looks like:
// x0~x7: hold the first 8 arguments arg0~arg7 if existed
// x30: return address of "bl MCC_N2CStub"
// funcAddr and cpStackSize are saved on the stack
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
//                 | current sp   |
//                 | threadData   |
//                 | dirctCall    |
//                 | entersafe    |
// unwind context  | direct call  | directly invoke callee method
//                 | shadowframe  | the information of caller frame which is interpreted
//                 | UC Status    | unwind context status of caller frame
//                 | Context LR   | LR of unwind context frame
//                 | Context FP   | FP of unwind context frame
// unwind context  | Context PC   | PC of unwind context frame
//                 | lr           |
//   stub fp  -->  | r11 callerfp |
//                 | ...          | <== copy caller Stack start here
//                 | arg11        |
//                 | arg10        |
//                 | arg9         |
//   stub sp  -->  | arg8         | <== MCC_C2NStub frame ends at here

    .text
    .align 2
    .global CODE_MCC_N2CStub
    .type CODE_MCC_N2CStub, %function
CODE_MCC_N2CStub:
    .cfi_startproc
    add  lr, lr, #2
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

    // r10 <- previous sp
    add  r10, sp, #StubFrameContextSize
    add  r10, r10, #8  // cpStackSize slot + calleeAddr slot

    // r4 <- previous sp + cpStackSize
    add  r4, r10, r4

    mov  r11, sp
    cfi_def_cfa_register (r11)

    // store whether new CODEThread result in sp+#SafeStateOffset+0x8
    // and use it after calling Codira function.
    bl  MRT_TryNewAndRunCODEThread
    str  r0, [sp, #SafeStateOffset+4]

    // mutator mustn't be in safe region before setting context.
    // store whether leave saferegion result in sp+#SafeStateOffset
    // and use it after calling Codira function.
    bl  MRT_LeaveSaferegion
    str  r0, [sp, #SafeStateOffset]

    // frame info: tls -> stub
    mov  r0, r11
    bl  MRT_SaveTopManagedContextToN2CStub
    mov  r0, #0
    bl  MRT_SetStackGrow
    bl  MRT_GetThreadLocalData
    str  r0, [sp, #ThreadLocalDataOffset]

    ldr  r12, [sp, #FuncAddrAndCpStacksizeOffset]  // calleeAddr

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
    .global unwindPCForN2CStub
unwindPCForN2CStub:

    // keep potential return value
    mov  r6, r0
    mov  r7, r1

    vstr  d0, [r11, #StubCalleeSaveAreaSize+32]
    vstr  d1, [r11, #StubCalleeSaveAreaSize+40]
    cfi_rel_offset(d0, StubCalleeSaveAreaSize+32)
    cfi_rel_offset(d1, StubCalleeSaveAreaSize+40)

    // Restore the value od rsp before copy arg8, arg9, arg10...
    mov  sp, r11
    cfi_def_cfa_register (sp)

    mov  r0, #1
    bl  MRT_SetStackGrow

    mov  r0, r11
    bl  MRT_RestoreTopManagedContextFromN2CStub
    ldr  r0, [r11, #SafeStateOffset+4]
    cmp  r0, #0
    beq  .L_no_need_end
    bl  MRT_EndCODEThread
    cmp  r0, #1
    beq  .L_none_enter
.L_no_need_end:
    ldr  r0, [r11, #SafeStateOffset]
    cmp  r0, #0
    beq .L_none_enter
    mov  r0, #0
    bl MRT_EnterSaferegion
.L_none_enter:

    /* set potential return value */
    mov  r0, r6
    mov  r1, r7

    vldr  d0, [sp, #StubCalleeSaveAreaSize+32]
    vldr  d1, [sp, #StubCalleeSaveAreaSize+40]
    cfi_restore (d0)
    cfi_restore (d1)

    mov  sp, r11
    cfi_def_cfa_register (sp)

    // restore all used callee-saved registers.
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
    sub  lr, lr, #2
    bx  lr
    .cfi_endproc
    .size CODE_MCC_N2CStub, .-CODE_MCC_N2CStub


.text
.align 2
.global  ExecuteCodiraStub
.type    ExecuteCodiraStub, %function
ExecuteCodiraStub:
    .cfi_startproc
    sub  sp, sp, #8
    str  r11, [sp]
    str  lr, [sp, #4]
    cfi_adjust_cfa_offset (8)
    cfi_rel_offset (r11, 0)
    cfi_rel_offset (lr, 4)
    blx  r3
    ldr  r11, [sp]
    ldr  lr, [sp, #4]
    add  sp, sp, #8
    cfi_adjust_cfa_offset (-8)
    cfi_restore (r11)
    cfi_restore (lr)

    bx  lr
    .cfi_endproc
    .size ExecuteCodiraStub, .-ExecuteCodiraStub


.text
.align 2
.global  InitCODELibraryStub
.type    InitCODELibraryStub, %function
InitCODELibraryStub:
    .cfi_startproc
    sub  sp, sp, #8
    str  r11, [sp]
    str  lr, [sp, #4]
    cfi_adjust_cfa_offset (8)
    cfi_rel_offset (r11, 0)
    cfi_rel_offset (lr, 4)

    mov  r11, sp
    mov  r12, r0
    sub  sp, sp, #8
    str  r12, [sp]
    mov  r12, #0
    str  r12, [sp, #4]
    bl  CODE_MCC_N2CStub
    
    ldr  r11, [sp]
    ldr  lr, [sp, #4]
    add  sp, sp, #8
    cfi_adjust_cfa_offset (8)
    cfi_restore (r11)
    cfi_restore (lr)

    bx lr
    .cfi_endproc
    .size InitCODELibraryStub, .-InitCODELibraryStub

.text
.align 2
.global  ResolveCycleRefStub
.type    ResolveCycleRefStub, %function
// TODO
ResolveCycleRefStub:
    .cfi_startproc
    sub  sp, sp, #8
    str  r11, [sp]
    str  lr, [sp, #4]
    cfi_adjust_cfa_offset (8)
    cfi_rel_offset (r11, 0)
    cfi_rel_offset (lr, 4)

    mov  r11, sp
    mov  r12, r0
    sub  sp, sp, #8
    str  r12, [sp]
    mov  r12, #0
    str  r12, [sp, #4]
    bl  CODE_MCC_N2CStub

    ldr  r11, [sp]
    ldr  lr, [sp, #4]
    add  sp, sp, #8
    cfi_adjust_cfa_offset (8)
    cfi_restore (r11)
    cfi_restore (lr)

    bx  lr
    .cfi_endproc
    .size ResolveCycleRefStub, .-ResolveCycleRefStub

#ifdef __OHOS__
.text
.align 2
.global  CODE_MRT_ARKTS_CreateEngineStub
.type    CODE_MRT_ARKTS_CreateEngineStub, %function
CODE_MRT_ARKTS_CreateEngineStub:
    .cfi_startproc
    sub  sp, sp, #8
    str  r11, [sp]
    str  lr, [sp, #4]
    cfi_adjust_cfa_offset(8)
    cfi_rel_offset (r11, 0)
    cfi_rel_offset (lr, 4)

    mov  r11, sp
    mov  r4, sp
    bl  IsForeignThread
    cmp  r0, #0
    bne .L_no_need_switch
    bl  GetNativeSPForUIThread
    mov  sp, r0
.L_no_need_switch:
    bl  CODE_MRT_ARKTS_CreateEngine
    mov  sp, r4
    ldr  r11, [sp]
    ldr  lr, [sp, #4]
    add  sp, sp, #8
    cfi_adjust_cfa_offset (-8)
    cfi_restore (lr)
    cfi_restore (r11)

    bx lr
    .cfi_endproc
    .size CODE_MRT_ARKTS_CreateEngineStub, .-CODE_MRT_ARKTS_CreateEngineStub
#endif


.text
.align 2
.global  ApplyCodiraMethodStub
.type    ApplyCodiraMethodStub, %function
.global  ApplyCodiraMethodStubFloat32
.type    ApplyCodiraMethodStubFloat32, %function
.global  ApplyCodiraMethodStubFloat64
.type    ApplyCodiraMethodStubFloat64, %function
ApplyCodiraMethodStub:
ApplyCodiraMethodStubFloat32:
ApplyCodiraMethodStubFloat64:
    .cfi_startproc
    sub  sp, sp, #StubFrameContextSize
    str  r11, [sp]
    str  lr, [sp, #4]
    cfi_adjust_cfa_offset (StubFrameContextSize)
    cfi_rel_offset (r11, 0)
    cfi_rel_offset (lr, 4)

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

    mov  r11, sp
    cfi_def_cfa_register (r11)

    mov  r5, r0  // save args
    mov  r6, r1  // stackSize
    mov  r7, r2  // save func
    mov  r8, r3  // threadData

    // copy parameter to stack
    // size align to 16 byte.
    add  r6, r6, #(8 - 1)
    and  r6, r6, #0xFFFFFFF8

    mov  r0, r5
    add  r0, r0, #80  // 4 * 4 + 8 * 8

    mov  r1, r5
    add  r1, r1, #80
    add  r1, r1, r6

.L_copy_args:
    cmp  r1, r0
    bls .L_copy_args_end
    sub  r1, r1, #8
    ldr  r2, [r1]
    ldr  r3, [r1, #4]
    // SP is always 8 byte-aligned
    sub  sp, sp, #8
    str  r2, [sp]
    str  r3, [sp, #4]
    b .L_copy_args
.L_copy_args_end:

    // mov dst, src
    // prepare arguments for invoking target method
    ldr  r0, [r5]
    ldr  r1, [r5, #4]
    ldr  r2, [r5, #8]
    ldr  r3, [r5, #12]

    add  r5, r5, #16
    vldr  d0, [r5]
    vldr  d1, [r5, #8]
    vldr  d2, [r5, #16]
    vldr  d3, [r5, #24]
    vldr  d4, [r5, #32]
    vldr  d5, [r5, #40]
    vldr  d6, [r5, #48]
    vldr  d7, [r5, #56]

    blx  r7

    // keep potential return value
    mov  r6, r0
    mov  r7, r1
    
    vstr  d0, [r11, #StubCalleeSaveAreaSize+32]
    vstr  d1, [r11, #StubCalleeSaveAreaSize+40]
    cfi_rel_offset(d0, StubCalleeSaveAreaSize+32)
    cfi_rel_offset(d1, StubCalleeSaveAreaSize+40)

    /* set potential return value */
    mov  r0, r6
    mov  r1, r7

    vldr  d0, [r11, #StubCalleeSaveAreaSize+32]
    vldr  d1, [r11, #StubCalleeSaveAreaSize+40]
    cfi_restore (d0)
    cfi_restore (d1)

    // restoring the SP Value. the stack which extended for invoking c method is useless now
    mov  sp, r11
    cfi_def_cfa_register (sp)

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

    bx  lr
    .cfi_endproc
    .size ApplyCodiraMethodStub, .-ApplyCodiraMethodStub