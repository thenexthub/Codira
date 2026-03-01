//===- Value.cpp - The Value class of Sandbox IR --------------------------===//
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

#include "vm/core/SandboxIR/Value.h"
#include "vm/core/SandboxIR/Context.h"
#include "vm/core/SandboxIR/User.h"
#include <sstream>

namespace vm::core::sandboxir {

Value::Value(ClassID SubclassID, toolchain::Value *Val, Context &Ctx)
    : SubclassID(SubclassID), Val(Val), Ctx(Ctx) {
#ifndef NDEBUG
  UID = Ctx.getNumValues();
#endif
}

Value::use_iterator Value::use_begin() {
  toolchain::Use *LLVMUse = nullptr;
  if (!Val->uses().empty())
    LLVMUse = &*Val->use_begin();
  User *User = LLVMUse != nullptr ? cast_or_null<sandboxir::User>(Ctx.getValue(
                                        Val->use_begin()->getUser()))
                                  : nullptr;
  return use_iterator(Use(LLVMUse, User, Ctx));
}

Value::user_iterator Value::user_begin() {
  auto UseBegin = Val->use_begin();
  auto UseEnd = Val->use_end();
  bool AtEnd = UseBegin == UseEnd;
  toolchain::Use *LLVMUse = AtEnd ? nullptr : &*UseBegin;
  User *User =
      AtEnd ? nullptr
            : cast_or_null<sandboxir::User>(Ctx.getValue(&*LLVMUse->getUser()));
  return user_iterator(Use(LLVMUse, User, Ctx), UseToUser());
}

unsigned Value::getNumUses() const { return range_size(Val->users()); }

Type *Value::getType() const { return Ctx.getType(Val->getType()); }

void Value::replaceUsesWithIf(
    Value *OtherV, toolchain::function_ref<bool(const Use &)> ShouldReplace) {
  assert(getType() == OtherV->getType() && "Can't replace with different type");
  toolchain::Value *OtherVal = OtherV->Val;
  // We are delegating RUWIf to LLVM IR's RUWIf.
  Val->replaceUsesWithIf(
      OtherVal, [&ShouldReplace, this, OtherV](toolchain::Use &LLVMUse) -> bool {
        User *DstU = cast_or_null<User>(Ctx.getValue(LLVMUse.getUser()));
        if (DstU == nullptr)
          return false;
        Use UseToReplace(&LLVMUse, DstU, Ctx);
        if (!ShouldReplace(UseToReplace))
          return false;
        Ctx.getTracker().emplaceIfTracking<UseSet>(UseToReplace);
        Ctx.runSetUseCallbacks(UseToReplace, OtherV);
        return true;
      });
}

void Value::replaceAllUsesWith(Value *Other) {
  assert(getType() == Other->getType() &&
         "Replacing with Value of different type!");
  auto &Tracker = Ctx.getTracker();
  for (auto Use : uses()) {
    Ctx.runSetUseCallbacks(Use, Other);
    if (Tracker.isTracking())
      Tracker.track(std::make_unique<UseSet>(Use));
  }
  // We are delegating RAUW to LLVM IR's RAUW.
  Val->replaceAllUsesWith(Other->Val);
}

#ifndef NDEBUG
std::string Value::getUid() const {
  std::stringstream SS;
  SS << "SB" << UID << ".";
  return SS.str();
}

void Value::dumpCommonHeader(raw_ostream &OS) const {
  OS << getUid() << " " << getSubclassIDStr(SubclassID) << " ";
}

void Value::dumpCommonFooter(raw_ostream &OS) const {
  OS.indent(2) << "Val: ";
  if (Val)
    OS << *Val;
  else
    OS << "NULL";
  OS << "\n";
}

void Value::dumpCommonPrefix(raw_ostream &OS) const {
  if (Val)
    OS << *Val;
  else
    OS << "NULL ";
}

void Value::dumpCommonSuffix(raw_ostream &OS) const {
  OS << " ; " << getUid() << " (" << getSubclassIDStr(SubclassID) << ")";
}

void Value::printAsOperandCommon(raw_ostream &OS) const {
  if (Val)
    Val->printAsOperand(OS);
  else
    OS << "NULL ";
}

void Value::dump() const {
  dumpOS(dbgs());
  dbgs() << "\n";
}
#endif // NDEBUG

} // namespace vm::core::sandboxir
