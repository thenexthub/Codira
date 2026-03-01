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

#define StubFrameContextSize      (4 * 12)
#define StubCalleeSaveAreaSize    (4 * 4)

// caller sp  -->  | arg7         |
//                 | null         |
// callee saved    | r10          |
//                 | r9           |
//                 | r8           |
//                 | r7           |
//                 | r6           |
//                 | r5           |
//                 | null         |
//                 | null         |
// callee saved    | r4           |
//                 | lr           |
//   stub fp  -->  | caller fp    |

#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
    .global CalleeSavedStubFrameSize
CalleeSavedStubFrameSize:
    .word StubFrameContextSize
#endif

    .macro CalleeSavedRegistersStub, funcName, funcNameStub
    .text
    .p2align 2
    .global \funcNameStub
    .type \funcNameStub, %function
\funcNameStub:
    .cfi_startproc

    sub  sp, sp, #StubFrameContextSize
    str  r11, [sp]
    str  lr, [sp, #4]
    cfi_adjust_cfa_offset (StubFrameContextSize)
    cfi_rel_offset (r11, 0)
    cfi_rel_offset (lr, 4)

    mov  r11, sp
    cfi_def_cfa_register (sp)

    // save all used callee-saved registers.
    str  r4, [sp, #StubCalleeSaveAreaSize]
    str  r5, [sp, #StubCalleeSaveAreaSize+4]
    str  r6, [sp, #StubCalleeSaveAreaSize+8]
    str  r7, [sp, #StubCalleeSaveAreaSize+12]
    str  r8, [sp, #StubCalleeSaveAreaSize+16]
    str  r9, [sp, #StubCalleeSaveAreaSize+20]
    str  r10, [sp, #StubCalleeSaveAreaSize+24]
    cfi_rel_offset(r4, StubCalleeSaveAreaSize)
    cfi_rel_offset(r5, StubCalleeSaveAreaSize+4)
    cfi_rel_offset(r6, StubCalleeSaveAreaSize+8)
    cfi_rel_offset(r7, StubCalleeSaveAreaSize+12)
    cfi_rel_offset(r8, StubCalleeSaveAreaSize+16)
    cfi_rel_offset(r9, StubCalleeSaveAreaSize+20)
    cfi_rel_offset(r10, StubCalleeSaveAreaSize+24)

    bl  \funcName

    // restore all used callee-saved registers.
    ldr  r4, [sp, #StubCalleeSaveAreaSize]
    ldr  r5, [sp, #StubCalleeSaveAreaSize+4]
    ldr  r6, [sp, #StubCalleeSaveAreaSize+8]
    ldr  r7, [sp, #StubCalleeSaveAreaSize+12]
    ldr  r8, [sp, #StubCalleeSaveAreaSize+16]
    ldr  r9, [sp, #StubCalleeSaveAreaSize+20]
    ldr  r10, [sp, #StubCalleeSaveAreaSize+24]
    cfi_restore(r4)
    cfi_restore(r5)
    cfi_restore(r6)
    cfi_restore(r7)
    cfi_restore(r8)
    cfi_restore(r9)
    cfi_restore(r10)

    ldr  r11, [sp]
    ldr  lr, [sp, #4]
    add  sp, sp, #StubFrameContextSize
    cfi_adjust_cfa_offset (-StubFrameContextSize)
    cfi_restore(r11)
    cfi_restore(lr)

    bx lr
    .cfi_endproc
    .size \funcNameStub, .-\funcNameStub
    .endm

#define StubFrameContextSizeForStackArgs    (4 * 18)
#define StubCalleeSaveAreaSizeForStackArgs  (4 * 10)
#define StubFrameAddrAreaSizeForStackArgs   (4 * 8)

// caller sp  -->  | arg7         |
//                 | null         |
// callee saved    | r10          |
//                 | r9           |
//                 | r8           |
//                 | r7           |
//                 | r6           |
//                 | r5           |
// callee saved    | r4           |
//                 | lr           |
//   stub fp  -->  | caller fp    |
//                 | arg11        |
//                 | arg10        |
//                 | arg9         |
//                 | arg8         |
//                 | arg7         |
//                 | arg6         |
//                 | arg5         |
//   stub sp  -->  | arg4         |

    .macro CalleeSavedRegistersAndStackArgsStub, funcName, funcNameStub
    .text
    .p2align 2
    .global \funcNameStub
    .type \funcNameStub, %function
\funcNameStub:
    .cfi_startproc

    sub  sp, sp, #StubFrameContextSizeForStackArgs
    str  r11, [sp, #StubFrameAddrAreaSizeForStackArgs]
    str  lr, [sp, #StubFrameAddrAreaSizeForStackArgs+4]
    cfi_adjust_cfa_offset (StubFrameContextSizeForStackArgs)
    cfi_rel_offset (r11, StubFrameAddrAreaSizeForStackArgs)
    cfi_rel_offset (lr, StubFrameAddrAreaSizeForStackArgs+4)

    mov  r11, sp
    cfi_def_cfa_register (sp)

    // save all used callee-saved registers.
    str  r4, [sp, #StubCalleeSaveAreaSizeForStackArgs]
    str  r5, [sp, #StubCalleeSaveAreaSizeForStackArgs+4]
    str  r6, [sp, #StubCalleeSaveAreaSizeForStackArgs+8]
    str  r7, [sp, #StubCalleeSaveAreaSizeForStackArgs+12]
    str  r8, [sp, #StubCalleeSaveAreaSizeForStackArgs+16]
    str  r9, [sp, #StubCalleeSaveAreaSizeForStackArgs+20]
    str  r10, [sp, #StubCalleeSaveAreaSizeForStackArgs+24]
    cfi_rel_offset(r4, StubCalleeSaveAreaSizeForStackArgs)
    cfi_rel_offset(r5, StubCalleeSaveAreaSizeForStackArgs+4)
    cfi_rel_offset(r6, StubCalleeSaveAreaSizeForStackArgs+8)
    cfi_rel_offset(r7, StubCalleeSaveAreaSizeForStackArgs+12)
    cfi_rel_offset(r8, StubCalleeSaveAreaSizeForStackArgs+16)
    cfi_rel_offset(r9, StubCalleeSaveAreaSizeForStackArgs+20)
    cfi_rel_offset(r10, StubCalleeSaveAreaSizeForStackArgs+24)

    // save extra parameters stored on stack. Fixed quntity is 8 temporarily.
    // r10 <- previous sp
    add  r10, sp, #StubFrameContextSizeForStackArgs
    // r4 <- caller fp
    ldr  r4, [sp, #StubFrameAddrAreaSizeForStackArgs]
    mov  r12, sp
    mov  r9, #0
.L_copy:
    cmp  r4, r10
    bls  .L_copy_end
    cmp  r9, #4
    bge  .L_copy_end
    ldr  r5, [r10]
    ldr  r6, [r10, #4]
    str  r5, [r12]
    str  r6, [r12, #4]
    add  r12, r12, #8
    add  r10, r10, #8
    add  r9, r9, #1
    b  .L_copy
.L_copy_end:

    bl  \funcName

    // restore all used callee-saved registers.
    ldr  r4, [sp, #StubCalleeSaveAreaSizeForStackArgs]
    ldr  r5, [sp, #StubCalleeSaveAreaSizeForStackArgs+4]
    ldr  r6, [sp, #StubCalleeSaveAreaSizeForStackArgs+8]
    ldr  r7, [sp, #StubCalleeSaveAreaSizeForStackArgs+12]
    ldr  r8, [sp, #StubCalleeSaveAreaSizeForStackArgs+16]
    ldr  r9, [sp, #StubCalleeSaveAreaSizeForStackArgs+20]
    ldr  r10, [sp, #StubCalleeSaveAreaSizeForStackArgs+24]
    cfi_restore(r4)
    cfi_restore(r5)
    cfi_restore(r6)
    cfi_restore(r7)
    cfi_restore(r8)
    cfi_restore(r9)
    cfi_restore(r10)

    ldr  r11, [sp, #StubFrameAddrAreaSizeForStackArgs]
    ldr  lr, [sp, #StubFrameAddrAreaSizeForStackArgs+4]
    add  sp, sp, #StubFrameContextSizeForStackArgs
    cfi_adjust_cfa_offset (-StubFrameContextSizeForStackArgs)
    cfi_restore(r11)
    cfi_restore(lr)

    bx lr
    .cfi_endproc
    .size \funcNameStub, .-\funcNameStub
    .endm

    .global unwindPCForC2RStubStart
unwindPCForC2RStubStart:
    CalleeSavedRegistersStub MCC_NewObject CODE_MCC_NewObject
    CalleeSavedRegistersStub MCC_NewWeakRefObject CODE_MCC_NewWeakRefObject
    CalleeSavedRegistersStub MCC_NewObjArray CODE_MCC_NewObjArray
    CalleeSavedRegistersStub MCC_NewFinalizer CODE_MCC_NewFinalizer
    CalleeSavedRegistersStub MCC_NewArray CODE_MCC_NewArray
    CalleeSavedRegistersStub MCC_NewArray8 CODE_MCC_NewArray8
    CalleeSavedRegistersStub MCC_NewArray16 CODE_MCC_NewArray16
    CalleeSavedRegistersStub MCC_NewArray32 CODE_MCC_NewArray32
    CalleeSavedRegistersStub MCC_NewArray64 CODE_MCC_NewArray64
    CalleeSavedRegistersStub MCC_InvokeGCImpl CODE_MCC_InvokeGC
    CalleeSavedRegistersStub MCC_FillInStackTraceImpl CODE_MCC_FillInStackTrace
    CalleeSavedRegistersAndStackArgsStub MCC_DecodeStackTraceImpl CODE_MCC_DecodeStackTrace
    CalleeSavedRegistersStub MCC_GetAllThreadSnapshotImpl CODE_MCC_GetAllThreadSnapshot
    CalleeSavedRegistersStub MCC_GetCurrentThreadSnapshotImpl CODE_MCC_GetCurrentThreadSnapshot
    CalleeSavedRegistersStub CODE_SchedulePreemptCheck CODE_SchedulePreemptCheckStub
    CalleeSavedRegistersStub MCC_AcquireRawData CODE_MCC_AcquireRawData
    CalleeSavedRegistersStub MRT_FutureWait CODE_MCC_FutureWait
    CalleeSavedRegistersStub MCC_MutexLock CODE_MCC_MutexLock
    CalleeSavedRegistersStub MCC_MutexLockSlowPath CODE_MCC_MutexLockSlowPath
    CalleeSavedRegistersStub MCC_MonitorWait CODE_MCC_MonitorWait
    CalleeSavedRegistersStub MCC_MultiConditionMonitorWait CODE_MCC_MultiConditionMonitorWait
    CalleeSavedRegistersStub MRT_Sleep CODE_MRT_Sleep
    CalleeSavedRegistersStub MCC_NewPinnedObject CODE_MCC_NewPinnedObject
    CalleeSavedRegistersStub MCC_ApplyCODEInstanceMethod CODE_MCC_ApplyCODEInstanceMethod
    CalleeSavedRegistersStub MCC_ApplyCODEStaticMethod CODE_MCC_ApplyCODEStaticMethod
    CalleeSavedRegistersStub MCC_ApplyCODEGenericInstanceMethod CODE_MCC_ApplyCODEGenericInstanceMethod
    CalleeSavedRegistersStub MCC_ApplyCODEGenericStaticMethod CODE_MCC_ApplyCODEGenericStaticMethod
    CalleeSavedRegistersStub MCC_GetMethodAnnotations CODE_MCC_GetMethodAnnotations
    CalleeSavedRegistersStub MCC_GetInstanceFieldAnnotations CODE_MCC_GetInstanceFieldAnnotations
    CalleeSavedRegistersStub MCC_GetStaticFieldAnnotations CODE_MCC_GetStaticFieldAnnotations
    CalleeSavedRegistersStub MCC_GetParameterAnnotations CODE_MCC_GetParameterAnnotations
    CalleeSavedRegistersStub MCC_GetTypeInfoAnnotations CODE_MCC_GetTypeInfoAnnotations
    CalleeSavedRegistersStub MCC_GetInstanceFieldValue CODE_MCC_GetInstanceFieldValue
    CalleeSavedRegistersStub MCC_GetStaticFieldValue CODE_MCC_GetStaticFieldValue
    CalleeSavedRegistersStub MCC_NewArrayGeneric CODE_MCC_NewArrayGeneric
    CalleeSavedRegistersStub MCC_DumpCODEHeapData CODE_MCC_DumpCODEHeapData
#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
    CalleeSavedRegistersStub MCC_ThrowException CODE_MCC_ThrowException
    CalleeSavedRegistersStub MCC_ThrowStackOverflowError CODE_MCC_ThrowStackOverflowError
#endif
    .global unwindPCForC2RStubEnd
unwindPCForC2RStubEnd: