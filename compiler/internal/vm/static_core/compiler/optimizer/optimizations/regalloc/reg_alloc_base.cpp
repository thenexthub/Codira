/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "reg_alloc_base.h"
#include "reg_type.h"
#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/datatype.h"
#include "optimizer/ir/graph.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/optimizations/locations_builder.h"
#include "split_resolver.h"
#include "spill_fills_resolver.h"
#include "reg_alloc_resolver.h"
#include "reg_alloc_stat.h"

namespace ark::compiler {

RegAllocBase::RegAllocBase(Graph *graph)
    : RegAllocBase(graph, graph->GetArchUsedRegs(), graph->GetArchUsedVRegs(), GetMaxNumStackSlots())
{
}

RegAllocBase::RegAllocBase(Graph *graph, const RegMask &regMask, const VRegMask &vregMask, size_t slotsCount)
    : Optimization(graph),
      regsMask_(graph->GetLocalAllocator()),
      vregsMask_(graph->GetLocalAllocator()),
      stackMask_(graph->GetLocalAllocator()),
      stackUseLastPositions_(graph->GetLocalAllocator()->Adapter())
{
    SetRegMask(regMask);
    SetVRegMask(vregMask);
    SetSlotsCount(slotsCount);
}

RegAllocBase::RegAllocBase(Graph *graph, size_t regsCount)
    : Optimization(graph),
      regsMask_(graph->GetLocalAllocator()),
      vregsMask_(graph->GetLocalAllocator()),
      stackMask_(graph->GetLocalAllocator()),
      stackUseLastPositions_(graph->GetLocalAllocator()->Adapter())
{
    GetRegMask().Resize(regsCount);
}

RegAllocBase::RegAllocBase(Graph *graph, LocationMask mask)
    : Optimization(graph),
      regsMask_(std::move(mask)),
      vregsMask_(graph->GetLocalAllocator()),
      stackMask_(graph->GetLocalAllocator()),
      stackUseLastPositions_(graph->GetLocalAllocator()->Adapter())
{
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
    // Set rzero is used for dynamic mask
    if (auto rzero = GetGraph()->GetZeroReg(); rzero != GetInvalidReg()) {
        GetRegMask().Set(rzero);
    }

    // Apply pre-allocated registers on construction
    GetGraph()->InitUsedRegs<DataType::INT64>(&GetRegMask().GetVector());
    GetGraph()->InitUsedRegs<DataType::FLOAT64>(&GetVRegMask().GetVector());

    GetGraph()->RunPass<DominatorsTree>();

    // Because linear numbers should stay unchanged from Liveness
    // pass, we have not to run any passes between. But may be ran
    // by previous try of allocation.
    if (!GetGraph()->RunPass<LivenessAnalyzer>()) {
        return false;
    }
    if (GetGraph()->IsBytecodeOptimizer()) {
        GetGraph()->InitDefaultLocations();
    }
    InitIntervals();
    PrepareIntervals();
    return true;
}

// Call resolvers (likely will be the same)
bool RegAllocBase::Resolve()
{
    if (g_options.IsCompilerDumpLifeIntervals()) {
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
        auto resolverReg = GetRegMask().GetReserved().value();
        auto regsCount = GetRegMask().GetSize();
        SpillFillsResolver(GetGraph(), resolverReg, regsCount).Run();
    } else {
        SpillFillsResolver(GetGraph()).Run();
    }

#ifndef NDEBUG
    if (!GetGraph()->IsBytecodeOptimizer() && g_options.IsCompilerVerifyRegalloc() &&
        !GetGraph()->RunPass<RegAllocVerifier>()) {
        LOG(FATAL, COMPILER) << "Regalloc verification failed";
    }
#endif  // NDEBUG
    return true;
}

bool RegAllocBase::Finish()
{
    // Update loop info after inserting resolving blocks
    GetGraph()->RunPass<LoopAnalyzer>();
#ifdef COMPILER_DEBUG_CHECKS
    GetGraph()->SetRegAllocApplied();
#endif  // COMPILER_DEBUG_CHECKS
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
    if (inst->GetDstReg() != GetInvalidReg()) {
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
            auto &mask = sf.GetSrc().IsFpRegister() ? vregsMask_ : regsMask_;
            if (GetGraph()->GetArch() != Arch::AARCH32 || !mask.IsSet(sf.SrcValue())) {
                interval->SetPreassignedReg(sf.SrcValue());
            }
        } else if (sf.GetSrc().IsStackParameter()) {
            interval->SetLocation(sf.GetSrc());
        }
        return;
    }

