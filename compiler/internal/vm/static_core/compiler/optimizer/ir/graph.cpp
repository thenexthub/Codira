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
#include "inst.h"
#include "bytecode_optimizer/bytecode_encoder.h"
#include "compiler_logger.h"
#include "optimizer/analysis/alias_analysis.h"
#include "optimizer/analysis/bounds_analysis.h"
#include "optimizer/analysis/dominators_tree.h"
#include "optimizer/analysis/rpo.h"
#include "optimizer/analysis/linear_order.h"
#include "optimizer/analysis/loop_analyzer.h"
#if defined(PANDA_WITH_CODEGEN) && !defined(PANDA_TARGET_WINDOWS) && !defined(PANDA_TARGET_MACOS)
#include "optimizer/code_generator/callconv.h"
#include "optimizer/code_generator/codegen.h"
#include "optimizer/code_generator/encode.h"
#include "optimizer/code_generator/registers_description.h"
#endif

namespace ark::compiler {
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
    if (encoder_ != nullptr) {
        encoder_->~Encoder();
    }
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
        if (bb->IsMarked(mrk)) {
            continue;
        }
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
        COMPILER_LOG(DEBUG, CLEANUP) << "Erase unreachable block " << bb->GetId();
        EraseBlock(bb);
    }
    EraseMarker(mrk);
}

void Graph::AddConstInStartBlock(ConstantInst *constInst)
{
    GetStartBlock()->AppendInst(constInst);
}

ParameterInst *Graph::AddNewParameter(uint16_t argNumber)
{
    ParameterInst *param = CreateInstParameter(argNumber);
    GetStartBlock()->AppendInst(param);
    return param;
}

ParameterInst *Graph::FindParameter(uint16_t argNumber)
{
    for (auto inst : GetStartBlock()->AllInsts()) {
        if (!inst->IsParameter()) {
            continue;
        }
        auto paramInst = inst->CastToParameter();
        if (paramInst->GetArgNumber() == argNumber) {
            return paramInst;
        }
    }
    return nullptr;
}

Inst *Graph::GetOrCreateNullPtr()
{
    if (nullptrInst_ == nullptr) {
        nullptrInst_ = CreateInstNullPtr(DataType::REFERENCE);
        GetStartBlock()->AppendInst(nullptrInst_);
    }
    return nullptrInst_;
}

Inst *Graph::GetOrCreateUniqueObjectInst()
{
    if (uniqueObjectInst_ == nullptr) {
        uniqueObjectInst_ = CreateInstLoadUniqueObject(DataType::REFERENCE);
        GetStartBlock()->AppendInst(uniqueObjectInst_);
    }
    return uniqueObjectInst_;
}

void Graph::RemoveConstFromList(ConstantInst *constInst)
{
    if (constInst == firstConstInst_) {
        firstConstInst_ = constInst->GetNextConst();
        constInst->SetNextConst(nullptr);
        return;
    }
    auto current = firstConstInst_;
    auto next = current->GetNextConst();
    while (next != nullptr && next != constInst) {
        current = next;
        next = next->GetNextConst();
    }
    ASSERT(next != nullptr);
    ASSERT(next == constInst);
    current->SetNextConst(constInst->GetNextConst());
    constInst->SetNextConst(nullptr);
}

void InvalidateBlocksOrderAnalyzes(Graph *graph)
{
    graph->InvalidateAnalysis<Rpo>();
    graph->InvalidateAnalysis<DominatorsTree>();
    graph->InvalidateAnalysis<LinearOrder>();
}

void Graph::AddBlock(BasicBlock *block)
{
    ASSERT(block != nullptr);
    block->SetId(vectorBb_.size());
    vectorBb_.push_back(block);
    block->SetGraph(this);
    InvalidateBlocksOrderAnalyzes(this);
}

#ifndef NDEBUG
void Graph::AddBlock(BasicBlock *block, uint32_t id)
{
    if (vectorBb_.size() <= id) {
        // (id + 1) for adding a block with index 0
        vectorBb_.resize((id + 1U) << 1U, nullptr);
    }
    ASSERT(vectorBb_[id] == nullptr);
    block->SetId(id);
    vectorBb_[id] = block;
    InvalidateBlocksOrderAnalyzes(this);
}
#endif

const BoundsRangeInfo *Graph::GetBoundsRangeInfo() const
{
    return GetValidAnalysis<BoundsAnalysis>().GetBoundsRangeInfo();
}

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

