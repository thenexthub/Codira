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

#include "context_offset.h"
#include "schedule_rename.h"

.text
/* align Specifies the byte alignment of the instruction. Note that the semantics of this
 * statement are different in ARM and x86. Arm indicates that the statement is aligned to
 * the nth power of 2. In x86, n-byte alignment is used. If the call or jmp destination is
 * at an odd numbered address, an additional clock cycle may be required. Note: align can
 * only ensure that the start address of a function is aligned, but cannot ensure that each
 * instruction is aligned.
 */
.align 2

/* macro: save current context, context is context pointer.
 * r11 is the temp register.
 */
.macro ContextSave context
    /* Save general registers */
    /* Note: In this case, the rsp is changed after the push rip. Therefore, rsp needs to be
     * moved back by eight bytes.
     */
    leaq 8(%rsp), %r11
    movq %r11, CONTEXT_RSP(\context)
    movq %rbp, CONTEXT_RBP(\context)
    movq %rbx, CONTEXT_RBX(\context)
    /* Note: The address (%rsp) is not directly stored in the rip register. This address refers
     * to the next instruction when the call mcall is invoked. When the call command is executed,
     * a push action is performed to store the return value.
     */
    movq (%rsp), %r11
    movq %r11, CONTEXT_RIP(\context)
    movq %r12, CONTEXT_R12(\context)
    movq %r13, CONTEXT_R13(\context)
    movq %r14, CONTEXT_R14(\context)
    movq %r15, CONTEXT_R15(\context)

#ifndef SCHEDULE_SOFT_FLOAT
    stmxcsr CONTEXT_MXCSR(\context)     /* save mxcsr */
    fstcw CONTEXT_FPU_CW(\context)      /* stores floating-point control words */
#endif

#ifdef MRT_WINDOWS
    movq %rdi, CONTEXT_RDI(\context)
    movq %rsi, CONTEXT_RSI(\context)
    movdqu %xmm6, CONTEXT_XMM6(\context)
    movdqu %xmm7, CONTEXT_XMM7(\context)
    movdqu %xmm8, CONTEXT_XMM8(\context)
    movdqu %xmm9, CONTEXT_XMM9(\context)
    movdqu %xmm10, CONTEXT_XMM10(\context)
    movdqu %xmm11, CONTEXT_XMM11(\context)
    movdqu %xmm12, CONTEXT_XMM12(\context)
    movdqu %xmm13, CONTEXT_XMM13(\context)
    movdqu %xmm14, CONTEXT_XMM14(\context)
    movdqu %xmm15, CONTEXT_XMM15(\context)

    movq %gs:0x10, %r11
    movq %r11, CONTEXT_GS_STACK_LOW(\context)
    movq %gs:0x08, %r11
    movq %r11, CONTEXT_GS_STACK_HIGH(\context)
    movq %gs:0x1478, %r11
    movq %r11, CONTEXT_GS_STACK_DEALOCATION(\context)
#endif
.endm


/* macro: Resume the context, context is context pointer
 * r11 is the temp register.
 * For windows, r10 is also the temp register.
 */
.macro ContextResume context
    /* resume general registers */
    movq CONTEXT_RSP(\context), %rsp
    movq CONTEXT_RBP(\context), %rbp
    movq CONTEXT_RBX(\context), %rbx
    movq CONTEXT_RIP(\context), %r11
    movq CONTEXT_R12(\context), %r12
    movq CONTEXT_R13(\context), %r13
    movq CONTEXT_R14(\context), %r14
    movq CONTEXT_R15(\context), %r15

#ifndef SCHEDULE_SOFT_FLOAT
    ldmxcsr CONTEXT_MXCSR(\context)
    fldcw CONTEXT_FPU_CW(\context)      /* resume floating-point control words */
#endif

#ifdef MRT_WINDOWS
    movq CONTEXT_RDI(\context), %rdi
    movq CONTEXT_RSI(\context), %rsi
    movdqu CONTEXT_XMM6(\context), %xmm6
    movdqu CONTEXT_XMM7(\context), %xmm7
    movdqu CONTEXT_XMM8(\context), %xmm8
    movdqu CONTEXT_XMM9(\context), %xmm9
    movdqu CONTEXT_XMM10(\context), %xmm10
    movdqu CONTEXT_XMM11(\context), %xmm11
    movdqu CONTEXT_XMM12(\context), %xmm12
    movdqu CONTEXT_XMM13(\context), %xmm13
    movdqu CONTEXT_XMM14(\context), %xmm14
    movdqu CONTEXT_XMM15(\context), %xmm15

    movq CONTEXT_GS_STACK_LOW(\context), %r10
    movq %r10, %gs:0x10
    movq CONTEXT_GS_STACK_HIGH(\context), %r10
    movq %r10, %gs:0x08
    movq CONTEXT_GS_STACK_DEALOCATION(\context), %r10
    movq %r10, %gs:0x1478
