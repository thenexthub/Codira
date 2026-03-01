//===- SSAContext.cpp -------------------------------------------*- C++ -*-===//
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
///
/// This file defines a specialization of the GenericSSAContext<X>
/// template class for LLVM IR.
///
//===----------------------------------------------------------------------===//

#include "vm/core/IR/SSAContext.h"
#include "vm/core/IR/BasicBlock.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Intrinsics.h"
#include "vm/core/IR/ModuleSlotTracker.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

template <>
void SSAContext::appendBlockDefs(SmallVectorImpl<Value *> &defs,
                                 BasicBlock &block) {
  for (auto &instr : block) {
    if (instr.isTerminator())
      break;
    defs.push_back(&instr);
  }
}

template <>
void SSAContext::appendBlockDefs(SmallVectorImpl<const Value *> &defs,
                                 const BasicBlock &block) {
  for (auto &instr : block) {
    if (instr.isTerminator())
      break;
    defs.push_back(&instr);
  }
}

template <>
void SSAContext::appendBlockTerms(SmallVectorImpl<Instruction *> &terms,
                                  BasicBlock &block) {
  terms.push_back(block.getTerminator());
}

template <>
void SSAContext::appendBlockTerms(SmallVectorImpl<const Instruction *> &terms,
                                  const BasicBlock &block) {
  terms.push_back(block.getTerminator());
}

template <>
const BasicBlock *SSAContext::getDefBlock(const Value *value) const {
  if (const auto *instruction = dyn_cast<Instruction>(value))
    return instruction->getParent();
  return nullptr;
}

template <>
bool SSAContext::isConstantOrUndefValuePhi(const Instruction &Instr) {
  if (auto *Phi = dyn_cast<PHINode>(&Instr))
    return Phi->hasConstantOrUndefValue();
  return false;
}

template <> Intrinsic::ID SSAContext::getIntrinsicID(const Instruction &I) {
  if (auto *CB = dyn_cast<CallBase>(&I))
    return CB->getIntrinsicID();
  return Intrinsic::not_intrinsic;
}

template <> Printable SSAContext::print(const Value *V) const {
  return Printable([V](raw_ostream &Out) { V->print(Out); });
}

template <> Printable SSAContext::print(const Instruction *Inst) const {
  return print(cast<Value>(Inst));
}

template <> Printable SSAContext::print(const BasicBlock *BB) const {
  if (!BB)
    return Printable([](raw_ostream &Out) { Out << "<nullptr>"; });
  if (BB->hasName())
    return Printable([BB](raw_ostream &Out) { Out << BB->getName(); });

  return Printable([BB](raw_ostream &Out) {
    ModuleSlotTracker MST{BB->getParent()->getParent(), false};
    MST.incorporateFunction(*BB->getParent());
    Out << MST.getLocalSlot(BB);
  });
}

template <> Printable SSAContext::printAsOperand(const BasicBlock *BB) const {
  return Printable([BB](raw_ostream &Out) { BB->printAsOperand(Out); });
}
