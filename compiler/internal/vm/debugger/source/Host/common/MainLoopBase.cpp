//===-- MainLoopBase.cpp --------------------------------------------------===//
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

#include "lldb/Host/MainLoopBase.h"
#include <chrono>

using namespace lldb;
using namespace lldb_private;

bool MainLoopBase::AddCallback(const Callback &callback, TimePoint point) {
  bool interrupt_needed;
  bool interrupt_succeeded = true;
  {
    std::lock_guard<std::mutex> lock{m_callback_mutex};
    // We need to interrupt the main thread if this callback is scheduled to
    // execute at an earlier time than the earliest callback registered so far.
    interrupt_needed = m_callbacks.empty() || point < m_callbacks.top().first;
    m_callbacks.emplace(point, callback);
  }
  if (interrupt_needed)
    interrupt_succeeded = Interrupt();
  return interrupt_succeeded;
}

void MainLoopBase::ProcessCallbacks() {
  while (true) {
    Callback callback;
    {
      std::lock_guard<std::mutex> lock{m_callback_mutex};
      if (m_callbacks.empty() ||
          std::chrono::steady_clock::now() < m_callbacks.top().first)
        return;
      callback = std::move(m_callbacks.top().second);
      m_callbacks.pop();
    }

    callback(*this);
  }
}

std::optional<MainLoopBase::TimePoint> MainLoopBase::GetNextWakeupTime() {
  std::lock_guard<std::mutex> lock(m_callback_mutex);
  if (m_callbacks.empty())
    return std::nullopt;
  return m_callbacks.top().first;
}
