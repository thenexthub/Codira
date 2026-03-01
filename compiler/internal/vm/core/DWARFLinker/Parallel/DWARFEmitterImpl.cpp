//===- DWARFEmitterImpl.cpp -----------------------------------------------===//
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

#include "DWARFEmitterImpl.h"
#include "DWARFLinkerCompileUnit.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCInstPrinter.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/MCTargetOptions.h"
#include "vm/core/MC/MCTargetOptionsCommandFlags.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/FormattedStream.h"

using namespace vm::core;
using namespace dwarf_linker;
using namespace dwarf_linker::parallel;

Error DwarfEmitterImpl::init(Triple TheTriple,
                             StringRef Swift5ReflectionSegmentName) {
  std::string ErrorStr;
  std::string TripleName;

  // Get the target.
  const Target *TheTarget =
      TargetRegistry::lookupTarget(TripleName, TheTriple, ErrorStr);
  if (!TheTarget)
    return createStringError(std::errc::invalid_argument, ErrorStr.c_str());
  TripleName = TheTriple.getTriple();

  // Create all the MC Objects.
  MRI.reset(TheTarget->createMCRegInfo(TheTriple));
  if (!MRI)
    return createStringError(std::errc::invalid_argument,
                             "no register info for target %s",
                             TripleName.c_str());

  MCTargetOptions MCOptions = mc::InitMCTargetOptionsFromFlags();
  MCOptions.AsmVerbose = true;
  MCOptions.MCUseDwarfDirectory = MCTargetOptions::EnableDwarfDirectory;
  MAI.reset(TheTarget->createMCAsmInfo(*MRI, TheTriple, MCOptions));
  if (!MAI)
    return createStringError(std::errc::invalid_argument,
                             "no asm info for target %s", TripleName.c_str());

  MSTI.reset(TheTarget->createMCSubtargetInfo(TheTriple, "", ""));
  if (!MSTI)
    return createStringError(std::errc::invalid_argument,
                             "no subtarget info for target %s",
                             TripleName.c_str());

  MC.reset(new MCContext(TheTriple, MAI.get(), MRI.get(), MSTI.get(), nullptr,
                         nullptr, true, Swift5ReflectionSegmentName));
  MOFI.reset(TheTarget->createMCObjectFileInfo(*MC, /*PIC=*/false, false));
  MC->setObjectFileInfo(MOFI.get());

  MAB = TheTarget->createMCAsmBackend(*MSTI, *MRI, MCOptions);
  if (!MAB)
    return createStringError(std::errc::invalid_argument,
                             "no asm backend for target %s",
                             TripleName.c_str());

  MII.reset(TheTarget->createMCInstrInfo());
  if (!MII)
    return createStringError(std::errc::invalid_argument,
                             "no instr info info for target %s",
                             TripleName.c_str());

  MCE = TheTarget->createMCCodeEmitter(*MII, *MC);
  if (!MCE)
    return createStringError(std::errc::invalid_argument,
                             "no code emitter for target %s",
                             TripleName.c_str());

  switch (OutFileType) {
  case DWARFLinker::OutputFileType::Assembly: {
    std::unique_ptr<MCInstPrinter> MIP(TheTarget->createMCInstPrinter(
        TheTriple, MAI->getAssemblerDialect(), *MAI, *MII, *MRI));
    MS = TheTarget->createAsmStreamer(
        *MC, std::make_unique<formatted_raw_ostream>(OutFile), std::move(MIP),
        std::unique_ptr<MCCodeEmitter>(MCE),
        std::unique_ptr<MCAsmBackend>(MAB));
    break;
  }
  case DWARFLinker::OutputFileType::Object: {
    MS = TheTarget->createMCObjectStreamer(
        TheTriple, *MC, std::unique_ptr<MCAsmBackend>(MAB),
        MAB->createObjectWriter(OutFile), std::unique_ptr<MCCodeEmitter>(MCE),
        *MSTI);
    break;
  }
  }

  if (!MS)
    return createStringError(std::errc::invalid_argument,
                             "no object streamer for target %s",
                             TripleName.c_str());

  // Finally create the AsmPrinter we'll use to emit the DIEs.
  TM.reset(TheTarget->createTargetMachine(TheTriple, "", "", TargetOptions(),
                                          std::nullopt));
  if (!TM)
    return createStringError(std::errc::invalid_argument,
                             "no target machine for target %s",
                             TripleName.c_str());

  Asm.reset(TheTarget->createAsmPrinter(*TM, std::unique_ptr<MCStreamer>(MS)));
  if (!Asm)
    return createStringError(std::errc::invalid_argument,
                             "no asm printer for target %s",
                             TripleName.c_str());
  Asm->setDwarfUsesRelocationsAcrossSections(false);

  DebugInfoSectionSize = 0;

  return Error::success();
}