    if (inst->IsZeroRegInst()) {
        interval->SetPreassignedReg(GetGraph()->GetZeroReg());
    }
}

void RegAllocBase::PrepareIntervals()
{
    auto &la = GetGraph()->GetAnalysis<LivenessAnalyzer>();
    for (auto interval : la.GetLifeIntervals()) {
        if (interval->HasInst()) {
            [[maybe_unused]] auto inst = interval->GetInst();
            ASSERT(inst->IsPhi() || inst->IsCatchPhi() || la.GetInstByLifeNumber(interval->GetBegin()) == inst ||
                   (IsPseudoUserOfMultiOutput(inst) &&
                    la.GetInstByLifeNumber(interval->GetBegin()) == inst->GetInput(0).GetInst()));
            SetType(interval);
            SetPreassignedRegisters(interval);
        }
        // perform implementation specific actions
        PrepareInterval(interval);
    }
}

/**
 * Reserve one register in bytecode-optimizer mode to break cyclic spill-fills
 * dependency by 'SpillFillResolver'
 */
void RegAllocBase::ReserveTempRegisters()
{
    if (GetGraph()->IsBytecodeOptimizer()) {
        auto fixup =
            static_cast<int32_t>(GetGraph()->GetRuntime()->GetMethodTotalArgumentsCount(GetGraph()->GetMethod()));
        auto reservedBit = GetRegMask().GetSize() - 1 - fixup;
        GetRegMask().Reserve(reservedBit);
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
    auto paramSlots = GetGraph()->GetStackSlotsCount();
    auto spillSlotsCount = GetStackMask().GetUsedCount();
    size_t langExtSlots = 0U;
    if (GetGraph()->GetArch() != Arch::NONE) {
        langExtSlots = GetGraph()->GetRuntime()->GetLanguageExtensionSize(GetGraph()->GetArch()) /
                       PointerSize(GetGraph()->GetArch());
        // We reserve space in frame for currect method and all inlined methods
        langExtSlots *= GetGraph()->GetMaxInliningDepth() + 1;
    }

    // language extension slots lies after spill slots
    GetGraph()->SetExtSlotsStart(spillSlotsCount);
    COMPILER_LOG(INFO, REGALLOC) << "Call parameters slots: " << paramSlots;
    COMPILER_LOG(INFO, REGALLOC) << "Spill slots: " << spillSlotsCount;
    COMPILER_LOG(INFO, REGALLOC) << "Language Extension slots: " << langExtSlots;
    auto totalSlots = RoundUp(paramSlots + langExtSlots + spillSlotsCount, 2U);
    return totalSlots;
}

void ConnectIntervals(SpillFillInst *spillFill, const LifeIntervals *src, const LifeIntervals *dst)
{
    ASSERT(spillFill->IsSpillFill());
    spillFill->AddSpillFill(src->GetLocation(), dst->GetLocation(), dst->GetType());

    if (dst->HasReg()) {
        dst->GetInst()->GetBasicBlock()->GetGraph()->SetRegUsage(dst->GetReg(), dst->GetType());
    }
}

bool TryToSpillConstant(LifeIntervals *interval, Graph *graph)
{
    auto inst = interval->GetInst();
    ASSERT(inst != nullptr);
    if (!inst->IsConst() || graph->IsBytecodeOptimizer() || !g_options.IsCompilerRematConst()) {
        return false;
    }
    auto immSlot = graph->AddSpilledConstant(inst->CastToConstant());
    if (immSlot == GetInvalidImmTableSlot()) {
        return false;
    }
    interval->SetLocation(Location::MakeConstant(immSlot));
    COMPILER_LOG(DEBUG, REGALLOC) << interval->GetLocation().ToString(graph->GetArch())
                                  << " was assigned to the interval " << interval->ToString();
    return true;
}

}  // namespace ark::compiler
