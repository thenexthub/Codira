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

#ifndef LANGUAGE_RUNTIME_CONCURRENCY_H
#define LANGUAGE_RUNTIME_CONCURRENCY_H

#include "language/ABI/AsyncLet.h"
#include "language/ABI/Task.h"
#include "language/ABI/TaskGroup.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"

// Does the runtime use a cooperative global executor?
#if defined(LANGUAGE_STDLIB_SINGLE_THREADED_CONCURRENCY)
#define LANGUAGE_CONCURRENCY_COOPERATIVE_GLOBAL_EXECUTOR 1
#else
#define LANGUAGE_CONCURRENCY_COOPERATIVE_GLOBAL_EXECUTOR 0
#endif

// Does the runtime use a task-thread model?
#if defined(LANGUAGE_STDLIB_TASK_TO_THREAD_MODEL_CONCURRENCY)
#define LANGUAGE_CONCURRENCY_TASK_TO_THREAD_MODEL 1
#else
#define LANGUAGE_CONCURRENCY_TASK_TO_THREAD_MODEL 0
#endif

// Does the runtime integrate with libdispatch?
#if defined(LANGUAGE_CONCURRENCY_USES_DISPATCH)
#define LANGUAGE_CONCURRENCY_ENABLE_DISPATCH LANGUAGE_CONCURRENCY_USES_DISPATCH
#else
#define LANGUAGE_CONCURRENCY_ENABLE_DISPATCH 0
#endif

namespace language {
class DefaultActor;
class TaskOptionRecord;

struct CodiraError;

struct AsyncTaskAndContext {
  AsyncTask *Task;
  AsyncContext *InitialContext;
};

/// Create a task object.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
AsyncTaskAndContext language_task_create(
    size_t taskCreateFlags,
    TaskOptionRecord *options,
    const Metadata *futureResultType,
    void *closureEntry, HeapObject *closureContext);

/// Caution: not all future-initializing functions actually throw, so
/// this signature may be incorrect.
using FutureAsyncSignature =
  AsyncSignature<void(void*), /*throws*/ true>;

/// Create a task object.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
AsyncTaskAndContext language_task_create_common(
    size_t taskCreateFlags,
    TaskOptionRecord *options,
    const Metadata *futureResultType,
    TaskContinuationFunction *function, void *closureContext,
    size_t initialContextSize);

#if LANGUAGE_CONCURRENCY_TASK_TO_THREAD_MODEL
#define LANGUAGE_TASK_RUN_INLINE_INITIAL_CONTEXT_BYTES 4096
/// Begin an async context in the current sync context and run the indicated
/// closure in it.
///
/// This is only supported under the task-to-thread concurrency model and
/// relies on a synchronous implementation of task blocking in order to work.
LANGUAGE_EXPORT_FROM(language_Concurrency)
LANGUAGE_CC(language)
void language_task_run_inline(OpaqueValue *result, void *closureAFP,
                           OpaqueValue *closureContext,
                           const Metadata *futureResultType);
#endif

/// Allocate memory in a task.
///
/// This must be called synchronously with the task.
///
/// All allocations will be rounded to a multiple of MAX_ALIGNMENT.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void *language_task_alloc(size_t size);

/// Deallocate memory in a task.
///
/// The pointer provided must be the last pointer allocated on
/// this task that has not yet been deallocated; that is, memory
/// must be allocated and deallocated in a strict stack discipline.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_dealloc(void *ptr);

/// Deallocate multiple memory allocations in a task.
///
/// The pointer provided must be a pointer previously allocated on
/// this task that has not yet been deallocated.  All allocations up to and
/// including that allocation will be deallocated.
LANGUAGE_EXPORT_FROM(language_Concurrency)
LANGUAGE_CC(language) void language_task_dealloc_through(void *ptr);

/// Deallocate memory in a task.
///
/// The pointer provided must be the last pointer allocated on
/// this task that has not yet been deallocated; that is, memory
/// must be allocated and deallocated in a strict stack discipline.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_dealloc(void *ptr);

/// Allocate memory in a job.
///
/// All allocations will be rounded to a multiple of MAX_ALIGNMENT;
/// if the job does not support allocation, this will return NULL.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void *language_job_allocate(Job *job, size_t size);

/// Deallocate memory in a job.
///
/// The pointer provided must be the last pointer allocated on
/// this task that has not yet been deallocated; that is, memory
/// must be allocated and deallocated in a strict stack discipline.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_job_deallocate(Job *job, void *ptr);

/// Cancel a task and all of its child tasks.
///
/// This can be called from any thread.
///
/// This has no effect if the task is already cancelled.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_cancel(AsyncTask *task);

/// Cancel all the child tasks that belong to the `group`.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_cancel_group_child_tasks(TaskGroup *group);

/// Escalate the priority of a task and all of its child tasks.
///
/// This can be called from any thread.
///
/// This has no effect if the task already has at least the given priority.
/// Returns the priority of the task.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
JobPriority
language_task_escalate(AsyncTask *task, JobPriority newPriority);

// TODO: "async let wait" and "async let destroy" would be expressed
//       similar to like TaskFutureWait;

/// Wait for a non-throwing future task to complete.
///
/// This can be called from any thread. Its Codira signature is
///
/// \code
/// fn language_task_future_wait(on task: _owned Builtin.NativeObject) async
///     -> Success
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(languageasync)
void language_task_future_wait(OpaqueValue *,
         LANGUAGE_ASYNC_CONTEXT AsyncContext *, AsyncTask *,
         TaskContinuationFunction *,
         AsyncContext *);

/// Wait for a potentially-throwing future task to complete.
///
/// This can be called from any thread. Its Codira signature is
///
/// \code
/// fn language_task_future_wait_throwing(on task: _owned Builtin.NativeObject)
///    async throws -> Success
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(languageasync)
void language_task_future_wait_throwing(
  OpaqueValue *,
  LANGUAGE_ASYNC_CONTEXT AsyncContext *,
  AsyncTask *,
  ThrowingTaskFutureWaitContinuationFunction *,
  AsyncContext *);

/// Wait for a readyQueue of a Channel to become non empty.
///
/// This can be called from any thread. Its Codira signature is
///
/// \code
/// fn language_taskGroup_wait_next_throwing(
///     waitingTask: Builtin.NativeObject, // current task
///     group: Builtin.RawPointer
/// ) async throws -> T
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency)
LANGUAGE_CC(languageasync)
void language_taskGroup_wait_next_throwing(
    OpaqueValue *resultPointer,
    LANGUAGE_ASYNC_CONTEXT AsyncContext *callerContext,
    TaskGroup *group,
    ThrowingTaskFutureWaitContinuationFunction *resumeFn,
    AsyncContext *callContext);

