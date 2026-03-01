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

#include <algorithm>
#include <sstream>

#include "transforms/passes/gc_intrusion.h"

#include "llvm_compiler_options.h"
#include "transforms/gc_utils.h"
#include "transforms/transform_utils.h"

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/SetVector.h>
#include <llvm/ADT/PostOrderIterator.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/MDBuilder.h>
#include <llvm/IR/Statepoint.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Pass.h>
#include <llvm/Transforms/Utils/BasicBlockUtils.h>

#define DEBUG_TYPE "gc-intrusion"

using llvm::ArrayRef;
using llvm::BasicBlock;
using llvm::DenseMap;
using llvm::DenseSet;
using llvm::Function;
using llvm::FunctionAnalysisManager;
using llvm::Optional;
using llvm::SetVector;
using llvm::SmallVector;
using llvm::Use;
using llvm::Value;

using llvm::Argument;
using llvm::CallInst;
using llvm::CastInst;
using llvm::Instruction;
using llvm::PHINode;

using ark::llvmbackend::gc_utils::IsDerived;
using ark::llvmbackend::gc_utils::IsGcRefType;

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static llvm::cl::opt<bool> g_moveCmps("gc-intrusion-move-cmps", llvm::cl::Hidden, llvm::cl::init(true),
                                      llvm::cl::desc("Move comparisons closer to their usages"));

namespace ark::llvmbackend::passes {

class GcRefLiveness {
    using BlockValuesMap = DenseMap<BasicBlock *, SetVector<Value *>>;

public:
    explicit GcRefLiveness(Function *function)
    {
        LLVM_DEBUG(llvm::dbgs() << "Running GC references liveness analysis for: " << function->getName() << "\n");
        ComputeLiveSets(function);
        LLVM_DEBUG(Dump());
    }

    const SetVector<Value *> &GetLiveInsForBlock(BasicBlock *block) const
    {
        ASSERT(liveIns_.find(block) != liveIns_.end());
        return liveIns_.find(block)->second;
    }

    const SetVector<Value *> &GetLiveOutsForBlock(BasicBlock *block) const
    {
        ASSERT(liveOuts_.find(block) != liveOuts_.end());
        return liveOuts_.find(block)->second;
    }

    void ReplaceValue(Value *from, Value *to);

private:
    void PopulateLiveInByUsers(BasicBlock &block);

    void PopulateLiveOutByPhis(BasicBlock &block);

    bool PropagateLiveInsLiveOuts(BasicBlock *block);

    void ComputeLiveSets(Function *function);

    void Dump() const;

    void DumpBlockValuesMap(const BlockValuesMap &map, const char *header = nullptr) const;

