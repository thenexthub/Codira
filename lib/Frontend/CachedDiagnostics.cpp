//===--- CachedDiagnostics.cpp - Cached Diagnostics -----------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
//  This file implements the CachedDiagnosticsProcessor class.
//
//===----------------------------------------------------------------------===//

#include "language/Frontend/CachedDiagnostics.h"

#include "language/AST/DiagnosticBridge.h"
#include "language/AST/DiagnosticConsumer.h"
#include "language/AST/DiagnosticsCommon.h"
#include "language/AST/DiagnosticsFrontend.h"
#include "language/AST/DiagnosticsSema.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/SourceManager.h"
#include "language/Frontend/Frontend.h"
#include "language/Frontend/FrontendInputsAndOutputs.h"
#include "toolchain/ADT/IntrusiveRefCntPtr.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/StringExtras.h"
#include "toolchain/ADT/StringRef.h"
#include "toolchain/ADT/TinyPtrVector.h"
#include "toolchain/CAS/ObjectStore.h"
#include "toolchain/Support/Compression.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/PrefixMapper.h"
#include "toolchain/Support/SMLoc.h"
#include "toolchain/Support/VirtualFileSystem.h"
#include "toolchain/Support/YAMLTraits.h"
#include "toolchain/Support/raw_ostream.h"
#include <memory>
#include <system_error>

#define DEBUG_TYPE "cached-diags"

using namespace language;

namespace {

struct SerializedSourceLoc {
  unsigned FileID = 0;
  unsigned Offset = 0;

  bool operator==(const SerializedSourceLoc &RHS) const {
    return FileID == RHS.FileID && Offset == RHS.Offset;
  }
};

struct SerializedCharSourceRange {
  SerializedSourceLoc Start;
  unsigned ByteLength;
};

struct SerializedFixIt {
  SerializedCharSourceRange Range;
  std::string Text;
};

struct SerializedDiagnosticInfo {
  static_assert(sizeof(language::DiagID) == sizeof(uint32_t), "DiagID size");
  static_assert(sizeof(language::DiagnosticKind) == sizeof(uint8_t),
                "DiagKind size");
  uint32_t ID;
  SerializedSourceLoc Loc;
  uint8_t Kind;
  std::string FormatString;
  std::string Category;
  SerializedSourceLoc BufferIndirectlyCausingDiagnostic;
  std::vector<SerializedDiagnosticInfo> ChildDiagnosticInfo;
  std::vector<std::string> EducationalNotePaths;
  std::vector<SerializedCharSourceRange> Ranges;
  std::vector<SerializedFixIt> FixIts;
  bool IsChildNote;
};

struct SerializedFile {
  std::string FileName;
  SerializedSourceLoc IncludeLoc = SerializedSourceLoc();
  std::string ContentCASID;
  StringRef Content;
};

struct SerializedVirtualFile {
  std::string FileName;
  SerializedCharSourceRange Range;
  int LineOffset;
};

struct SerializedGeneratedFileInfo {
  uint8_t Kind;
  unsigned FileID;
  SerializedCharSourceRange OriginalRange;
  SerializedCharSourceRange GeneratedRange;
  std::string MacroName;
};

struct DiagnosticSerializer {
  DiagnosticSerializer(toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS,
                       toolchain::PrefixMapper &Mapper, toolchain::cas::ObjectStore &CAS)
      : SrcMgr(FS), Mapper(Mapper), CAS(CAS) {}

  using ReplayFunc = toolchain::function_ref<toolchain::Error(const DiagnosticInfo &)>;

  // public APIs for serialization.
  void handleDiagnostic(SourceManager &SM, const DiagnosticInfo &Info,
                        ReplayFunc Fn = nullptr);
  toolchain::Error serializeEmittedDiagnostics(toolchain::raw_ostream &os);

  static toolchain::Error
  emitDiagnosticsFromCached(toolchain::StringRef Buffer,
                            DiagnosticEngine &Diags, toolchain::PrefixMapper &Mapper,
                            toolchain::cas::ObjectStore &CAS,
                            const FrontendInputsAndOutputs &InAndOut) {
    // Create a new DiagnosticSerializer since this cannot be shared with a
    // serialization instance. Using an empty in-memory file system as
    // underlying file system because the replay logic should not touch file
    // system.
    auto FS = toolchain::makeIntrusiveRefCnt<toolchain::vfs::InMemoryFileSystem>();
    DiagnosticSerializer DS(std::move(FS), Mapper, CAS);
    return DS.doEmitFromCached(Buffer, Diags);
  }

  SourceManager &getSourceMgr() { return SrcMgr; }