void DwarfEmitterImpl::emitAbbrevs(
    const SmallVector<std::unique_ptr<DIEAbbrev>> &Abbrevs,
    unsigned DwarfVersion) {
  MS->switchSection(MOFI->getDwarfAbbrevSection());
  MC->setDwarfVersion(DwarfVersion);
  Asm->emitDwarfAbbrevs(Abbrevs);
}

void DwarfEmitterImpl::emitCompileUnitHeader(DwarfUnit &Unit) {
  MS->switchSection(MOFI->getDwarfInfoSection());
  MC->setDwarfVersion(Unit.getVersion());

  // Emit size of content not including length itself. The size has already
  // been computed in CompileUnit::computeOffsets(). Subtract 4 to that size to
  // account for the length field.
  Asm->emitInt32(Unit.getUnitSize() - 4);
  Asm->emitInt16(Unit.getVersion());

  if (Unit.getVersion() >= 5) {
    Asm->emitInt8(dwarf::DW_UT_compile);
    Asm->emitInt8(Unit.getFormParams().AddrSize);
    // Proper offset to the abbreviations table will be set later.
    Asm->emitInt32(0);
    DebugInfoSectionSize += 12;
  } else {
    // Proper offset to the abbreviations table will be set later.
    Asm->emitInt32(0);
    Asm->emitInt8(Unit.getFormParams().AddrSize);
    DebugInfoSectionSize += 11;
  }
}

void DwarfEmitterImpl::emitDIE(DIE &Die) {
  MS->switchSection(MOFI->getDwarfInfoSection());
  Asm->emitDwarfDIE(Die);
  DebugInfoSectionSize += Die.getSize();
}

void DwarfEmitterImpl::emitDebugNames(DWARF5AccelTable &Table,
                                      DebugNamesUnitsOffsets &CUOffsets,
                                      CompUnitIDToIdx &CUidToIdx) {
  if (CUOffsets.empty())
    return;

  Asm->OutStreamer->switchSection(MOFI->getDwarfDebugNamesSection());
  dwarf::Form Form =
      DIEInteger::BestForm(/*IsSigned*/ false, (uint64_t)CUidToIdx.size() - 1);
  // FIXME: add support for type units + .debug_names. For now the behavior is
  // unsuported.
  emitDWARF5AccelTable(
      Asm.get(), Table, CUOffsets,
      [&](const DWARF5AccelTableData &Entry)
          -> std::optional<DWARF5AccelTable::UnitIndexAndEncoding> {
        if (CUidToIdx.size() > 1)
          return {{CUidToIdx[Entry.getUnitID()],
                   {dwarf::DW_IDX_compile_unit, Form}}};
        return std::nullopt;
      });
}

void DwarfEmitterImpl::emitAppleNamespaces(
    AccelTable<AppleAccelTableStaticOffsetData> &Table) {
  Asm->OutStreamer->switchSection(MOFI->getDwarfAccelNamespaceSection());
  auto *SectionBegin = Asm->createTempSymbol("namespac_begin");
  Asm->OutStreamer->emitLabel(SectionBegin);
  emitAppleAccelTable(Asm.get(), Table, "namespac", SectionBegin);
}

void DwarfEmitterImpl::emitAppleNames(
    AccelTable<AppleAccelTableStaticOffsetData> &Table) {
  Asm->OutStreamer->switchSection(MOFI->getDwarfAccelNamesSection());
  auto *SectionBegin = Asm->createTempSymbol("names_begin");
  Asm->OutStreamer->emitLabel(SectionBegin);
  emitAppleAccelTable(Asm.get(), Table, "names", SectionBegin);
}

void DwarfEmitterImpl::emitAppleObjc(
    AccelTable<AppleAccelTableStaticOffsetData> &Table) {
  Asm->OutStreamer->switchSection(MOFI->getDwarfAccelObjCSection());
  auto *SectionBegin = Asm->createTempSymbol("objc_begin");
  Asm->OutStreamer->emitLabel(SectionBegin);
  emitAppleAccelTable(Asm.get(), Table, "objc", SectionBegin);
}

void DwarfEmitterImpl::emitAppleTypes(
    AccelTable<AppleAccelTableStaticTypeData> &Table) {
  Asm->OutStreamer->switchSection(MOFI->getDwarfAccelTypesSection());
  auto *SectionBegin = Asm->createTempSymbol("types_begin");
  Asm->OutStreamer->emitLabel(SectionBegin);
  emitAppleAccelTable(Asm.get(), Table, "types", SectionBegin);
}
