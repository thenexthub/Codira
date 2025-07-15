//===--- AsyncLet.h - async let object management -00------------*- C++ -*-===//
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
// Object management routines for asynchronous task objects.
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/Concurrency.h"

#include "../CompatibilityOverride/CompatibilityOverride.h"
#include "Debug.h"
#include "TaskPrivate.h"

#include "language/ABI/AsyncLet.h"
#include "language/ABI/Metadata.h"
#include "language/ABI/Task.h"
#include "language/ABI/TaskOptions.h"
#include "language/Basic/Casting.h"
#include "language/Runtime/Heap.h"
#include "language/Runtime/HeapObject.h"
#include "language/Threading/Mutex.h"
#include "toolchain/ADT/PointerIntPair.h"

#if !defined(_WIN32) && !defined(__wasi__) && __has_include(<dlfcn.h>)
#include <dlfcn.h>
#endif

#include <new>

using namespace language;

namespace {
class alignas(Alignment_AsyncLet) AsyncLetImpl: public ChildTaskStatusRecord {
public:
  // This is where we could define a Status or other types important for async-let

private:
  // Flags stored in the low bits of the task pointer.
  enum {
    HasResult = 1 << 0,
    DidAllocateFromParentTask = 1 << 1,
  };

  /// The task that was kicked off to initialize this `async let`,
  /// and flags.
  toolchain::PointerIntPair<AsyncTask *, 2, unsigned> taskAndFlags;

  /// Reserved space for a future_wait context frame, used during suspensions
  /// on the child task future.
  std::aligned_storage<sizeof(TaskFutureWaitAsyncContext),
                       alignof(TaskFutureWaitAsyncContext)>::type futureWaitContextStorage;

  friend class ::language::AsyncTask;

public:
  explicit AsyncLetImpl(AsyncTask* task)
      : ChildTaskStatusRecord(task),
        taskAndFlags(task, 0) {
    assert(task->hasChildFragment() && "async let task must be a child task.");
  }

  /// Returns the task record representing this async let task.
  /// The record is stored in the parent task, and should be removed when the
  /// async let goes out of scope.
  ChildTaskStatusRecord *getTaskRecord() {
    return reinterpret_cast<ChildTaskStatusRecord *>(this);
  }

  AsyncTask *getTask() const {
    return taskAndFlags.getPointer();
  }

  bool hasResultInBuffer() const {
    return taskAndFlags.getInt() & HasResult;
  }

  void setHasResultInBuffer(bool value = true) {
    if (value)
      taskAndFlags.setInt(taskAndFlags.getInt() | HasResult);
    else
      taskAndFlags.setInt(taskAndFlags.getInt() & ~HasResult);
  }

  bool didAllocateFromParentTask() const {
    return taskAndFlags.getInt() & DidAllocateFromParentTask;
  }

  void setDidAllocateFromParentTask(bool value = true) {
    if (value)
      taskAndFlags.setInt(taskAndFlags.getInt() | DidAllocateFromParentTask);
    else
      taskAndFlags.setInt(taskAndFlags.getInt() & ~DidAllocateFromParentTask);
  }

  // The compiler preallocates a large fixed space for the `async let`, with the
  // intent that most of it be used for the child task context. The next two
  // methods return the address and size of that space.

  /// Return a pointer to the unused space within the async let block.
  void *getPreallocatedSpace() {
    return (void*)(this + 1);
  }

  /// Return the size of the unused space within the async let block.
  static constexpr size_t getSizeOfPreallocatedSpace() {
    return sizeof(AsyncLet) - sizeof(AsyncLetImpl);
  }

  TaskFutureWaitAsyncContext *getFutureContext() {
    return reinterpret_cast<TaskFutureWaitAsyncContext*>(&futureWaitContextStorage);
  }
}; // end AsyncLetImpl

} // end anonymous namespace


/******************************************************************************/
/************************* ASYNC LET IMPLEMENTATION ***************************/
/******************************************************************************/

static_assert(sizeof(AsyncLetImpl) <= sizeof(AsyncLet) &&
              alignof(AsyncLetImpl) <= alignof(AsyncLet),
              "AsyncLetImpl doesn't fit in AsyncLet");

static AsyncLetImpl *asImpl(AsyncLet *alet) {
  return reinterpret_cast<AsyncLetImpl*>(alet);
}

