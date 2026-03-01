//===- CtorUtils.cpp - Helpers for working with global_ctors ----*- C++ -*-===//
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
//
// This file defines functions that are used to process toolchain.global_ctors.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/CtorUtils.h"
#include "vm/core/ADT/BitVector.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/GlobalVariable.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
#include <numeric>

#define DEBUG_TYPE "ctor_utils"

using namespace vm::core;

/// Given a specified toolchain.global_ctors list, remove the listed elements.
static void removeGlobalCtors(GlobalVariable *GCL, const BitVector &CtorsToRemove) {
  // Filter out the initializer elements to remove.
  ConstantArray *OldCA = cast<ConstantArray>(GCL->getInitializer());
  SmallVector<Constant *, 10> CAList;
  for (unsigned I = 0, E = OldCA->getNumOperands(); I < E; ++I)
    if (!CtorsToRemove.test(I))
      CAList.push_back(OldCA->getOperand(I));

  // Create the new array initializer.
  ArrayType *ATy =
      ArrayType::get(OldCA->getType()->getElementType(), CAList.size());
  Constant *CA = ConstantArray::get(ATy, CAList);

  // If we didn't change the number of elements, don't create a new GV.
  if (CA->getType() == OldCA->getType()) {
    GCL->setInitializer(CA);
    return;
  }

  // Create the new global and insert it next to the existing list.
  GlobalVariable *NGV = new GlobalVariable(
      CA->getType(), GCL->isConstant(), GCL->getLinkage(), CA, "",
      GCL->getThreadLocalMode(), GCL->getAddressSpace());
  GCL->getParent()->insertGlobalVariable(GCL->getIterator(), NGV);
  NGV->takeName(GCL);

  // Nuke the old list, replacing any uses with the new one.
  if (!GCL->use_empty())
    GCL->replaceAllUsesWith(NGV);

  GCL->eraseFromParent();
}

/// Given a toolchain.global_ctors list that we can understand,
/// return a list of the functions and null terminator as a vector.
static std::vector<std::pair<uint32_t, Function *>>
parseGlobalCtors(GlobalVariable *GV) {
  ConstantArray *CA = cast<ConstantArray>(GV->getInitializer());
  std::vector<std::pair<uint32_t, Function *>> Result;
  Result.reserve(CA->getNumOperands());
  for (auto &V : CA->operands()) {
    ConstantStruct *CS = cast<ConstantStruct>(V);
    Result.emplace_back(cast<ConstantInt>(CS->getOperand(0))->getZExtValue(),
                        dyn_cast<Function>(CS->getOperand(1)));
  }
  return Result;
}

/// Find the toolchain.global_ctors list.
static GlobalVariable *findGlobalCtors(Module &M) {
  GlobalVariable *GV = M.getGlobalVariable("toolchain.global_ctors");
  if (!GV)
    return nullptr;

  // Verify that the initializer is simple enough for us to handle. We are
  // only allowed to optimize the initializer if it is unique.
  if (!GV->hasUniqueInitializer())
    return nullptr;

  // If there are no ctors, then the initializer might be null/undef/poison.
  // Ignore anything but an array.
  ConstantArray *CA = dyn_cast<ConstantArray>(GV->getInitializer());
  if (!CA)
    return nullptr;

  for (auto &V : CA->operands()) {
    if (isa<ConstantAggregateZero>(V))
      continue;
    ConstantStruct *CS = cast<ConstantStruct>(V);
    if (isa<ConstantPointerNull>(CS->getOperand(1)))
      continue;

    // Can only handle global constructors with no arguments.
    Function *F = dyn_cast<Function>(CS->getOperand(1));
    if (!F || F->arg_size() != 0)
      return nullptr;
  }
  return GV;
}

/// Call "ShouldRemove" for every entry in M's global_ctor list and remove the
/// entries for which it returns true.  Return true if anything changed.
bool toolchain::optimizeGlobalCtorsList(
    Module &M, function_ref<bool(uint32_t, Function *)> ShouldRemove) {
  GlobalVariable *GlobalCtors = findGlobalCtors(M);
  if (!GlobalCtors)
    return false;

  std::vector<std::pair<uint32_t, Function *>> Ctors =
      parseGlobalCtors(GlobalCtors);
  if (Ctors.empty())
    return false;

  bool MadeChange = false;
  // Loop over global ctors, optimizing them when we can.
  BitVector CtorsToRemove(Ctors.size());
  std::vector<size_t> CtorsByPriority(Ctors.size());
  std::iota(CtorsByPriority.begin(), CtorsByPriority.end(), 0);
  stable_sort(CtorsByPriority, [&](size_t LHS, size_t RHS) {
    return Ctors[LHS].first < Ctors[RHS].first;
  });
  for (unsigned CtorIndex : CtorsByPriority) {
    const uint32_t Priority = Ctors[CtorIndex].first;
    Function *F = Ctors[CtorIndex].second;
    if (!F)
      continue;

    LLVM_DEBUG(dbgs() << "Optimizing Global Constructor: " << *F << "\n");

    // If we can evaluate the ctor at compile time, do.
    if (ShouldRemove(Priority, F)) {
      Ctors[CtorIndex].second = nullptr;
      CtorsToRemove.set(CtorIndex);
      MadeChange = true;
      continue;
    }
  }

  if (!MadeChange)
    return false;

  removeGlobalCtors(GlobalCtors, CtorsToRemove);
  return true;
}
