///===--- ExecutorImpl.h - Global executor implementation interface --------===///
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
/// Contains the declarations you need to write a custom global
/// executor in plain C; this file intentionally does not include any
/// Codira ABI headers, because pulling those in necessitates use of
/// C++ and also pulls in a whole load of other things we don't need.
///
/// Note also that the global executor is expected to be statically
/// linked with language_Concurrency, so we needn't worry about dynamic
/// linking from here.
///
///===----------------------------------------------------------------------===///

#ifndef LANGUAGE_CONCURRENCY_EXECUTORIMPL_H
#define LANGUAGE_CONCURRENCY_EXECUTORIMPL_H

#if !defined(__language__) && __has_feature(ptrauth_calls)
#include <ptrauth.h>
#endif
#ifndef __ptrauth_objc_isa_pointer
#define __ptrauth_objc_isa_pointer
#endif

#include <inttypes.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LANGUAGE_CC
#define LANGUAGE_CC(x)     LANGUAGE_CC_##x
#define LANGUAGE_CC_language  __attribute__((languagecall))
#endif

#ifndef LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN
#define LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN __attribute__((noreturn))
#endif

// -- C versions of types executors might need ---------------------------------

/// Represents a Codira type
typedef struct CodiraHeapMetadata CodiraHeapMetadata;

/// Jobs have flags, which currently encode a kind and a priority
typedef uint32_t CodiraJobFlags;

typedef size_t CodiraJobKind;
enum {
  CodiraTaskJobKind = 0,

  // Job kinds >= 192 are private to the implementation
  CodiraFirstReservedJobKind = 192,
};

typedef size_t CodiraJobPriority;
enum {
  CodiraUserInteractiveJobPriority = 0x21, /* UI */
  CodiraUserInitiatedJobPriority   = 0x19, /* IN */
  CodiraDefaultJobPriority         = 0x15, /* DEF */
  CodiraUtilityJobPriority         = 0x11, /* UT */
  CodiraBackgroundJobPriority      = 0x09, /* BG */
  CodiraUnspecifiedJobPriority     = 0x00, /* UN */
};

enum { CodiraJobPriorityBucketCount = 5 };

static inline int language_priority_getBucketIndex(CodiraJobPriority priority) {
  if (priority > CodiraUserInitiatedJobPriority)
    return 0;
  else if (priority > CodiraDefaultJobPriority)
    return 1;
  else if (priority > CodiraUtilityJobPriority)
    return 2;
  else if (priority > CodiraBackgroundJobPriority)
    return 3;
  else
    return 4;
}

/// Used by the Concurrency runtime to represent a job.  The `schedulerPrivate`
/// field may be freely used by the executor implementation.
typedef struct {
  CodiraHeapMetadata const *__ptrauth_objc_isa_pointer _Nonnull metadata;
  uintptr_t refCounts;
  void * _Nullable schedulerPrivate[2];
  CodiraJobFlags flags;
} __attribute__((aligned(2 * sizeof(void *)))) CodiraJob;

/// Indexes in the schedulerPrivate array
enum {
  CodiraJobNextWaitingTaskIndex = 0,

  // These are only relevant for the Dispatch executor
  CodiraJobDispatchHasLongObjectHeader = sizeof(void *) == sizeof(int),
  CodiraJobDispatchLinkageIndex = CodiraJobDispatchHasLongObjectHeader ? 1 : 0,
  CodiraJobDispatchQueueIndex = CodiraJobDispatchHasLongObjectHeader? 0 : 1
};

/// Get the kind of a job, by directly accessing the flags field.
static inline CodiraJobKind language_job_getKind(CodiraJob * _Nonnull job) {
  return (CodiraJobKind)(job->flags & 0xff);
}

/// Get the priority of a job, by directly accessing the flags field.
static inline CodiraJobPriority language_job_getPriority(CodiraJob * _Nonnull job) {
  return (CodiraJobPriority)((job->flags >> 8) & 0xff);
}