/// Initialize a `TaskGroup` in the passed `group` memory location.
/// The caller is responsible for retaining and managing the group's lifecycle.
///
/// Its Codira signature is
///
/// \code
/// fn language_taskGroup_initialize(group: Builtin.RawPointer)
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_taskGroup_initialize(TaskGroup *group, const Metadata *T);

/// Initialize a `TaskGroup` in the passed `group` memory location.
/// The caller is responsible for retaining and managing the group's lifecycle.
///
/// Its Codira signature is
///
/// \code
/// fn language_taskGroup_initialize(flags: Int, group: Builtin.RawPointer)
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_taskGroup_initializeWithFlags(size_t flags, TaskGroup *group, const Metadata *T);

/// Initialize a `TaskGroup` in the passed `group` memory location.
/// The caller is responsible for retaining and managing the group's lifecycle.
///
/// Its Codira signature is
///
/// \code
/// fn language_taskGroup_initialize(flags: Int, group: Builtin.RawPointer)
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_taskGroup_initializeWithOptions(size_t flags, TaskGroup *group, const Metadata *T, TaskOptionRecord *options);

/// Attach a child task to the parent task's task group record.
///
/// This function MUST be called from the AsyncTask running the task group.
///
/// Since the group (or rather, its record) is inserted in the parent task at
/// creation we do not need the parent task here, the group already is attached
/// to it.
/// Its Codira signature is
///
/// \code
/// fn language_taskGroup_attachChild(
///     group: Builtin.RawPointer,
///     child: Builtin.NativeObject
/// )
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_taskGroup_attachChild(TaskGroup *group, AsyncTask *child);

/// Its Codira signature is
///
/// This function MUST be called from the AsyncTask running the task group.
///
/// \code
/// fn language_taskGroup_destroy(_ group: Builtin.RawPointer)
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_taskGroup_destroy(TaskGroup *group);

/// Before starting a task group child task, inform the group that there is one
/// more 'pending' child to account for.
///
/// This function SHOULD be called from the AsyncTask running the task group,
/// however is generally thread-safe as it only works with the group status.
///
/// Its Codira signature is
///
/// \code
/// fn language_taskGroup_addPending(
///     group: Builtin.RawPointer,
///     unconditionally: Bool
/// ) -> Bool
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
bool language_taskGroup_addPending(TaskGroup *group, bool unconditionally);

