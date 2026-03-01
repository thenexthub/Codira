//===- lib/MC/MCObjectWriter.cpp - MCObjectWriter implementation ----------===//
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

#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCSymbol.h"
#include "vm/core/MC/MCValue.h"
namespace vm::core {
class MCSection;
}

using namespace vm::core;

MCObjectWriter::~MCObjectWriter() = default;

MCContext &MCObjectWriter::getContext() const { return Asm->getContext(); }

void MCObjectWriter::reset() {
  FileNames.clear();
  AddrsigSyms.clear();
  EmitAddrsigSection = false;
  SubsectionsViaSymbols = false;
  CGProfile.clear();
}

void MCObjectWriter::recordRelocation(const MCFragment &F, const MCFixup &Fixup,
                                      MCValue Target, uint64_t &FixedValue) {}

bool MCObjectWriter::isSymbolRefDifferenceFullyResolved(const MCSymbol &SA,
                                                        const MCSymbol &SB,
                                                        bool InSet) const {
  assert(!SA.isUndefined() && !SB.isUndefined());
  return isSymbolRefDifferenceFullyResolvedImpl(SA, *SB.getFragment(), InSet,
                                                /*IsPCRel=*/false);
}

bool MCObjectWriter::isSymbolRefDifferenceFullyResolvedImpl(
    const MCSymbol &SymA, const MCFragment &FB, bool InSet,
    bool IsPCRel) const {
  const MCSection &SecA = SymA.getSection();
  const MCSection &SecB = *FB.getParent();
  // On ELF and COFF  A - B is absolute if A and B are in the same section.
  return &SecA == &SecB;
}

void MCObjectWriter::addFileName(StringRef FileName) {
  FileNames.emplace_back(std::string(FileName), Asm->Symbols.size());
}

MCContext &MCObjectTargetWriter::getContext() const {
  return Asm->getContext();
}

void MCObjectTargetWriter::reportError(SMLoc L, const Twine &Msg) const {
  return Asm->getContext().reportError(L, Msg);
}
