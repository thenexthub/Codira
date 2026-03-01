//===-- TaskTimer.h ---------------------------------------------*- C++ -*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_TRACE_INTEL_PT_TASKTIMER_H
#define LLDB_SOURCE_PLUGINS_TRACE_INTEL_PT_TASKTIMER_H

#include "lldb/lldb-types.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"
#include <chrono>
#include <functional>
#include <unordered_map>

namespace lldb_private {
namespace trace_intel_pt {

/// Class used to track the duration of long running tasks related to a single
/// scope for reporting.
class ScopedTaskTimer {
public:
  /// Execute the given \p task and record its duration.
  ///
  /// \param[in] name
  ///     The name used to identify this task for reporting.
  ///
  /// \param[in] task
  ///     The task function.
  ///
  /// \return
  ///     The return value of the task.
  template <typename C> auto TimeTask(llvm::StringRef name, C task) {
    auto start = std::chrono::steady_clock::now();
    auto result = task();
    auto end = std::chrono::steady_clock::now();
    std::chrono::milliseconds duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    m_timed_tasks.insert({name.str(), duration});
    return result;
  }

  /// Executive the given \p callback on each recorded task.
  ///
  /// \param[in] callback
  ///     The first parameter of the callback is the name of the recorded task,
  ///     and the second parameter is the duration of that task.
  void ForEachTimedTask(std::function<void(const std::string &name,
                                           std::chrono::milliseconds duration)>
                            callback);

private:
  std::unordered_map<std::string, std::chrono::milliseconds> m_timed_tasks;
};

/// Class used to track the duration of long running tasks for reporting.
class TaskTimer {
public:
  /// \return
  ///     The timer object for the given thread.
  ScopedTaskTimer &ForThread(lldb::tid_t tid);

  /// \return
  ///     The timer object for global tasks.
  ScopedTaskTimer &ForGlobal();

private:
  llvm::DenseMap<lldb::tid_t, ScopedTaskTimer> m_thread_timers;
  ScopedTaskTimer m_global_timer;
};

} // namespace trace_intel_pt
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_TRACE_INTEL_PT_TASKTIMER_H
