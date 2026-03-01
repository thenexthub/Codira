//===-- SBQueueItem.cpp ---------------------------------------------------===//
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

#include "lldb/lldb-forward.h"

#include "lldb/API/SBAddress.h"
#include "lldb/API/SBQueueItem.h"
#include "lldb/API/SBThread.h"
#include "lldb/Core/Address.h"
#include "lldb/Target/Process.h"
#include "lldb/Target/QueueItem.h"
#include "lldb/Target/Thread.h"
#include "lldb/Utility/Instrumentation.h"

using namespace lldb;
using namespace lldb_private;

// Constructors
SBQueueItem::SBQueueItem() { LLDB_INSTRUMENT_VA(this); }

SBQueueItem::SBQueueItem(const QueueItemSP &queue_item_sp)
    : m_queue_item_sp(queue_item_sp) {
  LLDB_INSTRUMENT_VA(this, queue_item_sp);
}

// Destructor
SBQueueItem::~SBQueueItem() { m_queue_item_sp.reset(); }

bool SBQueueItem::IsValid() const {
  LLDB_INSTRUMENT_VA(this);
  return this->operator bool();
}
SBQueueItem::operator bool() const {
  LLDB_INSTRUMENT_VA(this);

  return m_queue_item_sp.get() != nullptr;
}

void SBQueueItem::Clear() {
  LLDB_INSTRUMENT_VA(this);

  m_queue_item_sp.reset();
}

void SBQueueItem::SetQueueItem(const QueueItemSP &queue_item_sp) {
  LLDB_INSTRUMENT_VA(this, queue_item_sp);

  m_queue_item_sp = queue_item_sp;
}

lldb::QueueItemKind SBQueueItem::GetKind() const {
  LLDB_INSTRUMENT_VA(this);

  QueueItemKind result = eQueueItemKindUnknown;
  if (m_queue_item_sp) {
    result = m_queue_item_sp->GetKind();
  }
  return result;
}

void SBQueueItem::SetKind(lldb::QueueItemKind kind) {
  LLDB_INSTRUMENT_VA(this, kind);

  if (m_queue_item_sp) {
    m_queue_item_sp->SetKind(kind);
  }
}

SBAddress SBQueueItem::GetAddress() const {
  LLDB_INSTRUMENT_VA(this);

  SBAddress result;
  if (m_queue_item_sp) {
    result.SetAddress(m_queue_item_sp->GetAddress());
  }
  return result;
}

void SBQueueItem::SetAddress(SBAddress addr) {
  LLDB_INSTRUMENT_VA(this, addr);

  if (m_queue_item_sp) {
    m_queue_item_sp->SetAddress(addr.ref());
  }
}

SBThread SBQueueItem::GetExtendedBacktraceThread(const char *type) {
  LLDB_INSTRUMENT_VA(this, type);

  SBThread result;
  if (m_queue_item_sp) {
    ProcessSP process_sp = m_queue_item_sp->GetProcessSP();
    Process::StopLocker stop_locker;
    if (process_sp && stop_locker.TryLock(&process_sp->GetRunLock())) {
      ThreadSP thread_sp;
      ConstString type_const(type);
      thread_sp = m_queue_item_sp->GetExtendedBacktraceThread(type_const);
      if (thread_sp) {
        // Save this in the Process' ExtendedThreadList so a strong pointer
        // retains the object
        process_sp->GetExtendedThreadList().AddThread(thread_sp);
        result.SetThread(thread_sp);
      }
    }
  }
  return result;
}
