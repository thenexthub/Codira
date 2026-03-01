/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "assembly-parser.h"
#include "runtime/compiler_queue_aged_counter_priority.h"
#include "runtime/compiler_queue_counter_priority.h"
#include "runtime/include/class-inl.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"

namespace ark::test {

class CompilerQueueTest : public testing::Test {
public:
    CompilerQueueTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~CompilerQueueTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(CompilerQueueTest);
    NO_MOVE_SEMANTIC(CompilerQueueTest);

protected:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    ark::MTManagedThread *thread_;
};

static Class *TestClassPrepare()
{
    auto source = R"(
        .function i32 g() {
            ldai 0
            return
        }

        .function i32 f() {
            ldai 0
            return
        }

        .function void main() {
            call f
            return.void
        }
    )";
    pandasm::Parser p;

    auto res = p.Parse(source);
    auto pf = pandasm::AsmEmitter::Emit(res.Value());

    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    classLinker->AddPandaFile(std::move(pf));

    PandaString descriptor;

    Class *klass = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                       ->GetClass(ClassHelper::GetDescriptor(utf::CStringAsMutf8("_GLOBAL"), &descriptor));
    return klass;
}

// The methods may be added to the queue not simultaneously
// So, when we try to get the first method, it may be expired (very rarely),
// But the two last are still present
static void GetAndCheckMethodsIfExists(CompilerQueueInterface *queue, Method *target1, Method *target2, Method *target3)
{
    auto method1 = queue->GetTask().GetMethod();
    if (method1 == nullptr) {
        return;
    }
    auto method2 = queue->GetTask().GetMethod();
    if (method2 == nullptr) {
        ASSERT_EQ(method1, target3);
        return;
    }
    auto method3 = queue->GetTask().GetMethod();
    if (method3 == nullptr) {
        ASSERT_EQ(method1, target2);
        ASSERT_EQ(method2, target3);
        return;
    }
    ASSERT_EQ(method1, target1);
    ASSERT_EQ(method2, target2);
    ASSERT_EQ(method3, target3);
}

static void WaitForExpire(uint millis)
{
    constexpr uint DELTA = 10;
    uint64_t startTime = time::GetCurrentTimeInMillis();
    std::this_thread::sleep_for(std::chrono::milliseconds(millis));
    // sleep_for() works nondeterministically
    // use an additional check for more confidence
    // Note, the queue implementation uses GetCurrentTimeInMillis
    // to update aged counter
    while (time::GetCurrentTimeInMillis() < startTime + millis) {
        std::this_thread::sleep_for(std::chrono::milliseconds(DELTA));
    }
}

// Testing of CounterQueue

TEST_F(CompilerQueueTest, AddGet)
{
    Class *klass = TestClassPrepare();

    Method *mainMethod = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(mainMethod, nullptr);

    Method *fMethod = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(fMethod, nullptr);

    Method *gMethod = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(gMethod, nullptr);

    // Manual range
    mainMethod->SetHotnessCounter(3U);
    fMethod->SetHotnessCounter(2U);
    gMethod->SetHotnessCounter(1U);

    RuntimeOptions options;
    CompilerPriorityCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                       options.GetCompilerQueueMaxLength(), options.GetCompilerTaskLifeSpan());
    queue.AddTask(CompilerTask {mainMethod, false});
    queue.AddTask(CompilerTask {fMethod, false});
    queue.AddTask(CompilerTask {gMethod, false});

    GetAndCheckMethodsIfExists(&queue, gMethod, fMethod, mainMethod);
}

TEST_F(CompilerQueueTest, EqualCounters)
{
    Class *klass = TestClassPrepare();

    Method *mainMethod = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(mainMethod, nullptr);

    Method *fMethod = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(fMethod, nullptr);

    Method *gMethod = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(gMethod, nullptr);

    // Manual range
    mainMethod->SetHotnessCounter(3U);
    fMethod->SetHotnessCounter(3U);
    gMethod->SetHotnessCounter(3U);

    RuntimeOptions options;
    CompilerPriorityCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                       options.GetCompilerQueueMaxLength(), options.GetCompilerTaskLifeSpan());

    queue.AddTask(CompilerTask {fMethod, false});
    queue.AddTask(CompilerTask {gMethod, false});
    queue.AddTask(CompilerTask {mainMethod, false});

    GetAndCheckMethodsIfExists(&queue, fMethod, gMethod, mainMethod);
}

