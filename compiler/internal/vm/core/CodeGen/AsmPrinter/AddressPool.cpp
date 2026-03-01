//===- toolchain/CodeGen/AddressPool.cpp - Dwarf Debug Framework ---------------===//
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

#include "AddressPool.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/Target/TargetLoweringObjectFile.h"

using namespace vm::core;

unsigned AddressPool::getIndex(const MCSymbol *Sym, bool TLS) {
  resetUsedFlag(true);
  auto IterBool = Pool.try_emplace(Sym, Pool.size(), TLS);
  return IterBool.first->second.Number;
}

MCSymbol *AddressPool::emitHeader(AsmPrinter &Asm, MCSection *Section) {
  static const uint8_t AddrSize = Asm.MAI->getCodePointerSize();

  MCSymbol *EndLabel =
      Asm.emitDwarfUnitLength("debug_addr", "Length of contribution");
  Asm.OutStreamer->AddComment("DWARF version number");
  Asm.emitInt16(Asm.getDwarfVersion());
  Asm.OutStreamer->AddComment("Address size");
  Asm.emitInt8(AddrSize);
  Asm.OutStreamer->AddComment("Segment selector size");
  Asm.emitInt8(0); // TODO: Support non-zero segment_selector_size.

  return EndLabel;
}

// Emit addresses into the section given.
void AddressPool::emit(AsmPrinter &Asm, MCSection *AddrSection) {
  if (isEmpty())
    return;

  // Start the dwarf addr section.
  Asm.OutStreamer->switchSection(AddrSection);

  MCSymbol *EndLabel = nullptr;

  if (Asm.getDwarfVersion() >= 5)
    EndLabel = emitHeader(Asm, AddrSection);

  // Define the symbol that marks the start of the contribution.
  // It is referenced via DW_AT_addr_base.
  Asm.OutStreamer->emitLabel(AddressTableBaseSym);

  // Order the address pool entries by ID
  SmallVector<const MCExpr *, 64> Entries(Pool.size());

  for (const auto &I : Pool)
    Entries[I.second.Number] =
        I.second.TLS
            ? Asm.getObjFileLowering().getDebugThreadLocalSymbol(I.first)
            : MCSymbolRefExpr::create(I.first, Asm.OutContext);

  for (const MCExpr *Entry : Entries)
    Asm.OutStreamer->emitValue(Entry, Asm.MAI->getCodePointerSize());

  if (EndLabel)
    Asm.OutStreamer->emitLabel(EndLabel);
}
