/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "libarkbase/utils/time.h"
#include "runtime/mem/gc/g1/g1_pause_tracker.h"

namespace ark::mem {
class G1PauseTrackerTest : public testing::Test {
public:
    G1PauseTrackerTest() : nowUs_(ark::time::GetCurrentTimeInMillis()) {}

    int64_t Now()
    {
        return nowUs_;
    }

    bool AddPauseTime(G1PauseTracker &pauseTracker, int64_t pauseTimeUs)
    {
        auto result = pauseTracker.AddPause(nowUs_, nowUs_ + pauseTimeUs);
        AddTime(pauseTimeUs);
        return result;
    }

    void AddTime(int64_t timeUs)
    {
        nowUs_ += timeUs;
    }

private:
    int64_t nowUs_;
};

// NOLINTBEGIN(readability-magic-numbers)
TEST_F(G1PauseTrackerTest, MaxGcPauseRequiresNewTimeSlice)
{
    G1PauseTracker pauseTracker(20L, 10L);
    AddPauseTime(pauseTracker, 1'000L);
    ASSERT_EQ(10'000L, pauseTracker.MinDelayBeforePauseInMicros(Now(), 10'000L));
    ASSERT_EQ(10'000L, pauseTracker.MinDelayBeforeMaxPauseInMicros(Now()));
    AddTime(4'000L);
    ASSERT_EQ(6'000L, pauseTracker.MinDelayBeforePauseInMicros(Now(), 10'000L));
    ASSERT_EQ(6'000L, pauseTracker.MinDelayBeforeMaxPauseInMicros(Now()));
}

TEST_F(G1PauseTrackerTest, MinDelayBeforePauseInMicrosQueueIsFull)
{
    G1PauseTracker pauseTracker(20L, 10L);
    AddPauseTime(pauseTracker, 1'000L);
    AddTime(2'000L);
    AddPauseTime(pauseTracker, 2'000L);
    AddTime(5'000L);
    AddPauseTime(pauseTracker, 1'000L);
    AddTime(5'000L);
    AddPauseTime(pauseTracker, 3'000L);
    AddTime(1'000L);
    ASSERT_EQ(0, pauseTracker.MinDelayBeforePauseInMicros(Now(), 6'000L));
    ASSERT_EQ(4'000L, pauseTracker.MinDelayBeforePauseInMicros(Now(), 7'000L));
}

TEST_F(G1PauseTrackerTest, MinDelayBeforePauseInMicrosQueueIsNotFull)
{
    G1PauseTracker pauseTracker(20L, 10L);
    for (size_t i = 0; i < 16U; i++) {
        ASSERT_TRUE(AddPauseTime(pauseTracker, 1'000L));
        AddTime(1'000L);
    }
    ASSERT_EQ(1'000L, pauseTracker.MinDelayBeforePauseInMicros(Now(), 2'000L));
    ASSERT_EQ(2'000L, pauseTracker.MinDelayBeforePauseInMicros(Now(), 3'000L));
    ASSERT_EQ(9'000L, pauseTracker.MinDelayBeforePauseInMicros(Now(), 10'000L));
    ASSERT_EQ(4'000L, pauseTracker.MinDelayBeforePauseInMicros(Now() + 5'000L, 10'000L));
    ASSERT_EQ(9'000L, pauseTracker.MinDelayBeforeMaxPauseInMicros(Now()));
    ASSERT_EQ(4'000L, pauseTracker.MinDelayBeforeMaxPauseInMicros(Now() + 5'000L));
}

TEST_F(G1PauseTrackerTest, ExceedMaxGcTime)
{
    G1PauseTracker pauseTracker(20L, 10L);
    ASSERT_TRUE(AddPauseTime(pauseTracker, 3'000L));
    AddTime(3'000L);
    ASSERT_TRUE(AddPauseTime(pauseTracker, 3'000L));
    AddTime(3'000L);
    ASSERT_TRUE(AddPauseTime(pauseTracker, 4'000L));
    AddTime(3'999L);
    ASSERT_FALSE(AddPauseTime(pauseTracker, 1));
}

TEST_F(G1PauseTrackerTest, NotExceedMaxGcTime)
{
    G1PauseTracker pauseTracker(20L, 10L);
    ASSERT_TRUE(AddPauseTime(pauseTracker, 3'000L));
    AddTime(3'000L);
    ASSERT_TRUE(AddPauseTime(pauseTracker, 3'000L));
    AddTime(3'000L);
    ASSERT_TRUE(AddPauseTime(pauseTracker, 4'000L));
    AddTime(4'000L);
    ASSERT_TRUE(AddPauseTime(pauseTracker, 1));
}
// NOLINTEND(readability-magic-numbers)
}  // namespace ark::mem
