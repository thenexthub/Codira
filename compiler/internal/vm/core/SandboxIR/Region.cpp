//===- Region.cpp ---------------------------------------------------------===//
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

#include "vm/core/SandboxIR/Region.h"
#include "vm/core/SandboxIR/Function.h"

namespace vm::core::sandboxir {

InstructionCost ScoreBoard::getCost(Instruction *I) const {
  auto *LLVMI = cast<toolchain::Instruction>(I->Val);
  SmallVector<const toolchain::Value *> Operands(LLVMI->operands());
  return TTI.getInstructionCost(LLVMI, Operands, CostKind);
}

void ScoreBoard::remove(Instruction *I) {
  auto Cost = getCost(I);
  if (Rgn.contains(I))
    // If `I` is one the newly added ones, then we should adjust `AfterCost`
    AfterCost -= Cost;
  else
    // If `I` is one of the original instructions (outside the region) then it
    // is part of the original code, so adjust `BeforeCost`.
    BeforeCost += Cost;
}

#ifndef NDEBUG
void ScoreBoard::dump() const { dump(dbgs()); }
#endif

Region::Region(Context &Ctx, TargetTransformInfo &TTI)
    : Ctx(Ctx), Scoreboard(*this, TTI) {
  LLVMContext &LLVMCtx = Ctx.LLVMCtx;
  auto *RegionStrMD = MDString::get(LLVMCtx, RegionStr);
  RegionMDN = MDNode::getDistinct(LLVMCtx, {RegionStrMD});

  CreateInstCB = Ctx.registerCreateInstrCallback(
      [this](Instruction *NewInst) { add(NewInst); });
  EraseInstCB = Ctx.registerEraseInstrCallback([this](Instruction *ErasedInst) {
    remove(ErasedInst);
    removeFromAux(ErasedInst);
  });
}

Region::~Region() {
  Ctx.unregisterCreateInstrCallback(CreateInstCB);
  Ctx.unregisterEraseInstrCallback(EraseInstCB);
}

void Region::addImpl(Instruction *I, bool IgnoreCost) {
  Insts.insert(I);
  // TODO: Consider tagging instructions lazily.
  cast<toolchain::Instruction>(I->Val)->setMetadata(MDKind, RegionMDN);
  if (!IgnoreCost)
    // Keep track of the instruction cost.
    Scoreboard.add(I);
}

void Region::setAux(ArrayRef<Instruction *> Aux) {
  this->Aux = SmallVector<Instruction *>(Aux);
  auto &LLVMCtx = Ctx.LLVMCtx;
  for (auto [Idx, I] : enumerate(Aux)) {
    toolchain::ConstantInt *IdxC =
        toolchain::ConstantInt::get(toolchain::Type::getInt32Ty(LLVMCtx), Idx, false);
    assert(cast<toolchain::Instruction>(I->Val)->getMetadata(AuxMDKind) == nullptr &&
           "Instruction already in Aux!");
    cast<toolchain::Instruction>(I->Val)->setMetadata(
        AuxMDKind, MDNode::get(LLVMCtx, ConstantAsMetadata::get(IdxC)));
    // Aux instrs should always be in a region.
    addImpl(I, /*DontTrackCost=*/true);
  }
}

void Region::setAux(unsigned Idx, Instruction *I) {
  assert((Idx >= Aux.size() || Aux[Idx] == nullptr) &&
         "There is already an Instruction at Idx in Aux!");
  unsigned ExpectedSz = Idx + 1;
  if (Aux.size() < ExpectedSz) {
    auto SzBefore = Aux.size();
    Aux.resize(ExpectedSz);
    // Initialize the gap with nullptr.
    for (unsigned Idx = SzBefore; Idx + 1 < ExpectedSz; ++Idx)
      Aux[Idx] = nullptr;
  }
  Aux[Idx] = I;
  // Aux instrs should always be in a region.
  addImpl(I, /*DontTrackCost=*/true);
}

void Region::dropAuxMetadata(Instruction *I) {
  auto *LLVMI = cast<toolchain::Instruction>(I->Val);
  LLVMI->setMetadata(AuxMDKind, nullptr);
}

void Region::removeFromAux(Instruction *I) {
  auto It = find(Aux, I);
  if (It == Aux.end())
    return;
  dropAuxMetadata(I);
  Aux.erase(It);
}

void Region::clearAux() {
  for (unsigned Idx : seq<unsigned>(0, Aux.size()))
    dropAuxMetadata(Aux[Idx]);
  Aux.clear();
}

void Region::remove(Instruction *I) {
  // Keep track of the instruction cost. This need to be done *before* we remove
  // `I` from the region.
  Scoreboard.remove(I);

  Insts.remove(I);
  cast<toolchain::Instruction>(I->Val)->setMetadata(MDKind, nullptr);
}

#ifndef NDEBUG
bool Region::operator==(const Region &Other) const {
  if (Insts.size() != Other.Insts.size())
    return false;
  if (!std::is_permutation(Insts.begin(), Insts.end(), Other.Insts.begin()))
    return false;
  return true;
}

void Region::dump(raw_ostream &OS) const {
  for (auto *I : Insts)
    OS << *I << "\n";
  if (!Aux.empty()) {
    OS << "\nAux:\n";
    for (auto *I : Aux) {
      if (I == nullptr)
        OS << "NULL\n";
      else
        OS << *I << "\n";
    }
  }
}

void Region::dump() const {
  dump(dbgs());
  dbgs() << "\n";
}
#endif // NDEBUG

SmallVector<std::unique_ptr<Region>>
Region::createRegionsFromMD(Function &F, TargetTransformInfo &TTI) {
  SmallVector<std::unique_ptr<Region>> Regions;
  DenseMap<MDNode *, Region *> MDNToRegion;
  auto &Ctx = F.getContext();
  for (BasicBlock &BB : F) {
    for (Instruction &Inst : BB) {
      auto *LLVMI = cast<toolchain::Instruction>(Inst.Val);
      Region *R = nullptr;
      if (auto *MDN = LLVMI->getMetadata(MDKind)) {
        auto [It, Inserted] = MDNToRegion.try_emplace(MDN);
        if (Inserted) {
          Regions.push_back(std::make_unique<Region>(Ctx, TTI));
          R = Regions.back().get();
          It->second = R;
        } else {
          R = It->second;
        }
        R->addImpl(&Inst, /*IgnoreCost=*/true);
      }
      if (auto *AuxMDN = LLVMI->getMetadata(AuxMDKind)) {
        toolchain::Constant *IdxC =
            dyn_cast<ConstantAsMetadata>(AuxMDN->getOperand(0))->getValue();
        auto Idx = cast<toolchain::ConstantInt>(IdxC)->getSExtValue();
        if (R == nullptr) {
          errs() << "No region specified for Aux: '" << *LLVMI << "'\n";
          exit(1);
        }
        R->setAux(Idx, &Inst);
      }
    }
  }
#ifndef NDEBUG
  // Check that there are no gaps in the Aux vector.
  for (auto &RPtr : Regions)
    for (auto *I : RPtr->getAux())
      assert(I != nullptr && "Gap in Aux!");
#endif
  return Regions;
}

} // namespace vm::core::sandboxir