#endif

    jmp *%r11  /* jump to the codethread */
.endm

/* reset the context of codethread0 */
.macro Context0Reset context
    /* Restore the sp that always points to the bottom of the codethread0 stack. The stack of
     * codethread0 has been switched. rbp does not need to be restored here because func opens
     * the stack by itself.
     */
    movq CONTEXT_RSP(\context), %rsp

#ifndef SCHEDULE_SOFT_FLOAT
    ldmxcsr CONTEXT_MXCSR(\context)
    fldcw CONTEXT_FPU_CW(\context)
#endif
.endm

/* Rebind dstCODEThread and thread, unbind the origin curCODEThread from thread */
.macro CODEThreadRebind curCODEThread, dstCODEThread, thread, g_codethread
    movq $0x0, CODETHREAD_THREAD_OFFSET(\curCODEThread)          /* curCODEThread->thread = nullptr */
    movq \thread, CODETHREAD_THREAD_OFFSET(\dstCODEThread)       /* dstCODEThread->thread = thread */
    movq \dstCODEThread, THREAD_CODETHREAD_OFFSET(\thread)       /* thread->codethread = dstCODEThread */
    movq \dstCODEThread,(\g_codethread)                          /* g_codethread = dstCODEThread */
.endm

#ifdef MRT_WINDOWS
/* Go to codethread0 and run the func function.
 * sp points to codethread0 and pc points to func.
 * void CODEThreadMcall(void *func,&g_codethread);
 *                       rcx           rdx
 */
.global CODEThreadMcall
CODEThreadMcall:

    /* rcx is codethread, rdx is &g_codethread, r8 is context, r9 is func */
    movq %rcx,%r9
    movq (%rdx), %rcx                /* load g_codethread, codethread = g_codethread */
    leaq CODETHREAD_CONTEXT_OFFSET(%rcx), %r8
    /* save current context, r8 points to codethread.context */
    ContextSave %r8

    /* rcx is codethread, r8 is thread, r10 is codethread0 */
    movq CODETHREAD_THREAD_OFFSET(%rcx), %r8    /* thread = codethread->thread */
    movq THREAD_CODETHREAD0_OFFSET(%r8), %r10   /* codethread0 = thread->codethread0 */
    /* Rebind codethread0 and thread, unbind the origin codethread from thread */
    CODEThreadRebind %rcx, %r10, %r8, %rdx

    /* switch to the context of codethread0, r8 is the pointer of &codethread0->context */
    leaq CODETHREAD_CONTEXT_OFFSET(%r10), %r8
    Context0Reset %r8

    jmp *%r9                       /* jump to execute func */
#else
/* Go to codethread0 and run the func function.
 * sp points to codethread0 and pc points to func.
 * void CODEThreadMcall(void *func,&g_codethread);
 *                       rdi           rsi
 */
#ifdef MRT_MACOS
.global _CODE_CODEThreadMcall
_CODE_CODEThreadMcall:
#else
.global CODEThreadMcall
CODEThreadMcall:
#endif

    /* rdi is codethread, rsi is &g_codethread, rdx is context, r9 is func */
    movq %rdi,%r9
    movq (%rsi), %rdi                /* load g_codethread, codethread = g_codethread */
    leaq CODETHREAD_CONTEXT_OFFSET(%rdi), %rdx
    /* save current context, rdx points to codethread.context */
    ContextSave %rdx

    /* rdi is codethread, rdx is thread, rcx is codethread0 */
    movq CODETHREAD_THREAD_OFFSET(%rdi), %rdx    /* thread = codethread->thread */
    movq THREAD_CODETHREAD0_OFFSET(%rdx), %rcx   /* codethread0 = thread->codethread0 */
    /* Rebind codethread0 and thread, unbind the origin codethread from thread */
    CODEThreadRebind %rdi, %rcx, %rdx, %rsi

    /* switch to context of codethread0, rdx is the pointer of &codethread0->context */
    leaq CODETHREAD_CONTEXT_OFFSET(%rcx), %rdx
    Context0Reset %rdx

    jmp *%r9                       /* jump to execute func */

