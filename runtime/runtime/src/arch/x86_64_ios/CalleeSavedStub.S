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
#define CalleeSavedRegistersStubFrameSize           (8 * 8)

////////////////////////////////////////////////////////////////////////////////
// CalleeSavedRegistersStub simply forwards arguments passed by runtime, i.e., arguments for compiled method are passed
// according to C/C++ calling convention, which usually means efficiency.
////////////////////////////////////////////////////////////////////////////////
// the frame layout of stack(growing downwards) after CalleeSavedRegistersStub frame is built looks like:
//                 |  ...         |
//                 |  ra          | return address for the caller of CalleeSavedRegistersStub
// caller rbp  --> |  rbp         |
//                 |  ...         |
//                 | ra           |
//   stub rbp  --> | caller rbp   | <==CalleeSavedRegistersStub Start
// callee register | %r15         |
//                 | %r14         |
//                 | %r13         |
//                 | %r12         |
//                 | %rbx         |
// arg0            | %rdi         |
// arg1            | %rsi         |
//                 | null         |
// 16-byte aligned | null         | <==CalleeSavedRegistersStubFrameSize, CalleeSavedRegistersStub frame ends at here

.macro CalleeSavedRegistersStub, funcName
    .cfi_startproc
    pushq   %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset %rbp, -16
    movq    %rsp, %rbp
    .cfi_def_cfa_register %rbp
    subq    $CalleeSavedRegistersStubFrameSize, %rsp

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

    callq   _\funcName
    movq    %rax, -8(%rbp)

    callq   _MRT_GetThreadLocalData
    movq    %rax,  %r15

    // Restore the value of rsp before coping arg6, arg7...
    addq    $CalleeSavedRegistersStubFrameSize, %rsp

    movq    -8(%rbp),  %rax
    movq    -16(%rbp), %r14
    movq    -24(%rbp), %r13
    movq    -32(%rbp), %r12
    movq    -40(%rbp), %rbx

    popq    %rbp
    .cfi_def_cfa %rsp, 8
    retq
    .cfi_endproc
.endm

.global _unwindPCForC2RStubStart
_unwindPCForC2RStubStart:

    .text
    .align 2
    .global _CODE_MCC_NewObject
_CODE_MCC_NewObject:
    CalleeSavedRegistersStub MCC_NewObject

    .text
    .align 2
    .global _CODE_MCC_NewWeakRefObject
_CODE_MCC_NewWeakRefObject:
    CalleeSavedRegistersStub MCC_NewWeakRefObject

    .text
    .align 2
    .global _CODE_MCC_NewFinalizer
_CODE_MCC_NewFinalizer:
    CalleeSavedRegistersStub MCC_NewFinalizer

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