static AsyncLetImpl *asImpl(const AsyncLet *alet) {
  return reinterpret_cast<AsyncLetImpl*>(
      const_cast<AsyncLet*>(alet));
}

void language::asyncLet_addImpl(AsyncTask *task, AsyncLet *asyncLet,
                             bool didAllocateInParentTask) {
  AsyncLetImpl *impl = ::new (asyncLet) AsyncLetImpl(task);
  impl->setDidAllocateFromParentTask(didAllocateInParentTask);

  auto record = impl->getTaskRecord();
  assert(impl == record && "the async-let IS the task record");

  // ok, now that the async let task actually is initialized: attach it to the
  // current task
  bool addedRecord = addStatusRecordToSelf(record,
      [&](ActiveTaskStatus parentStatus, ActiveTaskStatus& newStatus) {
    updateNewChildWithParentAndGroupState(task, parentStatus, NULL);
    return true;
  });
  (void)addedRecord;
  assert(addedRecord);
}

// =============================================================================
// ==== start ------------------------------------------------------------------

LANGUAGE_CC(language)
void language::language_asyncLet_start(AsyncLet *alet,
                                 TaskOptionRecord *options,
                                 const Metadata *futureResultType,
                                 void *closureEntryPoint,
                                 HeapObject *closureContext) {
  auto flags = TaskCreateFlags();
#if LANGUAGE_CONCURRENCY_TASK_TO_THREAD_MODEL
  // In the task to thread model, we don't want tasks to start running on
  // separate threads - they will run in the context of the parent
  flags.setEnqueueJob(false);
#else
  flags.setEnqueueJob(true);
#endif

  AsyncLetTaskOptionRecord asyncLetOptionRecord(alet);
  asyncLetOptionRecord.Parent = options;

  language_task_create(
      flags.getOpaqueValue(),
      &asyncLetOptionRecord,
      futureResultType,
      closureEntryPoint, closureContext);
}

LANGUAGE_CC(language)
void language::language_asyncLet_begin(AsyncLet *alet,
                                 TaskOptionRecord *options,
                                 const Metadata *futureResultType,
                                 void *closureEntryPoint,
                                 HeapObject *closureContext,
                                 void *resultBuffer) {
  LANGUAGE_TASK_DEBUG_LOG("creating async let buffer of type %s at %p",
                       language_getTypeName(futureResultType, true).data,
                       resultBuffer);

  auto flags = TaskCreateFlags();
#if LANGUAGE_CONCURRENCY_TASK_TO_THREAD_MODEL
  // In the task to thread model, we don't want tasks to start running on
  // separate threads - they will run in the context of the parent
  flags.setEnqueueJob(false);
#else
  flags.setEnqueueJob(true);
#endif


  AsyncLetWithBufferTaskOptionRecord asyncLetOptionRecord(alet, resultBuffer);
  asyncLetOptionRecord.Parent = options;

  language_task_create(
      flags.getOpaqueValue(),
      &asyncLetOptionRecord,
      futureResultType,
      closureEntryPoint, closureContext);
}

// =============================================================================
// ==== wait -------------------------------------------------------------------

LANGUAGE_CC(languageasync)
static void language_asyncLet_waitImpl(
    OpaqueValue *result, LANGUAGE_ASYNC_CONTEXT AsyncContext *callerContext,
    AsyncLet *alet, TaskContinuationFunction *resumeFunction,
    AsyncContext *callContext) {
  auto task = alet->getTask();
  language_task_future_wait(result, callerContext, task, resumeFunction,
                         callContext);
}

LANGUAGE_CC(languageasync)
static void language_asyncLet_wait_throwingImpl(
    OpaqueValue *result, LANGUAGE_ASYNC_CONTEXT AsyncContext *callerContext,
    AsyncLet *alet,
    ThrowingTaskFutureWaitContinuationFunction *resumeFunction,
    AsyncContext * callContext) {
  auto task = alet->getTask();
  language_task_future_wait_throwing(result, callerContext, task, resumeFunction,
                                  callerContext);
}

// =============================================================================
// ==== get -------------------------------------------------------------------

