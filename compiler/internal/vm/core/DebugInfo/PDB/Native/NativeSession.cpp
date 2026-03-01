//===- NativeSession.cpp - Native implementation of IPDBSession -*- C++ -*-===//
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

#include "vm/core/DebugInfo/PDB/Native/NativeSession.h"

#include "vm/core/ADT/SmallString.h"
#include "vm/core/BinaryFormat/Magic.h"
#include "vm/core/DebugInfo/MSF/MappedBlockStream.h"
#include "vm/core/DebugInfo/PDB/IPDBEnumChildren.h"
#include "vm/core/DebugInfo/PDB/IPDBSourceFile.h"
#include "vm/core/DebugInfo/PDB/Native/DbiModuleDescriptor.h"
#include "vm/core/DebugInfo/PDB/Native/DbiModuleList.h"
#include "vm/core/DebugInfo/PDB/Native/DbiStream.h"
#include "vm/core/DebugInfo/PDB/Native/ISectionContribVisitor.h"
#include "vm/core/DebugInfo/PDB/Native/ModuleDebugStream.h"
#include "vm/core/DebugInfo/PDB/Native/NativeEnumInjectedSources.h"
#include "vm/core/DebugInfo/PDB/Native/NativeExeSymbol.h"
#include "vm/core/DebugInfo/PDB/Native/PDBFile.h"
#include "vm/core/DebugInfo/PDB/Native/RawConstants.h"
#include "vm/core/DebugInfo/PDB/Native/RawError.h"
#include "vm/core/DebugInfo/PDB/Native/RawTypes.h"
#include "vm/core/DebugInfo/PDB/Native/SymbolCache.h"
#include "vm/core/DebugInfo/PDB/PDBSymbol.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolCompiland.h"
#include "vm/core/DebugInfo/PDB/PDBSymbolExe.h"
#include "vm/core/Object/Binary.h"
#include "vm/core/Object/COFF.h"
#include "vm/core/Support/Allocator.h"
#include "vm/core/Support/BinaryByteStream.h"
#include "vm/core/Support/BinaryStreamArray.h"
#include "vm/core/Support/Error.h"
#include "vm/core/Support/ErrorOr.h"
#include "vm/core/Support/MemoryBuffer.h"
#include "vm/core/Support/Path.h"

#include <cassert>
#include <memory>
#include <utility>

using namespace vm::core;
using namespace vm::core::msf;
using namespace vm::core::pdb;

namespace vm::core {
namespace codeview {
union DebugInfo;
}
} // namespace vm::core

static DbiStream *getDbiStreamPtr(PDBFile &File) {
  Expected<DbiStream &> DbiS = File.getPDBDbiStream();
  if (DbiS)
    return &DbiS.get();

  consumeError(DbiS.takeError());
  return nullptr;
}

NativeSession::NativeSession(std::unique_ptr<PDBFile> PdbFile,
                             std::unique_ptr<BumpPtrAllocator> Allocator)
    : Pdb(std::move(PdbFile)), Allocator(std::move(Allocator)),
      Cache(*this, getDbiStreamPtr(*Pdb)), AddrToModuleIndex(IMapAllocator) {}

NativeSession::~NativeSession() = default;

Error NativeSession::createFromPdb(std::unique_ptr<MemoryBuffer> Buffer,
                                   std::unique_ptr<IPDBSession> &Session) {
  StringRef Path = Buffer->getBufferIdentifier();
  auto Stream = std::make_unique<MemoryBufferByteStream>(
      std::move(Buffer), toolchain::endianness::little);

  auto Allocator = std::make_unique<BumpPtrAllocator>();
  auto File = std::make_unique<PDBFile>(Path, std::move(Stream), *Allocator);
  if (auto EC = File->parseFileHeaders())
    return EC;
  if (auto EC = File->parseStreamData())
    return EC;

  Session =
      std::make_unique<NativeSession>(std::move(File), std::move(Allocator));

  return Error::success();
}

static Error validatePdbMagic(StringRef PdbPath) {
  file_magic Magic;
  if (auto EC = identify_magic(PdbPath, Magic))
    return make_error<RawError>(EC);

  if (Magic != file_magic::pdb)
    return make_error<RawError>(
        raw_error_code::invalid_format,
        "The input file did not contain the pdb file magic.");

  return Error::success();
}

