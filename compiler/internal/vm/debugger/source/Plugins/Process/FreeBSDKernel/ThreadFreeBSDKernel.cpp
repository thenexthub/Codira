//===-- ThreadFreeBSDKernel.cpp -------------------------------------------===//
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

#include "ThreadFreeBSDKernel.h"

#include "lldb/Target/Unwind.h"
#include "lldb/Utility/Log.h"

#include "Plugins/Process/Utility/RegisterContextFreeBSD_i386.h"
#include "Plugins/Process/Utility/RegisterContextFreeBSD_x86_64.h"
#include "Plugins/Process/Utility/RegisterInfoPOSIX_arm64.h"
#include "ProcessFreeBSDKernel.h"
#include "RegisterContextFreeBSDKernel_arm64.h"
#include "RegisterContextFreeBSDKernel_i386.h"
#include "RegisterContextFreeBSDKernel_x86_64.h"

using namespace lldb;
using namespace lldb_private;

ThreadFreeBSDKernel::ThreadFreeBSDKernel(Process &process, lldb::tid_t tid,
                                         lldb::addr_t pcb_addr,
                                         std::string thread_name)
    : Thread(process, tid), m_thread_name(std::move(thread_name)),
      m_pcb_addr(pcb_addr) {}

ThreadFreeBSDKernel::~ThreadFreeBSDKernel() {}

void ThreadFreeBSDKernel::RefreshStateAfterStop() {}

lldb::RegisterContextSP ThreadFreeBSDKernel::GetRegisterContext() {
  if (!m_reg_context_sp)
    m_reg_context_sp = CreateRegisterContextForFrame(nullptr);
  return m_reg_context_sp;
}

lldb::RegisterContextSP
ThreadFreeBSDKernel::CreateRegisterContextForFrame(StackFrame *frame) {
  RegisterContextSP reg_ctx_sp;
  uint32_t concrete_frame_idx = 0;

  if (frame)
    concrete_frame_idx = frame->GetConcreteFrameIndex();

  if (concrete_frame_idx == 0) {
    if (m_thread_reg_ctx_sp)
      return m_thread_reg_ctx_sp;

    ProcessFreeBSDKernel *process =
        static_cast<ProcessFreeBSDKernel *>(GetProcess().get());
    ArchSpec arch = process->GetTarget().GetArchitecture();

    switch (arch.GetMachine()) {
    case llvm::Triple::aarch64:
      m_thread_reg_ctx_sp =
          std::make_shared<RegisterContextFreeBSDKernel_arm64>(
              *this, std::make_unique<RegisterInfoPOSIX_arm64>(arch, 0),
              m_pcb_addr);
      break;
    case llvm::Triple::x86:
      m_thread_reg_ctx_sp = std::make_shared<RegisterContextFreeBSDKernel_i386>(
          *this, new RegisterContextFreeBSD_i386(arch), m_pcb_addr);
      break;
    case llvm::Triple::x86_64:
      m_thread_reg_ctx_sp =
          std::make_shared<RegisterContextFreeBSDKernel_x86_64>(
              *this, new RegisterContextFreeBSD_x86_64(arch), m_pcb_addr);
      break;
    default:
      assert(false && "Unsupported architecture passed to ThreadFreeBSDKernel");
      break;
    }

    reg_ctx_sp = m_thread_reg_ctx_sp;
  } else {
    reg_ctx_sp = GetUnwinder().CreateRegisterContextForFrame(frame);
  }
  return reg_ctx_sp;
}

bool ThreadFreeBSDKernel::CalculateStopInfo() { return false; }
