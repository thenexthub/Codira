//===-- QueueList.cpp -----------------------------------------------------===//
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

using namespace lldb;
using namespace lldb_private;

QueueList::QueueList(Process *process)
    : m_process(process), m_stop_id(0), m_queues(), m_mutex() {}

QueueList::~QueueList() { Clear(); }

uint32_t QueueList::GetSize() {
  std::lock_guard<std::mutex> guard(m_mutex);
  return m_queues.size();
}

lldb::QueueSP QueueList::GetQueueAtIndex(uint32_t idx) {
  std::lock_guard<std::mutex> guard(m_mutex);
  if (idx < m_queues.size()) {
    return m_queues[idx];
  } else {
    return QueueSP();
  }
}

void QueueList::Clear() {
  std::lock_guard<std::mutex> guard(m_mutex);
  m_queues.clear();
}

void QueueList::AddQueue(QueueSP queue_sp) {
  std::lock_guard<std::mutex> guard(m_mutex);
  if (queue_sp.get()) {
    m_queues.push_back(queue_sp);
  }
}

lldb::QueueSP QueueList::FindQueueByID(lldb::queue_id_t qid) {
  QueueSP ret;
  for (QueueSP queue_sp : Queues()) {
    if (queue_sp->GetID() == qid) {
      ret = queue_sp;
      break;
    }
  }
  return ret;
}

lldb::QueueSP QueueList::FindQueueByIndexID(uint32_t index_id) {
  QueueSP ret;
  for (QueueSP queue_sp : Queues()) {
    if (queue_sp->GetIndexID() == index_id) {
      ret = queue_sp;
      break;
    }
  }
  return ret;
}

std::mutex &QueueList::GetMutex() { return m_mutex; }
