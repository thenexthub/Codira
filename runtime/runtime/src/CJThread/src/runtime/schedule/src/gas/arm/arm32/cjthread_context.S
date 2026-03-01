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

.macro ContextSave context
    str r4, [\context, #CONTEXT_R4]
    str r5, [\context, #CONTEXT_R5]
    str r6, [\context, #CONTEXT_R6]
    str r7, [\context, #CONTEXT_R7]
    str r8, [\context, #CONTEXT_R8]
    str r9, [\context, #CONTEXT_R9]
    str r10, [\context, #CONTEXT_R10]
    str r11, [\context, #CONTEXT_R11FP]
    str sp, [\context, #CONTEXT_R13SP]
    str lr, [\context, #CONTEXT_R14LR]
    str lr, [\context, #CONTEXT_R15PC]

#ifndef SCHEDULE_SOFT_FLOAT
    vstr  d8, [\context, #CONTEXT_ARM32_D8]
    vstr  d9, [\context, #CONTEXT_ARM32_D9]
    vstr  d10, [\context, #CONTEXT_ARM32_D10]
    vstr  d11, [\context, #CONTEXT_ARM32_D11]
    vstr  d12, [\context, #CONTEXT_ARM32_D12]
    vstr  d13, [\context, #CONTEXT_ARM32_D13]
    vstr  d14, [\context, #CONTEXT_ARM32_D14]
    vstr  d15, [\context, #CONTEXT_ARM32_D15]

    vmrs r12, fpscr
    str r12, [\context, #CONTEXT_FPSCR]
#endif
.endm

.macro ContextResume context
    ldr r4, [\context, #CONTEXT_R4]
    ldr r5, [\context, #CONTEXT_R5]
    ldr r6, [\context, #CONTEXT_R6]
    ldr r7, [\context, #CONTEXT_R7]
    ldr r8, [\context, #CONTEXT_R8]
    ldr r9, [\context, #CONTEXT_R9]
    ldr r10, [\context, #CONTEXT_R10]
    ldr r11, [\context, #CONTEXT_R11FP]
    ldr lr, [\context, #CONTEXT_R14LR]
    ldr r3, [\context, #CONTEXT_R15PC]

    ldr sp, [\context, #CONTEXT_R13SP]

#ifndef SCHEDULE_SOFT_FLOAT
    ldr r12, [\context, #CONTEXT_FPSCR]
    vmsr fpscr, r12

    vldr d8, [\context, #CONTEXT_ARM32_D8]
    vldr d9, [\context, #CONTEXT_ARM32_D9]
    vldr d10, [\context, #CONTEXT_ARM32_D10]
    vldr d11, [\context, #CONTEXT_ARM32_D11]
    vldr d12, [\context, #CONTEXT_ARM32_D12]
    vldr d13, [\context, #CONTEXT_ARM32_D13]
    vldr d14, [\context, #CONTEXT_ARM32_D14]
    vldr d15, [\context, #CONTEXT_ARM32_D15]
#endif

    bx r3
.endm

.macro Context0Reset context
    ldr sp, [\context, #CONTEXT_R13SP]

    mov lr, #0

#ifndef SCHEDULE_SOFT_FLOAT
    ldr r12, [\context, #CONTEXT_FPSCR]
    vmsr fpscr, r12
#endif
.endm

/* Rebind the codethread to the thread */
.macro CODEThreadRebind curCODEThread, dstCODEThread, thread, g_codethread
    push {r9}
    mov r9, #0
    str r9, [\curCODEThread, #ARM32_CODETHREAD_THREAD_OFFSET]             /* curCODEThread->thread = nullptr */
    str \thread, [\dstCODEThread, #ARM32_CODETHREAD_THREAD_OFFSET]         /* dstCODEThread->thread = thread */
    str \dstCODEThread, [\thread, #ARM32_THREAD_CODETHREAD_OFFSET]         /* thread->codethread = dstCODEThread */
    str \dstCODEThread, [\g_codethread]                                    /* g_codethread = dstCODEThread */
    pop {r9}
.endm

/* Go to codethread0 and run the func function.
 * sp points to codethread0 and pc points to func.
 * void CODEThreadMcall(void *func,&g_codethread);
 */
.global CODEThreadMcall
CODEThreadMcall:
    mov r3, r0
    ldr r0, [r1]
    add r2, r0, #ARM32_CODETHREAD_CONTEXT_OFFSET
    ContextSave r2

    push {r4}
    ldr r4, [r0, #ARM32_CODETHREAD_THREAD_OFFSET]    /* thread = codethread-> thread*/
    ldr r12, [r4, #ARM32_THREAD_CODETHREAD0_OFFSET]    /*  codethread0 = thread->codethread0 */
    CODEThreadRebind r0, r12, r4, r1
    pop {r4}

    add r2, r12, #ARM32_CODETHREAD_CONTEXT_OFFSET
    Context0Reset r2

    bx r3

    .type CODEThreadMcall,%function
    .size CODEThreadMcall,.-CODEThreadMcall

/* Go to codethread0 and execute the func function. Unlike the mcall function, this interface
 * does not save the context of the current codethread. Currently, this API is used only for
 * codethread_exit.
 * void CODEThreadMcallNosave(void *func,&g_codethread)
 */
.global CODEThreadMcallNosave
CODEThreadMcallNosave:
    mov r3, r0
    ldr r0, [r1]

    ldr r2, [r0, #ARM32_CODETHREAD_THREAD_OFFSET]
    ldr r12, [r2, #ARM32_THREAD_CODETHREAD0_OFFSET]
    CODEThreadRebind r0, r12, r2, r1

    add r2, r12, #ARM32_CODETHREAD_CONTEXT_OFFSET
    Context0Reset r2

    bx r3

    .type CODEThreadMcallNosave,%function
    .size CODEThreadMcallNosave,.-CODEThreadMcallNosave

.global CODE_CODEThreadExecute
CODE_CODEThreadExecute:
    ldr r12, [r1]
    ldr r3, [r12, #ARM32_CODETHREAD_THREAD_OFFSET]
    CODEThreadRebind r12, r0, r3, r1

    add r2, r0, #ARM32_CODETHREAD_CONTEXT_OFFSET
    ContextResume r2

    .type CODEThreadExecute,%function
    .size CODEThreadExecute,.-CODEThreadExecute

#ifdef __OHOS__
/* This interface is used to save the sp value of the UI thread when a user executes the
 * codethread of spawn(Main) in the UI thread.
 */
.global SingleCODEThreadStoreSP
SingleCODEThreadStoreSP:
    push {r11, lr}

    mov r0, sp
    add r0, r0, #8
    bl StoreNativeSPForUIThread
    pop {r11, pc}

    .type SingleCODEThreadStoreSP,%function
    .size SingleCODEThreadStoreSP,.-SingleCODEThreadStoreSP

#endif

/*
 * void CODEThreadContextInit(struct CODEThreadContext *context, void *func, char *stackBaseAddr)
 */
.global CODEThreadContextInit
CODEThreadContextInit:
    str r1, [r0, #CONTEXT_R15PC]
    str r2, [r0, #CONTEXT_R13SP]

#ifndef SCHEDULE_SOFT_FLOAT
    vmrs r3, fpscr
    str r3, [r0, #CONTEXT_FPSCR]
#endif
    bx lr
    .type CODEThreadContextInit,%function
    .size CODEThreadContextInit,.-CODEThreadContextInit

/* Obtains the current context.
 * void CODEThreadContextGet(struct CODEThreadContext *context);
 */
.global CODEThreadContextGet
CODEThreadContextGet:
    ContextSave r0
    bx lr

    .type CODEThreadContextGet,%function
    .size CODEThreadContextGet,.-CODEThreadContextGet

/* Set the current context.
 * void CODEThreadContextSet(struct CODEThreadContext *context);
 */
.global CODEThreadContextSet
CODEThreadContextSet:
    ContextResume r0

    .type CODEThreadContextSet,%function
    .size CODEThreadContextSet,.-CODEThreadContextSet