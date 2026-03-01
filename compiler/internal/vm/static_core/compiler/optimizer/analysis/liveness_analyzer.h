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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_LIVENESS_ANALIZER_H
#define COMPILER_OPTIMIZER_ANALYSIS_LIVENESS_ANALIZER_H

#include "libarkbase/utils/arena_containers.h"
#include "optimizer/analysis/liveness_use_table.h"
#include "optimizer/ir/constants.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/marker.h"
#include "optimizer/pass.h"
#include "optimizer/ir/locations.h"
#include "compiler_logger.h"

namespace ark::compiler {
class BasicBlock;
class Graph;
class Inst;
class Loop;

class LiveRange {
public:
    LiveRange(LifeNumber begin, LifeNumber end) : begin_(begin), end_(end) {}
    LiveRange() = default;
    DEFAULT_MOVE_SEMANTIC(LiveRange);
    DEFAULT_COPY_SEMANTIC(LiveRange);
    ~LiveRange() = default;

    // Check if range contains other range
    bool Contains(const LiveRange &other) const
    {
        return (begin_ <= other.begin_ && other.end_ <= end_);
    }
    // Check if range contains point
    bool Contains(LifeNumber number) const
    {
        return (begin_ <= number && number <= end_);
    }
    // Check if ranges are equal
    bool operator==(const LiveRange &other) const
    {
        return begin_ == other.begin_ && end_ == other.end_;
    }

    void SetBegin(LifeNumber begin)
    {
        begin_ = begin;
    }
    LifeNumber GetBegin() const
    {
        return begin_;
    }

    void SetEnd(LifeNumber end)
    {
        end_ = end;
    }
    LifeNumber GetEnd() const
    {
        return end_;
    }

    std::string ToString() const
    {
        std::stringstream ss;
        ss << "[" << begin_ << ":" << end_ << ")";
        return ss.str();
    }

private:
    LifeNumber begin_ = 0;
    LifeNumber end_ = 0;
};

class LifeIntervals {
public:
    explicit LifeIntervals(ArenaAllocator *allocator) : LifeIntervals(allocator, nullptr) {}

    LifeIntervals(ArenaAllocator *allocator, Inst *inst) : LifeIntervals(allocator, inst, {}) {}

    LifeIntervals(ArenaAllocator *allocator, Inst *inst, LiveRange liveRange)
        : inst_(inst),
          liveRanges_(allocator->Adapter()),
          usePositions_(allocator->Adapter()),
          location_(Location::Invalid()),
          type_(DataType::NO_TYPE),
          isPreassigned_(0),
          isPhysical_(0),
          isSplitSibling_(0)
    {
#ifndef NDEBUG
        finalized_ = 0;
#endif
        if (liveRange.GetEnd() != 0) {
            liveRanges_.push_back(liveRange);
        }
    }

    DEFAULT_MOVE_SEMANTIC(LifeIntervals);
    DEFAULT_COPY_SEMANTIC(LifeIntervals);
    ~LifeIntervals() = default;

    /*
     * Basic blocks are visiting in descending lifetime order, so there are 3 ways to
     * update lifetime:
     * - append the first LiveRange
     * - extend the first LiveRange
     * - append a new one LiveRange due to lifetime hole
     */
    void AppendRange(LiveRange liveRange)
    {
        ASSERT(liveRange.GetEnd() >= liveRange.GetBegin());
        // [live_range],[back]
        if (liveRanges_.empty() || liveRange.GetEnd() < liveRanges_.back().GetBegin()) {
            liveRanges_.push_back(liveRange);
            /*
             * [live_range]
             *         [front]
             * ->
             * [    front    ]
             */
        } else if (liveRange.GetEnd() <= liveRanges_.back().GetEnd()) {
            liveRanges_.back().SetBegin(liveRange.GetBegin());
            /*
             * [ live_range  ]
             * [front]
             * ->
             * [    front    ]
             */
        } else if (!liveRanges_.back().Contains(liveRange)) {
            ASSERT(liveRanges_.back().GetBegin() == liveRange.GetBegin());
            liveRanges_.back().SetEnd(liveRange.GetEnd());
        }
    }

