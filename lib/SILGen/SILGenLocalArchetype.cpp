//===--- SILGenLocalArchetype.cpp - Local archetype transform -------------===//
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
//  This file implements the transformation which rewrites captured local
//  archetypes into primary archetypes in the enclosing function's generic
//  signature.
//
//===----------------------------------------------------------------------===//

#include "SILGen.h"
#include "language/AST/LocalArchetypeRequirementCollector.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILCloner.h"

using namespace language;
using namespace language::Lowering;

namespace {

class LocalArchetypeTransform : public SILCloner<LocalArchetypeTransform> {
  friend class SILCloner<LocalArchetypeTransform>;
  friend class SILInstructionVisitor<LocalArchetypeTransform>;

  GenericSignatureWithCapturedEnvironments sig;
  GenericEnvironment *env;

public:
  LocalArchetypeTransform(SILFunction *F,
                          GenericSignatureWithCapturedEnvironments sig)
      : SILCloner(*F), env(sig.genericSig.getGenericEnvironment()) {

    assert(!sig.capturedEnvs.empty() && "Why are we doing this?");

    // The primary archetypes of the old generic environment map to
    // primary archetypes of the new generic environment at the same
    // index and depth.
    Functor.SubsMap = env->getForwardingSubstitutionMap();

    // Captured local archetypes map to primary archetypes at higher
    // depths.
    Functor.BaseGenericSig = sig.baseGenericSig;
    Functor.CapturedEnvs = sig.capturedEnvs;
  }

  void doIt() {
    auto &F = getBuilder().getFunction();

    // Collect the old basic blocks that we're going to delete.
    llvm::SmallVector<SILBasicBlock *, 4> bbs;
    for (auto &bb : F)
      bbs.push_back(&bb);

    // Make F.mapTypeIntoContext() use the new environment.
    F.setGenericEnvironment(env);

    // Start by cloning the entry block.
    auto *origEntryBlock = F.getEntryBlock();
    auto *clonedEntryBlock = F.createBasicBlock();

    // Clone arguments.
    SmallVector<SILValue, 4> entryArgs;
    entryArgs.reserve(origEntryBlock->getArguments().size());
    for (auto &origArg : origEntryBlock->getArguments()) {

      // Remap the argument type into the new generic environment.
      SILType mappedType = remapType(origArg->getType());
      auto *NewArg = clonedEntryBlock->createFunctionArgument(
          mappedType, origArg->getDecl(), true);
      NewArg->copyFlags(cast<SILFunctionArgument>(origArg));
      entryArgs.push_back(NewArg);
    }

    // Clone the remaining body.
    getBuilder().setInsertionPoint(clonedEntryBlock);
    cloneFunctionBody(&F, clonedEntryBlock, entryArgs,
                      true /*replaceOriginalFunctionInPlace*/);

    // Insert the new entry block at the beginning.
    F.moveBlockBefore(clonedEntryBlock, F.begin());

    // Erase the old basic blocks.
    for (auto *bb : bbs) {
      bb->removeDeadBlock();
    }
  }
};

} // end anonymous namespace

void SILGenModule::recontextualizeCapturedLocalArchetypes(
    SILFunction *F, GenericSignatureWithCapturedEnvironments sig) {
  if (sig.capturedEnvs.empty())
    return;

  LocalArchetypeTransform(F, sig).doIt();
  M.reclaimUnresolvedLocalArchetypeDefinitions();
}
