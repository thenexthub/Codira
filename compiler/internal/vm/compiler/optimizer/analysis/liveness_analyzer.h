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

#ifndef COMPILER_OPTIMIZER_ANALYSIS_LIVENESS_ANALYZER_H
#define COMPILER_OPTIMIZER_ANALYSIS_LIVENESS_ANALYZER_H

#include "utils/arena_containers.h"
#include "optimizer/ir/constants.h"
#include "optimizer/ir/inst.h"
#include "optimizer/ir/marker.h"
#include "optimizer/pass.h"
#include "optimizer/ir/locations.h"
#include "compiler_logger.h"

namespace panda::compiler {
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

    LifeIntervals(ArenaAllocator *allocator, Inst *inst, LiveRange live_range)
        : inst_(inst),
          live_ranges_(allocator->Adapter()),
          use_positions_(allocator->Adapter()),
          location_(Location::Invalid()),
          type_(DataType::NO_TYPE),
          is_preassigned_(),
          is_physical_(),
          is_split_sibling_()
    {
        if (live_range.GetEnd() != 0) {
            live_ranges_.push_front(live_range);
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
    void AppendRange(LiveRange live_range)
    {
        ASSERT(live_range.GetEnd() >= live_range.GetBegin());
        // [live_range],[front]
        if (live_ranges_.empty() || live_range.GetEnd() < live_ranges_.front().GetBegin()) {
            live_ranges_.push_front(live_range);
            /*
             * [live_range]
             *         [front]
             * ->
             * [    front    ]
             */
        } else if (live_range.GetEnd() <= live_ranges_.front().GetEnd()) {
            live_ranges_.front().SetBegin(live_range.GetBegin());
            /*
             * [ live_range  ]
             * [front]
             * ->
             * [    front    ]
             */
        } else if (!live_ranges_.front().Contains(live_range)) {
            ASSERT(live_ranges_.front().GetBegin() == live_range.GetBegin());
            live_ranges_.front().SetEnd(live_range.GetEnd());
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
    void AppendGroupRange(LiveRange loop_range)
    {
        ASSERT(loop_range.GetBegin() == live_ranges_.front().GetBegin());
        // extend the first LiveRange
        live_ranges_.front().SetEnd(std::max(loop_range.GetEnd(), live_ranges_.front().GetEnd()));

        // resolve overlapping
        for (size_t i = 1; i < live_ranges_.size(); i++) {
            if (live_ranges_.front().Contains(live_ranges_[i])) {
                live_ranges_.erase(live_ranges_.begin() + i);
                i--;
            } else if (live_ranges_.front().Contains(live_ranges_[i].GetBegin())) {
                ASSERT(live_ranges_[i].GetEnd() > live_ranges_.front().GetEnd());
                live_ranges_.front().SetEnd(live_ranges_[i].GetEnd());
                live_ranges_.erase(live_ranges_.begin() + i);
                break;
            } else {
                break;
            }
        }
    }

    void Clear()
    {
        live_ranges_.clear();
    }

    /*
     * Shorten the first range or create it if instruction has no users
     */
    void StartFrom(LifeNumber from)
    {
        if (live_ranges_.empty()) {
            AppendRange(from, from + LIFE_NUMBER_GAP);
        } else {
            ASSERT(live_ranges_.front().GetEnd() >= from);
            live_ranges_.front().SetBegin(from);
        }
    }

    const ArenaDeque<LiveRange> &GetRanges() const
    {
        return live_ranges_;
    }

    LifeNumber GetBegin() const
    {
        ASSERT(!GetRanges().empty());
        return GetRanges().front().GetBegin();
    }

    LifeNumber GetEnd() const
    {
        ASSERT(!GetRanges().empty());
        return GetRanges().back().GetEnd();
    }

    template <bool include_border = false>
    bool SplitCover(LifeNumber position) const
    {
        for (auto range : GetRanges()) {
            if (range.GetBegin() <= position && position < range.GetEnd()) {
                return true;
            }
            if constexpr (include_border) {  // NOLINT(readability-braces-around-statements,
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
        is_preassigned_ = true;
    }

    void SetPhysicalReg(Register reg, DataType::Type type)
    {
        SetLocation(Location::MakeRegister(reg, type));
        SetType(type);
        is_physical_ = true;
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
        ASSERT(!is_physical_);
        return inst_;
    }

    const auto &GetUsePositions() const
    {
        return use_positions_;
    }

    void AddUsePosition(LifeNumber ln)
    {
        ASSERT(ln != 0 && ln != INVALID_LIFE_NUMBER);
        use_positions_.insert(ln);
    }

    LifeNumber GetNextUsage(LifeNumber pos)
    {
        auto it = use_positions_.lower_bound(pos);
        if (it != use_positions_.end()) {
            return *it;
        }
        return INVALID_LIFE_NUMBER;
    }

    LifeNumber GetLastUsageBefore(LifeNumber pos)
    {
        auto it = use_positions_.lower_bound(pos);
        if (it == use_positions_.begin()) {
            return INVALID_LIFE_NUMBER;
        }
        it = std::prev(it);
        return it == use_positions_.end() ? INVALID_LIFE_NUMBER : *it;
    }

    LifeNumber GetPrevUsage(LifeNumber pos) const
    {
        auto it = use_positions_.upper_bound(pos);
        if (it != use_positions_.begin()) {
            return *std::prev(it);
        }
        return INVALID_LIFE_NUMBER;
    }

    bool NoUsageUntil(LifeNumber pos) const
    {
        return use_positions_.empty() || (*use_positions_.begin() > pos);
    }

    bool NoDest() const
    {
        return inst_->NoDest();
    }

    bool IsPreassigned() const
    {
        return is_preassigned_;
    }

    bool IsPhysical() const
    {
        return is_physical_;
    }

    template <bool with_inst_id = true>
    std::string ToString() const
    {
        std::stringstream ss;
        auto delim = "";
        for (const auto &range : GetRanges()) {
            ss << delim << range.ToString();
            delim = " ";
        }
        // NOLINTNEXTLINE(readability-braces-around-statements, bugprone-suspicious-semicolon)
        if constexpr (with_inst_id) {
            if (!is_physical_) {
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
    LifeNumber GetFirstIntersectionWith(const LifeIntervals *other, LifeNumber search_from = 0) const;

    bool IntersectsWith(const LifeIntervals *other) const;

    bool IsSplitSibling() const
    {
        return is_split_sibling_;
    }

private:
    Inst *inst_ {nullptr};
    ArenaDeque<LiveRange> live_ranges_;
    LifeIntervals *sibling_ {nullptr};
    ArenaSet<LifeNumber> use_positions_;
    Location location_;
    DataType::Type type_;
    uint8_t is_preassigned_ : 1;
    uint8_t is_physical_ : 1;
    uint8_t is_split_sibling_ : 1;
};

/*
 * Class to hold live instruction set
 */
class InstLiveSet {
public:
    explicit InstLiveSet(size_t size, ArenaAllocator *allocator) : bits_(size, allocator->Adapter()) {};
    NO_MOVE_SEMANTIC(InstLiveSet);
    NO_COPY_SEMANTIC(InstLiveSet);
    ~InstLiveSet() = default;

    void Union(const InstLiveSet *other)
    {
        if (other == nullptr) {
            return;
        }
        ASSERT(bits_.size() == other->bits_.size());
        for (size_t i = 0; i < bits_.size(); i++) {
            bits_[i] = bits_[i] || other->bits_[i];
        }
    }

    void Add(size_t index)
    {
        ASSERT(index < bits_.size());
        bits_[index] = true;
    }

    void Remove(size_t index)
    {
        ASSERT(index < bits_.size());
        bits_[index] = false;
    }

    bool IsSet(size_t index)
    {
        ASSERT(index < bits_.size());
        return bits_[index];
    }

private:
    ArenaVector<bool> bits_;
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

    const ArenaVector<BasicBlock *> &GetLinearizedBlocks() const
    {
        return linear_blocks_;
    }
    LifeIntervals *GetInstLifeIntervals(const Inst *inst) const;

    Inst *GetInstByLifeNumber(LifeNumber ln) const
    {
        return insts_by_life_number_[ln / LIFE_NUMBER_GAP];
    }

    BasicBlock *GetBlockCoversPoint(LifeNumber ln) const
    {
        for (auto bb : linear_blocks_) {
            if (GetBlockLiveRange(bb).Contains(ln)) {
                return bb;
            }
        }
        return nullptr;
    }

    void Cleanup()
    {
        for (auto *interv : inst_life_intervals_) {
            if (!interv->IsPhysical() && !interv->IsPreassigned()) {
                interv->ClearLocation();
            }
            if (interv->GetSibling() != nullptr) {
                interv->MergeSibling();
            }
        }
    }

    const ArenaVector<LifeIntervals *> &GetLifeIntervals() const
    {
        return inst_life_intervals_;
    }
    LiveRange GetBlockLiveRange(const BasicBlock *block) const;

    template <typename Func>
    void EnumerateLiveIntervalsForInst(Inst *inst, Func func)
    {
        auto inst_number = GetInstLifeNumber(inst);
        for (auto &li : GetLifeIntervals()) {
            if (li->IsPhysical()) {
                continue;
            }
            auto li_inst = li->GetInst();
            // phi-inst could be removed after regalloc
            if (li_inst->GetBasicBlock() == nullptr) {
                ASSERT(li_inst->IsPhi());
                continue;
            }
            if (li_inst == inst || li->NoDest()) {
                continue;
            }
            auto sibling = li->FindSiblingAt(inst_number);
            if (sibling != nullptr && sibling->SplitCover(inst_number)) {
                func(sibling);
            }
        }
    }

    const char *GetPassName() const override
    {
        return "LivenessAnalysis";
    }

    size_t GetBlocksCount() const
    {
        return block_live_ranges_.size();
    }

    bool IsCallBlockingRegisters(Inst *inst) const;

    void DumpLifeIntervals(std::ostream &out = std::cout) const;
    void DumpLocationsUsage(std::ostream &out = std::cout) const;

private:
    ArenaAllocator *GetAllocator()
    {
        return allocator_;
    }

    void ResetLiveness();

    /*
     * Blocks linearization methods
     */
    bool AllForwardEdgesVisited(BasicBlock *block);
    void BuildBlocksLinearOrder();
    template <bool use_pc_order>
    void LinearizeBlocks();
    bool CheckLinearOrder();

    /*
     * Lifetime analysis methods
     */
    void BuildInstLifeNumbers();
    void BuildInstLifeIntervals();
    void ProcessBlockLiveInstructions(BasicBlock *block, InstLiveSet *live_set);
    void AdjustInputsLifetime(Inst *inst, LiveRange live_range, InstLiveSet *live_set);
    void SetInputRange(const Inst *inst, const Inst *input, LiveRange live_range) const;
    void CreateLifeIntervals(Inst *inst);
    InstLiveSet *GetInitInstLiveSet(BasicBlock *block);
    LifeNumber GetInstLifeNumber(Inst *inst) const;
    void SetInstLifeNumber(const Inst *inst, LifeNumber number);
    void SetBlockLiveRange(BasicBlock *block, LiveRange life_range);
    void SetBlockLiveSet(BasicBlock *block, InstLiveSet *live_set);
    InstLiveSet *GetBlockLiveSet(BasicBlock *block) const;
    LifeNumber GetLoopEnd(Loop *loop);
    LiveRange GetPropagatedLiveRange(Inst *inst, LiveRange live_range);
    void AdjustCatchPhiInputsLifetime(Inst *inst);

    template <bool is_fp>
    void BlockReg(Register reg, LifeNumber block_from);
    template <bool is_fp>
    void BlockPhysicalRegisters(LifeNumber block_from);
    void BlockFixedLocationRegister(Location location, LifeNumber ln);

private:
    ArenaAllocator *allocator_;
    ArenaVector<BasicBlock *> linear_blocks_;
    ArenaVector<LifeNumber> inst_life_numbers_;
    ArenaVector<LifeIntervals *> inst_life_intervals_;
    InstVector insts_by_life_number_;
    ArenaVector<LiveRange> block_live_ranges_;
    ArenaVector<InstLiveSet *> block_live_sets_;
    ArenaMultiMap<Inst *, Inst *> pending_catch_phi_inputs_;
    ArenaVector<LifeIntervals *> physical_general_intervals_;
    ArenaVector<LifeIntervals *> physical_vector_intervals_;
    Marker marker_ {UNDEF_MARKER};
};
}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_ANALYSIS_LIVENESS_ANALYZER_H
