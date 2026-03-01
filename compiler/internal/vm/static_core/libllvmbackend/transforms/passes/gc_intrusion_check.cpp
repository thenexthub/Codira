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

#include "transforms/passes/gc_intrusion_check.h"

#include "llvm_ark_interface.h"
#include "llvm_compiler_options.h"
#include "transforms/gc_utils.h"

#include <llvm/ADT/PostOrderIterator.h>
#include <llvm/IR/Statepoint.h>
#include <llvm/Pass.h>
#include <llvm/Support/Error.h>

#define DEBUG_TYPE "gc-intrusion-check"

// Basic classes
using llvm::ArrayRef;
using llvm::BasicBlock;
using llvm::DenseSet;
using llvm::Function;
using llvm::FunctionAnalysisManager;
using llvm::Use;
using llvm::Value;
// Instructions
using llvm::Argument;
using llvm::CastInst;
using llvm::Constant;
using llvm::GCRelocateInst;
using llvm::GCStatepointInst;
using llvm::Instruction;
using llvm::IntToPtrInst;
using llvm::LoadInst;
using llvm::PHINode;
using llvm::ZExtInst;
// Gc utils
using ark::llvmbackend::gc_utils::HasBeenGcRef;
using ark::llvmbackend::gc_utils::IsAllowedEscapedUser;
using ark::llvmbackend::gc_utils::IsDerived;
using ark::llvmbackend::gc_utils::IsGcRefType;

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static llvm::cl::opt<bool> g_checkAnyEscaped("gcic-any-escaped", llvm::cl::Hidden, llvm::cl::init(true));

/// No derived references must be presented in deopt and gc-live bundles.
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static llvm::cl::opt<bool> g_checkDerived("gcic-no-derived", llvm::cl::Hidden, llvm::cl::init(true));

/// Unreachable does not follow the gc.relocate instruction.
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static llvm::cl::opt<bool> g_checkUnreachableRelocates("gcic-no-unreachable-relocate", llvm::cl::Hidden,
                                                       llvm::cl::init(false));

/// Non-movable object is not relocated
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static llvm::cl::opt<bool> g_checkNonMovableRelocates("gcic-movable-relocates-only", llvm::cl::Hidden,
                                                      llvm::cl::init(true));