  void addInputsToSourceMgr(SourceManager &SM,
                            const FrontendInputsAndOutputs &InAndOut) {
    // Extract all the input file names so they can be added to the source
    // manager when replaying the diagnostics. All input files are needed even
    // they don't contain diagnostics because FileSpecificDiagConsumer need
    // has references to input files to find subconsumer.
    auto addInputToSourceMgr = [&](const InputFile &Input) {
      auto Path = remapFilePath(Input.getFileName());
      SrcMgr.getExternalSourceBufferID(Path);

      // Fetch the source buffer from original SourceManager and create a
      // serialized file from it.
      auto Idx = SM.getExternalSourceBufferID(Path);
      if (Idx != 0)
        getFileIDFromBufferID(SM, Idx);

      return false;
    };
    InAndOut.forEachInput(addInputToSourceMgr);
  }

private:
  // Serialization helper
  unsigned getFileIDFromBufferID(SourceManager &SM, unsigned Idx);
  SerializedSourceLoc convertSourceLoc(SourceManager &SM, const SourceLoc &Loc,
                                       bool AddVirtualFile = true);
  SerializedCharSourceRange convertSourceRange(SourceManager &SM,
                                               const CharSourceRange &Range,
                                               bool AddVirtualFile = true);
  SerializedFixIt convertFixIt(SourceManager &SM,
                               const DiagnosticInfo::FixIt &FI);
  SerializedDiagnosticInfo convertDiagnosticInfo(SourceManager &SM,
                                                 const DiagnosticInfo &Info);

  // Deserialization helper
  toolchain::Error doEmitFromCached(toolchain::StringRef Buffer, DiagnosticEngine &Diags);
  toolchain::Expected<SourceLoc> deserializeSourceLoc(const SerializedSourceLoc &);
  toolchain::Expected<CharSourceRange>
  deserializeSourceRange(const SerializedCharSourceRange &);
  toolchain::Expected<DiagnosticInfo::FixIt>
  deserializeFixIt(const SerializedFixIt &);

  // Stores the temporary data from deserialization.
  struct DiagnosticStorage {
    SmallVector<DiagnosticInfo, 2> ChildDiag;
    SmallVector<CharSourceRange, 2> Ranges;
    SmallVector<DiagnosticInfo::FixIt, 2> FixIts;
  };
  toolchain::Error deserializeDiagnosticInfo(const SerializedDiagnosticInfo &,
                                        DiagnosticStorage &, ReplayFunc);

  // Deserialize File and return the bufferID in serializing SourceManager.
  toolchain::Expected<unsigned> deserializeFile(const SerializedFile &File);
  toolchain::Error deserializeVirtualFile(const SerializedVirtualFile &VF);
  toolchain::Error deserializeGeneratedFileInfo(const SerializedGeneratedFileInfo &Info);
  std::string remapFilePath(StringRef Path) {
    return Mapper.mapToString(Path);
  }

public:
  std::vector<SerializedDiagnosticInfo> DiagInfos;
  std::vector<SerializedFile> Files;
  std::vector<SerializedVirtualFile> VFiles;
  std::vector<SerializedGeneratedFileInfo> GeneratedFileInfo;

private:
  // Handle FileID. ID 0 is reserved for SMLoc().
  unsigned CurrentFileID = 0;

  // Serializing SourceManager.
  SourceManager SrcMgr;
  toolchain::PrefixMapper &Mapper;

  // CAS for file system backing.
  toolchain::cas::ObjectStore &CAS;

  // Mapping of the FileID between SourceManager from CompilerInstance vs.
  // the serialized FileID in cached diagnostics. Lookup tables are
  // per-SourceManager to handle diagnostics from all sub-instances which
  // uses different SourceManager.
  toolchain::DenseMap<SourceManager *, toolchain::DenseMap<unsigned, unsigned>>
      FileMapper;
  toolchain::DenseMap<SourceManager *, toolchain::DenseSet<const char *>> SeenVFiles;
};
}

