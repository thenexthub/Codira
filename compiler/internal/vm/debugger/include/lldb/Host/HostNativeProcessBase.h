//===-- HostNativeProcessBase.h ---------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_HOSTNATIVEPROCESSBASE_H
#define LLDB_HOST_HOSTNATIVEPROCESSBASE_H

#include "lldb/Host/HostProcess.h"
#include "lldb/Utility/Status.h"
#include "lldb/lldb-defines.h"
#include "lldb/lldb-types.h"

namespace lldb_private {

class HostThread;

class HostNativeProcessBase {
  HostNativeProcessBase(const HostNativeProcessBase &) = delete;
  const HostNativeProcessBase &
  operator=(const HostNativeProcessBase &) = delete;

public:
  HostNativeProcessBase() : m_process(LLDB_INVALID_PROCESS) {}
  explicit HostNativeProcessBase(lldb::process_t process)
      : m_process(process) {}
  virtual ~HostNativeProcessBase() = default;

  virtual Status Terminate() = 0;

  virtual lldb::pid_t GetProcessId() const = 0;
  virtual bool IsRunning() const = 0;

  lldb::process_t GetSystemHandle() const { return m_process; }

  virtual llvm::Expected<HostThread>
  StartMonitoring(const Host::MonitorChildProcessCallback &callback) = 0;

protected:
  lldb::process_t m_process;
};
}

#endif
