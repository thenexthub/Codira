//===--- Concurrency.h - Runtime interface for concurrency ------*- C++ -*-===//
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
// The runtime interface for concurrency.
//
//===----------------------------------------------------------------------===//

#ifndef SWIFT_RUNTIME_CONCURRENCY_BACKDEPLOY56_H
#define SWIFT_RUNTIME_CONCURRENCY_BACKDEPLOY56_H

#include "Concurrency/Task.h"
#include "Concurrency/TaskStatus.h"
#include "Concurrency/AsyncLet.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"

// Does the runtime use a cooperative global executor?
#if defined(SWIFT_STDLIB_SINGLE_THREADED_RUNTIME)
#define SWIFT_CONCURRENCY_COOPERATIVE_GLOBAL_EXECUTOR 1
#else
#define SWIFT_CONCURRENCY_COOPERATIVE_GLOBAL_EXECUTOR 0
#endif

// Does the runtime integrate with libdispatch?
#ifndef SWIFT_CONCURRENCY_ENABLE_DISPATCH
#if SWIFT_CONCURRENCY_COOPERATIVE_GLOBAL_EXECUTOR
#define SWIFT_CONCURRENCY_ENABLE_DISPATCH 0
#else
#define SWIFT_CONCURRENCY_ENABLE_DISPATCH 1
#endif
#endif

namespace language {
class DefaultActor;
class TaskOptionRecord;

struct SwiftError;

struct AsyncTaskAndContext {
  AsyncTask *Task;
  AsyncContext *InitialContext;
};

/// Caution: not all future-initializing functions actually throw, so
/// this signature may be incorrect.
using FutureAsyncSignature =
  AsyncSignature<void(void*), /*throws*/ true>;

/// Escalate the priority of a task and all of its child tasks.
///
/// This can be called from any thread.
///
/// This has no effect if the task already has at least the given priority.
/// Returns the priority of the task.
SWIFT_CC(swift)
__attribute__((visibility("hidden")))
JobPriority swift_task_escalateBackdeploy56(AsyncTask *task,
                                            JobPriority newPriority);
} // namespace language

#pragma clang diagnostic pop

#endif // SWIFT_RUNTIME_CONCURRENCY_BACKDEPLOY56_H
