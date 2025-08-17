//===--- GlobalModuleIndex.cpp - Global Module Index ------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
// This file implements the GlobalModuleIndex class.
//
//===----------------------------------------------------------------------===//

#include "language/Core/Serialization/GlobalModuleIndex.h"
#include "ASTReaderInternals.h"
#include "language/Core/Basic/FileManager.h"
#include "language/Core/Serialization/ASTBitCodes.h"
#include "language/Core/Serialization/ModuleFile.h"
#include "language/Core/Serialization/PCHContainerOperations.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/MapVector.h"
#include "toolchain/ADT/SmallString.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/Bitstream/BitstreamReader.h"
#include "toolchain/Bitstream/BitstreamWriter.h"
#include "toolchain/Support/DJB.h"
#include "toolchain/Support/FileSystem.h"
#include "toolchain/Support/LockFileManager.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/OnDiskHashTable.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/TimeProfiler.h"
#include "toolchain/Support/raw_ostream.h"
#include <cstdio>
using namespace language::Core;
using namespace serialization;

//----------------------------------------------------------------------------//
// Shared constants
//----------------------------------------------------------------------------//
namespace {
  enum {
    /// The block containing the index.
    GLOBAL_INDEX_BLOCK_ID = toolchain::bitc::FIRST_APPLICATION_BLOCKID
  };

  /// Describes the record types in the index.
  enum IndexRecordTypes {
    /// Contains version information and potentially other metadata,
    /// used to determine if we can read this global index file.
    INDEX_METADATA,
    /// Describes a module, including its file name and dependencies.
    MODULE,
    /// The index for identifiers.
    IDENTIFIER_INDEX
  };
}

/// The name of the global index file.
static const char * const IndexFileName = "modules.idx";

/// The global index file version.
static const unsigned CurrentVersion = 1;

//----------------------------------------------------------------------------//
// Global module index reader.
//----------------------------------------------------------------------------//

namespace {

/// Trait used to read the identifier index from the on-disk hash
/// table.
class IdentifierIndexReaderTrait {
public:
  typedef StringRef external_key_type;
  typedef StringRef internal_key_type;
  typedef SmallVector<unsigned, 2> data_type;
  typedef unsigned hash_value_type;
  typedef unsigned offset_type;

  static bool EqualKey(const internal_key_type& a, const internal_key_type& b) {
    return a == b;
  }

  static hash_value_type ComputeHash(const internal_key_type& a) {
    return toolchain::djbHash(a);
  }

  static std::pair<unsigned, unsigned>
  ReadKeyDataLength(const unsigned char*& d) {
    using namespace toolchain::support;
    unsigned KeyLen = endian::readNext<uint16_t, toolchain::endianness::little>(d);
    unsigned DataLen = endian::readNext<uint16_t, toolchain::endianness::little>(d);
    return std::make_pair(KeyLen, DataLen);
  }

  static const internal_key_type&
  GetInternalKey(const external_key_type& x) { return x; }

  static const external_key_type&
  GetExternalKey(const internal_key_type& x) { return x; }

  static internal_key_type ReadKey(const unsigned char* d, unsigned n) {
    return StringRef((const char *)d, n);
  }

  static data_type ReadData(const internal_key_type& k,
                            const unsigned char* d,
                            unsigned DataLen) {
    using namespace toolchain::support;

    data_type Result;
    while (DataLen > 0) {
      unsigned ID = endian::readNext<uint32_t, toolchain::endianness::little>(d);
      Result.push_back(ID);
      DataLen -= 4;
    }

    return Result;
  }
};

typedef toolchain::OnDiskIterableChainedHashTable<IdentifierIndexReaderTrait>
    IdentifierIndexTable;

}

