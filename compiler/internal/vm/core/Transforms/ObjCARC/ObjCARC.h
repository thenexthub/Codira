//===- ObjCARC.h - ObjC ARC Optimization --------------*- C++ -*-----------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file defines common definitions/declarations used by the ObjC ARC
/// Optimizer. ARC stands for Automatic Reference Counting and is a system for
/// managing reference counts for objects in Objective C.
///
/// WARNING: This file knows about certain library functions. It recognizes them
/// by name, and hardwires knowledge of their semantics.
///
/// WARNING: This file knows about how certain Objective-C library functions are
/// used. Naive LLVM IR transformations which would otherwise be
/// behavior-preserving may break these assumptions.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_OBJCARC_OBJCARC_H
#define LLVM_LIB_TRANSFORMS_OBJCARC_OBJCARC_H

#include "ARCRuntimeEntryPoints.h"
#include "vm/core/Analysis/ObjCARCAnalysisUtils.h"
#include "vm/core/Analysis/ObjCARCUtil.h"
#include "vm/core/IR/EHPersonalities.h"
#include "vm/core/Transforms/Utils/Local.h"

namespace vm::core {
namespace objcarc {

/// Erase the given instruction.
///
/// Many ObjC calls return their argument verbatim,
/// so if it's such a call and the return value has users, replace them with the
/// argument value.
///
static inline void EraseInstruction(Instruction *CI) {
  Value *OldArg = cast<CallInst>(CI)->getArgOperand(0);

  bool Unused = CI->use_empty();

  if (!Unused) {
    // Replace the return value with the argument.
    assert((IsForwarding(GetBasicARCInstKind(CI)) ||
            (IsNoopOnNull(GetBasicARCInstKind(CI)) &&
             IsNullOrUndef(OldArg->stripPointerCasts()))) &&
           "Can't delete non-forwarding instruction with users!");
    CI->replaceAllUsesWith(OldArg);
  }

  CI->eraseFromParent();

  if (Unused)
    RecursivelyDeleteTriviallyDeadInstructions(OldArg);
}

/// If Inst is a ReturnRV and its operand is a call or invoke, return the
/// operand. Otherwise return null.
static inline const Instruction *getreturnRVOperand(const Instruction &Inst,
                                                    ARCInstKind Class) {
  if (Class != ARCInstKind::RetainRV)
    return nullptr;

  const auto *Opnd = Inst.getOperand(0)->stripPointerCasts();
  if (const auto *C = dyn_cast<CallInst>(Opnd))
    return C;
  return dyn_cast<InvokeInst>(Opnd);
}

/// Return the list of PHI nodes that are equivalent to PN.
template<class PHINodeTy, class VectorTy>
void getEquivalentPHIs(PHINodeTy &PN, VectorTy &PHIList) {
  auto *BB = PN.getParent();
  for (auto &P : BB->phis()) {
    if (&P == &PN) // Do not add PN to the list.
      continue;
    unsigned I = 0, E = PN.getNumIncomingValues();
    for (; I < E; ++I) {
      auto *BB = PN.getIncomingBlock(I);
      auto *PNOpnd = PN.getIncomingValue(I)->stripPointerCasts();
      auto *POpnd = P.getIncomingValueForBlock(BB)->stripPointerCasts();
      if (PNOpnd != POpnd)
        break;
    }
    if (I == E)
      PHIList.push_back(&P);
  }
}

static inline MDString *getRVInstMarker(Module &M) {
  const char *MarkerKey = getRVMarkerModuleFlagStr();
  return dyn_cast_or_null<MDString>(M.getModuleFlag(MarkerKey));
}

/// Create a call instruction with the correct funclet token. This should be
/// called instead of calling CallInst::Create directly unless the call is
/// going to be removed from the IR before WinEHPrepare.
CallInst *createCallInstWithColors(
    FunctionCallee Func, ArrayRef<Value *> Args, const Twine &NameStr,
    BasicBlock::iterator InsertBefore,
    const DenseMap<BasicBlock *, ColorVector> &BlockColors);

class BundledRetainClaimRVs {
public:
  BundledRetainClaimRVs(ARCRuntimeEntryPoints &EP, bool ContractPass,
                        bool UseClaimRV)
      : EP(EP), ContractPass(ContractPass), UseClaimRV(UseClaimRV) {}
  ~BundledRetainClaimRVs();

  /// Insert a retainRV/claimRV call to the normal destination blocks of invokes
  /// with operand bundle "clang.arc.attachedcall". If the edge to the normal
  /// destination block is a critical edge, split it.
  std::pair<bool, bool> insertAfterInvokes(Function &F, DominatorTree *DT);

  /// Insert a retainRV/claimRV call.
  CallInst *insertRVCall(BasicBlock::iterator InsertPt,
                         CallBase *AnnotatedCall);

  /// Insert a retainRV/claimRV call with colors.
  CallInst *insertRVCallWithColors(
      BasicBlock::iterator InsertPt, CallBase *AnnotatedCall,
      const DenseMap<BasicBlock *, ColorVector> &BlockColors);

  /// See if an instruction is a bundled retainRV/claimRV call.
  bool contains(const Instruction *I) const {
    if (auto *CI = dyn_cast<CallInst>(I))
      return RVCalls.count(CI);
    return false;
  }

  /// Remove a retainRV/claimRV call entirely.
  void eraseInst(CallInst *CI) {
    auto It = RVCalls.find(CI);
    if (It != RVCalls.end()) {
      // Remove call to @toolchain.objc.clang.arc.noop.use.
      for (User *U : It->second->users())
        if (auto *CI = dyn_cast<CallInst>(U))
          if (CI->getIntrinsicID() == Intrinsic::objc_clang_arc_noop_use) {
            CI->eraseFromParent();
            break;
          }

      auto *NewCall = CallBase::removeOperandBundle(
          It->second, LLVMContext::OB_clang_arc_attachedcall,
          It->second->getIterator());
      NewCall->copyMetadata(*It->second);
      It->second->replaceAllUsesWith(NewCall);
      It->second->eraseFromParent();
      RVCalls.erase(It);
    }
    EraseInstruction(CI);
  }

private:
  /// A map of inserted retainRV/claimRV calls to annotated calls/invokes.
  DenseMap<CallInst *, CallBase *> RVCalls;

  ARCRuntimeEntryPoints &EP;
  bool ContractPass;
  bool UseClaimRV;
};

} // end namespace objcarc
} // end namespace vm::core

#endif
