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

#include "optimizer/ir/basicblock.h"
#include "optimizer/ir/graph.h"
#include "liveness_analyzer.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/loop_analyzer.h"

namespace panda::compiler {
LivenessAnalyzer::LivenessAnalyzer(Graph *graph)
    : Analysis(graph),
      allocator_(graph->GetAllocator()),
      linear_blocks_(graph->GetAllocator()->Adapter()),
      inst_life_numbers_(graph->GetAllocator()->Adapter()),
      inst_life_intervals_(graph->GetAllocator()->Adapter()),
      insts_by_life_number_(graph->GetAllocator()->Adapter()),
      block_live_ranges_(graph->GetAllocator()->Adapter()),
      block_live_sets_(graph->GetLocalAllocator()->Adapter()),
      pending_catch_phi_inputs_(graph->GetAllocator()->Adapter()),
      physical_general_intervals_(graph->GetAllocator()->Adapter()),
      physical_vector_intervals_(graph->GetAllocator()->Adapter())
{
}

bool LivenessAnalyzer::RunImpl()
{
    GetGraph()->RunPass<DominatorsTree>();
    GetGraph()->RunPass<LoopAnalyzer>();
    ResetLiveness();
    BuildBlocksLinearOrder();
    BuildInstLifeNumbers();
    BuildInstLifeIntervals();
    if (!pending_catch_phi_inputs_.empty()) {
        COMPILER_LOG(ERROR, LIVENESS_ANALYZER)
            << "Graph contains CatchPhi instructions whose inputs were not processed";
        return false;
    }
    std::copy_if(physical_general_intervals_.begin(), physical_general_intervals_.end(),
                 std::back_inserter(inst_life_intervals_), [](auto li) { return li != nullptr; });
    std::copy_if(physical_vector_intervals_.begin(), physical_vector_intervals_.end(),
                 std::back_inserter(inst_life_intervals_), [](auto li) { return li != nullptr; });
    COMPILER_LOG(DEBUG, LIVENESS_ANALYZER) << "Liveness analysis is constructed";
    return true;
}

void LivenessAnalyzer::ResetLiveness()
{
    inst_life_numbers_.clear();
    inst_life_intervals_.clear();
    block_live_sets_.clear();
    block_live_ranges_.clear();
    physical_general_intervals_.clear();
    physical_vector_intervals_.clear();
    if (GetGraph()->GetArch() != Arch::NONE) {
        physical_general_intervals_.resize(REGISTERS_NUM);
        physical_vector_intervals_.resize(VREGISTERS_NUM);
    }
}

/*
 * Linear blocks order means:
 * - all dominators of a block are visiting before this block;
 * - all blocks belonging to the same loop are contiguous;
 */
void LivenessAnalyzer::BuildBlocksLinearOrder()
{
    ASSERT_PRINT(GetGraph()->IsAnalysisValid<DominatorsTree>(), "Liveness Analyzer needs valid Dom Tree");
    auto size = GetGraph()->GetBlocksRPO().size();
    linear_blocks_.reserve(size);
    linear_blocks_.clear();
    marker_ = GetGraph()->NewMarker();
    ASSERT_PRINT(marker_ != UNDEF_MARKER, "There are no free markers");
    if (GetGraph()->IsBytecodeOptimizer() && !GetGraph()->GetTryBeginBlocks().empty()) {
        LinearizeBlocks<true>();
    } else {
        LinearizeBlocks<false>();
    }
    ASSERT(linear_blocks_.size() == size);
    GetGraph()->EraseMarker(marker_);
    ASSERT_PRINT(CheckLinearOrder(), "Linear block order isn't correct");

    block_live_sets_.resize(GetGraph()->GetVectorBlocks().size());
    block_live_ranges_.resize(GetGraph()->GetVectorBlocks().size());
}

/*
 * Check if all forward edges of loop header were visited to get the resulting block order in RPO.
 * Predecessors which are not in the same loop with a header - are forward edges.
 */
bool LivenessAnalyzer::AllForwardEdgesVisited(BasicBlock *block)
{
    if (!block->IsLoopHeader()) {
        for (auto pred : block->GetPredsBlocks()) {
            if (!pred->IsMarked(marker_)) {
                return false;
            }
        }
    } else {
        // Head of irreducible loop can not dominate other blocks in the loop
        if (block->GetLoop()->IsIrreducible()) {
            return true;
        }
        // Predecessors which are not dominated - are forward edges,
        for (auto pred : block->GetPredsBlocks()) {
            if (!block->IsDominate(pred) && !pred->IsMarked(marker_)) {
                return false;
            }
        }
    }
    return true;
}

template <bool use_pc_order>
void LivenessAnalyzer::LinearizeBlocks()
{
    ArenaList<BasicBlock *> pending {GetGraph()->GetLocalAllocator()->Adapter()};
    pending.push_back(GetGraph()->GetStartBlock());

    while (!pending.empty()) {
        auto current = pending.front();
        pending.pop_front();

        linear_blocks_.push_back(current);
        current->SetMarker(marker_);

        auto succs = current->GetSuccsBlocks();
        // Each block is inserted into pending list before all already inserted blocks
        // from the same loop. To process edges forwarding to a "true" branches successors
        // should be processed in reversed order.
        for (auto it = succs.rbegin(); it != succs.rend(); ++it) {
            auto succ = *it;
            if (succ->IsMarked(marker_) || !AllForwardEdgesVisited(succ)) {
                continue;
            }

            if constexpr (use_pc_order) {  // NOLINT(readability-braces-around-statements)
                auto pc_compare = [succ](auto block) { return block->GetGuestPc() > succ->GetGuestPc(); };
                auto insert_before = std::find_if(pending.begin(), pending.end(), pc_compare);
                pending.insert(insert_before, succ);
            } else {  // NOLINT(readability-misleading-indentation)
                // Insert successor right before a block from the same loop, outer loop's block
                // or before a block from the root loop.
                // Such ordering guarantee that a loop and all it's inner loops will be processed
                // before following edges leading to outer loop blocks.
                auto is_same_or_inner_loop = [succ](auto block) {
                    return succ->GetLoop() == block->GetLoop() || block->GetLoop()->IsRoot() ||
                           succ->GetLoop()->IsInside(block->GetLoop());
                };
                auto insert_before = std::find_if(pending.begin(), pending.end(), is_same_or_inner_loop);
                pending.insert(insert_before, succ);
            }
        }
    }
}

/*
 * Check linear order correctness, using dominators tree
 */
bool LivenessAnalyzer::CheckLinearOrder()
{
    ArenaVector<size_t> block_pos(GetGraph()->GetVectorBlocks().size(), 0, GetAllocator()->Adapter());
    size_t position = 0;
    for (auto block : linear_blocks_) {
        block_pos[block->GetId()] = position++;
    }

    for (auto block : linear_blocks_) {
        if (block->GetDominator() != nullptr) {
            ASSERT_PRINT(block_pos[block->GetDominator()->GetId()] < block_pos[block->GetId()],
                         "Each block should be visited after its dominator");
        }
        if (!block->IsTryEnd()) {
            continue;
        }
        ASSERT(block->GetTryId() != INVALID_ID);
        for (auto bb : linear_blocks_) {
            if (bb->IsTry() && bb->GetTryId() == block->GetTryId()) {
                ASSERT_PRINT(block_pos[bb->GetId()] < block_pos[block->GetId()],
                             "Each try-block should be visited before its try-end block");
            }
        }
    }

    return true;
}

/*
 * Set lifetime number and visiting number for each instruction
 */
void LivenessAnalyzer::BuildInstLifeNumbers()
{
    LifeNumber block_begin;
    LifeNumber life_number = 0;
    LinearNumber linear_number = 0;

    for (auto block : GetLinearizedBlocks()) {
        block_begin = life_number;
        // set the same number for each phi in the block
        for (auto phi : block->PhiInsts()) {
            phi->SetLinearNumber(linear_number++);
            SetInstLifeNumber(phi, life_number);
            CreateLifeIntervals(phi);
        }
        // ignore PHI instructions
        insts_by_life_number_.push_back(nullptr);
        // set a unique number for each instruction in the block, differing by 2
        // for the reason of adding spill/fill instructions
        for (auto inst : block->Insts()) {
            inst->SetLinearNumber(linear_number++);
            CreateLifeIntervals(inst);
            life_number += LIFE_NUMBER_GAP;
            SetInstLifeNumber(inst, life_number);
            insts_by_life_number_.push_back(inst);
        }
        life_number += LIFE_NUMBER_GAP;
        SetBlockLiveRange(block, {block_begin, life_number});
    }
}

/*
 * Get lifetime intervals for each instruction
 */
void LivenessAnalyzer::BuildInstLifeIntervals()
{
    for (auto it = GetLinearizedBlocks().rbegin(); it != GetLinearizedBlocks().rend(); it++) {
        auto block = *it;
        auto live_set = GetInitInstLiveSet(block);
        ProcessBlockLiveInstructions(block, live_set);
    }
}

/*
 * The initial set of live instructions in the `block` is the the union of all live instructions at the beginning of
 * the block's successors.
 * Also for each phi-instruction of the successors: input corresponding to the `block` is added to the live set.
 */
InstLiveSet *LivenessAnalyzer::GetInitInstLiveSet(BasicBlock *block)
{
    unsigned instruction_count = inst_life_intervals_.size();
    auto live_set = GetAllocator()->New<InstLiveSet>(instruction_count, GetAllocator());
    CHECK_NOT_NULL(live_set);
    for (auto succ : block->GetSuccsBlocks()) {
        // catch-begin is pseudo successor, its live set will be processed for blocks with throwable instructions
        if (succ->IsCatchBegin()) {
            continue;
        }
        live_set->Union(GetBlockLiveSet(succ));
        for (auto phi : succ->PhiInsts()) {
            auto phi_input = phi->CastToPhi()->GetPhiDataflowInput(block);
            live_set->Add(phi_input->GetLinearNumber());
        }
    }

    // if basic block contains throwable instruction, all instucrions live in the catch-handlers should be live in this
    // block
    if (auto inst = block->GetFistThrowableInst(); inst != nullptr) {
        auto handlers = GetGraph()->GetThrowableInstHandlers(inst);
        for (auto catch_handler : handlers) {
            live_set->Union(GetBlockLiveSet(catch_handler));
        }
    }
    return live_set;
}

LifeNumber LivenessAnalyzer::GetLoopEnd(Loop *loop)
{
    LifeNumber loop_end = 0;
    // find max LifeNumber of inner loops
    for (auto inner : loop->GetInnerLoops()) {
        loop_end = std::max(loop_end, GetLoopEnd(inner));
    }
    // find max LifeNumber of back_edges
    for (auto back_edge : loop->GetBackEdges()) {
        loop_end = std::max(loop_end, GetBlockLiveRange(back_edge).GetEnd());
    }
    return loop_end;
}

/*
 * Append and adjust the lifetime intervals for each instruction live in the block and for their inputs
 */
void LivenessAnalyzer::ProcessBlockLiveInstructions(BasicBlock *block, InstLiveSet *live_set)
{
    // For each live instruction set initial life range equals to the block life range
    for (auto &interval : inst_life_intervals_) {
        if (live_set->IsSet(interval->GetInst()->GetLinearNumber())) {
            interval->AppendRange(GetBlockLiveRange(block));
        }
    }

    for (Inst *inst : block->InstsSafeReverse()) {
        // Shorten instruction lifetime to the position where its defined
        // and remove from the block's live set
        auto inst_life_number = GetInstLifeNumber(inst);
        auto interval = GetInstLifeIntervals(inst);
        interval->StartFrom(inst_life_number);
        live_set->Remove(inst->GetLinearNumber());
        if (inst->IsCatchPhi()) {
            // catch-phi's liveness should overlap all linked try blocks' livenesses
            for (auto pred : inst->GetBasicBlock()->GetPredsBlocks()) {
                inst_life_number = std::min(inst_life_number, GetBlockLiveRange(pred).GetBegin());
            }
            interval->StartFrom(inst_life_number);
            AdjustCatchPhiInputsLifetime(inst);
        } else {
            auto current_live_range = LiveRange {GetBlockLiveRange(block).GetBegin(), inst_life_number};
            AdjustInputsLifetime(inst, current_live_range, live_set);
        }
    }

    // The lifetime interval of phis instructions starts at the beginning of the block
    for (auto phi : block->PhiInsts()) {
        live_set->Remove(phi->GetLinearNumber());
    }

    // All instructions live at the beginning ot the loop header
    // must be live for the entire loop
    if (block->IsLoopHeader()) {
        LifeNumber loop_end_position = GetLoopEnd(block->GetLoop());

        for (auto &interval : inst_life_intervals_) {
            if (live_set->IsSet(interval->GetInst()->GetLinearNumber())) {
                interval->AppendGroupRange({GetBlockLiveRange(block).GetBegin(), loop_end_position});
            }
        }
    }
    SetBlockLiveSet(block, live_set);
}

/* static */
LiveRange LivenessAnalyzer::GetPropagatedLiveRange(Inst *inst, LiveRange live_range)
{
    /*
     * We need to propagate liveness for instruction with CallRuntime to save registers before call;
     * Otherwise, we will not be able to restore the value of the virtual registers
     */
    if (inst->IsPropagateLiveness()) {
        live_range.SetEnd(live_range.GetEnd() + 1);
    }
    return live_range;
}

/*
 * Adjust instruction inputs lifetime and add them to the block's live set
 */
void LivenessAnalyzer::AdjustInputsLifetime(Inst *inst, LiveRange live_range, InstLiveSet *live_set)
{
    for (auto input : inst->GetInputs()) {
        auto input_inst = inst->GetDataFlowInput(input.GetInst());
        live_set->Add(input_inst->GetLinearNumber());
        SetInputRange(inst, input_inst, live_range);
    }

    // Extend SaveState inputs lifetime to the end of SaveState's lifetime
    if (inst->RequireState()) {
        auto save_state = inst->GetSaveState();
        ASSERT(save_state != nullptr);
        auto propagated_range = GetPropagatedLiveRange(inst, live_range);
        for (auto ss_input : save_state->GetInputs()) {
            auto input_inst = save_state->GetDataFlowInput(ss_input.GetInst());
            live_set->Add(input_inst->GetLinearNumber());
            GetInstLifeIntervals(input_inst)->AppendRange(propagated_range);
        }
    }

    // Handle CatchPhi inputs associated with inst
    auto range = pending_catch_phi_inputs_.equal_range(inst);
    for (auto it = range.first; it != range.second; ++it) {
        auto throwable_input = it->second;
        auto throwable_input_interval = GetInstLifeIntervals(throwable_input);
        live_set->Add(throwable_input->GetLinearNumber());
        throwable_input_interval->AppendRange(live_range);
    }
    pending_catch_phi_inputs_.erase(inst);
}

/*
 * Increase ref-input liveness in the 'no-async-jit' mode, since GC can be triggered and delete ref during callee-method
 * compilation
 */
void LivenessAnalyzer::SetInputRange([[maybe_unused]] const Inst *inst, const Inst *input, LiveRange live_range) const
{
    GetInstLifeIntervals(input)->AppendRange(live_range);
}

/*
 * CatchPhi does not handle inputs as regular instructions - instead of performing
 * some action at CatchPhi's definition copy instruction are added before throwable instructions.
 * Instead of extending input life interval until CatchPhi it is extended until throwable instruction.
 */
void LivenessAnalyzer::AdjustCatchPhiInputsLifetime(Inst *inst)
{
    auto catch_phi = inst->CastToCatchPhi();

    for (ssize_t input_idx = catch_phi->GetInputsCount() - 1; input_idx >= 0; input_idx--) {
        auto input_inst = catch_phi->GetDataFlowInput(input_idx);
        auto throwable_inst = const_cast<Inst *>(catch_phi->GetThrowableInst(input_idx));

        pending_catch_phi_inputs_.insert({throwable_inst, input_inst});
    }
}

void LivenessAnalyzer::SetInstLifeNumber([[maybe_unused]] const Inst *inst, LifeNumber number)
{
    ASSERT(inst_life_numbers_.size() == inst->GetLinearNumber());
    inst_life_numbers_.push_back(number);
}

LifeNumber LivenessAnalyzer::GetInstLifeNumber(Inst *inst) const
{
    return inst_life_numbers_[inst->GetLinearNumber()];
}

/*
 * Create new lifetime intervals for instruction, check that instruction linear number is equal to intervals
 * position in vector
 */
void LivenessAnalyzer::CreateLifeIntervals(Inst *inst)
{
    ASSERT(inst->GetLinearNumber() == inst_life_intervals_.size());
    inst_life_intervals_.push_back(GetAllocator()->New<LifeIntervals>(GetAllocator(), inst));
}

LifeIntervals *LivenessAnalyzer::GetInstLifeIntervals(const Inst *inst) const
{
    ASSERT(inst->GetLinearNumber() != INVALID_LINEAR_NUM);
    return inst_life_intervals_[inst->GetLinearNumber()];
}

void LivenessAnalyzer::SetBlockLiveRange(BasicBlock *block, LiveRange life_range)
{
    block_live_ranges_[block->GetId()] = life_range;
}

LiveRange LivenessAnalyzer::GetBlockLiveRange(const BasicBlock *block) const
{
    return block_live_ranges_[block->GetId()];
}

void LivenessAnalyzer::SetBlockLiveSet(BasicBlock *block, InstLiveSet *live_set)
{
    block_live_sets_[block->GetId()] = live_set;
}

InstLiveSet *LivenessAnalyzer::GetBlockLiveSet(BasicBlock *block) const
{
    return block_live_sets_[block->GetId()];
}

void LivenessAnalyzer::DumpLifeIntervals(std::ostream &out) const
{
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        if (bb->GetId() >= GetBlocksCount()) {
            continue;
        }
        auto block_range = GetBlockLiveRange(bb);
        out << "BB " << bb->GetId() << "\t" << block_range.ToString() << std::endl;

        for (auto inst : bb->AllInsts()) {
            if (inst->GetLinearNumber() == INVALID_LINEAR_NUM) {
                continue;
            }
            auto interval = GetInstLifeIntervals(inst);

            out << "v" << inst->GetId() << "\t";
            for (auto sibling = interval; sibling != nullptr; sibling = sibling->GetSibling()) {
                out << sibling->ToString<false>() << "@ " << sibling->GetLocation().ToString(GetGraph()->GetArch())
                    << "; ";
            }
            out << std::endl;
        }
    }
    DumpLocationsUsage(out);
}