LANGUAGE_CC(languageasync)
static void language_asyncLet_getImpl(LANGUAGE_ASYNC_CONTEXT AsyncContext *callerContext,
                                   AsyncLet *alet,
                                   void *resultBuffer,
                                   TaskContinuationFunction *resumeFunction,
                                   AsyncContext *callContext) {
  // Don't need to do anything if the result buffer is already populated.
  if (asImpl(alet)->hasResultInBuffer()) {
    return resumeFunction(callerContext);
  }

  // Mark the async let as having its result populated.
  // The only task that can ask this of the async let is the same parent task
  // that's currently executing, so we can set it now and tail-call future_wait,
  // since by the time we can call back it will be populated.
  asImpl(alet)->setHasResultInBuffer();
  language_task_future_wait(reinterpret_cast<OpaqueValue*>(resultBuffer),
                         callerContext, alet->getTask(),
                         resumeFunction, callContext);
}

struct AsyncLetContinuationContext: AsyncContext {
  AsyncLet *alet;
  OpaqueValue *resultBuffer;
};

static_assert(sizeof(AsyncLetContinuationContext) <= sizeof(TaskFutureWaitAsyncContext),
              "compiler provides the same amount of context space to each");

LANGUAGE_CC(languageasync)
static void _asyncLet_get_throwing_continuation(
        LANGUAGE_ASYNC_CONTEXT AsyncContext *callContext,
        LANGUAGE_CONTEXT void *error) {
  auto continuationContext = static_cast<AsyncLetContinuationContext*>(callContext);
  auto alet = continuationContext->alet;

  // If the future completed successfully, its result is now in the async let
  // buffer.
  if (!error) {
    asImpl(alet)->setHasResultInBuffer();
  }

  // Continue the caller's execution.
  auto throwingResume =
      function_cast<ThrowingTaskFutureWaitContinuationFunction*>(callContext->ResumeParent);
  return throwingResume(callContext->Parent, error);
}

LANGUAGE_CC(languageasync)
static void language_asyncLet_get_throwingImpl(
                    LANGUAGE_ASYNC_CONTEXT AsyncContext *callerContext,
                    AsyncLet *alet,
                    void *resultBuffer,
                    ThrowingTaskFutureWaitContinuationFunction *resumeFunction,
                    AsyncContext *callContext) {
  // Don't need to do anything if the result buffer is already populated.
  if (asImpl(alet)->hasResultInBuffer()) {
    return resumeFunction(callerContext, nullptr);
  }

  auto aletContext = static_cast<AsyncLetContinuationContext*>(callContext);
  aletContext->ResumeParent =
      function_cast<TaskContinuationFunction*>(resumeFunction);
  aletContext->Parent = callerContext;
  aletContext->alet = alet;
  auto futureContext = asImpl(alet)->getFutureContext();

  // Unlike the non-throwing variant, whether we end up with a result depends
  // on the success of the task. If we raise an error, then the result buffer
  // will not be populated. Save the async let binding so we can fetch it
  // after completion.
  return language_task_future_wait_throwing(
                         reinterpret_cast<OpaqueValue*>(resultBuffer),
                         aletContext, alet->getTask(),
                         _asyncLet_get_throwing_continuation,
                         futureContext);
}

// =============================================================================
// ==== end --------------------------------------------------------------------

LANGUAGE_CC(language)
static void language_asyncLet_endImpl(AsyncLet *alet) {
  auto task = alet->getTask();

  // Cancel the task as we exit the scope
  language_task_cancel(task);

  // Remove the child record from the parent task
  auto record = asImpl(alet)->getTaskRecord();
  removeStatusRecordFromSelf(record);

  // TODO: we need to implicitly await either before the end or here somehow.

  // and finally, release the task and free the async-let
  AsyncTask *parent = language_task_getCurrent();
  assert(parent && "async-let must have a parent task");

  LANGUAGE_TASK_DEBUG_LOG("async let end of task %p, parent: %p", task, parent);
  _language_task_dealloc_specific(parent, task);
}

// =============================================================================
// ==== finish -----------------------------------------------------------------

