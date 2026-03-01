//===-- TargetThreadWindows.h -----------------------------------*- C++ -*-===//
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

#ifndef liblldb_Plugins_Process_Windows_TargetThreadWindows_H_
#define liblldb_Plugins_Process_Windows_TargetThreadWindows_H_

//#include "ForwardDecl.h"
#include "lldb/Host/HostThread.h"
#include "lldb/Target/Thread.h"
#include "lldb/lldb-forward.h"

#include "RegisterContextWindows.h"

namespace lldb_private {
class ProcessWindows;
class HostThread;
class StackFrame;

class TargetThreadWindows : public lldb_private::Thread {
public:
  TargetThreadWindows(ProcessWindows &process, const HostThread &thread);
  virtual ~TargetThreadWindows();

  // lldb_private::Thread overrides
  void RefreshStateAfterStop() override;
  void WillResume(lldb::StateType resume_state) override;
  void DidStop() override;
  lldb::RegisterContextSP GetRegisterContext() override;
  lldb::RegisterContextSP
  CreateRegisterContextForFrame(StackFrame *frame) override;
  bool CalculateStopInfo() override;
  const char *GetName() override;

  Status DoResume();

  HostThread GetHostThread() const { return m_host_thread; }

private:
  lldb::RegisterContextSP m_thread_reg_ctx_sp;
  HostThread m_host_thread;
  std::string m_name;
};
} // namespace lldb_private

#endif