void LivenessAnalyzer::DumpLocationsUsage(std::ostream &out) const
{
    std::map<Register, std::vector<LifeIntervals *>> regs_intervals;
    std::map<Register, std::vector<LifeIntervals *>> vregs_intervals;
    std::map<Register, std::vector<LifeIntervals *>> slots_intervals;
    for (auto &interval : inst_life_intervals_) {
        for (auto sibling = interval; sibling != nullptr; sibling = sibling->GetSibling()) {
            auto location = sibling->GetLocation();
            if (location.IsFpRegister()) {
                ASSERT(DataType::IsFloatType(interval->GetType()));
                vregs_intervals[location.GetValue()].push_back(sibling);
            } else if (location.IsRegister()) {
                regs_intervals[location.GetValue()].push_back(sibling);
            } else if (location.IsStack()) {
                slots_intervals[location.GetValue()].push_back(sibling);
            }
        }
    }

    for (auto intervals_map : {&regs_intervals, &vregs_intervals, &slots_intervals}) {
        std::string loc_symbol;
        if (intervals_map == &regs_intervals) {
            out << std::endl << "Registers intervals" << std::endl;
            loc_symbol = "r";
        } else if (intervals_map == &vregs_intervals) {
            out << std::endl << "Vector registers intervals" << std::endl;
            loc_symbol = "vr";
        } else {
            ASSERT(intervals_map == &slots_intervals);
            out << std::endl << "Stack slots intervals" << std::endl;
            loc_symbol = "s";
        }

        if (intervals_map->empty()) {
            out << "-" << std::endl;
            continue;
        }
        for (auto &[reg, intervals] : *intervals_map) {
            std::sort(intervals.begin(), intervals.end(),
                      [](const auto &lhs, const auto &rhs) { return lhs->GetBegin() < rhs->GetBegin(); });
            out << loc_symbol << std::to_string(reg) << ": ";
            auto delim = "";
            for (auto &interval : intervals) {
                out << delim << interval->ToString<false>();
                delim = "; ";
            }
            out << std::endl;
        }
    }
}

