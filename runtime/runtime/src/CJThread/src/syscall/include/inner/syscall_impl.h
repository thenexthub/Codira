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

#ifndef MRT_CODETHREAD_SYSCALL_IMPL_H
#define MRT_CODETHREAD_SYSCALL_IMPL_H

#include "mid.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

void SyscallEnter(void);

void SyscallExit(void);

/**
 * @brief Non-blocking system call with 3 arguments
 * @par syscall(int number, arg1, arg2, arg3),
 * This method is basically the same as the Linux system call. The only difference is that
 * before the system call is entered, if the current context is a codethread, the processor
 * and thread are unbound. After the call is complete, the processor is bound.
 * This method supports only three additional parameters.
 * @attention If the number of parameters is less than 3, fill them with 0.
 * @param  number           [IN]  Sequence number of the system call
 * @retval Return value of the corresponding system call.
 */
extern int Syscall3(int number, ...);

/**
 * @brief Non-blocking system call with 6 arguments
 * @par syscall(int number, arg1, arg2, arg3, arg4, arg5, arg6),
 * This method is basically the same as the Linux system call. The only difference is that
 * before the system call is entered, if the current context is a codethread, the processor
 * and thread are unbound. After the call is complete, the processor is bound.
 * This method supports only six additional parameters.
 * @attention If the number of parameters is less than 6, fill them with 0.
 * @param  number           [IN]  Sequence number of the system call
 * @retval Return value of the corresponding system call.
 */
extern int Syscall6(int number, ...);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
#endif /* MRT_CODETHREAD_SYSCALL_IMPL_H */