    BlockValuesMap liveIns_;
    BlockValuesMap liveOuts_;
};

void GcRefLiveness::DumpBlockValuesMap(const BlockValuesMap &map, [[maybe_unused]] const char *header) const
{
    LLVM_DEBUG(llvm::dbgs() << header << "\n");
    for (const auto &elt : map) {
        LLVM_DEBUG(llvm::dbgs() << "\t");
        LLVM_DEBUG(elt.first->printAsOperand(llvm::dbgs(), false));
        LLVM_DEBUG(llvm::dbgs() << ":");
        for ([[maybe_unused]] const auto &var : elt.second) {
            LLVM_DEBUG(llvm::dbgs() << " "; var->printAsOperand(llvm::dbgs(), false));
        }
        LLVM_DEBUG(llvm::dbgs() << "\n");
    }
}

#ifndef NDEBUG
void GcRefLiveness::Dump() const
{
    DumpBlockValuesMap(liveIns_, "LiveIns:");
    DumpBlockValuesMap(liveOuts_, "LiveOuts:");
}
#endif

void GcRefLiveness::ReplaceValue(Value *from, Value *to)
{
    for (auto &entry : liveIns_) {
        if (entry.second.remove(from)) {
            entry.second.insert(to);
        }
    }
    for (auto &entry : liveOuts_) {
        if (entry.second.remove(from)) {
            entry.second.insert(to);
        }
    }
}

void GcRefLiveness::PopulateLiveInByUsers(BasicBlock &block)
{
    for (auto &inst : block) {
        if (!IsGcRefType(inst.getType()) || IsDerived(&inst) || gc_utils::IsNonMovable(&inst)) {
            continue;
        }
        for (auto user : inst.users()) {
            auto userBlock = llvm::cast<Instruction>(user)->getParent();
            // Definitions and phi inputs are not live-ins
            if (userBlock == &block || llvm::isa<PHINode>(user)) {
                continue;
            }
            liveIns_[userBlock].insert(&inst);
        }
    }
}

void GcRefLiveness::PopulateLiveOutByPhis(BasicBlock &block)
{
    for (auto &phi : block.phis()) {
        if (!IsGcRefType(phi.getType())) {
            continue;
        }
        ASSERT(!IsDerived(&phi));
        for (auto &incoming : phi.incoming_values()) {
            // GcRegType has been already checked
            if (llvm::isa<Argument>(incoming) || llvm::isa<Instruction>(incoming)) {
                liveOuts_[phi.getIncomingBlock(incoming)].insert(llvm::cast<Value>(&incoming));
            }
        }
    }
}

bool GcRefLiveness::PropagateLiveInsLiveOuts(BasicBlock *block)
{
    // Fill live-outs based on live-ins of successors
    auto &blockOuts = liveOuts_[block];
    for (auto succ : llvm::successors(block)) {
        for (auto &succIn : liveIns_[succ]) {
            blockOuts.insert(succIn);
        }
    }

    // Fill live-ins based on live-outs that is not defined in block
    auto &blockIns = liveIns_[block];
    auto updated = false;
    for (auto out : blockOuts) {
        auto inst = llvm::dyn_cast<Instruction>(out);
        // If defined in block, it is not a live-in
        if (inst != nullptr) {
            if (inst->getParent() == block || gc_utils::IsNonMovable(inst)) {
                continue;
            }
        }
        updated |= blockIns.insert(out);
    }
    return updated;
}
void GcRefLiveness::ComputeLiveSets(Function *function)
{
    SetVector<BasicBlock *> worklist;
    // Push live-ins of arguments using its uses.
    for (auto &arg : function->args()) {
        // Arguments are considered as always non-derived.
        if (IsGcRefType(arg.getType())) {
            for (auto user : arg.users()) {
                liveIns_[llvm::cast<Instruction>(user)->getParent()].insert(&arg);
            }
        }
    }
    // Push live-ins of definitions using its uses.
    for (auto &block : *function) {
        worklist.insert(&block);
        PopulateLiveInByUsers(block);
        PopulateLiveOutByPhis(block);
    }

    DumpBlockValuesMap(liveIns_, "LiveIns init:");

    // Iteratively propagate live-ins and live-outs.
    while (!worklist.empty()) {
        auto block = worklist.pop_back_val();
        auto updated = PropagateLiveInsLiveOuts(block);
        // If live-ins have been updated, we reconsider all predecessors
        if (updated) {
            worklist.insert(pred_begin(block), pred_end(block));
        }
    }
}

bool GcIntrusion::ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return options->gcIntrusion;
}

llvm::PreservedAnalyses GcIntrusion::run(Function &function, FunctionAnalysisManager & /*AM*/)
{
    LLVM_DEBUG(llvm::dbgs() << "Function: " << function.getName() << "\n" << function << "\n");

    if (!gc_utils::IsFunctionSupplemental(function)) {
        function.setCallingConv(llvm::CallingConv::ArkMethod);
    }

    if (!gc_utils::IsGcFunction(function) || gc_utils::IsFunctionSupplemental(function)) {
        return llvm::PreservedAnalyses::all();
    }

    if (g_moveCmps) {
        MoveComparisons(&function);
    }
    HoistForRelocation(&function);

    // Set up auxiliary structures
    GcRefLiveness liveness(&function);
    DenseSet<BasicBlock *> visited;
    llvm::ReversePostOrderTraversal<Function *> rpo(&function);
    GcIntrusionContext gcContext;
    uint32_t rpoCounter = 0;
    uint64_t orderCounter = 0;
    for (auto block : rpo) {
        gcContext.rpoMap[block] = rpoCounter++;
        for (auto &inst : *block) {
            // Leave space before calls for possible new instructions
            if (llvm::isa<CallInst>(inst)) {
                orderCounter++;
            }
            gcContext.orderMap[&inst] = orderCounter++;
        }
    }
    SetVector<Value *> alive;
    // Iterate through instructions and replace each instruction where GC can happen with statepoint
    for (auto block : rpo) {
        RewriteWithGcInBlock(block, &liveness, &alive, &visited, &gcContext);
    }

    // Update phis because we had no information on backedges
    for (auto &block : function) {
        UpdatePhiInputs(&block, &gcContext.relocs);
    }
    return llvm::PreservedAnalyses::none();
}

