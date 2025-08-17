//===--- APINotesManager.cpp - Manage API Notes Files ---------------------===//
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

#include "language/Core/APINotes/APINotesManager.h"
#include "language/Core/APINotes/APINotesReader.h"
#include "language/Core/APINotes/APINotesYAMLCompiler.h"
#include "language/Core/Basic/Diagnostic.h"
#include "language/Core/Basic/FileManager.h"
#include "language/Core/Basic/LangOptions.h"
#include "language/Core/Basic/Module.h"
#include "language/Core/Basic/SourceManager.h"
#include "language/Core/Basic/SourceMgrAdapter.h"
#include "toolchain/ADT/APInt.h"
#include "toolchain/ADT/SetVector.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/ADT/Statistic.h"
#include "toolchain/Support/MemoryBuffer.h"
#include "toolchain/Support/Path.h"
#include "toolchain/Support/PrettyStackTrace.h"

using namespace language::Core;
using namespace api_notes;

#define DEBUG_TYPE "API Notes"
STATISTIC(NumHeaderAPINotes, "non-framework API notes files loaded");
STATISTIC(NumPublicFrameworkAPINotes, "framework public API notes loaded");
STATISTIC(NumPrivateFrameworkAPINotes, "framework private API notes loaded");
STATISTIC(NumFrameworksSearched, "frameworks searched");
STATISTIC(NumDirectoriesSearched, "header directories searched");
STATISTIC(NumDirectoryCacheHits, "directory cache hits");

namespace {
/// Prints two successive strings, which much be kept alive as long as the
/// PrettyStackTrace entry.
class PrettyStackTraceDoubleString : public toolchain::PrettyStackTraceEntry {
  StringRef First, Second;

public:
  PrettyStackTraceDoubleString(StringRef First, StringRef Second)
      : First(First), Second(Second) {}
  void print(raw_ostream &OS) const override { OS << First << Second; }
};
} // namespace

APINotesManager::APINotesManager(SourceManager &SM, const LangOptions &LangOpts)
    : SM(SM), ImplicitAPINotes(LangOpts.APINotes),
      VersionIndependentSwift(LangOpts.SwiftVersionIndependentAPINotes) {}

APINotesManager::~APINotesManager() {
  // Free the API notes readers.
  for (const auto &Entry : Readers) {
    if (auto Reader = dyn_cast_if_present<APINotesReader *>(Entry.second))
      delete Reader;
  }

  delete CurrentModuleReaders[ReaderKind::Public];
  delete CurrentModuleReaders[ReaderKind::Private];
}

std::unique_ptr<APINotesReader>
APINotesManager::loadAPINotes(FileEntryRef APINotesFile) {
  PrettyStackTraceDoubleString Trace("Loading API notes from ",
                                     APINotesFile.getName());

  // Open the source file.
  auto SourceFileID = SM.getOrCreateFileID(APINotesFile, SrcMgr::C_User);
  auto SourceBuffer = SM.getBufferOrNone(SourceFileID, SourceLocation());
  if (!SourceBuffer)
    return nullptr;

  // Compile the API notes source into a buffer.
  // FIXME: Either propagate OSType through or, better yet, improve the binary
  // APINotes format to maintain complete availability information.
  // FIXME: We don't even really need to go through the binary format at all;
  // we're just going to immediately deserialize it again.
  toolchain::SmallVector<char, 1024> APINotesBuffer;
  std::unique_ptr<toolchain::MemoryBuffer> CompiledBuffer;
  {
    SourceMgrAdapter SMAdapter(
        SM, SM.getDiagnostics(), diag::err_apinotes_message,
        diag::warn_apinotes_message, diag::note_apinotes_message, APINotesFile);
    toolchain::raw_svector_ostream OS(APINotesBuffer);
    if (api_notes::compileAPINotes(
            SourceBuffer->getBuffer(), SM.getFileEntryForID(SourceFileID), OS,
            SMAdapter.getDiagHandler(), SMAdapter.getDiagContext()))
      return nullptr;

    // Make a copy of the compiled form into the buffer.
    CompiledBuffer = toolchain::MemoryBuffer::getMemBufferCopy(
        StringRef(APINotesBuffer.data(), APINotesBuffer.size()));
  }

  // Load the binary form we just compiled.
  auto Reader = APINotesReader::Create(std::move(CompiledBuffer), SwiftVersion);
  assert(Reader && "Could not load the API notes we just generated?");
  return Reader;
}