// NOLINTBEGIN(readability-magic-numbers)

TEST_F(CompilerQueueTest, Expire)
{
    auto klass = TestClassPrepare();

    Method *mainMethod = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(mainMethod, nullptr);

    Method *fMethod = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(fMethod, nullptr);

    Method *gMethod = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(gMethod, nullptr);

    RuntimeOptions options;
    constexpr int COMPILER_TASK_LIFE_SPAN1 = 500;
    CompilerPriorityCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                       options.GetCompilerQueueMaxLength(), COMPILER_TASK_LIFE_SPAN1);
    queue.AddTask(CompilerTask {mainMethod, false});
    queue.AddTask(CompilerTask {fMethod, false});
    queue.AddTask(CompilerTask {gMethod, false});

    WaitForExpire(1000U);

    // All tasks should expire after sleep
    auto method = queue.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);

    constexpr int COMPILER_TASK_LIFE_SPAN2 = 0;
    CompilerPriorityCounterQueue queue2(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                        options.GetCompilerQueueMaxLength(), COMPILER_TASK_LIFE_SPAN2);
    queue2.AddTask(CompilerTask {mainMethod, false});
    queue2.AddTask(CompilerTask {fMethod, false});
    queue2.AddTask(CompilerTask {gMethod, false});

    // All tasks should expire without sleep
    method = queue2.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);
}

TEST_F(CompilerQueueTest, Reorder)
{
    auto klass = TestClassPrepare();

    Method *mainMethod = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(mainMethod, nullptr);

    Method *fMethod = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(fMethod, nullptr);

    Method *gMethod = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(gMethod, nullptr);

    mainMethod->SetHotnessCounter(3U);
    fMethod->SetHotnessCounter(2U);
    gMethod->SetHotnessCounter(1U);

    RuntimeOptions options;
    CompilerPriorityCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                       options.GetCompilerQueueMaxLength(), options.GetCompilerTaskLifeSpan());

    // It is possible, that the first added method is expired, and others are not
    // So, add to queue in reversed order to be sure, that the first method is present anyway
    queue.AddTask(CompilerTask {gMethod, false});
    queue.AddTask(CompilerTask {fMethod, false});
    queue.AddTask(CompilerTask {mainMethod, false});

    // Change the order
    mainMethod->SetHotnessCounter(-6U);
    fMethod->SetHotnessCounter(-5U);
    gMethod->SetHotnessCounter(-4U);

    GetAndCheckMethodsIfExists(&queue, mainMethod, fMethod, gMethod);
}

TEST_F(CompilerQueueTest, MaxLimit)
{
    auto klass = TestClassPrepare();

    Method *mainMethod = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(mainMethod, nullptr);

    Method *fMethod = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(fMethod, nullptr);

    Method *gMethod = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(gMethod, nullptr);

    mainMethod->SetHotnessCounter(1U);
    fMethod->SetHotnessCounter(2U);
    gMethod->SetHotnessCounter(3U);

    RuntimeOptions options;
    constexpr int COMPILER_QUEUE_MAX_LENGTH1 = 100;
    CompilerPriorityCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                       COMPILER_QUEUE_MAX_LENGTH1, options.GetCompilerTaskLifeSpan());

    for (size_t i = 0; i < 40U; i++) {
        queue.AddTask(CompilerTask {mainMethod, false});
        queue.AddTask(CompilerTask {fMethod, false});
        queue.AddTask(CompilerTask {gMethod, false});
    }

    // 100 as Max_Limit
    for (size_t i = 0; i < 100U; i++) {
        queue.GetTask();
        // Can not check as the task may expire on an overloaded machine
    }

    auto method = queue.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);

    // check an option
    constexpr int COMPILER_QUEUE_MAX_LENGTH2 = 1;
    CompilerPriorityCounterQueue queue2(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                        COMPILER_QUEUE_MAX_LENGTH2, options.GetCompilerTaskLifeSpan());

    queue2.AddTask(CompilerTask {mainMethod, false});
    queue2.AddTask(CompilerTask {fMethod, false});
    queue2.AddTask(CompilerTask {gMethod, false});

    queue2.GetTask().GetMethod();
    method = queue2.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);
}

