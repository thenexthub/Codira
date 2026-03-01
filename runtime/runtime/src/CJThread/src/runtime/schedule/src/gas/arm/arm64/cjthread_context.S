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
 * x17 is the temp register.
 */
.macro ContextSave context
    stp x18, x19, [\context, #CONTEXT_X18]
    stp x20, x21, [\context, #CONTEXT_X20]
    stp x22, x23, [\context, #CONTEXT_X22]
    stp x24, x25, [\context, #CONTEXT_X24]
    stp x26, x27, [\context, #CONTEXT_X26]
    stp x28, x29, [\context, #CONTEXT_X28]
    stp x30, x30, [\context, #CONTEXT_X30_LR]
    /* save sp */
    mov x17, sp
    str x17, [\context, #CONTEXT_SP]

#ifndef SCHEDULE_SOFT_FLOAT
    /* save float registers */
    stp d8, d9, [\context, #CONTEXT_D8]
    stp d10, d11, [\context, #CONTEXT_D10]
    stp d12, d13, [\context, #CONTEXT_D12]
    stp d14, d15, [\context, #CONTEXT_D14]
    /* save the float control register */
    mrs x17, fpcr
    str x17, [\context, #CONTEXT_FPCR]
#endif

.endm

/* macro: save current context, context is context pointer.
 * x17 and x3 are the temp register.
 */
.macro ContextResume context
    /* resume the general registers */
    ldp x18, x19, [\context, #CONTEXT_X18]
    ldp x20, x21, [\context, #CONTEXT_X20]
    ldp x22, x23, [\context, #CONTEXT_X22]
    ldp x24, x25, [\context, #CONTEXT_X24]
    ldp x26, x27, [\context, #CONTEXT_X26]
    ldp x28, x29, [\context, #CONTEXT_X28]
    ldp x30, x3, [\context, #CONTEXT_X30_LR]    /* x3 temporarily stores pc to be restored. */

    /* resume sp */
    ldr x17, [\context, #CONTEXT_SP]
    mov sp, x17

#ifndef SCHEDULE_SOFT_FLOAT
    /* resume float context */
    ldr x17, [\context, #CONTEXT_FPCR]
    msr fpcr, x17
    /* resume float registers */
    ldp d8, d9, [\context, #CONTEXT_D8]
    ldp d10, d11, [\context, #CONTEXT_D10]
    ldp d12, d13, [\context, #CONTEXT_D12]
    ldp d14, d15, [\context, #CONTEXT_D14]
#endif

    br x3                                       /* jump tp pc */
.endm

/* reset codethread0 context */
.macro Context0Reset context
    /* Read sp pointer of codethread0. Note that after codethread0 is initialized, the SP stored in
     * the structure should not be modified. Some general purpose registers do not need to be
     * restored when restoring to the context of codethread0.
     */
    ldr x17, [\context, #CONTEXT_SP]
    mov sp, x17

    /* set lr register to 0 */
    mov x30, xzr

#ifndef SCHEDULE_SOFT_FLOAT
    /* Only the float context is restored. Float registers are not restored. */
    ldr x17, [\context, #CONTEXT_FPCR]
    msr fpcr, x17
#endif

.endm

/* Rebind the codethread to the thread */
.macro CODEThreadRebind curCODEThread, dstCODEThread, thread, g_codethread
    str xzr, [\curCODEThread, #CODETHREAD_THREAD_OFFSET]             /* curCODEThread->thread = nullptr */
    str \thread, [\dstCODEThread, #CODETHREAD_THREAD_OFFSET]         /* dstCODEThread->thread = thread */
    str \dstCODEThread, [\thread, #THREAD_CODETHREAD_OFFSET]         /* thread->codethread = dstCODEThread */
    str \dstCODEThread, [\g_codethread]                              /* g_codethread = dstCODEThread */
.endm


/* Go to codethread0 and run the func function.
 * sp points to codethread0 and pc points to func.
 * void CODEThreadMcall(void *func,&g_codethread);
 *                       x0           x1
 */
#ifdef MRT_MACOS
.global _CODE_CODEThreadMcall
_CODE_CODEThreadMcall:
#else
.global CODEThreadMcall
CODEThreadMcall:
#endif

    /* x0 is codethread, x1 is &g_codethread, x2 is &context, x3 is func */
    mov x3, x0
    ldr x0, [x1]                                /* load g_codethread, codethread = g_codethread */

    /* save current context, x2 is codethread.context */
    add x2, x0, #CODETHREAD_CONTEXT_OFFSET
    ContextSave x2

    /* x0 is codethread，x1 is &g_codethread，x4 is thread，x5 is codethread0 */
    ldr x4, [x0, #CODETHREAD_THREAD_OFFSET]      /* thread = codethread->thread */
    ldr x5, [x4, #THREAD_CODETHREAD0_OFFSET]     /* codethread0 = thread->codethread0 */
    /* rebind codethread0 and thread, unbind the origin codethread from thread */
    CODEThreadRebind x0, x5, x4, x1

    /* switch to codethread0 context, x2 is the pointer of &codethread0->context */
    add x2, x5, #CODETHREAD_CONTEXT_OFFSET
    Context0Reset x2

    /* Direct redirection without saving the return address. The address of the
     * context that needs to be restored is stored in context_lr.
     */
    br x3

#ifdef MRT_LINUX
    .type CODEThreadMcall,@function
    .size CODEThreadMcall,.-CODEThreadMcall
#endif

/* Go to codethread0 and execute the func function. Unlike the mcall function, this interface
 * does not save the context of the current codethread. Currently, this API is used only for
 * codethread_exit.
 * void CODEThreadMcallNosave(void *func,&g_codethread)
 *                                x0          x1
 */
 #ifdef MRT_MACOS
.global _CODE_CODEThreadMcallNosave
_CODE_CODEThreadMcallNosave:
#else
.global CODEThreadMcallNosave
CODEThreadMcallNosave:
#endif

    /* x0 is codethread, x1 is &g_codethread, x3 is func */
    mov x3, x0
    ldr x0, [x1]                    /* load g_codethread, codethread = g_codethread */

    /* x0 is codethread, x1 is &g_codethread, x4 is thread, x5 is codethread0 */
    ldr x4, [x0, #CODETHREAD_THREAD_OFFSET]      /* thread = codethread->thread */
    ldr x5, [x4, #THREAD_CODETHREAD0_OFFSET]     /* codethread0 = thread->codethread0 */
    /* rebind codethread0 and thread, unbind the origin codethread from thread */
    CODEThreadRebind x0, x5, x4, x1

    /* switch to codethread0 context, x2 is the pointer of &codethread0->context */
    add x2, x5, #CODETHREAD_CONTEXT_OFFSET
    Context0Reset x2

    /* Direct redirection without saving the return address. The address of the
     * context that needs to be restored is stored in context_lr.
     */
    br x3

#ifdef MRT_LINUX
    .type CODEThreadMcallNosave,@function
    .size CODEThreadMcallNosave,.-CODEThreadMcallNosave
#endif

/* This interface is used to switch to user codethread in codethread0, so there is no need
 * to save the current context.
 * void CODEThreadExecute(void *nextCODEThread, &g_codethread);
 *                               x0                x1
 */
 #ifdef MRT_MACOS
.global _CODE_CODEThreadExecute
_CODE_CODEThreadExecute:
#else
.global CODEThreadExecute
CODEThreadExecute:
#endif

    ldr x5, [x1]                    /* load g_codethread, codethread0 = g_codethread */

    /* x0 is codethread, x1 is &g_codethread, x4 is thread, x5 is codethread0 */
    ldr x4, [x5, #CODETHREAD_THREAD_OFFSET]      /* thread = codethread0->thread */
    CODEThreadRebind x5, x0, x4, x1

    add x2, x0, #CODETHREAD_CONTEXT_OFFSET       /* x2 = &codethread->context */
    ContextResume x2

#ifdef MRT_LINUX
    .type CODEThreadExecute,@function
    .size CODEThreadExecute,.-CODEThreadExecute
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
    stp  x29, x30, [sp, #-16]!
    mov x0, sp
    add x0, x0, #16
    bl  StoreNativeSPForUIThread
    ldp x29, x30, [sp], #16
    ret
#ifdef MRT_LINUX
    .type SingleCODEThreadStoreSP,@function
    .size SingleCODEThreadStoreSP,.-SingleCODEThreadStoreSP
#endif
#endif

/*
 * void CODEThreadContextInit(struct CODEThreadContext *context, void *func, char *stackBaseAddr)                                        x0                 x1                  x2
 */
 #ifdef MRT_MACOS
.global _CODE_CODEThreadContextInit
_CODE_CODEThreadContextInit:
#else
.global CODEThreadContextInit
CODEThreadContextInit:
#endif

    str x1, [x0, #CONTEXT_PC]       /* Store the address that is about to be redirected */
    str x2, [x0, #CONTEXT_SP]       /* Initialize the stack address. */

#ifndef SCHEDULE_SOFT_FLOAT
    mrs x4, fpcr
    str x4, [x0, #CONTEXT_FPCR]
#endif

    ret

#ifdef MRT_LINUX
    .type CODEThreadContextInit,@function
    .size CODEThreadContextInit,.-CODEThreadContextInit
#endif

/* Obtains the current context.
 * void CODEThreadContextGet(struct CODEThreadContext *context);
 *                                              x0
 */
#ifdef MRT_MACOS
.global _CODE_CODEThreadContextGet
_CODE_CODEThreadContextGet:
#else
.global CODEThreadContextGet
CODEThreadContextGet:
#endif

    ContextSave x0
    ret

#ifdef MRT_LINUX
    .type CODEThreadContextGet,@function
    .size CODEThreadContextGet,.-CODEThreadContextGet
#endif

#ifdef MRT_MACOS
.global _CODE_CODEThreadContextSet
_CODE_CODEThreadContextSet:
#else
.global CODEThreadContextSet
CODEThreadContextSet:
#endif

    ContextResume x0

#ifdef MRT_LINUX
    .type CODEThreadContextSet,@function
    .size CODEThreadContextSet,.-CODEThreadContextSet
#endif
