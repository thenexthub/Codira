//===--- GenericCloner.h - Specializes generic functions  -------*- C++ -*-===//
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
//
// This contains the definition of a cloner class for creating specialized
// versions of generic functions by substituting concrete types.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_GENERICCLONER_H
#define LANGUAGE_SIL_GENERICCLONER_H

#include "language/AST/Type.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/TypeSubstCloner.h"
#include "language/SILOptimizer/Utils/BasicBlockOptUtils.h"
#include "language/SILOptimizer/Utils/Generics.h"
#include "toolchain/ADT/StringRef.h"
#include <functional>

namespace language {

class GenericCloner
  : public TypeSubstCloner<GenericCloner, SILOptFunctionBuilder> {
  using SuperTy = TypeSubstCloner<GenericCloner, SILOptFunctionBuilder>;

  SILOptFunctionBuilder &FuncBuilder;
  SerializedKind_t Serialized;
  const ReabstractionInfo &ReInfo;
  CloneCollector::CallbackType Callback;
  toolchain::SmallDenseMap<const SILDebugScope *, const SILDebugScope *, 8>
      RemappedScopeCache;

  toolchain::SmallVector<AllocStackInst *, 8> AllocStacks;
  toolchain::SmallVector<StoreBorrowInst *, 8> StoreBorrowsToCleanup;
  AllocStackInst *ReturnValueAddr = nullptr;
  AllocStackInst *ErrorValueAddr = nullptr;

public:
  friend class SILCloner<GenericCloner>;

  GenericCloner(SILOptFunctionBuilder &FuncBuilder, SILFunction *F,
                const ReabstractionInfo &ReInfo, SubstitutionMap ParamSubs,
                StringRef NewName, CloneCollector::CallbackType Callback)
      : SuperTy(*createDeclaration(FuncBuilder, F, ReInfo, NewName), *F,
                ParamSubs),
        FuncBuilder(FuncBuilder), ReInfo(ReInfo), Callback(Callback) {
    assert(F->getDebugScope()->Parent != getCloned()->getDebugScope()->Parent);
  }
  /// Clone and remap the types in \p F according to the substitution
  /// list in \p Subs. Parameters are re-abstracted (changed from indirect to
  /// direct) according to \p ReInfo.
  static SILFunction *
  cloneFunction(SILOptFunctionBuilder &FuncBuilder,
                SILFunction *F,
                const ReabstractionInfo &ReInfo,
                SubstitutionMap ParamSubs,
                StringRef NewName,
                CloneCollector::CallbackType Callback =nullptr) {
    // Clone and specialize the function.
    GenericCloner SC(FuncBuilder, F, ReInfo, ParamSubs,
                     NewName, Callback);
    SC.populateCloned();
    return SC.getCloned();
  }

  void postFixUp(SILFunction *calleeFunction);

  static SILFunction *createDeclaration(SILOptFunctionBuilder &FuncBuilder,
                                        SILFunction *Orig,
                                        const ReabstractionInfo &ReInfo,
                                        StringRef NewName);

protected:
  void visitTerminator(SILBasicBlock *BB);

  // FIXME: We intentionally call SILClonerWithScopes here to ensure
  //        the debug scopes are set correctly for cloned
  //        functions. TypeSubstCloner, SILClonerWithScopes, and
  //        SILCloner desperately need refactoring and/or combining so
  //        that the obviously right things are happening for cloning
  //        vs. inlining.
  void postProcess(SILInstruction *Orig, SILInstruction *Cloned) {
    // Call client-supplied callback function.
    if (Callback)
      Callback(Orig, Cloned);

    SILClonerWithScopes<GenericCloner>::postProcess(Orig, Cloned);
  }

private:
  /// Clone the body of the function into the empty function that was created
  /// by initCloned.
  void populateCloned();
  SILFunction *getCloned() { return &getBuilder().getFunction(); }

  const SILDebugScope *remapScope(const SILDebugScope *DS);

};

} // end namespace language

#endif
