//===--- TracingSignpost.h - Tracing with the signpost API ---------*- C++ -*-//
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

#ifndef LANGUAGE_CONCURRENCY_TRACINGSIGNPOST_H
#define LANGUAGE_CONCURRENCY_TRACINGSIGNPOST_H

#include "TaskPrivate.h"
#include "Tracing.h"
#include "language/ABI/Task.h"
#include "language/Basic/Lazy.h"
#include "language/Runtime/Casting.h"
#include "language/Runtime/HeapObject.h"
#include "language/Runtime/TracingCommon.h"
#include <inttypes.h>
#include <os/log.h>
#include <os/signpost.h>

// Compatibility notes:
//
// These signposts can be read by external software that isn't synced with the
// Codira runtime build. Changes here must be considered carefully to avoid
// breaking users of these signposts.
//
// We may:
// * Add new signpost calls with new names. (Keeping in mind that they won't be
//   picked up by software that doesn't know about them.)
// * Remove existing calls if the given event is somehow no longer relevant.
// * Change format strings.
// * Add format string arguments.
//
// We may NOT:
// * Change the order of existing format string arguments.
// * Change event names.
// * Change subsystem names.

#define LANGUAGE_LOG_ACTOR_LIFETIME_NAME "actor_lifetime"
#define LANGUAGE_LOG_ACTOR_DEALLOCATE_NAME "actor_deallocate"
#define LANGUAGE_LOG_ACTOR_ENQUEUE_NAME "actor_enqueue"
#define LANGUAGE_LOG_ACTOR_DEQUEUE_NAME "actor_dequeue"
#define LANGUAGE_LOG_ACTOR_STATE_CHANGED_NAME "actor_state_changed"
#define LANGUAGE_LOG_ACTOR_JOB_QUEUE_NAME "actor_job_queue"
#define LANGUAGE_LOG_TASK_LIFETIME_NAME "task_lifetime"
#define LANGUAGE_LOG_TASK_FLAGS_CHANGED_NAME "task_flags_changed"
#define LANGUAGE_LOG_TASK_STATUS_CHANGED_NAME "task_status_changed"
#define LANGUAGE_LOG_TASK_WAIT_NAME "task_wait"
#define LANGUAGE_LOG_TASK_CONTINUATION "task_continuation"
#define LANGUAGE_LOG_TASK_CONTINUATION_AWAIT "task_continuation_await"
#define LANGUAGE_LOG_JOB_ENQUEUE_GLOBAL_NAME "job_enqueue_global"
#define LANGUAGE_LOG_JOB_ENQUEUE_GLOBAL_WITH_DELAY_NAME                           \
  "job_enqueue_global_with_delay"
#define LANGUAGE_LOG_JOB_ENQUEUE_MAIN_EXECUTOR_NAME "job_enqueue_main_executor"
#define LANGUAGE_LOG_JOB_RUN_NAME "job_run"

