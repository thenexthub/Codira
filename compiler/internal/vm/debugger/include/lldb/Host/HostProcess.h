//===-- HostProcess.h ------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_HOSTPROCESS_H
#define LLDB_HOST_HOSTPROCESS_H

#include "lldb/Host/Host.h"
#include "lldb/lldb-types.h"

/// A class that represents a running process on the host machine.
///
/// HostProcess allows querying and manipulation of processes running on the
/// host machine.  It is not intended to be represent a process which is being
/// debugged, although the native debug engine of a platform may likely back
/// inferior processes by a HostProcess.
///
/// HostProcess is implemented using static polymorphism so that on any given
/// platform, an instance of HostProcess will always be able to bind
/// statically to the concrete Process implementation for that platform.  See
/// HostInfo for more details.
///

namespace lldb_private {

class HostNativeProcessBase;
class HostThread;

class HostProcess {
public:
  HostProcess();
  HostProcess(lldb::process_t process);
  ~HostProcess();

  Status Terminate();

  lldb::pid_t GetProcessId() const;
  bool IsRunning() const;

  llvm::Expected<HostThread>
  StartMonitoring(const Host::MonitorChildProcessCallback &callback);

  HostNativeProcessBase &GetNativeProcess();
  const HostNativeProcessBase &GetNativeProcess() const;

private:
  std::shared_ptr<HostNativeProcessBase> m_native_process;
};
}

#endif
