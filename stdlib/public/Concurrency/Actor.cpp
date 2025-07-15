///===--- Actor.cpp - Standard actor implementation ------------------------===///
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
/// The default actor implementation for Codira actors, plus related
/// routines such as generic executor enqueuing and switching.
///
///===----------------------------------------------------------------------===///

#include "language/Runtime/Concurrency.h"
#include <atomic>
#include <new>
#if __has_feature(ptrauth_calls)
#include <ptrauth.h>
#endif

#include "../CompatibilityOverride/CompatibilityOverride.h"
#include "language/ABI/Actor.h"
#include "language/ABI/Task.h"
#include "ExecutorBridge.h"
#include "TaskPrivate.h"
#include "language/Basic/HeaderFooterLayout.h"
#include "language/Basic/PriorityQueue.h"
#include "language/Concurrency/Actor.h"
#include "language/Runtime/AccessibleFunction.h"
#include "language/Runtime/Atomic.h"
#include "language/Runtime/Bincompat.h"
#include "language/Runtime/Casting.h"
#include "language/Runtime/DispatchShims.h"
#include "language/Runtime/EnvironmentVariables.h"
#include "language/Runtime/Heap.h"
#include "language/Threading/Mutex.h"
#include "language/Threading/Once.h"
#include "language/Threading/Thread.h"
#include "language/Threading/ThreadLocalStorage.h"
#ifdef LANGUAGE_CONCURRENCY_BACK_DEPLOYMENT
// All platforms where we care about back deployment have a known
// configurations.
#define HAVE_PTHREAD_H 1
#define LANGUAGE_OBJC_INTEROP 1
#endif
#include "toolchain/ADT/PointerIntPair.h"

#include "TaskPrivate.h"
#include "VoucherSupport.h"

#if LANGUAGE_CONCURRENCY_ENABLE_DISPATCH
#include <dispatch/dispatch.h>
#endif

#if LANGUAGE_CONCURRENCY_TASK_TO_THREAD_MODEL
#define LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS 1
#else
#define LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS 0
#endif

#if LANGUAGE_STDLIB_HAS_ASL
#include <asl.h>
#elif defined(__ANDROID__)
#include <android/log.h>
#endif

#if defined(__ELF__)
#include <unwind.h>
#endif

#if defined(__ELF__)
#include <sys/syscall.h>
#endif

#if defined(_WIN32)
#include <io.h>
#endif

#if LANGUAGE_OBJC_INTEROP
extern "C" void *objc_autoreleasePoolPush();
extern "C" void objc_autoreleasePoolPop(void *);
#endif

using namespace language;

/// Should we yield the thread?
static bool shouldYieldThread() {
  // return dispatch_language_job_should_yield();
  return false;
}

/*****************************************************************************/
/******************************* TASK TRACKING ******************************/
/*****************************************************************************/

namespace {

/// An extremely silly class which exists to make pointer
/// default-initialization constexpr.
template <class T> struct Pointer {
  T *Value;
  constexpr Pointer() : Value(nullptr) {}
  constexpr Pointer(T *value) : Value(value) {}
  operator T *() const { return Value; }
  T *operator->() const { return Value; }
};

/// A class which encapsulates the information we track about
/// the current thread and active executor.
class ExecutorTrackingInfo {
  /// A thread-local variable pointing to the active tracking
  /// information about the current thread, if any.
  ///
  /// TODO: this is obviously runtime-internal and therefore not
  /// reasonable to make ABI. We might want to also provide a way
  /// for generated code to efficiently query the identity of the
  /// current executor, in order to do a cheap comparison to avoid
  /// doing all the work to suspend the task when we're already on
  /// the right executor. It would make sense for that to be a
  /// separate thread-local variable (or whatever is most efficient
  /// on the target platform).
  static LANGUAGE_THREAD_LOCAL_TYPE(Pointer<ExecutorTrackingInfo>,
                                 tls_key::concurrency_executor_tracking_info)
      ActiveInfoInThread;

  /// The active executor.
  SerialExecutorRef ActiveExecutor = SerialExecutorRef::generic();

  /// The current task executor, if present, otherwise `undefined`.
  /// The task executor should be used to execute code when the active executor
  /// is `generic`.
  TaskExecutorRef TaskExecutor = TaskExecutorRef::undefined();

  /// Whether this context allows switching.  Some contexts do not;
  /// for example, we do not allow switching from language_job_run
  /// unless the passed-in executor is generic.
  bool AllowsSwitching = true;

  VoucherManager voucherManager;

  /// The tracking info that was active when this one was entered.
  ExecutorTrackingInfo *SavedInfo;

public:
  ExecutorTrackingInfo() = default;

  ExecutorTrackingInfo(const ExecutorTrackingInfo &) = delete;
  ExecutorTrackingInfo &operator=(const ExecutorTrackingInfo &) = delete;

  /// Unconditionally initialize a fresh tracking state on the
  /// current state, shadowing any previous tracking state.
  /// leave() must be called before the object goes out of scope.
  void enterAndShadow(SerialExecutorRef currentExecutor,
                      TaskExecutorRef taskExecutor) {
    ActiveExecutor = currentExecutor;
    TaskExecutor = taskExecutor;
    SavedInfo = ActiveInfoInThread.get();
    ActiveInfoInThread.set(this);
  }

  void swapToJob(Job *job) { voucherManager.swapToJob(job); }

  void restoreVoucher(AsyncTask *task) { voucherManager.restoreVoucher(task); }

  SerialExecutorRef getActiveExecutor() const { return ActiveExecutor; }
  void setActiveExecutor(SerialExecutorRef newExecutor) {
    ActiveExecutor = newExecutor;
  }

  TaskExecutorRef getTaskExecutor() const { return TaskExecutor; }

  void setTaskExecutor(TaskExecutorRef newExecutor) {
    TaskExecutor = newExecutor;
  }

  bool allowsSwitching() const {
    return AllowsSwitching;
  }

  /// Disallow switching in this tracking context.  This should only
  /// be set on a new tracking info, before any jobs are run in it.
  void disallowSwitching() {
    AllowsSwitching = false;
  }

  static ExecutorTrackingInfo *current() {
    return ActiveInfoInThread.get();
  }

  void leave() {
    voucherManager.leave();
    ActiveInfoInThread.set(SavedInfo);
  }
};

class ActiveTask {
  /// A thread-local variable pointing to the active tracking
  /// information about the current thread, if any.
  static LANGUAGE_THREAD_LOCAL_TYPE(Pointer<AsyncTask>,
                                 tls_key::concurrency_task) Value;

public:
  static void set(AsyncTask *task) { Value.set(task); }
  static AsyncTask *get() { return Value.get(); }
  static AsyncTask *swap(AsyncTask *newTask) {
    return Value.swap(newTask);
  }
};

/// Define the thread-locals.
LANGUAGE_THREAD_LOCAL_TYPE(Pointer<AsyncTask>, tls_key::concurrency_task)
ActiveTask::Value;

LANGUAGE_THREAD_LOCAL_TYPE(Pointer<ExecutorTrackingInfo>,
                        tls_key::concurrency_executor_tracking_info)
ExecutorTrackingInfo::ActiveInfoInThread;

} // end anonymous namespace

void language::runJobInEstablishedExecutorContext(Job *job) {
  _language_tsan_acquire(job);
  LANGUAGE_TASK_DEBUG_LOG("Run job in established context %p", job);

#if LANGUAGE_OBJC_INTEROP
  auto pool = objc_autoreleasePoolPush();
#endif

  if (auto task = dyn_cast<AsyncTask>(job)) {
    // Update the active task in the current thread.
    auto oldTask = ActiveTask::swap(task);

    // Update the task status to say that it's running on the
    // current thread.  If the task suspends somewhere, it should
    // update the task status appropriately; we don't need to update
    // it afterwards.
    task->flagAsRunning();

    auto traceHandle = concurrency::trace::job_run_begin(job);
    task->runInFullyEstablishedContext();
    concurrency::trace::job_run_end(traceHandle);

    assert(ActiveTask::get() == nullptr &&
           "active task wasn't cleared before suspending?");
    if (oldTask) ActiveTask::set(oldTask);
  } else {
    // There's no extra bookkeeping to do for simple jobs besides swapping in
    // the voucher.
    ExecutorTrackingInfo::current()->swapToJob(job);
    job->runSimpleInFullyEstablishedContext();
  }

#if LANGUAGE_OBJC_INTEROP
  objc_autoreleasePoolPop(pool);
#endif
}

void language::adoptTaskVoucher(AsyncTask *task) {
  ExecutorTrackingInfo::current()->swapToJob(task);
}

void language::restoreTaskVoucher(AsyncTask *task) {
  ExecutorTrackingInfo::current()->restoreVoucher(task);
}

LANGUAGE_CC(language)
AsyncTask *language::language_task_getCurrent() {
  return ActiveTask::get();
}

AsyncTask *language::_language_task_clearCurrent() {
  return ActiveTask::swap(nullptr);
}

AsyncTask *language::_language_task_setCurrent(AsyncTask *new_task) {
  return ActiveTask::swap(new_task);
}

LANGUAGE_CC(language)
static SerialExecutorRef language_task_getCurrentExecutorImpl() {
  auto currentTracking = ExecutorTrackingInfo::current();
  auto result = (currentTracking ? currentTracking->getActiveExecutor()
                                 : SerialExecutorRef::generic());
  LANGUAGE_TASK_DEBUG_LOG("getting current executor %p", result.getIdentity());
  return result;
}