namespace language {
namespace concurrency {
namespace trace {

extern os_log_t ActorLog;
extern os_log_t TaskLog;
extern language::once_t LogsToken;
extern bool TracingEnabled;

void setupLogs(void *unused);

// Check a representative os_signpost function for NULL rather than doing a
// standard availability check, for better performance if the check doesn't get
// optimized out.
#define ENSURE_LOGS(...)                                                       \
  do {                                                                         \
    if (!runtime::trace::tracingReady())                                       \
      return __VA_ARGS__;                                                      \
    language::once(LogsToken, setupLogs, nullptr);                                \
    if (!TracingEnabled)                                                       \
      return __VA_ARGS__;                                                      \
  } while (0)

// Every function does ENSURE_LOGS() before making any os_signpost calls, so
// we can skip availability checking on all the individual calls.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability"
#pragma clang diagnostic ignored "-Wunguarded-availability-new"

// Actor trace calls.

inline void actor_create(HeapObject *actor) {
  ENSURE_LOGS();

  // Do an explicit enabled check here to avoid the cost of language_getTypeName in
  // the common case.
  if (!os_signpost_enabled(ActorLog))
    return;

  auto typeName = language_getTypeName(language_getObjectType(actor), true);

  auto id = os_signpost_id_make_with_pointer(ActorLog, actor);
  os_signpost_interval_begin(ActorLog, id, LANGUAGE_LOG_ACTOR_LIFETIME_NAME,
                             "actor=%p typeName:%.*s", actor,
                             (int)typeName.length, typeName.data);
}

inline void actor_destroy(HeapObject *actor) {
  ENSURE_LOGS();
  auto id = os_signpost_id_make_with_pointer(ActorLog, actor);
  os_signpost_interval_end(ActorLog, id, LANGUAGE_LOG_ACTOR_LIFETIME_NAME,
                           "actor=%p", actor);
}

inline void actor_deallocate(HeapObject *actor) {
  ENSURE_LOGS();
  auto id = os_signpost_id_make_with_pointer(ActorLog, actor);
  os_signpost_event_emit(ActorLog, id, LANGUAGE_LOG_ACTOR_DEALLOCATE_NAME,
                         "actor=%p", actor);
}

inline void actor_enqueue(HeapObject *actor, Job *job) {
  if (AsyncTask *task = dyn_cast<AsyncTask>(job)) {
    ENSURE_LOGS();
    auto id = os_signpost_id_make_with_pointer(ActorLog, actor);
    os_signpost_event_emit(ActorLog, id, LANGUAGE_LOG_ACTOR_ENQUEUE_NAME,
                           "actor=%p task=%" PRId64, actor, task->getTaskId());
  }
}

inline void actor_dequeue(HeapObject *actor, Job *job) {
  if (AsyncTask *task = dyn_cast_or_null<AsyncTask>(job)) {
    ENSURE_LOGS();
    auto id = os_signpost_id_make_with_pointer(ActorLog, actor);
    os_signpost_event_emit(ActorLog, id, LANGUAGE_LOG_ACTOR_DEQUEUE_NAME,
                           "actor=%p task=%" PRId64, actor, task->getTaskId());
  }
}

inline void actor_state_changed(HeapObject *actor, Job *firstJob, uint8_t state,
                                bool isDistributedRemote,
                                bool isPriorityEscalated, uint8_t maxPriority) {
  ENSURE_LOGS();
  auto id = os_signpost_id_make_with_pointer(ActorLog, actor);
  os_signpost_event_emit(ActorLog, id, LANGUAGE_LOG_ACTOR_STATE_CHANGED_NAME,
                         "actor=%p needsPreprocessing=%d "
                         "state=%u isDistributedRemote=%{bool}d "
                         "isPriorityEscalated=%{bool}d, maxPriority=%u",
                         actor, (firstJob != nullptr), state,
                         isDistributedRemote, isPriorityEscalated, maxPriority);
}

inline void actor_note_job_queue(HeapObject *actor, Job *first,
                                 Job *(*getNext)(Job *)) {
  ENSURE_LOGS();

  // Do an explicit enabled check here, since the loop is potentially expensive.
  if (!os_signpost_enabled(ActorLog))
    return;

  // Count the number of pending jobs. We may want to track this separately
  // rather than looping to count, but this gets the job done for now.

  unsigned jobCount = 0;
  for (Job *job = first; job; job = getNext(job))
    if (isa<AsyncTask>(job))
      jobCount++;

  auto id = os_signpost_id_make_with_pointer(ActorLog, actor);
  os_signpost_event_emit(ActorLog, id, LANGUAGE_LOG_ACTOR_JOB_QUEUE_NAME,
                         "actor=%p jobCount=%u", actor, jobCount);
}

// Task trace calls.

inline void task_create(AsyncTask *task, AsyncTask *parent, TaskGroup *group,
                        AsyncLet *asyncLet, uint8_t jobPriority,
                        bool isChildTask, bool isFuture, bool isGroupChildTask,
                        bool isAsyncLetTask, bool isDiscardingTask,
                        bool hasInitialTaskExecutorPreference,
                        const char* taskName) {
  ENSURE_LOGS();
  auto id = os_signpost_id_make_with_pointer(TaskLog, task);
  auto parentID = parent ? parent->getTaskId() : 0;
  os_signpost_interval_begin(
      TaskLog, id, LANGUAGE_LOG_TASK_LIFETIME_NAME,
      "task=%" PRId64
      " resumefn=%p jobPriority=%u isChildTask=%{bool}d, isFuture=%{bool}d "
      "isGroupChildTask=%{bool}d isAsyncLetTask=%{bool}d parent=%" PRId64
      " group=%p asyncLet=%p "
      "isDiscardingTask=%{bool}d hasInitialTaskExecutorPreference=%{bool}d "
      "taskName=%{public}s",
      task->getTaskId(), task->getResumeFunctionForLogging(true), jobPriority,
      isChildTask, isFuture, isGroupChildTask, isAsyncLetTask, parentID, group,
      asyncLet, isDiscardingTask, hasInitialTaskExecutorPreference, taskName);
}

inline void task_destroy(AsyncTask *task) {
  ENSURE_LOGS();
  auto id = os_signpost_id_make_with_pointer(TaskLog, task);
  os_signpost_interval_end(TaskLog, id, LANGUAGE_LOG_TASK_LIFETIME_NAME,
                           "task=%" PRId64 "", task->getTaskId());
}

inline void task_status_changed(AsyncTask *task, uint8_t maxPriority,
                                bool isCancelled, bool isEscalated,
                                bool isStarting, bool isRunning, bool isEnqueued) {
  ENSURE_LOGS();
  auto id = os_signpost_id_make_with_pointer(TaskLog, task);
  os_signpost_event_emit(
      TaskLog, id, LANGUAGE_LOG_TASK_STATUS_CHANGED_NAME,
      "task=%" PRId64 " resumefn=%p "
      "maxPriority=%u, isCancelled=%{bool}d "
      "isEscalated=%{bool}d, isRunning=%{bool}d, isEnqueued=%{bool}d",
      task->getTaskId(), task->getResumeFunctionForLogging(isStarting), maxPriority,
      isCancelled, isEscalated, isRunning, isEnqueued);
}

inline void task_flags_changed(AsyncTask *task, uint8_t jobPriority,
                               bool isChildTask, bool isFuture,
                               bool isGroupChildTask, bool isAsyncLetTask) {
  ENSURE_LOGS();
  auto id = os_signpost_id_make_with_pointer(TaskLog, task);
  os_signpost_event_emit(
      TaskLog, id, LANGUAGE_LOG_TASK_FLAGS_CHANGED_NAME,
      "task=%" PRId64 " jobPriority=%u isChildTask=%{bool}d, isFuture=%{bool}d "
                      "isGroupChildTask=%{bool}d isAsyncLetTask=%{bool}d",
      task->getTaskId(), jobPriority, isChildTask, isFuture, isGroupChildTask,
      isAsyncLetTask);
}

inline void task_wait(AsyncTask *task, AsyncTask *waitingOn, uintptr_t status) {
  ENSURE_LOGS();
  auto id = os_signpost_id_make_with_pointer(TaskLog, task);
  auto waitingID = waitingOn ? waitingOn->getTaskId() : 0;
  os_signpost_interval_begin(TaskLog, id, LANGUAGE_LOG_TASK_WAIT_NAME,
                             "task=%" PRId64 " waitingOnTask=%" PRId64
                             " status=0x%" PRIxPTR,
                             task->getTaskId(), waitingID, status);
}

inline void task_resume(AsyncTask *task) {
  auto id = os_signpost_id_make_with_pointer(TaskLog, task);
  os_signpost_interval_end(TaskLog, id, LANGUAGE_LOG_TASK_WAIT_NAME,
                           "task=%" PRId64, task->getTaskId());
}

inline void task_continuation_init(AsyncTask *task,
                                   ContinuationAsyncContext *context) {
  ENSURE_LOGS();
  auto id = os_signpost_id_make_with_pointer(TaskLog, context);
  os_signpost_interval_begin(TaskLog, id, LANGUAGE_LOG_TASK_CONTINUATION,
                             "task=%" PRId64 " context=%p", task->getTaskId(),
                             context);
}

inline void task_continuation_await(ContinuationAsyncContext *context) {
  ENSURE_LOGS();
  auto id = os_signpost_id_make_with_pointer(TaskLog, context);
  os_signpost_event_emit(TaskLog, id, LANGUAGE_LOG_TASK_CONTINUATION_AWAIT,
                         "context=%p", context);
}

inline void task_continuation_resume(ContinuationAsyncContext *context,
                                     bool error) {
  ENSURE_LOGS();
  auto id = os_signpost_id_make_with_pointer(TaskLog, context);
  os_signpost_interval_end(TaskLog, id, LANGUAGE_LOG_TASK_CONTINUATION,
                           "context=%p error=%{bool}d", context, error);
}

inline void job_enqueue_global(Job *job) {
  if (AsyncTask *task = dyn_cast<AsyncTask>(job)) {
    ENSURE_LOGS();
    auto id = os_signpost_id_make_with_pointer(TaskLog, job);
    os_signpost_event_emit(TaskLog, id, LANGUAGE_LOG_JOB_ENQUEUE_GLOBAL_NAME,
                           "task=%" PRId64, task->getTaskId());
  }
}

inline void job_enqueue_global_with_delay(unsigned long long delay, Job *job) {
  if (AsyncTask *task = dyn_cast<AsyncTask>(job)) {
    ENSURE_LOGS();
    auto id = os_signpost_id_make_with_pointer(TaskLog, job);
    os_signpost_event_emit(
        TaskLog, id, LANGUAGE_LOG_JOB_ENQUEUE_GLOBAL_WITH_DELAY_NAME,
        "task=%" PRId64 " delay=%llu", task->getTaskId(), delay);
  }
}

inline void job_enqueue_main_executor(Job *job) {
  if (AsyncTask *task = dyn_cast<AsyncTask>(job)) {
    ENSURE_LOGS();
    auto id = os_signpost_id_make_with_pointer(TaskLog, job);
    os_signpost_event_emit(TaskLog, id,
                           LANGUAGE_LOG_JOB_ENQUEUE_MAIN_EXECUTOR_NAME,
                           "task=%" PRId64, task->getTaskId());
  }
}

inline job_run_info job_run_begin(Job *job) {
  auto invalidInfo = []{
    return job_run_info{ 0, OS_SIGNPOST_ID_INVALID };
  };

  if (AsyncTask *task = dyn_cast<AsyncTask>(job)) {
    ENSURE_LOGS(invalidInfo());
    auto handle = os_signpost_id_generate(TaskLog);
    auto taskId = task->getTaskId();
    os_signpost_interval_begin(TaskLog, handle, LANGUAGE_LOG_JOB_RUN_NAME,
                               "task=%" PRId64, taskId);
    return { taskId, handle };
  }
  return invalidInfo();
}

inline void job_run_end(job_run_info info) {
  if (info.handle != OS_SIGNPOST_ID_INVALID) {
    ENSURE_LOGS();
    os_signpost_interval_end(TaskLog, info.handle, LANGUAGE_LOG_JOB_RUN_NAME,
                             "task=%" PRId64, info.taskId);
  }
}

#pragma clang diagnostic pop

} // namespace trace
} // namespace concurrency
} // namespace language

#endif