std::unique_ptr<APINotesReader>
APINotesManager::loadAPINotes(StringRef Buffer) {
  toolchain::SmallVector<char, 1024> APINotesBuffer;
  std::unique_ptr<toolchain::MemoryBuffer> CompiledBuffer;
  SourceMgrAdapter SMAdapter(
      SM, SM.getDiagnostics(), diag::err_apinotes_message,
      diag::warn_apinotes_message, diag::note_apinotes_message, std::nullopt);
  toolchain::raw_svector_ostream OS(APINotesBuffer);

  if (api_notes::compileAPINotes(Buffer, nullptr, OS,
                                 SMAdapter.getDiagHandler(),
                                 SMAdapter.getDiagContext()))
    return nullptr;

  CompiledBuffer = toolchain::MemoryBuffer::getMemBufferCopy(
      StringRef(APINotesBuffer.data(), APINotesBuffer.size()));
  auto Reader = APINotesReader::Create(std::move(CompiledBuffer), SwiftVersion);
  assert(Reader && "Could not load the API notes we just generated?");
  return Reader;
}

bool APINotesManager::loadAPINotes(const DirectoryEntry *HeaderDir,
                                   FileEntryRef APINotesFile) {
  assert(!Readers.contains(HeaderDir));
  if (auto Reader = loadAPINotes(APINotesFile)) {
    Readers[HeaderDir] = Reader.release();
    return false;
  }

  Readers[HeaderDir] = nullptr;
  return true;
}

OptionalFileEntryRef
APINotesManager::findAPINotesFile(DirectoryEntryRef Directory,
                                  StringRef Basename, bool WantPublic) {
  FileManager &FM = SM.getFileManager();

  toolchain::SmallString<128> Path(Directory.getName());

  StringRef Suffix = WantPublic ? "" : "_private";

  // Look for the source API notes file.
  toolchain::sys::path::append(Path, toolchain::Twine(Basename) + Suffix + "." +
                                    SOURCE_APINOTES_EXTENSION);
  return FM.getOptionalFileRef(Path, /*Open*/ true);
}

OptionalDirectoryEntryRef APINotesManager::loadFrameworkAPINotes(
    toolchain::StringRef FrameworkPath, toolchain::StringRef FrameworkName, bool Public) {
  FileManager &FM = SM.getFileManager();

  toolchain::SmallString<128> Path(FrameworkPath);
  unsigned FrameworkNameLength = Path.size();

  StringRef Suffix = Public ? "" : "_private";

  // Form the path to the APINotes file.
  toolchain::sys::path::append(Path, "APINotes");
  toolchain::sys::path::append(Path, (toolchain::Twine(FrameworkName) + Suffix + "." +
                                 SOURCE_APINOTES_EXTENSION));

  // Try to open the APINotes file.
  auto APINotesFile = FM.getOptionalFileRef(Path);
  if (!APINotesFile)
    return std::nullopt;

  // Form the path to the corresponding header directory.
  Path.resize(FrameworkNameLength);
  toolchain::sys::path::append(Path, Public ? "Headers" : "PrivateHeaders");

  // Try to access the header directory.
  auto HeaderDir = FM.getOptionalDirectoryRef(Path);
  if (!HeaderDir)
    return std::nullopt;

  // Try to load the API notes.
  if (loadAPINotes(*HeaderDir, *APINotesFile))
    return std::nullopt;

  // Success: return the header directory.
  if (Public)
    ++NumPublicFrameworkAPINotes;
  else
    ++NumPrivateFrameworkAPINotes;
  return *HeaderDir;
}

static void checkPrivateAPINotesName(DiagnosticsEngine &Diags,
                                     const FileEntry *File, const Module *M) {
  if (File->tryGetRealPathName().empty())
    return;

  StringRef RealFileName =
      toolchain::sys::path::filename(File->tryGetRealPathName());
  StringRef RealStem = toolchain::sys::path::stem(RealFileName);
  if (RealStem.ends_with("_private"))
    return;

  unsigned DiagID = diag::warn_apinotes_private_case;
  if (M->IsSystem)
    DiagID = diag::warn_apinotes_private_case_system;

  Diags.Report(SourceLocation(), DiagID) << M->Name << RealFileName;
}

/// \returns true if any of \p module's immediate submodules are defined in a
/// private module map
static bool hasPrivateSubmodules(const Module *M) {
  return toolchain::any_of(M->submodules(), [](const Module *Submodule) {
    return Submodule->ModuleMapIsPrivate;
  });
}