/// Cancel all tasks in the group.
/// This also prevents new tasks from being added.
///
/// This can be called from any thread.
///
/// Its Codira signature is
///
/// \code
/// fn language_taskGroup_cancelAll(group: Builtin.RawPointer)
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_taskGroup_cancelAll(TaskGroup *group);

/// Check ONLY if the group was explicitly cancelled, e.g. by `cancelAll`.
///
/// This check DOES NOT take into account the task in which the group is running
/// being cancelled or not.
///
/// This can be called from any thread. Its Codira signature is
///
/// \code
/// fn language_taskGroup_isCancelled(group: Builtin.RawPointer)
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
bool language_taskGroup_isCancelled(TaskGroup *group);

/// Wait until all pending tasks from the task group have completed.
/// If this task group is accumulating results, this also discards all those results.
///
/// This can be called from any thread. Its Codira signature is
///
/// \code
/// fn language_taskGroup_waitAll(
///     waitingTask: Builtin.NativeObject, // current task
///     group: Builtin.RawPointer,
///     bodyError: Codira.Error?
/// ) async throws
/// \endcode
  LANGUAGE_EXPORT_FROM(language_Concurrency)
  LANGUAGE_CC(languageasync)
  void language_taskGroup_waitAll(
      OpaqueValue *resultPointer,
      LANGUAGE_ASYNC_CONTEXT AsyncContext *callerContext,
      TaskGroup *group,
      CodiraError *bodyError,
      ThrowingTaskFutureWaitContinuationFunction *resumeFn,
      AsyncContext *callContext);

/// Check the readyQueue of a task group, return true if it has no pending tasks.
///
/// This can be called from any thread. Its Codira signature is
///
/// \code
/// fn language_taskGroup_isEmpty(
///     _ group: Builtin.RawPointer
/// ) -> Bool
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
bool language_taskGroup_isEmpty(TaskGroup *group);

/// DEPRECATED. language_asyncLet_begin is used instead.
/// Its Codira signature is
///
/// \code
/// fn language_asyncLet_start<T>(
///     asyncLet: Builtin.RawPointer,
///     options: Builtin.RawPointer?,
///     operation: __owned @Sendable () async throws -> T
/// )
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_asyncLet_start(AsyncLet *alet,
                          TaskOptionRecord *options,
                          const Metadata *futureResultType,
                          void *closureEntryPoint, HeapObject *closureContext);

/// Begin an async let child task.
/// Its Codira signature is
///
/// \code
/// fn language_asyncLet_start<T>(
///     asyncLet: Builtin.RawPointer,
///     options: Builtin.RawPointer?,
///     operation: __owned @Sendable () async throws -> T,
///     resultBuffer: Builtin.RawPointer
/// )
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_asyncLet_begin(AsyncLet *alet,
                          TaskOptionRecord *options,
                          const Metadata *futureResultType,
                          void *closureEntryPoint, HeapObject *closureContext,
                          void *resultBuffer);

/// This matches the ABI of a closure `<T>(Builtin.RawPointer) async -> T`
using AsyncLetWaitSignature =
    LANGUAGE_CC(languageasync)
    void(OpaqueValue *,
         LANGUAGE_ASYNC_CONTEXT AsyncContext *, AsyncTask *, Metadata *);

/// DEPRECATED. language_asyncLet_get is used instead.
/// Wait for a non-throwing async-let to complete.
///
/// This can be called from any thread. Its Codira signature is
///
/// \code
/// fn language_asyncLet_wait(
///     _ asyncLet: _owned Builtin.RawPointer
/// ) async -> Success
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(languageasync)
void language_asyncLet_wait(OpaqueValue *,
                         LANGUAGE_ASYNC_CONTEXT AsyncContext *,
                         AsyncLet *, TaskContinuationFunction *,
                         AsyncContext *);

/// DEPRECATED. language_asyncLet_get_throwing is used instead.
/// Wait for a potentially-throwing async-let to complete.
///
/// This can be called from any thread. Its Codira signature is
///
/// \code
/// fn language_asyncLet_wait_throwing(
///     _ asyncLet: _owned Builtin.RawPointer
/// ) async throws -> Success
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(languageasync)
void language_asyncLet_wait_throwing(OpaqueValue *,
                                  LANGUAGE_ASYNC_CONTEXT AsyncContext *,
                                  AsyncLet *,
                                  ThrowingTaskFutureWaitContinuationFunction *,
                                  AsyncContext *);

/// DEPRECATED. language_asyncLet_finish is used instead.
/// Its Codira signature is
///
/// \code
/// fn language_asyncLet_end(_ alet: Builtin.RawPointer)
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_asyncLet_end(AsyncLet *alet);

