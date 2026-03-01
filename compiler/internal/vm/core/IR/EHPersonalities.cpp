//===- EHPersonalities.cpp - Compute EH-related information ---------------===//
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

#include "vm/core/IR/EHPersonalities.h"
#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/IR/CFG.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Module.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/TargetParser/Triple.h"
using namespace vm::core;

/// See if the given exception handling personality function is one that we
/// understand.  If so, return a description of it; otherwise return Unknown.
EHPersonality toolchain::classifyEHPersonality(const Value *Pers) {
  const GlobalValue *F =
      Pers ? dyn_cast<GlobalValue>(Pers->stripPointerCasts()) : nullptr;
  if (!F || !F->getValueType() || !F->getValueType()->isFunctionTy())
    return EHPersonality::Unknown;

  StringRef Name = F->getName();
  if (F->getParent()->getTargetTriple().isWindowsArm64EC()) {
    // ARM64EC function symbols are mangled by prefixing them with "#".
    // Demangle them by skipping this prefix.
    Name.consume_front("#");
  }

  return StringSwitch<EHPersonality>(Name)
      .Case("__gnat_eh_personality", EHPersonality::GNU_Ada)
      .Case("__gxx_personality_v0", EHPersonality::GNU_CXX)
      .Case("__gxx_personality_seh0", EHPersonality::GNU_CXX)
      .Case("__gxx_personality_sj0", EHPersonality::GNU_CXX_SjLj)
      .Case("__gcc_personality_v0", EHPersonality::GNU_C)
      .Case("__gcc_personality_seh0", EHPersonality::GNU_C)
      .Case("__gcc_personality_sj0", EHPersonality::GNU_C_SjLj)
      .Case("__objc_personality_v0", EHPersonality::GNU_ObjC)
      .Case("_except_handler3", EHPersonality::MSVC_X86SEH)
      .Case("_except_handler4", EHPersonality::MSVC_X86SEH)
      .Case("__C_specific_handler", EHPersonality::MSVC_TableSEH)
      .Case("__CxxFrameHandler3", EHPersonality::MSVC_CXX)
      .Case("ProcessCLRException", EHPersonality::CoreCLR)
      // Rust mangles its personality function, so we can't test exact equality.
      .EndsWith("rust_eh_personality", EHPersonality::Rust)
      .Case("__gxx_wasm_personality_v0", EHPersonality::Wasm_CXX)
      .Case("__xlcxx_personality_v1", EHPersonality::XL_CXX)
      .Case("__zos_cxx_personality_v2", EHPersonality::ZOS_CXX)
      .Default(EHPersonality::Unknown);
}

StringRef toolchain::getEHPersonalityName(EHPersonality Pers) {
  switch (Pers) {
  case EHPersonality::GNU_Ada:
    return "__gnat_eh_personality";
  case EHPersonality::GNU_CXX:
    return "__gxx_personality_v0";
  case EHPersonality::GNU_CXX_SjLj:
    return "__gxx_personality_sj0";
  case EHPersonality::GNU_C:
    return "__gcc_personality_v0";
  case EHPersonality::GNU_C_SjLj:
    return "__gcc_personality_sj0";
  case EHPersonality::GNU_ObjC:
    return "__objc_personality_v0";
  case EHPersonality::MSVC_X86SEH:
    return "_except_handler3";
  case EHPersonality::MSVC_TableSEH:
    return "__C_specific_handler";
  case EHPersonality::MSVC_CXX:
    return "__CxxFrameHandler3";
  case EHPersonality::CoreCLR:
    return "ProcessCLRException";
  case EHPersonality::Rust:
    llvm_unreachable(
        "Cannot get personality name of Rust personality, since it is mangled");
  case EHPersonality::Wasm_CXX:
    return "__gxx_wasm_personality_v0";
  case EHPersonality::XL_CXX:
    return "__xlcxx_personality_v1";
  case EHPersonality::ZOS_CXX:
    return "__zos_cxx_personality_v2";
  case EHPersonality::Unknown:
    llvm_unreachable("Unknown EHPersonality!");
  }

  llvm_unreachable("Invalid EHPersonality!");
}

EHPersonality toolchain::getDefaultEHPersonality(const Triple &T) {
  if (T.isPS5())
    return EHPersonality::GNU_CXX;
  else
    return EHPersonality::GNU_C;
}

bool toolchain::canSimplifyInvokeNoUnwind(const Function *F) {
  EHPersonality Personality = classifyEHPersonality(F->getPersonalityFn());
  // We can't simplify any invokes to nounwind functions if the personality
  // function wants to catch asynch exceptions.  The nounwind attribute only
  // implies that the function does not throw synchronous exceptions.

  // Cannot simplify CXX Personality under AsynchEH
  const toolchain::Module *M = (const toolchain::Module *)F->getParent();
  bool EHa = M->getModuleFlag("eh-asynch");
  return !EHa && !isAsynchronousEHPersonality(Personality);
}

DenseMap<BasicBlock *, ColorVector> toolchain::colorEHFunclets(Function &F) {
  SmallVector<std::pair<BasicBlock *, BasicBlock *>, 16> Worklist;
  BasicBlock *EntryBlock = &F.getEntryBlock();
  DenseMap<BasicBlock *, ColorVector> BlockColors;

  // Build up the color map, which maps each block to its set of 'colors'.
  // For any block B the "colors" of B are the set of funclets F (possibly
  // including a root "funclet" representing the main function) such that
  // F will need to directly contain B or a copy of B (where the term "directly
  // contain" is used to distinguish from being "transitively contained" in
  // a nested funclet).
  //
  // Note: Despite not being a funclet in the truest sense, a catchswitch is
  // considered to belong to its own funclet for the purposes of coloring.

  DEBUG_WITH_TYPE("win-eh-prepare-coloring",
                  dbgs() << "\nColoring funclets for " << F.getName() << "\n");

  Worklist.push_back({EntryBlock, EntryBlock});

  while (!Worklist.empty()) {
    BasicBlock *Visiting;
    BasicBlock *Color;
    std::tie(Visiting, Color) = Worklist.pop_back_val();
    DEBUG_WITH_TYPE("win-eh-prepare-coloring",
                    dbgs() << "Visiting " << Visiting->getName() << ", "
                           << Color->getName() << "\n");
    BasicBlock::iterator VisitingHead = Visiting->getFirstNonPHIIt();
    if (VisitingHead->isEHPad()) {
      // Mark this funclet head as a member of itself.
      Color = Visiting;
    }
    // Note that this is a member of the given color.
    ColorVector &Colors = BlockColors[Visiting];
    if (!is_contained(Colors, Color))
      Colors.push_back(Color);
    else
      continue;

    DEBUG_WITH_TYPE("win-eh-prepare-coloring",
                    dbgs() << "  Assigned color \'" << Color->getName()
                           << "\' to block \'" << Visiting->getName()
                           << "\'.\n");

    BasicBlock *SuccColor = Color;
    Instruction *Terminator = Visiting->getTerminator();
    if (auto *CatchRet = dyn_cast<CatchReturnInst>(Terminator)) {
      Value *ParentPad = CatchRet->getCatchSwitchParentPad();
      if (isa<ConstantTokenNone>(ParentPad))
        SuccColor = EntryBlock;
      else
        SuccColor = cast<Instruction>(ParentPad)->getParent();
    }

    for (BasicBlock *Succ : successors(Visiting))
      Worklist.push_back({Succ, SuccColor});
  }
  return BlockColors;
}