GlobalModuleIndex::GlobalModuleIndex(
    std::unique_ptr<toolchain::MemoryBuffer> IndexBuffer,
    toolchain::BitstreamCursor Cursor)
    : Buffer(std::move(IndexBuffer)), IdentifierIndex(), NumIdentifierLookups(),
      NumIdentifierLookupHits() {
  auto Fail = [&](toolchain::Error &&Err) {
    report_fatal_error("Module index '" + Buffer->getBufferIdentifier() +
                       "' failed: " + toString(std::move(Err)));
  };

  toolchain::TimeTraceScope TimeScope("Module LoadIndex");
  // Read the global index.
  bool InGlobalIndexBlock = false;
  bool Done = false;
  while (!Done) {
    toolchain::BitstreamEntry Entry;
    if (Expected<toolchain::BitstreamEntry> Res = Cursor.advance())
      Entry = Res.get();
    else
      Fail(Res.takeError());

    switch (Entry.Kind) {
    case toolchain::BitstreamEntry::Error:
      return;

    case toolchain::BitstreamEntry::EndBlock:
      if (InGlobalIndexBlock) {
        InGlobalIndexBlock = false;
        Done = true;
        continue;
      }
      return;


    case toolchain::BitstreamEntry::Record:
      // Entries in the global index block are handled below.
      if (InGlobalIndexBlock)
        break;

      return;

    case toolchain::BitstreamEntry::SubBlock:
      if (!InGlobalIndexBlock && Entry.ID == GLOBAL_INDEX_BLOCK_ID) {
        if (toolchain::Error Err = Cursor.EnterSubBlock(GLOBAL_INDEX_BLOCK_ID))
          Fail(std::move(Err));
        InGlobalIndexBlock = true;
      } else if (toolchain::Error Err = Cursor.SkipBlock())
        Fail(std::move(Err));
      continue;
    }

    SmallVector<uint64_t, 64> Record;
    StringRef Blob;
    Expected<unsigned> MaybeIndexRecord =
        Cursor.readRecord(Entry.ID, Record, &Blob);
    if (!MaybeIndexRecord)
      Fail(MaybeIndexRecord.takeError());
    IndexRecordTypes IndexRecord =
        static_cast<IndexRecordTypes>(MaybeIndexRecord.get());
    switch (IndexRecord) {
    case INDEX_METADATA:
      // Make sure that the version matches.
      if (Record.size() < 1 || Record[0] != CurrentVersion)
        return;
      break;

    case MODULE: {
      unsigned Idx = 0;
      unsigned ID = Record[Idx++];

      // Make room for this module's information.
      if (ID == Modules.size())
        Modules.push_back(ModuleInfo());
      else
        Modules.resize(ID + 1);

      // Size/modification time for this module file at the time the
      // global index was built.
      Modules[ID].Size = Record[Idx++];
      Modules[ID].ModTime = Record[Idx++];

      // File name.
      unsigned NameLen = Record[Idx++];
      Modules[ID].FileName.assign(Record.begin() + Idx,
                                  Record.begin() + Idx + NameLen);
      Idx += NameLen;

      // Dependencies
      unsigned NumDeps = Record[Idx++];
      Modules[ID].Dependencies.insert(Modules[ID].Dependencies.end(),
                                      Record.begin() + Idx,
                                      Record.begin() + Idx + NumDeps);
      Idx += NumDeps;

      // Make sure we're at the end of the record.
      assert(Idx == Record.size() && "More module info?");

      // Record this module as an unresolved module.
      // FIXME: this doesn't work correctly for module names containing path
      // separators.
      StringRef ModuleName = toolchain::sys::path::stem(Modules[ID].FileName);
      // Remove the -<hash of ModuleMapPath>
      ModuleName = ModuleName.rsplit('-').first;
      UnresolvedModules[ModuleName] = ID;
      break;
    }

    case IDENTIFIER_INDEX:
      // Wire up the identifier index.
      if (Record[0]) {
        IdentifierIndex = IdentifierIndexTable::Create(
            (const unsigned char *)Blob.data() + Record[0],
            (const unsigned char *)Blob.data() + sizeof(uint32_t),
            (const unsigned char *)Blob.data(), IdentifierIndexReaderTrait());
      }
      break;
    }
  }
}

GlobalModuleIndex::~GlobalModuleIndex() {
  delete static_cast<IdentifierIndexTable *>(IdentifierIndex);
}