// Testing of AgedCounterQueue

TEST_F(CompilerQueueTest, AgedAddGet)
{
    Class *klass = TestClassPrepare();

    Method *mainMethod = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(mainMethod, nullptr);

    Method *fMethod = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(fMethod, nullptr);

    Method *gMethod = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(gMethod, nullptr);

    // Manual range
    mainMethod->SetHotnessCounter(-1000U);
    fMethod->SetHotnessCounter(-1200U);
    gMethod->SetHotnessCounter(-1300U);

    RuntimeOptions options;
    CompilerPriorityAgedCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                           options.GetCompilerQueueMaxLength(), options.GetCompilerDeathCounterValue(),
                                           options.GetCompilerEpochDuration());
    queue.AddTask(CompilerTask {mainMethod, false});
    queue.AddTask(CompilerTask {fMethod, false});
    queue.AddTask(CompilerTask {gMethod, false});

    GetAndCheckMethodsIfExists(&queue, gMethod, fMethod, mainMethod);
}

TEST_F(CompilerQueueTest, AgedEqualCounters)
{
    Class *klass = TestClassPrepare();

    Method *mainMethod = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(mainMethod, nullptr);

    Method *gMethod = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(gMethod, nullptr);

    Method *fMethod = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(fMethod, nullptr);

    // Manual range
    mainMethod->SetHotnessCounter(3000U);
    gMethod->SetHotnessCounter(3000U);
    fMethod->SetHotnessCounter(3000U);

    RuntimeOptions options;
    CompilerPriorityAgedCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                           options.GetCompilerQueueMaxLength(), options.GetCompilerDeathCounterValue(),
                                           options.GetCompilerEpochDuration());
    // Add in reversed order, as methods with equal counters will be ordered by timestamp
    queue.AddTask(CompilerTask {fMethod, false});
    queue.AddTask(CompilerTask {gMethod, false});
    queue.AddTask(CompilerTask {mainMethod, false});

    GetAndCheckMethodsIfExists(&queue, fMethod, gMethod, mainMethod);
}

TEST_F(CompilerQueueTest, AgedExpire)
{
    auto klass = TestClassPrepare();

    Method *mainMethod = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(mainMethod, nullptr);

    Method *fMethod = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(fMethod, nullptr);

    Method *gMethod = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(gMethod, nullptr);

    mainMethod->SetHotnessCounter(1000U);
    fMethod->SetHotnessCounter(1000U);
    gMethod->SetHotnessCounter(1000U);

    RuntimeOptions options;
    constexpr int COMPILER_EPOCH_DURATION1 = 500;
    CompilerPriorityAgedCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                           options.GetCompilerQueueMaxLength(), options.GetCompilerDeathCounterValue(),
                                           COMPILER_EPOCH_DURATION1);
    queue.AddTask(CompilerTask {mainMethod, false});
    queue.AddTask(CompilerTask {fMethod, false});
    queue.AddTask(CompilerTask {gMethod, false});

    WaitForExpire(1600U);

    // All tasks should expire after sleep
    auto method = queue.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);

    constexpr int COMPILER_EPOCH_DURATION2 = 1;
    CompilerPriorityAgedCounterQueue queue2(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                            options.GetCompilerQueueMaxLength(), options.GetCompilerDeathCounterValue(),
                                            COMPILER_EPOCH_DURATION2);

    queue2.AddTask(CompilerTask {mainMethod, false});
    queue2.AddTask(CompilerTask {fMethod, false});
    queue2.AddTask(CompilerTask {gMethod, false});

    WaitForExpire(5U);

    method = queue2.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);
}

