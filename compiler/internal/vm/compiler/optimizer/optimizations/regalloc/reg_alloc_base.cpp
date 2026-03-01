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

#include "reg_alloc_base.h"
#include "reg_type.h"
#include "split_resolver.h"
#include "spill_fills_resolver.h"
#include "reg_alloc_resolver.h"
#include "reg_alloc_stat.h"

namespace panda::compiler {

RegAllocBase::RegAllocBase(Graph *graph)
    : RegAllocBase(graph, 0, 0, MAX_NUM_STACK_SLOTS)
{
}

RegAllocBase::RegAllocBase(Graph *graph, const RegMask &reg_mask, const VRegMask &vreg_mask, size_t slots_count)
    : Optimization(graph),
      regs_mask_(graph->GetLocalAllocator()),
      vregs_mask_(graph->GetLocalAllocator()),
      stack_mask_(graph->GetLocalAllocator()),
      stack_use_last_positions_(graph->GetLocalAllocator()->Adapter())
{
    SetRegMask(reg_mask);
    SetVRegMask(vreg_mask);
    SetSlotsCount(slots_count);
}

RegAllocBase::RegAllocBase(Graph *graph, size_t regs_count)
    : Optimization(graph),
      regs_mask_(graph->GetLocalAllocator()),
      vregs_mask_(graph->GetLocalAllocator()),
      stack_mask_(graph->GetLocalAllocator()),
      stack_use_last_positions_(graph->GetLocalAllocator()->Adapter())
{
    GetRegMask().Resize(regs_count);
}

bool RegAllocBase::RunImpl()
{
    if (!Prepare()) {
        return false;
    }

    if (!Allocate()) {
        return false;
    }

    if (!Resolve()) {
        return false;
    }

    if (!Finish()) {
        return false;
    }

#ifndef NDEBUG
    RegAllocStat st(GetGraph()->GetAnalysis<LivenessAnalyzer>().GetLifeIntervals());
    COMPILER_LOG(INFO, REGALLOC) << "RegAllocated " << GetPassName() << " reg " << st.GetRegCount() << " vreg "
                                 << st.GetVRegCount() << " slots " << st.GetSlotCount() << " vslots "
                                 << st.GetVSlotCount();
#endif

    return true;
}

// Call required passes (likely will be the same)
bool RegAllocBase::Prepare()
{
    // Apply pre-allocated registers on construction
    GetGraph()->InitUsedRegs<DataType::INT64>(&GetRegMask().GetVector());
    GetGraph()->InitUsedRegs<DataType::FLOAT64>(&GetVRegMask().GetVector());
    GetGraph()->RunPass<DominatorsTree>();

    // Because linear numbers should stay unchanged from Liveness
    // pass, we have not to run any passes between. But may be ran
    // by previous try of allocation.
    bool res = GetGraph()->RunPass<LivenessAnalyzer>();
    if (GetGraph()->IsBytecodeOptimizer()) {
        GetGraph()->InitDefaultLocations();
    }
    InitIntervals();
    if (!PrepareIntervals()) {
        return false;
    }
    return res;
}

// Call resolvers (likely will be the same)
bool RegAllocBase::Resolve()
{
    if (options.IsCompilerDumpLifeIntervals()) {
        GetGraph()->GetPassManager()->DumpLifeIntervals(GetPassName());
    }

    // Save stack slot information in graph for codegen.
    GetGraph()->SetStackSlotsCount(GetTotalSlotsCount());

    // Resolve Phi and SaveState
    RegAllocResolver(GetGraph()).Resolve();

    // Connect segmented life intervals
    SplitResolver(GetGraph()).Run();

    // Resolve spill-fills overwriting
    if (GetGraph()->IsBytecodeOptimizer()) {
        auto resolver_reg = GetRegMask().GetReserved().value();
        auto regs_count = GetRegMask().GetSize();
        SpillFillsResolver(GetGraph(), resolver_reg, regs_count).Run();
    } else {
        SpillFillsResolver(GetGraph()).Run();
    }

    return true;
}

bool RegAllocBase::Finish()
{
    // Update loop info after inserting resolving blocks
    GetGraph()->RunPass<LoopAnalyzer>();
#ifndef NDEBUG
    GetGraph()->SetRegAllocApplied();
#endif  // NDEBUG
    COMPILER_LOG(DEBUG, REGALLOC) << "Regalloc " << GetPassName() << " complete";
    return true;
}

const char *RegAllocBase::GetPassName() const
{
    return "RegAllocBase";
}

bool RegAllocBase::AbortIfFailed() const
{
    return true;
}

void RegAllocBase::SetType(LifeIntervals *interval)
{
    // Skip instructions without destination register and zero-constant
    if (interval->NoDest()) {
        ASSERT(interval->GetLocation().IsInvalid());
        return;
    }
    auto type = interval->GetInst()->GetType();
    interval->SetType(ConvertRegType(GetGraph(), type));
}

void RegAllocBase::SetPreassignedRegisters(LifeIntervals *interval)
{
    auto inst = interval->GetInst();
    if (inst->GetDstReg() != INVALID_REG) {
        interval->SetPreassignedReg(inst->GetDstReg());
        return;
    }

    if (inst->GetDstLocation().IsFixedRegister() && !inst->NoDest()) {
        interval->SetPreassignedReg(inst->GetDstLocation().GetValue());
        return;
    }

    if (inst->GetOpcode() == Opcode::Parameter) {
        auto sf = inst->CastToParameter()->GetLocationData();
        if (sf.GetSrc().IsAnyRegister()) {
            auto &mask = sf.GetSrc().IsFpRegister() ? vregs_mask_ : regs_mask_;
            if (GetGraph()->GetArch() != Arch::AARCH32 || !mask.IsSet(sf.SrcValue())) {
                interval->SetPreassignedReg(sf.SrcValue());
            }
        } else if (sf.GetSrc().IsStackParameter()) {
            interval->SetLocation(sf.GetSrc());
        }
        return;
    }

    if (inst->IsZeroRegInst()) {
        interval->SetPreassignedReg(INVALID_REG);
    }
}

bool RegAllocBase::PrepareIntervals()
{
    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    for (auto interval : la.GetLifeIntervals()) {
        if (!interval->IsPhysical()) {
            [[maybe_unused]] auto inst = interval->GetInst();
            ASSERT(inst->IsPhi() || inst->IsCatchPhi() || la.GetInstByLifeNumber(interval->GetBegin()) == inst);
            SetType(interval);
            SetPreassignedRegisters(interval);
        }
        // perform implementation specific actions
        PrepareInterval(interval);
    }
    return true;
}

/**
 * Reserve one register in bytecode-optimizer mode to break cyclic spill-fills
 * dependency by 'SpillFillResolver'
 */
void RegAllocBase::ReserveTempRegisters()
{
    if (GetGraph()->IsBytecodeOptimizer()) {
        auto fixup =
            static_cast<size_t>(GetGraph()->GetRuntime()->GetMethodTotalArgumentsCount(GetGraph()->GetMethod()));
        auto reserved_bit = GetRegMask().GetSize() - 1U - fixup;
        GetRegMask().Reserve(reserved_bit);
        return;
    }

    // We don't support temp registers for arm32. Reserve stack slot instead
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        GetStackMask().Reserve(0);
    }
}