static Expected<std::unique_ptr<PDBFile>>
loadPdbFile(StringRef PdbPath, std::unique_ptr<BumpPtrAllocator> &Allocator) {
  ErrorOr<std::unique_ptr<MemoryBuffer>> ErrorOrBuffer =
      MemoryBuffer::getFile(PdbPath, /*IsText=*/false,
                            /*RequiresNullTerminator=*/false);
  if (!ErrorOrBuffer)
    return make_error<RawError>(ErrorOrBuffer.getError());
  std::unique_ptr<toolchain::MemoryBuffer> Buffer = std::move(*ErrorOrBuffer);

  PdbPath = Buffer->getBufferIdentifier();
  if (auto EC = validatePdbMagic(PdbPath))
    return std::move(EC);

  auto Stream = std::make_unique<MemoryBufferByteStream>(
      std::move(Buffer), toolchain::endianness::little);

  auto File = std::make_unique<PDBFile>(PdbPath, std::move(Stream), *Allocator);
  if (auto EC = File->parseFileHeaders())
    return std::move(EC);

  if (auto EC = File->parseStreamData())
    return std::move(EC);

  return std::move(File);
}

Error NativeSession::createFromPdbPath(StringRef PdbPath,
                                       std::unique_ptr<IPDBSession> &Session) {
  auto Allocator = std::make_unique<BumpPtrAllocator>();
  auto PdbFile = loadPdbFile(PdbPath, Allocator);
  if (!PdbFile)
    return PdbFile.takeError();

  Session = std::make_unique<NativeSession>(std::move(PdbFile.get()),
                                            std::move(Allocator));
  return Error::success();
}

static Expected<std::string> getPdbPathFromExe(StringRef ExePath) {
  Expected<object::OwningBinary<object::Binary>> BinaryFile =
      object::createBinary(ExePath);
  if (!BinaryFile)
    return BinaryFile.takeError();

  const object::COFFObjectFile *ObjFile =
      dyn_cast<object::COFFObjectFile>(BinaryFile->getBinary());
  if (!ObjFile)
    return make_error<RawError>(raw_error_code::invalid_format);

  StringRef PdbPath;
  const toolchain::codeview::DebugInfo *PdbInfo = nullptr;
  if (Error E = ObjFile->getDebugPDBInfo(PdbInfo, PdbPath))
    return std::move(E);

  return std::string(PdbPath);
}

Error NativeSession::createFromExe(StringRef ExePath,
                                   std::unique_ptr<IPDBSession> &Session) {
  Expected<std::string> PdbPath = getPdbPathFromExe(ExePath);
  if (!PdbPath)
    return PdbPath.takeError();

  if (auto EC = validatePdbMagic(PdbPath.get()))
    return EC;

  auto Allocator = std::make_unique<BumpPtrAllocator>();
  auto File = loadPdbFile(PdbPath.get(), Allocator);
  if (!File)
    return File.takeError();

  Session = std::make_unique<NativeSession>(std::move(File.get()),
                                            std::move(Allocator));

  return Error::success();
}

Expected<std::string>
NativeSession::searchForPdb(const PdbSearchOptions &Opts) {
  Expected<std::string> PathOrErr = getPdbPathFromExe(Opts.ExePath);
  if (!PathOrErr)
    return PathOrErr.takeError();
  StringRef PathFromExe = PathOrErr.get();
  sys::path::Style Style = PathFromExe.starts_with("/")
                               ? sys::path::Style::posix
                               : sys::path::Style::windows;
  StringRef PdbName = sys::path::filename(PathFromExe, Style);

  // Check if pdb exists in the executable directory.
  SmallString<128> PdbPath = StringRef(Opts.ExePath);
  sys::path::remove_filename(PdbPath);
  sys::path::append(PdbPath, PdbName);

  auto Allocator = std::make_unique<BumpPtrAllocator>();

  if (auto File = loadPdbFile(PdbPath, Allocator))
    return std::string(PdbPath);
  else
    consumeError(File.takeError());

  // Check path that was in the executable.
  if (auto File = loadPdbFile(PathFromExe, Allocator))
    return std::string(PathFromExe);
  else
    return File.takeError();

  return make_error<RawError>("PDB not found");
}

uint64_t NativeSession::getLoadAddress() const { return LoadAddress; }

bool NativeSession::setLoadAddress(uint64_t Address) {
  LoadAddress = Address;
  return true;
}

std::unique_ptr<PDBSymbolExe> NativeSession::getGlobalScope() {
  return PDBSymbol::createAs<PDBSymbolExe>(*this, getNativeGlobalScope());
}

std::unique_ptr<PDBSymbol>
NativeSession::getSymbolById(SymIndexId SymbolId) const {
  return Cache.getSymbolById(SymbolId);
}

