//===--- ExecutorBridge.cpp - C++ side of executor bridge -----------------===//
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

#if SWIFT_CONCURRENCY_USES_DISPATCH
#include <dispatch/dispatch.h>
#endif

#include "language/Threading/Once.h"

#include "Error.h"
#include "ExecutorBridge.h"

using namespace language;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"

extern "C" SWIFT_CC(swift)
void _swift_exit(int result) {
  exit(result);
}

extern "C" SWIFT_CC(swift)
void swift_createDefaultExecutorsOnce() {
  static swift::once_t createExecutorsOnce;

  swift::once(createExecutorsOnce, swift_createDefaultExecutors);
}

#if SWIFT_STDLIB_TASK_TO_THREAD_MODEL_CONCURRENCY
extern "C" SWIFT_CC(swift)
SerialExecutorRef swift_getMainExecutor() {
  return SerialExecutorRef::generic();
}
#endif

extern "C" SWIFT_CC(swift)
void _swift_task_checkIsolatedSwift(HeapObject *executor,
                                    const Metadata *executorType,
                                    const SerialExecutorWitnessTable *witnessTable);

extern "C" SWIFT_CC(swift)
uint8_t swift_job_getPriority(Job *job) {
  return (uint8_t)(job->getPriority());
}

extern "C" SWIFT_CC(swift)
uint8_t swift_job_getKind(Job *job) {
  return (uint8_t)(job->Flags.getKind());
}

extern "C" SWIFT_CC(swift)
void *swift_job_getExecutorPrivateData(Job *job) {
  return &job->SchedulerPrivate[0];
}

#if SWIFT_CONCURRENCY_USES_DISPATCH
extern "C" SWIFT_CC(swift) __attribute__((noreturn))
void swift_dispatchMain() {
  dispatch_main();
}

extern "C" SWIFT_CC(swift)
void swift_dispatchAssertMainQueue() {
  dispatch_assert_queue(dispatch_get_main_queue());
}
#endif // SWIFT_CONCURRENCY_ENABLE_DISPATCH

#pragma clang diagnostic pop
