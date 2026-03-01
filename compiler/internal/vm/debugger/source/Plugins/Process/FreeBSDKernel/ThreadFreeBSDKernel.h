//===-- ThreadFreeBSDKernel.h ------------------------------------- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_FREEBSDKERNEL_THREADFREEBSDKERNEL_H
#define LLDB_SOURCE_PLUGINS_PROCESS_FREEBSDKERNEL_THREADFREEBSDKERNEL_H

#include "lldb/Target/Thread.h"

class ThreadFreeBSDKernel : public lldb_private::Thread {
public:
  ThreadFreeBSDKernel(lldb_private::Process &process, lldb::tid_t tid,
                      lldb::addr_t pcb_addr, std::string thread_name);

  ~ThreadFreeBSDKernel() override;

  void RefreshStateAfterStop() override;

  lldb::RegisterContextSP GetRegisterContext() override;

  lldb::RegisterContextSP
  CreateRegisterContextForFrame(lldb_private::StackFrame *frame) override;

  const char *GetName() override {
    if (m_thread_name.empty())
      return nullptr;
    return m_thread_name.c_str();
  }

  void SetName(const char *name) override {
    if (name && name[0])
      m_thread_name.assign(name);
    else
      m_thread_name.clear();
  }

protected:
  bool CalculateStopInfo() override;

private:
  std::string m_thread_name;
  lldb::RegisterContextSP m_thread_reg_ctx_sp;
  lldb::addr_t m_pcb_addr;
};

#endif // LLDB_SOURCE_PLUGINS_PROCESS_FREEBSDKERNEL_THREADFREEBSDKERNEL_H
