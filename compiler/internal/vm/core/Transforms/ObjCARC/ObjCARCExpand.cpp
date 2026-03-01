//===- ObjCARCExpand.cpp - ObjC ARC Optimization --------------------------===//
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
/// This file defines ObjC ARC optimizations. ARC stands for Automatic
/// Reference Counting and is a system for managing reference counts for objects
/// in Objective C.
///
/// This specific file deals with early optimizations which perform certain
/// cleanup operations.
///
/// WARNING: This file knows about certain library functions. It recognizes them
/// by name, and hardwires knowledge of their semantics.
///
/// WARNING: This file knows about how certain Objective-C library functions are
/// used. Naive LLVM IR transformations which would otherwise be
/// behavior-preserving may break these assumptions.
///
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/ObjCARCAnalysisUtils.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/IR/Value.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Transforms/ObjCARC.h"

#define DEBUG_TYPE "objc-arc-expand"

using namespace vm::core;
using namespace vm::core::objcarc;

namespace {
static bool runImpl(Function &F) {
  if (!EnableARCOpts)
    return false;

  // If nothing in the Module uses ARC, don't do anything.
  if (!ModuleHasARC(*F.getParent()))
    return false;

  bool Changed = false;

  LLVM_DEBUG(dbgs() << "ObjCARCExpand: Visiting Function: " << F.getName()
                    << "\n");

  for (Instruction &Inst : instructions(&F)) {
    LLVM_DEBUG(dbgs() << "ObjCARCExpand: Visiting: " << Inst << "\n");

    switch (GetBasicARCInstKind(&Inst)) {
    case ARCInstKind::Retain:
    case ARCInstKind::RetainRV:
    case ARCInstKind::Autorelease:
    case ARCInstKind::AutoreleaseRV:
    case ARCInstKind::FusedRetainAutorelease:
    case ARCInstKind::FusedRetainAutoreleaseRV: {
      // These calls return their argument verbatim, as a low-level
      // optimization. However, this makes high-level optimizations
      // harder. Undo any uses of this optimization that the front-end
      // emitted here. We'll redo them in the contract pass.
      Changed = true;
      Value *Value = cast<CallInst>(&Inst)->getArgOperand(0);
      LLVM_DEBUG(dbgs() << "ObjCARCExpand: Old = " << Inst
                        << "\n"
                           "               New = "
                        << *Value << "\n");
      Inst.replaceAllUsesWith(Value);
      break;
    }
    default:
      break;
    }
  }

  LLVM_DEBUG(dbgs() << "ObjCARCExpand: Finished List.\n\n");

  return Changed;
}

} // namespace

PreservedAnalyses ObjCARCExpandPass::run(Function &F,
                                         FunctionAnalysisManager &AM) {
  if (!runImpl(F))
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