namespace toolchain::yaml {
template <>
struct MappingTraits<SerializedSourceLoc> {
  static void mapping(IO &io, SerializedSourceLoc &Loc) {
    io.mapRequired("FileID", Loc.FileID);
    io.mapRequired("Offset", Loc.Offset);
  }
};

template <>
struct MappingTraits<SerializedCharSourceRange> {
  static void mapping(IO &io, SerializedCharSourceRange &Range) {
    io.mapRequired("Loc", Range.Start);
    io.mapRequired("Length", Range.ByteLength);
  }
};

template <>
struct MappingTraits<SerializedFixIt> {
  static void mapping(IO &io, SerializedFixIt &FixIt) {
    io.mapRequired("Range", FixIt.Range);
    io.mapRequired("Text", FixIt.Text);
  }
};

template <>
struct MappingTraits<SerializedDiagnosticInfo> {
  static void mapping(IO &io, SerializedDiagnosticInfo &Info) {
    io.mapRequired("DiagID", Info.ID);
    io.mapRequired("Loc", Info.Loc);
    io.mapRequired("Kind", Info.Kind);
    io.mapRequired("Text", Info.FormatString);
    io.mapOptional("Category", Info.Category, "");
    io.mapOptional("BICD", Info.BufferIndirectlyCausingDiagnostic);
    io.mapOptional("ChildDiag", Info.ChildDiagnosticInfo);
    io.mapOptional("EducationalNotePaths", Info.EducationalNotePaths);
    io.mapOptional("Ranges", Info.Ranges);
    io.mapOptional("FixIts", Info.FixIts);
    io.mapOptional("IsChildNote", Info.IsChildNote, false);
  }
};

template <>
struct MappingTraits<SerializedFile> {
  static void mapping(IO &io, SerializedFile &F) {
    io.mapRequired("Name", F.FileName);
    io.mapOptional("IncludeLoc", F.IncludeLoc, SerializedSourceLoc());
    io.mapOptional("Content", F.Content, StringRef());
    io.mapOptional("CASID", F.ContentCASID, "");
  }
};

template <>
struct MappingTraits<SerializedVirtualFile> {
  static void mapping(IO &io, SerializedVirtualFile &F) {
    io.mapRequired("Name", F.FileName);
    io.mapRequired("Range", F.Range);
    io.mapOptional("LineOffset", F.LineOffset, 0);
  }
};

template <>
struct MappingTraits<SerializedGeneratedFileInfo> {
  static void mapping(IO &io, SerializedGeneratedFileInfo &Info) {
    io.mapRequired("Kind", Info.Kind);
    io.mapRequired("FileID", Info.FileID);
    io.mapRequired("OriginalRange", Info.OriginalRange);
    io.mapRequired("GeneratedRange", Info.GeneratedRange);
    io.mapOptional("MacroName", Info.MacroName, "");
  }
};

template <>
struct MappingTraits<DiagnosticSerializer> {
  static void mapping(IO &io, DiagnosticSerializer &S) {
    io.mapRequired("Files", S.Files);
    io.mapOptional("VirtualFiles", S.VFiles);
    io.mapRequired("Diagnostics", S.DiagInfos);
    io.mapOptional("GeneratedFileInfo", S.GeneratedFileInfo);
  }
};


} // namespace toolchain::yaml

TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(SerializedCharSourceRange)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(SerializedFixIt)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(SerializedDiagnosticInfo)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(SerializedFile)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(SerializedVirtualFile)
TOOLCHAIN_YAML_IS_SEQUENCE_VECTOR(SerializedGeneratedFileInfo)

void DiagnosticSerializer::handleDiagnostic(SourceManager &SM,
                                            const DiagnosticInfo &Info,
                                            ReplayFunc Fn) {
  DiagInfos.emplace_back(convertDiagnosticInfo(SM, Info));
  if (Fn) {
    DiagnosticStorage Storage;
    cantFail(deserializeDiagnosticInfo(DiagInfos.back(), Storage, Fn));
  }
}

unsigned DiagnosticSerializer::getFileIDFromBufferID(SourceManager &SM,
                                                     unsigned Idx) {
  // See if the file is already constructed.
  auto &Allocated = FileMapper[&SM];
  auto ID = Allocated.find(Idx);
  if (ID != Allocated.end())
    return ID->second;

  auto &Buf = SM.getLLVMSourceMgr().getBufferInfo(Idx);
  auto Filename = Buf.Buffer->getBufferIdentifier();
  bool IsFileBacked = !Filename.empty() && SM.getFileSystem()->exists(Filename);

  // Construct and add to files. If there is an IncludeLoc, the file from
  // IncludeLoc is added before current file.
  assert(CurrentFileID == Files.size() && "File index mismatch");

  StringRef FileContent = Buf.Buffer->getBuffer();
  SerializedFile File = {Filename.str(),
                         convertSourceLoc(SM, SourceLoc(Buf.IncludeLoc)),
                         {},
                         IsFileBacked ? "" : FileContent};

  // Add file to serializing source manager.
  unsigned NewIdx = SrcMgr.addMemBufferCopy(Buf.Buffer.get());
  FileMapper[&SrcMgr].insert({CurrentFileID, NewIdx});

  Files.emplace_back(std::move(File));
  Allocated.insert({Idx, ++CurrentFileID});
  unsigned NewFileID = CurrentFileID;

  auto Info = SM.getGeneratedSourceInfo(Idx);
  auto convertGeneratedFileInfo =
      [&](const GeneratedSourceInfo &Info) -> SerializedGeneratedFileInfo {
    return {(uint8_t)Info.kind, CurrentFileID,
            convertSourceRange(SM, Info.originalSourceRange),
            convertSourceRange(SM, Info.generatedSourceRange), Info.macroName};
  };
  if (Info) {
    auto GI = convertGeneratedFileInfo(*Info);
    // Add generated file info to source manager.
    cantFail(deserializeGeneratedFileInfo(GI));
    GeneratedFileInfo.emplace_back(std::move(GI));
  }

  return NewFileID;
}

