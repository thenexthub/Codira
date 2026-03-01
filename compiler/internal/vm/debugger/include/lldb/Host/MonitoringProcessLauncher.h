//===-- MonitoringProcessLauncher.h -----------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_MONITORINGPROCESSLAUNCHER_H
#define LLDB_HOST_MONITORINGPROCESSLAUNCHER_H

#include <memory>
#include "lldb/Host/ProcessLauncher.h"

namespace lldb_private {

class MonitoringProcessLauncher : public ProcessLauncher {
public:
  explicit MonitoringProcessLauncher(
      std::unique_ptr<ProcessLauncher> delegate_launcher);

  /// Launch the process specified in launch_info. The monitoring callback in
  /// launch_info must be set, and it will be called when the process
  /// terminates.
  HostProcess LaunchProcess(const ProcessLaunchInfo &launch_info,
                            Status &error) override;

private:
  std::unique_ptr<ProcessLauncher> m_delegate_launcher;
};

} // namespace lldb_private

#endif // LLDB_HOST_MONITORINGPROCESSLAUNCHER_H
