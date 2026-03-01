//===-- CSKYTargetStreamer.h - CSKY Target Streamer ----------*- C++ -*----===//
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

#include "CSKYTargetStreamer.h"
#include "MCTargetDesc/CSKYMCAsmInfo.h"
#include "vm/core/CodeGen/MachineFrameInfo.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCSectionELF.h"
#include "vm/core/Support/FormattedStream.h"

using namespace vm::core;

//
// ConstantPool implementation
//
// Emit the contents of the constant pool using the provided streamer.
void CSKYConstantPool::emitAll(MCStreamer &Streamer) {
  if (Entries.empty())
    return;

  if (CurrentSection != nullptr)
    Streamer.switchSection(CurrentSection);

  Streamer.emitDataRegion(MCDR_DataRegion);
  for (const ConstantPoolEntry &Entry : Entries) {
    Streamer.emitCodeAlignment(
        Align(Entry.Size),
        Streamer.getContext().getSubtargetInfo()); // align naturally
    Streamer.emitLabel(Entry.Label);
    Streamer.emitValue(Entry.Value, Entry.Size, Entry.Loc);
  }
  Streamer.emitDataRegion(MCDR_DataRegionEnd);
  Entries.clear();
}

const MCExpr *CSKYConstantPool::addEntry(MCStreamer &Streamer,
                                         const MCExpr *Value, unsigned Size,
                                         SMLoc Loc, const MCExpr *AdjustExpr) {
  if (CurrentSection == nullptr)
    CurrentSection = Streamer.getCurrentSectionOnly();

  auto &Context = Streamer.getContext();

  const MCConstantExpr *C = dyn_cast<MCConstantExpr>(Value);

  // Check if there is existing entry for the same constant. If so, reuse it.
  auto Itr = C ? CachedEntries.find(C->getValue()) : CachedEntries.end();
  if (Itr != CachedEntries.end())
    return Itr->second;

  MCSymbol *CPEntryLabel = Context.createTempSymbol();
  const auto SymRef = MCSymbolRefExpr::create(CPEntryLabel, Context);

  if (AdjustExpr) {
    auto *CSKYExpr = cast<MCSpecifierExpr>(Value);

    Value = MCBinaryExpr::createSub(AdjustExpr, SymRef, Context);
    Value = MCBinaryExpr::createSub(CSKYExpr->getSubExpr(), Value, Context);
    Value = MCSpecifierExpr::create(Value, CSKYExpr->getSpecifier(), Context);
  }

  Entries.push_back(ConstantPoolEntry(CPEntryLabel, Value, Size, Loc));

  if (C)
    CachedEntries[C->getValue()] = SymRef;
  return SymRef;
}

bool CSKYConstantPool::empty() { return Entries.empty(); }

void CSKYConstantPool::clearCache() {
  CurrentSection = nullptr;
  CachedEntries.clear();
}

CSKYTargetStreamer::CSKYTargetStreamer(MCStreamer &S)
    : MCTargetStreamer(S), ConstantPool(new CSKYConstantPool()) {}

const MCExpr *
CSKYTargetStreamer::addConstantPoolEntry(const MCExpr *Expr, SMLoc Loc,
                                         const MCExpr *AdjustExpr) {
  uint8_t ELFRefKind = CSKY::S_Invalid;
  ConstantCounter++;

  const MCExpr *OrigExpr = Expr;

  if (auto *CE = dyn_cast<MCSpecifierExpr>(Expr)) {
    Expr = CE->getSubExpr();
    ELFRefKind = CE->getSpecifier();
  }

  if (const MCSymbolRefExpr *SymExpr = dyn_cast<MCSymbolRefExpr>(Expr)) {
    const MCSymbol *Sym = &SymExpr->getSymbol();

    SymbolIndex Index = {Sym, ELFRefKind};

    if (ConstantMap.find(Index) == ConstantMap.end()) {
      ConstantMap[Index] =
          ConstantPool->addEntry(getStreamer(), OrigExpr, 4, Loc, AdjustExpr);
    }
    return ConstantMap[Index];
  }

  return ConstantPool->addEntry(getStreamer(), Expr, 4, Loc, AdjustExpr);
}

void CSKYTargetStreamer::emitCurrentConstantPool() {
  ConstantPool->emitAll(Streamer);
  ConstantPool->clearCache();
}

// finish() - write out any non-empty assembler constant pools.
void CSKYTargetStreamer::finish() {
  if (ConstantCounter != 0) {
    ConstantPool->emitAll(Streamer);
  }

  finishAttributeSection();
}

void CSKYTargetStreamer::emitTargetAttributes(const MCSubtargetInfo &STI) {}

void CSKYTargetStreamer::emitAttribute(unsigned Attribute, unsigned Value) {}
void CSKYTargetStreamer::emitTextAttribute(unsigned Attribute,
                                           StringRef String) {}
void CSKYTargetStreamer::finishAttributeSection() {}

void CSKYTargetAsmStreamer::emitAttribute(unsigned Attribute, unsigned Value) {
  OS << "\t.csky_attribute\t" << Attribute << ", " << Twine(Value) << "\n";
}

void CSKYTargetAsmStreamer::emitTextAttribute(unsigned Attribute,
                                              StringRef String) {
  OS << "\t.csky_attribute\t" << Attribute << ", \"" << String << "\"\n";
}

void CSKYTargetAsmStreamer::finishAttributeSection() {}
