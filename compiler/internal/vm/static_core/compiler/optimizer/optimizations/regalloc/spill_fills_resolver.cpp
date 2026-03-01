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

#include "optimizer/ir/graph.h"
#include "spill_fills_resolver.h"
#include "reg_alloc_base.h"

namespace ark::compiler {
SpillFillsResolver::SpillFillsResolver(Graph *graph)
    : SpillFillsResolver(graph, GetInvalidReg(), MAX_NUM_REGS, MAX_NUM_VREGS)
{
}

SpillFillsResolver::SpillFillsResolver(Graph *graph, Register resolver, size_t regsCount, size_t vregsCount)
    : graph_(graph),
      movesTable_(graph->GetLocalAllocator()->Adapter()),
      loadsCount_(graph->GetLocalAllocator()->Adapter()),
      preMoves_(graph->GetLocalAllocator()->Adapter()),
      postMoves_(graph->GetLocalAllocator()->Adapter()),
      resolver_(resolver),
      vregsTableOffset_(regsCount),
      slotsTableOffset_(vregsTableOffset_ + vregsCount),
      parameterSlotsOffset_(slotsTableOffset_ + graph->GetStackSlotsCount()),
      locationsCount_(parameterSlotsOffset_ + graph->GetParametersSlotsCount()),
      regWrite_(graph->GetLocalAllocator()->Adapter()),
      stackWrite_(graph->GetLocalAllocator()->Adapter())
{
    ASSERT_PRINT(std::numeric_limits<LocationIndex>::max() > locationsCount_,
                 "Summary amount of registers and slots overflow. Change LocationIndex type");

    regWrite_.resize(slotsTableOffset_);
    stackWrite_.resize(locationsCount_ - slotsTableOffset_);
    movesTable_.resize(locationsCount_);
    loadsCount_.resize(locationsCount_);
}

void SpillFillsResolver::Run()
{
    VisitGraph();
}

void SpillFillsResolver::Resolve(SpillFillInst *spillFillInst)
{
    CollectSpillFillsData(spillFillInst);
    Reorder(spillFillInst);
}

void SpillFillsResolver::ResolveIfRequired(SpillFillInst *spillFillInst)
{
    if (NeedToResolve(spillFillInst->GetSpillFills())) {
        Resolve(spillFillInst);
    }
}

Graph *SpillFillsResolver::GetGraph() const
{
    return graph_;
}

const ArenaVector<BasicBlock *> &SpillFillsResolver::GetBlocksToVisit() const
{
    return GetGraph()->GetBlocksRPO();
}

void SpillFillsResolver::VisitSpillFill(GraphVisitor *visitor, Inst *inst)
{
    auto resolver = static_cast<SpillFillsResolver *>(visitor);
    auto spillFillInst = inst->CastToSpillFill();
    if (resolver->NeedToResolve(spillFillInst->GetSpillFills())) {
        resolver->Resolve(spillFillInst);
    } else {
#ifndef NDEBUG
        // Verify spill_fill_inst
        resolver->CollectSpillFillsData(spillFillInst);
#endif
    }
}

static void MarkRegWrite(Location location, ArenaVector<bool> *regWrite, bool paired, size_t offset)
{
    auto reg = location.IsFpRegister() ? location.GetValue() + offset : location.GetValue();
    ASSERT(reg < regWrite->size());
    (*regWrite)[reg] = true;
    if (paired) {
        (*regWrite)[reg + 1] = true;
    }
}

static bool IsRegWrite(Location location, ArenaVector<bool> *regWrite, bool paired, size_t offset)
{
    auto reg = location.IsFpRegister() ? location.GetValue() + offset : location.GetValue();
    ASSERT(reg < regWrite->size());
    return (*regWrite)[reg] || (paired && (*regWrite)[reg + 1]);
}

static void MarkStackWrite(Location location, ArenaVector<bool> *stackWrite, size_t offset)
{
    auto slot = location.IsStackParameter() ? location.GetValue() + offset : location.GetValue();
    ASSERT(slot < stackWrite->size());
    (*stackWrite)[slot] = true;
}

static bool IsStackWrite(Location location, ArenaVector<bool> *stackWrite, size_t offset)
{
    auto slot = location.IsStackParameter() ? location.GetValue() + offset : location.GetValue();
    ASSERT(slot < stackWrite->size());
    return (*stackWrite)[slot];
}

/*
 * Find if there are conflicts between reading/writing registers
 */
bool SpillFillsResolver::NeedToResolve(const ArenaVector<SpillFillData> &spillFills)
{
    if (spillFills.size() < 2U) {
        return false;
    }

    std::fill(regWrite_.begin(), regWrite_.end(), false);
    std::fill(stackWrite_.begin(), stackWrite_.end(), false);
    auto paramSlotOffset = parameterSlotsOffset_ - slotsTableOffset_;

    for (const auto &sf : spillFills) {
        if (sf.DstType() == sf.SrcType() && sf.DstValue() == sf.SrcValue()) {
            continue;
        }

        bool paired = IsPairedReg(GetGraph()->GetArch(), sf.GetType());
        // set registers, that are rewrited
        if (sf.GetDst().IsAnyRegister()) {
            MarkRegWrite(sf.GetDst(), &regWrite_, paired, vregsTableOffset_);
        }

        // if register was rewrited previously - that is a conflict
        if (sf.GetSrc().IsAnyRegister()) {
            if (IsRegWrite(sf.GetSrc(), &regWrite_, paired, vregsTableOffset_)) {
                return true;
            }
        }

        // set stack slots, that are rewrited
        if (sf.DstType() == LocationType::STACK || sf.DstType() == LocationType::STACK_PARAMETER) {
            MarkStackWrite(sf.GetDst(), &stackWrite_, paramSlotOffset);
        }

        // if stack slot was rewrited previously - that is a conflict
        if (sf.SrcType() == LocationType::STACK || sf.SrcType() == LocationType::STACK_PARAMETER) {
            if (IsStackWrite(sf.GetSrc(), &stackWrite_, paramSlotOffset)) {
                return true;
            }
        }
    }
    return false;
}

/*
 * Parse spill-fills and populate `moves_table_` and `loads_count_`
 */
void SpillFillsResolver::CollectSpillFillsData(SpillFillInst *spillFillInst)
{
    std::fill(movesTable_.begin(), movesTable_.end(), MoveInfo {INVALID_LOCATION_INDEX, DataType::NO_TYPE});
    std::fill(loadsCount_.begin(), loadsCount_.end(), 0);
    preMoves_.clear();
    postMoves_.clear();

    for (const auto &sf : spillFillInst->GetSpillFills()) {
        if (sf.DstType() == sf.SrcType() && sf.DstValue() == sf.SrcValue()) {
            continue;
        }

        if (sf.SrcType() == LocationType::IMMEDIATE) {
            postMoves_.push_back(sf);
            continue;
        }

        if (sf.DstType() == LocationType::STACK_ARGUMENT) {
            preMoves_.push_back(sf);
            continue;
        }

        auto srcIndex = Map(sf.GetSrc());
        auto destIndex = Map(sf.GetDst());
        ASSERT(destIndex < locationsCount_);
        ASSERT(srcIndex < locationsCount_);
        ASSERT(movesTable_[destIndex].src == INVALID_LOCATION_INDEX);
        movesTable_[destIndex].src = srcIndex;
        movesTable_[destIndex].regType = sf.GetType();
        loadsCount_[srcIndex]++;
    }
}

/**
 * Iterate over dst-regs and add a chain of moves, containing this register, in two ways:
 * - dst-reg is NOT used as src-reg in the other spill-fills
 * - dst-reg is in the cyclically dependent chain of moves: (R1->R2, R2->R1)
 */
void SpillFillsResolver::Reorder(SpillFillInst *spillFillInst)
{
    spillFillInst->ClearSpillFills();
    ArenaVector<LocationIndex> remap(locationsCount_, INVALID_LOCATION_INDEX,
                                     GetGraph()->GetLocalAllocator()->Adapter());

    for (auto &sf : preMoves_) {
        spillFillInst->AddSpillFill(sf);
    }

    // First we process chains which have tails
    for (LocationIndex dstReg = 0; dstReg < static_cast<LocationIndex>(locationsCount_); ++dstReg) {
        if (loadsCount_[dstReg] == 0 && movesTable_[dstReg].src != INVALID_LOCATION_INDEX) {
            AddMovesChain<false>(dstReg, &remap, spillFillInst);
        }
    }

    // And than only loops should left
    for (LocationIndex dstReg = 0; dstReg < static_cast<LocationIndex>(locationsCount_); ++dstReg) {
        if (movesTable_[dstReg].src != INVALID_LOCATION_INDEX) {
            ASSERT(loadsCount_[dstReg] > 0);
            auto tempReg = CheckAndResolveCyclicDependency(dstReg);
            AddMovesChain<true>(tempReg, &remap, spillFillInst);
        }
    }

    for (auto &sf : postMoves_) {
        spillFillInst->AddSpillFill(sf);
    }
}

/**
 * Check if the chain of moves is cyclically dependent (R3->R1, R2->R3, R1->R2) and resolve it with a `temp-reg`:
 * (R1->temp, R3->R1, R2->R3, temp->R2)
 */

SpillFillsResolver::LocationIndex SpillFillsResolver::CheckAndResolveCyclicDependency(LocationIndex dstFirst)
{
    auto dstReg = dstFirst;
    auto srcReg = movesTable_[dstReg].src;

    [[maybe_unused]] size_t movesCounter = 0;
    while (srcReg != dstFirst) {
        dstReg = srcReg;
        srcReg = movesTable_[dstReg].src;
        ASSERT(srcReg != INVALID_LOCATION_INDEX);
        ASSERT_PRINT(movesCounter++ < movesTable_.size(), "Unresolved cyclic dependency");
    }

    auto resolver = GetResolver(movesTable_[dstFirst].regType);
    movesTable_[resolver].src = dstFirst;
    movesTable_[dstReg].src = resolver;
    loadsCount_[resolver]++;
    movesTable_[resolver].regType = movesTable_[dstReg].regType;
    return resolver;
}

/**
 * Add a chain of `spill_fills`, which starts with a `dst` register:
 * [src_0 -> dst],
 * [src_1 -> src_0],
 * [src_2 -> src_1], etc
 * A chain finishes with an `INVALID_LOCATION_INDEX`
 *
 * After chain building remap the remaining moves with a new sources
 */
template <bool IS_CYCLIC>
void SpillFillsResolver::AddMovesChain(LocationIndex dst, ArenaVector<LocationIndex> *remap,
                                       SpillFillInst *spillFillInst)
{
    [[maybe_unused]] auto firstDst = dst;
    ASSERT(firstDst != INVALID_LOCATION_INDEX);
    ASSERT(remap->at(firstDst) == INVALID_LOCATION_INDEX);

    auto src = movesTable_[dst].src;
    [[maybe_unused]] auto firstSrc = src;
    ASSERT(firstSrc != INVALID_LOCATION_INDEX);

    // Make a chain of spill-fills
    while (src != INVALID_LOCATION_INDEX) {
        auto re = remap->at(src);
        auto type = movesTable_[dst].regType;
        if (re == INVALID_LOCATION_INDEX) {
            spillFillInst->AddSpillFill(ToLocation(src), ToLocation(dst), type);
            remap->at(src) = dst;
        } else {
            spillFillInst->AddSpillFill(ToLocation(re), ToLocation(dst), type);
        }
        ASSERT(loadsCount_[src] > 0);
        loadsCount_[src]--;
        movesTable_[dst].src = INVALID_LOCATION_INDEX;
        dst = src;
        src = movesTable_[dst].src;
    }

    // Fixup temp register remapping
    // NOLINTNEXTLINE(readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (IS_CYCLIC) {
        ASSERT(dst == firstDst);
        auto re = remap->at(firstDst);
        ASSERT(re != INVALID_LOCATION_INDEX);
        remap->at(firstSrc) = re;
        remap->at(firstDst) = INVALID_LOCATION_INDEX;
    }
}

SpillFillsResolver::LocationIndex SpillFillsResolver::GetResolver(DataType::Type type)
{
    // There is a preassigned resolver
    if (resolver_ != GetInvalidReg()) {
        ASSERT(!DataType::IsFloatType(type));
        GetGraph()->SetRegUsage(resolver_, type);
        return resolver_;
    }

    // There are no temp registers in the Arch::AARCH32, use stack slot to resolve
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        return Map(Location::MakeStackSlot(0));
    }

    if (DataType::IsFloatType(type)) {
        auto resolverReg = GetGraph()->GetArchTempVReg();
        ASSERT(resolverReg != GetInvalidReg());
        return resolverReg + vregsTableOffset_;
    }

    auto resolverReg = GetGraph()->GetArchTempReg();
    ASSERT(resolverReg != GetInvalidReg());
    return resolverReg;
}

}  // namespace ark::compiler
