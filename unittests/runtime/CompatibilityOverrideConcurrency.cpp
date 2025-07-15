//===--- CompatibilityOverrideConcurrency.cpp - Compatibility override tests ---===//
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

#if defined(__APPLE__) && defined(__MACH__)

#define LANGUAGE_TARGET_LIBRARY_NAME language_Concurrency

#include "../../stdlib/public/CompatibilityOverride/CompatibilityOverride.h"
#include "language/Runtime/Concurrency.h"
#include "gtest/gtest.h"

#if __has_include("pthread.h")

#define RUN_ASYNC_MAIN_DRAIN_QUEUE_TEST 1
#include <pthread.h>
#endif // HAVE_PTHREAD_H

#include <stdio.h>

using namespace language;

static bool EnableOverride;
static bool Ran;

namespace {
template <typename T>
T getEmptyValue() {
  return T();
}
template <>
SerialExecutorRef getEmptyValue() {
  return SerialExecutorRef::generic();
}
} // namespace

#define OVERRIDE(name, ret, attrs, ccAttrs, namespace, typedArgs, namedArgs)   \
  static ccAttrs ret name##Override(COMPATIBILITY_UNPAREN_WITH_COMMA(          \
      typedArgs) Original_##name originalImpl) {                               \
    if (!EnableOverride)                                                       \
      return originalImpl COMPATIBILITY_PAREN(namedArgs);                      \
    Ran = true;                                                                \
    return getEmptyValue<ret>();                                               \
  }
#define OVERRIDE_TASK_NORETURN(name, attrs, ccAttrs, namespace, typedArgs,     \
                               namedArgs)                                      \
  static ccAttrs void name##Override(COMPATIBILITY_UNPAREN_WITH_COMMA(         \
      typedArgs) Original_##name originalImpl) {                               \
    if (!EnableOverride)                                                       \
      originalImpl COMPATIBILITY_PAREN(namedArgs);                             \
    Ran = true;                                                                \
  }

#include "../../stdlib/public/CompatibilityOverride/CompatibilityOverrideConcurrency.def"

struct OverrideSection {
  uintptr_t version;

#define OVERRIDE(name, ret, attrs, ccAttrs, namespace, typedArgs, namedArgs)   \
  Override_##name name;
#include "../../stdlib/public/CompatibilityOverride/CompatibilityOverrideConcurrency.def"
};

OverrideSection ConcurrencyOverrides
    __attribute__((section("__DATA," COMPATIBILITY_OVERRIDE_SECTION_NAME_language_Concurrency))) = {
        0,
#define OVERRIDE(name, ret, attrs, ccAttrs, namespace, typedArgs, namedArgs)   \
  name##Override,
#include "../../stdlib/public/CompatibilityOverride/CompatibilityOverrideConcurrency.def"
};

LANGUAGE_CC(language)
static void
language_task_enqueueGlobal_override(Job *job,
                                  language_task_enqueueGlobal_original original) {
  Ran = true;
}

LANGUAGE_CC(language)
static void
language_task_checkIsolated_override(SerialExecutorRef executor,
                                      language_task_checkIsolated_original original) {
  Ran = true;
}

LANGUAGE_CC(language)
static int8_t
language_task_isIsolatingCurrentContext_override(SerialExecutorRef executor,
                                      language_task_isIsolatingCurrentContext_original original) {
  Ran = true;
  return 0;
}

LANGUAGE_CC(language)
static void language_task_enqueueGlobalWithDelay_override(
    unsigned long long delay, Job *job,
    language_task_enqueueGlobalWithDelay_original original) {
  Ran = true;
}

LANGUAGE_CC(language)
static void language_task_enqueueMainExecutor_override(
    Job *job, language_task_enqueueMainExecutor_original original) {
  Ran = true;
}

LANGUAGE_CC(language)
static void language_task_startOnMainActor_override(AsyncTask* task) {
  Ran = true;
}

LANGUAGE_CC(language)
static void language_task_immediate_override(AsyncTask* task, SerialExecutorRef targetExecutor) {
  Ran = true;
}

