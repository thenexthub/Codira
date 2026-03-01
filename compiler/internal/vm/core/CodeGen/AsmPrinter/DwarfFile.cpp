//===- toolchain/CodeGen/DwarfFile.cpp - Dwarf Debug Framework -----------------===//
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

#include "DwarfFile.h"
#include "DwarfCompileUnit.h"
#include "DwarfDebug.h"
#include "DwarfUnit.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/IR/DebugInfoMetadata.h"
#include "vm/core/MC/MCStreamer.h"
#include <cstdint>

using namespace vm::core;

DwarfFile::DwarfFile(AsmPrinter *AP, StringRef Pref, BumpPtrAllocator &DA)
    : Asm(AP), Abbrevs(AbbrevAllocator), StrPool(DA, *Asm, Pref) {}

void DwarfFile::addUnit(std::unique_ptr<DwarfCompileUnit> U) {
  CUs.push_back(std::move(U));
}

// Emit the various dwarf units to the unit section USection with
// the abbreviations going into ASection.
void DwarfFile::emitUnits(bool UseOffsets) {
  for (const auto &TheU : CUs)
    emitUnit(TheU.get(), UseOffsets);
}

void DwarfFile::emitUnit(DwarfUnit *TheU, bool UseOffsets) {
  if (TheU->getCUNode()->isDebugDirectivesOnly())
    return;

  MCSection *S = TheU->getSection();

  if (!S)
    return;

  // Skip CUs that ended up not being needed (split CUs that were abandoned
  // because they added no information beyond the non-split CU)
  if (TheU->getUnitDie().values().empty())
    return;

  Asm->OutStreamer->switchSection(S);
  TheU->emitHeader(UseOffsets);
  Asm->emitDwarfDIE(TheU->getUnitDie());

  if (MCSymbol *EndLabel = TheU->getEndLabel())
    Asm->OutStreamer->emitLabel(EndLabel);
}

// Compute the size and offset for each DIE.
void DwarfFile::computeSizeAndOffsets() {
  // Offset from the first CU in the debug info section is 0 initially.
  uint64_t SecOffset = 0;

  // Iterate over each compile unit and set the size and offsets for each
  // DIE within each compile unit. All offsets are CU relative.
  for (const auto &TheU : CUs) {
    if (TheU->getCUNode()->isDebugDirectivesOnly())
      continue;

    // Skip CUs that ended up not being needed (split CUs that were abandoned
    // because they added no information beyond the non-split CU)
    if (TheU->getUnitDie().values().empty())
      return;

    TheU->setDebugSectionOffset(SecOffset);
    SecOffset += computeSizeAndOffsetsForUnit(TheU.get());
  }
  if (SecOffset > UINT32_MAX && !Asm->isDwarf64())
    report_fatal_error("The generated debug information is too large "
                       "for the 32-bit DWARF format.");
}

unsigned DwarfFile::computeSizeAndOffsetsForUnit(DwarfUnit *TheU) {
  // CU-relative offset is reset to 0 here.
  unsigned Offset = Asm->getUnitLengthFieldByteSize() + // Length of Unit Info
                    TheU->getHeaderSize();              // Unit-specific headers

  // The return value here is CU-relative, after laying out
  // all of the CU DIE.
  return computeSizeAndOffset(TheU->getUnitDie(), Offset);
}

// Compute the size and offset of a DIE. The offset is relative to start of the
// CU. It returns the offset after laying out the DIE.
unsigned DwarfFile::computeSizeAndOffset(DIE &Die, unsigned Offset) {
  return Die.computeOffsetsAndAbbrevs(Asm->getDwarfFormParams(), Abbrevs,
                                      Offset);
}

void DwarfFile::emitAbbrevs(MCSection *Section) { Abbrevs.Emit(Asm, Section); }

// Emit strings into a string section.
void DwarfFile::emitStrings(MCSection *StrSection, MCSection *OffsetSection,
                            bool UseRelativeOffsets) {
  StrPool.emit(*Asm, StrSection, OffsetSection, UseRelativeOffsets);
}

void DwarfFile::addScopeVariable(LexicalScope *LS, DbgVariable *Var) {
  auto &ScopeVars = ScopeVariables[LS];
  const DILocalVariable *DV = Var->getVariable();
  if (unsigned ArgNum = DV->getArg()) {
    auto Ret = ScopeVars.Args.insert({ArgNum, Var});
    assert(Ret.second);
    (void)Ret;
  } else {
    ScopeVars.Locals.push_back(Var);
  }
}

void DwarfFile::addScopeLabel(LexicalScope *LS, DbgLabel *Label) {
  SmallVectorImpl<DbgLabel *> &Labels = ScopeLabels[LS];
  Labels.push_back(Label);
}

std::pair<uint32_t, RangeSpanList *>
DwarfFile::addRange(const DwarfCompileUnit &CU, SmallVector<RangeSpan, 2> R) {
  bool CanReuseLastRange = false;

  if (!CURangeLists.empty()) {
    auto Last = CURangeLists.back();
    if (Last.CU == &CU && Last.Ranges == R) {
      CanReuseLastRange = true;
    }
  }

  if (!CanReuseLastRange) {
    CURangeLists.push_back(RangeSpanList{Asm->createTempSymbol("debug_ranges"),
                                         &CU, std::move(R)});
  }

  return std::make_pair(CURangeLists.size() - 1, &CURangeLists.back());
}
