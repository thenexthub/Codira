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

#include "reg_alloc_linear_scan.h"
#include "optimizer/analysis/loop_analyzer.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/graph.h"

namespace ark::compiler {

static constexpr auto MAX_LIFE_NUMBER = std::numeric_limits<LifeNumber>::max();

/// Add interval in sorted order
static void AddInterval(LifeIntervals *interval, InstructionsIntervals *dest)
{
    auto cmp = [](LifeIntervals *lhs, LifeIntervals *rhs) { return lhs->GetBegin() >= rhs->GetBegin(); };

    if (dest->empty()) {
        dest->push_back(interval);
        return;
    }

    auto iter = dest->end();
    --iter;
    while (true) {
        if (cmp(interval, *iter)) {
            dest->insert(++iter, interval);
            break;
        }
        if (iter == dest->begin()) {
            dest->push_front(interval);
            break;
        }
        --iter;
    }
}

/// Use graph's registers masks and `MAX_NUM_STACK_SLOTS` stack slots
RegAllocLinearScan::RegAllocLinearScan(Graph *graph)
    : RegAllocBase(graph),
      workingIntervals_(graph->GetLocalAllocator()),
      regsUsePositions_(graph->GetLocalAllocator()->Adapter()),
      generalIntervals_(graph->GetLocalAllocator()),
      vectorIntervals_(graph->GetLocalAllocator()),
      regMap_(graph->GetLocalAllocator()),
      rematConstants_(!graph->IsBytecodeOptimizer() && g_options.IsCompilerRematConst())
{
}

/// Use dynamic general registers mask (without vector regs) and zero stack slots
RegAllocLinearScan::RegAllocLinearScan(Graph *graph, [[maybe_unused]] EmptyRegMask mask)
    : RegAllocBase(graph, GetFrameSize()),
      workingIntervals_(graph->GetLocalAllocator()),
      regsUsePositions_(graph->GetLocalAllocator()->Adapter()),
      generalIntervals_(graph->GetLocalAllocator()),
      vectorIntervals_(graph->GetLocalAllocator()),
      regMap_(graph->GetLocalAllocator()),
      rematConstants_(!graph->IsBytecodeOptimizer() && g_options.IsCompilerRematConst())
{
}

/// Allocate registers
bool RegAllocLinearScan::Allocate()
{
    AssignLocations<false>();
    AssignLocations<true>();
    return success_;
}

void RegAllocLinearScan::InitIntervals()
{
    GetIntervals<false>().fixed.resize(MAX_NUM_REGS);
    GetIntervals<true>().fixed.resize(MAX_NUM_VREGS);
    ReserveTempRegisters();
}

void RegAllocLinearScan::PrepareInterval(LifeIntervals *interval)
{
    bool isFp = DataType::IsFloatType(interval->GetType());
    auto &intervals = isFp ? GetIntervals<true>() : GetIntervals<false>();

    if (interval->IsPhysical()) {
        ASSERT(intervals.fixed.size() > interval->GetReg());
        ASSERT(intervals.fixed[interval->GetReg()] == nullptr);
        intervals.fixed[interval->GetReg()] = interval;
        return;
    }

    if (!interval->HasInst()) {
        AddInterval(interval, &intervals.regular);
        return;
    }

    if (interval->NoDest() || interval->GetInst()->GetDstCount() > 1U || interval->GetReg() == GetAccReg()) {
        return;
    }

    if (interval->IsPreassigned() && interval->GetReg() == GetGraph()->GetZeroReg()) {
        ASSERT(interval->GetReg() != GetInvalidReg());
        return;
    }

    AddInterval(interval, &intervals.regular);
}

template <bool IS_FP>
void RegAllocLinearScan::AssignLocations()
{
    auto &regularIntervals = GetIntervals<IS_FP>().regular;
    if (regularIntervals.empty()) {
        return;
    }

    // NOLINTNEXTLINE(readability-magic-numbers,readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (IS_FP) {
        GetGraph()->SetHasFloatRegs();
    }

