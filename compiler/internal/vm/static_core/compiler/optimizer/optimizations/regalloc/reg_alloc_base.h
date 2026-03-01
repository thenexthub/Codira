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

#ifndef COMPILER_OPTIMIZER_OPTIMIZATIONS_REG_ALLOC_BASE_H
#define COMPILER_OPTIMIZER_OPTIMIZATIONS_REG_ALLOC_BASE_H

#include "compiler/optimizer/ir/graph.h"
#include "optimizer/analysis/liveness_analyzer.h"
#include "libarkbase/utils/arena_containers.h"
#include "location_mask.h"

namespace ark::compiler {

void ConnectIntervals(SpillFillInst *spillFill, const LifeIntervals *src, const LifeIntervals *dst);
bool TryToSpillConstant(LifeIntervals *interval, Graph *graph);

class PANDA_PUBLIC_API RegAllocBase : public Optimization {
public:
    explicit RegAllocBase(Graph *graph);
    RegAllocBase(Graph *graph, size_t regsCount);
    RegAllocBase(Graph *graph, const RegMask &regMask, const VRegMask &vregMask, size_t slotsCount);
    RegAllocBase(Graph *graph, LocationMask mask);

    NO_MOVE_SEMANTIC(RegAllocBase);
    NO_COPY_SEMANTIC(RegAllocBase);

    ~RegAllocBase() override = default;

    bool RunImpl() override;
    const char *GetPassName() const override;
    bool AbortIfFailed() const override;

    template <typename T>
    void SetRegMask(const T &regMask)
    {
        regsMask_.Init(regMask);
    }

    LocationMask &GetRegMask()
    {
        return regsMask_;
    }

    const LocationMask &GetRegMask() const
    {
        return regsMask_;
    }

    template <typename T>
    void SetVRegMask(const T &vregMask)
    {
        vregsMask_.Init(vregMask);
    }

    LocationMask &GetVRegMask()
    {
        return vregsMask_;
    }

    const LocationMask &GetVRegMask() const
    {
        return vregsMask_;
    }

    void SetSlotsCount(size_t slotsCount)
    {
        stackMask_.Resize(slotsCount);
        stackUseLastPositions_.resize(slotsCount);
    }

    LocationMask &GetStackMask()
    {
        return stackMask_;
    }

    void ReserveTempRegisters();

    // resolve graph
    virtual bool Resolve();

protected:
    StackSlot GetNextStackSlot(LifeIntervals *interval)
    {
        return !GetStackMask().AllSet() ? GetNextStackSlotImpl(interval) : GetInvalidStackSlot();
    }

    StackSlot GetNextStackSlotImpl(LifeIntervals *interval)
    {
        auto size = GetStackMask().GetSize();
        for (size_t slot = 0; slot < size; slot++) {
            if (GetStackMask().IsSet(slot)) {
                continue;
            }

            ASSERT(slot < stackUseLastPositions_.size());
            if (stackUseLastPositions_[slot] > interval->GetBegin()) {
                continue;
            }

            GetStackMask().Set(slot);
            stackUseLastPositions_[slot] = interval->GetEnd();
            return slot;
        }
        return GetInvalidStackSlot();
    }

    void PrepareIntervals();

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
    LocationMask regsMask_;
    LocationMask vregsMask_;
    LocationMask stackMask_;
    ArenaVector<LifeNumber> stackUseLastPositions_;
};

}  // namespace ark::compiler

#endif  // COMPILER_OPTIMIZER_OPTIMIZATIONS_REG_ALLOC_BASE_H