void GcIntrusion::RewriteWithGcInBlock(BasicBlock *block, GcRefLiveness *liveness, SetVector<Value *> *alive,
                                       DenseSet<BasicBlock *> *visited, GcIntrusionContext *gcContext)
{
    auto &liveOuts = liveness->GetLiveOutsForBlock(block);

    alive->insert(liveOuts.begin(), liveOuts.end());
    auto inst = &(*block->rbegin());
    while (inst != nullptr && !llvm::isa<PHINode>(inst)) {
        alive->remove(inst);
        for (auto &ops : inst->operands()) {
            if (IsGcRefType(ops->getType()) && (llvm::isa<Argument>(ops) || llvm::isa<Instruction>(ops)) &&
                !IsDerived(ops) && !gc_utils::IsNonMovable(ops)) {
                alive->insert(ops);
            }
        }

        auto call = llvm::dyn_cast<CallInst>(inst);
        inst = inst->getPrevNode();
        if (call != nullptr && !call->isInlineAsm() && call->getIntrinsicID() == llvm::Intrinsic::not_intrinsic &&
            (call->getDebugLoc() || call->hasFnAttr("safepoint"))) {
            RewriteWithGc(call, liveness, alive, gcContext);
        }
    }
    PropagateRelocs(liveness, visited, block, gcContext);
    visited->insert(block);
    alive->clear();
}

/*static*/
bool GcIntrusion::ComesBefore(Value *a, Value *b, InstructionOrderMap *orderMap)
{
    return orderMap->lookup(a) <= orderMap->lookup(b);
}

uint32_t GcIntrusion::GetStatepointId(const Instruction &inst)
{
    if (inst.getDebugLoc()) {
        return inst.getDebugLoc().getLine();
    }
    auto call = llvm::dyn_cast<llvm::CallBase>(&inst);
    if (call != nullptr && call->hasFnAttr("safepoint")) {
        return 0;
    }
    llvm_unreachable("Unknown statepoint-id metadata");
}

/// Copy relocation info about BLOCK live-ins for BLOCK from its unique predecessor
void GcIntrusion::CopySinglePredRelocs(GcRefLiveness *liveness, BasicBlock *block, GcIntrusionContext *gcContext)
{
    LLVM_DEBUG(llvm::dbgs() << "Propagate single predecessor liveins for ");
    LLVM_DEBUG(block->printAsOperand(llvm::dbgs(), false));
    LLVM_DEBUG(llvm::dbgs() << "\n");
    for (auto var : liveness->GetLiveInsForBlock(block)) {
        auto pred = block->getUniquePredecessor();
        auto varIter = gcContext->relocs.find(var);
        // No relocations have been met
        if (varIter == gcContext->relocs.end()) {
            continue;
        }
        auto &varRelocs = varIter->second;
        if (varRelocs.find(pred) != varRelocs.end()) {
            LLVM_DEBUG(llvm::dbgs() << "\t");
            LLVM_DEBUG(var->printAsOperand(llvm::dbgs(), false));
            LLVM_DEBUG(llvm::dbgs() << " :");

            auto alt = varRelocs[pred];
            varRelocs.insert({block, alt});
            ReplaceDominatedUses(var, alt, block, gcContext);

            LLVM_DEBUG(llvm::dbgs() << " ");
            LLVM_DEBUG(alt->printAsOperand(llvm::dbgs(), false));
            LLVM_DEBUG(llvm::dbgs() << " \n");
        }
    }
}

void GcIntrusion::ReplaceWithPhi(Value *var, BasicBlock *block, GcIntrusionContext *gcContext)
{
    ASSERT(var != nullptr);
    auto &varBlocks = gcContext->relocs.FindAndConstruct(var).second;

    PHINode *phi = PHINode::Create(var->getType(), pred_size(block), "", &(*block->begin()));
    if (var->hasName()) {
        phi->setName(var->getName() + ".rel.phi");
    }

    for (auto pred : predecessors(block)) {
        if (varBlocks.find(pred) != varBlocks.end()) {
            phi->addIncoming(varBlocks[pred], pred);
        } else {
            phi->addIncoming(var, pred);
        }
    }
    LLVM_DEBUG(llvm::dbgs() << *phi << " \n");
    varBlocks.insert({block, phi});
    ReplaceDominatedUses(var, phi, block, gcContext);
}