    workingIntervals_.Clear();
    auto arch = GetGraph()->GetArch();
    size_t priority = arch != Arch::NONE ? GetFirstCalleeReg(arch, IS_FP) : 0;
    regMap_.SetMask(GetLocationMask<IS_FP>(), priority);
    regsUsePositions_.resize(regMap_.GetAvailableRegsCount());

    AddFixedIntervalsToWorkingIntervals<IS_FP>();
    PreprocessPreassignedIntervals<IS_FP>();
    while (!regularIntervals.empty() && success_) {
        ExpireIntervals<IS_FP>(regularIntervals.front()->GetBegin());
        WalkIntervals<IS_FP>();
    }
    RemapRegistersIntervals();
}

template <bool IS_FP>
void RegAllocLinearScan::PreprocessPreassignedIntervals()
{
    for (auto &interval : GetIntervals<IS_FP>().regular) {
        if (!interval->IsPreassigned() || interval->IsSplitSibling() || interval->GetReg() == GetAccReg()) {
            continue;
        }
        interval->SetPreassignedReg(regMap_.CodegenToRegallocReg(interval->GetReg()));
        COMPILER_LOG(DEBUG, REGALLOC) << "Preassigned interval " << interval->template ToString<true>();
    }
}

/// Free registers from expired intervals
template <bool IS_FP>
void RegAllocLinearScan::ExpireIntervals(LifeNumber currentPosition)
{
    IterateIntervalsWithErasion(workingIntervals_.active, [this, currentPosition](const auto &interval) {
        if (!interval->HasReg() || interval->GetEnd() <= currentPosition) {
            workingIntervals_.handled.push_back(interval);
            return true;
        }
        if (!interval->SplitCover(currentPosition)) {
            AddInterval(interval, &workingIntervals_.inactive);
            return true;
        }
        return false;
    });
    IterateIntervalsWithErasion(workingIntervals_.inactive, [this, currentPosition](const auto &interval) {
        if (!interval->HasReg() || interval->GetEnd() <= currentPosition) {
            workingIntervals_.handled.push_back(interval);
            return true;
        }
        if (interval->SplitCover(currentPosition)) {
            AddInterval(interval, &workingIntervals_.active);
            return true;
        }
        return false;
    });
    IterateIntervalsWithErasion(workingIntervals_.stack, [this, currentPosition](const auto &interval) {
        if (interval->GetEnd() <= currentPosition) {
            GetStackMask().Reset(interval->GetLocation().GetValue());
            return true;
        }
        return false;
    });
}

/// Working intervals processing
template <bool IS_FP>
void RegAllocLinearScan::WalkIntervals()
{
    auto currentInterval = GetIntervals<IS_FP>().regular.front();
    GetIntervals<IS_FP>().regular.pop_front();
    COMPILER_LOG(DEBUG, REGALLOC) << "----------------";
    COMPILER_LOG(DEBUG, REGALLOC) << "Process interval " << currentInterval->template ToString<true>();

    // Parameter that was passed in the stack slot: split its interval before first use
    if (currentInterval->GetLocation().IsStackParameter()) {
        ASSERT(currentInterval->GetInst()->IsParameter());
        COMPILER_LOG(DEBUG, REGALLOC) << "Interval was defined in the stack parameter slot";
        auto nextUse = currentInterval->GetNextUsage(currentInterval->GetBegin() + 1U);
        SplitBeforeUse<IS_FP>(currentInterval, nextUse);
        return;
    }

    if (!currentInterval->HasReg()) {
        if (TryToAssignRegister<IS_FP>(currentInterval)) {
            COMPILER_LOG(DEBUG, REGALLOC)
                << currentInterval->GetLocation().ToString(GetGraph()->GetArch()) << " was assigned to the interval "
                << currentInterval->template ToString<true>();
        } else {
            COMPILER_LOG(ERROR, REGALLOC) << "There are no available registers";
            success_ = false;
            return;
        }
    } else {
        ASSERT(currentInterval->IsPreassigned());
        COMPILER_LOG(DEBUG, REGALLOC) << "Interval has preassigned "
                                      << currentInterval->GetLocation().ToString(GetGraph()->GetArch());
        if (!IsIntervalRegFree(currentInterval, currentInterval->GetReg())) {
            SplitAndSpill<IS_FP>(&workingIntervals_.active, currentInterval);
            SplitAndSpill<IS_FP>(&workingIntervals_.inactive, currentInterval);
        }
    }
    HandleFixedIntervalIntersection<IS_FP>(currentInterval);
    AddInterval(currentInterval, &workingIntervals_.active);
}