/// Determine whether we are currently executing on the main thread
/// independently of whether we know that we are on the main actor.
static bool isExecutingOnMainThread() {
#if LANGUAGE_STDLIB_SINGLE_THREADED_CONCURRENCY
  return true;
#else
  return Thread::onMainThread();
#endif
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"

extern "C" LANGUAGE_CC(language)
SerialExecutorRef _language_getActiveExecutor() {
  auto currentTracking = ExecutorTrackingInfo::current();
  if (currentTracking) {
    SerialExecutorRef executor = currentTracking->getActiveExecutor();
    // This might be an actor, in which case return nil ("generic")
    if (executor.isDefaultActor())
      return SerialExecutorRef::generic();
    return executor;
  }

  // If there's no tracking and we're on the main thread, then the main
  // executor is notionally active.
  if (isExecutingOnMainThread())
    return language_getMainExecutor();

  return SerialExecutorRef::generic();
}

extern "C" LANGUAGE_CC(language)
TaskExecutorRef _language_getCurrentTaskExecutor() {
  auto currentTracking = ExecutorTrackingInfo::current();
  if (currentTracking)
    return currentTracking->getTaskExecutor();
  return TaskExecutorRef::undefined();
}

extern "C" LANGUAGE_CC(language)
TaskExecutorRef _language_getPreferredTaskExecutor() {
  AsyncTask *task = language_task_getCurrent();
  if (!task)
    return TaskExecutorRef::undefined();
  return task->getPreferredTaskExecutor();
}

#pragma clang diagnostic pop

JobPriority language::language_task_getCurrentThreadPriority() {
#if LANGUAGE_STDLIB_SINGLE_THREADED_CONCURRENCY
  return JobPriority::UserInitiated;
#elif LANGUAGE_CONCURRENCY_TASK_TO_THREAD_MODEL
  return JobPriority::Unspecified;
#elif defined(__APPLE__) && LANGUAGE_CONCURRENCY_ENABLE_DISPATCH
  return static_cast<JobPriority>(qos_class_self());
#else
  if (isExecutingOnMainThread())
    return JobPriority::UserInitiated;

  return JobPriority::Unspecified;
#endif
}

const char *language_task_getTaskName(AsyncTask *task) {
  if (!task) {
    return nullptr;
  }
  return task->getTaskName();
}

const char *language::language_task_getCurrentTaskName() {
  auto task = language_task_getCurrent();
  return language_task_getTaskName(task);
}

// Implemented in Codira to avoid some annoying hard-coding about
// SerialExecutor's protocol witness table.  We could inline this
// with effort, though.
extern "C" LANGUAGE_CC(language)
bool _task_serialExecutor_isSameExclusiveExecutionContext(
    HeapObject *currentExecutor, HeapObject *executor,
    const Metadata *selfType,
    const SerialExecutorWitnessTable *wtable);

// We currently still support "legacy mode" in which isCurrentExecutor is NOT
// allowed to crash, because it is used to power "log warnings" data race
// detector. This mode is going away in Codira 6, but until then we allow this.
// This override exists primarily to be able to test both code-paths.
enum IsCurrentExecutorCheckMode : unsigned {
  /// The default mode when an app was compiled against "new" enough SDK.
  /// It allows crashing in isCurrentExecutor, and calls into `checkIsolated`.
  Codira6_UseCheckIsolated_AllowCrash,
  /// Legacy mode; Primarily to support old applications which used data race
  /// detector with "warning" mode, which is no longer supported. When such app
  /// is re-compiled against a new SDK, it will see crashes in what was
  /// previously warnings; however, until until recompiled, warnings will be
  /// used, and `checkIsolated` cannot be invoked.
  Legacy_NoCheckIsolated_NonCrashing,
};

namespace {
using CodiraTaskIsCurrentExecutorOptions =
    OptionSet<language_task_is_current_executor_flag>;
}

static void _language_task_debug_dumpIsCurrentExecutorFlags(
    const char *hint,
    language_task_is_current_executor_flag flags) {
  if (flags == language_task_is_current_executor_flag::None) {
    LANGUAGE_TASK_DEBUG_LOG("%s language_task_is_current_executor_flag::%s",
                         hint, "None");
    return;
  }

  auto options = CodiraTaskIsCurrentExecutorOptions(flags);
  if (options.contains(language_task_is_current_executor_flag::Assert))
    LANGUAGE_TASK_DEBUG_LOG("%s language_task_is_current_executor_flag::%s",
                         hint, "Assert");
}

// Shimming call to Codira runtime because Codira Embedded does not have
// these symbols defined.
language_task_is_current_executor_flag
__language_bincompat_useLegacyNonCrashingExecutorChecks() {
  language_task_is_current_executor_flag options = language_task_is_current_executor_flag::None;
#if !LANGUAGE_CONCURRENCY_EMBEDDED
  if (!language::runtime::bincompat::
      language_bincompat_useLegacyNonCrashingExecutorChecks()) {
    options = language_task_is_current_executor_flag(
        options | language_task_is_current_executor_flag::Assert);
  }
#endif
  _language_task_debug_dumpIsCurrentExecutorFlags("runtime linking determined default mode", options);
  return options;
}

// Shimming call to Codira runtime because Codira Embedded does not have
// these symbols defined.
const char *__language_runtime_env_useLegacyNonCrashingExecutorChecks() {
  // Potentially, override the platform detected mode, primarily used in tests.
#if LANGUAGE_STDLIB_HAS_ENVIRON && !LANGUAGE_CONCURRENCY_EMBEDDED
  return language::runtime::environment::
      concurrencyIsCurrentExecutorLegacyModeOverride();
#else
  return nullptr;
#endif
}

// Determine the default effective executor checking mode, and apply environment
// variable overrides of the executor checking mode.

// Done this way because of the interaction with the initial value of
// 'unexpectedExecutorLogLevel'.
language_task_is_current_executor_flag language_bincompat_selectDefaultIsCurrentExecutorCheckingMode() {
  // Default options as determined by linked runtime,
  // i.e. very old runtimes were not allowed to crash but then we introduced 'checkIsolated'
  // which was allowed to crash;
  language_task_is_current_executor_flag options =
      __language_bincompat_useLegacyNonCrashingExecutorChecks();

  // Potentially, override the platform detected mode, primarily used in tests.
  if (const char *modeStr =
          __language_runtime_env_useLegacyNonCrashingExecutorChecks()) {

    if (strlen(modeStr) == 0) {
      _language_task_debug_dumpIsCurrentExecutorFlags("mode override is empty", options);
      return options;
    }

    if (strcmp(modeStr, "nocrash") == 0 ||
        strcmp(modeStr, "legacy") == 0) {
      // Since we're in nocrash/legacy mode:
      // Remove the assert option which is what would cause the "crash" mode
      options = language_task_is_current_executor_flag(
        options & ~language_task_is_current_executor_flag::Assert);
    } else if (strcmp(modeStr, "crash") == 0 ||
               strcmp(modeStr, "language6") == 0) {
      options = language_task_is_current_executor_flag(
        options | language_task_is_current_executor_flag::Assert);
    } // else, just use the platform detected mode
  } // no override, use the default mode

  return options;
}

// Implemented in Codira to avoid some annoying hard-coding about
// TaskExecutor's protocol witness table.  We could inline this
// with effort, though.
extern "C" LANGUAGE_CC(language) void _language_task_enqueueOnTaskExecutor(
    Job *job, HeapObject *executor, const Metadata *selfType,
    const TaskExecutorWitnessTable *wtable);

// Implemented in Codira to avoid some annoying hard-coding about
// SerialExecutor's protocol witness table.  We could inline this
// with effort, though.
extern "C" LANGUAGE_CC(language) void _language_task_enqueueOnExecutor(
    Job *job, HeapObject *executor, const Metadata *executorType,
    const SerialExecutorWitnessTable *wtable);

LANGUAGE_CC(language)
static bool language_task_isCurrentExecutorWithFlagsImpl(
    SerialExecutorRef expectedExecutor,
    language_task_is_current_executor_flag flags) {
  auto current = ExecutorTrackingInfo::current();

  auto options = CodiraTaskIsCurrentExecutorOptions(flags);
  _language_task_debug_dumpIsCurrentExecutorFlags(__FUNCTION__, flags);

  if (!current) {
    // We have no current executor, i.e. we are running "outside" of Codira
    // Concurrency. We could still be running on a thread/queue owned by
    // the expected executor however, so we need to try a bit harder before
    // we fail.

    // Special handling the main executor by detecting the main thread.
    if (expectedExecutor.isMainExecutor() && isExecutingOnMainThread()) {
      LANGUAGE_TASK_DEBUG_LOG("executor checking: expected is main executor & current thread is main thread => pass", nullptr);
      return true;
    }

    // We cannot use 'complexEquality' as it requires two executor instances,
    // and we do not have a 'current' executor here.

    // Invoke the 'isIsolatingCurrentContext', if "undecided" (i.e. nil), we need to make further calls
    LANGUAGE_TASK_DEBUG_LOG("executor checking, invoke (%p).isIsolatingCurrentContext",
                         expectedExecutor.getIdentity());
    // The executor has the most recent 'isIsolatingCurrentContext' API
    // so available so we prefer calling that to 'checkIsolated'.
    auto isIsolatingCurrentContextDecision =
        getIsIsolatingCurrentContextDecisionFromInt(
            language_task_isIsolatingCurrentContext(expectedExecutor));

    LANGUAGE_TASK_DEBUG_LOG("executor checking mode option: UseIsIsolatingCurrentContext; invoke (%p).isIsolatingCurrentContext => %s",
                   expectedExecutor.getIdentity(), getIsIsolatingCurrentContextDecisionNameStr(isIsolatingCurrentContextDecision));
    switch (isIsolatingCurrentContextDecision) {
      case IsIsolatingCurrentContextDecision::Isolated:
        // We know for sure that this serial executor is isolating this context, return the decision.
        return true;
      case IsIsolatingCurrentContextDecision::NotIsolated:
        // We know for sure that this serial executor is NOT isolating this context, return this decision.
        return false;
      case IsIsolatingCurrentContextDecision::Unknown:
        // We don't know, so we have to continue trying to check using other methods.
        // This most frequently would happen if a serial executor did not implement isIsolatingCurrentContext.
        break;
    }

    // Otherwise, as last resort, let the expected executor check using
    // external means, as it may "know" this thread is managed by it etc.
    if (options.contains(language_task_is_current_executor_flag::Assert)) {
      LANGUAGE_TASK_DEBUG_LOG("executor checking mode option: Assert; invoke (%p).expectedExecutor",
                           expectedExecutor.getIdentity());
      language_task_checkIsolated(expectedExecutor); // will crash if not same context

      // checkIsolated did not crash, so we are on the right executor, after all!
      return true;
    }

    assert(!options.contains(language_task_is_current_executor_flag::Assert));
    return false;
  }

  SerialExecutorRef currentExecutor = current->getActiveExecutor();
  LANGUAGE_TASK_DEBUG_LOG("executor checking: current executor %p %s",
                       currentExecutor.getIdentity(), currentExecutor.getIdentityDebugName());

  // Fast-path: the executor is exactly the same memory address;
  // We assume executors do not come-and-go appearing under the same address,
  // and treat pointer equality of executors as good enough to assume the executor.
  if (currentExecutor == expectedExecutor) {
    LANGUAGE_TASK_DEBUG_LOG("executor checking: current executor %p, equal to expected executor => pass",
                         currentExecutor.getIdentity());
    return true;
  }

  // Fast-path, specialize the common case of comparing two main executors.
  if (currentExecutor.isMainExecutor() && expectedExecutor.isMainExecutor()) {
    LANGUAGE_TASK_DEBUG_LOG("executor checking: current executor %p is main executor, and expected executor (%p) is main executor => pass",
                     currentExecutor.getIdentity(),
                     expectedExecutor.getIdentity());
    return true;
  }

  // Only in legacy mode:
  // We check if the current xor expected executor are the main executor.
  // If so only one of them is, we know that WITHOUT 'checkIsolated' or invoking
  // 'dispatch_assert_queue' we cannot be truly sure the expected/current truly
  // are "on the same queue". There exists no non-crashing API to check this,
  // so we PESSIMISTICALLY return false here.
  //
  // In Codira6 mode:
  // We don't do this naive check, because we'll fall back to
  // `expected.checkIsolated()` which, if it is the main executor, will invoke
  // the crashing 'dispatch_assert_queue(main queue)' which will either crash
  // or confirm we actually are on the main queue; or the custom expected
  // executor has a chance to implement a similar queue check.
  if (!options.contains(language_task_is_current_executor_flag::Assert)) {
    if ((expectedExecutor.isMainExecutor() && !currentExecutor.isMainExecutor())) {
      LANGUAGE_TASK_DEBUG_LOG("executor checking: expected executor %p%s is main executor, and current executor %p%s is NOT => fail",
                 expectedExecutor.getIdentity(), expectedExecutor.getIdentityDebugName(),
                 currentExecutor.getIdentity(), currentExecutor.getIdentityDebugName());
      return false;
    }
    if ((!expectedExecutor.isMainExecutor() && currentExecutor.isMainExecutor())) {
      LANGUAGE_TASK_DEBUG_LOG("executor checking: expected executor %p%s is NOT main executor, and current executor %p%s is => fail",
           expectedExecutor.getIdentity(), expectedExecutor.getIdentityDebugName(),
           currentExecutor.getIdentity(), currentExecutor.getIdentityDebugName());
      return false;
    }
  }

  // Complex equality means that if two executors of the same type have some
  // special logic to check if they are "actually the same".
  //
  // If any of the executors does not have a witness table we can't complex
  // equality compare with it.
  //
  // We may be able to prove we're on the same executor as expected by
  // using 'checkIsolated' later on though.
  if (expectedExecutor.isComplexEquality()) {
    LANGUAGE_TASK_DEBUG_LOG("executor checking: expectedExecutor is complex equality (%p)",
                         expectedExecutor.getIdentity());
    if (currentExecutor.getIdentity() &&
        currentExecutor.hasSerialExecutorWitnessTable() &&
        expectedExecutor.getIdentity() &&
        expectedExecutor.hasSerialExecutorWitnessTable() &&
        language_compareWitnessTables(
            reinterpret_cast<const WitnessTable *>(
                currentExecutor.getSerialExecutorWitnessTable()),
            reinterpret_cast<const WitnessTable *>(
                expectedExecutor.getSerialExecutorWitnessTable()))) {
      LANGUAGE_TASK_DEBUG_LOG("executor checking: can check isComplexEquality (%p, and %p)",
           expectedExecutor.getIdentity(), currentExecutor.getIdentity());

      auto isSameExclusiveExecutionContextResult =
          _task_serialExecutor_isSameExclusiveExecutionContext(
              currentExecutor.getIdentity(), expectedExecutor.getIdentity(),
              language_getObjectType(currentExecutor.getIdentity()),
              expectedExecutor.getSerialExecutorWitnessTable());

      // if the 'isSameExclusiveExecutionContext' returned true we trust
      // it and return; if it was false, we need to give checkIsolated another
      // chance to check.
      if (isSameExclusiveExecutionContextResult) {
        LANGUAGE_TASK_DEBUG_LOG("executor checking: isComplexEquality (%p, and %p) is true => pass",
             expectedExecutor.getIdentity(), currentExecutor.getIdentity());
        return true;
      } // else, we must give 'checkIsolated' a last chance to verify isolation
    }
  }

  // Invoke the 'isIsolatingCurrentContext' function if we can; If so, we can
  // avoid calling the `checkIsolated` because their result will be the same.
  LANGUAGE_TASK_DEBUG_LOG("executor checking: call (%p).isIsolatingCurrentContext",
    expectedExecutor.getIdentity());

  const auto isIsolatingCurrentContextDecision =
    getIsIsolatingCurrentContextDecisionFromInt(language_task_isIsolatingCurrentContext(expectedExecutor));

  LANGUAGE_TASK_DEBUG_LOG("executor checking: can call (%p).isIsolatingCurrentContext => %p",
    expectedExecutor.getIdentity(), getIsIsolatingCurrentContextDecisionNameStr(isIsolatingCurrentContextDecision));
  switch (isIsolatingCurrentContextDecision) {
  case IsIsolatingCurrentContextDecision::Isolated:
    return true;
  case IsIsolatingCurrentContextDecision::NotIsolated:
    return false;
  case IsIsolatingCurrentContextDecision::Unknown:
    break;
  }

  // This provides a last-resort check by giving the expected SerialExecutor the
  // chance to perform a check using some external knowledge if perhaps we are,
  // after all, on this executor, but the Codira concurrency runtime was just not
  // aware.
  //
  // Unless handled in `language_task_checkIsolated` directly, this should call
  // through to the executor's `SerialExecutor.checkIsolated`.
  //
  // This call is expected to CRASH, unless it has some way of proving that
  // we're actually indeed running on this executor.
  //
  // For example, when running outside of Codira concurrency tasks, but trying to
  // `MainActor.assumeIsolated` while executing DIRECTLY on the main dispatch
  // queue, this allows Dispatch to check for this using its own tracking
  // mechanism, and thus allow the assumeIsolated to work correctly, even though
  // the code executing is not even running inside a Task.
  //
  // Note that this only works because the closure in assumeIsolated is
  // synchronous, and will not cause suspensions, as that would require the
  // presence of a Task.
  if (options.contains(language_task_is_current_executor_flag::Assert)) {
    LANGUAGE_TASK_DEBUG_LOG("executor checking: call (%p).checkIsolated",
      expectedExecutor.getIdentity());
    language_task_checkIsolated(expectedExecutor); // will crash if not same context

    // The checkIsolated call did not crash, so we are on the right executor.
    LANGUAGE_TASK_DEBUG_LOG("executor checking: call (%p).checkIsolated passed => pass",
      expectedExecutor.getIdentity());
    return true;
  } else {
    LANGUAGE_TASK_DEBUG_LOG("executor checking: can NOT call (%p).checkIsolated",
                         expectedExecutor.getIdentity());
  }

  // In the end, since 'checkIsolated' could not be used, so we must assume
  // that the executors are not the same context.
  assert(!options.contains(language_task_is_current_executor_flag::Assert));
  return false;
}

// Check override of executor checking mode.
static void language_task_setDefaultExecutorCheckingFlags(void *context) {
  auto *options = static_cast<language_task_is_current_executor_flag *>(context);

  auto modeOverride = language_bincompat_selectDefaultIsCurrentExecutorCheckingMode();
  if (modeOverride != language_task_is_current_executor_flag::None) {
    *options = modeOverride;
  }

  LANGUAGE_TASK_DEBUG_LOG("executor checking: resulting options = %d", *options);
  _language_task_debug_dumpIsCurrentExecutorFlags(__FUNCTION__, *options);
}

LANGUAGE_CC(language)
static bool
language_task_isCurrentExecutorImpl(SerialExecutorRef expectedExecutor) {
  // To support old applications on apple platforms which assumed this call
  // does not crash, try to use a more compatible mode for those apps.
  //
  // We only allow returning `false` directly from this function when operating
  // in 'Legacy_NoCheckIsolated_NonCrashing' mode. If allowing crashes, we
  // instead must call into 'checkIsolated' or crash directly.
  //
  // Whenever we confirm an executor equality, we can return true, in any mode.
  static language_task_is_current_executor_flag isCurrentExecutorFlag;
  static language::once_t isCurrentExecutorFlagToken;
  language::once(isCurrentExecutorFlagToken,
              language_task_setDefaultExecutorCheckingFlags,
              &isCurrentExecutorFlag);

  return language_task_isCurrentExecutorWithFlags(expectedExecutor,
                                               isCurrentExecutorFlag);
}

/// Logging level for unexpected executors:
/// 0 - no logging -- will be IGNORED when Codira6 mode of isCurrentExecutor is used
/// 1 - warn on each instance -- will be IGNORED when Codira6 mode of isCurrentExecutor is used
/// 2 - fatal error
///
/// NOTE: The default behavior on Apple platforms depends on the SDK version
/// an application was linked to. Since Codira 6 the default is to crash,
/// and the logging behavior is no longer available.
static unsigned unexpectedExecutorLogLevel =
    (language_bincompat_selectDefaultIsCurrentExecutorCheckingMode() == language_task_is_current_executor_flag::None)
        ? 1 // legacy apps default to the logging mode, and cannot use `checkIsolated`
        : 2; // new apps will only crash upon concurrency violations, and will call into `checkIsolated`

static void checkUnexpectedExecutorLogLevel(void *context) {
#if LANGUAGE_STDLIB_HAS_ENVIRON
  const char *levelStr = getenv("LANGUAGE_UNEXPECTED_EXECUTOR_LOG_LEVEL");
  if (!levelStr)
    return;

  long level = strtol(levelStr, nullptr, 0);
  if (level >= 0 && level < 3) {
    auto options = CodiraTaskIsCurrentExecutorOptions(
        language_bincompat_selectDefaultIsCurrentExecutorCheckingMode());
    if (options.contains(language_task_is_current_executor_flag::Assert)) {
      // We are in language6/crash mode of isCurrentExecutor which means that
      // rather than returning false, that method will always CRASH when an
      // executor mismatch is discovered.
      //
      // Thus, for clarity, we set this mode also to crashing, as runtime should
      // not expect to be able to get any logging or ignoring done. In practice,
      // the crash would happen before logging or "ignoring", but this should
      // help avoid confusing situations like "I thought it should log" when
      // debugging the runtime.
      LANGUAGE_TASK_DEBUG_LOG("executor checking: crash mode, unexpectedExecutorLogLevel = %d", level);
      unexpectedExecutorLogLevel = 2;
    } else {
      // legacy mode permits doing nothing or just logging, since the method
      // used to perform the check itself is not going to crash:
      LANGUAGE_TASK_DEBUG_LOG("executor checking: legacy mode, unexpectedExecutorLogLevel = %d", level);
      unexpectedExecutorLogLevel = level;
    }
  }
#endif // LANGUAGE_STDLIB_HAS_ENVIRON
}

LANGUAGE_CC(language)
void language::language_task_reportUnexpectedExecutor(
    const unsigned char *file, uintptr_t fileLength, bool fileIsASCII,
    uintptr_t line, SerialExecutorRef executor) {
  LANGUAGE_TASK_DEBUG_LOG("CHECKING language_task_reportUnexpectedExecutor %s", "");
  // Make sure we have an appropriate log level.
  static language::once_t logLevelToken;
  language::once(logLevelToken, checkUnexpectedExecutorLogLevel, nullptr);

  bool isFatalError = false;
  switch (unexpectedExecutorLogLevel) {
  case 0:
    return;

  case 1:
    isFatalError = false;
    break;

  case 2:
    isFatalError = true;
    break;
  }

  const char *functionIsolation;
  const char *whereExpected;
  if (executor.isMainExecutor()) {
    functionIsolation = "@MainActor function";
    whereExpected = "the main thread";
  } else {
    functionIsolation = "actor-isolated function";
    whereExpected = "the same actor";
  }

  char *message;
  language_asprintf(
      &message,
      "%s: data race detected: %s at %.*s:%d was not called on %s\n",
      isFatalError ? "error" : "warning", functionIsolation,
      (int)fileLength, file, (int)line, whereExpected);

  if (_language_shouldReportFatalErrorsToDebugger()) {
    RuntimeErrorDetails details = {
        .version = RuntimeErrorDetails::currentVersion,
        .errorType = "actor-isolation-violation",
        .currentStackDescription = "Actor-isolated function called from another thread",
        .framesToSkip = 1,
        .memoryAddress = nullptr,
        .numExtraThreads = 0,
        .threads = nullptr,
        .numFixIts = 0,
        .fixIts = nullptr,
        .numNotes = 0,
        .notes = nullptr,
    };
    _language_reportToDebugger(
        isFatalError ? RuntimeErrorFlagFatal : RuntimeErrorFlagNone, message,
        &details);
  }

#if defined(_WIN32) && !LANGUAGE_CONCURRENCY_EMBEDDED
#define STDERR_FILENO 2
  _write(STDERR_FILENO, message, strlen(message));
#elif !LANGUAGE_CONCURRENCY_EMBEDDED
  fputs(message, stderr);
  fflush(stderr);
#else
  puts(message);
#endif
#if LANGUAGE_STDLIB_HAS_ASL
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
  asl_log(nullptr, nullptr, ASL_LEVEL_ERR, "%s", message);
#pragma clang diagnostic pop
#elif defined(__ANDROID__)
  __android_log_print(ANDROID_LOG_FATAL, "CodiraRuntime", "%s", message);
#endif

  free(message);

  if (isFatalError)
    abort();
}

/*****************************************************************************/
/*********************** DEFAULT ACTOR IMPLEMENTATION ************************/
/*****************************************************************************/

namespace {

class DefaultActorImpl;

#if !LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
/// A job to process a default actor that's allocated separately from
/// the actor.
class ProcessOutOfLineJob : public Job {
  DefaultActorImpl *Actor;
public:
  ProcessOutOfLineJob(DefaultActorImpl *actor, JobPriority priority)
    : Job({JobKind::DefaultActorSeparate, priority}, &process),
      Actor(actor) {}

  LANGUAGE_CC(languageasync)
  static void process(Job *job);

  static bool classof(const Job *job) {
    return job->Flags.getKind() == JobKind::DefaultActorSeparate;
  }
};

/// Similar to the ActiveTaskStatus, this denotes the ActiveActorState for
/// tracking the atomic state of the actor
///
/// The runtime needs to track the following state about the actor within the
/// same atomic:
///
/// * The current status of the actor - scheduled, running, idle, etc
/// * The current maximum priority of the jobs enqueued in the actor
/// * The identity of the thread currently holding the actor lock
/// * Pointer to list of jobs enqueued in actor
///
/// It is important for all of this information to be in the same atomic so that
/// when the actor's state changes, the information is visible to all threads
/// that may be modifying the actor, allowing the algorithm to eventually
/// converge.
///
/// In order to provide priority escalation support with actors, deeper
/// integration is required with the OS in order to have the intended side
/// effects. On Darwin, Codira Concurrency Tasks runs on dispatch's queues. As
/// such, we need to use an encoding of thread identity vended by libdispatch
/// called dispatch_lock_t, and a futex-style dispatch API in order to escalate
/// the priority of a thread. Henceforth, the dispatch_lock_t tracked in the
/// ActiveActorStatus will be called the DrainLock.
///
/// When a thread starts running on an actor, it's identity is recorded in the
/// ActiveActorStatus. This way, if a higher priority job is enqueued behind the
/// thread executing the actor, we can escalate the thread holding the actor
/// lock, thereby resolving the priority inversion. When a thread hops off of
/// the actor, any priority boosts it may have gotten as a result of contention
/// on the actor, is removed as well.
///
/// In order to keep the entire ActiveActorStatus size to 2 words, the thread
/// identity is only tracked on platforms which can support 128 bit atomic
/// operations. The ActiveActorStatus's layout has thus been changed to have the
/// following layout depending on the system configuration supported:
///
/// 32 bit systems with LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION=1
///
///          Flags               Drain Lock               Unused Job*
/// |----------------------|----------------------|----------------------|-------------------|
///          32 bits                32 bits                32 bits              32 bits
///
/// 64 bit systems with LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION=1
///
///         Flags                Drain Lock             Job*
/// |----------------------|-------------------|----------------------|
///          32 bits                32 bits             64 bits
///
/// 32 bit systems with LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION=0
///
///          Flags                  Job*
/// |----------------------|----------------------|
///          32 bits                32 bits
//
/// 64 bit systems with LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION=0
///
///         Flags                  Unused                 Job*
/// |----------------------|----------------------|---------------------|
///         32 bits                 32 bits               64 bits
///
/// Size requirements:
///     On 64 bit systems or if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION=1,
///     the field is 16 bytes long.
///
///     Otherwise, it is 8 bytes long.
///
/// Alignment requirements:
///     On 64 bit systems or if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION=1,
///     this 16-byte field needs to be 16 byte aligned to be able to do aligned
///     atomic stores field.
///
///     On all other systems, it needs to be 8 byte aligned for the atomic
///     stores.
///
///     As a result of varying alignment needs, we've marked the class as
///     needing 2-word alignment but on arm64_32 with
///     LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION=1, 16 byte alignment is
///     achieved through careful arrangement of the storage for this in the
///     DefaultActorImpl. The additional alignment requirements are
///     enforced by static asserts below.
class alignas(sizeof(void *) * 2) ActiveActorStatus {
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION && LANGUAGE_POINTER_IS_4_BYTES
  uint32_t Flags;
  dispatch_lock_t DrainLock;
  TOOLCHAIN_ATTRIBUTE_UNUSED uint32_t Unused = {};
#elif LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION && LANGUAGE_POINTER_IS_8_BYTES
  uint32_t Flags;
  dispatch_lock_t DrainLock;
#elif !LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION && LANGUAGE_POINTER_IS_4_BYTES
  uint32_t Flags;
#else /* !LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION && LANGUAGE_POINTER_IS_8_BYTES */
  uint32_t Flags;
  TOOLCHAIN_ATTRIBUTE_UNUSED uint32_t Unused = {};
#endif
  Job *FirstJob;

#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
  ActiveActorStatus(uint32_t flags, dispatch_lock_t drainLockValue, Job *job)
      : Flags(flags), DrainLock(drainLockValue), FirstJob(job) {}
#else
  ActiveActorStatus(uint32_t flags, Job *job) : Flags(flags), FirstJob(job) {}
#endif

  uint32_t getActorState() const {
    return Flags & concurrency::ActorFlagConstants::ActorStateMask;
  }
  uint32_t setActorState(uint32_t state) const {
    return (Flags & ~concurrency::ActorFlagConstants::ActorStateMask) | state;
  }

public:
  bool operator==(ActiveActorStatus other) const {
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
    return (Flags == other.Flags) && (DrainLock == other.DrainLock) && (FirstJob == other.FirstJob);
#else
    return (Flags == other.Flags) && (FirstJob == other.FirstJob);
#endif
  }

#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
  constexpr ActiveActorStatus()
      : Flags(), DrainLock(DLOCK_OWNER_NULL), FirstJob(nullptr) {}
#else
  constexpr ActiveActorStatus() : Flags(), FirstJob(nullptr) {}
#endif

  bool isIdle() const {
    bool isIdle = (getActorState() == concurrency::ActorFlagConstants::Idle);
    if (isIdle) {
      assert(!FirstJob);
    }
    return isIdle;
  }
  ActiveActorStatus withIdle() const {
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
    return ActiveActorStatus(
        setActorState(concurrency::ActorFlagConstants::Idle), DLOCK_OWNER_NULL,
        FirstJob);
#else
    return ActiveActorStatus(
        setActorState(concurrency::ActorFlagConstants::Idle), FirstJob);
#endif
  }

  bool isAnyRunning() const {
    uint32_t state = getActorState();
    return (state == concurrency::ActorFlagConstants::Running) ||
           (state ==
            concurrency::ActorFlagConstants::Zombie_ReadyForDeallocation);
  }
  bool isRunning() const {
    return getActorState() == concurrency::ActorFlagConstants::Running;
  }
  ActiveActorStatus withRunning() const {
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
    return ActiveActorStatus(
        setActorState(concurrency::ActorFlagConstants::Running),
        dispatch_lock_value_for_self(), FirstJob);
#else
    return ActiveActorStatus(
        setActorState(concurrency::ActorFlagConstants::Running), FirstJob);
#endif
  }

  bool isScheduled() const {
    return getActorState() == concurrency::ActorFlagConstants::Scheduled;
  }

  ActiveActorStatus withScheduled() const {
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
    return ActiveActorStatus(
        setActorState(concurrency::ActorFlagConstants::Scheduled),
        DLOCK_OWNER_NULL, FirstJob);
#else
    return ActiveActorStatus(
        setActorState(concurrency::ActorFlagConstants::Scheduled), FirstJob);
#endif
  }

  bool isZombie_ReadyForDeallocation() const {
    return getActorState() ==
           concurrency::ActorFlagConstants::Zombie_ReadyForDeallocation;
  }
  ActiveActorStatus withZombie_ReadyForDeallocation() const {
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
    assert(dispatch_lock_owner(DrainLock) != DLOCK_OWNER_NULL);
    return ActiveActorStatus(
        setActorState(
            concurrency::ActorFlagConstants::Zombie_ReadyForDeallocation),
        DrainLock, FirstJob);
#else
    return ActiveActorStatus(
        setActorState(
            concurrency::ActorFlagConstants::Zombie_ReadyForDeallocation),
        FirstJob);
#endif
  }

  JobPriority getMaxPriority() const {
    return (
        JobPriority)((Flags & concurrency::ActorFlagConstants::PriorityMask) >>
                     concurrency::ActorFlagConstants::PriorityShift);
  }
  ActiveActorStatus withNewPriority(JobPriority priority) const {
    uint32_t flags =
        Flags & ~concurrency::ActorFlagConstants::PriorityAndOverrideMask;
    flags |=
        (uint32_t(priority) << concurrency::ActorFlagConstants::PriorityShift);
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
    return ActiveActorStatus(flags, DrainLock, FirstJob);
#else
    return ActiveActorStatus(flags, FirstJob);
#endif
  }
  ActiveActorStatus resetPriority() const {
    return withNewPriority(JobPriority::Unspecified);
  }

  bool isMaxPriorityEscalated() const {
    return Flags & concurrency::ActorFlagConstants::IsPriorityEscalated;
  }
  ActiveActorStatus withEscalatedPriority(JobPriority priority) const {
    JobPriority currentPriority =
        JobPriority((Flags & concurrency::ActorFlagConstants::PriorityMask) >>
                    concurrency::ActorFlagConstants::PriorityShift);
    (void)currentPriority;
    assert(priority > currentPriority);

    uint32_t flags =
        (Flags & ~concurrency::ActorFlagConstants::PriorityMask) |
        (uint32_t(priority) << concurrency::ActorFlagConstants::PriorityShift);
    flags |= concurrency::ActorFlagConstants::IsPriorityEscalated;
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
    return ActiveActorStatus(flags, DrainLock, FirstJob);
#else
    return ActiveActorStatus(flags, FirstJob);
#endif
  }
  ActiveActorStatus withoutEscalatedPriority() const {
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
    return ActiveActorStatus(
        Flags & ~concurrency::ActorFlagConstants::IsPriorityEscalated,
        DrainLock, FirstJob);
#else
    return ActiveActorStatus(
        Flags & ~concurrency::ActorFlagConstants::IsPriorityEscalated,
        FirstJob);
#endif
  }

  Job *getFirstUnprioritizedJob() const { return FirstJob; }
  ActiveActorStatus withFirstUnprioritizedJob(Job *firstJob) const {
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
    return ActiveActorStatus(Flags, DrainLock, firstJob);
#else
    return ActiveActorStatus(Flags, firstJob);
#endif
  }

  uint32_t getOpaqueFlags() const {
    return Flags;
  }

  uint32_t currentDrainer() const {
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
    return dispatch_lock_owner(DrainLock);
#else
    return 0;
#endif
  }

#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
  static size_t drainLockOffset() {
    return offsetof(ActiveActorStatus, DrainLock);
  }
#endif

  void traceStateChanged(HeapObject *actor, bool distributedActorIsRemote) {
    // Convert our state to a consistent raw value. These values currently match
    // the enum values, but this explicit conversion provides room for change.
    uint8_t traceState = 255;
    switch (getActorState()) {
    case concurrency::ActorFlagConstants::Idle:
      traceState = 0;
      break;
    case concurrency::ActorFlagConstants::Scheduled:
      traceState = 1;
      break;
    case concurrency::ActorFlagConstants::Running:
      traceState = 2;
      break;
    case concurrency::ActorFlagConstants::Zombie_ReadyForDeallocation:
      traceState = 3;
      break;
    }
    concurrency::trace::actor_state_changed(
        actor, getFirstUnprioritizedJob(), traceState, distributedActorIsRemote,
        isMaxPriorityEscalated(), static_cast<uint8_t>(getMaxPriority()));
  }
};

#if !LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS

/// Given that a job is enqueued normally on a default actor, get/set
/// the next job in the actor's queue.
static Job *getNextJob(Job *job) {
  return *reinterpret_cast<Job **>(job->SchedulerPrivate);
}
static void setNextJob(Job *job, Job *next) {
  *reinterpret_cast<Job **>(job->SchedulerPrivate) = next;
}

struct JobQueueTraits {
  static Job *getNext(Job *job) { return getNextJob(job); }
  static void setNext(Job *job, Job *next) { setNextJob(job, next); }

  enum { prioritiesCount = PriorityBucketCount };
  static int getPriorityIndex(Job *job) {
    return getPriorityBucketIndex(job->getPriority());
  }
};

#endif

#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION && LANGUAGE_POINTER_IS_4_BYTES
#define ACTIVE_ACTOR_STATUS_SIZE (4 * (sizeof(uintptr_t)))
#else
#define ACTIVE_ACTOR_STATUS_SIZE (2 * (sizeof(uintptr_t)))
#endif
static_assert(sizeof(ActiveActorStatus) == ACTIVE_ACTOR_STATUS_SIZE,
  "ActiveActorStatus is of incorrect size");
#endif /* !LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS */

class DefaultActorImplHeader : public HeapObject {
protected:
  // TODO (rokhinip): Make this a flagset
  bool isDistributedRemoteActor;
#if LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
  // If actors are locks, we don't need to maintain any extra bookkeeping in the
  // ActiveActorStatus since all threads which are contending will block
  // synchronously, no job queue is needed and the lock will handle all priority
  // escalation logic
  Mutex drainLock;
  // Actor can be deinitialized while lock is still being held.
  // We maintain separate reference counter for the lock to make sure that
  // lock is destroyed and memory is freed when both actor is deinitialized
  // and lock is unlocked.
  std::atomic<int> lockReferenceCount;
#else
  // Note: There is some padding that is added here by the compiler in order to
  // enforce alignment. This is space that is available for us to use in
  // the future
  alignas(sizeof(ActiveActorStatus)) char StatusStorage[sizeof(ActiveActorStatus)];
#endif
};

// All the fields accessed under the actor's lock should be moved
// to the end of the default-actor reservation to minimize false sharing.
// The memory following the DefaultActorImpl object are the stored properties of
// the actor, which are all accessed only by the current processing thread.
class DefaultActorImplFooter {
protected:
#if !LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
  using PriorityQueue = language::PriorityQueue<Job *, JobQueueTraits>;

  // When enqueued, jobs are atomically added to a linked list with the head
  // stored inside ActiveActorStatus. This list contains jobs in the LIFO order
  // regardless of their priorities.
  //
  // When the processing thread sees new incoming jobs in
  // ActiveActorStatus, it reverses them and inserts them into
  // prioritizedJobs in the appropriate priority bucket.
  //
  PriorityQueue prioritizedJobs;
#endif
};

// We can't use sizeof(DefaultActor) since the alignment requirement on the
// default actor means that we have some padding added when calculating
// sizeof(DefaultActor). However that padding isn't available for us to use
// in DefaultActorImpl.
enum {
  DefaultActorSize = sizeof(void *) * NumWords_DefaultActor + sizeof(HeapObject)
};

/// The default actor implementation.
///
/// Ownership of the actor is subtle.  Jobs are assumed to keep the actor
/// alive as long as they're executing on it; this allows us to avoid
/// retaining and releasing whenever threads are scheduled to run a job.
/// While jobs are enqueued on the actor, there is a conceptual shared
/// ownership of the currently-enqueued jobs which is passed around
/// between threads and processing jobs and managed using extra retains
/// and releases of the actor.  The basic invariant is as follows:
///
/// - Let R be 1 if there are jobs enqueued on the actor or if a job
///   is currently running on the actor; otherwise let R be 0.
/// - Let N be the number of active processing jobs for the actor. N may be > 1
///   because we may have stealers for actors if we have to escalate the max
///   priority of the actor
/// - N >= R
/// - There are N - R extra retains of the actor.
///
/// We can think of this as there being one "owning" processing job
/// and K "extra" jobs.  If there is a processing job that is actively
/// running the actor, it is always the owning job; otherwise, any of
/// the N jobs may win the race to become the owning job.
///
/// We then have the following ownership rules:
///
/// (1) When we enqueue the first job on an actor, then R becomes 1, and
///     we must create a processing job so that N >= R.  We do not need to
///     retain the actor.
/// (2) When we create an extra job to process an actor (e.g. because of
///     priority overrides), N increases but R remains the same.  We must
///     retain the actor - ie stealer for actor has a reference on the actor
/// (3) When we start running an actor, our job definitively becomes the
///     owning job, but neither N nor R changes.  We do not need to retain
///     the actor.
/// (4) When we go to start running an actor and for whatever reason we
///     don't actually do so, we are eliminating an extra processing job,
///     and so N decreases but R remains the same.  We must release the
///     actor.
/// (5) When we are running an actor and give it up, and there are no
///     remaining jobs on it, then R becomes 0 and N decreases by 1.
///     We do not need to release the actor.
/// (6) When we are running an actor and give it up, and there are jobs
///     remaining on it, then R remains 1 but N is decreasing by 1.
///     We must either release the actor or create a new processing job
///     for it to maintain the balance.
///
/// The current behaviour of actors is such that we only have a single
/// processing job for an actor at a given time. Stealers jobs support does not
/// exist yet. As a result, the subset of rules that currently apply
/// are (1), (3), (5), (6).
class DefaultActorImpl
    : public HeaderFooterLayout<DefaultActorImplHeader, DefaultActorImplFooter,
                                DefaultActorSize> {
public:
  /// Properly construct an actor, except for the heap header.
  void initialize(bool isDistributedRemote = false) {
    this->isDistributedRemoteActor = isDistributedRemote;
#if LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
    new (&this->drainLock) Mutex();
    lockReferenceCount = 1;
#else
   _status().store(ActiveActorStatus(), std::memory_order_relaxed);
   new (&this->prioritizedJobs) PriorityQueue();
#endif
    LANGUAGE_TASK_DEBUG_LOG("Creating default actor %p", this);
    concurrency::trace::actor_create(this);
  }

  /// Properly destruct an actor, except for the heap header.
  void destroy();

  /// Properly respond to the last release of a default actor.  Note
  /// that the actor will have been completely torn down by the time
  /// we reach this point.
  void deallocate();

  /// Try to lock the actor, if it is already running or scheduled, fail
  bool tryLock(bool asDrainer);

  /// Unlock an actor
  bool unlock(bool forceUnlock);

#if LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
  void retainLock();
  void releaseLock();
#endif

#if !LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
  /// Enqueue a job onto the actor.
  void enqueue(Job *job, JobPriority priority);

  /// Enqueue a stealer for the given task since it has been escalated to the
  /// new priority
  void enqueueStealer(Job *job, JobPriority priority);

  /// Dequeues one job from `prioritizedJobs`.
  /// The calling thread must be holding the actor lock while calling this
  Job *drainOne();

  /// Atomically claims incoming jobs from ActiveActorStatus, and calls `handleUnprioritizedJobs()`.
  /// Called with actor lock held on current thread.
  void processIncomingQueue();
#endif

  /// Check if the actor is actually a distributed *remote* actor.
  ///
  /// Note that a distributed *local* actor instance is the same as any other
  /// ordinary default (local) actor, and no special handling is needed for them.
  bool isDistributedRemote();

#if !LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
  language::atomic<ActiveActorStatus> &_status() {
    return reinterpret_cast<language::atomic<ActiveActorStatus> &>(this->StatusStorage);
  }

  const language::atomic<ActiveActorStatus> &_status() const {
    return reinterpret_cast<const language::atomic<ActiveActorStatus> &>(this->StatusStorage);
  }

  // Only for static assert use below, not for actual use otherwise
  static constexpr size_t offsetOfActiveActorStatus() {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
    return offsetof(DefaultActorImpl, StatusStorage);
#pragma clang diagnostic pop
  }
#endif /* !LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS */

private:
#if !LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
  dispatch_lock_t *drainLockAddr();
#endif
  /// Schedule a processing job.
  /// It can be done when actor transitions from Idle to Scheduled or
  /// when actor gets a priority override and we schedule a stealer.
  ///
  /// When the task executor is `undefined` the task will be scheduled on the
  /// default global executor.
  void scheduleActorProcessJob(JobPriority priority, TaskExecutorRef taskExecutor);

  /// Processes claimed incoming jobs into `prioritizedJobs`.
  /// Incoming jobs are of mixed priorities and in LIFO order.
  /// Called with actor lock held on current thread.
  void handleUnprioritizedJobs(Job *head);
#endif /* !LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS */

  void deallocateUnconditional();
};

class NonDefaultDistributedActorImpl : public HeapObject {
  // TODO (rokhinip): Make this a flagset
  bool isDistributedRemoteActor;

public:
  /// Properly construct an actor, except for the heap header.
  void initialize(bool isDistributedRemote = false) {
    this->isDistributedRemoteActor = isDistributedRemote;
    LANGUAGE_TASK_DEBUG_LOG("Creating non-default distributed actor %p", this);
    concurrency::trace::actor_create(this);
  }

  /// Properly destruct an actor, except for the heap header.
  void destroy() {
    // empty
  }

  /// Properly respond to the last release of a default actor.  Note
  /// that the actor will have been completely torn down by the time
  /// we reach this point.
  void deallocate() {
    // empty
  }

  /// Check if the actor is actually a distributed *remote* actor.
  ///
  /// Note that a distributed *local* actor instance is the same as any other
  /// ordinary default (local) actor, and no special handling is needed for them.
  bool isDistributedRemote() {
    return isDistributedRemoteActor;
  }
};

} /// end anonymous namespace

static_assert(size_without_trailing_padding<DefaultActorImpl>::value <=
                      DefaultActorSize &&
                  alignof(DefaultActorImpl) <= alignof(DefaultActor),
              "DefaultActorImpl doesn't fit in DefaultActor");
#if !LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
static_assert(DefaultActorImpl::offsetOfActiveActorStatus() % ACTIVE_ACTOR_STATUS_SIZE == 0,
              "ActiveActorStatus is aligned to the right size");
#endif

static_assert(sizeof(DefaultActor) == sizeof(NonDefaultDistributedActor),
              "NonDefaultDistributedActor size should be the same as DefaultActor");
static_assert(sizeof(NonDefaultDistributedActorImpl) <= ((sizeof(void *) * NumWords_NonDefaultDistributedActor) + sizeof(HeapObject)) &&
              alignof(NonDefaultDistributedActorImpl) <= alignof(NonDefaultDistributedActor),
              "NonDefaultDistributedActorImpl doesn't fit in NonDefaultDistributedActor");

static DefaultActorImpl *asImpl(DefaultActor *actor) {
  return reinterpret_cast<DefaultActorImpl*>(actor);
}

static DefaultActor *asAbstract(DefaultActorImpl *actor) {
  return reinterpret_cast<DefaultActor*>(actor);
}

static NonDefaultDistributedActorImpl *asImpl(NonDefaultDistributedActor *actor) {
  return reinterpret_cast<NonDefaultDistributedActorImpl*>(actor);
}

/*****************************************************************************/
/******************** NEW DEFAULT ACTOR IMPLEMENTATION ***********************/
/*****************************************************************************/

TaskExecutorRef TaskExecutorRef::fromTaskExecutorPreference(Job *job) {
  if (auto task = dyn_cast<AsyncTask>(job)) {
    return task->getPreferredTaskExecutor();
  }
  return TaskExecutorRef::undefined();
}

#if !LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS

static void traceJobQueue(DefaultActorImpl *actor, Job *first) {
  concurrency::trace::actor_note_job_queue(
      actor, first, [](Job *job) { return getNextJob(job); });
}

static LANGUAGE_ATTRIBUTE_ALWAYS_INLINE void traceActorStateTransition(DefaultActorImpl *actor,
    ActiveActorStatus oldState, ActiveActorStatus newState, bool distributedActorIsRemote) {

  LANGUAGE_TASK_DEBUG_LOG("Actor %p transitioned from %#x to %#x (%s)", actor,
                       oldState.getOpaqueFlags(), newState.getOpaqueFlags(),
                       __FUNCTION__);
  newState.traceStateChanged(actor, distributedActorIsRemote);
}

#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
dispatch_lock_t *DefaultActorImpl::drainLockAddr() {
  ActiveActorStatus *actorStatus = (ActiveActorStatus *) &this->StatusStorage;
  return (dispatch_lock_t *) (((char *) actorStatus) + ActiveActorStatus::drainLockOffset());
}
#endif /* LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION */

void DefaultActorImpl::scheduleActorProcessJob(
    JobPriority priority, TaskExecutorRef taskExecutor) {
  Job *job = language_cxx_newObject<ProcessOutOfLineJob>(this, priority);
  LANGUAGE_TASK_DEBUG_LOG(
      "Scheduling processing job %p for actor %p at priority %#zx, with taskExecutor %p", job, this,
      priority, taskExecutor.getIdentity());

  if (taskExecutor.isDefined()) {
#if LANGUAGE_CONCURRENCY_EMBEDDED
    language_unreachable("task executors not supported in embedded Codira");
#else
    auto taskExecutorIdentity = taskExecutor.getIdentity();
    auto taskExecutorType = language_getObjectType(taskExecutorIdentity);
    auto taskExecutorWtable = taskExecutor.getTaskExecutorWitnessTable();

    return _language_task_enqueueOnTaskExecutor(
        job, taskExecutorIdentity, taskExecutorType, taskExecutorWtable);
#endif
  }

  language_task_enqueueGlobal(job);
}

void DefaultActorImpl::enqueue(Job *job, JobPriority priority) {
  // We can do relaxed loads here, we are just using the current head in the
  // atomic state and linking that into the new job we are inserting, we don't
  // need acquires
  LANGUAGE_TASK_DEBUG_LOG("Enqueueing job %p onto actor %p at priority %#zx", job,
                       this, priority);
  concurrency::trace::actor_enqueue(this, job);
  bool distributedActorIsRemote = language_distributed_actor_is_remote(this);

  auto oldState = _status().load(std::memory_order_relaxed);
  CodiraDefensiveRetainRAII thisRetainHelper{this};
  while (true) {
    auto newState = oldState;

    // Link this into the queue in the atomic state
    Job *currentHead = oldState.getFirstUnprioritizedJob();
    setNextJob(job, currentHead);
    newState = newState.withFirstUnprioritizedJob(job);

    if (oldState.isIdle()) {
      // Schedule the actor
      newState = newState.withScheduled();
      newState = newState.withNewPriority(priority);
    } else {
      if (priority > oldState.getMaxPriority()) {
        newState = newState.withEscalatedPriority(priority);
      }
    }

    // Fetch the task executor from the job for later use. This can be somewhat
    // expensive, so only do it if we're likely to need it. The conditions here
    // match the conditions of the if statements below which use `taskExecutor`.
    TaskExecutorRef taskExecutor;
    bool needsScheduling = !oldState.isScheduled() && newState.isScheduled();
    bool needsStealer =
        oldState.getMaxPriority() != newState.getMaxPriority() &&
        newState.isRunning();
    if (needsScheduling || needsStealer)
      taskExecutor = TaskExecutorRef::fromTaskExecutorPreference(job);

    // In some cases (we aren't scheduling the actor and priorities don't
    // match) then we need to access `this` after the enqueue. But the enqueue
    // can cause the job to run and release `this`, so we need to retain `this`
    // in those cases. The conditional here matches the conditions where we can
    // get to the code below that uses `this`.
    bool willSchedule = !oldState.isScheduled() && newState.isScheduled();
    bool priorityMismatch = oldState.getMaxPriority() != newState.getMaxPriority();
    if (!willSchedule && priorityMismatch)
        thisRetainHelper.defensiveRetain();

    // This needs to be a store release so that we also publish the contents of
    // the new Job we are adding to the atomic job queue. Pairs with consume
    // in drainOne.
    if (_status().compare_exchange_weak(oldState, newState,
                   /* success */ std::memory_order_release,
                   /* failure */ std::memory_order_relaxed)) {
      // NOTE: `job` is off limits after this point, as another thread might run
      // and destroy it now that it's enqueued. `this` is only accessible if
      // `retainedThis` is true.
      job = nullptr; // Ensure we can't use it accidentally.

      // NOTE: only the pointer value of `this` is used here, so this one
      // doesn't need a retain.
      traceActorStateTransition(this, oldState, newState, distributedActorIsRemote);

      if (!oldState.isScheduled() && newState.isScheduled()) {
        // We took responsibility to schedule the actor for the first time. See
        // also ownership rule (1)
        return scheduleActorProcessJob(newState.getMaxPriority(), taskExecutor);
      }

#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
      if (oldState.getMaxPriority() != newState.getMaxPriority()) {
        // We still need `this`, assert that we did a defensive retain.
        assert(thisRetainHelper.isRetained());

        if (newState.isRunning()) {
          // Actor is running on a thread, escalate the thread running it
          LANGUAGE_TASK_DEBUG_LOG("[Override] Escalating actor %p which is running on %#x to %#x priority", this, newState.currentDrainer(), priority);
          dispatch_lock_t *lockAddr = this->drainLockAddr();
          language_dispatch_lock_override_start_with_debounce(lockAddr, newState.currentDrainer(),
                         (qos_class_t) priority);
        } else {
          // We are scheduling a stealer for an actor due to priority override.
          // This extra processing job has a reference on the actor. See
          // ownership rule (2). That means that we need to retain `this`, which
          // we'll take from the retain helper.
          thisRetainHelper.takeRetain();
          LANGUAGE_TASK_DEBUG_LOG(
              "[Override] Scheduling a stealer for actor %p at %#x priority",
              this, newState.getMaxPriority());

          scheduleActorProcessJob(newState.getMaxPriority(), taskExecutor);
        }
      }
#endif
      return;
    }
  }
}

// The input job is already escalated to the new priority and has already been
// enqueued into the actor. Push a stealer job for it on the actor.
//
// The caller of this function is escalating the input task and holding its
// TaskStatusRecordLock and escalating this executor via the
// TaskDependencyStatusRecord.
void DefaultActorImpl::enqueueStealer(Job *job, JobPriority priority) {

  LANGUAGE_TASK_DEBUG_LOG("[Override] Escalating an actor %p due to job that is enqueued being escalated", this);

  bool distributedActorIsRemote = language_distributed_actor_is_remote(this);
  auto oldState = _status().load(std::memory_order_relaxed);
  while (true) {
    // Until we figure out how to safely enqueue a stealer and rendevouz with
    // the original job so that we don't double-invoke the job, we shall simply
    // escalate the actor's max priority to match the new one.
    //
    // Ideally, we'd also re-sort the job queue so that the escalated job gets
    // to the front of the queue but since the actor's max QoS is a saturating
    // function, this still handles the priority inversion correctly but with
    // priority overhang instead.

    if (oldState.isIdle()) {
      // We are observing a race. Possible scenarios:
      //
      //  1. Escalator is racing with the drain of the actor/task. The task has
      //  just been popped off the actor and is about to run. The thread running
      //  the task will readjust its own priority once it runs since it should
      //  see the escalation in the ActiveTaskStatus and we don't need to
      //  escalate the actor as it will be spurious.
      //
      //  2. Escalator is racing with the enqueue of the task. The task marks
      //  the place it will enqueue in the dependency record before it enqueues
      //  itself. Escalator raced in between these two operations and escalated the
      //  task. Pushing a stealer job for the task onto the actor should fix it.
      return;
    }
    auto newState = oldState;

    if (priority > oldState.getMaxPriority()) {
      newState = newState.withEscalatedPriority(priority);
    }

    if (oldState == newState)
      return;

    // Fetch the task executor from the job for later use. This can be somewhat
    // expensive, so only do it if we're likely to need it. The conditions here
    // match the conditions of the if statements below which use `taskExecutor`.
    TaskExecutorRef taskExecutor;
    if (!newState.isRunning() && newState.isScheduled())
      taskExecutor = TaskExecutorRef::fromTaskExecutorPreference(job);

    if (_status().compare_exchange_weak(oldState, newState,
                   /* success */ std::memory_order_relaxed,
                   /* failure */ std::memory_order_relaxed)) {
      // NOTE: `job` is off limits after this point, as another thread might run
      // and destroy it now that it's enqueued.

      traceActorStateTransition(this, oldState, newState, distributedActorIsRemote);
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
      if (newState.isRunning()) {
        // Actor is running on a thread, escalate the thread running it
        LANGUAGE_TASK_DEBUG_LOG("[Override] Escalating actor %p which is running on %#x to %#x priority", this, newState.currentDrainer(), priority);
        dispatch_lock_t *lockAddr = this->drainLockAddr();
        language_dispatch_lock_override_start_with_debounce(lockAddr, newState.currentDrainer(),
                       (qos_class_t) priority);

      } else if (newState.isScheduled()) {
        // We are scheduling a stealer for an actor due to priority override.
        // This extra processing job has a reference on the actor. See
        // ownership rule (2).
        LANGUAGE_TASK_DEBUG_LOG(
            "[Override] Scheduling a stealer for actor %p at %#x priority",
            this, newState.getMaxPriority());
        language_retain(this);
        scheduleActorProcessJob(newState.getMaxPriority(), taskExecutor);
      }
#endif
    }
  }

}

void DefaultActorImpl::processIncomingQueue() {
  // Pairs with the store release in DefaultActorImpl::enqueue
  bool distributedActorIsRemote = language_distributed_actor_is_remote(this);
  auto oldState = _status().load(LANGUAGE_MEMORY_ORDER_CONSUME);
  _language_tsan_consume(this);

  // We must ensure that any jobs not seen by collectJobs() don't have any
  // dangling references to the jobs that have been collected. For that we must
  // atomically set head pointer to NULL. If it fails because more jobs have
  // been added in the meantime, we have to re-read the head pointer.
  while (true) {
    // If there aren't any new jobs in the incoming queue, we can return
    // immediately without updating the status.
    if (!oldState.getFirstUnprioritizedJob()) {
      return;
    }
    assert(oldState.isAnyRunning());

    auto newState = oldState;
    newState = newState.withFirstUnprioritizedJob(nullptr);

    if (_status().compare_exchange_weak(
            oldState, newState,
            /* success */ std::memory_order_relaxed,
            /* failure */ std::memory_order_relaxed)) {
      LANGUAGE_TASK_DEBUG_LOG("Collected some jobs from actor %p", this);
      traceActorStateTransition(this, oldState, newState,
                                distributedActorIsRemote);
      break;
    }
  }

  handleUnprioritizedJobs(oldState.getFirstUnprioritizedJob());
}

// Called with actor lock held on current thread
void DefaultActorImpl::handleUnprioritizedJobs(Job *head) {
  // Reverse jobs from LIFO to FIFO order
  Job *reversed = nullptr;
  while (head) {
    auto next = getNextJob(head);
    setNextJob(head, reversed);
    reversed = head;
    head = next;
  }
  prioritizedJobs.enqueueContentsOf(reversed);
}

// Called with actor lock held on current thread
Job *DefaultActorImpl::drainOne() {
  LANGUAGE_TASK_DEBUG_LOG("Draining one job from default actor %p", this);

  traceJobQueue(this, prioritizedJobs.peek());
  auto firstJob = prioritizedJobs.dequeue();
  if (!firstJob) {
    LANGUAGE_TASK_DEBUG_LOG("No jobs to drain on actor %p", this);
  } else {
    LANGUAGE_TASK_DEBUG_LOG("Drained first job %p from actor %p", firstJob, this);
    concurrency::trace::actor_dequeue(this, firstJob);
  }
  return firstJob;
}

// Called from processing jobs which are created to drain an actor. We need to
// reason about this function together with language_task_switch because threads
// can switch from "following actors" to "following tasks" and vice versa.
//
// The way this function works is as following:
//
// 1. We grab the actor lock for the actor we were scheduled to drain.
// 2. Drain the first task out of the actor and run it. Henceforth, the thread
// starts executing the task and following it around. It is no longer a
// processing job for the actor. Note that it will hold the actor lock until it
// needs to hop off the actor but conceptually, it is now a task executor as well.
// 3. When the thread needs to hop off the actor, it is done deep in the
// callstack of this function with an indirection through user code:
// defaultActorDrain -> runJobInEstablishedExecutorContext ->  user
// code -> a call to language_task_switch to hop out of actor
// 4. This call to language_task_switch() will attempt to hop off the existing
// actor and jump to the new one. There are 2 possible outcomes at that point:
//   (a) We were able to hop to the new actor and so thread continues executing
//   task. We then schedule a job for the old actor to continue if it has
//   pending work
//   (b) We were not able to take the fast path to the new actor, the task gets
//   enqueued onto the new actor it is going to, and the thread now follows the
//   actor we were trying to hop off. At this point, note that we don't give up
//   the actor lock in `language_task_switchImpl` so we will come back to
//   defaultActorDrain with the actor lock still held.
//
// At the point of return from the job execution, we may not be holding the lock
// of the same actor that we had started off with, so we need to reevaluate what
// the current actor is
static void defaultActorDrain(DefaultActorImpl *actor) {
  LANGUAGE_TASK_DEBUG_LOG("Draining default actor %p", actor);
  DefaultActorImpl *currentActor = actor;

  bool actorLockAcquired = actor->tryLock(true);
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
  if (!actorLockAcquired) {
    // tryLock may fail when we compete with other stealers for the actor.
    goto done;
  }
#endif

  (void)actorLockAcquired;
  assert(actorLockAcquired);

  // Setup a TSD for tracking current execution info
  ExecutorTrackingInfo trackingInfo;
  trackingInfo.enterAndShadow(
      SerialExecutorRef::forDefaultActor(asAbstract(currentActor)),
      /*taskExecutor, will be replaced per each job. */
      TaskExecutorRef::undefined());

  while (true) {
    Job *job = currentActor->drainOne();
    if (job == NULL) {
      // No work left to do, try unlocking the actor. This may fail if there is
      // work concurrently enqueued in which case, we'd try again in the loop
      if (currentActor->unlock(false)) {
        break;
      }
    } else {
      if (AsyncTask *task = dyn_cast<AsyncTask>(job)) {
        auto taskExecutor = task->getPreferredTaskExecutor();
        trackingInfo.setTaskExecutor(taskExecutor);
      }

      // This thread is now going to follow the task on this actor.
      // It may hop off the actor
      runJobInEstablishedExecutorContext(job);

      // We could have come back from the job on a generic executor and not as
      // part of a default actor. If so, there is no more work left for us to do
      // here.
      auto currentExecutor = trackingInfo.getActiveExecutor();
      if (!currentExecutor.isDefaultActor()) {
        currentActor = nullptr;
        break;
      }
      currentActor = asImpl(currentExecutor.getDefaultActor());
    }

    if (shouldYieldThread()) {
      currentActor->unlock(true);
      break;
    }

    currentActor->processIncomingQueue();
  }

  // Leave the tracking info.
  trackingInfo.leave();

#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
done:
  ; // Suppress a -Wc++23-extensions warning
#endif
}

LANGUAGE_CC(languageasync)
void ProcessOutOfLineJob::process(Job *job) {
  auto self = cast<ProcessOutOfLineJob>(job);
  DefaultActorImpl *actor = self->Actor;

  language_cxx_deleteObject(self);
  return defaultActorDrain(actor); // 'return' forces tail call
}

#endif /* !LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS */

void DefaultActorImpl::destroy() {
#if LANGUAGE_CONCURRENCY_EMBEDDED
  // Embedded runtime does not track the refcount inside deinit
  // See language_release_n_(object:,n:) in EmbeddedRuntime.code
#else
  HeapObject *object = asAbstract(this);
  size_t retainCount = language_retainCount(object);
  if (LANGUAGE_UNLIKELY(retainCount > 1)) {
    auto descriptor = object->metadata->getTypeContextDescriptor();

    language_Concurrency_fatalError(0,
                      "Object %p of class %s deallocated with non-zero retain "
                      "count %zd. This object's deinit, or something called "
                      "from it, may have created a strong reference to self "
                      "which outlived deinit, resulting in a dangling "
                      "reference.\n",
                      object,
                      descriptor ? descriptor->Name.get() : "<unknown>",
                      retainCount);
  }
#endif

#if LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
  // TODO (rokhinip): Do something to assert that the lock is unowned
#else
  auto oldState = _status().load(std::memory_order_acquire);
  // Tasks on an actor are supposed to keep the actor alive until they start
  // running and we can only get here if ref count of the object = 0 which means
  // there should be no more tasks enqueued on the actor.
  assert(!oldState.getFirstUnprioritizedJob() && "actor has queued jobs at destruction");

  if (oldState.isIdle()) {
    assert(prioritizedJobs.empty() && "actor has queued jobs at destruction");
    return;
  }
  assert(oldState.isRunning() && "actor scheduled but not running at destruction");
  // In running state we cannot safely access prioritizedJobs to assert that it is empty.
#endif
}

void DefaultActorImpl::deallocate() {
#if LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
  releaseLock();
#else
  // If we're running, mark ourselves as ready for deallocation but don't
  // deallocate yet. When we stop running the actor - at unlock() time - we'll
  // do the actual deallocation.
  //
  // If we're idle, just deallocate immediately
  auto oldState = _status().load(std::memory_order_relaxed);
  while (oldState.isRunning()) {
    auto newState = oldState.withZombie_ReadyForDeallocation();
    if (_status().compare_exchange_weak(oldState, newState,
                                           std::memory_order_relaxed,
                                           std::memory_order_relaxed))
      return;
  }

  assert(oldState.isIdle());
  deallocateUnconditional();
#endif
}

void DefaultActorImpl::deallocateUnconditional() {
  concurrency::trace::actor_deallocate(this);

#if !LANGUAGE_CONCURRENCY_EMBEDDED
  auto metadata = cast<ClassMetadata>(this->metadata);
  language_deallocClassInstance(this, metadata->getInstanceSize(),
                             metadata->getInstanceAlignMask());
#else
  // Embedded Codira's runtime doesn't actually use the size/mask values.
  language_deallocClassInstance(this, 0, 0);
#endif
}

bool DefaultActorImpl::tryLock(bool asDrainer) {
#if LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
  retainLock();
  this->drainLock.lock();
  return true;
#else /* LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS */

#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
  LANGUAGE_TASK_DEBUG_LOG("Thread %#x attempting to jump onto %p, as drainer = %d", dispatch_lock_value_for_self(), this, asDrainer);
  dispatch_thread_override_info_s threadOverrideInfo;
  threadOverrideInfo = language_dispatch_thread_get_current_override_qos_floor();
  qos_class_t overrideFloor = threadOverrideInfo.override_qos_floor;
retry:;
#else
  LANGUAGE_TASK_DEBUG_LOG("Thread attempting to jump onto %p, as drainer = %d", this, asDrainer);
#endif

  bool distributedActorIsRemote = language_distributed_actor_is_remote(this);
  auto oldState = _status().load(std::memory_order_relaxed);
  while (true) {
    bool assertNoJobs = false;
    if (asDrainer) {
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
      if (!oldState.isScheduled()) {
        // Some other actor stealer won the race and started running the actor
        // and potentially be done with it if state is observed as idle here.

        // This extra processing jobs releases its reference. See ownership rule
        // (4).
        language_release(this);

        return false;
      }
#endif

      // We are still in the race with other stealers to take over the actor.
      assert(oldState.isScheduled());

#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
      // We only want to self override a thread if we are taking the actor lock
      // as a drainer because there might have been higher priority work
      // enqueued that might have escalated the max priority of the actor to be
      // higher than the original thread request.
      qos_class_t maxActorPriority = (qos_class_t) oldState.getMaxPriority();

      if (threadOverrideInfo.can_override && (maxActorPriority > overrideFloor)) {
        LANGUAGE_TASK_DEBUG_LOG("[Override] Self-override thread with oq_floor %#x to match max actor %p's priority %#x", overrideFloor, this, maxActorPriority);

        (void) language_dispatch_thread_override_self(maxActorPriority);
        overrideFloor = maxActorPriority;
        goto retry;
      }
#endif /* LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION */
    } else {
      // We're trying to take the lock in an uncontended manner
      if (oldState.isRunning() || oldState.isScheduled()) {
        LANGUAGE_TASK_DEBUG_LOG("Failed to jump to %p in fast path", this);
        return false;
      }

      assert(oldState.getMaxPriority() == JobPriority::Unspecified);
      assert(!oldState.getFirstUnprioritizedJob());
      // We cannot assert here that prioritizedJobs is empty,
      // because lock is not held yet. Raise a flag to assert after getting the lock.
      assertNoJobs = true;
    }

    // Taking the drain lock clears the max priority escalated bit because we've
    // already represented the current max priority of the actor on the thread.
    auto newState = oldState.withRunning();
    newState = newState.withoutEscalatedPriority();

    // Claim incoming jobs when obtaining lock as a drainer, to save one
    // round of atomic load and compare-exchange.
    // This is not useful when obtaining lock for assuming thread during actor
    // switching, because arbitrary use code can run between locking and
    // draining the next job. So we still need to call processIncomingQueue() to
    // check for higher priority jobs that could have been scheduled in the
    // meantime. And processing is more efficient when done in larger batches.
    if (asDrainer) {
      newState = newState.withFirstUnprioritizedJob(nullptr);
    }

    // This needs an acquire since we are taking a lock
    if (_status().compare_exchange_weak(oldState, newState,
                                 std::memory_order_acquire,
                                 std::memory_order_relaxed)) {
      _language_tsan_acquire(this);
      if (assertNoJobs) {
        assert(prioritizedJobs.empty());
      }
      traceActorStateTransition(this, oldState, newState, distributedActorIsRemote);
      if (asDrainer) {
        handleUnprioritizedJobs(oldState.getFirstUnprioritizedJob());
      }
      return true;
    }
  }
#endif /* LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS */
}

/* This can be called in 2 ways:
 *    * force_unlock = true: Thread is following task and therefore wants to
 *    give up the current actor since task is switching away.
 *    * force_unlock = false: Thread is not necessarily following the task, it
 *    may want to follow actor if there is more work on it.
 *
 *    Returns true if we managed to unlock the actor, false if we didn't. If the
 *    actor has more jobs remaining on it during unlock, this method will
 *    schedule the actor
 */
bool DefaultActorImpl::unlock(bool forceUnlock)
{
#if LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
  this->drainLock.unlock();
  releaseLock();
  return true;
#else
  bool distributedActorIsRemote = language_distributed_actor_is_remote(this);
  auto oldState = _status().load(std::memory_order_relaxed);
  LANGUAGE_TASK_DEBUG_LOG("Try unlock-ing actor %p with forceUnlock = %d", this, forceUnlock);

  _language_tsan_release(this);
  while (true) {
    assert(oldState.isAnyRunning());
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
    assert(dispatch_lock_is_locked_by_self(*(this->drainLockAddr())));
#endif

    if (oldState.isZombie_ReadyForDeallocation()) {
#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
      // Reset any override on this thread as a result of this thread running
      // the actor
      if (oldState.isMaxPriorityEscalated()) {
        language_dispatch_lock_override_end((qos_class_t)oldState.getMaxPriority());
      }
#endif
      deallocateUnconditional();
      LANGUAGE_TASK_DEBUG_LOG("Unlock-ing actor %p succeeded with full deallocation", this);
      return true;
    }

    auto newState = oldState;
    // Lock is still held at this point, so it is safe to access prioritizedJobs
    if (!prioritizedJobs.empty() || oldState.getFirstUnprioritizedJob()) {
      // There is work left to do, don't unlock the actor
      if (!forceUnlock) {
        LANGUAGE_TASK_DEBUG_LOG("Unlock-ing actor %p failed", this);
        return false;
      }
      // We need to schedule the actor - remove any escalation bits since we'll
      // schedule the actor at the max priority currently on it

      // N decreases by 1 as this processing job is going away; but R is
      // still 1. We schedule a new processing job to maintain N >= R.

      // It is possible that there are stealers scheduled for the actor already;
      // but, we still schedule one anyway. This is because it is possible that
      // those stealers got scheduled when we were running the actor and gone
      // away. (See tryLock function.)
      newState = newState.withScheduled();
      newState = newState.withoutEscalatedPriority();
    } else {
      // There is no work left to do - actor goes idle

      // R becomes 0 and N decreases by 1.
      // But, we may still have stealers scheduled so N could be > 0. This is
      // fine since N >= R. Every such stealer, once scheduled, will observe
      // actor as idle, will release its ref and return. (See tryLock function.)
      newState = newState.withIdle();
      newState = newState.resetPriority();
    }

    // This needs to be a release since we are unlocking a lock
    if (_status().compare_exchange_weak(oldState, newState,
                      /* success */ std::memory_order_release,
                      /* failure */ std::memory_order_relaxed)) {
      _language_tsan_release(this);
      traceActorStateTransition(this, oldState, newState, distributedActorIsRemote);

      if (newState.isScheduled()) {
        // See ownership rule (6) in DefaultActorImpl
        // FIXME: should we specify some task executor here, since otherwise we'll schedule on the global pool
        scheduleActorProcessJob(newState.getMaxPriority(), TaskExecutorRef::undefined());
      } else {
        // See ownership rule (5) in DefaultActorImpl
        LANGUAGE_TASK_DEBUG_LOG("Actor %p is idle now", this);
      }

#if LANGUAGE_CONCURRENCY_ENABLE_PRIORITY_ESCALATION
      // Reset any asynchronous escalations we may have gotten on this thread
      // after taking the drain lock.
      //
      // Only do this after we have reenqueued the actor so that we don't lose
      // any "mojo" prior to the enqueue.
      if (oldState.isMaxPriorityEscalated()) {
        language_dispatch_lock_override_end((qos_class_t) oldState.getMaxPriority());
      }
#endif
      return true;
    }
  }
#endif
}

#if LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
void DefaultActorImpl::retainLock() {
  lockReferenceCount.fetch_add(1, std::memory_order_acquire);
}
void DefaultActorImpl::releaseLock() {
  if (1 == lockReferenceCount.fetch_add(-1, std::memory_order_release)) {
    drainLock.~Mutex();
    deallocateUnconditional();
  }
}
#endif

LANGUAGE_CC(language)
static void language_job_runImpl(Job *job, SerialExecutorRef executor) {
  ExecutorTrackingInfo trackingInfo;

  // language_job_run is a primary entrypoint for executors telling us to
  // run jobs.  Actor executors won't expect us to switch off them
  // during this operation.  But do allow switching if the executor
  // is generic.
  if (!executor.isGeneric()) {
    trackingInfo.disallowSwitching();
  }

  auto taskExecutor = executor.isGeneric()
                          ? TaskExecutorRef::fromTaskExecutorPreference(job)
                          : TaskExecutorRef::undefined();

  trackingInfo.enterAndShadow(executor, taskExecutor);

  LANGUAGE_TASK_DEBUG_LOG("job %p", job);
  runJobInEstablishedExecutorContext(job);

  trackingInfo.leave();

  // Give up the current executor if this is a switching context
  // (which, remember, only happens if we started out on a generic
  // executor) and we've switched to a default actor.
  auto currentExecutor = trackingInfo.getActiveExecutor();
  if (trackingInfo.allowsSwitching() && currentExecutor.isDefaultActor()) {
    asImpl(currentExecutor.getDefaultActor())->unlock(true);
  }
}

LANGUAGE_CC(language)
static void language_job_run_on_serial_and_task_executorImpl(Job *job,
                                             SerialExecutorRef serialExecutor,
                                             TaskExecutorRef taskExecutor) {
  ExecutorTrackingInfo trackingInfo;
  LANGUAGE_TASK_DEBUG_LOG("Run job %p on serial executor %p task executor %p", job,
                       serialExecutor.getIdentity(), taskExecutor.getIdentity());

  // TODO: we don't allow switching
  trackingInfo.disallowSwitching();
  trackingInfo.enterAndShadow(serialExecutor, taskExecutor);

  LANGUAGE_TASK_DEBUG_LOG("job %p", job);
  runJobInEstablishedExecutorContext(job);

  trackingInfo.leave();

  // Give up the current executor if this is a switching context
  // (which, remember, only happens if we started out on a generic
  // executor) and we've switched to a default actor.
  auto currentExecutor = trackingInfo.getActiveExecutor();
  if (trackingInfo.allowsSwitching() && currentExecutor.isDefaultActor()) {
    asImpl(currentExecutor.getDefaultActor())->unlock(true);
  }
}

LANGUAGE_CC(language)
static void language_job_run_on_task_executorImpl(Job *job,
                                               TaskExecutorRef taskExecutor) {
  language_job_run_on_serial_and_task_executor(
      job, SerialExecutorRef::generic(), taskExecutor);
}

void language::language_defaultActor_initialize(DefaultActor *_actor) {
  asImpl(_actor)->initialize();
}

void language::language_defaultActor_destroy(DefaultActor *_actor) {
  asImpl(_actor)->destroy();
}

void language::language_defaultActor_enqueue(Job *job, DefaultActor *_actor) {
#if LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
  assert(false && "Should not enqueue onto default actor in actor as locks model");
#else
  asImpl(_actor)->enqueue(job, job->getPriority());
#endif
}

void language::language_defaultActor_deallocate(DefaultActor *_actor) {
  asImpl(_actor)->deallocate();
}

static bool isDefaultActorClass(const ClassMetadata *metadata) {
  assert(metadata->isTypeMetadata());
  while (true) {
    // Trust the class descriptor if it says it's a default actor.
    if (!metadata->isArtificialSubclass() &&
        metadata->getDescription()->isDefaultActor()) {
      return true;
    }

    // Go to the superclass.
    metadata = metadata->Superclass;

    // If we run out of Codira classes, it's not a default actor.
    if (!metadata || !metadata->isTypeMetadata()) {
      return false;
    }
  }
}

void language::language_defaultActor_deallocateResilient(HeapObject *actor) {
#if !LANGUAGE_CONCURRENCY_EMBEDDED
  auto metadata = cast<ClassMetadata>(actor->metadata);
  if (isDefaultActorClass(metadata))
    return language_defaultActor_deallocate(static_cast<DefaultActor*>(actor));

  language_deallocObject(actor, metadata->getInstanceSize(),
                      metadata->getInstanceAlignMask());
#else
  return language_defaultActor_deallocate(static_cast<DefaultActor*>(actor));
#endif
}

/// FIXME: only exists for the quick-and-dirty MainActor implementation.
namespace language {
  Metadata* MainActorMetadata = nullptr;
}

/*****************************************************************************/
/****************************** ACTOR SWITCHING ******************************/
/*****************************************************************************/

/// Can the current executor give up its thread?
static bool canGiveUpThreadForSwitch(ExecutorTrackingInfo *trackingInfo,
                                     SerialExecutorRef currentExecutor) {
  assert(trackingInfo || currentExecutor.isGeneric());

  // Some contexts don't allow switching at all.
  if (trackingInfo && !trackingInfo->allowsSwitching()) {
    return false;
  }

  // We can certainly "give up" a generic executor to try to run
  // a task for an actor.
  if (currentExecutor.isGeneric()) {
    if (currentExecutor.isForSynchronousStart()) {
      return false;
    }

    return true;
  }

  // If the current executor is a default actor, we know how to make
  // it give up its thread.
  if (currentExecutor.isDefaultActor())
    return true;

  return false;
}

/// Tell the current executor to give up its thread, given that it
/// returned true from canGiveUpThreadForSwitch().
///
/// Note that we don't update DefaultActorProcessingFrame here; we'll
/// do that in runOnAssumedThread.
static void giveUpThreadForSwitch(SerialExecutorRef currentExecutor) {
  if (currentExecutor.isGeneric()) {
    LANGUAGE_TASK_DEBUG_LOG("Giving up current generic executor %p",
                         currentExecutor.getIdentity());
    return;
  }

  asImpl(currentExecutor.getDefaultActor())->unlock(true);
}

/// Try to assume control of the current thread for the given executor
/// in order to run the given job.
///
/// This doesn't actually run the job yet.
///
/// Note that we don't update DefaultActorProcessingFrame here; we'll
/// do that in runOnAssumedThread.
static bool tryAssumeThreadForSwitch(SerialExecutorRef newExecutor,
                                     TaskExecutorRef newTaskExecutor) {
  // If the new executor is generic, we don't need to do anything.
  if (newExecutor.isGeneric() && newTaskExecutor.isUndefined()) {
    return true;
  }

  // If the new executor is a default actor, ask it to assume the thread.
  if (newExecutor.isDefaultActor()) {
    return asImpl(newExecutor.getDefaultActor())->tryLock(false);
  }

  return false;
}

static bool mustSwitchToRun(SerialExecutorRef currentSerialExecutor,
                            SerialExecutorRef newSerialExecutor,
                            TaskExecutorRef currentTaskExecutor,
                            TaskExecutorRef newTaskExecutor) {
  if (currentSerialExecutor.getIdentity() != newSerialExecutor.getIdentity()) {
    return true; // must switch, new isolation context
  }

  // else, we may have to switch if the preferred task executor is different
  auto differentTaskExecutor =
      currentTaskExecutor.getIdentity() != newTaskExecutor.getIdentity();
  if (differentTaskExecutor) {
    return true;
  }

  return false;
}

/// Given that we've assumed control of an executor on this thread,
/// continue to run the given task on it.
LANGUAGE_CC(languageasync)
static void runOnAssumedThread(AsyncTask *task, SerialExecutorRef executor,
                               ExecutorTrackingInfo *oldTracking) {
  // Note that this doesn't change the active task and so doesn't
  // need to either update ActiveTask or flagAsRunning/flagAsSuspended.

  // If there's already tracking info set up, just change the executor
  // there and tail-call the task.  We don't want these frames to
  // potentially accumulate linearly.
  if (oldTracking) {
    oldTracking->setActiveExecutor(executor);
    oldTracking->setTaskExecutor(task->getPreferredTaskExecutor());

    return task->runInFullyEstablishedContext(); // 'return' forces tail call
  }

  // Otherwise, set up tracking info.
  ExecutorTrackingInfo trackingInfo;
  trackingInfo.enterAndShadow(executor, task->getPreferredTaskExecutor());

  // Run the new task.
  task->runInFullyEstablishedContext();

  // Leave the tracking frame, and give up the current actor if
  // we have one.
  //
  // In principle, we could execute more tasks from the actor here, but
  // that's probably not a reasonable thing to do in an assumed context
  // rather than a dedicated actor-processing job.
  executor = trackingInfo.getActiveExecutor();
  trackingInfo.leave();

  LANGUAGE_TASK_DEBUG_LOG("leaving assumed thread, current executor is %p",
                       executor.getIdentity());

  if (executor.isDefaultActor())
    asImpl(executor.getDefaultActor())->unlock(true);
}

LANGUAGE_CC(languageasync)
static void language_task_switchImpl(LANGUAGE_ASYNC_CONTEXT AsyncContext *resumeContext,
                                  TaskContinuationFunction *resumeFunction,
                                  SerialExecutorRef newExecutor) {
  auto task = language_task_getCurrent();
  assert(task && "no current task!");

  auto trackingInfo = ExecutorTrackingInfo::current();
  auto currentExecutor =
    (trackingInfo ? trackingInfo->getActiveExecutor()
                  : SerialExecutorRef::generic());
  auto currentTaskExecutor = (trackingInfo ? trackingInfo->getTaskExecutor()
                                           : TaskExecutorRef::undefined());
  auto newTaskExecutor = task->getPreferredTaskExecutor();
  LANGUAGE_TASK_DEBUG_LOG("Task %p trying to switch executors: executor %p%s to "
                       "new serial executor: %p%s; task executor: from %p%s to %p%s",
                       task,
                       currentExecutor.getIdentity(),
                       currentExecutor.getIdentityDebugName(),
                       newExecutor.getIdentity(),
                       newExecutor.getIdentityDebugName(),
                       currentTaskExecutor.getIdentity(),
                       currentTaskExecutor.isDefined() ? "" : " (undefined)",
                       newTaskExecutor.getIdentity(),
                       newTaskExecutor.isDefined() ? "" : " (undefined)");

  // If the current executor is compatible with running the new executor,
  // we can just immediately continue running with the resume function
  // we were passed in.
  if (!mustSwitchToRun(currentExecutor, newExecutor, currentTaskExecutor,
                       newTaskExecutor)) {
    LANGUAGE_TASK_DEBUG_LOG("Task %p run inline", task);
    return resumeFunction(resumeContext); // 'return' forces tail call
  }

  // Park the task for simplicity instead of trying to thread the
  // initial resumption information into everything below.
  task->ResumeContext = resumeContext;
  task->ResumeTask = resumeFunction;

  // If the current executor can give up its thread, and the new executor
  // can take over a thread, try to do so; but don't do this if we've
  // been asked to yield the thread.
  LANGUAGE_TASK_DEBUG_LOG("Task %p can give up thread?", task);
  if (currentTaskExecutor.isUndefined() &&
      canGiveUpThreadForSwitch(trackingInfo, currentExecutor) &&
      !shouldYieldThread() &&
      tryAssumeThreadForSwitch(newExecutor, newTaskExecutor)) {
    LANGUAGE_TASK_DEBUG_LOG(
        "switch succeeded, task %p assumed thread for executor %p", task,
        newExecutor.getIdentity());
    giveUpThreadForSwitch(currentExecutor);
    // 'return' forces tail call
    return runOnAssumedThread(task, newExecutor, trackingInfo);
  }

  // Otherwise, just asynchronously enqueue the task on the given
  // executor.
  LANGUAGE_TASK_DEBUG_LOG(
      "switch failed, task %p enqueued on executor %p (task executor: %p)",
      task, newExecutor.getIdentity(), currentTaskExecutor.getIdentity());

  _language_task_clearCurrent();
  task->flagAsAndEnqueueOnExecutor(newExecutor);
}

LANGUAGE_CC(language)
static void
language_task_immediateImpl(AsyncTask *task,
                         SerialExecutorRef targetExecutor) {
  language_retain(task);
  if (targetExecutor.isGeneric()) {
  // If the target is generic, it means that the closure did not specify
  // an isolation explicitly. According to the "start synchronously" rules,
  // we should therefore ignore the global and just start running on the
  // caller immediately.
  SerialExecutorRef executor = SerialExecutorRef::forSynchronousStart();

  auto originalTask = ActiveTask::swap(task);
  language_job_run(task, executor);
  _language_task_setCurrent(originalTask);
  } else {
    assert(language_task_isCurrentExecutor(targetExecutor) &&
           "'immediate' must only be invoked when it is correctly in "
           "the same isolation already, but wasn't!");

    // We can run synchronously, we're on the expected executor so running in
    // the caller context is going to be in the same context as the requested
    // "caller" context.
    AsyncTask *originalTask = _language_task_clearCurrent();

    language_job_run(task, targetExecutor);
    _language_task_setCurrent(originalTask);
  }
}

#if !LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
namespace {
/// Job that allows to use executor API to schedule a block of task-less
/// synchronous code.
class IsolatedDeinitJob : public Job {
private:
  void *Object;
  DeinitWorkFunction *__ptrauth_language_deinit_work_function Work;

public:
  IsolatedDeinitJob(JobPriority priority, void *object,
                    DeinitWorkFunction * work)
      : Job({JobKind::IsolatedDeinit, priority}, &process), Object(object),
        Work(work) {}

  LANGUAGE_CC(languageasync)
  static void process(Job *_job) {
    auto *job = cast<IsolatedDeinitJob>(_job);
    void *object = job->Object;
    DeinitWorkFunction *work = job->Work;
    language_cxx_deleteObject(job);
    return work(object);
  }

  static bool classof(const Job *job) {
    return job->Flags.getKind() == JobKind::IsolatedDeinit;
  }
};
} // namespace
#endif

LANGUAGE_CC(language)
static void language_task_deinitOnExecutorImpl(void *object,
                                            DeinitWorkFunction *work,
                                            SerialExecutorRef newExecutor,
                                            size_t rawFlags) {
  // Sign the function pointer
  work = language_auth_code(
      work, SpecialPointerAuthDiscriminators::DeinitWorkFunction);
  // If the current executor is compatible with running the new executor,
  // we can just immediately continue running with the resume function
  // we were passed in.
  //
  // Note that isCurrentExecutor() returns true for @MainActor
  // when running on the main thread without any executor.
  if (language_task_isCurrentExecutorWithFlags(
          newExecutor, language_task_is_current_executor_flag::None)) {
    TaskLocal::StopLookupScope scope;
    return work(object);
  }

#if LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
  // In this mode taking actor lock is the only possible implementation
#else
  // Otherwise, it is an optimisation applied when deinitializing default actors
  if (newExecutor.isDefaultActor() && object == newExecutor.getIdentity()) {
#endif
    // Try to take the lock. This should always succeed, unless someone is
    // running the actor using unsafe unowned reference.
    if (asImpl(newExecutor.getDefaultActor())->tryLock(false)) {

      // Don't unlock current executor, because we must preserve it when
      // returning. If we release the lock, we might not be able to get it back.
      // It cannot produce deadlocks, because:
      //   * we use tryLock(), not lock()
      //   * each object can be deinitialized only once, so call graph of
      //   deinit's cannot have cycles.

      // Function runOnAssumedThread() tries to reuse existing tracking info,
      // but we don't have a tail call anyway, so this does not help much here.
      // Always create new tracking info to keep code simple.
      ExecutorTrackingInfo trackingInfo;

      // The only place where ExecutorTrackingInfo::getTaskExecutor() is
      // called is in language_task_switch(), but language_task_switch() cannot be
      // called from the synchronous code. So it does not really matter what is
      // set in the ExecutorTrackingInfo::ActiveExecutor for the duration of the
      // isolated deinit - it is unobservable anyway.
      TaskExecutorRef taskExecutor = TaskExecutorRef::undefined();
      trackingInfo.enterAndShadow(newExecutor, taskExecutor);

      // Run the work.
      {
        TaskLocal::StopLookupScope scope;
        work(object);
      }

      // `work` is a synchronous function, it cannot call language_task_switch()
      // If it calls any synchronous API that may change executor inside
      // tracking info, that API is also responsible for changing it back.
      assert(newExecutor == trackingInfo.getActiveExecutor());
      assert(taskExecutor == trackingInfo.getTaskExecutor());

      // Leave the tracking frame
      trackingInfo.leave();

      // Give up the current actor.
      asImpl(newExecutor.getDefaultActor())->unlock(true);
      return;
    } else {
#if LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
      assert(false && "Should not enqueue onto default actor in actor as locks model");
#endif
    }
#if !LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
  }
  auto currentTask = language_task_getCurrent();
  auto priority = currentTask ? language_task_currentPriority(currentTask)
                              : language_task_getCurrentThreadPriority();

  auto job = language_cxx_newObject<IsolatedDeinitJob>(priority, object, work);
  language_task_enqueue(job, newExecutor);
#endif
}

/*****************************************************************************/
/************************* GENERIC ACTOR INTERFACES **************************/
/*****************************************************************************/

extern "C" LANGUAGE_CC(language) void _language_task_makeAnyTaskExecutor(
    HeapObject *executor, const Metadata *selfType,
    const TaskExecutorWitnessTable *wtable);

LANGUAGE_CC(language)
static void language_task_enqueueImpl(Job *job, SerialExecutorRef serialExecutorRef) {
#ifndef NDEBUG
  auto _taskExecutorRef = TaskExecutorRef::undefined();
  if (auto task = dyn_cast<AsyncTask>(job)) {
    _taskExecutorRef = task->getPreferredTaskExecutor();
  }
  LANGUAGE_TASK_DEBUG_LOG("enqueue job %p on serial serialExecutor %p, taskExecutor = %p", job,
                       serialExecutorRef.getIdentity(),
                       _taskExecutorRef.getIdentity());
#endif

  assert(job && "no job provided");
  job->SchedulerPrivate[0] = NULL;
  job->SchedulerPrivate[1] = NULL;

  _language_tsan_release(job);

  if (serialExecutorRef.isGeneric()) {
    if (auto task = dyn_cast<AsyncTask>(job)) {
      auto taskExecutorRef = task->getPreferredTaskExecutor();
      if (taskExecutorRef.isDefined()) {
#if LANGUAGE_CONCURRENCY_EMBEDDED
        language_unreachable("task executors not supported in embedded Codira");
#else
        auto taskExecutorIdentity = taskExecutorRef.getIdentity();
        auto taskExecutorType = language_getObjectType(taskExecutorIdentity);
        auto taskExecutorWtable = taskExecutorRef.getTaskExecutorWitnessTable();

        return _language_task_enqueueOnTaskExecutor(
            job,
            taskExecutorIdentity, taskExecutorType, taskExecutorWtable);
#endif // LANGUAGE_CONCURRENCY_EMBEDDED
      } // else, fall-through to the default global enqueue
    }
    return language_task_enqueueGlobal(job);
  }

  if (serialExecutorRef.isDefaultActor()) {
    return language_defaultActor_enqueue(job, serialExecutorRef.getDefaultActor());
  }

#if LANGUAGE_CONCURRENCY_EMBEDDED
  language_unreachable("custom executors not supported in embedded Codira");
#else
  // For main actor or actors with custom executors
  auto serialExecutorIdentity = serialExecutorRef.getIdentity();
  auto serialExecutorType = language_getObjectType(serialExecutorIdentity);
  auto serialExecutorWtable = serialExecutorRef.getSerialExecutorWitnessTable();
  _language_task_enqueueOnExecutor(job, serialExecutorIdentity, serialExecutorType,
                                serialExecutorWtable);
#endif // LANGUAGE_CONCURRENCY_EMBEDDED
}

static void
language_actor_escalate(DefaultActorImpl *actor, AsyncTask *task, JobPriority newPriority) {
#if !LANGUAGE_CONCURRENCY_ACTORS_AS_LOCKS
  return actor->enqueueStealer(task, newPriority);
#endif
}

LANGUAGE_CC(language)
void language::language_executor_escalate(SerialExecutorRef executor, AsyncTask *task,
  JobPriority newPriority) {
  if (executor.isGeneric()) {
    // TODO (rokhinip): We'd push a stealer job for the task on the executor.
    return;
  }

  if (executor.isDefaultActor()) {
    return language_actor_escalate(asImpl(executor.getDefaultActor()), task, newPriority);
  }

  // TODO (rokhinip): This is either the main actor or an actor with a custom
  // executor. We need to let the executor know that the job has been escalated.
  // For now, do nothing
  return;
}

#define OVERRIDE_ACTOR COMPATIBILITY_OVERRIDE
#include "../CompatibilityOverride/CompatibilityOverrideIncludePath.h"

/*****************************************************************************/
/***************************** DISTRIBUTED ACTOR *****************************/
/*****************************************************************************/

void language::language_nonDefaultDistributedActor_initialize(NonDefaultDistributedActor *_actor) {
  asImpl(_actor)->initialize();
}

OpaqueValue*
language::language_distributedActor_remote_initialize(const Metadata *actorType) {
  const ClassMetadata *metadata = actorType->getClassObject();

  // TODO(distributed): make this allocation smaller
  // ==== Allocate the memory for the remote instance
  HeapObject *alloc = language_allocObject(metadata,
                                        metadata->getInstanceSize(),
                                        metadata->getInstanceAlignMask());

  // TODO: remove this memset eventually, today we only do this to not have
  //       to modify the destructor logic, as releasing zeroes is no-op
  memset(alloc + 1, 0, metadata->getInstanceSize() - sizeof(HeapObject));

  // TODO(distributed): a remote one does not have to have the "real"
  //  default actor body, e.g. we don't need an executor at all; so
  //  we can allocate more efficiently and only share the flags/status field
  //  between the both memory representations
  // If it is a default actor, we reuse the same layout as DefaultActorImpl,
  // and store flags in the allocation directly as we initialize it.
  if (isDefaultActorClass(metadata)) {
    auto actor = asImpl(reinterpret_cast<DefaultActor *>(alloc));
    actor->initialize(/*remote*/true);
    assert(language_distributed_actor_is_remote(alloc));
    return reinterpret_cast<OpaqueValue*>(actor);
  } else {
    auto actor = asImpl(reinterpret_cast<NonDefaultDistributedActor *>(alloc));
    actor->initialize(/*remote*/true);
    assert(language_distributed_actor_is_remote(alloc));
    return reinterpret_cast<OpaqueValue*>(actor);
  }
}

bool language::language_distributed_actor_is_remote(HeapObject *_actor) {
#if !LANGUAGE_CONCURRENCY_EMBEDDED
  const ClassMetadata *metadata = cast<ClassMetadata>(_actor->metadata);
  if (isDefaultActorClass(metadata)) {
    return asImpl(reinterpret_cast<DefaultActor *>(_actor))->isDistributedRemote();
  } else {
    return asImpl(reinterpret_cast<NonDefaultDistributedActor *>(_actor))->isDistributedRemote();
  }
#else
  return false;
#endif
}

bool DefaultActorImpl::isDistributedRemote() {
  return this->isDistributedRemoteActor;
}
