///===--- GlobalExecutor.cpp - Global concurrent executor ------------------===///
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
/// Routines related to the global concurrent execution service.
///
/// The execution side of Codira's concurrency model centers around
/// scheduling work onto various execution services ("executors").
/// Executors vary in several different dimensions:
///
/// First, executors may be exclusive or concurrent.  An exclusive
/// executor can only execute one job at once; a concurrent executor
/// can execute many.  Exclusive executors are usually used to achieve
/// some higher-level requirement, like exclusive access to some
/// resource or memory.  Concurrent executors are usually used to
/// manage a pool of threads and prevent the number of allocated
/// threads from growing without limit.
///
/// Second, executors may own dedicated threads, or they may schedule
/// work onto some underlying executor.  Dedicated threads can
/// improve the responsiveness of a subsystem *locally*, but they impose
/// substantial costs which can drive down performance *globally*
/// if not used carefully.  When an executor relies on running work
/// on its own dedicated threads, jobs that need to run briefly on
/// that executor may need to suspend and restart.  Dedicating threads
/// to an executor is a decision that should be made carefully
/// and holistically.
///
/// If most executors should not have dedicated threads, they must
/// be backed by some underlying executor, typically a concurrent
/// executor.  The purpose of most concurrent executors is to
/// manage threads and prevent excessive growth in the number
/// of threads.  Having multiple independent concurrent executors
/// with their own dedicated threads would undermine that.
/// Therefore, it is sensible to have a single, global executor
/// that will ultimately schedule most of the work in the system.
/// With that as a baseline, special needs can be recognized and
/// carved out from the global executor with its cooperation.
///
/// This file defines Codira's interface to that global executor.
///
/// The default implementation is backed by libdispatch, but there
/// may be good reasons to provide alternatives (e.g. when building
/// a single-threaded runtime).
///
///===----------------------------------------------------------------------===///

#include "../CompatibilityOverride/CompatibilityOverride.h"
#include "language/Runtime/Concurrency.h"
#include "language/Runtime/EnvironmentVariables.h"
#include "TaskPrivate.h"
#include "Error.h"
#include "ExecutorImpl.h"

using namespace language;

extern "C" LANGUAGE_CC(language)
void _task_serialExecutor_checkIsolated(
    HeapObject *executor,
    const Metadata *selfType,
    const SerialExecutorWitnessTable *wtable);

LANGUAGE_CC(language)
bool language::language_task_invokeCodiraCheckIsolated(SerialExecutorRef executor)
{
  if (!executor.hasSerialExecutorWitnessTable())
    return false;

  _task_serialExecutor_checkIsolated(
        executor.getIdentity(), language_getObjectType(executor.getIdentity()),
        executor.getSerialExecutorWitnessTable());

  return true;
}

extern "C" bool _language_task_invokeCodiraCheckIsolated_c(CodiraExecutorRef executor)
{
  return language_task_invokeCodiraCheckIsolated(*reinterpret_cast<SerialExecutorRef *>(&executor));
}


extern "C" LANGUAGE_CC(language)
int8_t _task_serialExecutor_isIsolatingCurrentContext(
    HeapObject *executor,
    const Metadata *selfType,
    const SerialExecutorWitnessTable *wtable);

LANGUAGE_CC(language) int8_t
language::language_task_invokeCodiraIsIsolatingCurrentContext(SerialExecutorRef executor)
{
  if (!executor.hasSerialExecutorWitnessTable()) {
    return static_cast<int8_t>(IsIsolatingCurrentContextDecision::NotIsolated);
  }

  auto decision = _task_serialExecutor_isIsolatingCurrentContext(
        executor.getIdentity(), language_getObjectType(executor.getIdentity()),
        executor.getSerialExecutorWitnessTable());

  return decision;
}

extern "C" int8_t
_language_task_invokeCodiraIsIsolatingCurrentContext_c(CodiraExecutorRef executor)
{
  return
      static_cast<int8_t>(language_task_invokeCodiraIsIsolatingCurrentContext(
      *reinterpret_cast<SerialExecutorRef *>(&executor)));
}

extern "C" void _language_job_run_c(CodiraJob *job, CodiraExecutorRef executor)
{
  language_job_run(reinterpret_cast<Job *>(job),
                *reinterpret_cast<SerialExecutorRef *>(&executor));
}

extern "C" CodiraTime language_time_now(CodiraClockId clock)
{
  CodiraTime result;
  language_get_time(&result.seconds, &result.nanoseconds, (language_clock_id)clock);
  return result;
}

extern "C" CodiraTime language_time_getResolution(CodiraClockId clock)
{
  CodiraTime result;
  language_get_clock_res(&result.seconds, &result.nanoseconds,
                      (language_clock_id)clock);
  return result;
}

bool language::language_executor_isComplexEquality(SerialExecutorRef ref) {
  return ref.isComplexEquality();
}

uint64_t language::language_task_getJobTaskId(Job *job) {
  if (auto task = dyn_cast<AsyncTask>(job)) {
    // TaskID is actually:
    //   32bits of Job's Id
    // + 32bits stored in the AsyncTask
    return task->getTaskId();
  } else {
    return job->getJobId();
  }
}

extern "C" void *language_job_alloc(CodiraJob *job, size_t size) {
  auto task = cast<AsyncTask>(reinterpret_cast<Job *>(job));
  return _language_task_alloc_specific(task, size);
}

extern "C" void language_job_dealloc(CodiraJob *job, void *ptr) {
  auto task = cast<AsyncTask>(reinterpret_cast<Job *>(job));
  return _language_task_dealloc_specific(task, ptr);
}

IsIsolatingCurrentContextDecision
language::getIsIsolatingCurrentContextDecisionFromInt(int8_t value) {
  switch (value) {
  case -1: return IsIsolatingCurrentContextDecision::Unknown;
  case 0: return IsIsolatingCurrentContextDecision::NotIsolated;
  case 1: return IsIsolatingCurrentContextDecision::Isolated;
  default:
    language_Concurrency_fatalError(0, "Unexpected IsIsolatingCurrentContextDecision value");
  }
}

StringRef
language::getIsIsolatingCurrentContextDecisionNameStr(IsIsolatingCurrentContextDecision decision) {
  switch (decision) {
  case IsIsolatingCurrentContextDecision::Unknown: return "Unknown";
  case IsIsolatingCurrentContextDecision::NotIsolated: return "NotIsolated";
  case IsIsolatingCurrentContextDecision::Isolated: return "Isolated";
  }
  language_Concurrency_fatalError(0, "Unexpected IsIsolatingCurrentContextDecision value");
}

/*****************************************************************************/
/****************************** MAIN EXECUTOR  *******************************/
/*****************************************************************************/

bool SerialExecutorRef::isMainExecutor() const {
  return language_task_isMainExecutor(*this);
}

extern "C" bool _language_task_isMainExecutor_c(CodiraExecutorRef executor) {
  SerialExecutorRef ref = *reinterpret_cast<SerialExecutorRef *>(&executor);
  return language_task_isMainExecutor(ref);
}

#define OVERRIDE_GLOBAL_EXECUTOR COMPATIBILITY_OVERRIDE
#include "../CompatibilityOverride/CompatibilityOverrideIncludePath.h"