template <bool IS_FP>
bool RegAllocLinearScan::TryToAssignRegister(LifeIntervals *currentInterval)
{
    auto reg = GetSuitableRegister(currentInterval);
    if (reg != GetInvalidReg()) {
        currentInterval->SetReg(reg);
        return true;
    }

    // Try to assign blocked register
    auto [blocked_reg, next_blocked_use] = GetBlockedRegister(currentInterval);
    auto nextUse = currentInterval->GetNextUsage(currentInterval->GetBegin());
    // Spill current interval if its first use later than use of blocked register
    if (blocked_reg != GetInvalidReg() && next_blocked_use < nextUse && !IsNonSpillableConstInterval(currentInterval)) {
        SplitBeforeUse<IS_FP>(currentInterval, nextUse);
        AssignStackSlot(currentInterval);
        return true;
    }

    // Blocked register that will be used in the next position mustn't be reassigned
    if (blocked_reg == GetInvalidReg() || next_blocked_use < currentInterval->GetBegin() + LIFE_NUMBER_GAP) {
        return false;
    }

    currentInterval->SetReg(blocked_reg);
    SplitAndSpill<IS_FP>(&workingIntervals_.active, currentInterval);
    SplitAndSpill<IS_FP>(&workingIntervals_.inactive, currentInterval);
    return true;
}

template <bool IS_FP>
void RegAllocLinearScan::SplitAndSpill(const InstructionsIntervals *intervals, const LifeIntervals *currentInterval)
{
    for (auto interval : *intervals) {
        if (interval->GetReg() != currentInterval->GetReg() ||
            interval->GetFirstIntersectionWith(currentInterval) == INVALID_LIFE_NUMBER) {
            continue;
        }
        COMPILER_LOG(DEBUG, REGALLOC) << "Found active interval: " << interval->template ToString<true>();
        SplitActiveInterval<IS_FP>(interval, currentInterval->GetBegin());
    }
}

/**
 * Split interval with assigned 'reg' into 3 parts:
 * [interval|split|split_next]
 *
 * 'interval' - holds assigned register;
 * 'split' - stack slot is assigned to it;
 * 'split_next' - added to the queue for future assignment;
 */
template <bool IS_FP>
void RegAllocLinearScan::SplitActiveInterval(LifeIntervals *interval, LifeNumber splitPos)
{
    BeforeConstantIntervalSpill(interval, splitPos);
    auto prevUsePos = interval->GetPrevUsage(splitPos);
    auto nextUsePos = interval->GetNextUsage(splitPos + 1U);
    COMPILER_LOG(DEBUG, REGALLOC) << "Prev use position: " << std::to_string(prevUsePos)
                                  << ", Next use position: " << std::to_string(nextUsePos);
    auto split = interval;
    if (prevUsePos == INVALID_LIFE_NUMBER) {
        COMPILER_LOG(DEBUG, REGALLOC) << "Spill the whole interval " << interval->template ToString<true>();
        interval->ClearLocation();
    } else {
        auto splitPosition = (splitPos % 2U == 1) ? splitPos : splitPos - 1;
        COMPILER_LOG(DEBUG, REGALLOC) << "Split interval " << interval->template ToString<true>() << " at position "
                                      << static_cast<int>(splitPosition);

        split = interval->SplitAt(splitPosition, GetGraph()->GetAllocator());
    }
    SplitBeforeUse<IS_FP>(split, nextUsePos);
    AssignStackSlot(split);
}