/// Get the value of a non-throwing async-let, awaiting the result if necessary.
///
/// This can be called from any thread. Its Codira signature is
///
/// \code
/// fn language_asyncLet_get(
///     _ asyncLet: Builtin.RawPointer,
///     _ resultBuffer: Builtin.RawPointer
/// ) async
/// \endcode
///
/// \c result points at the variable storage for the binding. It is
/// uninitialized until the first call to \c language_asyncLet_get or
/// \c language_asyncLet_get_throwing. That first call initializes the storage
/// with the result of the child task. Subsequent calls do nothing and leave
/// the value in place.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(languageasync)
void language_asyncLet_get(LANGUAGE_ASYNC_CONTEXT AsyncContext *,
                        AsyncLet *,
                        void *,
                        TaskContinuationFunction *,
                        AsyncContext *);

/// Get the value of a throwing async-let, awaiting the result if necessary.
///
/// This can be called from any thread. Its Codira signature is
///
/// \code
/// fn language_asyncLet_get_throwing(
///     _ asyncLet: Builtin.RawPointer,
///     _ resultBuffer: Builtin.RawPointer
/// ) async throws
/// \endcode
///
/// \c result points at the variable storage for the binding. It is
/// uninitialized until the first call to \c language_asyncLet_get or
/// \c language_asyncLet_get_throwing. That first call initializes the storage
/// with the result of the child task. Subsequent calls do nothing and leave
/// the value in place. A pointer to the storage inside the child task is
/// returned if the task completes successfully, otherwise the error from the
/// child task is thrown.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(languageasync)
void language_asyncLet_get_throwing(LANGUAGE_ASYNC_CONTEXT AsyncContext *,
                                 AsyncLet *,
                                 void *,
                                 ThrowingTaskFutureWaitContinuationFunction *,
                                 AsyncContext *);

/// Exit the scope of an async-let binding. If the task is still running, it
/// is cancelled, and we await its completion; otherwise, we destroy the
/// value in the variable storage.
///
/// Its Codira signature is
///
/// \code
/// fn language_asyncLet_finish(_ asyncLet: Builtin.RawPointer,
///                            _ resultBuffer: Builtin.RawPointer) async
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(languageasync)
void language_asyncLet_finish(LANGUAGE_ASYNC_CONTEXT AsyncContext *,
                           AsyncLet *,
                           void *,
                           TaskContinuationFunction *,
                           AsyncContext *);

/// Get the value of a non-throwing async-let, awaiting the result if necessary,
/// and then destroy the child task. The result buffer is left initialized after
/// returning.
///
/// This can be called from any thread. Its Codira signature is
///
/// \code
/// fn language_asyncLet_get(
///     _ asyncLet: Builtin.RawPointer,
///     _ resultBuffer: Builtin.RawPointer
/// ) async
/// \endcode
///
/// \c result points at the variable storage for the binding. It is
/// uninitialized until the first call to \c language_asyncLet_get or
/// \c language_asyncLet_get_throwing. The child task will be invalidated after
/// this call, so the `async let` can not be gotten or finished afterward.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(languageasync)
void language_asyncLet_consume(LANGUAGE_ASYNC_CONTEXT AsyncContext *,
                            AsyncLet *,
                            void *,
                            TaskContinuationFunction *,
                            AsyncContext *);

/// Get the value of a throwing async-let, awaiting the result if necessary,
/// and then destroy the child task. The result buffer is left initialized after
/// returning.
///
/// This can be called from any thread. Its Codira signature is
///
/// \code
/// fn language_asyncLet_get_throwing(
///     _ asyncLet: Builtin.RawPointer,
///     _ resultBuffer: Builtin.RawPointer
/// ) async throws
/// \endcode
///
/// \c result points at the variable storage for the binding. It is
/// uninitialized until the first call to \c language_asyncLet_get or
/// \c language_asyncLet_get_throwing. That first call initializes the storage
/// with the result of the child task. Subsequent calls do nothing and leave
/// the value in place. The child task will be invalidated after
/// this call, so the `async let` can not be gotten or finished afterward.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(languageasync)
void language_asyncLet_consume_throwing(LANGUAGE_ASYNC_CONTEXT AsyncContext *,
                                     AsyncLet *,
                                     void *,
                                     ThrowingTaskFutureWaitContinuationFunction *,
                                     AsyncContext *);

