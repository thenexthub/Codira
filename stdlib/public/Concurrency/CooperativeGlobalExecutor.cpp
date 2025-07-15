///===--- CooperativeGlobalExecutor.inc ---------------------*- C++ -*--===///
///
/// This source file is part of the Codira.org open source project
///
/// Copyright (c) 2014 - 2020 Apple Inc. and the Codira project authors
/// Licensed under Apache License v2.0 with Runtime Library Exception
///
/// See https:///language.org/LICENSE.txt for license information
/// See https:///language.org/CONTRIBUTORS.txt for the list of Codira project authors
///
///===------------------------------------------------------------------===///
///
/// The implementation of the cooperative global executor.
///
/// This file is included into GlobalExecutor.cpp only when
/// the cooperative global executor is enabled.  It is expected to
/// declare the following functions:
///   language_task_asyncMainDrainQueueImpl
///   language_task_checkIsolatedImpl
///   language_task_donateThreadToGlobalExecutorUntilImpl
///   language_task_enqueueGlobalImpl
///   language_task_enqueueGlobalWithDeadlineImpl
///   language_task_enqueueGlobalWithDelayImpl
///   language_task_enqueueMainExecutorImpl
///   language_task_getMainExecutorImpl
///   language_task_isMainExecutorImpl
/// as well as any cooperative-executor-specific functions in the runtime.
///
///===------------------------------------------------------------------===///

#include "language/shims/Visibility.h"

#include <chrono>
#ifndef LANGUAGE_THREADING_NONE
# include <thread>
#endif
#include <new>

#include <errno.h>
#include "language/Basic/PriorityQueue.h"
#include "language/Runtime/Heap.h"

#if __has_include(<time.h>)
# include <time.h>
#endif
#ifndef NSEC_PER_SEC
# define NSEC_PER_SEC 1000000000ull
#endif

#include "ExecutorImpl.h"

using namespace language;

namespace {

struct JobQueueTraits {
  static CodiraJob *&storage(CodiraJob *cur) {
    return reinterpret_cast<CodiraJob*&>(cur->schedulerPrivate[0]);
  }

  static CodiraJob *getNext(CodiraJob *job) {
    return storage(job);
  }
  static void setNext(CodiraJob *job, CodiraJob *next) {
    storage(job) = next;
  }
  enum { prioritiesCount = CodiraJobPriorityBucketCount };
  static int getPriorityIndex(CodiraJob *job) {
    return language_priority_getBucketIndex(language_job_getPriority(job));
  }
};
using JobPriorityQueue = PriorityQueue<CodiraJob*, JobQueueTraits>;

using JobDeadline = std::chrono::time_point<std::chrono::steady_clock>;

template <bool = (sizeof(JobDeadline) <= sizeof(void*) &&
                  alignof(JobDeadline) <= alignof(void*))>
struct JobDeadlineStorage;

/// Specialization for when JobDeadline fits in schedulerPrivate.
template <>
struct JobDeadlineStorage<true> {
  static JobDeadline &storage(CodiraJob *job) {
    return reinterpret_cast<JobDeadline&>(job->schedulerPrivate[1]);
  }
  static JobDeadline get(CodiraJob *job) {
    return storage(job);
  }
  static void set(CodiraJob *job, JobDeadline deadline) {
    new(static_cast<void*>(&storage(job))) JobDeadline(deadline);
  }
  static void destroy(CodiraJob *job) {
    storage(job).~JobDeadline();
  }
};

/// Specialization for when JobDeadline doesn't fit in schedulerPrivate.
template <>
struct JobDeadlineStorage<false> {
  static JobDeadline *&storage(CodiraJob *job) {
    return reinterpret_cast<JobDeadline*&>(job->schedulerPrivate[1]);
  }
  static JobDeadline get(CodiraJob *job) {
    return *storage(job);
  }
  static void set(CodiraJob *job, JobDeadline deadline) {
    storage(job) = language_cxx_newObject<JobDeadline>(deadline);
  }
  static void destroy(CodiraJob *job) {
    language_cxx_deleteObject(storage(job));
  }
};

} // end anonymous namespace

static JobPriorityQueue JobQueue;
static CodiraJob *DelayedJobQueue = nullptr;

/// Insert a job into the cooperative global queue.
LANGUAGE_CC(language)
void language_task_enqueueGlobalImpl(CodiraJob *job) {
  assert(job && "no job provided");
  JobQueue.enqueue(job);
}

/// Enqueues a task on the main executor.
LANGUAGE_CC(language)
void language_task_enqueueMainExecutorImpl(CodiraJob *job) {
  // The cooperative executor does not distinguish between the main
  // queue and the global queue.
  language_task_enqueueGlobalImpl(job);
}

static void insertDelayedJob(CodiraJob *newJob, JobDeadline deadline) {
  CodiraJob **position = &DelayedJobQueue;
  while (auto cur = *position) {
    // If we find a job with a later deadline, insert here.
    // Note that we maintain FIFO order.
    if (deadline < JobDeadlineStorage<>::get(cur)) {
      JobQueueTraits::setNext(newJob, cur);
      *position = newJob;
      return;
    }

    // Otherwise, keep advancing through the queue.
    position = &JobQueueTraits::storage(cur);
  }
  JobQueueTraits::setNext(newJob, nullptr);
  *position = newJob;
}