    void AppendRange(LifeNumber begin, LifeNumber end)
    {
        AppendRange({begin, end});
    }

    /*
     * Group range extends the first LiveRange, because it is covering the hole group,
     * starting from its header
     */
    void AppendGroupRange(LiveRange loopRange)
    {
        auto firstRange = liveRanges_.back();
        liveRanges_.pop_back();
        ASSERT(loopRange.GetBegin() == firstRange.GetBegin());
        // extend the first LiveRange
        firstRange.SetEnd(std::max(loopRange.GetEnd(), firstRange.GetEnd()));

        // resolve overlapping
        while (!liveRanges_.empty()) {
            if (firstRange.Contains(liveRanges_.back())) {
                liveRanges_.pop_back();
            } else if (firstRange.Contains(liveRanges_.back().GetBegin())) {
                ASSERT(liveRanges_.back().GetEnd() > firstRange.GetEnd());
                firstRange.SetEnd(liveRanges_.back().GetEnd());
                liveRanges_.pop_back();
                break;
            } else {
                break;
            }
        }
        liveRanges_.push_back(firstRange);
    }

    void Clear()
    {
        liveRanges_.clear();
    }

    /*
     * Shorten the first range or create it if instruction has no users
     */
    void StartFrom(LifeNumber from)
    {
        if (liveRanges_.empty()) {
            AppendRange(from, from + LIFE_NUMBER_GAP);
        } else {
            ASSERT(liveRanges_.back().GetEnd() >= from);
            liveRanges_.back().SetBegin(from);
        }
    }

    const ArenaVector<LiveRange> &GetRanges() const
    {
        return liveRanges_;
    }

    LifeNumber GetBegin() const
    {
        ASSERT(!GetRanges().empty());
        return GetRanges().back().GetBegin();
    }

    LifeNumber GetEnd() const
    {
        ASSERT(!GetRanges().empty());
        return GetRanges().front().GetEnd();
    }

    template <bool INCLUDE_BORDER = false>
    bool SplitCover(LifeNumber position) const
    {
        for (auto range : GetRanges()) {
            if (range.GetBegin() <= position && position < range.GetEnd()) {
                return true;
            }
            if constexpr (INCLUDE_BORDER) {  // NOLINT(readability-braces-around-statements,
                                             // bugprone-suspicious-semicolon)
                if (position == range.GetEnd()) {
                    return true;
                }
            }
        }
        return false;
    }

    void SetReg(Register reg)
    {
        SetLocation(Location::MakeRegister(reg, type_));
    }

    void SetPreassignedReg(Register reg)
    {
        SetReg(reg);
        isPreassigned_ = true;
    }

    void SetPhysicalReg(Register reg, DataType::Type type)
    {
        SetLocation(Location::MakeRegister(reg, type));
        SetType(type);
        isPhysical_ = true;
    }

    Register GetReg() const
    {
        return location_.GetValue();
    }

    bool HasReg() const
    {
        return location_.IsFixedRegister();
    }

    void SetLocation(Location location)
    {
        location_ = location;
    }

    Location GetLocation() const
    {
        return location_;
    }

    void ClearLocation()
    {
        SetLocation(Location::Invalid());
    }

    void SetType(DataType::Type type)
    {
        type_ = type;
    }

    DataType::Type GetType() const
    {
        return type_;
    }

    Inst *GetInst() const
    {
        ASSERT(!isPhysical_);
        return inst_;
    }

    bool HasInst() const
    {
        return inst_ != nullptr;
    }

    const auto &GetUsePositions() const
    {
        return usePositions_;
    }

    void AddUsePosition(LifeNumber ln)
    {
        ASSERT(ln != 0 && ln != INVALID_LIFE_NUMBER);
        ASSERT(!finalized_);
        usePositions_.push_back(ln);
    }

    void PrependUsePosition(LifeNumber ln)
    {
        ASSERT(ln != 0 && ln != INVALID_LIFE_NUMBER);
        ASSERT(finalized_);
        ASSERT(usePositions_.empty() || ln <= usePositions_.front());
        usePositions_.insert(usePositions_.begin(), ln);
    }