TEST_F(CompilerQueueTest, AgedReorder)
{
    auto klass = TestClassPrepare();

    Method *mainMethod = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(mainMethod, nullptr);

    Method *fMethod = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(fMethod, nullptr);

    Method *gMethod = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(gMethod, nullptr);

    mainMethod->SetHotnessCounter(-1500U);
    fMethod->SetHotnessCounter(-2000U);
    gMethod->SetHotnessCounter(-3000U);

    RuntimeOptions options;
    CompilerPriorityAgedCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                           options.GetCompilerQueueMaxLength(), options.GetCompilerDeathCounterValue(),
                                           options.GetCompilerEpochDuration());
    // It is possible, that the first added method is expired, and others are not
    // So, add to queue in reversed order to be sure, that the first method is present anyway
    queue.AddTask(CompilerTask {gMethod, false});
    queue.AddTask(CompilerTask {fMethod, false});
    queue.AddTask(CompilerTask {mainMethod, false});

    // Change the order
    mainMethod->SetHotnessCounter(-6000U);
    fMethod->SetHotnessCounter(-5000U);
    gMethod->SetHotnessCounter(-4000U);

    GetAndCheckMethodsIfExists(&queue, mainMethod, fMethod, gMethod);
}

TEST_F(CompilerQueueTest, AgedMaxLimit)
{
    auto klass = TestClassPrepare();

    Method *mainMethod = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(mainMethod, nullptr);

    Method *fMethod = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(fMethod, nullptr);

    Method *gMethod = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(gMethod, nullptr);

    mainMethod->SetHotnessCounter(1000U);
    fMethod->SetHotnessCounter(2000U);
    gMethod->SetHotnessCounter(3000U);

    RuntimeOptions options;
    CompilerPriorityAgedCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                           options.GetCompilerQueueMaxLength(), options.GetCompilerDeathCounterValue(),
                                           options.GetCompilerEpochDuration());

    for (size_t i = 0; i < 40U; i++) {
        queue.AddTask(CompilerTask {mainMethod, false});
        queue.AddTask(CompilerTask {fMethod, false});
        queue.AddTask(CompilerTask {gMethod, false});
    }

    // 100 as Max_Limit
    for (size_t i = 0; i < 100U; i++) {
        queue.GetTask();
        // Can not check as the task may expire on an overloaded machine
    }

    auto method = queue.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);

    // check an option
    constexpr int COMPILER_QUEUE_MAX_LENGTH = 1;
    CompilerPriorityAgedCounterQueue queue2(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                            COMPILER_QUEUE_MAX_LENGTH, options.GetCompilerDeathCounterValue(),
                                            options.GetCompilerEpochDuration());

    queue2.AddTask(CompilerTask {mainMethod, false});
    queue2.AddTask(CompilerTask {fMethod, false});
    queue2.AddTask(CompilerTask {gMethod, false});

    queue2.GetTask().GetMethod();
    method = queue2.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);
}

TEST_F(CompilerQueueTest, AgedDeathCounter)
{
    auto klass = TestClassPrepare();

    Method *mainMethod = klass->GetDirectMethod(utf::CStringAsMutf8("main"));
    ASSERT_NE(mainMethod, nullptr);

    Method *fMethod = klass->GetDirectMethod(utf::CStringAsMutf8("f"));
    ASSERT_NE(fMethod, nullptr);

    Method *gMethod = klass->GetDirectMethod(utf::CStringAsMutf8("g"));
    ASSERT_NE(gMethod, nullptr);

    mainMethod->SetHotnessCounter(10U);
    fMethod->SetHotnessCounter(20U);
    gMethod->SetHotnessCounter(30000U);

    RuntimeOptions options;
    constexpr int COMPILER_DEATH_COUNTER_VALUE = 50;
    CompilerPriorityAgedCounterQueue queue(thread_->GetVM()->GetHeapManager()->GetInternalAllocator(),
                                           options.GetCompilerQueueMaxLength(), COMPILER_DEATH_COUNTER_VALUE,
                                           options.GetCompilerEpochDuration());

    queue.AddTask(CompilerTask {mainMethod, false});
    queue.AddTask(CompilerTask {fMethod, false});
    queue.AddTask(CompilerTask {gMethod, false});

    auto method = queue.GetTask().GetMethod();
    ASSERT_EQ(method, gMethod);
    method = queue.GetTask().GetMethod();
    ASSERT_EQ(method, nullptr);
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::test
