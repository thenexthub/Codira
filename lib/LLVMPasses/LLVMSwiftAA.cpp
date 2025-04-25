//===--- LLVMSwiftAA.cpp - LLVM Alias Analysis for Swift ------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//

#include "language/LLVMPasses/Passes.h"
#include "LLVMARCOpts.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/LegacyPassManager.h" 
#include "llvm/IR/Module.h"

using namespace llvm;
using namespace language;

//===----------------------------------------------------------------------===//
//                           Alias Analysis Result
//===----------------------------------------------------------------------===//

static ModRefInfo getConservativeModRefForKind(const llvm::Instruction &I) {
  switch (classifyInstruction(I)) {
#define KIND(Name, MemBehavior) case RT_ ## Name: return ModRefInfo:: MemBehavior;
#include "LLVMSwift.def"
  }

  llvm_unreachable("Not a valid Instruction.");
}

ModRefInfo SwiftAAResult::getModRefInfo(const llvm::CallBase *Call,
                                        const llvm::MemoryLocation &Loc,
                                        llvm::AAQueryInfo &AAQI) {
  // We know at compile time that certain entry points do not modify any
  // compiler-visible state ever. Quickly check if we have one of those
  // instructions and return if so.
  if (ModRefInfo::NoModRef == getConservativeModRefForKind(*Call))
    return ModRefInfo::NoModRef;

  // Otherwise, delegate to the rest of the AA ModRefInfo machinery.
  return AAResultBase::getModRefInfo(Call, Loc, AAQI);
}

//===----------------------------------------------------------------------===//
//                        Alias Analysis Wrapper Pass
//===----------------------------------------------------------------------===//

char SwiftAAWrapperPass::ID = 0;
INITIALIZE_PASS_BEGIN(SwiftAAWrapperPass, "swift-aa",
                      "Swift Alias Analysis", false, true)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_END(SwiftAAWrapperPass, "swift-aa",
                    "Swift Alias Analysis", false, true)

SwiftAAWrapperPass::SwiftAAWrapperPass() : ImmutablePass(ID) {
  initializeSwiftAAWrapperPassPass(*PassRegistry::getPassRegistry());
}

bool SwiftAAWrapperPass::doInitialization(Module &M) {
  Result.reset(new SwiftAAResult());
  return false;
}

bool SwiftAAWrapperPass::doFinalization(Module &M) {
  Result.reset();
  return false;
}

void SwiftAAWrapperPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
}

AnalysisKey SwiftAA::Key;

SwiftAAResult SwiftAA::run(llvm::Function &F,
                           llvm::FunctionAnalysisManager &AM) {
  return SwiftAAResult();
}

//===----------------------------------------------------------------------===//
//                           Top Level Entry Point
//===----------------------------------------------------------------------===//

llvm::ImmutablePass *swift::createSwiftAAWrapperPass() {
  return new SwiftAAWrapperPass();
}
