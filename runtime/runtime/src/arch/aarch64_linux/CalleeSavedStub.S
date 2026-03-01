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
// CalleeSavedRegistersStub simply forwards arguments passed by runtime, i.e., arguments for compiled method are passed
// according to C/C++ calling convention, which usually means efficiency.
////////////////////////////////////////////////////////////////////////////////

#define StubFrameContextSize      (8 * 14)
#define StubCalleeSaveAreaSize (8 * 4)

// the frame layout of stack(growing downwards) after MCC_C2NStub frame is built looks like:

// caller sp  -->  | arg7         |
// callee saved    | avalid null  |
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
//                 | x30          |
//   stub fp  -->  | caller fp    |
//                 |  ...         |
//                 | arg10        |
//                 | arg9         |
//                 | arg8         |
//   stub sp  -->  | arg7         | <== MCC_C2NStub frame ends at here

#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
    .global CalleeSavedStubFrameSize
CalleeSavedStubFrameSize:
    .quad StubFrameContextSize
#endif

    .macro CalleeSavedRegistersStub, funcName, funcNameStub
    .text
    .align 2
    .global \funcNameStub
    .type \funcNameStub, @function
\funcNameStub:
    .cfi_startproc
#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
    paciasp
#endif
    stp  x29, x30, [sp,  #-StubFrameContextSize]!
    cfi_adjust_cfa_offset (StubFrameContextSize)
    cfi_rel_offset (x29, 0)
    cfi_rel_offset (x30, 8)

    mov  x29, sp
    cfi_def_cfa_register (sp)

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

    bl   \funcName
    str  x0,  [sp, #StubCalleeSaveAreaSize+0x48]

    bl   MRT_GetThreadLocalData
    mov  x28, x0

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
    ldr  x27, [sp, #StubCalleeSaveAreaSize+0x40]
    cfi_restore (x27)
    ldr  x0,  [sp, #StubCalleeSaveAreaSize+0x48]
    cfi_restore (x0)

    ldp  x29, x30, [sp], #StubFrameContextSize
    cfi_adjust_cfa_offset (-StubFrameContextSize)
    cfi_restore (x29)
    cfi_restore (x30)
#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
    // x30 is signed in ExceptionHandling::ChangeEHStackInfoLR.
    autiasp
#endif
    ret
    .cfi_endproc
    .size \funcNameStub, .-\funcNameStub
    .endm

    .macro CalleeSavedRegistersStubNew, funcName, funcNameStub
    .text
    .align 2
    .global \funcNameStub
    .type \funcNameStub, @function
\funcNameStub:
    .cfi_startproc
    ldr x30, [sp], #16
    stp  x29, x30, [sp,  #-StubFrameContextSize]!
    cfi_adjust_cfa_offset (StubFrameContextSize)
    cfi_rel_offset (x29, 0)
    cfi_rel_offset (x30, 8)

    mov  x29, sp
    cfi_def_cfa_register (sp)

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

    bl   \funcName
    str  x0,  [sp, #StubCalleeSaveAreaSize+0x48]

    bl   MRT_GetThreadLocalData
    mov  x28, x0

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
    ldr  x27, [sp, #StubCalleeSaveAreaSize+0x40]
    cfi_restore (x27)
    ldr  x0,  [sp, #StubCalleeSaveAreaSize+0x48]
    cfi_restore (x0)

    ldp  x29, x30, [sp], #StubFrameContextSize
    cfi_adjust_cfa_offset (-StubFrameContextSize)
    cfi_restore (x29)
    cfi_restore (x30)
#if defined(ENABLE_BACKWARD_PTRAUTH_CFI)
    // x30 is signed in ExceptionHandling::ChangeEHStackInfoLR.
    autiasp
#endif
    ret
    .cfi_endproc
    .size \funcNameStub, .-\funcNameStub
    .endm


    .global unwindPCForC2RStubStart
unwindPCForC2RStubStart:
    CalleeSavedRegistersStubNew MCC_NewObject CODE_MCC_NewObject
    CalleeSavedRegistersStub MCC_NewWeakRefObject CODE_MCC_NewWeakRefObject
    CalleeSavedRegistersStub MCC_NewObjArray CODE_MCC_NewObjArray
    CalleeSavedRegistersStubNew MCC_NewFinalizer CODE_MCC_NewFinalizer
    CalleeSavedRegistersStub MCC_NewArray CODE_MCC_NewArray
    CalleeSavedRegistersStub MCC_NewArray8 CODE_MCC_NewArray8
    CalleeSavedRegistersStub MCC_NewArray16 CODE_MCC_NewArray16
    CalleeSavedRegistersStub MCC_NewArray32 CODE_MCC_NewArray32
    CalleeSavedRegistersStub MCC_NewArray64 CODE_MCC_NewArray64
    CalleeSavedRegistersStub MCC_InvokeGCImpl CODE_MCC_InvokeGC
    CalleeSavedRegistersStub MCC_FillInStackTraceImpl CODE_MCC_FillInStackTrace
    CalleeSavedRegistersStub MCC_DecodeStackTraceImpl CODE_MCC_DecodeStackTrace
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
    CalleeSavedRegistersStub MRT_ThreadWait CODE_MRT_ThreadWait
    CalleeSavedRegistersStub MRT_ThreadResumeAndWait CODE_MRT_ThreadResumeAndWait
    CalleeSavedRegistersStub MRT_ThreadReady CODE_MRT_ThreadReady
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
