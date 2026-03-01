//===-- CodeGen/AsmPrinter/WinCFGuard.cpp - Control Flow Guard Impl ------===//
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
// This file contains support for writing the metadata for Windows Control Flow
// Guard, including address-taken functions and valid longjmp targets.
//
//===----------------------------------------------------------------------===//

#include "WinCFGuard.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineModuleInfo.h"
#include "vm/core/IR/InstrTypes.h"
#include "vm/core/IR/Module.h"
#include "vm/core/MC/MCObjectFileInfo.h"
#include "vm/core/MC/MCStreamer.h"

#include <vector>

using namespace vm::core;

WinCFGuard::WinCFGuard(AsmPrinter *A) : Asm(A) {}

WinCFGuard::~WinCFGuard() = default;

void WinCFGuard::endFunction(const MachineFunction *MF) {

  // Skip functions without any longjmp targets.
  if (MF->getLongjmpTargets().empty())
    return;

  // Copy the function's longjmp targets to a module-level list.
  toolchain::append_range(LongjmpTargets, MF->getLongjmpTargets());
}

/// Returns true if this function's address is escaped in a way that might make
/// it an indirect call target. Function::hasAddressTaken gives different
/// results when a function is called directly with a function prototype
/// mismatch, which requires a cast.
static bool isPossibleIndirectCallTarget(const Function *F) {
  SmallVector<const Value *, 4> Users{F};
  while (!Users.empty()) {
    const Value *FnOrCast = Users.pop_back_val();
    for (const Use &U : FnOrCast->uses()) {
      const User *FnUser = U.getUser();
      if (const auto *Call = dyn_cast<CallBase>(FnUser)) {
        if ((!Call->isCallee(&U) || U.get() != F) &&
            !Call->getFunction()->getName().ends_with("$exit_thunk")) {
          // Passing a function pointer to a call may lead to an indirect
          // call. As an exception, ignore ARM64EC exit thunks.
          return true;
        }
      } else if (isa<Instruction>(FnUser)) {
        // Consider any other instruction to be an escape. This has some weird
        // consequences like no-op intrinsics being an escape or a store *to* a
        // function address being an escape.
        return true;
      } else if (const auto *G = dyn_cast<GlobalValue>(FnUser)) {
        // Ignore toolchain.arm64ec.symbolmap; it doesn't lower to an actual address.
        if (G->getName() == "toolchain.arm64ec.symbolmap")
          continue;
        // Globals (for example, vtables) are escapes.
        return true;
      } else if (isa<Constant>(FnUser)) {
        // Constants which aren't a global are intermediate values; recursively
        // analyze the users to see if they actually escape.
        Users.push_back(FnUser);
      }
    }
  }
  return false;
}

MCSymbol *WinCFGuard::lookupImpSymbol(const MCSymbol *Sym) {
  if (Sym->getName().starts_with("__imp_"))
    return nullptr;
  return Asm->OutContext.lookupSymbol(Twine("__imp_") + Sym->getName());
}

void WinCFGuard::endModule() {
  const Module *M = Asm->MMI->getModule();
  std::vector<const MCSymbol *> GFIDsEntries;
  std::vector<const MCSymbol *> GIATsEntries;
  for (const Function &F : *M) {
    if (isPossibleIndirectCallTarget(&F)) {
      // If F is a dllimport and has an "__imp_" symbol already defined, add the
      // "__imp_" symbol to the .giats section.
      if (F.hasDLLImportStorageClass()) {
        if (MCSymbol *impSym = lookupImpSymbol(Asm->getSymbol(&F))) {
          GIATsEntries.push_back(impSym);
        }
      }
      // Add the function's symbol to the .gfids section.
      // Note: For dllimport functions, MSVC sometimes does not add this symbol
      // to the .gfids section, but only adds the corresponding "__imp_" symbol
      // to the .giats section. Here we always add the symbol to the .gfids
      // section, since this does not introduce security risks.
      GFIDsEntries.push_back(Asm->getSymbol(&F));
    }
  }

  if (GFIDsEntries.empty() && GIATsEntries.empty() && LongjmpTargets.empty())
    return;

  // Emit the symbol index of each GFIDs entry to form the .gfids section.
  auto &OS = *Asm->OutStreamer;
  OS.switchSection(Asm->OutContext.getObjectFileInfo()->getGFIDsSection());
  for (const MCSymbol *S : GFIDsEntries)
    OS.emitCOFFSymbolIndex(S);

  // Emit the symbol index of each GIATs entry to form the .giats section.
  OS.switchSection(Asm->OutContext.getObjectFileInfo()->getGIATsSection());
  for (const MCSymbol *S : GIATsEntries) {
    OS.emitCOFFSymbolIndex(S);
  }

  // Emit the symbol index of each longjmp target to form the .gljmp section.
  OS.switchSection(Asm->OutContext.getObjectFileInfo()->getGLJMPSection());
  for (const MCSymbol *S : LongjmpTargets) {
    OS.emitCOFFSymbolIndex(S);
  }
}
