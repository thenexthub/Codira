//===- ConvergenceVerifier.cpp - Verify convergence control -----*- C++ -*-===//
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

#include "vm/core/IR/ConvergenceVerifier.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/GenericConvergenceVerifierImpl.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/SSAContext.h"

using namespace vm::core;

template <>
auto GenericConvergenceVerifier<SSAContext>::getConvOp(const Instruction &I)
    -> ConvOpKind {
  const auto *CB = dyn_cast<CallBase>(&I);
  if (!CB)
    return CONV_NONE;
  switch (CB->getIntrinsicID()) {
  default:
    return CONV_NONE;
  case Intrinsic::experimental_convergence_anchor:
    return CONV_ANCHOR;
  case Intrinsic::experimental_convergence_entry:
    return CONV_ENTRY;
  case Intrinsic::experimental_convergence_loop:
    return CONV_LOOP;
  }
}

template <>
void GenericConvergenceVerifier<SSAContext>::checkConvergenceTokenProduced(
    const Instruction &I) {}

template <>
const Instruction *
GenericConvergenceVerifier<SSAContext>::findAndCheckConvergenceTokenUsed(
    const Instruction &I) {
  auto *CB = dyn_cast<CallBase>(&I);
  if (!CB)
    return nullptr;

  unsigned Count =
      CB->countOperandBundlesOfType(LLVMContext::OB_convergencectrl);
  CheckOrNull(Count <= 1,
              "The 'convergencectrl' bundle can occur at most once on a call",
              {Context.print(CB)});
  if (!Count)
    return nullptr;

  auto Bundle = CB->getOperandBundle(LLVMContext::OB_convergencectrl);
  CheckOrNull(Bundle->Inputs.size() == 1 &&
                  Bundle->Inputs[0]->getType()->isTokenTy(),
              "The 'convergencectrl' bundle requires exactly one token use.",
              {Context.print(CB)});
  auto *Token = Bundle->Inputs[0].get();
  auto *Def = dyn_cast<Instruction>(Token);

  CheckOrNull(Def && getConvOp(*Def) != CONV_NONE,
              "Convergence control tokens can only be produced by calls to the "
              "convergence control intrinsics.",
              {Context.print(Token), Context.print(&I)});

  if (Def)
    Tokens[&I] = Def;

  return Def;
}

template <>
bool GenericConvergenceVerifier<SSAContext>::isInsideConvergentFunction(
    const Instruction &I) {
  auto *F = I.getFunction();
  return F->isConvergent();
}

template <>
bool GenericConvergenceVerifier<SSAContext>::isConvergent(
    const Instruction &I) {
  if (auto *CB = dyn_cast<CallBase>(&I)) {
    return CB->isConvergent();
  }
  return false;
}

template class toolchain::GenericConvergenceVerifier<SSAContext>;
