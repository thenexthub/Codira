//===-- HostProcessWindows.h ------------------------------------*- C++ -*-===//
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

#ifndef lldb_Host_HostProcessWindows_h_
#define lldb_Host_HostProcessWindows_h_

#include "lldb/Host/HostNativeProcessBase.h"
#include "lldb/lldb-types.h"

namespace lldb_private {

class FileSpec;

class HostProcessWindows : public HostNativeProcessBase {
public:
  HostProcessWindows();
  explicit HostProcessWindows(lldb::process_t process);
  ~HostProcessWindows();

  void SetOwnsHandle(bool owns);

  Status Terminate() override;

  lldb::pid_t GetProcessId() const override;
  bool IsRunning() const override;

  virtual llvm::Expected<HostThread>
  StartMonitoring(const Host::MonitorChildProcessCallback &callback) override;

private:
  void Close();

  bool m_owns_handle;
};
}

#endif