template <bool IS_FP>
void RegAllocLinearScan::AddToQueue(LifeIntervals *interval)
{
    COMPILER_LOG(DEBUG, REGALLOC) << "Add to the queue: " << interval->template ToString<true>();
    AddInterval(interval, &GetIntervals<IS_FP>().regular);
}

Register RegAllocLinearScan::GetSuitableRegister(const LifeIntervals *currentInterval)
{
    if (!currentInterval->HasInst()) {
        return GetFreeRegister(currentInterval);
    }
    // First of all, try to assign register using hint
    auto &useTable = GetGraph()->GetAnalysis<LivenessAnalyzer>().GetUseTable();
    auto hintReg = useTable.GetNextUseOnFixedLocation(currentInterval->GetInst(), currentInterval->GetBegin());
    if (hintReg != GetInvalidReg()) {
        auto reg = regMap_.CodegenToRegallocReg(hintReg);
        if (regMap_.IsRegAvailable(reg, GetGraph()->GetArch()) && IsIntervalRegFree(currentInterval, reg)) {
            COMPILER_LOG(DEBUG, REGALLOC) << "Hint-register is available";
            return reg;
        }
    }
    // If hint doesn't exist or hint-register not available, try to assign any free register
    return GetFreeRegister(currentInterval);
}

Register RegAllocLinearScan::GetFreeRegister(const LifeIntervals *currentInterval)
{
    std::fill(regsUsePositions_.begin(), regsUsePositions_.end(), MAX_LIFE_NUMBER);

    auto setFixedUsage = [this, &currentInterval](const auto &interval, LifeNumber intersection) {
        // If intersection is equal to the current_interval's begin
        // than it means that current_interval is a call and fixed interval's range was
        // created for it.
        // Do not disable fixed register for a call.
        if (intersection == currentInterval->GetBegin()) {
            LiveRange range;
            [[maybe_unused]] bool succ = interval->FindRangeCoveringPosition(intersection, &range);
            ASSERT(succ);
            if (range.GetBegin() == intersection) {
                return;
            }
        }
        ASSERT(regsUsePositions_.size() > interval->GetReg());
        regsUsePositions_[interval->GetReg()] = intersection;
    };
    auto setInactiveUsage = [this](const auto &interval, LifeNumber intersection) {
        ASSERT(regsUsePositions_.size() > interval->GetReg());
        auto &regUse = regsUsePositions_[interval->GetReg()];
        regUse = std::min<size_t>(intersection, regUse);
    };

    EnumerateIntersectedIntervals(workingIntervals_.fixed, currentInterval, setFixedUsage);
    EnumerateIntersectedIntervals(workingIntervals_.inactive, currentInterval, setInactiveUsage);
    EnumerateIntervals(workingIntervals_.active, [this](const auto &interval) {
        ASSERT(regsUsePositions_.size() > interval->GetReg());
        regsUsePositions_[interval->GetReg()] = 0;
    });

    BlockOverlappedRegisters(currentInterval);
    BlockIndirectCallRegisters(currentInterval);

    // Select register with max position
    auto it = std::max_element(regsUsePositions_.cbegin(), regsUsePositions_.cend());
    // Register is free if it's available for the whole interval
    return (*it >= currentInterval->GetEnd()) ? std::distance(regsUsePositions_.cbegin(), it) : GetInvalidReg();
}

