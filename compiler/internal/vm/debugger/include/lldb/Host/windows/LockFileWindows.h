//===-- LockFileWindows.h ---------------------------------------*- C++ -*-===//
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

#ifndef liblldb_Host_posix_LockFileWindows_h_
#define liblldb_Host_posix_LockFileWindows_h_

#include "lldb/Host/LockFileBase.h"
#include "lldb/Host/windows/windows.h"

namespace lldb_private {

class LockFileWindows : public LockFileBase {
public:
  explicit LockFileWindows(int fd);
  ~LockFileWindows();

protected:
  Status DoWriteLock(const uint64_t start, const uint64_t len) override;

  Status DoTryWriteLock(const uint64_t start, const uint64_t len) override;

  Status DoReadLock(const uint64_t start, const uint64_t len) override;

  Status DoTryReadLock(const uint64_t start, const uint64_t len) override;

  Status DoUnlock() override;

  bool IsValidFile() const override;

private:
  HANDLE m_file;
};

} // namespace lldb_private

#endif // liblldb_Host_posix_LockFileWindows_h_
