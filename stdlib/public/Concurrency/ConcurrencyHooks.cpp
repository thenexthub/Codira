///===--- Hooks.cpp - Concurrency hook variables --------------------------===///
///
/// This source file is part of the Codira.org open source project
///
/// Copyright (c) 2014 - 2020 Apple Inc. and the Codira project authors
/// Licensed under Apache License v2.0 with Runtime Library Exception
///
/// See https:///language.org/LICENSE.txt for license information
/// See https:///language.org/CONTRIBUTORS.txt for the list of Codira project authors
///
///===----------------------------------------------------------------------===///
///
/// Defines all of the hook variables.
///
///===----------------------------------------------------------------------===///

#include "language/ABI/Task.h"
#include "language/Runtime/Concurrency.h"
#include "language/Runtime/HeapObject.h"
#include "language/shims/Visibility.h"
#include "ExecutorImpl.h"
#include "TaskPrivate.h"

// Define all the hooks
#define LANGUAGE_CONCURRENCY_HOOK(returnType, name, ...)           \
  language::name##_hook_t language::name##_hook = nullptr
#define LANGUAGE_CONCURRENCY_HOOK0(returnType, name)               \
  language::name##_hook_t language::name##_hook = nullptr
#define LANGUAGE_CONCURRENCY_HOOK_OVERRIDE0(returnType, name)      \
  language::name##_hook_t language::name##_hook = nullptr

#include "language/Runtime/ConcurrencyHooks.def"

using namespace language;

// Define the external entry points; because the Impl functions use the C
// types from `ExecutorHooks.h`, we need to make an Orig version containing
// appropriate type casts in each case.

LANGUAGE_CC(language) static void
language_task_enqueueGlobalOrig(Job *job) {
  language_task_enqueueGlobalImpl(reinterpret_cast<CodiraJob *>(job));
}

void
language::language_task_enqueueGlobal(Job *job) {
  _language_tsan_release(job);

  concurrency::trace::job_enqueue_global(job);

  if (LANGUAGE_UNLIKELY(language_task_enqueueGlobal_hook)) {
    language_task_enqueueGlobal_hook(job, language_task_enqueueGlobalOrig);
  } else {
    language_task_enqueueGlobalOrig(job);
  }
}

LANGUAGE_CC(language) static void
language_task_enqueueGlobalWithDelayOrig(JobDelay delay, Job *job) {
  language_task_enqueueGlobalWithDelayImpl(
      static_cast<CodiraJobDelay>(delay), reinterpret_cast<CodiraJob *>(job));
}

void
language::language_task_enqueueGlobalWithDelay(JobDelay delay, Job *job) {
  concurrency::trace::job_enqueue_global_with_delay(delay, job);

  if (LANGUAGE_UNLIKELY(language_task_enqueueGlobalWithDelay_hook))
    language_task_enqueueGlobalWithDelay_hook(
      delay, job, language_task_enqueueGlobalWithDelayOrig);
  else
    language_task_enqueueGlobalWithDelayOrig(delay, job);
}

LANGUAGE_CC(language) static void
language_task_enqueueGlobalWithDeadlineOrig(
    long long sec,
    long long nsec,
    long long tsec,
    long long tnsec,
    int clock, Job *job) {
  language_task_enqueueGlobalWithDeadlineImpl(sec, nsec, tsec, tnsec, clock,
                                           reinterpret_cast<CodiraJob *>(job));
}

void
language::language_task_enqueueGlobalWithDeadline(
    long long sec,
    long long nsec,
    long long tsec,
    long long tnsec,
    int clock, Job *job) {
  if (LANGUAGE_UNLIKELY(language_task_enqueueGlobalWithDeadline_hook))
    language_task_enqueueGlobalWithDeadline_hook(
        sec, nsec, tsec, tnsec, clock, job,
        language_task_enqueueGlobalWithDeadlineOrig);
  else
    language_task_enqueueGlobalWithDeadlineOrig(sec, nsec, tsec, tnsec, clock, job);
}

LANGUAGE_CC(language) static void
language_task_checkIsolatedOrig(SerialExecutorRef executor) {
  language_task_checkIsolatedImpl(*reinterpret_cast<CodiraExecutorRef *>(&executor));
}

void
language::language_task_checkIsolated(SerialExecutorRef executor) {
  if (LANGUAGE_UNLIKELY(language_task_checkIsolated_hook))
    language_task_checkIsolated_hook(executor, language_task_checkIsolatedOrig);
  else
    language_task_checkIsolatedOrig(executor);
}

