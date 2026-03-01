//===-- ThreadMemory.cpp --------------------------------------------------===//
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

#include "Plugins/Process/Utility/ThreadMemory.h"

#include "Plugins/Process/Utility/RegisterContextThreadMemory.h"
#include "lldb/Target/OperatingSystem.h"
#include "lldb/Target/Process.h"
#include "lldb/Target/RegisterContext.h"
#include "lldb/Target/StopInfo.h"
#include "lldb/Target/Unwind.h"

#include <memory>

using namespace lldb;
using namespace lldb_private;

ThreadMemoryProvidingNameAndQueue::ThreadMemoryProvidingNameAndQueue(
    Process &process, lldb::tid_t tid,
    const ValueObjectSP &thread_info_valobj_sp)
    : ThreadMemoryProvidingName(process, tid, LLDB_INVALID_ADDRESS, ""),
      m_thread_info_valobj_sp(thread_info_valobj_sp), m_queue() {}

ThreadMemoryProvidingNameAndQueue::ThreadMemoryProvidingNameAndQueue(
    Process &process, lldb::tid_t tid, llvm::StringRef name,
    llvm::StringRef queue, lldb::addr_t register_data_addr)
    : ThreadMemoryProvidingName(process, tid, register_data_addr, name),
      m_thread_info_valobj_sp(), m_queue(std::string(queue)) {}

ThreadMemory::~ThreadMemory() { DestroyThread(); }

void ThreadMemory::WillResume(StateType resume_state) {
  if (m_backing_thread_sp)
    m_backing_thread_sp->WillResume(resume_state);
}

void ThreadMemory::ClearStackFrames() {
  if (m_backing_thread_sp)
    m_backing_thread_sp->ClearStackFrames();
  Thread::ClearStackFrames();
}

RegisterContextSP ThreadMemory::GetRegisterContext() {
  if (!m_reg_context_sp)
    m_reg_context_sp = std::make_shared<RegisterContextThreadMemory>(
        *this, m_register_data_addr);
  return m_reg_context_sp;
}

RegisterContextSP
ThreadMemory::CreateRegisterContextForFrame(StackFrame *frame) {
  uint32_t concrete_frame_idx = 0;

  if (frame)
    concrete_frame_idx = frame->GetConcreteFrameIndex();

  if (concrete_frame_idx == 0)
    return GetRegisterContext();
  return GetUnwinder().CreateRegisterContextForFrame(frame);
}

bool ThreadMemory::CalculateStopInfo() {
  if (m_backing_thread_sp) {
    lldb::StopInfoSP backing_stop_info_sp(
        m_backing_thread_sp->GetPrivateStopInfo());
    if (backing_stop_info_sp &&
        backing_stop_info_sp->IsValidForOperatingSystemThread(*this)) {
      backing_stop_info_sp->SetThread(shared_from_this());
      SetStopInfo(backing_stop_info_sp);
      return true;
    }
  } else {
    ProcessSP process_sp(GetProcess());

    if (process_sp) {
      OperatingSystem *os = process_sp->GetOperatingSystem();
      if (os) {
        SetStopInfo(os->CreateThreadStopReason(this));
        return true;
      }
    }
  }
  return false;
}

void ThreadMemory::RefreshStateAfterStop() {
  if (m_backing_thread_sp)
    return m_backing_thread_sp->RefreshStateAfterStop();

  if (m_reg_context_sp)
    m_reg_context_sp->InvalidateAllRegisters();
}
