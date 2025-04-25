//===--- GenInit.cpp - IR Generation for Initialization -------------------===//
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
//  This file implements IR generation for the initialization of
//  local and global variables.
//
//
//===----------------------------------------------------------------------===//

#include "language/AST/Pattern.h"
#include "language/Basic/Assertions.h"
#include "language/SIL/SILDeclRef.h"
#include "language/SIL/SILGlobalVariable.h"
#include "language/IRGen/Linking.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Module.h"

#include "DebugTypeInfo.h"
#include "GenTuple.h"
#include "IRGenDebugInfo.h"
#include "IRGenFunction.h"
#include "IRGenModule.h"
#include "FixedTypeInfo.h"
#include "Temporary.h"

using namespace language;
using namespace irgen;

/// Emit a global variable.
void IRGenModule::emitSILGlobalVariable(SILGlobalVariable *var) {
  auto &ti = getTypeInfo(var->getLoweredType());
  auto expansion = getResilienceExpansionForLayout(var);

  // If the variable is empty in all resilience domains that can access this
  // variable directly, don't actually emit it; just return undef.
  if (ti.isKnownEmpty(expansion)) {
    if (DebugInfo && var->getDecl()) {
      auto DbgTy = DebugTypeInfo::getGlobal(var, *this);
      DebugInfo->emitGlobalVariableDeclaration(nullptr, var->getDecl()->getName().str(),
                                               "", DbgTy,
                                               var->getLinkage() != SILLinkage::Public &&
                                               var->getLinkage() != SILLinkage::Package,
                                               SILLocation(var->getDecl()));
    }
    return;
  }

  /// Create the global variable.
  getAddrOfSILGlobalVariable(var, ti,
                     var->isDefinition() ? ForDefinition : NotForDefinition);
}

StackAddress FixedTypeInfo::allocateStack(IRGenFunction &IGF, SILType T,
                                          const Twine &name) const {
  // If the type is known to be empty, don't actually allocate anything.
  if (isKnownEmpty(ResilienceExpansion::Maximal)) {
    auto addr = getUndefAddress();
    return { addr };
  }

  Address alloca =
    IGF.createAlloca(getStorageType(), getFixedAlignment(), name);
  IGF.Builder.CreateLifetimeStart(alloca, getFixedSize());
  
  return { alloca };
}

void FixedTypeInfo::destroyStack(IRGenFunction &IGF, StackAddress addr,
                                 SILType T, bool isOutlined) const {
  destroy(IGF, addr.getAddress(), T, isOutlined);
  FixedTypeInfo::deallocateStack(IGF, addr, T);
}

void FixedTypeInfo::deallocateStack(IRGenFunction &IGF, StackAddress addr,
                                    SILType T) const {
  if (isKnownEmpty(ResilienceExpansion::Maximal))
    return;
  IGF.Builder.CreateLifetimeEnd(addr.getAddress(), getFixedSize());
}

void TemporarySet::destroyAll(IRGenFunction &IGF) const {
  assert(!hasBeenCleared() && "destroying a set that's been cleared?");

  // Deallocate all the temporaries.
  for (auto &temporary : llvm::reverse(Stack)) {
    temporary.destroy(IGF);
  }
}

void Temporary::destroy(IRGenFunction &IGF) const {
  auto &ti = IGF.getTypeInfo(Type);
  if (Type.isSensitive()) {
    llvm::Value *size = ti.getSize(IGF, Type);
    IGF.emitClearSensitive(Addr.getAddress(), size);
  }
  ti.deallocateStack(IGF, Addr, Type);
}