std::pair<GlobalModuleIndex *, toolchain::Error>
GlobalModuleIndex::readIndex(StringRef Path) {
  // Load the index file, if it's there.
  toolchain::SmallString<128> IndexPath;
  IndexPath += Path;
  toolchain::sys::path::append(IndexPath, IndexFileName);

  toolchain::ErrorOr<std::unique_ptr<toolchain::MemoryBuffer>> BufferOrErr =
      toolchain::MemoryBuffer::getFile(IndexPath.c_str());
  if (!BufferOrErr)
    return std::make_pair(nullptr,
                          toolchain::errorCodeToError(BufferOrErr.getError()));
  std::unique_ptr<toolchain::MemoryBuffer> Buffer = std::move(BufferOrErr.get());

  /// The main bitstream cursor for the main block.
  toolchain::BitstreamCursor Cursor(*Buffer);

  // Sniff for the signature.
  for (unsigned char C : {'B', 'C', 'G', 'I'}) {
    if (Expected<toolchain::SimpleBitstreamCursor::word_t> Res = Cursor.Read(8)) {
      if (Res.get() != C)
        return std::make_pair(
            nullptr, toolchain::createStringError(std::errc::illegal_byte_sequence,
                                             "expected signature BCGI"));
    } else
      return std::make_pair(nullptr, Res.takeError());
  }

  return std::make_pair(new GlobalModuleIndex(std::move(Buffer), std::move(Cursor)),
                        toolchain::Error::success());
}

void GlobalModuleIndex::getModuleDependencies(
       ModuleFile *File,
       SmallVectorImpl<ModuleFile *> &Dependencies) {
  // Look for information about this module file.
  toolchain::DenseMap<ModuleFile *, unsigned>::iterator Known
    = ModulesByFile.find(File);
  if (Known == ModulesByFile.end())
    return;

  // Record dependencies.
  Dependencies.clear();
  ArrayRef<unsigned> StoredDependencies = Modules[Known->second].Dependencies;
  for (unsigned I = 0, N = StoredDependencies.size(); I != N; ++I) {
    if (ModuleFile *MF = Modules[I].File)
      Dependencies.push_back(MF);
  }
}

bool GlobalModuleIndex::lookupIdentifier(StringRef Name, HitSet &Hits) {
  Hits.clear();

  // If there's no identifier index, there is nothing we can do.
  if (!IdentifierIndex)
    return false;

  // Look into the identifier index.
  ++NumIdentifierLookups;
  IdentifierIndexTable &Table
    = *static_cast<IdentifierIndexTable *>(IdentifierIndex);
  IdentifierIndexTable::iterator Known = Table.find(Name);
  if (Known == Table.end()) {
    return false;
  }

  SmallVector<unsigned, 2> ModuleIDs = *Known;
  for (unsigned I = 0, N = ModuleIDs.size(); I != N; ++I) {
    if (ModuleFile *MF = Modules[ModuleIDs[I]].File)
      Hits.insert(MF);
  }

  ++NumIdentifierLookupHits;
  return true;
}

bool GlobalModuleIndex::loadedModuleFile(ModuleFile *File) {
  // Look for the module in the global module index based on the module name.
  StringRef Name = File->ModuleName;
  toolchain::StringMap<unsigned>::iterator Known = UnresolvedModules.find(Name);
  if (Known == UnresolvedModules.end()) {
    return true;
  }

  // Rectify this module with the global module index.
  ModuleInfo &Info = Modules[Known->second];

  //  If the size and modification time match what we expected, record this
  // module file.
  bool Failed = true;
  if (File->File.getSize() == Info.Size &&
      File->File.getModificationTime() == Info.ModTime) {
    Info.File = File;
    ModulesByFile[File] = Known->second;

    Failed = false;
  }

  // One way or another, we have resolved this module file.
  UnresolvedModules.erase(Known);
  return Failed;
}

void GlobalModuleIndex::printStats() {
  std::fprintf(stderr, "*** Global Module Index Statistics:\n");
  if (NumIdentifierLookups) {
    fprintf(stderr, "  %u / %u identifier lookups succeeded (%f%%)\n",
            NumIdentifierLookupHits, NumIdentifierLookups,
            (double)NumIdentifierLookupHits*100.0/NumIdentifierLookups);
  }
  std::fprintf(stderr, "\n");
}

