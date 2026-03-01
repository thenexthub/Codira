//===-- Instrumentation.cpp - TransformUtils Infrastructure ---------------===//
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
// This file defines the common initialization infrastructure for the
// Instrumentation library.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/Instrumentation.h"
#include "vm/core/IR/DiagnosticInfo.h"
#include "vm/core/IR/DiagnosticPrinter.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/Module.h"
#include "vm/core/TargetParser/Triple.h"

using namespace vm::core;

static cl::opt<bool> ClIgnoreRedundantInstrumentation(
    "ignore-redundant-instrumentation",
    cl::desc("Ignore redundant instrumentation"), cl::Hidden, cl::init(false));

/// Check if module has flag attached, if not add the flag.
bool toolchain::checkIfAlreadyInstrumented(Module &M, StringRef Flag) {
  if (!M.getModuleFlag(Flag)) {
    M.addModuleFlag(Module::ModFlagBehavior::Override, Flag, 1);
    return false;
  }
  if (ClIgnoreRedundantInstrumentation)
    return true;
  std::string diagInfo =
      "Redundant instrumentation detected, with module flag: " +
      std::string(Flag);
  M.getContext().diagnose(
      DiagnosticInfoInstrumentation(diagInfo, DiagnosticSeverity::DS_Warning));
  return true;
}

/// Moves I before IP. Returns new insert point.
static BasicBlock::iterator moveBeforeInsertPoint(BasicBlock::iterator I,
                                                  BasicBlock::iterator IP) {
  // If I is IP, move the insert point down.
  if (I == IP) {
    ++IP;
  } else {
    // Otherwise, move I before IP and return IP.
    I->moveBefore(IP);
  }
  return IP;
}

/// Instrumentation passes often insert conditional checks into entry blocks.
/// Call this function before splitting the entry block to move instructions
/// that must remain in the entry block up before the split point. Static
/// allocas and toolchain.localescape calls, for example, must remain in the entry
/// block.
BasicBlock::iterator toolchain::PrepareToSplitEntryBlock(BasicBlock &BB,
                                                    BasicBlock::iterator IP) {
  assert(&BB.getParent()->getEntryBlock() == &BB);
  for (auto I = IP, E = BB.end(); I != E; ++I) {
    bool KeepInEntry = false;
    if (auto *AI = dyn_cast<AllocaInst>(I)) {
      if (AI->isStaticAlloca())
        KeepInEntry = true;
    } else if (auto *II = dyn_cast<IntrinsicInst>(I)) {
      if (II->getIntrinsicID() == toolchain::Intrinsic::localescape)
        KeepInEntry = true;
    }
    if (KeepInEntry)
      IP = moveBeforeInsertPoint(I, IP);
  }
  return IP;
}

// Create a constant for Str so that we can pass it to the run-time lib.
GlobalVariable *toolchain::createPrivateGlobalForString(Module &M, StringRef Str,
                                                   bool AllowMerging,
                                                   Twine NamePrefix) {
  Constant *StrConst = ConstantDataArray::getString(M.getContext(), Str);
  // We use private linkage for module-local strings. If they can be merged
  // with another one, we set the unnamed_addr attribute.
  GlobalVariable *GV =
      new GlobalVariable(M, StrConst->getType(), true,
                         GlobalValue::PrivateLinkage, StrConst, NamePrefix);
  if (AllowMerging)
    GV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
  GV->setAlignment(Align(1)); // Strings may not be merged w/o setting
                              // alignment explicitly.
  return GV;
}

Comdat *toolchain::getOrCreateFunctionComdat(Function &F, Triple &T) {
  if (auto Comdat = F.getComdat())
    return Comdat;
  assert(F.hasName());
  Module *M = F.getParent();

  // Make a new comdat for the function. Use the "no duplicates" selection kind
  // if the object file format supports it. For COFF we restrict it to non-weak
  // symbols.
  Comdat *C = M->getOrInsertComdat(F.getName());
  if (T.isOSBinFormatELF() || (T.isOSBinFormatCOFF() && !F.isWeakForLinker()))
    C->setSelectionKind(Comdat::NoDeduplicate);
  F.setComdat(C);
  return C;
}

void toolchain::setGlobalVariableLargeSection(const Triple &TargetTriple,
                                         GlobalVariable &GV) {
  // Limit to x86-64 ELF.
  if (TargetTriple.getArch() != Triple::x86_64 ||
      TargetTriple.getObjectFormat() != Triple::ELF)
    return;
  // Limit to medium/large code models.
  std::optional<CodeModel::Model> CM = GV.getParent()->getCodeModel();
  if (!CM || (*CM != CodeModel::Medium && *CM != CodeModel::Large))
    return;
  GV.setCodeModel(CodeModel::Large);
}