LANGUAGE_CC(language) static int8_t
language_task_isIsolatingCurrentContextOrig(SerialExecutorRef executor) {
  return language_task_isIsolatingCurrentContextImpl(
      *reinterpret_cast<CodiraExecutorRef *>(&executor));
}

int8_t
language::language_task_isIsolatingCurrentContext(SerialExecutorRef executor) {
  if (LANGUAGE_UNLIKELY(language_task_isIsolatingCurrentContext_hook)) {
    return language_task_isIsolatingCurrentContext_hook(
        executor, language_task_isIsolatingCurrentContextOrig);
  } else {
    return language_task_isIsolatingCurrentContextOrig(executor);
  }
}

// Implemented in Codira because we need to obtain the user-defined flags on the executor ref.
//
// We could inline this with effort, though.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C" LANGUAGE_CC(language)
language::SerialExecutorRef _task_serialExecutor_getExecutorRef(
        language::HeapObject *executor, const language::Metadata *selfType,
        const language::SerialExecutorWitnessTable *wtable);
#pragma clang diagnostic pop

/// WARNING: This method is expected to CRASH in new runtimes, and cannot be
/// used to implement "log warnings" mode. We would need a new entry point to
/// implement a "only log warnings" actor isolation checking mode, and it would
/// no be able handle more complex situations, as `SerialExecutor.checkIsolated`
/// is able to (by calling into dispatchPrecondition on old runtimes).
static LANGUAGE_CC(language) bool
language_task_isOnExecutorImpl(language::HeapObject *executor,
                            const language::Metadata *selfType,
                            const language::SerialExecutorWitnessTable *wtable)
{
  auto executorRef = _task_serialExecutor_getExecutorRef(executor,
                                                         selfType,
                                                         wtable);
  return language_task_isCurrentExecutor(executorRef);
}

bool
language::language_task_isOnExecutor(HeapObject *executor,
                               const Metadata *selfType,
                               const SerialExecutorWitnessTable *wtable) {
  if (LANGUAGE_UNLIKELY(language_task_isOnExecutor_hook))
    return language_task_isOnExecutor_hook(
             executor, selfType, wtable, language_task_isOnExecutorImpl);
  else
    return language_task_isOnExecutorImpl(executor, selfType, wtable);
}

LANGUAGE_CC(language) static void
language_task_enqueueMainExecutorOrig(Job *job) {
  language_task_enqueueMainExecutorImpl(reinterpret_cast<CodiraJob *>(job));
}

void
language::language_task_enqueueMainExecutor(Job *job) {
  concurrency::trace::job_enqueue_main_executor(job);
  if (LANGUAGE_UNLIKELY(language_task_enqueueMainExecutor_hook))
    language_task_enqueueMainExecutor_hook(job, language_task_enqueueMainExecutorOrig);
  else
    language_task_enqueueMainExecutorOrig(job);
}

LANGUAGE_CC(language) static language::SerialExecutorRef
language_task_getMainExecutorOrig() {
  auto ref = language_task_getMainExecutorImpl();
  return *reinterpret_cast<SerialExecutorRef *>(&ref);
}

language::SerialExecutorRef
language::language_task_getMainExecutor() {
  if (LANGUAGE_UNLIKELY(language_task_getMainExecutor_hook))
    return language_task_getMainExecutor_hook(language_task_getMainExecutorOrig);
  else {
    return language_task_getMainExecutorOrig();
  }
}

LANGUAGE_CC(language) static bool
language_task_isMainExecutorOrig(SerialExecutorRef executor) {
  return language_task_isMainExecutorImpl(
           *reinterpret_cast<CodiraExecutorRef *>(&executor));
}

bool
language::language_task_isMainExecutor(SerialExecutorRef executor) {
  if (LANGUAGE_UNLIKELY(language_task_isMainExecutor_hook))
    return language_task_isMainExecutor_hook(
             executor, language_task_isMainExecutorOrig);
  else
    return language_task_isMainExecutorOrig(executor);
}

LANGUAGE_CC(language) static void
language_task_donateThreadToGlobalExecutorUntilOrig(bool (*condition)(void *),
                                                 void *context) {
  return language_task_donateThreadToGlobalExecutorUntilImpl(condition, context);
}

void language::
language_task_donateThreadToGlobalExecutorUntil(bool (*condition)(void *),
                                             void *context) {
  if (LANGUAGE_UNLIKELY(language_task_donateThreadToGlobalExecutorUntil_hook))
    return language_task_donateThreadToGlobalExecutorUntil_hook(
              condition, context,
              language_task_donateThreadToGlobalExecutorUntilOrig);
  else
    return language_task_donateThreadToGlobalExecutorUntilOrig(condition, context);
}
