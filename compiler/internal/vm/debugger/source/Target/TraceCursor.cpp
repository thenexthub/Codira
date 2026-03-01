//===-- TraceCursor.cpp -----------------------------------------*- C++ -*-===//
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

#include "lldb/Target/TraceCursor.h"

#include "lldb/Target/ExecutionContext.h"
#include "lldb/Target/Trace.h"

using namespace lldb;
using namespace lldb_private;
using namespace llvm;

TraceCursor::TraceCursor(lldb::ThreadSP thread_sp)
    : m_exe_ctx_ref(ExecutionContext(thread_sp)) {}

ExecutionContextRef &TraceCursor::GetExecutionContextRef() {
  return m_exe_ctx_ref;
}

void TraceCursor::SetForwards(bool forwards) { m_forwards = forwards; }

bool TraceCursor::IsForwards() const { return m_forwards; }

bool TraceCursor::IsError() const {
  return GetItemKind() == lldb::eTraceItemKindError;
}

bool TraceCursor::IsEvent() const {
  return GetItemKind() == lldb::eTraceItemKindEvent;
}

bool TraceCursor::IsInstruction() const {
  return GetItemKind() == lldb::eTraceItemKindInstruction;
}

const char *TraceCursor::GetEventTypeAsString() const {
  return EventKindToString(GetEventType());
}

const char *TraceCursor::EventKindToString(lldb::TraceEvent event_kind) {
  switch (event_kind) {
  case lldb::eTraceEventDisabledHW:
    return "hardware disabled tracing";
  case lldb::eTraceEventDisabledSW:
    return "software disabled tracing";
  case lldb::eTraceEventCPUChanged:
    return "CPU core changed";
  case lldb::eTraceEventHWClockTick:
    return "HW clock tick";
  case lldb::eTraceEventSyncPoint:
    return "trace synchronization point";
  }
  llvm_unreachable("Fully covered switch above");
}