size_t RegAllocBase::GetTotalSlotsCount()
{
    if (GetGraph()->IsBytecodeOptimizer() || GetGraph()->GetMode().IsFastPath()) {
        return GetStackMask().GetUsedCount();
    }
    auto param_slots = GetGraph()->GetStackSlotsCount();
    auto spill_slots_count = GetStackMask().GetUsedCount();
    size_t lang_ext_slots = 0U;

    GetGraph()->SetExtSlotsStart(param_slots + spill_slots_count);
    COMPILER_LOG(INFO, REGALLOC) << "Call parameters slots: " << param_slots;
    COMPILER_LOG(INFO, REGALLOC) << "Spill slots: " << spill_slots_count;
    COMPILER_LOG(INFO, REGALLOC) << "Language Extension slots: " << lang_ext_slots;
    auto total_slots = RoundUp(param_slots + lang_ext_slots + spill_slots_count, 2U);
    return total_slots;
}

void ConnectIntervals(SpillFillInst *spill_fill, const LifeIntervals *src, const LifeIntervals *dst)
{
    ASSERT(spill_fill->IsSpillFill());
    spill_fill->AddSpillFill(src->GetLocation(), dst->GetLocation(), dst->GetType());

    if (dst->HasReg()) {
        dst->GetInst()->GetBasicBlock()->GetGraph()->SetRegUsage(dst->GetReg(), dst->GetType());
    }
}

}  // namespace panda::compiler
