//===-- MonitoringProcessLauncher.cpp -------------------------------------===//
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

#include "lldb/Host/MonitoringProcessLauncher.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Host/HostProcess.h"
#include "lldb/Host/ProcessLaunchInfo.h"
#include "lldb/Utility/LLDBLog.h"
#include "lldb/Utility/Log.h"

#include "llvm/Support/FileSystem.h"

using namespace lldb;
using namespace lldb_private;

MonitoringProcessLauncher::MonitoringProcessLauncher(
    std::unique_ptr<ProcessLauncher> delegate_launcher)
    : m_delegate_launcher(std::move(delegate_launcher)) {}

HostProcess
MonitoringProcessLauncher::LaunchProcess(const ProcessLaunchInfo &launch_info,
                                         Status &error) {
  ProcessLaunchInfo resolved_info(launch_info);

  error.Clear();

  FileSystem &fs = FileSystem::Instance();
  FileSpec exe_spec(resolved_info.GetExecutableFile());

  if (!fs.Exists(exe_spec))
    FileSystem::Instance().Resolve(exe_spec);

  if (!fs.Exists(exe_spec))
    FileSystem::Instance().ResolveExecutableLocation(exe_spec);

  if (!fs.Exists(exe_spec)) {
    error = Status::FromErrorStringWithFormatv(
        "executable doesn't exist: '{0}'", exe_spec);
    return HostProcess();
  }

  resolved_info.SetExecutableFile(exe_spec, false);
  assert(!resolved_info.GetFlags().Test(eLaunchFlagLaunchInTTY));

  HostProcess process =
      m_delegate_launcher->LaunchProcess(resolved_info, error);

  if (process.GetProcessId() != LLDB_INVALID_PROCESS_ID) {
    Log *log = GetLog(LLDBLog::Process);

    assert(launch_info.GetMonitorProcessCallback());
    llvm::Expected<HostThread> maybe_thread =
        process.StartMonitoring(launch_info.GetMonitorProcessCallback());
    if (!maybe_thread)
      error = Status::FromErrorStringWithFormatv(
          "failed to launch host thread: {}",
          llvm::toString(maybe_thread.takeError()));
    if (log)
      log->PutCString("started monitoring child process.");
  } else {
    // Invalid process ID, something didn't go well
    if (error.Success())
      error =
          Status::FromErrorString("process launch failed for unknown reasons");
  }
  return process;
}