/// Get a pointer to the scheduler private data.
static inline void * _Nonnull * _Nonnull language_job_getPrivateData(CodiraJob * _Nonnull job) {
  return &job->schedulerPrivate[0];
}

/// Allocate memory associated with a job.
void * _Nullable language_job_alloc(CodiraJob * _Nonnull job, size_t size);

/// Release memory allocated using `language_job_alloc()`.
void language_job_dealloc(CodiraJob * _Nonnull job, void * _Nonnull ptr);

/// Codira's refcounted objects start with this header
typedef struct {
  CodiraHeapMetadata const *__ptrauth_objc_isa_pointer _Nonnull metadata;
} CodiraHeapObject;

/// A reference to an executor consists of two words; the first is a pointer
/// which may or may not be to a Codira heap object.
typedef struct {
  CodiraHeapObject * _Nullable identity;
  uintptr_t implementation;
} CodiraExecutorRef;

/// Indicates whether or not an executor can be compared by just looking at
/// the `identity` field.
typedef unsigned CodiraExecutorKind;
enum {
  CodiraExecutorOrdinaryKind = 0,
  CodiraExecutorComplexEqualityKind = 1
};

typedef struct CodiraExecutorWitnessTable CodiraExecutorWitnessTable;

/// Return the generic executor.
static inline CodiraExecutorRef language_executor_generic(void) {
  return (CodiraExecutorRef){ NULL, 0 };
}

/// Return an ordinary executor with the specified identity and witness table.
static inline CodiraExecutorRef
language_executor_ordinary(CodiraHeapObject * _Nullable identity,
                        CodiraExecutorWitnessTable * _Nullable witnessTable) {
  return (CodiraExecutorRef){ identity,
                             (uintptr_t)witnessTable
                             | CodiraExecutorOrdinaryKind };
}

/// Return a complex equality executor with the specified identity and
/// witness table.
static inline CodiraExecutorRef
language_executor_complexEquality(CodiraHeapObject * _Nullable identity,
                               CodiraExecutorWitnessTable * _Nullable witnessTable) {
  return (CodiraExecutorRef){ identity,
                             (uintptr_t)witnessTable
                             | CodiraExecutorComplexEqualityKind };
}

/// Get the type of executor (ordinary vs complex equality).
static inline CodiraExecutorKind
language_executor_getKind(CodiraExecutorRef executor) {
  const uintptr_t mask = ~(uintptr_t)(sizeof(void *) - 1);
  return executor.implementation & ~mask;
}

/// Return `true` if this is the generic executor.
static inline bool language_executor_isGeneric(CodiraExecutorRef executor) {
  return executor.identity == NULL;
}

/// Return `true` if this executor is in fact the default actor.
static inline bool language_executor_isDefaultActor(CodiraExecutorRef executor) {
  return !language_executor_isGeneric(executor) && executor.implementation == 0;
}

/// Retrieve the identity of an executor.
static inline CodiraHeapObject * _Nullable
language_executor_getIdentity(CodiraExecutorRef executor) {
  return executor.identity;
}

/// Test if an executor has a valid witness table.
static inline bool language_executor_hasWitnessTable(CodiraExecutorRef executor) {
  return (!language_executor_isGeneric(executor)
          && !language_executor_isDefaultActor(executor));
}

/// Retrieve the witness table of an executor.
static inline const CodiraExecutorWitnessTable * _Nullable
language_executor_getWitnessTable(CodiraExecutorRef executor) {
  const uintptr_t mask = ~(uintptr_t)(sizeof(void *) - 1);
  return (const CodiraExecutorWitnessTable *)(executor.implementation & mask);
}

/// Test if this is the main executor.
static inline bool language_executor_isMain(CodiraExecutorRef executor) {
  extern bool _language_task_isMainExecutor_c(CodiraExecutorRef executor);

  return _language_task_isMainExecutor_c(executor);
}

/// Assert that we are isolated on the specified executor.
static inline bool
language_executor_invokeCodiraCheckIsolated(CodiraExecutorRef executor) {
  extern bool _language_task_invokeCodiraCheckIsolated_c(CodiraExecutorRef executor);

  return _language_task_invokeCodiraCheckIsolated_c(executor);
}