#ifdef MRT_LINUX
    .type CODEThreadMcall,@function
    .size CODEThreadMcall,.-CODEThreadMcall
#endif

#endif


#ifdef MRT_WINDOWS
/* Go to codethread0 and execute the func function. Unlike the mcall function, this interface
 * does not save the context of the current codethread. Currently, this API is used only for
 * codethread_exit.
 * void CODEThreadMcallNosave(void *func,&g_codethread)
 *                                 rcx          rdx
 */
.global CODEThreadMcallNosave
CODEThreadMcallNosave:

    /* rcx is codethread,rsi is &g_codethread,r8 is thread,r10 is codethread0 */
    movq %rcx, %r9                              /* codethread = g_codethread */
    movq (%rdx), %rcx
    movq CODETHREAD_THREAD_OFFSET(%rcx), %r8    /* thread = codethread->thread */
    movq THREAD_CODETHREAD0_OFFSET(%r8), %r10   /* codethread0 = thread->codethread0 */
    /* Rebind codethread0 and thread, unbind the origin codethread from thread */
    CODEThreadRebind %rcx, %r10, %r8, %rdx

    leaq CODETHREAD_CONTEXT_OFFSET(%r10), %r8
    Context0Reset %r8

    jmp *%r9
#else
/* Go to codethread0 and execute the func function. Unlike the mcall function, this interface
 * does not save the context of the current codethread. Currently, this API is used only for
 * codethread_exit.
 * void CODEThreadMcallNosave(void *func,&g_codethread)
 *                                rdi          rsi
 */
#ifdef MRT_MACOS
.global _CODE_CODEThreadMcallNosave
_CODE_CODEThreadMcallNosave:
#else
.global CODEThreadMcallNosave
CODEThreadMcallNosave:
#endif

    /* rdi is codethread,rsi is &g_codethread,rdx is thread,rcx is codethread0 */
    movq %rdi, %r9                              /* codethread = g_codethread */
    movq (%rsi), %rdi
    movq CODETHREAD_THREAD_OFFSET(%rdi), %rdx    /* thread = codethread->thread */
    movq THREAD_CODETHREAD0_OFFSET(%rdx), %rcx   /* codethread0 = thread->codethread0 */
    /* Rebind codethread0 and thread, unbind the origin codethread from thread */
    CODEThreadRebind %rdi, %rcx, %rdx, %rsi

    leaq CODETHREAD_CONTEXT_OFFSET(%rcx), %rdx
    Context0Reset %rdx

    jmp *%r9

#ifdef MRT_LINUX
    .type CODEThreadMcallNosave,@function
    .size CODEThreadMcallNosave,.-CODEThreadMcallNosave
#endif

#endif


#ifdef MRT_WINDOWS
/* This interface is used to switch to user codethread in codethread0, so there is no need to
 * save the current context.
 * void CODEThreadExecute(void *nextCODEThread, &g_codethread);
 *                               rcx                rdx
 */
.global CODEThreadExecute
CODEThreadExecute:

    /* rcx is nextCODEThread,rdx is &g_codethread,r9 is thread, r8 is codethread0 */
    movq (%rdx), %r8                            /* codethread0 = g_codethread */
    movq CODETHREAD_THREAD_OFFSET(%r8), %r9      /* thread = codethread0->thread */

    /* Rebind codethread and thread */
    CODEThreadRebind %r8, %rcx, %r9, %rdx

    leaq CODETHREAD_CONTEXT_OFFSET(%rcx), %r9   /* r9 is codethread->context */
    ContextResume %r9
#else
/* This interface is used to switch to user codethread in codethread0, so there is no need to
 * save the current context.
 * void CODEThreadExecute(void *nextCODEThread, &g_codethread);
 *                               rdi                rsi
 */
#ifdef MRT_MACOS
.global _CODE_CODEThreadExecute
_CODE_CODEThreadExecute:
#else
.global CODEThreadExecute
CODEThreadExecute:
#endif

    /* rdi is nextCODEThread,rsi is &g_codethread, rdx is thread, rcx is codethread0 */
    movq (%rsi), %rcx                           /* codethread0 = g_codethread */
    movq CODETHREAD_THREAD_OFFSET(%rcx), %rdx    /* thread = codethread0->thread */

    /* Rebind codethread and thread */
    CODEThreadRebind %rcx, %rdi, %rdx, %rsi

    leaq CODETHREAD_CONTEXT_OFFSET(%rdi), %rdx   /* rdx is codethread->context */
    ContextResume %rdx

