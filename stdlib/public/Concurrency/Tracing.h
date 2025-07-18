//===--- Tracing.h - Support code for concurrency tracing ----------*- C++ -*-//
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
// Support code for tracing events in the concurrency runtime
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CONCURRENCY_TRACING_H
#define LANGUAGE_CONCURRENCY_TRACING_H

#include <stdint.h>

namespace language {
class AsyncLet;
class AsyncTask;
class ContinuationAsyncContext;
class SerialExecutorRef;
struct HeapObject;
class Job;
class TaskGroup;
class TaskStatusRecord;

namespace concurrency {
namespace trace {

// Actor trace calls.

void actor_create(HeapObject *actor);

void actor_destroy(HeapObject *actor);

void actor_deallocate(HeapObject *actor);

void actor_enqueue(HeapObject *actor, Job *job);

void actor_dequeue(HeapObject *actor, Job *job);

// State values are:
// Idle = 0, Scheduled = 1, Running = 2, Zombie_ReadyForDeallocation = 3,
// invalid/unknown = 255
void actor_state_changed(HeapObject *actor, Job *firstJob, uint8_t state,
                         bool isDistributedRemote, bool isPriorityEscalated,
                         uint8_t maxPriority);

void actor_note_job_queue(HeapObject *actor, Job *first,
                          Job *(*getNext)(Job *));

// Task trace calls.

void task_create(AsyncTask *task, AsyncTask *parent, TaskGroup *group,
                 AsyncLet *asyncLet, uint8_t jobPriority, bool isChildTask,
                 bool isFuture, bool isGroupChildTask, bool isAsyncLetTask);

void task_destroy(AsyncTask *task);

void task_status_changed(AsyncTask *task, uint8_t maxPriority, bool isCancelled,
                         bool isEscalated, bool isStarting, bool isRunning, bool isEnqueued);

void task_flags_changed(AsyncTask *task, uint8_t jobPriority, bool isChildTask,
                        bool isFuture, bool isGroupChildTask,
                        bool isAsyncLetTask);

// The `status` parameter is the value of the corresponding
// FutureFragment::Status.
void task_wait(AsyncTask *task, AsyncTask *waitingOn, uintptr_t status);

void task_resume(AsyncTask *task);

// The context parameter is the context pointer used to create the continuation.
// This same pointer will be passed to the corresponding call to
// task_continuation_await and task_continuation_resume.
void task_continuation_init(AsyncTask *task, ContinuationAsyncContext *context);

void task_continuation_await(ContinuationAsyncContext *context);

void task_continuation_resume(ContinuationAsyncContext *context, bool error);

void job_enqueue_global(Job *job);

void job_enqueue_global_with_delay(unsigned long long delay, Job *job);

void job_enqueue_main_executor(Job *job);

struct job_run_info {
  /// The ID of the task that started running.
  uint64_t taskId;

  /// The signpost ID for this task execution, or OS_SIGNPOST_ID_INVALID
  /// if the job was not a task.
  uint64_t handle;
};

// This returns information that must be passed to the corresponding
// call to task_run_end.  Any information we want to log must be
// extracted from the job when we start to run it because execution
// will invalidate the job.
job_run_info job_run_begin(Job *job);

void job_run_end(job_run_info info);

} // namespace trace
} // namespace concurrency
} // namespace language

#if LANGUAGE_STDLIB_CONCURRENCY_TRACING
#include "TracingSignpost.h"
#else
#include "TracingStubs.h"
#endif

#endif