void GcIntrusion::PropagateRelocs(GcRefLiveness *liveness, DenseSet<BasicBlock *> *visited, BasicBlock *block,
                                  GcIntrusionContext *gcContext)
{
    if (pred_empty(block)) {
        return;
    }
    if (pred_size(block) == 1) {
        CopySinglePredRelocs(liveness, block, gcContext);
        return;
    }

    LLVM_DEBUG(llvm::dbgs() << "Update liveins in ");
    LLVM_DEBUG(block->printAsOperand(llvm::dbgs(), false));
    LLVM_DEBUG(llvm::dbgs() << "\n");
    for (auto var : liveness->GetLiveInsForBlock(block)) {
        bool backEdge = false;
        bool valid = true;
        for (auto pred : predecessors(block)) {
            backEdge |= visited->find(pred) == visited->end();
            // Variable is not a live-out of one of predecessors. No need to merge.
            valid &= liveness->GetLiveOutsForBlock(pred).contains(var);
        }
        if (!valid) {
            continue;
        }

        Value *unique = nullptr;
        LLVM_DEBUG(llvm::dbgs() << "\t");
        LLVM_DEBUG(var->printAsOperand(llvm::dbgs(), false));
        LLVM_DEBUG(llvm::dbgs() << " : ");

        if (!backEdge && (unique = GetUniqueLiveOut(&gcContext->relocs, block, var)) != nullptr) {
            auto &varBlocks = gcContext->relocs.FindAndConstruct(var).second;
            // It is not a backedge and all predecessors has the same live-out.  Just update uses in
            // the BLOCK.
            LLVM_DEBUG(unique->printAsOperand(llvm::dbgs(), false));
            LLVM_DEBUG(llvm::dbgs() << "\n");
            varBlocks.insert({block, unique});
            ReplaceDominatedUses(var, unique, block, gcContext);
        } else {
            // Otherwise we need to create a phi instruction
            ReplaceWithPhi(var, block, gcContext);
        }
    }
}

Value *GcIntrusion::GetUniqueLiveOut(SSAVarRelocs *relocs, BasicBlock *block, Value *var) const
{
    auto varEnt = relocs->find(var);
    // No info about relocated values has been collected. Var is unique by itself
    if (varEnt == relocs->end()) {
        return var;
    }
    auto &varBlocks = varEnt->second;
    Value *unique = nullptr;
    for (auto pred : predecessors(block)) {
        if (varBlocks.find(pred) != varBlocks.end()) {
            if (unique == nullptr) {
                unique = varBlocks[pred];
            } else if (unique != varBlocks[pred]) {
                return nullptr;
            }
        } else {
            if (unique == nullptr) {
                unique = var;
            } else if (unique != var) {
                return nullptr;
            }
        }
    }
    return unique;
}

void GcIntrusion::ReplaceWithRelocated(CallInst *call, CallInst *gcCall, Value *inst, Value *relocated,
                                       GcIntrusionContext *gcContext)
{
    if (inst->hasName()) {
        relocated->setName(inst->getName() + ".relocated");
    }
    gcContext->relocs.FindAndConstruct(inst).second.insert({call->getParent(), relocated});
    // Since relocates don't use each other, it doesn't matter if they all have the same order
    gcContext->orderMap.FindAndConstruct(relocated).second = gcContext->orderMap.lookup(call);
    bool needInsert = gcContext->sortedUses.count(inst) != 0;
    ReplaceDominatedUses(inst, relocated, call->getParent(), gcContext);
    if (needInsert) {
        for (auto useIter = inst->use_begin(), useEnd = inst->use_end(); useIter != useEnd;) {
            auto &gcUse = *useIter++;
            if (gcUse.getUser() != gcCall) {
                break;
            }
            gcContext->sortedUses.FindAndConstruct(inst).second.push_back(&gcUse);
        }
    }
}

