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
#include "libarkbase/utils/utils.h"
#include "runtime/mem/gc/g1/g1_analytics.h"
#include "runtime/include/runtime_options.h"
#include "runtime/include/runtime.h"

namespace ark::mem {
class G1AnalyticsTest : public testing::Test {
public:
    G1AnalyticsTest()
    {
        RuntimeOptions options;
        options.SetHeapSizeLimit(HEAP_SIZE);
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetGcType("epsilon");
        Runtime::Create(options);
        for (size_t i = 0; i < ALL_REGIONS_NUM; i++) {
            regions_.emplace_back(nullptr, 0, 0);
        }
    }

    ~G1AnalyticsTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(G1AnalyticsTest);
    NO_MOVE_SEMANTIC(G1AnalyticsTest);

    CollectionSet CreateCollectionSet(size_t edenLength, size_t oldLength = 0)
    {
        ASSERT(edenLength + oldLength < ALL_REGIONS_NUM);
        PandaVector<Region *> vector;
        auto it = regions_.begin();
        for (size_t i = 0; i < edenLength; ++it, ++i) {
            vector.push_back(&*it);
        }
        auto cs = CollectionSet(vector);
        for (size_t i = 0; i < oldLength; ++it, ++i) {
            auto &region = *it;
            region.AddFlag(RegionFlag::IS_OLD);
            cs.AddRegion(&region);
        }
        return cs;
    }

