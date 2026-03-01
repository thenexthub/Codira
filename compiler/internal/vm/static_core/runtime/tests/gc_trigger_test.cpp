/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */

#include <gtest/gtest.h>
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_trigger.h"
#include "runtime/handle_scope-inl.h"
#include "test_utils.h"

namespace ark::test {
class GCTriggerTest : public testing::Test {
public:
    GCTriggerTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetGcTriggerType("adaptive-heap-trigger");

        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~GCTriggerTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(GCTriggerTest);
    NO_MOVE_SEMANTIC(GCTriggerTest);

    [[nodiscard]] mem::GCAdaptiveTriggerHeap *CreateGCTriggerHeap() const
    {
        return new mem::GCAdaptiveTriggerHeap(nullptr, nullptr, MIN_HEAP_SIZE,
                                              mem::GCTriggerHeap::DEFAULT_PERCENTAGE_THRESHOLD,
                                              DEFAULT_INCREASE_MULTIPLIER, MIN_EXTRA_HEAP_SIZE, MAX_EXTRA_HEAP_SIZE);
    }

    static size_t GetTargetFootprint(const mem::GCAdaptiveTriggerHeap *trigger)
    {
        // Atomic with relaxed order reason: simple getter for test
        return trigger->targetFootprint_.load(std::memory_order_relaxed);
    }

protected:
    static constexpr size_t MIN_HEAP_SIZE = 8_MB;
    static constexpr size_t MIN_EXTRA_HEAP_SIZE = 1_MB;
    static constexpr size_t MAX_EXTRA_HEAP_SIZE = 8_MB;
    static constexpr uint32_t DEFAULT_INCREASE_MULTIPLIER = 3U;

private:
    MTManagedThread *thread_ = nullptr;
};

TEST_F(GCTriggerTest, ThresholdTest)
{
    static constexpr size_t BEFORE_HEAP_SIZE = 50_MB;
    static constexpr size_t CURRENT_HEAP_SIZE = MIN_HEAP_SIZE;
    static constexpr size_t FIRST_THRESHOLD = 2U * MIN_HEAP_SIZE;
    static constexpr size_t HEAP_SIZE_AFTER_BASE_TRIGGER = (BEFORE_HEAP_SIZE + CURRENT_HEAP_SIZE) / 2U;
    auto *trigger = CreateGCTriggerHeap();
    GCTask task(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);

    trigger->ComputeNewTargetFootprint(task, BEFORE_HEAP_SIZE, CURRENT_HEAP_SIZE);

    ASSERT_EQ(GetTargetFootprint(trigger), FIRST_THRESHOLD);

    trigger->ComputeNewTargetFootprint(task, BEFORE_HEAP_SIZE, CURRENT_HEAP_SIZE);
    ASSERT_EQ(GetTargetFootprint(trigger), HEAP_SIZE_AFTER_BASE_TRIGGER);
    trigger->ComputeNewTargetFootprint(task, BEFORE_HEAP_SIZE, CURRENT_HEAP_SIZE);
    ASSERT_EQ(GetTargetFootprint(trigger), HEAP_SIZE_AFTER_BASE_TRIGGER);
    trigger->ComputeNewTargetFootprint(task, BEFORE_HEAP_SIZE, CURRENT_HEAP_SIZE);
    ASSERT_EQ(GetTargetFootprint(trigger), HEAP_SIZE_AFTER_BASE_TRIGGER);
    trigger->ComputeNewTargetFootprint(task, BEFORE_HEAP_SIZE, CURRENT_HEAP_SIZE);

    // Check that we could to avoid locale triggering
    ASSERT_EQ(GetTargetFootprint(trigger), FIRST_THRESHOLD);

    delete trigger;
}

class GCChecker : public mem::GCListener {
public:
    void GCFinished(const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                    [[maybe_unused]] size_t heapSize) override
    {
        reason_ = task.reason;
        counter_++;
    }

    GCTaskCause GetCause() const
    {
        return reason_;
    }