void GcIntrusion::RewriteWithGc(CallInst *call, GcRefLiveness *liveness, SetVector<Value *> *refs,
                                GcIntrusionContext *gcContext)
{
    llvm::IRBuilder<> builder(call);

    std::vector<Value *> callArgs(call->arg_begin(), call->arg_end());
    std::vector<Value *> gced(refs->begin(), refs->end());
    const auto &bundle = call->getOperandBundle(llvm::LLVMContext::OB_deopt);
    CallInst *gcCall = nullptr;
    if (bundle && !bundle->Inputs.empty()) {
        ASSERT(!call->hasFnAttr("inline-info"));
        gcCall = builder.CreateGCStatepointCall(GetStatepointId(*call), 0,
                                                llvm::FunctionCallee(call->getFunctionType(), call->getCalledOperand()),
                                                0, callArgs, llvm::None, bundle->Inputs, gced);
    } else {
        auto inlineInfo = GetDeoptsFromInlineInfo(builder, call);
        Optional<ArrayRef<Value *>> deoptArgs = inlineInfo.empty() ? llvm::None : llvm::makeArrayRef(inlineInfo);
        gcCall = builder.CreateGCStatepointCall(GetStatepointId(*call), 0,
                                                llvm::FunctionCallee(call->getFunctionType(), call->getCalledOperand()),
                                                callArgs, deoptArgs, gced);
    }
    gcCall->setCallingConv(call->getCallingConv());
    gcContext->orderMap.FindAndConstruct(gcCall).second = gcContext->orderMap.lookup(call) - 1;

    for (size_t i = 0; i < callArgs.size(); i++) {
        if (call->getParamAttr(i, llvm::Attribute::SExt).isValid()) {
            gcCall->addParamAttr(llvm::GCStatepointInst::CallArgsBeginPos + i, llvm::Attribute::SExt);
        }
        if (call->getParamAttr(i, llvm::Attribute::ZExt).isValid()) {
            gcCall->addParamAttr(llvm::GCStatepointInst::CallArgsBeginPos + i, llvm::Attribute::ZExt);
        }
    }

    if (call->hasFnAttr("use-ark-spills")) {
        gcCall->addFnAttr(call->getFnAttr("use-ark-spills"));
    }

    for (size_t i = 0; i < gced.size(); ++i) {
        auto inst = gced[i];
        auto relocated = builder.CreateGCRelocate(gcCall, i, i, inst->getType());
        ReplaceWithRelocated(call, gcCall, inst, relocated, gcContext);
    }

    if (!call->getType()->isVoidTy()) {
        auto ret = builder.CreateGCResult(gcCall, call->getType());
        liveness->ReplaceValue(call, ret);
        BasicBlock::iterator ii(call);
        if (gcContext->relocs.find(call) != gcContext->relocs.end()) {
            gcContext->relocs.insert({ret, gcContext->relocs.lookup(call)});
            gcContext->relocs.erase(call);
        }
        gcContext->sortedUses.erase(call);
        ReplaceInstWithValue(call->getParent()->getInstList(), ii, ret);
    } else {
        call->eraseFromParent();
    }
}

/// Create deopts from `inline-info` attribute
std::vector<Value *> GcIntrusion::GetDeoptsFromInlineInfo(llvm::IRBuilder<> &builder, CallInst *call)
{
    if (!call->hasFnAttr("inline-info")) {
        return {};
    }

    std::vector<Value *> inlineInfo;
    auto attr = call->getFnAttr("inline-info");
    ASSERT(attr.isStringAttribute());
    ASSERT(!attr.getValueAsString().empty());
    std::stringstream stream(attr.getValueAsString().str());

    constexpr auto RECORD_SIZE = 4U;
    constexpr auto POISON_LINE_NUMBER = 0;

    auto debugLoc = call->getDebugLoc().get();
    /* If llvm can't merge debug location properly, then it set column = 0 */
    if (debugLoc != nullptr && debugLoc->getColumn() == 0) {
        debugLoc = nullptr;
    }

    std::vector<size_t> savedBpc;
    for (size_t id = 0; stream >> id;) {
        auto bpc = debugLoc != nullptr ? debugLoc->getLine() : POISON_LINE_NUMBER;
        savedBpc.push_back(bpc);
        inlineInfo.push_back(builder.getInt32(id)); /* Method ID */
        inlineInfo.push_back(nullptr);              /* BPC */
        inlineInfo.push_back(builder.getInt32(0));  /* need_regmap */
        inlineInfo.push_back(builder.getInt32(0));  /* vregs_total */
        debugLoc = debugLoc != nullptr ? debugLoc->getInlinedAt() : debugLoc;
        if (stream.peek() == ',') {
            stream.ignore();
        }
    }
    auto depth = savedBpc.size();

    /* Sequence of bpc is inverted, we need reverse it */
    for (size_t i = 0; i < inlineInfo.size(); i += RECORD_SIZE) {
        auto bpc = savedBpc.back();
        inlineInfo[i + 1U] = builder.getInt32(bpc);
        savedBpc.pop_back();
    }

    inlineInfo.resize(inlineInfo.size() + depth + 1);
    std::rotate(inlineInfo.rbegin(), inlineInfo.rbegin() + depth + 1, inlineInfo.rend());
    inlineInfo.front() = builder.getInt32(depth);
    for (size_t i = 0; i < depth; ++i) {
        inlineInfo[i + 1] = builder.getInt32(1 + depth + i * RECORD_SIZE);
    }

    return inlineInfo;
}

