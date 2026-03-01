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

#include "graph.h"
#include "basicblock.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/analysis/loop_analyzer.h"

namespace panda::compiler {
static void MarkBlocksRec(Marker mrk, BasicBlock *block)
{
    if (block->SetMarker(mrk)) {
        return;
    }
    for (auto succ : block->GetSuccsBlocks()) {
        MarkBlocksRec(mrk, succ);
    }
}

Graph::~Graph()
{
}

void Graph::RemoveUnreachableBlocks()
{
    Marker mrk = NewMarker();
    MarkBlocksRec(mrk, GetStartBlock());
    // Remove unreachable blocks
    for (auto &bb : *this) {
        if (bb == nullptr) {
            continue;
        }
        if (!bb->IsMarked(mrk)) {
            RemovePredecessors(bb, false);
            RemoveSuccessors(bb);
            if (bb->IsTryBegin()) {
                EraseTryBeginBlock(bb);
                // Remove try_end mark from paired bb
                if (!bb->IsEmpty()) {
                    GetTryBeginInst(bb)->GetTryEndBlock()->SetTryEnd(false);
                }
            }
            // Clear DF:
            for (auto inst : bb->AllInsts()) {
                inst->RemoveInputs();
                if (IsInstThrowable(inst)) {
                    RemoveThrowableInst(inst);
                }
            }
            EraseBlock(bb);
        }
    }
    EraseMarker(mrk);
}

void Graph::AddConstInStartBlock(ConstantInst *const_inst)
{
    GetStartBlock()->AppendInst(const_inst);
}

ParameterInst *Graph::AddNewParameter(uint16_t arg_number)
{
    ParameterInst *param = CreateInstParameter(arg_number);
    GetStartBlock()->AppendInst(param);
    return param;
}

void Graph::RemoveConstFromList(ConstantInst *const_inst)
{
    if (const_inst == first_const_inst_) {
        first_const_inst_ = const_inst->GetNextConst();
        const_inst->SetNextConst(nullptr);
        return;
    }
    auto current = first_const_inst_;
    auto next = current->GetNextConst();
    while (next != nullptr && next != const_inst) {
        current = next;
        next = next->GetNextConst();
    }
    ASSERT(next != nullptr);
    ASSERT(next == const_inst);
    current->SetNextConst(const_inst->GetNextConst());
    const_inst->SetNextConst(nullptr);
}

void InvalidateBlocksOrderAnalyzes(Graph *graph)
{
    graph->InvalidateAnalysis<Rpo>();
    graph->InvalidateAnalysis<DominatorsTree>();
    graph->InvalidateAnalysis<LinearOrder>();
}

void Graph::AddBlock(BasicBlock *block)
{
    block->SetId(vector_bb_.size());
    vector_bb_.push_back(block);
    block->SetGraph(this);
    InvalidateBlocksOrderAnalyzes(this);
}

#ifndef NDEBUG
void Graph::AddBlock(BasicBlock *block, uint32_t id)
{
    if (vector_bb_.size() <= id) {
        // (id + 1) for adding a block with index 0
        vector_bb_.resize((id + 1U) << 1U, nullptr);
    }
    ASSERT(vector_bb_[id] == nullptr);
    block->SetId(id);
    vector_bb_[id] = block;
    InvalidateBlocksOrderAnalyzes(this);
}
#endif

const ArenaVector<BasicBlock *> &Graph::GetBlocksRPO() const
{
    return GetValidAnalysis<Rpo>().GetBlocks();
}

const ArenaVector<BasicBlock *> &Graph::GetBlocksLinearOrder() const
{
    return GetValidAnalysis<LinearOrder>().GetBlocks();
}

template <class Callback>
void Graph::VisitAllInstructions(Callback callback)
{
    for (auto bb : GetBlocksRPO()) {
        for (auto inst : bb->AllInsts()) {
            callback(inst);
        }
    }
}

BasicBlock *Graph::CreateEmptyBlock(uint32_t guest_pc)
{
    auto block = GetAllocator()->New<BasicBlock>(this, guest_pc);
    CHECK_NOT_NULL(block);
    AddBlock(block);
    return block;
}

// Create empty block with base block's properties
BasicBlock *Graph::CreateEmptyBlock(BasicBlock *base_block)
{
    ASSERT(base_block != nullptr);
    auto block = CreateEmptyBlock();
    block->SetGuestPc(base_block->GetGuestPc());
    block->SetAllFields(base_block->GetAllFields());
    block->SetTryId(base_block->GetTryId());
    return block;
}

#ifndef NDEBUG
BasicBlock *Graph::CreateEmptyBlock(uint32_t id, uint32_t guest_pc)
{
    auto block = GetAllocator()->New<BasicBlock>(this, guest_pc);
    CHECK_NOT_NULL(block);
    AddBlock(block, id);
    return block;
}
#endif

BasicBlock *Graph::CreateStartBlock()
{
    auto block = CreateEmptyBlock(0U);
    SetStartBlock(block);
    return block;
}

BasicBlock *Graph::CreateEndBlock(uint32_t guest_pc)
{
    auto block = CreateEmptyBlock(guest_pc);
    SetEndBlock(block);
    return block;
}

void RemovePredecessorUpdateDF(BasicBlock *block, BasicBlock *rm_pred)
{
    constexpr auto IMM_2 = 2;
    if (block->GetPredsBlocks().size() == IMM_2) {
        for (auto phi : block->PhiInstsSafe()) {
            auto rm_index = phi->CastToPhi()->GetPredBlockIndex(rm_pred);
            auto remaining_inst = phi->GetInput(1 - rm_index).GetInst();
            if (phi != remaining_inst && remaining_inst->GetBasicBlock() != nullptr) {
                phi->ReplaceUsers(remaining_inst);
            }
            block->RemoveInst(phi);
        }
    } else if (block->GetPredsBlocks().size() > IMM_2) {
        for (auto phi : block->PhiInstsSafe()) {
            auto rm_index = phi->CastToPhi()->GetPredBlockIndex(rm_pred);
            phi->CastToPhi()->RemoveInput(rm_index);
        }
    } else {
        ASSERT(block->GetPredsBlocks().size() == 1);
    }
    block->RemovePred(rm_pred);
    InvalidateBlocksOrderAnalyzes(block->GetGraph());
}

/*
 * Remove edges between `block` and its successors and
 * update phi-instructions in successors blocks
 */
void Graph::RemoveSuccessors(BasicBlock *block)
{
    for (auto succ : block->GetSuccsBlocks()) {
        RemovePredecessorUpdateDF(succ, block);
    }
    block->GetSuccsBlocks().clear();
}

/*
 * Remove edges between `block` and its predecessors,
 * update last instructions in predecessors blocks
 */
void Graph::RemovePredecessors(BasicBlock *block, bool remove_last_inst)
{
    for (auto pred : block->GetPredsBlocks()) {
        if (remove_last_inst && !pred->IsTryBegin() && !pred->IsTryEnd()) {
            if (pred->GetSuccsBlocks().size() == 2U) {
                auto last = pred->GetLastInst();
                ASSERT(last->GetOpcode() == Opcode::If || last->GetOpcode() == Opcode::IfImm);
                pred->RemoveInst(last);
            } else {
                ASSERT(pred->GetSuccsBlocks().size() == 1 && pred->GetSuccessor(0) == block);
            }
        }
        if (std::find(pred->GetSuccsBlocks().begin(), pred->GetSuccsBlocks().end(), block) !=
            pred->GetSuccsBlocks().end()) {
            pred->RemoveSucc(block);
        }
    }
    block->GetPredsBlocks().clear();
}

// Helper for the next 2 methods
static void FinishBlockRemoval(BasicBlock *block)
{
    auto graph = block->GetGraph();
    graph->GetAnalysis<DominatorsTree>().SetValid(true);
    auto dominator = block->GetDominator();
    if (dominator != nullptr) {
        dominator->RemoveDominatedBlock(block);
        for (auto dom_block : block->GetDominatedBlocks()) {
            ASSERT(dom_block->GetDominator() == block);
            dominator->AddDominatedBlock(dom_block);
            dom_block->SetDominator(dominator);
        }
    }
    block->SetDominator(nullptr);

    block->SetGraph(nullptr);
    if (graph->GetAnalysis<Rpo>().IsValid()) {
        graph->GetAnalysis<Rpo>().RemoveBasicBlock(block);
    }
}

/**
 * @param block - a block which is disconnecting from the graph with clearing control-flow and data-flow
 */
void Graph::DisconnectBlock(BasicBlock *block, bool remove_last_inst, bool fix_dom_tree)
{
    ASSERT(IsAnalysisValid<DominatorsTree>() || !fix_dom_tree);
    RemovePredecessors(block, remove_last_inst);
    RemoveSuccessors(block);

    if (block->IsTryBegin()) {
        EraseTryBeginBlock(block);
    }

    // Remove all instructions from `block`
    block->Clear();

    if (block->IsEndBlock()) {
        SetEndBlock(nullptr);
    }
    if (fix_dom_tree) {
        FinishBlockRemoval(block);
    }
    EraseBlock(block);
    // NB! please do not forget to fix LoopAnalyzer or invalidate it after the end of the pass
}

void Graph::DisconnectBlockRec(BasicBlock *block, bool remove_last_inst, bool fix_dom_tree)
{
    if (block->GetGraph() == nullptr) {
        return;
    }
    bool loop_flag = false;
    if (block->IsLoopHeader()) {
        loop_flag = true;
        auto loop = block->GetLoop();
        for (auto pred : block->GetPredsBlocks()) {
            loop_flag &= (std::find(loop->GetBackEdges().begin(), loop->GetBackEdges().end(), pred) !=
                          loop->GetBackEdges().end());
        }
    }
    if (block->GetPredsBlocks().empty() || loop_flag) {
        ArenaVector<BasicBlock *> succs(block->GetSuccsBlocks(), GetLocalAllocator()->Adapter());
        DisconnectBlock(block, remove_last_inst, fix_dom_tree);
        for (auto succ : succs) {
            DisconnectBlockRec(succ, remove_last_inst, fix_dom_tree);
        }
    }
}

void Graph::EraseBlock(BasicBlock *block)
{
    vector_bb_[block->GetId()] = nullptr;
    if (GetEndBlock() == block) {
        SetEndBlock(nullptr);
    }
    ASSERT(GetStartBlock() != block);
    block->SetGraph(nullptr);
}

void Graph::RestoreBlock(BasicBlock *block)
{
    ASSERT(vector_bb_[block->GetId()] == nullptr);
    vector_bb_[block->GetId()] = block;
    block->SetGraph(this);
    InvalidateBlocksOrderAnalyzes(this);
}

/**
 * @param block - same for block without instructions at all
 */
void Graph::RemoveEmptyBlock(BasicBlock *block)
{
    ASSERT(IsAnalysisValid<DominatorsTree>());
    ASSERT(block->GetLastInst() == nullptr);
    ASSERT(block->GetPredsBlocks().empty());
    ASSERT(block->GetSuccsBlocks().empty());

    FinishBlockRemoval(block);
    EraseBlock(block);
    // NB! please do not forget to fix LoopAnalyzer or invalidate it after the end of the pass
}

/**
 * @param block - same for block without instructions, may have Phi(s)
 */
void Graph::RemoveEmptyBlockWithPhis(BasicBlock *block, bool irr_loop)
{
    ASSERT(IsAnalysisValid<DominatorsTree>());
    ASSERT(block->IsEmpty());

    ASSERT(!block->GetSuccsBlocks().empty());
    ASSERT(!block->GetPredsBlocks().empty());
    block->RemoveEmptyBlock(irr_loop);

    FinishBlockRemoval(block);
    EraseBlock(block);
}

ConstantInst *Graph::FindConstant(DataType::Type type, uint64_t value)
{
    for (auto constant = GetFirstConstInst(); constant != nullptr; constant = constant->GetNextConst()) {
        if (constant->GetType() != type) {
            continue;
        }
        if (IsBytecodeOptimizer() && IsInt32Bit(type) && (constant->GetInt32Value() == static_cast<uint32_t>(value))) {
            return constant;
        }
        if (constant->IsEqualConst(type, value)) {
            return constant;
        }
    }
    return nullptr;
}

ConstantInst *Graph::FindOrAddConstant(ConstantInst *inst)
{
    auto existing_const = FindConstant(inst->GetType(), inst->GetRawValue());
    if (existing_const != nullptr) {
        return existing_const;
    }
    AddConstInStartBlock(inst);
    inst->SetNextConst(first_const_inst_);
    first_const_inst_ = inst;
    return inst;
}

void Graph::ResetParameterInfo()
{
    param_info_ = nullptr;
    return;
}

bool Graph::HasLoop() const
{
    ASSERT(GetAnalysis<LoopAnalyzer>().IsValid());
    return !GetRootLoop()->GetInnerLoops().empty();
}

bool Graph::HasIrreducibleLoop() const
{
    ASSERT(GetAnalysis<LoopAnalyzer>().IsValid());
    return FlagIrredicibleLoop::Get(bit_fields_);
}

bool Graph::HasInfiniteLoop() const
{
    ASSERT(GetAnalysis<LoopAnalyzer>().IsValid());
    return FlagInfiniteLoop::Get(bit_fields_);
}

bool Graph::HasFloatRegs() const
{
    ASSERT(IsRegAllocApplied());
    return FlagFloatRegs::Get(bit_fields_);
}

/*
 * Mark blocks, which have successor from external loop
 */
void MarkLoopExits(const Graph *graph, Marker marker)
{
    for (auto block : graph->GetBlocksRPO()) {
        if (block->GetSuccsBlocks().size() == MAX_SUCCS_NUM) {
            if (block->GetSuccessor(0)->GetLoop() != block->GetSuccessor(1)->GetLoop()) {
                block->SetMarker(marker);
            }
        } else if (block->GetSuccsBlocks().size() > MAX_SUCCS_NUM) {
            ASSERT(block->IsTryEnd() || block->IsTryBegin());
            auto loop = block->GetSuccessor(0)->GetLoop();
            for (size_t i = 1; i < block->GetSuccsBlocks().size(); i++) {
                if (loop != block->GetSuccessor(i)->GetLoop()) {
                    block->SetMarker(marker);
                }
            }
        }
    }
}

std::string GetMethodFullName(const Graph *graph, RuntimeInterface::MethodPtr method)
{
    std::stringstream sstream;
    sstream << graph->GetRuntime()->GetClassNameFromMethod(method)
            << "::" << graph->GetRuntime()->GetMethodName(method);
    return sstream.str();
}

// NOLINTNEXTLINE(readability-identifier-naming,-warnings-as-errors)
Graph::ParameterList::Iterator Graph::ParameterList::begin()
{
    auto start_bb = graph_->GetStartBlock();
    Iterator it(start_bb->GetFirstInst());
    if (*it != nullptr && it->GetOpcode() != Opcode::Parameter) {
        ++it;
    }
    return it;
}

void Graph::RemoveThrowableInst(const Inst *inst)
{
    ASSERT(IsInstThrowable(inst));
    for (auto catch_handler : throwable_insts_.at(inst)) {
        for (auto catch_inst : catch_handler->AllInsts()) {
            if (!catch_inst->IsCatchPhi() || catch_inst->CastToCatchPhi()->IsAcc()) {
                continue;
            }
            auto catch_phi = catch_inst->CastToCatchPhi();
            const auto &vregs = catch_phi->GetThrowableInsts();
            auto it = std::find(vregs->begin(), vregs->end(), inst);
            if (it != vregs->end()) {
                int index = std::distance(vregs->begin(), it);
                catch_phi->RemoveInput(index);
            }
        }
    }
    throwable_insts_.erase(inst);
}

void Graph::ReplaceThrowableInst(Inst *old_inst, Inst *new_inst)
{
    auto it = throwable_insts_.emplace(new_inst, GetAllocator()->Adapter()).first;
    it->second = std::move(throwable_insts_.at(old_inst));

    for (auto catch_handler : it->second) {
        for (auto catch_inst : catch_handler->AllInsts()) {
            if (!catch_inst->IsCatchPhi() || catch_inst->CastToCatchPhi()->IsAcc()) {
                continue;
            }
            auto catch_phi = catch_inst->CastToCatchPhi();
            const auto &vregs = catch_phi->GetThrowableInsts();
            auto iter = std::find(vregs->begin(), vregs->end(), old_inst);
            if (iter != vregs->end()) {
                catch_phi->ReplaceThrowableInst(old_inst, new_inst);
            }
        }
    }
    throwable_insts_.erase(old_inst);
}

void Graph::DumpThrowableInsts(std::ostream *out) const
{
    for (auto &[inst, handlers] : throwable_insts_) {
        (*out) << "Throwable Inst";
        inst->Dump(out);
        (*out) << "Catch handlers:";
        auto sep = " ";
        for (auto bb : handlers) {
            (*out) << sep << "BB " << bb->GetId();
            sep = ", ";
        }
        (*out) << std::endl;
    }
}

void Graph::InitDefaultLocations()
{
    if (IsDefaultLocationsInit()) {
        return;
    }
    VisitAllInstructions([this](Inst *inst) {
        if (!inst->IsOperandsDynamic() || inst->IsPhi()) {
            return;
        }
        [[maybe_unused]] LocationsInfo *locations = GetAllocator()->New<LocationsInfo>(GetAllocator(), inst);
        for (size_t i = 0; i < inst->GetInputsCount(); i++) {
            if (inst->GetInputType(i) != DataType::NO_TYPE) {
                locations->SetLocation(i, Location::RequireRegister());
            }
        }
    });
    SetDefaultLocationsInit();
}

int64_t Graph::GetBranchCounter(const BasicBlock *block, bool true_succ)
{
    ASSERT(block->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
    auto last_inst = block->GetLastInst();
    RuntimeInterface::MethodPtr method;
    if (last_inst->GetOpcode() == Opcode::IfImm) {
        method = last_inst->CastToIfImm()->GetMethod();
    } else if (last_inst->GetOpcode() == Opcode::If) {
        method = last_inst->CastToIf()->GetMethod();
    } else {
        return 0;
    }

    if (method == nullptr) {
        // corresponded branch instruction was not present in bytecode, e.g. IfImmInst was created while inlining
        return 0;
    }

    return block->IsInverted() == 0;
}

uint32_t Graph::GetParametersSlotsCount() const
{
    uint32_t max_slot = 0;
    for (auto param_inst : GetParameters()) {
        auto location = param_inst->CastToParameter()->GetLocationData().GetSrc();
        if (location.IsStackParameter()) {
            max_slot = location.GetValue() + 1U;
        }
    }
    return max_slot;
}

void GraphMode::Dump(std::ostream &stm)
{
    const char *sep = "";
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DUMP_MODE(name)      \
    if (Is##name()) {        \
        stm << sep << #name; \
        sep = ", ";          \
    }

    DUMP_MODE(Osr);
    DUMP_MODE(BytecodeOpt);
    DUMP_MODE(DynamicMethod);
    DUMP_MODE(Native);
    DUMP_MODE(FastPath);
    DUMP_MODE(Boundary);
    DUMP_MODE(Interpreter);
    DUMP_MODE(InterpreterEntry);
}

}  // namespace panda::compiler