SerializedSourceLoc
DiagnosticSerializer::convertSourceLoc(SourceManager &SM, const SourceLoc &Loc,
                                       bool AddVirtualFile) {
  if (Loc == SourceLoc())
    return SerializedSourceLoc();

  // This is locked with ABI for toolchain::SMLoc and toolchain::SourceMgr.
  assert(SM.isOwning(Loc) && "SourceLoc is not owned by SourceManager");

  // Check VirtualFile. If the SourceLoc is from a virtual file, create a
  // 1 byte virtual file that is just enough to map the diagnostic.
  // Don't try to remap the entire region when the diagnostics was handled since
  // virtual file region can change if the diagnostics are from parser.
  // This assumes the same SourceLoc cannot be mapped to different virtual file
  // during the compilation.
  if (AddVirtualFile) {
    auto convertVirtualFile = [&](const std::string &Name, SourceLoc Start,
                                  int LineOffset) -> SerializedVirtualFile {
      CharSourceRange Range(Start, 1);
      return {Name, convertSourceRange(SM, Range, /*AddVirtualFile=*/false),
              LineOffset};
    };
    if (auto *VF = SM.getVirtualFile(Loc)) {
      const char* VFStart = (const char*) Loc.getOpaquePointerValue();
      if (!SeenVFiles[&SM].count(VFStart)) {
        auto SVF = convertVirtualFile(VF->Name, Loc, VF->LineOffset);
        cantFail(deserializeVirtualFile(SVF));
        VFiles.emplace_back(std::move(SVF));
        SeenVFiles[&SM].insert(VFStart);
      }
    }
  }
  unsigned BufID = SM.findBufferContainingLoc(Loc);
  unsigned FileID = getFileIDFromBufferID(SM, BufID);
  auto &Info = SM.getLLVMSourceMgr().getBufferInfo(BufID);
  unsigned Offset =
      (const char *)Loc.getOpaquePointerValue() - Info.Buffer->getBufferStart();

  return {FileID, Offset};
}

SerializedCharSourceRange DiagnosticSerializer::convertSourceRange(
    SourceManager &SM, const CharSourceRange &Range, bool AddVirtualFile) {
  return {convertSourceLoc(SM, Range.getStart(), AddVirtualFile),
          Range.isValid() ? Range.getByteLength() : 0};
}

SerializedFixIt
DiagnosticSerializer::convertFixIt(SourceManager &SM,
                                   const DiagnosticInfo::FixIt &FI) {
  return {convertSourceRange(SM, FI.getRange()), FI.getText().str()};
}

SerializedDiagnosticInfo
DiagnosticSerializer::convertDiagnosticInfo(SourceManager &SM,
                                            const DiagnosticInfo &Info) {
  toolchain::SmallString<256> Text;
  {
    toolchain::SmallString<256> Formatted;
    toolchain::raw_svector_ostream OS(Formatted);
    DiagnosticEngine::formatDiagnosticText(OS, Info.FormatString,
                                           Info.FormatArgs);

    // If formatted diagnostic has "%" in it, it needs to be rewrite to "%%".
    StringRef InText(Formatted);
    toolchain::raw_svector_ostream Out(Text);
    auto Percent = InText.split('%');
    Out << Percent.first;
    while (!Percent.second.empty()) {
      Out << "%%";
      Percent = Percent.second.split('%');
      Out << Percent.first;
    }
  }

  auto convertDiagnosticInfoArray = [&](ArrayRef<DiagnosticInfo *> Infos) {
    std::vector<SerializedDiagnosticInfo> Serialized;
    Serialized.reserve(Infos.size());
    toolchain::for_each(Infos, [&](DiagnosticInfo *Info) {
      return Serialized.emplace_back(convertDiagnosticInfo(SM, *Info));
    });
    return Serialized;
  };

  auto convertSourceRangeArray = [&](ArrayRef<CharSourceRange> Ranges) {
    std::vector<SerializedCharSourceRange> Serialized;
    Serialized.reserve(Ranges.size());
    toolchain::for_each(Ranges, [&](const CharSourceRange &Range) {
      return Serialized.emplace_back(convertSourceRange(SM, Range));
    });
    return Serialized;
  };

  auto convertFixItArray = [&](ArrayRef<DiagnosticInfo::FixIt> FixIts) {
    std::vector<SerializedFixIt> Serialized;
    Serialized.reserve(FixIts.size());
    toolchain::for_each(FixIts, [&](const DiagnosticInfo::FixIt &FI) {
      return Serialized.emplace_back(convertFixIt(SM, FI));
    });
    return Serialized;
  };

  std::vector<std::string> educationalNotes;
  if (!Info.CategoryDocumentationURL.empty())
    educationalNotes.push_back(Info.CategoryDocumentationURL);
  return {(uint32_t)Info.ID,
          convertSourceLoc(SM, Info.Loc),
          (uint8_t)Info.Kind,
          std::string(Text.data(), Text.size()),
          Info.Category.str(),
          convertSourceLoc(SM, Info.BufferIndirectlyCausingDiagnostic),
          convertDiagnosticInfoArray(Info.ChildDiagnosticInfo),
          educationalNotes,
          convertSourceRangeArray(Info.Ranges),
          convertFixItArray(Info.FixIts),
          Info.IsChildNote};
}

