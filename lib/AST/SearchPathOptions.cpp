//===----------------------- SearchPathOptions.cpp ------------------------===//
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

#include "language/AST/SearchPathOptions.h"
#include "language/Basic/Assertions.h"
#include "toolchain/ADT/SmallSet.h"
#include "toolchain/Support/Errc.h"

using namespace language;

void ModuleSearchPathLookup::addFilesInPathToLookupTable(
    toolchain::vfs::FileSystem *FS, StringRef SearchPath, ModuleSearchPathKind Kind,
    bool IsSystem, unsigned SearchPathIndex) {
  std::error_code Error;
  auto entryAlreadyExists = [this](ModuleSearchPathKind Kind,
                                   unsigned SearchPathIndex) -> bool {
    return toolchain::any_of(LookupTable, [&](const auto &LookupTableEntry) {
      return toolchain::any_of(
          LookupTableEntry.second, [&](ModuleSearchPathPtr ExistingSearchPath) {
            return ExistingSearchPath->getKind() == Kind &&
                   ExistingSearchPath->getIndex() == SearchPathIndex;
          });
    });
  };
  assert(!entryAlreadyExists(Kind, SearchPathIndex) &&
         "Search path with this kind and index already exists");
  ModuleSearchPathPtr TableEntry =
      new ModuleSearchPath(SearchPath, Kind, IsSystem, SearchPathIndex);
  for (auto Dir = FS->dir_begin(SearchPath, Error);
       !Error && Dir != toolchain::vfs::directory_iterator(); Dir.increment(Error)) {
    StringRef Filename = toolchain::sys::path::filename(Dir->path());
    LookupTable[Filename].push_back(TableEntry);
  }
}

void ModuleSearchPathLookup::rebuildLookupTable(const SearchPathOptions *Opts,
                                                toolchain::vfs::FileSystem *FS,
                                                bool IsOSDarwin) {
  clearLookupTable();

  for (auto Entry : toolchain::enumerate(Opts->getImportSearchPaths())) {
    addFilesInPathToLookupTable(FS, Entry.value().Path,
                                ModuleSearchPathKind::Import,
                                Entry.value().IsSystem, Entry.index());
  }

  for (auto Entry : toolchain::enumerate(Opts->getFrameworkSearchPaths())) {
    addFilesInPathToLookupTable(FS, Entry.value().Path, ModuleSearchPathKind::Framework,
                                Entry.value().IsSystem, Entry.index());
  }

  for (auto Entry : toolchain::enumerate(Opts->getImplicitFrameworkSearchPaths())) {
    addFilesInPathToLookupTable(FS, Entry.value(),
                                ModuleSearchPathKind::ImplicitFramework,
                                /*isSystem=*/true, Entry.index());
  }

  for (auto Entry : toolchain::enumerate(Opts->getRuntimeLibraryImportPaths())) {
    addFilesInPathToLookupTable(FS, Entry.value(),
                                ModuleSearchPathKind::RuntimeLibrary,
                                /*isSystem=*/true, Entry.index());
  }

  State.FileSystem = FS;
  State.IsOSDarwin = IsOSDarwin;
  State.Opts = Opts;
  State.IsPopulated = true;
}

static std::string computeSDKPlatformPath(StringRef SDKPath,
                                          toolchain::vfs::FileSystem *FS) {
  if (SDKPath.empty())
    return "";

  SmallString<128> platformPath;
  if (auto err = FS->getRealPath(SDKPath, platformPath))
    toolchain::sys::path::append(platformPath, SDKPath);

  toolchain::sys::path::remove_filename(platformPath); // specific SDK
  toolchain::sys::path::remove_filename(platformPath); // SDKs
  toolchain::sys::path::remove_filename(platformPath); // Developer

  if (!toolchain::sys::path::filename(platformPath).ends_with(".platform"))
    return "";

  return platformPath.str().str();
}

std::optional<StringRef>
SearchPathOptions::getSDKPlatformPath(toolchain::vfs::FileSystem *FS) const {
  if (!SDKPlatformPath)
    SDKPlatformPath = computeSDKPlatformPath(getSDKPath(), FS);
  if (SDKPlatformPath->empty())
    return std::nullopt;
  return *SDKPlatformPath;
}