LLVM_DUMP_METHOD void GlobalModuleIndex::dump() {
  toolchain::errs() << "*** Global Module Index Dump:\n";
  toolchain::errs() << "Module files:\n";
  for (auto &MI : Modules) {
    toolchain::errs() << "** " << MI.FileName << "\n";
    if (MI.File)
      MI.File->dump();
    else
      toolchain::errs() << "\n";
  }
  toolchain::errs() << "\n";
}

//----------------------------------------------------------------------------//
// Global module index writer.
//----------------------------------------------------------------------------//

namespace {
  /// Provides information about a specific module file.
  struct ModuleFileInfo {
    /// The numberic ID for this module file.
    unsigned ID;

    /// The set of modules on which this module depends. Each entry is
    /// a module ID.
    SmallVector<unsigned, 4> Dependencies;
    ASTFileSignature Signature;
  };

  struct ImportedModuleFileInfo {
    off_t StoredSize;
    time_t StoredModTime;
    ASTFileSignature StoredSignature;
    ImportedModuleFileInfo(off_t Size, time_t ModTime, ASTFileSignature Sig)
        : StoredSize(Size), StoredModTime(ModTime), StoredSignature(Sig) {}
  };

  /// Builder that generates the global module index file.
  class GlobalModuleIndexBuilder {
    FileManager &FileMgr;
    const PCHContainerReader &PCHContainerRdr;

    /// Mapping from files to module file information.
    using ModuleFilesMap = toolchain::MapVector<FileEntryRef, ModuleFileInfo>;

    /// Information about each of the known module files.
    ModuleFilesMap ModuleFiles;

    /// Mapping from the imported module file to the imported
    /// information.
    using ImportedModuleFilesMap =
        std::multimap<FileEntryRef, ImportedModuleFileInfo>;

    /// Information about each importing of a module file.
    ImportedModuleFilesMap ImportedModuleFiles;

    /// Mapping from identifiers to the list of module file IDs that
    /// consider this identifier to be interesting.
    typedef toolchain::StringMap<SmallVector<unsigned, 2> > InterestingIdentifierMap;

    /// A mapping from all interesting identifiers to the set of module
    /// files in which those identifiers are considered interesting.
    InterestingIdentifierMap InterestingIdentifiers;

    /// Write the block-info block for the global module index file.
    void emitBlockInfoBlock(toolchain::BitstreamWriter &Stream);

    /// Retrieve the module file information for the given file.
    ModuleFileInfo &getModuleFileInfo(FileEntryRef File) {
      auto [It, Inserted] = ModuleFiles.try_emplace(File);
      if (Inserted) {
        unsigned NewID = ModuleFiles.size();
        ModuleFileInfo &Info = It->second;
        Info.ID = NewID;
      }
      return It->second;
    }

  public:
    explicit GlobalModuleIndexBuilder(
        FileManager &FileMgr, const PCHContainerReader &PCHContainerRdr)
        : FileMgr(FileMgr), PCHContainerRdr(PCHContainerRdr) {}

    /// Load the contents of the given module file into the builder.
    toolchain::Error loadModuleFile(FileEntryRef File);

    /// Write the index to the given bitstream.
    /// \returns true if an error occurred, false otherwise.
    bool writeIndex(toolchain::BitstreamWriter &Stream);
  };
}

static void emitBlockID(unsigned ID, const char *Name,
                        toolchain::BitstreamWriter &Stream,
                        SmallVectorImpl<uint64_t> &Record) {
  Record.clear();
  Record.push_back(ID);
  Stream.EmitRecord(toolchain::bitc::BLOCKINFO_CODE_SETBID, Record);

  // Emit the block name if present.
  if (!Name || Name[0] == 0) return;
  Record.clear();
  while (*Name)
    Record.push_back(*Name++);
  Stream.EmitRecord(toolchain::bitc::BLOCKINFO_CODE_BLOCKNAME, Record);
}

static void emitRecordID(unsigned ID, const char *Name,
                         toolchain::BitstreamWriter &Stream,
                         SmallVectorImpl<uint64_t> &Record) {
  Record.clear();
  Record.push_back(ID);
  while (*Name)
    Record.push_back(*Name++);
  Stream.EmitRecord(toolchain::bitc::BLOCKINFO_CODE_SETRECORDNAME, Record);
}