void GcIntrusion::CreateSortedUseList(BasicBlock *block, Value *from, GcIntrusionContext *gcContext)
{
    auto comp = [gcContext](const Use *u1, const Use *u2) -> bool {
        auto *uinst1 = llvm::cast<Instruction>(u1->getUser());
        auto *uinst2 = llvm::cast<Instruction>(u2->getUser());
        if (uinst1->getParent() == uinst2->getParent()) {
            return ComesBefore(uinst1, uinst2, &(gcContext->orderMap));
        }
        return gcContext->rpoMap.lookup(uinst1->getParent()) > gcContext->rpoMap.lookup(uinst2->getParent());
    };
    auto &useList = gcContext->sortedUses.FindAndConstruct(from).second;
    for (auto &use : from->uses()) {
        if (gcContext->rpoMap.lookup(llvm::cast<Instruction>(use.getUser())->getParent()) >=
            gcContext->rpoMap.lookup(block)) {
            auto it = std::upper_bound(useList.begin(), useList.end(), &use, comp);
            useList.insert(it, &use);
        }
    }
}

/// Replace uses of FROM to TO in scope of BLOCK.
void GcIntrusion::ReplaceDominatedUses(Value *from, Value *to, BasicBlock *block, GcIntrusionContext *gcContext)
{
    if (gcContext->sortedUses.count(from) == 0) {
        CreateSortedUseList(block, from, gcContext);
    }

    auto &sortedList = gcContext->sortedUses.FindAndConstruct(from).second;
    // needDominance = false assumes that FROM definitely dominates TO
    bool needDominance = llvm::isa<Instruction>(to) && llvm::cast<Instruction>(to)->getParent() == block;
    while (!sortedList.empty()) {
        auto *use = *sortedList.rbegin();
        auto *uinst = llvm::cast<Instruction>(use->getUser());
        // Drop any uses left from previous blocks
        if (gcContext->rpoMap.lookup(uinst->getParent()) < gcContext->rpoMap.lookup(block)) {
            sortedList.pop_back();
            continue;
        }
        // If no more uses in this block
        if (uinst->getParent() != block) {
            return;
        }
        // Do not replace inputs in instruction itself
        if (uinst == to) {
            return;
        }

        if (needDominance) {
            /* Phi use cannot be dominated */
            if (llvm::isa<PHINode>(uinst)) {
                return;
            }
            if (!llvm::isa<PHINode>(to) && !ComesBefore(to, uinst, &(gcContext->orderMap))) {
                return;
            }
        }
        use->set(to);
        sortedList.pop_back();
    }
}

/// Update input values of phi based on relocations made for GC
void GcIntrusion::UpdatePhiInputs(BasicBlock *block, SSAVarRelocs *relocs)
{
    for (auto &phi : block->phis()) {
        for (uint32_t i = 0; i < phi.getNumOperands(); ++i) {
            auto inBlock = phi.getIncomingBlock(i);
            auto inValue = phi.getIncomingValue(i);
            auto relBlocks = relocs->find(inValue);
            if (relBlocks != relocs->end() && relBlocks->second.find(inBlock) != relBlocks->second.end()) {
                phi.setIncomingValue(i, relBlocks->second[inBlock]);
            }
        }
    }
}