template <bool is_fp>
void LivenessAnalyzer::BlockPhysicalRegisters(LifeNumber block_from)
{
    auto arch = GetGraph()->GetArch();
    RegMask caller_regs {GetCallerRegsMask(arch, is_fp)};
    for (auto reg = GetFirstCallerReg(arch, is_fp); reg <= GetLastCallerReg(arch, is_fp); ++reg) {
        if (caller_regs.test(reg)) {
            BlockReg<is_fp>(reg, block_from);
        }
    }
}

void LivenessAnalyzer::BlockFixedLocationRegister(Location location, LifeNumber ln)
{
    if (location.IsRegister() && location.IsRegisterValid()) {
        BlockReg<false>(location.GetValue(), ln);
    } else if (location.IsFpRegister() && location.IsRegisterValid()) {
        BlockReg<true>(location.GetValue(), ln);
    }
}

template <bool is_fp>
void LivenessAnalyzer::BlockReg(Register reg, LifeNumber block_from)
{
    auto &intervals = is_fp ? physical_vector_intervals_ : physical_general_intervals_;
    auto interval = intervals.at(reg);
    if (interval == nullptr) {
        interval = GetGraph()->GetAllocator()->New<LifeIntervals>(GetGraph()->GetAllocator());
        CHECK_NOT_NULL(interval);
        interval->SetPhysicalReg(reg, is_fp ? DataType::FLOAT64 : DataType::UINT64);
        intervals.at(reg) = interval;
    }
    interval->AppendRange(block_from, block_from + 1U);
    interval->AddUsePosition(block_from);
}

