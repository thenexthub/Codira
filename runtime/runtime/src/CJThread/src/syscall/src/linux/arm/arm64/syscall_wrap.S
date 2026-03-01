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
#include "schedule_rename.h"

.text
/* Align specifies the byte alignment of the instruction. The CPU is faster when accessing
 * instructions with even addresses. Note that the semantics of this statement are different
 * between ARM and x86, where ARM represents alignment to the nth power of 2.
 * In x86, it is aligned by n bytes.
 */
.align 2

/* int Syscall3(num, arg1, arg2, arg3)
                 w0   x1    x2    x3
*/
.global Syscall3
Syscall3:
    uxtw x8, w0                 /* Expand syscall num to an 8-byte size and place it in an x8 register */
    stp x29, x30, [sp, #-16]!   /* Save fp and lr to the stack */
                                /*                                
                                 *         ------------------------
                                 * sp - 48 |        x2            |
                                 *         ------------------------
                                 * sp - 40 |        x3            |
                                 *         ------------------------
                                 * sp - 32 |        x8            |
                                 *         ------------------------
                                 * sp - 24 |        x1 /x0        |
                                 *         ------------------------
                                 * sp - 16 |        x29           |
                                 *         ------------------------
                                 * sp - 8  |        x30           | 
                                 *         ------------------------
                                 */
    stp x8, x1, [sp, #-16]!     /* Save all registers as the next step is to enter SyscallEnter */
    stp x2, x3, [sp, #-16]!

    bl SyscallEnter

    ldp x1, x2, [sp], #16       /* Get back previously saved registers */
    ldp x8, x0, [sp], #16
    svc 0x0                     /* syscall */
    str x0, [sp, #-16]!         /* Save the return value, where -16 is required because sp can only point to multiples of 16 */
    bl SyscallExit
    ldr x0, [sp], #16           /* Get back return value */
    cmn x0, #4095               /* If there is an error, it will return a value of [-4095, -1] */
    bcc finish
    neg x0, x0
    str x0, [sp, #-16]!         /* Put the error code on the stack */

    bl __errno_location         /* Store errno address in x0 */
    ldr x1, [sp], #16           /* Put the error code into w1 */
    str w1, [x0]                /* Save the error code into the errno pointer */ 
    mov x0, #-1
finish:
    ldp x29, x30, [sp], #16
    ret
    .type Syscall3,@function
    .size Syscall3,.-Syscall3


/* int Syscall6(num, arg1, arg2, arg3, arg4, arg5, arg6)
                 w0   x1    x2    x3    x4    x5    x6
*/
.global Syscall6
Syscall6:
    uxtw x8, w0
    stp x29, x30, [sp, #-16]!
    stp x8, x1, [sp, #-16]!
    stp x2, x3, [sp, #-16]!
    stp x4, x5, [sp, #-16]!
    str x6, [sp, #-16]!

    bl SyscallEnter

    ldr x5, [sp], #16
    ldp x3, x4, [sp], #16
    ldp x1, x2, [sp], #16
    ldp x8, x0, [sp], #16

    svc 0x0
    str x0, [sp, #-16]!         /* Save return value */
    bl SyscallExit
    ldr x0, [sp], #16

    cmn x0, #4095
    bcc finish6
    neg x0, x0
    str x0, [sp, #-16]!         /* Put the error code on the stack */

    bl __errno_location         /* Store errno address in x0 */
    ldr x1, [sp], #16           /* Put the error code into w1 */
    str w1, [x0]                /* Save the error code into the errno pointer */ 
    mov x0, #-1
finish6:
    ldp x29, x30, [sp], #16
    ret
    .type Syscall6,@function
    .size Syscall6,.-Syscall6