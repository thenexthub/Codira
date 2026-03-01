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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_REG_ALLOC_LINEAR_SCAN_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_REG_ALLOC_LINEAR_SCAN_H

#include "libarkbase/utils/arena_containers.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "optimizer/code_generator/registers_description.h"
#include "reg_alloc_base.h"
#include "reg_map.h"
#include "compiler_logger.h"

namespace ark::compiler {
class BasicBlock;
class Graph;
class LifeIntervals;

using InstructionsIntervals = ArenaList<LifeIntervals *>;
using EmptyRegMask = std::bitset<0>;

/**
 * Algorithm is based on paper "Linear Scan Register Allocation" by Christian Wimmer
 *
 * LinearScan works with instructions lifetime intervals, computed by the LivenessAnalyzer.
 * It scans forward through the intervals, ordered by increasing starting point and assign registers while the number of
 * the active intervals at the point is less then the number of available registers. When there are no available
 * registers for some interval, LinearScan assigns one of the blocked ones. Previous holder of this blocked register is
 * spilled to the memory at this point. Blocked register to be assigned is selected according to the usage information,
 * LinearScan selects register with the most distant use point from the current one.
 */
class RegAllocLinearScan : public RegAllocBase {
    struct WorkingIntervals {
        explicit WorkingIntervals(ArenaAllocator *allocator)
            : active(allocator->Adapter()),
              inactive(allocator->Adapter()),
              stack(allocator->Adapter()),
              handled(allocator->Adapter()),
              fixed(allocator->Adapter())
        {
        }

        void Clear()
        {
            active.clear();
            inactive.clear();
            stack.clear();
            handled.clear();
            fixed.clear();
        }

        InstructionsIntervals active;        // NOLINT(misc-non-private-member-variables-in-classes)
        InstructionsIntervals inactive;      // NOLINT(misc-non-private-member-variables-in-classes)
        InstructionsIntervals stack;         // NOLINT(misc-non-private-member-variables-in-classes)
        InstructionsIntervals handled;       // NOLINT(misc-non-private-member-variables-in-classes)
        ArenaVector<LifeIntervals *> fixed;  // NOLINT(misc-non-private-member-variables-in-classes)
    };

    struct PendingIntervals {
        explicit PendingIntervals(ArenaAllocator *allocator)
            : regular(allocator->Adapter()), fixed(allocator->Adapter())
        {
        }
        InstructionsIntervals regular;       // NOLINT(misc-non-private-member-variables-in-classes)
        ArenaVector<LifeIntervals *> fixed;  // NOLINT(misc-non-private-member-variables-in-classes)
    };

public:
    explicit RegAllocLinearScan(Graph *graph);
    RegAllocLinearScan(Graph *graph, [[maybe_unused]] EmptyRegMask mask);
    ~RegAllocLinearScan() override = default;
    NO_MOVE_SEMANTIC(RegAllocLinearScan);
    NO_COPY_SEMANTIC(RegAllocLinearScan);

    const char *GetPassName() const override
    {
        return "RegAllocLinearScan";
    }

    bool AbortIfFailed() const override
    {
        return true;
    }

protected:
    bool Allocate() override;
    void InitIntervals() override;
    void PrepareInterval(LifeIntervals *interval) override;

private:
    template <bool IS_FP>
    const LocationMask &GetLocationMask() const
    {
        return IS_FP ? GetVRegMask() : GetRegMask();
    }

    template <bool IS_FP>
    PendingIntervals &GetIntervals()
    {
        return IS_FP ? vectorIntervals_ : generalIntervals_;
    }

    template <bool IS_FP>
    void AssignLocations();
    template <bool IS_FP>
    void PreprocessPreassignedIntervals();
    template <bool IS_FP>
    void ExpireIntervals(LifeNumber currentPosition);
    template <bool IS_FP>
    void WalkIntervals();
    template <bool IS_FP>
    bool TryToAssignRegister(LifeIntervals *currentInterval);
    template <bool IS_FP>
    void SplitAndSpill(const InstructionsIntervals *intervals, const LifeIntervals *currentInterval);
    template <bool IS_FP>
    void SplitActiveInterval(LifeIntervals *interval, LifeNumber splitPos);
    template <bool IS_FP>
    void AddToQueue(LifeIntervals *interval);
    template <bool IS_FP>
    void SplitBeforeUse(LifeIntervals *currentInterval, LifeNumber usePos);
    bool IsIntervalRegFree(const LifeIntervals *currentInterval, Register reg) const;
    void AssignStackSlot(LifeIntervals *interval);
    void RemapRegallocReg(LifeIntervals *interval);
    void RemapRegistersIntervals();
    template <bool IS_FP>
    void AddFixedIntervalsToWorkingIntervals();
    template <bool IS_FP>
    void HandleFixedIntervalIntersection(LifeIntervals *currentInterval);

    Register GetSuitableRegister(const LifeIntervals *currentInterval);
    Register GetFreeRegister(const LifeIntervals *currentInterval);
    std::pair<Register, LifeNumber> GetBlockedRegister(const LifeIntervals *currentInterval);

    template <class T, class Callback>
    void IterateIntervalsWithErasion(T &intervals, const Callback &callback) const
    {
        for (auto it = intervals.begin(); it != intervals.end();) {
            auto interval = *it;
            if (callback(interval)) {
                it = intervals.erase(it);
            } else {
                ++it;
            }
        }
    }

    template <class T, class Callback>
    void EnumerateIntervals(const T &intervals, const Callback &callback) const
    {
        for (const auto &interval : intervals) {
            if (interval == nullptr || interval->GetReg() >= regMap_.GetAvailableRegsCount()) {
                continue;
            }
            callback(interval);
        }
    }

    template <class T, class Callback>
    void EnumerateIntersectedIntervals(const T &intervals, const LifeIntervals *current, const Callback &callback) const
    {
        for (const auto &interval : intervals) {
            if (interval == nullptr || interval->GetReg() >= regMap_.GetAvailableRegsCount()) {
                continue;
            }
            auto intersection = interval->GetFirstIntersectionWith(current);
            if (intersection != INVALID_LIFE_NUMBER) {
                callback(interval, intersection);
            }
        }
    }

    void BlockOverlappedRegisters(const LifeIntervals *currentInterval);
    void BlockIndirectCallRegisters(const LifeIntervals *currentInterval);
    bool IsNonSpillableConstInterval(LifeIntervals *interval);
    void BeforeConstantIntervalSpill(LifeIntervals *interval, LifeNumber splitPos);

private:
    WorkingIntervals workingIntervals_;
    ArenaVector<LifeNumber> regsUsePositions_;
    PendingIntervals generalIntervals_;
    PendingIntervals vectorIntervals_;
    RegisterMap regMap_;
    bool rematConstants_;
    bool success_ {true};
};
}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_REG_ALLOC_LINEAR_SCAN_H