/// Check if the current context is isolated by the specified executor.
///
/// The numeric values correspond to `language::IsIsolatingCurrentContextDecision`.
///
/// Specifically ONLY `1` means "isolated", while smaller values mean not isolated or unknown.
/// See ``IsIsolatingCurrentContextDecision`` for details.
static inline int8_t
language_executor_invokeCodiraIsIsolatingCurrentContext(CodiraExecutorRef executor) {
  extern int8_t _language_task_invokeCodiraIsIsolatingCurrentContext_c(CodiraExecutorRef executor);

  return _language_task_invokeCodiraIsIsolatingCurrentContext_c(executor);
}

/// Execute the specified job while running on the specified executor.
static inline void language_job_run(CodiraJob * _Nonnull job,
                                 CodiraExecutorRef executor) {
  extern void _language_job_run_c(CodiraJob * _Nonnull job,
                               CodiraExecutorRef executor);

  _language_job_run_c(job, executor);
}

/// A delay in nanoseconds.
typedef unsigned long long CodiraJobDelay;

/// Names of clocks supported by the Codira time functions.
typedef int CodiraClockId;
enum {
  CodiraContinuousClock = 1,
  CodiraSuspendingClock = 2
};

/// An accurate timestamp, with *up to* nanosecond precision.
typedef struct {
  long long seconds;
  long long nanoseconds;
} CodiraTime;

/// Get the current time from the specified clock
extern CodiraTime language_time_now(CodiraClockId clock);

/// Get the resolution of the specified clock
extern CodiraTime language_time_getResolution(CodiraClockId clock);

// -- Functions that executors must implement ----------------------------------

/// Enqueue a job on the global executor.
LANGUAGE_CC(language) void language_task_enqueueGlobalImpl(CodiraJob * _Nonnull job);

/// Enqueue a job on the global executor, with a specific delay before it
/// should execute.
LANGUAGE_CC(language) void language_task_enqueueGlobalWithDelayImpl(CodiraJobDelay delay,
                                                           CodiraJob * _Nonnull job);

/// Enqueue a job on the global executor, with a specific deadline before
/// which it must execute.
LANGUAGE_CC(language) void language_task_enqueueGlobalWithDeadlineImpl(long long sec,
                                                              long long nsec,
                                                              long long tsec,
                                                              long long tnsec,
                                                              int clock,
                                                              CodiraJob * _Nonnull job);

/// Enqueue a job on the main executor (which may or may not be the same as
/// the global executor).
LANGUAGE_CC(language) void language_task_enqueueMainExecutorImpl(CodiraJob * _Nonnull job);

/// Assert that the specified executor is the current executor.
LANGUAGE_CC(language) void language_task_checkIsolatedImpl(CodiraExecutorRef executor);

/// Check if the specified executor isolates the current context.
LANGUAGE_CC(language) int8_t
  language_task_isIsolatingCurrentContextImpl(CodiraExecutorRef executor);

/// Get a reference to the main executor.
LANGUAGE_CC(language) CodiraExecutorRef language_task_getMainExecutorImpl(void);

/// Check if the specified executor is the main executor.
LANGUAGE_CC(language) bool language_task_isMainExecutorImpl(CodiraExecutorRef executor);

/// Drain the main executor's queue, processing jobs enqueued on it; this
/// should never return.
LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN LANGUAGE_CC(language) void
  language_task_asyncMainDrainQueueImpl(void);

/// Hand control of the current thread to the global executor until the
/// condition function returns `true`.  Support for this function is optional,
/// but you should assert or provide a dummy implementation if your executor
/// does not support it.
LANGUAGE_CC(language) void
  language_task_donateThreadToGlobalExecutorUntilImpl(bool (* _Nonnull condition)(void * _Nullable),
                                                   void * _Nullable conditionContext);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // LANGUAGE_CONCURRENCY_EXECUTORIMPL_H