static toolchain::Error createDeserializationError(StringRef Msg) {
  return toolchain::createStringError(std::errc::protocol_error, Msg.str().c_str());
}

toolchain::Expected<SourceLoc>
DiagnosticSerializer::deserializeSourceLoc(const SerializedSourceLoc &Loc) {
  if (Loc.FileID == 0)
    return SourceLoc();

  auto BufID = FileMapper[&SrcMgr].find(Loc.FileID - 1);
  if (BufID == FileMapper[&SrcMgr].end())
    return createDeserializationError("File doesn't exist in SourceManager");
  auto &Info = SrcMgr.getLLVMSourceMgr().getBufferInfo(BufID->second);
  const char *Buffer = Info.Buffer->getBufferStart();
  toolchain::SMLoc SL = toolchain::SMLoc::getFromPointer(Buffer + Loc.Offset);
  return SourceLoc(SL);
}

toolchain::Expected<CharSourceRange> DiagnosticSerializer::deserializeSourceRange(
    const SerializedCharSourceRange &Range) {
  auto Start = deserializeSourceLoc(Range.Start);
  if (!Start)
    return Start.takeError();

  return CharSourceRange(*Start, Range.ByteLength);
}

toolchain::Expected<DiagnosticInfo::FixIt>
DiagnosticSerializer::deserializeFixIt(const SerializedFixIt &FI) {
  auto Range = deserializeSourceRange(FI.Range);
  if (!Range)
    return Range.takeError();

  return DiagnosticInfo::FixIt(*Range, FI.Text, {});
}

toolchain::Expected<unsigned>
DiagnosticSerializer::deserializeFile(const SerializedFile &File) {
  assert(File.IncludeLoc.FileID == 0 && "IncludeLoc not supported yet");
  auto FileName = remapFilePath(File.FileName);

  if (!File.ContentCASID.empty()) {
    auto ID = CAS.parseID(File.ContentCASID);
    if (!ID)
      return ID.takeError();

    auto Proxy = CAS.getProxy(*ID);
    if (!Proxy)
      return Proxy.takeError();

    auto Content = Proxy->getMemoryBuffer(FileName);
    return SrcMgr.addNewSourceBuffer(std::move(Content));
  }

  auto Content = toolchain::MemoryBuffer::getMemBufferCopy(File.Content, FileName);
  return SrcMgr.addNewSourceBuffer(std::move(Content));
}

toolchain::Error
DiagnosticSerializer::deserializeVirtualFile(const SerializedVirtualFile &VF) {
  auto Range = deserializeSourceRange(VF.Range);
  if (!Range)
    return Range.takeError();
  unsigned Length = (const char *)Range->getEnd().getOpaquePointerValue() -
                    (const char *)Range->getStart().getOpaquePointerValue();
  auto FileName = remapFilePath(VF.FileName);
  SrcMgr.createVirtualFile(Range->getStart(), FileName, VF.LineOffset, Length);
  return toolchain::Error::success();
}