    static constexpr uint64_t START_TIME = 1686312829587000000U;

private:
    static constexpr size_t ALL_REGIONS_NUM = 32;
    static constexpr size_t HEAP_SIZE = 64_MB;
    std::list<Region> regions_;
};

static void FillAnalyticsUndefinedBehaviorTest(G1Analytics &analytics, const CollectionSet &collectionSet, uint64_t now)
{
    analytics.ReportCollectionStart(now);
    {
        const size_t remsetSize = 100;
        const size_t remsetRefsCount = 1000;
        analytics.ReportRemsetSize(remsetSize, remsetRefsCount);
    }
    {
        const uint64_t delta = 200'000;
        analytics.ReportScanDirtyCardsStart(now + delta);
    }
    {
        const uint64_t delta = 250'000;
        const size_t dirtyCardsCount = 2;
        analytics.ReportScanDirtyCardsEnd(now + delta, dirtyCardsCount);
    }
    {
        const uint64_t delta = 1'000'000;
        analytics.ReportMarkingStart(now + delta);
    }
    {
        const uint64_t delta = 2'000'000;
        const size_t remsetRefsCount = 10000;
        analytics.ReportMarkingEnd(now + delta, remsetRefsCount);
    }
    {
        const uint64_t delta = 3'000'000;
        analytics.ReportEvacuationStart(now + delta);
    }
    {
        const uint64_t delta = 6'000'000;
        analytics.ReportEvacuationEnd(now + delta);
        analytics.ReportUpdateRefsStart(now + delta);
    }
    {
        const uint64_t delta = 7'000'000;
        analytics.ReportUpdateRefsEnd(now + delta);
    }
    {
        const uint64_t delta = 10'000'000;
        analytics.ReportCollectionEnd(GCTaskCause::YOUNG_GC_CAUSE, now + delta, collectionSet, false);
    }
}

TEST_F(G1AnalyticsTest, UndefinedBehaviorTest)
{
    const size_t edenLength = 16;
    uint64_t now = START_TIME;
    const auto startTimeDelta = -20'000'000;
    auto startTime = now + startTimeDelta;
    G1Analytics analytics(startTime);
    ASSERT_EQ(0, analytics.PredictYoungCollectionTimeInMicros(edenLength));

    auto collectionSet = CreateCollectionSet(edenLength);
    FillAnalyticsUndefinedBehaviorTest(analytics, collectionSet, now);

    const uint64_t expectedTime = 7'000;
    ASSERT_EQ(expectedTime, analytics.PredictYoungCollectionTimeInMicros(edenLength));
}

static void FillAnalyticsPause0AllPromotedUndefinedBehaviorTest(G1Analytics &analytics,
                                                                const CollectionSet &collectionSet, uint64_t now)
{
    analytics.ReportCollectionStart(now);
    {
        const size_t remsetSize = 100;
        const size_t remsetRefsCount = 1000;
        analytics.ReportRemsetSize(remsetSize, remsetRefsCount);
    }
    {
        const uint64_t delta = 200'000;
        analytics.ReportScanDirtyCardsStart(now + delta);
    }
    {
        const uint64_t delta = 250'000;
        const size_t dirtyCardsCount = 2;
        analytics.ReportScanDirtyCardsEnd(now + delta, dirtyCardsCount);
    }
    {
        const uint64_t delta = 1'000'000;
        analytics.ReportMarkingStart(now + delta);
    }
    {
        const uint64_t delta = 2'000'000;
        const size_t remsetRefsCount = 12000;
        analytics.ReportMarkingEnd(now + delta, remsetRefsCount);
    }
    {
        const uint64_t delta = 3'000'000;
        analytics.ReportEvacuationStart(now + delta);
    }
    {
        const uint64_t delta = 6'000'000;
        analytics.ReportEvacuationEnd(now + delta);
        analytics.ReportUpdateRefsStart(now + delta);
    }
    {
        const uint64_t delta = 7'000'000;
        analytics.ReportUpdateRefsEnd(now + delta);
    }
    for (size_t i = 0; i < collectionSet.Young().size(); i++) {
        analytics.ReportPromotedRegion();
    }
    {
        const uint64_t delta = 10'000'000;
        analytics.ReportCollectionEnd(GCTaskCause::YOUNG_GC_CAUSE, now + delta, collectionSet, false);
    }
}

static void FillAnalyticsPause1AllPromotedUndefinedBehaviorTest(G1Analytics &analytics,
                                                                const CollectionSet &collectionSet, uint64_t now)
{
    analytics.ReportCollectionStart(now);
    {
        const size_t liveObjects = 32000;
        analytics.ReportLiveObjects(liveObjects);
    }
    {
        const size_t remsetSize = 100;
        const size_t remsetRefsCount = 1000;
        analytics.ReportRemsetSize(remsetSize, remsetRefsCount);
    }
    {
        const uint64_t delta = 200'000;
        analytics.ReportScanDirtyCardsStart(now + delta);
    }
    {
        const uint64_t delta = 250'000;
        const size_t dirtyCardsCount = 2;
        analytics.ReportScanDirtyCardsEnd(now + delta, dirtyCardsCount);
    }
    {
        const uint64_t delta = 1'000'000;
        analytics.ReportMarkingStart(now + delta);
    }
    {
        const uint64_t delta = 2'000'000;
        const size_t remsetRefsCount = 11000;
        analytics.ReportMarkingEnd(now + delta, remsetRefsCount);
    }
    {
        const uint64_t delta = 3'000'000;
        analytics.ReportEvacuationStart(now + delta);
    }
    {
        const uint64_t delta = 6'000'000;
        analytics.ReportEvacuationEnd(now + delta);
        analytics.ReportUpdateRefsStart(now + delta);
    }
    {
        const uint64_t delta = 7'000'000;
        analytics.ReportUpdateRefsEnd(now + delta);
    }
    {
        const uint64_t delta = 10'000'000;
        analytics.ReportCollectionEnd(GCTaskCause::YOUNG_GC_CAUSE, now + delta, collectionSet, false);
    }
}

TEST_F(G1AnalyticsTest, AllPromotedUndefinedBehaviorTest)
{
    uint64_t now = START_TIME;
    const auto startTimeDelta = -20'000'000;
    auto startTime = now + startTimeDelta;
    const size_t edenLength = 16;
    G1Analytics analytics(startTime);
    ASSERT_EQ(0, analytics.PredictYoungCollectionTimeInMicros(edenLength));

    auto collectionSet = CreateCollectionSet(edenLength);
    FillAnalyticsPause0AllPromotedUndefinedBehaviorTest(analytics, collectionSet, now);
    {
        const uint64_t expectedTime = 7'800;
        ASSERT_EQ(expectedTime, analytics.PredictYoungCollectionTimeInMicros(edenLength));
    }

    const uint64_t nextPauseDelta = 20'000'000;
    now += nextPauseDelta;

    FillAnalyticsPause1AllPromotedUndefinedBehaviorTest(analytics, collectionSet, now);

    {
        const uint64_t expectedTime = 7'732;
        ASSERT_EQ(expectedTime, analytics.PredictYoungCollectionTimeInMicros(edenLength));
    }
}

static void FillAnalyticsPause0PredictionTest(G1Analytics &analytics, const CollectionSet &collectionSet, uint64_t now)
{
    analytics.ReportCollectionStart(now);
    {
        const size_t remsetSize = 100;
        const size_t remsetRefsCount = 1000;
        analytics.ReportRemsetSize(remsetSize, remsetRefsCount);
    }
    {
        const uint64_t delta = 200'000;
        analytics.ReportScanDirtyCardsStart(now + delta);
    }
    {
        const uint64_t delta = 250'000;
        const size_t dirtyCardsCount = 2;
        analytics.ReportScanDirtyCardsEnd(now + delta, dirtyCardsCount);
    }
    {
        const uint64_t delta = 1'000'000;
        analytics.ReportMarkingStart(now + delta);
    }
    {
        const uint64_t delta = 2'000'000;
        const size_t remsetRefsCount = 12000;
        analytics.ReportMarkingEnd(now + delta, remsetRefsCount);
    }
    {
        const uint64_t delta = 3'000'000;
        analytics.ReportEvacuationStart(now + delta);
    }
    {
        const uint64_t delta = 6'000'000;
        analytics.ReportEvacuationEnd(now + delta);
        const size_t evacuatedBytes = 2 * 1024 * 1024;
        analytics.ReportEvacuatedBytes(evacuatedBytes);
        const size_t liveObjects = 32000;
        analytics.ReportLiveObjects(liveObjects);
    }
    {
        const uint64_t delta = 6'000'000;
        analytics.ReportUpdateRefsStart(now + delta);
    }
    {
        const uint64_t delta = 7'000'000;
        analytics.ReportUpdateRefsEnd(now + delta);
    }
    {
        const uint64_t delta = 10'000'000;
        analytics.ReportCollectionEnd(GCTaskCause::YOUNG_GC_CAUSE, now + delta, collectionSet, false);
    }
}

static void FillAnalyticsPause1PredictionTest(G1Analytics &analytics, const CollectionSet &collectionSet, uint64_t now)
{
    analytics.ReportCollectionStart(now);

    {
        const size_t remsetSize = 110;
        const size_t remsetRefsCount = 1500;
        analytics.ReportRemsetSize(remsetSize, remsetRefsCount);
    }
    {
        const uint64_t delta = 900'000;
        analytics.ReportScanDirtyCardsStart(now + delta);
    }
    {
        const uint64_t delta = 950'000;
        const size_t dirtyCardsCount = 2;
        analytics.ReportScanDirtyCardsEnd(now + delta, dirtyCardsCount);
    }
    {
        const uint64_t delta = 1'000'000;
        analytics.ReportMarkingStart(now + delta);
    }
    {
        const uint64_t delta = 2'500'000;
        const size_t remsetRefsCount = 12000;
        analytics.ReportMarkingEnd(now + delta, remsetRefsCount);
    }
    {
        const uint64_t delta = 3'000'000;
        analytics.ReportEvacuationStart(now + delta);
    }
    {
        const uint64_t delta = 6'500'000;
        analytics.ReportEvacuationEnd(now + delta);
        const size_t evacuatedBytes = 2 * 1024 * 1024;
        analytics.ReportEvacuatedBytes(evacuatedBytes);
        analytics.ReportPromotedRegion();
        analytics.ReportPromotedRegion();
        const size_t liveObjects = 30000;
        analytics.ReportLiveObjects(liveObjects);
    }
    {
        const uint64_t delta = 6'000'000;
        analytics.ReportUpdateRefsStart(now + delta);
    }
    {
        const uint64_t delta = 7'500'000;
        analytics.ReportUpdateRefsEnd(now + delta);
    }
    const uint64_t delta = 10'000'000;
    analytics.ReportCollectionEnd(GCTaskCause::YOUNG_GC_CAUSE, now + delta, collectionSet, false);
}

static void FillAnalyticsPause2PredictionTest(G1Analytics &analytics, const CollectionSet &collectionSet, uint64_t now)
{
    analytics.ReportCollectionStart(now);
    {
        const size_t remsetSize = 120;
        const size_t remsetRefsCount = 1600;
        analytics.ReportRemsetSize(remsetSize, remsetRefsCount);
    }
    {
        const uint64_t delta = 900'000;
        analytics.ReportScanDirtyCardsStart(now + delta);
    }
    {
        const uint64_t delta = 950'000;
        const size_t dirtyCardsCount = 2;
        analytics.ReportScanDirtyCardsEnd(now + delta, dirtyCardsCount);
    }
    {
        const uint64_t delta = 1'000'000;
        analytics.ReportMarkingStart(now + delta);
    }
    {
        const uint64_t delta = 3'000'000;
        const size_t remsetRefsCount = 13000;
        analytics.ReportMarkingEnd(now + delta, remsetRefsCount);
    }
    {
        const uint64_t delta = 3'000'000;
        analytics.ReportEvacuationStart(now + delta);
    }
    {
        const uint64_t delta = 7'000'000;
        analytics.ReportEvacuationEnd(now + delta);
        const size_t evacuatedBytes = 2 * 1024 * 1024;
        analytics.ReportEvacuatedBytes(evacuatedBytes);
        analytics.ReportPromotedRegion();
        const size_t liveObjects = 33000;
        analytics.ReportLiveObjects(liveObjects);
    }
    {
        const uint64_t delta = 7'000'000;
        analytics.ReportUpdateRefsStart(now + delta);
    }
    {
        const uint64_t delta = 8'500'000;
        analytics.ReportUpdateRefsEnd(now + delta);
    }
    const uint64_t delta = 11'000'000;
    analytics.ReportCollectionEnd(GCTaskCause::YOUNG_GC_CAUSE, now + delta, collectionSet, false);
}

TEST_F(G1AnalyticsTest, PredictionTest)
{
    uint64_t now = START_TIME;
    const auto startTimeDelta = -20'000'000;
    auto startTime = now + startTimeDelta;
    G1Analytics analytics(startTime);

    {
        const size_t edenLength = 16;
        auto collectionSet = CreateCollectionSet(edenLength);
        FillAnalyticsPause0PredictionTest(analytics, collectionSet, now);

        const size_t expectedTime = 10'000;
        ASSERT_EQ(expectedTime, analytics.PredictYoungCollectionTimeInMicros(edenLength));
        const double expectedAllocationRate = 0.0008;
        const double maxError = 1e-6;
        ASSERT_NEAR(expectedAllocationRate, analytics.PredictAllocationRate(), maxError);
    }

    const uint64_t nextPauseDelta = 20'000'000;
    now += nextPauseDelta;

    {
        const size_t edenLength = 10;
        auto collectionSet = CreateCollectionSet(edenLength);
        FillAnalyticsPause1PredictionTest(analytics, collectionSet, now);

        const uint64_t expectedTime = 9'266;
        ASSERT_EQ(expectedTime, analytics.PredictYoungCollectionTimeInMicros(edenLength));
        const double expectedAllocationRate = 0.000905;
        const double maxError = 1e-6;
        ASSERT_NEAR(expectedAllocationRate, analytics.PredictAllocationRate(), maxError);
    }

    now += nextPauseDelta;

    const size_t edenLength = 14;
    const size_t oldLength = 10;
    auto collectionSet = CreateCollectionSet(edenLength, oldLength);
    FillAnalyticsPause2PredictionTest(analytics, collectionSet, now);

    const uint64_t expectedTime = 10'772;
    ASSERT_EQ(expectedTime, analytics.PredictYoungCollectionTimeInMicros(edenLength));
    const uint64_t expectedOldTime = 110;
    const size_t remsetSize = 50;
    const size_t liveBytes = 64 * 1024;
    const size_t liveObjects = 100;
    ASSERT_EQ(expectedOldTime, analytics.PredictOldCollectionTimeInMicros(remsetSize, liveBytes, liveObjects));
    const double expectedAllocationRate = 0.001151;
    const double maxError = 1e-6;
    ASSERT_NEAR(expectedAllocationRate, analytics.PredictAllocationRate(), maxError);
}

TEST_F(G1AnalyticsTest, SinglePassCompactionPredictionTest)
{
    uint64_t now = START_TIME;
    const auto startTimeDelta = -20'000'000;
    auto startTime = now + startTimeDelta;
    const uint64_t nextPauseDelta = 20'000'000;
    G1Analytics analytics(startTime);

    {
        const size_t edenLength = 16;
        auto collectionSet = CreateCollectionSet(edenLength);
        analytics.ReportCollectionStart(now);

        const size_t remsetSize = 100;
        const uint64_t scanRemsetTime = 2'000'000;
        analytics.ReportScanRemsetTime(remsetSize, scanRemsetTime);
        analytics.ReportEvacuationStart(now);
        const uint64_t evacuationDuration = 7'000'000;
        analytics.ReportEvacuationEnd(now + evacuationDuration);
        const size_t evacuatedBytes = 2 * 1024 * 1024;
        analytics.ReportEvacuatedBytes(evacuatedBytes);

        const uint64_t pauseDuration = 10'000'000;
        analytics.ReportCollectionEnd(GCTaskCause::YOUNG_GC_CAUSE, now + pauseDuration, collectionSet, true);

        const uint64_t expectedTime = 9'999;
        ASSERT_EQ(expectedTime, analytics.PredictYoungCollectionTimeInMicros(edenLength));
    }

    now += nextPauseDelta;

    {
        const size_t edenLength = 20;
        auto collectionSet = CreateCollectionSet(edenLength);
        analytics.ReportCollectionStart(now);

        const size_t remsetSize = 80;
        const uint64_t scanRemsetTime = 1'000'000;
        analytics.ReportScanRemsetTime(remsetSize, scanRemsetTime);
        analytics.ReportEvacuationStart(now);
        const uint64_t evacuationDuration = 6'000'000;
        analytics.ReportEvacuationEnd(now + evacuationDuration);
        const size_t evacuatedBytes = 2 * 1024 * 1024;
        analytics.ReportEvacuatedBytes(evacuatedBytes);

        const uint64_t pauseDuration = 8'000'000;
        analytics.ReportCollectionEnd(GCTaskCause::YOUNG_GC_CAUSE, now + pauseDuration, collectionSet, true);

        const uint64_t expectedTime = 10'762;
        ASSERT_EQ(expectedTime, analytics.PredictYoungCollectionTimeInMicros(edenLength));
    }
}
}  // namespace ark::mem
