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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_REG_ALLOC_BASE_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_REG_ALLOC_BASE_H

#include "compiler/optimizer/ir/graph.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "utils/arena_containers.h"
#include "location_mask.h"

namespace panda::compiler {

void ConnectIntervals(SpillFillInst *spill_fill, const LifeIntervals *src, const LifeIntervals *dst);

class RegAllocBase : public Optimization {
public:
    explicit RegAllocBase(Graph *graph);
    RegAllocBase(Graph *graph, size_t regs_count);
    RegAllocBase(Graph *graph, const RegMask &reg_mask, const VRegMask &vreg_mask, size_t slots_count);

    NO_MOVE_SEMANTIC(RegAllocBase);
    NO_COPY_SEMANTIC(RegAllocBase);

    ~RegAllocBase() override = default;

    bool RunImpl() override;
    const char *GetPassName() const override;
    bool AbortIfFailed() const override;

    template <typename T>
    void SetRegMask(const T &reg_mask)
    {
        regs_mask_.Init(reg_mask);
    }

    LocationMask &GetRegMask()
    {
        return regs_mask_;
    }

    const LocationMask &GetRegMask() const
    {
        return regs_mask_;
    }

    template <typename T>
    void SetVRegMask(const T &vreg_mask)
    {
        vregs_mask_.Init(vreg_mask);
    }

    LocationMask &GetVRegMask()
    {
        return vregs_mask_;
    }

    const LocationMask &GetVRegMask() const
    {
        return vregs_mask_;
    }

    void SetSlotsCount(size_t slots_count)
    {
        stack_mask_.Resize(slots_count);
        stack_use_last_positions_.resize(slots_count);
    }

    LocationMask &GetStackMask()
    {
        return stack_mask_;
    }

    void ReserveTempRegisters();

    // resolve graph
    virtual bool Resolve();

protected:
    StackSlot GetNextStackSlot(LifeIntervals *interval)
    {
        return !GetStackMask().AllSet() ? GetNextStackSlotImpl(interval) : INVALID_STACK_SLOT;
    }

    StackSlot GetNextStackSlotImpl(LifeIntervals *interval)
    {
        for (size_t slot = 0; slot < GetStackMask().GetSize(); slot++) {
            if (GetStackMask().IsSet(slot)) {
                continue;
            }

            ASSERT(slot < stack_use_last_positions_.size());
            if (stack_use_last_positions_[slot] > interval->GetBegin()) {
                continue;
            }

            GetStackMask().Set(slot);
            stack_use_last_positions_[slot] = interval->GetEnd();
            return slot;
        }
        return INVALID_STACK_SLOT;
    }

    bool PrepareIntervals();

    virtual void InitIntervals() {}

    virtual void PrepareInterval([[maybe_unused]] LifeIntervals *interval) {}

    // Prepare allocations and run passes such as LivenessAnalyzer
    virtual bool Prepare();

    // Arrange intervals/ranges and RA
    virtual bool Allocate() = 0;

    // Post resolve actions
    virtual bool Finish();

private:
    void SetType(LifeIntervals *interval);
    void SetPreassignedRegisters(LifeIntervals *interval);
    size_t GetTotalSlotsCount();

private:
    LocationMask regs_mask_;
    LocationMask vregs_mask_;
    LocationMask stack_mask_;
    ArenaVector<LifeNumber> stack_use_last_positions_;
};

}  // namespace panda::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_REGALLOC_REG_ALLOC_BASE_H