// Return blocked register and its next use position
std::pair<Register, LifeNumber> RegAllocLinearScan::GetBlockedRegister(const LifeIntervals *currentInterval)
{
    // Using blocked registers is impossible in the `BytecodeOptimizer` mode
    if (GetGraph()->IsBytecodeOptimizer()) {
        return {GetInvalidReg(), INVALID_LIFE_NUMBER};
    }

    std::fill(regsUsePositions_.begin(), regsUsePositions_.end(), MAX_LIFE_NUMBER);

    auto setFixedUsage = [this, currentInterval](const auto &interval, LifeNumber intersection) {
        // If intersection is equal to the current_interval's begin
        // than it means that current_interval is a call and fixed interval's range was
        // created for it.
        // Do not disable fixed register for a call.
        if (intersection == currentInterval->GetBegin()) {
            LiveRange range;
            [[maybe_unused]] bool succ = interval->FindRangeCoveringPosition(intersection, &range);
            ASSERT(succ);
            if (intersection == range.GetBegin()) {
                return;
            }
        }
        ASSERT(interval->GetReg() < regsUsePositions_.size());
        regsUsePositions_[interval->GetReg()] = intersection;
    };
    auto setInactiveUsage = [this](const auto &interval, LifeNumber intersection) {
        ASSERT(regsUsePositions_.size() > interval->GetReg());
        auto &regUse = regsUsePositions_[interval->GetReg()];
        regUse = std::min<size_t>(interval->GetNextUsage(intersection), regUse);
    };

    EnumerateIntersectedIntervals(workingIntervals_.fixed, currentInterval, setFixedUsage);
    EnumerateIntersectedIntervals(workingIntervals_.inactive, currentInterval, setInactiveUsage);
    EnumerateIntervals(workingIntervals_.active, [this, &currentInterval](const auto &interval) {
        ASSERT(regsUsePositions_.size() > interval->GetReg());
        auto &regUse = regsUsePositions_[interval->GetReg()];
        regUse = std::min<size_t>(interval->GetNextUsage(currentInterval->GetBegin()), regUse);
    });

    BlockOverlappedRegisters(currentInterval);
    BlockIndirectCallRegisters(currentInterval);

    // Select register with max position
    auto it = std::max_element(regsUsePositions_.cbegin(), regsUsePositions_.cend());
    auto reg = std::distance(regsUsePositions_.cbegin(), it);
    COMPILER_LOG(DEBUG, REGALLOC) << "Selected blocked r" << static_cast<int>(reg) << " with use next use position "
                                  << *it;
    return {reg, regsUsePositions_[reg]};
}

bool RegAllocLinearScan::IsIntervalRegFree(const LifeIntervals *currentInterval, Register reg) const
{
    for (auto interval : workingIntervals_.fixed) {
        if (interval == nullptr || interval->GetReg() != reg) {
            continue;
        }
        if (interval->GetFirstIntersectionWith(currentInterval) < currentInterval->GetBegin() + LIFE_NUMBER_GAP) {
            return false;
        }
    }

    for (auto interval : workingIntervals_.inactive) {
        if (interval->GetReg() == reg && interval->GetFirstIntersectionWith(currentInterval) != INVALID_LIFE_NUMBER) {
            return false;
        }
    }
    for (auto interval : workingIntervals_.active) {
        if (interval->GetReg() == reg) {
            return false;
        }
    }
    return true;
}

void RegAllocLinearScan::AssignStackSlot(LifeIntervals *interval)
{
    ASSERT(!interval->GetLocation().IsStack());
    ASSERT(interval->HasInst());
    if (rematConstants_ && interval->GetInst()->IsConst()) {
        auto immSlot = GetGraph()->AddSpilledConstant(interval->GetInst()->CastToConstant());
        if (immSlot != GetInvalidImmTableSlot()) {
            interval->SetLocation(Location::MakeConstant(immSlot));
            COMPILER_LOG(DEBUG, REGALLOC) << interval->GetLocation().ToString(GetGraph()->GetArch())
                                          << " was assigned to the interval " << interval->template ToString<true>();
            return;
        }
    }

    auto slot = GetNextStackSlot(interval);
    if (slot != GetInvalidStackSlot()) {
        interval->SetLocation(Location::MakeStackSlot(slot));
        COMPILER_LOG(DEBUG, REGALLOC) << interval->GetLocation().ToString(GetGraph()->GetArch())
                                      << " was assigned to the interval " << interval->template ToString<true>();
        workingIntervals_.stack.push_back(interval);
    } else {
        COMPILER_LOG(ERROR, REGALLOC) << "There are no available stack slots";
        success_ = false;
    }
}