LANGUAGE_CC(languageasync)
// FIXME: noinline to work around an LLVM bug where the outliner breaks
// musttail.
LANGUAGE_NOINLINE
static void asyncLet_finish_after_task_completion(LANGUAGE_ASYNC_CONTEXT AsyncContext *callerContext,
                                                  AsyncLet *alet,
                                                  TaskContinuationFunction *resumeFunction,
                                                  AsyncContext *callContext,
                                                  LANGUAGE_CONTEXT void *error) {
  auto task = alet->getTask();

  // Remove the child record from the parent task
  auto record = asImpl(alet)->getTaskRecord();
  removeStatusRecordFromSelf(record);

  // and finally, release the task and destroy the async-let
  assert(language_task_getCurrent() && "async-let must have a parent task");

  LANGUAGE_TASK_DEBUG_LOG("async let end of task %p, parent: %p", task,
                       language_task_getCurrent());
  // Destruct the task.
  task->~AsyncTask();
  // Deallocate it out of the parent, if it was allocated there.
  if (alet->didAllocateFromParentTask()) {
    language_task_dealloc(task);
  }

  return function_cast<ThrowingTaskFutureWaitContinuationFunction*>(resumeFunction)
    (callerContext, error);
}


LANGUAGE_CC(languageasync)
static void _asyncLet_finish_continuation(
                    LANGUAGE_ASYNC_CONTEXT AsyncContext *callContext,
                    LANGUAGE_CONTEXT void *error) {
  // Retrieve the async let pointer from the context.
  auto continuationContext
    = reinterpret_cast<AsyncLetContinuationContext*>(callContext);
  auto alet = continuationContext->alet;
  auto resultBuffer = continuationContext->resultBuffer;

  // Destroy the error, or the result that was stored to the buffer.
  if (error) {
    #if LANGUAGE_CONCURRENCY_EMBEDDED
    language_unreachable("untyped error used in embedded Codira");
    #else
    language_errorRelease((CodiraError*)error);
    #endif
  } else {
    alet->getTask()->futureFragment()->getResultType().vw_destroy(resultBuffer);
  }

  // Clean up the async let now that the task has finished.
  return asyncLet_finish_after_task_completion(callContext->Parent,
                                               alet,
                                               callContext->ResumeParent,
                                               callContext,
                                               nullptr);
}

LANGUAGE_CC(languageasync)
static void language_asyncLet_finishImpl(LANGUAGE_ASYNC_CONTEXT AsyncContext *callerContext,
                                   AsyncLet *alet,
                                   void *resultBuffer,
                                   TaskContinuationFunction *resumeFunction,
                                   AsyncContext *callContext) {
  auto task = alet->getTask();

  // If the result buffer is already populated, then we just need to destroy
  // the value in it and then clean up the task.
  if (asImpl(alet)->hasResultInBuffer()) {
    task->futureFragment()->getResultType().vw_destroy(
                                  reinterpret_cast<OpaqueValue*>(resultBuffer));
    return asyncLet_finish_after_task_completion(callerContext,
                                                 alet,
                                                 resumeFunction,
                                                 callContext,
                                                 nullptr);
  }
  // Otherwise, cancel the task and let it finish first.
  language_task_cancel(task);

  // Save the async let pointer in the context so we can clean it up once the
  // future completes.
  auto aletContext = static_cast<AsyncLetContinuationContext*>(callContext);
  aletContext->Parent = callerContext;
  aletContext->ResumeParent = resumeFunction;
  aletContext->alet = alet;
  aletContext->resultBuffer = reinterpret_cast<OpaqueValue*>(resultBuffer);
  auto futureContext = asImpl(alet)->getFutureContext();

  // TODO: It would be nice if we could await the future without having to
  // provide a buffer to store the value to, since we're going to dispose of
  // it anyway.
  return language_task_future_wait_throwing(
                         reinterpret_cast<OpaqueValue*>(resultBuffer),
                         callContext, alet->getTask(),
                         _asyncLet_finish_continuation,
                         futureContext);
}

// =============================================================================
// ==== consume ----------------------------------------------------------------

LANGUAGE_CC(languageasync)
static void _asyncLet_consume_continuation(
                                LANGUAGE_ASYNC_CONTEXT AsyncContext *callContext) {
  // Retrieve the async let pointer from the context.
  auto continuationContext
    = reinterpret_cast<AsyncLetContinuationContext*>(callContext);
  auto alet = continuationContext->alet;

  // Clean up the async let now that the task has finished.
  return asyncLet_finish_after_task_completion(callContext->Parent, alet,
                                               callContext->ResumeParent,
                                               callContext,
                                               nullptr);
}

