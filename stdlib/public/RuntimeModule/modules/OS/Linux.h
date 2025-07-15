//===--- Linux.h - Linux specifics ------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
//  Linux specific includes and declarations.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BACKTRACING_LINUX_H
#define LANGUAGE_BACKTRACING_LINUX_H
#ifdef __linux__

#define _GNU_SOURCE
#include <sys/uio.h>
#include <ucontext.h>

#ifdef __cplusplus
extern "C" {
#endif

ssize_t process_vm_readv(pid_t pid,
                         const struct iovec *local_iov,
                         unsigned long liovcnt,
                         const struct iovec *remote_iov,
                         unsigned long riovcnt,
                         unsigned long flags);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __linux__
#endif // LANGUAGE_BACKTRACING_LINUX_H