    size_t GetCounter() const
    {
        return counter_;
    }

private:
    GCTaskCause reason_ = GCTaskCause::INVALID_CAUSE;
    size_t counter_ {0};
};

static RuntimeOptions GetRuntimeOptions(const std::string &triggerType)
{
    RuntimeOptions options;
    options.SetShouldLoadBootPandaFiles(false);
    options.SetShouldInitializeIntrinsics(false);
    if (triggerType == "debug-never") {
        options.SetGcTriggerType("debug-never");
        options.SetGcUseNthAllocTrigger(true);
    } else if (triggerType == "pause-time-goal-trigger") {
        options.SetGcTriggerType("pause-time-goal-trigger");
        options.SetRunGcInPlace(true);
        constexpr size_t YOUNG_SIZE = 512 * 1024;
        options.SetYoungSpaceSize(YOUNG_SIZE);
        options.SetInitYoungSpaceSize(YOUNG_SIZE);
        options.SetGcWorkersCount(0);
    } else if (triggerType == "heap-trigger-test") {
        options.SetGcTriggerType("heap-trigger-test");
    } else if (triggerType == "heap-trigger") {
        options.SetGcTriggerType("heap-trigger");
    } else if (triggerType == "adaptive-heap-trigger") {
        options.SetGcTriggerType("adaptive-heap-trigger");
    } else if (triggerType == "no-gc-for-start-up") {
        options.SetGcTriggerType("no-gc-for-start-up");
    } else if (triggerType == "trigger-heap-occupancy") {
        options.SetGcTriggerType("trigger-heap-occupancy");
    } else if (triggerType == "debug") {
        options.SetGcTriggerType("debug");
    } else if (triggerType == "pause-time-goal-trigger") {
        options.SetGcTriggerType("pause-time-goal-trigger");
    } else {
        UNREACHABLE();
    }
    return options;
}

TEST(SchedGCOnNthAllocTriggerTest, TestTrigger)
{
    Runtime::Create(GetRuntimeOptions("debug-never"));
    ManagedThread *thread = ark::ManagedThread::GetCurrent();
    thread->ManagedCodeBegin();
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    PandaVM *vm = Runtime::GetCurrent()->GetPandaVM();
    auto *trigger = vm->GetGCTrigger();
    ASSERT_EQ(mem::GCTriggerType::ON_NTH_ALLOC, trigger->GetType());
    auto *schedTrigger = reinterpret_cast<mem::SchedGCOnNthAllocTrigger *>(trigger);
    GCChecker checker;
    vm->GetGC()->AddListener(&checker);

    schedTrigger->ScheduleGc(GCTaskCause::YOUNG_GC_CAUSE, 2);
    coretypes::String::CreateEmptyString(ctx, vm);
    EXPECT_FALSE(schedTrigger->IsTriggered());
    EXPECT_EQ(GCTaskCause::INVALID_CAUSE, checker.GetCause());
    coretypes::String::CreateEmptyString(ctx, vm);
    EXPECT_EQ(GCTaskCause::YOUNG_GC_CAUSE, checker.GetCause());
    EXPECT_TRUE(schedTrigger->IsTriggered());

    thread->ManagedCodeEnd();
    Runtime::Destroy();
}

TEST(PauseTimeGoalTriggerTest, TestTrigger)
{
    Runtime::Create(GetRuntimeOptions("pause-time-goal-trigger"));
    auto *thread = ark::ManagedThread::GetCurrent();
    {
        ScopedManagedCodeThread s(thread);
        HandleScope<ObjectHeader *> scope(thread);

        auto *runtime = Runtime::GetCurrent();
        auto ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        auto *vm = runtime->GetPandaVM();
        auto *trigger = vm->GetGCTrigger();
        ASSERT_EQ(mem::GCTriggerType::PAUSE_TIME_GOAL_TRIGGER, trigger->GetType());
        GCChecker checker;
        vm->GetGC()->AddListener(&checker);

        auto *pauseTimeGoalTrigger = static_cast<ark::mem::PauseTimeGoalTrigger *>(trigger);
        constexpr size_t INIT_TARGET_FOOTPRINT = 1258291;
        ASSERT_EQ(INIT_TARGET_FOOTPRINT, pauseTimeGoalTrigger->GetTargetFootprint());

        constexpr size_t ARRAY_LENGTH = 5 * 32 * 1024;  // big enough to provoke several collections
        VMHandle<coretypes::String> dummy(thread, coretypes::String::CreateEmptyString(ctx, vm));
        VMHandle<coretypes::Array> array(
            thread, ark::mem::ObjectAllocator::AllocArray(ARRAY_LENGTH, ClassRoot::ARRAY_STRING, false));

        size_t expectedCounter = 1;
        size_t startIdx = 0;
        for (size_t i = 0; i < ARRAY_LENGTH; i++) {
            array->Set(i, coretypes::String::CreateEmptyString(ctx, vm));

            if (expectedCounter == checker.GetCounter()) {
                // objects become garbage
                for (size_t j = startIdx; j < i; j++) {
                    array->Set(j, dummy.GetPtr());
                    startIdx = i;
                }
                if (expectedCounter < 2) {
                    ASSERT_EQ(GCTaskCause::YOUNG_GC_CAUSE, checker.GetCause());
                } else if (expectedCounter == 2) {
                    ASSERT_EQ(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE, checker.GetCause());
                } else if (expectedCounter == 3) {
                    ASSERT_EQ(GCTaskCause::YOUNG_GC_CAUSE, checker.GetCause());
                } else if (expectedCounter > 3) {
                    break;
                }

                expectedCounter++;
            }
        }

        ASSERT_GT(expectedCounter, 3);

        // previous mixed collection should update target footprint
        ASSERT_GT(pauseTimeGoalTrigger->GetTargetFootprint(), INIT_TARGET_FOOTPRINT);
    }
    Runtime::Destroy();
}

TEST(CreateGCTriggerTest, PAUSE_TIME_GOAL_TRIGGER)
{
    Runtime::Create(GetRuntimeOptions("pause-time-goal-trigger"));
    auto *thread = ark::ManagedThread::GetCurrent();
    {
        ScopedManagedCodeThread s(thread);
        HandleScope<ObjectHeader *> scope(thread);

        auto *runtime = Runtime::GetCurrent();
        auto *vm = runtime->GetPandaVM();
        auto *trigger = vm->GetGCTrigger();
        ASSERT_EQ(mem::GCTriggerType::PAUSE_TIME_GOAL_TRIGGER, trigger->GetType());
    }
    Runtime::Destroy();
}

TEST(CreateGCTriggerTest, TRIGGER_HEAP_OCCUPANCY)
{
    Runtime::Create(GetRuntimeOptions("trigger-heap-occupancy"));
    auto *thread = ark::ManagedThread::GetCurrent();
    {
        ScopedManagedCodeThread s(thread);
        HandleScope<ObjectHeader *> scope(thread);

        auto *runtime = Runtime::GetCurrent();
        auto *vm = runtime->GetPandaVM();
        auto *trigger = vm->GetGCTrigger();
        ASSERT_EQ(mem::GCTriggerType::TRIGGER_HEAP_OCCUPANCY, trigger->GetType());
    }
    Runtime::Destroy();
}

TEST(CreateGCTriggerTest, NO_GC_FOR_START_UP)
{
    Runtime::Create(GetRuntimeOptions("no-gc-for-start-up"));
    auto *thread = ark::ManagedThread::GetCurrent();
    {
        ScopedManagedCodeThread s(thread);
        HandleScope<ObjectHeader *> scope(thread);

        auto *runtime = Runtime::GetCurrent();
        auto *vm = runtime->GetPandaVM();
        auto *trigger = vm->GetGCTrigger();
        ASSERT_EQ(mem::GCTriggerType::HEAP_TRIGGER, trigger->GetType());
    }
    Runtime::Destroy();
}

TEST(CreateGCTriggerTest, HEAP_TRIGGER_TEST)
{
    Runtime::Create(GetRuntimeOptions("heap-trigger-test"));
    auto *thread = ark::ManagedThread::GetCurrent();
    {
        ScopedManagedCodeThread s(thread);
        HandleScope<ObjectHeader *> scope(thread);

        auto *runtime = Runtime::GetCurrent();
        auto *vm = runtime->GetPandaVM();
        auto *trigger = vm->GetGCTrigger();
        ASSERT_EQ(mem::GCTriggerType::HEAP_TRIGGER, trigger->GetType());
    }
    Runtime::Destroy();
}
}  // namespace ark::test