/// Returns true if the currently executing AsyncTask has a
/// 'TaskGroupTaskStatusRecord' present.
///
/// This can be called from any thread.
///
/// Its Codira signature is
///
/// \code
/// fn language_taskGroup_hasTaskGroupRecord()
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
bool language_taskGroup_hasTaskGroupRecord(); // FIXME: not used? we have language_task_hasTaskGroupStatusRecord

/// Signifies whether the current task is in the middle of executing the
/// operation block of a `with(Throwing)TaskGroup(...) { <operation> }`.
///
/// Task local values must use un-structured allocation for values bound in this
/// scope, as they may be referred to by `group.spawn`-ed tasks and therefore
/// out-life the scope of a task-local value binding.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
bool language_task_hasTaskGroupStatusRecord();

/// Push an executor preference onto the current task.
/// The pushed reference does not keep the executor alive, and it is the
/// responsibility of the end user to ensure that the task executor reference
/// remains valid throughout the time it may be used by any task.
///
/// Runtime availability: Codira 9999.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
TaskExecutorPreferenceStatusRecord*
language_task_pushTaskExecutorPreference(TaskExecutorRef executor);

/// Remove a single task executor preference record from the current task.
///
/// Must be passed the record intended to be removed (returned by
/// `language_task_pushTaskExecutorPreference`).
///
/// Failure to remove the expected record should result in a runtime crash as it
/// signals a bug in record handling by the concurrency library -- a record push
/// must always be paired with a record pop.
///
/// Runtime availability: Codira 6.0
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_popTaskExecutorPreference(TaskExecutorPreferenceStatusRecord* record);

LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
size_t language_task_getJobFlags(AsyncTask* task);

LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
bool language_task_isCancelled(AsyncTask* task);

/// Returns the current priority of the task which is >= base priority of the
/// task. This function does not exist in the base ABI of this library and must
/// be deployment limited
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
JobPriority
language_task_currentPriority(AsyncTask *task);

/// Returns the base priority of the task. This function does not exist in the
/// base ABI of this library and must be deployment limited.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
JobPriority
language_task_basePriority(AsyncTask *task);

/// Returns the priority of the job.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
JobPriority
language_concurrency_jobPriority(Job *job);

/// Create and add an cancellation record to the task.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
CancellationNotificationStatusRecord*
language_task_addCancellationHandler(
    CancellationNotificationStatusRecord::FunctionType handler,
    void *handlerContext);

/// Remove the passed cancellation record from the task.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_removeCancellationHandler(
    CancellationNotificationStatusRecord *record);

/// Create and add an priority escalation record to the task.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
EscalationNotificationStatusRecord*
language_task_addPriorityEscalationHandler(
    EscalationNotificationStatusRecord::FunctionType handler,
    void *handlerContext);

/// Remove the passed priority escalation record from the task.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_removePriorityEscalationHandler(
    EscalationNotificationStatusRecord *record);

/// Create a NullaryContinuationJob from a continuation.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
NullaryContinuationJob*
language_task_createNullaryContinuationJob(
    size_t priority,
    AsyncTask *continuation);

LANGUAGE_EXPORT_FROM(language_Concurrency)
LANGUAGE_CC(language)
void language_task_deinitOnExecutor(void *object, DeinitWorkFunction *work,
                                 SerialExecutorRef newExecutor, size_t flags);

/// Report error about attempting to bind a task-local value from an illegal context.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_reportIllegalTaskLocalBindingWithinWithTaskGroup(
    const unsigned char *file, uintptr_t fileLength,
    bool fileIsASCII, uintptr_t line);

/// Get a task local value from either the current task, or fallback task-local
/// storage.
///
/// Its Codira signature is
///
/// \code
/// fn _taskLocalValueGet<Key>(
///   keyType: Any.Type /*Key.Type*/
/// ) -> UnsafeMutableRawPointer? where Key: TaskLocalKey
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
OpaqueValue*
language_task_localValueGet(const HeapObject *key);

/// Bind a task local key to a value in the context of either the current
/// AsyncTask if present, or in the thread-local fallback context if no task
/// available.
///
/// Its Codira signature is
///
/// \code
///  public fn _taskLocalValuePush<Value>(
///    keyType: Any.Type/*Key.Type*/,
///    value: __owned Value
///  )
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_localValuePush(const HeapObject *key,
                                   /* +1 */ OpaqueValue *value,
                                   const Metadata *valueType);

/// Pop a single task local binding from the binding stack of the current task,
/// or the fallback thread-local storage if no task is available.
///
/// This operation must be paired up with a preceding "push" operation, as otherwise
/// it may attempt to "pop" off an empty value stuck which will lead to a crash.
///
/// The Codira surface API ensures proper pairing of push and pop operations.
///
/// Its Codira signature is
///
/// \code
///  public fn _taskLocalValuePop()
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_localValuePop();