void
GlobalModuleIndexBuilder::emitBlockInfoBlock(toolchain::BitstreamWriter &Stream) {
  SmallVector<uint64_t, 64> Record;
  Stream.EnterBlockInfoBlock();

#define BLOCK(X) emitBlockID(X ## _ID, #X, Stream, Record)
#define RECORD(X) emitRecordID(X, #X, Stream, Record)
  BLOCK(GLOBAL_INDEX_BLOCK);
  RECORD(INDEX_METADATA);
  RECORD(MODULE);
  RECORD(IDENTIFIER_INDEX);
#undef RECORD
#undef BLOCK

  Stream.ExitBlock();
}

namespace {
  class InterestingASTIdentifierLookupTrait
    : public serialization::reader::ASTIdentifierLookupTraitBase {

  public:
    /// The identifier and whether it is "interesting".
    typedef std::pair<StringRef, bool> data_type;

    data_type ReadData(const internal_key_type& k,
                       const unsigned char* d,
                       unsigned DataLen) {
      // The first bit indicates whether this identifier is interesting.
      // That's all we care about.
      using namespace toolchain::support;
      IdentifierID RawID =
          endian::readNext<IdentifierID, toolchain::endianness::little>(d);
      bool IsInteresting = RawID & 0x01;
      return std::make_pair(k, IsInteresting);
    }
  };
}