#ifdef RUN_ASYNC_MAIN_DRAIN_QUEUE_TEST
[[noreturn]] LANGUAGE_CC(language)
static void language_task_asyncMainDrainQueue_override_fn(
    language_task_asyncMainDrainQueue_original original,
    language_task_asyncMainDrainQueue_override compatOverride) {
  Ran = true;
  pthread_exit(nullptr); // noreturn function
}
#endif

class CompatibilityOverrideConcurrencyTest : public ::testing::Test {
protected:
  virtual void SetUp() {
    // This check is a bit pointless, as long as you trust the section
    // attribute to work correctly, but it ensures that the override
    // section actually gets linked into the test executable. If it's not
    // used then it disappears and the real tests begin to fail.
    ASSERT_EQ(ConcurrencyOverrides.version, static_cast<uintptr_t>(0));

    EnableOverride = true;
    Ran = false;

    // Executor hooks need to be set manually.
    language_task_enqueueGlobal_hook = language_task_enqueueGlobal_override;
    language_task_enqueueGlobalWithDelay_hook =
        language_task_enqueueGlobalWithDelay_override;
    language_task_enqueueMainExecutor_hook =
        language_task_enqueueMainExecutor_override;
    language_task_checkIsolated_hook =
        language_task_checkIsolated_override;
    language_task_isIsolatingCurrentContext_hook =
        language_task_isIsolatingCurrentContext_override;
#ifdef RUN_ASYNC_MAIN_DRAIN_QUEUE_TEST
    language_task_asyncMainDrainQueue_hook =
        language_task_asyncMainDrainQueue_override_fn;
#endif
  }

  virtual void TearDown() {
    EnableOverride = false;
    ASSERT_TRUE(Ran);
  }
};

static Job fakeJob{{JobKind::DefaultActorInline},
                   static_cast<JobInvokeFunction *>(nullptr),
                   nullptr};

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_task_enqueue) {
  language_task_enqueue(&fakeJob, SerialExecutorRef::generic());
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_job_run) {
  language_job_run(&fakeJob, SerialExecutorRef::generic());
}

TEST_F(CompatibilityOverrideConcurrencyTest,
       test_language_job_run_on_task_executor) {
  language_job_run_on_task_executor(&fakeJob, TaskExecutorRef::undefined());
}

TEST_F(CompatibilityOverrideConcurrencyTest,
       test_language_job_run_on_serial_and_task_executor) {
  language_job_run_on_serial_and_task_executor(
      &fakeJob, SerialExecutorRef::generic(), TaskExecutorRef::undefined());
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_task_getCurrentExecutor) {
  language_task_getCurrentExecutor();
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_task_switch) {
  language_task_switch(nullptr, nullptr, SerialExecutorRef::generic());
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_task_enqueueGlobal) {
  language_task_enqueueGlobal(&fakeJob);
}

TEST_F(CompatibilityOverrideConcurrencyTest,
       test_language_task_enqueueGlobalWithDelay) {
  language_task_enqueueGlobalWithDelay(0, &fakeJob);
}

TEST_F(CompatibilityOverrideConcurrencyTest,
       test_language_task_checkIsolated) {
  language_task_checkIsolated(SerialExecutorRef::generic());
}

TEST_F(CompatibilityOverrideConcurrencyTest,
       test_language_task_isIsolatingCurrentContext) {
  language_task_isIsolatingCurrentContext(SerialExecutorRef::generic());
}

