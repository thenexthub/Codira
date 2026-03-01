//===-- HostProcessPosix.h --------------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_POSIX_HOSTPROCESSPOSIX_H
#define LLDB_HOST_POSIX_HOSTPROCESSPOSIX_H

#include "lldb/Host/HostNativeProcessBase.h"
#include "lldb/Utility/Status.h"
#include "lldb/lldb-types.h"

namespace lldb_private {

class FileSpec;

class HostProcessPosix : public HostNativeProcessBase {
public:
  HostProcessPosix();
  HostProcessPosix(lldb::process_t process);
  ~HostProcessPosix() override;

  virtual Status Signal(int signo) const;
  static Status Signal(lldb::process_t process, int signo);

  Status Terminate() override;

  lldb::pid_t GetProcessId() const override;
  bool IsRunning() const override;

  llvm::Expected<HostThread>
  StartMonitoring(const Host::MonitorChildProcessCallback &callback) override;
};

} // namespace lldb_private

#endif // LLDB_HOST_POSIX_HOSTPROCESSPOSIX_H
