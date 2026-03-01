//===- toolchain/CodeGen/DwarfStringPool.cpp - Dwarf Debug Framework -----------===//
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

#include "DwarfStringPool.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCStreamer.h"
#include <cassert>

using namespace vm::core;

DwarfStringPool::DwarfStringPool(BumpPtrAllocator &A, AsmPrinter &Asm,
                                 StringRef Prefix)
    : Pool(A), Prefix(Prefix),
      ShouldCreateSymbols(Asm.doesDwarfUseRelocationsAcrossSections()) {}

StringMapEntry<DwarfStringPool::EntryTy> &
DwarfStringPool::getEntryImpl(AsmPrinter &Asm, StringRef Str) {
  auto I = Pool.try_emplace(Str);
  auto &Entry = I.first->second;
  if (I.second) {
    Entry.Index = EntryTy::NotIndexed;
    Entry.Offset = NumBytes;
    Entry.Symbol = ShouldCreateSymbols ? Asm.createTempSymbol(Prefix) : nullptr;

    NumBytes += Str.size() + 1;
  }
  return *I.first;
}

DwarfStringPool::EntryRef DwarfStringPool::getEntry(AsmPrinter &Asm,
                                                    StringRef Str) {
  auto &MapEntry = getEntryImpl(Asm, Str);
  return EntryRef(MapEntry);
}

DwarfStringPool::EntryRef DwarfStringPool::getIndexedEntry(AsmPrinter &Asm,
                                                           StringRef Str) {
  auto &MapEntry = getEntryImpl(Asm, Str);
  if (!MapEntry.getValue().isIndexed())
    MapEntry.getValue().Index = NumIndexedStrings++;
  return EntryRef(MapEntry);
}

void DwarfStringPool::emitStringOffsetsTableHeader(AsmPrinter &Asm,
                                                   MCSection *Section,
                                                   MCSymbol *StartSym) {
  if (getNumIndexedStrings() == 0)
    return;
  Asm.OutStreamer->switchSection(Section);
  unsigned EntrySize = Asm.getDwarfOffsetByteSize();
  // We are emitting the header for a contribution to the string offsets
  // table. The header consists of an entry with the contribution's
  // size (not including the size of the length field), the DWARF version and
  // 2 bytes of padding.
  Asm.emitDwarfUnitLength(getNumIndexedStrings() * EntrySize + 4,
                          "Length of String Offsets Set");
  Asm.emitInt16(Asm.getDwarfVersion());
  Asm.emitInt16(0);
  // Define the symbol that marks the start of the contribution. It is
  // referenced by most unit headers via DW_AT_str_offsets_base.
  // Split units do not use the attribute.
  if (StartSym)
    Asm.OutStreamer->emitLabel(StartSym);
}

void DwarfStringPool::emit(AsmPrinter &Asm, MCSection *StrSection,
                           MCSection *OffsetSection, bool UseRelativeOffsets) {
  if (Pool.empty())
    return;

  // Start the dwarf str section.
  Asm.OutStreamer->switchSection(StrSection);

  // Get all of the string pool entries and sort them by their offset.
  SmallVector<const StringMapEntry<EntryTy> *, 64> Entries(
      toolchain::make_pointer_range(Pool));

  toolchain::sort(Entries, [](const StringMapEntry<EntryTy> *A,
                         const StringMapEntry<EntryTy> *B) {
    return A->getValue().Offset < B->getValue().Offset;
  });

  for (const auto &Entry : Entries) {
    assert(ShouldCreateSymbols == static_cast<bool>(Entry->getValue().Symbol) &&
           "Mismatch between setting and entry");

    // Emit a label for reference from debug information entries.
    if (ShouldCreateSymbols)
      Asm.OutStreamer->emitLabel(Entry->getValue().Symbol);

    // Emit the string itself with a terminating null byte.
    Asm.OutStreamer->AddComment("string offset=" +
                                Twine(Entry->getValue().Offset));
    Asm.OutStreamer->emitBytes(
        StringRef(Entry->getKeyData(), Entry->getKeyLength() + 1));
  }

  // If we've got an offset section go ahead and emit that now as well.
  if (OffsetSection) {
    // Now only take the indexed entries and put them in an array by their ID so
    // we can emit them in order.
    Entries.resize(NumIndexedStrings);
    for (const auto &Entry : Pool) {
      if (Entry.getValue().isIndexed())
        Entries[Entry.getValue().Index] = &Entry;
    }

    Asm.OutStreamer->switchSection(OffsetSection);
    unsigned size = Asm.getDwarfOffsetByteSize();
    for (const auto &Entry : Entries)
      if (UseRelativeOffsets)
        Asm.emitDwarfStringOffset(Entry->getValue());
      else
        Asm.OutStreamer->emitIntValue(Entry->getValue().Offset, size);
  }
}