void GcIntrusion::HoistForRelocation(Function *function)
{
    SmallVector<Instruction *> toMove;
    llvm::ReversePostOrderTraversal<Function *> rpo(function);
    for (auto block : rpo) {
        for (auto &inst : *block) {
            if ((inst.getOpcode() == Instruction::IntToPtr || inst.getOpcode() == Instruction::AddrSpaceCast) &&
                IsGcRefType(inst.getType())) {
                toMove.push_back(&inst);
            }
        }
    }

    SmallVector<Instruction *> casts;
    for (auto inst : toMove) {
        // We need to traverse a chain of casts in a reverse order
        for (auto cast = llvm::cast<CastInst>(inst); cast != nullptr;
             cast = llvm::dyn_cast<CastInst>(cast->getOperand(0))) {
            casts.push_back(cast);
        }
        // It is proven that inst introduces a GC reference. So, we need to
        // fixup other usages than casts.
        FixupEscapedUsages(inst, casts);

        // Place the first cast right after a definition
        auto last = casts.pop_back_val();
        auto def = llvm::cast<Instruction>(last->getOperand(0));
        if (llvm::isa<PHINode>(def)) {
            // We believe in phis (actually no)
            last->moveBefore(*def->getParent(), def->getParent()->getFirstInsertionPt());
        } else {
            last->moveAfter(def);
        }
        // Put the rest casts after the first cast
        while (!casts.empty()) {
            auto curr = casts.pop_back_val();
            curr->moveAfter(last);
            last = curr;
        }
    }

    if (llvm::verifyFunction(*function, &llvm::errs())) {
        LLVM_DEBUG(function->print(llvm::dbgs()));
        llvm_unreachable("Function verification failed: HoistForRelocation");
    }
}

void GcIntrusion::FixupEscapedUsages(Instruction *inst, const llvm::SmallVector<Instruction *> &casts)
{
    if (casts.empty()) {
        return;
    }
    llvm::IRBuilder<> builder(inst);
    auto def = casts.back()->getOperand(0);
    for (auto uiter = def->user_begin(); uiter != def->user_end();) {
        auto user = *uiter++;
        if (gc_utils::IsAllowedEscapedUser(user)) {
            continue;
        }
        auto phi = llvm::dyn_cast<PHINode>(user);

        if (phi == nullptr) {
            builder.SetInsertPoint(llvm::cast<Instruction>(user));
            auto fixed = CreateBackwardCasts(&builder, inst, casts);
            user->replaceUsesOfWith(def, fixed);
            continue;
        }

        for (size_t i = 0; i < phi->getNumIncomingValues(); ++i) {
            auto ival = phi->getIncomingValue(i);
            if (ival != def) {
                continue;
            }
            auto iblock = phi->getIncomingBlock(i);
            builder.SetInsertPoint(iblock->getTerminator());
            auto fixed = CreateBackwardCasts(&builder, inst, casts);
            phi->setIncomingValueForBlock(iblock, fixed);
        }
    }
}

/// Creates a sequence of casts opposite to a given one.
Value *GcIntrusion::CreateBackwardCasts(llvm::IRBuilder<> *builder, Value *from,
                                        const llvm::SmallVector<Instruction *> &casts)
{
    Value *last = from;
    for (auto cast : casts) {
        auto targetType = cast->getOperand(0)->getType();
        switch (cast->getOpcode()) {
            case Instruction::IntToPtr:
                last = builder->CreatePtrToInt(last, targetType);
                break;
            case Instruction::ZExt:
                last = builder->CreateTrunc(last, targetType);
                break;
            case Instruction::AddrSpaceCast:
                last = builder->CreateAddrSpaceCast(last, targetType);
                break;
            default:
                llvm_unreachable("Unsupported backward cast to fix escaped usages");
                break;
        }
    }
    return last;
}

bool GcIntrusion::MoveComparisons(Function *function)
{
    bool changed = false;
    // clang-format off
    auto getConditionInst = [](Instruction *instruction) -> Instruction* {
        if (auto *branchInst = llvm::dyn_cast<llvm::BranchInst>(instruction)) {
            if (branchInst->isConditional()) {
                return llvm::dyn_cast<Instruction>(branchInst->getCondition());
            }
        }
        return nullptr;
    };
    // clang-format on
    for (auto &basicBlock : *function) {
        Instruction *terminator = basicBlock.getTerminator();
        if (auto *cond = getConditionInst(terminator)) {
            if (llvm::isa<llvm::ICmpInst>(cond) && cond->hasOneUse()) {
                changed = true;
                cond->moveBefore(terminator);
            }
        }
    }
    return changed;
}

}  // namespace ark::llvmbackend::passes