bool NativeSession::addressForVA(uint64_t VA, uint32_t &Section,
                                 uint32_t &Offset) const {
  uint32_t RVA = VA - getLoadAddress();
  return addressForRVA(RVA, Section, Offset);
}

bool NativeSession::addressForRVA(uint32_t RVA, uint32_t &Section,
                                  uint32_t &Offset) const {
  Section = 0;
  Offset = 0;

  auto Dbi = Pdb->getPDBDbiStream();
  if (!Dbi)
    return false;

  if ((int32_t)RVA < 0)
    return true;

  Offset = RVA;
  for (; Section < Dbi->getSectionHeaders().size(); ++Section) {
    auto &Sec = Dbi->getSectionHeaders()[Section];
    if (RVA < Sec.VirtualAddress)
      return true;
    Offset = RVA - Sec.VirtualAddress;
  }
  return true;
}

std::unique_ptr<PDBSymbol>
NativeSession::findSymbolByAddress(uint64_t Address, PDB_SymType Type) {
  uint32_t Section;
  uint32_t Offset;
  addressForVA(Address, Section, Offset);
  return findSymbolBySectOffset(Section, Offset, Type);
}

std::unique_ptr<PDBSymbol> NativeSession::findSymbolByRVA(uint32_t RVA,
                                                          PDB_SymType Type) {
  uint32_t Section;
  uint32_t Offset;
  addressForRVA(RVA, Section, Offset);
  return findSymbolBySectOffset(Section, Offset, Type);
}

std::unique_ptr<PDBSymbol>
NativeSession::findSymbolBySectOffset(uint32_t Sect, uint32_t Offset,
                                      PDB_SymType Type) {
  if (AddrToModuleIndex.empty())
    parseSectionContribs();

  return Cache.findSymbolBySectOffset(Sect, Offset, Type);
}

std::unique_ptr<IPDBEnumLineNumbers>
NativeSession::findLineNumbers(const PDBSymbolCompiland &Compiland,
                               const IPDBSourceFile &File) const {
  return nullptr;
}

std::unique_ptr<IPDBEnumLineNumbers>
NativeSession::findLineNumbersByAddress(uint64_t Address,
                                        uint32_t Length) const {
  return Cache.findLineNumbersByVA(Address, Length);
}

std::unique_ptr<IPDBEnumLineNumbers>
NativeSession::findLineNumbersByRVA(uint32_t RVA, uint32_t Length) const {
  return Cache.findLineNumbersByVA(getLoadAddress() + RVA, Length);
}

std::unique_ptr<IPDBEnumLineNumbers>
NativeSession::findLineNumbersBySectOffset(uint32_t Section, uint32_t Offset,
                                           uint32_t Length) const {
  uint64_t VA = getVAFromSectOffset(Section, Offset);
  return Cache.findLineNumbersByVA(VA, Length);
}

std::unique_ptr<IPDBEnumSourceFiles>
NativeSession::findSourceFiles(const PDBSymbolCompiland *Compiland,
                               StringRef Pattern,
                               PDB_NameSearchFlags Flags) const {
  return nullptr;
}

std::unique_ptr<IPDBSourceFile>
NativeSession::findOneSourceFile(const PDBSymbolCompiland *Compiland,
                                 StringRef Pattern,
                                 PDB_NameSearchFlags Flags) const {
  return nullptr;
}

std::unique_ptr<IPDBEnumChildren<PDBSymbolCompiland>>
NativeSession::findCompilandsForSourceFile(StringRef Pattern,
                                           PDB_NameSearchFlags Flags) const {
  return nullptr;
}

std::unique_ptr<PDBSymbolCompiland>
NativeSession::findOneCompilandForSourceFile(StringRef Pattern,
                                             PDB_NameSearchFlags Flags) const {
  return nullptr;
}

std::unique_ptr<IPDBEnumSourceFiles> NativeSession::getAllSourceFiles() const {
  return nullptr;
}

std::unique_ptr<IPDBEnumSourceFiles> NativeSession::getSourceFilesForCompiland(
    const PDBSymbolCompiland &Compiland) const {
  return nullptr;
}

std::unique_ptr<IPDBSourceFile>
NativeSession::getSourceFileById(uint32_t FileId) const {
  return Cache.getSourceFileById(FileId);
}

std::unique_ptr<IPDBEnumDataStreams> NativeSession::getDebugStreams() const {
  return nullptr;
}