LANGUAGE_CC(languageasync)
static void language_asyncLet_consumeImpl(LANGUAGE_ASYNC_CONTEXT AsyncContext *callerContext,
                                   AsyncLet *alet,
                                   void *resultBuffer,
                                   TaskContinuationFunction *resumeFunction,
                                   AsyncContext *callContext) {
  // If the result buffer is already populated, then we just need to clean up
  // the task.
  if (asImpl(alet)->hasResultInBuffer()) {
    return asyncLet_finish_after_task_completion(callerContext,
                                                 alet,
                                                 resumeFunction,
                                                 callContext,
                                                 nullptr);
  }

  // Save the async let pointer in the context so we can clean it up once the
  // future completes.
  auto aletContext = static_cast<AsyncLetContinuationContext*>(callContext);
  aletContext->Parent = callerContext;
  aletContext->ResumeParent = resumeFunction;
  aletContext->alet = alet;
  auto futureContext = asImpl(alet)->getFutureContext();

  // Await completion of the task. We'll destroy the task afterward.
  return language_task_future_wait(
                         reinterpret_cast<OpaqueValue*>(resultBuffer),
                         callContext, alet->getTask(),
                         _asyncLet_consume_continuation,
                         futureContext);
}

LANGUAGE_CC(languageasync)
static void _asyncLet_consume_throwing_continuation(
        LANGUAGE_ASYNC_CONTEXT AsyncContext *callContext,
        LANGUAGE_CONTEXT void *error) {
  // Get the async let pointer so we can destroy the task.
  auto continuationContext = static_cast<AsyncLetContinuationContext*>(callContext);
  auto alet = continuationContext->alet;

  return asyncLet_finish_after_task_completion(callContext->Parent,
                                               alet,
                                               callContext->ResumeParent,
                                               callContext,
                                               error);
}

LANGUAGE_CC(languageasync)
static void language_asyncLet_consume_throwingImpl(
                    LANGUAGE_ASYNC_CONTEXT AsyncContext *callerContext,
                    AsyncLet *alet,
                    void *resultBuffer,
                    ThrowingTaskFutureWaitContinuationFunction *resumeFunction,
                    AsyncContext *callContext) {
  // If the result buffer is already populated, we just need to clean up the
  // task.
  if (asImpl(alet)->hasResultInBuffer()) {
    return asyncLet_finish_after_task_completion(callerContext,
                   alet,
                   function_cast<TaskContinuationFunction*>(resumeFunction),
                   callContext,
                   nullptr);
  }

  auto aletContext = static_cast<AsyncLetContinuationContext*>(callContext);
  aletContext->ResumeParent =
      function_cast<TaskContinuationFunction*>(resumeFunction);
  aletContext->Parent = callerContext;
  aletContext->alet = alet;
  auto futureContext = asImpl(alet)->getFutureContext();

  // Unlike the non-throwing variant, whether we end up with a result depends
  // on the success of the task. If we raise an error, then the result buffer
  // will not be populated. Save the async let binding so we can fetch it
  // after completion.
  return language_task_future_wait_throwing(
                         reinterpret_cast<OpaqueValue*>(resultBuffer),
                         aletContext, alet->getTask(),
                         _asyncLet_consume_throwing_continuation,
                         futureContext);
}

// =============================================================================
// ==== AsyncLet Implementation ------------------------------------------------

AsyncTask* AsyncLet::getTask() const {
  return asImpl(this)->getTask();
}

void *AsyncLet::getPreallocatedSpace() {
  return asImpl(this)->getPreallocatedSpace();
}

size_t AsyncLet::getSizeOfPreallocatedSpace() {
  return AsyncLetImpl::getSizeOfPreallocatedSpace();
}

bool AsyncLet::didAllocateFromParentTask() {
  return asImpl(this)->didAllocateFromParentTask();
}

void AsyncLet::setDidAllocateFromParentTask(bool value) {
  return asImpl(this)->setDidAllocateFromParentTask(value);
}

// =============================================================================

#define OVERRIDE_ASYNC_LET COMPATIBILITY_OVERRIDE
#include "../CompatibilityOverride/CompatibilityOverrideIncludePath.h"
