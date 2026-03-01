//===-- MemoryMonitorMacOSX.mm --------------------------------------------===//
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

#include "lldb/Host/MemoryMonitor.h"
#include <cassert>
#include <dispatch/dispatch.h>

using namespace lldb_private;

class MemoryMonitorMacOSX : public MemoryMonitor {
  using MemoryMonitor::MemoryMonitor;
  void Start() override {
    m_memory_pressure_source = dispatch_source_create(
        DISPATCH_SOURCE_TYPE_MEMORYPRESSURE, 0,
        DISPATCH_MEMORYPRESSURE_WARN | DISPATCH_MEMORYPRESSURE_CRITICAL,
        dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0));

    if (!m_memory_pressure_source)
      return;

    dispatch_source_set_event_handler(m_memory_pressure_source, ^{
      dispatch_source_memorypressure_flags_t pressureLevel =
          dispatch_source_get_data(m_memory_pressure_source);
      if (pressureLevel &
          (DISPATCH_MEMORYPRESSURE_WARN | DISPATCH_MEMORYPRESSURE_CRITICAL)) {
        m_callback();
      }
    });
    dispatch_activate(m_memory_pressure_source);
  }

  void Stop() override {
    if (m_memory_pressure_source) {
      dispatch_source_cancel(m_memory_pressure_source);
      dispatch_release(m_memory_pressure_source);
    }
  }

private:
  dispatch_source_t m_memory_pressure_source;
};

std::unique_ptr<MemoryMonitor> MemoryMonitor::Create(Callback callback) {
  return std::make_unique<MemoryMonitorMacOSX>(callback);
}
