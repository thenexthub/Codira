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
#include <string_view>

#include "gtest/gtest.h"
#include "runtime/include/runtime.h"
#include "libarkbase/test_utilities.h"

namespace ark::test {

class ThreadTest : public testing::Test {
public:
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    MTManagedThread *thread {};
    ThreadTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        /*
         * gtest ASSERT_DEATH doesn't work with multiple threads:
         * "In particular, death tests don't like having multiple threads in the parent process"
         * turn off gc-thread & compiler-thread here, because we use a lot of ASSERT_DEATH and test can hang.
         */
        options.SetCompilerEnableJit(false);
        options.SetGcType("epsilon");
        Logger::InitializeStdLogging(Logger::Level::ERROR, 0);
        Runtime::Create(options);
        thread = MTManagedThread::GetCurrent();
    }

    ~ThreadTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(ThreadTest);
    NO_MOVE_SEMANTIC(ThreadTest);

    void AssertNative() const
    {
        ASSERT_TRUE(thread->IsInNativeCode());
        ASSERT_FALSE(thread->IsManagedCode());
    }

    void AssertManaged() const
    {
        ASSERT_FALSE(thread->IsInNativeCode());
        ASSERT_TRUE(thread->IsManagedCode());
    }

    void BeginToStateAndEnd(MTManagedThread::ThreadState state) const
    {
        if (state == MTManagedThread::ThreadState::NATIVE_CODE) {
            thread->NativeCodeBegin();
            AssertNative();
            thread->NativeCodeEnd();
        } else if (state == MTManagedThread::ThreadState::MANAGED_CODE) {
            thread->ManagedCodeBegin();
            AssertManaged();
            thread->ManagedCodeEnd();
        } else {
            UNREACHABLE();
        }
    }
};

/**
 * call stack:
 * native #0
 *   managed #1
 *      native #2
 *          access #3
 *   access #4
 *
 */
TEST_F(ThreadTest, LegalThreadStatesTest)
{
    AssertNative();
    thread->ManagedCodeBegin();  // #1
    AssertManaged();
    thread->NativeCodeBegin();  // #2
    AssertNative();

    thread->NativeCodeEnd();  // #2
    AssertManaged();
    thread->ManagedCodeEnd();  // #1
    AssertNative();
}

DEATH_TEST_F(ThreadTest, BeginForbiddenStatesFromNativeFrame)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    AssertNative();
#ifndef NDEBUG
    ASSERT_DEATH(thread->NativeCodeBegin(), "last frame is: NATIVE_CODE");
#endif
    AssertNative();
}

DEATH_TEST_F(ThreadTest, BeginForbiddenStatesFromManagedFrame)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    AssertNative();
    thread->ManagedCodeBegin();
    AssertManaged();
#ifndef NDEBUG
    ASSERT_DEATH(thread->ManagedCodeBegin(), "last frame is: MANAGED_CODE");
#endif
    AssertManaged();
    thread->ManagedCodeEnd();
    AssertNative();
}

DEATH_TEST_F(ThreadTest, EndNativeStateByOtherStates)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    AssertNative();

#ifndef NDEBUG
    ASSERT_DEATH(thread->ManagedCodeEnd(), "last frame is: NATIVE_CODE");
    ASSERT_DEATH(thread->ManagedCodeEnd(), "last frame is: NATIVE_CODE");
#endif
}

DEATH_TEST_F(ThreadTest, EndManagedStateByOtherStates)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    AssertNative();
    thread->ManagedCodeBegin();

#ifndef NDEBUG
    ASSERT_DEATH(thread->NativeCodeEnd(), "last frame is: MANAGED_CODE");
#endif
    thread->ManagedCodeEnd();
}

DEATH_TEST_F(ThreadTest, TestAllConversions)
{
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    // from NATIVE_CODE
    AssertNative();
#ifndef NDEBUG
    ASSERT_DEATH(BeginToStateAndEnd(MTManagedThread::ThreadState::NATIVE_CODE), "last frame is: NATIVE_CODE");
#endif
    BeginToStateAndEnd(MTManagedThread::ThreadState::MANAGED_CODE);

    // from MANAGED_CODE
    thread->ManagedCodeBegin();
    AssertManaged();

    BeginToStateAndEnd(MTManagedThread::ThreadState::NATIVE_CODE);
#ifndef NDEBUG
    ASSERT_DEATH(BeginToStateAndEnd(MTManagedThread::ThreadState::MANAGED_CODE), "last frame is: MANAGED_CODE");
#endif
    thread->ManagedCodeEnd();
    AssertNative();
}

std::string GetThreadStatus(ThreadStatus status)
{
    std::string expected;
    switch (status) {
        case ThreadStatus::CREATED:
            expected = "New";
            break;
        case ThreadStatus::RUNNING:
            expected = "Runnable";
            break;
        case ThreadStatus::IS_BLOCKED:
            expected = "Blocked";
            break;
        case ThreadStatus::IS_WAITING:
            expected = "Waiting";
            break;
        case ThreadStatus::IS_TIMED_WAITING:
            expected = "Timed_waiting";
            break;
        case ThreadStatus::IS_SUSPENDED:
            expected = "Suspended";
            break;
        case ThreadStatus::IS_COMPILER_WAITING:
            expected = "Compiler_waiting";
            break;
        case ThreadStatus::IS_WAITING_INFLATION:
            expected = "Waiting_inflation";
            break;
        case ThreadStatus::IS_SLEEPING:
            expected = "Sleeping";
            break;
        case ThreadStatus::IS_TERMINATED_LOOP:
            expected = "Terminated_loop";
            break;
        case ThreadStatus::TERMINATING:
            expected = "Terminating";
            break;
        case ThreadStatus::NATIVE:
            expected = "Native";
            break;
        case ThreadStatus::FINISHED:
            expected = "Terminated";
            break;
        default:
            expected = "unknown";
            break;
    }
    return expected;
}

TEST_F(ThreadTest, ThreadStatusAsStringTest)
{
    int start = static_cast<int>(ThreadStatus::CREATED);
    int end = static_cast<int>(ThreadStatus::FINISHED);
    for (int i = start; i <= end; ++i) {
        auto status = static_cast<ThreadStatus>(i);
        std::string expected = GetThreadStatus(status);
        EXPECT_EQ(std::string_view(ManagedThread::ThreadStatusAsString(status)), std::string_view(expected));
    }
}

}  // namespace ark::test
