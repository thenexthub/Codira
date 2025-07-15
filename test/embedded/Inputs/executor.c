/* A custom executor to test that we can build an external executor
   and use it with liblanguage_Concurrency.a. */

#include <language/ExecutorImpl.h>

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef DEBUG_EXECUTOR
#define DEBUG_EXECUTOR 1
#endif

static void debug(const char *fmt, ...) {
#if DEBUG_EXECUTOR
  va_list val;

  va_start(val, fmt);
  vprintf(fmt, val);
  va_end(val);
#else
  (void)fmt;
#endif
}

// .. A max-heap to hold jobs in priority order ................................

/* This is an explicit binary heap threaded through the schedulerPrivate[]
   fields in the job. */

static CodiraJob *jobHeap = NULL;

static CodiraJob *job_left(CodiraJob *job) {
  return (CodiraJob *)(job->schedulerPrivate[0]);
}
static void job_setLeft(CodiraJob *job, CodiraJob *left) {
  job->schedulerPrivate[0] = left;
}
static CodiraJob *job_right(CodiraJob *job) {
  return (CodiraJob *)(job->schedulerPrivate[1]);
}
static void job_setRight(CodiraJob *job, CodiraJob *right) {
  job->schedulerPrivate[1] = right;
}

static CodiraJob *job_heap_fixup(CodiraJob *job) {
  CodiraJob *left = job_left(job);
  CodiraJob *right = job_right(job);

  CodiraJobPriority priority = language_job_getPriority(job);
  CodiraJobPriority leftPriority = left ? language_job_getPriority(left) : 0;
  CodiraJobPriority rightPriority = right ? language_job_getPriority(right) : 0;

  CodiraJob *temp;

  if (left && leftPriority >= rightPriority && priority <= leftPriority) {
    job_setLeft(job, job_left(left));
    job_setRight(job, job_right(left));
    job_setRight(left, right);
    job_setLeft(left, job_heap_fixup(job));
    return left;
  }
  if (right && rightPriority > leftPriority && priority <= rightPriority) {
    job_setLeft(job, job_left(right));
    job_setRight(job, job_right(right));
    job_setLeft(right, left);
    job_setRight(right, job_heap_fixup(job));
    return right;
  }

  return job;
}

static void job_heap_push(CodiraJob *job) {
  job_setLeft(job, NULL);
  job_setRight(job, NULL);

  // If the heap is empty, this job is the top
  if (!jobHeap) {
    jobHeap = job;
    return;
  }

  // Otherwise, make the existing top node a child of this one, then fix the
  // heap condition.
  CodiraJobPriority jobPriority = language_job_getPriority(job);

  job_setLeft(job, jobHeap);
  jobHeap = job_heap_fixup(job);
}

static CodiraJob *job_heap_pop(void) {
  if (!jobHeap)
    return NULL;

  CodiraJob *job = jobHeap;

  // The easy case: job has at most one child
  if (!job_left(job)) {
    jobHeap = job_right(job);
    return job;
  }
  if (!job_right(job)) {
    jobHeap = job_left(job);
    return job;
  }

  // Otherwise, find a job with no children
  CodiraJob *parent = NULL;
  CodiraJob *ptr = job;
  while (job_left(ptr) || job_right(ptr)) {
    parent = ptr;
    if (job_right(ptr))
      ptr = job_right(ptr);
    else
      ptr = job_left(ptr);
  }

  // Move it to the head of the queue
  if (job_right(parent) == ptr)
    job_setRight(parent, NULL);
  else
    job_setLeft(parent, NULL);

  job_setLeft(ptr, job_left(job));
  job_setRight(ptr, job_right(job));

  // And fix the heap condition
  jobHeap = job_heap_fixup(ptr);

  return job;
}

// .. A list of delayed jobs ...................................................

// One for each clock
static CodiraJob *delayQueues[2] = { NULL, NULL };

static CodiraJob *job_next(CodiraJob *job) {
  return (CodiraJob *)(job->schedulerPrivate[0]);
}

static CodiraJob **job_ptrToNext(CodiraJob *job) {
  return (CodiraJob **)&job->schedulerPrivate[0];
}

static void job_setNext(CodiraJob *job, CodiraJob *next) {
  job->schedulerPrivate[0] = next;
}

static uint64_t job_deadline(CodiraJob *job) {
  return (uint64_t)(job->schedulerPrivate[1]);
}

static void job_setDeadline(CodiraJob *job, uint64_t deadline) {
  job->schedulerPrivate[1] = (void *)(uintptr_t)deadline;
}

static void job_schedule(CodiraJob *job, CodiraClockId clock, uint64_t deadline) {
  assert(clock >= 1 && clock <= 2 && "clock out of range");

  job_setDeadline(job, deadline);

  CodiraJob **pos = &delayQueues[clock - 1];
  CodiraJob *ptr;
  while ((ptr = *pos) && deadline >= job_deadline(ptr)) {
    pos = job_ptrToNext(ptr);
  }

  job_setNext(job, ptr);
  *pos = job;
}

static uint64_t job_getTime(CodiraClockId clock) {
  CodiraTime now = language_time_now(clock);
  return now.seconds * 1000000000ull + now.nanoseconds;
}