bool LivenessAnalyzer::IsCallBlockingRegisters(Inst *inst) const
{
    if (inst->IsCall()) {
        return true;
    }
    if (inst->IsIntrinsic() && inst->CastToIntrinsic()->IsNativeCall()) {
        return true;
    }
    return false;
}

LifeIntervals *LifeIntervals::SplitAt(LifeNumber ln, ArenaAllocator *alloc)
{
    ASSERT(!IsPhysical());
    ASSERT(ln > GetBegin() && ln <= GetEnd());
    auto split_child = alloc->New<LifeIntervals>(alloc, GetInst());
    CHECK_NOT_NULL(split_child);
    if (sibling_ != nullptr) {
        split_child->sibling_ = sibling_;
    }
    split_child->is_split_sibling_ = true;

    sibling_ = split_child;
    split_child->SetType(GetType());

    for (auto &range = live_ranges_.back(); range.GetEnd() > ln; range = live_ranges_.back()) {
        live_ranges_.pop_back();
        if (range.GetBegin() > ln) {
            split_child->AppendRange(range);
        } else {
            split_child->AppendRange(ln, range.GetEnd());
            range.SetEnd(ln);
            if (range.GetBegin() != range.GetEnd()) {
                live_ranges_.push_back(range);
            }

            break;
        }
    }

    // Move use positions to the child
    auto it = use_positions_.lower_bound(ln);
    split_child->use_positions_.insert(it, use_positions_.end());
    use_positions_.erase(it, use_positions_.end());

    return split_child;
}