void RegAllocLinearScan::RemapRegallocReg(LifeIntervals *interval)
{
    if (interval->HasReg()) {
        auto reg = interval->GetReg();
        interval->SetReg(regMap_.RegallocToCodegenReg(reg));
    }
}

void RegAllocLinearScan::RemapRegistersIntervals()
{
    for (auto interval : workingIntervals_.handled) {
        RemapRegallocReg(interval);
    }
    for (auto interval : workingIntervals_.active) {
        RemapRegallocReg(interval);
    }
    for (auto interval : workingIntervals_.inactive) {
        RemapRegallocReg(interval);
    }
    for (auto interval : workingIntervals_.fixed) {
        if (interval != nullptr) {
            RemapRegallocReg(interval);
        }
    }
}

template <bool IS_FP>
void RegAllocLinearScan::AddFixedIntervalsToWorkingIntervals()
{
    workingIntervals_.fixed.resize(GetLocationMask<IS_FP>().GetSize());
    // remap registers for fixed intervals and add it to working intervals
    for (auto fixedInterval : GetIntervals<IS_FP>().fixed) {
        if (fixedInterval == nullptr) {
            continue;
        }
        auto reg = regMap_.CodegenToRegallocReg(fixedInterval->GetReg());
        fixedInterval->SetReg(reg);
        workingIntervals_.fixed[reg] = fixedInterval;
        COMPILER_LOG(DEBUG, REGALLOC) << "Fixed interval for r" << static_cast<int>(fixedInterval->GetReg()) << ": "
                                      << fixedInterval->template ToString<true>();
    }
}

template <bool IS_FP>
void RegAllocLinearScan::HandleFixedIntervalIntersection(LifeIntervals *currentInterval)
{
    if (!currentInterval->HasReg()) {
        return;
    }
    auto reg = currentInterval->GetReg();
    if (reg >= workingIntervals_.fixed.size() || workingIntervals_.fixed[reg] == nullptr) {
        return;
    }
    auto fixedInterval = workingIntervals_.fixed[reg];
    auto intersection = currentInterval->GetFirstIntersectionWith(fixedInterval);
    if (intersection == currentInterval->GetBegin()) {
        // Current interval can intersect fixed interval at the beginning of its live range
        // only if it's a call and fixed interval's range was created for it.
        // Try to find first intersection excluding the range blocking registers during a call.
        intersection = currentInterval->GetFirstIntersectionWith(fixedInterval, intersection + 1U);
    }
    if (intersection == INVALID_LIFE_NUMBER) {
        return;
    }
    COMPILER_LOG(DEBUG, REGALLOC) << "Intersection with fixed interval at: " << std::to_string(intersection);

    auto &useTable = GetGraph()->GetAnalysis<LivenessAnalyzer>().GetUseTable();
    if (useTable.HasUseOnFixedLocation(currentInterval->GetInst(), intersection)) {
        // Instruction is used at intersection position: split before that use
        SplitBeforeUse<IS_FP>(currentInterval, intersection);
        return;
    }

    BeforeConstantIntervalSpill(currentInterval, intersection);
    auto lastUseBefore = currentInterval->GetLastUsageBefore(intersection);
    if (lastUseBefore != INVALID_LIFE_NUMBER) {
        // Split after the last use before intersection
        SplitBeforeUse<IS_FP>(currentInterval, lastUseBefore + LIFE_NUMBER_GAP);
        return;
    }

    // There is no use before intersection, split after intersection add splitted-interval to the queue
    auto nextUse = currentInterval->GetNextUsage(intersection);
    currentInterval->ClearLocation();
    SplitBeforeUse<IS_FP>(currentInterval, nextUse);
    AssignStackSlot(currentInterval);
}

