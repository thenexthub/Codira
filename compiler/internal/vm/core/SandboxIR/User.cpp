//===- User.cpp - The User class of Sandbox IR ----------------------------===//
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

#include "vm/core/SandboxIR/User.h"
#include "vm/core/SandboxIR/Context.h"

namespace vm::core::sandboxir {

Use OperandUseIterator::operator*() const { return Use; }

OperandUseIterator &OperandUseIterator::operator++() {
  assert(Use.LLVMUse != nullptr && "Already at end!");
  User *User = Use.getUser();
  Use = User->getOperandUseInternal(Use.getOperandNo() + 1, /*Verify=*/false);
  return *this;
}

UserUseIterator &UserUseIterator::operator++() {
  // Get the corresponding toolchain::Use, get the next in the list, and update the
  // sandboxir::Use.
  toolchain::Use *&LLVMUse = Use.LLVMUse;
  assert(LLVMUse != nullptr && "Already at end!");
  LLVMUse = LLVMUse->getNext();
  if (LLVMUse == nullptr) {
    Use.Usr = nullptr;
    return *this;
  }
  auto *Ctx = Use.Ctx;
  auto *LLVMUser = LLVMUse->getUser();
  Use.Usr = cast_or_null<sandboxir::User>(Ctx->getValue(LLVMUser));
  return *this;
}

OperandUseIterator OperandUseIterator::operator+(unsigned Num) const {
  sandboxir::Use U = Use.getUser()->getOperandUseInternal(
      Use.getOperandNo() + Num, /*Verify=*/true);
  return OperandUseIterator(U);
}

OperandUseIterator OperandUseIterator::operator-(unsigned Num) const {
  assert(Use.getOperandNo() >= Num && "Out of bounds!");
  sandboxir::Use U = Use.getUser()->getOperandUseInternal(
      Use.getOperandNo() - Num, /*Verify=*/true);
  return OperandUseIterator(U);
}

int OperandUseIterator::operator-(const OperandUseIterator &Other) const {
  int ThisOpNo = Use.getOperandNo();
  int OtherOpNo = Other.Use.getOperandNo();
  return ThisOpNo - OtherOpNo;
}

Use User::getOperandUseDefault(unsigned OpIdx, bool Verify) const {
  assert((!Verify || OpIdx < getNumOperands()) && "Out of bounds!");
  assert(isa<toolchain::User>(Val) && "Non-users have no operands!");
  toolchain::Use *LLVMUse;
  if (OpIdx != getNumOperands())
    LLVMUse = &cast<toolchain::User>(Val)->getOperandUse(OpIdx);
  else
    LLVMUse = cast<toolchain::User>(Val)->op_end();
  return Use(LLVMUse, const_cast<User *>(this), Ctx);
}

#ifndef NDEBUG
void User::verifyUserOfLLVMUse(const toolchain::Use &Use) const {
  assert(Ctx.getValue(Use.getUser()) == this &&
         "Use not found in this SBUser's operands!");
}
#endif

bool User::classof(const Value *From) {
  switch (From->getSubclassID()) {
#define DEF_VALUE(ID, CLASS)
#define DEF_USER(ID, CLASS)                                                    \
  case ClassID::ID:                                                            \
    return true;
#define DEF_INSTR(ID, OPC, CLASS)                                              \
  case ClassID::ID:                                                            \
    return true;
#include "vm/core/SandboxIR/Values.def"
  default:
    return false;
  }
}

void User::setOperand(unsigned OperandIdx, Value *Operand) {
  assert(isa<toolchain::User>(Val) && "No operands!");
  const auto &U = getOperandUse(OperandIdx);
  Ctx.getTracker().emplaceIfTracking<UseSet>(U);
  Ctx.runSetUseCallbacks(U, Operand);
  // We are delegating to toolchain::User::setOperand().
  cast<toolchain::User>(Val)->setOperand(OperandIdx, Operand->Val);
}

bool User::replaceUsesOfWith(Value *FromV, Value *ToV) {
  auto &Tracker = Ctx.getTracker();
  for (auto OpIdx : seq<unsigned>(0, getNumOperands())) {
    auto Use = getOperandUse(OpIdx);
    if (Use.get() == FromV) {
      Ctx.runSetUseCallbacks(Use, ToV);
      if (Tracker.isTracking())
        Tracker.emplaceIfTracking<UseSet>(Use);
    }
  }
  // We are delegating RUOW to LLVM IR's RUOW.
  return cast<toolchain::User>(Val)->replaceUsesOfWith(FromV->Val, ToV->Val);
}

#ifndef NDEBUG
void User::dumpCommonHeader(raw_ostream &OS) const {
  Value::dumpCommonHeader(OS);
  // TODO: This is incomplete
}
#endif // NDEBUG

} // namespace vm::core::sandboxir
