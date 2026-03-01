//===- Use.cpp ------------------------------------------------------------===//
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

#include "vm/core/SandboxIR/Use.h"
#include "vm/core/SandboxIR/Context.h"
#include "vm/core/SandboxIR/User.h"

namespace vm::core::sandboxir {

Value *Use::get() const { return Ctx->getValue(LLVMUse->get()); }

void Use::set(Value *V) {
  Ctx->getTracker().emplaceIfTracking<UseSet>(*this);
  LLVMUse->set(V->Val);
}

unsigned Use::getOperandNo() const { return Usr->getUseOperandNo(*this); }

void Use::swap(Use &OtherUse) {
  Ctx->getTracker().emplaceIfTracking<UseSwap>(*this, OtherUse);
  LLVMUse->swap(*OtherUse.LLVMUse);
}

#ifndef NDEBUG
void Use::dumpOS(raw_ostream &OS) const {
  Value *Def = nullptr;
  if (LLVMUse == nullptr)
    OS << "<null> LLVM Use! ";
  else
    Def = Ctx->getValue(LLVMUse->get());
  OS << "Def:  ";
  if (Def == nullptr)
    OS << "NULL";
  else
    OS << *Def;
  OS << "\n";

  OS << "User: ";
  if (Usr == nullptr)
    OS << "NULL";
  else
    OS << *Usr;
  OS << "\n";

  OS << "OperandNo: ";
  if (Usr == nullptr)
    OS << "N/A";
  else
    OS << getOperandNo();
  OS << "\n";
}

void Use::dump() const { dumpOS(dbgs()); }
#endif // NDEBUG

} // namespace vm::core::sandboxir