toolchain::Error DiagnosticSerializer::deserializeGeneratedFileInfo(
    const SerializedGeneratedFileInfo &GI) {
  auto ID = FileMapper[&SrcMgr].find(GI.FileID - 1);
  if (ID == FileMapper[&SrcMgr].end())
    return createDeserializationError(
        "BufferID for GeneratedSourceInfo not found");
  GeneratedSourceInfo Info;
  Info.kind = (GeneratedSourceInfo::Kind)GI.Kind;
  auto OriginalRange = deserializeSourceRange(GI.OriginalRange);
  if (!OriginalRange)
    return OriginalRange.takeError();
  Info.originalSourceRange = *OriginalRange;
  auto GeneratedRange = deserializeSourceRange(GI.GeneratedRange);
  if (!GeneratedRange)
    return GeneratedRange.takeError();
  Info.generatedSourceRange = *GeneratedRange;
  Info.macroName = GI.MacroName;
  SrcMgr.setGeneratedSourceInfo(ID->second, Info);
  return toolchain::Error::success();
}

toolchain::Error DiagnosticSerializer::deserializeDiagnosticInfo(
    const SerializedDiagnosticInfo &Info, DiagnosticStorage &Storage,
    ReplayFunc callback) {
  DiagID ID = (DiagID)Info.ID;
  auto Loc = deserializeSourceLoc(Info.Loc);
  if (!Loc)
    return Loc.takeError();
  DiagnosticKind Kind = (DiagnosticKind)Info.Kind;
  auto BICD = deserializeSourceLoc(Info.BufferIndirectlyCausingDiagnostic);
  if (!BICD)
    return BICD.takeError();

  toolchain::TinyPtrVector<DiagnosticInfo *> ChildDiagPtrs;
  for (auto &CD : Info.ChildDiagnosticInfo) {
    auto E =
        deserializeDiagnosticInfo(CD, Storage, [&](const DiagnosticInfo &Info) {
          Storage.ChildDiag.emplace_back(Info);
          ChildDiagPtrs.push_back(&Storage.ChildDiag.back());
          return toolchain::Error::success();
        });
    if (E)
      return E;
  }
  for (auto &R : Info.Ranges) {
    auto Range = deserializeSourceRange(R);
    if (!Range)
      return Range.takeError();
    Storage.Ranges.emplace_back(*Range);
  }
  auto Ranges = ArrayRef(Storage.Ranges).take_back(Info.Ranges.size());
  for (auto &F : Info.FixIts) {
    auto FixIt = deserializeFixIt(F);
    if (!FixIt)
      return FixIt.takeError();
    Storage.FixIts.emplace_back(*FixIt);
  }
  auto FixIts = ArrayRef(Storage.FixIts).take_back(Info.FixIts.size());

  DiagnosticInfo DeserializedInfo{ID,
                                  *Loc,
                                  Kind,
                                  Info.FormatString,
                                  {},
                                  Info.Category,
                                  *BICD,
                                  ChildDiagPtrs,
                                  Ranges,
                                  FixIts,
                                  Info.IsChildNote};
  if (Info.EducationalNotePaths.size() == 1)
    DeserializedInfo.CategoryDocumentationURL = Info.EducationalNotePaths[0];
  return callback(DeserializedInfo);
}

toolchain::Error
DiagnosticSerializer::serializeEmittedDiagnostics(toolchain::raw_ostream &os) {
  // If no diagnostics is produced, do not write anything so output is smaller
  // without referencing any files.
  if (DiagInfos.empty())
    return toolchain::Error::success();

  // Convert all file backed source file into CASIDs.
  for (auto &File : Files) {
    if (!File.Content.empty() || !File.ContentCASID.empty())
      continue;

    auto Ref =
        SrcMgr.getFileSystem()->getObjectRefForFileContent(File.FileName);
    if (!Ref)
      return toolchain::createFileError(File.FileName, Ref.getError());

    if (*Ref) {
      File.ContentCASID = CAS.getID(**Ref).toString();
      continue;
    }

    // Probably a file system that is not CAS based. Ingest the buffer.
    auto Buf = SrcMgr.getFileSystem()->getBufferForFile(File.FileName);
    if (!Buf)
      return toolchain::createFileError(File.FileName, Buf.getError());

    auto BufRef = CAS.storeFromString({}, (*Buf)->getBuffer());
    if (!BufRef)
      return toolchain::createFileError(File.FileName, BufRef.takeError());
    File.ContentCASID = CAS.getID(*BufRef).toString();
  }

  toolchain::yaml::Output yout(os);
  yout << *this;
  return toolchain::Error::success();
}