toolchain::Error GlobalModuleIndexBuilder::loadModuleFile(FileEntryRef File) {
  // Open the module file.

  auto Buffer = FileMgr.getBufferForFile(File, /*isVolatile=*/true);
  if (!Buffer)
    return toolchain::createStringError(Buffer.getError(),
                                   "failed getting buffer for module file");

  // Initialize the input stream
  toolchain::BitstreamCursor InStream(PCHContainerRdr.ExtractPCH(**Buffer));

  // Sniff for the signature.
  for (unsigned char C : {'C', 'P', 'C', 'H'})
    if (Expected<toolchain::SimpleBitstreamCursor::word_t> Res = InStream.Read(8)) {
      if (Res.get() != C)
        return toolchain::createStringError(std::errc::illegal_byte_sequence,
                                       "expected signature CPCH");
    } else
      return Res.takeError();

  // Record this module file and assign it a unique ID (if it doesn't have
  // one already).
  unsigned ID = getModuleFileInfo(File).ID;

  // Search for the blocks and records we care about.
  enum { Other, ControlBlock, ASTBlock, DiagnosticOptionsBlock } State = Other;
  bool Done = false;
  while (!Done) {
    Expected<toolchain::BitstreamEntry> MaybeEntry = InStream.advance();
    if (!MaybeEntry)
      return MaybeEntry.takeError();
    toolchain::BitstreamEntry Entry = MaybeEntry.get();

    switch (Entry.Kind) {
    case toolchain::BitstreamEntry::Error:
      Done = true;
      continue;

    case toolchain::BitstreamEntry::Record:
      // In the 'other' state, just skip the record. We don't care.
      if (State == Other) {
        if (toolchain::Expected<unsigned> Skipped = InStream.skipRecord(Entry.ID))
          continue;
        else
          return Skipped.takeError();
      }

      // Handle potentially-interesting records below.
      break;

    case toolchain::BitstreamEntry::SubBlock:
      if (Entry.ID == CONTROL_BLOCK_ID) {
        if (toolchain::Error Err = InStream.EnterSubBlock(CONTROL_BLOCK_ID))
          return Err;

        // Found the control block.
        State = ControlBlock;
        continue;
      }

      if (Entry.ID == AST_BLOCK_ID) {
        if (toolchain::Error Err = InStream.EnterSubBlock(AST_BLOCK_ID))
          return Err;

        // Found the AST block.
        State = ASTBlock;
        continue;
      }

      if (Entry.ID == UNHASHED_CONTROL_BLOCK_ID) {
        if (toolchain::Error Err = InStream.EnterSubBlock(UNHASHED_CONTROL_BLOCK_ID))
          return Err;

        // Found the Diagnostic Options block.
        State = DiagnosticOptionsBlock;
        continue;
      }

      if (toolchain::Error Err = InStream.SkipBlock())
        return Err;

      continue;

    case toolchain::BitstreamEntry::EndBlock:
      State = Other;
      continue;
    }

    // Read the given record.
    SmallVector<uint64_t, 64> Record;
    StringRef Blob;
    Expected<unsigned> MaybeCode = InStream.readRecord(Entry.ID, Record, &Blob);
    if (!MaybeCode)
      return MaybeCode.takeError();
    unsigned Code = MaybeCode.get();

    // Handle module dependencies.
    if (State == ControlBlock && Code == IMPORT) {
      unsigned Idx = 0;
      // Read information about the AST file.

      // Skip the imported kind
      ++Idx;

      // Skip the import location
      ++Idx;

      // Skip the module name (currently this is only used for prebuilt
      // modules while here we are only dealing with cached).
      Blob = Blob.substr(Record[Idx++]);

      // Skip if it is standard C++ module
      ++Idx;

      // Load stored size/modification time.
      off_t StoredSize = (off_t)Record[Idx++];
      time_t StoredModTime = (time_t)Record[Idx++];

      // Skip the stored signature.
      // FIXME: we could read the signature out of the import and validate it.
      StringRef SignatureBytes = Blob.substr(0, ASTFileSignature::size);
      auto StoredSignature = ASTFileSignature::create(SignatureBytes.begin(),
                                                      SignatureBytes.end());
      Blob = Blob.substr(ASTFileSignature::size);

      // Retrieve the imported file name.
      unsigned Length = Record[Idx++];
      StringRef ImportedFile = Blob.substr(0, Length);
      Blob = Blob.substr(Length);

      // Find the imported module file.
      auto DependsOnFile =
          FileMgr.getOptionalFileRef(ImportedFile, /*OpenFile=*/false,
                                     /*CacheFailure=*/false);

      if (!DependsOnFile)
        return toolchain::createStringError(std::errc::bad_file_descriptor,
                                       "imported file \"%s\" not found",
                                       std::string(ImportedFile).c_str());

      // Save the information in ImportedModuleFileInfo so we can verify after
      // loading all pcms.
      ImportedModuleFiles.insert(std::make_pair(
          *DependsOnFile, ImportedModuleFileInfo(StoredSize, StoredModTime,
                                                 StoredSignature)));

      // Record the dependency.
      unsigned DependsOnID = getModuleFileInfo(*DependsOnFile).ID;
      getModuleFileInfo(File).Dependencies.push_back(DependsOnID);

      continue;
    }

    // Handle the identifier table
    if (State == ASTBlock && Code == IDENTIFIER_TABLE && Record[0] > 0) {
      typedef toolchain::OnDiskIterableChainedHashTable<
          InterestingASTIdentifierLookupTrait> InterestingIdentifierTable;
      std::unique_ptr<InterestingIdentifierTable> Table(
          InterestingIdentifierTable::Create(
              (const unsigned char *)Blob.data() + Record[0],
              (const unsigned char *)Blob.data() + sizeof(uint32_t),
              (const unsigned char *)Blob.data()));
      for (InterestingIdentifierTable::data_iterator D = Table->data_begin(),
                                                     DEnd = Table->data_end();
           D != DEnd; ++D) {
        std::pair<StringRef, bool> Ident = *D;
        if (Ident.second)
          InterestingIdentifiers[Ident.first].push_back(ID);
        else
          (void)InterestingIdentifiers[Ident.first];
      }
    }

    // Get Signature.
    if (State == DiagnosticOptionsBlock && Code == SIGNATURE) {
      auto Signature = ASTFileSignature::create(Blob.begin(), Blob.end());
      assert(Signature != ASTFileSignature::createDummy() &&
             "Dummy AST file signature not backpatched in ASTWriter.");
      getModuleFileInfo(File).Signature = Signature;
    }

    // We don't care about this record.
  }

  return toolchain::Error::success();
}

namespace {

/// Trait used to generate the identifier index as an on-disk hash
/// table.
class IdentifierIndexWriterTrait {
public:
  typedef StringRef key_type;
  typedef StringRef key_type_ref;
  typedef SmallVector<unsigned, 2> data_type;
  typedef const SmallVector<unsigned, 2> &data_type_ref;
  typedef unsigned hash_value_type;
  typedef unsigned offset_type;