std::unique_ptr<IPDBEnumTables> NativeSession::getEnumTables() const {
  return nullptr;
}

std::unique_ptr<IPDBEnumInjectedSources>
NativeSession::getInjectedSources() const {
  auto ISS = Pdb->getInjectedSourceStream();
  if (!ISS) {
    consumeError(ISS.takeError());
    return nullptr;
  }
  auto Strings = Pdb->getStringTable();
  if (!Strings) {
    consumeError(Strings.takeError());
    return nullptr;
  }
  return std::make_unique<NativeEnumInjectedSources>(*Pdb, *ISS, *Strings);
}

std::unique_ptr<IPDBEnumSectionContribs>
NativeSession::getSectionContribs() const {
  return nullptr;
}

std::unique_ptr<IPDBEnumFrameData>
NativeSession::getFrameData() const {
  return nullptr;
}

void NativeSession::initializeExeSymbol() {
  if (ExeSymbol == 0)
    ExeSymbol = Cache.createSymbol<NativeExeSymbol>();
}

NativeExeSymbol &NativeSession::getNativeGlobalScope() const {
  const_cast<NativeSession &>(*this).initializeExeSymbol();

  return Cache.getNativeSymbolById<NativeExeSymbol>(ExeSymbol);
}

uint32_t NativeSession::getRVAFromSectOffset(uint32_t Section,
                                             uint32_t Offset) const {
  if (Section <= 0)
    return 0;

  auto Dbi = getDbiStreamPtr(*Pdb);
  if (!Dbi)
    return 0;

  uint32_t MaxSection = Dbi->getSectionHeaders().size();
  if (Section > MaxSection + 1)
    Section = MaxSection + 1;
  auto &Sec = Dbi->getSectionHeaders()[Section - 1];
  return Sec.VirtualAddress + Offset;
}

uint64_t NativeSession::getVAFromSectOffset(uint32_t Section,
                                            uint32_t Offset) const {
  return LoadAddress + getRVAFromSectOffset(Section, Offset);
}

bool NativeSession::moduleIndexForVA(uint64_t VA, uint16_t &ModuleIndex) const {
  ModuleIndex = 0;
  auto Iter = AddrToModuleIndex.find(VA);
  if (Iter == AddrToModuleIndex.end())
    return false;
  ModuleIndex = Iter.value();
  return true;
}

bool NativeSession::moduleIndexForSectOffset(uint32_t Sect, uint32_t Offset,
                                             uint16_t &ModuleIndex) const {
  ModuleIndex = 0;
  auto Iter = AddrToModuleIndex.find(getVAFromSectOffset(Sect, Offset));
  if (Iter == AddrToModuleIndex.end())
    return false;
  ModuleIndex = Iter.value();
  return true;
}

void NativeSession::parseSectionContribs() {
  auto Dbi = Pdb->getPDBDbiStream();
  if (!Dbi)
    return;

  class Visitor : public ISectionContribVisitor {
    NativeSession &Session;
    IMap &AddrMap;

  public:
    Visitor(NativeSession &Session, IMap &AddrMap)
        : Session(Session), AddrMap(AddrMap) {}
    void visit(const SectionContrib &C) override {
      if (C.Size == 0)
        return;

      uint64_t VA = Session.getVAFromSectOffset(C.ISect, C.Off);
      uint64_t End = VA + C.Size;

      // Ignore overlapping sections based on the assumption that a valid
      // PDB file should not have overlaps.
      if (!AddrMap.overlaps(VA, End))
        AddrMap.insert(VA, End, C.Imod);
    }
    void visit(const SectionContrib2 &C) override { visit(C.Base); }
  };

  Visitor V(*this, AddrToModuleIndex);
  Dbi->visitSectionContributions(V);
}

Expected<ModuleDebugStreamRef>
NativeSession::getModuleDebugStream(uint32_t Index) const {
  auto *Dbi = getDbiStreamPtr(*Pdb);
  assert(Dbi && "Dbi stream not present");

  DbiModuleDescriptor Modi = Dbi->modules().getModuleDescriptor(Index);

  uint16_t ModiStream = Modi.getModuleStreamIndex();
  if (ModiStream == kInvalidStreamIndex)
    return make_error<RawError>("Module stream not present");

  std::unique_ptr<msf::MappedBlockStream> ModStreamData =
      Pdb->createIndexedStream(ModiStream);

  ModuleDebugStreamRef ModS(Modi, std::move(ModStreamData));
  if (auto EC = ModS.reload())
    return std::move(EC);

  return std::move(ModS);
}
