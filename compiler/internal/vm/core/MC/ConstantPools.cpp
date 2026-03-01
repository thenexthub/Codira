//===- ConstantPools.cpp - ConstantPool class -----------------------------===//
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
// This file implements the ConstantPool and  AssemblerConstantPools classes.
//
//===----------------------------------------------------------------------===//

#include "vm/core/MC/ConstantPools.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCDirectives.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/Support/Casting.h"

using namespace vm::core;

//
// ConstantPool implementation
//
// Emit the contents of the constant pool using the provided streamer.
void ConstantPool::emitEntries(MCStreamer &Streamer) {
  if (Entries.empty())
    return;
  Streamer.emitDataRegion(MCDR_DataRegion);
  for (const ConstantPoolEntry &Entry : Entries) {
    Streamer.emitValueToAlignment(Align(Entry.Size)); // align naturally
    Streamer.emitLabel(Entry.Label);
    Streamer.emitValue(Entry.Value, Entry.Size, Entry.Loc);
  }
  Streamer.emitDataRegion(MCDR_DataRegionEnd);
  Entries.clear();
}

const MCExpr *ConstantPool::addEntry(const MCExpr *Value, MCContext &Context,
                                     unsigned Size, SMLoc Loc) {
  const MCConstantExpr *C = dyn_cast<MCConstantExpr>(Value);
  const MCSymbolRefExpr *S = dyn_cast<MCSymbolRefExpr>(Value);

  // Check if there is existing entry for the same constant. If so, reuse it.
  if (C) {
    auto CItr = CachedConstantEntries.find(std::make_pair(C->getValue(), Size));
    if (CItr != CachedConstantEntries.end())
      return CItr->second;
  }

  // Check if there is existing entry for the same symbol. If so, reuse it.
  if (S) {
    auto SItr =
        CachedSymbolEntries.find(std::make_pair(&(S->getSymbol()), Size));
    if (SItr != CachedSymbolEntries.end())
      return SItr->second;
  }

  MCSymbol *CPEntryLabel = Context.createTempSymbol();

  Entries.push_back(ConstantPoolEntry(CPEntryLabel, Value, Size, Loc));
  const auto SymRef = MCSymbolRefExpr::create(CPEntryLabel, Context, Loc);
  if (C)
    CachedConstantEntries[std::make_pair(C->getValue(), Size)] = SymRef;
  if (S)
    CachedSymbolEntries[std::make_pair(&(S->getSymbol()), Size)] = SymRef;
  return SymRef;
}

bool ConstantPool::empty() { return Entries.empty(); }

void ConstantPool::clearCache() {
  CachedConstantEntries.clear();
  CachedSymbolEntries.clear();
}

//
// AssemblerConstantPools implementation
//
ConstantPool *AssemblerConstantPools::getConstantPool(MCSection *Section) {
  ConstantPoolMapTy::iterator CP = ConstantPools.find(Section);
  if (CP == ConstantPools.end())
    return nullptr;

  return &CP->second;
}

ConstantPool &
AssemblerConstantPools::getOrCreateConstantPool(MCSection *Section) {
  return ConstantPools[Section];
}

static void emitConstantPool(MCStreamer &Streamer, MCSection *Section,
                             ConstantPool &CP) {
  if (!CP.empty()) {
    Streamer.switchSection(Section);
    CP.emitEntries(Streamer);
  }
}

void AssemblerConstantPools::emitAll(MCStreamer &Streamer) {
  // Dump contents of assembler constant pools.
  for (auto &CPI : ConstantPools) {
    MCSection *Section = CPI.first;
    ConstantPool &CP = CPI.second;

    emitConstantPool(Streamer, Section, CP);
  }
}

void AssemblerConstantPools::emitForCurrentSection(MCStreamer &Streamer) {
  MCSection *Section = Streamer.getCurrentSectionOnly();
  if (ConstantPool *CP = getConstantPool(Section))
    emitConstantPool(Streamer, Section, *CP);
}

void AssemblerConstantPools::clearCacheForCurrentSection(MCStreamer &Streamer) {
  MCSection *Section = Streamer.getCurrentSectionOnly();
  if (ConstantPool *CP = getConstantPool(Section))
    CP->clearCache();
}

const MCExpr *AssemblerConstantPools::addEntry(MCStreamer &Streamer,
                                               const MCExpr *Expr,
                                               unsigned Size, SMLoc Loc) {
  MCSection *Section = Streamer.getCurrentSectionOnly();
  return getOrCreateConstantPool(Section).addEntry(Expr, Streamer.getContext(),
                                                   Size, Loc);
}
