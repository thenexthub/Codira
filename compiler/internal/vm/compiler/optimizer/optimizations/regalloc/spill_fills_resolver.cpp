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
#include "spill_fills_resolver.h"

namespace panda::compiler {
SpillFillsResolver::SpillFillsResolver(Graph *graph)
    : SpillFillsResolver(graph, INVALID_REG, MAX_NUM_REGS, MAX_NUM_VREGS)
{
}

SpillFillsResolver::SpillFillsResolver(Graph *graph, Register resolver, size_t regs_count, size_t vregs_count)
    : graph_(graph),
      moves_table_(graph->GetLocalAllocator()->Adapter()),
      loads_count_(graph->GetLocalAllocator()->Adapter()),
      pre_moves_(graph->GetLocalAllocator()->Adapter()),
      post_moves_(graph->GetLocalAllocator()->Adapter()),
      resolver_(resolver),
      VREGS_TABLE_OFFSET(regs_count),
      SLOTS_TABLE_OFFSET(VREGS_TABLE_OFFSET + vregs_count),
      PARAMETER_SLOTS_OFFSET(SLOTS_TABLE_OFFSET + graph->GetStackSlotsCount()),
      LOCATIONS_COUNT(PARAMETER_SLOTS_OFFSET + graph->GetParametersSlotsCount()),
      reg_write_(graph->GetLocalAllocator()->Adapter()),
      stack_write_(graph->GetLocalAllocator()->Adapter())
{
    ASSERT_PRINT(std::numeric_limits<LocationIndex>::max() > LOCATIONS_COUNT,
                 "Summary amount of registers and slots overflow. Change LocationIndex type");

    reg_write_.resize(SLOTS_TABLE_OFFSET);
    stack_write_.resize(LOCATIONS_COUNT - SLOTS_TABLE_OFFSET);
    moves_table_.resize(LOCATIONS_COUNT);
    loads_count_.resize(LOCATIONS_COUNT);
}

void SpillFillsResolver::Run()
{
    VisitGraph();
}

void SpillFillsResolver::Resolve(SpillFillInst *spill_fill_inst)
{
    CollectSpillFillsData(spill_fill_inst);
    Reorder(spill_fill_inst);
}

void SpillFillsResolver::ResolveIfRequired(SpillFillInst *spill_fill_inst)
{
    if (NeedToResolve(spill_fill_inst->GetSpillFills())) {
        Resolve(spill_fill_inst);
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
    auto spill_fill_inst = inst->CastToSpillFill();
    if (resolver->NeedToResolve(spill_fill_inst->GetSpillFills())) {
        resolver->Resolve(spill_fill_inst);
    } else {
#ifndef NDEBUG
        // Verify spill_fill_inst
        resolver->CollectSpillFillsData(spill_fill_inst);
#endif
    }
}

static void MarkRegWrite(Location location, ArenaVector<bool> *reg_write, bool paired, size_t offset)
{
    auto reg = location.IsFpRegister() ? location.GetValue() + offset : location.GetValue();
    ASSERT(reg < reg_write->size());
    (*reg_write)[reg] = true;
    if (paired) {
        (*reg_write)[reg + 1] = true;
    }
}

static bool IsRegWrite(Location location, ArenaVector<bool> *reg_write, bool paired, size_t offset)
{
    auto reg = location.IsFpRegister() ? location.GetValue() + offset : location.GetValue();
    ASSERT(reg < reg_write->size());
    return (*reg_write)[reg] || (paired && (*reg_write)[reg + 1]);
}

static void MarkStackWrite(Location location, ArenaVector<bool> *stack_write, size_t offset)
{
    auto slot = location.IsStackParameter() ? location.GetValue() + offset : location.GetValue();
    ASSERT(slot < stack_write->size());
    (*stack_write)[slot] = true;
}

static bool IsStackWrite(Location location, ArenaVector<bool> *stack_write, size_t offset)
{
    auto slot = location.IsStackParameter() ? location.GetValue() + offset : location.GetValue();
    ASSERT(slot < stack_write->size());
    return (*stack_write)[slot];
}

/*
 * Find if there are conflicts between reading/writing registers
 */
bool SpillFillsResolver::NeedToResolve(const ArenaVector<SpillFillData> &spill_fills)
{
    if (spill_fills.size() < 2U) {
        return false;
    }

    std::fill(reg_write_.begin(), reg_write_.end(), false);
    std::fill(stack_write_.begin(), stack_write_.end(), false);
    auto param_slot_offset = PARAMETER_SLOTS_OFFSET - SLOTS_TABLE_OFFSET;

    for (const auto &sf : spill_fills) {
        if (sf.DstType() == sf.SrcType() && sf.DstValue() == sf.SrcValue()) {
            continue;
        }

        bool paired = IsPairedReg(GetGraph()->GetArch(), sf.GetType());
        // set registers, that are rewrited
        if (sf.GetDst().IsAnyRegister()) {
            MarkRegWrite(sf.GetDst(), &reg_write_, paired, VREGS_TABLE_OFFSET);
        }

        // if register was rewrited previously - that is a conflict
        if (sf.GetSrc().IsAnyRegister()) {
            if (IsRegWrite(sf.GetSrc(), &reg_write_, paired, VREGS_TABLE_OFFSET)) {
                return true;
            }
        }

        // set stack slots, that are rewrited
        if (sf.DstType() == LocationType::STACK || sf.DstType() == LocationType::STACK_PARAMETER) {
            MarkStackWrite(sf.GetDst(), &stack_write_, param_slot_offset);
        }

        // if stack slot was rewrited previously - that is a conflict
        if (sf.SrcType() == LocationType::STACK || sf.SrcType() == LocationType::STACK_PARAMETER) {
            if (IsStackWrite(sf.GetSrc(), &stack_write_, param_slot_offset)) {
                return true;
            }
        }
    }
    return false;
}

/*
 * Parse spill-fills and populate `moves_table_` and `loads_count_`
 */
void SpillFillsResolver::CollectSpillFillsData(SpillFillInst *spill_fill_inst)
{
    std::fill(moves_table_.begin(), moves_table_.end(), MoveInfo {INVALID_LOCATION_INDEX, DataType::NO_TYPE});
    std::fill(loads_count_.begin(), loads_count_.end(), 0);
    pre_moves_.clear();
    post_moves_.clear();

    for (const auto &sf : spill_fill_inst->GetSpillFills()) {
        if (sf.DstType() == sf.SrcType() && sf.DstValue() == sf.SrcValue()) {
            continue;
        }

        if (sf.SrcType() == LocationType::IMMEDIATE) {
            post_moves_.push_back(sf);
            continue;
        }

        if (sf.DstType() == LocationType::STACK_ARGUMENT) {
            pre_moves_.push_back(sf);
            continue;
        }

        auto src_index = Map(sf.GetSrc());
        auto dest_index = Map(sf.GetDst());
        ASSERT(dest_index < LOCATIONS_COUNT);
        ASSERT(src_index < LOCATIONS_COUNT);
        ASSERT(moves_table_[dest_index].src == INVALID_LOCATION_INDEX);
        moves_table_[dest_index].src = src_index;
        moves_table_[dest_index].reg_type = sf.GetType();
        loads_count_[src_index]++;
    }
}

/**
 * Iterate over dst-regs and add a chain of moves, containing this register, in two ways:
 * - dst-reg is NOT used as src-reg in the other spill-fills
 * - dst-reg is in the cyclically dependent chain of moves: (R1->R2, R2->R1)
 */
void SpillFillsResolver::Reorder(SpillFillInst *spill_fill_inst)
{
    spill_fill_inst->ClearSpillFills();
    ArenaVector<LocationIndex> remap(LOCATIONS_COUNT, INVALID_LOCATION_INDEX,
                                     GetGraph()->GetLocalAllocator()->Adapter());

    for (auto &sf : pre_moves_) {
        spill_fill_inst->AddSpillFill(sf);
    }

    // First we process chains which have tails
    for (LocationIndex dst_reg = 0; dst_reg < static_cast<LocationIndex>(LOCATIONS_COUNT); ++dst_reg) {
        if (loads_count_[dst_reg] == 0 && moves_table_[dst_reg].src != INVALID_LOCATION_INDEX) {
            AddMovesChain<false>(dst_reg, &remap, spill_fill_inst);
        }
    }

    // And than only loops should left
    for (LocationIndex dst_reg = 0; dst_reg < static_cast<LocationIndex>(LOCATIONS_COUNT); ++dst_reg) {
        if (moves_table_[dst_reg].src != INVALID_LOCATION_INDEX) {
            ASSERT(loads_count_[dst_reg] > 0);
            auto temp_reg = CheckAndResolveCyclicDependency(dst_reg);
            AddMovesChain<true>(temp_reg, &remap, spill_fill_inst);
        }
    }

    for (auto &sf : post_moves_) {
        spill_fill_inst->AddSpillFill(sf);
    }
}

/**
 * Check if the chain of moves is cyclically dependent (R3->R1, R2->R3, R1->R2) and resolve it with a `temp-reg`:
 * (R1->temp, R3->R1, R2->R3, temp->R2)
 */

SpillFillsResolver::LocationIndex SpillFillsResolver::CheckAndResolveCyclicDependency(LocationIndex dst_first)
{
    auto dst_reg = dst_first;
    auto src_reg = moves_table_[dst_reg].src;

    [[maybe_unused]] size_t moves_counter = 0;
    while (src_reg != dst_first) {
        dst_reg = src_reg;
        src_reg = moves_table_[dst_reg].src;
        ASSERT(src_reg != INVALID_LOCATION_INDEX);
        ASSERT_PRINT(moves_counter++ < moves_table_.size(), "Unresolved cyclic dependency");
    }

    auto resolver = GetResolver(moves_table_[dst_first].reg_type);
    moves_table_[resolver].src = dst_first;
    moves_table_[dst_reg].src = resolver;
    loads_count_[resolver]++;
    moves_table_[resolver].reg_type = moves_table_[dst_reg].reg_type;
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
template <bool is_cyclic>
void SpillFillsResolver::AddMovesChain(LocationIndex dst, ArenaVector<LocationIndex> *remap,
                                       SpillFillInst *spill_fill_inst)
{
    [[maybe_unused]] auto first_dst = dst;
    ASSERT(first_dst != INVALID_LOCATION_INDEX);
    ASSERT(remap->at(first_dst) == INVALID_LOCATION_INDEX);

    auto src = moves_table_[dst].src;
    [[maybe_unused]] auto first_src = src;
    ASSERT(first_src != INVALID_LOCATION_INDEX);

    // Make a chain of spill-fills
    while (src != INVALID_LOCATION_INDEX) {
        auto re = remap->at(src);
        auto type = moves_table_[dst].reg_type;
        if (re == INVALID_LOCATION_INDEX) {
            spill_fill_inst->AddSpillFill(ToLocation(src), ToLocation(dst), type);
            remap->at(src) = dst;
        } else {
            spill_fill_inst->AddSpillFill(ToLocation(re), ToLocation(dst), type);
        }
        ASSERT(loads_count_[src] > 0);
        loads_count_[src]--;
        moves_table_[dst].src = INVALID_LOCATION_INDEX;
        dst = src;
        src = moves_table_[dst].src;
    }

    // Fixup temp register remapping
    // NOLINTNEXTLINE(readability-braces-around-statements,bugprone-suspicious-semicolon)
    if constexpr (is_cyclic) {
        ASSERT(dst == first_dst);
        auto re = remap->at(first_dst);
        ASSERT(re != INVALID_LOCATION_INDEX);
        remap->at(first_src) = re;
        remap->at(first_dst) = INVALID_LOCATION_INDEX;
    }
}

SpillFillsResolver::LocationIndex SpillFillsResolver::GetResolver(DataType::Type type)
{
    // There is a preassigned resolver
    if (resolver_ != INVALID_REG) {
        ASSERT(!DataType::IsFloatType(type));
        GetGraph()->SetRegUsage(resolver_, type);
        return resolver_;
    }
    // There are no temp registers in the Arch::AARCH32, use stack slot to resolve
    if (GetGraph()->GetArch() == Arch::AARCH32) {
        return Map(Location::MakeStackSlot(0));
    }
    if (DataType::IsFloatType(type)) {
        return VREGS_TABLE_OFFSET;
    }
    return 0;
}
}  // namespace panda::compiler