AliasType Graph::CheckInstAlias(Inst *mem1, Inst *mem2)
{
    return GetValidAnalysis<AliasAnalysis>().CheckInstAlias(mem1, mem2);
}

BasicBlock *Graph::CreateEmptyBlock(uint32_t guestPc)
{
    auto block = GetAllocator()->New<BasicBlock>(this, guestPc);
    AddBlock(block);
    return block;
}

// Create empty block with base block's properties
BasicBlock *Graph::CreateEmptyBlock(BasicBlock *baseBlock)
{
    ASSERT(baseBlock != nullptr);
    auto block = CreateEmptyBlock();
    ASSERT(block != nullptr);
    block->SetGuestPc(baseBlock->GetGuestPc());
    block->SetAllFields(baseBlock->GetAllFields());
    block->SetTryId(baseBlock->GetTryId());
    block->SetOsrEntry(false);
    return block;
}

#ifndef NDEBUG
BasicBlock *Graph::CreateEmptyBlock(uint32_t id, uint32_t guestPc)
{
    auto block = GetAllocator()->New<BasicBlock>(this, guestPc);
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

BasicBlock *Graph::CreateEndBlock(uint32_t guestPc)
{
    auto block = CreateEmptyBlock(guestPc);
    SetEndBlock(block);
    return block;
}

void Graph::RemovePredecessorUpdateDF(BasicBlock *block, BasicBlock *rmPred)
{
    constexpr auto IMM_2 = 2;
    if (block->GetPredsBlocks().size() == IMM_2) {
        for (auto phi : block->PhiInstsSafe()) {
            auto rmIndex = phi->CastToPhi()->GetPredBlockIndex(rmPred);
            auto remainingInst = phi->GetInput(1 - rmIndex).GetInst();
            if (phi != remainingInst && remainingInst->GetBasicBlock() != nullptr) {
                phi->ReplaceUsers(remainingInst);
            }
            block->RemoveInst(phi);
        }
    } else if (block->GetPredsBlocks().size() > IMM_2) {
        for (auto phi : block->PhiInstsSafe()) {
            auto rmIndex = phi->CastToPhi()->GetPredBlockIndex(rmPred);
            phi->CastToPhi()->RemoveInput(rmIndex);
        }
    } else {
        ASSERT(block->GetPredsBlocks().size() == 1);
    }
    block->RemovePred(rmPred);
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
void Graph::RemovePredecessors(BasicBlock *block, bool removeLastInst)
{
    for (auto pred : block->GetPredsBlocks()) {
        if (removeLastInst && !pred->IsTryBegin() && !pred->IsTryEnd()) {
            if (pred->GetSuccsBlocks().size() == 2U) {
                auto last = pred->GetLastInst();
                ASSERT(last->GetOpcode() == Opcode::If || last->GetOpcode() == Opcode::IfImm ||
                       last->GetOpcode() == Opcode::AddOverflow || last->GetOpcode() == Opcode::SubOverflow ||
                       last->GetOpcode() == Opcode::Throw);
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
        for (auto domBlock : block->GetDominatedBlocks()) {
            ASSERT(domBlock->GetDominator() == block);
            dominator->AddDominatedBlock(domBlock);
            domBlock->SetDominator(dominator);
        }
    }
    block->SetDominator(nullptr);

    block->SetGraph(nullptr);
    if (graph->GetAnalysis<Rpo>().IsValid()) {
        graph->GetAnalysis<Rpo>().RemoveBasicBlock(block);
    }
}

/// @param block - a block which is disconnecting from the graph with clearing control-flow and data-flow
void Graph::DisconnectBlock(BasicBlock *block, bool removeLastInst, bool fixDomTree)
{
    ASSERT(IsAnalysisValid<DominatorsTree>() || !fixDomTree);
    RemovePredecessors(block, removeLastInst);
    RemoveSuccessors(block);

    if (block->IsTryBegin()) {
        EraseTryBeginBlock(block);
    }

    // Remove all instructions from `block`
    block->Clear();

    if (block->IsEndBlock()) {
        SetEndBlock(nullptr);
    }
    if (fixDomTree) {
        FinishBlockRemoval(block);
    }
    EraseBlock(block);
    // NB! please do not forget to fix LoopAnalyzer or invalidate it after the end of the pass
}

void Graph::DisconnectBlockRec(BasicBlock *block, bool removeLastInst, bool fixDomTree)
{
    ASSERT(block != nullptr);
    if (block->GetGraph() == nullptr) {
        return;
    }
    bool loopFlag = false;
    if (block->IsLoopHeader()) {
        loopFlag = true;
        auto loop = block->GetLoop();
        for (auto pred : block->GetPredsBlocks()) {
            loopFlag &= (std::find(loop->GetBackEdges().begin(), loop->GetBackEdges().end(), pred) !=
                         loop->GetBackEdges().end());
        }
    }
    if (block->GetPredsBlocks().empty() || loopFlag) {
        BasicBlock::SuccsVector succs(block->GetSuccsBlocks());
        DisconnectBlock(block, removeLastInst, fixDomTree);
        for (auto succ : succs) {
            DisconnectBlockRec(succ, removeLastInst, fixDomTree);
        }
    }
}

void Graph::EraseBlock(BasicBlock *block)
{
    vectorBb_[block->GetId()] = nullptr;
    if (GetEndBlock() == block) {
        SetEndBlock(nullptr);
    }
    ASSERT(GetStartBlock() != block);
    block->SetGraph(nullptr);
}

void Graph::RestoreBlock(BasicBlock *block)
{
    ASSERT(vectorBb_[block->GetId()] == nullptr);
    vectorBb_[block->GetId()] = block;
    block->SetGraph(this);
    InvalidateBlocksOrderAnalyzes(this);
}

/// @param block - same for block without instructions at all
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

/// @param block - same for block without instructions, may have Phi(s)
void Graph::RemoveEmptyBlockWithPhis(BasicBlock *block, bool irrLoop)
{
    ASSERT(IsAnalysisValid<DominatorsTree>());
    ASSERT(block->IsEmpty());

    ASSERT(!block->GetSuccsBlocks().empty());
    ASSERT(!block->GetPredsBlocks().empty());
    block->RemoveEmptyBlock(irrLoop);

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
    auto existingConst = FindConstant(inst->GetType(), inst->GetRawValue());
    if (existingConst != nullptr) {
        return existingConst;
    }
    AddConstInStartBlock(inst);
    inst->SetNextConst(firstConstInst_);
    firstConstInst_ = inst;
    return inst;
}

Encoder *Graph::GetEncoder()
{
    if (encoder_ == nullptr) {
        if (IsBytecodeOptimizer()) {
            return encoder_ = GetAllocator()->New<bytecodeopt::BytecodeEncoder>(GetAllocator());
        }
#if defined(PANDA_WITH_CODEGEN) && !defined(PANDA_TARGET_WINDOWS) && !defined(PANDA_TARGET_MACOS)
        encoder_ = Encoder::Create(GetAllocator(), GetArch(), g_options.IsCompilerEmitAsm(), IsDynamicMethod());
#endif
    }
    return encoder_;
}

RegistersDescription *Graph::GetRegisters() const
{
#if !defined(PANDA_WITH_CODEGEN) || defined(PANDA_TARGET_WINDOWS) || defined(PANDA_TARGET_MACOS)
    return nullptr;
#else
    if (registers_ == nullptr) {
        registers_ = RegistersDescription::Create(GetAllocator(), GetArch());
    }
    return registers_;
#endif
}

CallingConvention *Graph::GetCallingConvention()
{
#if !defined(PANDA_WITH_CODEGEN) || defined(PANDA_TARGET_WINDOWS) || defined(PANDA_TARGET_MACOS)
    return nullptr;
#else
    if (callconv_ == nullptr) {
        // We use encoder_ instead of GetEncoder() because we use CallingConvention for ParameterInfo.
        // This doesn't require an encoder, so we don't create one
        bool isOptIrtoc = (mode_.IsInterpreter() || mode_.IsInterpreterEntry()) && IsIrtocPrologEpilogOptimized();
        callconv_ = CallingConvention::Create(GetAllocator(), encoder_, GetRegisters(), GetArch(),
                                              //   is_panda_abi,      is_osr,           is_dyn
                                              mode_.SupportManagedCode(), IsOsrMode(),
                                              IsDynamicMethod() && !GetMode().IsDynamicStub(), false, isOptIrtoc);
    }
    return callconv_;
#endif
}

const MethodProperties &Graph::GetMethodProperties()
{
#if !defined(PANDA_WITH_CODEGEN) || defined(PANDA_TARGET_WINDOWS) || defined(PANDA_TARGET_MACOS)
    UNREACHABLE();
#else
    if (!methodProperties_) {
        methodProperties_.emplace(this);
    }
#endif
    return methodProperties_.value();
}

void Graph::ResetParameterInfo()
{
#if defined(PANDA_WITH_CODEGEN) && !defined(PANDA_TARGET_WINDOWS) && !defined(PANDA_TARGET_MACOS)
    auto callconv = GetCallingConvention();
    if (callconv == nullptr) {
        paramInfo_ = nullptr;
        return;
    }
    auto regsReserve = 0;
    if (GetMode().SupportManagedCode()) {
        regsReserve++;
        if (GetMode().IsDynamicMethod() && GetMode().IsDynamicStub()) {
            regsReserve++;
        }
    }
    paramInfo_ = callconv->GetParameterInfo(regsReserve);
#endif
}

Register Graph::GetZeroReg() const
{
    auto regfile = GetRegisters();
    if (regfile == nullptr) {
        return GetInvalidReg();
    }
    auto reg = regfile->GetZeroReg();
    if (reg == INVALID_REGISTER) {
        return INVALID_REG;
    }
    return reg.GetId();
}

Register Graph::GetArchTempReg() const
{
    auto tempMask = Target(GetArch()).GetTempRegsMask();
    for (ssize_t reg = static_cast<ssize_t>(RegMask::Size()) - 1; reg >= 0; reg--) {
        if (tempMask[reg] && const_cast<Graph *>(this)->GetArchUsedRegs()[reg]) {
            return reg;
        }
    }
    return INVALID_REG;
}

Register Graph::GetArchTempVReg() const
{
    auto regfile = GetRegisters();
    if (regfile == nullptr) {
        return INVALID_REG;
    }
    auto regId = regfile->GetTempVReg();
    if (regId == INVALID_REG_ID) {
        return INVALID_REG;
    }
    return regId;
}

RegMask Graph::GetArchUsedRegs()
{
    auto regfile = GetRegisters();
    if (regfile == nullptr && archUsedRegs_.None()) {
        return RegMask();
    }
    if (archUsedRegs_.None()) {
        archUsedRegs_ = regfile->GetRegMask();
    }
    return archUsedRegs_;
}

void Graph::SetArchUsedRegs(RegMask mask)
{
    archUsedRegs_ = mask;
    auto regs = GetRegisters();
    ASSERT(regs != nullptr);
    regs->SetRegMask(mask);
}

VRegMask Graph::GetArchUsedVRegs()
{
    auto regfile = GetRegisters();
    if (regfile == nullptr) {
        return VRegMask();
    }
    return regfile->GetVRegMask();
}

bool Graph::IsRegScalarMapped() const
{
    auto regfile = GetRegisters();
    if (regfile == nullptr) {
        return false;
    }
    return regfile->SupportMapping(RegMapping::SCALAR_SCALAR);
}

bool Graph::HasLoop() const
{
    ASSERT(GetAnalysis<LoopAnalyzer>().IsValid());
    return !GetRootLoop()->GetInnerLoops().empty();
}

bool Graph::HasIrreducibleLoop() const
{
    ASSERT(GetAnalysis<LoopAnalyzer>().IsValid());
    return FlagIrredicibleLoop::Get(bitFields_);
}

bool Graph::HasInfiniteLoop() const
{
    ASSERT(GetAnalysis<LoopAnalyzer>().IsValid());
    return FlagInfiniteLoop::Get(bitFields_);
}

bool Graph::HasFloatRegs() const
{
    ASSERT(IsRegAllocApplied());
    return FlagFloatRegs::Get(bitFields_);
}

static void MarkSuccessBlocks(BasicBlock *block, Marker marker)
{
    auto loop = block->GetSuccessor(0)->GetLoop();
    for (size_t i = 1; i < block->GetSuccsBlocks().size(); i++) {
        if (loop != block->GetSuccessor(i)->GetLoop()) {
            block->SetMarker(marker);
        }
    }
}

/*
 * Mark blocks, which have successor from external loop
 */
void MarkLoopExits(const Graph *graph, Marker marker)
{
    for (auto block : graph->GetBlocksRPO()) {
        if (block->GetSuccsBlocks().size() == MAX_SUCCS_NUM) {
            auto thisLoop = block->GetLoop();
            auto loop0 = block->GetSuccessor(0)->GetLoop();
            auto loop1 = block->GetSuccessor(1)->GetLoop();
            if (loop0 != thisLoop && !loop0->IsInside(thisLoop)) {
                block->SetMarker(marker);
            }
            if (loop1 != thisLoop && !loop1->IsInside(thisLoop)) {
                block->SetMarker(marker);
            }
        } else if (block->GetSuccsBlocks().size() > MAX_SUCCS_NUM) {
            ASSERT(block->IsTryEnd() || block->IsTryBegin() || block->GetLastInst()->GetOpcode() == Opcode::Throw);
            MarkSuccessBlocks(block, marker);
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

SpillFillData Graph::GetDataForNativeParam(DataType::Type type)
{
#if !defined(PANDA_WITH_CODEGEN) || defined(PANDA_TARGET_WINDOWS) || defined(PANDA_TARGET_MACOS)
    (void)type;
    return {};
#else
    // NOTE(pishin) change to ASSERT
    if (paramInfo_ == nullptr) {
        // NOTE(pishin) enable after fixing arch in tests - UNREACHABLE()
        return {};
    }

    auto param = paramInfo_->GetNativeParam(Codegen::ConvertDataType(type, GetArch()));
    if (std::holds_alternative<Reg>(param)) {
        auto reg = std::get<Reg>(param);
        // NOTE! Vector parameter can be put to scalar register in aarch32
        DataType::Type regType;
        if (reg.IsFloat()) {
            regType = DataType::FLOAT64;
        } else if (reg.GetType() == INT64_TYPE) {
            regType = DataType::UINT64;
        } else {
            regType = DataType::UINT32;
        }
        auto loc = reg.IsFloat() ? LocationType::FP_REGISTER : LocationType::REGISTER;
        return SpillFillData(SpillFillData {loc, LocationType::INVALID, reg.GetId(), GetInvalidReg(), regType});
    }
    ASSERT(std::holds_alternative<uint8_t>(param));
    auto slot = std::get<uint8_t>(param);
    DataType::Type regType;
    if (DataType::IsFloatType(type)) {
        regType = type;
    } else if (DataType::Is32Bits(type, GetArch())) {
        regType = DataType::UINT32;
    } else {
        regType = DataType::UINT64;
    }
    return SpillFillData(
        SpillFillData {LocationType::STACK_PARAMETER, LocationType::INVALID, slot, GetInvalidReg(), regType});
#endif
}

// NOLINTNEXTLINE(readability-identifier-naming,-warnings-as-errors)
Graph::ParameterList::Iterator Graph::ParameterList::begin()
{
    auto startBb = graph_->GetStartBlock();
    Iterator it(startBb->GetFirstInst());
    if (*it != nullptr && it->GetOpcode() != Opcode::Parameter) {
        ++it;
    }
    return it;
}

void Graph::RemoveThrowableInst(const Inst *inst)
{
    ASSERT(IsInstThrowable(inst));
    for (auto catchHandler : throwableInsts_.at(inst)) {
        for (auto catchInst : catchHandler->AllInsts()) {
            if (!catchInst->IsCatchPhi() || catchInst->CastToCatchPhi()->IsAcc()) {
                continue;
            }
            auto catchPhi = catchInst->CastToCatchPhi();
            const auto &vregs = catchPhi->GetThrowableInsts();
            if (vregs == nullptr || vregs->empty()) {
                continue;
            }
            auto it = std::find(vregs->begin(), vregs->end(), inst);
            if (it != vregs->end()) {
                int index = std::distance(vregs->begin(), it);
                catchPhi->RemoveInput(index);
            }
        }
    }
    throwableInsts_.erase(inst);
}

void Graph::ReplaceThrowableInst(Inst *oldInst, Inst *newInst)
{
    auto it = throwableInsts_.emplace(newInst, GetAllocator()->Adapter()).first;
    it->second = std::move(throwableInsts_.at(oldInst));

    for (auto catchHandler : it->second) {
        for (auto catchInst : catchHandler->AllInsts()) {
            if (!catchInst->IsCatchPhi() || catchInst->CastToCatchPhi()->IsAcc()) {
                continue;
            }
            auto catchPhi = catchInst->CastToCatchPhi();
            const auto &vregs = catchPhi->GetThrowableInsts();
            auto iter = std::find(vregs->begin(), vregs->end(), oldInst);
            if (iter != vregs->end()) {
                catchPhi->ReplaceThrowableInst(oldInst, newInst);
            }
        }
    }
    throwableInsts_.erase(oldInst);
}

void Graph::DumpThrowableInsts(std::ostream *out) const
{
    for (auto &[inst, handlers] : throwableInsts_) {
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

int64_t Graph::GetBranchCounter(const BasicBlock *block, bool trueSucc)
{
    ASSERT(block->GetSuccsBlocks().size() == MAX_SUCCS_NUM);
    auto lastInst = block->GetLastInst();
    ASSERT(lastInst != nullptr);
    if (lastInst->GetPc() == 0) {
        return 0;
    }
    RuntimeInterface::MethodPtr method;
    if (lastInst->GetOpcode() == Opcode::IfImm) {
        method = lastInst->CastToIfImm()->GetMethod();
    } else if (lastInst->GetOpcode() == Opcode::If) {
        method = lastInst->CastToIf()->GetMethod();
    } else {
        return 0;
    }

    if (method == nullptr) {
        // corresponded branch instruction was not present in bytecode, e.g. IfImmInst was created while inlining
        return 0;
    }

    return block->IsInverted() == trueSucc ? GetRuntime()->GetBranchNotTakenCounter(method, lastInst->GetPc())
                                           : GetRuntime()->GetBranchTakenCounter(method, lastInst->GetPc());
}

int64_t Graph::GetThrowCounter(const BasicBlock *block)
{
    auto lastInst = block->GetLastInst();
    if (lastInst == nullptr || lastInst->GetOpcode() != Opcode::Throw || lastInst->GetPc() == INVALID_PC) {
        return 0;
    }

    auto method = lastInst->CastToThrow()->GetCallMethod();
    if (method == nullptr) {
        return 0;
    }

    return GetRuntime()->GetThrowTakenCounter(method, lastInst->GetPc());
}

uint32_t Graph::GetParametersSlotsCount() const
{
    uint32_t maxSlot = 0;
    for (auto paramInst : GetParameters()) {
        auto location = paramInst->CastToParameter()->GetLocationData().GetSrc();
        if (location.IsStackParameter()) {
            maxSlot = location.GetValue() + 1U;
        }
    }
    return maxSlot;
}

void GraphMode::Dump(std::ostream &stm)
{
    const char *sep = "";
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DUMP_MODE(name)                                \
    if (Is##name()) {                                  \
        /* CC-OFFNXT(G.PRE.10) function scope macro */ \
        stm << sep << #name;                           \
        /* CC-OFFNXT(G.PRE.10) function scope macro */ \
        sep = ", ";                                    \
    }

    DUMP_MODE(Osr);
    DUMP_MODE(BytecodeOpt);
    DUMP_MODE(DynamicMethod);
    DUMP_MODE(Native);
    DUMP_MODE(FastPath);
    DUMP_MODE(Boundary);
    DUMP_MODE(Interpreter);
    DUMP_MODE(InterpreterEntry);
    DUMP_MODE(AbcKit);
    DUMP_MODE(NativePlus);

#undef DUMP_MODE
}

size_t GetObjectOffset(const Graph *graph, ObjectType objType, RuntimeInterface::FieldPtr field, uint32_t typeId)
{
    switch (objType) {
        case ObjectType::MEM_DYN_GLOBAL:
            return graph->GetRuntime()->GetPropertyBoxOffset(graph->GetArch());
        case ObjectType::MEM_DYN_ELEMENTS:
            return graph->GetRuntime()->GetElementsOffset(graph->GetArch());
        case ObjectType::MEM_DYN_PROPS:
            return graph->GetRuntime()->GetPropertiesOffset(graph->GetArch());
        case ObjectType::MEM_DYN_PROTO_HOLDER:
            return graph->GetRuntime()->GetPrototypeHolderOffset(graph->GetArch());
        case ObjectType::MEM_DYN_PROTO_CELL:
            return graph->GetRuntime()->GetPrototypeCellOffset(graph->GetArch());
        case ObjectType::MEM_DYN_CHANGE_FIELD:
            return graph->GetRuntime()->GetIsChangeFieldOffset(graph->GetArch());
        case ObjectType::MEM_DYN_ARRAY_LENGTH:
            return graph->GetRuntime()->GetDynArrayLenthOffset(graph->GetArch());
        case ObjectType::MEM_DYN_INLINED:
            return typeId;
        case ObjectType::MEM_DYN_CLASS:
            return graph->GetRuntime()->GetObjClassOffset(graph->GetArch());
        case ObjectType::MEM_DYN_METHOD:
            return graph->GetRuntime()->GetFunctionTargetOffset(graph->GetArch());
        case ObjectType::MEM_DYN_HCLASS:
            return graph->GetRuntime()->GetHClassOffset(graph->GetArch());
        default:
            return graph->GetRuntime()->GetFieldOffset(field);
    }
}

}  // namespace ark::compiler
