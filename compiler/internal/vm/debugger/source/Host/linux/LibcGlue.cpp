//===-- LibcGlue.cpp ------------------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

// This file adds functions missing from libc on older versions of linux

#include <cerrno>
#include <lldb/Host/linux/Uio.h>
#include <sys/syscall.h>
#include <unistd.h>

#if !HAVE_PROCESS_VM_READV
// If the syscall wrapper is not available, provide one.
ssize_t process_vm_readv(::pid_t pid, const struct iovec *local_iov,
                         unsigned long liovcnt, const struct iovec *remote_iov,
                         unsigned long riovcnt, unsigned long flags) {
#if HAVE_NR_PROCESS_VM_READV
  // If we have the syscall number, we can issue the syscall ourselves.
  return syscall(__NR_process_vm_readv, pid, local_iov, liovcnt, remote_iov,
                 riovcnt, flags);
#else // If not, let's pretend the syscall is not present.
  errno = ENOSYS;
  return -1;
#endif
}
#endif