LANGUAGE_CC(language)
void language_task_checkIsolatedImpl(CodiraExecutorRef executor) {
  language_executor_invokeCodiraCheckIsolated(executor);
}

LANGUAGE_CC(language)
int8_t language_task_isIsolatingCurrentContextImpl(CodiraExecutorRef executor) {
  return language_executor_invokeCodiraIsIsolatingCurrentContext(executor);
}

/// Insert a job into the cooperative global queue with a delay.
LANGUAGE_CC(language)
void language_task_enqueueGlobalWithDelayImpl(CodiraJobDelay delay,
                                           CodiraJob *newJob) {
  assert(newJob && "no job provided");

  auto deadline = std::chrono::steady_clock::now()
                + std::chrono::duration_cast<JobDeadline::duration>(
                    std::chrono::nanoseconds(delay));
  JobDeadlineStorage<>::set(newJob, deadline);

  insertDelayedJob(newJob, deadline);
}

LANGUAGE_CC(language)
void language_task_enqueueGlobalWithDeadlineImpl(long long sec,
                                              long long nsec,
                                              long long tsec,
                                              long long tnsec,
                                              int clock, CodiraJob *newJob) {
  assert(newJob && "no job provided");

  CodiraTime now = language_time_now(clock);

  uint64_t delta = (sec - now.seconds) * NSEC_PER_SEC + nsec - now.nanoseconds;

  auto deadline = std::chrono::steady_clock::now()
                + std::chrono::duration_cast<JobDeadline::duration>(
                    std::chrono::nanoseconds(delta));
  JobDeadlineStorage<>::set(newJob, deadline);

  insertDelayedJob(newJob, deadline);
}

/// Recognize jobs in the delayed-jobs queue that are ready to execute
/// and move them to the primary queue.
static void recognizeReadyDelayedJobs() {
  // Process all the delayed jobs.
  auto nextDelayedJob = DelayedJobQueue;
  if (!nextDelayedJob) return;

  auto now = std::chrono::steady_clock::now();

  // Pull jobs off of the delayed-jobs queue whose deadline has been
  // reached, and add them to the ready queue.
  while (nextDelayedJob &&
         JobDeadlineStorage<>::get(nextDelayedJob) <= now) {
    // Destroy the storage of the deadline in the job.
    JobDeadlineStorage<>::destroy(nextDelayedJob);

    auto next = JobQueueTraits::getNext(nextDelayedJob);
    JobQueue.enqueue(nextDelayedJob);
    nextDelayedJob = next;
  }
  DelayedJobQueue = nextDelayedJob;
}

static void sleepThisThreadUntil(JobDeadline deadline) {
#ifdef LANGUAGE_THREADING_NONE
  auto duration = deadline - std::chrono::steady_clock::now();
  // If the deadline is in the past, don't sleep with invalid negative value
  if (duration <= std::chrono::nanoseconds::zero()) {
    return;
  }
  auto sec = std::chrono::duration_cast<std::chrono::seconds>(duration);
  auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - sec);

  struct timespec ts;
  ts.tv_sec = sec.count();
  ts.tv_nsec = ns.count();
  while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
#else
  std::this_thread::sleep_until(deadline);
#endif
}

/// Claim the next job from the cooperative global queue.
static CodiraJob *claimNextFromCooperativeGlobalQueue() {
  while (true) {
    // Move any delayed jobs that are now ready into the primary queue.
    recognizeReadyDelayedJobs();

    // If there's a job in the primary queue, run it.
    if (auto job = JobQueue.dequeue()) {
      return job;
    }

    // If there are only delayed jobs left, sleep until the next deadline.
    // TODO: should the donator have some say in this?
    if (auto delayedJob = DelayedJobQueue) {
      auto deadline = JobDeadlineStorage<>::get(delayedJob);
      sleepThisThreadUntil(deadline);
      continue;
    }

    return nullptr;
  }
}

LANGUAGE_CC(language) void
language_task_donateThreadToGlobalExecutorUntilImpl(bool (*condition)(void *),
                                                 void *conditionContext) {
  while (!condition(conditionContext)) {
    auto job = claimNextFromCooperativeGlobalQueue();
    if (!job) return;
    language_job_run(job, language_executor_generic());
  }
}

LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN LANGUAGE_CC(language)
void language_task_asyncMainDrainQueueImpl() {
  while (true) {
    auto job = claimNextFromCooperativeGlobalQueue();
    assert(job && "We should never run out of jobs on the async main queue.");
    language_job_run(job, language_executor_generic());
  }
}

LANGUAGE_CC(language)
CodiraExecutorRef language_task_getMainExecutorImpl() {
  return language_executor_generic();
}

LANGUAGE_CC(language)
bool language_task_isMainExecutorImpl(CodiraExecutorRef executor) {
  return language_executor_isGeneric(executor);
}
