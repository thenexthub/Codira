//===- Function.cpp - The Function class of Sandbox IR --------------------===//
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

#include "vm/core/SandboxIR/Function.h"
#include "vm/core/IR/Value.h"
#include "vm/core/SandboxIR/Context.h"

namespace vm::core::sandboxir {

FunctionType *Function::getFunctionType() const {
  return cast<FunctionType>(
      Ctx.getType(cast<toolchain::Function>(Val)->getFunctionType()));
}

void Function::setAlignment(MaybeAlign Align) {
  Ctx.getTracker()
      .emplaceIfTracking<
          GenericSetter<&Function::getAlign, &Function::setAlignment>>(this);
  cast<toolchain::Function>(Val)->setAlignment(Align);
}

#ifndef NDEBUG
void Function::dumpNameAndArgs(raw_ostream &OS) const {
  auto *F = cast<toolchain::Function>(Val);
  OS << *F->getReturnType() << " @" << F->getName() << "(";
  interleave(
      F->args(),
      [this, &OS](const toolchain::Argument &LLVMArg) {
        auto *SBArg = cast_or_null<Argument>(Ctx.getValue(&LLVMArg));
        if (SBArg == nullptr)
          OS << "NULL";
        else
          SBArg->printAsOperand(OS);
      },
      [&] { OS << ", "; });
  OS << ")";
}

void Function::dumpOS(raw_ostream &OS) const {
  dumpNameAndArgs(OS);
  OS << " {\n";
  auto *LLVMF = cast<toolchain::Function>(Val);
  interleave(
      *LLVMF,
      [this, &OS](const toolchain::BasicBlock &LLVMBB) {
        auto *BB = cast_or_null<BasicBlock>(Ctx.getValue(&LLVMBB));
        if (BB == nullptr)
          OS << "NULL";
        else
          OS << *BB;
      },
      [&OS] { OS << "\n"; });
  OS << "}\n";
}
#endif // NDEBUG

} // namespace vm::core::sandboxir
