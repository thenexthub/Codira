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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <random>
#include <thread>

#include "libarkbase/utils/utils.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/runtime.h"
#include "runtime/include/time_utils.h"

#ifndef PANDA_NIGHTLY_TEST_ON
constexpr size_t ITERATION = 64;
#else
constexpr size_t ITERATION = 1024;
#endif

namespace ark::time::test {

class TimeTest : public testing::Test {
public:
    TimeTest()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        Runtime::Create(options);
        thread_ = ark::MTManagedThread::GetCurrent();
        thread_->ManagedCodeBegin();
    }

    ~TimeTest() override
    {
        thread_->ManagedCodeEnd();
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(TimeTest);
    NO_MOVE_SEMANTIC(TimeTest);

private:
    ark::MTManagedThread *thread_;
};

TEST_F(TimeTest, TimerTest)
{
    uint64_t duration = 0;
    {
        Timer timer(&duration);
        // NOLINTNEXTLINE(readability-magic-numbers)
        std::this_thread::sleep_for(std::chrono::nanoseconds(10_I));
    }
    ASSERT_GT(duration, 0);

    uint64_t lastDuration = duration;
    {
        Timer timer(&duration);
        // NOLINTNEXTLINE(readability-magic-numbers)
        std::this_thread::sleep_for(std::chrono::nanoseconds(10_I));
    }
    ASSERT_GT(duration, lastDuration);

    {
        Timer timer(&duration, true);
        ASSERT_EQ(duration, 0);
    }
    // There is some nondeterminism in sleep, moreover with small values
    // and check (duration_ < last_duration_) may fail
    ASSERT_GT(duration, 0);
}

TEST_F(TimeTest, CurrentTimeStringTest)
{
    constexpr auto PATTERN =
        "(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) [0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{3}";
    for (size_t i = 0; i < ITERATION; i++) {
        auto date = GetCurrentTimeString();
        ASSERT_EQ(date.size(), 19U);
        ASSERT_THAT(date.c_str(), ::testing::MatchesRegex(PATTERN));
        // NOLINTNEXTLINE(readability-magic-numbers)
        std::this_thread::sleep_for(std::chrono::milliseconds(10_I));
    }
}

}  // namespace ark::time::test
