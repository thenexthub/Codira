/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "transforms/passes/gep_propagation.h"

#include "transforms/gc_utils.h"
#include "transforms/transform_utils.h"

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/PostOrderIterator.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/Pass.h>
#include <llvm/Support/CommandLine.h>

#define DEBUG_TYPE "gep-propagation"

// Basic classes
using llvm::BasicBlock;
using llvm::DenseMap;
using llvm::Function;
using llvm::SmallVector;
using llvm::Type;
using llvm::Use;
using llvm::Value;
// Instructions
using llvm::CastInst;
using llvm::Constant;
using llvm::GetElementPtrInst;
using llvm::Instruction;
using llvm::PHINode;
using llvm::SelectInst;
// Gc utils
using ark::llvmbackend::gc_utils::IsGcRefType;

/// Optimize no-op PHINodes and Selects in place.
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static llvm::cl::opt<bool> g_optimizeNoop("gprop-optimize", llvm::cl::Hidden, llvm::cl::init(true));

namespace ark::llvmbackend::passes {

llvm::PreservedAnalyses GepPropagation::run(llvm::Function &function, llvm::FunctionAnalysisManager & /*AM*/)
{
    LLVM_DEBUG(llvm::dbgs() << "Function: " << function.getName() << "\n");
    if (!gc_utils::IsGcFunction(function) || gc_utils::IsFunctionSupplemental(function)) {
        return llvm::PreservedAnalyses::all();
    }

    Propagate(&function);
    return llvm::PreservedAnalyses::none();
}

void GepPropagation::AddToVector(Instruction *inst, SmallVector<Instruction *> *toExpand,
                                 SmallVector<Instruction *> *selectors)
{
    switch (inst->getOpcode()) {
        // Derived references
        case Instruction::GetElementPtr:
            if (IsGcRefType(inst->getType())) {
                toExpand->push_back(inst);
            }
            break;
        // Escaping managed scope
        case Instruction::PtrToInt:
        case Instruction::AddrSpaceCast:
            if (IsGcRefType(inst->getOperand(0)->getType())) {
                toExpand->push_back(inst);
            }
            break;
        case Instruction::Select:
        case Instruction::PHI:
            if ((IsGcRefType(inst->getType()) && gc_utils::IsDerived(inst)) || gc_utils::HasBeenGcRef(inst, false)) {
                selectors->push_back(inst);
                toExpand->push_back(inst);
            }
            break;
        default:
            break;
    }
}

void GepPropagation::Propagate(Function *function)
{
    SmallVector<Instruction *> toExpand;
    SmallVector<Instruction *> selectors;
    llvm::ReversePostOrderTraversal<Function *> rpo(function);
    for (auto block : rpo) {
        for (auto &inst : *block) {
            AddToVector(&inst, &toExpand, &selectors);
        }
    }

    DenseMap<Instruction *, Instruction *> sgeps;
    SplitGepSelectors(function, &selectors, &sgeps);

    while (!toExpand.empty()) {
        auto gep = toExpand.pop_back_val();
        if (sgeps.find(gep) != sgeps.end()) {
            gep = sgeps[gep];
        }
        SmallVector<Instruction *, 1> seq;
        ReplaceRecursively(gep, &seq);
    }
}

static std::pair<Instruction *, Instruction *> FindSplitGep(Function *function, Instruction *inst, Instruction **ipoint)
{
    auto &ctx = function->getContext();
    auto bptrTy = llvm::PointerType::get(ctx, ark::llvmbackend::LLVMArkInterface::GC_ADDR_SPACE);
    auto undefBase = llvm::UndefValue::get(bptrTy);
    auto undefOffset = llvm::UndefValue::get(Type::getInt32Ty(ctx));
    auto phi = llvm::dyn_cast<PHINode>(inst);
    Instruction *mbase;
    Instruction *moff;
    if (phi != nullptr) {
        mbase = PHINode::Create(bptrTy, phi->getNumIncomingValues(), "base", phi);
        moff = PHINode::Create(Type::getInt32Ty(ctx), phi->getNumIncomingValues(), "gepoff", phi);
        for (size_t i = 0; i < phi->getNumIncomingValues(); ++i) {
            auto bb = phi->getIncomingBlock(i);
            llvm::cast<PHINode>(mbase)->addIncoming(undefBase, bb);
            llvm::cast<PHINode>(moff)->addIncoming(undefOffset, bb);
        }
        *ipoint = phi->getParent()->getFirstNonPHI();
    } else {
        auto condition = llvm::cast<SelectInst>(inst)->getCondition();
        mbase = SelectInst::Create(condition, undefBase, undefBase, "base", inst);
        moff = SelectInst::Create(condition, undefOffset, undefOffset, "gepoff", inst);
        *ipoint = inst;
    }
    return {mbase, moff};
}

void GepPropagation::SplitGepSelectors(Function *function, SmallVector<Instruction *> *selectors,
                                       DenseMap<Instruction *, Instruction *> *sgeps)
{
    auto &ctx = function->getContext();
    SelectorSplitMap mapping;
    for (auto inst : *selectors) {
        Instruction *ipoint = nullptr;
        auto splitGep = FindSplitGep(function, inst, &ipoint);
        mapping[inst] = splitGep;
        auto gep = GetElementPtrInst::CreateInBounds(Type::getInt8Ty(ctx), splitGep.first, splitGep.second, "", ipoint);
        gep->takeName(inst);
        sgeps->insert({inst, gep});
    }

    for (auto inst : *selectors) {
        ASSERT((llvm::isa<PHINode, SelectInst>(inst)));
        GenerateSelectorInputs(inst, mapping);
        ReplaceWithSplitGep(inst, sgeps->lookup(inst));
    }

    // Erase GEP selectors as we replaced them with their split versions.
    while (!selectors->empty()) {
        auto val = selectors->pop_back_val();
        [[maybe_unused]] auto safeToDelete = [selectors](auto user) {
            auto inst = llvm::cast<Instruction>(user);
            if (!llvm::isa<PHINode, SelectInst>(inst)) {
                return false;
            }
            return std::find(selectors->begin(), selectors->end(), inst) != selectors->end();
        };
        ASSERT(std::find_if_not(val->user_begin(), val->user_end(), safeToDelete) == val->user_end());
        val->replaceAllUsesWith(llvm::UndefValue::get(val->getType()));
        val->eraseFromParent();
    }

    if (g_optimizeNoop) {
        OptimizeSelectors(mapping);
    }
}

std::pair<Value *, Value *> GepPropagation::GenerateInput(Value *input, Instruction *inst, Instruction *inPoint,
                                                          const SelectorSplitMap &mapping)
{
    auto [mbase, moff] = mapping.lookup(inst);
    auto offTy = moff->getType();
    auto instInput = llvm::dyn_cast<Instruction>(input);
    if (instInput != nullptr) {
        auto mapit = mapping.find(instInput);
        if (mapit != mapping.end()) {
            return mapit->second;
        }
    }
    if (auto nulloffset = llvm::dyn_cast<Constant>(input)) {
        return {Constant::getNullValue(mbase->getType()), GetConstantOffset(nulloffset, offTy)};
    }

    auto [base, derived] = GetBasePointer(input);
    Value *offset = nullptr;
    auto instBase = llvm::dyn_cast<Instruction>(base);
    if (instBase != nullptr) {
        auto mapit = mapping.find(instBase);
        if (mapit != mapping.end()) {
            base = mapit->second.first;
            offset = mapit->second.second;
        }
    }

    if (derived) {
        // Calculate offset
        auto baseRaw = CastInst::Create(Instruction::PtrToInt, base, offTy, "", inPoint);
        Value *gepRaw = nullptr;

        ASSERT(input->getType()->isIntOrPtrTy() && "Unexpected type of gep selector");
        if (input->getType()->isPointerTy()) {
            gepRaw = CastInst::Create(Instruction::PtrToInt, input, offTy, "", inPoint);
        } else if (input->getType()->getScalarSizeInBits() < offTy->getScalarSizeInBits()) {
            gepRaw = CastInst::Create(Instruction::ZExt, input, offTy, "", inPoint);
        } else if (input->getType()->getScalarSizeInBits() > offTy->getScalarSizeInBits()) {
            gepRaw = CastInst::Create(Instruction::Trunc, input, offTy, "", inPoint);
        } else {
            gepRaw = input;
        }
        offset = llvm::BinaryOperator::Create(Instruction::Sub, gepRaw, baseRaw, "", inPoint);
    } else if (offset == nullptr) {
        offset = llvm::ConstantInt::getSigned(offTy, 0);
    }
    return {base, offset};
}

void GepPropagation::GenerateSelectorInputs(Instruction *inst, const SelectorSplitMap &mapping)
{
    ASSERT((llvm::isa<SelectInst, PHINode>(inst)));
    auto [mbase, moff] = mapping.lookup(inst);

    auto setInputs = [mbase = mbase, moff = moff](auto idx, auto ibase, auto ioff) {
        if (auto phi = llvm::dyn_cast<PHINode>(mbase)) {
            auto bb = phi->getIncomingBlock(idx);
            phi->setIncomingValueForBlock(bb, ibase);
            llvm::cast<PHINode>(moff)->setIncomingValueForBlock(bb, ioff);
        } else {
            mbase->setOperand(idx, ibase);
            moff->setOperand(idx, ioff);
        }
    };
    // Generate inputs
    bool select = llvm::isa<SelectInst>(inst);
    for (size_t i = select ? 1 : 0; i < inst->getNumOperands(); ++i) {
        auto input = inst->getOperand(i);
        if (!llvm::isa<llvm::UndefValue>(mbase->getOperand(i))) {
            continue;
        }
        auto inPoint = select ? mbase : llvm::cast<PHINode>(inst)->getIncomingBlock(i)->getTerminator();
        auto [base, offset] = GenerateInput(input, inst, inPoint, mapping);
        setInputs(i, base, offset);
    }
}

/// Generate a ConstaintInt of type TYPE that represents an OFFSET.
Value *GepPropagation::GetConstantOffset(Constant *offset, Type *type)
{
    offset = offset->stripPointerCasts();
    if (llvm::isa<llvm::ConstantExpr>(offset)) {
        ASSERT(offset->getNumOperands() == 1);
        auto offsetRaw = offset->getOperand(0);
        return llvm::ConstantInt::getSigned(type, llvm::cast<llvm::ConstantInt>(offsetRaw)->getSExtValue());
    }
    if (offset->isNullValue() || llvm::isa<llvm::PoisonValue, llvm::UndefValue>(offset)) {
        return llvm::ConstantInt::getNullValue(type);
    }

    return llvm::ConstantInt::getSigned(type, llvm::cast<llvm::ConstantInt>(offset)->getSExtValue());
}

std::pair<Value *, bool> GepPropagation::GetBasePointer(Value *value)
{
    bool derived = false;
    auto base = value;
    // This loop is needed to get to addrspace(271) through addrspace (0).
    // It is required if the value is an escaped reference.
    while (!IsGcRefType(base->getType()) && llvm::isa<GetElementPtrInst, CastInst>(base)) {
        derived |= llvm::isa<GetElementPtrInst>(base);
        base = llvm::cast<Instruction>(base)->getOperand(0);
    }
    // Here we try to find a base pointer inside addrspace (271).
    while (llvm::isa<GetElementPtrInst, CastInst>(base)) {
        derived |= llvm::isa<GetElementPtrInst>(base);
        auto next = llvm::cast<Instruction>(base)->getOperand(0);
        // Go until instruction introducing a GC reference.
        if (!IsGcRefType(next->getType())) {
            break;
        }
        base = next;
    }
    ASSERT(IsGcRefType(base->getType()));
    return {base, derived};
}

/// Returns a constant input of a selector.
Value *GepPropagation::GetConstantInput(Instruction *inst)
{
    llvm::DenseSet<Value *> visited;
    llvm::SmallVector<Instruction *> queueLocal;
    queueLocal.push_back(inst);
    Value *single = nullptr;
    while (!queueLocal.empty()) {
        auto elt = queueLocal.pop_back_val();
        if (!visited.insert(elt).second) {
            continue;
        }
        bool select = llvm::isa<SelectInst>(elt);
        for (size_t i = select ? 1 : 0; i < elt->getNumOperands(); ++i) {
            auto input = elt->getOperand(i);
            if (llvm::isa<SelectInst, PHINode>(input)) {
                queueLocal.push_back(llvm::cast<Instruction>(input));
                continue;
            }
            if (single == nullptr) {
                single = input;
            }
            if (single != input) {
                return nullptr;
            }
        }
    }
    return single;
}

void GepPropagation::OptimizeGepoffs(SelectorSplitMap &mapping)
{
    llvm::SmallVector<Instruction *> queue;
    for (auto entry : mapping) {
        queue.push_back(entry.second.second);
    }
    while (!queue.empty()) {
        auto off = queue.pop_back_val();
        auto coff = GetConstantInput(off);
        if (coff == nullptr) {
            continue;
        }
        for (auto uiter = off->use_begin(), uend = off->use_end(); uiter != uend;) {
            Use &use = *uiter++;
            auto user = use.getUser();
            use.set(coff);

            if (llvm::isa<PHINode, SelectInst>(user)) {
                queue.push_back(llvm::cast<Instruction>(user));
                continue;
            }
            auto gep = llvm::dyn_cast<GetElementPtrInst>(user);
            if (gep == nullptr) {
                continue;
            }
            if (gep->hasAllZeroIndices()) {
                auto pointer = gep->getPointerOperand();
                pointer->takeName(gep);
                gep->replaceAllUsesWith(pointer);
            }
        }
    }
}

/// Basic optimization of generated PHI and Select instructions that have identical inputs.
void GepPropagation::OptimizeSelectors(SelectorSplitMap &mapping)
{
    llvm::SmallVector<Instruction *> queue;
    // Optimize bases
    for (auto entry : mapping) {
        queue.push_back(entry.second.first);
    }
    while (!queue.empty()) {
        auto base = queue.pop_back_val();
        auto cbase = GetConstantInput(base);
        if (cbase == nullptr) {
            continue;
        }
        for (auto uiter = base->use_begin(), uend = base->use_end(); uiter != uend;) {
            Use &use = *uiter++;
            auto user = use.getUser();
            use.set(cbase);

            if (llvm::isa<PHINode, SelectInst>(user)) {
                queue.push_back(llvm::cast<Instruction>(user));
            }
        }
    }
    // Optimize gepoffs
    OptimizeGepoffs(mapping);
    // Remove replaced instructions
    for (auto entry : mapping) {
        auto [base, off] = entry.second;
        if (base->user_empty()) {
            base->eraseFromParent();
        }
        if (off->user_empty()) {
            off->eraseFromParent();
        }
    }
}

/// Update users of GEP selector using GEP obtained after selector splitting.
void GepPropagation::ReplaceWithSplitGep(Instruction *inst, Instruction *splitGep)
{
    bool needCast = splitGep->getType() != inst->getType();
    auto generateCast = [splitGep, type = inst->getType()]() {
        auto cast = llvm::Instruction::CastOps::CastOpsEnd;
        if (type->isIntegerTy()) {
            cast = Instruction::PtrToInt;
        } else if (type->isPointerTy()) {
            cast = IsGcRefType(type) ? Instruction::BitCast : Instruction::AddrSpaceCast;
        }
        ASSERT(cast != llvm::Instruction::CastOps::CastOpsEnd && "Unsupported selector type");
        return CastInst::Create(cast, splitGep, type);
    };
    for (auto uiter = inst->use_begin(), uend = inst->use_end(); uiter != uend;) {
        Use &use = *uiter++;
        auto *uinst = llvm::cast<Instruction>(use.getUser());
        if (llvm::isa<PHINode, SelectInst>(uinst)) {
            continue;
        }
        if (needCast) {
            auto cast = generateCast();
            cast->insertBefore(uinst);
            use.set(cast);
        } else {
            use.set(splitGep);
        }
    }
}

/// Recursively collect a sequence of instructions to clone them for each use of this sequence.
void GepPropagation::ReplaceRecursively(Instruction *inst, SmallVector<Instruction *, 1> *seq)
{
    llvm::IRBuilder<> builder(inst);
    // Mapping of (phi, incoming basic block) -> expanded value
    DenseMap<std::pair<Instruction *, BasicBlock *>, Instruction *> phiCache;
    seq->push_back(inst);
    for (auto iter = inst->use_begin(), end = inst->use_end(); iter != end;) {
        Use &use = *iter++;
        auto *uinst = llvm::cast<Instruction>(use.getUser());

        if (uinst->isCast()) {
            // This assert guaranties that we can set zero operands during
            // cloning the sequence of casts
            ASSERT(uinst->getNumOperands() == 1);
            ReplaceRecursively(uinst, seq);
            continue;
        }
        // Reached the end of sequence
        Instruction *clone = nullptr;
        // If use a phi, insert it to the block where inst comes from.
        if (llvm::isa<PHINode>(uinst)) {
            auto incoming = llvm::cast<PHINode>(uinst)->getIncomingBlock(use);
            auto visited = phiCache.find({uinst, incoming});
            if (visited != phiCache.end()) {
                use.set(visited->second);
                continue;
            }
            builder.SetInsertPoint(incoming->getTerminator());
            clone = CloneSequence(&builder, seq);
            phiCache.insert({{uinst, incoming}, clone});
        } else {
            builder.SetInsertPoint(uinst);
            clone = CloneSequence(&builder, seq);
        }
        use.set(clone);
    }
    seq->pop_back();
    // Since we have replaced all uses the original can be erased
    ASSERT(inst->user_empty());
    inst->eraseFromParent();
}

Instruction *GepPropagation::CloneSequence(llvm::IRBuilder<> *builder, SmallVector<Instruction *, 1> *seq)
{
    ASSERT(!seq->empty());
    auto prev = (*seq->begin())->clone();
    builder->Insert(prev);
    for (auto siter = seq->begin() + 1; siter != seq->end(); siter++) {
        auto clone = (*siter)->clone();
        clone->setOperand(0, prev);
        builder->Insert(clone);
        prev = clone;
    }
    return prev;
}

}  // namespace ark::llvmbackend::passes
