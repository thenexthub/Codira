//===- Module.cpp ---------------------------------------------------------===//
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

#include "vm/core/SandboxIR/Module.h"
#include "vm/core/SandboxIR/Constant.h"
#include "vm/core/SandboxIR/Context.h"
#include "vm/core/SandboxIR/Function.h"
#include "vm/core/SandboxIR/Value.h"

using namespace vm::core::sandboxir;

Function *Module::getFunction(StringRef Name) const {
  toolchain::Function *LLVMF = LLVMM.getFunction(Name);
  return cast_or_null<Function>(Ctx.getValue(LLVMF));
}

GlobalVariable *Module::getGlobalVariable(StringRef Name,
                                          bool AllowInternal) const {
  return cast_or_null<GlobalVariable>(
      Ctx.getValue(LLVMM.getGlobalVariable(Name, AllowInternal)));
}

GlobalAlias *Module::getNamedAlias(StringRef Name) const {
  return cast_or_null<GlobalAlias>(Ctx.getValue(LLVMM.getNamedAlias(Name)));
}

GlobalIFunc *Module::getNamedIFunc(StringRef Name) const {
  return cast_or_null<GlobalIFunc>(Ctx.getValue(LLVMM.getNamedIFunc(Name)));
}

#ifndef NDEBUG
void Module::dumpOS(raw_ostream &OS) const { OS << LLVMM; }

void Module::dump() const {
  dumpOS(dbgs());
  dbgs() << "\n";
}
#endif // NDEBUG
