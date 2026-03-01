//===-- ThreadKDP.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_PROCESS_MACOSX_KERNEL_THREADKDP_H
#define LLDB_SOURCE_PLUGINS_PROCESS_MACOSX_KERNEL_THREADKDP_H

#include <string>

#include "lldb/Target/Process.h"
#include "lldb/Target/Thread.h"

class ProcessKDP;

class ThreadKDP : public lldb_private::Thread {
public:
  ThreadKDP(lldb_private::Process &process, lldb::tid_t tid);

  ~ThreadKDP() override;

  void RefreshStateAfterStop() override;

  const char *GetName() override;

  const char *GetQueueName() override;

  lldb::RegisterContextSP GetRegisterContext() override;

  lldb::RegisterContextSP
  CreateRegisterContextForFrame(lldb_private::StackFrame *frame) override;

  void Dump(lldb_private::Log *log, uint32_t index);

  static bool ThreadIDIsValid(lldb::tid_t thread);

  bool ShouldStop(bool &step_more);

  const char *GetBasicInfoAsString();

  void SetName(const char *name) override {
    if (name && name[0])
      m_thread_name.assign(name);
    else
      m_thread_name.clear();
  }

  lldb::addr_t GetThreadDispatchQAddr() { return m_thread_dispatch_qaddr; }

  void SetThreadDispatchQAddr(lldb::addr_t thread_dispatch_qaddr) {
    m_thread_dispatch_qaddr = thread_dispatch_qaddr;
  }

  void SetStopInfoFrom_KDP_EXCEPTION(
      const lldb_private::DataExtractor &exc_reply_packet);

protected:
  friend class ProcessKDP;

  // Member variables.
  std::string m_thread_name;
  std::string m_dispatch_queue_name;
  lldb::addr_t m_thread_dispatch_qaddr;
  lldb::StopInfoSP m_cached_stop_info_sp;
  // Protected member functions.
  bool CalculateStopInfo() override;
};

#endif // LLDB_SOURCE_PLUGINS_PROCESS_MACOSX_KERNEL_THREADKDP_H