    LifeNumber GetNextUsage(LifeNumber pos) const
    {
        ASSERT(finalized_);
        auto it = std::lower_bound(usePositions_.begin(), usePositions_.end(), pos);
        if (it != usePositions_.end()) {
            return *it;
        }
        return INVALID_LIFE_NUMBER;
    }

    LifeNumber GetLastUsageBefore(LifeNumber pos) const
    {
        ASSERT(finalized_);
        auto it = std::lower_bound(usePositions_.begin(), usePositions_.end(), pos);
        if (it == usePositions_.begin()) {
            return INVALID_LIFE_NUMBER;
        }
        it = std::prev(it);
        return it == usePositions_.end() ? INVALID_LIFE_NUMBER : *it;
    }

    LifeNumber GetPrevUsage(LifeNumber pos) const
    {
        ASSERT(finalized_);
        auto it = std::upper_bound(usePositions_.begin(), usePositions_.end(), pos);
        if (it != usePositions_.begin()) {
            return *std::prev(it);
        }
        return INVALID_LIFE_NUMBER;
    }

    bool NoUsageUntil(LifeNumber pos) const
    {
        ASSERT(finalized_);
        return usePositions_.empty() || (*usePositions_.begin() > pos);
    }

    bool NoDest() const
    {
        if (IsPseudoUserOfMultiOutput(inst_)) {
            return false;
        }
        return inst_->NoDest();
    }

    bool IsPreassigned() const
    {
        return isPreassigned_;
    }

    bool IsPhysical() const
    {
        return isPhysical_;
    }

    template <bool WITH_INST_ID = true>
    std::string ToString() const
    {
        std::stringstream ss;
        auto delim = "";
        for (auto it = GetRanges().rbegin(); it != GetRanges().rend(); it++) {
            ss << delim << it->ToString();
            delim = " ";
        }
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (WITH_INST_ID) {
            if (HasInst()) {
                ss << " {inst v" << std::to_string(GetInst()->GetId()) << "}";
            } else {
                ss << " {physical}";
            }
        }
        return ss.str();
    }

    // Split current interval at specified life number and return new interval starting at `ln`.
    // If interval has range [begin, end) then SplitAt call will truncate it to [begin, ln) and
    // returned interval will have range [ln, end).
    LifeIntervals *SplitAt(LifeNumber ln, ArenaAllocator *alloc);

    // Split current interval before or after the first use, then recursively do the same with splits.
    // The result will be
    // [use1---use2---useN) -> [use1),[---),[use2),[---)[useN)
    void SplitAroundUses(ArenaAllocator *alloc);

    // Helper to merge interval, which was splitted at the beginning: [a, a+1) [a+1, b) -> [a,b)
    void MergeSibling();

    // Return sibling interval created by SplitAt call or nullptr if there is no sibling for current interval.
    LifeIntervals *GetSibling() const
    {
        return sibling_;
    }

    // Return sibling interval covering specified life number or nullptr if there is no such sibling.
    LifeIntervals *FindSiblingAt(LifeNumber ln);

    bool Intersects(const LiveRange &range) const;
    // Return first point where `this` interval intersects with the `other
    LifeNumber GetFirstIntersectionWith(const LifeIntervals *other, LifeNumber searchFrom = 0) const;

    template <bool OTHER_IS_PHYSICAL = false>
    bool IntersectsWith(const LifeIntervals *other) const
    {
        auto intersection = GetFirstIntersectionWith(other);
        if constexpr (OTHER_IS_PHYSICAL) {
            ASSERT(other->IsPhysical());
            // Interval can intersect the physical one at the beginning of its live range only if that physical
            // interval's range was created for it. Try to find next intersection
            //
            // interval [--------------------------]
            // physical [-]       [-]           [-]
            //           ^         ^
            //          skip      intersection
            if (intersection == GetBegin()) {
                intersection = GetFirstIntersectionWith(other, intersection + 1U);
            }
        }
        return intersection != INVALID_LIFE_NUMBER;
    }

    bool IsSplitSibling() const
    {
        return isSplitSibling_;
    }

    bool FindRangeCoveringPosition(LifeNumber ln, LiveRange *dst) const;