  static hash_value_type ComputeHash(key_type_ref Key) {
    return toolchain::djbHash(Key);
  }

  std::pair<unsigned,unsigned>
  EmitKeyDataLength(raw_ostream& Out, key_type_ref Key, data_type_ref Data) {
    using namespace toolchain::support;
    endian::Writer LE(Out, toolchain::endianness::little);
    unsigned KeyLen = Key.size();
    unsigned DataLen = Data.size() * 4;
    LE.write<uint16_t>(KeyLen);
    LE.write<uint16_t>(DataLen);
    return std::make_pair(KeyLen, DataLen);
  }

  void EmitKey(raw_ostream& Out, key_type_ref Key, unsigned KeyLen) {
    Out.write(Key.data(), KeyLen);
  }

  void EmitData(raw_ostream& Out, key_type_ref Key, data_type_ref Data,
                unsigned DataLen) {
    using namespace toolchain::support;
    for (unsigned I = 0, N = Data.size(); I != N; ++I)
      endian::write<uint32_t>(Out, Data[I], toolchain::endianness::little);
  }
};

}

bool GlobalModuleIndexBuilder::writeIndex(toolchain::BitstreamWriter &Stream) {
  for (auto MapEntry : ImportedModuleFiles) {
    auto File = MapEntry.first;
    ImportedModuleFileInfo &Info = MapEntry.second;
    if (getModuleFileInfo(File).Signature) {
      if (getModuleFileInfo(File).Signature != Info.StoredSignature)
        // Verify Signature.
        return true;
    } else if (Info.StoredSize != File.getSize() ||
               Info.StoredModTime != File.getModificationTime())
      // Verify Size and ModTime.
      return true;
  }

  using namespace toolchain;
  toolchain::TimeTraceScope TimeScope("Module WriteIndex");

  // Emit the file header.
  Stream.Emit((unsigned)'B', 8);
  Stream.Emit((unsigned)'C', 8);
  Stream.Emit((unsigned)'G', 8);
  Stream.Emit((unsigned)'I', 8);

  // Write the block-info block, which describes the records in this bitcode
  // file.
  emitBlockInfoBlock(Stream);

  Stream.EnterSubblock(GLOBAL_INDEX_BLOCK_ID, 3);

  // Write the metadata.
  SmallVector<uint64_t, 2> Record;
  Record.push_back(CurrentVersion);
  Stream.EmitRecord(INDEX_METADATA, Record);

  // Write the set of known module files.
  for (ModuleFilesMap::iterator M = ModuleFiles.begin(),
                                MEnd = ModuleFiles.end();
       M != MEnd; ++M) {
    Record.clear();
    Record.push_back(M->second.ID);
    Record.push_back(M->first.getSize());
    Record.push_back(M->first.getModificationTime());

    // File name
    StringRef Name(M->first.getName());
    Record.push_back(Name.size());
    Record.append(Name.begin(), Name.end());

    // Dependencies
    Record.push_back(M->second.Dependencies.size());
    Record.append(M->second.Dependencies.begin(), M->second.Dependencies.end());
    Stream.EmitRecord(MODULE, Record);
  }

  // Write the identifier -> module file mapping.
  {
    toolchain::OnDiskChainedHashTableGenerator<IdentifierIndexWriterTrait> Generator;
    IdentifierIndexWriterTrait Trait;

    // Populate the hash table.
    for (InterestingIdentifierMap::iterator I = InterestingIdentifiers.begin(),
                                            IEnd = InterestingIdentifiers.end();
         I != IEnd; ++I) {
      Generator.insert(I->first(), I->second, Trait);
    }

    // Create the on-disk hash table in a buffer.
    SmallString<4096> IdentifierTable;
    uint32_t BucketOffset;
    {
      using namespace toolchain::support;
      toolchain::raw_svector_ostream Out(IdentifierTable);
      // Make sure that no bucket is at offset 0
      endian::write<uint32_t>(Out, 0, toolchain::endianness::little);
      BucketOffset = Generator.Emit(Out, Trait);
    }

    // Create a blob abbreviation
    auto Abbrev = std::make_shared<BitCodeAbbrev>();
    Abbrev->Add(BitCodeAbbrevOp(IDENTIFIER_INDEX));
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Fixed, 32));
    Abbrev->Add(BitCodeAbbrevOp(BitCodeAbbrevOp::Blob));
    unsigned IDTableAbbrev = Stream.EmitAbbrev(std::move(Abbrev));

    // Write the identifier table
    uint64_t Record[] = {IDENTIFIER_INDEX, BucketOffset};
    Stream.EmitRecordWithBlob(IDTableAbbrev, Record, IdentifierTable);
  }

  Stream.ExitBlock();
  return false;
}

