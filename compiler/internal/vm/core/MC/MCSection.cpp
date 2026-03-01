//===- lib/MC/MCSection.cpp - Machine Code Section Representation ---------===//
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

#include "vm/core/MC/MCSection.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/Config/toolchain-config.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCSymbol.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/raw_ostream.h"
#include <utility>

using namespace vm::core;

MCSection::MCSection(StringRef Name, bool IsText, bool IsBss, MCSymbol *Begin)
    : Begin(Begin), HasInstructions(false), IsRegistered(false), IsText(IsText),
      IsBss(IsBss), Name(Name) {
  DummyFragment.setParent(this);
}

MCSymbol *MCSection::getEndSymbol(MCContext &Ctx) {
  if (!End)
    End = Ctx.createTempSymbol("sec_end");
  return End;
}

bool MCSection::hasEnded() const { return End && End->isInSection(); }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void MCSection::dump(
    DenseMap<const MCFragment *, SmallVector<const MCSymbol *, 0>> *FragToSyms)
    const {
  raw_ostream &OS = errs();

  OS << "MCSection Name:" << getName();
  if (isLinkerRelaxable())
    OS << " FirstLinkerRelaxable:" << firstLinkerRelaxable();
  for (auto &F : *this) {
    OS << '\n';
    F.dump();
    if (!FragToSyms)
      continue;
    auto It = FragToSyms->find(&F);
    if (It == FragToSyms->end())
      continue;
    for (auto *Sym : It->second) {
      OS << "\n  Symbol @" << Sym->getOffset() << ' ' << Sym->getName();
      if (Sym->isTemporary())
        OS << " Temporary";
    }
  }
}
#endif

void MCFragment::setVarContents(ArrayRef<char> Contents) {
  auto &S = getParent()->ContentStorage;
  if (VarContentStart + Contents.size() > VarContentEnd) {
    VarContentStart = S.size();
    S.resize_for_overwrite(S.size() + Contents.size());
  }
  VarContentEnd = VarContentStart + Contents.size();
  toolchain::copy(Contents, S.begin() + VarContentStart);
}

void MCFragment::addFixup(MCFixup Fixup) { appendFixups({Fixup}); }

void MCFragment::appendFixups(ArrayRef<MCFixup> Fixups) {
  auto &S = getParent()->FixupStorage;
  if (LLVM_UNLIKELY(FixupEnd != S.size())) {
    // Move the elements to the end. Reserve space to avoid invalidating
    // S.begin()+I for `append`.
    auto Size = FixupEnd - FixupStart;
    auto I = std::exchange(FixupStart, S.size());
    S.reserve(S.size() + Size);
    S.append(S.begin() + I, S.begin() + I + Size);
  }
  S.append(Fixups.begin(), Fixups.end());
  FixupEnd = S.size();
}

void MCFragment::setVarFixups(ArrayRef<MCFixup> Fixups) {
  assert(Fixups.size() < 256 &&
         "variable-size tail cannot have more than 256 fixups");
  auto &S = getParent()->FixupStorage;
  if (Fixups.size() > VarFixupSize) {
    VarFixupStart = S.size();
    S.resize_for_overwrite(S.size() + Fixups.size());
  }
  VarFixupSize = Fixups.size();
  // Source fixup offsets are relative to the variable part's start. Add the
  // fixed part size to make them relative to the fixed part's start.
  std::transform(Fixups.begin(), Fixups.end(), S.begin() + VarFixupStart,
                 [Fixed = getFixedSize()](MCFixup F) {
                   F.setOffset(Fixed + F.getOffset());
                   return F;
                 });
}
