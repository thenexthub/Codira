//===-- LockFileBase.h ------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_LOCKFILEBASE_H
#define LLDB_HOST_LOCKFILEBASE_H

#include "lldb/Utility/Status.h"

#include <functional>

namespace lldb_private {

class LockFileBase {
public:
  virtual ~LockFileBase() = default;

  bool IsLocked() const;

  Status WriteLock(const uint64_t start, const uint64_t len);
  Status TryWriteLock(const uint64_t start, const uint64_t len);

  Status ReadLock(const uint64_t start, const uint64_t len);
  Status TryReadLock(const uint64_t start, const uint64_t len);

  Status Unlock();

protected:
  using Locker = std::function<Status(const uint64_t, const uint64_t)>;

  LockFileBase(int fd);

  virtual bool IsValidFile() const;

  virtual Status DoWriteLock(const uint64_t start, const uint64_t len) = 0;
  virtual Status DoTryWriteLock(const uint64_t start, const uint64_t len) = 0;

  virtual Status DoReadLock(const uint64_t start, const uint64_t len) = 0;
  virtual Status DoTryReadLock(const uint64_t start, const uint64_t len) = 0;

  virtual Status DoUnlock() = 0;

  Status DoLock(const Locker &locker, const uint64_t start, const uint64_t len);

  int m_fd; // not owned.
  bool m_locked;
  uint64_t m_start;
  uint64_t m_len;
};
}

#endif