/// Copy all task locals from the current context to the target task.
///
/// Its Codira signature is
///
/// \code
/// fn language_task_localsCopyTo<Key>(AsyncTask* task)
/// \endcode
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_localsCopyTo(AsyncTask* target);

/// Switch the current task to a new executor if we aren't already
/// running on a compatible executor.
///
/// The resumption function pointer and continuation should be set
/// appropriately in the task.
///
/// Generally the compiler should inline a fast-path compatible-executor
/// check to avoid doing the suspension work.  This function should
/// generally be tail-called, as it may continue executing the task
/// synchronously if possible.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(languageasync)
void language_task_switch(LANGUAGE_ASYNC_CONTEXT AsyncContext *resumeToContext,
                       TaskContinuationFunction *resumeFunction,
                       SerialExecutorRef newExecutor);

/// Mark a task for enqueue on a new executor and then enqueue it.
///
/// The resumption function pointer and continuation should be set
/// appropriately in the task.
///
/// Generally you should call language_task_switch to switch execution
/// synchronously when possible.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void
language_task_enqueueTaskOnExecutor(AsyncTask *task, SerialExecutorRef executor);

/// Enqueue the given job to run asynchronously on the given executor.
///
/// The resumption function pointer and continuation should be set
/// appropriately in the task.
///
/// Generally you should call language_task_switch to switch execution
/// synchronously when possible.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_enqueue(Job *job, SerialExecutorRef executor);

/// Enqueue the given job to run asynchronously on the global
/// execution pool.
///
/// The resumption function pointer and continuation should be set
/// appropriately in the task.
///
/// Generally you should call language_task_switch to switch execution
/// synchronously when possible.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_enqueueGlobal(Job *job);

/// Invoke an executor's `checkIsolated` implementation;
/// It will crash if the current executor is NOT the passed executor.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_checkIsolated(SerialExecutorRef executor);

/// Invoke a Codira executor's `checkIsolated` implementation; returns
/// `true` if it invoked the Codira implementation, `false` otherwise.
/// Executors will want to call this from their `language_task_checkIsolatedImpl`
/// implementation.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
bool language_task_invokeCodiraCheckIsolated(SerialExecutorRef executor);

/// Invoke an executor's `isIsolatingCurrentContext` implementation;
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
int8_t language_task_isIsolatingCurrentContext(SerialExecutorRef executor);

/// Invoke a Codira executor's `isIsolatingCurrentContext` implementation; returns
/// `true` if it invoked the Codira implementation, `false` otherwise.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
int8_t language_task_invokeCodiraIsIsolatingCurrentContext(SerialExecutorRef executor);

/// A count in nanoseconds.
using JobDelay = unsigned long long;

LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_enqueueGlobalWithDelay(JobDelay delay, Job *job);

LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_enqueueGlobalWithDeadline(long long sec, long long nsec,
    long long tsec, long long tnsec, int clock, Job *job);

/// Enqueue the given job on the main executor.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_enqueueMainExecutor(Job *job);

/// WARNING: This method is expected to CRASH when caller is not on the
/// expected executor.
///
/// Return true if the caller is running in a Task on the passed Executor.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
bool language_task_isOnExecutor(
    HeapObject * executor,
    const Metadata *selfType,
    const SerialExecutorWitnessTable *wtable);

LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
bool language_executor_isComplexEquality(SerialExecutorRef ref);

/// Return the 64bit TaskID (if the job is an AsyncTask),
/// or the 32bits of the job Id otherwise.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
uint64_t language_task_getJobTaskId(Job *job);

#if LANGUAGE_CONCURRENCY_ENABLE_DISPATCH

/// Enqueue the given job on the main executor.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_enqueueOnDispatchQueue(Job *job, HeapObject *queue);

#endif