static void job_runTimers(void) {
  for (int clock = 0; clock < 2; ++clock) {
    uint64_t flatNow = job_getTime((CodiraClockId)(clock + 1));

    CodiraJob *job = delayQueues[clock];
    while (job && flatNow >= job_deadline(job)) {
      CodiraJob *next = job_next(job);
      debug("executor: job %d is ready\n", job);
      job_heap_push(job);
      job = next;
    }
    delayQueues[clock] = job;
  }
}

static bool job_haveDelayedJobs(void) {
  for (int clock = 0; clock < 2; ++clock) {
    if (delayQueues[clock])
      return true;
  }
  return false;
}

// Sleep for a specified number of nanoseconds
static void job_sleep(uint64_t ns) {
  if (ns == 0)
    return;

  debug("executor: sleeping for %"PRIu64" ns\n", ns);

  // We don't know how to do this "properly", so spin instead
  uint64_t flatNow = job_getTime(CodiraContinuousClock);
  uint64_t deadline = flatNow + ns;

  while (flatNow < deadline)
    flatNow = job_getTime(CodiraContinuousClock);
}

// Wait until the next timer is about to fire
static void job_wait(void) {
  uint64_t toSleep = ~(uint64_t)0;

  for (int clock = 0; clock < 2; ++clock) {
    uint64_t flatNow = job_getTime((CodiraClockId)(clock + 1));

    if (delayQueues[clock]) {
      uint64_t deadline = job_deadline(delayQueues[clock]);
      uint64_t delay = deadline - flatNow;
      if (delay < toSleep)
        toSleep = delay;
    }
  }

  job_sleep(toSleep);
}

// .. Main loop ................................................................

static CodiraJob *job_getNextJob() {
  while (true) {
    job_runTimers();

    CodiraJob *job = job_heap_pop();
    if (job) {
      return job;
    }

    if (!job_haveDelayedJobs())
      return NULL;

    job_wait();
  }

  return NULL;
}

// .. Interface functions ......................................................

/// Enqueue a job on the global executor.
LANGUAGE_CC(language) void
language_task_enqueueGlobalImpl(CodiraJob *job) {
  debug("executor: job %p enqueued\n", job);

  job_heap_push(job);
}

/// Enqueue a job on the global executor, with a specific delay before it
/// should execute.
LANGUAGE_CC(language) void
language_task_enqueueGlobalWithDelayImpl(CodiraJobDelay delay,
                                      CodiraJob *job) {
  CodiraTime now = language_time_now(CodiraContinuousClock);
  uint64_t deadline = now.seconds * 1000000000ull + now.nanoseconds + delay;

  debug("executor: job %p scheduled with delay %llu ns\n", job, delay);

  job_schedule(job, CodiraContinuousClock, deadline);
}

/// Enqueue a job on the global executor, with a specific deadline before
/// which it must execute.
LANGUAGE_CC(language)
void language_task_enqueueGlobalWithDeadlineImpl(long long sec,
                                              long long nsec,
                                              long long tsec,
                                              long long tnsec,
                                              int clock,
                                              CodiraJob *job) {
  uint64_t deadline = sec * 1000000000ull + nsec;

  debug("executor: job %p scheduled with deadline %"PRIu64" on clock %d\n",
         job, deadline, clock);

  job_schedule(job, clock, deadline);
}

/// Enqueue a job on the main executor (which may or may not be the same as
/// the global executor).
LANGUAGE_CC(language)
void language_task_enqueueMainExecutorImpl(CodiraJob *job) {
  language_task_enqueueGlobalImpl(job);
}

/// Assert that the specified executor is the current executor.
LANGUAGE_CC(language)
void language_task_checkIsolatedImpl(CodiraExecutorRef executor) {
  language_executor_invokeCodiraCheckIsolated(executor);
}

/// Check if the specified executor is the current executor.
LANGUAGE_CC(language)
int8_t language_task_isIsolatingCurrentContextImpl(CodiraExecutorRef executor) {
  return language_executor_invokeCodiraIsIsolatingCurrentContext(executor);
}

/// Get a reference to the main executor.
LANGUAGE_CC(language)
CodiraExecutorRef language_task_getMainExecutorImpl() {
  return language_executor_generic();
}

/// Check if the specified executor is the main executor.
LANGUAGE_CC(language)
bool language_task_isMainExecutorImpl(CodiraExecutorRef executor) {
  return language_executor_isGeneric(executor);
}

/// Drain the main executor's queue, processing jobs enqueued on it; this
/// should never return.
LANGUAGE_RUNTIME_ATTRIBUTE_NORETURN LANGUAGE_CC(language) void
language_task_asyncMainDrainQueueImpl() {
  debug("executor: running\n");
  while (true) {
    CodiraJob *job = job_getNextJob();
    assert(job && "We shouldn't run out of jobs here.");
    debug("executor: job %p running\n", job);
    language_job_run(job, language_executor_generic());
  }
}

/// Hand control of the current thread to the global executor until the
/// condition function returns `true`.  Support for this function is optional,
/// but you should assert or provide a dummy implementation if your executor
/// does not support it.
LANGUAGE_CC(language) void
language_task_donateThreadToGlobalExecutorUntilImpl(bool (*condition)(void *),
                                                 void *conditionContext) {
  debug("executor: running until condition\n");
  while (!condition(conditionContext)) {
    CodiraJob *job = job_getNextJob();
    if (!job)
      return;
    debug("executor: job %p running\n", job);
    language_job_run(job, language_executor_generic());
  }
  debug("executor: condition satisfied or no more jobs\n");
}