    void Finalize()
    {
#ifndef NDEBUG
        ASSERT(!finalized_);
        finalized_ = true;
#endif
        std::sort(usePositions_.begin(), usePositions_.end());
    }

private:
    Inst *inst_ {nullptr};
    ArenaVector<LiveRange> liveRanges_;
    LifeIntervals *sibling_ {nullptr};
    ArenaVector<LifeNumber> usePositions_;
    Location location_;
    DataType::Type type_;
    uint8_t isPreassigned_ : 1;
    uint8_t isPhysical_ : 1;
    uint8_t isSplitSibling_ : 1;
#ifndef NDEBUG
    uint8_t finalized_ : 1;
#endif
};

/*
 * Class to hold live instruction set
 */
class InstLiveSet {
public:
    explicit InstLiveSet(size_t size, ArenaAllocator *allocator) : bits_(size, allocator) {};
    NO_MOVE_SEMANTIC(InstLiveSet);
    NO_COPY_SEMANTIC(InstLiveSet);
    ~InstLiveSet() = default;

    void Union(const InstLiveSet *other)
    {
        if (other == nullptr) {
            return;
        }
        ASSERT(bits_.size() == other->bits_.size());
        bits_ |= other->bits_;
    }

    void Add(size_t index)
    {
        ASSERT(index < bits_.size());
        bits_.SetBit(index);
    }

    void Remove(size_t index)
    {
        ASSERT(index < bits_.size());
        bits_.ClearBit(index);
    }

    bool IsSet(size_t index)
    {
        ASSERT(index < bits_.size());
        return bits_.GetBit(index);
    }

private:
    ArenaBitVector bits_;
};

using LocationHints = ArenaMap<LifeNumber, Register>;
/*
 * `LivenessAnalyzer` is based on algorithm, published by Christian Wimmer and Michael Franz in
 * "Linear Scan Register Allocation on SSA Form" paper. ACM, 2010.
 */
class LivenessAnalyzer : public Analysis {
public:
    explicit LivenessAnalyzer(Graph *graph);

    NO_MOVE_SEMANTIC(LivenessAnalyzer);
    NO_COPY_SEMANTIC(LivenessAnalyzer);
    ~LivenessAnalyzer() override = default;

    bool RunImpl() override;
    void Cleanup();
    void Finalize();
    const char *GetPassName() const override;

    const ArenaVector<BasicBlock *> &GetLinearizedBlocks() const;
    const ArenaVector<LifeIntervals *> &GetLifeIntervals() const;

    LifeIntervals *GetInstLifeIntervals(const Inst *inst) const;
    Inst *GetInstByLifeNumber(LifeNumber ln) const;
    BasicBlock *GetBlockCoversPoint(LifeNumber ln) const;
    LiveRange GetBlockLiveRange(const BasicBlock *block) const;

    template <typename Func>
    void EnumerateLiveIntervalsForInst(Inst *inst, Func func)
    {
        auto instNumber = GetInstLifeNumber(inst);
        for (auto &li : GetLifeIntervals()) {
            if (!li->HasInst()) {
                continue;
            }
            auto liInst = li->GetInst();
            // phi-inst could be removed after regalloc
            if (liInst->GetBasicBlock() == nullptr) {
                ASSERT(liInst->IsPhi());
                continue;
            }
            if (liInst == inst || li->NoDest()) {
                continue;
            }
            auto sibling = li->FindSiblingAt(instNumber);
            if (sibling != nullptr && sibling->SplitCover(instNumber)) {
                func(sibling);
            }
        }
    }

    /*
     * 'interval_for_temp' - is additional life interval for an instruction required temp;
     * - Find instruction for which was created 'interval_for_temp'
     * - Enumerate instruction's inputs with fixed locations
     */
    template <typename Func>
    void EnumerateFixedLocationsOverlappingTemp(const LifeIntervals *intervalForTemp, Func func) const
    {
        ASSERT(!intervalForTemp->HasInst());
        ASSERT(intervalForTemp->GetBegin() + 1 == intervalForTemp->GetEnd());

        auto ln = intervalForTemp->GetEnd();
        auto inst = GetInstByLifeNumber(ln);
        ASSERT(inst != nullptr);

        for (size_t i = 0; i < inst->GetInputsCount(); i++) {
            auto location = inst->GetLocation(i);
            if (location.IsFixedRegister()) {
                func(location);
            }
        }
    }

