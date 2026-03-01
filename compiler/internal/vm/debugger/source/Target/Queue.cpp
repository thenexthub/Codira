//===-- Queue.cpp ---------------------------------------------------------===//
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

#include "lldb/Target/Queue.h"
#include "lldb/Target/Process.h"
#include "lldb/Target/QueueList.h"
#include "lldb/Target/SystemRuntime.h"
#include "lldb/Target/Thread.h"

using namespace lldb;
using namespace lldb_private;

Queue::Queue(ProcessSP process_sp, lldb::queue_id_t queue_id,
             const char *queue_name)
    : m_process_wp(), m_queue_id(queue_id), m_queue_name(),
      m_running_work_items_count(0), m_pending_work_items_count(0),
      m_pending_items(), m_dispatch_queue_t_addr(LLDB_INVALID_ADDRESS),
      m_kind(eQueueKindUnknown) {
  if (queue_name)
    m_queue_name = queue_name;

  m_process_wp = process_sp;
}

Queue::~Queue() = default;

queue_id_t Queue::GetID() { return m_queue_id; }

const char *Queue::GetName() {
  return (m_queue_name.empty() ? nullptr : m_queue_name.c_str());
}

uint32_t Queue::GetIndexID() { return m_queue_id; }

std::vector<lldb::ThreadSP> Queue::GetThreads() {
  std::vector<ThreadSP> result;
  ProcessSP process_sp = m_process_wp.lock();
  if (process_sp) {
    for (ThreadSP thread_sp : process_sp->Threads()) {
      if (thread_sp->GetQueueID() == m_queue_id) {
        result.push_back(thread_sp);
      }
    }
  }
  return result;
}

void Queue::SetNumRunningWorkItems(uint32_t count) {
  m_running_work_items_count = count;
}

uint32_t Queue::GetNumRunningWorkItems() const {
  return m_running_work_items_count;
}

void Queue::SetNumPendingWorkItems(uint32_t count) {
  m_pending_work_items_count = count;
}

uint32_t Queue::GetNumPendingWorkItems() const {
  return m_pending_work_items_count;
}

void Queue::SetLibdispatchQueueAddress(addr_t dispatch_queue_t_addr) {
  m_dispatch_queue_t_addr = dispatch_queue_t_addr;
}

addr_t Queue::GetLibdispatchQueueAddress() const {
  return m_dispatch_queue_t_addr;
}

const std::vector<lldb::QueueItemSP> &Queue::GetPendingItems() {
  if (m_pending_items.empty()) {
    ProcessSP process_sp = m_process_wp.lock();
    if (process_sp && process_sp->GetSystemRuntime()) {
      process_sp->GetSystemRuntime()->PopulatePendingItemsForQueue(this);
    }
  }
  return m_pending_items;
}

lldb::QueueKind Queue::GetKind() { return m_kind; }

void Queue::SetKind(lldb::QueueKind kind) { m_kind = kind; }