toolchain::Error
GlobalModuleIndex::writeIndex(FileManager &FileMgr,
                              const PCHContainerReader &PCHContainerRdr,
                              StringRef Path) {
  toolchain::SmallString<128> IndexPath;
  IndexPath += Path;
  toolchain::sys::path::append(IndexPath, IndexFileName);

  // Coordinate building the global index file with other processes that might
  // try to do the same.
  toolchain::LockFileManager Lock(IndexPath);
  bool Owned;
  if (toolchain::Error Err = Lock.tryLock().moveInto(Owned)) {
    toolchain::consumeError(std::move(Err));
    return toolchain::createStringError(std::errc::io_error, "LFS error");
  }
  if (!Owned) {
    // Someone else is responsible for building the index. We don't care
    // when they finish, so we're done.
    return toolchain::createStringError(std::errc::device_or_resource_busy,
                                   "someone else is building the index");
  }

  // We're responsible for building the index ourselves.

  // The module index builder.
  GlobalModuleIndexBuilder Builder(FileMgr, PCHContainerRdr);

  // Load each of the module files.
  std::error_code EC;
  for (toolchain::sys::fs::directory_iterator D(Path, EC), DEnd;
       D != DEnd && !EC;
       D.increment(EC)) {
    // If this isn't a module file, we don't care.
    if (toolchain::sys::path::extension(D->path()) != ".pcm") {
      // ... unless it's a .pcm.lock file, which indicates that someone is
      // in the process of rebuilding a module. They'll rebuild the index
      // at the end of that translation unit, so we don't have to.
      if (toolchain::sys::path::extension(D->path()) == ".pcm.lock")
        return toolchain::createStringError(std::errc::device_or_resource_busy,
                                       "someone else is building the index");

      continue;
    }

    // If we can't find the module file, skip it.
    auto ModuleFile = FileMgr.getOptionalFileRef(D->path());
    if (!ModuleFile)
      continue;

    // Load this module file.
    if (toolchain::Error Err = Builder.loadModuleFile(*ModuleFile))
      return Err;
  }

  // The output buffer, into which the global index will be written.
  SmallString<16> OutputBuffer;
  {
    toolchain::BitstreamWriter OutputStream(OutputBuffer);
    if (Builder.writeIndex(OutputStream))
      return toolchain::createStringError(std::errc::io_error,
                                     "failed writing index");
  }

  return toolchain::writeToOutput(IndexPath, [&OutputBuffer](toolchain::raw_ostream &OS) {
    OS << OutputBuffer;
    return toolchain::Error::success();
  });
}

namespace {
  class GlobalIndexIdentifierIterator : public IdentifierIterator {
    /// The current position within the identifier lookup table.
    IdentifierIndexTable::key_iterator Current;

    /// The end position within the identifier lookup table.
    IdentifierIndexTable::key_iterator End;

  public:
    explicit GlobalIndexIdentifierIterator(IdentifierIndexTable &Idx) {
      Current = Idx.key_begin();
      End = Idx.key_end();
    }

    StringRef Next() override {
      if (Current == End)
        return StringRef();

      StringRef Result = *Current;
      ++Current;
      return Result;
    }
  };
}

IdentifierIterator *GlobalModuleIndex::createIdentifierIterator() const {
  IdentifierIndexTable &Table =
    *static_cast<IdentifierIndexTable *>(IdentifierIndex);
  return new GlobalIndexIdentifierIterator(Table);
}