    const UseTable &GetUseTable() const;
    size_t GetBlocksCount() const;
    LifeIntervals *GetTmpRegInterval(const Inst *inst);
    bool IsCallBlockingRegisters(Inst *inst) const;

    void DumpLifeIntervals(std::ostream &out = std::cout) const;
    void DumpLocationsUsage(std::ostream &out = std::cout) const;

private:
    ArenaAllocator *GetAllocator();
    void ResetLiveness();

    /*
     * Blocks linearization methods
     */
    bool AllForwardEdgesVisited(BasicBlock *block);
    void BuildBlocksLinearOrder();
    template <bool USE_PC_ORDER>
    void LinearizeBlocks();
    template <bool USE_PC_ORDER>
    void InsertSuccToPendingList(ArenaList<BasicBlock *> &pending, BasicBlock *succ);
    bool CheckLinearOrder();

    /*
     * Lifetime analysis methods
     */
    void BuildInstLifeNumbers();
    void BuildInstLifeIntervals();
    void ProcessBlockLiveInstructions(BasicBlock *block, InstLiveSet *liveSet);
    void AdjustInputsLifetime(Inst *inst, LiveRange liveRange, InstLiveSet *liveSet);
    void SetInputRange(const Inst *inst, const Inst *input, LiveRange liveRange) const;
    void CreateLifeIntervals(Inst *inst);
    void CreateIntervalForTemp(LifeNumber ln);
    InstLiveSet *GetInitInstLiveSet(BasicBlock *block);
    LifeNumber GetInstLifeNumber(const Inst *inst) const;
    void SetInstLifeNumber(const Inst *inst, LifeNumber number);
    void SetBlockLiveRange(BasicBlock *block, LiveRange lifeRange);
    void SetBlockLiveSet(BasicBlock *block, InstLiveSet *liveSet);
    InstLiveSet *GetBlockLiveSet(BasicBlock *block) const;
    LifeNumber GetLoopEnd(Loop *loop);
    LiveRange GetPropagatedLiveRange(Inst *inst, LiveRange liveRange);
    void AdjustCatchPhiInputsLifetime(Inst *inst);
    void SetUsePositions(Inst *userInst, LifeNumber lifeNumber);

    void BlockFixedRegisters(Inst *inst);
    template <bool IS_FP>
    void BlockReg(Register reg, LifeNumber blockFrom, LifeNumber blockTo, bool isUse);
    template <bool IS_FP>
    void BlockPhysicalRegisters(Inst *inst, LifeNumber blockFrom);
    void BlockFixedLocationRegister(Location location, LifeNumber ln);
    void BlockFixedLocationRegister(Location location, LifeNumber blockFrom, LifeNumber blockTo, bool isUse);
    void ProcessOpcodeLiveOut(BasicBlock *block, LifeIntervals *interval, LifeNumber instLifeNumber);
    static bool IsNativeApiRuntimeCall(Inst *inst);

private:
    ArenaAllocator *allocator_;
    ArenaVector<BasicBlock *> linearBlocks_;
    ArenaVector<LifeNumber> instLifeNumbers_;
    ArenaVector<LifeIntervals *> instLifeIntervals_;
    InstVector instsByLifeNumber_;
    ArenaVector<LiveRange> blockLiveRanges_;
    ArenaVector<InstLiveSet *> blockLiveSets_;
    ArenaMultiMap<Inst *, Inst *> pendingCatchPhiInputs_;
    ArenaVector<LifeIntervals *> physicalGeneralIntervals_;
    ArenaVector<LifeIntervals *> physicalVectorIntervals_;
    ArenaVector<LifeIntervals *> intervalsForTemps_;
    UseTable useTable_;
    bool hasSafepointDuringCall_;

    Marker marker_ {UNDEF_MARKER};
#ifndef NDEBUG
    bool finalized_ {};
#endif
};

float CalcSpillWeight(const LivenessAnalyzer &la, LifeIntervals *interval);
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_LIVENESS_ANALIZER_H
