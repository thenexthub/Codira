//===- BasicBlock.cpp - The BasicBlock class of Sandbox IR ----------------===//
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

#include "vm/core/SandboxIR/BasicBlock.h"
#include "vm/core/SandboxIR/Context.h"
#include "vm/core/SandboxIR/Function.h"
#include "vm/core/SandboxIR/Instruction.h"

namespace vm::core::sandboxir {

BBIterator &BBIterator::operator++() {
  auto ItE = BB->end();
  assert(It != ItE && "Already at end!");
  ++It;
  if (It == ItE)
    return *this;
  Instruction &NextI = *cast<sandboxir::Instruction>(Ctx->getValue(&*It));
  unsigned Num = NextI.getNumOfIRInstrs();
  assert(Num > 0 && "Bad getNumOfIRInstrs()");
  It = std::next(It, Num - 1);
  return *this;
}

BBIterator &BBIterator::operator--() {
  assert(It != BB->begin() && "Already at begin!");
  if (It == BB->end()) {
    --It;
    return *this;
  }
  Instruction &CurrI = **this;
  unsigned Num = CurrI.getNumOfIRInstrs();
  assert(Num > 0 && "Bad getNumOfIRInstrs()");
  assert(std::prev(It, Num - 1) != BB->begin() && "Already at begin!");
  It = std::prev(It, Num);
  return *this;
}

BasicBlock *BBIterator::getNodeParent() const {
  toolchain::BasicBlock *Parent = const_cast<BBIterator *>(this)->It.getNodeParent();
  return cast<BasicBlock>(Ctx->getValue(Parent));
}

BasicBlock::iterator::pointer
BasicBlock::iterator::getInstr(toolchain::BasicBlock::iterator It) const {
  return cast_or_null<Instruction>(Ctx->getValue(&*It));
}

Function *BasicBlock::getParent() const {
  auto *BB = cast<toolchain::BasicBlock>(Val);
  auto *F = BB->getParent();
  if (F == nullptr)
    // Detached
    return nullptr;
  return cast_or_null<Function>(Ctx.getValue(F));
}

void BasicBlock::buildBasicBlockFromLLVMIR(toolchain::BasicBlock *LLVMBB) {
  for (toolchain::Instruction &IRef : reverse(*LLVMBB)) {
    toolchain::Instruction *I = &IRef;
    Ctx.getOrCreateValue(I);
    for (auto [OpIdx, Op] : enumerate(I->operands())) {
      // Skip instruction's label operands
      if (isa<toolchain::BasicBlock>(Op))
        continue;
      Ctx.getOrCreateValue(Op);
    }
  }
#if !defined(NDEBUG)
  verify();
#endif
}

BasicBlock::iterator BasicBlock::begin() const {
  toolchain::BasicBlock *BB = cast<toolchain::BasicBlock>(Val);
  toolchain::BasicBlock::iterator It = BB->begin();
  if (!BB->empty()) {
    auto *V = Ctx.getValue(&*BB->begin());
    assert(V != nullptr && "No SandboxIR for BB->begin()!");
    auto *I = cast<Instruction>(V);
    unsigned Num = I->getNumOfIRInstrs();
    assert(Num >= 1u && "Bad getNumOfIRInstrs()");
    It = std::next(It, Num - 1);
  }
  return iterator(BB, It, &Ctx);
}

Instruction *BasicBlock::getTerminator() const {
  auto *TerminatorV =
      Ctx.getValue(cast<toolchain::BasicBlock>(Val)->getTerminator());
  return cast_or_null<Instruction>(TerminatorV);
}

Instruction &BasicBlock::front() const {
  auto *BB = cast<toolchain::BasicBlock>(Val);
  assert(!BB->empty() && "Empty block!");
  auto *SBI = cast<Instruction>(getContext().getValue(&*BB->begin()));
  assert(SBI != nullptr && "Expected Instr!");
  return *SBI;
}

Instruction &BasicBlock::back() const {
  auto *BB = cast<toolchain::BasicBlock>(Val);
  assert(!BB->empty() && "Empty block!");
  auto *SBI = cast<Instruction>(getContext().getValue(&*BB->rbegin()));
  assert(SBI != nullptr && "Expected Instr!");
  return *SBI;
}

#ifndef NDEBUG
void BasicBlock::dumpOS(raw_ostream &OS) const {
  toolchain::BasicBlock *BB = cast<toolchain::BasicBlock>(Val);
  const auto &Name = BB->getName();
  OS << Name;
  if (!Name.empty())
    OS << ":\n";
  // If there are Instructions in the BB that are not mapped to SandboxIR, then
  // use a crash-proof dump.
  if (any_of(*BB, [this](toolchain::Instruction &I) {
        return Ctx.getValue(&I) == nullptr;
      })) {
    OS << "<Crash-proof mode!>\n";
    DenseSet<Instruction *> Visited;
    for (toolchain::Instruction &IRef : *BB) {
      Value *SBV = Ctx.getValue(&IRef);
      if (SBV == nullptr)
        OS << IRef << " *** No SandboxIR ***\n";
      else {
        auto *SBI = dyn_cast<Instruction>(SBV);
        if (SBI == nullptr) {
          OS << IRef << " *** Not a SBInstruction!!! ***\n";
        } else {
          if (Visited.insert(SBI).second)
            OS << *SBI << "\n";
        }
      }
    }
  } else {
    for (auto &SBI : *this) {
      SBI.dumpOS(OS);
      OS << "\n";
    }
  }
}

void BasicBlock::verify() const {
  assert(isa<toolchain::BasicBlock>(Val) && "Expected BasicBlock!");
  for (const auto &I : *this) {
    I.verify();
  }
}
#endif // NDEBUG

} // namespace vm::core::sandboxir