TEST_F(CompatibilityOverrideConcurrencyTest,
       test_language_task_enqueueMainExecutor) {
  language_task_enqueueMainExecutor(&fakeJob);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_task_create_common) {
  language_task_create_common(0, nullptr, nullptr, nullptr, nullptr, 0);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_task_future_wait) {
  language_task_future_wait(nullptr, nullptr, nullptr, nullptr, nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest,
       test_language_task_future_wait_throwing) {
  language_task_future_wait_throwing(nullptr, nullptr, nullptr, nullptr, nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_continuation_resume) {
  language_continuation_resume(nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest,
       test_language_continuation_throwingResume) {
  language_continuation_throwingResume(nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest,
       test_language_continuation_throwingResumeWithError) {
  language_continuation_throwingResumeWithError(nullptr, nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_asyncLet_wait) {
  language_asyncLet_wait(nullptr, nullptr, nullptr, nullptr, nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_asyncLet_wait_throwing) {
  language_asyncLet_wait(nullptr, nullptr, nullptr, nullptr, nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_asyncLet_end) {
  language_asyncLet_end(nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_taskGroup_initialize) {
  language_taskGroup_initialize(nullptr, nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_taskGroup_initializeWithFlags) {
  language_taskGroup_initializeWithFlags(0, nullptr, nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_taskGroup_attachChild) {
  language_taskGroup_attachChild(nullptr, nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_taskGroup_destroy) {
  language_taskGroup_destroy(nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest,
       test_language_taskGroup_wait_next_throwing) {
  language_taskGroup_wait_next_throwing(nullptr, nullptr, nullptr, nullptr,
                                     nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_taskGroup_isEmpty) {
  language_taskGroup_isEmpty(nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_taskGroup_isCancelled) {
  language_taskGroup_isCancelled(nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_taskGroup_cancelAll) {
  language_taskGroup_cancelAll(nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_taskGroup_waitAll) {
  language_taskGroup_waitAll(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_taskGroup_addPending) {
  language_taskGroup_addPending(nullptr, true);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_task_localValuePush) {
  language_task_localValuePush(nullptr, nullptr, nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_task_localValueGet) {
  language_task_localValueGet(nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_task_localValuePop) {
  language_task_localValuePop();
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_task_localsCopyTo) {
  language_task_localsCopyTo(nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, task_hasTaskGroupStatusRecord) {
  language_task_hasTaskGroupStatusRecord();
}

TEST_F(CompatibilityOverrideConcurrencyTest, task_getPreferredTaskExecutor) {
  language_task_getPreferredTaskExecutor();
}

TEST_F(CompatibilityOverrideConcurrencyTest, task_pushTaskExecutorPreference) {
  language_task_pushTaskExecutorPreference(TaskExecutorRef::undefined());
}

TEST_F(CompatibilityOverrideConcurrencyTest, task_popTaskExecutorPreference) {
  language_task_popTaskExecutorPreference(nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_task_cancel) {
  language_task_cancel(nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_task_cancel_group_child_tasks) {
  language_task_cancel_group_child_tasks(nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_task_escalate) {
  language_task_escalate(nullptr, {});
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_startOnMainActorImpl) {
  language_task_startOnMainActor(nullptr);
}

TEST_F(CompatibilityOverrideConcurrencyTest, test_language_immediately) {
  language_task_immediate(nullptr, SerialExecutorRef::generic());
}

TEST_F(CompatibilityOverrideConcurrencyTest,
       test_language_task_isCurrentExecutorWithFlags) {
  language_task_isCurrentExecutorWithFlags(
      language_task_getMainExecutor(), language_task_is_current_executor_flag::None);
}

#if RUN_ASYNC_MAIN_DRAIN_QUEUE_TEST
TEST_F(CompatibilityOverrideConcurrencyTest, test_language_task_asyncMainDrainQueue) {

  auto runner = [](void *) -> void * {
    language_task_asyncMainDrainQueue();
    return nullptr;
  };

  int ret = 0;
  pthread_t thread;
  pthread_attr_t attrs;
  ret = pthread_attr_init(&attrs);
  ASSERT_EQ(ret, 0);
  ret = pthread_create(&thread, &attrs, runner, nullptr);
  ASSERT_EQ(ret, 0);
  void * result = nullptr;
  ret = pthread_join(thread, &result);
  ASSERT_EQ(ret, 0);
  pthread_attr_destroy(&attrs);
  ASSERT_EQ(ret, 0);
}
#endif

#endif
