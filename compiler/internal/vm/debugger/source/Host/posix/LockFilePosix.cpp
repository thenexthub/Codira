//===-- LockFilePosix.cpp -------------------------------------------------===//
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

#include "lldb/Host/posix/LockFilePosix.h"

#include "llvm/Support/Errno.h"

#include <fcntl.h>
#include <unistd.h>

using namespace lldb;
using namespace lldb_private;

static Status fileLock(int fd, int cmd, int lock_type, const uint64_t start,
                       const uint64_t len) {
  struct flock fl;

  fl.l_type = lock_type;
  fl.l_whence = SEEK_SET;
  fl.l_start = start;
  fl.l_len = len;
  fl.l_pid = ::getpid();

  if (llvm::sys::RetryAfterSignal(-1, ::fcntl, fd, cmd, &fl) == -1)
    return Status::FromErrno();

  return Status();
}

LockFilePosix::LockFilePosix(int fd) : LockFileBase(fd) {}

LockFilePosix::~LockFilePosix() { Unlock(); }

Status LockFilePosix::DoWriteLock(const uint64_t start, const uint64_t len) {
  return fileLock(m_fd, F_SETLKW, F_WRLCK, start, len);
}

Status LockFilePosix::DoTryWriteLock(const uint64_t start, const uint64_t len) {
  return fileLock(m_fd, F_SETLK, F_WRLCK, start, len);
}

Status LockFilePosix::DoReadLock(const uint64_t start, const uint64_t len) {
  return fileLock(m_fd, F_SETLKW, F_RDLCK, start, len);
}

Status LockFilePosix::DoTryReadLock(const uint64_t start, const uint64_t len) {
  return fileLock(m_fd, F_SETLK, F_RDLCK, start, len);
}

Status LockFilePosix::DoUnlock() {
  return fileLock(m_fd, F_SETLK, F_UNLCK, m_start, m_len);
}