template <bool IS_FP>
void RegAllocLinearScan::SplitBeforeUse(LifeIntervals *currentInterval, LifeNumber usePos)
{
    if (usePos == INVALID_LIFE_NUMBER) {
        return;
    }
    COMPILER_LOG(DEBUG, REGALLOC) << "Split at " << std::to_string(usePos - 1);
    ASSERT(currentInterval != nullptr);
    auto split = currentInterval->SplitAt(usePos - 1, GetGraph()->GetAllocator());
    AddToQueue<IS_FP>(split);
}

void RegAllocLinearScan::BlockIndirectCallRegisters(const LifeIntervals *currentInterval)
{
    if (!currentInterval->HasInst()) {
        return;
    }
    auto inst = currentInterval->GetInst();
    for (auto &user : inst->GetUsers()) {
        auto userInst = user.GetInst();
        if (userInst->IsIndirectCall() && userInst->GetInput(0).GetInst() == inst) {
            // CallIndirect is a special case:
            //  - input[0] is a call address, which may be allocated to any register
            //  - other inputs are call args, and LocationsBuilder binds them
            //    to the corresponding target CPU registers.
            //
            // Thus input[0] must not be allocated to the registers used to pass arguments.
            for (size_t i = 1; i < userInst->GetInputsCount(); ++i) {
                auto location = userInst->GetLocation(i);
                ASSERT(location.IsFixedRegister());
                auto reg = regMap_.CodegenToRegallocReg(location.GetValue());
                regsUsePositions_[reg] = 0;
            }
            // LocationBuilder assigns locations the same way for all CallIndirect instructions.
            break;
        }
    }
}

void RegAllocLinearScan::BlockOverlappedRegisters(const LifeIntervals *currentInterval)
{
    if (!currentInterval->HasInst()) {
        // current_interval - is additional life interval for an instruction required temp, block fixed registers of
        // that instruction
        auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
        la.EnumerateFixedLocationsOverlappingTemp(currentInterval, [this](Location location) {
            ASSERT(location.IsFixedRegister());
            auto reg = regMap_.CodegenToRegallocReg(location.GetValue());
            if (regMap_.IsRegAvailable(reg, GetGraph()->GetArch())) {
                ASSERT(regsUsePositions_.size() > reg);
                regsUsePositions_[reg] = 0;
            }
        });
    }
}

/// Returns true if the interval corresponds to Constant instruction and it could not be spilled to a stack.
bool RegAllocLinearScan::IsNonSpillableConstInterval(LifeIntervals *interval)
{
    if (interval->IsSplitSibling() || interval->IsPhysical()) {
        return false;
    }
    auto inst = interval->GetInst();
    return inst != nullptr && inst->IsConst() && rematConstants_ &&
           inst->CastToConstant()->GetImmTableSlot() == GetInvalidImmTableSlot() &&
           !GetGraph()->HasAvailableConstantSpillSlots();
}

/**
 * If constant rematerialization is enabled then constant intervals won't have use position
 * at the beginning of an interval. If there are available immediate slots then it is not an issue
 * as the interval will have a slot assigned to it during the spill. But if there are not available
 * immediate slots then the whole interval will be spilled (which is incorrect) unless we add a use
 * position at the beginning and thus force split creation right after it.
 */
void RegAllocLinearScan::BeforeConstantIntervalSpill(LifeIntervals *interval, LifeNumber splitPos)
{
    if (!IsNonSpillableConstInterval(interval)) {
        return;
    }
    if (interval->GetPrevUsage(splitPos) != INVALID_LIFE_NUMBER) {
        return;
    }
    interval->PrependUsePosition(interval->GetBegin());
}

}  // namespace ark::compiler