void SearchPathOptions::dump(bool isDarwin) const {
  toolchain::errs() << "Module import search paths:\n";
  for (auto Entry : toolchain::enumerate(getImportSearchPaths())) {
    toolchain::errs() << "  [" << Entry.index() << "] "
                 << (Entry.value().IsSystem ? "(system) " : "(non-system) ")
                 << Entry.value().Path << "\n";
  }

  toolchain::errs() << "Framework search paths:\n";
  for (auto Entry : toolchain::enumerate(getFrameworkSearchPaths())) {
    toolchain::errs() << "  [" << Entry.index() << "] "
                 << (Entry.value().IsSystem ? "(system) " : "(non-system) ")
                 << Entry.value().Path << "\n";
  }

  toolchain::errs() << "Implicit framework search paths:\n";
  for (auto Entry : toolchain::enumerate(getImplicitFrameworkSearchPaths())) {
    toolchain::errs() << "  [" << Entry.index() << "] " << Entry.value() << "\n";
  }

  toolchain::errs() << "Runtime library import search paths:\n";
  for (auto Entry : toolchain::enumerate(getRuntimeLibraryImportPaths())) {
    toolchain::errs() << "  [" << Entry.index() << "] " << Entry.value() << "\n";
  }

  toolchain::errs() << "(End of search path lists.)\n";
}

SmallVector<const ModuleSearchPath *, 4>
ModuleSearchPathLookup::searchPathsContainingFile(
    const SearchPathOptions *Opts, toolchain::ArrayRef<std::string> Filenames,
    toolchain::vfs::FileSystem *FS, bool IsOSDarwin) {
  if (!State.IsPopulated || State.FileSystem != FS ||
      State.IsOSDarwin != IsOSDarwin || State.Opts != Opts) {
    rebuildLookupTable(Opts, FS, IsOSDarwin);
  }

  // Gather all search paths that include a file whose name is in Filenames.
  // To make sure that we don't include the same search paths twice, keep track
  // of which search paths have already been added to Result by their kind and
  // Index in ResultIds.
  // Note that if a search path is specified twice by including it twice in
  // compiler arguments or by specifying it as different kinds (e.g. once as
  // import and once as framework search path), these search paths are
  // considered different (because they have different indices/kinds and may
  // thus still be included twice.
  toolchain::SmallVector<const ModuleSearchPath *, 4> Result;
  toolchain::SmallSet<std::pair<ModuleSearchPathKind, unsigned>, 4> ResultIds;

  for (auto &Filename : Filenames) {
    if (LookupTable.contains(Filename)) {
      for (auto &Entry : LookupTable.at(Filename)) {
        if (ResultIds.insert(std::make_pair(Entry->getKind(), Entry->getIndex()))
                .second) {
          Result.push_back(Entry.get());
        }
      }
    }
  }

  // Make sure we maintain the same search paths order that we had used in
  // populateLookupTableIfNecessary after merging results from
  // different filenames.
  toolchain::sort(Result, [](const ModuleSearchPath *Lhs,
                        const ModuleSearchPath *Rhs) { return *Lhs < *Rhs; });
  return Result;
}

/// Loads a VFS YAML file located at \p File using \p BaseFS and adds it to
/// \p OverlayFS. Returns an error if either loading the \p File failed or it
/// is invalid.
static toolchain::Error loadAndValidateVFSOverlay(
    const std::string &File,
    const toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> &BaseFS,
    const toolchain::IntrusiveRefCntPtr<toolchain::vfs::OverlayFileSystem> &OverlayFS) {
  auto Buffer = BaseFS->getBufferForFile(File);
  if (!Buffer)
    return toolchain::createFileError(File, Buffer.getError());

  auto VFS = toolchain::vfs::getVFSFromYAML(std::move(Buffer.get()), nullptr, File);
  if (!VFS)
    return toolchain::createFileError(File, toolchain::errc::invalid_argument);

  OverlayFS->pushOverlay(std::move(VFS));
  return toolchain::Error::success();
}

toolchain::Expected<toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem>>
SearchPathOptions::makeOverlayFileSystem(
    toolchain::IntrusiveRefCntPtr<toolchain::vfs::FileSystem> BaseFS) const {
  // TODO: This implementation is different to how Clang reads overlays in.
  // Expose a helper in Clang rather than doing this ourselves.

  auto OverlayFS =
      toolchain::makeIntrusiveRefCnt<toolchain::vfs::OverlayFileSystem>(BaseFS);

  toolchain::Error AllErrors = toolchain::Error::success();
  bool hasOverlays = false;
  for (const auto &File : VFSOverlayFiles) {
    hasOverlays = true;
    if (auto Err = loadAndValidateVFSOverlay(File, BaseFS, OverlayFS))
      AllErrors = toolchain::joinErrors(std::move(AllErrors), std::move(Err));
  }

  if (AllErrors)
    return std::move(AllErrors);

  if (hasOverlays)
    return OverlayFS;
  return BaseFS;
}
