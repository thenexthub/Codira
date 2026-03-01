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

.macro CalleeSavedRegistersStub, funcName
    .cfi_startproc
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

    bl   _\funcName
    str  x0,  [sp, #StubCalleeSaveAreaSize+0x48]

    bl   _MRT_GetThreadLocalData
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
    ret
    .cfi_endproc
.endm

.macro CalleeSavedRegistersStubNew, funcName
    .cfi_startproc
    ldp  x29, x30, [sp], #16
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

    bl   _\funcName
    str  x0,  [sp, #StubCalleeSaveAreaSize+0x48]

    bl   _MRT_GetThreadLocalData
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
    ret
    .cfi_endproc
.endm



    .global _unwindPCForC2RStubStart
_unwindPCForC2RStubStart:

    .text
    .align 2
    .global _CODE_MCC_NewObject
_CODE_MCC_NewObject:
    CalleeSavedRegistersStubNew MCC_NewObject

    .text
    .align 2
    .global _CODE_MCC_NewWeakRefObject
_CODE_MCC_NewWeakRefObject:
    CalleeSavedRegistersStub MCC_NewWeakRefObject

    .text
    .align 2
    .global _CODE_MCC_NewFinalizer
_CODE_MCC_NewFinalizer:
    CalleeSavedRegistersStubNew MCC_NewFinalizer

    .text
    .align 2
    .global _CODE_MCC_NewObjArray
_CODE_MCC_NewObjArray:
    CalleeSavedRegistersStub MCC_NewObjArray

    .text
    .align 2
    .global _CODE_MCC_NewArray
_CODE_MCC_NewArray:
    CalleeSavedRegistersStub MCC_NewArray

    .text
    .align 2
    .global _CODE_MCC_NewArray8
_CODE_MCC_NewArray8:
    CalleeSavedRegistersStub MCC_NewArray8

    .text
    .align 2
    .global _CODE_MCC_NewArray16
_CODE_MCC_NewArray16:
    CalleeSavedRegistersStub MCC_NewArray16

    .text
    .align 2
    .global _CODE_MCC_NewArray32
_CODE_MCC_NewArray32:
    CalleeSavedRegistersStub MCC_NewArray32

    .text
    .align 2
    .global _CODE_MCC_NewArray64
_CODE_MCC_NewArray64:
    CalleeSavedRegistersStub MCC_NewArray64

    .text
    .align 2
    .global _CODE_MCC_InvokeGC
_CODE_MCC_InvokeGC:
    CalleeSavedRegistersStub MCC_InvokeGCImpl

    .text
    .align 2
    .global _CODE_MCC_FillInStackTrace
_CODE_MCC_FillInStackTrace:
    CalleeSavedRegistersStub MCC_FillInStackTraceImpl

    .text
    .align 2
    .global _CODE_MCC_DecodeStackTrace
_CODE_MCC_DecodeStackTrace:
    CalleeSavedRegistersStub MCC_DecodeStackTraceImpl

    .text
    .align 2
    .global _CODE_MCC_GetAllThreadSnapshot
_CODE_MCC_GetAllThreadSnapshot:
    CalleeSavedRegistersStub MCC_GetAllThreadSnapshotImpl

    .text
    .align 2
    .global _CODE_MCC_GetCurrentThreadSnapshot
_CODE_MCC_GetCurrentThreadSnapshot:
    CalleeSavedRegistersStub MCC_GetCurrentThreadSnapshotImpl

    .text
    .align 2
    .global _CODE_SchedulePreemptCheckStub
_CODE_SchedulePreemptCheckStub:
    CalleeSavedRegistersStub CODE_SchedulePreemptCheck

    .text
    .align 2
    .global _CODE_MCC_AcquireRawData
_CODE_MCC_AcquireRawData:
    CalleeSavedRegistersStub MCC_AcquireRawData

    .text
    .align 2
    .global _CODE_MCC_FutureWait
_CODE_MCC_FutureWait:
    CalleeSavedRegistersStub MRT_FutureWait

    .text
    .align 2
    .global _CODE_MCC_MutexLock
_CODE_MCC_MutexLock:
    CalleeSavedRegistersStub MCC_MutexLock

    .text
    .align 2
    .global _CODE_MCC_MutexLockSlowPath
_CODE_MCC_MutexLockSlowPath:
    CalleeSavedRegistersStub MCC_MutexLockSlowPath

    .text
    .align 2
    .global _CODE_MCC_MonitorWait
_CODE_MCC_MonitorWait:
    CalleeSavedRegistersStub MCC_MonitorWait

    .text
    .align 2
    .global _CODE_MCC_MultiConditionMonitorWait
_CODE_MCC_MultiConditionMonitorWait:
    CalleeSavedRegistersStub MCC_MultiConditionMonitorWait

    .text
    .align 2
    .global _CODE_MRT_Sleep
_CODE_MRT_Sleep:
    CalleeSavedRegistersStub MRT_Sleep

    .text
    .align 2
    .global _CODE_MRT_ThreadWait
_CODE_MRT_ThreadWait:
    CalleeSavedRegistersStub MRT_ThreadWait

    .text
    .align 2
    .global _CODE_MRT_ThreadResumeAndWait
_CODE_MRT_ThreadResumeAndWait:
    CalleeSavedRegistersStub MRT_ThreadResumeAndWait

    .text
    .align 2
    .global _CODE_MRT_ThreadReady
_CODE_MRT_ThreadReady:
    CalleeSavedRegistersStub MRT_ThreadReady

    .text
    .align 2
    .global _CODE_MCC_NewPinnedObject
_CODE_MCC_NewPinnedObject:
    CalleeSavedRegistersStub MCC_NewPinnedObject

    .text
    .align 2
    .global _CODE_MCC_ApplyCODEInstanceMethod
_CODE_MCC_ApplyCODEInstanceMethod:
    CalleeSavedRegistersStub MCC_ApplyCODEInstanceMethod

    .text
    .align 2
    .global _CODE_MCC_ApplyCODEStaticMethod
_CODE_MCC_ApplyCODEStaticMethod:
    CalleeSavedRegistersStub MCC_ApplyCODEStaticMethod

    .text
    .align 2
    .global _CODE_MCC_ApplyCODEGenericInstanceMethod
_CODE_MCC_ApplyCODEGenericInstanceMethod:
    CalleeSavedRegistersStub MCC_ApplyCODEGenericInstanceMethod

.text
    .align 2
    .global _CODE_MCC_ApplyCODEGenericStaticMethod
_CODE_MCC_ApplyCODEGenericStaticMethod:
    CalleeSavedRegistersStub MCC_ApplyCODEGenericStaticMethod

    .text
    .align 2
    .global _CODE_MCC_GetMethodAnnotations
_CODE_MCC_GetMethodAnnotations:
    CalleeSavedRegistersStub MCC_GetMethodAnnotations

    .text
    .align 2
    .global _CODE_MCC_GetInstanceFieldAnnotations
_CODE_MCC_GetInstanceFieldAnnotations:
    CalleeSavedRegistersStub MCC_GetInstanceFieldAnnotations

    .text
    .align 2
    .global _CODE_MCC_GetStaticFieldAnnotations
_CODE_MCC_GetStaticFieldAnnotations:
    CalleeSavedRegistersStub MCC_GetStaticFieldAnnotations

    .text
    .align 2
    .global _CODE_MCC_GetParameterAnnotations
_CODE_MCC_GetParameterAnnotations:
    CalleeSavedRegistersStub MCC_GetParameterAnnotations

    .text
    .align 2
    .global _CODE_MCC_GetTypeInfoAnnotations
_CODE_MCC_GetTypeInfoAnnotations:
    CalleeSavedRegistersStub MCC_GetTypeInfoAnnotations

    .text
    .align 2
    .global _CODE_MCC_GetInstanceFieldValue
_CODE_MCC_GetInstanceFieldValue:
    CalleeSavedRegistersStub MCC_GetInstanceFieldValue

    .text
    .align 2
    .global _CODE_MCC_GetStaticFieldValue
_CODE_MCC_GetStaticFieldValue:
    CalleeSavedRegistersStub MCC_GetStaticFieldValue

    .text
    .align 2
    .global _CODE_MCC_NewArrayGeneric
_CODE_MCC_NewArrayGeneric:
    CalleeSavedRegistersStub MCC_NewArrayGeneric

    .text
    .align 2
    .global _CODE_MCC_DumpCODEHeapData
_CODE_MCC_DumpCODEHeapData:
    CalleeSavedRegistersStub MCC_DumpCODEHeapData

    .global _unwindPCForC2RStubEnd
_unwindPCForC2RStubEnd:
