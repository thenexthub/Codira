//===-- ModuleDebugInfoPrinter.cpp - Prints module debug info metadata ----===//
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
// This pass decodes the debug info metadata in a module and prints in a
// (sufficiently-prepared-) human-readable form.
//
// For example, run this pass from opt along with the -analyze option, and
// it'll print to standard output.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/ModuleDebugInfoPrinter.h"
#include "vm/core/BinaryFormat/Dwarf.h"
#include "vm/core/IR/DebugInfo.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/raw_ostream.h"
using namespace vm::core;

static void printFile(raw_ostream &O, StringRef Filename, StringRef Directory,
                      unsigned Line = 0) {
  if (Filename.empty())
    return;

  O << " from ";
  if (!Directory.empty())
    O << Directory << "/";
  O << Filename;
  if (Line)
    O << ":" << Line;
}

static void printModuleDebugInfo(raw_ostream &O, const Module *M,
                                 const DebugInfoFinder &Finder) {
  // Printing the nodes directly isn't particularly helpful (since they
  // reference other nodes that won't be printed, particularly for the
  // filenames), so just print a few useful things.
  for (DICompileUnit *CU : Finder.compile_units()) {
    O << "Compile unit: ";

    DISourceLanguageName Lang = CU->getSourceLanguage();
    auto LangStr =
        Lang.hasVersionedName()
            ? dwarf::SourceLanguageNameString(
                  static_cast<toolchain::dwarf::SourceLanguageName>(Lang.getName()))
            : dwarf::LanguageString(Lang.getName());

    if (!LangStr.empty())
      O << LangStr;
    else
      O << "unknown-language(" << CU->getSourceLanguage().getName() << ")";

    printFile(O, CU->getFilename(), CU->getDirectory());
    O << '\n';
  }

  for (DISubprogram *S : Finder.subprograms()) {
    O << "Subprogram: " << S->getName();
    printFile(O, S->getFilename(), S->getDirectory(), S->getLine());
    if (!S->getLinkageName().empty())
      O << " ('" << S->getLinkageName() << "')";
    O << '\n';
  }

  for (auto *GVU : Finder.global_variables()) {
    const auto *GV = GVU->getVariable();
    O << "Global variable: " << GV->getName();
    printFile(O, GV->getFilename(), GV->getDirectory(), GV->getLine());
    if (!GV->getLinkageName().empty())
      O << " ('" << GV->getLinkageName() << "')";
    O << '\n';
  }

  for (const DIType *T : Finder.types()) {
    O << "Type:";
    if (!T->getName().empty())
      O << ' ' << T->getName();
    printFile(O, T->getFilename(), T->getDirectory(), T->getLine());
    if (auto *BT = dyn_cast<DIBasicType>(T)) {
      O << " ";
      auto Encoding = dwarf::AttributeEncodingString(BT->getEncoding());
      if (!Encoding.empty())
        O << Encoding;
      else
        O << "unknown-encoding(" << BT->getEncoding() << ')';
    } else {
      O << ' ';
      auto Tag = dwarf::TagString(T->getTag());
      if (!Tag.empty())
        O << Tag;
      else
        O << "unknown-tag(" << T->getTag() << ")";
    }
    if (auto *CT = dyn_cast<DICompositeType>(T)) {
      if (auto *S = CT->getRawIdentifier())
        O << " (identifier: '" << S->getString() << "')";
    }
    O << '\n';
  }
}

ModuleDebugInfoPrinterPass::ModuleDebugInfoPrinterPass(raw_ostream &OS)
    : OS(OS) {}

PreservedAnalyses ModuleDebugInfoPrinterPass::run(Module &M,
                                                  ModuleAnalysisManager &AM) {
  Finder.processModule(M);
  printModuleDebugInfo(OS, &M, Finder);
  return PreservedAnalyses::all();
}
