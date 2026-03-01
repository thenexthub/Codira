/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_MEM_GC_G1_G1_ANALYTICS_H
#define PANDA_MEM_GC_G1_G1_ANALYTICS_H

#include <atomic>
#include "runtime/include/gc_task.h"
#include "runtime/mem/gc/g1/g1_predictions.h"
#include "runtime/mem/gc/g1/collection_set.h"

namespace ark::mem {
class G1Analytics {
public:
    explicit G1Analytics(uint64_t now);

    void ReportCollectionStart(uint64_t time);
    void ReportCollectionEnd(GCTaskCause cause, uint64_t endTime, const CollectionSet &collectionSet,
                             bool singlePassCompactionEnabled, bool dump = false);

    void ReportEvacuatedBytes(size_t bytes);
    void ReportRemsetSize(size_t remsetSize, size_t remsetRefsCount);
    void ReportScanDirtyCardsStart(uint64_t time);
    void ReportScanDirtyCardsEnd(uint64_t time, size_t dirtyCardsCount);
    void ReportMarkingStart(uint64_t time);
    void ReportMarkingEnd(uint64_t time, size_t remsetRefsCount);
    void ReportEvacuationStart(uint64_t time);
    void ReportEvacuationEnd(uint64_t time);
    void ReportUpdateRefsStart(uint64_t time);
    void ReportUpdateRefsEnd(uint64_t time);
    void ReportPromotedRegion();
    void ReportLiveObjects(size_t num);
    void ReportSurvivedBytesRatio(const CollectionSet &collectionSet);
    void ReportScanRemsetTime(size_t remsetSize, uint64_t time);
    double PredictAllocationRate() const;
    uint64_t PredictYoungCollectionTimeInMicros(size_t edenLength) const;
    uint64_t PredictYoungCollectionTimeInMicros(const CollectionSet &collectionSet) const;
    uint64_t PredictOldCollectionTimeInMicros(size_t remsetSize, size_t liveBytes, size_t liveObjects) const;
    uint64_t PredictOldCollectionTimeInMicros(Region *region) const;
    uint64_t PredictScanDirtyCardsTime(size_t dirtyCardsCount) const;
    double PredictSurvivedBytesRatio() const;

    void ReportPredictedMixedPause(uint64_t time)
    {
        predictedMixedPause_ = time;
    }

private:
    enum StatType { MARKING_COLLECTION = 0, SINGLE_PASS_COMPACTION, COUNT };
    size_t GetPromotedRegions() const;
    size_t GetEvacuatedBytes() const;
    double PredictPromotedRegions(size_t edenLength) const;
    uint64_t EstimatePromotionTimeInMicros(size_t promotedRegions) const;
    uint64_t PredictUpdateRefsTimeInMicros(size_t liveObjects, size_t remsetRefsCount) const;
    uint64_t PredictMarkingTimeInMicros(size_t liveObjects, size_t remsetRefsCount) const;
    uint64_t PredictCopyingTimeInMicros(size_t copiedBytes) const;
    size_t PredictRemsetRefsCount(size_t remsetSize) const;
    uint64_t PredictRemsetScanTimeInMicros(size_t edenLength) const;
    uint64_t PredictYoungSinglePassCompactionTimeInMicros(size_t edenLength) const;
    uint64_t PredictYoungMarkingCollectionTimeInMicros(size_t edenLength, size_t expectedRemsetRefsCount) const;
    uint64_t PredictOtherTime(StatType type) const;

    uint64_t PredictTime(size_t volume, ark::Sequence rateSeq) const
    {
        auto ratePrediction = predictor_.Predict(rateSeq);
        return ratePrediction == 0 ? 0 : volume / ratePrediction;
    }

    void DumpMetrics(const CollectionSet &collectionSet, uint64_t pauseTime, double allocationRate) const;
    void DumpSinglePassCompactionMetrics(size_t edenLength, uint64_t pauseTime) const;
    void DumpMarkingCollectionMetrics(size_t edenLength, uint64_t pauseTime) const;
    void ReportMarkingCollectionEnd(GCTaskCause gcCause, uint64_t pauseTime, size_t edenLength);
    void ReportSinglePassCompactionEnd(GCTaskCause gcCause, uint64_t pauseTime, size_t edenLength);
    void UpdateCopiedBytesStat(size_t compactedRegions);
    void UpdateCopiedBytesRateStat(uint64_t compactionTime);

    static constexpr uint64_t DEFAULT_PROMOTION_COST = 50;
    const uint64_t promotionCost_ {DEFAULT_PROMOTION_COST};
    uint64_t previousYoungCollectionEnd_;
    uint64_t currentYoungCollectionStart_ {0};
    size_t remsetSize_ {0};
    size_t remsetRefsCount_ {0};
    size_t totalRemsetRefsCount_ {0};
    size_t dirtyCardsCount_ {0};
    uint64_t markingStart_ {0};
    uint64_t markingEnd_ {0};
    uint64_t evacuationStart_ {0};
    uint64_t evacuationEnd_ {0};
    uint64_t updateRefsStart_ {0};
    uint64_t updateRefsEnd_ {0};
    uint64_t scanDirtyCardsStart_ {0};
    uint64_t scanDirtyCardsEnd_ {0};
    uint64_t predictedMixedPause_ {0};
    uint64_t scanRemsetTime_ {0};
    ark::Sequence allocationRateSeq_;
    ark::Sequence copiedBytesSeq_;
    ark::Sequence copyingBytesRateSeq_;
    ark::Sequence remsetRefsSeq_;
    ark::Sequence remsetRefsPerChunkSeq_;
    ark::Sequence liveObjecstSeq_;
    ark::Sequence markingRateSeq_;
    ark::Sequence updateRefsRateSeq_;
    ark::Sequence promotionSeq_;
    std::array<ark::Sequence, StatType::COUNT> otherSeq_;
    ark::Sequence liveObjectsSeq_;
    ark::Sequence scanDirtyCardsRateSeq_;
    ark::Sequence survivedBytesRatioSeq_;
    ark::Sequence remsetSizeSeq_;
    ark::Sequence remsetScanRateSeq_;
    static constexpr double DEFAULT_CONFIDENCE_FACTOR = 0.5;
    G1Predictor predictor_ {DEFAULT_CONFIDENCE_FACTOR};
    std::atomic<size_t> copiedBytes_ {0};
    std::atomic<size_t> promotedRegions_ {0};
    std::atomic<size_t> liveObjects_ {0};
    bool previousWasSinglePassCompaction_ {false};

    friend class G1GCTest;
};
}  // namespace ark::mem
#endif
