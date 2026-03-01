//===- StructuralHash.cpp - Function Hash Printing ------------------------===//
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
// This file defines the StructuralHashPrinterPass which is used to show
// the structural hash of all functions in a module and the module itself.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/StructuralHash.h"
#include "vm/core/IR/Module.h"
#include "vm/core/IR/StructuralHash.h"
#include "vm/core/Support/Format.h"

using namespace vm::core;

PreservedAnalyses StructuralHashPrinterPass::run(Module &M,
                                                 ModuleAnalysisManager &MAM) {
  OS << "Module Hash: "
     << format("%016" PRIx64,
               StructuralHash(M, Options != StructuralHashOptions::None))
     << "\n";
  for (Function &F : M) {
    if (F.isDeclaration())
      continue;
    if (Options == StructuralHashOptions::CallTargetIgnored) {
      auto IgnoreOp = [&](const Instruction *I, unsigned OpndIdx) {
        return I->getOpcode() == Instruction::Call &&
               isa<Constant>(I->getOperand(OpndIdx));
      };
      auto FuncHashInfo = StructuralHashWithDifferences(F, IgnoreOp);
      OS << "Function " << F.getName()
         << " Hash: " << format("%016" PRIx64, FuncHashInfo.FunctionHash)
         << "\n";
      for (auto &[IndexPair, OpndHash] : *FuncHashInfo.IndexOperandHashMap) {
        auto [InstIndex, OpndIndex] = IndexPair;
        OS << "\tIgnored Operand Hash: " << format("%016" PRIx64, OpndHash)
           << " at (" << InstIndex << "," << OpndIndex << ")\n";
      }
    } else {
      OS << "Function " << F.getName() << " Hash: "
         << format(
                "%016" PRIx64,
                StructuralHash(F, Options == StructuralHashOptions::Detailed))
         << "\n";
    }
  }
  return PreservedAnalyses::all();
}
