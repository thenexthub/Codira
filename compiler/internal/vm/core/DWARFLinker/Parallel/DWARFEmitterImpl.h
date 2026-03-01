//===- DwarfEmitterImpl.h ---------------------------------------*- C++ -*-===//
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

#ifndef LLVM_LIB_DWARFLINKER_PARALLEL_DWARFEMITTERIMPL_H
#define LLVM_LIB_DWARFLINKER_PARALLEL_DWARFEMITTERIMPL_H

#include "DWARFLinkerCompileUnit.h"
#include "vm/core/BinaryFormat/Swift.h"
#include "vm/core/CodeGen/AccelTable.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/DWARFLinker/Parallel/DWARFLinker.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCInstPrinter.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCObjectFileInfo.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/Target/TargetMachine.h"

namespace vm::core {

///   User of DwarfEmitterImpl should call initialization code
///   for AsmPrinter:
///
///   InitializeAllTargetInfos();
///   InitializeAllTargetMCs();
///   InitializeAllTargets();
///   InitializeAllAsmPrinters();

template <typename DataT> class AccelTable;
class MCCodeEmitter;

namespace dwarf_linker {
namespace parallel {

using DebugNamesUnitsOffsets = std::vector<std::variant<MCSymbol *, uint64_t>>;
using CompUnitIDToIdx = DenseMap<unsigned, unsigned>;

/// This class emits DWARF data to the output stream. It emits already
/// generated section data and specific data, which could not be generated
/// by CompileUnit.
class DwarfEmitterImpl {
public:
  DwarfEmitterImpl(DWARFLinker::OutputFileType OutFileType,
                   raw_pwrite_stream &OutFile)
      : OutFile(OutFile), OutFileType(OutFileType) {}

  /// Initialize AsmPrinter data.
  Error init(Triple TheTriple, StringRef Swift5ReflectionSegmentName);

  /// Returns triple of output stream.
  const Triple &getTargetTriple() { return MC->getTargetTriple(); }

  /// Dump the file to the disk.
  void finish() { MS->finish(); }

  /// Emit abbreviations.
  void emitAbbrevs(const SmallVector<std::unique_ptr<DIEAbbrev>> &Abbrevs,
                   unsigned DwarfVersion);

  /// Emit compile unit header.
  void emitCompileUnitHeader(DwarfUnit &Unit);

  /// Emit DIE recursively.
  void emitDIE(DIE &Die);

  /// Returns size of generated .debug_info section.
  uint64_t getDebugInfoSectionSize() const { return DebugInfoSectionSize; }

  /// Emits .debug_names section according to the specified \p Table.
  void emitDebugNames(DWARF5AccelTable &Table,
                      DebugNamesUnitsOffsets &CUOffsets,
                      CompUnitIDToIdx &UnitIDToIdxMap);

  /// Emits .apple_names section according to the specified \p Table.
  void emitAppleNames(AccelTable<AppleAccelTableStaticOffsetData> &Table);

  /// Emits .apple_namespaces section according to the specified \p Table.
  void emitAppleNamespaces(AccelTable<AppleAccelTableStaticOffsetData> &Table);

  /// Emits .apple_objc section according to the specified \p Table.
  void emitAppleObjc(AccelTable<AppleAccelTableStaticOffsetData> &Table);

  /// Emits .apple_types section according to the specified \p Table.
  void emitAppleTypes(AccelTable<AppleAccelTableStaticTypeData> &Table);

private:
  // Enumerate all string patches and write them into the destination section.
  // Order of patches is the same as in original input file. To avoid emitting
  // the same string twice we accumulate NextOffset value. Thus if string
  // offset smaller than NextOffset value then the patch is skipped (as that
  // string was emitted earlier).
  template <typename PatchTy>
  void emitStringsImpl(ArrayList<PatchTy> &StringPatches,
                       const StringEntryToDwarfStringPoolEntryMap &Strings,
                       uint64_t &NextOffset, MCSection *OutSection);

  /// \defgroup MCObjects MC layer objects constructed by the streamer
  /// @{
  std::unique_ptr<MCRegisterInfo> MRI;
  std::unique_ptr<MCAsmInfo> MAI;
  std::unique_ptr<MCObjectFileInfo> MOFI;
  std::unique_ptr<MCContext> MC;
  MCAsmBackend *MAB; // Owned by MCStreamer
  std::unique_ptr<MCInstrInfo> MII;
  std::unique_ptr<MCSubtargetInfo> MSTI;
  std::unique_ptr<MCInstPrinter> MIP; // Owned by AsmPrinter
  MCCodeEmitter *MCE; // Owned by MCStreamer
  MCStreamer *MS;     // Owned by AsmPrinter
  std::unique_ptr<TargetMachine> TM;
  std::unique_ptr<AsmPrinter> Asm;
  /// @}

  /// The output file we stream the linked Dwarf to.
  raw_pwrite_stream &OutFile;
  DWARFLinkerBase::OutputFileType OutFileType =
      DWARFLinkerBase::OutputFileType::Object;

  uint64_t DebugInfoSectionSize = 0;
};

} // end of namespace parallel
} // end of namespace dwarf_linker
} // end of namespace vm::core

#endif // LLVM_LIB_DWARFLINKER_PARALLEL_DWARFEMITTERIMPL_H
