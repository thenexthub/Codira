//=== WebAssemblyLowerRefTypesIntPtrConv.cpp -
//                     Lower IntToPtr and PtrToInt on Reference Types   ---===//
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
///
/// \file
/// Lowers IntToPtr and PtrToInt instructions on reference types to
/// Trap instructions since they have been allowed to operate
/// on non-integral pointers.
///
//===----------------------------------------------------------------------===//

#include "Utils/WebAssemblyTypeUtilities.h"
#include "WebAssembly.h"
#include "WebAssemblySubtarget.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/Pass.h"
#include <set>

using namespace vm::core;

#define DEBUG_TYPE "wasm-lower-reftypes-intptr-conv"

namespace {
class WebAssemblyLowerRefTypesIntPtrConv final : public FunctionPass {
  StringRef getPassName() const override {
    return "WebAssembly Lower RefTypes Int-Ptr Conversions";
  }

  bool runOnFunction(Function &MF) override;

public:
  static char ID; // Pass identification
  WebAssemblyLowerRefTypesIntPtrConv() : FunctionPass(ID) {}
};
} // end anonymous namespace

char WebAssemblyLowerRefTypesIntPtrConv::ID = 0;
INITIALIZE_PASS(WebAssemblyLowerRefTypesIntPtrConv, DEBUG_TYPE,
                "WebAssembly Lower RefTypes Int-Ptr Conversions", false, false)

FunctionPass *toolchain::createWebAssemblyLowerRefTypesIntPtrConv() {
  return new WebAssemblyLowerRefTypesIntPtrConv();
}

bool WebAssemblyLowerRefTypesIntPtrConv::runOnFunction(Function &F) {
  LLVM_DEBUG(dbgs() << "********** Lower RefTypes IntPtr Convs **********\n"
                       "********** Function: "
                    << F.getName() << '\n');

  // This function will check for uses of ptrtoint and inttoptr on reference
  // types and replace them with a trap instruction.
  //
  // We replace the instruction by a trap instruction
  // and its uses by null in the case of inttoptr and 0 in the
  // case of ptrtoint.
  std::set<Instruction *> worklist;

  for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
    PtrToIntInst *PTI = dyn_cast<PtrToIntInst>(&*I);
    IntToPtrInst *ITP = dyn_cast<IntToPtrInst>(&*I);
    if (!(PTI && WebAssembly::isWebAssemblyReferenceType(
                     PTI->getPointerOperand()->getType())) &&
        !(ITP && WebAssembly::isWebAssemblyReferenceType(ITP->getDestTy())))
      continue;

    I->replaceAllUsesWith(PoisonValue::get(I->getType()));

    Function *TrapIntrin =
        Intrinsic::getOrInsertDeclaration(F.getParent(), Intrinsic::debugtrap);
    CallInst::Create(TrapIntrin, {}, "", I->getIterator());

    worklist.insert(&*I);
  }

  // erase each instruction replaced by trap
  for (Instruction *I : worklist)
    I->eraseFromParent();

  return !worklist.empty();
}
