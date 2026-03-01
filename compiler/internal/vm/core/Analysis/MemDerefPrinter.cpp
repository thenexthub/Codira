//===- MemDerefPrinter.cpp - Printer for isDereferenceablePointer ---------===//
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

#include "vm/core/Analysis/MemDerefPrinter.h"
#include "vm/core/Analysis/Loads.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

PreservedAnalyses MemDerefPrinterPass::run(Function &F,
                                           FunctionAnalysisManager &AM) {
  OS << "Memory Dereferencibility of pointers in function '" << F.getName()
     << "'\n";

  SmallVector<Value *, 4> Deref;
  SmallPtrSet<Value *, 4> DerefAndAligned;

  const DataLayout &DL = F.getDataLayout();
  for (auto &I : instructions(F)) {
    if (LoadInst *LI = dyn_cast<LoadInst>(&I)) {
      Value *PO = LI->getPointerOperand();
      if (isDereferenceablePointer(PO, LI->getType(), DL, LI))
        Deref.push_back(PO);
      if (isDereferenceableAndAlignedPointer(PO, LI->getType(), LI->getAlign(),
                                             DL, LI))
        DerefAndAligned.insert(PO);
    }
  }

  OS << "The following are dereferenceable:\n";
  for (Value *V : Deref) {
    OS << "  ";
    V->print(OS);
    if (DerefAndAligned.count(V))
      OS << "\t(aligned)";
    else
      OS << "\t(unaligned)";
    OS << "\n";
  }
  return PreservedAnalyses::all();
}