toolchain::SmallVector<FileEntryRef, 2>
APINotesManager::getCurrentModuleAPINotes(Module *M, bool LookInModule,
                                          ArrayRef<std::string> SearchPaths) {
  FileManager &FM = SM.getFileManager();
  auto ModuleName = M->getTopLevelModuleName();
  auto ExportedModuleName = M->getTopLevelModule()->ExportAsModule;
  toolchain::SmallVector<FileEntryRef, 2> APINotes;

  // First, look relative to the module itself.
  if (LookInModule && M->Directory) {
    // Local function to try loading an API notes file in the given directory.
    auto tryAPINotes = [&](DirectoryEntryRef Dir, bool WantPublic) {
      if (auto File = findAPINotesFile(Dir, ModuleName, WantPublic)) {
        if (!WantPublic)
          checkPrivateAPINotesName(SM.getDiagnostics(), *File, M);

        APINotes.push_back(*File);
      }
      // If module FooCore is re-exported through module Foo, try Foo.apinotes.
      if (!ExportedModuleName.empty())
        if (auto File = findAPINotesFile(Dir, ExportedModuleName, WantPublic))
          APINotes.push_back(*File);
    };

    if (M->IsFramework) {
      // For frameworks, we search in the "Headers" or "PrivateHeaders"
      // subdirectory.
      //
      // Public modules:
      // - Headers/Foo.apinotes
      // - PrivateHeaders/Foo_private.apinotes (if there are private submodules)
      // Private modules:
      // - PrivateHeaders/Bar.apinotes (except that 'Bar' probably already has
      //   the word "Private" in it in practice)
      toolchain::SmallString<128> Path(M->Directory->getName());

      if (!M->ModuleMapIsPrivate) {
        unsigned PathLen = Path.size();

        toolchain::sys::path::append(Path, "Headers");
        if (auto APINotesDir = FM.getOptionalDirectoryRef(Path))
          tryAPINotes(*APINotesDir, /*wantPublic=*/true);

        Path.resize(PathLen);
      }

      if (M->ModuleMapIsPrivate || hasPrivateSubmodules(M)) {
        toolchain::sys::path::append(Path, "PrivateHeaders");
        if (auto PrivateAPINotesDir = FM.getOptionalDirectoryRef(Path))
          tryAPINotes(*PrivateAPINotesDir,
                      /*wantPublic=*/M->ModuleMapIsPrivate);
      }
    } else {
      // Public modules:
      // - Foo.apinotes
      // - Foo_private.apinotes (if there are private submodules)
      // Private modules:
      // - Bar.apinotes (except that 'Bar' probably already has the word
      //   "Private" in it in practice)
      tryAPINotes(*M->Directory, /*wantPublic=*/true);
      if (!M->ModuleMapIsPrivate && hasPrivateSubmodules(M))
        tryAPINotes(*M->Directory, /*wantPublic=*/false);
    }

    if (!APINotes.empty())
      return APINotes;
  }

  // Second, look for API notes for this module in the module API
  // notes search paths.
  for (const auto &SearchPath : SearchPaths) {
    if (auto SearchDir = FM.getOptionalDirectoryRef(SearchPath)) {
      if (auto File = findAPINotesFile(*SearchDir, ModuleName)) {
        APINotes.push_back(*File);
        return APINotes;
      }
    }
  }

  // Didn't find any API notes.
  return APINotes;
}

bool APINotesManager::loadCurrentModuleAPINotes(
    Module *M, bool LookInModule, ArrayRef<std::string> SearchPaths) {
  assert(!CurrentModuleReaders[ReaderKind::Public] &&
         "Already loaded API notes for the current module?");

  auto APINotes = getCurrentModuleAPINotes(M, LookInModule, SearchPaths);
  unsigned NumReaders = 0;
  for (auto File : APINotes) {
    CurrentModuleReaders[NumReaders++] = loadAPINotes(File).release();
    if (!getCurrentModuleReaders().empty())
      M->APINotesFile = File.getName().str();
  }

  return NumReaders > 0;
}

bool APINotesManager::loadCurrentModuleAPINotesFromBuffer(
    ArrayRef<StringRef> Buffers) {
  unsigned NumReader = 0;
  for (auto Buf : Buffers) {
    auto Reader = loadAPINotes(Buf);
    assert(Reader && "Could not load the API notes we just generated?");

    CurrentModuleReaders[NumReader++] = Reader.release();
  }
  return NumReader;
}