// Declare all the hooks
#define LANGUAGE_CONCURRENCY_HOOK(returnType, name, ...)                   \
  typedef LANGUAGE_CC(language) returnType (*name##_original)(__VA_ARGS__);   \
  typedef LANGUAGE_CC(language) returnType                                    \
    (*name##_hook_t)(__VA_ARGS__, name##_original original);            \
  LANGUAGE_EXPORT_FROM(language_Concurrency) name##_hook_t name##_hook

#define LANGUAGE_CONCURRENCY_HOOK0(returnType, name)                       \
  typedef LANGUAGE_CC(language) returnType (*name##_original)();              \
  typedef LANGUAGE_CC(language) returnType                                    \
    (*name##_hook_t)(name##_original original);                         \
  LANGUAGE_EXPORT_FROM(language_Concurrency) name##_hook_t name##_hook

#include "ConcurrencyHooks.def"

// This is a compatibility hook, *not* a concurrency hook
typedef LANGUAGE_CC(language) void (*language_task_asyncMainDrainQueue_original)();
typedef LANGUAGE_CC(language) void (*language_task_asyncMainDrainQueue_override)(
    language_task_asyncMainDrainQueue_original original);
LANGUAGE_EXPORT_FROM(language_Concurrency)
LANGUAGE_CC(language) void (*language_task_asyncMainDrainQueue_hook)(
    language_task_asyncMainDrainQueue_original original,
    language_task_asyncMainDrainQueue_override compatOverride);

/// Initialize the runtime storage for a default actor.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_defaultActor_initialize(DefaultActor *actor);

/// Destroy the runtime storage for a default actor.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_defaultActor_destroy(DefaultActor *actor);

/// Deallocate an instance of a default actor.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_defaultActor_deallocate(DefaultActor *actor);

/// Deallocate an instance of what might be a default actor.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_defaultActor_deallocateResilient(HeapObject *actor);

/// Initialize the runtime storage for a non-default distributed actor.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_nonDefaultDistributedActor_initialize(NonDefaultDistributedActor *actor);

/// Create and initialize the runtime storage for a distributed remote actor.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
OpaqueValue*
language_distributedActor_remote_initialize(const Metadata *actorType);

/// Enqueue a job on the default actor implementation.
///
/// The job must be ready to run.  Notably, if it's a task, that
/// means that the resumption function and context should have been
/// set appropriately.
///
/// Jobs are assumed to be "self-consuming": once it starts running,
/// the job memory is invalidated and the executor should not access it
/// again.
///
/// Jobs are generally expected to keep the actor alive during their
/// execution.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_defaultActor_enqueue(Job *job, DefaultActor *actor);

/// Check if the actor is a distributed 'remote' actor instance.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
bool language_distributed_actor_is_remote(HeapObject *actor);

/// Do a primitive suspension of the current task, as if part of
/// a continuation, although this does not provide any of the
/// higher-level continuation semantics.  The current task is returned;
/// its ResumeFunction and ResumeContext will need to be initialized,
/// and then it will need to be enqueued or run as a job later.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
AsyncTask *language_task_suspend();

/// Prepare a continuation in the current task.
///
/// The caller should initialize the Parent, ResumeParent,
/// and NormalResult fields.  This function will initialize the other
/// fields with appropriate defaults; the caller may then overwrite
/// them if desired.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
AsyncTask *language_continuation_init(ContinuationAsyncContext *context,
                                   AsyncContinuationFlags flags);

/// Await an initialized continuation.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(languageasync)
void language_continuation_await(ContinuationAsyncContext *continuationContext);

/// Resume a task from a non-throwing continuation, given a normal
/// result which has already been stored into the continuation.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_continuation_resume(AsyncTask *continuation);

/// Resume a task from a potentially-throwing continuation, given a
/// normal result which has already been stored into the continuation.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_continuation_throwingResume(AsyncTask *continuation);

/// Resume a task from a potentially-throwing continuation by throwing
/// an error.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_continuation_throwingResumeWithError(AsyncTask *continuation,
                                                /* +1 */ CodiraError *error);

/// SPI helper to log a misuse of a `CheckedContinuation` to the appropriate places in the OS.
extern "C" LANGUAGE_CC(language)
void language_continuation_logFailedCheck(const char *message);

/// Drain the queue
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_asyncMainDrainQueue [[noreturn]]();

/// Drain the global executor.  This does the same as the above, but
/// language_task_asyncMainDrainQueue() is a compatibility override point,
/// whereas this function has a concurrency hook.  The default
/// language_task_asyncMainDrainQueue() implementation just calls this function.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_drainGlobalExecutor [[noreturn]]();

/// Establish that the current thread is running as the given
/// executor, then run a job.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_job_run(Job *job, SerialExecutorRef executor);

/// Establish that the current thread is running as the given
/// executor, then run a job.
///
/// Runtime availability: Codira 6.0
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_job_run_on_task_executor(Job *job, TaskExecutorRef executor);

/// Establish that the current thread is running as the given
/// executor, then run a job.
///
/// Runtime availability: Codira 6.0
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_job_run_on_serial_and_task_executor(Job *job,
                                    SerialExecutorRef serialExecutor,
                                    TaskExecutorRef taskExecutor);

/// Return the current thread's active task reference.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
AsyncTask *language_task_getCurrent(void);

/// Return the current thread's active executor reference.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
SerialExecutorRef language_task_getCurrentExecutor(void);

/// Return the main-actor executor reference.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
SerialExecutorRef language_task_getMainExecutor(void);

/// Test if an executor is the main executor.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
bool language_task_isMainExecutor(SerialExecutorRef executor);

/// Return the preferred task executor of the current task,
/// or ``TaskExecutorRef::undefined()`` if no preference.
///
/// A stored preference may be `undefined` explicitly,
/// which is semantically equivalent to having no preference.
///
/// The returned reference must be treated carefully,
/// because it is *unmanaged*, meaning that the fact
/// that the task "has" this preference does not imply its lifetime.
///
/// Developers who use task executor preference MUST guarantee
/// their lifetime exceeds any use of such executor. For example,
/// they should be created as "forever" alive singletons, or otherwise
/// guarantee their lifetime extends beyond all potential uses of them by tasks.
///
/// Runtime availability: Codira 9999
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
TaskExecutorRef language_task_getPreferredTaskExecutor(void);

LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
bool language_task_isCurrentExecutor(SerialExecutorRef executor);

/// This is an options enum that is used to pass flags to
/// language_task_isCurrentExecutorWithFlags. It is meant to be a flexible toggle.
///
/// Since this is an options enum, so all values should be powers of 2.
///
/// NOTE: We are purposely leaving this as a uint64_t so that on all platforms
/// this could be a pointer to a different enum instance if we need it to be.
enum language_task_is_current_executor_flag : uint64_t {
  /// We aren't passing any flags.
  /// Effectively this is a backwards compatible mode.
  None = 0x0,

  /// This is not used today, but is just future ABI reservation.
  ///
  /// The intention is that we may want the ability to tell future versions of
  /// the runtime that this uint64_t is actually a pointer that it should
  /// dereference and then have further extended behavior controlled by a
  /// different enum. By placing this here, we ensure that we will have a tagged
  /// pointer compatible flag for this purpose.
  TaggedPointer = 0x1,

  /// This is not used today, but is just future ABI reservation.
  ///
  /// \see language_task_is_current_executor_flag::TaggedPointer
  TaggedPointer2 = 0x2,

  /// This is not used today, but is just future ABI reservation.
  ///
  /// \see language_task_is_current_executor_flag::TaggedPointer
  TaggedPointer3 = 0x4,

  /// The routine should assert on failure.
  Assert = 0x8,

  /// The routine MUST NOT assert on failure.
  /// Even at the cost of not calling 'checkIsolated' if it is available.
  MustNotAssert = 0x10,
};

LANGUAGE_EXPORT_FROM(language_Concurrency)
LANGUAGE_CC(language)
bool language_task_isCurrentExecutorWithFlags(
    SerialExecutorRef executor, language_task_is_current_executor_flag flags);

LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_reportUnexpectedExecutor(
    const unsigned char *file, uintptr_t fileLength, bool fileIsASCII,
    uintptr_t line, SerialExecutorRef executor);

LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
JobPriority language_task_getCurrentThreadPriority(void);

LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
const char *language_task_getCurrentTaskName(void);

LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_startOnMainActor(AsyncTask* job);

LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_immediate(AsyncTask* job, SerialExecutorRef targetExecutor);

/// Donate this thread to the global executor until either the
/// given condition returns true or we've run out of cooperative
/// tasks to run.
LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_task_donateThreadToGlobalExecutorUntil(bool (*condition)(void*),
                                                  void *context);

enum language_clock_id : int {
  language_clock_id_continuous = 1,
  language_clock_id_suspending = 2,
  language_clock_id_wall = 3
};

LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_get_time(long long *seconds,
                    long long *nanoseconds,
                    language_clock_id clock_id);

LANGUAGE_EXPORT_FROM(language_Concurrency) LANGUAGE_CC(language)
void language_get_clock_res(long long *seconds,
                         long long *nanoseconds,
                         language_clock_id clock_id);

#ifdef __APPLE__
/// A magic symbol whose address is the mask to apply to a frame pointer to
/// signal that it is an async frame. Do not try to read the actual value of
/// this global, it will crash.
///
/// On ARM64_32, the address is only 32 bits, and therefore this value covers
/// the top 32 bits of the in-memory frame pointer. On other 32-bit platforms,
/// the bit is not used and the address is always 0.
LANGUAGE_EXPORT_FROM(language_Concurrency)
struct { char c; } language_async_extendedFramePointerFlags;
#endif

}

#pragma clang diagnostic pop

#endif