#ifdef MRT_LINUX
    .type CODEThreadExecute,@function
    .size CODEThreadExecute,.-CODEThreadExecute
#endif

#endif

#ifdef __OHOS__
/* This interface is used to save the sp value of the UI thread when a user executes the
 * codethread of spawn(Main) in the UI thread.
 */
 #ifdef MRT_MACOS
.global _CODE_SingleCODEThreadStoreSP
_CODE_SingleCODEThreadStoreSP:
#else
.global SingleCODEThreadStoreSP
SingleCODEThreadStoreSP:
#endif
    movq %rsp, %rdi
    callq  StoreNativeSPForUIThread
#ifdef MRT_LINUX
    .type SingleCODEThreadStoreSP,@function
    .size SingleCODEThreadStoreSP,.-SingleCODEThreadStoreSP
#endif
#endif

#ifdef MRT_WINDOWS
/*
 * void CODEThreadContextInit(struct CODEThreadContext *context, void *func, char *stackBaseAddr)
 *                                                 rcx                rdx               r8
 */
.global CODEThreadContextInit
CODEThreadContextInit:
    /* rcx is context */
    movq %rdx, CONTEXT_RIP(%rcx)    /* context->rip = (uintptr_t)func; */
    /* In x86_64, there is a push rbp instruction at the beginning of all functions. The stack
     * address must be 16-byte aligned after push. Therefore, the stack address must be 8-byte
     * aligned. By default, the value of stackBaseAddr is 16-byte aligned. In x86_64, the
     * called function copies parameters to the end of the call stack of the calling function,
     * adjacent to the return address. Therefore, at least four parameters are reserved.
     */
    sub $0x28, %r8
    movq $0, (%r8)
    movq %r8, CONTEXT_RSP(%rcx)    /* context->rsp = (uintptr_t)stackBaseAddr - 40; */

#ifndef SCHEDULE_SOFT_FLOAT
    stmxcsr CONTEXT_MXCSR(%rcx)
    fstcw CONTEXT_FPU_CW(%rcx)
#endif
    ret
#else
/*
 * void CODEThreadContextInit(struct CODEThreadContext *context, void *func, char *stackBaseAddr)
 *                                                 rdi                rsi               rdx
 */
#ifdef MRT_MACOS
.global _CODE_CODEThreadContextInit
_CODE_CODEThreadContextInit:
#else
.global CODEThreadContextInit
CODEThreadContextInit:
#endif

    /* rdi is context */
    movq %rsi, CONTEXT_RIP(%rdi)    /* context->rip = (uintptr_t)func; */
    sub $0x8, %rdx
    movq $0x0, (%rdx)
    movq %rdx, CONTEXT_RSP(%rdi)    /* context->rsp = (uintptr_t)stackBaseAddr - 8; */

#ifndef SCHEDULE_SOFT_FLOAT
    stmxcsr CONTEXT_MXCSR(%rdi)
    fstcw CONTEXT_FPU_CW(%rdi)
#endif
    ret

#ifdef MRT_LINUX
    .type CODEThreadContextInit,@function
    .size CODEThreadContextInit,.-CODEThreadContextInit
#endif

#endif


#ifdef MRT_WINDOWS
/* Obtains the current context.
 * void CODEThreadContextGet(struct CODEThreadContext *context);
 *                                              rcx
 */
.global CODEThreadContextGet
CODEThreadContextGet:
    ContextSave %rcx
    ret
#else
#ifdef MRT_MACOS
.global _CODE_CODEThreadContextGet
_CODE_CODEThreadContextGet:
#else
.global CODEThreadContextGet
CODEThreadContextGet:
#endif
    ContextSave %rdi
    ret

#ifdef MRT_LINUX
    .type CODEThreadContextGet,@function
    .size CODEThreadContextGet,.-CODEThreadContextGet
#endif

#endif


#ifdef MRT_WINDOWS
/* Jump to the specified context.
 * void CODEThreadContextSet(struct CODEThreadContext *context);
 *                                              rcx
*/
.global CODEThreadContextSet
CODEThreadContextSet:
    ContextResume %rcx
#else
#ifdef MRT_MACOS
.global _CODE_CODEThreadContextSet
_CODE_CODEThreadContextSet:
#else
.global CODEThreadContextSet
CODEThreadContextSet:
#endif
    ContextResume %rdi

#ifdef MRT_LINUX
    .type CODEThreadContextSet,@function
    .size CODEThreadContextSet,.-CODEThreadContextSet
#endif

#endif