void LifeIntervals::MergeSibling()
{
    ASSERT(sibling_ != nullptr);
    while (!live_ranges_.empty()) {
        sibling_->AppendRange(live_ranges_.back());
        live_ranges_.pop_back();
    }
    live_ranges_ = std::move(sibling_->live_ranges_);

    for (auto &use_pos : sibling_->use_positions_) {
        AddUsePosition(use_pos);
    }
    sibling_ = nullptr;
}

LifeIntervals *LifeIntervals::FindSiblingAt(LifeNumber ln)
{
    ASSERT(!IsSplitSibling());
    for (auto head = this; head != nullptr; head = head->GetSibling()) {
        if (head->GetBegin() <= ln && ln <= head->GetEnd()) {
            return head;
        }
    }
    return nullptr;
}

bool LifeIntervals::Intersects(const LiveRange &range) const
{
    return  // the interval starts within the range
        (range.GetBegin() <= GetBegin() && GetBegin() <= range.GetEnd()) ||
        // the interval ends within the range
        (range.GetBegin() <= GetEnd() && GetEnd() <= range.GetEnd()) ||
        // the range is fully covered by the interval
        (GetBegin() <= range.GetBegin() && range.GetEnd() <= GetEnd());
}

LifeNumber LifeIntervals::GetFirstIntersectionWith(const LifeIntervals *other, LifeNumber search_from) const
{
    for (auto range : GetRanges()) {
        if (range.GetEnd() <= search_from) {
            continue;
        }
        for (auto other_range : other->GetRanges()) {
            if (other_range.GetEnd() <= search_from) {
                continue;
            }
            auto range_begin = std::max<LifeNumber>(search_from, range.GetBegin());
            auto other_range_begin = std::max<LifeNumber>(search_from, other_range.GetBegin());
            if (range_begin <= other_range_begin) {
                if (other_range_begin < range.GetEnd()) {
                    // [range]
                    //    [other]
                    return other_range_begin;
                }
                ASSERT(other_range_begin >= range.GetEnd());
            } else {
                //     [range]
                // [other]
                if (range_begin < other_range.GetEnd()) {
                    return range_begin;
                }
                ASSERT(range_begin >= other_range.GetEnd());
            }
        }
    }
    return INVALID_LIFE_NUMBER;
}

bool LifeIntervals::IntersectsWith(const LifeIntervals *other) const
{
    return GetFirstIntersectionWith(other) != INVALID_LIFE_NUMBER;
}

}  // namespace panda::compiler