toolchain::SmallVector<APINotesReader *, 2>
APINotesManager::findAPINotes(SourceLocation Loc) {
  toolchain::SmallVector<APINotesReader *, 2> Results;

  // If there are readers for the current module, return them.
  if (!getCurrentModuleReaders().empty()) {
    Results.append(getCurrentModuleReaders().begin(),
                   getCurrentModuleReaders().end());
    return Results;
  }

  // If we're not allowed to implicitly load API notes files, we're done.
  if (!ImplicitAPINotes)
    return Results;

  // If we don't have source location information, we're done.
  if (Loc.isInvalid())
    return Results;

  // API notes are associated with the expansion location. Retrieve the
  // file for this location.
  SourceLocation ExpansionLoc = SM.getExpansionLoc(Loc);
  FileID ID = SM.getFileID(ExpansionLoc);
  if (ID.isInvalid())
    return Results;
  OptionalFileEntryRef File = SM.getFileEntryRefForID(ID);
  if (!File)
    return Results;

  // Look for API notes in the directory corresponding to this file, or one of
  // its its parent directories.
  OptionalDirectoryEntryRef Dir = File->getDir();
  FileManager &FileMgr = SM.getFileManager();
  toolchain::SetVector<const DirectoryEntry *,
                  SmallVector<const DirectoryEntry *, 4>,
                  toolchain::SmallPtrSet<const DirectoryEntry *, 4>>
      DirsVisited;
  do {
    // Look for an API notes reader for this header search directory.
    auto Known = Readers.find(*Dir);

    // If we already know the answer, chase it.
    if (Known != Readers.end()) {
      ++NumDirectoryCacheHits;

      // We've been redirected to another directory for answers. Follow it.
      if (Known->second && isa<DirectoryEntryRef>(Known->second)) {
        DirsVisited.insert(*Dir);
        Dir = cast<DirectoryEntryRef>(Known->second);
        continue;
      }

      // We have the answer.
      if (auto Reader = dyn_cast_if_present<APINotesReader *>(Known->second))
        Results.push_back(Reader);
      break;
    }

    // Look for API notes corresponding to this directory.
    StringRef Path = Dir->getName();
    if (toolchain::sys::path::extension(Path) == ".framework") {
      // If this is a framework directory, check whether there are API notes
      // in the APINotes subdirectory.
      auto FrameworkName = toolchain::sys::path::stem(Path);
      ++NumFrameworksSearched;

      // Look for API notes for both the public and private headers.
      OptionalDirectoryEntryRef PublicDir =
          loadFrameworkAPINotes(Path, FrameworkName, /*Public=*/true);
      OptionalDirectoryEntryRef PrivateDir =
          loadFrameworkAPINotes(Path, FrameworkName, /*Public=*/false);

      if (PublicDir || PrivateDir) {
        // We found API notes: don't ever look past the framework directory.
        Readers[*Dir] = nullptr;

        // Pretend we found the result in the public or private directory,
        // as appropriate. All headers should be in one of those two places,
        // but be defensive here.
        if (!DirsVisited.empty()) {
          if (PublicDir && DirsVisited.back() == *PublicDir) {
            DirsVisited.pop_back();
            Dir = *PublicDir;
          } else if (PrivateDir && DirsVisited.back() == *PrivateDir) {
            DirsVisited.pop_back();
            Dir = *PrivateDir;
          }
        }

        // Grab the result.
        if (auto Reader = Readers[*Dir].dyn_cast<APINotesReader *>())
          Results.push_back(Reader);
        break;
      }
    } else {
      // Look for an APINotes file in this directory.
      toolchain::SmallString<128> APINotesPath(Dir->getName());
      toolchain::sys::path::append(
          APINotesPath, (toolchain::Twine("APINotes.") + SOURCE_APINOTES_EXTENSION));

      // If there is an API notes file here, try to load it.
      ++NumDirectoriesSearched;
      if (auto APINotesFile = FileMgr.getOptionalFileRef(APINotesPath)) {
        if (!loadAPINotes(*Dir, *APINotesFile)) {
          ++NumHeaderAPINotes;
          if (auto Reader = Readers[*Dir].dyn_cast<APINotesReader *>())
            Results.push_back(Reader);
          break;
        }
      }
    }

    // We didn't find anything. Look at the parent directory.
    if (!DirsVisited.insert(*Dir)) {
      Dir = std::nullopt;
      break;
    }

    StringRef ParentPath = toolchain::sys::path::parent_path(Path);
    while (toolchain::sys::path::stem(ParentPath) == "..")
      ParentPath = toolchain::sys::path::parent_path(ParentPath);

    Dir = ParentPath.empty() ? std::nullopt
                             : FileMgr.getOptionalDirectoryRef(ParentPath);
  } while (Dir);

  // Path compression for all of the directories we visited, redirecting
  // them to the directory we ended on. If no API notes were found, the
  // resulting directory will be NULL, indicating no API notes.
  for (const auto Visited : DirsVisited)
    Readers[Visited] = Dir ? ReaderEntry(*Dir) : ReaderEntry();

  return Results;
}