toolchain::Error DiagnosticSerializer::doEmitFromCached(toolchain::StringRef Buffer,
                                                   DiagnosticEngine &Diags) {
  toolchain::yaml::Input yin(Buffer);
  yin >> *this;

  if (yin.error())
    return toolchain::errorCodeToError(yin.error());

  // Populate SourceManager with Files.
  unsigned ID = 0;
  for (auto &File : Files) {
    assert(File.IncludeLoc.FileID == 0 && "IncludeLoc not supported yet");
    auto Idx = deserializeFile(File);
    if (!Idx)
      return Idx.takeError();
    FileMapper[&SrcMgr].insert({ID++, *Idx});
  }

  for (auto &VF : VFiles) {
    if (auto E = deserializeVirtualFile(VF))
      return E;
  }

  for (auto &GI : GeneratedFileInfo) {
    if (auto E = deserializeGeneratedFileInfo(GI))
      return E;
  }

  for (auto &Info : DiagInfos) {
    DiagnosticStorage Storage;
    auto E = deserializeDiagnosticInfo(Info, Storage,
                                       [&](const DiagnosticInfo &Info) {
                                         for (auto *Diag : Diags.getConsumers())
                                           Diag->handleDiagnostic(SrcMgr, Info);
                                         return toolchain::Error::success();
                                       });
    if (E)
      return E;
  }
  return toolchain::Error::success();
}

class CachingDiagnosticsProcessor::Implementation
    : public language::DiagnosticConsumer {
public:
  Implementation(CompilerInstance &Instance)
      : InstanceSourceMgr(Instance.getSourceMgr()),
        InAndOut(
            Instance.getInvocation().getFrontendOptions().InputsAndOutputs),
        Diags(Instance.getDiags()), CAS(*Instance.getSharedCASInstance()) {
    SmallVector<toolchain::MappedPrefix, 4> Prefixes;
    toolchain::MappedPrefix::transformJoinedIfValid(
        Instance.getInvocation().getFrontendOptions().CacheReplayPrefixMap,
        Prefixes);
    Mapper.addRange(Prefixes);
    Mapper.sort();
  }
  ~Implementation() {}

  void startDiagnosticCapture() {
    assert(!IsCapturing && "Already started capturing");
    OrigConsumers = Diags.takeConsumers();
    Diags.addConsumer(*this);
    IsCapturing = true;
  }

  void endDiagnosticCapture() {
    assert(IsCapturing && "Did not start capturing");
    assert(Diags.getConsumers().size() == 1 && "Overlapping capture");
    Diags.removeConsumer(*this);
    toolchain::for_each(OrigConsumers, [&](DiagnosticConsumer *DC) {
      Diags.addConsumer(*DC);
    });
    OrigConsumers.clear();
    IsCapturing = false;
  }

  toolchain::Error replayCachedDiagnostics(toolchain::StringRef Buffer) {
    return DiagnosticSerializer::emitDiagnosticsFromCached(
        Buffer, Diags, Mapper, CAS, InAndOut);
  }

  void handleDiagnostic(SourceManager &SM,
                        const DiagnosticInfo &Info) override {
    auto &Serializer = getSerializer();
    assert(SM.getFileSystem() == Serializer.getSourceMgr().getFileSystem() &&
           "Caching for a different file system");

    // Bypass the caching.
    if (BypassDiagIDs.count(Info.ID)) {
      for (auto *Diag : OrigConsumers)
        Diag->handleDiagnostic(Serializer.getSourceMgr(), Info);
      return;
    }
    Serializer.handleDiagnostic(SM, Info, [&](const DiagnosticInfo &Info) {
      for (auto *Diag : OrigConsumers)
        Diag->handleDiagnostic(Serializer.getSourceMgr(), Info);
      return toolchain::Error::success();
    });
  }

  bool finishProcessing() override {
    // Finish all the consumers that are being captured.
    for (auto *Diag : OrigConsumers)
      Diag->finishProcessing();

    endDiagnosticCapture();
    toolchain::SmallString<256> Text;
    toolchain::raw_svector_ostream OS(Text);
    if (auto Err = serializeEmittedDiagnostics(OS)) {
      Diags.diagnose(SourceLoc(), diag::error_failed_cached_diag,
                     toString(std::move(Err)));
      return true;
    }
    return serializedOutputCallback(OS.str());
  }

  toolchain::Error serializeEmittedDiagnostics(toolchain::raw_ostream &os) {
    assert(!IsCapturing && "End capture before emitting");
    return getSerializer().serializeEmittedDiagnostics(os);
  }

private:
  DiagnosticSerializer &getSerializer() {
    // If the DiagnosticSerializer is not setup, create it. It cannot
    // be created on the creation of CachingDiagnosticsProcessor because the
    // Job can overwrite the FileSystem in CompilerInstance. Diagnostics
    // SourceManager is created with the filesystem in source manager in
    // compiler instance on the first diagnostics and assert if the underlying
    // file system changes on later diagnostics.
    if (!Serializer) {
      Serializer.reset(new DiagnosticSerializer(
          InstanceSourceMgr.getFileSystem(), Mapper, CAS));
      Serializer->addInputsToSourceMgr(InstanceSourceMgr, InAndOut);
    }

    return *Serializer;
  }

private:
  friend CachingDiagnosticsProcessor;
  std::vector<DiagnosticConsumer*> OrigConsumers;

  // Owning SourceManager for replaying diagnostics. SourceManager needs to
  // be alive until all consumers finishProcessing() and user needs to keep
  // Processor/Serializer alive until then.
  std::unique_ptr<DiagnosticSerializer> Serializer;

  const toolchain::SmallDenseSet<DiagID> BypassDiagIDs = {diag::macro_loaded.ID};

  SourceManager &InstanceSourceMgr;
  const FrontendInputsAndOutputs &InAndOut;
  DiagnosticEngine &Diags;
  toolchain::PrefixMapper Mapper;
  toolchain::cas::ObjectStore &CAS;

  toolchain::unique_function<bool(StringRef)> serializedOutputCallback;

  bool IsCapturing = false;
};

