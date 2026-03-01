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
                rdi   rsi   rdx   rcx
*/
.global Syscall3
Syscall3:
    pushq %rdi                         /* Save parameters, as other functions will be called later */
    pushq %rsi
    pushq %rdx
    pushq %rcx
    sub $0x08, %rsp                    /* Ensure that rsp is 16 byte aligned when calling syscall, otherwise moveps will encounter segment errors */

    call SyscallEnter@plt

    add $0x08, %rsp
    popq %rdx                           /* Retrieve parameters and call syscall */
    popq %rsi
    popq %rdi
    popq %rax

    syscall

    /* Check if syscall is successful */
    pushq %rax                          /* Save the rax register(return value), as SyscallExit will be called later */
    call SyscallExit@plt
    popq %rax
    cmpq $-4095, %rax                   /* In Linux, using negative values to indicate errors ensures that values in the [-4095, -1] range(unsigned) will not be used upon success */
    jae error
    ret
error:
    negq %rax                           /* If an error occurs, the 2's complement of the return value is the error code */
    pushq %rax                          /* Put the error code on the stack */
    call __errno_location@plt           /* Get the errno address and put it into rax */
    popq %rdi                           /* Put the error code into rdi */
    movl %edi, (%rax)                   /* Store the error code in the errno pointer */
    movq $-1, %rax
    ret
    .type Syscall3,@function
    .size Syscall3,.-Syscall3

/* int Syscall6(num, arg1, arg2, arg3, arg4, arg5, arg6)
                rdi   rsi   rdx   rcx   r8    r9  8(%rsp)
*/
.global Syscall6
Syscall6:
    /* Save parameters, as other functions will be called later */
    movq 8(%rsp), %rax                  /* rax = arg6 */

    pushq %rdi                          /* Push syscall num onto the stack */
    pushq %rsi                          /* Push arg1 onto the stack */
    pushq %rdx                          /* Push arg2 onto the stack */
    pushq %rcx                          /* Push arg3 onto the stack */
    pushq %r8                           /* Push arg4 onto the stack */
    pushq %r9                           /* Push arg5 onto the stack */
    pushq %rax                          /* Push arg6 onto the stack */
    
    call SyscallEnter@plt

    popq %r9                            /* r9 = arg6 */
    popq %r8                            /* r8 = arg5 */
    popq %r10                           /* r10 = arg4 */
    popq %rdx                           /* rdx = arg3 */
    popq %rsi                           /* rsi = arg2 */
    popq %rdi                           /* rdi = arg1 */
    popq %rax                           /* rax = syscall num */

    syscall

    /* Check if the return value is an error code */
    pushq %rax                          /* Save the rax register(return value), as SyscallExit will be called later */
    call SyscallExit@plt
    popq %rax
    cmpq $-4095, %rax                   /* In Linux, using negative values to indicate errors ensures that values in the [-4095, -1] range(unsigned) will not be used upon success */
    jae error6
    ret
error6:
    negq %rax                           /* If an error occurs, the 2's complement of the return value is the error code */
    pushq %rax                          /* Put the error code on the stack */
    call __errno_location@plt           /* Get the errno address and put it into rax */
    popq %rdi                           /* Put the error code into rdi */
    movl %edi, (%rax)                   /* Store the error code in the errno pointer */
    movq $-1, %rax
    ret
    .type Syscall6,@function
    .size Syscall6,.-Syscall6