namespace ark::llvmbackend::passes {

void GcIntrusionCheck::CheckStatepoint(const Function &function, const GCStatepointInst &statepoint)
{
    if (!g_checkDerived) {
        return;
    }
    constexpr std::array BUNDLES = {llvm::LLVMContext::OB_deopt, llvm::LLVMContext::OB_gc_live};
    constexpr std::array NAMES = {"deopt", "gc-live"};
    for (unsigned i = 0; i < BUNDLES.size(); ++i) {
        auto entries = GetBundle(statepoint, BUNDLES[i]);
        for (auto &entry : entries) {
            if (!IsDerived(entry)) {
                continue;
            }
            llvm::errs() << "Post GC Intrusion check failed in function " << function.getName() << "\n";
            llvm::errs() << "Incorrect bundle in instruction: " << statepoint << "\n";
            llvm::errs() << NAMES[i] << " bundle contains a derived pointer\n\t" << *entry << "\n";
            llvm::report_fatal_error("Post GC Intrusion check failed for statepoint");
        }
    }
}

void GcIntrusionCheck::CheckRelocate(const Function &function, const GCRelocateInst &relocate)
{
    auto baseCst = llvm::dyn_cast<Constant>(relocate.getBasePtr());
    auto derivedCst = llvm::dyn_cast<Constant>(relocate.getDerivedPtr());
    if (baseCst != nullptr && baseCst->isNullValue()) {
        llvm::errs() << "Post GC Intrusion check failed in function " << function.getName() << "\n";
        llvm::errs() << "relocate instruction " << relocate << " has been generated for a null base reference\n";
        llvm::report_fatal_error("Post GC Intrusion check failed for relocate");
    }
    if (derivedCst != nullptr && derivedCst->isNullValue()) {
        llvm::errs() << "Post GC Intrusion check failed in function " << function.getName() << "\n";
        llvm::errs() << "relocate instruction " << relocate << " has been generated for a null derived reference\n";
        llvm::report_fatal_error("Post GC Intrusion check failed for relocate");
    }
    if (g_checkUnreachableRelocates && llvm::isa_and_nonnull<llvm::UnreachableInst>(relocate.getNextNode())) {
        llvm::errs() << "Post GC Intrusion check failed in function " << function.getName() << "\n";
        llvm::errs() << "relocate instruction " << relocate << " is followed by unreachable instruction\n";
        llvm::report_fatal_error("Post GC Intrusion check failed for relocate");
    }
    if (g_checkNonMovableRelocates) {
        if (baseCst != nullptr && gc_utils::IsNonMovable(baseCst)) {
            llvm::errs() << "Post GC Intrusion check failed in function " << function.getName() << "\n";
            llvm::errs() << "relocate instruction " << relocate << " relocates non-movable value " << *baseCst << "\n";
            llvm::report_fatal_error("Post GC Intrusion check failed for relocate");
        }
        if (derivedCst != nullptr && gc_utils::IsNonMovable(derivedCst)) {
            llvm::errs() << "Post GC Intrusion check failed in function " << function.getName() << "\n";
            llvm::errs() << "relocate instruction " << relocate << " relocates non-movable value " << *derivedCst
                         << "\n";
            llvm::report_fatal_error("Post GC Intrusion check failed for relocate");
        }
    }
}

void GcIntrusionCheck::CheckInstruction(const Function &function, const Instruction &inst)
{
    for (unsigned int i = 0; i < inst.getNumOperands(); i++) {
        auto val = inst.getOperand(i);
        // Support only instructions and arguments
        if (!llvm::isa<Instruction>(val) && !llvm::isa<Argument>(val)) {
            continue;
        }

        // Track only GC refs
        if ((!HasBeenGcRef(val, g_checkAnyEscaped) && !IsHiddenGcRef(val)) || gc_utils::IsNonMovable(val)) {
            continue;
        }

        // Strip any casts
        for (auto cast = llvm::dyn_cast<CastInst>(val); cast != nullptr; cast = llvm::dyn_cast<CastInst>(val)) {
            val = cast->getOperand(0);
        }

        // If it's a function argument, just let FindDefOrStatepoint exhaust all BBs
        auto instVal = llvm::isa<Argument>(val) ? nullptr : llvm::cast<Instruction>(val);
        const Instruction *start = &inst;
        if (llvm::isa<PHINode>(start)) {
            // For PHIs, we want not just any path, but a path that specifically goes
            // from the associated BB. Easiest way is to check if we have
            // a no-statepoint way to the incoming block terminator
            auto phi = llvm::cast<PHINode>(start);
            auto incBlock = phi->getIncomingBlock(i);
            start = incBlock->getTerminator();
        }

        auto statepoint = FindDefOrStatepoint(start, instVal);
        if (statepoint != nullptr) {
            llvm::errs() << "Post GC Intrusion check failed in function " << function.getName() << "\n";
            if (llvm::isa<Argument>(val)) {
                llvm::errs() << "Function argument: ";
            } else if (IsHiddenGcRef(val)) {
                llvm::errs() << "Hidden definition: ";
            } else {
                llvm::errs() << "Definition: ";
            }
            llvm::errs() << *val << "\n";
            llvm::errs() << "Statepoint in between: " << *statepoint << "\n";
            llvm::errs() << "User: " << inst << "\n";
            for (auto cast = llvm::dyn_cast<CastInst>(inst.getOperand(i)); cast != nullptr;
                 cast = llvm::dyn_cast<CastInst>(cast->getOperand(0))) {
                llvm::errs() << "    Casted from: " << *cast << "\n";
            }
            llvm::report_fatal_error("Post GC Intrusion check failed for value input");
        }
    }
}

const Instruction *GcIntrusionCheck::FindDefOrStatepoint(const Instruction *start, const Instruction *def)
{
    DenseSet<const BasicBlock *> blockVisited;
    return FindDefOrStatepointRecursive(start, def, &blockVisited);
}

const Instruction *GcIntrusionCheck::FindDefOrStatepointRecursive(const Instruction *start, const Instruction *def,
                                                                  DenseSet<const BasicBlock *> *visited)
{
    // The idea here is as follows: for invariant to be true we need to know that no paths between
    // definition and user contain statepoint instructions
    auto block = start->getParent();
    start = llvm::isa<GCStatepointInst>(start) ? start->getPrevNode() : start;
    for (auto inst = start; inst != nullptr; inst = inst->getPrevNode()) {
        if (inst == def) {
            return nullptr;
        }
        if (llvm::isa<GCStatepointInst>(inst)) {
            return inst;
        }
    }
    // We've reached the end of BB and haven't found anything. Look in both successors
    for (auto pred : llvm::predecessors(block)) {
        if (visited->find(pred) == visited->end()) {
            visited->insert(pred);
            auto result = FindDefOrStatepointRecursive(pred->getTerminator(), def, visited);
            if (result != nullptr) {
                return result;
            }
        }
    }
    return nullptr;
}

/// Return true if a loaded value is casted to addrspace (271).
bool GcIntrusionCheck::IsHiddenGcRef(Value *ref)
{
    // Checking for escaped i32 pointer
    constexpr auto POINTER_WIDTH_BITS = 32;
    if (!llvm::isa<LoadInst>(ref) || !ref->getType()->isIntegerTy(POINTER_WIDTH_BITS)) {
        return false;
    }
    for (auto user : ref->users()) {
        auto zext = llvm::dyn_cast<ZExtInst>(user);
        if (zext == nullptr) {
            continue;
        }
        for (auto zextUser : zext->users()) {
            if (llvm::isa<IntToPtrInst>(zextUser) && IsGcRefType(zextUser->getType())) {
                return true;
            }
        }
    }
    return false;
}

ArrayRef<Use> GcIntrusionCheck::GetBundle(const GCStatepointInst &call, uint32_t bundleId)
{
    const auto &bundle = call.getOperandBundle(bundleId);
    if (!bundle) {
        return llvm::None;
    }
    return bundle->Inputs;
}

}  // namespace ark::llvmbackend::passes

namespace ark::llvmbackend::passes {

bool GcIntrusionCheck::ShouldInsert(const ark::llvmbackend::LLVMCompilerOptions *options)
{
    return options->gcIntrusionChecks;
}

llvm::PreservedAnalyses GcIntrusionCheck::run(Function &function, FunctionAnalysisManager & /*AM*/)
{
    llvm::ReversePostOrderTraversal<Function *> rpo(&function);
    for (auto block : rpo) {
        for (auto &inst : *block) {
            if (llvm::isa<GCStatepointInst>(&inst)) {
                CheckStatepoint(function, llvm::cast<GCStatepointInst>(inst));
                CheckInstruction(function, inst);
            } else if (llvm::isa<GCRelocateInst>(&inst)) {
                CheckRelocate(function, llvm::cast<GCRelocateInst>(inst));
            } else if (!IsAllowedEscapedUser(&inst)) {
                CheckInstruction(function, inst);
            }
        }
    }

    return llvm::PreservedAnalyses::all();
}

}  // namespace ark::llvmbackend::passes
