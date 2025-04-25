//===--- TracingSignpost.cpp - Tracing with the signpost API -------*- C++ -*-//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//
//
// Concurrency tracing implemented with the os_signpost API.
//
//===----------------------------------------------------------------------===//

#if SWIFT_STDLIB_CONCURRENCY_TRACING

#include "TracingSignpost.h"
#include <stdio.h>

#define SWIFT_LOG_CONCURRENCY_SUBSYSTEM "com.apple.swift.concurrency"
#define SWIFT_LOG_ACTOR_CATEGORY "Actor"
#define SWIFT_LOG_TASK_CATEGORY "Task"

namespace language {
namespace concurrency {
namespace trace {

os_log_t ActorLog;
os_log_t TaskLog;
swift::once_t LogsToken;
bool TracingEnabled;

void setupLogs(void *unused) {
  if (!swift::runtime::trace::shouldEnableTracing()) {
    TracingEnabled = false;
    return;
  }

  TracingEnabled = true;
  ActorLog = os_log_create(SWIFT_LOG_CONCURRENCY_SUBSYSTEM,
                           SWIFT_LOG_ACTOR_CATEGORY);
  TaskLog = os_log_create(SWIFT_LOG_CONCURRENCY_SUBSYSTEM,
                          SWIFT_LOG_TASK_CATEGORY);
}

} // namespace trace
} // namespace concurrency
} // namespace language

#endif