CachingDiagnosticsProcessor::CachingDiagnosticsProcessor(
    CompilerInstance &Instance)
    : Impl(*new Implementation(Instance)) {
  Impl.serializedOutputCallback = [&](StringRef Output) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << Output << "\n";);
    if (!Instance.getInvocation().getCASOptions().EnableCaching)
      return false;

    // compress the YAML file.
    toolchain::SmallVector<uint8_t, 512> Compression;
    if (!Output.empty()) {
      if (toolchain::compression::zstd::isAvailable())
        toolchain::compression::zstd::compress(arrayRefFromStringRef(Output),
                                          Compression);
      else if (toolchain::compression::zlib::isAvailable())
        toolchain::compression::zlib::compress(arrayRefFromStringRef(Output),
                                          Compression);
    }

    // Write the uncompressed size in the end.
    if (!Compression.empty()) {
      toolchain::raw_svector_ostream BufOS((SmallVectorImpl<char> &)Compression);
      toolchain::support::endian::Writer Writer(BufOS, toolchain::endianness::little);
      Writer.write(uint32_t(Output.size()));
    }

    StringRef Content = Compression.empty() ? Output : toStringRef(Compression);
    // Store CachedDiagnostics in the CAS/Cache.
    // FIXME: Currently associated with first output producing input file.
    auto Err = Instance.getCASOutputBackend().storeCachedDiagnostics(
        Instance.getInvocation()
            .getFrontendOptions()
            .InputsAndOutputs.getIndexOfFirstOutputProducingInput(),
        Content);

    if (Err) {
      Instance.getDiags().diagnose(SourceLoc(), diag::error_cas,
                                   "storing outputs", toString(std::move(Err)));
      return true;
    }

    return false;
  };
}

CachingDiagnosticsProcessor::~CachingDiagnosticsProcessor() { delete &Impl; }

void CachingDiagnosticsProcessor::startDiagnosticCapture() {
  Impl.startDiagnosticCapture();
}

void CachingDiagnosticsProcessor::endDiagnosticCapture() {
  Impl.endDiagnosticCapture();
}

toolchain::Error CachingDiagnosticsProcessor::serializeEmittedDiagnostics(
    toolchain::raw_ostream &os) {
  return Impl.serializeEmittedDiagnostics(os);
}

toolchain::Error
CachingDiagnosticsProcessor::replayCachedDiagnostics(toolchain::StringRef Buffer) {
  // If empty buffer, no diagnostics to replay.
  if (Buffer.empty())
    return toolchain::Error::success();

  SmallVector<uint8_t, 512> Uncompressed;
  if (toolchain::compression::zstd::isAvailable() ||
      toolchain::compression::zlib::isAvailable()) {
    if (Buffer.size() < sizeof(uint32_t))
      return toolchain::errorCodeToError(
          std::make_error_code(std::errc::message_size));

    uint32_t UncompressedSize =
        toolchain::support::endian::read<uint32_t, toolchain::endianness::little>(
            Buffer.data() + Buffer.size() - sizeof(uint32_t));

    StringRef CompressedData = Buffer.drop_back(sizeof(uint32_t));
    if (toolchain::compression::zstd::isAvailable()) {
      if (auto E = toolchain::compression::zstd::decompress(
              arrayRefFromStringRef(CompressedData), Uncompressed,
              UncompressedSize))
        return E;
    } else if (toolchain::compression::zlib::isAvailable()) {
      if (auto E = toolchain::compression::zlib::decompress(
              arrayRefFromStringRef(CompressedData), Uncompressed,
              UncompressedSize))
        return E;
    }
  }

  StringRef InputBuffer =
      Uncompressed.empty() ? Buffer : toStringRef(Uncompressed);

  return Impl.replayCachedDiagnostics(InputBuffer);
}
