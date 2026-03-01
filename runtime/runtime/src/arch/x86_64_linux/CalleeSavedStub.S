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

    .macro CalleeSavedRegistersStub, funcName, funcNameStub
    .text
    .align 2
    .global \funcNameStub
    .type \funcNameStub, @function
\funcNameStub:
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

    callq   \funcName
    movq    %rax, -8(%rbp)

    callq   MRT_GetThreadLocalData@PLT
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
    .size \funcNameStub, .-\funcNameStub
    .endm

.global unwindPCForC2RStubStart
unwindPCForC2RStubStart:
CalleeSavedRegistersStub MCC_NewObject CODE_MCC_NewObject
CalleeSavedRegistersStub MCC_NewWeakRefObject CODE_MCC_NewWeakRefObject
CalleeSavedRegistersStub MCC_NewFinalizer CODE_MCC_NewFinalizer
CalleeSavedRegistersStub MCC_NewObjArray CODE_MCC_NewObjArray
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
.global unwindPCForC2RStubEnd
unwindPCForC2RStubEnd:
